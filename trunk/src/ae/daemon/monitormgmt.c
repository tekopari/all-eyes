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
#include "aemsg.h"

/*
 * Buffer for Monitor messages.
 */
char monitorMsg[NUM_OF_MONITOR_MSGS][MONITOR_MSG_BUFSIZE];
unsigned int monMsgIndex = 0;

/*
 * Maximum of pollfd array for polling the I/O from monitors.
 */
static struct pollfd aePollFd[MAXMONITORS];

/*
 * Build the pollfd array for the poll system call.
 */
int buildFd()
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
 * Check first whether the monitor needs respawn as it might have died.
 * Check whether the monitor sent any message in the last AE_HEARTBEAT_INTERVAL seconds
 */
void monHeartbeatCheck()
{
    int i = 0;
    time_t t = AE_INVALID;
    time_t interval = AE_INVALID;

    t = time(NULL);
    if (t < 0)  {
        // aeDEBUG("processMonitorMsg: error getting time: errno =%d\n", errno);
        aeLOG("processMonitorMsg: error getting time: errno =%d\n", errno);
        return;
    }

    for(i=0; i < MAXMONITORS; i++)  {

        // If a monitor needs respawning, do it.
        if(monarray[i].status == MONITOR_NEEDS_RESPAWN)  {
            restartMonitor (&(monarray[i]));
        }

        // If a monitor is not running, leave it alone.
        if(monarray[i].status != MONITOR_RUNNING)
            continue;

        // aeDEBUG("monHeartbeatCheck: time = %d, hbtime = %d, name=%s\n", t, monarray[i].hbtime, monarray[i].name);
        // If we didn't receive heart message within 30 seconds, restart the monitor.
        interval = (t - monarray[i].hbtime);
        if (interval > AE_HEARTBEAT_INTERVAL)  {
            aeDEBUG("monHeartbeatCheck: late heartbeat msg.  Restarting the monitor = %s, interval = %d\n", monarray[i].name, interval);
            aeLOG("monHeartbeatCheck: late heartbeat msg.  Restarting the monitor = %s, interval = %d\n", monarray[i].name, interval);
            restartMonitor (&(monarray[i]));
        }

    }
}

/*
 * Given a file descriptor, see whether it belongs to
 * any running monitor's socketpair file descriptor.
 * If there one, return a pointer to it, else return NULL.
 */
MONCOMM * getMonFromFd(int fd)
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

    // Well, we got something to process from monitor(s).
    for(i=0; i < numFd; i++)  {
        static unsigned int numMsg = 0;
        static char lBuf[MONITOR_MSG_BUFSIZE];
        static char oBuf[MONITOR_MSG_BUFSIZE];

        // aeDEBUG("Checking the POLLIN i = %d, revents = %x, POLLIN=%d\n", i, aePollFd[i].revents, POLLIN);

        // Get monitor structure pointer.
        m = getMonFromFd(aePollFd[i].fd);
        if (m == NULL)  {
            aeDEBUG("monitor polling without valid fd \n");
            aeLOG("ERROR: monitor polling without valid fd \n");
            continue;
        }

        /*
         * If the monitor is not running when we come back from poll,
         * don't process anything for the monitor.
         */
        if (m->status != MONITOR_RUNNING)  {
            aeDEBUG("After poll found monitor %s is not running\n", m->name);
            aeLOG("After poll found monitor %s is not running\n", m->name);
            continue;
        }

        // Check whether there is a poll error 
        if((aePollFd[i].revents & POLLERR) || (aePollFd[i].revents & POLLHUP))  {
            // Poll error.  Kill and respawn the monitor.
            aeDEBUG("monitor-manager: [ERROR] We got data to read\n");
            aeLOG("poll [ERROR]: data for the monitor %s\n", m->name);
            m->status = MONITOR_NEEDS_RESPAWN;
            continue;
        }

        if(aePollFd[i].revents & POLLIN)  {

            // aeDEBUG("Reading data for the monitor %s\n", m->name);

            // We have data to read
            // For now, just read and send a simple response message.
            memset(lBuf, 0, MONITOR_MSG_BUFSIZE);
            memset(oBuf, 0, MONITOR_MSG_BUFSIZE);
            ret = read(aePollFd[i].fd, lBuf, MAX_MONITOR_MSG_LENGTH);
            if (ret < 0)  {
                aeDEBUG("Reading data for the monitor %s failed\n", m->name);
                aeLOG("Reading data for the monitor %s failed\n", m->name);
                aeDEBUG("Restarting the monitor for %s\n",  m->name);
                aeLOG("Restarting the monitor for %s\n",  m->name);
                m->status = MONITOR_NEEDS_RESPAWN;
            }

            if (ret == 0)  {
                aeLOG("Read ONLY ZERO bytes for the monitor %s\n", m->name);
                aeDEBUG("Read ONLY ZERO bytes for the monitor %s\n", m->name);
                // SECURITY: There is nothing to read 
                continue;
            }

            // Increment the number of messages received.
            numMsg++;
            aeDEBUG("monitor-manager: data from: %s = %s, numMsg = %d \n", m->name, lBuf, numMsg);

            // Make sure to null terminate the message from monitors, before processing.
            lBuf[MONITOR_MSG_BUFSIZE -1] = '\0';

            // If it s duplicate message form a monitor, restart the monitor.
            if (isDuplicateMsg(lBuf) == AE_INVALID)  {
                aeDEBUG("Duplicate msg from monitor: %s", m->name);
                aeLOG("Duplicate msg from monitor: %s", m->name);
                m->status = MONITOR_NEEDS_RESPAWN;
                continue;
            }

            // Process the message from monitor.  Log the invalid message receive.
            ret = processMonitorMsg(m, lBuf, oBuf);
            if (ret == AE_INVALID)  {
                // SECURITY:  Should we kill the monitor since it sent an invalid message?
                aeDEBUG("invalid msg from monitor: %s", m->name);
                aeLOG("invalid msg from monitor: %s", m->name);
                continue;
            }

            /*
             * For the prototype, we only send one type of response to all
             * the monitors.  In the future, this might change.
             */
            ret = write(aePollFd[i].fd, oBuf, strlen(oBuf));
            if (ret < 0)  {
                aeLOG("WRITING data for the monitor %s FAILED\n", m->name);
                aeDEBUG("WRITING data for the monitor %s FAILED\n", m->name);
                aeDEBUG("Restarting the monitor for %s\n",  m->name);
                aeLOG("Restarting the monitor for %s\n",  m->name);
                m->status = MONITOR_NEEDS_RESPAWN;
            } 
            // aeDEBUG("monitor-manager: wrote %d bytes to monitor %s\n", ret, m->name);
        }
    } 

    // Check the monitors have been sending their heartbeat.
    monHeartbeatCheck();
}

/*
 * Process the message over socketpair from a monitor.
 * After identifying the message is from the right monitor,
 * i.e. "SM" message should be from Socket Monitor,
 * the message is stored in MONCOMM structure.
 */
int processMonitorMsg(MONCOMM *m, char *msg, char *out)
{
    static char lBuf[MONITOR_MSG_BUFSIZE];
    time_t t = AE_INVALID;
    AEMSG aeMsg;

    if ((m == NULL) || (msg == NULL))  {
        aeDEBUG("processMonitorMsg: received invalid parameters: m=%x, msg=%x", m, msg);
        aeLOG("processMonitorMsg: received invalid parameters: m=%x, msg=%x", m, msg);
        return AE_INVALID;
    }

    if(m->status != MONITOR_RUNNING)  {
        return AE_INVALID;
    }

    /*
     * If the message length is longer than MAX_MONITOR_MSG_LENGTH, restart the monitor
     * and return error.
     */
    if (strlen(msg) > MAX_MONITOR_MSG_LENGTH)  {
        aeDEBUG("Over size msg %d, from = %s\n", MAX_MONITOR_MSG_LENGTH, m->name);
        aeLOG("Over size msg %d, from = %s\n", MAX_MONITOR_MSG_LENGTH, m->name);
        m->status = MONITOR_NEEDS_RESPAWN;
        return AE_INVALID;
    }

    /*
     * Check for the integrity of the message.
     * This means make sure the message starts with the msg-header and 
     * and ends with the msg-trailer.
     * If the message is not intact, discard the message.
     *
     * IMPORTANT: a message can come across two reads, split into two TCP packet.
     * The above bug is documented as defect #43.
     */
     if (chkAeMsgIntegrity (msg) == AE_INVALID)  {
        aeDEBUG("Received invalid message %s, from = %s\n", msg, m->name);
        aeLOG("Received invalid message %s, from = %s\n", msg, m->name);
        return AE_INVALID;
     }

     // Copy the message before processing, since processing will null terminate the tokens in it.
     memset(lBuf, 0, MONITOR_MSG_BUFSIZE);
     strncpy(lBuf, msg, MAX_MONITOR_MSG_LENGTH);

    /*
     * Go, process and message and digest it into a structure.
     */
    if (processMsg(msg, &aeMsg) == AE_INVALID)  {
        aeDEBUG("Invalid msg %s, from = %s\n", msg, m->name);
        aeLOG("Invalid size msg %s, from = %s\n", msg, m->name);
        m->status = MONITOR_NEEDS_RESPAWN;
        return AE_INVALID;
    }  else {
        // aeDEBUG("msg version: %s\n", aeMsg.version );
        // aeDEBUG("msg msgType: %s\n", aeMsg.msgType );
        // aeDEBUG("msg monCodeName: %s\n", aeMsg.monCodeName );
    }

    /*
     * Make sure the message is indeed from the right monitor.
     * This means, make sure 'socketmon' message has the monitor code "SM" in the message etc.
     * If this is not checked, socketmon can be sending messages for filemon.
     * Even worse, a rogue monitor can be mimicking other monitor after killing it.
     * Check whether monitor name in MONCOMM structure chimes with 'monitor code'
     * in the message.
     * SECURITY:
     */

    /*
     * Since we received a valid messge, update the timer.
     * SECURITY: Should we take serous action in case time system call fails?
     */
    t = time(NULL);
    if (t < 0)  {
        aeDEBUG("processMonitorMsg: error getting time: errno =%d\n", errno);
        aeLOG("processMonitorMsg: error getting time: errno =%d\n", errno);
    }
    // Note the latest heartbeat message
    m->hbtime = t;

    /*
     * If it is NOT heartbeat message, store it in monitor's msg buffer.
     * NOTE: We do not store heatbeat messages
     */
    if (isHeartBeatMsg(&aeMsg) != AE_SUCCESS)  {

        /*
         * Critical section.  Since we are reading monitor message, go get the aeLock.
         */
        if (pthread_mutex_lock(&aeLock) != 0)  {
            aeDEBUG("processMonitorMsg: unable to get aeLock. errno = %d\n", errno);
            aeLOG("processMonitorMsg: unable to get aeLock. errno = %d\n", errno);
            return AE_INVALID;
        }   

        /*
         * Store the message (only one msg deep buffer) in the monitor structure.
         * SECURITY: Check whether there is a message with same message-id from the
         * same monitor.  If we do, don't copy it.  Restart the monitor.
         */
        memset(m->monMsg, 0, sizeof(m->monMsg));
        strncpy(m->monMsg, lBuf, MAX_MONITOR_MSG_LENGTH);

        /*
         * Copy it to our global buffer also.
         */
        memset(monitorMsg[monMsgIndex], 0, sizeof(monitorMsg[monMsgIndex]));
        strncpy(monitorMsg[monMsgIndex], m->monMsg, MAX_MONITOR_MSG_LENGTH);
        monitorMsg[monMsgIndex][MAX_MONITOR_MSG_LENGTH - 1] = '\0';
        aeDEBUG("processMonitorMsg: monitorMsg[%d] = %s\n", monMsgIndex, monitorMsg[monMsgIndex]);

        /*
         * Check whether need to wrap around the monitorMsg array.
         * NOTE:  In this logic, if monMsgIndex is non-zero, it has not
         * not wrapped around and hence the beginMsgIndex is zero.
         */
        monMsgIndex++;
        if (monMsgIndex >= NUM_OF_MONITOR_MSGS)  {
            monMsgIndex = 0; // Reached the end.  Start all over from the beginning.
            // Wrapped around, clear the entry we are going to copy to.
        }

        // zero out the next message we are going to fill in.
        memset(monitorMsg[monMsgIndex], 0, sizeof(monitorMsg[monMsgIndex]));

        // Make sure to nullterminate the message.
        m->monMsg[MAX_MONITOR_MSG_LENGTH - 1] = '\0';
        aeDEBUG("processMonitorMsg: stored msg: =%s\n", m->monMsg);

        /*      
         * End of critical section.  Release the lock.
         */
        if (pthread_mutex_unlock(&aeLock) != 0)  {
            aeDEBUG("processMonitorMsg: Unable to get aeLock.  errno = %d\n", errno);
            aeLOG("processMonitorMsg: Unable to get aeLock.  errno = %d\n", errno);
            return AE_INVALID;
        }
    }

    // Construct monitor response
    constructMonResponse(&aeMsg, out);

    return AE_SUCCESS;
}

/*
 * Construct monitor response message
 * Construct response message to monitor as per ae protocol.
 * Example: [:10:985765636438765-734:11:AE:]
 */
void constructMonResponse(AEMSG *aeMsg, char *out)
{
    strncat(out, AE_MSG_OPEN, strlen(AE_MSG_OPEN)); 
    strncat(out, AE_MSG_DELIMITER, strlen(AE_MSG_DELIMITER));
    strncat(out, AE_PROTCOL_VER, strlen(AE_PROTCOL_VER));
    strncat(out, AE_MSG_DELIMITER, strlen(AE_MSG_DELIMITER));
    strncat(out, aeMsg->msgId, strlen(aeMsg->msgId));
    strncat(out, AE_MSG_DELIMITER, strlen(AE_MSG_DELIMITER));
    strncat(out, AE_MONITOR_ACK, strlen(AE_MONITOR_ACK));
    strncat(out, AE_MSG_DELIMITER, strlen(AE_MSG_DELIMITER));
    strncat(out, AE_DAEMON, strlen(AE_DAEMON));  // say 'ae' daemon is sending the response.
    strncat(out, AE_MSG_DELIMITER, strlen(AE_MSG_DELIMITER));
    strncat(out, AE_MSG_END, strlen(AE_MSG_END)); 
    strncat(out, AE_END_OF_RESPONSE, strlen(AE_END_OF_RESPONSE)); 
    aeDEBUG("constructMonResponse: %s", out);
}

/*
 * See whether the received monitor message is a duplicate.
 * Simple string compare would do here since, even for the same
 * eventId message, the timestamp must vary.
 */
int isDuplicateMsg(char *lBuf)
{
    int i = 0;

    for(i=0; i < NUM_OF_MONITOR_MSGS; i++)  {
        if (strcmp(monitorMsg[i], lBuf) == 0)
            return AE_INVALID;
    }

    return AE_SUCCESS;
}

#ifdef _AE_LATER
/*
 * For Debug purpose only.
 * Hand modify this routine to run just one monitor for debugging purposes.
 */
void justDoOnemon()
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
#endif // _AE_LATER


