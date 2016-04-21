#include <string>
#include "WMEVideoHost.h"
#include "json/json.h"
#include "CmThread.h"
#include "CmThreadManager.h"
#include "MediaConnection.h"
#include "Synchapi.h"
#include "Loopback.h"
#include "PeerCall.h"
#include "ClickCall.h"
#include "MediaConnection.h"
#include <time.h>
#include "backdooragent.h"
#include "TAHTTPServer.h"
#include "CmInetAddr.h"

using namespace WMEVideoComponent;
using namespace Platform;
using namespace Windows::System::Threading;
using namespace Windows::Foundation;
using namespace wme;

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) \
	if(p) \
	delete p;\
	p = NULL;
#endif

WMEVideoHost::WMEVideoHost():
m_pTracer(NULL)
{
    m_pWp8LocalRender = ref new RenderDemoWindowMode();
    m_pWp8RemoteRender = ref new RenderDemoWindowMode();
	m_pWp8ASRender = nullptr;// ref new RenderDemoWindowMode();
	m_pBackdoorAgent = NULL;
	m_pTAServer = NULL;

}

WMEVideoHost :: ~WMEVideoHost() {
    if (NULL != m_pTracer) {
        delete m_pTracer;
        m_pTracer = NULL;
    }
	if (m_pWp8LocalRender)
	{
		delete m_pWp8LocalRender;
		m_pWp8LocalRender = nullptr;
	}
	if (m_pWp8RemoteRender)
	{
		delete m_pWp8RemoteRender;
		m_pWp8RemoteRender = nullptr;
	}
	if (m_pWp8ASRender)
	{
		delete m_pWp8ASRender;
		m_pWp8ASRender = nullptr;
	}

	SAFE_DELETE(m_pTAServer);
	SAFE_DELETE(m_pBackdoorAgent);
}

void WMEVideoHost::OnTimer(){
    MainThreadHeartbeat();
}

RenderDemoWindowMode^ WMEVideoHost::GetRender(RenderType kType)
{
    switch (kType)
    {
    case RenderType::LocalRender:
        return m_pWp8LocalRender;
    case RenderType::RemoteRender:
        return m_pWp8RemoteRender;
    case RenderType::ASRender:
        return m_pWp8ASRender;
    default:
        return nullptr;
    }
}

void WMEVideoHost::Start(unsigned int csi)
{
    if (NULL == m_pTracer) {
        m_pTracer = new WMETraceSinkEnd();
    }

    if (NULL != m_pTracer) {
        WmeSetMediaEngineOption(WmeOption_TraceSink, m_pTracer, 0);
    }
    LogSink::G2LogSink::Instance->SetFilePrefixName(L"wp8_video");
    LogSink::G2LogSink::Instance->StartLog(true);

    InitThreadManager();
    InitMediaEngine(false);

    m_statsInfo.SetSink(&m_sink);
    TestConfig::i().m_csi = csi | 0x00000001;
}

void WMEVideoHost::SetStatisticStatus(bool open)
{
    if (true == open) {
        m_statsInfo.Start();
    } else {
        m_statsInfo.Stop();
    }    
}

void WMEVideoHost::Stop()
{

}

void WMEVideoHost::Mute(bool isMute)
{
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    if (bLoopbackCall){
        LoopbackCall::Instance()->mute(WmeSessionType_Video, isMute, true);
    }
    else{
        PeerCall::Instance()->mute(WmeSessionType_Video, isMute, true);
    }
}

void WMEVideoHost::StartCall(Platform::String^ server, Platform::String^ linus, SWITCHINFO switchinfo)
{
    IWmeVideoRender* pRender = NULL;
    void *winRemote = NULL;
    void *winLocal = NULL;
    void *winScreenViewer = NULL;

    if (!m_pWp8LocalRender)
        return;
    m_pWp8LocalRender->getRenderInstance(&pRender);
    winLocal = (void*)pRender;

    if (!m_pWp8RemoteRender)
        return;
    m_pWp8RemoteRender->getRenderInstance(&pRender);
    winRemote = (void*)pRender;

/*    if (!m_pWp8ASRender)
        return;
    m_pWp8ASRender->getRenderInstance(&pRender);
    winScreenViewer = (void*)pRender;
	*/

    m_bSwitchInfo = switchinfo;


    TestConfig::i().m_bLoopback = m_bSwitchInfo.loop;
    TestConfig::i().m_bCalliope = m_bSwitchInfo.calliope;
    TestConfig::i().m_videoDebugOption["enableSRTP"] = m_bSwitchInfo.srtp;
    TestConfig::i().m_bHasAudio = false;
    TestConfig::i().m_bHasVideo = (m_bSwitchInfo.enable_video);
	if (m_bSwitchInfo.dump_video) {
		TestConfig::i().m_videoDebugOption["enableDataDump"] = WME_DATA_DUMP_VIDEO_RAW_AFTER_DECODE_TO_RENDER;
	}
    //const char *server = "10.224.166.110:8080/echo?r=123";
    TestConfig::i().m_sWSUrl = platStrToStdStr(server);

    //const char *linus = "http://10.224.166.110:5000/";
    TestConfig::i().m_sLinusUrl = platStrToStdStr(linus);

    doStartCall(winRemote, winLocal, winScreenViewer, &m_sink);
}

void WMEVideoHost::StopCall()
{
    doStop();    
}

void WMEVideoHost::SetAudioLocalSDP(Platform::String^ sdp)
{
    TestConfig::i().m_bgSdpOffer = platStrToStdStr(sdp);
}

void WMEVideoHost::SetDataChannelDelegate(DataChannelDelegate^ aDelegate)
{
    m_sink.m_delegate = aDelegate;
}

void WMEVideoHost::SetParam(Platform::String^ key, Platform::String^ value)
{
    if (PeerCall::Instance()->isActive() && PeerCall::Instance()->m_endCaller)
    {
        PeerCall::Instance()->m_endCaller->SendEvent(platStrToStdStr(key), platStrToStdStr(value));
    }
    else {
        LoopbackCall::Instance()->m_endCaller.SendEvent(platStrToStdStr(key), platStrToStdStr(value));
    }
}

void WMEVideoHost::InitTAContext()
{
	IAppSink *pApp = NULL;
	IWmeVideoRender* pRender = NULL;
	void *winRemote = NULL;
	void *winLocal = NULL;
	void *winScreenViewer = NULL;

	if (!m_pWp8LocalRender)
		return;
	m_pWp8LocalRender->getRenderInstance(&pRender);
	winLocal = (void*)pRender;

	if (!m_pWp8RemoteRender)
		return;
	m_pWp8RemoteRender->getRenderInstance(&pRender);
	winRemote = (void*)pRender;

	/*if (!m_pWp8ASRender)
		return;
	m_pWp8ASRender->getRenderInstance(&pRender);
	winScreenViewer = (void*)pRender;*/

	if (!m_pBackdoorAgent && !m_pTAServer)
	{

		if (m_pTracer)
		{
			m_pTracer->OnTrace(0, "InitTAContext start");
		}

		char szFileName[1024] = { 0 };

		m_pBackdoorAgent = new CBackdoorAgent(szFileName, winRemote, winLocal,
			winScreenViewer, &m_sink);

		m_pTAServer = new TAServer(m_pBackdoorAgent);

		// test
		TestConfig::Instance().m_nTAPort = 6621;
		TestConfig::Instance().m_bHasAudio = false;

		CCmInetAddr addr("0.0.0.0", TestConfig::Instance().m_nTAPort);
		m_pTAServer->Init(addr);
	}
}

void WMEVideoHost::ChangeOrientation(int eFlag)
{
    if (PeerCall::Instance()->isActive() && PeerCall::Instance()->m_endCaller)
    {
        PeerCall::Instance()->m_endCaller->changeOrientation((WmeCameraOrientation)eFlag);

    }
    else {
        LoopbackCall::Instance()->m_endCaller.changeOrientation((WmeCameraOrientation)eFlag);
    }
}

void WMEVideoHost::StartStopTrack(bool bStart, bool bRemote)
{
    if (PeerCall::Instance()->isActive() && PeerCall::Instance()->m_endCaller){
        PeerCall::Instance()->startStopTrack(WmeSessionType_Video, bRemote, bStart);
    }
    else{
        LoopbackCall::Instance()->startStopTrack(WmeSessionType_Video, bRemote, bStart);
    }
}

void WMEVideoHost::EnableVideoDump(bool enable)
{
    Json::FastWriter w;
    Json::Value enableDump;

    if (true == enable)
    {
        enableDump["enableDataDump"] = WME_DATA_DUMP_VIDEO_RAW_AFTER_DECODE_TO_RENDER;
    }
    else {
        enableDump["enableDataDump"] = 0;
    }
	Json::Value root(Json::objectValue);
	root["video"] = enableDump;
    
	std::string sParam = w.write(root);

    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    
    if (bLoopbackCall){
        LoopbackCall::Instance()->m_endCaller.setParam(PARAM_VIDEO, sParam.c_str());
    }
    else{
        PeerCall::Instance()->m_endCaller->setParam(PARAM_VIDEO, sParam.c_str());
    }

    
}