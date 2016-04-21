///
///  WmeTrack.h
///
///
///  Created by Sand Pei on 13-1-23.
///  Copyright (c) 2013 Cisco. All rights reserved.
///


#ifndef WME_TRACK_INTERFACE_H
#define WME_TRACK_INTERFACE_H

#include "WmeDefine.h"
#include "WmeUnknown.h"
#include "WmeCodec.h"
#include "WmeStatistics.h"
#include "WmeDevice.h"
#include "WmeExternalCapturer.h"
#include "WmeExternalRenderer.h"
#include "WmeEvent.h"

namespace wrtp
{
	class IRTPChannel;
}

namespace wme
{

///============================Media Track Define============================

/// An enum of track type
typedef enum
{
    WmeTrackType_Uknown = 0,			///< Unknown type
	
	WmeTrackType_Audio = 1,		 		///< Audio type, basic track type 
    WmeTrackType_Video,					///< Video type, basic track type
	WmeTrackType_Data,					///< Data type, basic track type
	
	WmeTrackType_Composite = 10,		///< Composite type, it should has multi basic track types, including audio, video or data type
}WmeTrackType;

/// An enum of track role
typedef enum {
	WmeTrackRole_Unknown = 0,			///< Unknown track role
	WmeTrackRole_Local,					///< Local track role
	WmeTrackRole_Remote,				///< Remote track role
	WmeTrackRole_Preview,				///< Preview track role
}WmeTrackRole;

/// An enum of track state
typedef enum
{
	WmeTrackState_Unknown = 0,			///< Unknown state, for output param
    WmeTrackState_Initializing,			///< Track created but not fully initilized
	WmeTrackState_Ready,				///< Initialized and resource is occupied
    WmeTrackState_Live,					///< Live state
    WmeTrackState_Ended,				///< Track is over, should be removed
    WmeTrackState_Failed				///< Failed state
}WmeTrackState;

/// An enum of layer type
typedef enum {
	WmeVideoSingleLayer,				///< Encode only one layer				
	WmeVideoMultiLayer					///< Encode multi layers
}WmeVideoEncodeLayerType;

/// An enum of video quality type
typedef enum
{
	WmeVideoQuality_SLD = 0,		///< Small low standard video quality, 160x90 
	WmeVideoQuality_LD,				///< Low standard video quality, 320x180
	WmeVideoQuality_SD,				///< Standard definition video quality, 640x360 
    WmeVideoQuality_HD_720P,		///< High definition video quality, 1280x720
    //WmeVideoQuality_HD_1080P,		///< High definition video quality, 1920x1080
}WmeVideoQualityType;

/// An enum of performance profile type
typedef enum
{
	WmePerformanceProfileLow      = 0,	///< Low performance
	WmePerformanceProfileMedium   = 1,	///< Medium performance
	WmePerformanceProfileNormal   = 2,	///< Normal performance
	WmePerformanceProfileHigh	 = 3,	///< High performance

	WmePerformanceProfileAuto	= 10	///< Auto performance, track should get internal static performance
}WmePerformanceProfileType;
    
typedef enum
{
    WmePortraitUp,		///<	Portrait orientation.
    WmePortraitDown,	///<	Portrait orientation. This orientation is never used.
    WmeLandscapeLeft,	///<	Landscape orientation with the top of the page rotated to the left.
    WmeLandscapeRight	///<	Landscape orientation with the top of the page rotated to the right.
}WmeCameraOrientation;

/// An structure of network statistics info
typedef struct _tagNetStat
{
	uint32_t loss_ratio;				///< Network loss ratio info
	uint32_t delay;						///< Network delay info
	uint32_t jitter;					///< Network jitter info
	uint32_t recv_rate;					///< Network receive rate
}WmeNetStat;

/// An structure of uplink network statistics info
typedef struct _tagUpLinkNetStat
{
	WmeNetStat	NetStat;				///< Uplink network statistics info
	uint32_t	dwEvaluateBandwidth;	///< Uplink evaluate bandwidth
	uint32_t	nCongestStatus;			///< Uplink congest status
}WmeUpLinkNetStat;

typedef struct _tagStreamIdInfo
{
    uint8_t   DID;
    uint32_t  BwInBytePerSec;
    uint8_t   vidCount;
    uint8_t*  vidArray;
}WmeStreamInfo;

/// An structure of bandwith  of every stream reported from source channel to WRTP
typedef struct _tagSessionBasedBandwidth
{
    uint32_t   uTrackLabel;                ///< Track ID
    uint32_t   uNumStreams;                ///< Actrual number of streams, max value is 4;
    WmeStreamInfo sStreamInfo[4];          ///< info of every stream
}WmeAllStreamsBandwidth;
    
/// An structure of video encode configuration
typedef struct _tagVideoEncodeConfiguraion
{
	uint32_t	iMaxWidth;					///< The max encode width
	uint32_t	iMaxHeight;					///< The max encode height
	uint32_t	iMaxSpacialLayer;			///< The max encode layers
	float		iMaxFrameRate;				///< The max encode frame rate
}WmeVideoEncodeConfiguraion;

/// An structure of video encode capability
typedef struct _tagVideoEncodeCapability
{
	uint32_t uProfileLevelID;				///< The profile level ID
	uint32_t uMaxMBPS;						///< The max macroblock processing rate in units of macroblocks per second
	uint32_t uMaxFS;						///< The max frame size in units of macroblocks
	uint32_t uMaxFPS;						///< The max frame number per second 
	uint32_t uMaxBitRate;					///< The max bits per second
	uint32_t uMaxNalUnitSize;				///< The max bytes of NAL unit size
    uint32_t uMaxDPB;				        ///< The max bytes of DPB (
    uint8_t  uVids[8];                      ///< The VID array
    uint32_t uNumVids;                      ///< The number of VIDs in array
    bool     bSucceeded;                    ///< Subscription succeeded or failed
}WmeVideoEncodeCapability;

typedef struct _tagVideoSubscribeMultiStream
{
    uint32_t uNumStreams;
    WmeVideoEncodeCapability sStream[4];
    //uint8_t  uVid[4];
}WmeVideoSubscribeMultiStream;
    
/// An structure of media bandwidth
typedef struct _tagMediaBandwidth
{
	uint32_t	uiMaxBandwidth;			///< The max bandwidth
	uint32_t	uiMinBandwidth;			///< The min bandwidth
}WmeMediaBandwidth;

/// An enum of filter result
typedef enum {
	WmeFilter_Unknown,					///< Unknown filter result
	WmeFilter_Passed,					///< Filter passed
	WmeFilter_Dropped					///< Filter dropped
}WmeFilterResult;

/// An structure of sending filter info
typedef struct _tagSendingFilterInfo
{
	WmeFilterResult result;				///< Filter result
	uint32_t totalLen;					///< Total sending data length
	uint8_t* infoData;					///< Info data pointer
	uint32_t infoDataLen;				///< Info data length
}WmeSendingFilterInfo;

/// An structure of sending status changed info
typedef struct _tagSendingStatusInfo
{
	uint32_t	uUsedBufferSize;		///< Used buffer size for sending
	uint32_t	uTotalBufferSize;		///< Total buffer size for sending
    uint32_t    uDid;                   ///< Stream index of sending buffer
}WmeSendingStatusInfo;

/// An structure of video window-less render info
typedef struct _tagWindowLessRenderInfo
{
	WmeDCHandle	hDC;					///< DC handle
	WmeRect		stRect;					///< Render rectangle
	bool		bReDraw;				///< Re-draw flag
}WmeWindowLessRenderInfo;

typedef struct _tagVidInfo
{
    uint8_t  uVidCount;
    uint8_t* pVidArray;
}WmeVidInfo;
    
/// An enum of captuer start result
typedef enum
{
	WmeCaptureStartedSuccess = 0,		///< Capture started successfully
	WmeCaptureStartedFail,				///< Capture failed to start
}WmeCaptureStartedResult;

    
typedef enum{
    WmeAudioAecTypeNone = 0,         	///< No AEC 
    WmeAudioAecTypeBuildin = 1,		///< Build AEC, only VPIO mode is used.
    WmeAudioAecTypeWme = 2,          	///<  Windows/Mac/Linux uses Modified Movi AEC, Android/IOS use WebRTC AECM
    WmeAudioAecTypeTc = 3,           	///< TC AEC, Only IOS/Android supported.
    WmeAudioAecTypeAlpha = 4            ///< A-AEC, only Mac/Windows supported.
}WmeAudioAecType;
    
    
/// An enum of media event
typedef enum
{
	// for all media track
	WmeTrackEvent_UpLinkStat = 0,				///< Value type: WmeUpLinkNetStat
	WmeTrackEvent_ServerFilterInfo,				///< Value type: [To be determined]
	WmeTrackEvent_SendingFilterInfo,			///< Value type: WmeSendingFilterInfo
	WmeTrackEvent_SendingStatusChanged,			///< Value type: WmeSendingStatusInfo

	// for video track
	WmeTrackEvent_ForceKeyFrame,				///< Value type: bool, instant
	WmeTrackEvent_RenderWithDC,					///< Value type: WmeWindowLessRenderInfo [Windows only, for window-less render]
	WmeTrackEvent_RenderingDisplayChanged,		///< Value type: NULL [Windows only, for window render]
	WmeTrackEvent_RenderingPositionChanged,		///< Value type: void*(window handle) [Windows only, for window render]
	WmeTrackEvent_RenderingReDrawRequest,
    WMETrackEvent_StreamInfo,                   ///< Value type: WmeStreamInfo
    WMETrackEvent_RenderIndicationRequest,      ///< Value type: bool

	// for audio track
	WmeTrackEvent_AudioCaptureDataError,		///< Value type: AudioCaptureDataError error
	WmeTrackEvent_AudioPlaybackDataError,		///< Value type: AudioPlaybackDataError error
    WmeTrackEvent_AudioPlaybackTimestamp,       ///< Value type: uint32_t [Windows Phone only, for remote audio agent]
    
}WmeTrackEvent;

/// An enum of media option
typedef enum
{
	// for video track
	WmeTrackOption_VideoQuality = 0,			///< Value type: WmeMeetingVideoQualityType
	WmeTrackOption_VideoWindowLessRender,		///< Value type: bool [Windows only, for window-less render]
	WmeTrackOption_AdaptiveAspectRatio,	WME_DEPRECATED		///< Value type: bool [Default is true - it means that the aspect ratio of the encoder output is as the same as that of the capturing video] [WME_DEPRECATED: This option type has been deprecated and replace with WmeTrackOption_LandscapeForce] 
	WmeTrackOption_RenderScalingMode,			///< Value type: WmeTrackRenderScalingModeType

	// for video local track
	WmeTrackOption_VideoEncodeConfiguraion,		///< Value type: WmeVideoEncodeConfiguraion		
	WmeTrackOption_VideoEncodeLayerType,		///< Value type: WmeVideoEncodeLayerType
	WmeTrackOption_VideoEncoderCabacSupport,	///< Value type: bool
	WmeTrackOption_VideoEncodeMaxCapability,	///< Value type: WmeVideoEncodeCapability [Coming soon]
    WmeTrackOption_VideoMultiStreamSubscribe,
	WmeTrackOption_VideoEncodeIdrPeriod,		///< Value type: uint32_t [Intreval frame number, default is 0, the IDR period should be a multiple of 2^(layer number - 1)]
    WmeTrackOption_EnableHardwareVideoCapencoder,  ///< Value type: bool
	WmeTrackOption_VideoFrameTimestampAdjustment,		///< Value type: bool 
	WmeTrackOption_VideoStabilization,          ///< Value type: bool
    WmeTrackOption_EnableSimulcastAVC,          ///< Value type: bool
    WmeTrackOption_ChangeOrientation,           ///< Value type: WmeCameraOrientation
    WmeTrackOption_LandscapeForce,              ///< Value type: bool
    WmeTrackOption_EnableCVO,                   ///< Value type: bool, Coordination of video orientation(CVO)
    WmeTrackOption_EnableUpdateCapParam,        ///< Value type: bool
    WmeTrackOption_Disable90P,                  ///< Vlaue type: bool

	// for video remote track
	WmeTrackOption_EnableDecoderMosaic,			///< Value type: bool
    WmeTrackOption_EnableHardwareVideoDecrender,  ///< Value type: bool
    WmeTrackOption_DecoderControlInProgress,      ///< Value type: bool [Read only]
    WmeTrackOption_TimestampOfLatestDecodedFrame, ///< Value type: uint32_t [Read only]
    WmeTrackOption_Block_Thresdhold,              ///< Value type: uint32_t

	// for all local track
	WmeTrackOption_CapturePause,				///< Value type: bool
	WmeTrackOption_CaptureStatus,				///< Value type: WmeExternalCapturerStatus [Read only]
	WmeTrackOption_CaptureStartAsync,			///< Value type: bool
	WmeTrackOption_SendingData,					///< Value type: bool
	WmeTrackOption_MaxPayloadSize,				///< Value type: uint32_t [Default is 0, the value 0 signify not to limit max payload size]
    WmeTrackOption_VID,                         ///< Value type: WmeVidInfo

	// for all media track
	WmeTrackOption_StaticPerformanceProfile,	///< Value type: WmePerformanceProfileType [Deprecated]
	WmeTrackOption_Bandwidth,					///< Value type: WmeMediaBandwidth [Read only]

	//for audio local track
	WmeTrackOption_EnableDAGC,                  ///< Value type: bool  //Add by Boris(2015-4-8)
	WmeTrackOption_EnableAAGC,                  ///< Value type: bool
	WmeTrackOption_EnableEC,					///< Value type: bool
	WmeTrackOption_EnableVAD,					///< Value type: bool
	WmeTrackOption_EnableNS,					///< Value type: bool
	WmeTrackOption_EnableDropSeconds,			///< Value type: bool
    WmeTrackOption_OnHoldStatus,                ///< Value type: bool  //Add by Ark(2015.12.07)
    WmeTrackOption_ECType,                      ///< Value type: WmeAudioAecType
  
    WmeTrackOption_NoiseIndication,             ///< Value type, float  // db level, Get only
    WmeTrackOption_AECIndication,               ///< Value type, float  //         , Get only
    WmeTrackOption_VoiceLevelIndication,        ///< Value type, float  // db level, Get only
    
    
	//for audio local dump file (only for development period)
	WmeTrackOption_DE_EnableAll_Dumpfile,          ///< Value type: bool
	WmeTrackOption_DE_EnableCapture_Dumpfile,          ///< Value type: bool
	WmeTrackOption_DE_EnablePlayback_Dumpfile,          ///< Value type: bool
	WmeTrackOption_DE_EnableAEC_Dumpfile,          ///< Value type: bool

    // for audio dump file path
    WmeTrackOption_AudioDumpfilePath,				///< Value type: char*

	///for debugging audio use (for client)
	WmeTrackOption_EnableKey_Dumpfile,			///< Value type: int

	//for all media track
	WmeTrackOption_Cid,							///<Value type: char *
    
    // for screen local track
    WmeTrackOption_MaxScreenCaptureFps,				///< Value type: uint32_t
    WmeTrackOption_MaxScreenCaptureDownSampleMinHeight,				///< Value type: uint32_t
    WmeTrackOption_ShareFilterSelf,                 ///< Value type: bool
	WmeTrackOption_AddSharedWindow,                 ///< Value type: void*
    WmeTrackOption_RemoveSharedWindow,                 ///< Value type: void*

    WmeTrackOption_EnableFEC,                       ///< Value type: bool  // enable & disable Opus FEC
    WmeTrackOption_IsActiveLocalVideoExternalTrack,  ///<
    WmeTrackOption_DataTerminatedAtRTP,             ///< Data will be terminated at RTP layer. Value type: bool

    // Add by Conan(xiasu@cisco.com) (2015-12-30)
    // for change playback stream mode
    WmeTrackOption_SetPlaybackStreamMode,          ///< Value type: int
    WmeTrackOption_GetPlaybackStreamMode,          ///< Value type: int

 }WmeTrackOption;
    
// An enum of source type for track
typedef enum
{
    Wme_SourceType_UNKNOWN		   = 0,
    Wme_SourceType_Meeting_Camera  = 1,  //for webex camera video
    Wme_SourceType_Video_Sharing   = 2,  //for webex video sharing
    Wme_SourceType_Desktop_Sharing = 3,  //for webex desktop/application sharing
    Wme_SourceType_File_Capture    = 4,  //for file capture (only TA)
    Wme_SourceType_By_Pass         = 5   //for interface unified purpose, audio and remote use this
}WmeSourceType;

///============================Media Track Interface============================

/// IWmeMediaTrack interface
/*!	
	\note	Supported observers: None
*/
static const WMEIID WMEIID_IWmeMediaTrack = 
{ 0xca45909d, 0x166c, 0x49c2, { 0x81, 0x9, 0x49, 0x6, 0x5c, 0x32, 0xc7, 0x6 } };

class IWmeMediaContentCipher;
class WME_NOVTABLE IWmeMediaTrack : public /*virtual*/ IWmeMediaEventNotifier
{
public:
	/// Get the track type.
	/*!
		\param eType : [out] Track type
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT GetTrackType(WmeTrackType &eType) = 0;

	/// Get role of track
	/*!
		\param eRole : [out] Role type
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT GetTrackRole(WmeTrackRole &eRole) = 0;

	/// Get track label
	/*!
		\param uLabel : [out] Reference of track label
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT GetTrackLabel(uint32_t &uLabel) = 0;

	/// Set track label
	/*!
		\param uLabel : [in] Track label
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT SetTrackLabel(uint32_t uLabel) = 0;
    
	/// Set enable or disable the track
	/*!
		\param bEnabled : [in] Enable value
		\return The error value of the function result
		\note note thread safe
	*/
    virtual WMERESULT SetTrackEnabled(bool bEnabled) = 0;

	/// Get the enable or disable of the track
	/*!
		\param bEnabled : [out] Enable value
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT GetTrackEnabled(bool &bEnabled) = 0;
	
	/// Get the state of the track
	/*!
		\param eState : [out] State value
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT GetTrackState(WmeTrackState &eState) = 0;
        
	/// Set the desired codec type
	/*!
		\param pCodec : [in] Codec type
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT SetCodec(IWmeMediaCodec *pCodec) = 0;
	
	/// Get the desired codec type
	/*!
		\param ppCodec : [out] Codec type
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT GetCodec(IWmeMediaCodec **ppCodec) = 0;
    
    /// Switch the codec type
	/*!
     \param pCodec : [in] Codec type
     \return The error value of the function result
     \note not thread safe
     */
    virtual WMERESULT SwitchCodec(IWmeMediaCodec *pCodec) = 0;
    
    
	/// Set a RTP Channel associated with the track
	/*!
		\param pRTPChannel : [in] RTP channel object
		\return The error value of the function result
		\note not thread safe
	*/
	WME_DEPRECATED virtual WMERESULT SetRTPChannel_deprecated(wrtp::IRTPChannel *pRTPChannel) = 0;

	/// Send an event to track to handle it in sync
	/*!
		\param eEvent : [in] Identifies the event
		\param pValue : [in] Event content
		\param uSize : [in] Size of "pValue", for security check
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT SendEvent(WmeTrackEvent eEvent, void* pValue, uint32_t uSize) = 0;

	/// Set options
	/*!
		\param eOption : [in] Identifies the option
		\param pValue : [in] Option content
		\param uSize : [in] Size of "pValue", for security check
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT SetOption(WmeTrackOption eOption, void* pValue, uint32_t uSize) = 0;

	/// Get options
	/*!
		\param eOption : [in] Identifies the option
		\param pValue : [out] Option content
		\param uSize : [in] Size of "pValue", for security check
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT GetOption(WmeTrackOption eOption, void* pValue, uint32_t uSize) = 0;

	/// Start the track, every component will work
	/*!
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT Start() = 0;

	/// Stop the track, every work will end
	/*!
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT Stop() = 0;

    /// Set media content encryption/decryption callback, mainly for Train backward compatibility
    /*! \param pCipher: [in] Media conent encryption/decryption callback
     \return 0 if success, otherwise fail
     \note not thread safe
    */
    virtual WMERESULT SetMediaContentCipher(IWmeMediaContentCipher* cipher) = 0;
protected:
	/// The destructor function
    virtual ~IWmeMediaTrack(){}
};

/// IWmeVideoRenderProtocol interface
static const WMEIID WMEIID_IWmeVideoRenderProtocol = 
{ 0x1ba751f6, 0xdacd, 0x40bb, { 0xa7, 0x30, 0x12, 0xf7, 0xb1, 0xab, 0x52, 0x68 } };

class WME_NOVTABLE IWmeVideoRenderProtocol
{
public:
	/// Add a display handle for render, with a specified format
	/*!
		\param pWindow : [in] Display handle
		\param pRenderFormat : [in] Format of video raw data
		\return The error value of the funtion result
		\note thread safe
	*/
    virtual WMERESULT AddRenderWindow(void *pWindow, WmeVideoRawFormat *pRenderFormat) = 0;
	
	/// Remove a display handle
	/*!
		\param pWindow : [in] Display handle
		\return The error value of the funtion result
		\note thread safe
	*/
    virtual WMERESULT RemoveRenderWindow(void *pWindow) = 0;
    
	/// Add an external render tunnel, with a specified format
	/*!
		\param pExternalRender : [in] External render tunnel
		\param pRenderFormat : [in] Format of video raw data
		\return The error value of the funtion result
		\note no implementation
	*/
    virtual WMERESULT AddExternalRenderer(IWmeExternalRenderer *pExternalRender, WmeVideoRawFormat *pRenderFormat) = 0;
	
	/// Remove a render tunnel
	/*!
		\param pExternalRender : [in] External render tunnel
		\return The error value of the funtion result
		\note no implementation
	*/
    virtual WMERESULT RemoveExternalRenderer(IWmeExternalRenderer *pExternalRender) = 0;
protected:
	/// The destructor function
	virtual ~IWmeVideoRenderProtocol(){}
};

/// IWmeVideoTrack interface
/*!
	\note	Supported observers: IWmeVideoRenderObserver
*/
static const WMEIID WMEIID_IWmeVideoTrack = 
{ 0x8b5d4798, 0xe2ad, 0x4b9c, { 0x9a, 0xc4, 0x8e, 0xf7, 0xce, 0x9c, 0x32, 0x76 } };

class WME_NOVTABLE IWmeVideoTrack : public IWmeMediaTrack, public IWmeVideoRenderProtocol
{    
public:
	/// Get statistics of track
	/*!
		\param stat : [out] reference of WmeVideoStatistics
		\return The error value of the funtion result
		\note no implementation
	*/
    virtual WMERESULT GetStatistics(WmeVideoStatistics &stat) = 0;
    
protected:
	/// The destructor function
    virtual ~IWmeVideoTrack(){}
};

/// IWmeLocalVideoTrack interface
/*!
	\note	Supported observers: IWmeVideoRenderObserver, IWmeMediaCaptureObserver, IWmeLocalVideoTrackObserver
*/
static const WMEIID WMEIID_IWmeLocalVideoTrack = 
{ 0xaec6b482, 0xf71, 0x4366, { 0xb9, 0x8c, 0xb8, 0xa1, 0xed, 0xfe, 0x77, 0x96 } };

class WME_NOVTABLE IWmeLocalVideoTrack : public IWmeVideoTrack
{
public:
	/// Set the desired video capture device
	/*!
		\param pDevice : [in] Device handle
		\return The error value of the funtion result
		\note not thread safe
	*/
    virtual WMERESULT SetCaptureDevice(IWmeMediaDevice *pDevice) = 0;
	
	/// Get the current video capture device
	/*!
		\param ppDevice : [out] Pointer of device handle
		\return The error value of the funtion result
		\note not thread safe
	*/
    virtual WMERESULT GetCaptureDevice(IWmeMediaDevice **ppDevice) = 0;
    
	/// Set the desired capture format (NEED DISCUSS)
	/*!
		\param pFormat : [in] Format type(eg: picture size, FPS)
		\return The error value of the funtion result
		\note no implementation
	*/
    virtual WMERESULT SetCaptureFormat(WmeVideoRawFormat *pFormat) = 0;
	
	/// Get the current capture format (NEED DISCUSS)
	/*!
		\param format : [out] Reference of WmeVideoRawFormat
		\return The error value of the funtion result
		\note no implementation
	*/
    virtual WMERESULT GetCaptureFormat(WmeVideoRawFormat &format) = 0;
    
protected:
	/// The destructor function
    virtual ~IWmeLocalVideoTrack(){}
};

/// IWmeRemoteVideoTrack interface
/*!
	\note	Supported observers: IWmeVideoRenderObserver, IWmeRemoteVideoTrackObserver
*/
static const WMEIID WMEIID_IWmeRemoteVideoTrack = 
{ 0x1176f2d0, 0xa5dc, 0x4981, { 0xa5, 0x61, 0x6e, 0x4b, 0x9d, 0xff, 0xc5, 0xd3 } };

class WME_NOVTABLE IWmeRemoteVideoTrack : public IWmeVideoTrack
{
public:
	

protected:
	/// The destructor function
	virtual ~IWmeRemoteVideoTrack(){}
};

/// IWmeLocalVideoExternalTrack interface
/*!	
	\note	Video data is send to track from outside.
			Supported observers: IWmeVideoRenderObserver, IWmeLocalVideoTrackObserver
*/
static const WMEIID WMEIID_IWmeLocalVideoExternalTrack = 
{ 0x130f27b7, 0x274f, 0x428e, { 0x9c, 0x7d, 0xd2, 0x7e, 0x19, 0x55, 0x23, 0xa8 } };

class WME_NOVTABLE IWmeLocalVideoExternalTrack : public IWmeVideoTrack
{
public:
	/// Get inputter handle which is used for data input
	/*!
		\param ppInputter : [out] Pointer of inputter handle
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT GetExternalInputter(IWmeExternalInputter **ppInputter) = 0;
    virtual WMERESULT SetDefaultSVSConfig() = 0;
    virtual WMERESULT SetResolutionInfo(WmeVideoSourceInfoType* pInfo) = 0;
protected:
	/// The destructor function
    virtual ~IWmeLocalVideoExternalTrack(){}
};

/// IWmeVideoPreviewTrack interface
/*!	
	\note	Supported observers: IWmeVideoRenderObserver, IWmeMediaCaptureObserver
*/
static const WMEIID WMEIID_IWmeVideoPreviewTrack = 
{ 0x94318bd1, 0xd0a2, 0x4fe0, { 0xba, 0xfb, 0x73, 0x27, 0x3f, 0x89, 0x1c, 0x11 } };

class WME_NOVTABLE IWmeVideoPreviewTrack : public IWmeVideoTrack {
public:
	/// Set the desired video capture device
	/*!
		\param pDevice : [in] Device handle
		\return The error value of the funtion result
		\note not thread safe
	*/
    virtual WMERESULT SetCaptureDevice(IWmeMediaDevice *pDevice) = 0;
	
	/// Get the current video capture device
	/*!
		\param ppDevice : [out] Pointer of device handle
		\return The error value of the funtion result
		\note not thread safe
	*/
    virtual WMERESULT GetCaptureDevice(IWmeMediaDevice **ppDevice) = 0;

	/// Set the desired capture format (NEED DISCUSS)
	/*!
		\param pFormat : [in] Format type(eg: picture size, FPS)
		\return The error value of the funtion result
		\note not thread safe
	*/
    virtual WMERESULT SetCaptureFormat(WmeVideoRawFormat *pFormat) = 0;
	
	/// Get the current capture format (NEED DISCUSS)
	/*!
		\param format : [out] Reference of WmeVideoRawFormat
		\return The error value of the funtion result
		\note not thread safe
	*/
    virtual WMERESULT GetCaptureFormat(WmeVideoRawFormat &format) = 0;

protected:
	/// The destructor function
	virtual ~IWmeVideoPreviewTrack(){}
};

/// IWmeAudioTrack interface
/*!	
	\note	Supported observers: None
*/
static const WMEIID WMEIID_IWmeAudioTrack = 
{ 0x983e4448, 0x12fe, 0x4e75, { 0x8e, 0x67, 0x9d, 0xd4, 0x33, 0x6b, 0x25, 0xe } };

class WME_NOVTABLE IWmeAudioTrack : public IWmeMediaTrack
{
public:
	/// Get statistics of track
	/*!
		\param stat : [out] reference of WmeAudioStatistics
		\return The error value of the funtion result
		\note no implementation
	*/
    virtual WMERESULT GetStatistics(WmeAudioStatistics &stat) = 0;
    virtual WMERESULT GetVoiceLevel(uint32_t &level)=0;
    
protected:
	/// The destructor function
    virtual ~IWmeAudioTrack(){}
};

/// IWmeLocalAudioTrack interface
/*!	
	\note	Supported observers: None
*/
static const WMEIID WMEIID_IWmeLocalAudioTrack = 
{ 0xa2842be, 0x4a50, 0x4899, { 0x9f, 0xa, 0xf7, 0xa2, 0x64, 0xcf, 0xc3, 0x12 } };


/// IWmeAudioRenderProtocol interface
static const WMEIID WMEIID_IWmeAudioRenderProtocol = // {582EB810-23F8-48be-9425-CD3F8F2D4B25}
{ 0x582eb810, 0x23f8, 0x48be, { 0x94, 0x25, 0xcd, 0x3f, 0x8f, 0x2d, 0x4b, 0x25 } };

class WME_NOVTABLE IWmeAudioRenderProtocol
{
public:
	/// Add an external render tunnel, with a specified format
	/*!
	\param pExternalRender : [in] External render tunnel
	\param pRenderFormat : [in] Format of video raw data
	\return The error value of the funtion result
	\note no implementation
	*/
	virtual WMERESULT AddExternalRenderer(IWmeExternalRenderer *pExternalRender, WmeAudioRawFormat *pRenderFormat) = 0;

	/// Remove a render tunnel
	/*!
	\param pExternalRender : [in] External render tunnel
	\return The error value of the funtion result
	\note no implementation
	*/
	virtual WMERESULT RemoveExternalRenderer(IWmeExternalRenderer *pExternalRender) = 0;

protected:
	/// The destructor function
	virtual ~IWmeAudioRenderProtocol(){}
};

class WME_NOVTABLE IWmeLocalAudioTrack : public IWmeAudioTrack, public IWmeAudioRenderProtocol
{
public:
	/// Get the current audio capture device
	/*!
		\param ppDevice : [out] Return a pointer of the device handle
		\return The error value of the funtion result
		\note no implementation
	*/
    virtual WMERESULT GetCaptureDevice(IWmeMediaDevice **ppDevice) = 0;
	
	/// Get the current audio raw format
	/*!
		\param format : [out] Reference of WmeAudioRawFormat
		\return The error value of the funtion result
		\note no implementation
	*/
    virtual WMERESULT GetCaptureFormat(WmeAudioRawFormat &format) = 0;
    
protected:
	/// The destructor function
    virtual ~IWmeLocalAudioTrack(){}
};
// IWmeRemoteAudioMixTrack interface
/*!	
	\note	Supported observers: None
*/
static const WMEIID  WMEIID_IWmeRemoteAudioMixTrack= 
{ 0x9db5a407, 0x18, 0x49a3, { 0xa3, 0x8e, 0x4b, 0x7a, 0xaf, 0xa2, 0x2e, 0x18 } };

class WME_NOVTABLE IWmeRemoteAudioMixTrack : public IWmeAudioTrack,public IWmeAudioRenderProtocol
{
public:
	/// Get media device
	/*!
		\param ppDevice : [out] Return the device handle
		\return The error value of the funtion result
		\note no implementation
	*/
    virtual WMERESULT GetPlayDevice(IWmeMediaDevice **ppDevice) = 0;
	
	/// Get format of audio raw data
	/*!
		\param format : [out] Return audio raw format
		\return The error value of the funtion result
		\note no implementation
	*/
    virtual WMERESULT GetPlayFormat(WmeAudioRawFormat &format) = 0;
    
protected:
	/// The destructor function
    virtual ~IWmeRemoteAudioMixTrack(){}
};

/// IWmeRemoteAudioTrack interface
/*!	
	\note	Supported observers: None
*/
static const WMEIID WMEIID_IWmeRemoteAudioTrack = 
{ 0xa8e20d36, 0xdfff, 0x4a03, { 0x93, 0x17, 0x16, 0x34, 0xce, 0x12, 0x83, 0xae } };

class WME_NOVTABLE IWmeRemoteAudioTrack : public IWmeAudioTrack,public IWmeAudioRenderProtocol
{
public:
	/// Get media device
	/*!
		\param ppDevice : [out] Return the device handle
		\return The error value of the funtion result
		\note no implementation
	*/
    virtual WMERESULT GetPlayDevice(IWmeMediaDevice **ppDevice) = 0;
	
	/// Get format of audio raw data
	/*!
		\param format : [out] Return audio raw format
		\return The error value of the funtion result
		\note no implementation
	*/
    virtual WMERESULT GetPlayFormat(WmeAudioRawFormat &format) = 0;
	/// Get remote audio mix track 
	/*!
		\param ppTrack : [out] Return mix track
		\return The error value of the funtion result
		\note thread safe
	*/
    virtual WMERESULT GetPlayMixTrack(IWmeRemoteAudioMixTrack **ppTrack) = 0;
    /// set or get audio play buffer min and max delay
    /*!
     \param int nMin,int nMax in ms, the max value should not exceed 5000ms
     \return The error value of the funtion result
     \note thread safe
     */
    virtual WMERESULT SetPlayBufferTime(int nMin,int nMax) = 0;
    virtual WMERESULT GetPlayBufferTime(int &nMin,int &nMax) = 0;
protected:
	/// The destructor function
    virtual ~IWmeRemoteAudioTrack(){}
};

/// IWmeLocalAudioExternalTrack interface
/*!	
	\note	Audio data is send to track from outside
			Supported observers: None
*/
static const WMEIID WMEIID_IWmeLocalAudioExternalTrack = 
{ 0x563500df, 0x6d95, 0x429a, { 0xa0, 0xf4, 0x38, 0x30, 0x2f, 0xb0, 0xb8, 0xa8 } };

class WME_NOVTABLE IWmeLocalAudioExternalTrack : public IWmeAudioTrack
{
public:
	/// Get inputter handle which is used for data input
	/*!
		\param ppInputter : [out] Pointer of inputter handle
		\return The error value of the funtion result
		\note no implementation
	*/
    virtual WMERESULT GetExternalInputter(IWmeExternalInputter **ppInputter) = 0;
protected:
	/// The destructor function
    virtual ~IWmeLocalAudioExternalTrack(){}
};

/// IWmeRemoteAudioExternalTrack interface
/*!	
	\note	Supported observers: None
*/
static const WMEIID WMEIID_IWmeRemoteAudioExternalTrack = // {5F090F6C-E015-4c45-87C3-CC800E20621A}
{ 0x5f090f6c, 0xe015, 0x4c45, { 0x87, 0xc3, 0xcc, 0x80, 0xe, 0x20, 0x62, 0x1a } };

class WME_NOVTABLE IWmeRemoteAudioExternalTrack : public IWmeAudioTrack,public IWmeAudioRenderProtocol
{
public:

	/// Get format of audio raw data
	/*!
		\param format : [out] Return audio raw format
		\return The error value of the funtion result
		\note no implementation
	*/
    virtual WMERESULT GetPlayFormat(WmeAudioRawFormat &format) = 0;
	/// Get remote audio mix track 
	/*!
		\param ppTrack : [out] Return mix track
		\return The error value of the funtion result
		\note thread safe
	*/
    virtual WMERESULT GetPlayMixTrack(IWmeRemoteAudioMixTrack **ppTrack) = 0;
    
protected:
	/// The destructor function
    virtual ~IWmeRemoteAudioExternalTrack(){}
};
/// IWmeDataTrack interface
/*!	
	\note	Supported observers: None
*/
static const WMEIID WMEIID_IWmeDataTrack = 
{ 0x75abf500, 0xa369, 0x4cf4, { 0xb2, 0x9c, 0x70, 0xee, 0xda, 0xdb, 0xa2, 0x89 } };

class WME_NOVTABLE IWmeDataTrack : public IWmeMediaTrack
{    
public:
	/// Get inputter handle which is used for data input
	/*!
		\param ppInputter : [out] Pointer of inputter handle
		\return The error value of the function result
		\note thread safe
	*/
    virtual WMERESULT GetExternalInputter(IWmeExternalInputter **ppInputter) = 0;

	/// Set outputter handle which is used for data output
	/*!
		\param ppOutputter : [out] Pointer of outputter handle
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT SetExternalOutputter(IWmeExternalOutputter *pOutputter) = 0;
    
protected:
	/// The destructor function
    virtual ~IWmeDataTrack(){}
};

//=======================================================================
//Screen Sharing Track >> Start
//========================================================================
/// IWmeScreenShareTrack interface
/*!	
	\note	Supported observers: IWmeVideoRenderObserver
*/
static const WMEIID WMEIID_IWmeScreenShareTrack = 
{ 0x4dd2425d, 0xf7ea, 0x471f, { 0xb8, 0xaa, 0xd4, 0xb7, 0x8, 0xa2, 0x2e, 0xd9 } };

class WME_NOVTABLE IWmeScreenShareTrack : public IWmeVideoTrack
{    
public:
	using IWmeVideoTrack::GetStatistics;
	/// Get statistics of track
	/*!
		\param stStat : [out] reference of WmeScreenShareStatistics
		\return The error value of the function result
		\note no implementation
	*/
    virtual WMERESULT GetStatistics(WmeScreenShareStatistics &stStat) = 0;
    
protected:
	/// The destructor function
    virtual ~IWmeScreenShareTrack(){}
};


/// IWmeLocalScreenShareTrack interface
/*!	
	\note	Supported observers: IWmeVideoRenderObserver, IWmeLocalVideoTrackObserver
*/
 
// {DB18F75B-E006-4cfb-8B12-2E6269CF49D6}
static const WMEIID WMEIID_IWmeLocalScreenShareTrack =
{ 0xdb18f75b, 0xe006, 0x4cfb, { 0x8b, 0x12, 0x2e, 0x62, 0x69, 0xcf, 0x49, 0xd6 } };

class IWmeLocalScreenShareTrack : public IWmeScreenShareTrack
{
public:
	virtual WMERESULT AddScreenSource(IWmeScreenSource *pSource) = 0;
	virtual WMERESULT RemoveScreenSource(IWmeScreenSource *pSource) = 0;
	virtual WMERESULT IsDesktopSharing(bool &bSharing) =0;
	virtual WMERESULT IsApplicationShared(WmeWindowHandle hWindow, uint32_t uProcessID, bool &bSharing) =0;
	virtual WMERESULT GetSharedApplicationNumber(int32_t &iNumber) =0;
    virtual WMERESULT SetScreenCapturer(IWmeScreenCapturer * pIWmeScreenCapturer)=0;
protected:
	/// The destructor function
	virtual ~IWmeLocalScreenShareTrack(){}
};


/// IWmeLocalScreenShareExternalTrack interface
/*!	
	\note	Supported observers: IWmeVideoRenderObserver, IWmeLocalVideoTrackObserver
*/
static const WMEIID WMEIID_IWmeLocalScreenShareExternalTrack =  
{ 0xddc3f702, 0x4ce7, 0x4c17, { 0x9d, 0xa4, 0x20, 0x4f, 0xf7, 0x10, 0x6c, 0x9f } };
class IWmeLocalScreenShareExternalTrack : public IWmeLocalScreenShareTrack
{
public:
	//=====External Capturer====
	virtual WMERESULT SetCapturer(IWmeExternalCapturer * pIWmeExternalCapturer)=0;
	virtual WMERESULT SetCaptureEngine(IWmeScreenCaptureEngine * pIWmeScreenCaptureEngine)=0;
	//virtual WMERESULT IWmeLocalVideoExternalTrack::GetExternalInputter(IWmeExternalInputter **ppInputter) = 0;

	//=====External Codec====
	//virtual WMERESULT IWmeMediaTrack::SetCodec(IWmeMediaCodec *pCodec) = 0;

protected:
	/// The destructor function
	virtual ~IWmeLocalScreenShareExternalTrack(){}
};

/// IWmeRemoteScreenShareTrack interface
/*!	
	\note	Supported observers: IWmeVideoRenderObserver, IWmeRemoteVideoTrackObserver
*/
WME_DEPRECATED typedef   IWmeRemoteVideoTrack    IWmeRemoteScreenShareTrack;
/*
// {1ED00565-F654-4e1a-AB6C-792A72057B1F}
static const WMEIID WMEIID_IWmeRemoteScreenShareTrack =  
{ 0x1ed00565, 0xf654, 0x4e1a, { 0xab, 0x6c, 0x79, 0x2a, 0x72, 0x5, 0x7b, 0x1f } };
class IWmeRemoteScreenShareTrack : public IWmeScreenShareTrack
{
public:
protected:
	/// The destructor function
	virtual ~IWmeRemoteScreenShareTrack(){}
};
*/
/// IWmeLocalScreenShareDataTrack interface
/*!	
	\note	Supported observers: None
*/
WME_DEPRECATED typedef		IWmeDataTrack					IWmeLocalScreenShareDataTrack;

/// IwmeRemoteScreenShareDataTrack interface
/*!	
	\note	Supported observers: None
*/
WME_DEPRECATED typedef		IWmeDataTrack					IWmeRemoteScreenShareDataTrack;


/// IWmeCompositeMediaTrack
/*!	
	\note	Supported observers: None
*/
static const WMEIID WMEIID_IWmeCompositeMediaTrack = 
{ 0xd7b210b6, 0xa343, 0x4486, { 0xbb, 0xb6, 0xc7, 0x9d, 0xce, 0x7a, 0x27, 0x72 } };

class WME_NOVTABLE IWmeCompositeMediaTrack : public IWmeMediaTrack
{    
public:
	/// Get the total number of basic track type in the composite track
	/*!
		\param iNumber : [out] Reference of number
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT GetBasicTrackTypeNumber(int32_t &iNumber) = 0;


	/// Get basic track type by index in the composite track
	/*!
		\param iIndex : [in] Index of track type array
		\param eType : [out] Reference of basic track type
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT GetBasicTrackType(int32_t iIndex, WmeTrackType &eType) = 0;

protected:
	/// The destructor function
	virtual ~IWmeCompositeMediaTrack(){}
};


/// IWmeLocalCompositeScreenShareTrack interface
/*!	
	\note	Supported observers: IWmeVideoRenderObserver, IWmeScreenCaptureEngineEventObserver, IWmeLocalVideoTrackObserver
*/
static const WMEIID WMEIID_IWmeLocalCompositeScreenShareTrack = 
{ 0x65b43872, 0x31c5, 0x4395, { 0xbc, 0xd0, 0x4b, 0xbe, 0x88, 0xe9, 0xaa, 0x98 } };

class WME_NOVTABLE IWmeLocalCompositeScreenShareTrack : public IWmeCompositeMediaTrack
{    
public:
	/// Add screen source
	/*!
		\param pSource : [in] Screen source handle
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT AddScreenSource(IWmeScreenSource *pSource) = 0;

	/// Remove screen source
	/*!
		\param ppSource : [in] Screen source handle
		\return The error value of the function result
		\note not thread safe
	*/
    virtual WMERESULT RemoveScreenSource(IWmeScreenSource *ppSource) = 0;

	/// Check whether desktop is sharing in the screen share track 
	/*!
        \param bSharing : [out] Sharing result
        \return The error value of the function result
        \note not thread safe
    */
	virtual WMERESULT IsDesktopSharing(bool &bSharing) = 0;
    
    /// Check whether the application is sharing in the screen share track
	/*!
        \param bSharing : [out] Sharing result
        \return The error value of the function result
        \note not thread safe
    */
	virtual WMERESULT IsApplicationShared(WmeWindowHandle hWindow, uint32_t uProcessID, bool &bSharing) = 0;
    
    /// Get total number of shared applications
	/*!
        \param iNumber : [out] Total number of shared application sources
        \return The error value of the function result
        \note not thread safe
    */
	virtual WMERESULT GetSharedApplicationNumber(int32_t &iNumber) = 0;

protected:
	/// The destructor function
	virtual ~IWmeLocalCompositeScreenShareTrack(){}
};

/// IWmeRemoteCompositeScreenShareTrack interface
/*!	
	\note	Supported observers: IWmeVideoRenderObserver, IWmeRemoteVideoTrackObserver
*/
static const WMEIID WMEIID_IWmeRemoteCompositeScreenShareTrack = 
{ 0x2466ed4e, 0x182d, 0x4d6d, { 0x9b, 0x19, 0x3b, 0x9b, 0x6c, 0x15, 0x3d, 0x4f } };

class WME_NOVTABLE IWmeRemoteCompositeScreenShareTrack : public IWmeCompositeMediaTrack
{    
public:
	

protected:
	/// The destructor function
	virtual ~IWmeRemoteCompositeScreenShareTrack(){}
};

///============================Media Track Observer============================

/// IWmeMediaCaptureObserver interface
static const WMEIID WMEIID_IWmeMediaCaptureObserver = 
{ 0x9795aad4, 0xf29a, 0x4c47, { 0x9a, 0xa6, 0x7c, 0x72, 0x14, 0xc2, 0xe4, 0x92 } };

class WME_EXPORT WME_NOVTABLE IWmeMediaCaptureObserver : virtual public IWmeMediaEventObserver
{
public:
	/// Return capture start result asynchronously to application
	/*!
		\param pNotifier : [in] Caller's handle
		\param eStartedResult : [in] The result of capture start asynchronously
		\return The error value of the function result
	*/
	virtual WMERESULT OnStartedCapture(IWmeMediaEventNotifier *pNotifier, WMERESULT eStartedResult) = 0;

	/// Return picture width and height
	/*!
		\param pNotifier : [in] Caller's handle
		\param uWidth : [in] Picture width
		\param uHeight: [in] Picture height
		\return The error value of the function result
	*/
	virtual WMERESULT OnUpdateCaptureResolution(IWmeMediaEventNotifier *pNotifier, uint32_t uWidth, uint32_t uHeight) = 0;		//result come from capture engine

	/// Return recent capture FPS
	/*!
		\param pNotifier : [in] Caller's handle
		\param uFPS : [in] Capture framerate per second
		\return The error value of the function result
	*/
	virtual WMERESULT OnUpdateCaptureFPS(IWmeMediaEventNotifier *pNotifier, uint32_t uFPS) = 0;		//result come from capture engine
    
    /// Camera runtime fatal error which cannot be recovered
    /*!
        \param pNotifier : [in] Caller's handle
     */
    virtual WMERESULT OnCameraRuntimeDie(IWmeMediaEventNotifier *pNotifier) = 0;
};

/// IWmeVideoRenderObserver interface
static const WMEIID WMEIID_IWmeVideoRenderObserver = 
{ 0x299140c3, 0x225, 0x40cd, { 0x8b, 0xa8, 0x62, 0x51, 0xcc, 0xc8, 0x55, 0x49 } };

class WME_EXPORT WME_NOVTABLE IWmeVideoRenderObserver : virtual public IWmeMediaEventObserver
{
public:
	/// Request DC to observer(application)
	/*!
		\param pNotifier : [in] Caller's handle
		\param uRequestedID : [in] A source ID needed render in by caller
		\return The error value of the function result
	*/
	virtual WMERESULT OnRequestedDC(IWmeMediaEventNotifier *pNotifier, uint32_t uRequestedID) = 0;
};

/// IWmeLocalVideoTrackObserver interface
static const WMEIID WMEIID_IWmeLocalVideoTrackObserver = 
{ 0xdfab315, 0x6b43, 0x4478, { 0xb3, 0xe, 0x46, 0x72, 0x10, 0x47, 0xf2, 0xa1 } };

class WME_EXPORT WME_NOVTABLE IWmeLocalVideoTrackObserver : virtual public IWmeMediaEventObserver
{
public:
	/// Required frame rate update
	/*!
		\param fRequiredFrameRate : [in] Required frame rate
		\return The error value of the function result
	*/
	virtual WMERESULT OnRequiredFrameRateUpdate(float fRequiredFrameRate) = 0;

	/// Encoded size update
	/*!
		\param uWidth : [in] frame width
		\param uHeight : [in] frame height
		\return The error value of the function result
	*/
	virtual WMERESULT OnEncodedResolutionUpdate(uint32_t uWidth, uint32_t uHeight) = 0;

	/// Encoded FPS update
	/*!
		\param uFPS : [in] frame rate
		\return The error value of the function result
	*/
	virtual WMERESULT OnEncodedFrameRateUpdate(uint32_t uFPS) = 0;
    
    /// Stream based bandwidth update
    /*!
        \param 
        \return The error value of the function result
    */
    virtual WMERESULT OnStreamBandwidthUpdate(WmeAllStreamsBandwidth* pAllStreams) { return WME_E_NOTIMPL; }
    
    virtual WMERESULT OnLocalVideoTrackError(uint32_t nErrorCode) { return WME_E_NOTIMPL; }

    /// Selfview Size update
    virtual WMERESULT OnSelfviewSizeUpdate(WmeVideoSizeInfo *pSizeInfo) = 0;
};

/// IWmeRemoteVideoTrackObserver interface
static const WMEIID WMEIID_IWmeRemoteVideoTrackObserver = 
{ 0xc4ae49e0, 0x9806, 0x41d1, { 0x8e, 0xc3, 0xdf, 0x56, 0xaa, 0xd3, 0xc9, 0x99 } };

class WME_EXPORT WME_NOVTABLE IWmeRemoteVideoTrackObserver : virtual public IWmeMediaEventObserver
{
public:
	/// KeyFrame lost happen
	/*!
		\param uLabel : [in] Label of the video track that has KeyFrame lost
		\param uDID	  : [in] ID of the spatial layer with Key Frame lost
		\return The error value of the function result
	*/
	virtual WMERESULT OnKeyFrameLost(uint32_t uLabel, uint32_t uDID) = 0;

	/// Render blocked or not
	/*!
		\param uLabel : [in] Label of the video track whose render status has changed
		\param bBlocked : [in] blocked or not
		\return The error value of the function result
	*/
	virtual WMERESULT OnRenderBlocked(uint32_t uLabel, bool bBlocked) = 0;

	/// Decoded frame size changed
	/*!
		\param uLabel : [in] Label of the video track whose decode frame size has changed
		\param uWidth : [in] Decode frame width
		\param uHeight : [in] Decode frame height
		\return The error value of the function result
	*/
	virtual WMERESULT OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight) = 0;

	/// Render size changed when decoded frame size changed or CVO changed.
	/*!
		\param uLabel : [in] Label of the video track whose decode frame size has changed
		\param uWidth : [in] Render width
		\param uHeight : [in] Render height
		\return The error value of the function result
	*/
	virtual WMERESULT OnRemoteSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight)  {return WME_E_NOTIMPL;}
    
    virtual WMERESULT OnRemoteVideoTrackError(unsigned long vid, uint32_t nErrorCode) { return WME_E_NOTIMPL; }
    
    /// Render indication
    /*!
     \param uLabel : [in] Label of the video track who has rendered just now
     \return The error value of the function result
     */
    virtual WMERESULT OnRenderIndication(uint32_t uLabel) { return WME_E_NOTIMPL; }
};
    
// IWmeRemoteAudioTrackObserver interface
static const WMEIID WMEIID_IWmeRemoteAudioTrackObserver =
{ 0x7b1d749, 0xb7ff, 0x40d6, { 0xa8, 0x91, 0x4a, 0x5f, 0xbd, 0x3, 0xf3, 0xce } };
    
class WME_EXPORT WME_NOVTABLE IWmeRemoteAudioTrackObserver : virtual public IWmeMediaEventObserver
{
public:
    /// Playback timestamp updated
    /*!
         \param uLabel : [in] Label of the audio track that has update playback timestamp
         \param uPlaybackTimestamp	  : [in] Time stamp of the audio track playback
         \return The error value of the function result
     */
    virtual WMERESULT OnPlaybackTimestampUpdated(uint32_t uLabel, uint32_t uPlaybackTimestamp) = 0;
};

}	//namespace

#endif // WME_TRACK_INTERFACE_H
