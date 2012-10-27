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
#include <linux/fd.h>
#include <sys/ioctl.h>
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
 * Maximum of pollfd array for polling the I/O from monitors.
 */
static struct pollfd aePollFd[MAXMONITORS];



/*
 * Build the pollfd array for the poll system call.
 */
int
buildFd()
{
    int i = 0;
    int index = 0;

    memset(aePollFd, 0, (MAXMONITORS * sizeof(struct pollfd)));
    for(i=0; i < MAXMONITORS; i++)  {
        if(monarray[i].status == MONITOR_RUNNING)  {
            aePollFd[index].fd = monarray[i].socFd[0];

            // POLLRDHUP should work.  Due to Ubuntu specific problem, it is not being used.
            // aePollFd[index].events = (POLLIN | POLLRDHUP);
            aePollFd[index].events = (POLLIN | POLLHUP);
            // aeDEBUG("Will be polling for: %s, Fd = %d\n", monarray[i].name, monarray[i].socFd[0]);

            // Increment the number of Fd we will be polling.
            index++; 
        }
    }

    return index;
}

/*
 * Given a file descriptor, see whether it belongs to
 * any running monitor's socketpair file descriptor.
 * If there one, return a pointer to it, else return NULL.
 */
MONCOMM *
getMonFromFd(int fd)
{
    int i = 0;

    for(i=0; i < MAXMONITORS; i++)  {
        if(monarray[i].socFd[0] == fd)
            return &(monarray[i]);
    }
    return NULL;
}


/*
 * Check for input from monitors using poll system call.
 * If there is any input, process it.
 */
void monitormgmt()
{
    int numFd = AE_INVALID;
    int ret = -1;
    int i = 0;
    MONCOMM *m = NULL;

    // aeDEBUG("monitormgmt: entering monitormgmt \n");

    // For Debugging only
    // justDoOnemon();

    /*
     * Build the pollfd array to determine which monitor's Fd to poll
     * since some monitors may not be running.
     * If no file descriptors to be monitored, just return.
     */
    if ( (numFd = buildFd()) == 0)  {
        aeDEBUG("monitormgmt: no filedescriptor to poll for...................... \n");
        return;
    }
    // aeDEBUG("monitormgmt: Number of Fd = %d\n", numFd);

    // Poll waits for 100 milliseconds.
    ret = poll(aePollFd, numFd, 100);
    // aeDEBUG("monitormgmt: returned from POLL. ret = %d \n", ret);

    if (ret == -1)  {
        aeDEBUG("AeDaemon: Poll returned error.  Ret =  %d, errno = %d\n", ret, errno);
        return;
    }

    // If we are interrupted by a signal, nothing is valid, just return
    // Consider using ppoll
    if (errno == EINTR)  {
        return;
    }

    // Well, we got something to process
    for(i=0; i < numFd; i++)  {
        static unsigned int numMsg = 0;
        static char lBuf[4096];
        static char *helloBack = "[:10:11:AE:]\0";

        // aeDEBUG("Checking the POLLIN i = %d, revents = %x, POLLIN=%d\n", i, aePollFd[i].revents, POLLIN);

        // Get monitor structure pointer.
        m = getMonFromFd(aePollFd[i].fd);
        if (m == NULL)  {
            aeDEBUG("monitor polling without valid fd \n");
            aeLOG("ERROR: monitor polling without valid fd \n");
            continue;
        }

        // Check whether there is a poll error 
        // if((aePollFd[i].revents & POLLERR) || (aePollFd[i].revents & POLLHUP)) 
        if(aePollFd[i].revents & POLLERR)  {
            // Poll error.  Kill and respawn the monitor.
            aeDEBUG("monitor-manager: [ERROR] We got data to read\n");
            aeLOG("poll [ERROR]: data for the monitor %s\n", m->name);
            restartMonitor (m);
            continue;
        }

        if(aePollFd[i].revents & POLLIN)  {

            // aeDEBUG("Reading data for the monitor %s\n", m->name);

            // We have data to read
            // For now, just read and send a simple response message.
            memset(lBuf, 0, 2048);
            ret = read(aePollFd[i].fd, lBuf, 2048);
            if (ret < 0)  {
                aeDEBUG("Reading data for the monitor %s failed\n", m->name);
            }

            /*
             * Big black hole here.  Process the message from monitor.
             */

            // Increment the number of messages received.
            numMsg++;
            aeDEBUG("monitor-manager: data from: %s = %s, numMsg = %d \n", m->name, lBuf, numMsg);

            ret = write(aePollFd[i].fd, helloBack, strlen(helloBack));
            if (ret < 0)  {
                aeLOG("WRITING data for the monitor %s FAILED\n", m->name);
                aeDEBUG("WRITING data for the monitor %s FAILED\n", m->name);
                // SECURITY:  Should we kill the monitor at this point?
            } if (ret == 0)  {
                aeLOG("WROTE ONLY ZERO bytes for the monitor %s\n", m->name);
                aeDEBUG("WROTE ONLY ZERO bytes for the monitor %s\n", m->name);
                // SECURITY:  Should we kill the monitor at this point?
            }
            aeDEBUG("monitor-manager: wrote %d bytes to monitor %s\n", ret, m->name);
        }
    } 
}

/*
 * For Debug purpose only.
 * Hand modify this routine to run just one monitor for debugging purposes.
 */
void
justDoOnemon()
{
int i = 0;
int fd = AE_INVALID;

    for(i=0; i < MAXMONITORS; i++)  {
        if(strcmp(monarray[i].name,"selfmon") == 0)  {
        aeDEBUG("found the selfmon fd: %d", monarray[i].socFd[0]);
        fd = monarray[i].socFd[0];
            break;
        }
    }

    while (1)  {
        static char llbuf[4096];
        static char *hB = "[:10:11:AE:]";
        memset(llbuf, 0, 2048);
        aeDEBUG("whle loop reading from selfmon fd: %d", fd);
        read(fd, llbuf, 500);
        write(fd, hB, strlen(hB));

    }

}


