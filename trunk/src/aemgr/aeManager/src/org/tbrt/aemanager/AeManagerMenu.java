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

import android.os.Bundle;
import android.preference.PreferenceManager;
import android.app.Activity;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.SharedPreferences.OnSharedPreferenceChangeListener;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.support.v4.app.NavUtils;

public class AeManagerMenu extends Activity implements OnSharedPreferenceChangeListener {
	
    private static final String TAG = "AeManagerMenu";
    
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ae_manager_menu);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        loadPreferences();
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
