///
///  WmeDefine.h
///
///
///  Created by Sand Pei on 2013-1-14.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef WME_DEFINE_H
#define WME_DEFINE_H

#include "WmeCommonTypes.h"

namespace wme
{

/// Define WME_EXPORT
#if defined(WIN32)
    #if defined(WME_DLL_EXPORT)
        #define WME_EXPORT __declspec(dllexport)
    #else
        #define WME_EXPORT __declspec(dllimport)
    #endif // !defined(WSE_DLL_EXPORT)
#else
    #if defined(__GNUC__) && (__GNUC__ >= 4)
        #define WME_EXPORT __attribute__ ((visibility ("default")))
    #else
        #define WME_EXPORT
    #endif // !defined(__GNUC__) || !(__GNUC__ >= 4)
#endif // !defined(WIN32)

/// Define WME_DEPRECATED
#define	WME_DEPRECATED

/// Define WME_MAX
#ifndef WME_MAX
    #define WME_MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

/// Define WME_MIN
#ifndef WME_MIN
    #define WME_MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

/// Struct of point
typedef struct _tagPoint
{
	int32_t x;
	int32_t y;
}WmePoint;

/// Struct of size
typedef struct _tagSize
{
	int32_t width;
	int32_t height;
}WmeSize;

/// Struct of rectangle
typedef struct _tagRect
{
	WmePoint origin;
	WmeSize size;
}WmeRect;
    
typedef void*   WmeWindowHandle;
typedef void*	WmeDCHandle;

/// Enum of trace level
typedef enum {
	WME_TRACE_LEVEL_NOTRACE = -1,
	WME_TRACE_LEVEL_ERROR = 0,
	WME_TRACE_LEVEL_WARNING,
	WME_TRACE_LEVEL_INFO,
	WME_TRACE_LEVEL_DEBUG,
	WME_TRACE_LEVEL_DETAIL,
	WME_TRACE_LEVEL_ALL
}WmeTraceLevel;

/// Enum of media type
typedef enum
{
    WmeMediaTypeAudio,		///< Audio type
    WmeMediaTypeVideo,		///< Video type
    WmeMediaTypeText,		///< Text type
	WmeMediaTypeData,		///< Data type
}WmeMediaType;

/// Enum of media format
typedef enum
{
    WmeMediaFormatAudioRaw,				///< Audio raw type, struct WmeAudioRawFormat
    WmeMediaFormatAudioCodec,			///< Audio codec type, struct WmeAudioCodecFormat
    WmeMediaFormatVideoRaw,				///< Video raw type, struct WmeVideoRawFormat
    WmeMediaFormatVideoRawWithStride,	///< Video raw type with Stride, struct WmeVideoRawFormatWithStride
    WmeMediaFormatVideoCodec,			///< Video codec type, struct WmeVideoCodecFormat
    WmeMediaFormatTextRaw,				///< Text raw type
    WmeMediaFormatTextCodec,			///< Text codec type
	WmeMediaFormatScreenRaw,			///< Screen raw type, struct WmeScreenRawFormat
	WmeMediaFormatScreenRC,				///< Screen remote control type, struct WmeScreenRemoteControlFormat
	WmeMediaFormatScreenAnnotation,		///< Screen annotation type, struct WmeScreenAnnotationFormat
    WmeMediaFormatRTP,					///< RTP type
	WmeMediaFormatRTCP,					///< RTCP type
}WmeMediaFormatType;

typedef WmeMediaFormatType WmeMediaPackageType;

/// Struct of media format
typedef struct _tagMediaFormat
{
    WmeMediaFormatType eType;	///< Identify the media format type
	void *pFormat;				///< Pointer to a block of data structure
    int32_t iSize;				///< Block size
}WmeMediaFormat;

/// Enum of audio raw format type
typedef enum
{
	WmeAudioUnknown   = 0,  
	WmePCM,
}WmeAudioRawType;

/// Struct of audio raw format
typedef struct _tagAudioRawFormat
{
	WmeAudioRawType		eRawType;		///< Identify the audio raw format type	
	int32_t				iChannels;		///< number of channels (i.e. mono, stereo...) 
	int32_t				iSampleRate;	///< Sample rate
	int32_t				iBitsPerSample;	///< Number of bits per sample of raw data
}WmeAudioRawFormat;

/// Enum of video raw format type
typedef enum
{
	WmeVideoUnknown   = 0,
    /*yuv color formats*/
	WmeI420,
	WmeYV12,
	WmeNV12,
	WmeNV21,
	WmeYUY2,
    WmeUYVY,
    /*rgb color formats*/
	WmeRGB24,
	WmeBGR24,
	WmeRGB24Flip,
	WmeBGR24Flip,
	WmeRGBA32,
	WmeBGRA32,
	WmeARGB32,
	WmeABGR32,
	WmeRGBA32Flip,
	WmeBGRA32Flip,
	WmeARGB32Flip,
	WmeABGR32Flip,
    /*compression color formats*/
    WmeMJPG,
}WmeVideoRawType;

/// Struct of video raw format
typedef struct _tagVideoRawFormat
{
	WmeVideoRawType		eRawType;		///< Identify the video raw format type
	int32_t				iWidth;			///< Frame width			
	int32_t				iHeight;		///< Frame height
	float				fFrameRate;		///< Frame rate
	uint32_t			uTimestamp;		///< Frame timestamp
}WmeVideoRawFormat;

/// Struct of video raw format with stride
#ifndef MAX_PLANE_NUM
    #define MAX_PLANE_NUM   4
#endif
typedef struct _tagVideoRawFormatWithStride : public WmeVideoRawFormat
{
    int32_t             iOffset;        ///< Frame raw offset
    int32_t             iStride;        ///< Frame raw stride
    bool                bPlanar;        ///< Identify whether is the frame raw planar
    int32_t             iPlaneCount;    ///< Frame raw plane count. If it is not planar, the value is zero.
    int32_t             iPlaneOffset[MAX_PLANE_NUM];    ///< Frame raw plane offsets. If it is not planar, all values are zero.
    int32_t             iPlaneStride[MAX_PLANE_NUM];    ///< Frame raw plane strides. If it is not planar, all values are zero.
}WmeVideoRawFormatWithStride;

/// Struct of screen raw format
typedef struct _tagScreenRawFormat : public WmeVideoRawFormat
{
	WmePoint			stMousePoint;	///< Screen mouse point
}WmeScreenRawFormat;

/// Struct of screen remote control format
typedef struct _tagScreenRemoteControlFormat
{
	void *pData;
	int32_t iSize;
}WmeScreenRemoteControlFormat;

/// Struct of screen annotation format
typedef struct _tagScreenAnnotationFormat
{
	void *pData;
	int32_t iSize;
}WmeScreenAnnotationFormat;
    
typedef enum
{
    ///< Video frame type: P=0; IDR =1 , GDR=2,  LTR=3
    WmeRTPFrameType_VIDEO_P     = 0,
    WmeRTPFrameType_VIDEO_IDR   = 1,
    WmeRTPFrameType_VIDEO_GDR   = 2,
    WmeRTPFrameType_VIDEO_LTR   = 3,
    
    ///< Audio
    WmeRTPFrameType_AUDIO       = 10,
    
    ///< FEC
    WmeRTPFrameType_FEC         = 20
}WmeRTPFrameType;
    
/// Struct of RTP package format
typedef struct _tagRTPPackageFromat
{
    WmeRTPFrameType     eFrameType;     ///< the frame type to which this RTP packet belons
    uint32_t            uPacketCount;   ///< how many packets does the frame have?
    uint32_t            uPacketIndex;   ///< the index of this packet in the frame
    uint8_t             uDID;           ///< Dependency id of this frame
}WmeRTPPackageFormat;
// Just to keep backward compatibility
typedef WmeRTPPackageFormat WmeRTPPackageFromat;

/// Enum of codec type
typedef enum
{
	WmeCodecType_Unknown = 0,	///< unknown codec type

	WmeCodecType_G711_ULAW = 1,	///< audio codec G.711 uLaw
	WmeCodecType_G711_ALAW,		///< audio codec G.711 aLaw
	WmeCodecType_ILBC,			///< audio codec iLBC
	WmeCodecType_OPUS,			///< audio codec Opus
	WmeCodecType_G722,          ///< audio codec G.722
	WmeCodecType_CNG,			///< audio codec CNG

	WmeCodecType_AVC = 100,		///< video codec AVC
	WmeCodecType_SVC,			///< video codec SVC
	WmeCodecType_HEVC,			///< video codec HEVC
	WmeCodecType_VP8,			///< video codec VP8
    
    WmeCodecType_RSFEC = 126    ///< Mari Fec codec
}WmeCodecType;

/// Struct of media codec format
typedef struct _tagMediaCodecFormat
{
	WmeCodecType		eCodecType;
	uint32_t			uTimestamp;
	uint32_t			uSampleTimestamp;
	uint8_t				uMarker;
}WmeMediaCodecFormat;

/// Struct of audio codec format
typedef struct _tagAudioCodecFormat	: public WmeMediaCodecFormat
{
	// audio receive
	uint16_t			uSequenceNumber;
	uint32_t			uReceiveTime;
	uint32_t			uSSRC;

	// audio level indication extension
	bool				bVad;
	uint8_t				uLevel;
}WmeAudioCodecFormat;

/// Struct of video codec format
typedef struct _tagVideooCodecFormat : public WmeMediaCodecFormat
{
	// video send
	uint8_t				uLayerNumber;
	uint8_t				uLayerIndex;
	uint32_t			uLayerWidth;
	uint32_t			uLayerHeight;
	bool				bLayerMapAll;

	// video frame marking extension
	uint32_t			uFrameIDC;
	bool				bDisposable;	///< 1 -- disposable packet; non-disposable packet.
	bool				bSwitchable;	///< 1 -- switchable packet; 0 -- non-switchable packet.
	uint8_t				uPriority;		///< Priority for frame.
	uint8_t				uFrameType;		///< P=0; IDR =1 , GDR=2,  LTR=3
	uint8_t				uDID;			///< Spatial ID as defined in RFC6190
	uint8_t				uTID;			///< Temporal layer ID as defined in RFC6190.
	uint8_t				uMaxTID;		///< Max. temporal layer encoded
}WmeVideoCodecFormat;

/// Enum of media file config
typedef enum
{
	WmeMediaFileConfig_VideoRawFormat = 0,				///< The video raw data structure for YUV/RGB series. The value type is WmeVideoRawFormat
	WmeMediaFileConfig_AudioRawFormat = 1,				///< The audio raw data structure for PCM data. The value type is WmeAudioRawFormat

	WmeMediaFileConfig_GoLoop = 100,					///< The flag to enable file loop, when EOS. Default is enabled
	//WmeMediaFileConfig_PPMode = 101,					///< The data push mode or pull mode, only supported in renderer
    WmeMediaFileConfig_TimestampMode = 102,             ///< The flag to fixed rate timestamp info, supported in capturer
    WmeMediaFileConfig_DumpWithFormat = 103,            ///< The flag to dump a private format data, supported in renderer
    WmeMediaFileConfig_SaveToFile = 104,                ///< The flag to determine whether save the data to file
}WmeMediaFileConfig;


//Add by Boris(2015-1-21)
//Audio pairing
typedef enum
{
	WmeExternalRender_Type_Normal			= 0,        ///< normal rendering
    WmeExternalRender_Type_AudioPairing		= 1,		///< The External Render is for audio pairing
    WmeExternalRender_Type_CaptureFromHardware,         ///< capture end, audio data after caputre from hardware
    WmeExternalRender_Type_CaptureBeforeEncode,         ///< capture end, audio data before encode and sent out
    WmeExternalRender_Type_PlaybackToHardware,          ///< playback end, audio data before sent to hardware to play
    WmeExternalRender_Type_RTP,                         ///< RTP packets after basic RTP session, this is added for NBR usage.
}WmeMediaExternalRenderTypeConfig;

typedef enum
{
    WmeRenderModeFill = 0,           ///< Stretch the Pic to fit the window
    WmeRenderModeLetterBox = 1,			///< Maintain the aspect ratio and content of the Pic
    WmeRenderModeCropFill = 2,			///< Crop some content to fix the aspect ratio of the window
    WmeRenderModeOriginal = 3
}WmeTrackRenderScalingModeType;
   
//for SVS: set actual source info
typedef struct _tagVideoSourceInfo
{
    int  nWidth;
    int  nHeight;
    float  fFrameRate;
} WmeVideoSourceInfoType;
   
typedef struct _tagVideoSizeInfo
{
    unsigned int width;         // video width, including possible black bars or areas not interested
    unsigned int height;        // video height, including possible black bars or areas not interested
    unsigned int realWidth;     // real video content area width
    unsigned int realHeight;    // real video content area height
    unsigned int offsetW;       // offset realative to the left border
    unsigned int offsetH;       // offset realative to the top border
} WmeVideoSizeInfo;

#define THREAD_NAME_STATISTIC "low-pri-stat"
}

#endif // WME_DEFINE_H
