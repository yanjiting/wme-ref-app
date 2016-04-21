#include <string>
#include "WMEAudioHost.h"
#include "CmThread.h"
#include "CmThreadManager.h"
#include "MediaConnection.h"
#include "Loopback.h"
#include "ClickCall.h"
#include "MediaConnection.h"
#include "backdooragent.h"
#include "TAHTTPServer.h"
#include "CmInetAddr.h"
#include "PeerCall.h"
#include "ThreadEmulation.h"
#include "CallController.h"

#ifdef TEST_RINGTONE
#include "RTPFrame.h"
#include "beep_pcm.h"
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) \
	if(p) \
	delete p;\
	p = NULL;
#endif

using namespace WMEAudioComponent;
using namespace Platform;
using namespace Windows::Foundation;
using namespace wme;
using namespace Windows::Phone::Media::Devices;

WMEAudioHost::WMEAudioHost():
m_pTracer(NULL),
m_periodicTimer(nullptr),
m_bStopHeartBeatThreadFlag(true),
m_bStartedCall(false),
m_bStopCall(false)
{
	m_pBackdoorAgent = NULL;
	m_pTAServer = NULL;
    m_call = ref new CallController();
}

WMEAudioHost :: ~WMEAudioHost() {
    if (NULL != m_pTracer) {
        delete m_pTracer;
        m_pTracer = NULL;
    }

	SAFE_DELETE(m_pTAServer);
	SAFE_DELETE(m_pBackdoorAgent);
}

void WMEAudioHost::CreateHeartbeatsThread(bool isTa){
    auto workItemHandler = ref new WorkItemHandler([=](IAsyncAction^)
    {
        InitThreadManager();
        InitMediaEngine(false);
        while (TRUE) {
            if (false == m_bStopHeartBeatThreadFlag) {
                if (isTa) {
                    InitTAContext();
                }
                if (!m_bStartedCall && !isTa) {
                    _startCall();
                    m_bStartedCall = true;
                }
                if (m_bStopCall)
                {
                    doStop();
                    m_bStopCall = false;
                }
                ThreadEmulation::Sleep(100);
                MainThreadHeartbeat();
            }
            else {
                doStop();
                break;
            }
        }
        UninitMediaEngine();
    }, CallbackContext::Any);

    if (m_bStopHeartBeatThreadFlag) {
        m_bStopHeartBeatThreadFlag = false;
        m_heartbeatThread = ThreadPool::RunAsync(workItemHandler);
    }
}

void WMEAudioHost::Start(bool isTa, unsigned int csi)
{
    if (NULL == m_pTracer) {
        m_pTracer = new WMETraceSinkEnd();
    }

    if (NULL != m_pTracer) {
        WmeSetMediaEngineOption(WmeOption_TraceSink, m_pTracer, 0);
    }
    LogSink::G2LogSink::Instance->SetFilePrefixName(L"wp8_audio");
    LogSink::G2LogSink::Instance->StartLog(true);

    m_statsInfo.SetSink(&m_sink);

    if (true == isTa)
    {
        CreateHeartbeatsThread(isTa);
    }
    TestConfig::i().m_csi = csi; 
}

void WMEAudioHost::Stop()
{
    m_bStopHeartBeatThreadFlag = true;
}

void WMEAudioHost::Mute(bool isMute)
{
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();    
    if (bLoopbackCall){
        LoopbackCall::Instance()->mute(WmeSessionType_Audio, isMute, true);
    }
    else{
        PeerCall::Instance()->mute(WmeSessionType_Audio, isMute, true);
    }
}

void WMEAudioHost::SetStatisticStatus(bool open)
{
    if (true == open) {
        m_statsInfo.Start();
    }
    else {
        m_statsInfo.Stop();
    }
}

void WMEAudioHost::StartCall(Platform::String^ server, Platform::String^ linus, SWITCHINFO switchinfo)
{
    m_bSwitchInfo = switchinfo;
    m_server = server;
    m_linus = linus;
    m_bStartedCall = false;
    CreateHeartbeatsThread(false);
    m_call->StartOutgoingCall(L"Soya");
    m_call->OnOutgoingCallAnswered();
}

void WMEAudioHost::_startCall()
{
    IWmeVideoRender* pRender = NULL;
    void *winRemote = NULL;
    void *winLocal = NULL;
    void *winScreenViewer = NULL;

    m_bSwitchInfo = m_bSwitchInfo;
    TestConfig::i().m_bLoopback = m_bSwitchInfo.loop;
    TestConfig::i().m_bCalliope = m_bSwitchInfo.calliope;
    TestConfig::i().m_audioDebugOption["enableSRTP"] = m_bSwitchInfo.srtp;
    TestConfig::i().m_bHasAudio = (m_bSwitchInfo.enable_audio);
    TestConfig::i().m_bHasVideo = false;
    TestConfig::i().m_audioParam["enableAllDumpFile"] = m_bSwitchInfo.dump_audio;
    
    TestConfig::i().m_bAudioProcess = true;

    //const char *server = "10.224.166.110:8080/echo?r=123";
    std::wstring server_url = m_server->Data();
    TestConfig::i().m_sWSUrl = std::string(server_url.begin(), server_url.end());

    //const char *linus = "http://10.224.166.110:5000/";
    std::wstring linus_url = m_linus->Data();
    TestConfig::i().m_sLinusUrl = std::string(linus_url.begin(), linus_url.end());;

    doStartCall(winRemote, winLocal, winScreenViewer, &m_sink);

#ifdef TEST_RINGTONE
	PlayRingtone();
#endif
}

void WMEAudioHost::StopCall()
{
    m_bStopCall = true;
    m_call->OnCallEnded();
}

Platform::String^ WMEAudioHost::GetLocalSdp()
{
    Platform::String^ strReturn = "";
    std::string sdp;
    if (PeerCall::Instance()->isActive() && PeerCall::Instance()->m_endCaller)
    {
        sdp = PeerCall::Instance()->m_endCaller->m_localSdp;

    } else {
        sdp = LoopbackCall::Instance()->m_endCaller.m_localSdp;
    }
    return stdStrToPlatStr(sdp);
}

void WMEAudioHost::SetRemoteSdp(Platform::String^ sdp)
{
    const size_t cSize = sdp->Length() + 1;
    char* asdp = new char[cSize];
    wcstombs(asdp, sdp->Data(), cSize);
    if (PeerCall::Instance()->m_endCaller){
        PeerCall::Instance()->m_endCaller->answerReceived(asdp);
    }
}

void WMEAudioHost::SetBackgroundEvent(IBackgroundEventListener^ bgEventListener)
{
    m_sink.m_listener = bgEventListener;
}

void WMEAudioHost::InitTAContext()
{

	if (!m_pBackdoorAgent && !m_pTAServer)
	{

		if (m_pTracer)
		{
			m_pTracer->OnTrace(0, "InitTAContext start");
		}

		char szFileName[1024] = { 0 };

		m_pBackdoorAgent = new CBackdoorAgent(szFileName, NULL, NULL,
			NULL);

		m_pTAServer = new TAServer(m_pBackdoorAgent);

		// test
		TestConfig::Instance().m_nTAPort = 6620;

		TestConfig::Instance().m_bHasVideo = false;

		CCmInetAddr addr("0.0.0.0", TestConfig::Instance().m_nTAPort);
		m_pTAServer->Init(addr);
	}
}

#ifdef TEST_RINGTONE
RTPFrame* WMEAudioHost::BuildRTP(int frameSize, void* framedata,
    unsigned char playloadtype, WORD sequence, DWORD timestamp, DWORD SSID)
{
    RTPFrame* rtpdata = new RTPFrame(frameSize + 12);
    if (rtpdata == NULL)
        return NULL;

    rtpdata->SetSyncSource(SSID);
    rtpdata->SetPayloadType(playloadtype);
    rtpdata->SetSequenceNumber(sequence);
    rtpdata->SetTimestamp(timestamp);

    memset( rtpdata->GetBuffer(), 0x80, 1);

    //memcpy ( rtpdata->GetBuffer() + 12, framedata, frameSize );
    memcpy_s(rtpdata->GetBuffer() + 12, frameSize, framedata, frameSize);

    return rtpdata;
}

void WMEAudioHost::OnRingtoneTimer()
{
    LogSink::G2LogSink::Instance->Log(2, "andy: End WMEAudioHost::OnTimer()");
    static int i = 0;

    BYTE codec = 0;
    int frequency = 20;
    int sampleRate = 8;
    int frameSize = 160;
    static WORD sequence = 0;
    static DWORD timestamp = 0;
    static int frameNum = sizeof(spark_ring) / frameSize;

    DWORD timediff = frequency*sampleRate;
    static DWORD SSID = 0x1234;//+ AT_Rand()%100;

    if (i < frameNum)
    {
        sequence++;
        timestamp += timediff;
        void* databuffer = NULL;
        databuffer = (void*)(spark_ring + i* frameSize);

        RTPFrame* rtpdata = BuildRTP(frameSize, databuffer, codec, sequence, timestamp, SSID);
        if (rtpdata != NULL)
        {
            ((IWmeMediaConnection4T*)(PeerCall::Instance()->m_endCaller->m_pMediaConn))->ReceiveAdhocRTP(1234, (uint8_t*)rtpdata->GetBuffer(), frameSize + 12);
            SAFE_DELETE(rtpdata);
        }
        i++;
    }
    else {
        m_periodicTimer->Cancel();
    }
}

void WMEAudioHost::PlayRingtone()
{
    AudioRoutingManager^ m_audioRouteManager = AudioRoutingManager::GetDefault();
    m_audioRouteManager->SetAudioEndpoint(AudioRoutingEndpoint::Speakerphone);
    int frameSize = 160;
    TimeSpan period;
    period.Duration = 20* 10000; // 10,000,000 ticks per second
    ((IWmeMediaConnection4T*)(PeerCall::Instance()->m_endCaller->m_pMediaConn))->CreateAdhocAudioTrack(1234);

    m_periodicTimer = ThreadPoolTimer::CreatePeriodicTimer(
        ref new TimerElapsedHandler([this](ThreadPoolTimer^ source)
    {
        OnRingtoneTimer();
    }),
    period);

    return ;
}

int WMEAudioHost::PlayFunc()
{

    return 0;
}
#endif