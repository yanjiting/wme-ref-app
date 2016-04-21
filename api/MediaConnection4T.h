/**
 *   MediaSession interface (similar to webRTC PeerConnection, but simplified)
 *
 */

#ifndef __WEBEX_SQUARE_WME_MEDIA_CONNECTION_4T_HPP__
#define __WEBEX_SQUARE_WME_MEDIA_CONNECTION_4T_HPP__

#pragma once
#include "MediaConnection.h"

namespace wme
{

    
typedef enum
{
    WmeVideoPerformanceLow = 0,	    ///< Low performance
    WmeVideoPerformanceMedium = 1,	///< Medium performance
    WmeVideoPerformanceNormal = 2,	///< Normal performance
    WmeVideoPerformanceHigh = 3,	///< High performance

    WmeVideoPerformanceNone = -1,   ///< No video
}WmeVideoPerformance;
    
typedef struct _tagRtpFilter
{
    uint8_t vid;                    ///< vid for filter
    uint32_t correctCsi;            ///< the expected CSI for that vid, un-desired CSI should be dropped.
}WmeRtpFilter;

class IWmeMediaConnection4T : public IWmeMediaConnection
{
public:
    virtual ~IWmeMediaConnection4T() {}
    
    /// Set to use external transport instead of built-in ICE, so you can get the RTP packets emitted by media engine.
    /// Set it to NULL when transport is disconnected, you can change the transport when it has been changed
    /// @param bTransportChanged QoS module need to know when underlying transport type changed.
    virtual WMERESULT SetTransport(unsigned long mid, IWmeMediaTransport *transport, bool bTransportChanged = false) = 0;

    /// Set the encryption methond to override the built-in SRTP
    virtual WMERESULT SetMediaContentCipher(unsigned long mid, IWmeMediaTrackBase* track, IWmeMediaContentCipher* cipher) = 0;
    /// Turn on some propriatery RTP extensions for media switch
    virtual WMERESULT TurnOnRTPHeaderExtensions(unsigned long mid, WmeRTPHeaderExtType pExtTypes[], uint8_t uExtCount) = 0;
    virtual WMERESULT QueryRTPExtensionID(unsigned long mid, const char *extURI, eStreamDirection dir, uint8_t &id) = 0;
    /// Pass the RTP package received from external transport to WME
    /// @param correctVid override the vid in the RTP packet
    virtual WMERESULT ReceiveRTP(unsigned long mid, uint8_t *pRTPData, uint32_t uRTPSize, WmeRtpFilter *filter = NULL,
                                 uint8_t *correctVid = NULL) = 0;
    /// Pass the RTCP package received from external transport to WME
    virtual WMERESULT ReceiveRTCP(unsigned long mid, uint8_t *pRTCPData, uint32_t uRTCPSize) = 0;
    
    /// Create an ad-hoc audio track to play beep, this can also be accomplished by other system API.
    /// Train wants to keep it in the same device as the VoIP.
    /// @param trackId the unique ID for the new created track.
    virtual WMERESULT CreateAdhocAudioTrack(unsigned int trackId) = 0;
    /// Set the ad-hoc RTP packets to ad-hoc track to play
    virtual WMERESULT ReceiveAdhocRTP(unsigned int trackId, uint8_t *pRTPData, uint32_t uRTPSize) = 0;
    
    /// Explicitly tell WME it is starting to fail over, RTCP session needs to reset the SCR sequences.
    virtual WMERESULT Failover(unsigned long mid) = 0;
    /// Force to indicate a picture loss, for SVS
    virtual WMERESULT IndicatePictureLoss(unsigned long mid, unsigned int vid) = 0;
    
    /// Add extra local track for SVS
    virtual WMERESULT AddLocalTrack(unsigned long mid, uint32_t csi, uint8_t &vid) = 0;
    /// Remove extra local track, for SVS
    virtual WMERESULT RemoveLocalTrack(unsigned long mid, uint32_t csi) = 0;
    /// Set active local track, for SVS, bandwidth allocation
    virtual WMERESULT SetActiveLocalTrack(unsigned long mid, uint32_t csi) = 0;

    /// Update CSI for SVS, SVS csi was assigned by upper layer.
    virtual WMERESULT UpdateCSI(unsigned long mid, uint32_t label, uint32_t csi) = 0;
};
    
#ifdef __cplusplus
extern "C"
{
#endif

/// Create IWmeMediaConnection4T object
/// if svs is true, we would use cid as csi directly. And we would change qos priority the same as sharing for svs.
/// @param cid The call ID of the media connection, audio/video should be same to do AV sync. We suggest it to be nodeid of WebEx.
WME_EXPORT WMERESULT CreateMediaConnection4T(uint32_t cid, IWmeMediaConnection4T **ppMediaConnection,
                                             bool bSVS = false, bool bExtTransport = true);
    
/// Create video render with HWND (View) this is used to overlay button over the video window.
WME_EXPORT WMERESULT CreateVideoRender(void *pWindow, IWmeVideoRender** ppVideoRender);
    
/// Get the CPU information for display in UI.
WME_EXPORT WMERESULT GetCpuDescription(WmeCpuDescpription &cpuDesc);
    
/// Get the static performance level determined by WME.
/// @param bHardware If hardware is enabled, it is false for WebEx desktop client right now.
/// @return wme::WmeVideoPerformance
WME_EXPORT wme::WmeVideoPerformance GetStaticPerformance(bool bHardware = false);

#ifdef __cplusplus
}
#endif

} //end of namespace wme

#endif //!define __WEBEX_SQUARE_WME_MEDIA_CONNECTION_4T_HPP__
