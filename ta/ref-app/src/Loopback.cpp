//
//  Loopback.cpp
//  MediaSessionTest
//
//  Created by Soya Li on 5/14/14.
//  Copyright (c) 2014 Soya Li. All rights reserved.
//

#include "Loopback.h"
#include "CmThread.h"
#include "CmThreadManager.h"
#include "CmUtilMisc.h"

void LoopbackCall::SetupDebugOption(Endpoint* pEnd)
{
	unsigned short audioPort = TestConfig::i().m_nAudioPort;
	unsigned short videoPort = TestConfig::i().m_nVideoPort;
	if (pEnd == m_endCallee){
		audioPort += 100;
		videoPort += 100;
	}
	if (TestConfig::i().m_nAudioPort > 0){
		TestConfig::i().m_audioDebugOption["localPortBegin"] = audioPort;
		TestConfig::i().m_audioDebugOption["localPortEnd"] = audioPort + 1;
	}
	if (TestConfig::i().m_nVideoPort > 0){
		TestConfig::i().m_videoDebugOption["localPortBegin"] = videoPort;
		TestConfig::i().m_videoDebugOption["localPortEnd"] = videoPort + 1;
	}
}

void LoopbackCall::SetAutoSwitchDefaultMic(bool bFlag)
{
    if (m_endCallee)
    {
        m_endCallee->SetAutoSwitchDefaultMic(bFlag);
    }
    
    m_endCaller.SetAutoSwitchDefaultMic(bFlag);
}

void LoopbackCall::SetAutoSwitchDefaultSpeaker(bool bFlag)
{
    if (m_endCallee)
    {
        m_endCallee->SetAutoSwitchDefaultSpeaker(bFlag);
    }
    
    m_endCaller.SetAutoSwitchDefaultSpeaker(bFlag);
}

void LoopbackCall::startLoopback(void *winRemote, void *winLocal, void *winScreenViewer)
{
//	m_endCaller = new Endpoint(this, true);
    m_endCaller.m_winCaller = winLocal;
	m_endCallee = new Endpoint(this, false);
    m_endCallee->m_winCallee.push_back(winRemote);
    m_endCallee->m_winScreenViewer = winScreenViewer;

	m_endCaller.preview();
	m_endCallee->preview();
    
    if (TestConfig::i().m_bCalliope)
        createVenue();
    else
        m_endCaller.startCall();

#if defined(WIN32) && !defined(WP8)
	if (TestConfig::i().m_bAppshare && winScreenViewer)
	{
		::ShowWindow((HWND)winScreenViewer, SW_SHOW);
	}
#endif
}

std::string LoopbackCall::stopLoopback()
{
	std::string ret;
//    if(m_endCaller)
    {
		ret = m_endCaller.stopCall();
        SleepMs(100);
        //delete m_endCaller;
        //m_endCaller = NULL;
    }
    
    if(m_endCallee)
    {
		m_endCallee->stopCall();
        SleepMs(100);
        delete m_endCallee;
        m_endCallee = NULL;
    }
	m_linusCallee.DeleteConfluence(false);
	m_linusCaller.DeleteConfluence(true);
	return ret;
}

void LoopbackCall::setCamera(const char *cameraName)
{
    m_endCaller.setCamera(cameraName);
}

void LoopbackCall::startSVS() {
    m_endCaller.startSVS();
}

void LoopbackCall::resizeLoopback()
{
 //   if(m_endCaller)
        m_endCaller.resizeRender();
    if(m_endCallee)
        m_endCallee->resizeRender();
}

void LoopbackCall::resumeLoopback(bool resume)
{
 //   if(m_endCaller)
        m_endCaller.resume(resume);
    if(m_endCallee)
        m_endCallee->resume(resume);
}

void LoopbackCall::onEvent(Endpoint *origin, const string &type, const string &text, const string &extra)
{
    Endpoint *pEnd = (origin == &m_endCaller) ? m_endCallee : &m_endCaller;
	bool bCaller = (origin == &m_endCaller);
	if (TestConfig::i().m_bCalliope){
		createConfluence(bCaller, text);
	}
	else if(pEnd) {
        pEnd->OnSDPReceived(type, text);
	}
}
void LoopbackCall::getScreenStatistics(WmeScreenConnectionStatistics &screenStats){
	if (m_endCallee){
		//Loopback
		WmeScreenConnectionStatistics screenStatsLocal = { { 0 } }, screenStatsRemote = { { 0 } };

		m_endCaller.getScreenStatistics(screenStatsLocal);
		m_endCallee->getScreenStatistics(screenStatsRemote);

		screenStats.connStat = screenStatsRemote.connStat;
		screenStats.connStat.uRTPSent = screenStatsLocal.connStat.uRTPSent;
		screenStats.connStat.uRTCPSent = screenStatsLocal.connStat.uRTCPSent;
		screenStats.sessStat.stInNetworkStat = screenStatsRemote.sessStat.stInNetworkStat;
		screenStats.sessStat.stOutNetworkStat = screenStatsLocal.sessStat.stOutNetworkStat;
		screenStats.localScreenStat = screenStatsLocal.localScreenStat;
		screenStats.remoteScreenStat = screenStatsRemote.remoteScreenStat;
	}
}

void LoopbackCall::getFeatureToggleStatistics(string &stats)
{
    m_endCaller.getFeatureToggleStatistics(stats);
}

void LoopbackCall::getStatistics(WmeAudioConnectionStatistics &audioStats, WmeVideoConnectionStatistics &videoStats)
{
    if(m_endCallee){
        //Loopback
        WmeAudioConnectionStatistics audioStatsLocal = {{0}}, audioStatsRemote = {{0}};
        WmeVideoConnectionStatistics videoStatsLocal = {{0}}, videoStatsRemote = {{0}};
        
        m_endCaller.getStatistics(audioStatsLocal, videoStatsLocal);
        m_endCallee->getStatistics(audioStatsRemote, videoStatsRemote);
        
        audioStats.connStat = audioStatsLocal.connStat;
		audioStats.connStat.uRTPReceived = audioStatsRemote.connStat.uRTPReceived;
		audioStats.connStat.uRTCPReceived = audioStatsRemote.connStat.uRTCPReceived;
		audioStats.sessStat.stInNetworkStat = audioStatsRemote.sessStat.stInNetworkStat;
        audioStats.sessStat.stOutNetworkStat = audioStatsLocal.sessStat.stOutNetworkStat;
        audioStats.localAudioStat = audioStatsLocal.localAudioStat;
        audioStats.remoteAudioStat = audioStatsRemote.remoteAudioStat;
        
        videoStats.connStat = videoStatsLocal.connStat;
		videoStats.connStat.uRTPReceived = videoStatsRemote.connStat.uRTPReceived;
		videoStats.connStat.uRTCPReceived = videoStatsRemote.connStat.uRTCPReceived;
		videoStats.sessStat.stInNetworkStat = videoStatsRemote.sessStat.stInNetworkStat;
        videoStats.sessStat.stOutNetworkStat = videoStatsLocal.sessStat.stOutNetworkStat;
        videoStats.localVideoStat = videoStatsLocal.localVideoStat;
        videoStats.remoteVideoStat = videoStatsRemote.remoteVideoStat;
    }
}
void LoopbackCall::getCpuStatistics(WmeCpuUsage& cpuStats)
{
    if(m_endCallee){
        m_endCallee->GetCpuUsage(cpuStats);
    }
}
void LoopbackCall::getNetworkIndex(WmeNetworkDirection d, WmeNetworkIndex &index)
{
    m_endCaller.getNetworkIndex(d, index);
}

void LoopbackCall::getUplinkNetworkStatus(WmeNetworkStatus& status)
{
    m_endCaller.getUplinkNetworkStatus(status);
}

void LoopbackCall::GetNetworkMetrics(WmeAggregateNetworkMetricStats &stStat)
{
    m_endCaller.GetNetworkMetrics(stStat);
}

void LoopbackCall::createVenue()
{
	if (!TestConfig::i().m_sLinusUrl.empty())
		OnVenue(newUUID(), NULL);
	else
		m_linusCaller.CreateVenue();
}

void LoopbackCall::createConfluence(bool bCaller, const std::string& sdp)
{
	COrpheusClient *orpheus = bCaller ? &m_linusCaller : &m_linusCallee;
	if (TestConfig::i().m_sLinusUrl.empty())
		orpheus->CreateFlow(m_sVenueUrl, sdp, newUUID());
	else
		orpheus->CreateLinusConfluence(TestConfig::i().m_sLinusUrl, m_sVenueUrl, sdp, newUUID());
}
void LoopbackCall::requestFloor(){
    m_linusCaller.RequestFloor();
}
void LoopbackCall::releaseFloor(){
    m_linusCaller.ReleaseFloor();
}
void LoopbackCall::OnVenue(const string &venueUrl, COrpheusClient* pOrigin)
{
	m_sVenueUrl = venueUrl;
    if(m_endCallee)
        m_endCallee->startCall();
	m_endCaller.startCall();
}

void LoopbackCall::OnFlowParameters(const string &parameters, const string &url, COrpheusClient* pOrigin)
{
	if (pOrigin == &m_linusCaller){
		if(TestConfig::i().m_bAppshare           
           && TestConfig::i().m_bAutoRequestFloor)
			m_linusCaller.RequestFloor();
		m_endCaller.OnSDPReceived("answer", parameters);
	}
	else{
		m_endCallee->OnSDPReceived("answer", parameters);
	}
}

long LoopbackCall::startStopTrack(WmeSessionType mediaType, bool bRemote, bool bStart)
{
	long ret = -88;
	if (bRemote){
		if (m_endCallee)
			ret = m_endCallee->startStopTrack(mediaType, bRemote, bStart);
	}
	else{
		ret = m_endCaller.startStopTrack(mediaType, bRemote, bStart);
	}
	return ret;
}

unsigned int LoopbackCall::getVoiceLevel(bool bSpk)
{
    if (bSpk) {
        if (m_endCallee)
            return m_endCallee->getVoiceLevel(true);
        return 0;
    }
    return m_endCaller.getVoiceLevel(false);
}

void LoopbackCall::mute(WmeSessionType mediaType)
{
	if (mediaType == WmeSessionType_Audio) {
        m_endCaller.mute(true, mediaType);
	}
	if (m_endCallee)
		m_endCallee->mute(true, mediaType);
}

void LoopbackCall::unmute(WmeSessionType mediaType)
{
	if (mediaType == WmeSessionType_Audio) {
        m_endCaller.mute(false, mediaType);
	}
	if (m_endCallee)
		m_endCallee->mute(false, mediaType);
}

void LoopbackCall::mute(WmeSessionType mediaType, bool bMute, bool bLocal)
{
	if (bLocal) {
		m_endCaller.mute(bMute, mediaType, bLocal);
	}else {
        if (m_endCallee)
			m_endCallee->mute(bMute, mediaType, bLocal);
    }
}

Calabash * LoopbackCall::getCalabashCallee() {
    if (m_endCallee != NULL)
        return m_endCallee->getCalabash();
    
    return NULL;
}

Calabash * LoopbackCall::getCalabashCaller() {
    return m_endCaller.getCalabash();
}
