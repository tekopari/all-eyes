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
 * All Eyes Monitor code names made of 2 letters, as per AeMonitorProtocol (see Wiki).
 */
#define AE_DAEMON               "AE"     // All Eyes, AKA 'AE' Daemon code
#define AE_SELFMON              "SF"     // Self monitor code
#define AE_SOCKETMON            "SM"     // Socket Monitor code
#define AE_PROCMON              "PM"     // Process Monitor code
#define AE_FILEMON              "FL"     // File Monitor code
#define AE_FDMON                "FD"     // File Descriptor Monitor code
#define AE_AEMGR                "AM"     // Android All Eyes Manager code


#define AE_MSG_HEADER            "[:"     // Msg header.  Should it just have '[' and not the ':'?
#define AE_MSG_TRAILER           ":]"     // Msg header.  Should it just have ']' and not the ':'?
#define AE_MSG_DELIMITED         ":"      // Msg delimiter

extern int chkAeMsgIntegrity (char *msg);

#endif  // __AEMSG_H__
