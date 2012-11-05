
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
import java.util.HashMap;
import javax.net.ssl.*;
import java.security.KeyStore;

/*
 * This class implements the runnable class it takes a AeConnector
 * instance and periodically receives the responses to the heartbeat
 * from the ae daemon.  This thread takes the monitor status and event
 * messages and caches them awaiting the connection of the ae Manager
 * android application.
 */
public class AeStatusKeeper extends Thread {

    private AeConnector connector;
    private AeMessageStore messageStore;

    // The default constructor is private to prevent
    // it from being called.
    private AeStatusKeeper() {
    }

    public AeStatusKeeper(AeConnector connector, AeMessageStore store) {
        this.connector = connector;
        messageStore = store;
    }

    public void run() {

        //
        // Check if the specified connector is null
        //
        if(this.connector == null) {
            System.out.println("[ERROR] connector is null aborting");
            return;
        }

        //
        // Keep reading messages
        //
        while (true) {
            AeMessage inMsg = connector.read();
            if(inMsg == null) {
                System.out.println("[ERROR] Received an invalid message");
                connector.reconnect();
                try {
                    Thread.sleep(10000);
                }
                catch(Exception e) {
                }
            }
            else {
                System.out.println("[INFO] Received message:" + inMsg.toString());
                if(messageStore.reportMessage(inMsg)) {
                    System.out.println("[INFO] Successfully added message to the store");
                }
                else {
                    System.out.println("[ERROR] Failed to add message to the store");
                }
            }
        }
    }
}

