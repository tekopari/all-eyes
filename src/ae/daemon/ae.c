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
#include <pwd.h>
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
 * A .c include file.  This file contains compile time configuration parameters.
 * Separated out for convenience.
 */
#include "aeconf.c"

/*
 * This utility function logs messages in /var/log/syslog(Ubuntu) file.
 */
void aeLOG(char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    vsyslog(AE_SYSLOG_FLAGS, format, ap);
    va_end(ap);
}


/*
 * Print help and exit.
 */
void printHelp(int eCode)
{
    printf("\t All Eyes (Version %s) usage:\n", AE_VERSION);
    printf("\t   %s -a|-p\n", AE_NAME);
    printf("\t   -a  monitor and take action\n");
    printf("\t   -p  keep mintor data across reboot (persistent)\n");
    printf("\t       default is monitor only, volatile\n");
    exit(eCode);
}

/*
 * Given a pid, if a monitor of that pid exists,
 * return the pointer to that monitor structure.
 * If none exist, return NULL.
 */  
MONCOMM *getMonPtr(pid_t pid)
{
    int i = 0;;

    for(i=0; i < MAXMONITORS; i++)  {
        if(monarray[i].pid == pid)  {
            return &(monarray[i]);
        }
    }

    return NULL;
}

/*
 * Generic Signal Handler for ae.
 * SECURITY:  Note that this also may get inherited by monitors.
 * Every time we catch a signal, we also call waitpid to ensure
 * zombie process entries are not left.
 */
void aeSigHdlr(int sig, siginfo_t *siginfo, void *context)
{
    pid_t pid;
    int status;
    MONCOMM *monPtr = NULL;

    aeLOG("aeSigHdlr: Got signal: %d\n", sig);

    // Collect all the zombie process
    while((pid = waitpid(-1, &status, WNOHANG)) >= 0)  {
        aeLOG("Child died.  Pid = %d\n", pid);
        cleanMon(pid);

        /*
         * If one of the monitor had died, respawn it.
         * SECURITY:  Should we check whether we received SIGCHLD for that monitor,
         * before spawning a monitor?
         */
        if(monPtr != NULL)  {
            monPtr = getMonPtr(pid);
            spawnMonitor(monPtr);
        }
    }
}

/*
 * SECURITY:  sets up signal handlers.
 * More signals should be caught.
 */
void setupSigHandlers()
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

/*
 * Clean a monitor's structure, provided one exists
 * of the given pid.  Note that this function DOES NOT
 * set the monitor's function poiner to NULL, since
 * it will be required when we need to re-spawn the monitor.
 */
void cleanMon(pid_t pid)
{
    int i = 0;

    for(i=0; i < MAXMONITORS; i++)  {
        if(pid == (monarray[i].pid))  {
            monarray[i].status = MONITOR_NOT_RUNNING;
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
            memset(&(monarray[i].monMsg), 0, sizeof(monarray[i].monMsg));
        }
    }
}

/*
 * This routine is called from the Monitor, after ae-daemon forks, but
 * before the control is given to the monitor.  We don't want the
 * child monitor to know about the function entry points of other
 * monitors.  Hence, in addition to cleaning the monitor structure,
 * set the monitor function pointer to NULL.
 */
void cleanOtherMons(pid_t pid)
{
    int i = 0;

    for(i=0; i < MAXMONITORS; i++)  {
        if(pid != (monarray[i].pid))  {
            cleanMon(monarray[i].pid);
            /* Important:  SECURITY: Null out the monitor function, so except the daemon,
             * monitor functions do not know the function pointers of other
             * monitor's entry point.
             */
            monarray[i].monPtr = NULL;
        }
    }
}

/*
 * Get into chroot, drop our previliges.
 */
void dropPrivileges()
{
    /*
     * SECURITY, IMPORTANT:
     * This program must be invoked within chroot jail with root permission
     */
     if (chroot(AE_CHROOT) != 0)  {
         aeLOG("CHROOT failed.  Exiting, errno = %d\n", errno);
         aeDEBUG("CHROOT failed.  Exiting, errno = %d\n", errno);
         gracefulExit(CHROOT_JAIL_ERROR);
     }

#ifndef DEBUG
    /*
     * Drop the privileges before spawning the monitor.
     * Get the passwd structure pointer using the known user name in chroot-jail.
     * Then, set our effective user id to that, resulting in dropping our priviliges.
     */
     aePwdPtr = getpwnam(AE_USER);
     setuid(aePwdPtr->pw_uid);
#endif
}

/*
 * Spwan a monitor.  This routine may get called from
 * multiple places.
 */
void spawnMonitor(MONCOMM *monPtr)
{
    pid_t pid = -1;

    if (monPtr->monPtr != NULL)  {
        pid = -1;
        monPtr->mode = mode;
        monPtr->span = lifespan;
        monPtr->ppid = getpid();
        aeDEBUG("spawnMonitors: forking for: %s\n", monPtr->name);
        aeLOG("spawnMonitors: forking for: %s\n", monPtr->name);

        // Make sure to establish Secure Socket.
        if (getSocPair(&(monPtr->socFd[0])) < 0)  {
             aeLOG("SpawnMonitor:  Cannot get socketpair: %s, Exit Code: %d\n", 
                                         monPtr->name, SPAWN_MONITOR_ERROR);
             return;
        }
        pid = fork();
        if (pid == 0)  {
#ifndef DEBUG
            struct passwd *aePwdPtr = NULL;
#endif

            // Make sure to open the syslog.

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

                // Ubuntu distro specific, must be done, since we duped, close the socFd[1] also.
                close(monPtr->socFd[1]);

                // Ubuntu specific delay
                sleep(0);

                // SECURITY: Drop our previliges.  MAKE SURE TO ENABLE THIS.
                dropPrivileges();
		
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

              // close the child's side of socketpair
              close(monPtr->socFd[1]);

          }

    }
    else  {
        aeLOG("SpawnMonitor:  Cannot spawn monitor: %s, Exit Code: %d\n", 
                                         monPtr->name, SPAWN_MONITOR_ERROR);
    }
}

/*
 * Loop through the monitor strucuture and
 * spawn all configured monitors.
 */
void kickoffMonitors()
{
    int i = 0;;

    for(i=0; i < MAXMONITORS; i++)  {
        spawnMonitor(&monarray[i]);
    }
}


/*
 * Kill a monitor; then clean it up.  This routine may get called from
 * multiple places.
 */
void killMonitor(MONCOMM *monPtr)
{
    int ret = -1;

    if(monPtr == NULL)
        return;

    // SECURITY:  What is the time to send SIGKILL?
    if((ret = kill(monPtr->pid, SIGINT)) != 0)  {
        aeDEBUG("Unable to kill the monitor %s\n", monPtr->name);
        aeDEBUG("monitor-manager: We got data to read\n");
    }

    // Clean up the killed monitor.
    cleanMon(monPtr->pid);
}

/*
 * Kill, and spawn a monitor.  This routine may get called from
 * multiple places.
 * SECURITY:  For now this function just returns.  Fix it.
 */
void restartMonitor (MONCOMM *monPtr)
{
    aeDEBUG("Restarting the monitor for %s\n",  monPtr->name);
    aeLOG("Restarting the monitor for %s\n",  monPtr->name);
    return;

/*** SECURITY:
    killMonitor(monPtr);
    spawnMonitor(monPtr);
***/
}

/*
 *  Ideally, ae-daemon and monitors should never exit.
 *  However, in case there is a catastropy and ae-daemon exists,
 *  make sure it exits gracefully.
 */
void gracefulExit(int exitcode)
{
    int i = 0;

    for(i=0; i < MAXMONITORS; i++)  {
        if ((monarray[i].pid != 0) || (monarray[i].pid != AE_INVALID))
            kill(monarray[i].pid, SIGTERM);
    }

    // Wait for 5 seconds.  If the children are not dead, do kill -9.
    // What if the time for each monitor wait time varies?  Should it be in monitor struct?
    aeDEBUG("gracefulExit: Exiting gracefully\n");
    aeLOG("gracefulExit: Exiting gracefully\n");
    exit(exitcode);
}

/*
 * Entry point of ae daemon.
 */
int main(int argc, char *argv[])
{
    int opt = 0;;

    /*
     * Log messages, including this process id as user log messages.
     */
    openlog (argv[0], (LOG_PID|LOG_NOWAIT), LOG_LOCAL6);
    aeLOG("Starting ae monitoring daemon\n");
    setupSigHandlers();


    /*
     * We only accept two parameter: -a and -p.
     */
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

    // Spawn all configured monitors.
    kickoffMonitors();

    aeLOG("aedaemon-main: finished kicking off Monitors\n");
    aeDEBUG("aedaemon-main: finished kicking off Monitors\n");

    // Sleep for 2 seconds for things to settle down.
    sleep(2);

    while (1)  {
        // If necessary,spawn off a thread to take care of SSL client.
        // aeDEBUG("aedaemon-main: Starting aeMgrMgmt\n");
        aeMgrMgmt();

        // aeDEBUG("aedaemon-main: calling monitormgmt\n");
        monitormgmt();
        sleep(1);
    }

    // Should never reach here.
    gracefulExit (GRACEFUL_EXIT_CODE);
    return 0;
}
