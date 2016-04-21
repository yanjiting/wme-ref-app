package com.wx2.clickcall;

import android.database.ContentObserver;
import android.media.AudioManager;
import android.os.Handler;
import android.util.Log;


public class VolumeSettingChangedListener extends ContentObserver {

    private int currVoiceVolume = 0;
    private int currSysVolume = 0;
    private AudioManager audioManager;
    private boolean mLoopback;

    public VolumeSettingChangedListener(Handler handler, AudioManager audioManager, boolean bLoopback) {
        super(handler);
        this.mLoopback = bLoopback;
        this.audioManager = audioManager;
        currVoiceVolume = audioManager.getStreamVolume(AudioManager.STREAM_VOICE_CALL);
        currSysVolume = audioManager.getStreamVolume(AudioManager.STREAM_SYSTEM);
    }
    
    @Override
    public void onChange(boolean selfChange) {
        super.onChange(selfChange);
        //will change later, decide by the device type, Google serials is STREAM_SYSTEM, Samsung S3,S4 is STREAM_VOICE_CALL
        int voiceVol = audioManager.getStreamVolume(AudioManager.STREAM_VOICE_CALL);
        int systemVol = audioManager.getStreamVolume(AudioManager.STREAM_SYSTEM);
        int maxVol = audioManager.getStreamMaxVolume(AudioManager.STREAM_VOICE_CALL);
        int vol = voiceVol;
        if (voiceVol != currVoiceVolume) {
          currVoiceVolume = voiceVol;
        } else if (systemVol != currSysVolume) {
            maxVol = audioManager.getStreamMaxVolume(AudioManager.STREAM_SYSTEM);
            vol = systemVol;
            currSysVolume = vol;
        } else {
            return;
        }
        int volSetting = 65535 >> (maxVol - vol);
        if (0 == vol) {
            volSetting = 0;
        }
        Log.i("ClickCall", "VolumeSettingChangedListener,onChange volume setting:" + volSetting + ",max volume:" + maxVol + ",current volume:" + vol);
        if(mLoopback) {
        	Loopback.instance().setAudioVolume(volSetting);
        }else {
        	PeerCall.instance().setAudioVolume(volSetting);
        }
        
    }
}
