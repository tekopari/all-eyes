package org.tbrt.aemanager;

import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.support.v4.app.NavUtils;

public class AeManagerMenu extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ae_manager_menu);
        getActionBar().setDisplayHomeAsUpEnabled(true);
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

}
