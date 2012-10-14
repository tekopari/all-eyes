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
#ifndef __AE_H__
#define __AE_H__

#define	AE_NAME		"ae "
#define	AE_VERSION	"0.01"

/*
 * Debug Macro
 * NOTE:  Must define DEBUG before including this file
 *   to enable debug messages.
 */
#if DEBUG
static inline void
aeDEBUG (char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt,ap);
    va_end(ap);
}
#else
#define aeDEBUG(X)     {;}
#endif

/*
 * ae logging.
 */
#define AE_SYSLOG_FLAGS (LOG_LOCAL6 | LOG_NOTICE)
extern void aeLOG(char *format, ...);

/*
 * Declare the Monitors and functions as externs.
 * Monitor functions are called with 'monitor mode' as the parameter
 */

/*
 * Persistent/Volatile
 */
#define PERSISTENT	0x00000001
#define VOLATILE	0x00000002

/*
 * Monitor Modes
 */
#define MONITOR_MODE		0x1000
#define MONITOR_ACTION_MODE	0x2000


extern void monLock(pthread_mutex_t *mutexPtr);
extern void monUnlock(pthread_mutex_t *mutexPtr);
extern int  getSocPair(int *ptr);
#endif // __AE_H__
