package com.wx2.clickcall;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.app.Application;
import android.content.Context;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.hardware.Camera;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.util.Log;
import android.view.Display;
import android.view.Surface;

import com.webex.wme.DeviceManager;
import com.webex.wme.DeviceManager.CameraType;
import com.webex.wme.MediaConfig;
import com.webex.wme.MediaConnection.MediaDirection;
import com.webex.wme.MediaConnection.MediaStatus;
import com.webex.wme.MediaConnection.MediaType;
import com.webex.wme.MediaStatistics;
import com.webex.wme.MediaStatistics.MemoryUsage;
import com.webex.wme.MediaStatistics.VideoStats;
import com.webex.wme.MediaTrack;
import com.webex.wme.MediaTrack.ScalingMode;
import com.webex.wme.NativeMediaSession;
import com.webex.wseclient.WseEngine;
import com.wx2.log.LogFilePrint;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.io.File;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.ArrayList;
import java.util.Iterator;

/**
 * Created by weichen2 on 2014/4/21.
 */
@SuppressLint("SdCardPath")
public class MyApplication extends Application {
	// Instance States
    public Bundle mainActivityExtras = null;
	
    // Set para type
    public final static int PARAM_AUDIO = 1;
    public final static int PARAM_VIDEO = 2;
    public final static int PARAM_SCREEN = 3;
    public final static int PARAM_QOS = 4;

    private Activity mCurrentActivity = null;
    public final static String COMMAND_LINE = "com.wx2.cmdline";
    private String BACKDOOR_TAG = "backdoor";
    public final static String LOOPBACK = "com.wx2.loopback";
    public final static String CALLIOPE = "com.wx2.calliope";
    public final static String LINUS_ADDRESS = "com.wx2.linus";
    public final static String WS_ADDRESS = "com.wx2.ws";
    public final static String BACKDOOR = "com.wx2.backdoor";
    public final static String HASSHARING = "com.wx2.hassharing";
    private boolean mBackdoorLoopback = false;
    private static Context context;
    private LogFilePrint logFile;
    private Camera camera = null;
    private long mediaSessionRetCode = 0;
    private boolean printStatisticsLog = true;
    private final static int DEFAULT_MARI_FEC_CLOCKRATE = 8000;
    private final static int DEFAULT_MARI_FEC_PAYLOADTYPE = 111;
    public boolean m_bHWCodec = false;
    public boolean m_bScreenHWCodec = false;
    public boolean m_bEnableSimulcast = true;
    public boolean m_bDisable90PVideo = true;
    public boolean m_bEnableCVO = false;
    public boolean m_bEnableAaec = false;
    public long m_nVideoDumpFlag = 0;
    public int m_uMaxVideoStreams = 1;
    public int m_uMaxAudioStreams = 3;
    public static MediaConfig.WmeConnectionPolicy m_nIcePolicy = MediaConfig.WmeConnectionPolicy.UDPFallToTCP;
    public String mHWVideoSetting = "{\"video\": { "
			+ "\"mediaCodec\":true,"
			+ "\"yv12Capture\":false"
			+ "}}";

    public static String USER_ID;
    public static String USER_PASSWORD;
    public static String mProxyUserName;
    public static String mProxyPasswd;

    public void onCreate() {
        super.onCreate();
        MyApplication.context = getApplicationContext();
        logFile = new LogFilePrint(MyApplication.context);
        logFile.startCapture();
        createMiniDumpPath();
        resetParam();
    }

    public static Context getAppContext() {
        return MyApplication.context;
    }
    
    public boolean bPrintStatisticsLog(){
    	return printStatisticsLog;
    }
    
    public void switchToCallActivity(boolean bLoopback, String linusServer,
            boolean bCalliope, String wsServer, boolean bBackdoor, boolean bEnableAS) {
        Log.i("MediaSession", "switchToCallActivity, bLoopback=" + bLoopback
                + ", linusServer=" + linusServer + ", bCalliope=" + bCalliope
                + ", wsServer=" + wsServer + ", bBackdoor" + bBackdoor + ", enableAS=" + bEnableAS);
        Intent intent = new Intent(mCurrentActivity, CallActivityJava.class);
        intent.putExtra(LOOPBACK, String.valueOf(bLoopback));
        intent.putExtra(LINUS_ADDRESS, linusServer);
        intent.putExtra(CALLIOPE, String.valueOf(bCalliope));
        intent.putExtra(WS_ADDRESS, wsServer);
        intent.putExtra(BACKDOOR, String.valueOf(bBackdoor));
        intent.putExtra(HASSHARING, String.valueOf(bEnableAS));
        m_bHasShare = bEnableAS;
        mCurrentActivity.startActivity(intent);
    }

    /*public void switchToCallActivity(String cmd) {
        Intent intent = new Intent(mCurrentActivity, CallActivity.class);
        intent.putExtra(COMMAND_LINE, cmd);
        mCurrentActivity.startActivity(intent);
    }*/

    public void switchToMainActivity() {
        Intent intent = new Intent(mCurrentActivity, MainActivity.class);
        mCurrentActivity.startActivity(intent);
    }

    public void setCurrentActivity(Activity mCurrentActivity) {
        this.mCurrentActivity = mCurrentActivity;
    }
    
    public void setMSLastRetErrorCode(long retCode){
    	Log.i("MyApplication", "setMSLastRetErrorCode, retCode="+retCode);
    	mediaSessionRetCode = retCode;
    }

    public String testBackdoor(String arg) {
        Log.i("backdoor", "testBackdoor has been called.");
        return "This is echo: " + arg;
    }

    public String getIPAddress(String arg) {
        android.util.Log.i(BACKDOOR_TAG, "Cucumber request ip address");
        WifiManager wifiMgr = (WifiManager) getSystemService(WIFI_SERVICE);
        WifiInfo wifiInfo = wifiMgr.getConnectionInfo();
        final ByteBuffer byteBuffer = ByteBuffer.allocate(4);
        byteBuffer.order(ByteOrder.LITTLE_ENDIAN);
        byteBuffer.putInt(wifiInfo.getIpAddress());
        InetAddress address = null;
        try {
            address = InetAddress.getByAddress(null, byteBuffer.array());
        } catch (UnknownHostException e) {
            // TODO Auto-generated catch block
            Log.e(BACKDOOR_TAG, e.toString());
            return "0.0.0.0";
        }
        return address.getHostAddress();
    }
    
    private String m_sdpOffer = null;
    private JSONObject mParams = null;
    public boolean m_bHasVideo = true;
    public boolean m_bHasAudio = true;
    public boolean m_bHasShare = false;
    public boolean mInitSharingContext = false;
    public boolean mIsViewer = true;
    public boolean mEnableCMulti = true;
    public boolean mFileMode = false;
    
    private void UpdateFileCapture(JSONObject obj) {
        mFileMode = obj.optBoolean("filemode");
        boolean isAudioLoop = false, isVideoLoop = false;
        boolean isVideoTimestamp = false;
        String audioSrcStr = "/sdcard/wmetest/audio_1_8000_16.pcm";
        String audioDstStr = "audio_1_8000_16.pcm";
        String videoSrcStr = "";
        String videoDstStr = "";
        
        if (mFileMode) {
        	isAudioLoop = isVideoLoop = obj.optBoolean("isLoopFile");
        	isVideoTimestamp = obj.optBoolean("timestamp");
            int resolution = 2;
            if (obj.has("resolution")) {
                resolution = obj.optInt("resolution");
            }
            if (0 == resolution) {
                videoSrcStr = "/sdcard/wmetest/video_160_90_6_i420.yuv";
                videoDstStr = "video_160_90_6_i420.yuv";
            } else if (1 == resolution) {
                videoSrcStr = "/sdcard/wmetest/video_320_180_12_i420.yuv";
                videoDstStr = "video_320_180_12_i420.yuv";
            } else if (2 == resolution) {
                videoSrcStr = "/sdcard/wmetest/video_640_360_24_i420.yuv";
                videoDstStr = "video_640_360_24_i420.yuv";
            } else {
                videoSrcStr = "/sdcard/wmetest/video_1280_720_30_i420.yuv";
                videoDstStr = "video_1280_720_30_i420.yuv";
            }
            
            String audioSrcPath = obj.optString("audiosouce_file");
            if(audioSrcPath != null && !audioSrcPath.isEmpty())
            	audioSrcStr = "/sdcard/wmetest/" + audioSrcPath;
            
            String videoSrcPath = obj.optString("videosouce_file");
            if(videoSrcPath != null && !videoSrcPath.isEmpty())
            	videoSrcStr = "/sdcard/wmetest/" + videoSrcPath;
        }
        if(obj.has("files")) {
        	JSONObject files = obj.optJSONObject("files");
        	JSONObject audioFiles = files.optJSONObject("audio");
        	if(audioFiles != null && audioFiles.has("source")) {
        		String src_path = audioFiles.optString("source");
        		if(!src_path.isEmpty()) {
        			audioSrcStr = "/sdcard/wmetest/" + src_path;
        			mFileMode = true;
        		}
        		isAudioLoop = audioFiles.optBoolean("loop");
        	}
        	JSONObject videoFiles = files.optJSONObject("video");
        	if(videoFiles != null && videoFiles.has("source")) {
        		String src_path = videoFiles.optString("source");
        		if(!src_path.isEmpty()) {
        			videoSrcStr = "/sdcard/wmetest/" + src_path;
        			mFileMode = true;
        		}
        		isVideoTimestamp = videoFiles.optBoolean("timestamp");
        		isVideoLoop = videoFiles.optBoolean("loop");
        	}
        }
        if(mFileMode) {
            File folder = new File("/sdcard/wmetest/dst");
            folder.mkdirs();

            File audioDst = new File(folder, audioDstStr);
            File videoDst = new File(folder, videoDstStr);
            if (mBackdoorLoopback) {
                Loopback.instance().audioSrc = audioSrcStr;
                Loopback.instance().videoSrc = videoSrcStr;
                Loopback.instance().audioDst = audioDst.getPath();
                Loopback.instance().videoDst = videoDst.getPath();
                Loopback.instance().mIsAudioLoopFile = isAudioLoop;
                Loopback.instance().mIsVideoLoopFile = isVideoLoop;
                Loopback.instance().setIsTsMode(isVideoTimestamp);
            } else {
                PeerCall.instance().audioSrc = audioSrcStr;
                PeerCall.instance().videoSrc = videoSrcStr;
                PeerCall.instance().audioDst = audioDst.getPath();
                PeerCall.instance().videoDst = videoDst.getPath();
                PeerCall.instance().mIsAudioLoopFile = isAudioLoop;
                PeerCall.instance().mIsVideoLoopFile = isVideoLoop;
                PeerCall.instance().setIsTsMode(isVideoTimestamp);
            }
        }
    }
    
    /*
     * { "loopback": false "linus": "http://173.39.168.98:5000/" "filemode":
     * false }
     */
    public String backdoorStartCall(String arg) {
        Log.v("MediaSession", "backdoorStartCall(), arg = " + arg);
    	printStatisticsLog = true;
        JSONObject obj = null;
        m_sdpOffer = null;
        try {
            obj = new JSONObject(arg);
            if(obj.has("sdp")){
            	m_sdpOffer = obj.getString("sdp");
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        mediaSessionRetCode = 0;

        mBackdoorLoopback = obj.optBoolean("loopback");
        boolean isP2P = obj.optBoolean("p2p");
        String linus = obj.optString("linus");
        String overrideIP = obj.optString("overrideip");
        JSONObject ports = (JSONObject) obj.opt("overrideports");
        m_uMaxVideoStreams = obj.optInt("videoStreams");
        try {
            m_uMaxAudioStreams = obj.getInt("audiocount");
        } catch (JSONException e) {
            e.printStackTrace();
        }

        boolean isIce = true;
        if(obj.has("ice"))
			isIce = obj.optBoolean("ice");
       	Log.v("MediaSession", "isIce=" + isIce);
        boolean isSrtp = obj.optBoolean("srtp");
        boolean isDtlsSrtp = obj.optBoolean("dtls_srtp");
        boolean isRtcpMux = true;
        JSONObject options = (JSONObject)obj.opt("option");
        if(options != null){
        	m_bHasVideo = options.optBoolean("video");
        	m_bHasAudio = options.optBoolean("audio");
            m_bEnableAaec = options.optBoolean("enable_aaec");
            String share_option = options.optString("share");
        	m_bHasShare = !share_option.isEmpty();
        	mIsViewer = (share_option != "sharer");
        }

        boolean enableQos = true;
        if (obj.has("enableQos")) {
            enableQos = obj.optBoolean("enableQos");
        } else {
            enableQos = true;
        }

        if(obj.has("mux"))
        	isRtcpMux = obj.optBoolean("mux");
        boolean isMute = obj.optBoolean("mute");
        
        UpdateFileCapture(obj);
        
        if(obj.has("params"))
        {
        	JSONObject videoParams = obj.optJSONObject("params");
            Iterator<?> it = videoParams.keys();
            while (it.hasNext()) {
                String key = (String) it.next();
                try {
                    mParams.put(key, videoParams.get(key));
                } catch (JSONException e) {
                    e.printStackTrace();
                }
            }
        }
        
        MediaConfig.WmePerformanceDumpType performanceDumpType = MediaConfig.WmePerformanceDumpType.WmePerformanceDumpAll;
        if (obj.has("enablePerformanceStatDump")) {
            String type = obj.optString("enablePerformanceStatDump");
            if (type != null) {
                performanceDumpType = MediaConfig.ConvPerformanceDumpType(type);
            }
        }
        try {
        	JSONObject audioParam = (JSONObject)mParams.optJSONObject("audio");
        	if(audioParam == null){
        		mParams.put("audio", audioParam = new JSONObject());
        	}
        	audioParam.put("supportCmulti", true); //obj.optBoolean("multilayer"));

            JSONObject videoParam = (JSONObject)mParams.optJSONObject("video");
            if(videoParam == null){
                mParams.put("video", videoParam = new JSONObject());
            }
            videoParam.put("bLimitNalSize", obj.optBoolean("limitNalSize"));

            setHWCodec();

		} catch (JSONException e) {
			e.printStackTrace();
		}

        NativeMediaSession.dummy();

        if (mBackdoorLoopback) {
        	Loopback.instance().enableIce(isIce);
        	Loopback.instance().setMuteOption(isMute);
            Loopback.instance().enableQos(enableQos);
            Loopback.instance().setPerformanceDumpType(performanceDumpType);
			boolean bCalliope = isP2P ? false : true;
            switchToCallActivity(true, linus, bCalliope, "", false, m_bHasShare);
        } else {
            PeerCall.instance().enableIce(isIce);
            PeerCall.instance().enableSrtp(isSrtp);
            PeerCall.instance().enableDtlsSrtp(isDtlsSrtp);
            PeerCall.instance().enableRtcpMux(isRtcpMux);
            PeerCall.instance().setMuteOption(isMute);
            PeerCall.instance().enableQos(enableQos);
            PeerCall.instance().setPerformanceDumpType(performanceDumpType);
            int audioPort = 0, videoPort = 0, sharingPort = 0;
            if(ports != null){
            	audioPort = ports.optInt("audio");
            	videoPort = ports.optInt("video");
            	sharingPort = ports.optInt("sharing");
            }
            if(overrideIP != null && !overrideIP.isEmpty())
            	PeerCall.instance().setOverrideIPPort(overrideIP, audioPort, videoPort, sharingPort);
            switchToCallActivity(false, "", false, "", true, m_bHasShare);
        }
        return "done";
    }
                

    public String backdoorSetFecParam(String arg) {
        Log.v("MediaSession", "backdoorSetFecParam(), arg = " + arg);
        
        try {
            JSONObject obj = new JSONObject(arg);
            if(obj.has("rsfec")){ 
                int sessionType = -1;
                String sessName = "";
                JSONObject rsfec = (JSONObject) obj.opt("rsfec");

                if (rsfec.has("uSessionType")) {
                    sessionType = rsfec.optInt("uSessionType");
                    if (0 == sessionType) {
                        sessName = "audio";
                    } else if (1 == sessionType) {
                        sessName = "video";
                    } else if (2 == sessionType) {
                        sessName = "share";
                    } else {
                        return "fail";
                    }
                }
                    
                JSONObject sessParam = (JSONObject)mParams.opt(sessName);
                if (sessParam == null) {
                    sessParam = new JSONObject();
                }
                JSONObject fecParams = (JSONObject)sessParam.opt("fecParams");
                if (fecParams == null) {
                    fecParams = new JSONObject();
                }

                int fecPayloadType = 0;
                int fecClockRate = 0;
                boolean fecEnable = false;
                String fecOrder = "ORDER_SRTP_FEC";
                if (rsfec.has("bEnableFec")) {
                    fecEnable = rsfec.optBoolean("bEnableFec");
                    fecParams.put("bEnableFec", fecEnable);
                }
                if (rsfec.has("uClockRate")) {
                    fecClockRate = rsfec.optInt("uClockRate");
                    fecParams.put("uClockRate", fecClockRate);
                }
                if (rsfec.has("uPayloadType")) {
                    fecPayloadType = rsfec.optInt("uPayloadType");
                    fecParams.put("uPayloadType", fecPayloadType);
                }
                if (rsfec.has("fecFmtInfo")) {
                    JSONObject fecFmtInfo;
                    if (fecParams.has("fecFmtInfo")) {
                        fecFmtInfo = (JSONObject)fecParams.opt("fecFmtInfo");
                    } else {
                        fecFmtInfo = new JSONObject();
                    }
                    JSONObject tmpFmtInfo = (JSONObject)rsfec.opt("fecFmtInfo");
                    if (tmpFmtInfo.has("srtpFecOrder")) {
                        fecFmtInfo.put("srtpFecOrder", tmpFmtInfo.optString("srtpFecOrder"));
                    }
                    fecParams.put("fecFmtInfo", fecFmtInfo);
                }

                sessParam.put("fecParams", fecParams);
                mParams.put(sessName, sessParam);
                Log.v("MediaSession", "backdoorSetFecParam(), merge params, mParams = " + mParams);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
        return "done";
    }

    public String backdoorSetFeatureToggles(String arg) {
        Log.v("MediaSession", "backdoorSetFeatureToggles(), arg = " + arg);
        if (mBackdoorLoopback) {
            Loopback.instance().setFeatureToggles(arg);
        } else {
            PeerCall.instance().setFeatureToggles(arg);
        }
        return "done";
    }

    public String backdoorGetFeatureToggles(String arg) {
        Log.v("MediaSession", "backdoorGetFeatureToggles(), arg = " + arg);
        String res;
        JSONObject ret = new JSONObject();
        if (mBackdoorLoopback) {
            res = Loopback.instance().getFeatureToggles();
        } else {
            res = PeerCall.instance().getFeatureToggles();
        }
        
        Log.i("backdoor", "backdoorGetFeatureToggles:" + res);
        return res;
    }

    public String backdoorGetLocalSdp(String arg) {
        String sdp = PeerCall.instance().getsLocalSDP();
        return sdp;
    }
    
    public String backdoorSetRemoteSdp(String arg) {
        PeerCall.instance().answerReceived(arg);
        return "done";
    }

    public String backdoorSwitchCamera(String args){
        	long res = 0;
        	if(mBackdoorLoopback){
        		if(args.equals("front")){
        			res = Loopback.instance().switchCamera(CameraType.Front);
        		}else if(args.equals("back"))
        		{
        			res = Loopback.instance().switchCamera(CameraType.Back);
        		}
        	}
        	if(0 != res )
        		return "false";
        	return "true";
        }
    
    public String backdoorGetStatistics(String arg) {
        JSONObject audio, video, share;
        VideoStats[] remoteTracks = null;
        if (mBackdoorLoopback) {
            audio = Loopback.instance().getAudioStats().to_json();
            video = Loopback.instance().getVideoStats().to_json();
            share = Loopback.instance().getSharingStats().to_json();
        } else {
            audio = PeerCall.instance().getAudioStats().to_json();
            video = PeerCall.instance().getVideoStats().to_json();
            share = PeerCall.instance().getSharingStats().to_json();
            remoteTracks = PeerCall.instance().getVideoTrackStats();
        }
        
        JSONArray videoTracks = new JSONArray();
        if(remoteTracks != null) {
        	for(VideoStats stat : remoteTracks) {
        		if(stat == null)
        			continue;
        		videoTracks.put(MediaStatistics.to_json(stat));
        	}
        }

        JSONObject merged = new JSONObject();
        JSONObject[] objs = new JSONObject[] { audio, video, share };
        for (JSONObject obj : objs) {
            Iterator<?> it = obj.keys();
            while (it.hasNext()) {
                String key = (String) it.next();
                try {
                    merged.put(key, obj.get(key));
                } catch (JSONException e) {
                    e.printStackTrace();
                }
            }
        }
        if(remoteTracks != null) {
            try {
            	merged.put("videoTracks", videoTracks);
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }
        String sRet = merged.toString();
        Log.i("backdoor", "backdoorGetStats:" + sRet);
        return sRet;
    }
    
    public String backdoorGetNetworkMetrics(String arg) {
        JSONObject networkMetrics;
        if (mBackdoorLoopback) {
            networkMetrics = Loopback.instance().getNetworkMetrics().to_json();
        } else {
            networkMetrics = PeerCall.instance().getNetworkMetrics().to_json();
        }
        
        JSONObject result = new JSONObject();
        try {
            result.put("qos", networkMetrics);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        
        String sRet = result.toString();
        Log.i("backdoor", "backdoorGetNetworkMetrics:" + sRet);
        return sRet;
    }

    public void resetParam() {
        mParams = new JSONObject();
	mFileMode = false;
        JSONArray array = new JSONArray();
        JSONObject video = new JSONObject();
        try {
            array.put(0, "H264");
            video.put("selectedCodec", array);
            mParams.put("video", video);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }
    
    public String backdoorStopCall(String arg) {
    	String ret = "yyy";
    	printStatisticsLog = false;
        resetParam();
        if (mBackdoorLoopback)
        	ret = Loopback.instance().stopCall();
        else
        	ret = PeerCall.instance().stopCall();
        NativeClickCall.doGCov(1);
        return ret;
    }

    public String backdoorOutputTrace(String args){
    	JSONObject obj = null;
        try {
            obj = new JSONObject(args);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        Log.i("backdoor", "TA case Trace:" + obj.optString("trace_info") 
        		+ ",DeviceModelName=" + Build.MANUFACTURER +"-"+Build.MODEL
        		+ ",SystemOSVersion="+ Build.VERSION.RELEASE + "-" + Build.VERSION.SDK_INT);
        
    	return "";
    }
    public String backdoorChangeWifi(String arg) {
        boolean b = (arg.equals("enable"));
        WifiManager wfm = (WifiManager) getSystemService(Context.WIFI_SERVICE);
        try {
            boolean ret = wfm.setWifiEnabled(b);
            Log.i("backdoor", "backdoorChangeWifi:" + b + ret);
        } catch (Exception e) {
        }
        return "done";
    }

    public String backdoorIsWifiEnabled(String arg) {

        ConnectivityManager connManager = (ConnectivityManager) getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo mWifi = connManager
                .getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        boolean ret = mWifi.isConnected();
        Log.i("backdoor", "backdoorIsWifiEnabled:" + ret);
        return ret ? "enabled" : "disabled";
    }

    public String backdoorIsFileCaptureEnded(String arg) {
        boolean ret = false;
        if(arg == null || arg.isEmpty() || arg.equals("all")) {
            if (mBackdoorLoopback) {
                ret = Loopback.instance().mCaller.m_audioFileEnded
                        && Loopback.instance().mCaller.m_videoFileEnded;
            }else {
            	ret = PeerCall.instance().mCall.m_audioFileEnded 
            			&& PeerCall.instance().mCall.m_videoFileEnded;
            }
        }else if(arg.equals("audio")) {
            if (mBackdoorLoopback) {
                ret = Loopback.instance().mCaller.m_audioFileEnded;
            }else {
            	ret = PeerCall.instance().mCall.m_audioFileEnded;
            }
        }else if(arg.equals("video")) {
            if (mBackdoorLoopback) {
                ret = Loopback.instance().mCaller.m_videoFileEnded;
            }else {
            	ret = PeerCall.instance().mCall.m_videoFileEnded;
            }
        }
        return ret ? "ended" : "playing";
    }

    public String backdoorSetDefaultOrientationPort(String arg) {
        WseEngine.setDisplayRotation(Surface.ROTATION_0);
        mCurrentActivity
                .setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
        return "done";
    }

    public String backdoorSetDefaultOrientationLand(String arg) {
        WseEngine.setDisplayRotation(Surface.ROTATION_90);
        mCurrentActivity
                .setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        return "done";
    }

    public String backdoorRotateCamera(String arg) {
        Display display = mCurrentActivity.getWindowManager()
                .getDefaultDisplay();
        int orientation = display.getRotation();
        switch (orientation) {
        case 1:
            WseEngine.setDisplayRotation(Surface.ROTATION_0);
            mCurrentActivity
                    .setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
            break;
        case 0:
            WseEngine.setDisplayRotation(Surface.ROTATION_90);
            mCurrentActivity
                    .setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
            break;
        }

        return "done";

    }

    public String backdoorRotateWsePort (String arg) {
        WseEngine.setDisplayRotation(Surface.ROTATION_0);
        return "done";
    }

    public String backdoorRotateWseLand (String arg) {
        WseEngine.setDisplayRotation(Surface.ROTATION_90);
        return "done";
    }


    public String backdoorGetScalingMode(String arg) {
        String res = null;

        // PeerCall support only
        if (!mBackdoorLoopback) {
            if (PeerCall.instance().getScalingMode() == ScalingMode.CropFill) {
                res = "CropFill";
            } else {
                res = "LetterBox";
            }
        }
        return res;
    }

    public String backdoorSetParam(String arg) {
        if (null == arg || arg.length() < 3) {
            Log.e(BACKDOOR_TAG, "backdoorSetParam received invalid param");
            return "";
        }

        // Extract param type
        int paramType = Integer.parseInt(arg.substring(0, 1));
        Log.i(BACKDOOR_TAG, "paramType=" + paramType);
        String param = arg.substring(2);
        Log.i(BACKDOOR_TAG, "param=" + param);
        switch (paramType) {
        case PARAM_AUDIO: {
            backdoorSetAudioParam(param);
            return "done";
        }
        case PARAM_VIDEO: {
            backdoorSetVideoParam(param);
            return "done";
        }
        case PARAM_SCREEN: {
            backdoorSetScreenParam(param);
            return "done";
        }
        case PARAM_QOS: {
            backdoorSetAudioParam(param);
            backdoorSetVideoParam(param);
            return "done";
        }
        default: {
            return "";
        }
        }
    }

    public String backdoorSetAudioParam(String args) {
        if (args == null)
            return "fail";
        if (!mBackdoorLoopback) {
            try {
                JSONObject jObject = new JSONObject(args);
                PeerCall.instance().setParam(MediaType.Audio, jObject);
            } catch (JSONException e) {
                e.printStackTrace();
            }
        } else {
            try {
                JSONObject jObject = new JSONObject(args);
                Loopback.instance().setParam(MediaType.Audio, jObject);
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }
        return "done";
    }
    
    public String backdoorSetVideoParam(String args) {
        if (args == null)
            return "fail";
        if (!mBackdoorLoopback) {
            try {
                JSONObject jObject = new JSONObject(args);
                PeerCall.instance().setParam(MediaType.Video, jObject);
            } catch (JSONException e) {
                e.printStackTrace();
            }
        } else {
            try {
                JSONObject jObject = new JSONObject(args);
                Loopback.instance().setParam(MediaType.Video, jObject);
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }
        return "done";
    }

    public String backdoorSetScreenParam(String args) {
        if (args == null)
            return "fail";
        if (!mBackdoorLoopback) {
            try {
                JSONObject jObject = new JSONObject(args);
                PeerCall.instance().setParam(MediaType.Sharing, jObject);
            } catch (JSONException e) {
                e.printStackTrace();
            }
        }
        return "done";
    }
    
    public static MediaType ofSessionType(String mytype){
    	MediaType sessType = MediaType.Audio;
    	if (mytype.equals("audio"))
    		sessType =  MediaType.Audio;
    	else if (mytype.equals("video"))
    		sessType =  MediaType.Video;
    	else if (mytype.equals("sharing"))
    		sessType =  MediaType.Sharing;
    	return sessType;
    }
    
    public String backdoorStartStopTrack(String arg)
    {
    	JSONObject jObject = null;
        try {
        	jObject = new JSONObject(arg);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        
        String sType = jObject.optString("type");
    	MediaType sessType = ofSessionType(sType);
    	boolean bStart = jObject.optBoolean("start");
    	MediaDirection direction = MediaDirection.SendOnly;
    	if(jObject.optBoolean("remote"))
    		direction = MediaDirection.RecvOnly;
        
    	long ret = -80;
    	if (mBackdoorLoopback){
    		ret = Loopback.instance().startStopTrack(sessType, direction, bStart);
        }
    	else{
    		ret = PeerCall.instance().startStopTrack(sessType, direction, bStart);
        }   	

    	return "" + ret;
    }
    
    public String backdoorMuteUnMute(String arg)
    {
    	JSONObject jObject = null;
        try {
        	jObject = new JSONObject(arg);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    	String mytype = jObject.optString("type");

    	MediaType sessType = ofSessionType(mytype);
    	
    	boolean bMute = jObject.optBoolean("mute");
    	boolean bSpeaker = jObject.optBoolean("speaker");
    	Log.i("backdoor","backdoorMuteUnMute, loopback call="+mBackdoorLoopback+", sessType=" + sessType + ", bMute="+bMute+", bSpeaker"+bSpeaker);
		
    	if (mBackdoorLoopback){
    		Loopback.instance().mute(sessType, bMute, bSpeaker);
        }
    	else{
            PeerCall.instance().mute(sessType, bMute, bSpeaker);
        }   	

    	return "";
    }

    public String backdoorSetRenderModePort(String arg) {
        if (!mBackdoorLoopback) {
            PeerCall.instance().setRemoteRenderMode(
                    Configuration.ORIENTATION_PORTRAIT);
        }

        return "done";
    }

    public String backdoorSetRenderModeLand(String arg) {
        if (!mBackdoorLoopback) {
            PeerCall.instance().setRemoteRenderMode(
                    Configuration.ORIENTATION_LANDSCAPE);
        }

        return "done";
    }

    public String backdoorCheckSDPType(String arg) {
        if (!mBackdoorLoopback) {
            return PeerCall.instance().getSDPType();
        }
        return "done";
    }

    public String backdoorIsConnected(String arg) {
        if (!mBackdoorLoopback) {
            if (PeerCall.instance().isConnected())
                return "true";
            else
                return "false";
        }
        return "done";
    }

    public String backdoorIsMediaBlocked(String arg) {
        if (!mBackdoorLoopback) {
            if (PeerCall.instance().isMediaBlocked())
                return "true";
            else
                return "false";
        }
        return "done";
    }
    
    public String backdoorGetCSICount(String arg) {
        if (!mBackdoorLoopback) {
            int count = PeerCall.instance().getCSICount();
            return Integer.toString(count);
        }
        return "0";
    }

    public String backdoorGetMediaStatus(String arg) {
        MediaType type = ofSessionType(arg);
        MediaStatus status = MediaStatus.Available;
        if (!mBackdoorLoopback) {
            status = PeerCall.instance().getMediaStatus(type);
        }
        else {
            status = Loopback.instance().getMediaStatus(type);
        }
        return (status == MediaStatus.ERR_TEMP_UNAVAIL_NO_MEDIA) ? "unavailable" : "available";
    }

    public String backdoorGetScreenParameters(String key) {
        if (!mBackdoorLoopback) {
            return PeerCall.instance().getParameters(MediaType.Sharing, key);
        }
        return "done";
    }

    public String backdoorGetVideoParameters(String key) {
        if (!mBackdoorLoopback) {
            return PeerCall.instance().getParameters(MediaType.Video, key);
        }
        return "done";
    }

    public String backdoorGetAudioParameters(String key) {
        if (!mBackdoorLoopback) {
            return PeerCall.instance().getParameters(MediaType.Audio, key);
        }
        return "done";
    }
    
    public String backdoorGetMSLastRetCode(String args) {
    	String retCode = "0x";
    	retCode = retCode.concat(Long.toString(mediaSessionRetCode, 16));
    	Log.i("MyApplication", "backdoorGetMSLastRetCode, mediaSessionRetCode="+retCode);
    	return retCode;
    }
    
    public String backdoorSetPlaybackDevice(String args) {
        if (!mBackdoorLoopback) {
            if (args.equals("Earpiece")) {
                PeerCall.instance().setAudioOutType(DeviceManager.AudioOutType.Earpiece);
            } else if (args.equals("Speaker")) {
                PeerCall.instance().setAudioOutType(DeviceManager.AudioOutType.Speaker);
            } else {
                return "false";
            }
            return "done";
        }
        return "false";
    }
    
    public String backdoorCheckSyncStatus(String args) {
    	boolean bCheckPass = false;
    	JSONObject jObject = null;
        try {
        	jObject = new JSONObject(args);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    	String result = jObject.optString("result");
    	int rate = jObject.optInt("rate");
    	if (mBackdoorLoopback) {
    		bCheckPass = Loopback.instance().checkSyncStatus(result, rate);
        } else {
        	bCheckPass = PeerCall.instance().checkSyncStatus(result, rate);
        }
        return bCheckPass ? "pass" : "false";
    }
    
    public String backdoorOpenCamera(String args) {
    	Log.i("MyApplication","backdoorOpenCamera");
    	camera = Camera.open();
        return "done";
    }
    
    public String backdoorCloseCamera(String args) {
    	Log.i("MyApplication","backdoorCloseCamera");
    	camera.release();
        return "done";
    }

    public void createMiniDumpPath() {
        String breakpadPath = "/sdcard/breakpad_dump";
        Log.i("createMiniDumpPath","path="+breakpadPath);
        File folder = new File(breakpadPath);
        folder.mkdirs();
        File[] childFiles = folder.listFiles();
        for (int i = 0; i < childFiles.length; i++) {
            childFiles[i].delete();
        }
        try {
            NativeMediaSession.class.getMethod("initMiniDump",String.class).invoke(null, breakpadPath);
        } catch(Exception e) {
            Log.e("createMiniDumpPath", " can't find method initMiniDump in object NativeMediaSession");
        }
    }

    public String backdoorCheckOnMediaReady(String args) {
        Calabash myCalabash = null;
        ArrayList<?> myList = null;
        int count = 0;

        if (mBackdoorLoopback) {
            // Loopback has special case: it instantiates 3 tracks total. The "Video In" track is not used for
            // loopback mode
            myCalabash = Loopback.instance().getCalabashCaller();
        } else {
            myCalabash = PeerCall.instance().getCalabash();
        }

        if (myCalabash != null) {
            myList = myCalabash.getOnMediaReadyInfoList();
        }

        for (int i = 0; i < myList.size(); i++) {
            Calabash.OnMediaReadyArgs object = (Calabash.OnMediaReadyArgs) myList.get(i);
            if (object.dir == MediaDirection.SendOnly
                    && object.type == MediaType.Audio) {
                count++;
                Log.i("ClickCall", "backdoorCheckOnMediaReady, Audio Out track generated, num of tracks total = " + count);
            }

            if (object.dir == MediaDirection.SendOnly
                    && object.type == MediaType.Video) {
                count++;
                Log.i("ClickCall", "backdoorCheckOnMediaReady, Video Out track generated, num of tracks total = " + count);
            }

            if (object.dir == MediaDirection.RecvOnly
                    && object.type == MediaType.Audio) {
                count++;
                Log.i("ClickCall", "backdoorCheckOnMediaReady, Audio In track generated, num of tracks total = " + count);
            }

            if (object.dir == MediaDirection.RecvOnly
                    && object.type == MediaType.Video) {
                count++;
                Log.i("ClickCall", "backdoorCheckOnMediaReady, Video In track generated, num of tracks total = " + count);
            }

            // After confluence, there should be four tracks opened: Audio In/Out, Video In/Out
            if (count >= 4) {
                return "pass";
            }
        }

        Log.i("ClickCall", "backdoorCheckOnMediaReady, Number of tracks generated after confluence is less than four! " +
                "Missing media tracks");
        return "fail";
    }
    
    public JSONObject getStartupParams(){
        return mParams;
    }

	public String getSdpOffer() {
		return m_sdpOffer;
	}

    public String backdoorSetSharingContext(String args) {
        try {
            JSONObject obj = new JSONObject(args);
            if( obj.has("loopback") )
                obj.getBoolean("loopback");
            if( obj.has("sharer") )
                obj.getBoolean("sharer");

        } catch (JSONException e) {
            e.printStackTrace();
        }

        return  "{}";
    }
    public String backdoorGetSharingStatus(String args) {
        //check current sharing status , it is "start" or "end"
        //need to refactoring EndPoint to support specific
        JSONObject obj = null;
        try {
            obj = new JSONObject(args);
            if(obj.has("action")){
                String strAction = obj.getString("action");
                if (strAction == "ShareSharingRunningStatus"){
                    obj = new JSONObject();
                    obj.put("ShareSharingRunningStatus", "end");
                    String ret = obj.toString();
                    Log.i("ClickCall", "backdoorGetSharingStatus:" + ret);
                    System.out.format("backdoorGetSharingStatus %s", ret);
                    return ret;
                }
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }

        obj = new JSONObject();
        try {
            obj.put("ShareSharingRuningStatus", "end");
        } catch (JSONException e) {
            e.printStackTrace();
        }
        String ret = obj.toString();
        Log.i("ClickCall", "backdoorGetSharingStatus" + ret);
        System.out.format("backdoorGetSharingStatus %s", ret);
        return ret;
    }
    
    public String backdoorGetSharingResult(String args) {
        if (!mBackdoorLoopback) {
            return PeerCall.instance().getSharingStatsJSON();
        }
        return "{}";
    }
    
    public String backdoorGetMemory(String args) {
        MemoryUsage memory;
        if (mBackdoorLoopback) {
            // loopback mode
            memory = Loopback.instance().getMemoryUsage();
        } else {
            memory = PeerCall.instance().getMemoryUsage();
        }
        return memory.to_json().toString();
    }
    public String backdoorSetRemoteOffer(String args){
    	if (mBackdoorLoopback) {
    		Loopback.instance().acceptCall(args, true);
        }
        else{
            PeerCall.instance().acceptCall(args, true);
        }
    	return "";
    }
    
    public String backdoorUpdateSdp(String args){
    	JSONObject obj = null;
        try {
            obj = new JSONObject(args);
        } catch (JSONException e) {
            e.printStackTrace();
        }
        
        String op = obj.optString("op");
        Log.v("MediaSession", "backdoorUpdateSdp, op=" + op + ", mBackdoorLoopback=" + mBackdoorLoopback);
        if(op.compareTo("hold") == 0 || op.compareToIgnoreCase("unhold") == 0)
        {
            boolean isHold = (op.compareToIgnoreCase("hold") == 0);
            Log.v("MediaSession", "holdCall, isHold=" + isHold);
        	if (mBackdoorLoopback) {
        		Loopback.instance().holdCall(isHold);
            }
            else{
                PeerCall.instance().holdCall(isHold);
                Log.v("MediaSession", "holdCall, isHold=" + isHold);
            }
        }
        
    	return "";
    }
    
    private void setHWCodec() {
        Log.v("MediaSession", "setHWCodec(), mParams = " + mParams);
        try {
            if (mParams.has("video")
                    && mParams.getJSONObject("video").has("bHWAcceleration")) {

                m_bHWCodec = mParams.getJSONObject("video").getBoolean("bHWAcceleration");
                Log.v("MediaSession", "setHWCodec(), m_bHWCodec = " + m_bHWCodec);
            }
        } catch (JSONException e) {
            e.printStackTrace();
        }
	}
    
    public String backdoorGetCSIChangeHistory(String arg) {
    	String ret;
    	if (mBackdoorLoopback) {
    		ret = Loopback.instance().mCaller.getCalabash().GetCSIChangeHistory();
        }
        else{
            ret = PeerCall.instance().mCall.getCalabash().GetCSIChangeHistory();
        }
    	return ret;
    }

	public String backdoorHWCheckDevice(String device) {
                Log.i("MyApplication", "backdoorHWCheckDevice, Build.MODEL = " + Build.MODEL + ", device = " + device);
		if (device.equals(Build.MODEL)) {
			return "true";
		} else {
			return "false";
		}
	}

	public String backdoorHWCheckAPILevel(String args) {
		int req = 19; // Given by MediaPerformance.cpp: #define
						// ANDROID_HW_ENABLE_API_LEVEL 19

		Log.i("MyApplication", "backdoorHWCheckAPILevel, Build.VERSION.SDK_INT = " + Build.VERSION.SDK_INT);
		if (Build.VERSION.SDK_INT >= req) {
			return "true";
		} else {
			return "false";
		}
	}

    public StunTraceSinkTA mStunTraceSink = null;
    public TraceServerSinkTA mTraceServerSink = null;

    public String backdoorGetStunTraceResult(String args) {
        if(mStunTraceSink != null && mStunTraceSink.json != null)
            return mStunTraceSink.json;
        else
            return "failed";
    }

    public String backdoorGetTraceServerResult(String args) {
        if(mTraceServerSink != null && mTraceServerSink.json != null)
            return mTraceServerSink.json;
        else
            return "failed";
    }

    public String backdoorConfig(String args) {
        JSONObject objRoot = null;
        try {
            objRoot = new JSONObject(args);
            if (objRoot.has("video_config")) {
                JSONObject obj_video_config = objRoot.getJSONObject("video_config");
                if(obj_video_config.has("video_capture_feed_file")){
                    String strVideoCaptureFeedFile = obj_video_config.getString("video_capture_feed_file");
                    if(strVideoCaptureFeedFile.length()>0){
                        NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_FeedVideoCaptureData_file",strVideoCaptureFeedFile);
                    }
                }

                if(obj_video_config.has("video_size")){
                    String szVideoSize = obj_video_config.getString("video_size");
                    if(szVideoSize.length()>0){
                        NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_VIDEO_SIZE",szVideoSize);
                    }
                }
            }

            if (objRoot.has("audio_config")) {
                JSONObject obj_audio_config = objRoot.getJSONObject("audio_config");
                if(obj_audio_config.has("audio_capture_feed_file")){
                    String strAudioCaptureFeedFile = obj_audio_config.getString("audio_capture_feed_file");
                    if(strAudioCaptureFeedFile.length()>0){
                        NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_FeedSource_Enable","1");
                        NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_FeedSource_Format","0");
                        NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_CaptureReplaceFileName",strAudioCaptureFeedFile);
                        NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_CaptureReplaceCircleEnable","1");
                    }
                }
            }

        } catch (JSONException e) {
            e.printStackTrace();
        }
        return "";
    }

    public String backdoorGetAssertionsCount(String args) {
        return String.valueOf(NativeMediaSession.getAssertionsCount());
    }

    public String backdoorSubscribe(String args)
    {
        Log.i("clickcall", "backdoorSubscribe with param:" + args);
        JSONObject objRoot = null;
        try {
            objRoot = new JSONObject(args);
            MediaTrack.VideoQualityType resolution = MediaTrack.VideoQualityType.of(objRoot.optInt("resolution"));
            boolean newTrack = objRoot.optBoolean("newTrack");
            boolean unsubscribe = objRoot.optBoolean("unsubscribe");

            if(newTrack)
            {
                m_uMaxVideoStreams++;
            }

            if (mBackdoorLoopback){
                if(Loopback.instance().mCallee != null){
                    if(unsubscribe)
                        Loopback.instance().mCallee.unsubscribeVideo(1);
                    else
                        Loopback.instance().mCallee.subscribeVideo(resolution, newTrack, MediaConfig.WmeSubscribePolicy.ActiveSpeaker, 0);
                }
            }
            else if (PeerCall.instance().mCall != null){
                if(unsubscribe)
                    PeerCall.instance().mCall.unsubscribeVideo(1);
                else
                    PeerCall.instance().mCall.subscribeVideo(resolution, newTrack, MediaConfig.WmeSubscribePolicy.ActiveSpeaker, 0);
            }

        } catch (JSONException e) {
            e.printStackTrace();
        }

        return "";
    }
}

