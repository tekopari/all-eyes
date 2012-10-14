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

/*
 *  Declare monitors prototypes
 */
MONCOMM monarray[MAXMONITORS] = {
    {
        .name = "selfmon",
        .mode = 0,
        .span = 0,
        .status = 0,
        .ppid = 0,
        .basedir = NULL,
        .socFd[0] = 0,
        .socFd[1] = 0,
        .monPtr = selfMon
    },
    {
        .name = "socketmon",
        .mode = 0,
        .span = 0,
        .status = 0,
        .ppid = 0,
        .basedir = NULL,
        .socFd[0] = 0,
        .socFd[1] = 0,
        .monPtr = NULL
    },
    {
        .name = "binmon",
        .mode = 0,
        .span = 0,
        .status = 0,
        .ppid = 0,
        .basedir = NULL,
        .socFd[0] = 0,
        .socFd[1] = 0,
        .monPtr = NULL
    },
    {
        .name = "filemon",
        .mode = 0,
        .span = 0,
        .status = 0,
        .ppid = 0,
        .basedir = NULL,
        .socFd[0] = 0,
        .socFd[1] = 0,
        .monPtr = NULL
    }
};

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
    aeLOG("aeSigHdlr: Got signal: %d\n", sig);
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
}

static unsigned int mode = MONITOR_MODE;
static unsigned int lifespan = VOLATILE;

void
spawnMonitors(void)
{
int i;
pid_t pid;

    for(i=0; i < MAXMONITORS; i++)  {
        if (monarray[i].monPtr != NULL)  {
            pid = -1;
            monarray[i].mode = mode;
            monarray[i].span = lifespan;
            monarray[i].ppid = getpid();
            aeDEBUG("spawnMonitors: forking for: %s\n", monarray[i].name);
            if(getSocPair((monarray[i].socFd)) < 0)  {
                 gracefulExit(RESOURCE_UNAVAIL_EXIT);
            }
            pid = fork();
            if (pid == 0)  {
                // Child Process
                close(monarray[i].socFd[0]);
                (monarray[i].monPtr)();
            }
            if (pid < 0)  {
                perror("SpawnMonitors: Unable to Spawn threads.  Exiciting");
                exit(SPAWN_MONITOR_ERROR);
            } else  {
                // Parent Process.  Store child's PID, close child's soc.
                monarray[i].pid = pid;
                close(monarray[i].socFd[1]);
            }
        }
    }
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
                 printHelp(1);
                 break;
        }
    }

    spawnMonitors();

    while (1)  {
        /*
         * 1.  Do children heartbeat
         * 2.  Wait for Client's connection (Android app).
         */

        /*
         * what if a Monitor dies?
         */
    }
    
    return 0;
}
