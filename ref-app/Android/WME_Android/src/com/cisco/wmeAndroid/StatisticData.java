package com.cisco.wmeAndroid;

public class StatisticData {

	public static int videoCaptureWidth = 0;
	public static int videoCaptureHeight = 0;
	public static int videoCaptureFPS = 0;
	
	public static int videoEncodeMaxWidth = 0;
	public static int videoEncodeMaxHeight = 0;
	public static int videoEncodeMaxFPS = 0;
	
	public static int videoDecodeWidth = 0;
	public static int videoDecodeHeight = 0;
	public static int videoDecodeFPS = 0;
	
	public static void setVideoCaptureResolution_jni(int width, int height) {
		videoCaptureWidth = width;
		videoCaptureHeight = height;
	}
	public static void setVideoCaptureFPS_jni(int fps) {
		videoCaptureFPS = fps;
	}
	
	//disable the code, use GetTrackStatistics instead
//	public static void setVideoEncodeMaxResolution_jni(int max_width, int max_height) {
//		videoEncodeMaxWidth = max_width;
//		videoEncodeMaxHeight = max_height;
//	}
//	public static void setVideoEncodeMaxFPS_jni(int max_fps) {
//		videoEncodeMaxFPS = max_fps;
//	}
//	public static void setVideoDecodeResolution_jni(int width, int height) {
//		videoDecodeWidth = width;
//		videoDecodeHeight = height;
//	}
//	public static void setVideoDecodeFPS_jni(int fps) {
//		videoDecodeFPS = fps;
//	}
	
	public static void resetAll()
	{
		videoCaptureWidth = 0;
		videoCaptureHeight = 0;
		videoCaptureFPS = 0;
		
		videoEncodeMaxWidth = 0;
		videoEncodeMaxHeight = 0;
		videoEncodeMaxFPS = 0;
		
		videoDecodeWidth = 0;
		videoDecodeHeight = 0;
		videoDecodeFPS = 0;
	}
	
}