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
#define SPAWN_MONITOR_ERROR    1
#define SIGACTION_ERROR        2
#define RESOURCE_UNAVAIL_EXIT  3


/*
 * Structure to pass to the Monitor.
 */
typedef struct monComm  {
    char                *name;     // Name of the Monitor
    unsigned int        mode;      // Volatile or persistent
    unsigned int        span;      // lives across reboot or not.
    unsigned int        status;    // status is good or bad. Monitor fills
    unsigned int        pid;       // Monitor's PID
    unsigned int        ppid;      // ae daemon's PID
    unsigned int        action;    // Filled by the Monitor
    unsigned int        hbinterval;// heartbeat interval, per monitor based.
    unsigned int        hbtime;    // Last time heartbeat msg. was received
    unsigned int        forkorexc; // Monitor is forked or fork/execed
    char                *basedir;  // Dir for Monitors to store persistent data
    pthread_mutex_t     monMutex;  // Monitor Mutex, used by the monitor
    int                 socFd[2];  // socket IPC between ae daemon & monitor
                                   //   ae will use 0th socket; monitor 1st
    void (*monPtr)(void);          // Entry point of the Monitor. Look in ae.c
} MONCOMM;

#define MAXMONITORS    4


extern void monLock(pthread_mutex_t *mutexPtr);
extern void monUnlock(pthread_mutex_t *mutexPtr);
extern int getSocPair(int *ptr);
extern void gracefulExit(int);
void setupSigHandlers(void);
void aeSigHdlr(int sig, siginfo_t *siginfo, void *context);


extern void selfMon(void);
extern void socketMon(void);
extern void spawnMonitor(MONCOMM *monPtr);
extern void kickoffMonitors(void);
