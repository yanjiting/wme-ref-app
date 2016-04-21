//
//  Loopback.h
//  MediaSessionTest
//
//  Created by Soya Li on 5/14/14.
//  Copyright (c) 2014 Soya Li. All rights reserved.
//

#ifndef __MediaSessionTest__Loopback__
#define __MediaSessionTest__Loopback__
#include "ClickCall.h"

class LoopbackCall : public IOrpheusClientSink, public EventSink
{
public:
    static LoopbackCall* Instance()
    {
        static LoopbackCall loopback;
        return &loopback;
    }
    
    LoopbackCall()
//        : m_endCaller(NULL)
        : m_endCallee(NULL)
		, m_linusCaller(this)
		, m_linusCallee(this)
		, m_endCaller(this, true)
    {
    }
    
    ~LoopbackCall()
    {
        stopLoopback();
    }
    
	void startLoopback(void *winRemote, void *winLocal, void *winScreenViewer);

    std::string stopLoopback();
    void resizeLoopback();
    void resumeLoopback(bool resume);
    void getStatistics(WmeAudioConnectionStatistics &audioStats, WmeVideoConnectionStatistics &videoStats);
    void getCpuStatistics(WmeCpuUsage& cpuStats);
    void getNetworkIndex(WmeNetworkDirection d, WmeNetworkIndex &index);
    void getUplinkNetworkStatus(WmeNetworkStatus& status);
	void getScreenStatistics(WmeScreenConnectionStatistics &screenStats);
    void getFeatureToggleStatistics(string &stats);
    void GetNetworkMetrics(WmeAggregateNetworkMetricStats &stStat);
	void createVenue();
	void createConfluence(bool bCaller, const std::string& sdp);
	long startStopTrack(WmeSessionType mediaType, bool bRemote, bool bStart);
	void mute(WmeSessionType mediaType);
	void unmute(WmeSessionType mediaType);
	void mute(WmeSessionType mediaType, bool bMute, bool bLocal);
    virtual void onEvent(Endpoint *origin, const string &type, const string &text = "", const string &extra = "");
    void requestFloor();
    void releaseFloor();
    unsigned int getVoiceLevel(bool bSpk);
    void setCamera(const char *cameraName);
    void startSVS();
    
    void SetAutoSwitchDefaultMic(bool bFlag);
    void SetAutoSwitchDefaultSpeaker(bool bFlag);
    
    // Calabash Testing
    Calabash * getCalabashCallee();
    Calabash * getCalabashCaller();
    
    Endpoint m_endCaller;
    Endpoint *m_endCallee;
    
    COrpheusClient m_linusCaller;
    COrpheusClient m_linusCallee;

protected:
	virtual void OnVenue(const string &venueUrl, COrpheusClient* pOrigin);
	virtual void OnFlowParameters(const string &parameters, const string &url, COrpheusClient* pOrigin);

private:
	void SetupDebugOption(Endpoint* pEnd);
    std::string m_sVenueUrl;
};


#endif /* defined(__MediaSessionTest__Loopback__) */
