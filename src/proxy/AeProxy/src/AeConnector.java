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
import java.security.KeyStore;

/*
 * This program establishes an SSL connection to a remote host.
 * Once the connection is established it will periodically will
 * send a heartbeat message.  The server will send this process
 * status ae monitor status and event messages.
 */
public class AeConnector {

    private String hostname;
    private int port;
    private SSLSocket socket = null;
    private PrintWriter   out = null;
    private BufferedReader in = null;

    public AeConnector() {
    	hostname = "";
        port = 0;

        //
        // Set the location the trust store.  The trust store contains
        // the set of certificate authorities (CA) x.509 certificates
        // that we are authorized to establish an SSL connection with.
        //
        System.setProperty( "javax.net.ssl.trustStore", "jssecacerts" );
        System.setProperty( "javax.net.ssl.trustStorePassword", "changeit" );
    }

    public void setHostname(String host) {
        hostname = host.trim();
        return;
    }

    public String getHostname() {
        return hostname;
    }

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

    public void setPort(int p) {
        port = p;
        if(port < 1 || port > 65535) {
            port = 0;
        }
        return;
    }

    public int getPort() {
        return port;
    }

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

    public boolean connect() {
        //
        // Verify the connector is properly configured
        //
        if(!verify()) {
            return false;
        }

        //
        // Create the SSL Context and create the socket factory
        //
        SSLSocketFactory factory = null;
        try {
            SSLContext ctx = SSLContext.getInstance("TLS");
            KeyManagerFactory kmf = KeyManagerFactory.getInstance("SunX509");
            KeyStore ks = KeyStore.getInstance("JKS");
            char[] passphrase = "passphrase".toCharArray();

            ks.load(new FileInputStream("/home/tpari01/ssl/keystore.jks"), passphrase);
            kmf.init(ks, passphrase);
            ctx.init(kmf.getKeyManagers(), null, null);
            factory = ctx.getSocketFactory();
        } 
        catch (Exception e) {
            return false;
        }

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
            socket.startHandshake();

            //
            // Create the output writer
            //
            out = new PrintWriter(
                                  new BufferedWriter(
                                      new OutputStreamWriter(socket.getOutputStream())));

            //
            // Create the input reader
            //
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
            
            return true;
        }
        catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

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

    public AeMessage read() {
        if(in == null) {
            return null;
        }
        
        String inputLine = "";
        try {
        	inputLine = in.readLine();
        }
         catch (Exception e) {
            e.printStackTrace();
        }
        return AeMessage.parse(inputLine.trim());
    }

    public boolean write(AeMessage msg) {

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

    public static void main(String[] args) throws Exception {

        //
        // Check the proper usage was specified
        //
        if (args.length < 2) {
            System.out.println("USAGE: java AeConnector host port");
            System.exit(-1);
        }
    
        while(true) {
        	
	        //
	        // Extract passed in arguments
	        //
	        AeConnector connector = new AeConnector();
	        
	        //
	        // Connect to the ae daemon
	        //
	        while(true) {
	        	connector.setHostname(args[0]);
	            connector.setPort(args[1]);
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
	        // Create the proxy's heartbeat message
	        //
	        AeMessage outMsg = new AeMessage();
	        outMsg.setMessageType("00");
	        outMsg.setMonitorName("AM");
	        
	        //
	        // Send the server a heartbeat
	        //
	        connector.write(outMsg);
	        
	        //
	        // Read until
	        //
	        while(true) {
	        	AeMessage inMsg = connector.read();
	        	if(inMsg == null) {
	        		break;
	        	}
	        }
	        
	        //
	        // Disconnect
	        //
            connector.disconnect();
        }
        
    }
}
    

