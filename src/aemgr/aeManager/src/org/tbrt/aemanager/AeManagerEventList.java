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
import java.util.Vector;

import android.os.AsyncTask;
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
    
    class GetMessageListAsyncTask extends AsyncTask<Void, Void, List<AeMessage> > {
    
        @Override
        protected List<AeMessage> doInBackground(Void... params) {
            List<AeMessage> result = null;
            try {
            	 result = proxyService.getMessageList();
            } catch (Exception e) {
                 result = null;
                 Log.e("doInBackGround", e.getMessage());
            }
            return result;
        }
        
        protected void onPostExecute(List<AeMessage> result) {
			if(result == null) {
				Log.e("onPostExecute", "No messages or connection failed");
				adapter.clear();
			}
			else {
				Log.e("onPostExecute", "Found " + result.size() + " messages");
				adapter.clear();
		        for(int i = 0; i < result.size(); i++) {
		            adapter.add(result.get(i));
		        }
			}
        }
    }
    
    class SendActionAsyncTask extends AsyncTask<AeMessage, Void, AeMessage > {
        
        @Override
        protected AeMessage doInBackground(AeMessage ...params) {
            AeMessage result = null;
            try {
            	 result = proxyService.sendAction(params[0]);
            } catch (Exception e) {
                 result = null;
                 Log.e("doInBackGround", e.getMessage());
            }
            return result;
        }
        
        protected void onPostExecute(AeMessage result) {
			if(result == null) {
				Log.e("onPostExecute", "User selected cancel or send action failed");
			}
			else {
				Log.e("onPostExecute", "Action message sent");
			}
        }
    }
    
    private void registerUIEventHandlers() {
    	Button serviceButton = (Button) findViewById(R.id.refresh);
    	
    	serviceButton.setOnClickListener(new View.OnClickListener() {
    		public void onClick(View v) {
    			new GetMessageListAsyncTask().execute();
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
            Intent intent = new Intent(this, org.tbrt.aemanager.AeManagerAction.class);
            AeMessage msg = AeMessage.parse(data.get(position-1).toString());
            intent.putExtra("MESSAGE", msg);
            startActivityForResult(intent, AE_ACTION_REQUEST);
		}
		catch(Exception e) {
			e.printStackTrace();
		}
		return;
	}
	
	static final int AE_ACTION_REQUEST = 1;  // The request code
	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
	    // Check which request we're responding to
	    if (requestCode == AE_ACTION_REQUEST) {
	        // Make sure the request was successful
	        if (resultCode == RESULT_OK) {        	
	        	AeMessage message = data.getParcelableExtra("MESSAGE");
	        	Log.d("onActivityResult", "Take Action on Message:" + message.toString());
	        	new SendActionAsyncTask().execute(message);
	        }
	    }
	}
}
