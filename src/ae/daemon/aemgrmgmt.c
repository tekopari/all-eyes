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
#include "aedaemon.h"
#include "aemsg.h"

/*
 *  Put in code to manage the aemgr client.
 *  Should we allow only one at a time?
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

void *aemgrThread(void *ptr)
{
    SSL_CTX *srvCtx = NULL;
    BIO     *acc = NULL;
    BIO     *client = NULL;
    SSL     *ssl = NULL;
    char    *portPtr = NULL;
    sigset_t  set;  // SECURITY: If initialized, it gives a compilor error.

    // Mark SSL thread has come into being.
    sslThreadAlive = 1;

    // Block the SIGCHLD for the SSL handling thread.
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    if (pthread_sigmask(SIG_BLOCK, &set, NULL) != 0)  {
        aeDEBUG("aemgrThread: Unable to block SIGCHLD signal\n");
        aeLOG("aemgrThread: Unable to block SIGCHLD signal\n");
        SSLThreadExit();
    }
 
    portPtr = (char *)ptr;

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
    // Bind the socket we received to the aedaemon port
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
     */
    for(;;)  {
        char *errStrPtr = NULL;
        char static buf[TMP_BUF_SIZE];
        char static outBuf[TMP_BUF_SIZE * 10]; // Yes, this is 40K
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
        if (acceptrc <= 0)  {
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
        }
        aeDEBUG("aemgrThread: SSL connection ACCEPTED!!!!!!!!!!\n");

        do  {
            memset(buf, 0, sizeof(buf));
            aeDEBUG("aemgrThread: reading from SSL socket\n");
            aeLOG("aemgrThread: reading from SSL socket\n");
    
            err = SSL_read(ssl, buf, sizeof(buf));
            if (err == 0)  {
                // Nothing to read.  Continue.
                continue;
            }

            if (err < 0)  {
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

            // aeDEBUG("sending to=============> Android-SSL: %s\n", outBuf);

            // Write the output to the client.
            err = SSL_write(ssl, outBuf, sizeof(outBuf));
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
        } while (err > 0);  // Be in this loop, as long as the clinet is active.
    }
}

// Mark that SSL servicing thread has to be restarted.
void SSLThreadExit()
{
    sslThreadAlive = 0;
    pthread_exit((void *)AE_THREAD_EXIT);
}

int aeSSLProcess( char *inBuf, char *outBuf)
{
    int i = 0;

    /*
     * Check for integrity of the message from Android-SSL client.
     */

    /*
     * Lheck for the integrity of the message.
     * This means make sure the message starts with the msg-header and 
     * and ends with the msg-trailer.
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


    /*
     * Critical section.  Since we are reading monitor message, go get the aeLock.
     */
    if (pthread_mutex_lock(&aeLock) != 0)  {
        aeDEBUG("aeSSLProcess: unable to get aeLock. errno = %d\n", errno);
        aeLOG("aeSSLProcess: unable to get aeLock. errno = %d\n", errno);
        return AE_INVALID;
    }

    /*
     * For all the active monitors, just copy out the status buffers.
     */
    for(i=0; i < MAXMONITORS; i++)  {
        if(monarray[i].status == MONITOR_RUNNING)  {
            if(strlen(monarray[i].monMsg) <= MAX_MONITOR_MSG_LENGTH)  {
                // SECURITY: Should we check for the return value of strncat?
                // aeDEBUG("concating monitor message ------ %s\n", monarray[i].monMsg);
                strncat(outBuf, monarray[i].monMsg, strlen(monarray[i].monMsg));
                /*
                 * Adjust the outBuf pointer for copying the next status buffer.
                 * We add +1 to what strlen returns since it doesn't include the null byte.
                 */
                outBuf = outBuf + (strlen(outBuf) + 1); 
            }
        }
    }

    /*      
     * End of critical section.  Release the lock.
     */         
    if (pthread_mutex_unlock(&aeLock) != 0)  {
        aeDEBUG("aeSSLProcess: Unable to get aeLock.  errno = %d\n", errno);
        aeLOG("aeSSLProcess: Unable to get aeLock.  errno = %d\n", errno);
        return AE_INVALID;
    }

    return AE_SUCCESS;
}
