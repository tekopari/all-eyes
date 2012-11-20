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
 *
 * Original Author: Ravi Jagannathan
 * Updated: Blair Wolfinger, 10/27/12.  Adding file descriptor skeleton code.
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
#include <sys/timeb.h>
#include "ae.h"
#include "filedescr.h"

#define BUFSIZE 1024
void constructFDHelloMsg(FDMSG *aeMsg, char *out);


void fileDescr(int mode)
{
static char sbuf[BUFSIZE+1];
FDMSG fdMsg;
static char out[MONITOR_MSG_BUFSIZE];
int ret = -1, err = 0, count = 0;
static char *msg3="filedescr read ERROR**********\n";


    memset(sbuf, 0, BUFSIZE+1);

    //change priority of process to slow it down.  If error, exit.
    err = setpriority(PRIO_PROCESS, 0, 19);
    if( err != 0 )
      {
	aeLOG("setPriority failed\n");
	exit(0);
       }

    while (1)  {
    	count++;
   	    snprintf(fdMsg.msgCount, 6,"%d", count);
    	constructFDHelloMsg(&fdMsg, out);
    	if(count == 999999){
    		count = 0;
    	}
        write(1, out, strlen(out));
        memset(sbuf, 0, BUFSIZE+1);
        while (1)  {
           //sleep to avoid sending too many messages. 
           sleep(5);
           ret = read(0, sbuf, BUFSIZE); 
           if (ret < 0)  {
               write(1, msg3, strlen(msg3));
           } else if ( ret > 0)  {
               break;
           }
        }
    }
}

/*
 * Construct filedescr HELLO msg
 * Example: [:10:985765636438765-734:00:FD:]
 */
void constructFDHelloMsg(FDMSG *filedescrMsg, char *out)
{
    struct timeb tmb;
    const int LOW = 00000;
    const int HIGH = 99999;
    int randnum = 0;

	memset(out, 0, MONITOR_MSG_BUFSIZE);
	strncpy(out, AE_MSG_HEADER, strlen(AE_MSG_HEADER));
    strncat(out, AE_PROTCOL_VER, strlen(AE_PROTCOL_VER));
    strncat(out, AE_MSG_DELIMITER, strlen(AE_MSG_DELIMITER));
    /* setup 15 character timestamp field. For now will obtain miiliseconds
     * then will add 5 character random number.
     */
    ftime(&tmb);
	snprintf(filedescrMsg->msgTimeStamp, 11,"%lu", tmb.time);
	strncat(out, filedescrMsg->msgTimeStamp, strlen(filedescrMsg->msgTimeStamp));
    srand((unsigned int) tmb.time);
    randnum = rand() % (HIGH - LOW + 1) + LOW;
    snprintf(filedescrMsg->msgTimeRandom, 6, "%d", randnum);
    strncat(out, filedescrMsg->msgTimeRandom, 5);

    strncat(out, AE_MSG_DASH, strlen(AE_MSG_DASH));
    strncat(out, filedescrMsg->msgCount, strlen(filedescrMsg->msgCount));
    strncat(out, AE_MSG_DELIMITER, strlen(AE_MSG_DELIMITER));
    strncat(out, AE_MONITOR_HELLO, strlen(AE_MONITOR_HELLO));
    strncat(out, AE_MSG_DELIMITER, strlen(AE_MSG_DELIMITER));
    strncat(out, AE_FILEDESCR, strlen(AE_FILEDESCR));
    strncat(out, AE_MSG_DELIMITER, strlen(AE_MSG_DELIMITER));
    strncat(out, AE_MSG_END, strlen(AE_MSG_END));
}
