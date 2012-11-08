package org.tbrt.aemanager;

import java.util.Vector;

import android.app.Activity;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

public class AeMessageAdapter extends ArrayAdapter<AeMessage> {
	Context context;
	int layoutResourceId;
	Vector<AeMessage> data;
	
	public AeMessageAdapter(Context context, int layoutResourceId, Vector<AeMessage>data) {
		super(context, layoutResourceId, data);
		this.context = context;
		this.layoutResourceId = layoutResourceId;
		this.data = data;
	}
	
	@Override
	public View getView(int position, View convertView, ViewGroup parent) {
		View row = convertView;
		AeMessageHolder holder = null;
		
		if(row == null) {
			LayoutInflater inflater = ((Activity)context).getLayoutInflater();
			row = inflater.inflate(layoutResourceId, parent, false);
			holder = new AeMessageHolder();
			holder.txtTitle = (TextView)row.findViewById(R.id.txtTitle);
			row.setTag(holder);
		}
		else {
			holder = (AeMessageHolder)row.getTag();
		}
		AeMessage message = data.get(position);
		holder.txtTitle.setText(message.toString());
		return row;
	}
	
	static class AeMessageHolder {
		TextView txtTitle;
	}

}
