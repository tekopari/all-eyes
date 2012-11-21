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

#define  DEBUG 1  // Leave the DEBUG flag for the prototype
#include "ae.h"
#include "aedaemon.h"
#include "aemsg.h"

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
static char *msg3="selfmon read ERROR**********\n";
int ret = -1;

    /*
     *  Execute for ever.
     */
    memset(sbuf, 0, BUFSIZE);
    while (1)  {
        memset(sbuf, 0, BUFSIZE);
        selfmonResponse(sbuf);  // Construct the heartbeat message for selfmon.
        write(1, sbuf, strlen(sbuf)); // Send heart beat

        while (1)  {
           // sleep for 25 seconds and pick up the response from the 'ae' daemon.
           sleep(25);
           // sleep(250); // SECURITY: for TESTING FOR restartMonitor ONLY;
           ret = read(0, sbuf, BUFSIZE);  // Wait to hear daemon's response
           if (ret < 0)  {
               aeDEBUG("selfmon: no response from 'ae' daemon; Exiting.\n");
               aeLOG("selfmon: no response from 'ae' daemon; Exiting.\n");
               write(1, msg3, strlen(msg3));
           } else if ( ret > 0)  {  // Got the response from daemon.
               // Check the validity of response.
               if (chkAeResponse(sbuf) == AE_INVALID)  {
                   aeDEBUG("selfmon: Didn't get proper heartbeat message from ae daemon\n");
                   aeLOG("selfmon: Didn't get proper heartbeat message from ae daemon\n");
                   aeDEBUG("selfmon: MONITORING FAILED\n");
                   aeLOG("selfmon: MONITORING FAILED\n");
                   exit(SELFMON_EXIT);
               }
               break;
           }
        }
    }
}

/*
 * Construct monitor response message
 * Construct response message to monitor as per ae protocol.
 * Example: [:10:985765636438765-734:00:SF:]
 */
#define TIME_STRING_SIZE 256
void selfmonResponse(char *out)
{
    struct timeval tv;
    static unsigned int msgId = 1;
    static char timeStr[TIME_STRING_SIZE];

    memset(&tv, 0, sizeof(tv));

    // Set the header and delimiter
    strncat(out, AE_MSG_OPEN, strlen(AE_MSG_OPEN));
    strncat(out, AE_MSG_DELIMITER, strlen(AE_MSG_DELIMITER));
    strncat(out, AE_PROTCOL_VER, strlen(AE_PROTCOL_VER));
    strncat(out, AE_MSG_DELIMITER, strlen(AE_MSG_DELIMITER));

    // Following lines construct message-id. (time in milliseconds + unique msgId)
    if(gettimeofday(&tv, NULL) < 0)  {
       aeDEBUG("selfmon: Could not get time of the day\n");
       aeLOG("selfmon: Could not get time of the day\n");
       exit(SELFMON_EXIT);
    }
    aeDEBUG("selfmon: time in sec.microsec %u.%u\n", tv.tv_sec, tv.tv_usec);

    memset(timeStr, 0, sizeof(timeStr));
    snprintf(timeStr, sizeof(timeStr), "%u%u", (unsigned int)tv.tv_sec, (unsigned int)tv.tv_usec);
    strncat(out, timeStr, strlen(timeStr));
    strncat(out, "-", 1);  // a '-' should be there between time and msgId

    // Now process the msgId.
    memset(timeStr, 0, sizeof(timeStr));
    snprintf(timeStr, sizeof(timeStr), "%u", msgId);
    strncat(out, timeStr, strlen(timeStr));
    strncat(out, AE_MSG_DELIMITER, strlen(AE_MSG_DELIMITER));

    // Increment the msgId for the next message.
    msgId++;
    if(msgId > 999)  {
        msgId = 0;
    }

    // Finish rest of the messge.
    strncat(out, AE_MONITOR_HELLO, strlen(AE_MONITOR_HELLO));
    strncat(out, AE_MSG_DELIMITER, strlen(AE_MSG_DELIMITER));
    strncat(out, AE_SELFMON, strlen(AE_SELFMON));  // say 'ae' daemon is sending the response.
    strncat(out, AE_MSG_DELIMITER, strlen(AE_MSG_DELIMITER));
    strncat(out, AE_MSG_END, strlen(AE_MSG_END));
    strncat(out, AE_END_OF_RESPONSE, strlen(AE_END_OF_RESPONSE));
    aeDEBUG("selfmonResponse: %s", out);
}

int chkAeResponse(char *msg)
{
    AEMSG aeMsg;

     // Make sure the message we received from the daemon is as per ae-monitor protocol.
     if (chkAeMsgIntegrity (msg) == AE_INVALID)  {
        aeDEBUG("Selfcom invalid message %s, from 'ae' daemon\n", msg);
        aeLOG("Selfcom invalid message %s, from 'ae' daemon\n", msg);
        return AE_INVALID;
     }

    /*
     * Go, process and message and digest it into a structure.
     */
    if (processMsg(msg, &aeMsg) == AE_INVALID)  {
        aeDEBUG("Invalid msg %s, from 'ae' daemon\n", msg);
        aeLOG("Invalid msg %s, from 'ae' daemon\n", msg);
        return AE_INVALID;
    }  else {
        // aeDEBUG("selfmon msg version: %s\n", aeMsg.version );
        // aeDEBUG("selfmon msg msgType: %s\n", aeMsg.msgType );
        // aeDEBUG("selfmon msg monCodeName: %s\n", aeMsg.monCodeName );
    }

    // Make sure we got the message indeed from 'ae' daemon.
    if (strncmp(aeMsg.monCodeName, AE_DAEMON, strlen(AE_DAEMON)) != 0)  {
        // This message is not from 'ae' daemon.  Return error.
        aeDEBUG("Message not from 'ae' daemon. msg = %s\n", msg);
        aeLOG("Message not from 'ae' daemon. msg = %s\n", msg);
        return AE_INVALID;
    }

    // Make sure we got the response message from 'ae' daemon.
    if (strncmp(aeMsg.msgType, AE_MONITOR_ACK, strlen(AE_MONITOR_ACK)) != 0)  {
        // This message is not the right response message from 'ae' daemon.
        aeDEBUG("Response not from 'ae' daemon. msg = %s\n", msg);
        aeLOG("Response not from 'ae' daemon. msg = %s\n", msg);
        return AE_INVALID;
    }

    aeDEBUG("Selfcom received right response for 'ae' daemon\n");
    return AE_SUCCESS;
}
