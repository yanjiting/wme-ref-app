///
///  WmeStatistics.h
///
///
///  Created by Sand Pei on 13-1-24.
///  Copyright (c) 2013 Cisco. All rights reserved.
///

#ifndef WME_STATISTICS_DEFINE_H
#define WME_STATISTICS_DEFINE_H

#include "WmeCommonTypes.h"
#include "WmeDefine.h"
#include <vector>

namespace wme
{

typedef struct _tagNetworkStatistics
{
    bool  bFecEnabled;                  ///< whether fec enabled
    float fLossRatio;                   ///< the lossratio based on last RR
    uint32_t uJitter;                   ///< the moving average of jitter
    uint32_t uRoundTripTime;            ///< the RTT based on last SR/RR
    uint32_t uBytes;                    ///< number of bytes sent/received (local/remote) since session start
    uint32_t uPackets;                  ///< number of packets sent/received (local/remote) since session start
    uint32_t uFailedIOPackets;          ///< number of packets failed in/out (local) since session start
    uint32_t uProcessedPackets;         ///< "received" minus dropped by WME during processing for catching up or whatever else
    uint32_t uBitRate;                  ///< sendrate/recvrate in Bits per second,calculate every second since session start
    uint32_t uOOOPackets;               ///< number of packets arrived out-of-order (out of a total of uPackets) (remote only)
    uint32_t uDupPackets;               ///< number of duplicate packets (out of a total of uPackets) (remote only)
    uint32_t uLostPackets;              ///< number of lost packets since session started (remote only)
    uint32_t uRRtimeWindowMs;           ///< number of milliseconds since last RR.(remote only)
    uint32_t uFECLevel;                 ///< FEC recovery level, 0 means FEC disabled (local only)
    uint32_t uFECRecoveredPackets;      ///< number of recovered packet by FEC since session start (remote only)
    uint32_t uFECBitRate;               ///< FEC sendrate/recvrate in Bits per second, calculate every second since session start
    float fRTPDropRatio;                ///< ratio of dropped packet because packet loss or error since last RR (remote only)
    uint32_t uEvaluatedBW;              ///< Evaluated bandwidth in Bits per second (local only)
    uint32_t uFECBW;                    ///< Bandwidth for FEC in Bits per second (local only)
    float fFECResidualLoss;             ///< Packet lossratio after FEC recovered (local only)
    uint32_t uFecPackets;               ///< number of FEC packets sent/received (local/remote) since session start
    uint32_t uInputBitRate;             ///< bitrate of input date
    uint32_t uAdaptedFrames;            ///< frame count of send adaptor dropped
    uint32_t uDroppedFrames;            ///< frame count of smooth sender dropped
    uint32_t uMaxBufferedMs;            ///< max frame buffer time
}WmeNetworkStatistics;

/**
* SRTP failures in the call
*/
typedef struct _tagSrtpPMetric {
    bool        bEnabled;
    uint32_t    uRTPAuthFail;
    uint32_t    uRTCPAuthFail;
} WmeSrtpStatistics;

typedef WmeSrtpStatistics WmeSrtpMetric;

typedef struct _tagMariMetirc {
    uint32_t uReceiveRate;  //inout
    uint32_t uQdelay;       //inout
    uint32_t uRtt;          //out
    float fLossRatio;       //inout
} WmeMariStatistics;

const int MAX_LOSS_BURST_LEN_BUCKET_SIZE = 11;
const int MAX_LOSS_GAP_LEN_BUCKET_SIZE = 9;
const int MAX_OOO_GAP_LEN_BUCKET_SIZE = 10;// max out of order gap bucket size

typedef struct _BaseBucketInfo {
    int floor;
    int ceil;
    unsigned long int cnt;
} WmeLossBurstBucketInfo, WmeOOOGapBucketInfo;

typedef struct _LossBurstLen {
    WmeLossBurstBucketInfo burstLen[MAX_LOSS_BURST_LEN_BUCKET_SIZE];
    WmeLossBurstBucketInfo gapLen[MAX_LOSS_GAP_LEN_BUCKET_SIZE];
} WmeLossBurstLenStatistics;

typedef struct _OutOfOrderGapLen {
    WmeOOOGapBucketInfo gapLen[MAX_OOO_GAP_LEN_BUCKET_SIZE];
} WmeOOOGapLenStatistics;

typedef struct _BwEventStatistics {
    unsigned int lossEvent;
    unsigned int delayEvent;
} WmeBwEventStatistics;

const int MAX_HISTOGRAM_SIZE = 21;
    
typedef struct _tagSessionStatistics
{
    WmeNetworkStatistics stInNetworkStat;
    WmeNetworkStatistics stOutNetworkStat;
    WmeMariStatistics    stInMariStat;
    WmeMariStatistics    stOutMariStat;
    WmeSrtpStatistics    stSrtpStat;
}WmeSessionStatistics;

const int MAX_STREAM_NUM_IN_STATS = 64;
const int MAX_VID_NUM_IN_EACH_STREAM = 64;

typedef struct _tagTrackNetworkStatistics
{
    float       fLossRatio;           //inout
    float       fRtpDropRatio;        //in
    float       fFecResidualLoss;     //in
    float       fHopByHopLossRatio;   //in
    float       fHopByHopFecResidualLoss; //in
    uint32_t    uJitter;              //inout
    uint32_t    uBytes;               //inout
    uint32_t    uPackets;             //inout
    uint32_t    uBitRate;             //inout
    uint32_t    uSentFailures;         //out
    uint32_t    uProcessedPackages;   //in
    uint32_t    uOutOfOrderCount;     //in
    uint32_t    uDuplicateCount;      //in
    uint32_t    uLostCount;           //in
    uint32_t    uErrorCount;          //in
    uint32_t    uFecRecoveredPackets; //in
    uint32_t    uRtt;                 //out
    uint32_t    uEvaluatedBandwidth;  //out
    uint32_t    uInputBitRate;        //out
    uint32_t    uAdaptedFrames;       //out
    uint32_t    uDroppedFrames;       //out
    uint32_t    uMaxBufferedMs;       //out
} WmeTrackNetworkStatistics;

typedef struct _tagWmeTrackStatistics
{
    uint32_t ssrc;
    uint8_t  vids[MAX_VID_NUM_IN_EACH_STREAM];
    uint8_t  vidCount;
    uint16_t did;                      //out
    WmeTrackNetworkStatistics stNetworkStat;
} WmeTrackStatistics;

typedef struct _tagAudioStatistics
{
    bool bEnableCodecFEC;              ///< [Not support] if true, means codec has turn on FEC function
    uint32_t uCodecFECRecoveredPackets;///< [Not support] number of recovered packet by FEC since session start (remote only)
    uint32_t uJitterDropMs; ///< [Not support] number of ms of dropped by jitter buffer since session start (remote only)
    uint32_t uBitRate;        ///< sendrate/recvrate in Bits per second,calculate this every 1sec since session start
    wme::WmeCodecType codecType;            ///<  get current audio codec type
    uint32_t uEncodeDropMs;                 ///< For local Track only, get encode drop frames in ms.
    
    float fMaxNoiseLevel;                   ///< [Not Support]Max noise level during the statistics, (local only)
    float fMinNoiseLevel;                   ///< [Not Support]Min noise level during the statistics, (local only)
    float fAverageNoiseLevel;               ///< [Not Support]Average noise level during the statistics, (local only)
    
    float fMaxAECLevel;                     ///< [Not Support]Max AEC level during the statistics, (local only)
    float fMinAECLevel;                     ///< [Not Support]Min AEC level during the statistics, (local only)
    float fAverageAECLevel;                 ///< [Not Support]Average AEC level during the statistics, (local only)
    
    float fMaxSpeechLevel;                  ///< Max Speech level during the statistics
    float fMinSpeechLevel;                  ///< Min Speech level during the statistics
    float fAverageSpeechLevel;              ///< Average Speech level during the statistics
    
}WmeAudioStatistics;

typedef struct st_WmeVideoStreamStatistics
{
    uint32_t width;                 ///< the width of encode/decode frame
    uint32_t height;                ///< the height of encode/decode frame
    uint32_t totalframes;           ///< total frames encoded/decoded
    float fBitrate;                 ///< sendrate/recvrate in Bits per second,calculate this every 1sec since session start
    float fAvgBitrate;              ///< uAvgBitRate = number of packets received since session start / number of seconds, calculate this every 1sec since session start
    float fFps;                     ///< encoded fps = number of frames encoded / number of seconds, calculate this every 1 sec
    float fSendBufStat;             ///< send bufer status of this stream
}WmeVideoStreamStatistics;

typedef struct _tagVideoStatistics
{
    WmeVideoStreamStatistics streamStat[4];///< each video stream video statistics
    uint32_t uStreamNum;                    ///< stream number
    uint32_t uWidth;                    ///< the width of encode/decode frame
    uint32_t uHeight;                    ///< the height of encode/decode frame
    float fFrameRate;                    ///< the frame rate in per second,calculate this every one second
    float fBitRate;                        ///< sendrate/recvrate in Bits per second,calculate this every 1sec since session start
    float fAvgBitRate;                    ///< uAvgBitRate = number of packets received since session start / number of seconds, calculate this every 1sec since session start
    uint32_t uIDRReqNum;                ///< number of IDR requests received for local track; IDR requests sent for remote track since session start
    union{
    uint32_t uIDRSentNum;                ///< number of actual IDRs sent for local track since track start
    uint32_t uIDRRecvNum;                ///< number of actual IDR received for remote track since track start
    };
    float fDecoderFrameDropRatio;        ///< [Not support] ratio of dropped frame because of low decode performance calculated every second since session start (remote only)
    uint32_t uExceedMaxNalSizeCount;    ///< number of nals of which size exceed max nal size limitation
    int iPerformanceWarningCount;    ///< times that encoder performance showed problem
    uint32_t uRenderFrameCount;         ///< number of frames rendered for remote track since track start
    uint32_t uEncodeFrameCount;         ///< number of frames encoded for local track since track start
    uint32_t uEcNumCount;               ///< number of EC frames
    bool bHWEnable;                     ///< HW acceleration enable or disable
  
    uint32_t uInSyncMs;
    uint32_t uOutOfSyncMs;          ///< frames that decoder is out-of-sync, remote video track only
    uint32_t uAvgEcRatio;               ///< Avg Ec Ratio when the frames are Ec-ed, remote video track only
  bool bIsCurrentOutOfSync;           ///< whether current second the decoder is correct
    
    float fCaptureFPS;               ///< Local Track: Capture FPS before encoding
    float fFeedToEncoderFPS;         ///< Local Track: FPS of Feed to Encoder
    uint64_t uCapturedFrames;
    uint64_t uFeedToEncoderFrames;
    int nIDRDataLen;                     //Latest IDR data length(Byte).

}WmeVideoStatistics;

typedef struct _tagScreenShareCaptureStatistics
{
    uint32_t m_nTotalCapturedFrames;
    uint32_t m_nMsAverageCaptureTime;//millisecond
    uint32_t m_nDisplayWidth;
    uint32_t m_nDisplayHeight;
}WmeScreenShareCaptureStatistics;

typedef struct _tagScreenShareStatistics
{
    WmeVideoStatistics stVideoStat;
    WmeScreenShareCaptureStatistics stScreenShareCaptureStatistics;
}WmeScreenShareStatistics;
    
typedef struct _tagSyncStatistics
{
    bool bSetCommonTimestamp;           ///< The flag of whether the sync source has set common timestamp
    uint32_t uLatestCommonTimestamp;    ///< The latest common timestamp
    uint32_t uLatestUpdatedTime;        ///< The latest updated time of common timestamp
    uint32_t uMaxUpdatedInterval;       ///< The maximum time interval of common timestamp updating
    uint32_t uRollbackNumber;           ///< The times of common timestamp rollback
    uint32_t uMinRollbackInterval;      ///< The minimum timestamp interval of rollback common timestamp
    uint32_t uMaxRollbackInterval;      ///< The maximum timestamp interval of rollback common timestamp
    bool bDecidedPlay;                  ///< The flag of whether the sync listener has decided to play
    float fDecideDropRatio;             ///< The ratio of decisions to drop frames
    float fDecideSyncPlayRatio;         ///< The ratio of decisions to play frames with synced source
    float fDecideBufferRatio;           ///< The ratio of decisions to buffer frames
    float fDecideAheadPlayRatio;        ///< The ratio of decisions to play frames but ahead of sync source
    float fDecideNoSyncPlayRatio;       ///< The ratio of decisions to play frames without sync source
    uint32_t uMaxDropInterval;          ///< The maximum timestamp interval of decision to drop frames
    uint32_t uMaxBufferInterval;        ///< The maximum timestamp interval of decision to buffer frames
    uint32_t uMaxAheadPlayInterval;     ///< The maximum timestamp interval of decision to play frame but ahead of sync source
    uint32_t uDecideSyncPlayNumber;   ///< The timers of decisions to play frames with synced source
}WmeSyncStatistics;

const int MAX_THREAD_NAME_LEN = 260;
const int MAX_TOP_THREADS = 20;
const int MAX_CORES_NUMBER = 16;
/**
* CPU usage of a thread
*/
typedef struct _tagWmeThreadCpuUsage {
    char szThreadName[MAX_THREAD_NAME_LEN];    ///< The thread name or ID in string
    float fUsage;                            ///< The CPU usage of that thread
} WmeThreadCpuUsage;

/**
* CPU usage of the system
*/
typedef struct _tagWmeCPUUsage {
    float fTotalUsage;                        ///< The sum of cpu usage of cores
    float pfCores[MAX_CORES_NUMBER];        ///< The cpu usage per core
    uint16_t nCores;                        ///< The number of cores we have
    float fProcessUsage;                    ///< The CPU usage of this process as whole
    WmeThreadCpuUsage pThreadUsages[MAX_TOP_THREADS];    ///< The CPU usage of top threads
    uint16_t nThreads;                        ///< The number of top threads
} WmeCpuUsage;
    
/**
 * Memory usage of the system
 */
typedef struct _tagWmeMemoryUsage {
    float fMemroyUsage;            /// *%, value of *.
    uint32_t uMemoryUsed;          ///< KB
    uint32_t uMemoryTotal;         ///< KB
    uint32_t uProcessMemroyUsed;   ///< KB
} WmeMemoryUsage;

typedef struct _tagWmeCpuDescription {
    char szBrandString[256];
    unsigned int uFrequency;
}WmeCpuDescpription;

typedef struct _tagWmeWifiStatistics {
    char szBssid[256];
    int32_t iLinkRate;//Mbps
    int32_t iStrength;
    int32_t iSnr;//dBm
    int32_t iRssi;
    int32_t iNoise;
    int32_t iChannelNumber;
    int32_t iCenterFrequency;
    uint32_t uFrequency;
    uint32_t uPhyMode;
}WmeWifiStatistics;
    
typedef struct _tagWmeDynamicPerfControlStatistics {
    uint32_t uDowngradeCnt;
    uint32_t uUpgradeCnt;
    uint32_t uDurationInSecondFromFirstDowngrade;
}WmeDynamicPerfControlStatistics;
    
}
#endif // WME_STATISTICS_DEFINE_H
