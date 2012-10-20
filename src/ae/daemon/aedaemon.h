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
#ifndef __AEDAEMON_H__
#define __AEDAEMON_H__

#include <openssl/ssl.h>
#include <openssl/err.h>


/*
 * Monitor Modes
 */
#define MONITOR_MODE            0x1000
#define MONITOR_ACTION_MODE     0x2000

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
 * Monitor Status codes
 */
#define MONITOR_NOT_RUNNING      0x1
#define MONITOR_RUNNING          0x2

/*
 * Error codes in ae
 */
#define GRACEFUL_EXIT_CODE       0
#define SPAWN_MONITOR_ERROR      1
#define SIGACTION_ERROR          2
#define RESOURCE_UNAVAIL_EXIT    3
#define PERMISSION_DENIED_ERROR  5
#define EXIT_INVALID_PARAMETER   6
#define MONITOR_CONFIG_ERROR     7
#define FILE_DUP_ERROR           8

// Bad file descriptor to initialize socFD
#define AE_INVALID                -1

/*
 * Structure to pass to the Monitor.
 */
typedef struct monComm  {
    char                *name;      // Name of the Monitor
    unsigned int        mode;       // Volatile or persistent
    unsigned int        span;       // lives across reboot or not.
    unsigned int        status;     // status is good or bad. Monitor fills
    pid_t               pid;        // Monitor's PID
    pid_t               ppid;       // ae daemon's PID
    unsigned int        action;     // Filled by the Monitor
    unsigned int        hbinterval; // heartbeat interval, per monitor based.
    unsigned int        hbtime;     // Last time heartbeat msg. was received
    char                *basedir;   // Dir for Monitors to store persistent data
    int                 sSSLsoc;    // Server(i.e. daemon) SSL socket
    int                 cSSLsoc;    // Client SSL socket
    SSL_CTX             aeCtx;      // SSL Context of the monitor
    int                 socFd[2];   // socket IPC between ae daemon & monitor
                                    // daemon uses 0th socket; monitor 1st
    void (*monPtr)(int mode);       // Entry point of the Monitor. mode=persistent/volatile
} MONCOMM;

#define MAXMONITORS    5

extern MONCOMM monarray[];

/*
 * SSL definitions
 */
#define CA_PATH "../ssl_test/cert"
#define CA_FILE "../ssl_test/cert/cacert81.pem"
#define SERVER_CERT "../ssl_test/cert/servercert81.pem"
#define CLIENT_CERT "../ssl_test/cert/clientCrt_admin.pem"


extern void monLock(pthread_mutex_t *mutexPtr);
extern void monUnlock(pthread_mutex_t *mutexPtr);
extern int getSocPair(int *ptr);
extern void gracefulExit(int);
void setupSigHandlers(void);
void aeSigHdlr(int sig, siginfo_t *siginfo, void *context);
int isMonitorValid(MONCOMM *monPtr);


extern void selfMon(int mode);
extern void socketMon(void);
extern void spawnMonitor(MONCOMM *monPtr);
extern void kickoffMonitors(void);
void cleanOtherMons(pid_t pid);
void cleanMon(pid_t pid);
int getLocalSoc(int portNo);
SSL_CTX* getServerSSLCTX(void);
SSL_CTX* getClientSSLCTX(void);
void aeMgrMgmt(void);
void monitormgmt(void);
int buildFd(void);
MONCOMM * getMonFromFd(int fd);
MONCOMM *getMonPtr(pid_t pid);

/*
 * Test definitions.  For debug purpose only
 */
#define TEST_LINE "[:10:11:AE:]\n"
void justDoOnemon(void);

#endif  // __AEDAEMON_H__