/*
 * Copyright (C) <2012> <Blair Wolfinger, Ravi Jagannathan, Thomas Pari, Todd Chu>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies 
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR 
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR 
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, 
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Original Author: Ravi Jagannathan
 */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <syslog.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <sys/select.h>
#include <sys/stat.h>

#define  DEBUG 1
#include "ae.h"
#include "aedaemon.h"

/*
 * A C include file.  This file contains compile time configuration parameters.
 */
#include "aeconf.c"

void
aeLOG(char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vsyslog(AE_SYSLOG_FLAGS, format, ap);
    va_end(ap);
}


void
printHelp(int eCode)
{
    printf("\t All Eyes (Version %s) usage:\n", AE_VERSION);
    printf("\t   %s -a|-p\n", AE_NAME);
    printf("\t     -a  monitor and take action\n");
    printf("\t     -p  keep mintor data across reboot (persistent)\n");
    printf("\t       default is monitor only, volatile\n");
    exit(eCode);
}

void
aeSigHdlr(int sig, siginfo_t *siginfo, void *context)
{
pid_t pid;
int status;

    aeLOG("aeSigHdlr: Got signal: %d\n", sig);
    // Collect all the zombie process
    while((pid = waitpid(-1, &status, WNOHANG)) >= 0)  {
        aeLOG("Child died.  Pid = %d\n", pid);
        cleanMon(pid);
    }
}

void
setupSigHandlers()
{
struct sigaction sigact;

    memset (&sigact, 0, sizeof(sigact));
    sigact.sa_sigaction = aeSigHdlr;

    if (sigaction(SIGTERM, &sigact, NULL) < 0) {
        perror ("sigaction for SIGTERM Failed");
        exit(SIGACTION_ERROR);
    }
    if (sigaction(SIGCHLD, &sigact, NULL) < 0) {
        perror ("sigaction for SIGCHLD Failed");
        exit(SIGACTION_ERROR);
    }

    // SECURITY:  catach all the signal.  Except for debug
}

void
cleanMon(pid_t pid)
{
int i;
    for(i=0; i < MAXMONITORS; i++)  {
        if(pid == (monarray[i].pid))  {
            memset(&monarray[i], 0, sizeof(MAXMONITORS));
        }
    }
}

void
cleanOtherMons(pid_t pid)
{
int i;
    for(i=0; i < MAXMONITORS; i++)  {
        if(pid != (monarray[i].pid))  {
            // Close other monitor's file descriptors.
            if (monarray[i].socFd[1] != AE_INVALID || monarray[i].socFd[1] != 0)
                close(monarray[i].socFd[1]);
            memset(&monarray[i], 0, sizeof(MAXMONITORS));
            monarray[i].status = MONITOR_NOT_RUNNING;
        }
    }
}

void
spawnMonitor(MONCOMM *monPtr)
{
pid_t pid;

    if (monPtr->monPtr == NULL && monPtr->execpath == NULL)  {
        aeLOG("SpawnMonitor:  monitor: %s has NO monitor,  Exit Code: %d\n", 
                                         monPtr->name, MONITOR_CONFIG_ERROR);
        return;
    }

    if (monPtr->monPtr != NULL && monPtr->execpath != NULL)  {
        aeLOG("SpawnMonitor:  monitor: %s has both function and exec path,  Exit Code: %d\n", 
                                         monPtr->name, MONITOR_CONFIG_ERROR);
        return;
    }

    if (monPtr->monPtr != NULL && monPtr->forkorexec != JUST_FORK)  {
        aeLOG("SpawnMonitor:  monitor: %s function set, but is not JUST_FORK,  Exit Code: %d\n", 
                                         monPtr->name, MONITOR_CONFIG_ERROR);
        return;
    }

    if (monPtr->execpath != NULL && monPtr->forkorexec != FORK_EXEC)  {
        aeLOG("SpawnMonitor:  monitor: %s execpath is set, but is not FORK_EXEC,  Exit Code: %d\n", 
                                         monPtr->name, MONITOR_CONFIG_ERROR);
        return;
    }

    if (monPtr->monPtr != NULL || monPtr->execpath != NULL)  {
        pid = -1;
        monPtr->mode = mode;
        monPtr->span = lifespan;
        monPtr->ppid = getpid();
        aeDEBUG("spawnMonitors: forking for: %s\n", monPtr->name);

        // Make sure to establish Secure Socket.
        if (getSocPair(&(monPtr->socFd[0])) < 0)  {
             aeLOG("SpawnMonitor:  Cannot get socketpair: %s, Exit Code: %d\n", 
                                         monPtr->name, SPAWN_MONITOR_ERROR);
             return;
        }
        pid = fork();
        if (pid == 0)  {
            // Child Process
               // Zeroize other monitor's structure.
                monPtr->pid = getpid();
                monPtr->status = MONITOR_RUNNING;
                cleanOtherMons(monPtr->pid);
                // close the parent's side of socketpair
                close(monPtr->socFd[0]);

                // If we are just forking, call the monitor entry point (written in C).
                // Else, exec the file.
                if (monPtr->forkorexec == JUST_FORK)
                    (monPtr->monPtr)();
                if (monPtr->forkorexec == FORK_EXEC)  {
                    // FORK_EXEC case
                    // SECURITY: Check the integrity of the execing file.
                    // SECURITY: Do we want to keep the monitor exec path encrypted?
                    aeDEBUG("Exec'ing a Monitor\n");
                    execl(monPtr->execpath, "monitor",  NULL);
                }  else  {
                    aeLOG("SpawnMonitor:  monitor: %s is not configured properly,  Exit Code: %d\n", 
                                         monPtr->name, MONITOR_CONFIG_ERROR);
                    exit(MONITOR_CONFIG_ERROR);
                }
         }
         if (pid < 0)  { // fork error
             aeLOG("SpawnMonitor:  Cannot fork monitr: %s, Exit Code: %d\n", 
                                         monPtr->name, SPAWN_MONITOR_ERROR);
             return;
          } else  {
              // Parent Process.  Store child's PID, close child's soc.
              monPtr->pid = pid;
              monPtr->status = MONITOR_RUNNING;
                // close the child's side of socketpair
                close(monPtr->socFd[1]);
           }
    }
}

void
kickoffMonitors()
{
int i;
    for(i=0; i < MAXMONITORS; i++)  {
        spawnMonitor(&monarray[i]);
    }
}

void
gracefulExit(int exitcode)
{
int i;
    for(i=0; i < MAXMONITORS; i++)  {
        if (monarray[i].pid != 0)
            kill(monarray[i].pid, SIGTERM);
    }

    // Wait for 5 seconds.  If the children are not dead, do kill -9.
    // What if the time for each monitor wait time varies?  Should it be in monitor struct?
    aeLOG("gracefulExit: Exiting gracefully");
    exit(exitcode);
}


int
main(int argc, char *argv[])
{
    int opt;

    // Log messages, including this process id as user log messages.
    openlog (argv[0], (LOG_PID|LOG_NOWAIT), LOG_LOCAL6);
    aeLOG("Starting ae monitoring daemon\n");
    setupSigHandlers();


    while((opt = getopt(argc, argv, "ap")) != -1) {
        switch(opt)  {
            case 'a':
                mode = MONITOR_ACTION_MODE;
                break;

            case 'p':
                lifespan = PERSISTENT;
                break;

            default:
                 printHelp(EXIT_INVALID_PARAMETER);
                 break;
        }
    }

    kickoffMonitors();

    while (1)  {
        /*
         * 0.  This while loop must sleep most of the time.  Do as little processing as possible.
         * 1.  Do children heartbeat
         * 2.  Wait for Client's connection (Android app).
         * 3.  Must go to sleep
         */

        /*
         * what if a Monitor dies?
         */
    }

    gracefulExit (GRACEFUL_EXIT_CODE);
    return 0;
}
