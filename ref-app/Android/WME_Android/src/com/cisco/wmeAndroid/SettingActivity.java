package com.cisco.wmeAndroid;
 
import java.io.File;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;

import org.apache.http.NameValuePair;
import org.apache.http.message.BasicNameValuePair;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import com.cisco.webex.wme.WmeClient;
import com.cisco.webex.wme.WmeParameters;
import com.cisco.wmeAndroid.PlayPanelActivity.CallStartParam;
import com.cisco.wmeAndroid.WebServer.OnHttpServerListener;
import com.webex.wseclient.WseEngine;

import android.app.ActionBar;
import android.app.FragmentManager;
import android.app.ProgressDialog;
import android.app.ActionBar.Tab;
import android.os.Build;
import android.os.Bundle;
import android.os.Debug;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.Display;
import android.view.WindowManager;
import android.app.Activity;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Configuration;
import android.database.Cursor;
 

public class SettingActivity extends Activity {
	private Tab tab;
    private MyApplication mMyApp;
	private NormalFragment 			mNormalFragment;
	private AudioSettingFragment 	mAudioSettingFragment;
	private VideoSettingFragment	mVideoSettingFragment;
    private QoSSettingFragment      mQoSSettingFragment;
	private MessageHandleThread m_MH_thread;
    public static Handler m_MessageHandler;
    private ProgressDialog mCreateMediaClientProgress = null;
    private BatteryHelper mBattery;
    private CpuHelper mCPU;
    //private JSONArray mPerfInfos = null;			//It is designed to restore jsons temporarily, but now we don't use it
    private PerfCmdGenerator mPerfCmdGen = null;
    private WebServer mHttpServer = null;
    private OnHttpServerListener mHttpListener = null;
    private String mHttpServerURL = Constants.HTTP_UNKNOWN_URL;
    private boolean mbBatterySessionByebye = true;
    private Handler m_MainThreadHandler = null;
 
    @Override
    public void onCreate(Bundle savedInstanceState) {
    	Log.i("wme_android","SettingActivity::onCreate ++");
    	
    	
    	//start profile tracing
    	//Debug.startMethodTracing("wmeAndroid");
    	mMyApp = (MyApplication)this.getApplicationContext();
        mMyApp.setCurrentActivity(this);
    	getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        super.onCreate(savedInstanceState);
        
        m_MH_thread = new MessageHandleThread();
        if (m_MH_thread != null)
        {
        	m_MH_thread.start();
        }
        
        // Create the actionbar
        ActionBar actionBar = getActionBar();
 
        // Hide Actionbar Icon
        // actionBar.setDisplayShowHomeEnabled(false);
 
        // Hide Actionbar Title
        actionBar.setDisplayShowTitleEnabled(false);
 
        // Create Actionbar Tabs
        actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_TABS);
 
        // Create Tab
        mNormalFragment = new NormalFragment();
        tab = actionBar.newTab().setTabListener(mNormalFragment);
        // Create your own custom icon
        tab.setText("Normal Connection");
        //tab.setIcon(arg0);
        actionBar.addTab(tab);
        
        //[ziyzhang]ICE is not supported in Sprint 13
//        // Create Tab
//        tab = actionBar.newTab().setTabListener(new IceFragment());
//        // Set Tab Title
//        tab.setText("ICE Connection");
//        actionBar.addTab(tab);
        
        // Create Audio Tab
        mAudioSettingFragment = new AudioSettingFragment();
        tab = actionBar.newTab().setTabListener(mAudioSettingFragment);
        // Set Tab Title
        tab.setText("Audio Settings");
        //tab.setIcon(arg0);
        actionBar.addTab(tab);
        
        // Create Video Tab
        mVideoSettingFragment = new VideoSettingFragment();
        tab = actionBar.newTab().setTabListener(mVideoSettingFragment);
        // Set Tab Title
        tab.setText("Video Settings");
        //tab.setIcon(arg0);
        actionBar.addTab(tab);

        mNormalFragment.setVideoSettingFragment(mVideoSettingFragment);
        mNormalFragment.setAudioSettingFragment(mAudioSettingFragment);

        // Create QoS Tab
        mQoSSettingFragment = new QoSSettingFragment();
        tab = actionBar.newTab().setTabListener(mQoSSettingFragment);
        // Set Tab Title
        tab.setText("QoS Settings");
        //tab.setIcon(arg0);
        actionBar.addTab(tab);
        
        //String path = getSharedLibPath();
        //Log.i("wme_android","SettingActivity::onCreate, lib path = "+path);
        
        m_MainThreadHandler = new Handler(){
			@Override
			public void handleMessage(Message msg){					
				
				switch (msg.what) {
					case Constants.EVENT_START_SESSION_COMPLETE:
					{
						mNormalFragment.ShowCmdStatus("START-SESSION", "COMPLETE");
						removeMessages(msg.what);
					}
					break;
					
					case Constants.EVENT_STOP_SESSION_COMPLETE:
					{
						mNormalFragment.ShowCmdStatus("STOP-SESSION", "COMPLETE");
						removeMessages(msg.what);
					}
					break;
					
					case Constants.EVENT_START_CALL_COMPLETE:
					{
						mNormalFragment.ShowCmdStatus("START-CALL", "COMPLETE");
						removeMessages(msg.what);
					}
					break;
					
					case Constants.EVENT_STOP_CALL_COMPLETE:
					{
						mNormalFragment.ShowCmdStatus("STOP-CALL", "COMPLETE");
						removeMessages(msg.what);
					}
					break;
					
					case Constants.EVENT_INFO_COMPLETE:
					{
						mNormalFragment.ShowCmdStatus("INFO", "COMPLETE");
						removeMessages(msg.what);
					}
					break;
					
					
					case Constants.EVENT_START_SESSION_SENT:
					{
						mNormalFragment.ShowCmdStatus("START-SESSION", "SENT");
						removeMessages(msg.what);
					}
					break;
					
					case Constants.EVENT_STOP_SESSION_SENT:
					{
						mNormalFragment.ShowCmdStatus("STOP-SESSION", "SENT");
						removeMessages(msg.what);
					}
					break;
					
					case Constants.EVENT_START_CALL_SENT:
					{
						mNormalFragment.ShowCmdStatus("START-CALL", "SENT");
						removeMessages(msg.what);
					}
					break;
					
					case Constants.EVENT_STOP_CALL_SENT:
					{
						mNormalFragment.ShowCmdStatus("STOP-CALL", "SENT");
						removeMessages(msg.what);
					}
					break;
					
					case Constants.EVENT_INFO_SENT:
					{
						mNormalFragment.ShowCmdStatus("INFO", "SENT");
						removeMessages(msg.what);
					}
					break;
					
					default:
					break;
				}
			}
        };
        
        //mPerfInfos = new JSONArray();
        mPerfCmdGen = new PerfCmdGenerator();
        
        
        //register battery receiver
        mBattery = new BatteryHelper();
        IntentFilter filter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        registerReceiver(mBattery, filter);
        
        mCPU = new CpuHelper();
        mCPU.EnableUsage(2*1000);	//interval to calc once
        
        mHttpServer = WebServer.getInstance(this);
        int wait_time = 0;
        while(mHttpServer.isRunning() == false)
        {
    		try {
				Thread.sleep(50);
				wait_time += 50;
				if (wait_time > 2000)
					break;
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
        }
        mHttpListener = new OnHttpServerListener() {
			@Override
			public void onHttpResult(int msg, int result, String strData) {
				// TODO Auto-generated method stub
				switch (msg)
				{
				case Constants.WEBCALL_START_SESSION:
					{
						Log.i("wme_android", "onHttpResult(WEBCALL_START_SESSION), res="+result+",detail:"+strData);
						if (result == Constants.MSG_ERR_NONE)
						{
							mbBatterySessionByebye = false;
						}
						if (m_MainThreadHandler != null)
				        {
							m_MainThreadHandler.sendEmptyMessage(Constants.EVENT_START_SESSION_COMPLETE);
				        }
					}
					break;
				case Constants.WEBCALL_STOP_SESSION:
					{
						Log.i("wme_android", "onHttpResult(WEBCALL_STOP_SESSION), res="+result+",detail:"+strData);
						//do not care about result
				    	mbBatterySessionByebye = true;
				    	if (m_MainThreadHandler != null)
				        {
				    		m_MainThreadHandler.sendEmptyMessage(Constants.EVENT_STOP_SESSION_COMPLETE);
				        }
					}
					break;	
				case Constants.WEBCALL_START_CALL:
					{
						Log.i("wme_android", "onHttpResult(WEBCALL_START_CALL), res="+result+",detail:"+strData);
						if (m_MainThreadHandler != null)
				        {
							m_MainThreadHandler.sendEmptyMessage(Constants.EVENT_START_CALL_COMPLETE);
				        }
					}
					break;
				case Constants.WEBCALL_STOP_CALL:
					{
						Log.i("wme_android", "onHttpResult(WEBCALL_STOP_CALL), res="+result+",detail:"+strData);
						if (m_MainThreadHandler != null)
				        {
							m_MainThreadHandler.sendEmptyMessage(Constants.EVENT_STOP_CALL_COMPLETE);
				        }
					}
					break;
				case Constants.WEBCALL_INFO:
					{
						Log.i("wme_android", "onHttpResult(WEBCALL_INFO), res="+result);
						if (m_MainThreadHandler != null)
				        {
							m_MainThreadHandler.sendEmptyMessage(Constants.EVENT_INFO_COMPLETE);
				        }
					}
					break;
				}
			}
        	
        };
        mHttpServer.setHttpServerListener(mHttpListener);
        
        
        if (m_MessageHandler != null)
        {
        	m_MessageHandler.sendEmptyMessageDelayed(Constants.EVENT_GET_BATTERY, 1000);
        	m_MessageHandler.sendEmptyMessageDelayed(Constants.EVENT_INIT, 500);
        }
        mCreateMediaClientProgress = ProgressDialog.show(this, "Initializing", "Please wait......", true, true);
        
        
        Log.i("wme_android","SettingActivity::onCreate --");
    }
    
	//get host ip for backdoor
    public String bdGetHostIP()
    {
    	return mNormalFragment.bdGetHostIP();
    }
    public void bdConnectAsClient(String ipAddr)
    {
    	ActionBar actionBar = getActionBar();
    	Tab tab = actionBar.getTabAt(0);
    	actionBar.selectTab(tab);
    	mNormalFragment.bdSetAsClient();
    	mNormalFragment.bdConnectAsClient(ipAddr);
    }
    
    public void bdConnectAsHost()
    {
    	ActionBar actionBar = getActionBar();
    	Tab tab = actionBar.getTabAt(0);
    	actionBar.selectTab(tab);
    	mNormalFragment.bdConnectAsHost();
    }
    public void bdEnableExternalVideoTrack(boolean isChecked){
    	
    	ActionBar actionBar = getActionBar();
    	Tab tab = actionBar.getTabAt(2);
    	actionBar.selectTab(tab);
    	mVideoSettingFragment.bdEnableExternalVideoTrack(isChecked);
    }
    
    public void bdApplyExternalVideoTrack(){
    	ActionBar actionBar = getActionBar();
    	Tab tab = actionBar.getTabAt(2);
    	actionBar.selectTab(tab);
    	mVideoSettingFragment.bdApplyExternalVideoTrack();
    }
    
    public void bdSelectRawVideoFile(){
    	mVideoSettingFragment.bdSelectRawVideoFile();
    }
    
    public void bdApplyVideoOutputFile(boolean isChecked){
    	ActionBar actionBar = getActionBar();
    	Tab tab = actionBar.getTabAt(2);
    	actionBar.selectTab(tab);
    	mVideoSettingFragment.bdApplyVideoOutputFile(isChecked);
    }
    
    public void bdApplyAudioRawFile(){
    	ActionBar actionBar = getActionBar();
    	Tab tab = actionBar.getTabAt(1);
    	actionBar.selectTab(tab);
    	mAudioSettingFragment.bdApplyAudioRawFile();
    }
    
    public void bdSelectAudioTab(){
    	ActionBar actionBar = getActionBar();
    	Tab tab = actionBar.getTabAt(1);
    	actionBar.selectTab(tab);
    	Log.i("WME_Android", "bdSelectAudioTab");
    	//mAudioSettingFragment.bdApplyAudioRawFile();
    }

    public void bdSelectVideoTab(){
        ActionBar actionBar = getActionBar();
        Tab tab = actionBar.getTabAt(2);
        actionBar.selectTab(tab);
        Log.i("WME_Android", "bdSelectVideoTab");
        //mAudioSettingFragment.bdApplyAudioRawFile();
    }

    public void bdSelectResolution(int nIdx){
        mVideoSettingFragment.bdSelectResolution(nIdx);
        Log.i("WME_Android", "bdSelectResolution_SettingActivity" + nIdx);
    }
    
    public void bdSelectRawAudioFile(String str){
    	ActionBar actionBar = getActionBar();
    	Tab tab = actionBar.getTabAt(1);
    	actionBar.selectTab(tab);
    	mAudioSettingFragment.bdSelectRawAudioFile(str);
    }
    
    public void bdApplyAudioOutputFile(boolean isChecked){
    	ActionBar actionBar = getActionBar();
    	Tab tab = actionBar.getTabAt(1);
    	actionBar.selectTab(tab);
    	mAudioSettingFragment.bdApplyAudioOutputFile(isChecked);
    }
    

    @Override
    public void onDestroy()
    {
    	Log.i("wme_android","SettingActivity::onDestroy ");  
    		
    	if (m_MessageHandler != null)
    	{
    		m_MessageHandler.removeMessages(Constants.EVENT_GET_BATTERY);
    		m_MessageHandler.getLooper().quit();
    		Log.i("wme_android","SettingActivity::onDestroy, after close msg handler");
    	}
    	m_MH_thread = null;
    	
    	unregisterReceiver(mBattery);
    	mBattery = null;
    	
    	mCPU.DisableUsage();
    	mCPU = null;
    	
    	//shut down server
    	Log.i("wme_android","SettingActivity::onDestroy, before shut down server"); 
    	//mHttpServer.setHttpServerListener(null);	//don't invoke it, it is risky! Thread issue!
    	mHttpServer.destroy();
    	Log.i("wme_android","SettingActivity::onDestroy, after shut down server"); 
    	mPerfCmdGen = null;
    	mHttpListener = null;
    	mHttpServer = null;
    	
    	WmeClient.instance().Uninit();
    	Log.i("wme_android","SettingActivity::onDestroy, Uninit ");
    	
    	super.onDestroy();
    	
    	//stop profile tracing
    	//Debug.stopMethodTracing();
    	
    	
    	//finish();		//for valgrind
		//System.exit(0);	//for valgrind
    }
    
    @Override
	public void onConfigurationChanged(Configuration newConfig) {
		Log.i("wme_android", "SettingActivity::onConfigurationChanged");
		super.onConfigurationChanged(newConfig);
	}
    
    @Override
    public void onPause()
    {
    	Log.i("wme_android","SettingActivity::onPause ");
    	//stop preview , if directly press "back"
//    	if (mVideoSettingFragment.isPreviewOn() == true)
//    	{
//    		WmeClient.instance().StopPreviewTrack();
//    		WmeClient.instance().SetPreviewWindow(null);
//    		Log.i("wme_android","SettingActivity::onPause, kill preview ");
//    	}
    	//WmeClient.instance().DeletePreviewClient();	//FIXME: DeletePreviewClient is not clearly defined 
    	
    	
    	if (mCreateMediaClientProgress != null && mCreateMediaClientProgress.isShowing() == true)
    	{
			mCreateMediaClientProgress.dismiss();
			mCreateMediaClientProgress = null;
    	}
    	
    	super.onPause();
    }
    
    @Override
    public void onResume()
    {
    	Log.i("wme_android","SettingActivity::onResume ");
    	
    	if (mNormalFragment.isNetworkConnectted() == true)
    	{
    		Message msg = new Message();
    		msg.what = WmeParameters.TPDisconnect;	
    		WmeClient.instance().PushMessage(msg);
    		Log.i("wme_android","SettingActivity::onResume, Disconnect async ");
        	//WmeClient.instance().Disconnect();		//correspond with startAsHost or startAsClient, 
        	//Log.i("wme_android","SettingActivity::onResume, Disconnect sync ");
    	}
    	
    	super.onResume();
    }
    
    @Override
    public void onBackPressed() {
    	Log.i("wme_android","SettingActivity::onBackPressed ");

    	//performance test: stop session
    	if (!mHttpServerURL.equals(Constants.HTTP_UNKNOWN_URL))
    	{
    		Log.i("wme_android","SettingActivity::onBackPressed, send stop session to http server ");
    		mHttpServer.postHttp(Constants.WEBCALL_STOP_SESSION, mHttpServerURL, mPerfCmdGen.cmd_stopSession_nvp());
    		if (m_MainThreadHandler != null)
	        {
				m_MainThreadHandler.sendEmptyMessage(Constants.EVENT_STOP_SESSION_SENT);
	        }
    		mHttpServerURL = Constants.HTTP_UNKNOWN_URL;
    		int wait_time = 0;
        	while(mbBatterySessionByebye == false)
        	{
        		try {
    				Thread.sleep(50);
    				wait_time += 50;
    				if (wait_time > 2000)
    					break;
    			} catch (InterruptedException e) {
    				// TODO Auto-generated catch block
    				e.printStackTrace();
    			}
        	}
    	}
    	
    	super.onBackPressed();
    	Log.i("wme_android","SettingActivity::onBackPressed over");
    }
    
    public String getSharedLibPath() {
    	PackageManager pm = getPackageManager();
    	PackageInfo p = null;
    	try {
    		p = pm.getPackageInfo(this.getPackageName(), 0);
    		File sharedLibraryDir = new File(p.applicationInfo.dataDir + "/lib");
    		return sharedLibraryDir.getPath();// + File.separator;
    	} catch (NameNotFoundException e) {
    		e.printStackTrace();
    	}
    	return null;
    }
    
    private class MessageHandleThread extends Thread{				
		@Override
		public void run(){
			Looper.prepare();
			m_MessageHandler = new Handler(){
				@Override
				public void handleMessage(Message msg){					
					//Log.i("wme_android","SettingActivity, handleMessage, what= "+msg.what);
					switch (msg.what) {
						case Constants.EVENT_INIT:
						{
							Boolean bRes = WmeClient.instance().Init();	
					        Log.i("wme_android","SettingActivity, Init res = "+bRes);
					        if(bRes == true)
					        {
					        	 //set rotation at first
					            WindowManager windowManager = getWindowManager();  
					            Display display = windowManager.getDefaultDisplay();  
					            int screenRotation = display.getRotation(); 
					            WseEngine.setDisplayRotation(screenRotation);
					            Log.i("wme_android","SettingActivity, screenRotation = "+screenRotation);
					        	
					            removeMessages(msg.what);
					            
					            if (mCreateMediaClientProgress != null && mCreateMediaClientProgress.isShowing() == true)
						    	{
									mCreateMediaClientProgress.dismiss();
									mCreateMediaClientProgress = null;
						    	}
					            
					        }
						}
						break;	
						
						case Constants.EVENT_GET_BATTERY:
						{
							if (mCPU.usage >= 0)
							{
								//performance test: info
								if (!mHttpServerURL.equals(Constants.HTTP_UNKNOWN_URL))
								{
									mHttpServer.postHttp(Constants.WEBCALL_INFO, mHttpServerURL, mPerfCmdGen.cmd_info_nvp());
									if (m_MainThreadHandler != null)
							        {
										m_MainThreadHandler.sendEmptyMessage(Constants.EVENT_INFO_SENT);
							        }
								}
								
								Log.i("wme_android", "[WX2WME]Battery percentage = "+mBattery.battery_percent+", charging = "+mBattery.charging+", CPU usage = "+mCPU.usage);
							}
							sendEmptyMessageDelayed(Constants.EVENT_GET_BATTERY, Constants.BATTERY_INTERVAL);
						}
						break;
						
						case Constants.EVENT_POST_PERFORMANCE_CMD:
						{
							
						}
						break;
						
						case Constants.EVENT_START_CALL:
						{
							//performance test: start call
							if (mHttpServerURL != Constants.HTTP_UNKNOWN_URL)
							{
								CallStartParam param = (CallStartParam)msg.obj;
								mHttpServer.postHttp(Constants.WEBCALL_START_CALL, mHttpServerURL, mPerfCmdGen.cmd_startCall_nvp(param));
								if (m_MainThreadHandler != null)
						        {
									m_MainThreadHandler.sendEmptyMessage(Constants.EVENT_START_CALL_SENT);
						        }
							}
							removeMessages(msg.what);
						}
						break;
						
						case Constants.EVENT_STOP_CALL:
						{
							//performance test: stop call
							if (!mHttpServerURL.equals(Constants.HTTP_UNKNOWN_URL))
							{
								mHttpServer.postHttp(Constants.WEBCALL_STOP_CALL, mHttpServerURL, mPerfCmdGen.cmd_stopCall_nvp());
								if (m_MainThreadHandler != null)
						        {
									m_MainThreadHandler.sendEmptyMessage(Constants.EVENT_STOP_CALL_SENT);
						        }
							}
							removeMessages(msg.what);
						}
						break;

						default:
						break;
					}					
				}
			};
			Looper.loop();
		}		
	}
    
    public void triggerBatterySession(String httpurl) {
    	//performance test: start session
    	if (mHttpServerURL.equals(Constants.HTTP_UNKNOWN_URL))
    	{
    		Log.i("wme_android", "triggerBatterySession");
	    	mHttpServerURL = httpurl;
	        if (!mHttpServerURL.equals(Constants.HTTP_UNKNOWN_URL))
	        {
	        	WmeClient.instance().DisableSendingFilterFeedback();
	        	mHttpServer.postHttp(Constants.WEBCALL_START_SESSION, mHttpServerURL, mPerfCmdGen.cmd_startSession_nvp());
	        	if (m_MainThreadHandler != null)
		        {
					m_MainThreadHandler.sendEmptyMessage(Constants.EVENT_START_SESSION_SENT);
		        }
	        }
    	}
    }
    
    public void teardownBatterySession()
    {
    	if (!mHttpServerURL.equals(Constants.HTTP_UNKNOWN_URL))
    	{
    		Log.i("wme_android", "teardownBatterySession");
    		mHttpServer.postHttp(Constants.WEBCALL_STOP_SESSION, mHttpServerURL, mPerfCmdGen.cmd_stopSession_nvp());
    		if (m_MainThreadHandler != null)
	        {
				m_MainThreadHandler.sendEmptyMessage(Constants.EVENT_STOP_SESSION_SENT);
	        }
    		mHttpServerURL = Constants.HTTP_UNKNOWN_URL;
    		try {
				Thread.sleep(500);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
    	}
    }
    
    private class PerfCmdGenerator {
    	private String getDate()
    	{
    		SimpleDateFormat sDateFormat = new SimpleDateFormat("MM-dd-yyyy-hh-mm-ss");       
    		String date = sDateFormat.format(new java.util.Date());
    		return date;
    	}
    	
    	public JSONObject cmd_startSession_json()
    	{
    		//todo: construct json object
			JSONObject startSession = new JSONObject();
			try {
				startSession.put("SESSION", "PERFORMANCE");
				startSession.put("COMMAND", "START-SESSION");
				startSession.put("TIME", getDate());
				startSession.put("DEVICE", android.os.Build.BRAND + "_" + android.os.Build.MODEL);
				startSession.put("OS", "Android" + "(" + android.os.Build.VERSION.RELEASE + ")");
				startSession.put("CPU", android.os.Build.CPU_ABI);
				startSession.put("Memory", "NotSupport");			//not supported
			} catch (JSONException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			return startSession;
    	}
    	
    	public List<NameValuePair> cmd_startSession_nvp()
    	{
    		List<NameValuePair> params = new ArrayList<NameValuePair>();  
            params.add(new BasicNameValuePair("SESSION", "PERFORMANCE"));  
            params.add(new BasicNameValuePair("COMMAND", "START-SESSION"));  
            params.add(new BasicNameValuePair("TIME", getDate())); 
            params.add(new BasicNameValuePair("DEVICE", android.os.Build.BRAND + "_" + android.os.Build.MODEL)); 
            params.add(new BasicNameValuePair("OS", "Android" + "(" + android.os.Build.VERSION.RELEASE + ")"));
            params.add(new BasicNameValuePair("CPU", android.os.Build.CPU_ABI));
            params.add(new BasicNameValuePair("MEMORY", "NotSupport"));
            
            return params;
    	}
    	
    	public JSONObject cmd_stopSession_json()
    	{
    		//todo: construct json object
			JSONObject stopSession = new JSONObject();
			try {
				stopSession.put("SESSION", "PERFORMANCE");
				stopSession.put("COMMAND", "STOP-SESSION");
				stopSession.put("TIME", getDate());
			} catch (JSONException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			return stopSession;
    	}
    	
    	public List<NameValuePair> cmd_stopSession_nvp()
    	{
    		List<NameValuePair> params = new ArrayList<NameValuePair>();  
            params.add(new BasicNameValuePair("SESSION", "PERFORMANCE"));  
            params.add(new BasicNameValuePair("COMMAND", "STOP-SESSION"));  
            params.add(new BasicNameValuePair("TIME", getDate())); 
            
            return params;
    	}
    	
    	public JSONObject cmd_startCall_json(CallStartParam param)
    	{
    		JSONObject startCall = new JSONObject();
			try {
				startCall.put("SESSION", "PERFORMANCE");
				startCall.put("COMMAND", "START-CALL");
				startCall.put("TIME", getDate());
				if (mVideoSettingFragment.isVideoOn() == true)
				{
					startCall.put("SEND-VIDEO", "ON");
					startCall.put("RECV-VIDEO", "ON");
					//startCall.put("RESOLUTION", Integer.toString(param.res_width)+"-"+Integer.toString(param.res_height));			//not supported
					//startCall.put("FPS", Integer.toString(param.fps));					//not supported
				}
				else
				{
					startCall.put("SEND-VIDEO", "OFF");
					startCall.put("RECV-VIDEO", "OFF");
				}
				
				if (mAudioSettingFragment.isAudioOn() == true)
					startCall.put("AUDIO", "ON");
				else
					startCall.put("AUDIO", "OFF");
			} catch (JSONException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			return startCall;
    	}
    	
    	public List<NameValuePair> cmd_startCall_nvp(CallStartParam param)
    	{
    		List<NameValuePair> params = new ArrayList<NameValuePair>();  
            params.add(new BasicNameValuePair("SESSION", "PERFORMANCE"));  
            params.add(new BasicNameValuePair("COMMAND", "START-CALL"));  
            params.add(new BasicNameValuePair("TIME", getDate())); 
            if (mVideoSettingFragment.isVideoOn() == true)
            {
            	params.add(new BasicNameValuePair("SEND-VIDEO", "ON"));
            	params.add(new BasicNameValuePair("RECV-VIDEO", "ON"));
            	//params.add(new BasicNameValuePair("RESOLUTION", Integer.toString(param.res_width)+"-"+Integer.toString(param.res_height)));
            	//params.add(new BasicNameValuePair("FPS", Integer.toString(param.fps)));
            }
            else
            {
            	params.add(new BasicNameValuePair("SEND-VIDEO", "OFF"));
            	params.add(new BasicNameValuePair("RECV-VIDEO", "OFF"));
            }
            
            if (mAudioSettingFragment.isAudioOn() == true)
            	params.add(new BasicNameValuePair("AUDIO", "ON"));
            else
            	params.add(new BasicNameValuePair("AUDIO", "OFF"));
            
            return params;
    	}
    	
    	public JSONObject cmd_stopCall_json()
    	{
    		JSONObject stopCall = new JSONObject();
			try {
				stopCall.put("SESSION", "PERFORMANCE");
				stopCall.put("COMMAND", "STOP-CALL");
				stopCall.put("TIME", getDate());
			} catch (JSONException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			
			return stopCall;
    	}
    	
    	public List<NameValuePair> cmd_stopCall_nvp()
    	{
    		List<NameValuePair> params = new ArrayList<NameValuePair>();  
            params.add(new BasicNameValuePair("SESSION", "PERFORMANCE"));  
            params.add(new BasicNameValuePair("COMMAND", "STOP-CALL"));  
            params.add(new BasicNameValuePair("TIME", getDate())); 
            
            return params;
    	}
    	
    	public JSONObject cmd_info_json()
    	{
    		//todo: construct json object
			JSONObject info = new JSONObject();
			try {
				info.put("SESSION", "PERFORMANCE");
				info.put("COMMAND", "INFO");
				info.put("TIME", getDate());
				info.put("BATTERY", Integer.toString(mBattery.battery_percent));
				if (mBattery.charging == true)
					info.put("CHARGING", "YES");
				else
					info.put("CHARGING", "NO");
				info.put("CPU", Integer.toString((int)mCPU.usage));
			} catch (JSONException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			return info;
    	}
    	
    	public List<NameValuePair> cmd_info_nvp()
    	{
    		List<NameValuePair> params = new ArrayList<NameValuePair>();  
            params.add(new BasicNameValuePair("SESSION", "PERFORMANCE"));  
            params.add(new BasicNameValuePair("COMMAND", "INFO"));  
            params.add(new BasicNameValuePair("TIME", getDate())); 
            params.add(new BasicNameValuePair("BATTERY", Integer.toString(mBattery.battery_percent))); 
            if (mBattery.charging == true)
            	params.add(new BasicNameValuePair("CHARGING", "YES"));
            else
            	params.add(new BasicNameValuePair("CHARGING", "NO"));
            params.add(new BasicNameValuePair("CPU", Integer.toString((int)mCPU.usage)));
            
            params.add(new BasicNameValuePair("SEND-RES", Integer.toString(StatisticData.videoEncodeMaxWidth) + "-" + Integer.toString(StatisticData.videoEncodeMaxHeight)));
            params.add(new BasicNameValuePair("SEND-FPS", Integer.toString(StatisticData.videoEncodeMaxFPS)));
            
            params.add(new BasicNameValuePair("RECV-RES", Integer.toString(StatisticData.videoDecodeWidth) + "-" + Integer.toString(StatisticData.videoDecodeHeight)));
            params.add(new BasicNameValuePair("RECV-FPS", Integer.toString(StatisticData.videoDecodeFPS)));
            
            return params;
    	}

    }

    
}
