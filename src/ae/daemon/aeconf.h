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
#include <poll.h>
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
#include "aedaemon.h"

/*
 * IMPORTANT:  This is where the monitor authors will include
 * the parameter values of MONCOMM structure for their
 * monitor.
 */

/*
 * Declare monitors prototypes
 * When adding a new monitor, remember to update MAXMONITORS definition properly.
 * MAXMONITORS is defined in aedaemon.h.  MAXMONITORS is the total number of
 * monitors configured.  The configuration happens at COMPILE time. 
 */

// Self-monitor is special.  It is considered part of 'ae' daemon itself.
#define SELF_MONITOR_NAME "selfmon"

MONCOMM monarray[MAXMONITORS] = {
    {
        .name = SELF_MONITOR_NAME,
        .mode = AE_INVALID,
        .span = AE_INVALID,
        .status = MONITOR_NOT_RUNNING,
        .pid = AE_INVALID,
        .ppid = AE_INVALID,
        .basedir = NULL,
        .socFd[0] = AE_INVALID,
        .socFd[1] = AE_INVALID,
        .monPtr = selfMon
    },
    {
        .name = "socketmon",
        .mode = AE_INVALID,
        .span = AE_INVALID,
        .status = MONITOR_NOT_RUNNING,
        .pid = AE_INVALID,
        .ppid = AE_INVALID,
        .basedir = NULL,
        .socFd[0] = AE_INVALID,
        .socFd[1] = AE_INVALID,
        .monPtr = socketmon
    },
    {
        .name = "procmon",
        .mode = AE_INVALID,
        .span = AE_INVALID,
        .status = MONITOR_NOT_RUNNING,
        .ppid = AE_INVALID,
        .basedir = NULL,
        .socFd[0] = AE_INVALID,
        .socFd[1] = AE_INVALID,
        .monPtr = procmon
    },
    {
        .name = "binmon",
        .mode = AE_INVALID,
        .span = AE_INVALID,
        .status = MONITOR_NOT_RUNNING,
        .ppid = AE_INVALID,
        .basedir = NULL,
        .socFd[0] = AE_INVALID,
        .socFd[1] = AE_INVALID,
        .monPtr = NULL
    },
    {   
        .name = "filemon",
        .mode = AE_INVALID,
        .span = AE_INVALID,
        .status = MONITOR_NOT_RUNNING,
        .ppid = AE_INVALID,
        .basedir = NULL,
        .socFd[0] = AE_INVALID,
        .socFd[1] = AE_INVALID,
        .monPtr = fileMon
    },  
    {   
        .name = "filedescr",
        .mode = AE_INVALID,
        .span = AE_INVALID,
        .status = MONITOR_NOT_RUNNING,
        .ppid = AE_INVALID,
        .basedir = NULL,
        .socFd[0] = AE_INVALID,
        .socFd[1] = AE_INVALID,
        .monPtr = fileDescr
    }
};


/*
 * 'mode' variable can have one of two values.
 * By default, it is set so the ae-daemon only monitors
 * events and does not take action i.e. MONITOR_MODE.
 * To invoke ae-daemon in 'action' mode, invoke 
 * ae with -a opttion i.e. "ae -a"
 * 'mode' values are defined in aedaemon.h since these
 * values are known to ae-daemon only, not monitor's business.
 */
unsigned int mode = MONITOR_MODE;

/*
 * 'lifespan' is relevant to monitors.
 * If the lifespan is set to VOLATILE, then monitors should not
 * maintain any data across system reboot.
 * It can be set to PERSISTENT, by invoking the ae-daemon
 * with -p i.e. "ae -p".
 * 'lifespan' values are defined in
 * include/ae.h since it is also included by all
 * the monitors.
 */
static unsigned int lifespan = VOLATILE;

