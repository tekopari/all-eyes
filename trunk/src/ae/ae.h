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
void setupSigHandlers(void);
void aeSigHdlr(int sig, siginfo_t *siginfo, void *context);

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

/*
 * Actions
 */
#define DO_NOTHING		0x100
#define COMMIT_SEPAKU		0x101    // Samurai style
#define KILL_ACTION		0x102
#define REBOOT_SYSTEM		0x103
#define HALT_SYSTEM		0x104
#define DO_SYSLOG		0x105

/*
 * Error codes in ae
 */
#define THREAD_SPAWN_ERROR 1


/*
 * Structure to pass to the Monitor.
 */
typedef struct monComm  {
    char                *name;    // Name of the Monitor
    unsigned int        mode;     // Volatile or persistent
    unsigned int        span;     // lives across reboot or not.
    unsigned int        status;   // status is good or bad. Monitor fills.
    unsigned int        pid;      // Monitor's Pid.
    unsigned int        action;   // Filled by the Monitor
    unsigned int        heartbeat;// Filled by the Monitor
    char                *basedir; // Base directory under which Monitors will store their persistent data
    pthread_mutex_t     monMutex; // Monitor Mutex, used by the monitor
    void *(*monPtr)(void *);      // Entry point of the Monitor. Look in ae.c
} MONCOMM;

extern void monLock(pthread_mutex_t *mutexPtr);
extern void monUnlock(pthread_mutex_t *mutexPtr);

extern void *selfMon(void *);
extern void *socketMon(void *);
