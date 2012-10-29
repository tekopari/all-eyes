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
#include "aemsg.h"
#include "aedaemon.h"

/*
 * This file should contain only string processing routines.
 * Should not contain any code related to All Eyes Monitors.
 */

int chkAeMsgIntegrity (char *msg)
{
    int i = 0;
    int len = 0;

    // first 2 chars must be AE_MSG_HEADER.
    if (strncmp(msg, AE_MSG_HEADER, 2) == 0)   {
        // Having found the header, find the trailer.
        // Skip the AE_MSG_HEADER, which is 2 chars.
        len = strlen(msg);
        /*
         * Initialize 'i' so it is pointing to the string, after the AE_MSG_HEADER.
         * Also, note we are only going until (len -1) since we are checking for
         * The next char in the for loop.  Avoid the 'ONE OVER' buffer flow.
         */
        for(i=strlen(AE_MSG_HEADER); i < (len -1); i++)  {
            if ((msg[i] == ':') && (msg[i+1] == ']'))  {
                msg[i + 2] = '\0';  // Null terminate the char after the AE_MSG_TRAILER
                // aeDEBUG("Got GOOD string %s\n", msg);
                return AE_SUCCESS;
            }
        }
    }
         
    aeDEBUG("Got a bad string: %s\n", msg);
    return AE_INVALID;
}


/*
 * This function checks whether a given monitor message is a heartbeat message.
 * Message must be must be validated before calling this function.
 * Expected message format: [:10:00:FM:]
 * SECURITY: THIS IS A non-reentrant function.  It is tricky to use strsep.
 */
int isHeartBeatMsg (char *msg)
{

    // SECURITY:  Ugly, quick.  Revisit.
    char parseArray[MONITOR_MSG_BUFSIZE];
    char *tmp = NULL;

    // Check whether the message is too big.
    if (strlen(msg) > MAX_MONITOR_MSG_LENGTH)
        return AE_INVALID;

    // Zero out our local buffer.
    memset(parseArray, 0, sizeof(parseArray));
    strncpy(parseArray, msg, sizeof(msg));
    tmp = parseArray;

    // Take out the AE_MSG_HEADER 
    tmp = strsep(&tmp, AE_MSG_DELIMITER);

     // Take out the AE_PROTCOL_VER, which will give us the pointer
     // to determine whether this is heartbeat message or not.
    tmp = strsep(&tmp, AE_MSG_DELIMITER);

    // If it is the hello message, the return success.
    if (strncmp(tmp, AE_MONITOR_HELLO, strlen(AE_MONITOR_HELLO)) == 0)
        return AE_SUCCESS;
    else 
        return AE_INVALID;
}
