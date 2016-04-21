package com.cisco.webex.wme;


/**
 * class WmeParameters is for default setting value between Java and JNI lib
 */
public class WmeParameters {

    // Wme Media Type (mtype)
    public static final int WME_MEDIA_NONE = 0;
    public static final int WME_MEDIA_AUDIO = 1;
    public static final int WME_MEDIA_VIDEO = 2;
    public static final int WME_MEDIA_SHARE = 3;

    // Wme Track Type
    public static final int WME_LOCAL_TRACK = 0;
    public static final int WME_PREVIEW_TRACK = 1;
    public static final int WME_REMOTE_TRACK = 2;
    public static final int WME_LOCAL_SHARE_TRACK = 3;
    public static final int WME_REMOTE_SHARE_TRACK = 4;

    // Wme Device Type
    public static final int WME_DEV_CAMERA = 0;
    public static final int WME_DEV_MIC = 1;
    public static final int WME_DEV_SPEAKER = 2;
    
    //Wme Audio out type
    public static final int WME_AUDIO_OUT_VOICE = 0;
    public static final int WME_AUDIO_OUT_SPEAKER = 1;

    // Wme Codec Type
    public static final int WmeCodecType_Unknown = 0;   ///< uknown codec type
    public static final int WmeCodecType_G711_ULAW = 1; ///< audio codec G.711 uLaw
    public static final int WmeCodecType_G711_ALAW = 2;     ///< audio codec G.711 aLaw
    public static final int WmeCodecType_ILBC = 3;          ///< audio codec iLBC
    public static final int WmeCodecType_OPUS = 4;          ///< audio codec Opus
    public static final int WmeCodecType_G722 = 5;          ///< audio codec G.722
    public static final int WmeCodecType_CNG = 6;           ///< audio codec CNG
    public static final int WmeCodecType_AVC = 100;     ///< video codec AVC
    public static final int WmeCodecType_SVC = 101;           ///< video codec SVC
    public static final int WmeCodecType_HEVC = 102;          ///< video codec HEVC
    public static final int WmeCodecType_VP8  = 103;           ///< video codec VP8
    
    
    // Hearbeat msg for tp
    public static final int TPConnectToMsg = 0;
    public static final int TPInitHostMsg = 1;    	
    public static final int TPDisconnect = 2;   
	public static final int	TPInitMainThread = 3;
	public static final int TPConnectFileMsg = 4;

    public static final int WmeRenderMode_Stretch = 0;
    public static final int WmeRenderMode_LetterBox = 1;
    public static final int WmeRenderMode_FitAfterCrop = 2;

    // for Data dump flag (dflag)
    public static final int WME_DATA_DUMP_VIDEO_RAW_CAPTURE                             = 0x1;
    public static final int WME_DATA_DUMP_VIDEO_ENCODE_RTP_LAYER                        = 0x2;
    public static final int WME_DATA_DUMP_VIDEO_NAL_TO_LISTEN_CHANNEL                   = 0x4;
    public static final int WME_DATA_DUMP_VIDEO_NAL_TO_DECODER                          = 0x8;
    public static final int WME_DATA_DUMP_VIDEO_RAW_AFTER_DECODE_TO_RENDER              = 0x10;
    
    // for Raw Type
    public static final int WmeVideoUnknown   	= 0; 
    public static final int WmeI420 			= 1;
    public static final int WmeYV12 			= 2;
    public static final int WmeNV12 			= 3;
    public static final int WmeNV21 			= 4;
    public static final int WmeYUY2 			= 5;
    public static final int WmeRGB24 			= 6;
    public static final int WmeBGR24 			= 7;
    public static final int WmeRGB24Flip 		= 8;
    public static final int WmeBGR24Flip 		= 9;
    public static final int WmeRGBA32 			= 10;
    public static final int WmeBGRA32 			= 11;
    public static final int WmeARGB32 			= 12;
    public static final int WmeABGR32 			= 13;
    
    // for video source
    public static final int SOURCE_FILE 		= 1;
    public static final int SOURCE_CAMERA		= 0;
    public static final int SOURCE_MIC 		= 0;
    
    public static final int TARGET_FILE = 0;
    public static final int TARGET_SPEAKER = 1;
    public static final int TARGET_SCREEN = 1;
    
}
