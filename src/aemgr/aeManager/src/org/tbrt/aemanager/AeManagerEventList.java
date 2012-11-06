package org.tbrt.aemanager;

import android.os.Bundle;
import android.app.Activity;
import android.content.Intent;
import android.view.Menu;
import android.view.View;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.NumberPicker;
import android.widget.NumberPicker.OnScrollListener;
import android.widget.Toast;

public class AeManagerEventList extends Activity implements OnItemClickListener, OnScrollListener {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ae_manager_event_list);
    
        
        Intent intent = getIntent();
        
        AbsListView list = (AbsListView) findViewById(R.id.listView1);
        
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(
        		getApplicationContext(), 
        		android.R.layout.simple_list_item_1, 
        		names);
        list.setAdapter(adapter);
        list.setOnItemClickListener(this);
       // list.setOnScrollListener(this);
     
        getActionBar().setDisplayHomeAsUpEnabled(true);
    }
    
    String [] names = {"NAME1", "NAME2", "NAME3", "NAME4", "NAME5", "NAME6", "NAME7", "NAME8", "NAME9", "NAME10", "NAqqME1", "NAMqE2", "NAqME3", "NAqME4", "NAMqE5", "NAMqE6", "NAMqE7", "NAMqE8", "NAMqE9", "NqAME10", "NAMsE1", "NAMsE2", "NAMsE3", "NAMsE4", "NAMsE5", "NAMsE6", "NAMEs7", "NsAME8", "NAsME9", "NAMsE10"};

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_ae_manager_event_list, menu);
        return true;
    }

	@Override
	public void onItemClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
		// TODO Auto-generated method stub
		Toast.makeText(getApplicationContext(), "Selected item#" + position + " VALUE IS " + names[position], 3000).show();
		
	
		
	}

	@Override
	public void onScrollStateChange(NumberPicker view, int scrollState) {
		//public void onScrollStateChange(AbsListView view, int scrollState)
		// STATE=0  IDLE
		// STATE=1  SCROLLSTATE
		// STATE=2  INERTA STATE//	
		if(scrollState == 0) {
			Toast.makeText(getApplicationContext(), "OnScrollStateChanged: STOPPED" + scrollState, 3000).show();
		}
		else if(scrollState == 1) {
			
		}
	}
}
