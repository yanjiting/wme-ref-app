package com.cisco.webex.wme;

import java.util.LinkedList;
import java.util.Queue;

import com.cisco.wmeAndroid.NormalFragment;

import android.opengl.GLSurfaceView;
import android.os.Message;
import android.util.Log;


/**
 * class WmeClient is for java UI call using
 * how to use:
 *  step1: WmeClient.Init()
 *  step2: WmeClient.StartAsHost() or WmeClient.StartAsClient()
 *  step3: WmeClient.CreateMediaClient() for audio and video
 *  step4: WmeClient.SetCamera/SetMic/SetSpeaker()
 *  step5: WmeClient.Set Audio/Video Capability/Codec
 *  step6: WmeClient.Start/Stop Audio/Video Track
 *  step7: WmeClient.Start/Stop Audio/Video Sending
 *  step8: WmeClient.Disconnect()
 *  step9: WmeClient.Uninit()
 */
public class WmeClient {
	private static WmeClient m_self = null;
	private static boolean m_bTPMainThreadInitOnce = false;
	
	//private int mVideoCapIdx;
	private int mAudioCapIdx;	
	
//	private boolean mStartAudioTrack;
	private boolean mStartVideoTrack;
//	private boolean mStartPreviewTrack;
//	private boolean mStartAudioSending;
//	private boolean mStartVideoSending;
    private boolean mStartShareTrack = false;
	
	//private GLSurfaceView mLocalRender;
	//private GLSurfaceView mRemoteRender;
	//private GLSurfaceView mPreviewRender;
	
	private boolean mEnableAudio;
	private boolean mEnableVideo;
    private boolean mEnableShare = true;
    private boolean mEnableQoS = true;
	
	private boolean mEnableExternalAudioInput;
	private boolean mEnableExternalVideoInput;
	private boolean mEnableAudioOutputFile;
	private boolean mEnableVideoOutputFile;
	private String mVInputFilename;
	private int mHeight;
	private int mWidth;
	private int mColor;
	private int mFPS;
	private String mAInputFilename;
	private int mChannels;
	private int mSampleRate;
	private int mBitsPerSample;
	private String mAOutputFilename;
	private String mVOutputFilename;
    
    private float mQoSMaxLossRatio = 0.0f;
    private int mQoSMinBandwidth = 0;
    private int mInitialBandwidth = 0;
	
	public static WmeClient instance()
	{
		if (m_self == null)
		{
			m_self = new WmeClient();
		}
		return m_self;
	}
	
	private WmeClient()
	{
		//mVideoCapIdx = -1;
		mAudioCapIdx = -1;
		
//		mStartAudioTrack = false;
		mStartVideoTrack = false;
//		mStartPreviewTrack = false;
//		
//		mStartAudioSending = false;
//		mStartVideoSending = false;
		
//		mLocalRender = null;
//		mRemoteRender = null;
//		mPreviewRender = null;
		mEnableAudio = true;
		mEnableVideo = true;
        mEnableQoS = true;
		
        mEnableExternalAudioInput = false;
        mEnableExternalVideoInput = false;
		mVInputFilename = "";
		mHeight = 0;
		mWidth = 0;
		mColor = WmeParameters.WmeVideoUnknown;
		mFPS = 0;
		mAInputFilename = "";
		mChannels = 0;
		mSampleRate = 0;
		mBitsPerSample = 0;
		mAOutputFilename = "";
		mEnableVideoOutputFile = false;
		mEnableAudioOutputFile = false;
		mVOutputFilename = "";
	}

	///============================================
	/// Init and Uninit
	//WmeInit, create media session, create sync box, create notifier/enumerator
    public boolean Init()	
    {
    	Log.i("wme_android", "WmeClient::Init");
    	InitHB();
    	
    	if (m_bTPMainThreadInitOnce == false)
    	{
	    	Message msg = new Message();
			msg.what = WmeParameters.TPInitMainThread;	
			WmeClient.instance().PushMessage(msg);
			//wait m_bTPMainThreadInitOnce be true
			while (m_bTPMainThreadInitOnce == false)
			{
				try {
					Thread.sleep(10);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
    	}
    	
        return WmeNative.Init();
    }

    public void Uninit()
    {
    	Log.i("wme_android", "WmeClient::Uninit");    	
    	//[WARN] User thread must be global, so never stop/destroy it
    	//UninitHB();
        WmeNative.Uninit();
        m_self = null;
    }

    ///============================================
    /// init network as one role
    public int StartAsHost()
    {
    	Log.i("wme_android", "WmeClient::StartAsHost");    	
    	if (mEnableAudio)
    		WmeNative.InitHost(WmeParameters.WME_MEDIA_AUDIO);
    	
    	if (mEnableVideo)
    		WmeNative.InitHost(WmeParameters.WME_MEDIA_VIDEO);
        
        return 0;
    }

    public int StartAsClient(String ipstr)
    {
    	Log.i("wme_android", "WmeClient::StartAsClient");
   		if (mEnableAudio)
   			WmeNative.ConnectRemote(WmeParameters.WME_MEDIA_AUDIO, ipstr);
   		
   		if (mEnableVideo)
   			WmeNative.ConnectRemote(WmeParameters.WME_MEDIA_VIDEO, ipstr);

        if( mEnableShare )
            WmeNative.ConnectRemote(WmeParameters.WME_MEDIA_SHARE, ipstr);
        return 0;
    }
    
    public int StartAsFile(int mtype, String filename, String sourceip, int sourceport, String destinationip, int destinationport)
    {
    	Log.i("wme_android", "WmeClient::StartAsFile, mediatype =" + mtype);
   		if ((mEnableAudio && WmeParameters.WME_MEDIA_AUDIO == mtype) || (mEnableVideo && WmeParameters.WME_MEDIA_VIDEO == mtype))
   			WmeNative.ConnectFile(mtype, filename, sourceip, sourceport, destinationip, destinationport);
    	return 0;
    }

    public int Disconnect()
    {
    	Log.i("wme_android", "WmeClient::Disconnect");
    	DeleteMediaClient();
    	if (mEnableAudio)
    		WmeNative.DisConnect(WmeParameters.WME_MEDIA_AUDIO);
    	if (mEnableVideo)
    		WmeNative.DisConnect(WmeParameters.WME_MEDIA_VIDEO);

        if( mEnableShare )
            WmeNative.DisConnect(WmeParameters.WME_MEDIA_SHARE);
        return 0;
    }
    
    public void EnableAudio(boolean enable) {
    	mEnableAudio = enable;
    }
    
    public boolean IsEnableAudio() {
    	return mEnableAudio;
    }
    
    public void EnableVideo(boolean enable) {
    	mEnableVideo = enable;
    }
    
    public boolean IsEnableVideo() {
    	return mEnableVideo;
    }


    ///============================================
    /// for audio/video client
    public int CreateMediaClient(int mtype)
    {       
    	if (mtype == WmeParameters.WME_MEDIA_VIDEO) {
    		Log.i("wme_android", "WmeClient::CreateMediaClient for video: enable=" + mEnableExternalVideoInput + 
    				", file=" + mVInputFilename + ", width=" + mWidth + ", height=" + mHeight + ", color="+ mColor + ", fps=" + mFPS);
    		
    		if(mEnableExternalVideoInput && mVInputFilename.length() > 0 && mWidth > 0 && mHeight > 0 && mColor > 0) {   
    			WmeNative.SetVideoInputFile(mVInputFilename, mWidth, mHeight, mFPS, mColor);
    			WmeNative.SetVideoSource(WmeParameters.SOURCE_FILE);
    		}else {
    			WmeNative.SetVideoSource(WmeParameters.SOURCE_CAMERA);
    		}
    		
    		if(mEnableVideoOutputFile && mVOutputFilename.length() > 0)
    		{
    			WmeNative.SetVideoOutputFile(mVOutputFilename);
    			WmeNative.SetVideoTarget(WmeParameters.TARGET_FILE);
    		}
    		else
    		{
    			WmeNative.SetVideoTarget(WmeParameters.TARGET_SCREEN);
    		}
    	}
    	else if (mtype == WmeParameters.WME_MEDIA_AUDIO)
    	{
    		Log.i("wme_android", "WmeClient::CreateMediaClient for audio, enable="+mEnableExternalAudioInput+",mAFilename="+mAInputFilename+",mChannels="+mChannels+",mSampleRate="+mSampleRate+",mBitsPerSample="+mBitsPerSample);
    		if(mEnableExternalAudioInput && mAInputFilename.length() > 0 && mChannels > 0 && mBitsPerSample > 0 && mSampleRate > 0)
    		{
    			WmeNative.SetAudioInputFile(mAInputFilename, mChannels, mSampleRate, mBitsPerSample);
    			WmeNative.SetAudioSource(WmeParameters.SOURCE_FILE);
    		}
    		else
    		{
    			WmeNative.SetAudioSource(WmeParameters.SOURCE_MIC);
    		}
    		
    		if(mEnableAudioOutputFile && mAOutputFilename.length() > 0)
    		{
    			WmeNative.SetAudioOutputFile(mAOutputFilename);
    			WmeNative.SetAudioTarget(WmeParameters.TARGET_FILE);
    		}
    		else
    		{
    			WmeNative.SetAudioTarget(WmeParameters.TARGET_SPEAKER);
    		}
    		
    	}
    	return WmeNative.CreateMediaClient(mtype);
    }
    
    public void DeleteMediaClient(int mtype)
    {
    	WmeNative.DeleteMediaClient(mtype);
    }
    
    //create video local/remote/preview tracks, audio local/remote tracks
    private int CreateMediaClient()
    {
    	int res = 0;
    	Log.i("wme_android", "WmeClient::CreateMediaClient");
    	if (mEnableAudio) {
	    	res = CreateMediaClient(WmeParameters.WME_MEDIA_AUDIO);
    	}
    	
    	if (mEnableVideo) {
	    	res = CreateMediaClient(WmeParameters.WME_MEDIA_VIDEO);
    	}

    	if( mEnableShare ) {
            res = CreateMediaClient(WmeParameters.WME_MEDIA_SHARE);
        }

    	Log.i("wme_android", "WmeClient::CreateMediaClient OK");
    	return res;
    }
    
    private void DeleteMediaClient()
    {
    	Log.i("wme_android", "WmeClient::DeleteMediaClient");
    	if (mEnableAudio)
    		DeleteMediaClient(WmeParameters.WME_MEDIA_AUDIO);
    	
    	if (mEnableVideo)
    		DeleteMediaClient(WmeParameters.WME_MEDIA_VIDEO);

        if( mEnableShare )
            DeleteMediaClient(WmeParameters.WME_MEDIA_SHARE);

        Log.i("wme_android", "WmeClient::DeleteMediaClient OK");
    }
	
    public void Prepare()
    {
    	Log.i("wme_android", "WmeClient::Prepare");
        
        /// set QoS parameters
        WmeNative.SetQoSMaxLossRatio(mQoSMaxLossRatio);
        WmeNative.SetQoSMinBandwidth(mQoSMinBandwidth);
        WmeNative.SetInitialBandwidth(mInitialBandwidth);
        
    	CreateMediaClient();
    	
   		//SetLocalVideoCapability(mVideoCapIdx);	//video goes another way to set capability
    	if (mEnableAudio)
    		SetLocalAudioCapability(mAudioCapIdx);
        
   		if(mEnableQoS){
            WmeNative.EnableQoS(true);
        }
        
   		Log.i("wme_android", "WmeClient::Prepare OK");
    }
    
	///============================================
	/// for audio/video track in MediaSDK module
    public int StartAudioTrack()
    {	
    	if (!mEnableAudio)
    		return -1;
    	
    	Log.i("wme_android", "WmeClient::StartAudioTrack");
    	//mStartAudioTrack = true;
    	
    	int res = 0;
    	res = WmeNative.StartMediaTrack(WmeParameters.WME_MEDIA_AUDIO, WmeParameters.WME_REMOTE_TRACK);
    	if (res != 0)
    	{
    		Log.e("wme_android", "WmeClient::StartAudioTrack remote fail");
    	}
        res = WmeNative.StartMediaTrack(WmeParameters.WME_MEDIA_AUDIO, WmeParameters.WME_LOCAL_TRACK);  
        if (res != 0)
    	{
    		Log.e("wme_android", "WmeClient::StartAudioTrack local fail");
    	}
        Log.i("wme_android", "WmeClient::StartAudioTrack OK");
        return res;
    }
    
    public int StopAudioTrack()
    {
    	if (!mEnableAudio)
    		return -1;
    	
    	Log.i("wme_android", "WmeClient::StopAudioTrack");
    	//mStartAudioTrack = false;
    	
    	int res = 0;
        res = WmeNative.StopMediaTrack(WmeParameters.WME_MEDIA_AUDIO, WmeParameters.WME_LOCAL_TRACK);  
        if (res != 0)
    	{
    		Log.e("wme_android", "WmeClient::StopAudioTrack local fail");
    	}
        res = WmeNative.StopMediaTrack(WmeParameters.WME_MEDIA_AUDIO, WmeParameters.WME_REMOTE_TRACK);
        if (res != 0)
    	{
    		Log.e("wme_android", "WmeClient::StopAudioTrack remote fail");
    	}
        Log.i("wme_android", "WmeClient::StopAudioTrack OK");
        return res;
    }
    
    public int StartVideoTrack()
    {	
    	if (!mEnableVideo)
    		return -1;
    	
    	Log.i("wme_android", "WmeClient::StartVideoTrack");
    	//mStartVideoTrack = true;
    	
    	int res = 0;
        res = WmeNative.StartMediaTrack(WmeParameters.WME_MEDIA_VIDEO, WmeParameters.WME_LOCAL_TRACK); 
        if (res != 0)
    	{
    		Log.e("wme_android", "WmeClient::StartVideoTrack local fail");
    		//return res;
    	}
        res = WmeNative.StartMediaTrack(WmeParameters.WME_MEDIA_VIDEO, WmeParameters.WME_REMOTE_TRACK); 
        if (res != 0)
    	{
    		Log.e("wme_android", "WmeClient::StartVideoTrack remote fail");
    		//return res;
    	}
        if (res == 0)
        {
        	Log.i("wme_android", "WmeClient::StartVideoTrack OK");
        }
    	mStartVideoTrack = true;

        return res;
    }
    
    public int StopVideoTrack()
    {
    	if (!mEnableVideo)
    		return -1;
    	
    	Log.i("wme_android", "WmeClient::StopVideoTrack");
    	mStartVideoTrack = false;
    	
    	int res = 0;
        res = WmeNative.StopMediaTrack(WmeParameters.WME_MEDIA_VIDEO, WmeParameters.WME_LOCAL_TRACK);  
        if (res != 0)
    	{
    		Log.e("wme_android", "WmeClient::StopVideoTrack local fail");
    	}
        res = WmeNative.StopMediaTrack(WmeParameters.WME_MEDIA_VIDEO, WmeParameters.WME_REMOTE_TRACK); 
        if (res != 0)
    	{
    		Log.e("wme_android", "WmeClient::StopVideoTrack remote fail");
    	}
        if (res == 0)
        {
        	Log.i("wme_android", "WmeClient::StopVideoTrack OK");
        }
        return res;
    }

    public  int StartRemoteShareTrack()
    {
        if( !mEnableShare )
            return -1;

        Log.i("wme_android", "WmeClient::StartRemoteShareTrack");
        int res = 0;
        res = WmeNative.StartMediaTrack(WmeParameters.WME_MEDIA_SHARE, WmeParameters.WME_REMOTE_SHARE_TRACK);
        if (res != 0)
        {
            Log.e("wme_android", "WmeClient::StartRemoteShareTrack remote fail");
            //return res;
        }
        if (res == 0)
        {
            Log.i("wme_android", "WmeClient::StartRemoteShareTrack OK");
        }
        mStartShareTrack = true;

        return res;

    }

    public int StopRemoteShareTrack()
    {
        if( mStartShareTrack )
        {
            mStartShareTrack = false;

            int res = WmeNative.StopMediaTrack(WmeParameters.WME_MEDIA_SHARE, WmeParameters.WME_REMOTE_SHARE_TRACK);
            if (res != 0)
            {
                Log.e("wme_android", "WmeClient::StopRemoteShareTrack remote fail");
                //return res;
            }
            if (res == 0)
            {
                Log.i("wme_android", "WmeClient::StopRemoteShareTrack OK");
            }
            return res;
        }
        if( !mEnableShare )
            return -1;

        return 0;
    }

    public int StartPreviewTrack()
    {
    	if (!mEnableVideo)
    		return -1;
    	
    	Log.i("wme_android", "WmeClient::StartPreviewTrack");    	
    	//mStartPreviewTrack = true;    
    	    	
    	int res = 0;
        res = WmeNative.StartMediaTrack(WmeParameters.WME_MEDIA_VIDEO, WmeParameters.WME_PREVIEW_TRACK);  
        if (res != 0)
    	{
    		Log.e("wme_android", "WmeClient::StartPreviewTrack fail");
    	}
        else
        {
    		Log.i("wme_android", "WmeClient::StartPreviewTrack OK");
        }
        return res;
    }
    
    public int StopPreviewTrack()
    {
    	if (!mEnableVideo)
    		return -1;
    	
    	Log.i("wme_android", "WmeClient::StopPreviewTrack");
    	//mStartPreviewTrack = false;
    	
    	int res = 0;
        res = WmeNative.StopMediaTrack(WmeParameters.WME_MEDIA_VIDEO, WmeParameters.WME_PREVIEW_TRACK);
        if (res != 0)
    	{
    		Log.e("wme_android", "WmeClient::StopPreviewTrack fail");
    	}
        else
        {        	
        	Log.i("wme_android", "WmeClient::StopPreviewTrack OK");
        }
               
        return res;
    }
    
    public void SetRenderAspectRatioSameWithSource(int type, boolean same)
    {
    	if (!mEnableVideo)
    		return;
    	
    	WmeNative.SetRenderAspectRatioSameWithSource(type, same);
    }
    
    public void SetRenderMode(int type, int mode)
    {
    	if (!mEnableVideo)
    		return;
    	
    	Log.i("wme_android", "WmeClient::SetRenderMode, type="+type+",mode="+mode);
    	WmeNative.SetRenderMode(type, mode);
    }
    
    ///==================================================
    /// for video render window
    public void SetLocalWindow(GLSurfaceView surface)
    {
    	if (!mEnableVideo)
    		return;
    	
    	Log.i("wme_android", "WmeClient::SetLocalWindow");
    	//mLocalRender = surface;;
    	WmeNative.SetRenderView(WmeParameters.WME_LOCAL_TRACK, surface);
    }
    public void SetRemoteWindow(GLSurfaceView surface)
    {    
    	if (!mEnableVideo)
    		return;
    	
	    Log.i("wme_android", "WmeClient::SetRemoteWindow, surface="+surface);
	    //mRemoteRender = surface;;
	    WmeNative.SetRenderView(WmeParameters.WME_REMOTE_TRACK, surface);
    }

    public void SetRemoteShareWindow(GLSurfaceView surface)
    {
        if (!mEnableShare)
            return;

        Log.i("wme_android", "WmeClient::SetRemoteShareWindow, surface="+surface);
        //mRemoteRender = surface;;
        WmeNative.SetRenderView(WmeParameters.WME_REMOTE_SHARE_TRACK, surface);
    }

    public void SetPreviewWindow(GLSurfaceView surface)
    {    
    	if (!mEnableVideo)
    		return;
    	
	    Log.i("wme_android", "WmeClient::SetPreviewWindow, surface="+surface);
	    //mPreviewRender = surface;    	
	    WmeNative.SetRenderView(WmeParameters.WME_PREVIEW_TRACK, surface);
    }
    
    public void CreatePreviewClient()
    {    
    	if (!mEnableVideo)
    		return;
    	
    	Log.i("wme_android", "WmeClient::CreatePreviewClient"); 	
    	WmeNative.CreateMediaClient(WmeParameters.WME_MEDIA_VIDEO);    	  	
	    //WmeNative.SetRenderView(WmeParameters.WME_PREVIEW_TRACK, surface);
    }
    
    public void DeletePreviewClient()
    {    
    	if (!mEnableVideo)
    		return;

	    Log.i("wme_android", "WmeClient::DeletePreviewClient"); 	
	    WmeNative.DeleteMediaClient(WmeParameters.WME_MEDIA_VIDEO);    	   	
	    WmeNative.SetRenderView(WmeParameters.WME_PREVIEW_TRACK, null);
    }

    ///============================================
    /// for audio/video sending in TP module
    public void StartAudioSending()
    { 
    	if (!mEnableAudio)
    		return;
    	
	    Log.i("wme_android", "WmeClient::StartAudioSending");
	    //mStartAudioSending = true;
	    WmeNative.StartMediaSending(WmeParameters.WME_MEDIA_AUDIO);
    }
    public void StopAudioSending()
    { 
    	if (!mEnableAudio)
    		return;
    	
	    Log.i("wme_android", "WmeClient::StopAudioSending");
	    //mStartAudioSending = false;
	    WmeNative.StopMediaSending(WmeParameters.WME_MEDIA_AUDIO);  
    }
    public void StartVideoSending()
    {
    	if (!mEnableVideo)
    		return;
    	
	    Log.i("wme_android", "WmeClient::StartVideoSending");
	    //mStartVideoSending = true;
	    WmeNative.StartMediaSending(WmeParameters.WME_MEDIA_VIDEO); 
    }
    public void StopVideoSending()
    {
    	if (!mEnableVideo)
    		return;
    	
	    Log.i("wme_android", "WmeClient::StopVideoSending");
	    //mStartVideoSending = false;
	    WmeNative.StopMediaSending(WmeParameters.WME_MEDIA_VIDEO);  
    }
    

    ///==========================================
    /// video device switch
    public String[] GetVideoDeviceList()
    {
    	if (!mEnableVideo)
    		return new String[0];
    	
    	Log.i("wme_android", "WmeClient::GetVideoDeviceList");
    	return WmeNative.GetMediaDevices(WmeParameters.WME_MEDIA_VIDEO, WmeParameters.WME_DEV_CAMERA);
    }
    
    // @param index in getVideoDeviceList's devlist
    public void SwitchCamera(int track_type, int index)
    {
    	if (!mEnableVideo)
    		return;
    	
    	Log.i("wme_android", "WmeClient::SwitchCamera");
    	WmeNative.SetMediaDevice(track_type, WmeParameters.WME_DEV_CAMERA, index);
    }
    
    ///============================================
    /// for video options setting
    public String[] GetVideoCapabilityList()
    {
    	if (!mEnableVideo)
    		return new String[0];
    	
    	Log.i("wme_android", "WmeClient::GetVideoCapabilityList");
    	return WmeNative.GetMediaCapabilities(WmeParameters.WME_MEDIA_VIDEO);
    }
    
    public String[] GetVideoCaptureParamList(int dev_index)
    {
    	if (!mEnableVideo)
    		return new String[0];
    	
    	Log.i("wme_android", "WmeClient::GetVideoCaptureParamList, dev_index:"+dev_index);
    	return WmeNative.GetCaptureParamList(WmeParameters.WME_DEV_CAMERA, dev_index);
    }
    
    public void SetVideoCaptureParam(int track_type, int dev_index, int param_index)
    {
    	if (!mEnableVideo)
    		return;
    	
    	Log.i("wme_android", "WmeClient::SetVideoCaptureParam, dev_index:"+dev_index+",param_index="+param_index);
    	WmeNative.SetCaptureParam(track_type, WmeParameters.WME_DEV_CAMERA, dev_index, param_index);
    }

    // @param index in GetVideoCapabilityList()'s caplist
    public void SetLocalVideoCapability(int index)
    {
    	if (!mEnableVideo)
    		return;
    	
    	Log.i("wme_android", "WmeClient::SetLocalVideoCapability, index="+index);
    	//mVideoCapIdx = index;
    	WmeNative.SetMediaCapability(WmeParameters.WME_MEDIA_VIDEO, WmeParameters.WME_LOCAL_TRACK, index);
    }

    ///============================================
    /// for audio options setting
    public String[] GetAudioCapabilityList()
    {
    	if (!mEnableAudio)
    		return new String[0];
    	
    	Log.i("wme_android", "WmeClient::GetAudioCapabilityList"); 
    	return WmeNative.GetMediaCapabilities(WmeParameters.WME_MEDIA_AUDIO);
    }

    /// @param index in GetAudioCapabilityList's caplist
    public void SetLocalAudioCapability(int index)
    {
    	if (!mEnableAudio)
    		return;
    	
    	Log.i("wme_android", "WmeClient::SetLocalAudioCapability");
    	mAudioCapIdx = index;
    	WmeNative.SetMediaCapability(WmeParameters.WME_MEDIA_AUDIO, WmeParameters.WME_LOCAL_TRACK, index);
    }

    ///============================================
    /// for audio volume setting and mute/unmute
    public void SetMicVolume(int vol)
    {
    	if (!mEnableAudio)
    		return;
    	
    	Log.i("wme_android", "WmeClient::SetMicVolume");
    	WmeNative.SetAudioVolume(WmeParameters.WME_DEV_MIC, vol);
    }
    public void SetSpeakerVolume(int vol)
    {
    	if (!mEnableAudio)
    		return;
    	
    	Log.i("wme_android", "WmeClient::SetSpeakerVolume");
    	WmeNative.SetAudioVolume(WmeParameters.WME_DEV_SPEAKER, vol);
    }
    
    public int GetMicVolume()
    {
    	if (!mEnableAudio)
    		return -1;
    	
    	Log.i("wme_android", "WmeClient::GetMicVolume");
    	return WmeNative.GetAudioVolume(WmeParameters.WME_DEV_MIC);
    }
    public int GetSpeakerVolume()
    {
    	if (!mEnableAudio)
    		return -1;
    	
    	Log.i("wme_android", "WmeClient::GetSpeakerVolume");    	
    	return WmeNative.GetAudioVolume(WmeParameters.WME_DEV_SPEAKER);
    }
    
    public void SetMicState(boolean mute)
    {
    	if (!mEnableAudio)
    		return;
    	
    	Log.i("wme_android", "WmeClient::SetMicState");
    	if (mute)
    		WmeNative.MuteAudio(WmeParameters.WME_DEV_MIC);
    	else
    		WmeNative.UnMuteAudio(WmeParameters.WME_DEV_MIC);
    }
    
    public void SetSpeakerState(boolean mute)
    {
    	if (!mEnableAudio)
    		return;
    	
    	Log.i("wme_android", "WmeClient::SetSpeakerState");
    	if (mute)
    		WmeNative.MuteAudio(WmeParameters.WME_DEV_SPEAKER);
    	else
    		WmeNative.UnMuteAudio(WmeParameters.WME_DEV_SPEAKER);
    }
    
    public void SetAudioOutType(int atype)
    {
    	if (!mEnableAudio)
    		return;
    	
    	WmeNative.SetAudioOutType(atype);
    }
    
    public void SetVideoQuality(int quality)
    {
    	if (!mEnableVideo)
    		return;
    	
    	Log.i("wme_android", "WmeClient::SetVideoQuality, quality="+quality);
    	WmeNative.SetVideoQuality(WmeParameters.WME_LOCAL_TRACK, quality);
    }
    
    public void EnableExternalAudioInput(boolean enabled) 
    {
    	mEnableExternalAudioInput = enabled;
    }
    
    public void EnableExternalVideoInput(boolean enabled) 
    {
    	mEnableExternalVideoInput = enabled;
    }
    
    public void EnableVideoOutputFile(boolean enable)
    {
    	mEnableVideoOutputFile = enable;
    }
    
    public void EnableAudioOutputFile(boolean enable)
    {
    	mEnableAudioOutputFile = enable;
    }
    
//    public void ApplyExternalTrack() 
//    {
//    	DeleteMediaClient(WmeParameters.WME_MEDIA_VIDEO);
//    	CreateMediaClient(WmeParameters.WME_MEDIA_VIDEO);
//    }
    
    public void SetVideoInputFile(String fname) 
    {
    	mVInputFilename = fname;
    }
    public void SetVideoWidth(int width)
    {
    	mWidth = width;
    }
    public void SetVideoHeight(int height)
    {
    	mHeight = height;
    }
    public void SetVideoColor(int color)
    {
    	mColor = color;
    }
    public void SetFPS(int fps)
    {
    	mFPS = fps;
    }
    
    public void SetVideoOutputFile(String fname)
    {
    	mVOutputFilename = fname;
    }
    
    public void SetAudioInputFile(String fname)
    {
    	mAInputFilename = fname;
    }
    
    public void SetAudioOutputFile(String fname)
    {
    	mAOutputFilename = fname;
    }
    
    public void SetChannels(int n)
    {
    	mChannels = n;
    }
    
    public void SetSampleRate(int n)
    {
    	mSampleRate = n;
    }
    
    public void SetBitsPerSample(int n)
    {
    	mBitsPerSample = n;
    }
    public void SetQoSMaxLossRatio(float maxLossRatio)
    {
        mQoSMaxLossRatio = maxLossRatio;
    }
    
    public void SetQoSMinBandwidth(int minBandwidth)
    {
        mQoSMinBandwidth = minBandwidth;
    }
    
    public void SetInitialBandwidth(int initBandwidth)
    {
        mInitialBandwidth = initBandwidth;
    }
    
    public void EnableQoS(boolean enable)
    {
        mEnableQoS = enable;
        WmeNative.EnableQoS(enable);
    }
    
    public void GetVideoStatistic()
    {
    	WmeNative.GetTrackStatistics(WmeParameters.WME_MEDIA_VIDEO, WmeParameters.WME_LOCAL_TRACK);
    	WmeNative.GetTrackStatistics(WmeParameters.WME_MEDIA_VIDEO, WmeParameters.WME_REMOTE_TRACK);
    }
	
	 public void GetSessionStatistic()
    {
    	WmeNative.GetSessionStatistics(WmeParameters.WME_LOCAL_TRACK);
    	WmeNative.GetSessionStatistics(WmeParameters.WME_REMOTE_TRACK);
    }
    
    public void DisableSendingFilterFeedback()
    {
    	WmeNative.DisableSendingFilterFeedback();
    }

    public void HeartBeart() {
        //Log.i("wme_android", "WmeClient::HeartBeart");
        WmeNative.HeartBeat();
    }
    
	public float CheckAudioOutputFile(String strAudioPath)
    {
    	float fRet = WmeNative.CheckAudioOutputFile(strAudioPath);
    	return fRet;
    }
    
    public boolean AddVideoFileRenderSink()
    {
    	Log.i("wme_android", "WmeClient::AddVideoFileRenderSink");
    	WmeNative.AddVideoFileRenderSink();
    	return false;
    }

    ///===============================================
    /// for HeartBeat
    
    private static Queue<Message> mQueue = null;
    private static Thread mHBThread = null;
    private static boolean mThreadExit = true;
    
    //[WARN] Never call it. TP threadmanager is global and so user thread should be global too !!!
    // When press "back", the APP process is actually not destroyed, making TP threadmanager still living.
    private void UninitHB() {
    	//mHBThread.interrupt();
    	mThreadExit = true;
    	try {
			mHBThread.join(1*1000);
		} catch (InterruptedException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
    	mHBThread = null;
    	mQueue.clear();
    	mQueue = null;
    }
    
    //[WARN] User thread must be global, so create only once
    private void InitHB() {
    	if (mHBThread == null)
    	{
    		mQueue = new LinkedList<Message>();
        	mThreadExit = false;
    		mHBThread = new Thread() {
        		@Override
        		public void run() {
					int loop = 0;
        			do {    				
        				Message msg = PopMessage();
        				if (msg != null) {
        					switch(msg.what) {
	        					case WmeParameters.TPConnectToMsg: {
	        						String ipstr = msg.getData().getString("ip");
	        						StartAsClient(ipstr);
	        						NormalFragment.setNetworkConnectted(true);
	        						break;
	        					}
	        					case WmeParameters.TPInitHostMsg: {
	        						StartAsHost();
	        						NormalFragment.setNetworkConnectted(true);
	        						break;
	        					}
	        					case WmeParameters.TPDisconnect: {
	        						Disconnect();
	        						NormalFragment.setNetworkConnectted(false);
	        						break;
	        					}
	        					
	        					case WmeParameters.TPInitMainThread: {
	        						WmeNative.InitTPThread();
	        						m_bTPMainThreadInitOnce = true;
	        						Log.v ("wme_android", "Init TP main thread!");
	        						break;
	        					}
	        					
	        					case WmeParameters.TPConnectFileMsg: {
	        						int mediatype = msg.getData().getInt("media.type");
	        						String filename = msg.getData().getString("file");
	        						String sourceip = msg.getData().getString("source.ip");
	        						int sourceport = msg.getData().getInt("source.port");
	        						String destinationip = msg.getData().getString("destination.ip");
	        						int destinationport = msg.getData().getInt("destination.port");
	        						StartAsFile(mediatype, filename, sourceip, sourceport, destinationip, destinationport);
	        						NormalFragment.setNetworkConnectted(true);
	        						break;
	        					}
        					} 
        				}
						
						loop++;
//						if (mStartVideoTrack && ((loop % 1000) == 0)) { // output video info: 1000 x 100ms = 100s = 1.6min
//							GetVideoStatistic();
//						}
        				
        				try {
    						Thread.sleep(100);
    					} catch (InterruptedException e) {
    						// TODO Auto-generated catch block
    						e.printStackTrace();
    					}
        				HeartBeart();
        			}while(mThreadExit == false);			
        		}
        	};
        	
        	mHBThread.start();
    	}
    	
    }
    
    public synchronized void PushMessage(Message msg) {
    	if (mQueue != null) {
    		mQueue.offer(msg);
    	}
    }
    
    private synchronized Message PopMessage() {
    	Message msg = null;
    	if (mQueue != null && mQueue.size() > 0) {
    		msg = mQueue.poll();
    	}
    	return msg;
    }
}

