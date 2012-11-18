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
#include <linux/reboot.h>
#include <linux/capability.h>
#include <sys/reboot.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/socket.h>

#define  DEBUG 1
#include "ae.h"
#include "aemsg.h"
#include "aedaemon.h"

/*
 *  Only one aeMgr client supported.
 */
static unsigned int sslThreadAlive = 0;

void aeMgrMgmt()
{
    pthread_t sslthread;
    int       ret = -1;

    // aeDEBUG("aemgrThread: Entering...\n");

    /*
     * Create a thread to take care of managing the
     * aeManager SSL connection.  Wait for it to complete.
     * If it did, log the evenet, and start another one.
     * If there is a problem in creating thread, log the error and
     * and gracefully exit the daemon.
     */
    if (sslThreadAlive == 0)  {
        memset(&sslthread, 0, sizeof(sslthread));
        ret = pthread_create(&sslthread, NULL, aemgrThread, (void *)AE_PORT);
        if (ret != 0)  {
            aeDEBUG("aemgrThread: Unable to create SSL thread Exiting\n");
            aeLOG("aemgrThread: Unable to create SSL thread Exiting\n");
            gracefulExit(THREAD_CREATE_ERROR);
        }
    }

    // SECURITY:  What should we do here?  pthread_join(sslthread, NULL);

}

/*
 * This thread talks to aeMgr over SSL.
 */
void *aemgrThread(void *ptr)
{
    SSL_CTX *srvCtx = NULL;
    BIO     *acc = NULL;
    BIO     *client = NULL;
    SSL     *ssl = NULL;
    char    *portPtr = NULL;
    sigset_t  set;  // Initialized with sigemptyset below

    sslThreadAlive = 1; // Mark SSL thread has come into being.

    sigemptyset(&set); // initialize 'set' variable.

    /*
     * Block the SIGCHLD for the SSL handling thread,
     * since this thread shouldn't be handling SIGCHLD
     * sent to the parent.  In Linux, when a signal is sent
     * using SIGINT, unfortunately it gets delivered to all
     * the process/threads in the process group.
     * Protect ourselves from that.
     */
    sigaddset(&set, SIGCHLD);
    if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0)  {
        aeDEBUG("aemgrThread: Unable to block SIGCHLD signal\n");
        aeLOG("aemgrThread: Unable to block SIGCHLD signal\n");
        SSLThreadExit();
    }
 
    portPtr = (char *)ptr; // The port we are going to listen.

    /*
     *
     */
    srvCtx = getServerSSLCTX();

    if (srvCtx == NULL)  {
        aeDEBUG("aemgrThread: Unable to set SSL context\n");
        aeLOG("aemgrThread: Unable to set SSL context\n");
        SSLThreadExit();
    }

    aeDEBUG("aemgrThread: Calling BIO_new_accept. Creating Socket\n");
    aeLOG("aemgrThread: Calling BIO_new_accept. Creating Socket\n");
    // Get new server socket
    acc = BIO_new_accept(portPtr);
    if (acc == NULL)  {
        aeDEBUG("aemgrThread: Unable to get SSL server socket\n");
        aeLOG("aemgrThread: Unable to get SSL server socket\n");
        SSLThreadExit();
    }

    aeDEBUG("aemgrThread: Calling BIO_do_accept to bind to the socket...\n");
    aeLOG("aemgrThread: Calling BIO_do_accept to bind to the socket...\n");
    /*
     * Bind the socket we received to the aedaemon port
     * NOTE: openssl library call BIO_do_accept
     * does the bind.  SSL quarkiness.
     */
    if (BIO_do_accept(acc) <= 0)  {
        aeDEBUG("aemgrThread: Unable to bind server socket\n");
        aeLOG("aemgrThread: Unable to bind SSL server socket\n");
        SSLThreadExit();
    }

    aeDEBUG("aemgrThread: Getting into forever loop...\n");
    aeLOG("aemgrThread: Getting into forever loop...\n");
    /*
     * In this for loop we service once aeManager request at a time.
     * To be clear, we only support one manager connection at a time.
     * buf, outBuf are declared static to avoid malloc in embedded
     * runtime. Also within the for loop to limit it's scope.
     * outBuf size is lot bigger than needed for easy debugging of
     * the prototype.  
     */
    for(;;)  {
        char *errStrPtr = NULL;
        char static buf[MONITOR_MSG_BUFSIZE]; // no malloc, static allocation
        char static outBuf[MONITOR_MSG_BUFSIZE * NUM_OF_MONITOR_MSGS]; // For all possible msgs.
        int err = -1;

        /*
         * Start accepting the connection.  Note that we are calling
         * BIO_do_accept the second time deliberately.  In the second time
         * we start accepting the connection.
         */
        aeDEBUG("aemgrThread: Listening for CONNECTION\n");
        aeLOG("aemgrThread: Listening for CONNECTION\n");
        if (BIO_do_accept(acc) <0)  {
            aeDEBUG("aemgrThread: Unable to accept server socket\n");
            aeLOG("aemgrThread: Unable to accept SSL server socket\n");
            SSLThreadExit();
        }

        // Get the client socket to work with.
        aeDEBUG("aemgrThread: Accepted CONNECTION***************************\n");
        aeLOG("aemgrThread: Accepted CONNECTION***************************\n");

        aeDEBUG("aemgrThread: calling BIO_pop\n");
        aeLOG("aemgrThread: calling BIO_pop\n");
        client = BIO_pop(acc);  
        aeDEBUG("aemgrThread: Calling SSL_new after BIO_pop\n");
        aeLOG("aemgrThread: Calling SSL_new after BIO_pop\n");

        ssl = SSL_new(srvCtx);
        if (ssl == NULL)  {
            aeDEBUG("aemgrThread: Unable to create new context\n");
            aeLOG("aemgrThread: Unable to create new context\n");
            SSLThreadExit();
        }

        aeDEBUG("aemgrThread: Calling SSL_set_bio\n");
        aeLOG("aemgrThread: Calling SSL_set_bio\n");
        // This is a void function, hence no error checking.
        SSL_set_bio(ssl, client, client);

        int acceptrc = SSL_accept(ssl);
        if (acceptrc != 1)  { // look SSL_accept man page.
            // SECURITY:  Will this close the socket file descriptor?
            SSL_set_shutdown(ssl, SSL_SENT_SHUTDOWN);

            if(acceptrc < 0) {
                aeDEBUG("aemgrThread: [FATAL] Unable to create new context\n");
                aeLOG("aemgrThread: [FATAL] Unable to create new context\n");
            }
            else {
                aeDEBUG("aemgrThread: [ERROR] Unable to create new context\n");
                aeLOG("aemgrThread: [ERROR] Unable to create new context\n");
                continue;
            }
            SSLThreadExit();
        }  else  {
            // Everything is fine.
        }
        aeDEBUG("aemgrThread: SSL connection ACCEPTED!!!!!!!!!!\n");

        // Having accepted connection, start talking to the client.
        do  {
            memset(buf, 0, sizeof(buf));
            aeDEBUG("aemgrThread: reading from SSL socket\n");
            aeLOG("aemgrThread: reading from SSL socket\n");
    
            err = SSL_read(ssl, buf, sizeof(buf));
            if (err == 0)  {
                // Nothing to read.  Continue.
                continue;
            }

            if (err < 0)  {  // read error.  process it.
                err = SSL_get_error(ssl, err);
                errStrPtr = ERR_error_string((unsigned long) err, buf);
                aeDEBUG("aemgrThread: [ERROR] reading from SSL socket.  Msg = %s;   %s\n", buf, errStrPtr);
                aeLOG("aemgrThread: [ERROR] reading from SSL socket.  Msg = %s\n", buf);
                SSL_free(ssl);
                ssl = NULL;
                err = -1;
                break;
            }

            // SECURITY: For testing only. just write it to stdout for now
            // For some reason aeDEBUG doesn' work and hence using fprintf.
            aeDEBUG("from Android-SSL: %s\n", buf);

            // Process the input from the SSL client.
            memset(outBuf, 0, sizeof(outBuf));
            if (aeSSLProcess(buf, outBuf) == AE_INVALID)  {
                aeDEBUG("aemgrThread: Terminating due to invalid message\n", buf);
                aeLOG("aemgrThread: Terminating due to invalid message\n", buf);
                SSL_free(ssl);
                ssl = NULL;
                err = -1;
                break;
            }

            // Write the output to the client, only if there is something to write.
            if (strlen(outBuf) > 0)  {
                err = SSL_write(ssl, outBuf, strlen(outBuf));
                if (err < 0)  {
                    err = SSL_get_error(ssl, err);
                    errStrPtr = ERR_error_string((unsigned long) err, buf);
                    aeDEBUG("aemgrThread: [ERROR] Writing to SSL socket.  Msg = %s;   %s\n", buf, errStrPtr);
                    aeLOG("aemgrThread: [ERROR] Writing to SSL socket.  Msg = %s;   %s\n", buf, errStrPtr);
                    SSL_free(ssl);
                    ssl = NULL;
                    err = -1;
                    break;
                }
                aeDEBUG("aemgrThread: [INFO] Wrote to SSL socket.  Msg = %s\n", buf);
            }
        } while (err > 0);  // Be in this loop, as long as the clinet is active.
    }
}

/*
 * SSL Thread Exit.
 * Before exiting, mark that flag indicating the thread needs to be restarted.
 */
void SSLThreadExit()
{
    sslThreadAlive = 0;
    aeDEBUG("aemgrThread: Exiting\n");
    aeLOG("aemgrThread: Exiting\n");
    pthread_exit((void *)AE_THREAD_EXIT);
}

/*
 * Process the message from the SSL-client.
 */
int aeSSLProcess( char *inBuf, char *outBuf)
{
    int i = 0;
    static unsigned int numHeartBeat = 0;
    static char aBuf[MONITOR_MSG_BUFSIZE];
    AEMSG aeMsg;

    /*
     * Check for integrity of the message from Android-SSL client.
     */

    /*
     * Check for the integrity of the message.
     * This means make sure the message starts with the msg-header and 
     * and ends with the msg-trailer (defined in the aemsg.h file).
     * If the message is not intact, discard the message.
     *
     * IMPORTANT: a message can come across two reads, split into two TCP packet.
     * The above bug is documented as defect #43.
     */
     if (chkAeMsgIntegrity (inBuf) == AE_INVALID)  {
         aeDEBUG("invalid Android-SSL message %s\n", inBuf);
         aeLOG("invalid Android-SSL message %s\n", inBuf);
         return AE_INVALID;
     }

     aeDEBUG("VALID Android-SSL message %s\n", inBuf);
     // Copy the message before processing, since processing will null terminate the tokens in it.
     memset(aBuf, 0, sizeof(aBuf));
     strncpy(aBuf, inBuf, MAX_MONITOR_MSG_LENGTH);

    /*
     * Go, process and message and digest it into a structure.
     */
    memset(&aeMsg, 0, sizeof(aeMsg));  // zero out the message structure.
    if (processMsg(aBuf, &aeMsg) == AE_INVALID)  {
        aeDEBUG("aeSSLProcess: Invalid msg %s\n", inBuf);
        aeLOG("aeSSLProcess: Invalid msg %s\n", inBuf);
        return AE_INVALID;
    }  else {
        aeDEBUG("msg version: %s\n", aeMsg.version);
        aeDEBUG("msg msgType: %s\n", aeMsg.msgType);
        aeDEBUG("msg monCodeName: %s\n", aeMsg.monCodeName);
    }

    /*
     * Check whether this message is from AeMgr.
     * NOTE:  For action messages, it will look like it is from other monitors.
     */
    if ((strncmp(aeMsg.action, AE_MONITOR_ACTION, strlen(AE_MONITOR_ACTION)) == 0) &&
        (isValidMonitor(&aeMsg)) == 0)  {
        /*
         * Check whether the SSL client is sending the right codename.
         */
        aeDEBUG("aeSSLProcess: Not from AM  %s\n", inBuf);
        aeLOG("aeSSLProcess: Not from AM  %s\n", inBuf);
        return AE_INVALID;
    }


    if (isHeartBeatMsg(&aeMsg) == AE_SUCCESS)  {
        aeDEBUG("aeSSLProcess: heartbeat msg %s\n", inBuf);
        aeLOG("aeSSLProcess: heartbeat msg %s\n", inBuf);
        // Heartbeat message.  Go ahead and send cashed monitor messages.

        // Daemon will only respond to every 10th heart beat message.
        if (numHeartBeat < SSL_WAIT_INTERVAL)  {
            numHeartBeat++;
            return AE_SUCCESS;  // Don't send any response.  Just return.
        }  else  {
            numHeartBeat = 0;
        }

        /*
         * Critical section.  Since we are reading monitor message, go get the aeLock.
         */
        if (pthread_mutex_lock(&aeLock) != 0)  {
            aeDEBUG("aeSSLProcess: unable to get aeLock. errno = %d\n", errno);
            aeLOG("aeSSLProcess: unable to get aeLock. errno = %d\n", errno);
            return AE_INVALID;
        }

        /*
         * OK, give all the messages we have.
         */
        for(i=0; i < monMsgIndex; i++)  {
            if(strlen(monitorMsg[i]) <= MAX_MONITOR_MSG_LENGTH)  {
                aeDEBUG("aeSSLProcess: copying message = %s\n", monitorMsg[i]);
                // SECURITY: Should we check for the return value of strncat?
                // aeDEBUG("concating monitor message ------ %s\n", monarray[i].monMsg);
                strncat(outBuf, monitorMsg[i], strlen(monitorMsg[i]));
                /*
                 * Adjust the outBuf pointer for copying the next status buffer.
                 * We add +1 to what strlen returns since it doesn't include the null byte.
                 */
                //outBuf = outBuf + (strlen(outBuf) + 1); 
                outBuf = outBuf + strlen(monitorMsg[i]);
                memset(monitorMsg[monMsgIndex], 0, sizeof(monitorMsg[monMsgIndex]));
            }  else  {
                aeDEBUG("aeSSLProcess: monitor-msg larger than expected = %s\n", monitorMsg[monMsgIndex]);
                aeLOG("aeSSLProcess: monitor-msg larger than expected = %s\n", monitorMsg[monMsgIndex]);
            }
        }

        /*      
         * Since we copied monitor status to output buffer to send to SSL client
         * set the counter to zero now.
         */
        monMsgIndex = 0;

        /*      
         * End of critical section.  Release the lock.
         */
        if (pthread_mutex_unlock(&aeLock) != 0)  {
            aeDEBUG("aeSSLProcess: Unable to get aeLock.  errno = %d\n", errno);
            aeLOG("aeSSLProcess: Unable to get aeLock.  errno = %d\n", errno);
            return AE_INVALID;
        }

    }  else if (strncmp(aeMsg.msgType, AE_MONITOR_ACTION, strlen(AE_MONITOR_ACTION)) == 0)  {
        /*
         * The message contains action message related to a monitor.
         * SECURITY:  Make sure the monitor is running - right now it is not being checked.
         * If there is a valid one, process it.
         * If the action message is invalid, return error.
         */
        aeDEBUG("aeSSLProcess: received ACTION msg %s\n", inBuf);
        aeDEBUG("aeSSLProcess: monitor message ID %s\n", aeMsg.msgId);
        aeDEBUG("aeSSLProcess: monitor code name %s\n", aeMsg.monCodeName);
        aeDEBUG("aeSSLProcess: monitor EventId %s\n", aeMsg.eventId);
        aeDEBUG("aeSSLProcess: monitor statusOp %s\n", aeMsg.statusOp);
        aeDEBUG("aeSSLProcess: monitor action %s\n", aeMsg.action);
        if(strlen(aeMsg.action) > 0)  {
            if (aeAction(inBuf, &aeMsg)  == AE_INVALID)  {  // Process Action.
                return AE_INVALID;
            }  else  {
                // Successfully performed the action.
                return AE_SUCCESS;
            }
        }
    }  else  {
        // Unrecognizable message from the SSL client.  Return error.
        return AE_INVALID;
        aeLOG("aeSSLProcess: received WRONG message from the SSL client msg %s\n", inBuf);
        aeDEBUG("aeSSLProcess: received ** WRONG ** message from the SSL client msg %s\n", inBuf);
        aeDEBUG("aeSSLProcess: received ACTION msg %s\n", inBuf);
        aeDEBUG("aeSSLProcess: monitor message ID %s\n", aeMsg.msgId);
        aeDEBUG("aeSSLProcess: monitor code name %s\n", aeMsg.monCodeName);
        aeDEBUG("aeSSLProcess: monitor EventId %s\n", aeMsg.eventId);
        aeDEBUG("aeSSLProcess: monitor statusOp %s\n", aeMsg.statusOp);
        aeDEBUG("aeSSLProcess: monitor action %s\n", aeMsg.action);
    }

    return AE_SUCCESS;
}

int aeAction(char *orgMsg, AEMSG *aeMsg)
{
    if (mode == MONITOR_MODE)  {
        aeDEBUG("aeAction: In monitor only mode.  No action taken.\n");
        return AE_SUCCESS;
    }

    if (strcmp(aeMsg->action, AE_ACTION_IGNORE) == 0)  { // Ignore, no action to take.
        aeDEBUG("aeAction: Action = IGNORE\n");
        aeLOG("aeAction: Action = IGNORE\n");
        return AE_SUCCESS;
    }

    if (strcmp(aeMsg->action, AE_ACTION_LOG) == 0)  {
        aeDEBUG("aeAction: Loggin as requested = %s\n", orgMsg);
        aeLOG("aeAction: Loggin as requested = %s\n", orgMsg);
#ifdef _AE_LATER
        if (reboot(LINUX_REBOOT_CMD_HALT) < 0)  {
            aeDEBUG("!!!!!aeAction: HALT Action Failed!!!!!\n");
            aeLOG("!!!!!aeAction: HALT Action Failed!!!!!\n");
            // Should we call gracefulExit here?
        }
#endif
        return AE_SUCCESS;
    }

    return AE_INVALID;
}

int isValidMonitor(AEMSG *aeMsg)
{
    int i = 0;

    for(i=0; i < MAXMONITORS; i++)  {
        if (strncmp(monarray[i].codename, aeMsg->monCodeName, strlen(monarray[i].codename)) == 0)  {
            // SSL client sent a message for a valid and configured monitor.
            return AE_SUCCESS;
        }
    }
    return AE_INVALID;
}
