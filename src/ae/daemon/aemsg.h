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
#ifndef __AEMSG_H__
#define __AEMSG_H__

/*
 * NOE:  All the below defines are derived from 
 * the project wiki page, http://code.google.com/p/all-eyes/wiki/AeMonitorProtocol
 */

/*
 * All Eyes Monitor code names made of 2 letters, as per AeMonitorProtocol (see Wiki).
 */
#define AE_DAEMON               "AE"     // All Eyes, AKA 'AE' Daemon code
#define AE_SELFMON              "SF"     // Self monitor code
#define AE_SOCKETMON            "SM"     // Socket Monitor code
#define AE_PROCMON              "PM"     // Process Monitor code
#define AE_FILEMON              "FL"     // File Monitor code
#define AE_FDMON                "FD"     // File Descriptor Monitor code
#define AE_AEMGR                "AM"     // Android All Eyes Manager code


#define AE_PROTCOL_VER           "10"     // Version number, checked against the msg received
#define AE_MSG_HEADER            "[:"     // Msg header.  Should it just have '[' and not the ':'?
#define AE_MSG_TRAILER           ":]"     // Msg header.  Should it just have ']' and not the ':'?
#define AE_MSG_DELIMITER         ":"      // Msg delimiter
#define AE_MONITOR_HELLO         "00"     // Indicates message is a heartbeat message
#define AE_MONITOR_ACK           "11"     // Ack message. Only ae daemon can send this? SECURITY
#define AE_MONITOR_EVENT         "22"     // Indicates the message is of event type
#define AE_MONITOR_ACTION        "33"     // Indicates the message is of action type
#define AE_MSG_FIELD_LENGTH       256     // Fields of the version, type, monitor code name,
                                          // and message type cannot exceed 255 bytes.
#define AE_ACTION_IGNORE         "A0"     // Ignore, do nothing.
#define AE_ACTION_HALT           "A1"     // Ignore, do nothing.

/*
 * Structure that keeps messages after parsing
 * SECURITY: The structure has longer string length than needed.
 * Meant for fast prototyping.
 */
typedef struct asMsg  {
    char header[512];
    char version[512];
    char msgType[512];
    char monCodeName[512];
    char action[512];
} AEMSG;

extern int chkAeMsgIntegrity (char *msg);
extern int isHeartBeatMsg(AEMSG *msg);
extern int processMsg(char *msg, AEMSG *aeMsg);

#endif  // __AEMSG_H__