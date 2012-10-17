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
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <poll.h>

#define  DEBUG 1
#include "ae.h"
#include "aedaemon.h"

/*
 *  Put in code to manage the aemgr client.
 *  Should we allow only one at a time?
 */

static struct pollfd aePollFd[MAXMONITORS];

int
buildFd()
{
int i;
int index;

    index = 0;
    memset(aePollFd, 0, (MAXMONITORS * sizeof(struct pollfd)));
    for(i=0; i < MAXMONITORS; i++)  {
        if(monarray[i].status == MONITOR_RUNNING)  {
            aePollFd[index].fd = monarray[i].socFd[0];
            aePollFd[index].events = (POLLIN | POLLHUP);
            index++;
        }
    }

    return index;
}

void
monitormgmt()
{
int numFd;
int ret;
int i;

    /*
     * Build the pollfd array to determine which monitor's Fd to poll
     *   since some monitors may not be running.
     *   If no file descriptors to be monitored, just return.
     */
    if ( (numFd = buildFd()) == 0)
        return;

    ret = poll(aePollFd, numFd, 0);
    if (ret == -1)  {
        aeDEBUG("AeDaemon: Poll returned error.  Ret =  %d, errno = %d\n", ret, errno);
        return;
    }

    // Well, we got something to process
    for(i=0; i < numFd; i++)  {
        static char lBuf[4096];
        static char *helloBack = "[:10:11:AE:]";
        if(aePollFd[i].revents & POLLIN)  {
            aeDEBUG("monitor-manager: We got data to read\n");
            // We have data to read
            // For now, just read and send a simple response message.
            ret = read(aePollFd[i].fd, lBuf, 2048);
            ret = write(aePollFd[i].fd, helloBack, sizeof(helloBack));
        }
    } 
}
