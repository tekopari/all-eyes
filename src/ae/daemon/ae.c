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

#define  DEBUG 1  // Leave the DEBUG flag for the prototype
#include "ae.h"
#include "aedaemon.h"

/*
 * aeconf.h file contains user configurable parameters.
 * NOTE: This file MUST NOT be included by another file, 
 * since it will give linker problems.
 */
#include "aeconf.h"

/*
 * This mutex is used to protect the monitor messages in the buffer being
 * modified while the SSL thread is reading those messages to ae Manager client.
 * In this particular project, it is the Android Client.
 */
pthread_mutex_t aeLock;

/*
 * The below global variable holds the userid of AE_USER.
 * The numerical value of AE_USER may vary from system to system.
 * Hence, it is filled-in during the initialization.
 */
static uid_t monUserId = AE_INVALID;
static uid_t monGroupId = AE_INVALID;


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
    printf("\t   -h  get help\n");
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
void aeSigHdlr(int sig)
{
    pid_t pid = AE_INVALID;
    int status = AE_INVALID;
    int i = 0;

    aeLOG("aeSigHdlr: Got signal: %d\n", sig);

    // Collect all the zombie process
    if (sig == SIGCHLD)  {
        while((pid = waitpid(-1, &status, WNOHANG)) >= 0)  {
            aeLOG("Child died.  Pid = %d\n", pid);
            for(i=0; i < MAXMONITORS; i++)  {
                if(monarray[i].pid == pid)  { 
                    monarray[i].status = MONITOR_NEEDS_RESPAWN;
                }
            }
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
    // SECURITY: Assign real signal handler sigact.sa_handler = aeSigHdlr;
    sigact.sa_handler = SIG_IGN;

    if (sigaction(SIGTERM, &sigact, NULL) < 0) {
        aeDEBUG ("sigaction for SIGTERM Failed");
        aeLOG ("sigaction for SIGTERM Failed");
        exit(SIGACTION_ERROR);
    }
    if (sigaction(SIGCHLD, &sigact, NULL) < 0) {
        aeDEBUG ("sigaction for SIGCHLD Failed");
        aeLOG ("sigaction for SIGCHLD Failed");
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

    aeDEBUG ("cleanMon: my pid = %d\n", getpid());

    /*      
     * Critical section.  Since we are reading monitor message, go get the aeLock.
     */
    if (pthread_mutex_lock(&aeLock) != 0)  {
        aeDEBUG("cleanMon: Unable to get aeLock.  errno = %d\n", errno);
        aeLOG("cleanMon: Unable to get aeLock.  errno = %d\n", errno);
    }

    /*
     * Cleanup only the required fileds in the Monitor structure.
     * For example, don't cleanout the function pointer - monPtr.
     */
    for(i=0; i < MAXMONITORS; i++)  {
        if ((pid > 0) && (pid == (monarray[i].pid)))  {
            aeDEBUG("CleanMon.  Cleaning pid=%d, monitor = %s\n", pid, monarray[i].name);
            aeLOG("CleanMon.  Cleaning pid=%d, monitor = %s\n", pid, monarray[i].name);
            monarray[i].status = MONITOR_NOT_RUNNING;
            /*
             * Close Paren't side of socket file descriptors.
             * Don't close socFd[1].  It has already been closed by the parent.
             * Closing socFd[1] will lead to bad Fd in getting socketpair in
             * restarting the monitor. 
             */
            if (monarray[i].socFd[0] != AE_INVALID || monarray[i].socFd[0] != 0)
                close(monarray[i].socFd[0]);
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
            memset((monarray[i].monMsg), 0, sizeof(monarray[i].monMsg));
            break;  // Done cleaning the monitor, get out.
        }
    }

    /*      
     * End of critical section.  Release the lock.
     */
    if (pthread_mutex_unlock(&aeLock) != 0)  {
        aeDEBUG("cleanMon: Unable to get aeLock.  errno = %d\n", errno);
        aeLOG("cleanMon: Unable to get aeLock.  errno = %d\n", errno);
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
     errno = 0;
     if (chroot(AE_CHROOT) != 0)  {
         aeLOG("CHROOT failed.  Exiting, errno = %d\n", errno);
         aeDEBUG("CHROOT failed.  Exiting, errno = %d\n", errno);
         gracefulExit(CHROOT_JAIL_ERROR);
     }
    aeDEBUG("dropPrivileges: AFTER-CHROOT dropping priviligeds to = %d\n", errno);

    /*
     * Drop the privileges before spawning the monitor.
     * Note that we drop both the real and effective userids.
     */
    aeDEBUG("dropPrivileges: uid = %d, gid = %d\n", monUserId, monGroupId);

    /*
     * get the gid, before setting before calling setuid.  Important.
     * Can't call in the reverse order.
     */
/*********************  SECURITY problem
    SECURITY:
    IMPORTANT:  This is related to a bug in Ubuntu 12.04 "Precise Gangolin"
    release.  This has already been posted to Ubuntu form.  Waiting for the
    answer.  Documented as defect #67 in the project.
    if (setgid(monGroupId) != 0)  {
        aeDEBUG("dropPrivileges: problem in setgid to = %s\n", monGroupId);
        aeLOG("dropPrivileges: problem in setgid to = %s\n", monGroupId);
        gracefulExit(DROP_PRIV_ERROR);
    }

    if (setegid(monGroupId) != 0)  {
        aeDEBUG("dropPrivileges: problem in setegid to = %s\n", monGroupId);
        aeLOG("dropPrivileges: problem in setegid to = %s\n", monGroupId);
        gracefulExit(DROP_PRIV_ERROR);
    }
    aeDEBUG("dropPrivileges: setgid successfully\n");
********************/

    if (setuid(monUserId) != 0)  {
        aeDEBUG("dropPrivileges: problem in setuid to = %s\n", monUserId);
        aeLOG("dropPrivileges: problem in setuid to = %s\n", monUserId);
        gracefulExit(DROP_PRIV_ERROR);
    }
    if (seteuid(monUserId) != 0)  {
        aeDEBUG("dropPrivileges: problem in seteuid to = %s\n", monUserId);
        aeLOG("dropPrivileges: problem in seteuid to = %s\n", monUserId);
        gracefulExit(DROP_PRIV_ERROR);
    }

    aeDEBUG("dropPrivileges: Exiting Successfully\n");
}

void getMonUserId()
{

    struct passwd *aePwdPtr = NULL;

    /*
     * Get the passwd structure pointer using the known user name in chroot-jail.
     * Then, set our effective user id to that, resulting in dropping our priviliges.
     */
    aePwdPtr = getpwnam(AE_USER);
    if (aePwdPtr == NULL)   {
        aeDEBUG("dropPrivileges returned NULL pointer - failed. errno = %d\n", errno);
        aeLOG("dropPrivileges returned NULL pointer - failed. errno = %d\n", errno);
        aeDEBUG("CHROOT failed.  Exiting, errno = %d\n", errno);
        gracefulExit(DROP_PRIV_ERROR);
    }  else  {
        monUserId = aePwdPtr->pw_uid;
        monGroupId = aePwdPtr->pw_gid;
        aeDEBUG("dropPrivileges got the MonUserID = %d, monGroupId = %d\n", monUserId, monGroupId);
    }
}

/*
 * Spwan a monitor.  This routine may get called from
 * multiple places.
 */
void spawnMonitor(MONCOMM *monPtr)
{
    pid_t pid = -1;

    if (monPtr == NULL)  {
        aeDEBUG("spawnMonitors: MONITOR POINTER IS NULL. Exiting\n");
        aeLOG("spawnMonitors: MONITOR POINTER IS NULL. Exiting\n");
        return;
    }

    aeDEBUG("spawnMonitors: Starting the work for: %s\n", monPtr->name);

    if (monPtr->monPtr != NULL)  {
        pid = -1;
        monPtr->mode = mode;
        monPtr->span = lifespan;
        monPtr->ppid = getpid();
        aeDEBUG("spawnMonitors: forking for: %s\n", monPtr->name);
        aeLOG("spawnMonitors: forking for: %s\n", monPtr->name);

        // Make sure to establish Socket pair.
        monPtr->socFd[0] = AE_INVALID;
        monPtr->socFd[1] = AE_INVALID;
        if (getSocPair(&(monPtr->socFd[0])) != 0)  {
             aeDEBUG("SpawnMonitor:  Cannot get socketpair: %s, Exit Code: %d\n", 
                                         monPtr->name, SPAWN_MONITOR_ERROR);
             aeLOG("SpawnMonitor:  Cannot get socketpair: %s, Exit Code: %d\n", 
                                         monPtr->name, SPAWN_MONITOR_ERROR);
             return;
        }

        aeDEBUG("spawnMonitors: GOT THE SOCKET PAIR SUCCESSFULLY.\n");

        pid = fork();
        if (pid == 0)  {

            /*
             * Ignore SIGCHLD signal.  This is important.
             * Without this, when one monitor exists, all other monitors get SIGCHLD,
             * in particular if SIGINT is used to kill a process.  Beware.
             * SECURITY issue.
             */
            struct sigaction sigact;
            memset (&sigact, 0, sizeof(sigact));
            sigact.sa_handler = SIG_IGN;

            if (sigaction(SIGCHLD, &sigact, NULL) < 0) {
                aeDEBUG ("sigaction for forked-child SIGCHLD Failed");
                aeLOG ("sigaction for forked-child SIGCHLD Failed");
                exit(SIGACTION_ERROR);
            }

            // Child Process
            monPtr->pid = getpid();

            // Zeroize other monitor's structure.
            cleanOtherMons(monPtr->pid);

            // close the parent's side of socketpair
            aeDEBUG("Monitor: Parent's closing Fd: %d\n", monPtr->socFd[0]);
            close(monPtr->socFd[0]);

            // Check the file descriptors before dup'ing them.
            aeDEBUG("Monitor: Checking My Open Fd: %d\n", monPtr->socFd[1]);
            errno = 0;
            if (fcntl(monPtr->socFd[1], F_GETFL) == -1)  {
               aeDEBUG("spawnMon: BAD socFd, errno: %d\n", errno);
               aeLOG("spawnMon: BAD socFd, errno: %d\n", errno);
               close(monPtr->socFd[1]);
               exit(FILE_DUP_ERROR);
            }
            if (fcntl(0, F_GETFL) == -1 )  {
               aeDEBUG("spawnMon: STDIN BAD errno = %d\n", errno);
               aeLOG("spawnMon: STDIN BAD errno = %d\n", errno);
               close(monPtr->socFd[1]);
               exit(FILE_DUP_ERROR);
            }
            if (fcntl(1, F_GETFL) == -1 )  {
               aeDEBUG("spawnMon: STDOUT BAD. errno = %d\n", errno); 
               aeLOG("spawnMon: STDOUT BAD. errno = %d\n", errno); 
               close(monPtr->socFd[1]);
               exit(FILE_DUP_ERROR);
            }

            errno = 0;
            // set the STDOUT of the monitor to be daemon's socket.
            if (dup2(monPtr->socFd[1], STDOUT_FILENO) != STDOUT_FILENO)  {
                aeDEBUG("dup2 set STDOUT failed for Monitor: %s, errno = %d\n", monPtr->name, errno);
                aeLOG("dup2 set STDOUT failed for Monitor: %s, errno = %d\n", monPtr->name, errno);
                // Can't dup the socket.  Exit.
                close(monPtr->socFd[1]);
                exit(FILE_DUP_ERROR);
            }  else  {
                aeDEBUG("dup2 set STDOUT PASSED for Monitor: %s\n", monPtr->name);
            }  

            errno = 0;
            // set the STDIN of the monitor to be daemon's socket.
            if (dup2(monPtr->socFd[1], STDIN_FILENO) != STDIN_FILENO)  {
                aeLOG("dup2 set STDIN failed for Monitor: %s, errno = %d\n", monPtr->name, errno);
                aeDEBUG("dup2 set STDIN failed for Monitor: %s, errno = %d\n", monPtr->name, errno);
                // Can't dup the socket.  Exit.
                close(monPtr->socFd[1]);
                exit(FILE_DUP_ERROR);
            }  else  {
                aeDEBUG("dup2 set STDIN PASSED for Monitor: %s\n", monPtr->name);
            }  

            // Ubuntu distro specific, must be done, since we duped, close the socFd[1] also.
            close(monPtr->socFd[1]);

            // Ubuntu specific delay
            sleep(0);

            // SECURITY: Drop our previliges.  MAKE SURE TO ENABLE THIS.
            dropPrivileges();
		
            // Initialize the heartbeat time.  SECURIT: Do error check.
            monPtr->hbtime = time(NULL);
            if (monPtr->hbtime < 0)  {
                aeDEBUG("Monitor: Problem in setting heatbeat time.  Exiting: %s\n", monPtr->name);
                aeLOG("Monitor: Problem in setting heatbeat time.  Exiting: %s\n", monPtr->name);
                    exit(TIME_SET_ERROR);
            }

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

    aeDEBUG("killing the monitor %s, pid = %d, mypid = %d\n", monPtr->name, monPtr->pid, getpid());
    aeLOG("killing the monitor %s, pid = %d, mypid = %d\n", monPtr->name, monPtr->pid, getpid());

    // SECURITY:  What is the time to send SIGKILL?
    if((ret = kill(monPtr->pid, SIGKILL)) != 0)  {
        aeDEBUG("Unable to kill the monitor %s\n", monPtr->name);
        aeLOG("Unable to kill the monitor %s\n", monPtr->name);
    }

    // Give it a second to die, to get SIGCHLD etc.
    sleep(1);
}

/*
 * Kill, and spawn a monitor.  This routine may get called from
 * multiple places.
 */
void restartMonitor (MONCOMM *monPtr)
{
    aeDEBUG("Restarting the monitor for %s\n",  monPtr->name);
    aeLOG("Restarting the monitor for %s\n",  monPtr->name);

    /*
     * If the monitor is already dead and marked to be respawn, don't try to kill it
     * Just clean it.
     */
    if (monPtr->status != MONITOR_NEEDS_RESPAWN)  {
        killMonitor(monPtr);
    }

    // Monitor being dead, now clean up the monitor sructure.
    cleanMon(monPtr->pid);

    aeDEBUG("restartMon: Finished killing and cleaningup the monitor: %s\n",  monPtr->name);
    aeDEBUG("ReSpawning the monitor for %s\n",  monPtr->name);
    aeLOG("ReSpawning the monitor for %s\n",  monPtr->name);
    spawnMonitor(monPtr);
    // SECURITY:  Issue # 46. spawnMonitor(monPtr);

    return;
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
    pthread_mutex_destroy(&aeLock);
    exit(exitcode);
}

int setupMutexLock(pthread_mutex_t *aeLock)
{
    pthread_mutexattr_t attr;

    // Set the Mutex to give error if the treated tries to get 
    memset(&attr, 0, sizeof(attr));
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
    if (pthread_mutex_init(aeLock, &attr) != 0)  {
        return AE_INVALID;
    }

    return AE_SUCCESS;
}


/*
 * Entry point of ae daemon.
 */
int main(int argc, char *argv[])
{
    int opt = 0;  // will contain getopt return value.

    /*
     * This program must be run as root.
     * If not executed as root, exit.
     * Use sudo, if user is not root.
     */
    if ((geteuid() != 0) || (getegid() != 0))  {
        fprintf(stderr, "\n\t *** ae daemon needs to run as root.  Exiting *** \n\n");
        exit(NOTROOT_EXIT);
    }

    /*
     * Log messages as "user logs", include the process id 
     */
    openlog (argv[0], (LOG_PID|LOG_NOWAIT), LOG_LOCAL6);
    aeLOG("Starting ae monitoring daemon\n");


    setupSigHandlers();  // Setup signal handlers.

    /*
     * Initialize the Mutex lock, used for protecting
     * Monitor messages in monitors.
     */
    if (setupMutexLock(&aeLock) == AE_INVALID)  {
        aeDEBUG("Error initializing Mutex lock\n");
        aeLOG("Error initializing Mutex lock\n");
        exit(MUTEX_INIT_ERROR);
    }


    /*
     * We only accept two parameter: -a and -p.
     * '-a' = Take action based on aeMgr's command.
     * '-p' = Persistant mode, keep the monitered data
     *        monitor restart.   
     */
    while((opt = getopt(argc, argv, "aph")) != -1) {
        switch(opt)  {
            case 'a':
                mode = MONITOR_ACTION_MODE;
                break;

            case 'p':
                lifespan = PERSISTENT;
                break;

            case 'h':
                 printHelp(GRACEFUL_EXIT_CODE);
                 gracefulExit (GRACEFUL_EXIT_CODE);
                break;

            default:
                 printHelp(EXIT_INVALID_PARAMETER);
                 break;
        }
    }

    if (mode == MONITOR_ACTION_MODE)  {
        aeDEBUG("ae:  Running in Monitor and Action mode\n");
        aeLOG("ae:  Running in Monitor and Action mode\n");
    }  else  {
        aeDEBUG("ae:  Running in Monitor only mode\n");
        aeLOG("ae:  Running in Monitor only mode\n");
    }

    
    /*
     * Get the numerical value of AE_USER.
     * This will be used in the setuid call of
     * dropping the previliges before giving the
     * control the children i.e. Monitors.
     */
    getMonUserId();  

    kickoffMonitors();  // Fork all configured monitors.

    aeDEBUG("aedaemon-main: finished kicking off Monitors\n");
    aeLOG("aedaemon-main: finished kicking off Monitors\n");

    // Sleep for 2 seconds for things to settle down.
    sleep(2);

    while (1)  {
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
