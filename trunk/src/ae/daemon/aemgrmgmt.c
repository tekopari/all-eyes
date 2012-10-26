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

/*
 *  Put in code to manage the aemgr client.
 *  Should we allow only one at a time?
 */

void aeMgrMgmt()
{
    SSL_CTX *srvCtx = NULL;
    BIO     *acc = NULL;
    BIO     *client = NULL;
    SSL     *ssl = NULL;

    srvCtx = getServerSSLCTX();
    if (srvCtx == NULL)  {
        aeDEBUG("aeMgrMgmt: Unable to set SSL context\n");
        aeLOG("aeMgrMgmt: Unable to set SSL context\n");
        return;
    }

    // Get new server socket
    acc = BIO_new_accept(AE_PORT);
    if (acc == NULL)  {
        aeDEBUG("aeMgrMgmt: Unable to get SSL server socket\n");
        aeLOG("aeMgrMgmt: Unable to get SSL server socket\n");
        return;
    }

    // Bind the socket we received to the aedaemon port
    if (BIO_do_accept(acc) <= 0)  {
        aeDEBUG("aeMgrMgmt: Unable to bind server socket\n");
        aeLOG("aeMgrMgmt: Unable to bind SSL server socket\n");
        return;
    }

    /*
     * Start accepting the connection.  Note that we are calling
     * BIO_do_accept the second time deliberately.  In the second time
     * we start accepting the connection.
     */
    if (BIO_do_accept(acc) <0)  {
        aeDEBUG("aeMgrMgmt: Unable to accept server socket\n");
        aeLOG("aeMgrMgmt: Unable to accept SSL server socket\n");
        return;
    }

    // Get the client socket to work with.
    client = BIO_pop(acc);  
    if ( (ssl = SSL_new(srvCtx)))  {
        aeDEBUG("aeMgrMgmt: Unable to create new context\n");
        aeLOG("aeMgrMgmt: Unable to create new context\n");
        return;
    }

    // SECURITY: Check error messages here...
    SSL_set_bio(ssl, client, client);

    // Create a thread to process connection.

    
    aeDEBUG("======>  aemgrmgmt: is being Implemented  <======\n");
    aeLOG("======>  aemgrmgmt: is being Implemented  <======\n");
}
