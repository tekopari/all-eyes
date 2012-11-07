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
