#ifndef __WEBEX_SQUARE_WME_MEDIA_CONFIG_HEADER__
#define __WEBEX_SQUARE_WME_MEDIA_CONFIG_HEADER__

#include "WmeInterface.h"
#include "WmeDataDump.h"
#define IN
#define OUT
namespace wme
{
    /**
     * It describes different ICE connection preference, by default it would be UDPFallToTCP
     * Websocket is now not supported yet.
     */
    typedef enum{
        UDPOnly = 1,                        ///< It is locked to UDP, it will fail if that is not available.
        TCPOnly = 2,                        ///< It is locked to TCP, 
        WSOnly = 4,                         ///< It is locked to WebSocket
        ProxyOnly = 8 | TCPOnly,            ///< It is test only, it will only connect to proxy. 10
        TlsOnly = 16 | TCPOnly,             ///< It is test only, it will connect with tls or proxy with tls. 18
        TlsProxyOnly = 32 | TlsOnly,        ///< It is test only, it will only connect with proxy and tls tunnel. 50
        UDPFallToTCP = UDPOnly | TCPOnly,   ///< It will fallback to TCP if UDP is failed to pass connectivity check
        UDPFallToWS = UDPOnly | WSOnly,     ///< It will fallback to Websocket if UDP is failed to pass connectivity check
        TCPFallToWS = TCPOnly | WSOnly      ///< It will fallback to Websocket if TCP is failed
    }WmeConnectionPolicy;
    
    ///Network status enumerator
    typedef enum {
        WmeNetwork_bad,             ///< network is very bad
        WmeNetwork_video_off,       ///< network is so bad that we should turn off video to guarantee audio quality.
        WmeNetwork_recovered,       ///< network becomes better
    }WmeNetworkStatus;
    
    typedef enum{
        WmePolicy_NonePolicy = 0,
        WmePolicy_ReceiverSelected = 1,
        WmePolicy_ActiveSpeaker = 2,
        WmePolicy_All = WmePolicy_ReceiverSelected | WmePolicy_ActiveSpeaker,
    }WmeSubscribePolicy;
    
    typedef enum{
        WmeHWAcceleration_Encoder = 0,
        WmeHWAcceleration_Decoder = 1
    }WmeHWAccelerationConfig;
    
    typedef enum{
        WmePip_None = 0,    ///< don't support pip
        WmePip_4            ///< support 4 pip
    }WmePIPCapability;

    typedef enum{
        WmePerformanceDumpNone = 0,
        WmePerformanceDumpCpuBattery = 1,   ///< only ios/android support cpu/battery dump
        WmePerformanceDumpMem = 2,          ///< only ios/android/macos/win32/wp8 support memory dump
        WmePerformanceDumpAll = WmePerformanceDumpCpuBattery | WmePerformanceDumpMem
    }WmePerformanceDumpType;
    
    typedef enum{
        WmeAecTypeNone = 0,         		///< no AEC
        WmeAecTypeBuildin = 1,				///< Build AEC, only VPIO mode is used on IOS.
        WmeAecTypeWmeDefault = 2,          	///<  Windows/Mac/Linux uses Modified Movi AEC, Android/IOS use WebRTC AECM
        WmeAecTypeTc = 3,           		///< TC AEC, Only IOS/Android supported.
        WmeAecTypeAlpha = 4             	///< A-AEC, only Mac/Windows/Android supported.
    }WmeAecType;
    
    
    /**
     * Basic configuration which is shared between audio, video and sharing.
     */
    class IBaseConfig
    {
    public:
        virtual ~IBaseConfig() {}

        ///Get maximum bandwidth(TIAS) in unit bps for particular media
        virtual uint32_t GetMaxBandwidth() = 0;
        ///Set maximum bandwidth(TIAS) in unit bps for particular media, only applicable if set before start of call.
        virtual WMERESULT SetMaxBandwidth(uint32_t IN maxBw) = 0;
        ///Get initial bandwidth
        virtual uint32_t GetInitBandwidth() = 0;
        ///Set initial bandwidth in bps
        virtual WMERESULT SetInitBandwidth(uint32_t IN initBw) = 0;
        ///Get prefered CODEC type
        virtual WmeCodecType GetPreferedCodec() = 0;
        ///Set prefered CODEC type, it is only applicable if set before start of call
        virtual WMERESULT SetPreferedCodec(WmeCodecType IN preferred) = 0; 
        ///Get selected CODEC type if it has been locked
        virtual WmeCodecType GetSelectedCodec() = 0;
        ///Lockdown the CODEC type to a specified type, it is only applicable if set before start of call.
        virtual WMERESULT SetSelectedCodec(WmeCodecType IN selected) = 0;
        ///Overide the media IP address and port, it is used for test only. It is only applicable if set before start of call.
        virtual WMERESULT OverrideMediaIPAddress(const char* IN ipAddr, uint32_t IN port=0) = 0;
        ///Enable to dump the media data from render to file, and it is only applicable if set before start of call
        virtual WMERESULT EnableFileRender(const char* IN path, bool IN bSaveFile = true) = 0;
        ///Enable to read media data from file instead of camera, and it is only applicable if set before start of call
        virtual WMERESULT EnableFileCapture(const char* IN path, bool IN isLoop) = 0;
        ///Set ICE preference on connectivity check.
        virtual WMERESULT SetIcePolicy(WmeConnectionPolicy policy) = 0;
        ///Get ICE preference on connectivity check.
         virtual WmeConnectionPolicy GetIcePolicy() = 0;
        ///Enable or disable Fec, and it is only applicable if set before start of call
        virtual WMERESULT EnableFec(bool IN value) = 0;
        ///Is Fec enabled
        virtual bool IsFecEnabled() = 0;
        ///Set max Fec overhead
        virtual void SetMaxFecOverhead(uint32_t IN fecOverhead) = 0;
        ///Enable/Disable Fec probing feature
        virtual void SetFecProbing(bool IN fecProbing) = 0;
        ///Set dynamic fec level scheme
        virtual void SetDynamicFecScheme(DynamicFECLevelScheme targetScheme) = 0;
        ///Set SRTP FEC order
        virtual void SetSrtpFecOrder(WmeSecurityFECOrder targetOrder) = 0;

        // multistream max available sources
        virtual uint32_t GetMaxAvailableSources() = 0;
        // multistream subscribe policy
        virtual WmeSubscribePolicy GetSubscribePolicy() = 0;
        
        // !!! Deprecated by EnableClientMix and setInitSubscribeCount
        // multistream max subscribed streams count. Default is 1 for video/share; 3 for audio.
        virtual WMERESULT SetMaxStreamsCount(uint32_t IN maxStreams) = 0;
		//Add by Boris
		virtual WMERESULT SetAutoSwitchDefaultDeviceFlag(bool bEnable) = 0;
        
        virtual WMERESULT EnableRecordLossData(bool value) = 0;
        virtual bool IsRecordLossDataEnabled() = 0;
        
        /// Added for NBR, data will be terminated at RTP. It will stop to decode audio/video.
        virtual WMERESULT SetDataTerminatedAtRTP(bool value) = 0;
    };

    /**
     * Configurations only applied to audio session, it derived from IBaseConfig.
     */
    class IAudioConfig: virtual public IBaseConfig
    {
    public:
        virtual ~IAudioConfig() {}
        /// If AGC is enabled.
        virtual bool IsAGCEnabled() = 0;
        /// Enable AGC at any time.
        virtual WMERESULT EnableAGC(bool IN value) = 0;
        /// If is Echo Canccellation is enabled.
        virtual bool IsECEnabled() = 0;
        /// Enable Echo Canccellation at any time.
        virtual WMERESULT EnableEC(bool IN value) = 0;
        /// If is Voice Activity Detection is enabled.
        virtual bool IsVADEnabled() = 0;
        /// Enable Voice Activity Detection at any time.
        virtual WMERESULT EnableVAD(bool IN value) = 0;
        /// If is Noise Supression is enabled.
        virtual bool IsNSEnabled() = 0;
        /// Enable Noice Supression at any time.
        virtual WMERESULT EnableNS(bool IN value) = 0;
        /// Start to dump some key audio files for trouble shooting
        /// @param time How long the dump files last for, in milliseconds, 
        ///   set it to 0 to stop any current dumping.
        virtual WMERESULT EnableKeyDumpFiles(uint32_t IN time) = 0;
        /// Set the audio dump file path
        /// @param path
        /// @param length of path string
        virtual WMERESULT EnableKeyDumpFilesPath(char* pPath,unsigned int len) = 0;
        /// Default is 3 for audio.
        virtual WMERESULT EnableClientMix(uint32_t IN nMaxStream) = 0;
        /// to ensure audio quality,drop some seconds of audio data at the beginning of a call
        /// the default value of drop seconds is 2.
        virtual WMERESULT EnableDropSeconds(bool IN bEnable) = 0;
        virtual WMERESULT IsDropSecondsEnable(bool& OUT bEnable) = 0;

		//Add by Boris(2015-4-13)
		//The bandwidth for audio codec(Opus). 25000 or 40000. Default is 25000
		virtual WMERESULT SetCodecBandwidth(uint32_t uBPS) = 0;
		virtual WMERESULT GetCodecBandwidth(uint32_t& OUT uBPS) = 0;

		/// If DAGC is enabled.
		virtual bool IsDAGCEnabled() = 0;
		/// Enable DAGC at any time.
		virtual WMERESULT EnableDAGC(bool IN value) = 0;
		
		virtual WMERESULT SetAutoSwitchDefaultSpeakerDeviceFlag(bool bEnable) = 0;
        virtual WMERESULT SetAutoSwitchDefaultMicrophoneDeviceFlag(bool bEnable) = 0;
		virtual WMERESULT SetAutoSwitchDefaultDeviceFlagEx(bool bEnable) = 0;
        
        virtual WMERESULT SetPlayBufferTime(int nMin,int nMax) = 0;
        virtual WMERESULT GetPlayBufferTime(int &nMin,int &nMax) = 0;

        //
        // Add by Conan(xiasu@cisco.com) (2015-12-30)
        // change playback stream mode
        virtual WMERESULT SetPlaybackStreamMode(int mode) = 0;
        virtual WMERESULT GetPlaybackStreamMode(int & mode) = 0;
        
        /// Set AEC type
        /// @param AEC type
        virtual WMERESULT SetECType(WmeAecType IN ecType) = 0;
        
        /// Get AEC type
        /// @param AEC type
        virtual WMERESULT GetECType(WmeAecType&  OUT ecType) = 0;

    };

    /**
     * Configurations that applies to both video and application sharing session.
     * Derived from IBaseConfig.
     */
    class IBaseVideoConfig: virtual public IBaseConfig
    {
    public:
        virtual ~IBaseVideoConfig() {}
        ///Set decoder parameters, it is only applicable if set before start of call.
        ///@see http://en.wikipedia.org/wiki/H.264/MPEG-4_AVC
        ///@param codec input parameter, it select the CODEC type those parameters will apply to.
        ///@param uProfileLevelID H.264 profile level ID
        ///@param max_mbps maximum decoding speeds, in macro blocks per second.
        ///@param max_fs maximum macro blocks in a frame.
        ///@param max_fps maximum frames per second mulipled by 100
        ///@param max_br maximum bit rate for the stream, it should be and can overrided by TIAS
        virtual WMERESULT SetDecodeParams(WmeCodecType IN codec, uint32_t IN uProfileLevelID, 
            uint32_t IN max_mbps = 0, uint32_t IN max_fs = 0, uint32_t IN max_fps = 0, 
            uint32_t IN max_br = 0) = 0;
        ///Get decoder parameters with a CODEC type.
        virtual WMERESULT GetDecodeParams(WmeCodecType IN codec, uint32_t* OUT puProfileLevelID, 
            uint32_t* OUT pmax_mbps, uint32_t* OUT pmax_f, uint32_t* OUT pmax_fps, 
            uint32_t* OUT pmax_br) = 0;
        ///Set encoder parameters, it can be set at any time during the call for non-multistream, but it is only applicable if set before start of call for multistream.
        virtual WMERESULT SetEncodeParams(WmeCodecType IN codec, uint32_t IN uProfileLevelID, 
            uint32_t IN max_mbps = 0, uint32_t IN max_fs = 0, uint32_t IN max_fps = 0, 
            uint32_t IN max_br = 0) = 0;
        ///Get encoder parameters with a CODEC type.
        virtual WMERESULT GetEncodeParams(WmeCodecType IN codec, uint32_t* OUT puProfileLevelID, 
            uint32_t* OUT pmax_mbps, uint32_t* OUT pmax_fs, uint32_t* OUT pmax_fps, 
            uint32_t* OUT pmax_br) = 0;

        ///Get maximum packet size for RTP.
        virtual uint32_t GetMaxPacketSize() = 0;
        ///Set the maximum packet size for RTP, it is only applicable if set before start of call
        virtual WMERESULT SetMaxPacketSize(uint32_t IN uMaxPacketSize) = 0;
        ///Get the packetization mode
        virtual WmePacketizationMode GetPacketizationMode() = 0;
        ///Set the packetization mode, it is only applicable if set before start of call
        virtual WMERESULT SetPacketizationMode(WmePacketizationMode IN ePacketizationMode) = 0;
        ///Set video data dump flag, used for debug
        virtual WMERESULT SetDataDumpFlag(uint32_t IN uDataDumpFlag) = 0;
        ///Restrict VCL NAL within RTPPayload
        ///Only take effect when negotiatied with pack mode 1
        ///It should be called before createoffer
        virtual WMERESULT LimitNalSize(bool IN enable) = 0;
		    ///Enable hardware acceleration for encode/decode
		    virtual WMERESULT EnableHWAcceleration(bool IN value, WmeHWAccelerationConfig config) = 0;
        ///Enable Periodical IDR/keyframe
        virtual WMERESULT EnablePeriodIDR(bool IN value) = 0;
        ///Enable Simulcast
        virtual WMERESULT EnableAVCSimulcast(bool IN value) = 0;
        ///Enable CVO
        virtual WMERESULT EnableCVO(bool IN value) = 0;
        ///Is CVO enabled
        virtual bool IsCVOEnabled() = 0;
        ///Enable update camera capture parameter dynamically
        virtual WMERESULT EnableUpdateCapParam(bool IN value) = 0;
        ///Disable 90P
        virtual WMERESULT Disable90PVideo(bool IN value) = 0;
        
        ///// FOR TRAIN ONLY
        /// Default is 1 for video/share.
        virtual WMERESULT SetInitSubscribeCount(uint32_t IN count) = 0;

	};

    /**
     * Configurations that are only applied to video session. It derived from IBaseVideoConfig.
     */
    class IVideoConfig: virtual public IBaseVideoConfig
    {
    public:
        virtual ~IVideoConfig() {}
    };

    /**
     * Configurations that are only applied to application sharing session.
     * We current don't have any special configuration for sharing.
     * It derived from IBaseVideoConfig.
     */
    class IShareConfig: virtual public IBaseVideoConfig
    {
    public:
        virtual ~IShareConfig() {}
        virtual WMERESULT SetScreenMaxCaptureFps(unsigned int nFps) = 0;
        virtual unsigned int GetScreenMaxCaptureFps() = 0;
        virtual WMERESULT SetScreenCaptureDownSampleMinHeight(unsigned int nMinHeight) = 0;
        virtual WMERESULT SetScreenFilterSelf(bool bFilterSelf) = 0;
    };
    
    ///Proxy crendential callback interface
    class IProxyCredentialSink
    {
    public:
        virtual ~IProxyCredentialSink() {}
        
        /** Get the proxy credential for particular proxy server:port
         * @param szProxy proxy server IP address
         * @param port proxy port
         * @param szRealm realm information returned from proxy server in 407 response
         */
        virtual void OnProxyCredentialRequired(const char* szProxy, uint16_t port, const char* szRealm) = 0;
    };

    /**
     * Global configuration which will enforce to all sessions together.
     * All of them need to be set before start of call.
     */
    class IGlobalConfig
    {
    public:
        virtual ~IGlobalConfig() {}
        ///Is QoS enabled.
        virtual bool IsQosEnabled() = 0;
        ///Enable or disable QoS
        virtual WMERESULT EnableQos(bool IN value) = 0;
        
        ///Enable or disable Dynamic Performance Controller
        virtual WMERESULT EnableDPC(bool IN value) = 0;
        
        ///Is cisco multistream (simulcast) is supported?
        virtual bool IsMultiStreamEnabled() = 0;
        ///Enable or disable cisco multistream (simulcast)
        virtual WMERESULT EnableMultiStream(bool IN supportCmulti) = 0;
        ///Get QoS max loss ratio
        virtual float GetQoSMaxLossRatio() = 0;
        ///Set QoS max loss ratio
        virtual WMERESULT SetQoSMaxLossRatio(float fQoSMaxLossRatio) = 0;
        ///Get QoS min bandwidth
        virtual uint32_t GetQoSMinBandwidth() = 0;
        ///Set QoS min bandwidth
        virtual WMERESULT SetQoSMinBandwidth(uint32_t uQoSMinBandwidth) = 0;
        ///Get QoS init bandwidth
        virtual uint32_t GetQoSInitBandwidth() = 0;
        ///Set QoS init bandwidth
        virtual WMERESULT SetQoSInitBandwidth(uint32_t uQoSInitBandwidth) = 0;
        ///Set NetworkStatus notification wait time
        virtual WMERESULT SetNetworkNotificationParam(WmeNetworkStatus st, WmeNetworkDirection direc, uint32_t waitMilliseconds) = 0;
        ///Is ICE connectivity check supported?
        virtual bool IsICEEnabled() = 0;
        ///Enable or disalbe ICE connectivity check.
        virtual WMERESULT EnableICE(bool IN value) = 0;
        ///Is SRTP enabled?
        virtual bool IsSRTPEnabled() = 0;
        ///Enable or disable SRTP
        virtual WMERESULT EnableSRTP(bool IN value) = 0;
        ///Is DTLS-SRTP enabled?
        virtual bool IsDtlsSRTPEnabled() = 0;
        ///Is DTLS-SRTP server?
        virtual bool IsDtlsSRTPServer() = 0;
        ///Enable or disable DTLS-SRTP
        virtual WMERESULT EnableDtlsSRTP(bool IN value, bool IN isServer) = 0;
        ///Enable or disable RTCP multiplex.
        virtual WMERESULT EnableRTCPMux(bool value) = 0;
        ///Is RTCP multiplex enabled.
        virtual bool IsRTCPMuxEnabled() = 0;
        ///set device capability, the capability is a json object as { audio:{}, video:{} };
        virtual WMERESULT SetDeviceMediaSettings(const char* capability) = 0;
        ///Enable BGRA for capture, it is designed for iOS test only, and it is only applicable if set before start of call
        virtual WMERESULT EnableBGRA(bool IN value) = 0;
        ///Is BGRA mode enabled?
        virtual bool IsBGRAEnabled() = 0;
        ///Enable or disable DSCP marking
        virtual WMERESULT EnableDSCP(bool IN value) = 0;
        ///Is DSCP marking enabled
        virtual bool IsDSCPEnable() = 0;
        ///set DSCP marking for audio,video and screen sharing
        virtual WMERESULT SetDSCPMarking(uint32_t uDSCPofAudio, uint32_t uDSCPofVideo, uint32_t uDSCPofScreen) = 0;
        
        ///enable or disable battery&cpu&memory trace dump
        virtual WMERESULT EnablePerformanceTraceDump(WmePerformanceDumpType IN value) = 0;
        ///set call level feature toggles, the feature toggles is a json object as { media-mari-rate-adaptation:basic-hybrid, media-mari-fec:dynamic}, should be called before SetReceivedSDP;
        virtual WMERESULT SetFeatureToggles(const char* featureToggles) = 0;
        virtual const char* GetFeatureToggles() = 0;
    };
    
#ifdef __cplusplus
    extern "C"
    {
#endif
    ///Set proxy credential callback sink
    WME_EXPORT void SetProxyCredentialSink(IProxyCredentialSink *pSink);
    ///Set Proxy username and password
    WME_EXPORT WMERESULT SetProxyUsernamePassword(const char * szUserName, const char * szPassword);
    ///Allow or not to read from system key chain, which will prompt a system warning confirm dialog to interact with.
    ///Default behavior is allowed. This interface is only available in MAC OS.
    WME_EXPORT void AllowProxyGetSystemCredential(bool bAllow);
        
#ifdef __cplusplus
    }
#endif
}

#endif //!define __WEBEX_SQUARE_WME_MEDIA_CONFIG_HEADER__
