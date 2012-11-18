/*
 * copyright (C) <2012> <Blair Wolfinger, Ravi Jagannathan, Thomas Pari, Todd Chu>
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
#include <sys/socket.h>

#define  DEBUG 1
#include "ae.h"

/*
 * Mode tells the monitor whether it is going to 
 * operate in PERSISTENT or VOLATILE mode.
 */

int checksum_check(void) { return(0) }   //This line is replaced by real checksum functions during make

void
procmon(int mode)
{
    char mflag[4];

    if (checksum_check() != 0) {
        aeLOG("procmon-c: proc monitor: exec failed due to bad checksum\n");
        exit(1);
    }

    switch(mode) {
        case VOLATILE:
            strncpy(mflag, "-v", 2);
            break;

        case PERSISTENT:
            strncpy(mflag, "-p", 2);
            break;

        default:
            aeLOG("procmon-c: proc monitor: Unknown mode: %d\n", mflag);
            exit(1);
    }
           
#ifdef PRODUCTION
     if (execl("/usr/bin/perl", " ", "/bin/procmon.pl",  mflag, NULL) < 0)  {
#else
     if (execl("/usr/bin/perl", " ", "procmon.pl",  mflag, NULL) < 0)  {
#endif
         aeLOG("procmon-c: proc monitor: exec failed,  Exit Code: %d\n", errno);
         exit(1);
     }

     /*
      *  Should never reach here.
      */

    exit(2);
}
