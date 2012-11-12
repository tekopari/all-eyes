/*
 * Copyright (C) <2012> <Blair Wolfinger, Ravi Jagannathan, Thomas Pari, Todd Chu>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
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
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;
import android.support.v4.app.NavUtils;

public class AeManagerAction extends Activity {
	
	private AeMessage message;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_ae_manager_action);
        getActionBar().setDisplayHomeAsUpEnabled(true);
        Intent intent = getIntent();
        
        message = intent.getParcelableExtra("MESSAGE");
        
        TextView t1 = (TextView)findViewById(R.id.type_textView);
        t1.setText(message.getLongMessageType());
        TextView t2 = (TextView)findViewById(R.id.source_textView);
        t2.setText(message.getLongMonitorName());
        TextView t3 = (TextView)findViewById(R.id.event_textView);
        t3.setText(message.getLongEventName());
        TextView t4 = (TextView)findViewById(R.id.status_textView);
        t4.setText(message.getLongStatusCode());
        TextView t5 = (TextView)findViewById(R.id.text_textView);
        t5.setText(message.getMessageText());
        
        String actionList = message.getActionList() + "A";
        String A0 = "A0A";
        String A1 = "A1A";
        if(actionList.indexOf(A0) == -1) {
        	RadioButton r = (RadioButton)findViewById(R.id.a0);
        	r.setEnabled(false);
        	r.setChecked(false);
        }   
        if(actionList.indexOf(A1) == -1) {
        	RadioButton r = (RadioButton)findViewById(R.id.a1);
        	r.setEnabled(false);
           	r.setChecked(false);
        }        
        
        Button buttonSend = (Button)findViewById(R.id.saveButton); 
        buttonSend.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
            	sendAction(v);
            }
        }
        );

        Button buttonCancel = (Button)findViewById(R.id.cancelButton);       
        buttonCancel.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
            	cancelAction(v);
            }
        }
        );
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.activity_ae_manager_action, menu);
        return true;
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
    
    //Create an anonymous implementation of OnClickListener
    private void sendAction(View v) {
    		Toast.makeText(getApplicationContext(), 
 			       "Save Pressed", 
 			       Toast.LENGTH_SHORT).show();
     	
     	    //
     	    // Check is nothing was requested
     	    //
	     	RadioGroup radioGroup = (RadioGroup) findViewById(R.id.radioGroup1);
	     	if(radioGroup == null) {
	 			Toast.makeText(getApplicationContext(), 
	 				       "RadioGroup is null", 
	 				       Toast.LENGTH_SHORT).show();
	     		return;
	     	}
	     	
	     	int id = radioGroup.getCheckedRadioButtonId();
	     	if (id == -1){  
	 			Toast.makeText(getApplicationContext(), 
	 				       "No Action selected", 
	 				       Toast.LENGTH_SHORT).show();
	 	        Intent i = getIntent();
	 	        setResult(RESULT_CANCELED, i);
	     	}
	     	//
	     	// Check if Ignore was requested
	     	//
	     	else if (id == R.id.a0) {
	 			Toast.makeText(getApplicationContext(), 
	 				       "Ignore requested", 
	 				       Toast.LENGTH_SHORT).show();
	 	        Intent i = getIntent();
	        	message.setMessageType("33");
	        	message.setActionList("A0");
	 	        i.putExtra("MESSAGE", this.message);
	 	        setResult(RESULT_OK, i);
	     	}
	     	//
	     	// Check if Halt was requested
	     	//
	     	else if(id == R.id.a1) {
	 			Toast.makeText(getApplicationContext(), 
	 				       "Halt requested", 
	 				       Toast.LENGTH_SHORT).show();
	 	        Intent i = getIntent();
	        	message.setMessageType("33");
	        	message.setActionList("A1");
	 	        i.putExtra("MESSAGE", this.message);
	 	        setResult(RESULT_OK, i);
	     	}

			finish();
    }
     
    // Create an anonymous implementation of OnClickListener
    private void cancelAction(View v) {
        Toast.makeText(getApplicationContext(), 
		       "Cancel Pressed", 
		       Toast.LENGTH_SHORT).show();
        Intent i = getIntent();
        setResult(RESULT_CANCELED, i);
		finish();
    }
}


