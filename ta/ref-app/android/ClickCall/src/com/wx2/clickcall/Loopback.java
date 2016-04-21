package com.wx2.clickcall;

import com.webex.wme.DeviceManager;
import com.webex.wme.DeviceManager.AudioOutType;
import com.webex.wme.DeviceManager.CameraType;
import com.webex.wme.DeviceManager.MediaDevice;
import com.webex.wme.MediaConnection.MediaDirection;
import com.webex.wme.MediaConnection.SDPType;
import com.webex.wme.MediaConnection.MediaType;
import com.webex.wme.MediaConnection.MediaStatus;
import com.webex.wme.MediaStatistics.AudioStatistics;
import com.webex.wme.MediaStatistics.CpuUsage;
import com.webex.wme.MediaStatistics.AggregateNetworkMetricStats;
import com.webex.wme.MediaStatistics.MemoryUsage;
import com.webex.wme.MediaStatistics.VideoStatistics;
import com.webex.wme.MediaStatistics.SharingStatistics;
import com.webex.wme.MediaConfig.WmePerformanceDumpType;
import com.webex.wseclient.WseSurfaceView;
import com.wx2.support.CalliopeClient;

import android.os.Handler;
import android.util.Log;
import org.json.JSONException;
import org.json.JSONObject;

public class Loopback{
	public Endpoint mCaller = null, mCallee = null;
	private DeviceManager mDevManager = null;
	private static Loopback instance = null;
	private String venueUrl = null;
	public boolean mbCallerFirst = false;
	private boolean mIsMute = false; 
	private boolean enableQosAudio = true;
	private boolean enableQosVideo = true;
    private boolean mIsTsMode = false;
	private boolean mIsIceEnabled = false;
    private boolean mIsMultiEnabled = false;
    private boolean mIsFeatureTogglesSet = false;
    private String mMariRateAdaptation = null;
    public String audioSrc = null;
    public String audioDst = null;
    public String videoSrc = null;
    public String videoDst = null;
    public boolean mIsAudioLoopFile = false;
    public boolean mIsVideoLoopFile = false;
    private int mWmeDataDump = 0;
    private CalliopeClient mCalliopeCaller, mCalliopeCallee;

	protected Loopback() {
		mDevManager = new DeviceManager();
    }
	
	public static Loopback instance() {
	    if(instance == null) {
	    	instance = new Loopback();
	    }
	    return instance;
    }
	
	public void enableIce(boolean isIce) {
		mIsIceEnabled = isIce;
	}

    public void enableMulti(boolean isMutli) {
        mIsMultiEnabled = isMutli;
    }
	
	public void acceptCall(String sdp, boolean bUpdate){
		mCallee.acceptCall(sdp, bUpdate, new SDPReadyEvent(){
			public void onSDP(SDPType type, String sdp){
				Log.v("MediaSession", "Loopback, callee sdp answer received.");
				Loopback.this.mCaller.answerReceived(sdp);
			}
		});
	}
    private Handler mHandler = new Handler();
    private Runnable mDelayCreateVenue = new Runnable() {
        public void run() {
            onDelayCreateVenue();
        }
    };
    
	private void onDelayCreateVenue() {
        mCalliopeCaller.createVenue();
	}

    public void onVenue(String url) {
        venueUrl = url;
        Log.i("ClickCall", "onVenue, url=" + venueUrl);
        if(venueUrl == null)
            return;

        if(mbCallerFirst){
            mCaller.startCall(new SDPReadyEvent() {
                public void onSDP(SDPType type, String sdp) {
                    Log.v("MediaSession", "Loopback, caller sdp offer received.");
                    Loopback.this.mCalliopeCaller.createConfluence(Loopback.this.venueUrl, sdp);
                }
            });
            mCallee.startCall(new SDPReadyEvent() {
                public void onSDP(SDPType type, String sdp) {
                    Log.v("MediaSession", "Loopback, callee sdp offer received.");
                    Loopback.this.mCalliopeCallee.createConfluence(Loopback.this.venueUrl, sdp);
                }
            });
        }
        else {
            mCallee.startCall(new SDPReadyEvent() {
                public void onSDP(SDPType type, String sdp) {
                    Log.v("MediaSession", "Loopback, callee sdp offer received.");
                    Loopback.this.mCalliopeCallee.createConfluence(Loopback.this.venueUrl, sdp);
                }
            });
            mCaller.startCall(new SDPReadyEvent() {
                public void onSDP(SDPType type, String sdp) {
                    Log.v("MediaSession", "Loopback, caller sdp offer received.");
                    Loopback.this.mCalliopeCaller.createConfluence(Loopback.this.venueUrl, sdp);
                }
            });
        }
    }

    public void startCall(String mLinusAddr) {
        mCalliopeCaller = new CalliopeClient() {
            @Override
            public void onVenue(String venuUrl) {
                Loopback.this.onVenue(venuUrl);
            }

            @Override
            public void onConfluence(String sdp, String url) {
                MyApplication mApp = (MyApplication)MyApplication.getAppContext();
                if(!mApp.mIsViewer)
                    requestFloor();
                Loopback.this.mCaller.answerReceived(sdp);
            }
        };
        mCalliopeCallee = new CalliopeClient() {
            @Override
            public void onVenue(String venuUrl) {
            }

            @Override
            public void onConfluence(String sdp, String url) {
                Loopback.this.mCallee.answerReceived(sdp);
            }
        };

        mCalliopeCaller.setLinusUrl(mLinusAddr);
		mHandler.postDelayed(mDelayCreateVenue, 100);
	}
	
	public void preview(WseSurfaceView local, WseSurfaceView remote, WseSurfaceView asView){
		mCaller = new Endpoint(local, null, null);
		mCallee = new Endpoint(null, remote, asView);
		mCaller.setMuteOption(mIsMute);
		setupIceRtcpDebugOption();
		setFilePath();
        mCaller.preview();
        mCallee.preview();
        mCaller.enableQos(MediaType.Audio, enableQosAudio);
        mCaller.enableQos(MediaType.Video, enableQosVideo);
        mCallee.enableQos(MediaType.Audio, enableQosAudio);
        mCallee.enableQos(MediaType.Video, enableQosVideo);
        mCaller.setWmeDataDump(MediaType.Video, mWmeDataDump);
        mCallee.setWmeDataDump(MediaType.Video, mWmeDataDump);
        if (mIsFeatureTogglesSet) {
            mCaller.setFeatureToggles(mMariRateAdaptation);
            mCallee.setFeatureToggles(mMariRateAdaptation);
        }
        //mCaller.enableDSCP(false);
	}

	public void startCall(){
		mCaller.startCall(new SDPReadyEvent(){
			public void onSDP(SDPType type, String sdp){
				Log.v("MediaSession", "Loopback, caller sdp offer received.");
				Loopback.this.acceptCall(sdp, false);
			}
		});
	}

    public void setParam (MediaType type, JSONObject jsonKeyValue) {
        if (null == jsonKeyValue) {
            return;
        }

        try {
        	if(jsonKeyValue.has("caller")){
        		boolean isCaller = jsonKeyValue.getBoolean("caller");
        		jsonKeyValue = jsonKeyValue.getJSONObject("param");

        		Log.i("MediaSession", "loopback setParam, type=" + type + ", isCaller=" + isCaller + ", param=" + jsonKeyValue);


                JSONObject audioParam = (JSONObject)jsonKeyValue.optJSONObject("audio");
                if(audioParam == null){
                    jsonKeyValue.put("audio", audioParam = new JSONObject());
                }
                audioParam.put("supportCmulti", mIsMultiEnabled);

                if (isCaller) {
        			mCaller.setParam(type, jsonKeyValue);
        		} else {
        			mCallee.setParam(type, jsonKeyValue);
        		}
        	}
        	else{
                JSONObject audioParam = (JSONObject)jsonKeyValue.optJSONObject("audio");
                if(audioParam == null){
                    jsonKeyValue.put("audio", audioParam = new JSONObject());
                }
                audioParam.put("supportCmulti", mIsMultiEnabled);
                
    			mCaller.setParam(type, jsonKeyValue);
    			mCallee.setParam(type, jsonKeyValue);
        	}
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }
	
	public AudioStatistics getAudioStats(){
		AudioStatistics localAudio = mCaller.getAudioStats();
		AudioStatistics remoteAudio = mCallee.getAudioStats();

		AudioStatistics ret = new AudioStatistics();
		ret.mConnection = remoteAudio.mConnection;
		ret.mConnection.uRTPSent = localAudio.mConnection.uRTPSent;
		ret.mConnection.uRTCPSent = localAudio.mConnection.uRTCPSent;
		ret.mLocalSession = localAudio.mLocalSession;
		ret.mRemoteSession = remoteAudio.mRemoteSession;
		ret.mLocal = localAudio.mLocal;
		ret.mRemote = remoteAudio.mRemote;
		
		return ret;
	}
	
	public VideoStatistics getVideoStats(){
		VideoStatistics localVideo = mCaller.getVideoStats();
		VideoStatistics remoteVideo = mCallee.getVideoStats();
		
		VideoStatistics ret = new VideoStatistics();
		ret.mConnection = remoteVideo.mConnection;
		ret.mConnection.uRTPSent = localVideo.mConnection.uRTPSent;
		ret.mConnection.uRTCPSent = localVideo.mConnection.uRTCPSent;
		ret.mLocalSession = localVideo.mLocalSession;
		ret.mRemoteSession = remoteVideo.mRemoteSession;
		ret.mLocal = localVideo.mLocal;
		ret.mRemote = remoteVideo.mRemote;
		
		return ret;
	}
	public SharingStatistics getSharingStats(){
		//SharingStatistics localScreen = mCaller.getSharingStats();
		SharingStatistics remoteScreen = mCallee.getSharingStats();
		return remoteScreen;
	}

    public AggregateNetworkMetricStats getNetworkMetrics(){
        if(mCaller != null) {
            AggregateNetworkMetricStats networkMetrics = mCaller.getNetworkMetrics();
            return networkMetrics;
        }
        return new AggregateNetworkMetricStats();
    }

	public String stopCall(){
		String ret = "";
		if(mCaller != null)
			ret = mCaller.stopCall();
		if(mCallee != null)
			mCallee.stopCall();

        if(mCalliopeCallee != null)
            mCalliopeCallee.deleteConfluence("");

        if(mCalliopeCaller != null)
            mCalliopeCaller.deleteConfluence("");

        if(venueUrl != null && mCalliopeCaller != null){
            mCalliopeCaller.deleteVenue(venueUrl);
            venueUrl = null;
        }
		mCaller = null;
		mCallee = null;
		return ret;
	}
	
	public long switchCamera(CameraType type){
		Log.i("MediaSession", "loopback switchCamera, type=" + type + ", mDevManager=" + mDevManager);
		MediaDevice dev = mDevManager.getCamera(type);
		return mCaller.switchCamera(dev);
	}
	
	public void mute(MediaType mediaType, boolean bMute, boolean bSpk)
	{
		if(bSpk){
			mCaller.mute(bMute, mediaType, bSpk);
		}else{
			mCallee.mute(bMute, mediaType, bSpk);
		}
	}
	
	public void muteTrackLocal(MediaType mediaType) {
		mCaller.muteTrackLocal(mediaType);
		if (mediaType == MediaType.Audio) {
			mCallee.muteTrackLocal(mediaType);
		}
	}
	
	public void unmuteTrackLocal(MediaType mediaType) {
		mCaller.unmuteTrackLocal(mediaType);
		if (mediaType == MediaType.Audio) { 
			mCallee.unmuteTrackLocal(mediaType);
		}
	}
	
	public long startStopTrack(MediaType mediaType, MediaDirection direction, boolean bStart){
		long ret = -99;
		if(direction == MediaDirection.SendOnly){
			if (mCaller == null){
				ret = mCaller.stopStartTrack(mediaType, direction, bStart);
			}
			
		}else{
			if (mCallee == null){
				ret = mCallee.stopStartTrack(mediaType, direction, bStart);
			}
		}
		return ret;
	}
	
	public void setActivityOrientation (int currRotation) {
		mCaller.setActivityOrientation(currRotation);
	}
	
	private void setupIceRtcpDebugOption()
	{
		mCaller.setDebugOptions(MediaType.Audio, mIsIceEnabled, false, false, true, null, 0);
		mCaller.setDebugOptions(MediaType.Video, mIsIceEnabled, false, false, true, null, 0);
		mCaller.setDebugOptions(MediaType.Sharing, mIsIceEnabled, false, false, true, null, 0);
		mCallee.setDebugOptions(MediaType.Audio, mIsIceEnabled, false, false, true, null, 0);
		mCallee.setDebugOptions(MediaType.Video, mIsIceEnabled, false, false, true, null, 0);
		mCallee.setDebugOptions(MediaType.Sharing, mIsIceEnabled, false, false, true, null, 0);
	}

    public void setIsTsMode(boolean isTsMode)
    {
        mIsTsMode = isTsMode;
    }

	private void setFilePath() {
        mCaller.setFilePath(audioSrc, audioDst, MediaType.Audio, mIsAudioLoopFile, false);
        mCaller.setFilePath(videoSrc, videoDst, MediaType.Video, mIsVideoLoopFile, mIsTsMode);
        mCallee.setFilePath(audioSrc, audioDst, MediaType.Audio, mIsAudioLoopFile, false);
        mCallee.setFilePath(videoSrc, videoDst, MediaType.Video, mIsVideoLoopFile, mIsTsMode);
    }

    public void setAudioOutType (AudioOutType type) {
        MediaDevice dev = mDevManager.getAudioOutDev(type);
        if (mCaller != null && mCallee != null) {
            mCaller.setAudioOutType(dev);
            // mCallee.setAudioOutType(dev);
        }
    }
    
    public void setMuteOption(boolean isMute)
	{
    	mIsMute = isMute;
	}
	
	public void enableQos(boolean bEnableQos)
	{
		enableQosAudio = bEnableQos;
		enableQosVideo = bEnableQos;
	}

    public void setFeatureToggles(String arg)
    {
        mIsFeatureTogglesSet = true;
        mMariRateAdaptation = arg;
    }

    public String getFeatureToggles()
    {
        return mCaller.getFeatureToggles();
    }
    
    public void setWmeDataDump(int dumpFlag)
    {
        mWmeDataDump = dumpFlag;
    }

	public boolean checkSyncStatus(String result, int rate) {
		return mCallee.checkSyncStatus(result, rate);
	}

    public Calabash getCalabashCaller() {
        if (mCaller != null) {
            return mCaller.getCalabash();
        }
        return null;
    }

    public Calabash getCalabashCallee() {
        if (mCallee != null) {
            return mCallee.getCalabash();
        }
        return null;
    }

	public void holdCall(boolean isHold) {
		if(mCaller != null){
			mCaller.holdCall(isHold);
		}
	}
    public MediaStatus getMediaStatus(MediaType mediaType) {
        MediaStatus status = MediaStatus.Available;
        if(mCaller != null){
            status = mCaller.getMediaStatus(mediaType);
        }
        return status;
    }

	public CpuUsage getCpuUsage() {
        if(mCaller != null){
            return mCaller.getCpuUsage();
        }
		return null;
	}
	
	// For TA:
	public boolean isHWCodecEnabled() {
		return mCaller.isHWCodecEnabled();
	}

	public MemoryUsage getMemoryUsage() {
        if(mCaller != null){
            return mCaller.getMemoryUsage();
        }
		return null;
	}

	public void setAudioVolume(int volSetting) {
		if(mCaller != null) {
			mCaller.setAudioVolume(volSetting);
		}
		if(mCallee != null) {
			mCallee.setAudioVolume(volSetting);
		}
	}

	public void Resetlocalrender() {
		mCaller.Resetlocalrender();
		mCallee.Resetlocalrender();
	}
    public void setPerformanceDumpType(WmePerformanceDumpType type) {
        if (mCaller != null) {
            mCaller.setPerformanceDumpType(type);
        }
        if (mCallee != null) {
            mCallee.setPerformanceDumpType(type);
        }
    }
}
