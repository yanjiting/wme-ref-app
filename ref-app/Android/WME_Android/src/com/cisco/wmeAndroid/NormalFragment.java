package com.cisco.wmeAndroid;
 
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

import com.cisco.webex.wme.WmeClient;
import com.cisco.webex.wme.WmeParameters;
import com.cisco.wmeAndroid.R;

import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.SystemClock;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;
import android.app.ActionBar.Tab;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.app.ActionBar;
import android.app.FragmentManager;
import android.content.Intent;
 
public class NormalFragment extends Fragment implements ActionBar.TabListener, OnClickListener, CompoundButton.OnCheckedChangeListener {
    
    private Button mConnectBttn;
    RadioGroup m_radioHostClient;  
    RadioButton m_radioHost,m_radioClient; 
    private EditText mIPaddressEdt;
    private TextView mHostIPTxt;
    private ProgressBar mConnectBar;
    public Boolean m_isHost;
    public String m_IP;
    private Boolean m_pip;
    private Boolean m_enableBatteryInfo;
    private CheckBox m_cbPicInPic;
    private TextView m_tvIP;
    private CheckBox m_cbBatteryInfo;
    private EditText m_etHttpServerIP;
    private EditText m_etHttpServerPort;
    private TextView m_tvCmdSent;
    
    private Handler m_MessageHandler;
    private static Boolean m_bIsNetworkConnectted;
    private VideoSettingFragment m_vsetting;
    private AudioSettingFragment m_asetting;
    private int m_iForceStopTimer;
    public String m_HttpServerIP;
    public String m_HttpServerPort;
    
    public Boolean m_isFile;
    public PcapFileConfigure m_audioConfigure;
    public PcapFileConfigure m_videoConfigure;
    
    public NormalFragment()
    {
    	m_HttpServerIP = "";
    	m_HttpServerPort = "";
    	m_enableBatteryInfo = false;
    	m_pip = true;		//default use picture in picture for more debug info
    	m_isHost = true;
    	m_IP = "";
    	m_bIsNetworkConnectted = false;
    	m_iForceStopTimer = 1000*60*120;
    	m_MessageHandler = new Handler(){
			@Override
			public void handleMessage(Message msg){					
				
				switch (msg.what) {
					case Constants.EVENT_READY_TO_LAUNCH_PLAY:
					{
						ReadyToLaunchPlay();
						removeMessages(Constants.EVENT_READY_TO_LAUNCH_PLAY);
					}							
					break;
						
					case Constants.EVENT_FORCE_STOP_TIMER:
					{
						Log.i("wme_android", "EVENT_FORCE_STOP_TIMER message gotten");
						FinishPlayByForce();
						removeMessages(Constants.EVENT_FORCE_STOP_TIMER);
					}
					break;
					
					default:
						break;
				}
										
			}
		};
		
		m_isFile = false;
		m_audioConfigure = new PcapFileConfigure();
		m_videoConfigure = new PcapFileConfigure();
    }

 
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        getActivity().setContentView(R.layout.normal);
      
        // Sync XML views with Java
        mConnectBttn = (Button) getActivity().findViewById(R.id.connectBttn);
        m_radioHostClient = (RadioGroup) getActivity().findViewById(R.id.hostSwitch);
        m_radioHost = (RadioButton) getActivity().findViewById(R.id.radioHost);
        m_radioClient = (RadioButton) getActivity().findViewById(R.id.radioClient);
        mIPaddressEdt = (EditText) getActivity().findViewById(R.id.ipEdit);
        mHostIPTxt = (TextView) getActivity().findViewById(R.id.hostIP);
        mConnectBar = (ProgressBar) getActivity().findViewById(R.id.progressBar1);
        m_cbPicInPic = (CheckBox) getActivity().findViewById(R.id.picinpic);
        m_tvIP = (TextView) getActivity().findViewById(R.id.myip);
        m_cbBatteryInfo = (CheckBox) getActivity().findViewById(R.id.battery_performance);
        m_etHttpServerIP = (EditText) getActivity().findViewById(R.id.ServerAddressEdit);
        m_etHttpServerPort = (EditText) getActivity().findViewById(R.id.ServerPortEdit);
        m_tvCmdSent = (TextView) getActivity().findViewById(R.id.CommandSent);

        // Setup Listeners
        mConnectBttn.setOnClickListener(this);
        m_cbPicInPic.setOnCheckedChangeListener(this);
        m_cbBatteryInfo.setOnCheckedChangeListener(this);
        
        // Read pcap file configure
        Properties pro = new Properties();
        try {
        //	pro.load(NormalFragment.class.getResourceAsStream("/assets/pcapfile.properties"));
		//	pro.load(getActivity().getAssets().open("pcapfile.properties"));
        //	String filesPath = getActivity().getFilesDir().getAbsolutePath() + "/";
        	String filesPath = Environment.getExternalStorageDirectory().getAbsolutePath() + "/";
        	pro.load(new FileInputStream(filesPath + "pcapfile.properties"));
			m_isFile = pro.getProperty("pcapfile.enable").equalsIgnoreCase("yes");
			if(m_isFile) {
				// read audio configure
				m_audioConfigure.enable = pro.getProperty("audio.enable").equalsIgnoreCase("yes");
				if(m_audioConfigure.enable) {
					m_audioConfigure.fileName = filesPath + pro.getProperty("audio.file");
					m_audioConfigure.sourceIP = pro.getProperty("audio.source.ip");
					m_audioConfigure.sourcePort = Integer.parseInt(pro.getProperty("audio.source.port"));
					m_audioConfigure.destinationIP = pro.getProperty("audio.destination.ip");
					m_audioConfigure.destinationPort = Integer.parseInt(pro.getProperty("audio.destination.port"));
				}
				// read video configure
				m_videoConfigure.enable = pro.getProperty("video.enable").equalsIgnoreCase("yes");
				if(m_videoConfigure.enable) {
					m_videoConfigure.fileName = filesPath + pro.getProperty("video.file");
					m_videoConfigure.sourceIP = pro.getProperty("video.source.ip");
					m_videoConfigure.sourcePort = Integer.parseInt(pro.getProperty("video.source.port"));
					m_videoConfigure.destinationIP = pro.getProperty("video.destination.ip");
					m_videoConfigure.destinationPort = Integer.parseInt(pro.getProperty("video.destination.port"));
				}
				//
			}
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        
        m_radioHostClient.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {  
            @Override  
            public void onCheckedChanged(RadioGroup group, int checkedId) {  
                // TODO Auto-generated method stub  
                if(checkedId==m_radioHost.getId())  
                {  
                	mIPaddressEdt.setVisibility(View.INVISIBLE);
        			mHostIPTxt.setVisibility(View.INVISIBLE);
        			m_isHost = true;
                }else  
                {  
                	mIPaddressEdt.setVisibility(View.VISIBLE);
        			mHostIPTxt.setVisibility(View.VISIBLE);
        			m_isHost = false;
                }  
            }  
        });  

		if (m_isHost == true)
		{
			m_radioHost.setChecked(true);
			mIPaddressEdt.setVisibility(View.INVISIBLE);
			mHostIPTxt.setVisibility(View.INVISIBLE);
		}
		else
		{
			m_radioClient.setChecked(true);
			mIPaddressEdt.setVisibility(View.VISIBLE);
			mHostIPTxt.setVisibility(View.VISIBLE);
			mIPaddressEdt.setText(m_IP);
		}
		
		mConnectBar.setVisibility(View.INVISIBLE);
  
		m_cbPicInPic.setChecked(m_pip);
		
		m_etHttpServerIP.setText(m_HttpServerIP);
		m_etHttpServerPort.setText(m_HttpServerPort);
		m_cbBatteryInfo.setChecked(m_enableBatteryInfo);
		
		//test
		String myipv4 = NetworkHelper.getLocalIpAddressV4();
		m_tvIP.setText("my ip is: "+myipv4);
		//Log.i("wme_android", "my ip is "+myipv4);
    }

	  //backdoor functions
    public String bdGetHostIP()
    {
    	if(m_isHost == true)
    	{
    		String myipv4 = NetworkHelper.getLocalIpAddressV4();
    		return  myipv4;
    	}
    	else
    	{
    		return null;
    	}
    		
    }
    
    public void bdConnectAsClient(String ipAddr)
    {
    	WmeClient.instance().Prepare();
    	if (!m_isHost) {
			//WmeClient.instance().StartAsClient(m_ipAddress);
			Bundle bundle = new Bundle();
			bundle.putString("ip", ipAddr);
			Message msg = new Message();
			msg.setData(bundle);
			msg.what = WmeParameters.TPConnectToMsg;
			WmeClient.instance().PushMessage(msg);
			SystemClock.sleep(500 * 1);	//wait for tp connect, not necessary
			
			m_MessageHandler.sendEmptyMessage(Constants.EVENT_READY_TO_LAUNCH_PLAY);
			Log.v ("wme_android", "bdConnectAsClient called!");
		}
    }
    
    public void bdConnectAsHost()
    {
    	WmeClient.instance().Prepare();
    	Message msg = new Message();
		msg.what = WmeParameters.TPInitHostMsg;	
		WmeClient.instance().PushMessage(msg);
		m_MessageHandler.sendEmptyMessage(Constants.EVENT_READY_TO_LAUNCH_PLAY);
		Log.v ("wme_android_backDoor", "bdConnectAsHost!");
    }
    
    public void bdSetAsClient()
    {
    	m_radioHost.setChecked(false);
    	m_radioClient.setChecked(true);
    	m_isHost = false;
    	m_radioClient.setChecked(true);
		mIPaddressEdt.setVisibility(View.VISIBLE);
		mHostIPTxt.setVisibility(View.VISIBLE);
		mIPaddressEdt.setText(m_IP);
    }
    

    
 
    public void onTabSelected(Tab tab, FragmentTransaction ft) {
        // TODO Auto-generated method stub
        ft.add(android.R.id.content, this);  
        //ft.attach(mFragment);
    }
 
    public void onTabUnselected(Tab tab, FragmentTransaction ft) {
        // TODO Auto-generated method stub
        
    	if (m_isHost == false)
    	{
    		m_IP = mIPaddressEdt.getText().toString();
    	}
    	
    	m_HttpServerIP = m_etHttpServerIP.getText().toString();
    	m_HttpServerPort = m_etHttpServerPort.getText().toString();
    	
    	
        ft.remove(this);
    }
 
    public void onTabReselected(Tab tab, FragmentTransaction ft) {
        // TODO Auto-generated method stub
 
    }
    
    public void setVideoSettingFragment(VideoSettingFragment vsetting)
    {
    	m_vsetting = vsetting;
    }
    
    public void setAudioSettingFragment(AudioSettingFragment asetting)
    {
    	m_asetting = asetting;
    }
    
    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        //todo: add your own
    }
    
    
    private void connectAsync()
    {
    	WmeClient.instance().Prepare();
    	if (m_isFile) {
    		if(m_audioConfigure.enable) {
	    		Bundle audioBundle = new Bundle();
	    		audioBundle.putInt("media.type", WmeParameters.WME_MEDIA_AUDIO);
	    		audioBundle.putString("file", m_audioConfigure.fileName);
	    		audioBundle.putString("source.ip", m_audioConfigure.sourceIP);
	    		audioBundle.putInt("source.port", m_audioConfigure.sourcePort);
	    		audioBundle.putString("destination.ip", m_audioConfigure.destinationIP);
	    		audioBundle.putInt("destination.port", m_audioConfigure.destinationPort);
				Message audioMsg = new Message();
				audioMsg.setData(audioBundle);
				audioMsg.what = WmeParameters.TPConnectFileMsg;
				WmeClient.instance().PushMessage(audioMsg);
    		}
    		if(m_videoConfigure.enable) {
	    		Bundle videoBundle = new Bundle();
	    		videoBundle.putInt("media.type", WmeParameters.WME_MEDIA_VIDEO);
	    		videoBundle.putString("file", m_videoConfigure.fileName);
	    		videoBundle.putString("source.ip", m_videoConfigure.sourceIP);
	    		videoBundle.putInt("source.port", m_videoConfigure.sourcePort);
	    		videoBundle.putString("destination.ip", m_videoConfigure.destinationIP);
	    		videoBundle.putInt("destination.port", m_videoConfigure.destinationPort);
				Message videoMsg = new Message();
				videoMsg.setData(videoBundle);
				videoMsg.what = WmeParameters.TPConnectFileMsg;
				WmeClient.instance().PushMessage(videoMsg);
    		}
			Log.v ("wme_android", "Connecting file!");
    	} else if (!m_isHost) {
			//WmeClient.instance().StartAsClient(m_ipAddress);
			Bundle bundle = new Bundle();
			bundle.putString("ip", m_IP);
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
    }
    
    public static Boolean isNetworkConnectted()
    {
    	return m_bIsNetworkConnectted;
    }
    
    public static void setNetworkConnectted(Boolean isConn)
    {
    	m_bIsNetworkConnectted = isConn;
    }

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch(v.getId())
		{
		case R.id.connectBttn:
//			ConnectingStatus async_connect = null;
			if (m_isHost == true)
			{
//				async_connect = new ConnectingStatus(m_MessageHandler, mConnectBar, m_isHost, m_IP);
//				async_connect.execute();
				//mConnectBar.setVisibility(View.VISIBLE);
				
				connectAsync();
				
				SystemClock.sleep(500 * 1);	//wait for tp connect, not necessary
				
				//mConnectBar.setVisibility(View.INVISIBLE);
				
				m_MessageHandler.sendEmptyMessage(Constants.EVENT_READY_TO_LAUNCH_PLAY);
				
				//m_MessageHandler.sendEmptyMessageDelayed(Constants.EVENT_FORCE_STOP_TIMER, m_iForceStopTimer);
			}
			else {
				Boolean isValidIP = true;
	    		String ipAddress = mIPaddressEdt.getText().toString();
	    		isValidIP = NetworkHelper.isIPaddressValid(ipAddress);

	    		// Start connection
	    		if (isValidIP) {		
	    			m_IP = ipAddress;
//	    			async_connect = new ConnectingStatus(m_MessageHandler, mConnectBar, m_isHost, m_IP);
//					async_connect.execute();
	    			
	    			//mConnectBar.setVisibility(View.VISIBLE);
					
					connectAsync();
					
					SystemClock.sleep(500 * 1);	//wait for tp connect, not necessary
					
					//mConnectBar.setVisibility(View.INVISIBLE);
					
					m_MessageHandler.sendEmptyMessage(Constants.EVENT_READY_TO_LAUNCH_PLAY);
					
					//m_MessageHandler.sendEmptyMessageDelayed(Constants.EVENT_FORCE_STOP_TIMER, m_iForceStopTimer);
	    		}
	    		else
	    		{
	    			Toast.makeText(getActivity(), "Invalid IP address", Toast.LENGTH_SHORT).show();
	    		}
			}
//			async_connect = null;
    		break;
		}
	}
	
	public void ReadyToLaunchPlay()
    {
    	Intent i = new Intent(getActivity(), PlayPanelActivity.class);  
    	i.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
    	i.putExtra(Constants.PICTURE_IN_PICTURE, m_pip);
    	i.putExtra(Constants.CODEC_PARAM_INDEX, m_vsetting.getCodecParamIndex());
    	i.putExtra(Constants.VIDEO_QUALITY_INDEX, m_vsetting.getVideoQualityIndex());
    	i.putExtra(Constants.CAMERA_INDEX, m_vsetting.getCameraIndex());
    	i.putExtra(Constants.CAMERA_PARAM_INDEX, m_vsetting.getCameraParamIndex());
    	i.putExtra(Constants.ENABLE_VIDEO_FILE_RENDER, m_vsetting.getEnableFileRender());
    	i.putExtra(Constants.ENABLE_AUDIO_FILE_RENDER, m_asetting.getEnableFileRender());
	    startActivity(i);
    }
	
	public void FinishPlayByForce()
	{
		Intent i = new Intent(getActivity(), PlayPanelActivity.class);  
    	i.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP);
    	i.putExtra("finish",true);
	    startActivity(i);
	}


	@Override
	public void onCheckedChanged(CompoundButton arg0, boolean isChecked) {
		// TODO Auto-generated method stub
		switch(arg0.getId())
		{
		case R.id.picinpic:
			m_pip = isChecked;
			break;
		case R.id.battery_performance:
			if (m_enableBatteryInfo == isChecked)
			{
				break;
			}
			if (isChecked == true)
			{
	    		String ipAddress = m_etHttpServerIP.getText().toString();
	    		Boolean isValidIP = NetworkHelper.isIPaddressValid(ipAddress);
	    		if (isValidIP == true)
	    		{
	    			String port = m_etHttpServerPort.getText().toString();
	    			Boolean isValidPort = NetworkHelper.isPortValid(port);
	    			if (isValidPort == true)
	    			{
	    				String http_url = "http://"+ipAddress+":"+port;
		    			((SettingActivity)getActivity()).triggerBatterySession(http_url);
	    			}
	    			else {
	    				Toast.makeText(getActivity(), "Invalid port", Toast.LENGTH_SHORT).show();
	    				m_cbBatteryInfo.setChecked(false);
	    				break;
	    			}
	    			
	    		}
	    		else{
	    			Toast.makeText(getActivity(), "Invalid IP address", Toast.LENGTH_SHORT).show();
	    			m_cbBatteryInfo.setChecked(false);
	    			break;
	    		}
			}
			else
			{
				((SettingActivity)getActivity()).teardownBatterySession();
			}
			m_enableBatteryInfo = isChecked;
			break;
		}
	}
	
	public void ShowCmdStatus(String cmd, String status)
	{
		if (m_tvCmdSent != null)
		{
			m_tvCmdSent.setText(cmd + ":" + status);
		}
	}
	
	private class PcapFileConfigure {
		public Boolean enable = false; 
		public String fileName = "";
		public String sourceIP = "";
		public int sourcePort = 0;
		public String destinationIP = "";
		public int destinationPort = 0; 
	}
	
 
}