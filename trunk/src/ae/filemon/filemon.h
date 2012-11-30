/*
 * Copyright (C) <2012> <Blair Wolfinger, Ravi Jagannathan, Thomas Pari, Todd Chu>
 *is ca
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
 */

#define AE_FILEMON              "FM"     // File Monitor code

#define AE_PROTCOL_VER           "10"     // Version number, checked against the msg received
#define AE_MSG_OPEN              "["      // Msg open character
#define AE_MSG_END               "]"      // Msg end character
#define AE_MSG_HEADER            "[:"     // Msg header.  Should it just have '[' and not the ':'?
#define AE_MSG_TRAILER           ":]"     // Msg header.  Should it just have ']' and not the ':'?
#define AE_MSG_DELIMITER         ":"      // Msg delimiter
#define AE_MSG_DASH              "-"      // Msg separate time from id
#define AE_MONITOR_HELLO         "00"     // Indicates message is a heartbeat message
#define AE_MONITOR_ACK           "11"     // Ack message. Only ae daemon can send this? SECURITY
#define AE_MONITOR_EVENT         "22"     // Indicates the message is of event type
#define AE_MONITOR_ACTION        "33"     // Indicates the message is of action type
#define AE_MSG_ID_LENGTH          32      // Length of Message ID
#define AE_MSG_FIELD_LENGTH       256     // Fields of the version, type, monitor code name,
                                          // and message type cannot exceed 255 bytes.
#define AE_EVENTID               "003"    // filemon eventId.
#define AE_GREEN				 "00"     // Status: green
#define AE_ORANGE				 "01"     // Status: orange
#define AE_RED					 "11"     // Status: red
#define AE_ACTION_IGNORE         "A0"     // Ignore, do nothing.
#define AE_ACTION_HALT           "A1"     // Ignore, do nothing.
#define AE_END_OF_RESPONSE       "\n"     // Script based monitors need this.  For C based, it is fine.

#define MONITOR_MSG_BUFSIZE        (1024 * 4)




typedef struct fmMsg  {
    char msgTimeStamp[128];
    char msgCount[512];
    char failMsg[512];
} FMMSG;

extern int check_if_alpha(char buf[]);
