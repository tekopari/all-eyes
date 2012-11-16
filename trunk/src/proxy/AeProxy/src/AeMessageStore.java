
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
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Calendar;
import java.util.TimeZone;
import javax.net.ssl.*;
import java.security.KeyStore;

/*
 * The AeMessageStore thread manages the message store. The message store is an 
 * in-memory hashmap that stores the messages sent to the proxy by the 'ae' daemon 
 * as well as statistics on these messages. The statistics include the date/time
 * the event message was first reported, the date/time the message was last reported, 
 * and a count of how many times the message was reported. The statistics assist in 
 * the management of this ethereal message store. The monitors will report the message 
 * events roughly every thirty seconds. If no message updates are received within a 
 * two minute window the message is considered to have expired or have been corrected
 * by action response from the aeManager. Once expired the AeMessageStore removes the 
 * message and its statistics from the message store. 
 */
public class AeMessageStore extends Thread {

    private HashMap<String,AeMessageStatistics> messageStore;

    public AeMessageStore() {
        messageStore = new HashMap<String, AeMessageStatistics>();
    }

    public synchronized boolean reportMessage(AeMessage msg) {


        if(msg == null) {
            System.out.println("[ERROR] Can not add message to store it is null");
            return false;
        }

        if(!(msg.isValid())) {
            System.out.println("[ERROR] Can not add message to store it is invalid");
            return false;
        }

       // The message will be reported multiple times and it will have multiple
       // message ids. we will use this message as the without the message id 
       // as the key.
       String temp = msg.toString();
       AeMessage storemsg = AeMessage.parse(temp);
       storemsg.setMessageId(new String("1-1"));  
       try {
            String key = storemsg.toString();
            AeMessageStatistics stats = messageStore.get(key);
            if(stats == null) {
                stats = new AeMessageStatistics(temp);
            }
            else {
                stats.reportMessage();
                stats.setMessage(temp);   // Keep the last reported message because
                                          // the message could expire in the ae daemon
                                          // This could cause action message processing
                                          // to fail because the message could not be found
            }

            messageStore.put(key, stats);
            return true;
        }
        catch(Exception e) {
            System.out.println("[ERROR] Exception occurred reporting a message to the store.");
        }
        return false;
    }

    private synchronized void cleanStore() {

        //
        // Get the current time
        //
        Calendar currentDttm = Calendar.getInstance(TimeZone.getTimeZone("GMT"));
        currentDttm.add(Calendar.MINUTE, -2);

        //
        // Check the message to see if their data has expired
        //
        ArrayList <String>removeList = new ArrayList<String>();
        for (Object value : messageStore.values()) {
            AeMessageStatistics stats = (AeMessageStatistics)value;
            if(stats.getLastReportedDttm().before(currentDttm)) {
                removeList.add(stats.getMessage());
            }
        }
 
        //
        // Remove the messages that have expired
        //
        for(int i = 0; i < removeList.size(); i++) {
            String msg = removeList.get(i);
            messageStore.remove(msg);
            System.out.println("[INFO] Expired message " + msg);
        }
        return;
    }

    public void run() {

        //
        // Check the store each minute to see if any of the 
        // reported messages have expired.  By this I mean if 
        // a message has not been reported in a given time period 
        // then the issue has corrected it self.
        //
        while (true) {
            cleanStore();

            try {
                Thread.sleep(60000);
            }
            catch(Exception e) {
            }
        }
    }

    public synchronized String getMessages() {
   
        StringBuffer buffer = new StringBuffer("");

        //
        // Create a single string that has all the messages
        //
        for (Object value : messageStore.values()) {
            AeMessageStatistics stats = (AeMessageStatistics)value;
            buffer.append(stats.getMessage());
        }
 
        return buffer.toString();
    }

}

