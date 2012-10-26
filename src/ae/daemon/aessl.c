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
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define  DEBUG 1
#include "ae.h"
#include "aedaemon.h"

/*
 * Get a server SSL context.
 * Note that this function only establishes SSL context
 * with the server cert and the associated CA cert.
 * To use it, one still have to associated it with a socket.
 */

SSL_CTX* getServerSSLCTX()
{
    SSL_CTX *ctxPtr = NULL;

    aeLOG("getSSLCTX Begin\n");

    SSL_load_error_strings();

    // Initialize the SSL library.
    SSL_library_init();
    OpenSSL_add_all_ciphers();


    // SSL context for SSLv2/v3 and TLSv1.
    if ((ctxPtr = SSL_CTX_new(SSLv23_server_method())) == NULL)  {
        aeLOG("getSSLCTX: problem initializing SSLv23 context\n");
        aeDEBUG("getSSLCTX: problem initializing SSLv23 context\n");
        return ((SSL_CTX *) NULL);
    }

    // SECURITY RISK:  Do we have to free ctxPtr if the subsequent call fails?

    /*
     * Verify the peer and fail, if the verification is not possible.
     * This call doesn't return anything.  It just sets the parameters in ctx structure.
     */
    SSL_CTX_set_verify(ctxPtr,(SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT),NULL);

    /*
     * Verify the peer's certificate.  If not proper, fail
     * Check error messages.
     * Note that this may have to change if the peer verfication is not necessary.
     * This means we want to authentiate the peer.  This means the peer has to
     * have his/her own SSL cert and corresponding CA-cert.
     * SECURITY:  This may not be possible in the prototype.
     * NOTE:  This call returns 1 for success.
     */
    if (SSL_CTX_load_verify_locations(ctxPtr,CA_FILE,CA_PATH) != 1)  {
        aeDEBUG("getServerSSLCTX: SSL_CTX_load_verify_locations failed\n");
        aeLOG("getServerSSLCTX: SSL_CTX_load_verify_locations failed\n");
        return ((SSL_CTX *) NULL);
    }

    /*
     * Now setup our private key
     * server-cert-chain was created by the cmd
     * "cat servercert81.pem cacert81.pem  > serververt81chain.pem"
     */ 
    if (SSL_CTX_use_certificate_chain_file(ctxPtr,SERVER_CERT_CHAIN) != 1)  {
        aeDEBUG("getServerSSLCTX: SSL_CTX_use_certificate_chain_file failed\n");
        aeLOG("getServerSSLCTX: SSL_CTX_use_certificate_chain_file failed\n");
        return ((SSL_CTX *) NULL);
    }

    /*
     * Tell the SLL context i.e. CTX, which server private key to use and its format.
     */ 
    if (SSL_CTX_use_PrivateKey_file(ctxPtr, SERVER_CERT, SSL_FILETYPE_PEM) < 0)  {
        aeDEBUG("getServerSSLCTX: SSL_CTX_use_certificate_chain_file failed\n");
        aeLOG("getServerSSLCTX: SSL_CTX_use_certificate_chain_file failed\n");
        return ((SSL_CTX *) NULL);
    }

    return (ctxPtr);
}

/*
 * Get a client SSL context.
 * Note that this function only establishes SSL context
 * with the client cert and the associated CA cert.
 * To use it, one still have to associated it with a socket.
 * NOTE!!!! This function is not code complete.
 * Also, we need to perform error checks.
 */

SSL_CTX*
getClientSSLCTX()
{
SSL_CTX *ctxPtr;

    aeLOG("getSSLCTX Begin\n");

    SSL_load_error_strings();

    // Initialize the SSL library.
    SSL_library_init();
    OpenSSL_add_all_ciphers();


    // SSL context for SSLv2/v3 and TLSv1.
    // Check error messages
    if ((ctxPtr = SSL_CTX_new(SSLv23_server_method())) == NULL)  {
        aeLOG("getSSLCTX: problem initializing SSLv23 context\n");
        aeDEBUG("getSSLCTX: problem initializing SSLv23 context\n");
        return ((SSL_CTX *) AE_INVALID);
    }

    // SECURITY: Should we have SSL_VERIFY_PEER?
    // Check error messages
    SSL_CTX_set_verify(ctxPtr,(SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT),NULL);
    SSL_CTX_load_verify_locations(ctxPtr,CA_FILE,CA_PATH);

    return (ctxPtr);
}


int
getLocalSoc(int portNo)
{
int soc;
struct hostent *host;
// struct sockaddr_in addr;

    if( (host = gethostbyname("localhost")) == NULL)  {
        aeLOG("getLocalSoc: problem getting hostent for local host, errno: %d\n", errno);
        aeDEBUG("getLocalSoc: problem getting hostent for local host, errno: %d\n", errno);
        return (AE_INVALID);
    }

    // Create a local socket.  Notice AF_LOCAL.
    if( (soc = socket(AF_LOCAL, SOCK_STREAM, 0)) < 0)  {
        aeLOG("getLocalSoc: problem getting a socket, errno: %d\n", errno);
        aeDEBUG("getLocalSoc: problem getting a socket, errno: %d\n", errno);
        return (AE_INVALID);
    }

/*******************
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_LOCAL;
    addr.sin_port = htons(portNo);
    addr.sin_addr.s_addr = *(long*)(host->h_addr);
    if(bind(soc, (const struct stockaddr *)&addr, sizeof(addr)) != 0 )  {
        aeLOG("getLocalSoc: problem getting a socket, errno: %d\n", errno);
        aeDEBUG("getLocalSoc: problem getting a socket, errno: %d\n", errno);
        return (AE_INVALID);
    }
********************/
  
    return soc;
}
