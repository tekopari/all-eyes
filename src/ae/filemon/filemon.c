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
 * Updated: Blair Wolfinger, 10/27/12.  Setting up filemon for connection to ae daemon.  Following 
 *       instructions from wiki for creating Monitor (copying template, in this case used selfmon as template.
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
#include "ae.h"


// Sha256 checksum.  Cumulative sha25sum of /bin directory.  This involves algorithm.
// Don't use too much of memory or cpu.  Do it over time.

// bew.10/10/2012.  Test checking via svn
/*
 * Mode tells the monitor whether it is going to 
 * operate in PERSISTENT or VOLATILE mode.
 */

#define BUFSIZE 4096

void fileMon(int mode)
{
static char sbuf[BUFSIZE];
static char *msg="[:10:00:FM:]";
int ret = -1, err = 0;
static char *msg3="filemon read ERROR**********\n";
//int which=0, who=0;

    memset(sbuf, 0, BUFSIZE);

    //change priority of process to slow it down.  If error, exit.
    err = setpriority(PRIO_PROCESS, 0, 19);
    if( err != 0 )
      {
	aeLOG("setPriority failed\n");
	exit(0);
       }

    while (1)  {
        write(1, msg, strlen(msg));
        memset(sbuf, 0, BUFSIZE);
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

