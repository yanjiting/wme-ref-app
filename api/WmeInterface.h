///
///  WmeInterface.h
///
///
///  Created by Sand Pei on 2013-1-14.
///  Copyright (c) 2013 Cisco. All rights reserved.
///
#ifndef WME_INTERFACE_H
#define WME_INTERFACE_H

#include "WmeEngine.h"
#include "WmeTraceSink.h"

//Note: this should be removed once the std::string has been removed from some API
#ifdef LINUX
#include <string>
#endif

/**
 * @mainpage WME
 *
 * Webex Media Engine (WME) is a powerful, easy-to-use media framework, which integrates audio engine, video engine and media sync. It can be applied to audio/video conference, audio/video transcode, media stream share, and desktop share. 
 * WME can set up fundamental real-time communication via simple C++ APIs. Besides, it also provides rich extended APIs to help users to customize their own modules and integrate them into the media engine to meet the application requirements.

 * Its main features are:

 * @li Media transmission
 * @li Support of Multiple Operation System
 * @li Statistics Report 
 * @li Support both P2P and Client Server structure

 * @li Screen content video in standard format(in plan)
 * @li Support of Multiple Media Codec (audio/video)
 * @li Support of Standard Protocol (SDP/RTP/RTCP/SRTP/SRTCP, etc.) (SDP in plan)
 * @li Support of multistream proposal (in progress)
 * @li Support of MARI[25] proposal (in progress)
 * @li Strong internet adaptation including QoS strategy, prioritized audio and video data, and unequal error protection on packets
 * @li Media Processing (audio gain control/audio echo concealment/audio denoise/video denoise)
 * @li Support of Wideband audio (in plan)
 * @li Audio/Video Sync 
 * @li System capacity (CPU/memory) detection/adaption (in plan)
 * @li Support of external media source (in plan)
 * @li Directly connect to Cisco Telepresence endpoint such as SX20 (in plan)

 *
 * Documentation sections:
 * @li @ref WmeMediaEngine
 * @li @ref WmeSession
 * @li @ref WmeTrack
 * @li @ref WmeSyncBox
 */


/** @defgroup WmeMediaEngine Wme Media Engine
  * @{
  *
  * @brief This page describes the process and functions WmeMediaEngine.
  *
  * Preparation before using the functions provided by WME:
  *
  * @code
  * IWmeMediaEngine        *pMediaEngime = NULL;
  * unsigned long dwReserved = 0;
  * //initilize the environmental variables
  * WmeInit(dwReserved);
  * //create the wme object
  * WmeCreateMediaEngine(&pMediaEngine);
  *		//(optional) preparation of the creation of components
  *		//(optional) create the components
  *		IWmeMediaSession       *pMediaSessionAudio = NULL;
  *		IWmeLocalAudioTrack *pLocalAudioTrack = NULL;
  *		m_pMediaEngine->CreateMediaSession(WmeSessionType_Audio, &pMediaSessionAudio);
  *		m_pMediaEngime->CreateLocalAudioTrack(&pLocalAudioTrack) ;
  *		//(optional) do things wanted
  *		//(optional) delete the components
  *		SAFE_RELEASE(pMediaSessionAudio);
  *		SAFE_RELEASE(pLocalAudioTrack);
  * //delete the created objection
  * SAFE_RELEASE(pMediaEngine);
  * //clean up global environmental settings
  * WmeUninit();
  * @endcode
  *
  * From this point, @c pMediaEngine can be used. 
  *
  */

namespace wme
{

/// Struct of WME SDK version
/// E.g. SDK version is 2.3.0.0, major version number is 2, minor version number is 3, and revision number is 0.
typedef struct  _tagVersion
{
	uint32_t uMajor;				///< The major version number
	uint32_t uMinor;				///< The minor version number
	uint32_t uRevision;				///< The revision number
	uint32_t uReserved;				///< The reserved number, it should be 0.
}WmeVersion;

/// Enum of WME SDK option
typedef enum{
	WmeOption_TraceSink,			///< pValue is pointer of IWmeTraceSink, uSize is sizeof IWmeTraceSink
	WmeOption_TraceMaxLevel,		///< pValue is pointer of WmeTraceLevel, uSize is sizeof WmeTraceLevel
	WmeOption_WorkPath,				///< pValue is pointer of char string, uSize is string length
	WmeOption_DataDumpFlag,			///< pValue is pointer of unsigned int, uSize is sizeof unsigned int, each bit show one flag in WME_DATA_DUMP_FLAG_OPTION
	WmeOption_DataDumpPath,			///< pValue is pointer of char string, uSize is string length
	WmeOption_IOSVPIOEnable,        ///< pValue is pointer of bool, uSize is sizeof bool.  this option need be set before WmeCreateMediaEngine, avaliable iOS only
	WmeOption_IOSSessionDisable,    ///< pValue is pointer of bool, uSize is sizeof bool.  this option need be set before WmeCreateMediaEngine, avaliable iOS only
	WmeOption_TCAECEnable,
    
    WmeOption_QoSMaxLossRatio = 500,   ///< pValue is pointer of float, uSize is sizeof float.  Set max loss ratio for QoS
    WmeOption_QoSMinBandwidth,         ///< pValue is pointer of unsigned int, uSize is sizeof unsigned int. Set min bandwidth for QoS, bytes per second
    WmeOption_QoSAdjustmentPolicy,     ///< pValue is pointer of char string, uSize is string length. Set adjustment policy for QoS
    WmeOption_QoSBandwidth,            ///< Manually bandwidth specified, 0 means not using manually specified.
    WmeOption_QoSInitBandwidth,         ///< pValue is pointer of unsigned int, uSize is sizeof unsigned int. Set init bandwidth for QoS, bytes per second
}WmeMediaEngineOption;

//Network metrics
typedef enum {
    BANDWIDTH_UPGRADING = 0,
    BANDWIDTH_DOWNGRADING,
    BANDWIDTH_OSCILLATION,
    BANDWIDTH_UNKNOWN,
} BandwidthTrend;

const uint16_t MAX_WME_NETWORK_METRICS_HISTOGRAM_SIZE = 21;
const uint16_t MAX_WME_AGG_SMOOTH_DATA_SIZE = 10;
    
typedef struct _tagWmeNetworkMetricStats {
    double vHistogram[MAX_WME_NETWORK_METRICS_HISTOGRAM_SIZE];
    double dMean;
    double dStdDev;
    double dCov;
}WmeNetworkMetricStats;
    
typedef struct _tagWmeAggressiveSmoothData {
    BandwidthTrend eTrend;
    uint32_t uTimeCost;
    double dStartBandwidth;
    double dEndBandwidth;
    double dAggressiveSmoothness;
}WmeAggressiveSmoothData;
    
typedef struct _tagWmeAggregateNetworkMetricStats {
    WmeNetworkMetricStats stBandwidthStats;
    WmeNetworkMetricStats stRecvRateStats;
    WmeNetworkMetricStats stSendRateStats;
    WmeNetworkMetricStats stLossStats;
    WmeNetworkMetricStats stRttStats;
    WmeNetworkMetricStats stQdelayStats;
    float                 fStabilityRatio;
    float                 fBandwidthUsage;
    float                 fbandwidthEnoughRatio;
    uint32_t              uMaxDowngrade;
    bool                  bInherentLoss;
    WmeAggressiveSmoothData stAggressiveSmoothness[MAX_WME_AGG_SMOOTH_DATA_SIZE];
}WmeAggregateNetworkMetricStats;

    
extern "C"
{    
	/// Call this for get WME SDK version.
	WME_EXPORT WMERESULT WmeGetVersion(WmeVersion &stVersion);
    /// call this to get the wme build information, like build number and git revision.
    WME_EXPORT WMERESULT WmeGetBuildInfo(char *szBuildInfo, int &nStrLen);

	/// Call this before using any other interface functions 
    WME_EXPORT WMERESULT WmeInit(uint32_t dwReserved);
    /// Call this after finish using engine library;
    WME_EXPORT WMERESULT WmeUninit();
    
    /// Create media engine instance
    WME_EXPORT WMERESULT WmeCreateMediaEngine(IWmeMediaEngine** ppMediaEngine);
    
    WME_EXPORT WMERESULT WmeCreateVideoRender(void *pWindow, IWmeVideoRender** ppVideoRender);

	/// Call this to export media engine trace to sink object
	WME_EXPORT WMERESULT WmeSetTraceSink(IWmeTraceSink* pSink);
	/// Call this to set max exerpot level of media engine trace
	WME_EXPORT WMERESULT WmeSetTraceMaxLevel(WmeTraceLevel maxLevel);

	/// Set media engine option
	WME_EXPORT WMERESULT WmeSetMediaEngineOption(WmeMediaEngineOption eOption, void* pValue, uint32_t uSize);
    
    /// Get media engine option
    WME_EXPORT WMERESULT WmeGetMediaEngineOption(WmeMediaEngineOption eOption, void* pValue, uint32_t *uSize);
    
    WME_EXPORT WMERESULT WmeGetNetworkMetrics(WmeAggregateNetworkMetricStats& networkMetrics,const char *szConnContext, uint32_t nStrLen);

    WME_EXPORT bool WmeDeviceSupportCVOSend();
    WME_EXPORT bool WmeDeviceSupportCVORecv();
}

}

#endif	// WME_INTERFACE_H
