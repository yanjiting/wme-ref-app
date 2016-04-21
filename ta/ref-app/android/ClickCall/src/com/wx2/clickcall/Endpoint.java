package com.wx2.clickcall;

import org.json.JSONException;
import org.json.JSONObject;

import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.content.res.Configuration;
import android.view.ViewGroup;

import com.cisco.wme.appshare.ScreenShareContext;
import com.webex.wme.*;
import com.webex.wme.DeviceManager.MediaDevice;
import com.webex.wme.MediaConnection.ConnectionStatus;
import com.webex.wme.MediaConnection.MediaStatus;
import com.webex.wme.MediaConnection.MediaConnectionListener;
import com.webex.wme.MediaConnection.MediaDirection;
import com.webex.wme.MediaConnection.WmeVideoSizeInfo;
import com.webex.wme.MediaConnection.MediaType;
import com.webex.wme.MediaConnection.SDPType;
import com.webex.wme.MediaStatistics.AudioStatistics;
import com.webex.wme.MediaStatistics.CpuUsage;
import com.webex.wme.MediaStatistics.AggregateNetworkMetricStats;
import com.webex.wme.MediaStatistics.MemoryUsage;
import com.webex.wme.MediaStatistics.VideoStatistics;
import com.webex.wme.MediaStatistics.SharingStatistics;
import com.webex.wme.MediaStatistics.VideoStats;
import com.webex.wme.MediaTrack.ScalingMode;
import com.webex.wme.MediaTrack.ExternalAudioRenderType;
import com.webex.wme.MediaConnection.NetworkStatus;
import com.webex.wme.MediaConnection.NetworkDirection;
import com.webex.wme.MediaConfig.WmePerformanceDumpType;
import com.webex.wseclient.WseSurfaceView;

interface SDPReadyEvent {
    public void onSDP(SDPType type, String sdp);
}

public class Endpoint implements MediaConnectionListener{
	private MediaConnection mConn = null;
	private MediaTrack mTrackVideoLocal = null, mTrackAudioLocal = null;
	private MediaTrack  mTrackAudioRemote = null;
	private MediaTrack mTrackVideoRemotes[];
	private WseSurfaceView mRemoteRenders[];
    private MediaTrack mTrackShareRemote = null;
	private MediaTrack mTrackShareLocal = null;
	private final int AUDIO_MID = 1;
	private final int VIDEO_MID = 2;
    private final int SHARE_MID = 3;

	private SDPReadyEvent mSdpEvent = null;
    private JSONObject m_audioDebugOption = new JSONObject();
    private JSONObject m_videoDebugOption = new JSONObject();
    private JSONObject m_sharingDebugOption = new JSONObject();
    private WseSurfaceView  mLocalRender = null;
    private WseSurfaceView mASRender = null;
    private boolean m_audioMuted = false;
    private boolean m_videoMuted = false;

    private static int mCurrRotation;
    private static int decodedHeight;
    private static int decodedWidth;
    
    // Calabash variables
    private SDPType mCurrSDPType = null;
    private boolean mConnected = false;
    private boolean mBlocked = true;
    private static ScalingMode mCurrScalingMode;
    private Calabash mCalabash = new Calabash();

    public boolean m_audioFileEnded =false;
    public boolean m_videoFileEnded =false;
    private MyApplication mApp;
    private final int mMaxRemoteVideo = 4;
    private int mRemoteVideoIndex = 0;

	//for screen sharing
	private ScreenShareContext.OnShareStoppedListener mListener;

	//Add by Boris
    private MediaAudioPairingExternalRender audioPairingExternalRender = null;
    private MediaAudioExternalRender audioDataCaputreRaw = null;
    private MediaAudioExternalRender audioDataCaputreBeforeEncode = null;
    private MediaAudioExternalRender audioDataPlayback = null;

    private WmePerformanceDumpType mPerformanceDumpType = WmePerformanceDumpType.WmePerformanceDumpAll;
	private int mRenderWidth = 0;
	private int mRenderHeight = 0;
	public Endpoint(WseSurfaceView local, WseSurfaceView remote, WseSurfaceView as) {
		mConn = new MediaConnection();
		mConn.setListener(this);
		mLocalRender = local;
		mRemoteRenders = new WseSurfaceView[mMaxRemoteVideo];
		mTrackVideoRemotes = new MediaTrack[mMaxRemoteVideo];
		for(int i = 0; i < mMaxRemoteVideo; i++) {
			mRemoteRenders[i] = null;
			mTrackVideoRemotes[i] = null;
		}
		mRemoteRenders[0] = remote;
        mASRender = as;
        mApp = (MyApplication)MyApplication.getAppContext();
        mConn.GetGlobalConfig().SetDeviceMediaSettings(mApp.mHWVideoSetting);
        mConn.startMediaLogging(1000);

        //Add by Boris
        audioPairingExternalRender = new MediaAudioPairingExternalRender();
        audioDataCaputreRaw = new MediaAudioExternalRender(ExternalAudioRenderType.CaptureFromHardware);
        audioDataCaputreBeforeEncode = new MediaAudioExternalRender(ExternalAudioRenderType.CaptureBeforeEncode);
        audioDataPlayback = new MediaAudioExternalRender(ExternalAudioRenderType.PlaybackToHardware);
	}
	
	public void pushRemoteView(int idx, WseSurfaceView remote) {
		assert(idx > 0 && idx < mMaxRemoteVideo);
		mRemoteRenders[idx] = remote;
	}
	
	public void preview(){
		if(mLocalRender != null && mApp.m_bHasAudio)
			mConn.addMedia(MediaType.Audio, MediaDirection.SendOnly, AUDIO_MID, m_audioDebugOption.toString());
		if(mLocalRender != null && mApp.m_bHasVideo){				
			mConn.addMedia(MediaType.Video, MediaDirection.SendOnly, VIDEO_MID, m_videoDebugOption.toString());
		}
		if(mApp.m_bHasShare && !mApp.mIsViewer) {
			mConn.addMedia(MediaType.Sharing, MediaDirection.SendOnly, SHARE_MID, m_sharingDebugOption.toString());
		}
	}
	void installScreenSharingListener(){
		if(mListener==null){
			mListener = new ScreenShareContext.OnShareStoppedListener() {
				@Override
				public void onShareStopped() {
					if(mTrackShareLocal!=null) {
						mTrackShareLocal.Stop();
					}
				}
			};
		}
		ScreenShareContext.getInstance().registerCallback(mListener);
	}

	void uninstallScreenSharingListener(){
		if(mListener!=null)
			ScreenShareContext.getInstance().unregisterCallback(mListener);
	}

	public void startCall(SDPReadyEvent sdpEvent){
		installScreenSharingListener();
		mSdpEvent = sdpEvent;
		if(mApp.m_bHasAudio)
			mConn.addMedia(MediaType.Audio, MediaDirection.RecvOnly, AUDIO_MID, m_audioDebugOption.toString());
		if(mRemoteRenders[0] != null && mApp.m_bHasVideo) {
			mConn.addMedia(MediaType.Video, MediaDirection.RecvOnly, VIDEO_MID, m_videoDebugOption.toString());
		}
        if( mASRender != null  && mApp.m_bHasShare)
            mConn.addMedia(MediaType.Sharing, MediaDirection.RecvOnly, SHARE_MID, m_sharingDebugOption.toString());

        resetParam();
        mConn.createOffer();
	}
	
	void acceptCall(String sdp, boolean bUpdate, SDPReadyEvent sdpEvent) {
		installScreenSharingListener();
		mSdpEvent = sdpEvent;
		if(!bUpdate)
		{
			if(mApp.m_bHasAudio)
				mConn.addMedia(MediaType.Audio, MediaDirection.RecvOnly, AUDIO_MID, m_audioDebugOption.toString());
			if(mApp.m_bHasVideo)
				mConn.addMedia(MediaType.Video, MediaDirection.RecvOnly, VIDEO_MID, m_videoDebugOption.toString());
			if( mASRender != null  && mApp.m_bHasShare)
	            mConn.addMedia(MediaType.Sharing, MediaDirection.RecvOnly, SHARE_MID, m_sharingDebugOption.toString());
		}
        resetParam();
		WmeSdpParsedInfo[] parsedInfoList = mConn.setReceivedSDP(SDPType.Offer, sdp);
		Log.i("MediaSession", "acceptCall, len=" + parsedInfoList.length);
		checkDirection(parsedInfoList);
		
		if(!bUpdate)
			mConn.createAnswer();
	}
	
	public void answerReceived(String sdp){
		if(!sdp.isEmpty()){
			WmeSdpParsedInfo[] parsedInfoList = mConn.setReceivedSDP(SDPType.Answer, sdp);
			Log.i("MediaSession", "answerReceived, len=" + parsedInfoList.length);
			checkDirection(parsedInfoList);
		}
	}

	private MediaDirection mAudioRemoteDirection = MediaDirection.SendRecv;
	private MediaDirection mVideoRemoteDirection = MediaDirection.SendRecv;
	
	private void checkDirection(WmeSdpParsedInfo[] arraySdp)
	{
		for(WmeSdpParsedInfo sdpInfo : arraySdp) {
			int dir_sdp = sdpInfo.remoteNegotiatedDirection.value();
			int recv_value = MediaDirection.RecvOnly.value();
			if(sdpInfo.mediaType == MediaType.Audio) {
				if(mTrackAudioLocal != null)
				{
					if(((dir_sdp & recv_value) != 0) && ((mAudioRemoteDirection.value() & recv_value) == 0)){
						mTrackAudioLocal.Unmute();
					}
					else if(((dir_sdp & recv_value) == 0) && ((mAudioRemoteDirection.value() & recv_value) != 0)){
						mTrackAudioLocal.Mute();
					}
				}
				mAudioRemoteDirection = sdpInfo.remoteNegotiatedDirection;
			}
			else if(sdpInfo.mediaType == MediaType.Video){
				if(mTrackVideoLocal != null)
				{
					if(((dir_sdp & recv_value) != 0) && ((mVideoRemoteDirection.value() & recv_value) == 0)){
						mTrackVideoLocal.Unmute();
					}
					else if(((dir_sdp & recv_value) == 0) && ((mVideoRemoteDirection.value() & recv_value) != 0)){
						mTrackVideoLocal.Mute();
					}
				}
				mVideoRemoteDirection = sdpInfo.remoteNegotiatedDirection;
			}
		}
	}

    private void resetParam(){
        JSONObject params = mApp.getStartupParams();
        if(mApp.m_bHasAudio)
            setParam(MediaType.Audio, params);
        if(mApp.m_bHasVideo)
        setParam(MediaType.Video, params);
        if(mApp.m_bHasShare)
            setParam(MediaType.Sharing, params);
        mConn.GetGlobalConfig().EnablePerformanceTraceDump(mPerformanceDumpType);

		mConn.GetVideoConfig(VIDEO_MID).EnableCVO(mApp.m_bEnableCVO);
		mConn.GetVideoConfig(VIDEO_MID).EnableAVCSimulcast(mApp.m_bEnableSimulcast);
		mConn.GetVideoConfig(VIDEO_MID).Disable90PVideo(mApp.m_bDisable90PVideo);
		mConn.GetVideoConfig(VIDEO_MID).EnableHWAcceleration(mApp.m_bHWCodec, MediaConfig.WmeHWAccelerationConfig.WmeHWAcceleration_Encoder);
		mConn.GetVideoConfig(VIDEO_MID).EnableHWAcceleration(mApp.m_bHWCodec, MediaConfig.WmeHWAccelerationConfig.WmeHWAcceleration_Decoder);
		mConn.GetShareConfig(SHARE_MID).EnableHWAcceleration(mApp.m_bScreenHWCodec, MediaConfig.WmeHWAccelerationConfig.WmeHWAcceleration_Encoder);
		mConn.GetShareConfig(SHARE_MID).EnableHWAcceleration(mApp.m_bScreenHWCodec, MediaConfig.WmeHWAccelerationConfig.WmeHWAcceleration_Decoder);
    }

    public void setPerformanceDumpType(WmePerformanceDumpType type) {
        mPerformanceDumpType = type;
    }

	public void enableQos(boolean bEnable){
		enableQos(MediaType.Audio, bEnable);
		enableQos(MediaType.Video, bEnable);
	}
	
	public void enableQos(MediaType type, boolean bEnable)
	{
		try {
			JSONObject root = new JSONObject();
			JSONObject obj = new JSONObject();
			obj.put("enableQos", bEnable);
			int mid = 0;
			if(type == MediaType.Audio){
				mid = AUDIO_MID;
				root.put("audio", obj);
			}
			else if(type == MediaType.Video){
				mid = VIDEO_MID;
				root.put("video", obj);
			}
			if(mConn != null)
				mConn.setParameters(mid, root.toString());
		} catch (JSONException e) {
			e.printStackTrace();
		}
	}

    public void setFeatureToggles(String policy)
    {
        mConn.GetGlobalConfig().SetFeatureToggles(policy);
    }

    public String getFeatureToggles()
    {
        String ret = null;
        String strPolicy = null;
        try {
            strPolicy = mConn.GetGlobalConfig().GetFeatureToggles();
            JSONObject policy = new JSONObject(strPolicy);
            JSONObject root = new JSONObject();
            root.put("featureToggles", policy);
            ret = root.toString();
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return ret;
    }

    public void setWmeDataDump(MediaType type, int iDumpFlag)
    {
        try {
            JSONObject root = new JSONObject();
            JSONObject obj = new JSONObject();
            obj.put("enableDataDump", iDumpFlag);
            root.put("video",obj);
            if(mConn != null)
                mConn.setParameters(VIDEO_MID, root.toString());
        } catch (JSONException e) {
            e.printStackTrace();
        }
        
    }

	public void setParameters (MediaType type, JSONObject jsonKeyValue) {
		try {
			JSONObject jsonRoot = new JSONObject();
			int mid = 0;
			if (type == MediaType.Audio) {
				mid = AUDIO_MID;
				jsonRoot.put("audio", jsonKeyValue);
			}
			else if (type == MediaType.Video){
				mid = VIDEO_MID;
				jsonRoot.put("video", jsonKeyValue);
			}
            else if( type == MediaType.Sharing )
            {
                mid = SHARE_MID;
                jsonRoot.put("sharing", jsonKeyValue);
            }
			if (mConn != null)
				mConn.setParameters(mid, jsonRoot.toString());
		} catch (JSONException e) {
			e.printStackTrace();
		}
	}

	public void setParam (MediaType type, JSONObject jsonKeyValue) {
		int mid = 0;
        MediaConfig.BaseConfig basicConfig = null;
		if (type == MediaType.Audio) {
			mid = AUDIO_MID;
            basicConfig = mConn.GetAudioConfig(AUDIO_MID);
		} else if (type == MediaType.Video){
			mid = VIDEO_MID;
            basicConfig = mConn.GetVideoConfig(VIDEO_MID);
		}
        else if( type == MediaType.Sharing && mApp.m_bHasShare) {
            mid = SHARE_MID;
            basicConfig = mConn.GetShareConfig(SHARE_MID);
        }
		if (mConn != null && mid != 0) {
			if (type == MediaType.Video) {
				mConn.GetVideoConfig(VIDEO_MID).SetDataDumpFlag(mApp.m_nVideoDumpFlag);
			}

            mConn.setParameters(mid, jsonKeyValue.toString());
			if(mApp.m_uMaxVideoStreams > 1){
				mConn.GetVideoConfig(VIDEO_MID).SetInitSubscribeCount(mApp.m_uMaxVideoStreams);
				mConn.GetAudioConfig(AUDIO_MID).EnableClientMix(mApp.m_uMaxAudioStreams);
			}
			mConn.GetGlobalConfig().EnableMultiStream(mApp.mEnableCMulti);
		}
        if(basicConfig != null)
			basicConfig.SetIcePolicy(mApp.m_nIcePolicy);
	}

	public String getParameters (MediaType type, String key) {
		String value = "";		
		try {
			int mid = 0;
			if (type == MediaType.Audio){
				mid = AUDIO_MID;
			}
			else if (type == MediaType.Video) {
				mid = VIDEO_MID;
			}
            else if (type == MediaType.Sharing )
                mid = SHARE_MID;
			String res = mConn.getParameters(mid);
			JSONObject root = new JSONObject(res);			
			JSONObject param = new JSONObject();
			if (type == MediaType.Video) { 
				param = root.getJSONObject("video");
			} else if (type == MediaType.Audio) {
				param = root.getJSONObject("audio");
			}
            else if( type == MediaType.Sharing )
                param = root.getJSONObject("sharing");
			value = param.getString(key);			
			return value;			
		} catch (JSONException e) {
			e.printStackTrace();
		}		
		return value;
	}

	public void setMuteOption(boolean bMuted)
	{
		setMuteOption(MediaType.Audio, bMuted);
		setMuteOption(MediaType.Video, bMuted);
	}
	
	public void setMuteOption(MediaType type, boolean bMuted)
	{
		if(type == MediaType.Audio){
			m_audioMuted = bMuted;
		}
		else if(type == MediaType.Video){
			m_videoMuted = bMuted;
		}
	}

    public void onProxyCredentialRequired(String sProxy, int port)
    {
        Log.v("ClickCall", "onProxyCredentialRequired, sProxy=" + sProxy + ":" + port);
    }

    public void onAvailableMediaChanged(int mid, int count)
    {
        Log.v("ClickCall", "onAvailableMediaChanged, mid=" + mid + ", count=" + count);
    }
	
	public void onMediaReady(int mid, MediaDirection dir, MediaType type, MediaTrack track) {
	//	Log.v("MediaSession", "onMediaReady, track=" + track + ", mConn=" + mConn + ", mid=" + mid);
		boolean bMute = false;
		if(mid == VIDEO_MID){
			if(dir == MediaDirection.SendOnly){
				mTrackVideoLocal = track;
				if(mLocalRender!=null) {
					mTrackVideoLocal.addRenderWindow(mLocalRender);
					Log.v("MediaSession", "onMediaReady, Local,addRenderWindow render=" + mLocalRender + ", this=" + this);
				}
				bMute = m_videoMuted;
                mConn.GetVideoConfig(VIDEO_MID).EnableCVO(mApp.m_bEnableCVO);
                mConn.GetVideoConfig(VIDEO_MID).EnableAVCSimulcast(mApp.m_bEnableSimulcast);
				mConn.GetVideoConfig(VIDEO_MID).Disable90PVideo(mApp.m_bDisable90PVideo);

			}else if(dir == MediaDirection.RecvOnly){
				Log.v("MediaSession", "onMediaReady, remote index=" + mRemoteVideoIndex);
				if(mRemoteRenders[mRemoteVideoIndex]!=null) {
					track.addRenderWindow(mRemoteRenders[mRemoteVideoIndex]);
					Log.v("MediaSession", "onMediaReady, Remote,addRenderWindow render=" + mRemoteRenders[mRemoteVideoIndex] + ", this=" + this);
				}
				mTrackVideoRemotes[mRemoteVideoIndex] = track;
				mRemoteVideoIndex++;
			}

			if(mApp.m_bHWCodec) {
				mConn.GetVideoConfig(VIDEO_MID).EnableHWAcceleration(true, MediaConfig.WmeHWAccelerationConfig.WmeHWAcceleration_Encoder);
				mConn.GetVideoConfig(VIDEO_MID).EnableHWAcceleration(true, MediaConfig.WmeHWAccelerationConfig.WmeHWAcceleration_Decoder);
			}

		}else if(mid == AUDIO_MID){
			if(dir == MediaDirection.SendOnly){
				mTrackAudioLocal = track;
				bMute = m_audioMuted;
                //Add by Boris
                if (audioPairingExternalRender != null && !mApp.mFileMode) {
                    Log.v("ClickCall","MediaAudioPairingExternalRender, addAuidoPairingExternalRenderer~~~~~~~~~");
                    //mTrackAudioLocal.addAuidoPairingExternalRenderer(audioPairingExternalRender);
                };
                //add by alfang 
                if (audioDataCaputreRaw != null) {
                    Log.v("ClickCall","audioDataCaputreRaw, addAuidoDataExternalRender capture from hardware");
                    //mTrackAudioLocal.addAudioDataExternalRender(audioDataCaputreRaw,ExternalAudioRenderType.CaptureFromHardware);
                }
                if (audioDataCaputreBeforeEncode != null) {
                    Log.v("ClickCall","audioDataCaputreBeforeEncode, addAuidoDataExternalRender capture before encode");
                    //mTrackAudioLocal.addAudioDataExternalRender(audioDataCaputreBeforeEncode,ExternalAudioRenderType.CaptureBeforeEncode);
                }
                
			}else if(dir == MediaDirection.RecvOnly){
				mTrackAudioRemote = track;
				if (audioDataPlayback != null) {
                    Log.v("audioDataPlayback","addAuidoDataExternalRender playback to hardware");
                    //mTrackAudioRemote.addAudioDataExternalRender(audioDataPlayback,ExternalAudioRenderType.PlaybackToHardware);
                }
			}

            if (mApp.m_bEnableAaec) {
                Log.i("ClickCall", "AAEC is enabled.");
                mConn.GetAudioConfig(AUDIO_MID).SetECType(4);
				long aectype = mConn.GetAudioConfig(AUDIO_MID).GetECType();
				Log.i("ClickCall","Get AEC type from mediasession, type(0:None,1:Build-in,2:WMEdefault, 3: TC, 4: Alpha):" + aectype);
			}
            else {
                Log.i("ClickCall","AAEC is disabled.");
                mConn.GetAudioConfig(AUDIO_MID).SetECType(2);
				long aectype = mConn.GetAudioConfig(AUDIO_MID).GetECType();
				Log.i("ClickCall", "Get AEC type from mediasession, type(0:None,1:Build-in,2:WMEdefault, 3: TC, 4: Alpha):" + aectype);
			}

		}
        else if( mid == SHARE_MID )
        {
			if(mApp.m_bScreenHWCodec) {
				mConn.GetShareConfig(SHARE_MID).EnableHWAcceleration(true, MediaConfig.WmeHWAccelerationConfig.WmeHWAcceleration_Encoder);
				mConn.GetShareConfig(SHARE_MID).EnableHWAcceleration(true, MediaConfig.WmeHWAccelerationConfig.WmeHWAcceleration_Decoder);
			}

            if( dir == MediaDirection.RecvOnly ) {
                track.addRenderWindow(mASRender);
                mTrackShareRemote = track;
            }else if(dir==MediaDirection.SendOnly) {
				mTrackShareLocal = track;
			}
        }
		
		Log.v("MediaSession", "onMediaReady, track=" + track + ", mConn=" + mConn + ", mid=" + mid + ", bMute=" + bMute);
		long cret = 0;
		if(mLocalRender != null || dir == MediaDirection.RecvOnly){
			cret = track.Start(bMute);
		}
		if(cret != 0){
			mApp.setMSLastRetErrorCode(cret);
		}
        // Allow Calabash to verify these callback variables:
        mCalabash.onMediaReadyStore(mid, dir, type, track);
	}

	public void onSDPReady(SDPType type, String sdp) {
		Log.v("MediaSession", "onSDPReady, typ=" + type + ", sdp=" + sdp);
		if(mSdpEvent != null){
			mSdpEvent.onSDP(type, sdp);
		}
			
		mCurrSDPType = type;
	}
	
	public void OnCSIsChanged(long mid, long vid, long[] oldCSIArray, long[] newCSIArray){
		String mediaType = "audio";
		if(mid == VIDEO_MID)
			mediaType = "video";
		else if(mid == SHARE_MID)
			mediaType = "share";
		
		mCalabash.onCSIChanged(mediaType, vid, newCSIArray);
		Log.v("MediaSession", "Endpoint, OnCSIsChanged, mid=" + mid + ", vid=" + vid);
	}
	
	public String getSDPType () {
		if (mCurrSDPType == SDPType.None)
			return "None";
		else if (mCurrSDPType == SDPType.Offer)
			return "Offer";
		else if (mCurrSDPType == SDPType.Answer)
			return "Answer";
		else if (mCurrSDPType == SDPType.Pranswer)
			return "Pranswer";
		else return "Unknown";
	}
	
	public String stopCall(){
		String ret = "";
        m_audioFileEnded = false;
        m_videoFileEnded = false;
        uninstallScreenSharingListener();

		if(mConn == null)
			return ret;
        //Add by Boris
        if (mTrackAudioLocal != null){
            mTrackAudioLocal.removeAuidoPairingExternalRenderer();
            mTrackAudioLocal.removeAudioDataExternalRender(audioDataCaputreRaw);
            mTrackAudioLocal.removeAudioDataExternalRender(audioDataCaputreBeforeEncode);
        }
        audioPairingExternalRender = null;
        audioDataCaputreRaw = null;
        audioDataCaputreBeforeEncode = null;
        if(mTrackAudioRemote != null)
        {
            mTrackAudioRemote.removeAudioDataExternalRender(audioDataPlayback);
        }
        audioDataPlayback  = null;
        //
		mConn.stop();
		ret = mConn.getMediaSessionMetrics();
		if(mTrackVideoLocal != null){
			mTrackVideoLocal.Stop();
			mTrackVideoLocal.removeRenderWindow(mLocalRender);
			mTrackVideoLocal.destroy();
			mTrackVideoLocal = null;
			mLocalRender = null;
		}
		for(int i = 0; i < mMaxRemoteVideo; i++) {
			if(mTrackVideoRemotes[i] == null)
				continue;
			mTrackVideoRemotes[i].Stop();
			mTrackVideoRemotes[i].removeRenderWindow(mRemoteRenders[i]);
			mTrackVideoRemotes[i].destroy();
			mTrackVideoRemotes[i] = null;
			mRemoteRenders[i] = null;
		}
		if(mTrackAudioLocal != null){
			mTrackAudioLocal.Stop();
			mTrackAudioLocal.destroy();
			mTrackAudioLocal = null;
		}
		if(mTrackAudioRemote != null){
			mTrackAudioRemote.Stop();
			mTrackAudioRemote.destroy();
			mTrackAudioRemote = null;
		}
        if( mTrackShareRemote != null )
        {
            mTrackShareRemote.Stop();
            mTrackShareRemote.destroy();
            mTrackShareRemote = null;
        }
		if( mTrackShareLocal != null )
		{
			mTrackShareLocal.Stop();
			mTrackShareLocal.destroy();
			mTrackShareLocal = null;
		}
		mConn.destroy();
		mConn = null;
		return ret;
	}
	
	public JSONObject getDebugOption(MediaType mediaType)
	{
        JSONObject option = null;
        
        if(mediaType == MediaType.Audio)
        	option = m_audioDebugOption;
        else if(mediaType == MediaType.Video)
        	option = m_videoDebugOption;
        else if (mediaType == MediaType.Sharing )
        	option = m_sharingDebugOption;
        return option;
	}
	
	public void setDebugOptions(MediaType mediaType, boolean isIceEnabled, boolean isSrtpEnabled, boolean isDtlsSrtpEnalbed,
			boolean isRtcpMux, String sOverrideIP, int nOverridePort){
		JSONObject option = getDebugOption(mediaType);
		try {
			option.put("enableICE", isIceEnabled);
			option.put("enableSRTP", isSrtpEnabled);
			option.put("enableDtlsSRTP", isDtlsSrtpEnalbed);
			option.put("enableRTCPMux", isRtcpMux);
			if(sOverrideIP != null)
				option.put("overrideMediaIPAddress", sOverrideIP);
			if(nOverridePort > 0)
				option.put("overrideMediaPort", nOverridePort);
		} catch (JSONException e) {
			e.printStackTrace();
		}
	}
    
    public void setFilePath(String srcPath, String dstPath, MediaType mediaType, boolean isLoopFile, boolean isTsMode)
    {
        JSONObject option = getDebugOption(mediaType);
        
        try {
            if(srcPath != null && srcPath.length() != 0)
            {
                JSONObject capture = new JSONObject();
                capture.put("path", srcPath);
                capture.put("isLoop", isLoopFile);
                capture.put("timestamp", isTsMode);
                option.put("fileCapture", capture);
            }
            if(dstPath != null && dstPath.length() != 0)
            {
                JSONObject render = new JSONObject();
                render.put("path", dstPath);
                option.put("fileRender", render);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }
	
	public long switchCamera(MediaDevice dev){
		if(mTrackVideoLocal != null){
			return mTrackVideoLocal.SetCaptureDevice(dev);
		}else{
			Log.e("MediaSession", "switchCamera, it doesn't have local track!");
			return 1;
		}
	}
	
	public void setRemoteRenderMode() {
		ScalingMode mode;
		
		if ((getActivityRotation() == Configuration.ORIENTATION_PORTRAIT && (decodedHeight > decodedWidth)) ||
				(getActivityRotation() == Configuration.ORIENTATION_LANDSCAPE && (decodedWidth > decodedHeight))) {
			 mode = ScalingMode.CropFill;			
			Log.i("Endpoint", "onDecodeSizeChanged, using CropFill and Orientation is " + mCurrRotation);
		} else {
			mode = ScalingMode.LetterBox;
			Log.i("Endpoint", "onDecodeSizeChanged, using LetterBox and Orientation is " + mCurrRotation);
		}
		
		if(mTrackVideoRemotes[0] != null){
			mTrackVideoRemotes[0].SetRenderMode(mode);
		}else{
			Log.e("MediaSession", "changeScalingMode, it doesn't have remote track!");
		}
		if(mTrackShareRemote != null )
            mTrackShareRemote.SetRenderMode(mode);
		mCurrScalingMode = mode;
	}
	
	/**
	 * Backdoor method to allow Integration Tests to input fake activity orientation, controlled by Activity.setRequestedOrientation
	 * @param currOrientation
	 */
	public void setRemoteRenderMode(int currOrientation) {
		ScalingMode mode;
		
		if ((currOrientation == Configuration.ORIENTATION_PORTRAIT && (decodedHeight > decodedWidth)) ||
				(currOrientation == Configuration.ORIENTATION_LANDSCAPE && (decodedWidth > decodedHeight))) {
			 mode = ScalingMode.CropFill;			
			Log.i("Endpoint", "onDecodeSizeChanged, using CropFill and Orientation is " + mCurrRotation);
		} else {
			mode = ScalingMode.LetterBox;
			Log.i("Endpoint", "onDecodeSizeChanged, using LetterBox and Orientation is " + mCurrRotation);
		}
		
		if(mTrackVideoRemotes[0] != null){
			mTrackVideoRemotes[0].SetRenderMode(mode);
		}else{
			Log.e("MediaSession", "changeScalingMode, it doesn't have remote track!");
		}
		if( mTrackShareRemote != null )
            mTrackShareRemote.SetRenderMode(mode);
		mCurrScalingMode = mode;
	}
	
	public void setLocalRenderMode(ScalingMode mode) {
		if(mTrackVideoLocal != null){
			mTrackVideoLocal.SetRenderMode(mode);
		}else{
			Log.e("MediaSession", "changeScalingMode, it doesn't have local track!");
		}
	}

	public void mute(boolean bMute, MediaType mediaType, boolean bOutput)
	{
	    switch (mediaType) {
	        case Sharing:
	            if (!bOutput && mTrackShareRemote != null) {
	                if (bMute)
	                	mTrackShareRemote.Mute();
	                else
	                	mTrackShareRemote.Unmute();
	            }
	            break;
	        case Audio:
				if (bOutput && mTrackAudioLocal != null) {
	                if (bMute)
	                	mTrackAudioLocal.Mute();
	                else
	                	mTrackAudioLocal.Unmute();
	            }
				if (!bOutput && mTrackAudioRemote != null) {
					if (bMute)
						mTrackAudioRemote.Mute();
					else
						mTrackAudioRemote.Unmute();
				}

	            break;
	        case Video:
				if (bOutput && mTrackVideoLocal != null) {
	                if (bMute)
	                	mTrackVideoLocal.Mute();
	                else
	                	mTrackVideoLocal.Unmute();
	            }
				if (!bOutput && mTrackVideoRemotes[0] != null) {
					if (bMute)
						mTrackVideoRemotes[0].Mute();
					else
						mTrackVideoRemotes[0].Unmute();
				}
				break;
	        default:
	            break;
	    }
	}
	
	public void muteTrackLocal(MediaType mediaType) {
		if (mediaType == MediaType.Audio) {
			if (mTrackAudioLocal != null) {
				mTrackAudioLocal.Mute();
			}
			else {
				Log.e("MediaSession", "it doesn't have audio local track!");
			}
		}
		
		if (mediaType == MediaType.Video) {
			if (mTrackVideoLocal != null) {
				mTrackVideoLocal.Mute();
			}
			else {
				Log.e("MediaSession", "it doesn't have video local track");
			}
		}
	}
	
	public void unmuteTrackLocal(MediaType mediaType) {
		if (mediaType == MediaType.Audio) {
			if (mTrackAudioLocal != null) {
				mTrackAudioLocal.Unmute();
			}
			else {
				Log.e("MediaSession", "it doesn't have audio local track!");
			}
		}
		
		if (mediaType == MediaType.Video) {
			if (mTrackVideoLocal != null) {
				mTrackVideoLocal.Unmute();
			}
			else {
				Log.e("MediaSession", "it doesn't have video local track");
			}
		}	
	}
	
	public void muteTrackRemote(MediaType mediaType) {
		if (mediaType == MediaType.Audio) {
			if (mTrackAudioRemote != null) {
				mTrackAudioRemote.Mute();
			}
			else {
				Log.e("MediaSession", "it doesn't have audio remote track!");
			}
		}

		if (mediaType == MediaType.Video) {
			if (mTrackVideoRemotes[0] != null) {
				mTrackVideoRemotes[0].Mute();
			}
			else {
				Log.e("MediaSession", "it doesn't have video remote track");
			}
		}

        if(mediaType == MediaType.Sharing)
        {
            if(mTrackShareRemote != null) {
                mTrackShareRemote.Mute();
            }else {
				Log.e("MediaSession", "it doesn't have share remote track");
			}
        }
	}

	public void unmuteTrackRemote(MediaType mediaType) {
		if (mediaType == MediaType.Audio) {
			if (mTrackAudioRemote != null) {
				mTrackAudioRemote.Unmute();
			}
			else {
				Log.e("MediaSession", "it doesn't have audio remote track!");
			}
		}

		if (mediaType == MediaType.Video) {
			if (mTrackVideoRemotes[0] != null) {
				mTrackVideoRemotes[0].Unmute();
			}
			else {
				Log.e("MediaSession", "it doesn't have video remote track");
			}
		}

        if(mediaType == MediaType.Sharing)
        {
            if(mTrackShareRemote != null) {
                mTrackShareRemote.Unmute();
            }else {
				Log.e("MediaSession", "it doesn't have share remote track");
			}
        }
	}
	
	public long stopStartTrack(MediaType mediaType, MediaDirection direction, boolean bStart)
	{
		long ret = -34;
		Log.i("MediaSession", "stopStartTrack, mediaType=" + mediaType + ", direction=" + direction + ", bStart=" + bStart);
		MediaTrack aTrack = null;
		if (mediaType == MediaType.Audio) {
			if(direction == MediaDirection.SendOnly)
				aTrack = mTrackAudioLocal;
			else
				aTrack = mTrackAudioRemote;
		}
		else if (mediaType == MediaType.Video) {
			if(direction == MediaDirection.SendOnly)
				aTrack = mTrackVideoLocal;
			else
				aTrack = mTrackVideoRemotes[0];
		}
		else if( mediaType == MediaType.Sharing )
        {
			if(direction == MediaDirection.RecvOnly)
				aTrack = mTrackShareRemote;
			else
				aTrack = mTrackShareLocal;
        }
        if(aTrack != null){
        	if(bStart)
        		ret = aTrack.Start();
        	else
        		ret = aTrack.Stop();
        }
		else {
			Log.e("MediaSession", "it doesn't have found track: direction=" + direction + ", type=" + mediaType);
		}
        return ret;
	}
	
	public AudioStatistics getAudioStats() {
		if(mApp.m_bHasAudio) {
			return mConn.getAudioStatistics(AUDIO_MID);
		}
		else {
			AudioStatistics audioStatistics =  new AudioStatistics();
			audioStatistics.mRemote = new MediaStatistics.AudioStats();
			audioStatistics.mLocal = new MediaStatistics.AudioStats();
			audioStatistics.mLocalSession =  new MediaStatistics.NetworkStatistics();
			audioStatistics.mRemoteSession =  new MediaStatistics.NetworkStatistics();
			audioStatistics.mConnection = new MediaStatistics.ConnectionStats();
			return audioStatistics;
		}
	}

	public VideoStatistics getVideoStats() {
		return mConn.getVideoStatistics(VIDEO_MID);
	}
	
	public SharingStatistics getSharingStats() {
		if(mApp.m_bHasShare) {
			return mConn.getSharingStatistics(SHARE_MID);
		}
		else {
			return new SharingStatistics();
		}
	}
    
    public AggregateNetworkMetricStats getNetworkMetrics() {
        return mConn.getNetworkMetrics();
    }
	
    public String getSharingStatsJSON() {
        String ret = mConn.getSharingStatisticsJSON(SHARE_MID);
        Log.i("getSharingStatsJSON", ret);
        return ret;
    };

    public boolean checkSyncStatus(String result, int rate) {
		boolean bCheckPass = false;
		
		if(null != mTrackAudioRemote)
		{
			long[] csi = mTrackAudioRemote.getCSI();
			SyncStatistics syncStats = mConn.getSyncStatistics((int)csi[0]);
			Log.i("MediaSession", "Endpoint::checkSyncStatus, get sync stats: bDecidedPlay = " + syncStats.bDecidedPlay + ", fDecideSyncPlayRatio = " + syncStats.fDecideSyncPlayRatio + ", fDecideDropRatio = " + syncStats.fDecideDropRatio + ", fDecideAheadPlayRatio = " + syncStats.fDecideAheadPlayRatio + ", fDecideNoSyncPlayRatio = " + syncStats.fDecideNoSyncPlayRatio + ", fDecideBufferRatio = " + syncStats.fDecideBufferRatio);
			
			double fCheckRatio = rate / 100.0;
			double fStatsRatio = 0.0;
			double fTotalPlayRatio = syncStats.fDecideSyncPlayRatio + syncStats.fDecideDropRatio + syncStats.fDecideAheadPlayRatio + syncStats.fDecideNoSyncPlayRatio;
	        if(fTotalPlayRatio > 0.0)
	        {
				if(0 == result.compareToIgnoreCase("sync"))
		        {
		            fStatsRatio = syncStats.fDecideSyncPlayRatio / fTotalPlayRatio;
		        }
		        else if(0 == result.compareToIgnoreCase("delay"))
		        {
		            fStatsRatio = syncStats.fDecideDropRatio / fTotalPlayRatio;
		        }
		        else if(0 == result.compareToIgnoreCase("ahead"))
		        {
		            fStatsRatio = syncStats.fDecideAheadPlayRatio / fTotalPlayRatio;
		        }
		        else if(0 == result.compareToIgnoreCase("nosync"))
		        {
		            fStatsRatio = syncStats.fDecideNoSyncPlayRatio / fTotalPlayRatio;
		        }
	        }
	        Log.i("MediaSession", "Endpoint::checkSyncStatus, check sync stats: result = " + result + ", fStatsRatio = " + fStatsRatio + ", fCheckRatio = " + fCheckRatio);
	        if(fStatsRatio >= fCheckRatio)
	        {
	            bCheckPass = true;
	        }
		}
		
		return bCheckPass;
	}
	
	@Override
	public void onDecodeSizeChanged(int mid,int vid, int width, int height) {
		decodedHeight = height;
		decodedWidth = width;
		setRemoteRenderMode ();
	}

	@Override
	public void onRenderSizeChanged(int mid,int vid, MediaDirection direction, WmeVideoSizeInfo sizeInfo) {
//		 if (direction == MediaDirection.RecvOnly)
//		 {
//			decodedHeight = sizeInfo.GetHeight();
//			decodedWidth = sizeInfo.GetWidth();
//			setRemoteRenderMode ();
//		 }

		mRenderWidth = sizeInfo.GetWidth();
		mRenderHeight = sizeInfo.GetHeight();
		if (direction == MediaDirection.SendOnly)
		{
			(new Handler(Looper.getMainLooper())).post(new Runnable() {
				@Override
				public void run() {
					ViewGroup.LayoutParams lp = mLocalRender.getLayoutParams();

					if (lp.width < lp.height != mRenderWidth < mRenderHeight) {
						int temp = lp.width;
						lp.width = lp.height;
						lp.height = temp;
						mLocalRender.setLayoutParams(lp);
					}
				}
			});
		}
	}

    @Override
    public void onEncodeSizeChanged(int mid, int width, int height) {
    }

	@Override
	public void onError(int arg0) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onMediaBlocked(int mid,int vid, boolean blocked) {
		Log.v("MediaSession", "onMediaBlocked, mid=" + mid + ",vid=" + vid + ",blocked=" + blocked);		
		if (blocked)
			mBlocked = true;
		else mBlocked = false;
	}
	
	public boolean isMediaBlocked() {
		return mBlocked;
	}

	@Override
	public void onSessionStatus(int mid, MediaType mediaType, ConnectionStatus status){
        Log.v("MediaSession", "onSessionStatus, ConnectionStatus=" + status);
        if (status == ConnectionStatus.Connected)
            mConnected = true;
        else if (status == ConnectionStatus.Disconnected)
            mConnected = false;
        else if (status == ConnectionStatus.FileCaptureEnded) {
            if (mediaType == MediaType.Video) {
                m_videoFileEnded = true;
            } else if (mediaType == MediaType.Audio) {
                m_audioFileEnded = true;
            }
        }
    }

    @Override
    public void onMediaStatus(int mid, int vid, MediaStatus status, boolean hasCSI, long csi){
    }
    @Override
    public void onMediaError(int mid, int vid, int errorCode){
    }

    @Override
    public void onNetworkStatus(NetworkStatus st, NetworkDirection d){
    }
	
	public boolean isConnected() {
		Log.v("MediaSession", "isConnected, mConnected=" + mConnected);
		return mConnected;
    }
	
	public void setActivityOrientation (int currRotation) {
		mCurrRotation = currRotation;
	}
	
	public int getActivityRotation () {
		return mCurrRotation;
	}
	
	public ScalingMode getScalingMode () {
		return mCurrScalingMode;
	}
	
	public static String  formatStatisticsAudio(AudioStatistics stStat)
	{

		String retStr = "";
		
	    retStr+= "AudioStatistics("+stStat.mConnection.connectionType+"): ";
	    
	    retStr+= "Sent(" + stStat.mConnection.uLocalPort + "): ";
	    retStr+= "RTT=" + stStat.mLocalSession.uRoundTripTime + ", ";
	    retStr+= "Bytes=" + stStat.mLocalSession.uBytes + ", Packets=" + stStat.mLocalSession.uPackets + ", ";
	    retStr+= "LossRatio=" + stStat.mLocalSession.fLossRatio + ", LossPackets=" + stStat.mLocalSession.uLostPackets + ", ";
	    retStr+= "Jitter=" + stStat.mLocalSession.uJitter + ", ";
	    retStr+= "Bitrate=" + stStat.mLocal.uBitRate * 8 + "; ";
	    retStr+= "Received(" + stStat.mConnection.uRemotePort + "): ";
	    retStr+= "Bytes=" + stStat.mRemoteSession.uBytes + ", Packets=" + stStat.mRemoteSession.uPackets + ", ";
	    retStr+= "LossRatio=" + stStat.mRemoteSession.fLossRatio + ", LossPackets=" + stStat.mRemoteSession.uLostPackets + ", ";
	    retStr+= "Jitter=" + stStat.mRemoteSession.uJitter + ", ";
	    retStr+= "Bitrate=" + stStat.mRemote.uBitRate * 8;
	    
	    return retStr;
	}

	public static String  formatStatisticsVideo(VideoStatistics stStat)
	{
		String retStr = "";
		
	    retStr+= "VideoStatistics("+stStat.mConnection.connectionType+"): ";
	    
	    retStr+= "Sent(" + stStat.mConnection.uLocalPort + "): ";
	    retStr+= "RTT=" + stStat.mLocalSession.uRoundTripTime + ", ";
	    retStr+= "Bytes=" + stStat.mLocalSession.uBytes + ", Packets=" + stStat.mLocalSession.uPackets + ", ";
	    retStr+= "LossRatio=" + stStat.mLocalSession.fLossRatio + ", LossPackets=" + stStat.mLocalSession.uLostPackets + ", ";
	    retStr+= "Jitter=" + stStat.mLocalSession.uJitter + ", ";
	    retStr+= "Size:" + stStat.mLocal.uWidth + "x" + stStat.mLocal.uHeight + ", ";
	    retStr+= "Fps=" + (double)Math.round(stStat.mLocal.fFrameRate*100)/100 + ", ";
	    retStr+= "Bitrate=" + (stStat.mLocal.fBitRate) + "; ";
	    retStr+= "Received(" + stStat.mConnection.uRemotePort + "): ";
	    retStr+= "Bytes=" + stStat.mRemoteSession.uBytes + ", Packets=" + stStat.mRemoteSession.uPackets + ", ";
	    retStr+= "LossRatio=" + stStat.mRemoteSession.fLossRatio + ", LossPackets=" + stStat.mRemoteSession.uLostPackets + ", ";
	    retStr+= "Jitter=" + stStat.mRemoteSession.uJitter + ", ";
	    retStr+= "Size:" + stStat.mRemote.uWidth + "x" + stStat.mRemote.uHeight + ", ";
	    retStr+= "Fps=" + (double)Math.round(stStat.mRemote.fFrameRate*100)/100 + ", ";
	    retStr+= "Bitrate=" + (stStat.mRemote.fBitRate) + ", ";
	    retStr+= "RenderedFrame=" + stStat.mRemote.uRenderFrameCount;

	    
	    return retStr;
	}

	public static String  formatStatisticsScreen(SharingStatistics stStat)
	{
		if(stStat==null)
			return "";
		String retStr = "";
	    retStr+= "ScreenStatistics("+stStat.mConnection.connectionType+"): ";

		if(stStat.mLocal!=null) {
			retStr+= "Sent(" + stStat.mConnection.uLocalPort + "): ";
			retStr+= "RTT=" + stStat.mScreenOutNet.uRoundTripTime + ", ";
			retStr+= "Bytes=" + stStat.mScreenOutNet.uBytes + ", Packets=" + stStat.mScreenOutNet.uPackets + ", ";
			retStr+= "LossRatio=" + stStat.mScreenOutNet.fLossRatio + ", LossPackets=" + stStat.mScreenOutNet.uLostPackets + ", ";
			retStr+= "Jitter=" + stStat.mScreenOutNet.uJitter + ", ";
			retStr+= "Bitrate=" + stStat.mScreenOutNet.uBitRate + ", ";

			retStr += "Size:" + stStat.mLocal.mVideoStats.uWidth + "x" + stStat.mLocal.mVideoStats.uHeight + ", ";
			retStr += "Fps=" + (double) Math.round(stStat.mLocal.mVideoStats.fFrameRate * 100) / 100 + ", ";
			retStr += "IDRRecvNum=" + stStat.mLocal.mVideoStats.uIDRSentRecvNum + ", ";
			retStr += "IDRReqNum=" + stStat.mLocal.mVideoStats.uIDRReqNum + ", ";
			retStr += "TotalCapturedFrames=" + stStat.mLocal.mScreenCaptureStats.uTotalCapturedFrames + ", ";
			retStr += "AverageCaptureTime=" + stStat.mLocal.mScreenCaptureStats.uMsAverageCaptureTime + "; ";
		}

		if(stStat.mRemote != null) {
			retStr+= "Received(" + stStat.mConnection.uRemotePort + "): ";
			retStr+= "Bytes=" + stStat.mScreenInNet.uBytes + ", Packets=" + stStat.mScreenInNet.uPackets + ", ";
			retStr+= "LossRatio=" + stStat.mScreenInNet.fLossRatio + ", LossPackets=" + stStat.mScreenInNet.uLostPackets + ", ";
			retStr+= "Jitter=" + stStat.mScreenInNet.uJitter + ", ";
			retStr+= "Bitrate=" + stStat.mScreenInNet.uBitRate + ", ";
			retStr+= "Size:" + stStat.mRemote.mVideoStats.uWidth + "x" + stStat.mRemote.mVideoStats.uHeight + ", ";
			retStr+= "Fps=" + (double)Math.round(stStat.mRemote.mVideoStats.fFrameRate*100)/100 + ", ";

			retStr += "RenderedFrame=" + stStat.mRemote.mVideoStats.uRenderFrameCount + ", ";
			retStr += "IDRRecvNum=" + stStat.mRemote.mVideoStats.uIDRSentRecvNum + ", ";
			retStr += "IDRReqNum=" + stStat.mRemote.mVideoStats.uIDRReqNum + ", ";
			retStr += "TotalCapturedFrames=" + stStat.mRemote.mScreenCaptureStats.uTotalCapturedFrames + ", ";
			retStr += "AverageCaptureTime=" + stStat.mRemote.mScreenCaptureStats.uMsAverageCaptureTime;
		}

	    return retStr;
	}
	
	public static String formatStatistics(AudioStatistics audio, VideoStatistics video,
			SharingStatistics screen, CpuUsage cpu, MemoryUsage mem)
	{
		if(audio == null || video == null)
			return "Null pointer!";
		if(audio.mConnection == null || audio.mLocalSession == null || audio.mLocal == null || audio.mLocalSession == null || audio.mRemoteSession == null || audio.mRemote == null)
			return "Null pointer audio statistics";
		if(video.mLocalSession == null || video.mConnection == null || video.mLocal == null || video.mLocalSession == null || video.mRemoteSession == null || video.mRemote == null)
			return "Null pointer video statistics";

        String retStr = "";
        if(cpu != null){
        	retStr += "CPU:" + (int)cpu.fTotalUsage + ", app=" + (int)cpu.fProcessUsage;
        	int i = 0;
        	for(float a : cpu.pfCores){
        		retStr += " core" + i + "=" + (int)a;
        		i++;
        	}
        	retStr += "\r\n";
        }
        if(mem != null) {
        	retStr += "Mem:" + mem.uMemoryUsed + ", app=" + mem.uProcessMemroyUsed + ", total=" + mem.uMemoryTotal;
        	retStr += "\r\n";
        }
        retStr += "LocalIP:" + audio.mConnection.localIp + " ";
        retStr += "RemoteIP:" + audio.mConnection.remoteIp + "\r\n";
        //Audio
        retStr += "\r\nAudio:" + audio.mConnection.connectionType + "\r\n";
        retStr += "  RTT:" + audio.mLocalSession.uRoundTripTime + "\r\n";
        retStr += "  Sent(" + audio.mConnection.uLocalPort + "): ";
        retStr += String.format("    Bytes(Packets):%d(%d) ", audio.mLocalSession.uBytes, audio.mLocalSession.uPackets);
        retStr += String.format("    Loss:%s,Jitter:%d ", audio.mLocalSession.fLossRatio, audio.mLocalSession.uJitter);
        retStr += "    Bitrate:" + audio.mLocal.uBitRate * 8 + "\r\n";
        
        retStr += "  Received(" + audio.mConnection.uRemotePort + "): ";
        retStr += String.format("    Bytes(Packets):%d(%d) ", audio.mRemoteSession.uBytes, audio.mRemoteSession.uPackets);
        retStr += String.format("    Loss:%s(%d),Jitter:%d ", audio.mRemoteSession.fLossRatio, audio.mRemoteSession.uLostPackets, audio.mRemoteSession.uJitter);
        retStr += "    Bitrate:" + audio.mRemote.uBitRate * 8 + "\r\n";
        //
        retStr += "\r\nVideo:" + video.mConnection.connectionType + "\r\n";
        retStr += "  RTT:" + video.mLocalSession.uRoundTripTime + "\r\n";
        retStr += "  Sent(" + video.mConnection.uLocalPort + "): ";
        retStr += String.format("    Bytes(Packets):%d(%d) ", video.mLocalSession.uBytes, video.mLocalSession.uPackets);
        retStr += String.format("    Loss:%s,Jitter:%d ", video.mLocalSession.fLossRatio, video.mLocalSession.uJitter);
        retStr += String.format("    Size:%dx%d (%.2f) ", video.mLocal.uWidth, video.mLocal.uHeight, video.mLocal.fFrameRate);
        retStr += "    Bitrate:" + video.mLocal.fAvgBitRate + "\r\n";
        
        retStr += "  Received(" + video.mConnection.uRemotePort + "): ";
        retStr += String.format("    Bytes(Packets):%d(%d) ", video.mRemoteSession.uBytes, video.mRemoteSession.uPackets);
        retStr += String.format("    Loss:%s(%d), Jitter:%d ", video.mRemoteSession.fLossRatio, video.mRemoteSession.uLostPackets, video.mRemoteSession.uJitter);
        retStr += String.format("    Size:%dx%d (%.2f) ", video.mRemote.uWidth, video.mRemote.uHeight, video.mRemote.fFrameRate);
        retStr += "    Bitrate:" + video.mRemote.fAvgBitRate + "\r\n";
        //
		if(screen!=null) {
			retStr += "\r\nScreen:\r\n";
			if(screen.mLocal != null) {
				retStr += "  RTT:" + screen.mScreenOutNet.uRoundTripTime + "\r\n";
				retStr += "  Sent(" + screen.mConnection.uLocalPort + "): ";
				retStr += String.format("    Bytes(Packets):%d(%d) ", screen.mScreenOutNet.uBytes, screen.mScreenOutNet.uPackets);
				retStr += String.format("    Loss:%s,Jitter:%d ", screen.mScreenOutNet.fLossRatio, screen.mScreenOutNet.uJitter);
				retStr += String.format("    Size:%dx%d (%.2f) ", screen.mLocal.mVideoStats.uWidth, screen.mLocal.mVideoStats.uHeight, screen.mLocal.mVideoStats.fFrameRate);
				retStr += "    Bitrate:" + screen.mLocal.mVideoStats.fAvgBitRate + "\r\n";
			}

			if(screen.mRemote!=null) {
				retStr += "  Received(" + screen.mConnection.uRemotePort + "): ";
				retStr += String.format("    Bytes(Packets):%d(%d) ", screen.mScreenInNet.uBytes, screen.mScreenInNet.uPackets);
				retStr += String.format("    Loss:%s(%d), Jitter:%d ", screen.mScreenInNet.fLossRatio, screen.mScreenInNet.uLostPackets, screen.mScreenInNet.uJitter);
				retStr += String.format("    Size:%dx%d (%.2f) ", screen.mRemote.mVideoStats.uWidth, screen.mRemote.mVideoStats.uHeight, screen.mRemote.mVideoStats.fFrameRate);
				retStr += "    Bitrate:" + screen.mRemote.mVideoStats.fAvgBitRate + "\r\n";
			}
		}
        return retStr;
	}

    public void setAudioOutType (MediaDevice dev) {
        if (mTrackAudioRemote != null) {
            mTrackAudioRemote.SetAudioOutType(dev);
        } else {
            Log.e("MediaSession", "setAudioOutType, it doesn't have a remote track!");
        }
    }
    
    public int getCSICount() {
        int count = 0;
        if(mTrackAudioRemote != null) {
            long[] csi = mTrackAudioRemote.getCSI();
            count = csi.length;
        }
        return count;
    }

    public MediaStatus getMediaStatus(MediaType mediaType) {
        MediaStatus status = MediaStatus.Available;
        if (mediaType == MediaType.Audio) {
            if(mTrackAudioRemote != null) {
                status = mTrackAudioRemote.getMediaStatus();
            }
        }
        else if (mediaType == MediaType.Video) {
            if(mTrackVideoRemotes[0] != null) {
                status = mTrackVideoRemotes[0].getMediaStatus();
            }
        }
        else if( mediaType == MediaType.Sharing )
        {
            if(mTrackShareRemote != null) {
                status = mTrackShareRemote.getMediaStatus();
            }
        }

        return status;
    }

    public Calabash getCalabash() {
        return mCalabash;
    }

	public void holdCall(boolean isHold) {
	    if(mConn != null)
	    {
	        MediaDirection direction = isHold ? MediaDirection.SendOnly : MediaDirection.SendRecv;
	        mConn.updateMedia(direction, AUDIO_MID);
	        mConn.updateMedia(direction, VIDEO_MID);
	        mConn.createOffer();
	    }
	}

	public CpuUsage getCpuUsage() {
		if(mConn != null)
			return mConn.getCpuUsage();
		return null;
	}
	
	// For TA:
	public boolean isHWCodecEnabled() {
		return mTrackVideoLocal.getVideoTrackStatistics().bHWEnable;
	}

	public VideoStats[] getVideoTrackStats() {
		VideoStats[] aVideoStats = new VideoStats[mRemoteVideoIndex];
		for(int i = 0; i < mRemoteVideoIndex; i++) {
			if(mTrackVideoRemotes[i] != null) {
				aVideoStats[i] = mTrackVideoRemotes[i].getVideoTrackStatistics();
			}else {
				aVideoStats[i] = null;
			}
		}
		return aVideoStats;
	}

    public void enableDSCP(boolean bEnable)
    {
        if(mConn != null)
            mConn.GetGlobalConfig().EnableDSCP(bEnable);
     }

    public boolean IsDSCPEnable()
    {
        if(mConn != null) {
            return mConn.GetGlobalConfig().IsDSCPEnable();
        }

        return true;
    }

    public long SetDSCPMarking(long uDSCPofAudio, long uDSCPofVideo, long uDSCPofScreen)
    {
        if(mConn != null) {
            return mConn.GetGlobalConfig().SetDSCPMarking(uDSCPofAudio, uDSCPofVideo, uDSCPofScreen);
        }
        return -1;
    }

	public MemoryUsage getMemoryUsage() {
		if(mConn != null)
			return mConn.getMemoryUsage();
		return null;
	}

	public void setAudioVolume(int volSetting) {
		if(mTrackAudioRemote != null)
			mTrackAudioRemote.setTrackVolume(volSetting);
	}

	public void Resetlocalrender() {
		mTrackVideoLocal.updateRenderWindow(mLocalRender);

	}

	public void unsubscribeVideo(int vid) {
		if(mConn != null) {
			mConn.unsubscribe(VIDEO_MID, vid);
		}
	}

	public void subscribeVideo(MediaTrack.VideoQualityType resolution,
							   boolean newTrack, MediaConfig.WmeSubscribePolicy activeSpeaker,
							   int csi) {
        if(newTrack){
            if(mConn != null) {
                mConn.subscribe(VIDEO_MID, activeSpeaker, resolution, 255, 0, false, csi);
            }
        }
        else {
            if(mTrackVideoRemotes[0] != null) {
                mTrackVideoRemotes[0].RequestVideo(resolution);
            }
        }
	}

    public void testCSIClashes() {
        if(mConn == null)
            return;

        long[] audioCSI = mTrackAudioLocal.getCSI();
        long[] videoCSI = mTrackVideoLocal.getCSI();

        long[] csis = new long[2];
        csis[0] = audioCSI[0];
        csis[1] = videoCSI[0];

        long ret = mConn.addObservedCSIs(csis, "self");
        if(ret == WmeError.S_FALSE) {
            Log.i("ClickCall", "testCSIClashes passed");
        }
    }

    public void testCSIClash() {
        if(mConn == null)
            return;

        long[] audioCSI = mTrackAudioLocal.getCSI();

        long ret = mConn.addObservedCSI(audioCSI[0], "Self", MediaType.Audio);
        if(ret == WmeError.S_FALSE) {
            Log.i("ClickCall", "testCSIClash passed");
        }
    }
}

