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

import java.util.List;
import java.util.Vector;

import android.os.Bundle;
import android.os.IBinder;
import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.View;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.NumberPicker;
import android.widget.NumberPicker.OnScrollListener;
import android.widget.Toast;
//, OnScrollListener 
public class AeManagerEventList extends Activity implements OnItemClickListener {
	
	private IAeProxyService proxyService;
	private ListView listview1;
	private AeMessageAdapter adapter;
	
	//private LayoutInflater mInflater;
	private Vector<AeMessage> data;
	
	private ServiceConnection connection = new ServiceConnection() {

		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			Toast.makeText(getApplicationContext(), "Service Connected", Toast.LENGTH_SHORT).show();
			proxyService = IAeProxyService.Stub.asInterface(service);
			registerUIEventHandlers();
		}

		@Override
		public void onServiceDisconnected(ComponentName name) {
			Toast.makeText(getApplicationContext(), "Service Disconnected", Toast.LENGTH_SHORT).show();
			proxyService = null;
		}
	};

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ae_manager_event_list);
        data = new Vector<AeMessage>();
        //data.add(new AeMessage());
        
        //
        // Bind to the Service
        //
        Intent intent = new Intent(getApplicationContext(), AeProxyService.class);
        boolean rc = bindService(intent, connection, Context.BIND_AUTO_CREATE);
        if(rc) {
        	Log.d(AeProxyService.class.getSimpleName(), "Service bind successful");
        }
        else {
        	Log.d(AeProxyService.class.getSimpleName(), "Service bind failed");
        }
        
        adapter = new AeMessageAdapter(this, R.layout.listview_item_row, data);
        listview1 = (ListView) findViewById(R.id.listView1);
        View header = (View)getLayoutInflater().inflate(R.layout.listview_header_row, null);
        listview1.addHeaderView(header);
        listview1.setAdapter(adapter);        
        listview1.setOnItemClickListener(this); 
        getActionBar().setDisplayHomeAsUpEnabled(true);
    }
    
//    public void onCreate(Bundle savedInstanceState) {
//    	super.onCreate(savedInstanceState);
//    }
    
    private void registerUIEventHandlers() {
    	Button serviceButton = (Button) findViewById(R.id.refresh);
    	
    	serviceButton.setOnClickListener(new View.OnClickListener() {
    		public void onClick(View v) {
    			try {
    				List<AeMessage> result = proxyService.getMessageList();
    				Toast.makeText(getApplicationContext(), 
    						       "Found " + result.size() + " messages", 
    						       Toast.LENGTH_SHORT).show();
    				
    				adapter.clear();
    				for(int i = 0; i < result.size(); i++) {
    					adapter.add(result.get(i));
    				}
    			}
    			catch(Exception e) {	
    				e.printStackTrace();
    			}
    		}
    		
    	});
    }
    
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_ae_manager_event_list, menu);
        return true;
    }

	@Override
	public void onItemClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
		try {
		    Toast.makeText(getApplicationContext(), "Selected item#" + position + " who value is " + data.get(position-1).toString(), 3000).show();
            Intent intent = new Intent(this, org.tbrt.aemanager.AeManagerAction.class);
            AeMessage msg = AeMessage.parse(data.get(position-1).toString());
            intent.putExtra("MESSAGE", msg);
            startActivity(intent);
		}
		catch(Exception e) {
			e.printStackTrace();
		}
		return;
	}
/*
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
	*/
}
