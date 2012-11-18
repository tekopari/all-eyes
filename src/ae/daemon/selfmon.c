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
#include "ae.h"
#include "aedaemon.h"

/*
 * Only monarray[] only to this thread.
 */

/*
 * For the prototype, selfmon only checks whether
 * ae daemon is alive through the heartbeat messages.
 * In the next version, develop peer-process monitoring
 * algorithm.
 */

#define BUFSIZE 4096

void selfMon(int mode)
{
static char sbuf[BUFSIZE];
static char *msg="[:10:111111111111111-1:00:SF:]";
static char *response="[:10:111111111111111-1:11:AE:]";
int ret = -1;
// static char *msg2="HAVE NOT READ ANYTHING!!!!!!!!!!!!\n";
static char *msg3="selfmon read ERROR**********\n";

    /*
     *  Execute for ever.
     */
    memset(sbuf, 0, BUFSIZE);
    while (1)  {
        write(1, msg, strlen(msg)); // Send heart beat
        memset(sbuf, 0, BUFSIZE);
        while (1)  {
           sleep(25);
           // sleep(250); // SECURITY: for TESTING FOR restartMonitor ONLY;
           ret = read(0, sbuf, BUFSIZE);  // Wait to heat daemon's response
           if (ret < 0)  {
               aeDEBUG("selfmon: Something WRONG with ae daemon; Exiting by writing non-protocol message\n");
               aeLOG("selfmon: Something WRONG with ae daemon; Exiting by writing non-protocol message\n");
               write(1, msg3, strlen(msg3));
           } else if ( ret > 0)  {  // Got the response from daemon.
               // Check the validity of response.
               if (strncmp(sbuf, response, strlen(response)))  {
                   aeDEBUG("selfmon: Didn't get proper heartbeat message from ae daemon\n");
                   aeLOG("selfmon: Didn't get proper heartbeat message from ae daemon\n");
                   exit(SELFMON_EXIT);
               }
               break;
           }
        }
    }
}

