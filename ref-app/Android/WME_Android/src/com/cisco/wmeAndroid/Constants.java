package com.cisco.wmeAndroid;

public class Constants {
	public static final int DEFAULT_SPEAKER_VOLUME = 32768;
	public static final int DEFAULT_MIC_VOLUME = 32768;
	
	public static final int EVENT_READY_TO_LAUNCH_PLAY = 1;
	public static final int EVENT_INIT = 2;
	public static final int EVENT_ADJUST_PREVIEW_ORIENTATION = 3;
	public static final int EVENT_FORCE_STOP_TIMER = 4;
	public static final int EVENT_GET_BATTERY = 5;
	public static final int EVENT_GET_CPU = 6;
	public static final int EVENT_CPU_USAGE_SNAPSHOT = 7;
	public static final int EVENT_POST_PERFORMANCE_CMD = 8;
	public static final int EVENT_START_CALL = 9;
	public static final int EVENT_STOP_CALL = 10;
	public static final int EVENT_START_SESSION_COMPLETE = 11;
	public static final int EVENT_STOP_SESSION_COMPLETE = 12;
	public static final int EVENT_START_CALL_COMPLETE = 13;
	public static final int EVENT_STOP_CALL_COMPLETE = 14;
	public static final int EVENT_INFO_COMPLETE = 15;
	public static final int EVENT_START_SESSION_SENT = 16;
	public static final int EVENT_STOP_SESSION_SENT = 17;
	public static final int EVENT_START_CALL_SENT = 18;
	public static final int EVENT_STOP_CALL_SENT = 19;
	public static final int EVENT_INFO_SENT = 20;
	public static final int EVENT_STATISTIC_REFRESH = 21;
	
	public static final String PICTURE_IN_PICTURE = "PictureInPicture";
	public static final String CODEC_PARAM_INDEX = "CodecParamIndex";
	public static final String VIDEO_QUALITY_INDEX = "VideoQualityIndex";
	public static final String CAMERA_INDEX = "CameraIndex";
	public static final String CAMERA_PARAM_INDEX = "CameraParamIndex";
	public static final String ENABLE_VIDEO_FILE_RENDER = "EnableVideoFileRender";
	public static final String ENABLE_AUDIO_FILE_RENDER = "EnableAudioFileRender";
	
	public static final int MSG_ERR_NONE = 0;
	public static final int MSG_ERR_FAIL = 1;
	public static final int MSG_ERR_TIMEOUT = 2;
	public static final int MSG_ERR_CANCEL = 3;
	public static final int MSG_ERR_NO_NET_CONNECTION = 4;
	
	public static final int WEBCALL_START_SESSION = 0;
	public static final int WEBCALL_STOP_SESSION = 1;
	public static final int WEBCALL_START_CALL = 2;
	public static final int WEBCALL_STOP_CALL = 3;
	public static final int WEBCALL_INFO = 4;
	
	public static final String DEFAULT_HTTP_SERVER_URL = "http://173.39.168.28:8080";
	public static final String HTTP_UNKNOWN_URL = "http://unknownurl";
	
	public static final int BATTERY_INTERVAL = 10*1000;
	public static final int STATISTIC_INTERVAL = 1000;
}