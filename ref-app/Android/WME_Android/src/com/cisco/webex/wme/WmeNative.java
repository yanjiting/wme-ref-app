package com.cisco.webex.wme;

import android.util.Log;


/**
 * class WmeNative for internal using with JNI lib
 * The value of parameters can be referred in WmeParameters.java
 *  @mtype:  Wme Media Type
 *  @ttype:  Wme Track Type
 *  @dtype:  Wme Device Type
 *  @ctype:  Wme Codec Type
 *  @atype:  Wme Audio Out Type
 */
public class WmeNative {
	//load shared library
    static {
    	try {
    		System.loadLibrary("c++_shared");
    		
    		//load common library
    		System.loadLibrary("util");
    		System.loadLibrary("wqos");
    		System.loadLibrary("srtp");
    		System.loadLibrary("wrtp");
    		System.loadLibrary("tp");
    		
    		//load codec library
//    		System.loadLibrary("welsvp");
//    		System.loadLibrary("welsenc");
//    		System.loadLibrary("welsdec");
    		
    		//load wme library
    		System.loadLibrary("wsertp");
    		System.loadLibrary("wseclient");
    		System.loadLibrary("wmeutil");
    		System.loadLibrary("audioengine");
            System.loadLibrary("appshare");
    		System.loadLibrary("wmeclient");
			//System.loadLibrary("zbar");
    		
    		//load jni library
    		System.loadLibrary("wmenative-jni");
    	} 
    	catch (Exception e){
    		Log.v("wme_android","Load library failed");
    	}    	
    }

    public static native boolean Init();
    public static native void Uninit();
    public static native void HeartBeat();
    public static native void InitTPThread();	//only can be called once

    public static native String[] GetMediaDevices(int mtype, int dtype);
    public static native String[] GetMediaCapabilities(int mtype);
    
    public static native String[] GetCaptureParamList(int dtype, int dev_index);
    public static native void SetCaptureParam(int mtype, int dtype, int dev_index, int param_index);

    public static native int CreateMediaClient(int mtype);
    public static native int CreateMediaClientEx(int mtype, int ttype);
    public static native void DeleteMediaClient(int mtype);
    public static native void DeleteMediaClientEx(int mtype, int ttype);

    public static native void SetMediaDevice(int ttype, int dtype, int index);
    public static native void SetMediaCapability(int mtype, int ttype, int index);
    public static native void SetMediaCodec(int mtype, int ttype, int ctype);
    public static native void SetSessionCodec(int mtype, int ttype, int ctype);
    public static native void SetVideoQuality(int ttype, int quality);

    public static native int GetAudioVolume(int dtype);
    public static native void SetAudioVolume(int dtype, int volume);
    public static native void MuteAudio(int dtype);
    public static native void UnMuteAudio(int dtype);
    public static native boolean IsAudioMute(int dtype);
    public static native void SetAudioOutType(int atype);

    public static native void SetRenderView(int ttype, Object surface);
    public static native void SetRenderAspectRatioSameWithSource(int ttype, boolean same);
    public static native void SetRenderMode(int ttype, int mode);

    public static native void StartMediaSending(int mtype);
    public static native void StopMediaSending(int mtype);

    public static native void InitHost(int mtype);
    public static native void ConnectRemote(int mtype, String ipaddr);
    public static native void InitHostIce(int mtype, String myname, 
                                        String jingleip, int jingleport, String stunip, int stunport);
    public static native void ConnectRemoteIce(int mtype, String myname, String hostname,
                                        String jingleip, int jingleport, String stunip, int stunport);
    public static native void ConnectFile(int mtype, String filename, String sourceip, int sourceport, String destinationip, int destinationport);
    public static native void DisConnect(int mtype);

    public static native int StartMediaTrack(int mtype, int ttype);
    public static native int StopMediaTrack(int mtype, int ttype);

    public static native int GetTrackStatistics(int mtype, int ttype);
    public static native int GetSessionStatistics(int mtype);
    public static native int SetDumpEnabled(int dflag);

    public static native int SetVideoInputFile(String fname, int width, int height, int fps, int color);
    public static native void SetVideoSource(int stype);
    public static native int SetVideoOutputFile(String fname);
    public static native void SetVideoTarget(int stype);
    
    public static native int SetAudioInputFile(String fname, int channels, int samplerate, int bitspersample);
    public static native void SetAudioSource(int stype);
    public static native int SetAudioOutputFile(String fname);	//output wave format is fixed
    public static native void SetAudioTarget(int stype);
    public static native void DisableSendingFilterFeedback();
    
    public static native void SetQoSMaxLossRatio(float maxLossRatio);
    public static native void SetQoSMinBandwidth(int minBandwidth);
    public static native void SetInitialBandwidth(int initBandwidth);
    public static native void EnableQoS(boolean enable);
	public static native float CheckAudioOutputFile(String strAudioFileName);
    public static native void AddVideoFileRenderSink();
}

