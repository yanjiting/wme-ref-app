#pragma once

#include "json/json.h"
#include "MediaConnection.h"
#include "BackdoorAgentSink.h"

class IAppSink
{
public:
    virtual ~IAppSink() {}

    virtual void ShowWindow(void *pOverlapWindow, bool bShow) {}
    virtual void ShowMuteWindow(void *pOverlapWindow, bool bShow) {}
    virtual void onCheckMultiVideo() = 0;
    virtual void onVenueUrl(std::string sVenueUrl) {}
    virtual bool onCheckHWEnable() { return true; }
    virtual void onSdpReady(std::string from) {}
    virtual void onCallback(std::string key, std::string value) {}
	virtual void onQuitWP8() {}


};

class CBackdoorAgent : public IBackdoorSink
{
public:
	typedef std::string(CBackdoorAgent::*BackDoorFunc)(std::string arg);

	CBackdoorAgent(const char*szRefPath, void *winRemote, void *winLocal, void *winScreenViewer, IAppSink* sink = NULL);
	~CBackdoorAgent();

	virtual bool OnRequest(std::string selector, std::string arg, std::string &result);
	std::map<std::string, BackDoorFunc> m_mapBackdoors;

public:
    
    std::string backdoorTestEcho(std::string arg);
    std::string backdoorCalMos(std::string arg);
    
	std::string backdoorStartCall(std::string arg);
	std::string backdoorIsFileCaptureEnded(std::string arg);
	std::string backdoorGetLocalSdp(std::string arg);
	std::string backdoorSetRemoteSdp(std::string arg);
	std::string backdoorGetStatistics(std::string arg);
    std::string backdoorGetFeatureToggles(std::string arg);
    std::string backdoorGetNetworkMetrics(std::string arg);
    std::string backdoorSetManualBandwidth(std::string arg);
	std::string backdoorStopCall(std::string arg);
	std::string backdoorMuteUnMute(std::string arg);
	std::string backdoorStartStopTrack(std::string arg);
    
    std::string backdoorUpdateSdp(std::string arg);
    std::string backdoorSetRemoteOffer(std::string arg);

	std::string backdoorSetParam(std::string arg);
	std::string backdoorGetParam(std::string arg);
    std::string backdoorGetVideoParameters(std::string arg);
    std::string backdoorGetAudioParameters(std::string arg);
    
    std::string backdoorSetAudioParam(std::string arg);
    std::string backdoorSetVideoParam(std::string arg);
    std::string backdoorSetShareParam(std::string arg);
    
    std::string backdoorGetCSICount(std::string arg);
    std::string backdoorGetCSIChangeHistory(std::string arg);
    std::string backdoorGetMediaStatus(std::string arg);
    std::string backdoorGetVideoCSI(std::string arg);
    
    std::string backdoorCheckHWSupport(std::string arg);
    std::string backdoorGetMemory(std::string arg);

	//Screen sharing
//	std::string backdoorSetSharingContext(std::string arg);
//	std::string backdoorStartSharing(std::string arg);
	std::string backdoorStopSharing(std::string arg);
	std::string backdoorGetSharingStatus(std::string arg);
	std::string backdoorGetSharingResult(std::string arg);
//	std::string backdoorAddOneResource(std::string arg);
	std::string backdoorSetQRCodeContext(std::string arg);
	std::string backdoorCollectReceivedQRCodeContents(std::string arg);
    std::string backdoorRequestFloor(std::string arg);
    std::string backdoorScreenChangeCaptureFps(std::string arg);
    std::string backdoorSetScreenFileCaptureFilePath(std::string arg);
    std::string backdoorSubscribe(std::string arg);
    std::string backdoorRequestVideo(std::string arg);
    std::string backdoorOutputTrace(std::string arg);
    std::string backdoorGetVoiceLevel(std::string arg);
    std::string backdoorOverridePerfJson(std::string arg);
    
    //lip sync
    std::string backdoorCheckSyncStatus(std::string arg);
    
    //mari fec
    std::string backdoorSetFecParam(std::string arg);
    
    //mari rate adaptation
    std::string backdoorSetFeatureToggles(std::string arg);
    
    std::string backdoorGetCPUCores(std::string arg);
	std::string backdoorQuitWP8(std::string arg);

    
    //stuntrace
    std::string backdoorGetStunTraceResult(std::string arg);
    
    //traceserver
    std::string backdoorGetTraceServerResult(std::string arg);
    
    //misc
    std::string backdoorConfig(std::string arg);
    
    std::string backdoorGetAssertionsCount(std::string arg);
    
    std::string backdoorChangeLocalTrack(std::string arg);

protected:
	std::string _ScreenSharingRuningStatus();
	static wme::WmeSessionType ofSessionType(std::string sType);
    static wme::WmeDirection ofDirection(std::string sDirection);
    void LoadScreenSource();
    void UpdateScreenCaptureFile();
    void UpdateFileCaptureRender(Json::Value &root);

private:
	Json::Value GetStatsConnInfo(wme::WmeConnectionStatistics& connStats);
	Json::Value GetStatsNetworkInfo(wme::WmeNetworkStatistics &netStats);
	Json::Value GetStatsAudioInfo(wme::WmeAudioStatistics &audioStats);
	Json::Value GetStatsVideoInfo(wme::WmeVideoStatistics &videoStats);
    Json::Value GetStatsShareInfo(wme::WmeScreenShareStatistics &shareStats);
    Json::Value GetRunningStats(wme::WmeNetworkMetricStats& runningStats);
    Json::Value GetAggressiveness(wme::WmeAggregateNetworkMetricStats& networkMetrics);
    Json::Value GetSmoothness(wme::WmeAggregateNetworkMetricStats& networkMetrics);
    Json::Value GetOscillation(wme::WmeAggregateNetworkMetricStats& networkMetrics);

	std::string ExpandPath(std::string relPath);
	std::string m_sRefPath;
	void *m_winRemote;
	void *m_winLocal;
	void *m_winScreenViewer;
    IAppSink *m_sink;
};
