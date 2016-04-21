/**
 *   MediaSession interface (similar to webRTC PeerConnection, but simplified)
 *
 */

#ifndef __WEBEX_SQUARE_WME_MEDIA_CONNECTION_HPP__
#define __WEBEX_SQUARE_WME_MEDIA_CONNECTION_HPP__

#pragma once
#include "WmeInterface.h"
#include "mediaconfig.h"
#include <string>

namespace wme
{

class IWmeMediaConnectionSink;
class IWmeMediaTrackBase;

/**
 * SDP media line status, can be: sendonly, recvonly, sendrecv and inactive
 */
typedef enum {
    WmeDirection_Inactive = 0,
    WmeDirection_Send = 1,
    WmeDirection_Recv = 2,
    WmeDirection_SendRecv = WmeDirection_Send | WmeDirection_Recv,
}WmeDirection;
    
/**
* SDP negotiation role, can be offer or answer
*/
typedef enum {
    WmeSdp_None = 0,
    WmeSdp_Offer,
    WmeSdp_Anwser,
}WmeSdpType;

/**
 * Media line running status
 * @plantuml connectstate
   [*] --> Disconnected : ICE Failure
   [*] --> Connected : ICE success
   Connected --> Disconnected : Network Switch (Outage)
   Disconnected --> Reconnected: ICE success
   Disconnected --> Disconnected: ICE Failure
   Reconnected --> Disconnected : Network Switch (Outage)
   @endplantuml
 */
typedef enum {
    WmeSession_Disconnected = 0,		///< This media session has been disconnected due to network
    WmeSession_Connecting,				///< This media session is connecting to peer
    WmeSession_Connected,				///< This media session has connected with peer, It is callbacked when ICE connectivity check is done. It is fake in case of without ICE. 
    WmeSession_Received,				///< This media session has received media data from peer
    WmeSession_Sent,					///< We had sent data to peer in this media session
    WmeSession_ConnectFailed,			///< This media session has failed to pass ice connectivity check
    WmeSession_Reconnected,				///< This media session has reconnected after network disconnected
    WmeSession_FileCaptureEnded,		///< This media session has reached the end of the file capture, this is only available when file capture is enabled.
    WmeSession_TransportSwitched,       ///< Underlying transport has changed, for exmaple, ICE aggressive normination has incosistent result from normal normination.
    WmeSession_CameraStarted,           ///< Callback when camera is ready for the local track, it is for desktop where we asynchronously open camera
    WmeSession_CameraStartFailed        ///< Callback when camera is failed to open for local track, it is for desktop where we asynchronously open camera
}WmeSessionStatus;

typedef enum {
    WmeMedia_Available                     = 0,
    WmeMedia_ERR_INTERNAL                  = 0x10,
    WmeMedia_ERR_INTERNAL_TEMP             = 0x11,
    WmeMedia_ERR_INVALID_REQUEST           = 0x20,
    WmeMedia_ERR_PERMANENTLY_UNAVAILABLE   = 0x30,
    WmeMedia_ERR_TEMP_UNAVAIL_INSUFF_SRC   = 0x31,
    WmeMedia_ERR_TEMP_UNAVAIL_NO_MEDIA     = 0x32,
    WmeMedia_ERR_TEMP_UNAVAIL_WRONG_STREAM = 0x33,
    WmeMedia_ERR_TEMP_UNAVAIL_INSUFF_BW    = 0x34,
}WmeMediaStatus;

typedef enum {
    WMeMediaLogging_None = 0,               ///< Disable Media logging
    WmeMediaLogging_Audio = 0x1,            ///< Enable media logging for audio (send/receive)
    WmeMediaLogging_Video = 0x1 << 1,       ///< Enable media logging for video (send/receive, simulcast, multistream)
    WmeMediaLogging_Screen = 0x1 << 2,      ///< Enable media logging for screen sharing.
    WmeMediaLogging_Cpu = 0x1 << 3,         ///< Eanble media logging for cpu/memory information
    WmeMediaLogging_Wifi = 0x1 << 4,        ///< Enable media logging for wifi status
    WmeMediaLogging_Sync = 0x1 << 5,        ///< Enable media logging for AV sync status
    WmeMediaLogging_Mem = 0x1 << 6,          ///< Enable media logging for Memory status
    WmeMediaLogging_Default = WmeMediaLogging_Audio | WmeMediaLogging_Video | WmeMediaLogging_Screen | WmeMediaLogging_Cpu | WmeMediaLogging_Mem | WmeMediaLogging_Wifi,
    WmeMediaLogging_All = 0xffffffff        ///< Enable every media logging
}WmeMediaLoggingType;
    
#define MAX_IP_ADDRESS 256

/**
 * Media connection statistics in network connection level.
 */
typedef struct _tagConnectionStatistics
{
    uint32_t uRTPReceived;				///< RTP received packets number
    uint32_t uRTPSent;					///< RTP sent packets number
    uint32_t uRTCPReceived;				///< RTCP received packets number
    uint32_t uRTCPSent;					///< RTCP sent packets number
    char remoteIp[MAX_IP_ADDRESS];		///< Remote IP v4 address of current connection
    uint32_t uRemotePort;				///< Remote IP v4 port of current connection
    char localIp[MAX_IP_ADDRESS];		///< Local IP v4 address of current connection
    uint32_t uLocalPort;				///< Local v4 port of current connection
    WmeTransportType connectionType;	///< Current connection type (UDP/TCP)
    unsigned long timeBound;			///< System time in milliseconds when local SDP is created
    unsigned long timeConnecting;		///< System time in milliseconds before ICE connectivity check
    unsigned long timeConnected;		///< System time in milliseconds after ICE connectivity check
    unsigned long timeSent;				///< System time in milliseconds when first packet is sent out
    unsigned long timeReceived;			///< System time in milliseconds when first packet is received from peer
}WmeConnectionStatistics;

/**
 * Audio statistics
 */
typedef struct _tagAudioConnectionStatistics
{
    WmeConnectionStatistics connStat;		///< media connection network level statistics
    WmeSessionStatistics sessStat;			///< RTP session level statistics
    WmeAudioStatistics localAudioStat;		///< outbound audio engine level statistics
    WmeAudioStatistics remoteAudioStat;     ///< inbound audio engine level statistics
    WmeSyncStatistics avsyncStat;
}WmeAudioConnectionStatistics;
    
/**
* Video statistics
*/
typedef struct _tagVideoConnectionStatistics
{
    WmeConnectionStatistics connStat;		///< media connection network level statistics
    WmeSessionStatistics sessStat;			///< RTP session level statistics

    bool bLocalVideoStatAvailable;	      ///< flag of localTrack statistics availability, false when local is muted or stopped
    WmeVideoStatistics localVideoStat;		///< outbound video engine level statistics
    WmeVideoStatistics remoteVideoStat;     ///< inbound video engine level statistics
}WmeVideoConnectionStatistics;

/**
* Desktop sharing statistics
*/
typedef struct _tagScreenConnectionStatistics
{
    WmeConnectionStatistics connStat;			///< media connection network level statistics
    WmeSessionStatistics sessStat;				///< RTP session level statistics
    WmeScreenShareStatistics localScreenStat;	///< outbound sharing level statistics
    WmeScreenShareStatistics remoteScreenStat;	///< inbound sharing engine level statistics
}WmeScreenConnectionStatistics;

/**
* Parsed SDP media line information if client cares
*/
typedef struct _tagSdpParsedInfo
{
    WmeSessionType mediaType;				///< The media type of the parsed m line
    WmeDirection negotiatedRemoteDirection; ///< The remote SDP status after negotiated with local capability
	/** The mid of this media line, if you didn't call IWmeMediaConnection::AddMedia 
	 * before call IWmeMediaConnection::SetReceivedSDP, you need to use the mid returned here
	 * for the next IWmeMediaConnection::AddMedia to bind them.
	 * It will be the mid of your set if you called IWmeMediaConnection::AddMedia before 
	 * IWmeMediaConnection::SetReceivedSDP
	 */
    unsigned long mid;						
}WmeSdpParsedInfo;
    
class IWmeMediaConnection : public IWmeUnknown
{
public:
    virtual ~IWmeMediaConnection() {}

    /**
     * Add a media to the MediaConnection, a media line will be generated in the SDP offer/answer.
     * @param mediaType WmeSessionType can be WmeSessionType_Audio, WmeSessionType_Video or WmeSessionType_ScreenShare
     * @param status can be WmeDirection_Send, WmeDirection_Recv or WmeDirection_SendRecv
     * @param mid can be any unified long to identify the media line. In case of adding media after Offer received
     *  it MUST be set to the value returned from WmeSdpParsedInfo.
     * @param parameters Deprecated. It has been recommended to switch to MediaConfig API. Set it to NULL.
     */
    virtual WMERESULT AddMedia(WmeSessionType mediaType, WmeDirection direction, unsigned long mid,
        const char *parameters) = 0;
    
	/**
	 * Update the media status, for example, you want to hold a call.
     * @param direction media line status
     * @param mid the identify of the media line you want to update
	 */
    virtual WMERESULT UpdateMedia(WmeDirection direction, unsigned long mid) = 0;

	/**
	 * Create SDP for offer with the media you have added.
	 * AddMedia MUST be called before CreateOffer.
	 * You can CreateOffer again after you have changed media parameters, for example, IWmeMediaConnection::UpdateMedia.
	 * Every time you call, you will get an SDP for offer. IWmeMediaConnectionSink::OnSDPReady will be triggered.
	 *@plantuml offeranswerstate
		[*] --> OfferCreated : CreateOffer
		OfferCreated --> AnswerReceived: SetReceivedSDP
		OfferCreated --> OfferCreated: CreateOffer
		[*] --> OfferReceived : SetReceivedSDP
		OfferReceived --> OfferReceived: SetReceivedSDP
		OfferReceived --> AnswerCreated: CreateAnswer
	 @endplantuml
	 */
    virtual WMERESULT CreateOffer() = 0;

    /**
     * Set the received remote SDP(offer/answer) to MediaConnection. It will return remote media sessions in an array.
     * Upper layer can decide to add corresponding media or reject some of them.
	 * @param receivedSdpType The SDP type, it can be Offer or Answer
	 * @param receivedSDP The SDP itself in string
     * @param arraySdp It will return an array of parsed media line information 
	 * which client might be interest in.
     * @param num It is in/out param. IN: the memory size of the array. 
	 * OUT: actally number of items we returned. It MUST be NOT NULL.
     */
    virtual WMERESULT SetReceivedSDP(WmeSdpType receivedSdpType, const char *receivedSDP, WmeSdpParsedInfo *arraySdp = NULL, int* num = NULL) = 0;
    
	///Create SDP answer with the media you have added.
    ///SetReceivedSDP and AddMedia MUST be called before CreateAnswer.
    ///IWmeMediaConnectionSink::OnSDPReady will be triggered.
    virtual WMERESULT CreateAnswer() = 0;
    
	///Set the callback handler    
    virtual WMERESULT SetSink(IWmeMediaConnectionSink *sink) = 0;
    
    ///Get the audio statistics, the mid MUST be set correctly as in AddMedia even you have only one audio m-line.
    virtual WMERESULT GetAudioStatistics(unsigned long mid, WmeAudioConnectionStatistics &stStat) = 0;
	///Get the video statistics, the mid MUST be set correctly as in AddMedia even you have only one video m-line.
    virtual WMERESULT GetVideoStatistics(unsigned long mid, WmeVideoConnectionStatistics &stStat) = 0;
	///Get the screen-sharing statistics, the mid MUST be set correctly as in AddMedia even you have only one video m-line.
    virtual WMERESULT GetScreenStatistics(unsigned long mid, WmeScreenConnectionStatistics &stStat) = 0;    
	///Get the CPU usage of system, process and its top threads
	virtual WMERESULT GetCPUStatistics(WmeCpuUsage &cpuStat) = 0;
    ///Get the memory usage of system
    virtual WMERESULT GetMemoryStatistics(WmeMemoryUsage &memoryStat) = 0;
    ///Get network metrics
    virtual WMERESULT GetNetworkMetrics(WmeAggregateNetworkMetricStats &stStat) = 0;
	/**
     * Get the overall media metrics after the call has been ended and post it to splunk for further analysis online.
     * We have added a lot of splunk metrics inside this API.
     * Upper layer MUST decode the returned string as JSON object and combine the JSON object into a parent JSON object 
     * and POST to splunk.
     */
	virtual const char* GetMediaSessionMetrics() = 0;
	virtual WMERESULT GetSyncStatistics(unsigned int csi, WmeSyncStatistics &stStat) = 0;
    
    /** Subscribe remote video with Cisco multistream protocol. 
     * MediaConnection will automatically subscribe 1 active video with vid = 0, priority 255 and grouping 0. 
     * IWmeMediaConnectionSink::OnMediaReady will be called back if it succeeded.
     * An updated SCR(in RTCP FB) will be sent to media server.<br>
     * Note: Subscribe MUST be called after SDP negotiation done: CreateOffer or CreateAnswer has been called.
     * @param policy WmePolicy_ActiveSpeaker(active speaker policy) or WmePolicy_ReceiverSelected(receiver selected policy). Calliope doesn't support RS yet. WebEx MMP support both.
     * @param type video quality type, can select 90p/180p/360p/720p.
     * The parameters will be combined with the output of static performance control. For example,
     * An iPhone 4s will not able to subscribe 720p video due to performance issue. Even client set
     * the parameters to 720p but WME will decrease it as a protection.
     * @param priority only apply on activespeaker, 255 is highest priority and was reserved for the default subscription.
     * @param vid It is an output parameter, will return the MediaConnection allocated VID, it is 1-1 correlated with remote media track.
     * @param grouping only apply on AS policy, default grouping is 0 and is applied to the default subscription.
     * @param duplicate Whether duplication of the same video in different groups was allowed.
     * @param csi only apply on RS policy
     */
    virtual WMERESULT Subscribe(unsigned long mid, WmeSubscribePolicy policy, WmeVideoQualityType type, int &vid, 
        uint8_t priority = 255, uint8_t grouping = 0, bool duplicate = false, unsigned int csi = 0) = 0;
    
    /**
     * Verbose version of Subscribe, it will use detail video(H264) parameters instead of video quality.
     * Those max parameters MUST consistent and follow standard convention. In case of inconsistent parameters 
     * we will only honor maxMBPS. As described in Subscribe, the parameters will be combined with static
     * performance control.
     * @param uMaxFS max frame size
     * @param uMaxFPS max frames per second
     * @param uBitrate max bitrate
     * @param uMaxDPB mad DPB
     * @param uMaxMBPS max macro blocks per second
     */
    virtual WMERESULT Subscribe(unsigned long mid, WmeSubscribePolicy policy,
                                uint16_t uMaxFS, uint16_t uMaxFPS, uint32_t uBitrate, uint32_t uMaxDPB,
                                uint32_t uMaxMBPS, int &vid, uint8_t priority, uint8_t grouping, bool duplicate,
                                unsigned int csi = 0) = 0;
    
    /**
     * Unsubscribe a remote video track. You can safely release that remote track once it was Unsubscribed.
     * @param mid the media line ID
     * @param vid the VID returned in IWmeMediaConnection::Subscribe
     */
    virtual WMERESULT Unsubscribe(unsigned long mid, uint8_t vid) = 0;
    
    /**
     * Set global network warning notification parameters, Deprecated IGlobalConfig::SetNetworkNotificationParam.
     * @param st network warning type
     * @param direc transport direction (uplink/downlink or both)
     * @param waitMilliseconds timer interval for warning
     */
    virtual WMERESULT SetGlobalNetworkNotificationParam(WmeNetworkStatus st, WmeNetworkDirection direc, uint32_t waitMilliseconds) = 0;
    
    /**
     * Get network quality index
     * @param direction transport direction (uplink/downlink or both)
     */
    virtual WmeNetworkIndex GetNetworkIndex(WmeNetworkDirection direction) = 0;

	///Get the audio configuration interface with m-line identifier
	///@return IAudioConfig
    virtual IAudioConfig* GetAudioConfig(unsigned long mid) = 0;
	///Get the video configuration interface with m-line identifier
	///@return IVideoConfig
	virtual IVideoConfig* GetVideoConfig(unsigned long mid) = 0;
	///Get the sharing configuration interface with m-line identifier
	///@return IShareConfig
	virtual IShareConfig* GetShareConfig(unsigned long mid) = 0;
	///Get the global configuration interface
	///@return IGlobalConfig
	virtual IGlobalConfig* GetGlobalConfig() = 0;
    
    virtual WMERESULT SetParams(unsigned long mid, const char* szParams, unsigned int nSize) = 0;
    virtual const char* GetParams(unsigned long mid) = 0;

    /** Stop the media connection, so all tracks will be stopped and underneath resources (connections, camera, 
      * microphone, speaker, GL surfaces and etc) will be released.
      * The objects referenced by client need to be released by client after Stop.
      * MediaConnection is not re-usable, that means, once Stop has been called, 
      * it cannot AddMedia/CreateOffer/... again
      * @note You MUST call Stop before you release reference to avoid crash.
      */
	virtual void Stop() = 0;
    
    /**
     * Get the media engine to enumerate the devices or screens for sharing.
     */
    virtual IWmeMediaEngine* GetEngine() = 0;
    
	///Start to output media statistics into client logs (info level) in a timer
	///@param interval the interval of statistics log, in milliseconds
    ///@param logType client can choose to only print partial media logs because it may be too verbose.
	virtual WMERESULT StartMediaLogging(long interval = 1000, int logType = WmeMediaLogging_Default) = 0;
	///Stop to print media statistics in logs
	virtual WMERESULT StopMediaLogging() = 0;
    
    ///Windows Phone only, audio and video in different process,
    ///this interface is used to sync audio and video between them.
    virtual WMERESULT SyncWithAudioTimestamp(uint32_t csi, uint32_t timestamp) = 0;
    
    virtual WMERESULT PauseQoS(unsigned long mid) = 0;
    virtual WMERESULT ResumeQoS(unsigned long mid) = 0;

    /**
     * Client will tell WME all other participant CSI it observed from participant list.
     * Client can duplicate add CSI to us, we will save it in a local list 
     * and will try to avoid clash when generating new CSI.
     * Client need to set a unique ID for us to detect the csi change for remote track (WebEx) 
     * or other potential usage.
     * Client needs to CreateOffer and re-negotiate with Calliope server when it return WME_S_FALSE.
     *
     * @param csi the CSI of a participant
     * @param szUniqueID the unique ID of the participant.
     * @param nIDLen the string length of unique ID.
     * @param sessType the session type of this CSI, this information is not available for Spark client right now, please input WmeSessionType_Unknown instead.
     * @return WME_S_FALSE if a clash is detected, otherwise, WME_S_OK or CM_ERROR_INVALID_ARG
     */
    virtual WMERESULT AddObservedCSI(uint32_t csiOtherParticipant, const char *szUniqueID, unsigned long nIDLen,
                                     WmeSessionType sessType) = 0;
    
    /**
     * Client only has a list of participant CSIs without session type information, WME will try to resolve it.
     * This method have some limitations: it can only handle case (n x (audio + video) + 1 share).
     * This will eventually be deprecated once Locus support session type in CSI list.
     * @param pCsiOtherParticipants CSI array of other participants
     * @param nCsiCount count of the CSI array
     * @param szUniqueID the unique ID of the participant.
     * @param nIDLen the string length of unique ID.
     * @return WME_S_FALSE if a clash is detected, otherwise, WME_S_OK or CM_ERROR_INVALID_ARG. WME_E_FAIL if session type cannot be determined automatically.
     */
    virtual WMERESULT AddObservedCSIs(const uint32_t *pCsisOtherParticipant, unsigned long nCsiCount,
                                      const char *szUniqueID, unsigned long nIDLen) = 0;
};

class IWmeMediaTrackBase : public IWmeUnknown
{
public:
    virtual ~IWmeMediaTrackBase() {}

    /**
     * If it is local track, mute means we capture and preview but doesn't send the data out.
	 * If it is remote track, mute means we don't decode and render the remote media.
     */
    virtual WMERESULT Mute() = 0;

	///If it is local track, unmute will start to send data
	///If it is remote track, unmute will start to decode and render again.
	virtual WMERESULT Unmute() = 0;
    
	///Stop the track for capturing/encoding or decoding/rendering.
    ///The difference between Stop and Mute, for example: Stop will close camera, Stop and Start will
    ///re-initialize audio EC and thus bring extra delay.
    virtual WMERESULT Stop() = 0;
    
    /** set the digitalvolume of the track
      * @param nVolume volume in range [0,65535]
      * @param nType the target of the volume control: system/application/digital
     */
	//Add by Boris(2015-4-16)
	//For digital volume
	virtual WMERESULT SetVolume(unsigned int nVolume, WmeAudioVolumeCtrlType nType = WME_VOL_APPLICATION) = 0;
    
    /** get the volume of the track
      * @param nVolume return volume in range [0,65535]
      * @param nType the target of the volume to get from: system/application/digital
      */
	virtual WMERESULT GetVolume(unsigned int &nVolume, WmeAudioVolumeCtrlType nType = WME_VOL_APPLICATION) = 0;
    
    /** 
     * Get the voice level of the current playout(remote, after mix) or capture(local, mic)
     * The difference between voice level and volume: level is the strength of media data while volume is 
     * the max output/input strength.
     * @param voiceLevel return the voice level of current track, range [0, 100]
     */
    virtual WMERESULT GetVoiceLevel(unsigned int &voiceLevel)=0;
    
	///Change the audio or video capture device
    ///If the video track was external input source and SetCaptureDevice will change the internal video track.
    virtual WMERESULT SetCaptureDevice(IWmeMediaDevice* pDevice) = 0;
    
	///Change the audio playout device
    virtual WMERESULT SetPlayoutDevice(IWmeMediaDevice* pDevice) = 0;
    
	///Add a screen source for sharing, it can be get from Screen enumerator
	virtual WMERESULT AddScreenSource(IWmeScreenSource* pWmeScreenSource) = 0;
    
    ///Add a window as sharing source
	virtual WMERESULT AddSharedWindow(void *handle) = 0;
    ///Remove a shared window
	virtual WMERESULT RemoveSharedWindow(void *handle) = 0;
    
    ///Add external data source as sharing source.
    virtual WMERESULT SetExternalScreenCapturer(IWmeScreenCapturer * pIWmeScreenCapturer) = 0;
	///Remove a screen source from sharing
    virtual WMERESULT RemoveScreenSource(IWmeScreenSource* pWmeScreenSource) = 0;
    ///Set screen sharing id that is unique for every sharing.
    virtual WMERESULT SetScreenSharingID(const char *screenSharingID) = 0;
	///Set the rendering mode.
    virtual WMERESULT SetRenderMode(WmeTrackRenderScalingModeType eMode) = 0;
    
    /**
     * Start the capture/encode or decode/render procedure.
     * @param bMuted BOOL If mute is true, then it will start but not sending out. It is valid only for local track.
     */
    virtual WMERESULT Start(bool bMuted) = 0;
    
	///Add a new render window, the same track can be attached to multiple views    
    virtual WMERESULT AddRenderWindow(void *handle) = 0;
    
	///Remove a render window
    virtual WMERESULT RemoveRenderWindow(void *handle) = 0;
	
    ///Update the render window when the view is resized or hided,
	///the handle MUST has been added and the pointer should be the same.
    virtual WMERESULT UpdateRenderWindow(void *handle) = 0;
    
    /**
     * Add external render instead of built-in render.
     * You MUST NOT set external render for external track, for example, if the local track 
     * source(raw data) comes from file, it makes little sense to add a external render, you can read the data
     * (raw) from the file directly.
     * @param render the external render interface
     */
    virtual WMERESULT AddExternalRender(IWmeExternalRenderer* render) = 0;
    
    /**
     * Remove an addex external render.
     * @param render the external render interface
     */
    virtual WMERESULT RemoveExternalRender(IWmeExternalRenderer* render) = 0;
    
    /** 
     * Get the Contributers Source Indetifiers
     * @param pCSIArray the memory block to save returned CSI array
     * @param csiCount the input size of csi array and return the actual csi count in the array.
     */
    virtual WMERESULT GetCSI(unsigned int* pCSIArray, unsigned int& csiCount) = 0;
    
	///Get the Virtual ID of the track, it is only valid for remote track.
    virtual WMERESULT GetVid(unsigned int &vid) = 0;

    /**
     * Request video for different quality level, it is used only under cisco multistreaming.
	 * For example, if you start to join a sharing session, your remote video goes to a small window.
	 *  So you will request for a lower resolution video.
     * @param type video quality 90p/180p/... The video quality will be combined with static performance 
     * output, it is not allowed to request video more than the ability of the device.
     * An updated SCR will be sent to other peer (Calliope).
     */
    virtual WMERESULT RequestVideo(WmeVideoQualityType type) = 0;
    
    /**
     * Request video for different quality level with changed priority, duplicate and grouping flags
     * @param type video quality 90p/180p/...
     * @param priority only apply on AS policy, see IWmeMediaConnection::Subscribe
     * @param grouping only apply on AS policy
     * @param duplicate Whether duplication of the same video in different groups was allowed.
     */
    virtual WMERESULT RequestVideo(WmeVideoQualityType type, uint8_t priority, uint8_t grouping, bool duplicate) = 0;
    
    /**
     * A verbose version of RequestVideo with detailed video parameters.
     */
    virtual WMERESULT RequestVideo(uint16_t uMaxFS, uint16_t uMaxFPS, uint32_t uBitrate, uint32_t uMaxDPB,
                                   uint32_t uMaxMBPS, uint8_t priority, uint8_t grouping, bool duplicate) = 0;

    ///Get the Media status of the track, it is only valid for remote track.
    ///The media status only reflect the media status not signal status.
    ///For example, a people muted his video or network congestion would get the same media status.
    virtual WMERESULT GetMediaStatus(WmeMediaStatus &status) = 0;
    
    /** 
     * Send AudioDeviceNotification to audio device controller.
     * Different output device would result in different Echo pattern. 
     * In mobile platform, the output device may be controlled by upper layer, audio engine needs to know the event.
     * This is right now only for android platform.
     * @param notificationType 
     * @param deviceValue (ask Ark Li for detail)
     * @param moreValue (ask Ark Li for detail)
     */
    //Add by Ark
    virtual WMERESULT AudioDeviceNotification(WmeDeviceNotification notificationType, int deviceValue = 0,
                                              int moreValue = 0) = 0;
    
    ///Get Video track statistics for multi rx tracks case.
    virtual WMERESULT GetVideoStatistics(WmeVideoStatistics &statistics) = 0;
    
    ///Start to preview a device, this is different from Start.
    ///In some platform (windows), the device can only be opened once.
    ///Preview will allow to share the same device while it was already in Started status.
    ///Start was not allowed to share the same device.
    virtual WMERESULT StartPreview() = 0;
 
    /// Get the subscribed stream count in latest received SCR.
    /// Only applied for local track
    virtual WMERESULT GetSuscribedCount(unsigned long &count) = 0;
    
    /// set the audio play buffer time in milliseconds, it is only valid for remote track
    /// (Shall we move this to AudioConfig?)
    /// @param nMinDelay minimus delay in play buffer
    /// @param nMaxDelay the max delay should not exceed 5000ms
    virtual WMERESULT SetAudioPlayBufferTime(int nMinDelay,int nMaxDelay)=0;
    /// Get the audio play buffer time in ms, it is only valid for remote track
    /// (Shall we move this to AudioConfig?)
    virtual WMERESULT GetAudioPlayBufferTime(int &nMinDelay,int &nMaxDelay)=0;
  
    /// Change the camera orientation
    /// for winphone only, other platforms, there is other API to cover this.
    virtual WMERESULT ChangeOrientation(WmeCameraOrientation eOri) = 0;
    
    ///// FOR TRAIN ONLY
    ///Force to generate key frame
    /// Only applied for local video/share track
    virtual WMERESULT ForceKeyframe() = 0;
    ///Get the timestamp of the last decoded frame, for SVS, train only
    virtual WMERESULT GetLastDecodedTimestamp(uint32_t &nLastTimestamp) = 0;
    ///Get external inputter from CWmeLocalVideoExternalTrack for SVS
    virtual WMERESULT GetExternalInputPin(IWmeExternalInputter** pWmeExternalTrackSink) = 0;
    ///Set actual SVS source info (eg. resolution) to CWmeLocalVideoExternalTrack
    virtual WMERESULT SetSourceInfo(WmeVideoSourceInfoType* pInfo) = 0;
    ///Force local video to landscape mode. Train mobile UI can only accept landscape video.
    ///only applied for local video/share track
    virtual WMERESULT ForceLandscape(bool landscape) = 0;
    
    //
    virtual WMERESULT SetMediaBlockCheckingTimerInterval(uint32_t nTimerInterval) = 0;

};

class IWmeMediaConnectionSink
{
public:
    virtual ~IWmeMediaConnectionSink() {}

    /**
     * Callback of IWmeMediaConnection::AddMedia or IWmeMediaConnection::Subscribe
     * @param mid corresponding to mid in AddMedia
     * @param direction media direction
     * @param mediaType media type
     * @param pTrack the created local or remote track, client should call AddRef to retain it and dereference it after it has been unsubscribed or the media connection has been stopped.
     */
    virtual void OnMediaReady(unsigned long mid, WmeDirection direction,
        WmeSessionType mediaType, IWmeMediaTrackBase *pTrack) = 0;
    
    /**
     * The callback of IWmeMediaConnection::CreateOffer or IWmeMediaConnection::CreateAnswer, the SDP for media negotiation.
     * @param sdpType It could be OFFER or ANSWER
	 * @param sdp the SDP string
     */
    virtual void OnSDPReady(WmeSdpType sdpType, const char *sdp) = 0;
    
    /**
     * Called back when remote video is frozen, client can display some UI indicator, eg, spining circle.
     * This API was also used to indicate the video was decoded and ready for render.
     * For example, remote muted its video and then unmute, the signal message may come earlier than video was decoded.
     * In such a case, a frozen video frame will be shown and get confusion. Client can defer the video show up upon 
     * this callback to make sure the video was ready for render.
     * @param vid The virtual ID of the stream, it is the return value of Subscribe, for active video it is always 0.
     */
    virtual void OnMediaBlocked(unsigned long mid, unsigned long vid, bool blocked) = 0;
    
	///When the decoded video resolution changed,
    ///@param vid The virtual ID of the stream, it is the return value of Subscribe, for active video it is always 0.
    virtual void OnDecodeSizeChanged(unsigned long mid, unsigned long vid, uint32_t uWidth, uint32_t uHeight) = 0;

    ///When the encoded video resolution changed, such as receiving SCR with landscaped preferred.
    virtual void OnEncodeSizeChanged(unsigned long mid, uint32_t uWidth, uint32_t uHeight) {};
    
    ///When the video render size changed, such as resolution or aspect ratio
    virtual void OnRenderSizeChanged(unsigned long mid, unsigned long vid, WmeDirection direction,
                                     WmeVideoSizeInfo *pSizeInfo) {};
    
	/** 
     * When the ICE session status has been changed.
     * MediaConnection will try to connect to the remote peer for ever, so upper layer
     * should start a timer once it entered Disconnected status. If the timer is out but status is still
     * Disconnected, client should stop the MediaConnection and notify the user.
     *@plantuml icestate
     [*] --> Connecting : SetReceivedSDP
     Connecting --> Connected: ICE success
     Connected --> Disconnected: send error or not receiving for 10s
     Reconnected --> Disconnected: send error or not receiving for 10s
     Connecting --> Disconnected: ICE failure
     Disconnected --> Reconnected: ICE reconnect success
     Disconnected --> Disconnected: ICE reconnect failure
     @endplantuml
     */
    virtual void OnSessionStatus(unsigned long mid, WmeSessionType mediaType, WmeSessionStatus status) = 0;
    
    /**
     * mediastatus changed when we received SCA for that channel.
     * @param hasCSI if hasCSI is true, the csi is from the peer whose video is not available. 
     *  The CSI could be audio CSI or video CSI. Client should find in the participant list to find the people 
     *  and do not rely on the audio/video bit mask to check equal because one might have multiple audio sources.
     */
    virtual void OnMediaStatus(unsigned long mid, unsigned long vid, WmeMediaStatus status, bool hasCSI, uint32_t csi) = 0;
    
    /// The availabe media in SCA has changed, client can adjust the subscription based on this information.
    virtual void OnAvailableMediaChanged(unsigned long mid, int nCount) {}
    
    /// Called back when an error happened in one particular media.
    virtual void OnMediaError(unsigned long mid, unsigned long vid, WMERESULT errorCode) {}

    /**
     * For the purpose of this API, see http://tools.ietf.org/html/draft-ietf-avtcore-rtp-circuit-breakers-06
     * WmeNetwork_bad - Callback when bad network(NETWORK_SEVERE or NETWORK_USELESS) for a while, will notify only when switching from good to bad
     * WmeNetwork_recovered - Callback when good network(NETWORK_VERYGOOD or NETWORK_EXCELLENT) for a while, will notify only when switching from bad to good
     * WmeNetwork_video_off - Callback when QoS already tried to turn down the bitrate and video resolution, but still doesn't help
     * In this case, it worth trying stop video to guarantee audio quality
     */
    virtual void OnNetworkStatus(WmeNetworkStatus status, WmeNetworkDirection direc) {}
    
    /// General error callback from media connection.
    virtual void OnError(WMERESULT errorCode) = 0;
    
	/// When the contributers of one remote track are changed.
    virtual void OnCSIsChanged(unsigned long mid, unsigned int vid,
		const unsigned int* oldCSIArray, unsigned int oldCSICount, 
		const unsigned int* newCSIArray, unsigned int newCSICount) {}
    
    /// for winphone only
    virtual void OnAudioTimeStampChanged(uint32_t csi, uint32_t timestamp) {}
    
    /// only for train SVS. Client need put raw data for pause status
    virtual void OnPictureLossIndication() {}
};
    
typedef enum {
    WmeStunTrace_Stopped = -2,
    WmeStunTrace_Error = -1,
    WmeStunTrace_Success  = 0,
    WmeStunTrace_Timeout,
    WmeStunTrace_Unreachable,
    WmeStunTrace_ServerDown,
    WmeStunTrace_InternalUnreachable,
    WmeStunTrace_ExtUnreachable
}WmeStunTraceResult;
    
///Stun trace callback interface
class IWmeStunTraceSink {
public:
    /**
     * Stun trace result callback
     * @param reason the stun trace result
     * @param szDetail is JSON array in string. The IP address in the path has been hidden because privacy reason. result "to" means timeout, "un" means ICMP unreachable was received. In "unreachable" mode, icmp_code/icmp_type was in the node which indicates the reason of ICMP unreachable. The last node in the trace was "dest" type which is our destination media server. A splunk dashboard was created for stun trace analysis: https://splunk.wbx2.com/en-US/app/squared/ice_stun.
     <pre>
     {
         "dest": "10.224.166.110",
         "ice": "succeeded",
         "mapped": "10.140.49.112",
         "mapped_rtt": 0,
         "networkType": "Wifi",
         "ret": "succeeded",
         "server_rtt": 6.376,
         "tcp": true,
         "trace": [
             {
             "rtt": 15.816,
             "ttl": "1"
             }
             {
             "result": "to",
             "rtt": -1,
             "ttl": "3"
             },
             {
             "rtt": 4.773,
             "ttl": "4",
             "type": "dest"
             }
         ]
     }
     </pre>
     */
    virtual WMERESULT OnResult(WmeStunTraceResult reason, const char* szDetail) = 0;
};

///StunPing callback interface
class IWmeTraceServerSink {
public:
    /**
     * StunPing result of last call to StartTraceServer.
     * @param reason StunPing result.
     * @param szDetail The RTT of pinged nodes, "to" means timeout. Here is an example:
     <pre>
     {
        "squared.US-EAST.*": {
            "udp": {
                "latencyInMilliseconds": "45",
                "reachable": "true"
            }
        },
        "squared.US-WEST.*": {
            "udp": {
                "reachable": "false"
            }
        }
     }
     </pre>
     */
    virtual WMERESULT OnTraceServerResult(WmeStunTraceResult reason, const char* szDetail) = 0;
};

#ifdef __cplusplus
extern "C"
{
#endif
/**
 * Init WME, it should be called when app was started. 
 * It will prepare some internal environments and speed up the device open when user starts a call.
 * It maintains a reference count, so only the first call to it will initialize the environment.
 * @param bRemoteIO Would remote IO be used for iOS (audio)
 * @param bUseTC would the TC AEC algorithm be used. (audio)
 * @param bTimerAllTime deprecated, keep it default (false)
 * @param bIOSSessionDisable audiosession-control-disable for iOS (audio)
 */
WME_EXPORT void InitMediaEngine(bool bRemoteIO, bool bUseTC = false, bool bTimerAllTime = false,
                                bool bIOSSessionDisable = true);
    
/// It should be called when the app will exit, it MUST be paired with wme::InitMediaEngine.
WME_EXPORT void UninitMediaEngine();
    
/// Create MediaConnection instance
/// The lifetime of MediaConnection would be during the call. MediaConnection is not reusable.
/// @param ppMediaConnection output MediaConnection object with reference added.
WME_EXPORT WMERESULT CreateMediaConnection(IWmeMediaConnection **ppMediaConnection);
WME_EXPORT WMERESULT CreateMediaConnectionWithCsi(IWmeMediaConnection **ppMediaConnection, uint32_t csi);
    
///Set a callback to receive stun trace result and post to Splunk
WME_EXPORT void SetStunTraceSink(IWmeStunTraceSink *pSink);
    
///Set a callback to receive stun ping result and post to Orpheus for Linus selection algorithm.
WME_EXPORT void SetTraceServerSink(IWmeTraceServerSink *pSink);
    
/** Start a stun ping to the server list.
 * @param traceInfoList Server list for ping which is JSON returned from Orpheus:
 <pre>
 {
    "squared.US-EAST.*": {
        "udp": ["stun:biston-docker-int-02-public-direct-600.wbx2.com:33434", "stun:biston-docker-int-02-public-direct-600.wbx2.com:3478"],
        "tcp": ["stun:biston-docker-int-02-public-direct-600.wbx2.com:33434"],
        "https": ["https://sx-controller-19.cisco.com/api/v1/ping"],
        "xtls": ["stun:biston-docker-int-02-public-direct-600.wbx2.com:33434"]
    },
    "squared.US-WEST.*": {
        "udp": [ "stun:lqtservice-lqtproda1-403-riad-public.wbx2.com:3478"],
        "tcp": [ "stun:lqtservice-lqtproda1-403-riad-public.wbx2.com:33434"],
        "https": ["https://sx-controller-19.cisco.com/api/v1/ping"],
        "xtls": [ "stun:lqtservice-lqtproda1-403-riad-public.wbx2.com:33434"]
    }
 }
 </pre>
 * @param len the string length of first paramter. It is not the lines of servers, it is string length.
 */
WME_EXPORT WMERESULT StartTraceServer(char* traceInfoList, int len);
    
#ifdef _WIN32
WME_EXPORT void InitMinidump(const char * szDumpFileName);
#endif

#ifdef WP8
WME_EXPORT bool InitThreadManager();
WME_EXPORT void MainThreadHeartbeat();
#endif

#ifdef __cplusplus
}
#endif

} //end of namespace wme

#endif //!define __WEBEX_SQUARE_WME_MEDIA_CONNECTION_HPP__
