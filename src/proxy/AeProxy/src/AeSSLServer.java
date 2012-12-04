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
import java.net.*;
import java.util.HashMap;
import javax.net.ssl.*;
import java.security.KeyStore;
import java.security.*;
import java.net.URL;
import java.net.HttpURLConnection;

/*
 * This class implements the runnable class it takes a AeConnector
 * instance. This class act as an SSL server accepting connections 
 * from the android aeManager application.
 */
public class AeSSLServer extends Thread {

    private AeConnector connector;
    private AeMessageStore messageStore;
    private String keystorePath;
    private String truststorePath;

    // The default constructor is private to prevent
    // it from being called.
    private AeSSLServer() {
    }

    public AeSSLServer(AeConnector connector, AeMessageStore store) {
        this.connector = connector;
        this.messageStore = store;
        this.keystorePath = new String("/etc/ae/certs/keystore.jks");
        this.truststorePath = new String("/etc/ae/certs/jssecacerts");
    }

    public AeMessage read(BufferedReader in) {
        if(in == null) {
            return null;
        }

        // Message header is in the form
        char [] readbuf =  new char[1];   // The read buffer
        char [] buffer = new char[132];   // The raw message we are assembling
        int maxsize = buffer.length - 1;  // The maximum possible message size is 132 characters

        try {
            int count = 0;

            // Zeroize the buffer
            for(int i = 0; i < buffer.length; i++) {
                buffer[i] = '\0';
            }

            // Read the message from the socket one character at a time to simplify
            // the issues of dealing with variable length text messages.  We will
            // read until we hit the max message size or find the message tailer
            while(in.read(readbuf) == 1) {
                buffer[count] = readbuf[0];
System.out.print(buffer[count]);

                // Check for end of message
                if(count >= 1 && buffer[count-1] == ':' && buffer[count] == ']') {
                    break;
                }

                // Check to see if we reached the maximum size for an event message
                count++;
                if(count >= maxsize) {
                    System.out.println("Hit the max length of message and tailer not found");
                    return null;
                }
            }
        }
         catch (Exception e) {
            e.printStackTrace();
        }
System.out.println();
        return AeMessage.parse(new String(buffer));
    }

    public boolean write(PrintWriter out, AeMessage msg) {

        if(out == null) {
            return false;
        }

        if(!(msg.isValid())) {
            return false;
        }

        try {
            out.println(msg.toString());
            out.flush();
            if (out.checkError()) {
                System.out.println("SSLSocketClient: java.io.PrintWriter error");
                return false;
            }
            return true;
        }
         catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }


    public void run() {

        //
        // Create the SSL Context and create the socket factory
        //
        SSLServerSocketFactory factory = null;
        SSLServerSocket serversocket = null; 
        try {
            //TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
            //KeyStore trustSt = KeyStore.getInstance("JKS");
            //trustSt.load(new FileInputStream(truststorePath), "changeit".toCharArray());
            //trustManagerFactory.init(trustSt);

            KeyManagerFactory kmf = KeyManagerFactory.getInstance("SunX509");
            KeyStore ks = KeyStore.getInstance("JKS");
            char[] passphrase = "passphrase".toCharArray();

            ks.load(new FileInputStream(keystorePath), passphrase);
            kmf.init(ks, passphrase);
            SSLContext ctx = SSLContext.getInstance("TLS");
            ctx.init(kmf.getKeyManagers(), null, null);
            factory = ctx.getServerSocketFactory();
            serversocket = (SSLServerSocket) factory.createServerSocket(8080);
        }
        catch (Exception e) {
            System.out.println("[ERROR] Failed to become an SSL Server Reason: " + e.getMessage() );
            return;
        }

        //
        // Check if the specified connector is null
        //
        if(this.connector == null) {
            System.out.println("[ERROR] connector is null aborting");
            return;
        }

        //
        // Accept and process incoming connections
        //
        while (true) {
            try {
                System.out.println("[INFO] Waiting for aeManager to connect");
                SSLSocket client = (SSLSocket) serversocket.accept();
                client.setSoLinger(true, 5);

                PrintWriter out = new PrintWriter(
                                      new BufferedWriter(
                                          new OutputStreamWriter(client.getOutputStream())));

                BufferedReader in = new BufferedReader(
                                      new InputStreamReader(client.getInputStream()));

                //
                // Read the Heartbeat
                //
                AeMessage heartbeatMsg = this.read(in);
System.out.println("==========>" + heartbeatMsg.toString());
                if(heartbeatMsg.getMessageType().equals("77")) {
                    URL url = new URL("https://www.googleapis.com/oauth2/v1/userinfo?access_token=" + heartbeatMsg.getToken());
                    HttpURLConnection con = (HttpURLConnection) url.openConnection();
                    int serverCode = con.getResponseCode();
                    if (serverCode == 200) {
                        //successful query
                        boolean found = false;
                        String data = "";
                        String emailpat = "\"email\": \"" + heartbeatMsg.getEmail() + "\"";
                        BufferedReader httpout = new BufferedReader(new InputStreamReader(con.getInputStream()));
                        while ((data = httpout.readLine()) != null) {
                            System.out.println(data);
                            if(data.indexOf(emailpat) >= 0) {
                                found = true;
                                System.out.println("FOUND");
                                break;
                            }
                        }
                        httpout.close();
                        if(!found) {
                            System.out.println("NOT FOUND");
                            in.close();
                            out.close();
                            client.close();
                            continue;
                        }
                        
                    	System.out.println("User google authenticated");
                    	if(AeAuthentication.isAuthorized(heartbeatMsg.getEmail())) {
                    		System.out.println("User authorized");
                    	}
                    	else {
                    		System.out.println("User authorization failed");
                            in.close();
                            out.close();
                            client.close();
                            continue;
                    	}
                    } else if (serverCode == 401) {
                        // bad token
                        System.out.println("Server auth error: token is invalid");
                        in.close();
                        out.close();
                        client.close();
                        continue;
                    } else {
                        //unknown error, do something else
                        System.out.println("Server returned the following error code: " + serverCode);
                        in.close();
                        out.close();
                        client.close();
                        continue;
                    }
                }
                else if(heartbeatMsg.getMessageType().equals("88")) {
System.out.println("==========> GOT 88");
                    if(AeAuthentication.isAuthenticated(heartbeatMsg.getEmail(), heartbeatMsg.getToken())) {
                    	System.out.println("User authenticated");
                    	if(AeAuthentication.isAuthorized(heartbeatMsg.getEmail())) {
                    		System.out.println("User authorized");
                    	}
                    	else {
                    		System.out.println("User authorization failed");
                            in.close();
                            out.close();
                            client.close();
                            continue;
                    	}
                    }
                    else {
                    	System.out.println("User authentication failed");
                        in.close();
                        out.close();
                        client.close();
                        continue;
                    }
                }
                else {
                    System.out.println("No authorization message - disconnecting");
                    in.close();
                    out.close();
                    client.close();
                    continue;
                }

                //
                // Send the event messages
                //
                String msg = messageStore.getMessages();
                System.out.println("[INFO] Writing messages: " + msg);
                out.write(msg, 0, msg.length());
                out.flush();

                //
                // Send an ACK
                //
                System.out.println("[INFO] Sending Ack");
                AeMessage ackMsg = new AeMessage();
                ackMsg.setMessageId();
                ackMsg.setMessageType("11");
                ackMsg.setMonitorName("AM");
                String ackStr = ackMsg.toString();
                out.write(ackStr, 0, ackStr.length());
                out.flush();

                //
                // Read the action messages
                //
                System.out.println("[INFO] Reading action messages");
                while (true) {
                    AeMessage inMsg = this.read(in);
                    if(inMsg == null) {
                        System.out.println("[ERROR] Received an invalid message");
                        break;
                    }
                    else if(inMsg.getMessageType().equals("33")) {
                        System.out.println("==================================================================");
                        System.out.println("[INFO] Received Action Message:" + inMsg.toString());
                        System.out.println("==================================================================");
                        connector.write(inMsg);
                    }
                    else if(inMsg.getMessageType().equals("11")) {
                        System.out.println("[INFO] Received Ack Message:" + inMsg.toString());
                        break;
                    }
                    else {
                        System.out.println("[INFO] Received Unexpected Message:" + inMsg.toString());
                        break;
                    }
                }

                out.write(ackStr, 0, ackStr.length());
                out.flush();

                //
                // Close the socket
                //
                System.out.println("[INFO] Disconnecting client");
                in.close();
                out.close();
                client.close();
            }
            catch(Exception e) {
                System.out.println("[ERROR] " + e.getMessage());
            }
        }
    }
}
