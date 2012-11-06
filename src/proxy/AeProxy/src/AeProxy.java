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
 * Original Author: Thomas Pari
 */

package org.tbrt.ae;

import java.io.*;
import javax.net.ssl.*;
import java.security.KeyStore;

/*
 *
 */
public class AeProxy {

    public AeProxy() {
    }

    public static void main(String[] args) throws Exception {

        //
        // Check the proper usage was specified
        //
        if (args.length < 2) {
            System.out.println("USAGE: java AeConnector host port");
            System.exit(-1);
        }
    
        //
        // Extract passed in arguments
        //
        AeConnector connector = new AeConnector();
        connector.setHostname(args[0]);
        connector.setPort(args[1]);
            
        //
        // Connect to the ae daemon
        //
        while(true) {
            if(!connector.connect()) {
                try {
                    Thread.sleep(10000);
                }
                    catch(Exception e) {
                }
                continue;
            }
            break;
        }

        //
        // Start the message store
        //
        AeMessageStore store = new AeMessageStore();
        store.start();

        // 
        // Start the status keeper
        //
        AeStatusKeeper keeper = new AeStatusKeeper(connector, store);
        keeper.start();

        //
        // Start the heart beater
        //
        AeHeartbeat beater = new AeHeartbeat(connector, 15000);
        beater.start();

        //
        // Start the SSL Server
        //
        AeSSLServer server = new AeSSLServer(connector, store);
        server.start();
            
        //
        // Join the thread
        //
        beater.join();
        keeper.join();
        store.join();
        server.join();
        System.exit(0);
    }
}
    

