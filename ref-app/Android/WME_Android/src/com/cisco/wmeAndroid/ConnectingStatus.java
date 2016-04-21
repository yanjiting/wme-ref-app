package com.cisco.wmeAndroid;

import com.cisco.webex.wme.WmeClient;
import com.cisco.webex.wme.WmeParameters;

import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.util.Log;
import android.view.View;
import android.widget.ProgressBar;

public class ConnectingStatus extends AsyncTask<Void, Void, Void> {
	private Handler m_handler;
	private ProgressBar m_progress;
	private Boolean m_isHost;
	private String m_ipAddress;

	public ConnectingStatus(Handler handler, ProgressBar progress, Boolean isHost, String ipAddress) {
		m_handler = handler;
		m_progress = progress;
		m_isHost = isHost;
		m_ipAddress = ipAddress;
	}

	public void onPreExecute() {
		m_progress.setVisibility(View.VISIBLE);
	}

	// Calls connecting functions
	public Void doInBackground(Void... unused) {
		WmeClient.instance().Prepare();
		if (!m_isHost) {
			//WmeClient.instance().StartAsClient(m_ipAddress);
			Bundle bundle = new Bundle();
			bundle.putString("ip", m_ipAddress);
			Message msg = new Message();
			msg.setData(bundle);
			msg.what = WmeParameters.TPConnectToMsg;
			WmeClient.instance().PushMessage(msg);
			Log.v ("wme_android", "Joining as client!");
		} else {
			//WmeClient.instance().StartAsHost();
			Message msg = new Message();
			msg.what = WmeParameters.TPInitHostMsg;	
			WmeClient.instance().PushMessage(msg);
			Log.v ("wme_android", "Joining as host!");
		}
		
		SystemClock.sleep(500 * 1);	//wait for tp connect, not neccesary
		
		m_handler.sendEmptyMessage(Constants.EVENT_READY_TO_LAUNCH_PLAY);
		
		return null;
	}

	public void onPostExecute(Void unused) {
		m_progress.setVisibility(View.INVISIBLE);
	}
}