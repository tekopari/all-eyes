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
#include "aemsg.h"

/*
 * 'ae' user name and the chroot path
 */
#if PRODUCTION
#define AE_USER                "ae"  // SECURITY: Fix this to 'ae' during integration Testing.
#define AE_CHROOT                "/ae/jail"
#else
#define AE_USER                "ae"  // SECURITY: Fix this to 'ae' during integration Testing.
#define AE_CHROOT                "/"
#endif

// Self-monitor is special.  It is considered part of 'ae' daemon itself.
#define SELF_MONITOR_NAME "selfmon"

/*
 * 'ae' response message as per the protocol on wiki.
 * This response is sent to monitors and the SLL-client
 */
#define NUM_OF_MONITOR_MSGS        1024             // Total number of buffered monitor messages.
#define MONITOR_MSG_BUFSIZE        (1024 * 4)       // 4K byte buffer, yes the buffer is bigger.
#define MAX_MONITOR_MSG_LENGTH     1024             // 1K byte buffer
#define MIN_MONITOR_MSG_LENGTH     11               // 1K byte buffer
#define AE_HEARTBEAT_INTERVAL      30               // heartbeat interval is 30 seconds

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
#define MONITOR_NEEDS_RESPAWN    0x3

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
#define THREAD_CREATE_ERROR      9
#define CHROOT_JAIL_ERROR        10
#define TIME_SET_ERROR           11
#define MUTEX_INIT_ERROR         11
#define DROP_PRIV_ERROR          12
#define SELFMON_EXIT             13
#define NOTROOT_EXIT             14
#define AE_THREAD_EXIT           100

// ae daemon return value from functions
#define AE_INVALID                -1
#define AE_SUCCESS                0
#define MONITOR_CODE_NAME_LENGTH  16

/*
 * Structure to pass to the Monitor.
 */
typedef struct monComm  {
    char                *name;      // Name of the Monitor
    char                codename[MONITOR_CODE_NAME_LENGTH];      // Name of the Monitor
    unsigned int        mode;       // monitor or action mode.
    unsigned int        span;       // lives across reboot or not; volatile/persistent
    unsigned int        status;     // status is good or bad. Monitor fills
    pid_t               pid;        // Monitor's PID
    pid_t               ppid;       // ae daemon's PID
    unsigned int        action;     // Filled by the Monitor
    unsigned int        hbinterval; // heartbeat interval, per monitor based.
    time_t              hbtime;     // Last time heartbeat msg. was received
    char                *basedir;   // Dir for Monitors to store persistent data
    int                 sSSLsoc;    // Server(i.e. daemon) SSL socket
    int                 cSSLsoc;    // Client SSL socket
    pthread_mutex_t     monMutex;   // Mutex lock for the monitor structure
    SSL_CTX             aeCtx;      // SSL Context of the monitor
    void (*monPtr)(int mode);       // Entry point of the Monitor. mode=persistent/volatile
    int                 socFd[2];   // socket IPC between ae daemon & monitor
                                    // daemon uses 0th socket; monitor 1st
    char                monMsg[MONITOR_MSG_BUFSIZE]; // Message from monitor
} MONCOMM;

#define MAXMONITORS    6

extern MONCOMM monarray[];
extern pthread_mutex_t aeLock;
extern unsigned int mode;
extern char monitorMsg[NUM_OF_MONITOR_MSGS][MONITOR_MSG_BUFSIZE];
extern unsigned int monMsgIndex;

/*
 * SSL definitions
 */
#if PRODUCTION
#define CA_PATH 		"/etc/ae/certs"
#define CA_FILE 		"/etc/ae/certs/cacert81.pem"
#define SERVER_CERT		"/etc/ae/certs/servercert81.pem"
#define SERVER_CERT_CHAIN 	"/etc/ae/certs/servercert81chain.pem"
#define CLIENT_CERT		"/etc/ae/certs/clientCrt_admin.pem"
#else
#define CA_PATH 		"../devcerts"
#define CA_FILE 		"../devcerts/cacert81.pem"
#define SERVER_CERT		"../devcerts/servercert81.pem"
#define SERVER_CERT_CHAIN 	"../devcerts/servercert81chain.pem"
#define CLIENT_CERT		"../devcerts/clientCrt_admin.pem"
#endif

/*
 * aedaemon lisening port.
 */
#define AE_PORT			"6000"


/*
 * extern definitions of function to satisfy the gcc compiler.
 */
extern void monLock(pthread_mutex_t *mutexPtr);
extern void monUnlock(pthread_mutex_t *mutexPtr);
extern int getSocPair(int *ptr);
extern void gracefulExit(int);
void setupSigHandlers(void);
void aeSigHdlr(int sig);
int isMonitorValid(MONCOMM *monPtr);


extern void selfMon(int mode);
extern void fileMon(int mode);
extern void fileDescr(int mode);
extern void socketMon(void);
extern void spawnMonitor(MONCOMM *monPtr);
extern void kickoffMonitors(void);
extern void cleanOtherMons(pid_t pid);
extern void killMonitor(MONCOMM *monPtr);
extern void cleanMon(pid_t pid);
extern void restartMonitor (MONCOMM *monPtr);
extern int getLocalSoc(int portNo);
extern SSL_CTX* getServerSSLCTX(void);
extern SSL_CTX* getClientSSLCTX(void);
extern void aeMgrMgmt(void);
extern void monitormgmt(void);
extern int buildFd(void);
extern MONCOMM * getMonFromFd(int fd);
extern MONCOMM *getMonPtr(pid_t pid);
extern void *aemgrThread(void *ptr);
extern int aeSSLProcess(char *inBuf, char *outBuf);
extern void SSLThreadExit(void);
extern void dropPrivileges(void);
extern int processMonitorMsg(MONCOMM *m, char *lBuf, char *oBuf);
extern void monHeartbeatCheck(void);
extern void checkChildren(void);
extern int aeAction(char *orgMsg, AEMSG *aeMsg);
extern void getMonUserId(void);

/*
 * Test definitions.  For debug purpose only
 */
#define TEST_LINE "[:10:11:AE:]\n"
#define TMP_BUF_SIZE 4096
void justDoOnemon(void);

#endif  // __AEDAEMON_H__
