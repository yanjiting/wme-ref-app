package com.webex.wme.test;

import junit.framework.TestCase;
import com.webex.wme.MediaConnection;
import com.webex.wme.MediaConnection.MediaType;
import com.webex.wme.MediaConnection.MediaDirection;
import com.webex.wme.MediaConfig.GlobalConfig;
import com.webex.wme.MediaConfig.AudioConfig;
import com.webex.wme.MediaConfig.VideoConfig;
import com.webex.wme.MediaConfig.ShareConfig;
import com.webex.wme.MediaConfig.WmeCodecType;
import com.webex.wme.MediaConfig.WmeConnectionPolicy;
import com.webex.wme.MediaConfig.WmePacketizationMode;
import android.util.Log;


import org.json.JSONException;
import org.json.JSONObject;


public class MediaConnectionUT extends TestCase {
	private MediaConnection m_mediaConn;
	/*private GlobalConfig m_globalConfig;
	private AudioConfig m_audioConfig;
	private VideoConfig m_videoConfig;
	private ShareConfig m_shareConfig;
*/
    private final int AUDIO_MID = 1;
    private final int VIDEO_MID = 2;
    private final int SHARE_MID = 3;
	
	public MediaConnectionUT(String name) {
		super(name);
	}

	/*public static class MyMediaConnection extends MediaConnection{
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
	}*/
	protected void setUp() throws Exception {
		super.setUp();
		m_mediaConn = new MediaConnection();
		m_mediaConn.addMedia(MediaType.Audio, MediaDirection.SendOnly, AUDIO_MID, "");
		m_mediaConn.addMedia(MediaType.Video, MediaDirection.SendOnly, VIDEO_MID, "");
		m_mediaConn.addMedia(MediaType.Sharing, MediaDirection.SendOnly, SHARE_MID, "");
	}
	
	//-----Test GlobalConfig-----
	public void testGetMediaConfig() {
		Log.v("MediaConnectionUT", "testGetMediaConfig");
		
		GlobalConfig m_globalConfig = m_mediaConn.GetGlobalConfig();
		assertTrue(m_globalConfig != null);
		
		AudioConfig m_audioConfig = m_mediaConn.GetAudioConfig(AUDIO_MID);
		assertTrue(m_audioConfig != null);
		
		VideoConfig m_videoConfig = m_mediaConn.GetVideoConfig(VIDEO_MID);
		assertTrue(m_videoConfig != null);
		
		ShareConfig m_shareConfig  = m_mediaConn.GetShareConfig(SHARE_MID);
		assertTrue(m_shareConfig != null);
	}
	
	/*public void testEnableQos() {
		Log.v("MediaConfigUT", "testEnableQos");
		GlobalConfig m_globalConfig = m_mediaConn.GetGlobalConfig();
		boolean enabled = true;
		long cret = m_globalConfig.EnableQos(enabled);
		assertTrue(cret==0);
		boolean result = m_globalConfig.IsQosEnabled();
		assertTrue(result == enabled);
	}
	
	public void testEnableAGC() {
		Log.v("MediaConfigUT", "testEnableAGC");
		AudioConfig m_audioConfig = m_mediaConn.GetAudioConfig(AUDIO_MID);
		boolean enabled = true;
		long cret = m_audioConfig.EnableAGC(enabled);
		assertTrue(cret==0);
		boolean result = m_audioConfig.IsAGCEnabled();
		assertTrue(result == enabled);
	}*/
	
}
