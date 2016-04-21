#ifndef _PANGU_ENGINE_DEMO_CLIENT
#define _PANGU_ENGINE_DEMO_CLIENT

#include "DemoParameters.h"

#include "MediaManager.h"
#include "MediaTransport.h"

#include "AudioSender.h"
#include "AudioReceiver.h"
#include "VideoSender.h"
#include "VideoReceiver.h"
#include "VideoPreview.h"
#ifdef ENABLE_ZBAR
#include "WMEMediaFileRenderSink.h"
#endif
// whsu
#include "DeskShareSender.h"
#include "DeskShareReceiver.h"

#include "MiscMutex.h"
#include <string>

typedef enum
{
    DEMO_WINDOW_NONE		= 0,
    DEMO_WINDOW_SENDER		= 1,
    DEMO_WINDOW_RECEIVER	= 2,
	DEMO_WINDOW_PREVIEW		= 3,
	DEMO_WINDOW_DESKSHARING = 4, // whsu
}DEMO_WINDOW_TYPE;

typedef struct
{
    DEMO_WINDOW_TYPE type;
    void* vp;
    void* windowHandle;
}WINDOW_INFORMATION;


///
/// sink for DemoClient for event notification: network, device and volume
class backUISink
{
public:
    virtual void networkDisconnectFromClient(DEMO_MEDIA_TYPE eType)		= 0;
    virtual void connectRemoteFailureFromClient(DEMO_MEDIA_TYPE eType)	= 0;
    virtual void connectRemoteOKFromClient(DEMO_MEDIA_TYPE mtype) {};
    virtual void OnDeviceChanged(DeviceProperty *pDP, WmeEventDataDeviceChanged &changeEvent) = 0;
    virtual void OnVolumeChange(WmeEventDataVolumeChanged &changeEvent) = 0;
    virtual void OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight) {}
    virtual void OnEndOfStream(DEMO_MEDIA_TYPE mtype) {}

protected:
    virtual ~backUISink() {}
};


///
/// main interfaces for external callers
/// desc:
//      (a) Sender is for audio/video local track
//          Receiver is for audio/video remote track
//          Preview is only for video preview
//      (b) Transport is for audio/video network channel
//      (c) Manager is for audio/video device, capabilities and etc.
class DemoClient : 
    public IMediaClientSink,
    public IMediaTransportSink,
    public IMediaManagerSink
{

public:
    DemoClient(backUISink * pBackUISink);
    virtual ~DemoClient();

    void SetUISink(backUISink * pBackUISink);

    ///
    /// Init/Uninit DemoClient, only once
    long Init(WmeTraceLevel level=WME_TRACE_LEVEL_INFO);
    long UnInit();

#if defined(ANDROID) || defined(IOS)
    // Drive TP in iOS/Android Platform by external loop
    static void HeartBeat();
    static void InitTPThread();
#endif

    void SetTraceMaxLevel(WmeTraceLevel level);
    void SetDumpDataEnabled(unsigned int uDataDumpFlag);
    void SetDumpDataPath(const char *path);

    bool GetMediaPorts(DEMO_MEDIA_TYPE mtype, unsigned short &uRtpPort, unsigned short &uRtcpPort);

    ///
    /// Network interfaces for creating one server, which will wait for remote connection
    long InitHost(DEMO_MEDIA_TYPE mtype);
    long InitHost(DEMO_MEDIA_TYPE mtype, unsigned short uRtpPort, unsigned short uRtcpPort);
    long InitHost(DEMO_MEDIA_TYPE mtype, const char* pMyName,
            const char *szJingleIP, int nJinglePort, const char *szStunIP, int nStunPort);

    /// Network interfaces for connecting to remote server
    long ConnectRemote(DEMO_MEDIA_TYPE eMediaTyep, char *pIPAddress);
    long ConnectRemote(DEMO_MEDIA_TYPE mtype, char *pIPAddress, unsigned short uRtpPort, unsigned short uRtcpPort);
    long ConnectRemote(DEMO_MEDIA_TYPE eMediaTyep, const char* pMyName, const char * pHostName,
            const char *szJingleIP, int nJinglePort, const char *szStunIP, int nStunPort);
    
    /// Network interfaces for connecting to pcap dump file
    long ConnectFile(DEMO_MEDIA_TYPE eMediaTyep, const char *fileName, const char *sourceIP, unsigned short sourcePort, const char *destinationIP, unsigned short destinationPort);

    /// Stop network and Clean resources
    long DisConnect(DEMO_MEDIA_TYPE eMediaTyep);

    /// Start/Stop Audio/Video Media Track
    long StartMediaTrack(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype);
    long StopMediaTrack(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype);

    long SetVideoQuality(DEMO_TRACK_TYPE ttype, WmeVideoQualityType quality);
	long SetStaticPerformance(WmePerformanceProfileType perf);
    long GetCapability(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype, WmeMediaBaseCapability *pMC);
    
    /// Only valid for audio/video local track
    long StartMediaSending(DEMO_MEDIA_TYPE mtype);
    long StopMediaSending(DEMO_MEDIA_TYPE mtype);
    bool IsMediaSending(DEMO_MEDIA_TYPE mtype);


    ///
    /// Video Render Settings for video track(local/remote/preview)
    long SetRenderView(DEMO_TRACK_TYPE ttype, void * handle, bool windowless=false);
    long StopRenderView(DEMO_TRACK_TYPE ttype);
    long SetRenderAspectRatioSameWithSource(DEMO_TRACK_TYPE ttype, bool keepsame);
	long SetRenderMode(DEMO_TRACK_TYPE ttype, WmeTrackRenderScalingModeType eMode);
    
    long OnRenderWindowPositionChanged(DEMO_TRACK_TYPE ttype, void * render);
    long OnRenderingDisplayChanged(DEMO_TRACK_TYPE ttype);
#ifdef WIN32
    long OnRenderWindowClosed(DEMO_WINDOW_TYPE type);
#endif

    ///
    /// Create/Delete Media Client: 
    ///      (a) audio client: local and remote tracks, 
    ///      (b) video cleint: local, remote and preview tracks
    long CreateMediaClient(DEMO_MEDIA_TYPE mtype);
    long CreateMediaClient(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype);
    void DeleteMediaClient(DEMO_MEDIA_TYPE mtype);
    void DeleteMediaClient(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype);
    void EnableMyMedia(DEMO_MEDIA_TYPE mtype, bool bEnable);

	/// For QoS
	void EnableQoS(bool bEnable);


    ///
    //just temp use this test , we will use wbx_test in future.
    IWmeScreenSource * GetOneScreenSource();
    ShareSourceManager *GetShareSourceManager() {return m_pShareSourceManager;};
    long SetScreenSource(IWmeScreenSource * pIWmeScreenSource);

    /// For Audio/Video devices
    typedef vector<DeviceProperty> DevicePropertyList;
    long GetDeviceList(DEMO_MEDIA_TYPE mtype, DEMO_DEV_TYPE dtype, DevicePropertyList& dpList);
    long ClearDeviceList(DEMO_MEDIA_TYPE mtype, DevicePropertyList& dpList);
    
    long SetCamera(DEMO_TRACK_TYPE ttype, IWmeMediaDevice* pCamera);
    long SetMic(IWmeMediaDevice* pMic);
    long SetSpeaker(IWmeMediaDevice* pSpeaker);
	long GetCaptureDevice(DEMO_TRACK_TYPE ttype, DEMO_DEV_TYPE dtype, IWmeMediaDevice** pDev);
    ///
    /// For Audio/Video capability
    typedef vector <WmeVideoMediaCapability> VideoMediaCapabilityList;
    typedef vector <WmeAudioMediaCapability> AudioMediaCapabilityList;
    long GetMediaCapabilities(DEMO_MEDIA_TYPE mtype, void* mcList);

    long SetMediaCapability(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype, WmeMediaBaseCapability *pMC);
	long GetMediaCapability(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype, WmeMediaBaseCapability *pMc);
	long SetMediaCodec(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype, WmeCodecType codecType);
	long GetMediaCodec(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype, WmeCodecType &codecType); 
	long SetMediaSession(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype, WmeCodecType codec);
    
    typedef vector<WmeDeviceCapability> DeviceCapabilityList;
    long GetDeviceCapabilities(DEMO_DEV_TYPE dtype, IWmeMediaDevice *dev, DeviceCapabilityList &dcList);
    long ClearDeviceCapabilities(DEMO_DEV_TYPE dtype, DeviceCapabilityList &dcList);
    long SetCameraCapability(DEMO_TRACK_TYPE ttype, WmeDeviceCapability *pDC);

	/// For video file
	long SetVideoInputFile(const char* pFileName, WmeVideoRawFormat *pFormat);
	void SetVideoSource(int nSource); //nSource: 1 file; 0 camera;
    long SetVideoOutputFile(const char* pFileName);
	void SetVideoTarget(int nTarget); //nTarget: 1 screen ; 0 file;
    
    /// For audio file
    long SetAudioInputFile(const char* pFileName, WmeAudioRawFormat *pFormat);
    void SetAudioSource(int nSource); //nSource: 1 file; 0 mic;
    long SetAudioOutputFile(const char* pFileName);
#ifdef CUCUMBER_ENABLE
	long SetAudioFileRenderSink(IWmeMediaFileRenderSink* pSink);
    float CheckAudioOutputFile(char* pAudioOutputFileName, char* pAudioRefFileName);
    bool CheckVideoOutputFile();
    bool AddVideoFileRenderSink();
#endif
    void SetAudioTarget(int nTarget); //nTarget: 1 speaker ; 0 file;
    ///
    /// For audio volume control
    long GetAudioVolume(DEMO_DEV_TYPE dtype, int &vol);
    void SetAudioVolume(DEMO_DEV_TYPE dtype, int vol);
    void MuteAudio(DEMO_DEV_TYPE dtype, bool bmute);
    bool IsAudioMute(DEMO_DEV_TYPE dtype);
    void GetVoiceLevel(unsigned int &level);

    /// only valid for ANDROID platform
    void SetAudioOutType(DEMO_AUDIO_OUT_TYPE nType);

    /// For Audio/Video Session statistics
    long GetVideoStatistics(WmeSessionStatistics &wmeStatistics);
    long GetAudioStatistics(WmeSessionStatistics &wmeStatistics);
    long GetScreenStatistics(WmeSessionStatistics &wmeStatistics);

	/// For Audio/Video Track statistics
	long GetVideoStatistics(DEMO_TRACK_TYPE ttype, WmeVideoStatistics &wmeStatistics);
    long GetAudioStatistics(DEMO_TRACK_TYPE ttype, WmeAudioStatistics &wmeStatistics);
    long GetScreenStatistics(DEMO_TRACK_TYPE ttype, WmeScreenShareStatistics &wmeScreenShareStatistics);
    
    /// For Sync statistics
    long GetSyncStatistics(WmeSyncStatistics &wmeSyncStatistics);
    
    long GetNetworkIndex(WmeNetworkIndex& idx, WmeNetworkDirection d);
    
	/// For audio option
	long EnableEC(bool bEnable);
	long EnableVAD(bool bEnable);
	long EnableNS(bool bEnable);
	long EnableAGC(bool bEnable);
	long EnableAutoDropData(bool bEnable);

	/// For video option
	long EnableDecoderMosaic(bool bEnable);
	
	long DisableSendingFilterFeedback();
    
    void SetQoSMaxLossRatio(float maxLossRatio);
    void SetQoSMinBandwidth(int minBandwidth);
    void SetInitialBandwidth(int initBandwidth);

    
    void SetSpeakerMute(bool bEnable);
    void SetMicMute(bool bEnable);
///=================================
///

public:
    /// fro IMediaClientSink
    virtual void OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight);
    virtual void OnEndOfStream(DEMO_MEDIA_TYPE eType);

    /// for IMediaTransportSink
    virtual void OnConnected(bool bSuccess, ConnSinkType stype);
    virtual void OnDisConnected(ConnSinkType stype) ;

    /// for IMediaManagerSink
    virtual long OnAudioInChanged(WmeEventDataDeviceChanged &changeEvent);
    virtual long OnAudioOutChanged(WmeEventDataDeviceChanged &changeEvent);
    virtual long OnVolumeChanged(WmeEventDataVolumeChanged &stChangeEvent);
    virtual long OnVideoInChanged(WmeEventDataDeviceChanged &changeEvent);

protected:
    /// To manage audio/video devices changed which is emited by OnAudioInChanged/OnAudioOutChanged/OnVideoInChanged
    /// from IMediaManagerSink
    long OnDeviceChanged(DEMO_MEDIA_TYPE mtype, DEMO_DEV_TYPE dtype, WmeEventDataDeviceChanged &changeEvent);

    int ProcessDeviceDeleted(DeviceProperty &dp, WmeEventDataDeviceChanged &changeEvent);
    int ProcessDeviceAdded(DevicePropertyList &dpList, WmeEventDataDeviceChanged &changeEvent);
    int ProcessDefaultDeviceUpdated(DeviceProperty &dp, WmeEventDataDeviceChanged &changeEvent);

protected:
    MediaManager *GetMediaManager(DEMO_MEDIA_TYPE mtype);
    MediaClient *GetMediaClient(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype);
    VideoSettings *GetVideoSettings(DEMO_TRACK_TYPE ttype);
	AudioSettings *GetAudioSettings(DEMO_TRACK_TYPE ttype);
    
    /// for audio sender and receiver
    long CreateAudioSender();
    long DeleteAudioSender();
    long CreateAudioReceiver();
    long DeleteAudioReceiver();

    /// for audio network
    long CreateAudioSession();
    long DeleteAudioSession();
    long CreateAudioTransport(MediaTransport * &transport);
    long DeleteAudioTransport();

    /// for audio features manager: codec enum/volume control/device change/media capabilities
    long CreateAudioManager();
    long DeleteAudioManager();


    /// for video sender, receiver and preview
    long CreateVideoSender();
    long DeleteVideoSender();
    long CreateVideoReceiver();
    long DeleteVideoReceiver();
    long CreateVideoPreview();
    long DeleteVideoPreview();

    /// for video network
    long CreateVideoSession();
    long DeleteVideoSession();
    long CreateVideoTransport(MediaTransport * &transport);
    long DeleteVideoTransport();

    /// for video features manager: codec enum/device change/media capabilities
    long CreateVideoManager();
    long DeleteVideoManager();
    
    /// for Screen Share
	long CreateDeskShareSession(); // whsu
    long DeleteDeskShareSession();
    long CreateDeskShareTransport();
    long DeleteDeskShareTransport();
    
    long CreateDeskShareSender(); // whsu
    long DeleteDeskShareSender();
    long CreateDeskShareReceiver();
    long DeleteDeskShareReceiver();
    
	long CreateDeskShareSourceManager(); // whsu
    long DeleteDeskShareSourceManager();



private:
    /// for audio sender and receiver
    AudioSender     *m_pAudioSender;
    AudioReceiver   *m_pAudioReceiver;
    MediaTransport  *m_pAudioTransport;
    /// for audio manager(codec, device, etc.)
    AudioManager    *m_pAudioManager;
    unsigned short  m_uAudioPorts[2];
    WmeCodecType    m_AudioCodecType;

    /// for video preview, sender and receiver
    VideoSender     *m_pVideoSender;
    VideoReceiver   *m_pVideoReceiver;
    VideoPreview    *m_pVideoPreview;
    MediaTransport  *m_pVideoTransport;
    /// for video manager
    VideoManager    *m_pVideoManager;
    unsigned short  m_uVideoPorts[2];
    WmeCodecType    m_VideoCodecType;
    
    /// for DeskShare sender and receiver
    DeskShareSender		*m_pDeskShareSender;	// whsu
    DeskShareReceiver	*m_pDeskShareReceiver;
    //
    MediaTransport      *m_pDeskShareTransport;
    //
    ShareSourceManager  *m_pShareSourceManager;
	unsigned short  m_uDeskSharePorts[2];

    unsigned int    m_uInitBandwidth;

	/// for video file
	std::string		m_strVideoInputFileName;
	WmeVideoRawFormat m_sVideoRawFormat;
	BOOL			m_bUseVideoInputFile;
    std::string     m_strVideoOutputFileName;
    BOOL			m_bUseVideoOutputFile;

    ///for audio file
    std::string         m_strAudioInputFileName;
    WmeAudioRawFormat   m_sAudioRawFormat;
    BOOL                m_bUseAudioInputFile;
    std::string         m_strAudioOuputFileName;
    BOOL                m_bUseAudioOutputFile;

    /// mutex for DemoClient and some callback
    MiscMutex m_Mutex;

///
///=================================

private:

    ////////////////////////////////////////////////////////
    IWmeMediaEngine*		m_pWmeEngine;
    IWmeMediaSyncBox*		m_pWmeSyncBox;
    IWmeNetworkIndicator*   m_pNetworkIndicator;
    
    IWmeMediaSession*		m_pWmeAudioSession;
    IWmeMediaSession*		m_pWmeVideoSession;
    IWmeMediaSession*		m_pWmeDeskShareSession; // whsu

    ////////////////////////////////////////////////////////
    backUISink *m_pBackUISink;

    bool m_bEnableMyVideo;
    bool m_bEnableMyAudio;
    bool m_bStartVideoSending;
    bool m_bStartAudioSending;
	bool m_bEnableQoS;
	bool m_bEnableSendingFilterFeedback;
    
    /// for DeskShare
    bool m_bEnableMyDeskShare;		// whsu
    bool m_bStartDeskShareSending;

};

#endif
