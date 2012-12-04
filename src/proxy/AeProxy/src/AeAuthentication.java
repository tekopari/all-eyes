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

import java.security.MessageDigest;
import java.math.BigInteger;
import java.util.Random;

import java.io.File;
import java.io.FileWriter;
import java.io.FileInputStream;
import java.io.InputStreamReader;
import java.io.BufferedWriter;
import java.io.BufferedReader;

/**
 * This program establishes an SSL connection to a remote host.
 * Once the connection is established it will periodically will
 * send a heartbeat message.  The server will send this process
 * status ae monitor status and event messages.
 */
public class AeAuthentication {

    public AeAuthentication() {
    }

    /**
     * Check if a string is non-empty
     */
    private static boolean checkString(String s) {
        if(s == null || s.equals("")) {
            return false;
        }
        return true;
    }
    
    /**
     * Check to see if user is authenticated by its userid and password
     */
    public static boolean isAuthenticated(String userid, String password) {

    	//
    	// Userid and password can not be null
    	//
        if(!checkString(userid) || !checkString(password)) {
            return false;
        }
        
        //
        // Calculate the userid hash.  I don't care what is in the
        // string because I am storing the hash of the string
        //
        String userhash = calculateDigest("user", userid);
        if(!checkString(userhash)) {
            return false;
        }
  
        //
        // The entry can not be null
        //
        String entry = getUserEntry(userhash);
        if(!checkString(entry)) {
            return false;
        }

        //
        // Parse the user's password entry
        //    Userhash:salt:passwordhash
        // If more than three parts are returned then the entry is invalid
        //
        String[] parts = entry.split(":", 4);
        if(parts.length != 3) {
             return false;
        }

        String Dbuserhash = parts[0];
        String salt       = parts[1];
        String Dbpasshash = parts[2];

        //
        // The user's salt can not be empty
        //
        if(!checkString(salt)) {
            return false;
        }

        //
        // The user hashes should match
        //
        if(!userhash.equals(Dbuserhash)) {
            System.out.println("userid do not match\n");
            return false;
        }

        //
        // Calculate the password hash
        //
        String passhash = calculateDigest(salt, password);
        if(passhash == null) {
            System.out.println("Failed to calculate the password hash\n");
            return false;
        }

        //
        // If the password hashes match the user is authenticated
        //
        if(passhash.equals(Dbpasshash)) {
            System.out.println("Passwords match\n");
            return true;
        }

        // authentication failed
        return false;
    }

    /*
     * Check if the userid is authorized
     */
    public static boolean isAuthorized(String userid) {
    	//
    	// Userid can not be null
    	//
        if(!checkString(userid)) {
            return false;
        }
        
        //
        // Calculate the userid hash.  I don't care what is in the
        // string because I am storing the hash of the string
        //
        String userhash = calculateDigest("auth", userid);
        if(!checkString(userhash)) {
            return false;
        }
  
        //
        // Lookup the user in the auth file
        //
        String entry = getUserAuthEntry(userhash);
        if(checkString(entry)) {
            return true;
        }
        return false;
    }

    /**
     * Calculate the digest of the password and salt
     */
    private static String calculateDigest(String salt, String value) {

    	// The salt and the hash can not be empty strings
        if(!checkString(salt) || !checkString(value)) {
            System.out.println("Invalid credentials\n");
            return null;
        }
        
        String hashstr = "";
        byte[] buffer1 = salt.getBytes();
        byte[] buffer2 = value.getBytes();
        try {
            // Generate the hash
            MessageDigest digest = MessageDigest.getInstance("MD5");
            digest.update(buffer1);
            digest.update(buffer2);

            // Convert the hash to a hex string
            byte[] hash = digest.digest();
            BigInteger bigInt = new BigInteger(1, hash);
            hashstr = bigInt.toString(16);
        } 
        catch (Exception e) {
            return null;
        }
        return hashstr;
    }

    /**
     * Create the user's password entry
     */
    public static boolean adduser(String userid, String password) {
        //
        // Check for non-null user and password
        //
        if(!checkString(userid) || !checkString(password)) {
            System.out.println("Invalid credentials\n");
            return false;
        }

        //
        // Generate a salt
        //
        Random rnd = new Random();
        int i = 0;
        int saltlen = 3;
        char c = ' ';
        String salt = "";
        for (i = 0; i < saltlen; i++) {
            c = (char)(rnd.nextInt(25)+97);
            salt = salt + c;
        }

        //
        // Calculate the userid hash.  I don't care what is in the 
        // string because I am storing the hash of the string
        //
        String userhash = calculateDigest("user", userid);
        if(getUserEntry(userhash) != null) {
            System.out.println("User account already exists - remove it then recreate it\n");
            return false;
        }

        //
        // Calculate the password hash
        //
        String passhash = calculateDigest(salt, password);
        if(passhash == null) {
            System.out.println("Failed to calculate the password hash\n");
            return false;
        }

        //
        // Add the user hash
        //
        if(addUserEntry(userhash, salt, passhash)) {
            return true;
        }

        return false;
    }
    
    /**
     * Delete the user's password entry
     */
    public static boolean deluser(String userid) {
    	
        //
        // Check for non-null user and password
        //
        if(!checkString(userid)) {
            System.out.println("Invalid userid\n");
            return false;
        }
        
        //
        // Calculate the userid hash.  
        //
        String userhash = calculateDigest("user", userid);
        if(getUserEntry(userhash) == null) {
            System.out.println("User account does not exist\n");
            return false;
        }

        return true;
    }
    
    /**
     * Authorize the user's to use ae
     */
    public static boolean authuser(String userid) {
    	
        //
        // Check for non-null user
        //
        if(!checkString(userid)) {
            System.out.println("Invalid userid\n");
            return false;
        }
        
        //
        // Calculate the userid hash 
        //
        String userhash = calculateDigest("auth", userid);
        if(getUserAuthEntry(userhash) != null) {
            System.out.println("User is already authorized\n");
            return false;
        }      

        //
        // Authorize the user
        //
        if(addUserAuthEntry(userhash)) {
            return true;
        }

        return true;
    }

    /**
     *  Create the user's password entry in the file
     */
    private static String getUserEntry(String userhash) {

    	//
    	// Userid hash can not be null
    	//
        if(!checkString(userhash)) {
            return null;
        }

        //
        // Search for the user's entry and return it
        //
        try {
            FileInputStream fs = new FileInputStream("/etc/ae/password");
            BufferedReader br = new BufferedReader(new InputStreamReader(fs));

            String line = "";
            String tmp = userhash + ":";
            while ((line = br.readLine()) != null)   {
                if(line.startsWith(tmp)) {
                    return line;
                }
            }
            br.close();
        }
        catch(Exception e) {
        }
        return null;
    }
    
    /**
     *  Create the user's authorization entry in the file
     */
    private static String getUserAuthEntry(String userhash) {

    	//
    	// Userid hash can not be null
    	//
        if(!checkString(userhash)) {
            return null;
        }

        //
        // Search for the user's entry and return it
        //
        try {
            FileInputStream fs = new FileInputStream("/etc/ae/auth");
            BufferedReader br = new BufferedReader(new InputStreamReader(fs));

            String line = "";
            String tmp = userhash + ":";
            while ((line = br.readLine()) != null)   {
                if(line.startsWith(tmp)) {
                    return line;
                }
            }
            br.close();
        }
        catch(Exception e) {
        }
        return null;
    }

    /**
     * The addUserAuthEntry adds a entry to the password file for the user
     */
    private static boolean addUserAuthEntry(String userhash) {

        if(!checkString(userhash)) {
            return false;
        }

    	try {
    	    String data = userhash + ":\n";
 
    	    //
    	    // The password file must already exist
    	    //
            File file = new File("/etc/ae/auth");
            if(!file.exists()){
                return false;
    	    }
            
            //
            // Add the entry
            //
    	    FileWriter fw = new FileWriter(file, true);
    	    BufferedWriter bw = new BufferedWriter(fw);
    	    bw.write(data);
    	    bw.close();
    	    
            return true;
    	}
        catch(Exception e){
    	}
        return false;
    }

    /**
     * The addUserEntry adds a entry to the password file for the user
     */
    private static boolean addUserEntry(String userhash, String salt, String passhash) {

        if(!checkString(userhash) || !checkString(salt) || !checkString(passhash)) {
            return false;
        }

    	try {
    	    String data = userhash + ":" + salt  + ":" + passhash + "\n";
 
    	    //
    	    // The password file must already exist
    	    //
            File file = new File("/etc/ae/password");
            if(!file.exists()){
                return false;
    	    }
            
            //
            // Add the entry
            //
    	    FileWriter fw = new FileWriter(file, true);
    	    BufferedWriter bw = new BufferedWriter(fw);
    	    bw.write(data);
    	    bw.close();
    	    
            return true;
    	}
        catch(Exception e){
    	}
        return false;
    }
    
    /**
     *  Check that a userid has valid characters
     */
    public static boolean checkUserid(String userid) {
    	
        if(!checkString(userid)) {
            return false;
        }
        
    	for(char c : userid.toCharArray()) {
    		if((c >= 'a' && c <= 'z') || 
    		   (c >= 'A' && c <= 'Z') ||	
    		   (c >= '0' && c <= '9') ||
    		    c == '@' ||
    	    	c == '-' ||
    	    	c == '_' ||
    		    c == '.') {
    			continue;
    		}
    		return false;
    	}
    	return true;
    }

    public static void main(String[] args) {
    	
    	// 
    	// Check usage
    	//
    	if(args.length != 2) {
    		System.out.println("Usage: AeAuthenticate [-add|-remove|-auth] userid");
    		System.exit(1);
    	}
    	
        //
        // Check the userid
        //
        String userid = args[1];
        if(!checkUserid(userid)) {
    		System.out.println("Invalid userid!!!");
    		System.out.println("Usage: AeAuthenticate [-add|-remove|-auth] userid");
    		System.exit(1);
        }
        
        //
        // Check the options
        //
        if(args[0].equals("-add")) {
        	String s = "";
            try {
            	System.out.print("Enter password:");
                BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
                s = in.readLine();
            }
            catch(Exception e) {
            	s = "";
            }
        	
            if(adduser(userid, s)) {
            	System.out.println("Successfully created userid");
        		System.exit(0);
            }
            else {
            	System.out.println("Failed to create userid");
            }
        }
        else if(args[0].equals("-remove")) {
            if(deluser(userid)) {
            	System.out.println("Successfully removed userid");
        		System.exit(0);
            }
            else {
            	System.out.println("Failed to remove userid");
            }
        }
        else if(args[0].equals("-auth")) {
            if(authuser(userid)) {
            	System.out.println("Successfully authorized user");
        		System.exit(0);
            }
            else {
            	System.out.println("Failed to authorized userid");
            }
        }
        else {
    		System.out.println("Usage: AeAuthenticate [-add|-remove|-auth] userid");
        }
        
        System.exit(1);
    }
}

