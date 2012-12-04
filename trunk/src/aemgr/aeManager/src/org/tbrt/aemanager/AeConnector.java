package org.tbrt.aemanager;

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

import java.io.*;
import javax.net.ssl.*;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.util.Log;
import java.security.KeyStore;
import java.security.SecureRandom;

/*
 * This program establishes an SSL connection to a remote host.
 */
public class AeConnector {

    private String hostname;
    private int port;
    private SSLSocket socket = null;
    private PrintWriter   out = null;
    private BufferedReader in = null;
    private Context context;

    public AeConnector(Context c) {
        hostname = "";
        port = 0;
        context = c;
    }
    
    //
    // Checks if a network on the device is on
    //
	public boolean isNetworkUp() {
		try {
	        ConnectivityManager cm = (ConnectivityManager) context.getSystemService(Context.CONNECTIVITY_SERVICE);
	        NetworkInfo networkInfo = cm.getActiveNetworkInfo();
	        if (networkInfo != null && networkInfo.isConnected()) {
	            return true;
	        }
		}
		catch (Exception e) {
			Log.e("isNetworkUp", e.getMessage());
		}
	    return false; // Network on the device is not up
	} 

	//
	// Set the hostname
	//
    public void setHostname(String host) {
        hostname = host.trim();
        return;
    }

	//
	// Get the hostname
	//
    public String getHostname() {
        return hostname;
    }

	//
	// Set the port number via a string
	//
    public void setPort(String p) {
        if(p == null) {
            port = 0;
        }

        try {
            port = Integer.parseInt(p.trim());
        }
        catch (IllegalArgumentException e) {
            port = 0;
        }
        return;
    }

	//
	// Set the port number via an int
	//
    public void setPort(int p) {
        port = p;
        if(port < 1 || port > 65535) {
            port = 0;
        }
        return;
    }

    //
    // Get the port number
    //
    public int getPort() {
        return port;
    }

    //
    // Verify the hostname and port are set correctly
    //
    private boolean verify() {
        //
        // Verify the host name is valid
        //
        if(hostname == null || hostname.equals("")) {
            return false;
        }
    
        //
        // Verify the port number is valid.  Check to see if the port
        // is in the valid range of ports.
        //
        if(port < 1 || port > 65535) {
            return false;
        }
        return true;
    }

    //
    // Safe why to reconnect with multiple threads
    //
    public synchronized void reconnect() {
        disconnect();
        connect();
        return;
    }

    //
    // Connect to the server
    //
    public boolean connect() {
        //
        // Verify the connector is properly configured
        //
        if(!verify()) {
        	Log.e("connect", "Check ip and port");
            return false;
        }
        
        if(!isNetworkUp()) {
        	Log.e("connect", "Device network is down");
        	return false;
        }
        
        Log.e("connect", "CONNECTING");

        //
        // Create the SSL Context and create the socket factory
        //
        SSLSocketFactory factory = null;
        try {
            SSLContext ctx = SSLContext.getInstance("TLS");
            
            
            TrustManagerFactory trustManagerFactory = TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
            KeyStore trustSt = KeyStore.getInstance("BKS");
            InputStream trustStoreStream = context.getResources().openRawResource(R.raw.aecacerts);
            trustSt.load(trustStoreStream, "changeit".toCharArray());
            trustManagerFactory.init(trustSt);
            Log.e("connect", "GOT TRUSTSTORE");
            
            KeyManagerFactory kmf = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
            KeyStore ks = KeyStore.getInstance("BKS");
            InputStream keyStoreStream = context.getResources().openRawResource(R.raw.aekeystore);
            char[] passphrase = "passphrase".toCharArray();
            ks.load(keyStoreStream, passphrase); 
            kmf.init(ks, passphrase);

            ctx.init(kmf.getKeyManagers(), trustManagerFactory.getTrustManagers(), null);
            // ctx.init(null, null, null);
            factory = ctx.getSocketFactory();
        } 
        catch (Exception e) {
        	Log.e("connect", "SSL Context failed");
        	Log.e("connect", e.getMessage());
            return false;
        }
        
        Log.e("connect", "CONNECTED");
        
        SSLSocket socket = null;
        try {
            //
            // Open the socket to the specified host and port
            //
            socket = (SSLSocket) factory.createSocket(hostname, port);

            //
            // Initiate the SSL handshake to establish the session key
            // This will perform a mutual authentication as the server-side
            // enables that option.
            //
            Log.e("connect", "BEFORE HANDSHAKE");
            socket.startHandshake();

            //
            // Create the output writer
            //
            Log.e("connect", "BEFORE PRINTWRITER");
            out = new PrintWriter(
                                  new BufferedWriter(
                                      new OutputStreamWriter(socket.getOutputStream())));

            //
            // Create the input reader
            //
            Log.e("connect", "BEFORE INPUTSTREAM");
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            Log.e("connect", "GTG");
            return true;
        }
        catch (Exception e) {
	        Log.e("connect", "AE CONNECT FAILED");
	        Log.e("connect", e.getMessage());
        }
        return false;
    }

    //
    // Close the SSL connection
    //
    public void disconnect() {
        try {
            in.close();
            out.close();
            socket.close();
            in = null;
            out = null;
            socket = null;
        }
        catch(Exception e) {
        }
    }

    //
    // Read a message from the connection
    //
    public AeMessage read() {
        if(in == null) {
            return null;
        }
        
        // Message header is in the form
        char [] readbuf =  new char[1];   // The read buffer
        char [] buffer = new char[131];   // The raw message we are assembling
        int maxsize = buffer.length - 1;  // The maximum possible message size is 131 characters
        
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
                
                // Check for end of message
                if(count >= 1 && buffer[count-1] == ':' && buffer[count] == ']') {
                    break;
                }
                
                // Check to see if we reached the maximum size for an event message
                count++;
                if(count >= maxsize) {
                    System.out.println("Hit the max length of message and tailer not found");
                    disconnect();
                    return null;
                }
            }
        }
         catch (Exception e) {
	        Log.e("read", "Read failed");
	        Log.e("read", e.getMessage());
        }
        return AeMessage.parse(new String(buffer));
    }

    //
    // Write a message on the connection
    //
    public boolean write(AeMessage msg) {

        if(out == null) {
        	Log.e("write", "Write:  stream is null");
            return false;
        }
        
        if(!(msg.isValid())) {
        	Log.e("write", "Write:  message is invalid");
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
	        Log.e("write", "Write failed");
	        Log.e("write", e.getMessage());
        }
        return false;
    }
}
    

