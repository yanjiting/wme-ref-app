package com.cisco.wmeAndroid;

import com.cisco.webex.wme.WmeClient;

import android.app.Activity;
import android.app.Application;
import android.content.Intent;
import android.os.Handler;
import android.util.Log;
import android.view.Display;
import android.content.Context;
import android.content.pm.ActivityInfo;
import android.net.wifi.WifiManager;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;

/**
 * Created by weichen2 on 2014/4/21.
 */
public class MyApplication extends Application
{
    private Activity mCurrentActivity = null;

    public void onCreate() {
        super.onCreate();
    }

    public void switchToSettingActivity(String cmd){
    	Intent intent = new Intent(mCurrentActivity, SettingActivity.class);
       // intent.putExtra(COMMAND_LINE, cmd);
        mCurrentActivity.startActivity(intent);
    }

    public void switchToPlayPanelActivity(){
        Intent intent = new Intent(mCurrentActivity, PlayPanelActivity.class);
        mCurrentActivity.startActivity(intent);
    }

    public void setCurrentActivity(Activity mCurrentActivity){
        this.mCurrentActivity = mCurrentActivity;
    }


    // Connect as Host
    public String bdConnectAsHost(String arg){
    	//switchToSettingActivity("backdoor");
    	SettingActivity settingAct = (SettingActivity)(mCurrentActivity);
    	settingAct.bdConnectAsHost();
        Log.i("backdoor", "bdConnectAsHost has been called.");
    	return settingAct.bdGetHostIP();
    }
    
    public String bdConnectAsClient(String arg){
    	//switchToSettingActivity("backdoor");
    	SettingActivity settingAct = (SettingActivity)(mCurrentActivity);
    	settingAct.bdConnectAsClient(arg);
    	WmeClient.instance().AddVideoFileRenderSink();
    	Log.i("backdoor", "bdConnectAsClient has been called.");
    	return null;
    }
    
    public String bdEnableExternalVideoTrack(String str){
    	//switchToSettingActivity("backdoor");
    	SettingActivity settingAct = (SettingActivity)(mCurrentActivity);
    	settingAct.bdEnableExternalVideoTrack(true);
    	return null;
    }
    
    public String bdApplyExternalVideoTrack(String str){
    	//switchToSettingActivity("backdoor");
    	SettingActivity settingAct = (SettingActivity)(mCurrentActivity);
    	settingAct.bdApplyExternalVideoTrack();
    	return null;
    }
    
    
    public String bdSelectRawVideoFile(String str){
    	//switchToSettingActivity("backdoor");
    	SettingActivity settingAct = (SettingActivity)(mCurrentActivity);
    	settingAct.bdSelectRawVideoFile();
    	return null;
    }
    
    public String bdApplyVideoOutputFile(String str){
    	//switchToSettingActivity("backdoor");
    	SettingActivity settingAct = (SettingActivity)(mCurrentActivity);
    	settingAct.bdApplyVideoOutputFile(true);
    	//WmeClient.instance().AddVideoFileRenderSink();
    	return null;
    }
    
    public String bdSelectRawAudioFile(String str){
    	//switchToSettingActivity("backdoor");
    	SettingActivity settingAct = (SettingActivity)(mCurrentActivity);

        Log.i("MyApplication", "bdSelectRawAudioFile file = " + str);
    	settingAct.bdSelectRawAudioFile(str);
    	return null;
    }
    
    public String bdApplyAudioOutputFile(String str){
    	//switchToSettingActivity("backdoor");
    	SettingActivity settingAct = (SettingActivity)(mCurrentActivity);
    	settingAct.bdApplyAudioOutputFile(true);
  
    	return null;
    }
    
    public String bdApplyAudioRawFile(String str){
    	//switchToSettingActivity("backdoor");
    	SettingActivity settingAct = (SettingActivity)(mCurrentActivity);
    	settingAct.bdApplyAudioRawFile();
    	return null;
    }
    
    public String bdSelectAudioTab(String str){
    	//switchToSettingActivity("backdoor");
    	SettingActivity settingAct = (SettingActivity)(mCurrentActivity);
    	settingAct.bdSelectAudioTab();
    	return null;
    }

    public String bdSelectVideoTab(String str){
        //switchToSettingActivity("backdoor");
        SettingActivity settingAct = (SettingActivity)(mCurrentActivity);
        settingAct.bdSelectVideoTab();
        return null;
    }

    public String bdSelectResolution(String str){
	//switchToSettingActivity("backdoor");
        SettingActivity settingAct = (SettingActivity)(mCurrentActivity);
	    int nIdx = Integer.parseInt(str);
        settingAct.bdSelectResolution(nIdx);
        Log.i("WME_Android", "bdSelectResolution_MyApplication" + nIdx);
        return null;	
    }

    public String bdCheckAudioOutputFile(String str){

        String filePath = "/sdcard/" + str;
        Log.i("backdoor", "bdCheckAudioOutputFile"+ filePath);
    	float fRet = WmeClient.instance().CheckAudioOutputFile(filePath);
        String strRet = "true";
        strRet=String.valueOf(fRet);
        //Log.i("backdoor", "bdCheckAudioOutputFile nRet = " + str);
    	return strRet;
    }

    
}
