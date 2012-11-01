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

/*
 * Check the integegrity of the message.
 * In this function the term 'integrity' is used to check
 * whether the message contains proper Header and Trailer.
 * Not necessarily whether the message has been modified in
 * transit.
 * SECURITY:  Consider renaming this function.
 */
int chkAeMsgIntegrity (char *msg)
{
    int i = 0;
    int len = 0;

    // If the message is greater or less than the set limits, return error.
    // Check whether the message is too big or too small
    if ((strlen(msg) > MAX_MONITOR_MSG_LENGTH) || (strlen(msg) < MIN_MONITOR_MSG_LENGTH))
        return AE_INVALID;

    // first 2 chars must be AE_MSG_HEADER.
    if (strncmp(msg, AE_MSG_HEADER, 2) == 0)   {
        // Having found the header, find the trailer.
        // Skip the AE_MSG_HEADER, which is 2 chars.
        len = strlen(msg);
        /*
         * Initialize 'i' so it is pointing to the string, after the AE_MSG_HEADER.
         * Also, note we are only going until (len -1) since we are checking for
         * The next char in the for loop.  Avoid the 'ONE OVER' buffer flow.
         * SECUIRTY:  Terminating by the first appearance of AE_MSG_TRAILER.
         * Is this the right thing to do?  Or we should look for the last AE_MSG_TRAILER?
         */
        for(i=strlen(AE_MSG_HEADER); i < (len -1); i++)  {
            if ((msg[i] == ':') && (msg[i+1] == ']'))  {
                msg[i + 2] = '\0';  // Null terminate the char after the first AE_MSG_TRAILER, as a hacker could put multiple trailers.
                // aeDEBUG("Got GOOD string %s\n", msg);
                return AE_SUCCESS;
            }
        }
    }
         
    aeDEBUG("Got a bad string: %s\n", msg);
    return AE_INVALID;
}


/*
 * This function is used to process the text message from monitors and
 * the aeMgr(Android SSL client), and fill in the AEMSG structure.
 * Message must be must be validated before calling this function.
 * Expected message format: [:10:00:FM:]
 * SECURITY: THIS IS A non-reentrant function.  It is tricky to use strtok.
 */
int processMsg(char *msg, AEMSG *aeMsg)
{
    char *token = NULL;

    // Zero out the AEMSG structure
    memset(aeMsg, 0, sizeof(aeMsg));

    // aeDEBUG("ProcessMsg: received----- %s\n", msg);

    // Take out the AE_MSG_HEADER 
    token = strtok(msg, AE_MSG_DELIMITER);
    if (token == NULL)  {
        aeDEBUG("ProcessMsg: error extracting AE_MSG_HEADER: %s\n", msg);
        return AE_INVALID;
    }  else  {
        // aeDEBUG("After taking out AE_MSG_HEADER: %s\n", token);
        // SECURITY:  Should check the strlen of the string pointed by token?
        strcpy(aeMsg->header, token);
    }

    // NOTE: Subsequent strtok must use NULL pointer as the first arguement as per the man page.

    // Take out the AE_PROTCOL_VER, which will give us the pointer
    token = strtok(NULL, AE_MSG_DELIMITER);
    if (token == NULL)  {
        aeDEBUG("ProcessMsg: error extracting AE_PROTCOL_VER: %s\n", msg);
        return AE_INVALID;
    }  else  {
        // aeDEBUG("After taking out AE_MSG_HEADER: %s\n", token);
        // SECURITY:  Should check the strlen of the string pointed by token?
        strcpy(aeMsg->version, token);
    }

    // Take out the message type, which will give us the pointer
    token = strtok(NULL, AE_MSG_DELIMITER);
    if (token == NULL)  {
        aeDEBUG("ProcessMsg: error extracting message type: %s\n", msg);
        return AE_INVALID;
    }  else  {
        // aeDEBUG("After taking out AE_MSG_TYPE: %s\n", token);
        // SECURITY:  Should check the strlen of the string pointed by token?
        strcpy(aeMsg->msgType, token);
    }

    // Take out the ae monitor code name, which will give us the pointer
    token = strtok(NULL, AE_MSG_DELIMITER);
    if (token == NULL)  {
        aeDEBUG("ProcessMsg: error extracting ae monitor code name: %s\n", msg);
        return AE_INVALID;
    }  else  {
        // aeDEBUG("After taking out ae monitor code name: %s\n", token);
        // SECURITY:  Should check the strlen of the string pointed by token?
        strcpy(aeMsg->monCodeName, token);
    }

    /*
     * If monitor code is AE_AEMGR and msgType is of AE_MONITOR_ACTION 
     * then read the action sent by and from Android aeMgr, 
     * then fill in the action code.
     */
    if ((strcmp(aeMsg->monCodeName, AE_AEMGR) == 0)  &&
        (strcmp(aeMsg->msgType, AE_MONITOR_ACTION)  == 0))  {
        // Take out the ae action name, which will give us the pointer
        token = strtok(NULL, AE_MSG_DELIMITER);
        if (token == NULL)  {
            aeDEBUG("ProcessMsg: error extracting ae aeMgr action code: %s\n", msg);
            return AE_INVALID;
        }  else  {
            // aeDEBUG("After taking out ae monitor code name: %s\n", token);
            // SECURITY:  Should check the strlen of the string pointed by token?
            strcpy(aeMsg->actionName, token);
        }
    }

    return AE_SUCCESS;

}

/*
 * This function checks whether a given monitor message is a heartbeat message.
 * Message must be must be validated before calling this function.
 * Expected message format: [:10:00:FM:]
 * SECURITY: THIS IS A non-reentrant function.  It is tricky to use strtok.
 */
int isHeartBeatMsg (AEMSG *aeMsg)
{

    if ((strncmp(aeMsg->msgType, AE_MONITOR_HELLO, sizeof(AE_MONITOR_HELLO))) == 0 )  {
        return AE_SUCCESS;
    }  else  {
        return AE_INVALID;
    }

#ifdef _MORE_DANGEROUS_FUNCTION_
    // SECURITY:  Ugly, quick.  Revisit.
    char parseArray[MONITOR_MSG_BUFSIZE];
    char *tmp = NULL;
    char *token = NULL;

    // Zero out our local buffer.
    memset(parseArray, 0, sizeof(parseArray));
    strncpy(parseArray, msg, strlen(msg));
    tmp = parseArray;

    // aeDEBUG("isHeartBeatMsg: received----- %s\n", tmp);

    // Take out the AE_MSG_HEADER 
    token = strtok(tmp, AE_MSG_DELIMITER);
    // aeDEBUG("After taking out AE_MSG_HEADER: %s\n", token);

    // Take out the AE_PROTCOL_VER, which will give us the pointer
    token = strtok(NULL, AE_MSG_DELIMITER);
    // aeDEBUG("After taking out AE_PROTCOL_VER: %s\n", token);

    // Take out the the possible heartbeat (Hello) message
    token = strtok(NULL, AE_MSG_DELIMITER);
    // aeDEBUG("After taking out AE_MONITOR_HELLO: %s, ptr=%x\n", token, token);

    // If it is the hello message, the return success.
    if (strncmp(token, AE_MONITOR_HELLO, strlen(AE_MONITOR_HELLO)) == 0)  {
        // aeDEBUG("Yae...Got AE_MONITOR_HELLO: %s\n", token);
        return AE_SUCCESS;
    }  else  {
        return AE_INVALID;
    }

    // Zero out our local buffer.
    memset(parseArray, 0, sizeof(parseArray));
    strncpy(parseArray, msg, strlen(msg));
    tmp = parseArray;

    // Take out the AE_MSG_HEADER 
    token = strsep(&tmp, AE_MSG_DELIMITER);
    aeDEBUG("After taking out AE_MSG_HEADER: %s\n", token);

     // Take out the AE_PROTCOL_VER, which will give us the pointer
    token = strsep(&tmp, AE_MSG_DELIMITER);
    aeDEBUG("After taking out AE_PROTCOL_VER: %s\n", token);

    // Take out the the possible heartbeat (Hello) message
    token = strsep(&tmp, AE_MSG_DELIMITER);
    aeDEBUG("After taking out AE_MONITOR_HELLO: %s\n", token);

    // Take out the monitor name from the message
    token = strsep(&tmp, AE_MSG_DELIMITER);
    aeDEBUG("After taking out AE_MONITOR_NAME: %s\n", token);


    // If it is the hello message, the return success.
    if (strncmp(token, AE_MONITOR_HELLO, strlen(AE_MONITOR_HELLO)) == 0)  {
        aeDEBUG("Yae...Got AE_MONITOR_HELLO: %s\n", token);
        return AE_SUCCESS;
    }  else  {
        return AE_INVALID;
    }
#endif // _MORE_DANGEROUS_FUNCTION_

}
