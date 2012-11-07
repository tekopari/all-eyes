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
 * The aeProxy is a multi-threaded application that acts as the proxy between the aeManager Android 
 * application and the 'ae' daemon. Its purpose is to cache the ae message events until the user is 
 * able to receive and act upon the events. To accomplish this the aeProxy constructs a single SSL 
 * connection (mutual authenticated) to 'ae' daemon process and the following threads:
 *
 * - The AeHeartbeat thread every thirty seconds sends a heartbeat message to the 'ae' daemon. The 
 *   'ae' daemon on receipt of the heartbeat will respond with the latest status of all the monitors. 
 *   However the AeHeartbeat thread does not receive the status message as that is the job of the 
 *   AeStatusKeeper thread.
 *
 * - The AeStatusKeeper thread receives the response to the heartbeat from the 'ae' daemon. On 
 *   receipt validates each field of the message and valid messages are cached in the aeMessageStore.
 *
 * - The AeMessageStore thread manages the message store. The message store is an in-memory hashmap 
 *   that stores the messages sent to the proxy by the 'ae' daemon as well as statistics on these 
 *   messages. The statistics include the date/time the event message was first reported, the 
 *   date/time the message was last reported, and a count of how many times the message was reported. 
 *   The statistics assist in the management of this ethereal message store. The monitors will report
 *   the message events roughly every thirty seconds. If no message updates are received within a 
 *   two minute window the message is considered to have expired or have been corrected by action 
 *   response from the aeManager. Once expired the AeMessageStore removes the message and its 
 *   statistics from the message store.
 *
 * - The AeSSLServer thread manages connections from the aeManager Android application. When an SSL
 *   connection is accepted the AeSSLServer send a snapshot of the event messages that reside in 
 *   the AeMessageStore. In response the aeManager will respond with any action messages that the 
 *   user has sent. 
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
    

