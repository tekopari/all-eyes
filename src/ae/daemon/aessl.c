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
#include <sys/resource.h>
#include <sys/file.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define  DEBUG 1
#include "ae.h"
#include "aedaemon.h"

SSL_CTX*
getSSLCTX()
{
SSL_CTX *ctxPtr;

    aeLOG("getSSLCTX Begin\n");

    SSL_load_error_strings();

    // Initialize the SSL library.
    SSL_library_init();
    OpenSSL_add_all_ciphers();


    // SSL context for SSLv2/v3 and TLSv1.
    if ((ctxPtr = SSL_CTX_new(SSLv23_server_method())) == NULL)  {
        aeLOG("getSSLCTX: problem initializing SSLv23 context\n");
        aeDEBUG("getSSLCTX: problem initializing SSLv23 context\n");
        return ((SSL_CTX *) AE_INVALID);
    }

    // SECURITY: Should we have SSL_VERIFY_PEER?
    SSL_CTX_set_verify(ctxPtr,SSL_VERIFY_NONE,NULL);

    return (ctxPtr);
}
