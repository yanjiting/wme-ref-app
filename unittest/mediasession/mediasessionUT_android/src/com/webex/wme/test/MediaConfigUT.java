package com.webex.wme.test;

import junit.framework.TestCase;
import com.webex.wme.MediaConnection;
import com.webex.wme.MediaConfig.GlobalConfig;
import com.webex.wme.MediaConfig.AudioConfig;
import com.webex.wme.MediaConfig.VideoConfig;
import com.webex.wme.MediaConfig.ShareConfig;
import com.webex.wme.MediaConfig.WmeCodecType;
import com.webex.wme.MediaConfig.WmeConnectionPolicy;
import com.webex.wme.MediaConfig.WmePacketizationMode;
import com.webex.wme.MediaConfig.WmeNetworkStatus;
import com.webex.wme.MediaConfig.WmeNetworkDirection;
import com.webex.wme.MediaConfig.WmeVideoCodecCapability;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;


public class MediaConfigUT extends TestCase {
	private MediaConnection m_mediaConn;
	private GlobalConfig m_globalConfig;
	private AudioConfig m_audioConfig;
	private VideoConfig m_videoConfig;
	private ShareConfig m_shareConfig;

	public MediaConfigUT(String name) {
		super(name);
	}

	public static class MyMediaConnection extends MediaConnection{
		private JSONObject m_params = new JSONObject();
		public long setParameters(long mid, String input){
			Log.v("MediaConfigUT", "setParameters: input="+input);
			try{
				m_params = new JSONObject(input);
			} catch (JSONException e) {
				e.printStackTrace();
			}
			return 0;
		}
		public String getParameters(long mid){
			return m_params.toString();
		}
	}
	protected void setUp() throws Exception {
		super.setUp();
		m_mediaConn = new MyMediaConnection();
		m_globalConfig = new GlobalConfig(m_mediaConn);
		m_audioConfig = new AudioConfig(m_mediaConn, 0);
		m_videoConfig = new VideoConfig(m_mediaConn, 1);
		m_shareConfig = new ShareConfig(m_mediaConn, 2);
	}
	

	//-----Test BaseConfig-----	
	public void testSetMaxBandwidth() {
		Log.v("MediaConfigUT", "testSetMaxBandwidth");
		long value = 1;
		long cret = m_audioConfig.SetMaxBandwidth(value);
		assertTrue(cret==0);
		long result = m_audioConfig.GetMaxBandwidth();
		assertTrue(result == value);
	}
	
	public void testSetInitBandwidth() {		
		Log.v("MediaConfigUT", "testSetInitBandwidth");
		long value = 1;
		long cret = m_audioConfig.SetInitBandwidth(value);
		assertTrue(cret==0);
		long result = m_audioConfig.GetInitBandwidth();
		assertTrue(result == value);
	}
	
	public void testSetPreferedCodec() {
		Log.v("MediaConfigUT", "testSetPreferedCodec");
		WmeCodecType value = WmeCodecType.WmeCodecType_OPUS;
		long cret = m_audioConfig.SetPreferedCodec(value);
		assertTrue(cret==0);
		WmeCodecType result = m_audioConfig.GetPreferedCodec();
		assertTrue(result == value);
	}

	public void testSetSelectedCodec() {
		Log.v("MediaConfigUT", "testSetSelectedCodec");
		WmeCodecType value = WmeCodecType.WmeCodecType_OPUS;
		long cret = m_audioConfig.SetSelectedCodec(value);
		assertTrue(cret==0);
		WmeCodecType result = m_audioConfig.GetSelectedCodec();
		assertTrue(result == value);
	}
	
	public void testOverrideMediaIPAddress() {
		Log.v("MediaConfigUT", "testOverrideMediaIPAddress");
		String ipAddr = "127.0.0.1";
		long port = 1234;
		long cret = m_audioConfig.OverrideMediaIPAddress(ipAddr, port);
		assertTrue(cret==0);
	}
	
	public void testEnableFileRender() {
		Log.v("MediaConfigUT", "testEnableFileRender");
		String path = "";
		long cret = m_audioConfig.EnableFileRender(path);
		assertTrue(cret==0);
	}
	
	public void testEnableFileCapture() {
		Log.v("MediaConfigUT", "testEnableFileCapture");
		String path = "";
		boolean isLoop = true;
		long cret = m_audioConfig.EnableFileCapture(path, isLoop);
		assertTrue(cret==0);
	}
	
	public void testSetIcePolicy() {
		Log.v("MediaConfigUT", "testSetIcePolicy");
		WmeConnectionPolicy value = WmeConnectionPolicy.UDPOnly;
		long cret = m_audioConfig.SetIcePolicy(value);
		assertTrue(cret==0);
		WmeConnectionPolicy result = m_audioConfig.GetIcePolicy();
		assertTrue(result == value);
	}
	

    //-----Test AudioConfig-----	
	public void testEnableAGC() {
		Log.v("MediaConfigUT", "testEnableAGC");
		boolean enabled = true;
		long cret = m_audioConfig.EnableAGC(enabled);
		assertTrue(cret==0);
		boolean result = m_audioConfig.IsAGCEnabled();
		assertTrue(result == enabled);
	}
	
	public void testEnableEC() {
		Log.v("MediaConfigUT", "testEnableEC");
		boolean enabled = true;
		long cret = m_audioConfig.EnableEC(enabled);
		assertTrue(cret==0);
		boolean result = m_audioConfig.IsECEnabled();
		assertTrue(result == enabled);
	}
	
	public void testEnableVAD() {
		Log.v("MediaConfigUT", "testEnableVAD");
		boolean enabled = true;
		long cret = m_audioConfig.EnableVAD(enabled);
		assertTrue(cret==0);
		boolean result = m_audioConfig.IsVADEnabled();
		assertTrue(result == enabled);
	}
	
	public void testEnableNS() {
		Log.v("MediaConfigUT", "testEnableNS");
		boolean enabled = true;
		long cret = m_audioConfig.EnableNS(enabled);
		assertTrue(cret==0);
		boolean result = m_audioConfig.IsNSEnabled();
		assertTrue(result == enabled);
	}
	
	public void testEnableKeyDumpFiles() {
		Log.v("MediaConfigUT", "testEnableKeyDumpFiles");
		long time = 0;
		long cret = m_audioConfig.EnableKeyDumpFiles(time);
		assertTrue(cret==0);
	}

 	  
	//-----Test BaseVideoConfig-----	
	public void testSetDecodeParams() {
		Log.v("MediaConfigUT", "testSetDecodeParams");
		WmeCodecType codecType;
		WmeVideoCodecCapability codecParam, codecParam_r;
		codecType = WmeCodecType.WmeCodecType_AVC;
		codecParam = new WmeVideoCodecCapability();
		codecParam.uProfileLevelID = 0x42001E;
		codecParam.max_mbps = 40500;
		codecParam.max_fs = 1620;
		codecParam.max_fps = 3000;
		codecParam.max_br = 10000;
	
		long cret;
		cret = m_videoConfig.SetDecodeParams(codecType, codecParam);
		assertTrue(cret==0);
		codecParam_r = m_videoConfig.GetDecodeParams(codecType);
		assertTrue(cret==0);
		assertTrue(codecParam_r.uProfileLevelID == codecParam.uProfileLevelID);
		assertTrue(codecParam_r.max_mbps == codecParam.max_mbps);
		assertTrue(codecParam_r.max_fs == codecParam.max_fs);
		assertTrue(codecParam_r.max_fps == codecParam.max_fps);
		assertTrue(codecParam_r.max_br == codecParam.max_br);
		
		codecParam_r = m_videoConfig.GetDecodeParams(WmeCodecType.Unknown);
		assertTrue(codecParam_r == null);
	}
	public void swap(){
	}
	
	public void testSetEncodeParams() {
		Log.v("MediaConfigUT", "testSetEncodeParams");
		WmeCodecType codecType;
		WmeVideoCodecCapability codecParam, codecParam_r;
		codecType = WmeCodecType.WmeCodecType_AVC;
		codecParam = new WmeVideoCodecCapability();
		codecParam.uProfileLevelID = 0x42001E;
		codecParam.max_mbps = 40500;
		codecParam.max_fs = 1620;
		codecParam.max_fps = 3000;
		codecParam.max_br = 10000;
		
		long cret;
		cret = m_videoConfig.SetEncodeParams(codecType, codecParam);
		assertTrue(cret==0);
		codecParam_r = m_videoConfig.GetEncodeParams(codecType);
		assertTrue(codecParam_r.uProfileLevelID == codecParam.uProfileLevelID);
		assertTrue(codecParam_r.max_mbps == codecParam.max_mbps);
		assertTrue(codecParam_r.max_fs == codecParam.max_fs);
		assertTrue(codecParam_r.max_fps == codecParam.max_fps);
		assertTrue(codecParam_r.max_br == codecParam.max_br);
		
		codecParam_r = m_videoConfig.GetEncodeParams(WmeCodecType.Unknown);
		assertTrue(codecParam_r == null);
		
	}
	
	public void testSetMaxPacketSize() {
		Log.v("MediaConfigUT", "testSetMaxPacketSize");
		long value = 1;
		long cret = m_videoConfig.SetMaxPacketSize(value);
		assertTrue(cret==0);
		long result = m_videoConfig.GetMaxPacketSize();
		assertTrue(result == value);
	}

	public void testSetPacketizationMode() {
		Log.v("MediaConfigUT", "testSSetPacketizationMode");
		WmePacketizationMode value = WmePacketizationMode.WmePacketizationMode_0;
		long cret = m_videoConfig.SetPacketizationMode(value);
		assertTrue(cret==0);
		WmePacketizationMode result = m_videoConfig.GetPacketizationMode();
		assertTrue(result == value);
	}
    
	//-----Test VideoConfig-----
	public void testEnableBGRA() {
		Log.v("MediaConfigUT", "testEnableBGRA");
		boolean enabled = true;
		long cret = m_videoConfig.EnableBGRA(enabled);
		assertTrue(cret==0);
		boolean result = m_videoConfig.IsBGRAEnabled();
		assertTrue(result == enabled);
	}
	
	//-----Test GlobalConfig-----
	public void testEnableQos() {
		Log.v("MediaConfigUT", "testEnableQos");
		boolean enabled = true;
		long cret = m_globalConfig.EnableQos(enabled);
		assertTrue(cret==0);
		boolean result = m_globalConfig.IsQosEnabled();
		assertTrue(result == enabled);
	}
	
	public void testEnableMultiStream() {
		Log.v("MediaConfigUT", "testEnableMultiStream");
		boolean enabled = true;
		long cret = m_globalConfig.EnableMultiStream(enabled);
		assertTrue(cret==0);
		boolean result = m_globalConfig.IsMultiStreamEnabled();
		assertTrue(result == enabled);
	}

	public void testEnableICE() {
		Log.v("MediaConfigUT", "testEnableICE");
		boolean enabled = true;
		long cret = m_globalConfig.EnableICE(enabled);
		assertTrue(cret==0);
		boolean result = m_globalConfig.IsICEEnabled();
		assertTrue(result == enabled);
	}
	
	public void testEnableSRTP() {
		Log.v("MediaConfigUT", "testEnableSRTP");
		boolean enabled = true;
		long cret = m_globalConfig.EnableSRTP(enabled);
		assertTrue(cret==0);
		boolean result = m_globalConfig.IsSRTPEnabled();
		assertTrue(result == enabled);
	}
	
	public void testSetQoSMaxLossRatio() {
		Log.v("MediaConfigUT", "testSetQoSMaxLossRatio");
		float value = 0;
		long cret = m_globalConfig.SetQoSMaxLossRatio(value);
		assertTrue(cret==0);
		float result = m_globalConfig.GetQoSMaxLossRatio();
		assertTrue(result == value);
	}
	
	public void testSetQoSMinBandwidth() {
		Log.v("MediaConfigUT", "testSetQoSMinBandwidth");
		long value = 0;
		long cret = m_globalConfig.SetQoSMinBandwidth(value);
		assertTrue(cret==0);
		long result = m_globalConfig.GetQoSMinBandwidth();
		assertTrue(result == value);
	}
	
	public void testEnableRTCPMux() {
		Log.v("MediaConfigUT", "testEnableRTCPMux");
		boolean enabled = true;
		long cret = m_globalConfig.EnableRTCPMux(enabled);
		assertTrue(cret==0);
		boolean result = m_globalConfig.IsRTCPMuxEnabled();
		assertTrue(result == enabled);
	}
	
	public void testSetDeviceMediaSettings() {
		Log.v("MediaConfigUT", "testSetDeviceMediaSettings");
		String capability = "";
		long cret = m_globalConfig.SetDeviceMediaSettings(capability);
		assertTrue(cret==0);
	}
	
	public void testSetNetworkNotificationParam() {
		Log.v("MediaConfigUT", "testSetNetworkNotificationParam");
		WmeNetworkStatus status = WmeNetworkStatus.WmeNetwork_bad;
        WmeNetworkDirection direction = WmeNetworkDirection.DIRECTION_BOTHLINK;
        long waitMilliseconds = 1000;
		long cret = m_globalConfig.SetNetworkNotificationParam(status, direction, waitMilliseconds);
		assertTrue(cret==0);
	}
	
}
