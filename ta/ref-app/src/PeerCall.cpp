//
//  PeerCall.cpp
//  MediaSessionTest
//
//  Created by Soya Li on 5/14/14.
//  Copyright (c) 2014 Soya Li. All rights reserved.
//

#include "PeerCall.h"
#include "CmUtilMisc.h"
#include "BackdoorAgent.h"

void PeerCall::pushRemoteWindow(void *winRemote)
{
    m_endCaller->m_winCallee.push_back(winRemote);
}

void PeerCall::startPeer(void *winLocal, void *winScreenViewer, IAppSink *pSink)
{
    TestConfig::i().m_pAppSink = pSink;
	m_bActive = true;
    m_endCaller->m_winCaller = winLocal;
    m_endCaller->m_winScreenViewer = winScreenViewer;

    m_endCaller->preview();

#if defined(WIN32) && !defined(WP8)
	if (TestConfig::i().m_bAppshare && !TestConfig::i().m_bSharer && winScreenViewer)
	{
		::ShowWindow((HWND)winScreenViewer, SW_SHOW);
	}
#endif
}

void PeerCall::createVenue()
{
	if (!TestConfig::i().m_sLinusUrl.empty())
		OnVenue(newUUID(), NULL);
	else
		m_linusCall.CreateVenue();
}

void PeerCall::requestFloor(){
    m_linusCall.RequestFloor();
}
void PeerCall::releaseFloor(){
    m_linusCall.ReleaseFloor();
}

void PeerCall::setCamera(const char *cameraName)
{
    if(m_endCaller)
    {
        m_endCaller->setCamera(cameraName);
    }
}

void PeerCall::startSVS() {
    if(m_endCaller) {
        m_endCaller->startSVS();
    }
}

void PeerCall::createConfluence(const std::string& sdp)
{
	if (TestConfig::i().m_sLinusUrl.empty())
		m_linusCall.CreateFlow(m_sVenueUrl, sdp, newUUID());
	else
		m_linusCall.CreateLinusConfluence(TestConfig::i().m_sLinusUrl, m_sVenueUrl, sdp, newUUID());
}

std::string PeerCall::stopPeer()
{
	std::string ret;
    if(m_endCaller)
    {
		ret = m_endCaller->stopCall();
    }
    disconnect();
    bool deleteVenue = true;
    if(m_ws)
    {
        deleteVenue = (m_ws->GetCount()<=1);
    }
	m_linusCall.DeleteConfluence(deleteVenue);
    m_bActive = false;
	return ret;
}

void PeerCall::connect(const char *server)
{
    if (TestConfig::i().m_bAudioProcess){
        // for audio bg process, we startcall without connect signal server.
        m_endCaller->startCall();
        return;
    }

    if (TestConfig::i().m_bNoSignal){
        if (TestConfig::i().m_sVenuUrl.empty())
            createVenue();
        else
            OnSDPReceived("venue", TestConfig::i().m_sVenuUrl);
        return;
    }

    char dest[256] = {0};
    snprintf(dest, 256, "ws://%s", server);
    CM_INFO_TRACE_THIS("Endpoint, websocket, dest=" << dest);
    if(m_ws == NULL)
        m_ws = new CWsClient(this);
    m_ws->Open(dest);
}

long PeerCall::startStopTrack(WmeSessionType mediaType, bool bRemote, bool bStart)
{
	long ret = -90;
	if (m_endCaller)
		ret = m_endCaller->startStopTrack(mediaType, bRemote, bStart);
	return ret;
}

unsigned int PeerCall::getVoiceLevel(bool bSpk)
{
    if (m_endCaller)
        m_endCaller->getVoiceLevel(bSpk);
    return 0;
}

void PeerCall::mute(WmeSessionType mediaType)
{
	if (m_endCaller)
		m_endCaller->mute(true, mediaType);
}

void PeerCall::unmute(WmeSessionType mediaType)
{
	if (m_endCaller)
		m_endCaller->mute(false, mediaType);
}

void PeerCall::mute(WmeSessionType mediaType, bool bMute, bool bLocal)
{
	if (m_endCaller)
		m_endCaller->mute(bMute, mediaType, bLocal);
}

void PeerCall::disconnect()
{
    if(m_ws)
    {
        m_ws->Stop();
    }
}

void PeerCall::OnConnected()
{
}

void PeerCall::OnStartCall(int nCount)
{
    if(TestConfig::Instance().m_bCalliope)
    {
        // for calliope, the first one start the call session
        if (nCount == 1)
            createVenue();
    }
    else if(nCount > 1)
    {
        // for p2p, the second one start the call session
        m_endCaller->startCall();
    }
}

void PeerCall::OnSDPReceived(const string &type, const string &text)
{
	CM_INFO_TRACE_THIS("PeerCall::OnSDPReceived, type=" << type);
	if (type.compare("venue") == 0){
		m_sVenueUrl = text;
		if (m_endCaller)
			m_endCaller->startCall();
	}
	else{
		if (m_endCaller)
			m_endCaller->OnSDPReceived(type, text);
	}
}

void PeerCall::onEvent(Endpoint *origin, const string &type, const string &text, const string &extra)
{
    if (TestConfig::i().m_bCalliope){
        if(!m_sVenueUrl.empty())
        {
            createConfluence(text);
        }
    }
    else{
        if (m_ws)
            m_ws->SendTo(type, text);
    }
}

void PeerCall::OnVenue(const string &venueUrl, COrpheusClient* pOrigin)
{
	m_sVenueUrl = venueUrl;
	if (m_ws)
		m_ws->SendTo("venue", venueUrl);
	if (m_endCaller)
		m_endCaller->startCall();
    if (TestConfig::i().m_pAppSink)
        TestConfig::i().m_pAppSink->onVenueUrl(venueUrl);
}

void PeerCall::OnFlowParameters(const string &parameters, const string &url, COrpheusClient* pOrigin)
{
	if (TestConfig::i().m_bSharer
        && TestConfig::i().m_bAppshare
        && TestConfig::i().m_bAutoRequestFloor)
		pOrigin->RequestFloor();
	if (m_endCaller)
		m_endCaller->OnSDPReceived("answer", parameters);
}

Calabash * PeerCall::getCalabash() {
    if (m_endCaller != NULL)
        return m_endCaller->getCalabash();
    
    return NULL;
}

void PeerCall::SetAutoSwitchDefaultMic(bool bFlag)
{
    if (m_endCaller)
        m_endCaller->SetAutoSwitchDefaultMic(bFlag);
}

void PeerCall::SetAutoSwitchDefaultSpeaker(bool bFlag)
{
    if (m_endCaller)
        m_endCaller->SetAutoSwitchDefaultSpeaker(bFlag);
}
