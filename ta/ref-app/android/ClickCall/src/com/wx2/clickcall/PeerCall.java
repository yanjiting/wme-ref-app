package com.wx2.clickcall;

import org.json.JSONException;
import org.json.JSONObject;

import android.annotation.SuppressLint;
import android.util.Log;

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
import com.webex.wme.MediaStatistics.VideoStats;
import com.webex.wme.MediaTrack.ScalingMode;
import com.webex.wme.MediaConfig.WmePerformanceDumpType;
import com.webex.wseclient.WseSurfaceView;
import com.wx2.support.CalliopeClient;
import com.wx2.support.WSClient;

import android.os.Looper;
import android.os.Handler;

public class PeerCall{
	private JSONObject paramAudio = null;
	private JSONObject paramVideo = null;
	private boolean enableQosAudio = true;
	private boolean enableQosVideo = true;

	private WSClient mWsClient = null;
	public Endpoint mCall = null;
	private static PeerCall instance = null;
	private String sLocalSDP = null;
	private DeviceManager mDevManager = null;
    private boolean mIsIceEnabled = false;
    private boolean mIsSrtpEnabled = false;
	private boolean mIsDtlsSrtpEnabled = false;
    private boolean mIsMultiEnabled = false;
    private boolean mIsRtcpMux = true;
    private boolean mIsMute = false;

    public String audioSrc = null;
    public String audioDst = null;
    public String videoSrc = null;
    public String videoDst = null;
    public boolean mIsVideoLoopFile = false;
    public boolean mIsAudioLoopFile = false;
    private boolean mIsTsMode = false;
    private int mWmeDataDump = 0;
    private boolean mIsFeatureTogglesSet = false;
    private String mMariRateAdaptation = null;
    private CalliopeClient mCalliope;

	protected PeerCall() {
		mDevManager = new DeviceManager();
    }

	public static PeerCall instance() {
	    if(instance == null) {
	    	instance = new PeerCall();
	    }
	    return instance;
    }

	@SuppressLint("DefaultLocale")
	protected void sendSDPToWebsocket(SDPType sdpType, String sdp){
		if(mWsClient != null){
            mWsClient.send(sdpType.toString().toLowerCase(), sdp);
		}
	}

	protected void sendVenue() {
		mWsClient.send("venue", mVenueUrl);
	}

	private boolean mThroughCalliope = false;
	private String mVenueUrl = null;
	private String mOverrideIP = null;
	private int mOverrideAudioPort = 0;
	private int mOverrideVideoPort = 0;
	private int mOverrideSharingPort = 0;
	
	private void setupIceRtcpDebugOption()
	{
		mCall.setDebugOptions(MediaType.Audio, mIsIceEnabled, mIsSrtpEnabled, mIsDtlsSrtpEnabled, mIsRtcpMux, mOverrideIP, mOverrideAudioPort);
		mCall.setDebugOptions(MediaType.Video, mIsIceEnabled, mIsSrtpEnabled, mIsDtlsSrtpEnabled, mIsRtcpMux, mOverrideIP, mOverrideVideoPort);
		mCall.setDebugOptions(MediaType.Sharing, mIsIceEnabled, mIsSrtpEnabled, mIsDtlsSrtpEnabled, mIsRtcpMux, mOverrideIP, mOverrideSharingPort);
	}
	
	public void startCall(String linusAddr, String wsAddr)
	{
		mThroughCalliope = true;
        mCalliope = new CalliopeClient() {
            @Override
            public void onVenue(String url) {
                PeerCall.this.mVenueUrl = url;
                Log.i("MediaSession", "PeerCall, venue url=" + mVenueUrl);
                PeerCall.this.sendVenue();
                PeerCall.this.StartCallWithCalliope();
            }

            @Override
            public void onConfluence(String sdp, String url) {
                Log.v("ClickCall", "PeerCall, caller sdp answer received=" + sdp);

				MyApplication mApp = (MyApplication)MyApplication.getAppContext();
				if(!mApp.mIsViewer)
					requestFloor();

                PeerCall.this.mCall.answerReceived(sdp);

            }
        };
        mCalliope.setLinusUrl(linusAddr);
		startCall(wsAddr);
	}
	
	private void OnStartCall(final int nCount){
		(new Handler (Looper.getMainLooper ())).post(new Runnable() {
			@Override
			public void run() {
				if (mThroughCalliope) {
					// for calliope, the first one start the call session
					if (nCount == 1) {
						PeerCall.this.mCalliope.createVenue();
					}
				} else if (nCount > 1) {
					// for p2p, the second one start the call session
					PeerCall.this.startCall();
				}
			}
		});
	}

	public void startCall(String svr){
		String sWsUrl = "";
		if(!svr.startsWith("ws")){
			sWsUrl += "ws://";
		}
		sWsUrl += svr;
		if(!svr.contains("?r=")){
			sWsUrl += "/echo?r=1217";
		}
		Log.i("ClickCall", "connectWebsocket, sWsUrl=" + sWsUrl);
		mWsClient = new WSClient(sWsUrl) {
            public void onMessage(String type, String message) {
                if(type.equalsIgnoreCase("venue")){
                    Log.i("ClickCall", "PeerCall, venue.");
                    mVenueUrl = message;
                    PeerCall.this.StartCallWithCalliope();
                }
                else if(type.equalsIgnoreCase("answer")){
                    Log.i("ClickCall", "PeerCall, answer, mCall=" + PeerCall.this.mCall);
                    if(PeerCall.this.mCall != null){
                        PeerCall.this.mCall.answerReceived(message);
                    }
                }
                else if(type.equalsIgnoreCase("offer")){
                    Log.i("ClickCall", "PeerCall, offer");
                    PeerCall.this.acceptCall(message, false);
                }
                else{
                    Log.i("ClickCall", "PeerCall, type=" + type);
                }
            }

            public void onOpen() {
                Log.i("ClickCall", "Websocket client, connected");
            }

            public void onStartCall(int count) {
                OnStartCall(count);
            }

		};
		mWsClient.connect();
    }

	protected void StartCallWithCalliope() {
		mCall.startCall(new SDPReadyEvent() {
			public void onSDP(SDPType type, String sdp) {
				Log.v("ClickCall", "PeerCall, caller sdp offer received. (StartCallWithCalliope)");
				PeerCall.this.mCalliope.createConfluence(mVenueUrl, sdp);
			}
		});
	}

	protected void acceptCall(String sdp, boolean bUpdate) {
		Log.v("ClickCall", "acceptCall, sdp=" + sdp + ", mCall=" + mCall);

		mCall.acceptCall(sdp, bUpdate, new SDPReadyEvent() {
			public void onSDP(SDPType type, String sdp) {
				Log.v("ClickCall", "acceptCall, send out sdp answer.");
				sLocalSDP = sdp;
				PeerCall.this.sendSDPToWebsocket(SDPType.Answer, sdp);
			}
		});
	}
	
	public void preview(WseSurfaceView local, WseSurfaceView remote, WseSurfaceView asView)
	{
		mCall = new Endpoint(local, remote, asView);
		setupIceRtcpDebugOption();
		this.setFilePath();
		mCall.preview();
		mCall.enableQos(MediaType.Audio, enableQosAudio);
		mCall.enableQos(MediaType.Video, enableQosVideo);
        if (mIsFeatureTogglesSet) {
            mCall.setFeatureToggles(mMariRateAdaptation);
        }
		mCall.setParameters(MediaType.Audio, paramAudio);
		mCall.setParameters(MediaType.Video, paramVideo);
		mCall.setMuteOption(mIsMute);
        mCall.setWmeDataDump(MediaType.Video, mWmeDataDump);
        //mCall.enableDSCP(false);
	}

	public void startCall(){
		mCall.startCall(new SDPReadyEvent(){
			public void onSDP(SDPType type, String sdp){
				Log.v("ClickCall", "PeerCall, caller sdp offer received.");
				sLocalSDP = sdp;
				PeerCall.this.sendSDPToWebsocket(SDPType.Offer, sdp);
			}
		});
	}

	public String stopCall(){
		String ret = "";
		mThroughCalliope = false;
		if(mVenueUrl != null){
			mCalliope.deleteConfluence("");
            mCalliope.deleteVenue(mVenueUrl);
			mVenueUrl = null;
		}
		if(mWsClient != null){
			mWsClient.close();
			mWsClient = null;

		}
		if(mCall != null){
			ret = mCall.stopCall();
			mCall = null;
		}
		return ret;
	}

	public void answerReceived(String sdp){
		Log.v("ClickCall", "PeerCall, sdp answer received.");
		if(mCall != null){
			mCall.answerReceived(sdp);
		}
	}

	public AudioStatistics getAudioStats(){
		if(mCall != null){
			return mCall.getAudioStats();
		}
		return new AudioStatistics();
	}

	public VideoStatistics getVideoStats(){
		if(mCall != null){
			return mCall.getVideoStats();
		}
		return new VideoStatistics();
	}
	
	public VideoStats[] getVideoTrackStats() {
		if(mCall != null){
			return mCall.getVideoTrackStats();
		}
		return null;
	}
	
	public SharingStatistics getSharingStats(){
		if(mCall != null){
			return mCall.getSharingStats();
		}
		return new SharingStatistics();
	}
    
    public AggregateNetworkMetricStats getNetworkMetrics(){
        if(mCall != null){
            return mCall.getNetworkMetrics();
        }
        return new AggregateNetworkMetricStats();
    }
	
    public String getSharingStatsJSON(){
        if(mCall != null){
            return mCall.getSharingStatsJSON();
        }
        return "{}";
    }
	public String getsLocalSDP() {
		return sLocalSDP;
	}

	public void setsLocalSDP(String sLocalSDP) {
		this.sLocalSDP = sLocalSDP;
	}

	public void switchCamera(CameraType type){
		MediaDevice dev = mDevManager.getCamera(type);
		if(mCall != null){
			mCall.switchCamera(dev);
		}
	}

	public void setRemoteRenderMode(){
		mCall.setRemoteRenderMode();
	}

	public void setRemoteRenderMode(int fakeActivityOrientation){
		mCall.setRemoteRenderMode(fakeActivityOrientation);
	}
	
	public void mute(MediaType mediaType, boolean bMute, boolean bSpk){
		if (mCall != null){
			mCall.mute(bMute, mediaType, bSpk);
		}
	}
	
	public void muteTrackLocal(MediaType mediaType) {
		if (mCall == null)
			return;
		mCall.muteTrackLocal(mediaType);
	}

	public void unmuteTrackLocal(MediaType mediaType) {
		if (mCall == null)
			return;
		mCall.unmuteTrackLocal(mediaType);
	}

	public void muteTrackRemote(MediaType mediaType) {
		if (mCall == null)
			return;
		mCall.muteTrackRemote(mediaType);
	}

	public void unmuteTrackRemote(MediaType mediaType) {
		if (mCall == null)
			return;
		mCall.unmuteTrackRemote(mediaType);
	}
	
	public long startStopTrack(MediaType mediaType, MediaDirection direction, boolean bStart){
		if (mCall == null)
			return -45;
		return mCall.stopStartTrack(mediaType, direction, bStart);
	}

	public void setActivityOrientation(int currRotation){
		if (mCall == null)
			return;
		mCall.setActivityOrientation(currRotation);
	}

	public ScalingMode getScalingMode () {
		if (mCall == null)
			return ScalingMode.CropFill;
		return mCall.getScalingMode();
	}

	public String getSDPType() {
		if (mCall == null)
			return "unknown";
		return mCall.getSDPType();
	}

	public boolean isConnected() {
		if (mCall == null)
			return false;
		return mCall.isConnected();
	}

	public boolean isMediaBlocked() {
		if (mCall == null)
			return false;
		return mCall.isMediaBlocked();
	}
	
	public void pushRemoteView(int idx, WseSurfaceView remote) {
		if(mCall != null)
			mCall.pushRemoteView(idx, remote);
	}

	public String getParameters (MediaType type, String key) {
		if (mCall == null)
			return "{}";
		return mCall.getParameters(type, key);
	}

	public void setParameters (MediaType type, JSONObject jsonKeyValue) {
		if (null == jsonKeyValue) {
			return;
		}

		if(MediaType.Audio == type) {
			paramAudio = jsonKeyValue;
		} else if(MediaType.Video == type) {
			paramVideo = jsonKeyValue;
		}

		if (null != mCall) {
			mCall.setParameters(type, jsonKeyValue);
		}
	}

	public void setParam (MediaType type, JSONObject jsonKeyValue) {
		if (null == jsonKeyValue) {
			return;
		}

        try {
            jsonKeyValue = jsonKeyValue.getJSONObject("param");
        } catch (JSONException e) {
            //e.printStackTrace();
        }

        try {
            JSONObject jsonAudio = jsonKeyValue.getJSONObject("audio");
            enableQosAudio = jsonAudio.getBoolean("enableQos");
            
            JSONObject audioParam = (JSONObject)jsonKeyValue.optJSONObject("audio");
            if(audioParam == null){
                jsonKeyValue.put("audio", audioParam = new JSONObject());
            }
            audioParam.put("supportCmulti", mIsMultiEnabled);
        } catch (JSONException e) {
            //e.printStackTrace();
        }

        try {
            JSONObject jsonVideo = jsonKeyValue.getJSONObject("video");
            enableQosVideo = jsonVideo.getBoolean("enableQos");
        } catch (JSONException e) {
            //e.printStackTrace();
        }

		if(MediaType.Audio == type) {
			paramAudio = jsonKeyValue;
		} else if(MediaType.Video == type) {
			paramVideo = jsonKeyValue;
		}

		if (null != mCall) {
			mCall.setParam(type, jsonKeyValue);
		}
	}

    public void setAudioOutType (AudioOutType type) {
        MediaDevice dev = null;

        if (mDevManager != null) {
            dev = mDevManager.getAudioOutDev(type);
        }
        if (mCall != null && dev != null) {
            mCall.setAudioOutType(dev);
        }
    }
    
	public void enableIce(boolean isIce) {
		mIsIceEnabled = isIce;
	}
	
	public void enableSrtp(boolean isSrtp) {
		mIsSrtpEnabled = isSrtp;
	}

	public void enableDtlsSrtp(boolean isDtlsStrp) {
		mIsDtlsSrtpEnabled = isDtlsStrp;
	}

    public void enableMulti(boolean isMulti) { mIsMultiEnabled = isMulti; }

	public void enableRtcpMux(boolean isRtcpMux) {
		mIsRtcpMux = isRtcpMux;
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
        return mCall.getFeatureToggles();
    }

    public void setWmeDataDump(int dumpFlag)
    {
        mWmeDataDump = dumpFlag;
    }
	
	public void setOverrideIPPort(String ip, int audioPort, int videoPort, int sharingPort)
	{
		mOverrideIP = ip;
		mOverrideAudioPort = audioPort;
		mOverrideVideoPort = videoPort;
		mOverrideSharingPort = sharingPort;
	}
	
	public boolean checkSyncStatus(String result, int rate) {
		return mCall.checkSyncStatus(result, rate);
	}
    
    public int getCSICount() {
        if (mCall != null) {
            return mCall.getCSICount();
        }
        return 0;
    }

    public Calabash getCalabash() {
        if (mCall != null) {
            return mCall.getCalabash();
        }

        return null;
    }

    public void setIsTsMode(boolean isTsMode)
    {
        mIsTsMode = isTsMode;
    }

    private void setFilePath() {
        mCall.setFilePath(audioSrc, audioDst, MediaType.Audio, mIsAudioLoopFile, mIsTsMode);
        mCall.setFilePath(videoSrc, videoDst, MediaType.Video, mIsVideoLoopFile, mIsTsMode);
    }

	public void holdCall(boolean isHold) {
		if(mCall != null){
			mCall.holdCall(isHold);
		}
	}

    public MediaStatus getMediaStatus(MediaType mediaType) {
        MediaStatus status = MediaStatus.Available;
        if(mCall != null){
            status = mCall.getMediaStatus(mediaType);
        }
        return status;
    }

	public CpuUsage getCpuUsage() {
        if(mCall != null)
        	return mCall.getCpuUsage();
		return null;
	}
	
	// For TA:
	public boolean isHWCodecEnabled() {
		return mCall.isHWCodecEnabled();
	}

	public MemoryUsage getMemoryUsage() {
        if(mCall != null)
        	return mCall.getMemoryUsage();
		return null;
	}

	public void setAudioVolume(int volSetting) {
        if(mCall != null)
        	mCall.setAudioVolume(volSetting);
	}

    public void setPerformanceDumpType(WmePerformanceDumpType type) {
        if (mCall != null) {
            mCall.setPerformanceDumpType(type);
        }
    }
}
