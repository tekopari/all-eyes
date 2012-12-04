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

import java.io.IOException;

import org.tbrt.aemanager.AeManagerEventList.SendActionAsyncTask;

import com.google.android.gms.auth.*;
import com.google.android.gms.common.*;

import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.accounts.AccountManager;
import android.app.Activity;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Toast;
import android.support.v4.app.NavUtils;

public class AeManagerMenu extends Activity implements OnSharedPreferenceChangeListener {
	
    private static final String TAG = "AeManagerMenu";
	static final int AE_ACCT_REQUEST = 1;  
	static final int REQUEST_TOKEN = 2;
    public static String token = null;
    public static String accountName = null;
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ae_manager_menu);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        loadPreferences();
        int rc = GooglePlayServicesUtil.isGooglePlayServicesAvailable(this.getApplicationContext());
		if( rc == ConnectionResult.SUCCESS ) { 
            Intent intent = AccountPicker.newChooseAccountIntent(null, null, new String[]{"com.google"}, false, null, null, null, null);
            startActivityForResult(intent, AE_ACCT_REQUEST);
        }
        else {
        	Toast.makeText(getApplicationContext(), "GOOGLE PLAY SERVICES NOT AVAILABLE", Toast.LENGTH_SHORT).show();
            SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        	token = settings.getString("password", "");
            accountName = settings.getString("username", "");
        	GooglePlayServicesUtil.getErrorDialog(rc, this, 0);
        }
    }
    
    protected void onActivityResult(final int requestCode, final int resultCode, final Intent data) {
        if (requestCode == AE_ACCT_REQUEST && resultCode == RESULT_OK) {		
            String accountName = data.getStringExtra(AccountManager.KEY_ACCOUNT_NAME);
            Toast.makeText(getApplicationContext(), "USE ACCOUNT [" + accountName + "]", Toast.LENGTH_SHORT).show();
            new getAndUseAuthTokenInAsyncTask().execute(accountName);
        }
    }
    
    private String getAndUseAuthTokenBlocking(String accountName) {
        String token = null;
        String scope = "oauth2:https://www.googleapis.com/auth/plus.me https://www.googleapis.com/auth/userinfo.profile";
        try {
           token = GoogleAuthUtil.getToken(this, accountName, scope);
           mss1 = "token";
        } catch (GooglePlayServicesAvailabilityException playEx) {
        	mss1 = "1" + playEx.getMessage();
            Dialog dialog = GooglePlayServicesUtil.getErrorDialog(
                playEx.getConnectionStatusCode(),
                this,
                AE_ACCT_REQUEST);
            // Use the dialog to present to the user.
        } catch (UserRecoverableAuthException recoverableException) {
            Intent recoveryIntent = recoverableException.getIntent();
            // Use the intent in a custom dialog or just startActivityForResult.
            mss1 = "2" + recoverableException.getMessage();
            startActivityForResult(recoverableException.getIntent(), REQUEST_TOKEN);

        } catch (GoogleAuthException authEx) {
            // This is likely unrecoverable.
            Log.e(TAG, "Unrecoverable authentication exception: " + authEx.getMessage(), authEx);
            mss1 = "3" + authEx.toString();
        } catch (IOException ioEx) {
            Log.i(TAG, "transient error encountered: " + ioEx.getMessage());
            mss1 = "4" + ioEx.getMessage();
        }
        return token;
    }
    
    String mss1="";
    String mss2="";
    
    //====================================================================
    // The SendActionAsyncTask calls the service to send the action to the
    // AeProxy.  Android throws an exception if the
    // you try to create a network exception in the users thread.
    //====================================================================
    class getAndUseAuthTokenInAsyncTask extends AsyncTask<String, Void, String > {

        @Override
        protected String doInBackground(String ...params) {
            try {
            	 String accountName = params[0];
            	 return getAndUseAuthTokenBlocking(accountName);
            } catch (Exception e) {
                 token = null;
                 mss2 = e.getMessage();
                 Log.e("doInBackGround", e.getMessage());
            }
            return null;
        }
        
        protected void onPostExecute(String result) {
			if(result == null) {
				token = null;
				Log.e("onPostExecute", "Token is null");
		       	Toast.makeText(getApplicationContext(), "Token=[null]", Toast.LENGTH_SHORT).show();
		       	Toast.makeText(getApplicationContext(), "mss1=["+mss1+"]", Toast.LENGTH_SHORT).show();
		       	Toast.makeText(getApplicationContext(), "mss2=["+mss2+"]", Toast.LENGTH_SHORT).show();
			}
			else {
				token = result;
				Log.e("onPostExecute", "Token=[" + token + "]");
		       	Toast.makeText(getApplicationContext(), "Token=[" + token + "]", Toast.LENGTH_SHORT).show();
			}
        }
    }
    

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_ae_manager_menu, menu);
        return true;
    }

    /* Called when a user clicks the Events button */
    public void eventsMessage(View view) {
        Intent intent = new Intent(this, org.tbrt.aemanager.AeManagerEventList.class);
        startActivity(intent);
    }
    
    /* Called when a user clicks the Settings button */
    public void settingsMessage(View view) {
        Intent intent = new Intent(this, org.tbrt.aemanager.AeManagerSettings.class);
        startActivity(intent);
    }
    
    /* Called when a user clicks the Help button */
    public void helpMessage(View view) {
        Intent intent = new Intent(this, org.tbrt.aemanager.AeManagerHelp.class);
        startActivity(intent);
    }
    
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case android.R.id.home:
                NavUtils.navigateUpFromSameTask(this);
                return true;
        }
        return super.onOptionsItemSelected(item);
    }
    
    public void loadPreferences() {
        SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        settings.registerOnSharedPreferenceChangeListener(AeManagerMenu.this);
    }

	@Override
	public void onSharedPreferenceChanged(SharedPreferences arg0, String arg1) {
        SharedPreferences settings = PreferenceManager.getDefaultSharedPreferences(getApplicationContext());
        Log.i(TAG, "Set username to " + settings.getString("username", "")); 
        Log.i(TAG, "Set ipaddress to " + settings.getString("ipaddress", "")); 
        Log.i(TAG, "Set port to " + settings.getString("port", "")); 
	}

}
