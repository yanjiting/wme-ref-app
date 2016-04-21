#ifndef	_WME_MEIDA_INFO_H_
#define	_WME_MEIDA_INFO_H_


#include "WmeError.h"
#include "WmeDefine.h"
#include "WmeCodec.h"

namespace wme {
    
// for audio unify status
    typedef struct _WmeQoEMInfo
{
    typedef struct QoemSpkParams
    {
        typedef struct srcParams
        {
            unsigned int chanID;
            unsigned int ssrc;
            
            int R_factor_netImpair;
            float MOS_netImpair;
            int R_factor;
            float MOS;
            int bitrate;
            int packetRate;
            float energy;
        }SRCPARAMS;
        typedef struct dstParams :public SRCPARAMS
        {
            int packetDropRate;
            int plcPacketRate;
            int forcedSilenceRate;
            int jitter;             // in ms
            int loopDelay;          // in ms
            float fastPlay;
        }DSTPARAMS;
        int status; //-1: data is not valid(initial silent data)
        // 0: silet audio with previous valid data
        // 1: contain valid data
        SRCPARAMS src;
        DSTPARAMS dst;
    }QOEMSPKPARAMS;
    typedef struct QoemAvgMaxQoeStat
    {
        unsigned int maxLossSSRC;
        unsigned int maxJitterSSRC;
        unsigned int d2cLossAvg;
        unsigned int d2cLossMax;
        unsigned int d2cJitterAvg;
        unsigned int d2cJitterMax;
        int e2eLossAvg;
        unsigned int e2eLossMax;
        int e2eJitterAvg;
        int e2eJitterMax;
        int jitDepthAvg;
        int jitDepthMax;
        float playSpeedAvg;
        float playSpeedMax;
        float netMosAvg;
        float netMosMin;
        float jitMosAvg;
        float jitMosMin;
        int reportCount;
    }QOEMAVGMAXQOESTAT;
    //
    QOEMSPKPARAMS localUser;
    int spkNum;
    QOEMSPKPARAMS speaker[10];          //only supoort
    QOEMAVGMAXQOESTAT avgMaxStatus;
    }WmeAEQoEMInfo;
    
typedef struct WmeAE_QOEMStatics
{
        int aecType;
    int spkNum;
        float mosCount[5];// seconds
        float mosCount2[5];// seconds(the new Mos from CC)
        float silentCount;// seconds
        /*
        float duration; // seconds
        float captured; // seconds
        float received; // seconds
        struct {
            float min;
            uint32 ssrc;
        }mos;
        struct {
            uint32 max;
            uint32 ssrc;
        }delay;
        struct {
            float max;
            uint32 ssrc;
        }lostRate;
        //
        // AEC metrics
        // ??
        // AGC metrics
        // ??
        // other
        // ??
        */
    }WmeAEMetics;
// Enum of WME audio engine option
typedef enum _WmeMediaInfo_AudioEngine{
    WmeMediaInfo_AE_CaptureDataTimes =0,//
    WmeMediaInfo_AE_AECType,        //for AEC using in audio engine,0:default engine,1:webrtc aec,2:tc aec,3:Alpha aec
    WmeMediaInfo_AE_VPIOMode,       ///for ios platform, is the caputre engine using VPIO mode,0:VPIO mode,1:remoteIO mode
        WmeMediaInfo_AE_QoEM_Info,   ///for QOEM, get the report info struct WmeAEQoEMInfo
        WmeMediaInfo_AE_QoEM_Metrics,   ///for QOEM, get the WmeAEMetics,
    WmeMediaInfo_AE_QoEM_Enable,    ///for QOEM, enable or disable qoem function, the default value in Qoem is disable
    WmeMediaInfo_AE_Metrics, //for capture/playback/network data percentage during the call
}WmeMediaInfo_AudioEngine;
    
typedef struct tagWmeAEAudioDataStatics
{
    float fPer0_10;
    float fPer10_30;
    float fPer30_50;
    float fPer50_80;
    float fPer80_100;
}WmeAEAudioDataStatics;


typedef struct tagWmeAEAudioDataMetics
{
    WmeAEAudioDataStatics CaptureData;
    WmeAEAudioDataStatics RenderData;
    WmeAEAudioDataStatics NetworkData;
    WmeAEAudioDataStatics EncodeBitrateData;
    WmeAEAudioDataStatics DecodeBitrateData;
    WmeAEAudioDataStatics EncodeDropPacketData;
    unsigned int nStartCaptureResult;
    unsigned int nStartPlaybackResult;
}WmeAEAudioDataMetics;

typedef struct _WmeMediaInfo_VideoMax {
	int32_t maxWidth;
	int32_t maxHeight;
	int32_t maxLayerNum;
	float   maxFrameRate;
    bool    enableSimulcastAVC;
}WmeMediaInfo_VideoMax;

typedef enum {
	WmeWIC_VideoMax,			//To: WmeMediaInfo_VideoMax
}WmeMediaInfoConfiguration; 

struct WmeMediaBaseCapability 
{
	WmeMediaType eMediaType;	//To distinguish video/audio/data structure
	WmeCodecType eCodecType;
};

#define MAX_FRAME_LAYER_NUM     4
struct WmeVideoMediaCapability : public WmeMediaBaseCapability {
	WmeVideoMediaCapability() :	width(0), height(0), profile_level_id(0), max_mbps(0), max_fs(0), frame_layer_number(0) 
	{eMediaType = WmeMediaTypeVideo ;}
	uint32_t width;
	uint32_t height;
	uint32_t profile_level_id;
	uint32_t max_mbps;
	uint32_t max_fs;
	int32_t frame_layer_number;
    uint32_t  min_bitrate_bps;
	uint32_t frame_layer[MAX_FRAME_LAYER_NUM];
};

struct WmeAudioMediaCapability : public WmeMediaBaseCapability {
	WmeAudioMediaCapability() : uPayload(0), clockrate(0), channels(0), rate(0), uptime(0), maxptime(0), maxclockrate(0), useinbandfec(0), usedtx(0)
	{memset(stdname, 0, sizeof(char)*1024); stdnameLen = 0; eMediaType = WmeMediaTypeAudio;}
	uint32_t     uPayload;
	char stdname[1024];
	uint32_t stdnameLen;
	uint32_t clockrate;//samplerate
	//uint32_t ptime; //frame size in milliseconds 
	uint32_t channels;//
	uint32_t rate;	// average bandwidth

	//SDP request
	uint32_t uptime ;//frame size in milliseconds 
	uint32_t maxptime;//max interval between packets
	uint32_t maxclockrate;//max samplerate
	uint32_t useinbandfec;//fec in opus
	uint32_t usedtx;// DTX in opus/iLbc

};

class WME_NOVTABLE IWmeMediaInfo : public IWmeUnknown
{
public:
    virtual WMERESULT SetEnableSimulcast(bool bEnable) = 0;
	virtual WMERESULT GetMediaType(WmeMediaType & type) = 0;
	virtual WMERESULT GetCapabilityNumber(int32_t &iNumber) = 0;
	virtual WMERESULT GetMediaCapabilities(int iIndex, WmeMediaBaseCapability* pInfo, int32_t size) = 0;
};

class WME_NOVTABLE IWmeMediaInfoGetter : public IWmeUnknown
{
public:
	virtual WMERESULT SetConfig(WmeMediaInfoConfiguration config, void* param, int32_t size) = 0;
	virtual WMERESULT GetInfoNumber(int32_t &iNumber) = 0;
	virtual WMERESULT GetMediaInfo(int iIndex, IWmeMediaInfo **ppMediaInfo) = 0;
};


}	//namespace



#endif //_WME_MEIDA_INFO_H_
