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
#include <sys/socket.h>

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
            monarray[i].status = MONITOR_NOT_RUNNING;
/***********  SECURITY RISK
            // Close other monitor's file descriptors.
            if (monarray[i].socFd[0] != AE_INVALID || monarray[i].socFd[0] != 0)
                close(monarray[i].socFd[0]);
            if (monarray[i].socFd[1] != AE_INVALID || monarray[i].socFd[1] != 0)
                close(monarray[i].socFd[1]);
            monarray[i].socFd[0] = AE_INVALID;
            monarray[i].socFd[1] = AE_INVALID;
            monarray[i].mode = AE_INVALID;
            monarray[i].span = AE_INVALID;
            monarray[i].pid = AE_INVALID;
            monarray[i].ppid = AE_INVALID;
            monarray[i].action = AE_INVALID;
            monarray[i].hbinterval = AE_INVALID;
            monarray[i].hbtime = AE_INVALID;
            monarray[i].sSSLsoc = AE_INVALID;
            monarray[i].cSSLsoc = AE_INVALID;
            monarray[i].hbtime = AE_INVALID;
            monarray[i].basedir = (char *)AE_INVALID;
            memset(&(monarray[i].aeCtx), 0, sizeof(monarray[i].aeCtx));
*************/
        }
    }
}

void
cleanOtherMons(pid_t pid)
{
int i;
    for(i=0; i < MAXMONITORS; i++)  {
        if(pid != (monarray[i].pid))  {
            cleanMon(pid);
        }
    }
}

void
spawnMonitor(MONCOMM *monPtr)
{
pid_t pid;

    if (monPtr->monPtr != NULL)  {
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
                monPtr->pid = getpid();

                // Zeroize other monitor's structure.
                cleanOtherMons(monPtr->pid);

                // close the parent's side of socketpair
                aeDEBUG("Monitor: Parent's closing Fd: %d\n", monPtr->socFd[0]);
                close(monPtr->socFd[0]);


                if (monPtr->socFd[1] != STDOUT_FILENO)  {
                    // set the STDOUT of the monitor to be daemon's socket.
                    if (dup2(monPtr->socFd[1], STDOUT_FILENO) != STDOUT_FILENO)  {
                        aeLOG("dup2 to set STDOUT failed for Monitor: %s\n", monPtr->name);
                        aeDEBUG("dup2 to set STDOUT failed for Monitor: %s\n", monPtr->name);
                        // Can't dup the socket.  Exit.
                        exit(FILE_DUP_ERROR);
                    }  else  {
                        aeDEBUG("dup2 set STDOUT PASSED for Monitor: %s\n", monPtr->name);
                    }  
                }

                if (monPtr->socFd[1] != STDIN_FILENO)  {
                    // set the STDIN of the monitor to be daemon's socket.
                    if (dup2(monPtr->socFd[1], STDIN_FILENO) != STDIN_FILENO)  {
                        aeLOG("dup2 set STDIN failed for Monitor: %s\n", monPtr->name);
                        aeDEBUG("dup2 set STDIN failed for Monitor: %s\n", monPtr->name);
                        // Can't dup the socket.  Exit.
                        exit(FILE_DUP_ERROR);
                    }  else  {
                        aeDEBUG("dup2 set STDIN PASSED for Monitor: %s\n", monPtr->name);
                    }  
                }

                // Ubuntu distro specific, since we duped, close the socFd[1] also.
                close(monPtr->socFd[1]);

                // Ubuntu specific delay
                sleep(5);
                // Mark this monitor as running.
                monPtr->status = MONITOR_RUNNING;

                // Hey..jump to monitor.
                (monPtr->monPtr)(monPtr->mode);

         }
         if (pid < 0)  { // fork error
             aeLOG("SpawnMonitor:  Cannot fork monitor: %s, Exit Code: %d\n", 
                                         monPtr->name, SPAWN_MONITOR_ERROR);
             return;
          } else  {
              // Parent Process.  Store child's PID, close child's soc.
              monPtr->pid = pid;
              monPtr->status = MONITOR_RUNNING;
/*************
                // close the child's side of socketpair
                // close(monPtr->socFd[1]);
                // SECURITY RISK: For debugging.  Remove this.
                if (strcmp(monPtr->name, "socketmon") == 0)  {
                    char buf[4096];
                    // close the child's side of socketpair
                    // close(monPtr->socFd[1]);
                    aeDEBUG("Daemon's socketmon Fd = %d", monPtr->socFd[0]);
                    write(monPtr->socFd[0], TEST_LINE, strlen(TEST_LINE));
                    aeLOG("SpawnMonitor:  Wrote to socketmon.  Reading now...\n");
                    memset(buf, 0, 2048);
                    read(monPtr->socFd[0], buf, 500);
                    buf[0] = 'T'; buf[1] = 'o'; buf[2] = 'd';buf[3] = 'd';
                    write(STDOUT_FILENO, buf, strlen(buf));
                }
                // SECURITY RISK: delete until here.
*************/
           }

    }
    else  {
        aeLOG("SpawnMonitor:  Cannot spawn monitor: %s, Exit Code: %d\n", 
                                         monPtr->name, SPAWN_MONITOR_ERROR);
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

    aeDEBUG("aedaemon-main: finished kicking off Monitors\n");

    // Spawn off a thread to take care of SSL client
    aemgrmgmt();

   // Sleep for 2 seconds for things to settle down.
   sleep(2);

    while (1)  {
        // aeDEBUG("aedaemon-main: calling monitormgmt\n");
        monitormgmt();
        sleep(5);
    }

    // Should never reach here.
    gracefulExit (GRACEFUL_EXIT_CODE);
    return 0;
}
