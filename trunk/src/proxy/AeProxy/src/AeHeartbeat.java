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
 * This class implements the runnable class it takes a AeConnector
 * instance and periodically sends a heartbeat or hello message.
 * The ae daemon on receipt of the heartbeat will respond with the
 * the latest status of all the monitors.  However this class will
 * not receive the status instead the AeStatusKeeper is handling 
 * the receipt of the status messages.
 */
public class AeHeartbeat extends Thread {

    private AeConnector connector;
    private int heartbeatDelay;

    // The default constructor is private to prevent
    // it from being called.
    private AeHeartbeat() {
    }

    public AeHeartbeat(AeConnector connector, int delay) {
        this.connector = connector;
        if(heartbeatDelay < 0) {
            this.heartbeatDelay = 15000;
        }
        else {
            this.heartbeatDelay = delay;
        }
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
        // Create the proxy's heartbeat message
        //
        AeMessage outMsg = new AeMessage();
        outMsg.setMessageType("00");
        outMsg.setMonitorName("AM");

        while (true) {
            //
            // Send the server a heartbeat
            //
            if(!(connector.write(outMsg))) {
                 System.out.println("[ERROR] Heartbeat failed");
                 connector.reconnect();
            }
            else {
                 System.out.println("[INFO] Heartbeat sent");
            }

            //
            //  Delay before the heart beat is sent again
            //
            try {
                Thread.sleep(heartbeatDelay);
            }
            catch(Exception e) {
            }
        }
    }
}

