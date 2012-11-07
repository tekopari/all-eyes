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

package org.tbrt.aemanager;

import java.util.ArrayList;
import java.util.List;

import android.app.Service;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.os.IBinder;
import android.os.RemoteException;
import android.preference.PreferenceManager;
import android.util.Log;

public class AeProxyService extends Service implements OnSharedPreferenceChangeListener {
	
    private static final String TAG = "AeProxyService";
    
    private final IAeProxyService.Stub mBinder = new IAeProxyService.Stub() {
    	public List<AeMessage> getMessageList() throws RemoteException {
    		// TODO: Get the list
    		List <AeMessage>list = new ArrayList<AeMessage>();
    		String rawMessage = "[:10:33:SM:0001:11:A0A1:tcp_8080:]";
    		AeMessage msg = AeMessage.parse(rawMessage);
    		list.add(msg);
    		return list;
    	}
    	
    	public AeMessage sendAction(AeMessage actionMsg) throws RemoteException {
    		// TODO: Send the message
    		String rawMessage = "[:11:00:AM:]";
    		AeMessage msg = AeMessage.parse(rawMessage);
    		return msg;
    	}
    };
	
    public AeProxyService() {
    	loadPreferences();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mBinder;
    }
    
    /*
     * (non-Javadoc)
     * @see android.app.Service#onCreate()
     */
    public void onCreate() {
    	super.onCreate();
    }
    
    /*
     * (non-Javadoc)
     * @see android.app.Service#onStartCommand(android.content.Intent, int, int)
     */
    public int onStartCommand(Intent intent, int flags, int startId) {
		return super.onStartCommand(intent, flags, startId);
    }
    
    public void onDestroy() {	
		super.onDestroy();
    }
    
    public void loadPreferences() {
        //SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        //settings.registerOnSharedPreferenceChangeListener(AeProxyService.this);
    }

	@Override
	public void onSharedPreferenceChanged(SharedPreferences arg0, String arg1) {
       // SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        //Log.i(TAG, "Sees the username changed to " + settings.getString("username", "")); 
       // Log.i(TAG, "Sees the ipaddress changed to " + settings.getString("ipaddress", "")); 
        //Log.i(TAG, "Sees the port changed to " + settings.getString("port", "")); 
	}
}
