///
///  WmeSession.h
///
///
///  Created by Sand Pei on 2013-6-25.
///  Copyright (c) 2013 Cisco. All rights reserved.
///


/** @defgroup WmeSession Wme Session
  * @{
  *
  * @brief This page describes the process and functions WmeSession.
  *
  * Since Opus is a stateful codec, the encoding process starts with creating an encoder
  * state. This can be done with:
  *
  * @code
  * int          error;
  * OpusEncoder *enc;
  * enc = opus_encoder_create(Fs, channels, application, &error);
  * @endcode
  *
  * From this point, @c enc can be used for encoding an audio stream. An encoder state
  * @b must @b not be used for more than one stream at the same time. Similarly, the encoder
  * state @b must @b not be re-initialized for each frame.
  *
  * While opus_encoder_create() allocates memory for the state, it's also possible
  * to initialize pre-allocated memory:
  *
  * @code
  * int          size;
  * int          error;
  * OpusEncoder *enc;
  * size = opus_encoder_get_size(channels);
  * enc = malloc(size);
  * error = opus_encoder_init(enc, Fs, channels, application);
  * @endcode
  *
  * where opus_encoder_get_size() returns the required size for the encoder state. Note that
  * future versions of this code may change the size, so no assuptions should be made about it.
  *
  * The encoder state is always continuous in memory and only a shallow copy is sufficient
  * to copy it (e.g. memcpy())
  *
  * It is possible to change some of the encoder's settings using the opus_encoder_ctl()
  * interface. All these settings already default to the recommended value, so they should
  * only be changed when necessary. The most common settings one may want to change are:
  *
  * @code
  * opus_encoder_ctl(enc, OPUS_SET_BITRATE(bitrate));
  * opus_encoder_ctl(enc, OPUS_SET_COMPLEXITY(complexity));
  * opus_encoder_ctl(enc, OPUS_SET_SIGNAL(signal_type));
  * @endcode
  *
  * where
  *
  * @arg bitrate is in bits per second (b/s)
  * @arg complexity is a value from 1 to 10, where 1 is the lowest complexity and 10 is the highest
  * @arg signal_type is either OPUS_AUTO (default), OPUS_SIGNAL_VOICE, or OPUS_SIGNAL_MUSIC
  *
  * See @ref opus_encoderctls and @ref opus_genericctls for a complete list of parameters that can be set or queried. Most parameters can be set or changed at any time during a stream.
  *
  * To encode a frame, opus_encode() or opus_encode_float() must be called with exactly one frame (2.5, 5, 10, 20, 40 or 60 ms) of audio data:
  * @code
  * len = opus_encode(enc, audio_frame, frame_size, packet, max_packet);
  * @endcode
  *
  * where
  * <ul>
  * <li>audio_frame is the audio data in opus_int16 (or float for opus_encode_float())</li>
  * <li>frame_size is the duration of the frame in samples (per channel)</li>
  * <li>packet is the byte array to which the compressed data is written</li>
  * <li>max_packet is the maximum number of bytes that can be written in the packet (4000 bytes is recommended)</li>
  * </ul>
  *
  * opus_encode() and opus_encode_frame() return the number of bytes actually written to the packet.
  * The return value <b>can be negative</b>, which indicates that an error has occurred. If the return value
  * is 1 byte, then the packet does not need to be transmitted (DTX).
  *
  * Once the encoder state if no longer needed, it can be destroyed with
  *
  * @code
  * opus_encoder_destroy(enc);
  * @endcode
  *
  * If the encoder was created with opus_encoder_init() rather than opus_encoder_create(),
  * then no action is required aside from potentially freeing the memory that was manually
  * allocated for it (calling free(enc) for the example above)
  *
  */


#ifndef WME_SESSION_INTERFACE_H
#define WME_SESSION_INTERFACE_H

#include "WmeUnknown.h"
#include "WmeTrack.h"

namespace wme
{
/// An enum of media type
typedef enum
{
	WmeSessionType_Audio = 0,
	WmeSessionType_Video,
    WmeSessionType_ScreenShare,
    WmeSessionType_Unknown = -1
}WmeSessionType;

/// An enum of media option
typedef enum
{
	WmeSessionOption_EnableRTCP = 0,				///> The enable RTCP flag. Value type: bool
	WmeSessionOption_EnableQOS,						///> The enable QOS flag. Value type: bool
    WmeSessionOption_PauseQOS,						///> The pause QoS flag. Value type: bool
	WmeSessionOption_RTPFecInfo,					///> The RTP FEC info. Value type: WmeRTPFecInfo
	WmeSessionOption_RTPExtension,					///> The RTP extension. Value type: WmeRTPExtension
	WmeSessionOption_CodecPayloadTypeMap,			///> The Codec payload type map. Value type: WmeCodecPayloadTypeMap
	WmeSessionOption_SecurityConfiguration,			///> The security configuration. Value type: WmeSecurityDirection
	WmeSessionOption_InitialBandwidth,				///> The bytes per second. Value type: uint32_t
	WmeSessionOption_MaxPacketSize,					///> The MTU size (Bytes). Value type: uint32_t
	WmeSessionOption_PacketizationMode,				///> The packetization mode. Value type: WmePacketizationMode
	WmeSessionOption_MaxBandwidth,					///> The max bytes per second. Value type: uint32_t. Indicated by SDP: b=AS:<bandwidth>.
                                                    ///  Note that <bandwidth> is interpreted as kilobits per second by default. Need unit conversion
	WmeSessionOption_EnableSendingFilterFeedback,	///< The flag to disable sending filter feedback. Default is enabled, if you don't set it. Value type: bool.
    WmeSessionOption_RTPExtensionEx,				///> The RTP extension. Value type: WmeRTPExtensionEx
	WmeSessionOption_EnableCiscoSCR,                ///> The enable Cisco SCR/SCA flag. Value type: bool. Negotiated via SDP: a=rtcp-fb:* ccm cisco-scr
	WmeSessionOption_SetQosLabel,					///> The QoS peerID, it is a string in type of char*
    WmeSessionOption_MaxRTPPayloadSize,             ///> The max size of the RTP payload. Value type: uint32_t

    WmeSessionOption_MaxFecOverhead,			    ///> The max fec overhead, this is one percentage value, 25 stands for 25%. Value type: uint32_t, it is tmp api for client.
    WmeSessionOption_FecProbing,			        ///> Control fec probing feature
    WmeSessionOption_DynamicFecLevelScheme,         ///> Set dynamic fec level scheme
    WmeSessionOption_SetQosPriority,                ///> Set Qos priority. Value type: INT8
    WmeSessionOption_StreamBandwidth,
    WmeSessionOption_MaxSendingDelay,               ///> Set Max Sending delay(ms). Value type: UINT32
    
    WmeSessionOption_FeatureToggles,                ///> Set a variety of feature toggles
    
    WmeSessionOption_RecordLossData,                ///> Enable record loss raw data
    
	WmeSessionOption_CodecPayloadTypeMap_Ext = 2000, ///> The Codec payload type map with in/out stream indication. Value type : WmeCodecPayloadTypeMap
}WmeSessionOption;

/// An enum of RTP FEC type
typedef enum
{
	WmeRTPFecNone = 0,
	WmeRTPFecRs,
	WmeRTPFecXor
} WmeRTPFecType;

typedef enum {
    ONLY_LOSS_RATIO,
    LOSS_RATIO_WITH_PRIORITY
} DynamicFECLevelScheme;

/*
*  Choose stream dir for session configuration
*/
enum eStreamDirection {
	STREAM_NONE = 0,
	STREAM_OUT = 1,
	STREAM_IN = 2,
	STREAM_INOUT = 3 //BOTH
};

/// An enum of security FEC order
typedef enum
{
    WmeSecurityFECOrder_SRTP_FEC,	///> Only SRTP_FEC is supported
    WmeSecurityFECOrder_FEC_SRTP,	///> Not supported currently
} WmeSecurityFECOrder;

typedef struct
{
    bool bMultiSsrc;
    uint8_t uM;                     ///> The length of the elements in the finite field, in bits
    uint32_t uMaxEsel;              ///> The maximum size of a source symbol in bytes
    uint32_t uMaxN;                 ///> The upper limit on the number of encoding symbols
    WmeSecurityFECOrder srtpFecOrder; ///> The order of srtp/fec
} WmeRtpFecFmtInfo;

typedef struct
{
    uint8_t uVersion;
    uint8_t uPayloadType;
    WmeCodecType uCodecType;
    uint32_t uClockRate;
    WmeRTPFecType uFecType;
    WmeRtpFecFmtInfo fmtInfo;
} WmeRtpMariFecInfo;
    
typedef struct
{
    WmeRtpMariFecInfo local;
    WmeRtpMariFecInfo remote;
} WmeRtpMariFecInfoEx;
    
/// An struct of RTP extension
typedef struct  
{
	char* sExtURI;
	uint8_t uExtID;
}WmeRTPExtension;
    
/// An struct of RTP extension
typedef struct
{
    char* sExtURI;
    uint8_t uExtID;
    eStreamDirection uDirection;
}WmeRTPExtensionEx;

/// The signature of the callback function that iterates all built-in supported RTP header extension 
/*!
	\param sExtURI: [in] The URI name of the RTP header extension
	\return The error value of the function result
*/
typedef int32_t (*WmeBuiltinRTPExtensionIterFunc)(const char* sExtURI);

/// The type of the function to build an RTP header extension. The function will be called during RTP packet encoding.
/*!
	\param sExtURI: [in] The URI name of the RTP header extension
	\param pData: [in] The media data to be sent
	\param pExtBuffer: [in, out] The buffer to store the header extension
	\param uLength: [in, out] The length of the header extension
	\return The error value of the function result
*/
typedef int32_t (*WmeExtensionBuildFunc)(const char* sExtURI, /*wrtp::WRTPMediaData*/void* pData, uint8_t pExtBuffer[], uint32_t &uLength);

/// The type of the function to parse an RTP header extension. The function will be called during RTP packet decoding.
/*!
	\param sExtURI: [in] The URI name of the RTP header extension
	\param pExtBuffer: [in] The buffer to store the header extension
	\param uLength: [in] The length of the header extension
	\param pData: [in, out] The media data to be delivered to media engine, the parsed header extension will be stored in this structure
	\return The error value of the function result
*/
typedef int32_t (*WmeExtensionParseFunc)(const char* sExtURI, uint8_t pExtBuffer[], uint32_t uLength, /*wrtp::WRTPMediaData*/void* pData);

/// An struct of codec payload type map
typedef struct
{
	WmeCodecType eCodecType;	///< The encoding type
	uint8_t uPayloadType;		///< The RTP payload type related to encoding type
	uint32_t uClockRate;		///< The RTP clock rate related to encoding type, unit: Hz
}WmeCodecPayloadTypeMap;

/// An struct of codec payload type map base on stream direction 
typedef struct
{
	WmeCodecPayloadTypeMap wmecptMap;
	eStreamDirection dir; //stream direction
}WmeCodecPayloadTypeMap_Ext;

/// An enum of security direction
typedef enum
{
	WmeSecurityDirection_InBound,
	WmeSecurityDirection_OutBound,
}WmeSecurityDirection;

/// An enum of crypto suite type
typedef enum
{
	WmeCryptoSuiteType_NULL_CIPHER_HMAC_SHA1_80,	///> NULL cipher with HMAC-SHA1, [RFC3711-SRTP]
    WmeCryptoSuiteType_AES_CM_128_HMAC_SHA1_32,
	WmeCryptoSuiteType_AES_CM_128_HMAC_SHA1_80,		///> AES_CM_128_HMAC_SHA1_80, [RFC4568-SDESC]
	WmeCryptoSuiteType_AES_CM_256_HMAC_SHA1_80,		///> AES_CM_256_HMAC_SHA1_80
}WmeCryptoSuiteType;

/// An enum of security service
typedef enum
{
	WmeSecurityService_None				= 0x0,	///> No services
	WmeSecurityService_Confidentiality	= 0x1,	///> Confidentiality service
	WmeSecurityService_Authentication	= 0x2,	///> Authentication service
	WmeSecurityService_All				= 0x3	///> All services, including confidentiality and authentication
}WmeSecurityService;

/// An enum of EKT cipher type
typedef enum
{
	WmeEKTCipherType_AESKW_128,
}WmeEKTCipherType;

/// An struct of EKT configuration
typedef struct  
{
	WmeEKTCipherType eCipherType;		///< EKT_Cipher: The EKT cipher used to encrypt the SRTP Master Key
	uint8_t* pKey;						///< EKT_Key: The EKT key used to encrypt the SRTP Master Key
	uint32_t uKeyLength;				///< EKT_Key_Length: The length of the EKT_Key in bytes
	uint16_t uSPI;						///< EKT_SPI: The EKT Security Parameter Index, 15 bits
}WmeEKTConfiguration;

/// An struct of security configuration
typedef struct
{
	WmeSecurityDirection eSecurityDirection;

	WmeCryptoSuiteType eCryptoSuiteType;
	uint8_t* pMasterKeySalt;
	uint32_t uMasterKeySaltLength;

	WmeSecurityService eRTPSecurityService;
	WmeSecurityService eRTCPSecurityService;

	WmeSecurityFECOrder eFECOrder;

	WmeEKTConfiguration* pEKTConfiguration;	///< EKT configuration. If NULL == pEKTConfiguration, then EKT isn't enabled.
}WmeSecurityConfiguration;

/// An enum of packetization mode type
typedef enum
{
	WmePacketizationMode_0 = 0,			///< Single NAL unit mode, only support NAL unit 
	WmePacketizationMode_1,				///< Non-Interleaved mode, support NAL unit, STAP-A, FU-A
}WmePacketizationMode;

/// An struct of RTP info [WME_DEPRECATED]
/// The struct is deprecated and replaced with WmeCodecPayloadTypeMap struct.
WME_DEPRECATED typedef struct
{
	uint8_t uPayloadType;		///< The RTP payload type
	uint32_t uClockRate;		///< The RTP timestamp clock rate
}WmeRTPInfo;

/// An enum of media transport type
typedef enum
{
    WmeTransportType_Unknown = 0,
    WmeTransportType_UDP,
    WmeTransportType_TCP
}WmeTransportType;

/// IWmeMediaTransport interface
class IWmeMediaTransport 
{
public:
	/// Get transport type
    /*!
     \param eTransportType [out] Media transport type
     \return The error value of the function result
     \note thread safe
     */
	virtual WMERESULT GetTransportType(WmeTransportType &eTransportType) = 0;
    
    /// Send RTP packet
	/*!
		\param pRTPPackage [in] RTP package handle
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage) = 0;

	/// Send RTCP packet
	/*!
		\param pRTCPPackage [in] RTCP package handle
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage) = 0;
};

/// IWmeMediaContentCipher
class IWmeMediaContentCipher : public IWmeUnknown
{
public:
    virtual ~IWmeMediaContentCipher() {}
    
    /// Encrypt the media content
    /*!
     \param pData       [in] the media content buffer
     \param ulen        [in] the media content length
     \param pOutData    [out] the output buffer after encryption
     \param uOutLen     [out] the length of the output buffer
     \return The error value of the function result
     */
    virtual WMERESULT Encrypt(uint8_t* pData, uint32_t uLen, uint8_t** pOutData, uint32_t& uOutLen) = 0;
    
    /// Decrypt the encrypted media content
    /*!
     \param pData       [in] the media content buffer
     \param ulen        [in] the media content length
     \param pOutData    [out] the output buffer after decryption
     \param uOutLen     [out] the length of the output buffer
     \return The error value of the function result
     */
    virtual WMERESULT Decrypt(uint8_t* pData, uint32_t uLen, uint8_t** pOutData, uint32_t& uOutLen) = 0;
    
    /// Free the output buffer returned from Encrypt/Decrypt
    /*
     \param pOutData [in] the buffer that is to be released
     */
    virtual void      FreeBlock(uint8_t* pOutData) = 0;
    
    /// Get the count of media content that should be skipped during encryption/decryption
    /*
     Note: to keep backward compatibility, should return 0 for auido, 12 for video
     */
    //virtual uint32_t  GetSkipCount() = 0;
};
    
/// IWmeMediaMessageTransmitter interface
/*!	

	\note	Supported observers: IWmeMediaMessageTransmitterObserver

*/
static const WMEIID WMEIID_IWmeMediaMessageTransmitter = 
{ 0x4da6f9, 0xdde1, 0x4d24, { 0x82, 0x7f, 0xe9, 0x63, 0x7e, 0x9c, 0x80, 0x72 } };

class IWmeMediaMessageTransmitter : public IWmeMediaEventNotifier
{
public:
	/// To indicate that a picture loss event has been detected
	/*!
		\param uMediaSourceID  [in] The source of the media steam
		\param uMediaStreamId  [in] The ID of the stream, should be DID for SVC codec
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT IndicatePictureLoss(uint32_t uMediaSourceID, uint32_t uMediaStreamId) = 0;
};

/// IWmeMediaMessageTransmitterObserver
static const WMEIID WMEIID_IWmeMediaMessageTransmitterObserver = 
{ 0x2c20f859, 0x6d1f, 0x4df0, { 0xa3, 0x37, 0x5, 0x6e, 0x75, 0x96, 0x59, 0x2 } };

class WME_EXPORT WME_NOVTABLE IWmeMediaMessageTransmitterObserver : virtual public IWmeMediaEventObserver
{
public:
	/// To notify that an payload-specific message PLI needs processing
	/*!
		\param pNotifier  [in] Caller's handle
		\param uMediaSourceID  [in] The source of the media steam
		\param uMediaStreamId  [in] The ID of the stream, should be DID for SVC codec
		\return The error value of the function result
	*/
	virtual WMERESULT OnPictureLossIndication(IWmeMediaEventNotifier *pNotifier, uint32_t uMediaSourceID, uint32_t uMediaStreamId, bool instant) = 0;
};

typedef enum
{
    WME_RTPEXT_VID              = 0,
    WME_RTPEXT_AudioLevel       = 1,
    WME_RTPEXT_FrameMarking     = 1,
    WME_RTPEXT_TOFFSET          = 2,
    WME_RTPEXT_MARITimestamp    = 3,
    WME_RTPEXT_Priority         = 4,
    
    WME_RTPEXT_RapicSync64      = 5,
    
    WME_RTPEXT_CVO              = 6,
    
    WME_RTPEXT_MaxCount         = 7
}WmeRTPHeaderExtType;
    
///
typedef enum
{
    WmeSCRRequestPolicyType_NonePolicy = 0,
    WmeSCRRequestPolicyType_ReceiverSelected,
    WmeSCRRequestPolicyType_ActiveSpeaker,
}WmeSCRRequestPolicyType;

typedef enum
{
    WmeSCRRequestCodecType_Common = 0,
    WmeSCRRequestCodecType_H264,
}WmeSCRRequestCodecType;

typedef struct
{
    uint8_t     uSubsessionChannelId;
    uint8_t     uSourceId;
    uint32_t    uBitrate;
}WmeNonePolicyRequestInfo;
    
typedef struct
{
    uint8_t     uSubsessionChannelId;
    uint8_t     uSourceId;
    uint32_t    uBitrate;
    
    uint8_t     uPriority;
    uint8_t     uGroupingAdjacencyId;
    bool        bDuplicationFlag;
}WmeActiveSpeakerRequestInfo;

typedef struct
{
    uint8_t     uSubsessionChannelId;
    uint8_t     uSourceId;
    uint32_t    uBitrate;
    
    uint32_t    uCaptureSourceId;
}WmeSelectedSourceRequestInfo;
    
#define WME_MAX_TEMPORAL_LAYERS  4
typedef struct
{
    uint8_t     uPayloadType;
    bool        bLandscape;
    bool        bCvo;
}WmeCommonCodecCapability;
    
typedef struct
{
    uint8_t     uPayloadType;
    bool        bLandscape;
    bool        bCvo;
    uint32_t    uMaxMBPS;
    uint16_t    uMaxFS;
    uint16_t    uMaxFPS;
    uint8_t     uTemporalLayerCount;
    uint16_t    pTemporalLayers[WME_MAX_TEMPORAL_LAYERS];
}WmeH264CodecCapability;

typedef struct
{
    WmeSCRRequestPolicyType eRequestPolicyType;
    union {
        WmeNonePolicyRequestInfo        stNonePolicyInfo;
        WmeActiveSpeakerRequestInfo     stActiveSpeakerInfo;
        WmeSelectedSourceRequestInfo    stSelectedSourceInfo;
    }uRequestInfo;
    
    WmeSCRRequestCodecType  eRequestCodecType;
    union {
        WmeCommonCodecCapability    stCommonCodecCap;
        WmeH264CodecCapability      stH264CodecCap;
    }uCodecCapability;
}WmeSimulcastRequest;

typedef struct
{
    uint8_t     uSubsessionChannelId;
    uint8_t     uErrorCode;
    bool        bHasCSID;
    uint32_t    uCSID;
}WmeSimulcastInvalidRequestInfo;

typedef struct
{
    uint8_t                         uSubSessionsAvailable;
    uint8_t                         uMaxAdjSources;
    uint8_t                         uSimulcastInvalidRequestCount;
    WmeSimulcastInvalidRequestInfo* pSimulcastInvalidRequestInfos;
}WmeSimulcastAnnounce;
    
typedef struct
{
    uint8_t     uPayloadType;
    uint8_t     uDID;
    uint8_t     uMaxDID;
}WmeSimulcastMediaProperty;
    
static const WMEIID WMEIID_IWmeSimulcastRequestObserver =
{ 0x304c8899, 0xbf45, 0x46cf, { 0x96, 0xed, 0x76, 0x6a, 0x6c, 0x50, 0x29, 0x91 } };

class WME_EXPORT WME_NOVTABLE IWmeSimulcastRequestObserver : virtual public IWmeMediaEventObserver
{
public:
    virtual void OnAnnounce(const WmeSimulcastAnnounce& stAnnounce) = 0;
    
protected:
    virtual ~IWmeSimulcastRequestObserver() {}

};
  
static const WMEIID WMEIID_IWmeSimulcastResponseObserver =
{ 0x705f4da9, 0xf3a6, 0x4663, { 0xb8, 0xec, 0x47, 0x4f, 0x3a, 0x54, 0xe3, 0x58 } };
    
class WME_EXPORT WME_NOVTABLE IWmeSimulcastResponseObserver : virtual public IWmeMediaEventObserver
{
public:    
    virtual WMERESULT OnSubscribe(const WmeSimulcastRequest* pRequests, uint8_t uRequestCount) = 0;
protected:
    virtual ~IWmeSimulcastResponseObserver() {}

};

/*
* contains cname
*/
#define MAX_CNAME_LEN   256
typedef struct _CNameBlock{
    char cname[MAX_CNAME_LEN];
} CNameBlock;

static const WMEIID WMEIID_IWmeSimulcastRequester =
{ 0x8b6e6483, 0xa294, 0x43eb, { 0x93, 0xbe, 0xea, 0xea, 0x01, 0xb5, 0x3d, 0xb9 } };

class WME_NOVTABLE IWmeSimulcastRequester : public IWmeUnknown
{
public:
    // --------------Simulcast Request Configuration----------------
    // for a sprop-source: policies=as:1,rs:2
    virtual WMERESULT RegisterPolicyId(uint8_t uSourceId, WmeSCRRequestPolicyType ePolicyType, uint16_t uPolicyId, eStreamDirection eDir) = 0;
    virtual WMERESULT SetMaxSubsessionChannels(uint8_t uCount, eStreamDirection eDir) = 0;
    
    // deprecated APIs
    virtual WMERESULT Subscribe(const WmeSimulcastRequest& stRequest) = 0;
    virtual WMERESULT Unsubscribe(uint8_t uSubSessionChanneldId) = 0;
    virtual WMERESULT UnsubscribeAll() = 0;
    virtual WMERESULT SendRequest() = 0;
    //------------end-------------
    
    virtual WMERESULT Subscribe(const WmeSimulcastRequest* pRequests, uint8_t uRequestCount) = 0;
    
    /// Sub-session Channel Announce
    /*!
     \param stAnnounce  [in] the information needed by SCA
     \param bReliable [in] whether the transmission of SCA should be reliable. if true, the SCA need an acknowlegement,
     \                      and RTP layer will retransmit it until an SCA-ACK is received. 
     \                      Note: The mute/unmute event triggered SCA should be reliable, and the SCR triggered SCA is no necessary to be reliable.
     \return The error value of the function result
     */
    virtual WMERESULT Announce(const WmeSimulcastAnnounce& stAnnounce, bool bReliable) = 0;
    
protected:
	/// The destructor function
    virtual ~IWmeSimulcastRequester(){}
};

    
class WME_NOVTABLE IWmeRTPPacketQuerier
{
public:
    virtual WMERESULT   Bind(const uint8_t* pRtpPacket, uint16_t uPacketLen) = 0;
    virtual uint16_t    GetVIDCount() = 0;
    virtual WMERESULT   GetVIDAt(uint16_t uIndex, uint8_t& uVid) = 0;
    virtual WMERESULT   GetMultiVID(uint8_t* pVidArray, uint16_t& uVidCount) = 0;
    
    virtual uint16_t    GetCSICount() = 0;
    virtual WMERESULT   GetCSIAt(uint16_t uIndex, uint32_t& uCSI) = 0;
    virtual WMERESULT   GetMultiCSI(uint32_t* pCSIArray, uint16_t& csiCount) = 0;
    
    virtual ~IWmeRTPPacketQuerier() {}
};


/// IWmeMediaSession interface
/*!	

	\note	Supported observers: IWmeMediaMessageTransmitterObserver

*/
static const WMEIID WMEIID_IWmeMediaSession = 
{ 0x557266eb, 0x3aac, 0x4a78, { 0x97, 0xcb, 0x4f, 0xcb, 0xf4, 0x32, 0x3c, 0x32 } };

class IWmeMediaSession : public IWmeMediaMessageTransmitter
{
public:
	/// Get session type
	/*!
		\param eType : [in] Media session type
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT GetSessionType(WmeSessionType &eType) = 0;
	
	/// Add a track to session with RTP info [WME_DEPRECATED]
	/// The interface is deprecated and replaced with AddTrack(IWmeMediaTrack *pTrack) interface.
	/*!
		\param pTrack : [in] Media track handle
		\param stRTPInfo : [in] RTP info structure
		\return The error value of the function result
		\note thread safe
	*/
	WME_DEPRECATED virtual WMERESULT AddTrack(IWmeMediaTrack *pTrack, const WmeRTPInfo& stRTPInfo) = 0;

	/// Add a track to session
	/*!
		\param pTrack : [in] Media track handle
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT AddTrack(IWmeMediaTrack *pTrack) = 0;

	/// Remove a specified track from session
	/*!
		\param pTrack : [in] Media track handle
		\return The error value of the function result
		\note thread safe
	*/
	virtual WMERESULT RemoveTrack(IWmeMediaTrack *pTrack) = 0;
	
	/// Set options
	/*!
		\param eOption : [in] Identifies the option
		\param pValue : [in] Option content
		\param uSize : [in] Size of "pValue", for security check
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT SetOption(WmeSessionOption eOption, void* pValue, uint32_t uSize) = 0;

	/// Get options
	/*!
		\param eOption : [in] Identifies the option
		\param pValue : [out] Option content
		\param uSize : [in] Size of "pValue", for security check
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT GetOption(WmeSessionOption eOption, void* pValue, uint32_t uSize) = 0;

	/// Set RTCP transport
	/*!
		\param pTransport : [in] Media transport handle
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT SetTransport(IWmeMediaTransport *pTransport) = 0;
	
	/// Receive RTP packet
	/*!
		\param pTrack : [in] Media track handle associated with the RTP packet
		\param pRTPPackage : [in] RTP package handle
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT ReceiveRTPPacket(IWmeMediaTrack *pTrack, IWmeMediaPackage *pRTPPackage) = 0;

	/// Receive RTP packet
	/*!
		\param pTrack : [in] Media track handle associated with the RTP packet
		\param pRTPData : [in] RTP data handle
		\param uRTPSize : [in] RTP data size
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT ReceiveRTPPacket(IWmeMediaTrack *pTrack, uint8_t *pRTPData, uint32_t uRTPSize) = 0;
	
	/// Receive RTCP packet
	/*!
		\param pRTCPPackage : [in] RTCP package handle
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT ReceiveRTCPPacket(IWmeMediaPackage *pRTCPPackage) = 0;

	/// Receive RTCP packet
	/*!
		\param pRTCPData : [in] RTCP data handle
		\param uRTCPSize : [in] RTCP data size
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT ReceiveRTCPPacket(uint8_t *pRTCPData, uint32_t uRTCPSize) = 0;

    /// Turn on the specified RTP header extensions for SDP negotiation
    /*!
        \param pExtTypes: [in] An array of RTP header extensions to be turned on
        \param uExtCount: [in] The item count of the array
        \return 0 if suscess, otherwise failure
        \note no thread safe
    */
    virtual WMERESULT TurnOnRTPHeaderExtensions(WmeRTPHeaderExtType pExtTypes[], uint8_t uExtCount) = 0;
    
	///  Enumerate the RTP header extensions supported by WRTP lib
	/*!
		\param pIterFunc: [in] The function that handle each supported header extension
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT EnumBuiltinRTPHeaderExtensions(WmeBuiltinRTPExtensionIterFunc pIterFunc) = 0;
	
	/// Register two callback functions for an RTP header extension,
	/// one to build the extension, the other to parse it.
	/// The functions will be called during RTP packet encoding and decoding respectively.
	/*!
		\param sExtURI: [in] The URI name of the header extension
		\param pBuilder: [in] The function to build the header extension
		\param pParser: [in] The function to parse the header extension
		\return The error value of the function result
		\note not thread safe
	*/
	virtual WMERESULT RegisterRTPExtensionProcessFunc(const char* sExtURI, WmeExtensionBuildFunc pBuilder, WmeExtensionParseFunc pParser = NULL) = 0;
    
    virtual WMERESULT QueryRTPExtensionID(const char *extURI, eStreamDirection dir, uint8_t &id) = 0;
        
	/// Get statistics of media session
	/*!
		\param stat : [out] reference of WmeSessionStatistics
		\return The error value of the funtion result
	 */
	virtual WMERESULT GetStatistics(WmeSessionStatistics &stStat) = 0;

    virtual WMERESULT GetTrackNetStatistics(WmeTrackStatistics *localStat, uint32_t &len, eStreamDirection dir) = 0;
    virtual WMERESULT GetLossBurstStatitics(WmeLossBurstLenStatistics& stat) = 0;
    virtual WMERESULT GetOOOStatistics(WmeOOOGapLenStatistics& stat) = 0;
    virtual WMERESULT GetBwDownEventStatistics(WmeBwEventStatistics& stat) = 0;
    virtual WMERESULT GetSimulcastRequester(IWmeSimulcastRequester **ppRequester) = 0;

    virtual WMERESULT GetRTPPacketQuerier(eStreamDirection eDir, IWmeRTPPacketQuerier **pRTPPacketQuerier) = 0;

    /** Query remote cname used by ssrc
    * Parameters:
    *  @Param[in] ssrc: the stream ssrc number
    *  @Param[out] cnameblock: the cnameblock contains cname buff
    * Return value:
    *  0 is successful, otherwise is error number.
    */
    virtual WMERESULT QueryRemoteCName(uint32_t ssrc, CNameBlock& cnameblock) = 0;

    /** Set local cname
    * Parameters:
    *  @Param[in] cname: cname string
    */
    virtual WMERESULT SetLocalCName(const char* cname) = 0;

    /** Query local cname
    * Parameters:
    *  @Param[out] cnameblock: contains cname buffer
    * Return value:
    *   0 is successful, otherwise failed
    */
    virtual WMERESULT GetLocalCName(CNameBlock& cnameblock) = 0;
   
    /** Call this function to reset the internal state after failover
    *
    */
    virtual WMERESULT Failover() = 0;
    
    /** update the CSI that will be packed to RTP CSRC field
     * @Param[in] label: track label
     */
    virtual WMERESULT UpdateCSI(uint32_t label, uint32_t csi) = 0;
    virtual WMERESULT GetCSI(uint32_t label, uint32_t &csi) = 0;
    
    virtual uint32_t GetLossRawDataRecordedBytes() = 0;
    
protected:
	/// The destructor function
	virtual ~IWmeMediaSession(){}
};

}

#endif // WME_SESSION_INTERFACE_H
