#include "ClickCall.h"
#include "bearer_oauth.h"
#include "calliope_client.h"
#include <string>
#include "Loopback.h"
#include "PeerCall.h"
#include "atdef.h"
#include "ScreenTrackViewAnalyzer.h"
#include "mediaconfig.h"
#include <regex>
#include "BackdoorAgent.h"
#include <math.h>
#ifdef AT_IOS
#import <mach/mach.h>
#import <sys/sysctl.h>
#endif

#ifdef WP8
using namespace Windows::Phone::Media::Devices;
#endif

using namespace wme;
#define AUDIO_MID 1
#define VIDEO_MID 2
#define SCREEN_MID 3

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) do { if(x) x->Release(); x = NULL; } while(0);
#endif

#ifdef WIN32
#include <Rpc.h>
#elif defined(AT_ANDROID)
#else
#include <uuid/uuid.h>
#endif
ExternalAudioRender::ExternalAudioRender(WmeMediaExternalRenderTypeConfig nType):m_pFileSaveAudioData(NULL), m_renderType(WmeExternalRender_Type_Normal)
{
	m_renderType = nType;
	if (NULL == m_pFileSaveAudioData)
	{
		char strFileName[200];
		memset(strFileName, 0, 200);
#ifdef Win32 
		DWORD dtime = GetTickCount();
		switch (m_renderType)
		{
		case WmeExternalRender_Type_CaptureFromHardware://capture hardware data
			sprintf(strFileName, "CaptureHardware_%p_%d.pcm", this,dtime);
			break;
		case WmeExternalRender_Type_CaptureBeforeEncode://capture before encode
			sprintf(strFileName, "CaptureBeforeEncode_%p_%d.pcm", this, dtime);
			break;
		case WmeExternalRender_Type_PlaybackToHardware://playback hardware data
			sprintf(strFileName, "PlaybackHardware_%p_%d.pcm", this, dtime);
			break;
		default:
			return ;
		}
		m_pFileSaveAudioData = fopen(strFileName, "wb");
#endif
	}
}
ExternalAudioRender::~ExternalAudioRender()
{
	if (NULL != m_pFileSaveAudioData)
	{
		fclose(m_pFileSaveAudioData);
		m_pFileSaveAudioData = NULL;
	}
}
WMERESULT ExternalAudioRender::RenderMediaData(uint32_t uTimestamp, WmeMediaFormatType eType, void* pFormat, unsigned char *pData, uint32_t uLength)
{
	static long nSumCount = 0;
	nSumCount += uLength;
	if (nSumCount > 1024 * 1024 * 3)
	{
		if (NULL != m_pFileSaveAudioData)
		{
			fclose(m_pFileSaveAudioData);
			m_pFileSaveAudioData = NULL;
		}
	}
    //WmeAudioRawFormat* pAuidoFormat =(WmeAudioRawFormat*)pFormat;
	if (NULL != m_pFileSaveAudioData)
	{
		fwrite(pData, sizeof(char), uLength, m_pFileSaveAudioData);
	}
	return 0;
}

TestConfig TestConfig::testConfig;

Endpoint::Endpoint(EventSink *sink, bool bHost) :
		m_pSink(sink),  m_audioFileEnded(false), m_videoFileEnded(false),
		m_bHost(bHost), m_audioMid(AUDIO_MID), m_videoMid(VIDEO_MID), m_shareMid(SCREEN_MID), m_uplinkStatus(WmeNetwork_recovered),
        m_audioRemoteDirection(WmeDirection_SendRecv), m_videoRemoteDirection(WmeDirection_SendRecv), // Soya: set to sendrecv first to avoid unmute first time
        m_externalAudio(this, (int)m_audioMid),
        m_externalVideo(this, (int)m_videoMid),
        m_pSvsAudioInputPin(NULL)
{
    m_winCaller = NULL;
    m_winScreenViewer = NULL;

	m_pVideoTrackCaller = NULL;

	m_pAudioTrackCaller = NULL;
	m_pAudioTrackCallee = NULL;

    m_pScreenTrackSharer = NULL;
    m_pScreenTrackViewer = NULL;

	m_pMediaConn = NULL;
    m_pScreenTrackViewAnalyzer = NULL;
    m_pScreenFileCapturer = NULL;
    
    m_pSvsInputPin = NULL;
    m_pSvsInputPin2 = NULL;
    m_pSvsAudioInputPin2 = NULL;
    
    m_pSvsLocalVideo2 = NULL;
    m_pSvsLocalAudio2 = NULL;
    
    m_pAudioRtpDumper = NULL;
    m_pVideoRtpDumper = NULL;

	m_cameraList = getCameraList();
    
//test
    m_vectorCaptureDevice.clear();
    m_vectorPlayDevice.clear();
    
    // Calabash testing
    mCalabash = new Calabash();
    m_winScreenPreview = NULL;
    
    WmeCpuDescpription cpuDesc;
    GetCpuDescription(cpuDesc);

    m_trackIndex = 0;
	//
	m_externalAuidoRender[0] = NULL;
	m_externalAuidoRender[1] = NULL;
	m_externalAuidoRender[2] = NULL;
	m_externalAuidoRender[3] = NULL;
    
    m_possibleActiveCsi = (unsigned int)-1;
    
    m_pUISink = NULL;
}

Endpoint::~Endpoint() {
	stopCall();
    delete mCalabash;
	for (int i = 0; i < 4; i++)
	{
		if (NULL != m_externalAuidoRender[i])
		{
			delete m_externalAuidoRender[i];
			m_externalAuidoRender[i] = NULL;
		}
	}
}

void Endpoint::preview() {
	CM_INFO_TRACE_THIS("Endpoint::preview");   
	if (m_pMediaConn && m_pMediaConn->GetAudioConfig(m_audioMid))
	{
		bool value = TestConfig::i().m_bAutoSwitchAudioMicDevice;
		m_pMediaConn->GetAudioConfig(m_audioMid)->SetAutoSwitchDefaultMicrophoneDeviceFlag(value);
		value = TestConfig::i().m_bAutoSwitchAudioSpeakerDevice;
		m_pMediaConn->GetAudioConfig(m_audioMid)->SetAutoSwitchDefaultSpeakerDeviceFlag(value);
	}
    IWmeMediaConnection4T *pConn = NULL;
    if(TestConfig::i().m_bTrain) {
        CreateMediaConnection4T(1234567, &pConn);
        m_pMediaConn = pConn;
    }else if(TestConfig::i().m_bHasSVS) {
        CreateMediaConnection4T(1234568, &pConn, true, false);
        m_pMediaConn = pConn;
    }
    else {
#ifdef WP8
        CreateMediaConnectionWithCsi(&m_pMediaConn, TestConfig::i().m_csi);
#else
        CreateMediaConnection(&m_pMediaConn);
#endif
    }
	
    if(m_pMediaConn == NULL)
        return;
	m_pMediaConn->SetSink(this);
//#ifndef WP8
	m_pMediaConn->StartMediaLogging(1000, WmeMediaLogging_Default | WmeMediaLogging_Sync);
//#endif
	
	if (!TestConfig::i().m_bLoopback || m_bHost) {
		if (TestConfig::i().m_bHasAudio){
			m_pMediaConn->AddMedia(WmeSessionType_Audio, WmeDirection_Send, m_audioMid,
				GetDebugOption(WmeSessionType_Audio).c_str());
            if(TestConfig::i().m_bHasSVS && pConn) {
                //uint8_t vid = 0;
                //pConn->AddLocalTrack(m_audioMid, 5555, vid);
            }
		}
		if (TestConfig::i().m_bHasVideo){
			m_pMediaConn->AddMedia(WmeSessionType_Video, WmeDirection_Send, m_videoMid,
				GetDebugOption(WmeSessionType_Video).c_str());
            if(TestConfig::i().m_bHasSVS && pConn) {
                m_svsDelayAddTimer.Schedule(this, CCmTimeValue(5, 0), 1);
            }

            m_pMediaConn->GetVideoConfig(m_videoMid)->EnableAVCSimulcast(TestConfig::i().m_bEnableAVCSimulcast);
            m_pMediaConn->GetVideoConfig(m_videoMid)->Disable90PVideo(TestConfig::i().m_bDisable90PVideo);
            if (TestConfig::Instance().m_bVideoHW) {
                m_pMediaConn->GetVideoConfig(m_videoMid)->EnableHWAcceleration(true, WmeHWAcceleration_Encoder);
#ifdef ANDROID
                m_pMediaConn->GetVideoConfig(m_videoMid)->EnableHWAcceleration(true, WmeHWAcceleration_Decoder);
#else
                m_pMediaConn->GetVideoConfig(m_videoMid)->EnableHWAcceleration(true, WmeHWAcceleration_Decoder);
#endif
            }
            
            if(TestConfig::Instance().m_bVideoScreenHW) {
                m_pMediaConn->GetShareConfig(m_shareMid)->EnableHWAcceleration(true, WmeHWAcceleration_Encoder);
                m_pMediaConn->GetShareConfig(m_shareMid)->EnableHWAcceleration(true, WmeHWAcceleration_Decoder);
            }
        
        }
	}

	
	if (TestConfig::i().m_bAppshare)
	{
		if ((TestConfig::i().m_bLoopback && m_bHost)
			|| (!TestConfig::i().m_bLoopback && TestConfig::i().m_bSharer))
		{
			m_pMediaConn->AddMedia(WmeSessionType_ScreenShare, WmeDirection_Send, m_shareMid, GetDebugOption(WmeSessionType_ScreenShare).c_str());
            updateScreenCaptureMacFps();
			IShareConfig* pIShareConfig = m_pMediaConn->GetShareConfig(m_shareMid);
			if (pIShareConfig)
				pIShareConfig->SetScreenFilterSelf(TestConfig::i().m_bShareFilterSelf);
		}
	}

    setBandwidthMaually(TestConfig::i().m_mauallyAdaptationBps);
}

void Endpoint::startCall() 
{

    m_uplinkStatus = WmeNetwork_recovered;
    if (TestConfig::i().m_bHasAudio){
        m_pMediaConn->AddMedia(WmeSessionType_Audio, WmeDirection_Recv, m_audioMid,
            GetDebugOption(WmeSessionType_Audio).c_str());
        
        bool value = TestConfig::i().m_bAutoSwitchAudioMicDevice;
        m_pMediaConn->GetAudioConfig(m_audioMid)->SetAutoSwitchDefaultMicrophoneDeviceFlag(value);
        
        value = TestConfig::i().m_bAutoSwitchAudioSpeakerDevice;
        m_pMediaConn->GetAudioConfig(m_audioMid)->SetAutoSwitchDefaultSpeakerDeviceFlag(value);
 
        /*
         typedef enum{
         WmeAecTypeNone = 0,         		///< no AEC
         WmeAecTypeBuildin = 1,				///< Build AEC, only VPIO mode is used on IOS.
         WmeAecTypeWmeDefault = 2,          	///<  Windows/Mac/Linux uses Modified Movi AEC, Android/IOS use WebRTC AECM
         WmeAecTypeTc = 3,           		///< TC AEC, Only IOS/Android supported.
         WmeAecTypeAlpha = 4             	///< A-AEC, only Mac/Windows supported.
         }WmeAecType;
         */
        
//        printf("AEC Type:%s\n", TestConfig::Instance().m_aectype.c_str());
        WmeAecType  type =  WmeAecTypeWmeDefault;
        if (0 == strcmp(TestConfig::Instance().m_aectype.c_str(), "WmeAecTypeNone")) {
            type =  WmeAecTypeNone;
        }
        else if(0 == strcmp(TestConfig::Instance().m_aectype.c_str(), "WmeAecTypeBuildin"))
        {
            type =  WmeAecTypeBuildin;
            
        }
        else if(0 == strcmp(TestConfig::Instance().m_aectype.c_str(), "WmeAecTypeWmeDefault"))
        {
            type =  WmeAecTypeWmeDefault;
            
        }
        else if(0 == strcmp(TestConfig::Instance().m_aectype.c_str(), "WmeAecTypeTc"))
        {
            type =  WmeAecTypeTc;
            
        }
        else if(0 == strcmp(TestConfig::Instance().m_aectype.c_str(), "WmeAecTypeAlpha"))
        {
            type =  WmeAecTypeAlpha;
            
        }
        m_pMediaConn->GetAudioConfig(m_audioMid)->SetECType(type);
        
    }
    if (!TestConfig::i().m_bLoopback || !m_bHost)
	{
		if (TestConfig::i().m_bHasVideo){
			m_pMediaConn->AddMedia(WmeSessionType_Video, WmeDirection_Recv, m_videoMid,
				GetDebugOption(WmeSessionType_Video).c_str());
		}
	}
	
	if (TestConfig::i().m_bAppshare)
	{
		if ((TestConfig::i().m_bLoopback && !m_bHost)
			|| (!TestConfig::i().m_bLoopback && !TestConfig::i().m_bSharer))
		{
			m_pMediaConn->AddMedia(WmeSessionType_ScreenShare, WmeDirection_Recv, m_shareMid, GetDebugOption(WmeSessionType_ScreenShare).c_str());
		}
	}
    if(TestConfig::i().m_bEnableDtlsSrtp) {
        m_pMediaConn->GetGlobalConfig()->EnableDtlsSRTP(true, false);
    }
    m_pMediaConn->GetGlobalConfig()->EnableMultiStream(TestConfig::i().m_bMultiStreamEnable);
    if(!TestConfig::i().m_sOverrideDeviceSetting.empty())
        m_pMediaConn->GetGlobalConfig()->SetDeviceMediaSettings(TestConfig::i().m_sOverrideDeviceSetting.c_str());
    
    if (!TestConfig::i().m_globalParam.isNull()) {
        setGlobalParam(GetGlobalParam().c_str());
    }
    m_pMediaConn->GetGlobalConfig()->EnableQos(TestConfig::i().m_bQoSEnable);
    if (TestConfig::i().m_bSetFeatureToggles) {
        m_pMediaConn->GetGlobalConfig()->SetFeatureToggles(TestConfig::i().m_strFeatureToggles.c_str());
    }
    m_pMediaConn->GetGlobalConfig()->EnableDPC(TestConfig::i().m_bDPC);
    setVideoParam(GetParam(WmeSessionType_Video).c_str());
    setAudioParam(GetParam(WmeSessionType_Audio).c_str());
    setScreenParam(GetParam(WmeSessionType_ScreenShare).c_str());
	m_pMediaConn->CreateOffer();
	//m_pMediaConn->StartMediaLogging(1000);
    
	//Expecting onSDPReady callback to get the SDP to send to LOCUS
    StartExternalTrans();
}

void Endpoint::answerReceived(const char *sdp) {
    if(sdp == NULL)
        return;
    int num = 4;
    WmeSdpParsedInfo arraySdp[4];
    std::string strFaked = sdp;
    if(TestConfig::i().m_bFakeSVCByAVC)
    {
        strFaked = regex_replace(sdp, regex("H264/"), "H264-SVC/");
        sdp = strFaked.c_str();
    }
    
	WMERESULT ret = m_pMediaConn->SetReceivedSDP(WmeSdp_Anwser, sdp, arraySdp, &num);
    if(WME_SUCCEEDED(ret))
    {
        checkDirection(arraySdp, num);
    }

    int i = 0;
    for(i = 0; TestConfig::i().m_bCalliope && TestConfig::i().m_uMaxVideoStreams > 1 && i < TestConfig::i().m_uMaxVideoStreams; i++) {
        int vid = -1;
        m_pMediaConn->Subscribe(m_videoMid, WmePolicy_ActiveSpeaker, WmeVideoQuality_SLD, vid, 254,
                                1, TestConfig::i().m_bDuplication, -1);
    }
    
    if(TestConfig::i().m_bTestRequestVideoOverrideDefault && m_pVideoTrackCallee.size() > 0 && m_pVideoTrackCallee[0] != NULL) {
        m_pVideoTrackCallee[0]->RequestVideo(WmeVideoQuality_SD);
    }
    
    ConnectExternalTrans();
}

void Endpoint::acceptCall(const char *sdp, bool bUpdate) {

	if (!m_pMediaConn) return;

    int num = 4;
    WmeSdpParsedInfo arraySdp[4];
    WMERESULT ret = m_pMediaConn->SetReceivedSDP(WmeSdp_Offer, sdp, arraySdp, &num);
    if(WME_SUCCEEDED(ret))
    {
        if(!bUpdate)
        {
            for (int i = 0; i < num; i++) {
                OnParsedMLine(arraySdp[i].mediaType, arraySdp[i].mid);
            }
        }
        checkDirection(arraySdp, num);
    }
    if(!bUpdate)
    {
        m_pMediaConn->GetGlobalConfig()->EnableMultiStream(TestConfig::i().m_bMultiStreamEnable);
        if(!TestConfig::i().m_sOverrideDeviceSetting.empty())
            m_pMediaConn->GetGlobalConfig()->SetDeviceMediaSettings(TestConfig::i().m_sOverrideDeviceSetting.c_str());
        
        if (!TestConfig::i().m_globalParam.isNull()) {
            setGlobalParam(GetGlobalParam().c_str());
        }
        m_pMediaConn->GetGlobalConfig()->EnableQos(TestConfig::i().m_bQoSEnable);
        if (TestConfig::i().m_bSetFeatureToggles) {
            m_pMediaConn->GetGlobalConfig()->SetFeatureToggles(TestConfig::i().m_strFeatureToggles.c_str());
        }
        setVideoParam(GetParam(WmeSessionType_Video).c_str());
        setAudioParam(GetParam(WmeSessionType_Audio).c_str());
        setScreenParam(GetParam(WmeSessionType_ScreenShare).c_str());
    }

	m_pMediaConn->CreateAnswer();
	//Expecting onMediaReady callback to attach caller's video
    
    StartExternalTrans();
    ConnectExternalTrans();
}

void Endpoint::setParam(PARAM_TYPE_NUM paramType, const char *param)
{
    if (NULL == param) {
        CM_ERROR_TRACE("Endpoint::setParam received invalid param");
        return;
    }

    switch (paramType) {
        case PARAM_AUDIO:
            setAudioParam(param);
            break;
        case PARAM_VIDEO:
            setVideoParam(param);
            break;
        case PARAM_SCREEN:
            setScreenParam(param);
            break;
        case PARAM_QOS:
            setQosParam(param);
            break;
        default:
            break;
    }
}

void Endpoint::setGlobalParam(const char *param) {
    if(param == NULL) return;
    string sTxt(param);
    if(sTxt.length() == 0) return;
    if (m_pMediaConn) m_pMediaConn->SetParams(-1, sTxt.c_str(), (unsigned int)sTxt.size());
}

void Endpoint::setAudioParam(const char *param) {
    if(param == NULL) return;

    string sTxt(param);
    if(sTxt.length() == 0) return;
    
    if (m_pMediaConn && TestConfig::i().m_bHasAudio) {
        m_pMediaConn->SetParams(m_audioMid, sTxt.c_str(), (unsigned int)sTxt.size());
        IAudioConfig *pAudioConfig = m_pMediaConn->GetAudioConfig(m_audioMid);
        if (pAudioConfig) {
            pAudioConfig->EnableClientMix(TestConfig::i().m_uMaxAudioStreams);
            pAudioConfig->SetDataTerminatedAtRTP(TestConfig::i().m_bDataTerminatedAtRTP);
        }
    }
}

void Endpoint::setVideoParam(const char *param) {
    if(param == NULL) return;

    string sTxt(param);
    if(sTxt.length() == 0) return;

    if (m_pMediaConn && TestConfig::i().m_bHasVideo) {
        m_pMediaConn->SetParams(m_videoMid, sTxt.c_str(), (unsigned int)sTxt.size());
        IVideoConfig *pVideoConfig = m_pMediaConn->GetVideoConfig(m_videoMid);
        if (pVideoConfig) {
            //pVideoConfig->SetInitSubscribeCount(TestConfig::i().m_uMaxVideoStreams);
            if (TestConfig::i().m_videoParam.isMember("fecParams") &&
                TestConfig::i().m_videoParam["fecParams"].isMember("bEnableFec")) {
                pVideoConfig->SetFecProbing(TestConfig::i().m_videoParam["fecParams"]["bEnableFec"].asBool());
            }
            //Added by Wilson, if HW is enabled, we need to disable simulcast. Simulcast is by default.
            pVideoConfig->EnableAVCSimulcast(TestConfig::i().m_bEnableAVCSimulcast);
            
            if (TestConfig::Instance().m_bHasSVS || TestConfig::i().m_bForceSVC) {
                pVideoConfig->SetSelectedCodec(WmeCodecType_SVC);
            }

            // if CVO is enabled, set it here
            pVideoConfig->EnableCVO(TestConfig::i().m_bEnableCVO);
            pVideoConfig->SetDataTerminatedAtRTP(TestConfig::i().m_bDataTerminatedAtRTP);

            pVideoConfig->SetInitSubscribeCount(TestConfig::i().m_uMaxVideoStreams);
        }
    }
    if (m_pVideoTrackCallee.size() > 0 && m_pVideoTrackCallee[0] != NULL) {
        
    }
}

void Endpoint::setScreenParam(const char *param){
    if(param == NULL) return;

    string sTxt(param);
    if(sTxt.length() == 0) return;

	if (m_pMediaConn && TestConfig::i().m_bAppshare) m_pMediaConn->SetParams(m_shareMid, sTxt.c_str(), (unsigned int)sTxt.size());
}

void Endpoint::setQosParam(const char *param)
{
    if(param == NULL) return;

    string sTxt(param);
    if(sTxt.length() == 0) return;

    if(m_pMediaConn) {
        // Now, only audio and video support mari qos, 2014.06.27
		if(TestConfig::Instance().m_bHasAudio)
			m_pMediaConn->SetParams(m_audioMid, sTxt.c_str(), (unsigned int)sTxt.size());
		if (TestConfig::Instance().m_bHasVideo)
			m_pMediaConn->SetParams(m_videoMid, sTxt.c_str(), (unsigned int)sTxt.size());
    }
}

void Endpoint::setBandwidthMaually(unsigned int uBandwidthBps)
{
    if(m_pMediaConn) {
        Json::Value root(Json::objectValue);
        root["manualAdaptBitrate"] = uBandwidthBps;
        Json::FastWriter w;
        string sTxt = w.write(root);
        m_pMediaConn->SetParams(-1, sTxt.c_str(), (unsigned int)sTxt.size());
    }
}

std::string Endpoint::getParameters(PARAM_TYPE_NUM paramType)
{
    std::string ret;
    switch (paramType) {
        case PARAM_AUDIO:
            if (m_pMediaConn && TestConfig::i().m_bHasAudio) {
                ret = m_pMediaConn->GetParams(m_audioMid);
            }
            break;
        case PARAM_VIDEO:
            if (m_pMediaConn && TestConfig::i().m_bHasVideo) {
                ret = m_pMediaConn->GetParams(m_videoMid);
            }
            break;
        case PARAM_SCREEN:
            if (m_pMediaConn && TestConfig::i().m_bAppshare) {
                ret = m_pMediaConn->GetParams(m_shareMid);
            }
            break;
        case PARAM_QOS:
            if (m_pMediaConn) {
                ret = m_pMediaConn->GetParams(m_audioMid); //FIX ME, It should be no param.
            }
            break;
        default:
            break;
    }
    return ret;

}

std::string Endpoint::stopCall() {
	std::string ret;
    CM_INFO_TRACE_THIS("Endpoint::stopCall");
    m_audioFileEnded = false;
    m_videoFileEnded = false;
    DisconnectExternalTrans();
	if (m_pMediaConn) {
        //reset sharing id for callEnd metric
        if (m_pScreenTrackSharer) {
            m_pScreenTrackSharer->SetScreenSharingID("");
        }
        if (m_pScreenTrackViewer) {
            m_pScreenTrackViewer->SetScreenSharingID("");
        }
        
		ret = m_pMediaConn->GetMediaSessionMetrics();
        CM_INFO_TRACE_BIG_EX("RefApp","Endpoint Session Metrics="<<ret);
        m_pMediaConn->Stop();
        if (m_pVideoTrackCaller) {
            m_pVideoTrackCaller->RemoveRenderWindow(m_winCaller);
            m_pVideoTrackCaller->Release();
            m_pVideoTrackCaller = NULL;
        }
        for (int i = 0; i < m_pVideoTrackCallee.size(); i++) {
            if (m_pVideoTrackCallee[i]) {
                if(m_winCallee.size() > i) {
                    m_pVideoTrackCallee[i]->RemoveRenderWindow(m_winCallee[i]);
                }
                if(m_pVideoRtpDumper) {
                    m_pVideoTrackCallee[i]->RemoveExternalRender(m_pVideoRtpDumper);
                }
                m_pVideoTrackCallee[i]->Release();
                m_pVideoTrackCallee[i] = NULL;
            }
        }
        m_pVideoTrackCallee.clear();
        if(m_pVideoRtpDumper) {
            delete m_pVideoRtpDumper;
            m_pVideoRtpDumper = NULL;
        }
		
		if (m_pAudioTrackCaller) {
			m_pAudioTrackCaller->Release();
			m_pAudioTrackCaller = NULL;
		}
		if (m_pAudioTrackCallee) {
			m_pAudioTrackCallee->Release();
            if(m_pAudioRtpDumper) {
                m_pAudioTrackCallee->RemoveExternalRender(m_pAudioRtpDumper);
            }
			m_pAudioTrackCallee = NULL;
		}
        if(m_pAudioRtpDumper) {
            delete m_pAudioRtpDumper;
            m_pAudioRtpDumper = NULL;
        }
		if (m_pScreenTrackSharer) {
            if (m_winScreenPreview)
                m_pScreenTrackSharer->RemoveRenderWindow(m_winScreenPreview);
			m_pScreenTrackSharer->Release();
			m_pScreenTrackSharer = NULL;
		}
		if (m_pScreenTrackViewer) {
            if(m_winScreenViewer)
                m_pScreenTrackViewer->RemoveRenderWindow(m_winScreenViewer);
			m_pScreenTrackViewer->Release();
			m_pScreenTrackViewer = NULL;
		}
        
        
		m_pMediaConn->Release();
		m_pMediaConn = NULL;
	}
    m_winCaller = NULL;
    m_winCallee.clear();
    m_winScreenViewer = NULL;
    m_winScreenPreview = NULL;
	std::vector <IWmeScreenSource*>::iterator it;
	for (it = m_vctToShareScreenSource.begin(); it != m_vctToShareScreenSource.end(); it++){
		(*it)->Release();
	}
	m_vctToShareScreenSource.clear();
    
    if (m_pScreenTrackViewAnalyzer)
    {
		m_pScreenTrackViewAnalyzer->Release();
        m_pScreenTrackViewAnalyzer = NULL;
    }
    if(m_pScreenFileCapturer){
        m_pScreenFileCapturer->Release();
        m_pScreenFileCapturer = NULL;
    }

    if (TestConfig::Instance().m_bHasSVS) {
        m_svsTimer.Cancel();
        m_svsLocalStats.Cancel();
        m_svsDelayAddTimer.Cancel();
    }
    m_trackIndex = 0;    
	return ret;
}

void Endpoint::resizeRender() {
	CM_INFO_TRACE_THIS("Endpoint::resizeRender");
	if (m_pVideoTrackCaller) {
		m_pVideoTrackCaller->UpdateRenderWindow( m_winCaller);
	}

	for (int i = 0; i < m_pVideoTrackCallee.size(); i++) {
		if (m_pVideoTrackCallee[i] && i < m_winCallee.size()) {
			m_pVideoTrackCallee[i]->UpdateRenderWindow(m_winCallee[i]);
		}
	}

	if (m_pScreenTrackViewer && m_winScreenViewer) {
		m_pScreenTrackViewer->UpdateRenderWindow(m_winScreenViewer);
	}

	if (m_pScreenTrackSharer && m_winScreenPreview)
	{
		m_pScreenTrackSharer->UpdateRenderWindow(m_winScreenPreview);
	}

}

void Endpoint::OnRenderSizeChanged(unsigned long mid, unsigned long vid, WmeDirection direction, WmeVideoSizeInfo *pSizeInfo)
{
    if (!m_pUISink || !pSizeInfo)
        return;

#if 0
    // currently there's no need on UpdateUI because Render has done the similar job.
    CM_INFO_TRACE_THIS("Endpoint::OnRenderSizeChanged");
    if (direction == WmeDirection_Recv) {
        int i = 0;
        IWmeMediaTrackBase *pTrack = NULL;
        for(i = 0; i < m_pVideoTrackCallee.size(); i++) {
            unsigned int trackvid = 255;
            m_pVideoTrackCallee[i]->GetVid(trackvid);
            if(vid == trackvid) {
                pTrack = m_pVideoTrackCallee[i];
                break;
            }
        }

        if (pTrack) {
            m_pUISink->UpdateUI(pSizeInfo->width, pSizeInfo->height, m_winCallee[i]);
            pTrack->UpdateRenderWindow(m_winCallee[i]);
        }
    }
#endif
}

void Endpoint::getStatistics(WmeAudioConnectionStatistics &audio,
		WmeVideoConnectionStatistics &video) {
	if (m_pMediaConn) {
		m_pMediaConn->GetAudioStatistics(m_audioMid, audio);
		m_pMediaConn->GetVideoStatistics(m_videoMid, video);
		//m_pMediaConn->GetVideoStatistics(m_shareMid, screen);
	}
}

void Endpoint::getFeatureToggleStatistics(string &stats)
{
    const char* strPolicy = NULL;
    Json::Value policy(Json::objectValue);
    if (m_pMediaConn) {
        strPolicy = m_pMediaConn->GetGlobalConfig()->GetFeatureToggles();
    }
    if (NULL != strPolicy) {
        Json::Reader r;
        r.parse(strPolicy, policy);
    }
    Json::Value root(Json::objectValue);
    root["featureToggles"] = policy;
    Json::FastWriter w;
    stats = w.write(root);
}

void Endpoint::getCpuStatistics(WmeCpuUsage& cpuStats){
    if(m_pMediaConn){
        m_pMediaConn->GetCPUStatistics(cpuStats);
    }
}
bool Endpoint::getTrackStatistics(unsigned index, WmeVideoStatistics &video)
{
    if(m_pVideoTrackCallee.size() > index && m_pVideoTrackCallee[index])
    {
        m_pVideoTrackCallee[index]->GetVideoStatistics(video);
        return true;
    }
    else
    {
        return false;
    }
}

void Endpoint::getNetworkIndex(WmeNetworkDirection d, WmeNetworkIndex &index)
{
    if (m_pMediaConn) {
        index = m_pMediaConn->GetNetworkIndex(d);
    }
}

void Endpoint::getScreenStatistics(WmeScreenConnectionStatistics &screen){
	if (m_pMediaConn) {
		m_pMediaConn->GetScreenStatistics(m_shareMid, screen);
	}
}

unsigned int Endpoint::getVoiceLevel(bool bSpk)
{
    unsigned int voiceLevel = 0;
    if (bSpk && m_pAudioTrackCallee != NULL) {
        m_pAudioTrackCallee->GetVoiceLevel(voiceLevel);
    }
    else if (m_pAudioTrackCaller != NULL) {
        m_pAudioTrackCaller->GetVoiceLevel(voiceLevel);
    }

    return voiceLevel;
}

void Endpoint::mute(bool bmute, bool video) {
    if(video)
        mute(bmute,wme::WmeSessionType_Video);
    else
        mute(bmute,wme::WmeSessionType_Audio);
}

void Endpoint::mute(bool bMute, WmeSessionType eWmeSessionType, bool bLocal)
{
    switch (eWmeSessionType) {
        case wme::WmeSessionType_ScreenShare:
            if(m_pScreenTrackSharer){
                if (bMute)
                    m_pScreenTrackSharer->Mute();
                else
                    m_pScreenTrackSharer->Unmute();
            }
            break;
        case wme::WmeSessionType_Audio:
			if (bLocal && m_pAudioTrackCaller) {
                if (bMute)
                    m_pAudioTrackCaller->Mute();
                else
                    m_pAudioTrackCaller->Unmute();
            }
			if (!bLocal && m_pAudioTrackCallee) {
				if (bMute)
					m_pAudioTrackCallee->Mute();
				else
					m_pAudioTrackCallee->Unmute();
			}

            break;
        case wme::WmeSessionType_Video:
			if (bLocal && m_pVideoTrackCaller) {
                if (bMute)
                    m_pVideoTrackCaller->Mute();
                else
                    m_pVideoTrackCaller->Unmute();
            }
			if (!bLocal && m_pVideoTrackCallee[0]) {
				if (bMute)
					m_pVideoTrackCallee[0]->Mute();
				else
					m_pVideoTrackCallee[0]->Unmute();
			}
			break;
        default:
            break;
    }
}

void Endpoint::changeOrientation(WmeCameraOrientation eOri) {
    if(m_pVideoTrackCaller) {
        m_pVideoTrackCaller->ChangeOrientation(eOri);
    }
}

void Endpoint::resume(bool resume) {
	if (resume) {
		if (m_pVideoTrackCaller) {
#ifdef WP8
			m_pVideoTrackCaller->AddExternalRender((IWmeVideoRender*)m_winCaller);
#else
			m_pVideoTrackCaller->AddRenderWindow(m_winCaller);
#endif
            m_pVideoTrackCaller->Start(false);
		}
        for (unsigned int i = 0; i < m_pVideoTrackCallee.size(); i++) {
            if (m_pVideoTrackCallee[i]) {
				if (m_winCallee.size() > i)
				{
#ifdef WP8
					m_pVideoTrackCallee[i]->AddExternalRender((IWmeVideoRender*)m_winCallee[i]);
#else
					m_pVideoTrackCallee[i]->AddRenderWindow(m_winCallee[i]);
#endif
				}
                m_pVideoTrackCallee[i]->Start(false);
            }
        }
        if(m_pScreenTrackViewer && m_winScreenViewer){
#ifdef WP8
			m_pScreenTrackViewer->AddExternalRender((IWmeVideoRender*)m_winScreenViewer);
#else
			m_pScreenTrackViewer->AddRenderWindow(m_winScreenViewer);
#endif
            m_pScreenTrackViewer->Start(false);
        }
        if (m_pScreenTrackSharer && m_winScreenPreview)
        {
#ifdef WP8
			m_pScreenTrackSharer->AddExternalRender((IWmeVideoRender*)m_winScreenPreview);
#else
			m_pScreenTrackSharer->AddRenderWindow(m_winScreenPreview);
#endif
            m_pScreenTrackSharer->Start(false);
        }

        
        if (m_pAudioTrackCaller) {
			m_pAudioTrackCaller->Start(false);
		}
		if (m_pAudioTrackCallee) {
			m_pAudioTrackCallee->Start(false);
        }
	} else {
		if (m_pVideoTrackCaller) {
			m_pVideoTrackCaller->RemoveRenderWindow(m_winCaller);
            m_pVideoTrackCaller->Stop();
		}
        for (unsigned int i = 0; i < m_pVideoTrackCallee.size(); i++) {
            if (m_pVideoTrackCallee[i]) {
                if(m_winCallee.size() > i)
                    m_pVideoTrackCallee[i]->RemoveRenderWindow(m_winCallee[i]);
                m_pVideoTrackCallee[i]->Stop();
            }
        }
        if(m_pScreenTrackViewer && m_winScreenViewer){
            m_pScreenTrackViewer->RemoveRenderWindow(m_winScreenViewer);
            m_pScreenTrackViewer->Stop();
        }
        if (m_pScreenTrackSharer && m_winScreenPreview)
        {
            m_pScreenTrackSharer->RemoveRenderWindow(m_winScreenPreview);
            m_pScreenTrackSharer->Stop();
        }
        
        if (m_pAudioTrackCaller) {
			m_pAudioTrackCaller->Stop();
		}
		if (m_pAudioTrackCallee) {
			m_pAudioTrackCallee->Stop();
        }
	}
}

void Endpoint::holdCall(bool hold)
{
    if(m_pMediaConn)
    {
        WmeDirection direction = hold ? WmeDirection_Inactive : WmeDirection_SendRecv;
        m_pMediaConn->UpdateMedia(direction, AUDIO_MID);
        m_pMediaConn->UpdateMedia(direction, VIDEO_MID);
        m_pMediaConn->CreateOffer();
    }
}

long Endpoint::startStopTrack(WmeSessionType mediaType, bool bRemote, bool bStart)
{
	long ret = -1;
	IWmeMediaTrackBase* pTrack = NULL;
	if (mediaType == WmeSessionType_Audio){
		if (bRemote){
			pTrack = m_pAudioTrackCallee;
		}
		else{
			pTrack = m_pAudioTrackCaller;
		}
	}
	else if (mediaType == WmeSessionType_Video){
		if (bRemote){
			pTrack = m_pVideoTrackCallee[0];
		}
		else{
			pTrack = m_pVideoTrackCaller;
		}
	}
	else if (mediaType == WmeSessionType_ScreenShare){
		if (bRemote){
			pTrack = m_pScreenTrackViewer;
		}
		else{
			pTrack = m_pScreenTrackSharer;
		}
	}
	if (pTrack != NULL){
		if (bStart)
			ret = pTrack->Start(false);
		else
			ret = pTrack->Stop();
	}
	return ret;
}

std::string Endpoint::GetDebugOption(WmeSessionType mediaType)
{
	Json::FastWriter w;
	std::string ret;
	if (mediaType == WmeSessionType_Audio)
    {
		ret = w.write(TestConfig::i().m_audioDebugOption);
    }
	else if (mediaType == WmeSessionType_Video)
    {
		ret = w.write(TestConfig::i().m_videoDebugOption);
    }
    else if (mediaType == WmeSessionType_ScreenShare){
        ret = w.write(TestConfig::i().m_shareDebugOption);
    }
    
	return ret;
}

std::string Endpoint::GetParam(WmeSessionType mediaType)
{
    Json::Value root(Json::objectValue);
    if(mediaType == WmeSessionType_Audio){
        root["audio"] = TestConfig::i().m_audioParam;
    }else if(mediaType == WmeSessionType_Video){
		root["video"] = TestConfig::i().m_videoParam;
    }else if(mediaType == WmeSessionType_Video){
        root["share"] = TestConfig::i().m_shareParam;
    }
    Json::FastWriter w;
    string sTxt = w.write(root);
    return sTxt;
}


std::string Endpoint::GetGlobalParam()
{
    Json::Value root(Json::objectValue);
    root = TestConfig::i().m_globalParam;
    Json::FastWriter w;
    string sTxt = w.write(root);
    return sTxt;
}

CScreenTrackViewAnalyzer * Endpoint::getCScreenTrackViewAnalyzer(){
	if (!m_pScreenTrackViewAnalyzer)
	{
		try {
			m_pScreenTrackViewAnalyzer = new CScreenTrackViewAnalyzer;
			m_pScreenTrackViewAnalyzer->AddRef();
		}
		catch (std::bad_alloc) {
			return NULL;
		}
	}
	return m_pScreenTrackViewAnalyzer;
}
void Endpoint::AddScreenTrackViewAnalyzerAsExternalRender(IWmeMediaTrackBase *lpScreenTrackViewer) {
    
    CM_INFO_TRACE_THIS("Endpoint::AddScreenTrackViewAnalyzerAsExternalRender");
    
    if (!lpScreenTrackViewer)
        return;

	CScreenTrackViewAnalyzer *pCScreenTrackViewAnalyzer = getCScreenTrackViewAnalyzer();
	if (pCScreenTrackViewAnalyzer){
        pCScreenTrackViewAnalyzer->EnableDumpFrame2File(TestConfig::i().m_bDumpReceivedScreenSharingToImage);
        pCScreenTrackViewAnalyzer->EnableScanQRCode(TestConfig::i().m_bScreenTrackViewAnalyzer);
		lpScreenTrackViewer->AddExternalRender(pCScreenTrackViewAnalyzer);
		if (TestConfig::Instance().m_strScreenScanConfigJson.size() > 0){
			pCScreenTrackViewAnalyzer->SetReplayTempFiles(TestConfig::i().m_bScreenReplayTAQRcodeFiles);
			pCScreenTrackViewAnalyzer->SetQRCodeContextWithJson(TestConfig::Instance().m_strScreenScanConfigJson);
		}
	}
    
}

void Endpoint::OnTimer(CCmTimerWrapperID* aId)
{
    if(aId == &m_svsTimer)
    {
        static uint32_t s_ulTick = 0;
        //Use static string to allocate the buffer, so it will be released at the end of program.
        static std::string sBuffer;
        s_ulTick += 15;
        WmeVideoRawFormat svsFormat;
        svsFormat.uTimestamp = s_ulTick;
        svsFormat.eRawType = WmeBGR24;
        svsFormat.iWidth = 640;
        svsFormat.iHeight = 480;
        svsFormat.fFrameRate = 24.0f;
        unsigned int uiLength = svsFormat.iWidth * svsFormat.iHeight * 3;
        if (sBuffer.capacity() < uiLength) {
            sBuffer.reserve(uiLength);
        }
        unsigned char * pSrcData = (unsigned char *)sBuffer.c_str();
        memset(pSrcData, 55 + s_ulTick % 200, sizeof(unsigned char) * uiLength);
        m_pSvsInputPin->InputMediaData(0, WmeMediaFormatVideoRaw, &svsFormat, pSrcData, uiLength);
        
        if(m_pSvsInputPin2) {
            svsFormat.iWidth = 320;
            svsFormat.iHeight = 240;
            uiLength = svsFormat.iWidth * svsFormat.iHeight * 3;
            m_pSvsInputPin2->InputMediaData(0, WmeMediaFormatVideoRaw, &svsFormat, pSrcData, uiLength);
        }
    }
    else if(aId == &m_svsAudioTimer)//everty 10 ms call back
    {
        if(NULL != m_pSvsAudioInputPin)
        {
            WmeAudioRawFormat rawFormat;
            rawFormat.eRawType = WmePCM;
            rawFormat.iChannels = 1;
            rawFormat.iSampleRate = 16000;
            rawFormat.iBitsPerSample = 16;
            static uint32_t nTimeStamp = 0;
            nTimeStamp += 10*rawFormat.iSampleRate/1000;
            unsigned int uiLength = rawFormat.iChannels*rawFormat.iSampleRate*rawFormat.iBitsPerSample/8*10/1000;
            
            static std::string sAudioSvsData;
            if (sAudioSvsData.capacity() < uiLength) {
                sAudioSvsData.reserve(uiLength);
            }
            unsigned char * pSrcData = (unsigned char *)sAudioSvsData.c_str();
            short * pAudioData = (short*)pSrcData;
            static double fStart = 0;
            for(unsigned int i=0;i<uiLength/2;i++)
            {
                pAudioData[i] = (short)(16384.0*sin(fStart*2*3.1415926*1000/16000.0));//1000 hz samples
                fStart += 1.0;
            }
            m_pSvsAudioInputPin->InputMediaData(nTimeStamp, WmeMediaFormatAudioRaw, &rawFormat, pSrcData, uiLength);
        }
    }else if(aId == &m_svsLocalStats && m_pSvsLocalVideo2) {
        WmeVideoStatistics vidStats = {0};
        if(WME_SUCCEEDED(m_pSvsLocalVideo2->GetVideoStatistics(vidStats))) {
            CM_INFO_TRACE_THIS("sqvideotx, 2, dim:" << vidStats.uWidth << "x" << vidStats.uHeight << ", encoded=" << vidStats.uEncodeFrameCount);
        }
    }else if(aId == &m_svsDelayAddTimer) {
        IWmeMediaConnection4T *pConn = (IWmeMediaConnection4T*)m_pMediaConn;
        uint8_t vid = 0;
        pConn->AddLocalTrack(m_videoMid, 5555, vid);

    }
}

void Endpoint::OnMediaReady(unsigned long mid, WmeDirection direction,
		WmeSessionType mediaType, IWmeMediaTrackBase *pTrack) {
	CM_INFO_TRACE_THIS(
			"Endpoint::OnMediaReady, mid=" << mid << ", direction=" << direction << ", pTrack=" << pTrack);
	bool bNeedStart = false;
	if (mediaType == WmeSessionType_Video) {
        if (TestConfig::Instance().m_bHasSVS) {
            if (direction == WmeDirection_Send) {
                if(m_pSvsInputPin == NULL) {
                    pTrack->GetExternalInputPin(&m_pSvsInputPin);
                    m_svsTimer.ScheduleInThread(TT_MAIN, this, CCmTimeValue(0, 70000));
                } else {
                    m_svsLocalStats.ScheduleInThread(TT_MAIN, this, CCmTimeValue(1, 0));
                    m_pSvsLocalVideo2 = pTrack;
                    pTrack->AddRef();
                    pTrack->GetExternalInputPin(&m_pSvsInputPin2);
                    pTrack->Start(false);
                    return;
                }
            }
        }
        bNeedStart = TestConfig::Instance().m_bHasVideo;
        void *render = NULL;
        if(direction == WmeDirection_Send) {
            render = m_winCaller;
        }
        else if(m_winCallee.size() > m_trackIndex) {
            render = m_winCallee[m_trackIndex];
        }
        if(direction == WmeDirection_Recv) {
            if(TestConfig::i().m_bDumpRTP && m_pVideoRtpDumper == NULL) {
                m_pVideoRtpDumper = new ClickCall::CRtpDumper();
                pTrack->AddExternalRender(m_pVideoRtpDumper);
            }
            m_pVideoTrackCallee.push_back(pTrack);
            unsigned int vid = 255;
            pTrack->GetVid(vid);
            m_trackIndex++;
        }
        else {
            m_pVideoTrackCaller = pTrack;
        }
        
        if (TestConfig::Instance().m_bVideoHW) {
            m_pMediaConn->GetVideoConfig(mid)->EnableHWAcceleration(true, WmeHWAcceleration_Encoder);
#ifdef ANDROID
            m_pMediaConn->GetVideoConfig(mid)->EnableHWAcceleration(true, WmeHWAcceleration_Decoder);
#else
            m_pMediaConn->GetVideoConfig(mid)->EnableHWAcceleration(true, WmeHWAcceleration_Decoder);
#endif
        }
        
		pTrack->AddRef();
#ifndef WP8
        if (render != NULL) {

            pTrack->AddRenderWindow(render);

        }
#endif
        bool bMuteVideo = (direction == WmeDirection_Send) ? TestConfig::i().m_bMuteVideo : false;
        pTrack->Start(bMuteVideo);

#ifdef WP8
		if (render != NULL) {
			pTrack->AddExternalRender((IWmeVideoRender*)render);
		}
#endif
	}
	else if (mediaType == WmeSessionType_Audio) {
        if (TestConfig::Instance().m_bHasSVS) {
            if (direction == WmeDirection_Send) {
                if(m_pSvsAudioInputPin == NULL) {
                    pTrack->GetExternalInputPin(&m_pSvsAudioInputPin);
                    m_svsAudioTimer.ScheduleInThread(TT_MAIN, this, CCmTimeValue(0, 10000));
                }else{
                    m_pSvsLocalAudio2 = pTrack;
                    pTrack->AddRef();
                    pTrack->GetExternalInputPin(&m_pSvsAudioInputPin2);
                    pTrack->Start(false);
                    return;
                }
            }
        }
		bNeedStart = TestConfig::Instance().m_bHasAudio;
		IWmeMediaTrackBase ** ppTrack =
				(direction == WmeDirection_Send) ?
						&m_pAudioTrackCaller : &m_pAudioTrackCallee;
		*ppTrack = (IWmeMediaTrackBase *) pTrack;
		(*ppTrack)->AddRef();
        bool bMuteAudio = (direction == WmeDirection_Send) ? TestConfig::i().m_bMuteAudio : false;
        if(direction == WmeDirection_Send) {
			//m_externalAuidoRender[0] = new ExternalAudioRender(WmeExternalRender_Type_CaptureFromHardware);
			//(*ppTrack)->AddExternalRender(m_externalAuidoRender[0]);
			//m_externalAuidoRender[1] = new ExternalAudioRender(WmeExternalRender_Type_CaptureBeforeEncode);
			//(*ppTrack)->AddExternalRender(m_externalAuidoRender[1]);
            if (!TestConfig::i().m_bLoopback || m_bHost)
                pTrack->Start(bMuteAudio);
        }else {
			if (NULL == m_externalAuidoRender[2])
			{
				//m_externalAuidoRender[2] = new ExternalAudioRender(WmeExternalRender_Type_PlaybackToHardware);
				//(*ppTrack)->AddExternalRender(m_externalAuidoRender[2]);
			}
			else if (NULL == m_externalAuidoRender[3])
			{
				//m_externalAuidoRender[3] = new ExternalAudioRender(WmeExternalRender_Type_PlaybackToHardware);
				//(*ppTrack)->AddExternalRender(m_externalAuidoRender[3]);
			}
            if(TestConfig::i().m_bDumpRTP && m_pAudioRtpDumper == NULL) {
                m_pAudioRtpDumper = new ClickCall::CRtpDumper();
                pTrack->AddExternalRender(m_pAudioRtpDumper);
            }

            if (!TestConfig::i().m_bLoopback || !m_bHost)
                pTrack->Start(bMuteAudio);
        }
	}
	else if (mediaType == WmeSessionType_ScreenShare) {
		bNeedStart = TestConfig::Instance().m_bAppshare;
		IWmeMediaTrackBase ** ppTrack = (direction == WmeDirection_Send) ?
			&m_pScreenTrackSharer : &m_pScreenTrackViewer;
		*ppTrack = (IWmeMediaTrackBase *)pTrack;
		(*ppTrack)->AddRef();
		if (direction == WmeDirection_Recv){
            if(m_winScreenViewer){
#ifdef WP8
				(*ppTrack)->AddExternalRender((IWmeVideoRender*)m_winScreenViewer);
#else
				(*ppTrack)->AddRenderWindow(m_winScreenViewer);
#endif
                (*ppTrack)->SetRenderMode(TestConfig::i().m_eRenderModeScreenSharing);
            
                if (TestConfig::i().m_bScreenTrackViewAnalyzer
                    || TestConfig::i().m_bDumpReceivedScreenSharingToImage)
                {
                    AddScreenTrackViewAnalyzerAsExternalRender(m_pScreenTrackViewer);
                }
            }
		}
		else
		{

            if( m_pScreenTrackSharer )
            {
                updateScreenFileCaptureFileName();
                
                for (int i = 0; i < m_vctToShareScreenSource.size(); i++)
                {
                    addScreenSouce(m_vctToShareScreenSource[i]);
                }
				bNeedStart = (bNeedStart && (m_vctToShareScreenSource.size() != 0));
            }
		}
		if (bNeedStart)
			(*ppTrack)->Start(false);
        std::ostringstream strShareId;
        strShareId << "refApp-share-id-" << std::rand();
        (*ppTrack)->SetScreenSharingID(strShareId.str().c_str());
	}
    
    // Allow Calabash to verify these callback variables:
    mCalabash->onMediaReadyStore(mid, direction, mediaType, pTrack);
}

void Endpoint::OnParsedMLine(WmeSessionType mediaType, unsigned long mid)
{
    switch (mediaType) {
        case WmeSessionType_Audio:
            m_audioMid = mid;
            break;
        case WmeSessionType_Video:
            m_videoMid = mid;
            break;
        case WmeSessionType_ScreenShare:
            m_shareMid = mid;
            break;
        default:
            break;
    }
	m_pMediaConn->AddMedia(mediaType, WmeDirection_Recv, mid, GetDebugOption(mediaType).c_str());
}

void Endpoint::OnSDPReady(WmeSdpType sdpType, const char *sdp) {
//	CM_INFO_TRACE_THIS("Endpoint::OnSDPReady, type=" << sdpType << ", sdp=" << sdp);
    CM_INFO_TRACE_BIG_EX("EndPoint", "Endpoint::OnSDPReady, type=" << sdpType << ", sdp=" << sdp);
    if(sdp == NULL)
        return;

    std::string strFaked = sdp;
    if(TestConfig::i().m_bFakeSVCByAVC)
    {
        strFaked = regex_replace(sdp, regex("H264-SVC/"), "H264/");
        sdp = strFaked.c_str();
    }
    m_localSdp = sdp;
	string sType = (sdpType == WmeSdp_Offer) ? "offer" : "answer";
    if (m_pSink && !TestConfig::i().m_bAudioProcess) {
		m_pSink->onEvent(this, sType, sdp);
    }
    if (TestConfig::i().m_pAppSink) {
        TestConfig::i().m_pAppSink->onSdpReady("onSdpReady");
    }
}

void Endpoint::OnSessionStatus(unsigned long mid, WmeSessionType mediaType, WmeSessionStatus status)
{
    CM_INFO_TRACE_THIS("Endpoint::OnSessionStatus, mid=" << mid << ", mediaType=" << mediaType << ", status=" << status);
    switch (status) {
        case WmeSession_FileCaptureEnded:
            if (mediaType == WmeSessionType_Video) {
                m_videoFileEnded = true;
            } else if (mediaType == WmeSessionType_Audio) {
                m_audioFileEnded = true;
            }
            break;
#ifdef WP8
        case WmeSession_Connected: 
            if (mediaType == WmeSessionType_Audio)
            {
                AudioRoutingManager^ m_audioRouteManager = AudioRoutingManager::GetDefault();
                m_audioRouteManager->SetAudioEndpoint(AudioRoutingEndpoint::Speakerphone);
            }
            break;
#endif
        default:
            break;
    }
}
void Endpoint::OnError(int errorCode)
{
    switch(errorCode){
        case WME_E_SCREEN_SHARE_CAPTURE_FAIL:
            CM_ERROR_TRACE_THIS("Endpoint::OnError(WME_E_SCREEN_SHARE_CAPTURE_FAIL) >> screen capture fail");
            CM_ASSERTE(false);
            break;
        case WME_E_SCREEN_SHARE_CAPTURE_DISPLAY_PLUGOUT:
            CM_ERROR_TRACE_THIS("Endpoint::OnError(WME_E_SCREEN_SHARE_CAPTURE_DISPLAY_PLUGOUT) >> sharing display is plugout!");
            CM_ASSERTE(false);
            break;
        case WME_E_SCREEN_SHARE_CAPTURE_NO_APP_SOURCE:
            CM_ERROR_TRACE_THIS("Endpoint::OnError(WME_E_SCREEN_SHARE_CAPTURE_NO_APP_SOURCE) >> no appshare source!");
            CM_ASSERTE(false);
            break;
        default:
            CM_ERROR_TRACE_THIS("Endpoint::OnError, errorCode=" << errorCode);
    }
}
void Endpoint::OnMediaBlocked(unsigned long mid, unsigned long vid, bool blocked)
{
    /*
    //test code for using 1 m-line for video and screen
    if(mid==VIDEO_MID
       && !blocked
       && m_pMediaConn){
        string sParams = "{\"video\":{\"scr\":{\"vid\":0,\"br\":1000000,\"priority\":-1,\"csi\":0,\"fs\":8160,\"fps\":3000,\"mbps\":27600}}}";
        m_pMediaConn->SetParams(VIDEO_MID, sParams.c_str(), sParams.size());
        m_pMediaConn->Subscribe(VIDEO_MID, WmeSCRRequestPolicyType_ActiveSpeaker, 8160, 3000, 10000000, 2760, 27600, 0);
    }
     */
    
}
void Endpoint::OnMediaError(unsigned long mid, unsigned long vid, WMERESULT errorCode)
{
    switch(errorCode){
        case WME_E_VIDEO_ENCODE_FAIL:
            CM_ERROR_TRACE_THIS("Endpoint::OnMediaError(WME_E_VIDEO_ENCODE_FAIL),mid=" << mid
                                << ", vid=" << vid
                                << ",WME_E_VIDEO_ENCODE_FAIL");
            //CM_ASSERTE(false);
            break;
        case WME_E_VIDEO_DECODE_FAIL:
            CM_ERROR_TRACE_THIS("Endpoint::OnMediaError(WME_E_VIDEO_DECODE_FAIL),mid=" << mid
                                << ", vid=" << vid
                                << ",WME_E_VIDEO_DECODE_FAIL");
            //CM_ASSERTE(false);
            break;
        default:
            CM_ERROR_TRACE_THIS("Endpoint::OnMediaError, mid=" << mid
                                << ", vid=" << vid
                                << ", errorCode=" << errorCode);
    }
}

IWmeMediaTrackBase* Endpoint::FindTrackByVid(uint8_t vid)
{
    for(auto &pTrack : m_pVideoTrackCallee) {
        unsigned int trackvid = 255;
        pTrack->GetVid(trackvid);
        if(vid == trackvid) {
            return pTrack;
        }
    } 
    return NULL;
}

void Endpoint::HideDuplication(unsigned int csi)
{
    CM_INFO_TRACE_THIS("Endpoint::OnCSIsChanged, mid=2, HideDuplication, hide csi=" << csi);
    for(auto &pTrack : m_pVideoTrackCallee) {
        unsigned int tVid = 255;
        pTrack->GetVid(tVid);
        if(tVid == 0)
            continue;
        
        unsigned int itCSIArray = 0;
        unsigned int itCSICount = 1;
        pTrack->GetCSI(&itCSIArray, itCSICount);
        
        WmeMediaStatus mediaStatus = WmeMedia_Available;
        pTrack->GetMediaStatus(mediaStatus);
        
        if((itCSIArray & 0xFFFFFF00) == (csi & 0xFFFFFF00))
            ShowVideo(pTrack, tVid, false);
        else //if(mediaStatus == WmeMedia_Available)
            ShowVideo(pTrack, tVid, true);
    }
}

IWmeMediaTrackBase* Endpoint::FindTrackByCsi(unsigned int csi, bool bAvailable, uint8_t &itvid)
{
    for(auto &pTrack : m_pVideoTrackCallee) {
        unsigned int itCSIArray = 0;
        unsigned int itCSICount = 1;
        pTrack->GetCSI(&itCSIArray, itCSICount);
        unsigned int tVid = 255;
        pTrack->GetVid(tVid);
        itvid = tVid;
        WmeMediaStatus mediaStatus = WmeMedia_Available;
        pTrack->GetMediaStatus(mediaStatus);
        
        if(itvid != 0 && itvid < m_winCallee.size()) {
            if(itCSIArray == csi && (bAvailable == (mediaStatus == WmeMedia_Available))) {
                return pTrack;
            }
        }
    }
    return NULL;
}

void Endpoint::ShowVideo(IWmeMediaTrackBase* pTrack, unsigned int vid, bool bShow)
{
    if(pTrack == NULL || vid >= m_winCallee.size())
        return;
    CM_INFO_TRACE_THIS("Endpoint::OnCSIsChanged, mid=2, vid=" << vid << ", bShow=" << bShow);
    if(bShow) {
        pTrack->AddRenderWindow(m_winCallee[vid]);
    }else{
        pTrack->RemoveRenderWindow(m_winCallee[vid]);
    }
    if(TestConfig::i().m_pAppSink)
        TestConfig::i().m_pAppSink->ShowWindow(m_winCallee[vid], bShow);
}

void Endpoint::OnMediaStatus(unsigned long mid, unsigned long vid, WmeMediaStatus status, bool hasCSI, uint32_t csi)
{
    CM_INFO_TRACE_THIS("Endpoint::OnMediaStatus,OnCSIsChanged,mid=" << mid << ", vid=" << vid << ",status=" << status << ",csi=" << (hasCSI ? csi : (uint32_t)-1));
    if (TestConfig::i().m_pAppSink && vid < m_winCallee.size()) {
        TestConfig::i().m_pAppSink->ShowMuteWindow(m_winCallee[vid], status != WmeMedia_Available);
    }
    if(vid == 0 && hasCSI) {
        HideDuplication(csi);
        m_possibleActiveCsi = csi;
    }
}

void Endpoint::OnCSIsChanged(unsigned long mid, unsigned int vid, const unsigned int* oldCSIArray, unsigned int oldCSICount, const unsigned int* newCSIArray, unsigned int newCSICount)
{
    CM_INFO_TRACE_THIS("Endpoint::OnCSIsChanged, mid=" << mid << ", vid=" << vid << ",oldCSICount=" << oldCSICount << ",newCSICount=" << newCSICount);
    if (mCalabash != NULL) {
        const char *mediaType = "audio";
        if (mid == VIDEO_MID)
            mediaType = "video";
        else if (mid == SCREEN_MID)
            mediaType = "share";
        mCalabash->onCSIChanged(mediaType, vid, newCSIArray, newCSICount);
    }
    if(mid == m_videoMid && !TestConfig::i().m_bDuplication) {
        if(newCSICount == 0 && oldCSICount > 0) {
            for(auto & it : m_csivids) {
                if(it.csi == oldCSIArray[0] && vid == it.vid){
                    CM_INFO_TRACE_THIS("Endpoint::OnCSIsChanged, mid=" << mid << ",removed=" << it.csi << ",vid=" << it.vid);
                    it.vid = 255;
                    break;
                }
            }
        } else if(newCSICount > 0) {
            for(auto & it : m_csivids) {
                if(it.vid == vid){
                    CM_INFO_TRACE_THIS("Endpoint::OnCSIsChanged, mid=" << mid << ",removed=" << it.csi << ",vid=" << it.vid);
                    it.vid = 255;
                    if(vid == 0)
                        it.bHide = true;
                    break;
                }
            }
            bool bFound = false;
            for(auto & it : m_csivids) {
                if(it.csi == newCSIArray[0] && it.vid != 0){
                    it.vid = vid;
                    if(vid != 0)
                        it.o_vid = it.vid;
                    CM_INFO_TRACE_THIS("Endpoint::OnCSIsChanged, mid=" << mid << ",added=" << it.csi << ", vid=" << it.vid);
                    bFound = true;
                    break;
                }
            }
            if(!bFound) {
                CM_INFO_TRACE_THIS("Endpoint::OnCSIsChanged, mid=" << mid << ",added=" << newCSIArray[0] << ", vid=" << vid);
                struct csi_vid tmp_csivid = {newCSIArray[0], (uint8_t)vid, 255, false};
                if(vid != 0)
                    tmp_csivid.o_vid = tmp_csivid.vid;
                m_csivids.push_back(tmp_csivid);
            }
        }
        
        for(auto & it : m_csivids) {
            if(it.bHide && it.vid == 255 && it.o_vid != 255) {
                bool bConflict = false;
                for(auto itsec : m_csivids) {
                    if(itsec.vid == it.o_vid)
                        bConflict = true;
                }
                if(!bConflict) {
                    it.bHide = false;
                    it.vid = it.o_vid;
                    CM_INFO_TRACE_THIS("Endpoint::OnCSIsChanged, mid=" << mid << ",fix vid " << it.vid << " for " << it.csi);
                }
                break;
            }
        }
        
        bool vidEmpty[255] = {true};
        for(auto & it : m_csivids) {
            IWmeMediaTrackBase* pTrack = FindTrackByVid(it.vid);
            ShowVideo(pTrack, it.vid, true);
            CM_INFO_TRACE_THIS("Endpoint::OnCSIsChanged, mid=2, it.vid = " << it.vid);
            if(it.vid < 255)
                vidEmpty[it.vid] = false;
        }
        
        for(uint8_t itvid = 0; itvid < m_winCallee.size(); itvid++) {
            if(vidEmpty[itvid]) {
                IWmeMediaTrackBase* pTrack = FindTrackByVid(itvid);
                ShowVideo(pTrack, itvid, false);
            }
        }
    }
    
    if(mid == m_videoMid && TestConfig::i().m_bDuplication) {
        if(newCSICount == 0 && oldCSICount > 0) {
            return;
        }
        if(vid == 0 && newCSICount > 0) {
            HideDuplication(newCSIArray[0]);
        }else if(newCSICount > 0 && vid != 0) {
            //Save current active video csi, so if duplication vid comes later, we need to hide it.
            unsigned int itActiveCSI = (unsigned int)-1;
            unsigned int itCSICount = 1;
            IWmeMediaTrackBase* pActive = FindTrackByVid(0);
            
            if(pActive) {
                pActive->GetCSI(&itActiveCSI, itCSICount);
                WmeMediaStatus mediaStatus = WmeMedia_Available;
                pActive->GetMediaStatus(mediaStatus);
                if(mediaStatus != WmeMedia_Available)
                    itActiveCSI = m_possibleActiveCsi;
                HideDuplication(itActiveCSI);
            }
        }
    }
}

void Endpoint::OnNetworkStatus(WmeNetworkStatus status, WmeNetworkDirection direc)
{
    CM_INFO_TRACE_THIS("Endpoint::OnNetworkStatus, direc = "<< direc << ", status = " << status);
    if ( direc == DIRECTION_UPLINK ){
        m_uplinkStatus = status;
    }
    switch (status) {
        case WmeNetwork_bad:
            break;
        case WmeNetwork_video_off:
            if ( direc == DIRECTION_UPLINK ){
                if (TestConfig::i().m_bTestNetWarnning)
                    mute(true, true);
            }
            break;
        case WmeNetwork_recovered:
            // TODO: It's not easy to tell whether it's also to turn video on based on current metrics
            // The potential risk is after turning video on, the video will still freeze because the bandwidth
            // only enough for audio.
            // it would be better if we can get the wireless signal and only re-enable video when signal recovered.
            if ( direc == DIRECTION_UPLINK ){
                if (TestConfig::i().m_bTestNetWarnning)
                    mute(false, true);
            }
            break;
        default:
            break;
    }
}

void Endpoint::OnAudioTimeStampChanged(uint32_t csi, uint32_t timestamp)
{
    if (TestConfig::i().m_pAppSink) {
        Json::Value root(Json::objectValue);
        root["csi"] = csi;
        root["timestamp"] = timestamp;
        Json::FastWriter w;
        string sTxt = w.write(root);
        TestConfig::i().m_pAppSink->onCallback("OnAudioTimeStampChanged", sTxt);
    }
}

void Endpoint::OnSDPReceived(const string &type, const string &text) {
	CM_INFO_TRACE_THIS("Endpoint::OnSDPReceived, type=" << type);
    if (TestConfig::i().m_pAppSink) {
        TestConfig::i().m_pAppSink->onSdpReady("OnSDPReceived");
    }
	if (type == "offer") {
		acceptCall(text.c_str());
	} else if (type == "answer") {
		answerReceived(text.c_str());
	}
}
void Endpoint::updateScreenCaptureMacFps(){
    
    if(TestConfig::i().m_nSceenCaptureDownSampleMin>0){
        IShareConfig* pIShareConfig = m_pMediaConn->GetShareConfig(m_shareMid);
        if(pIShareConfig)
            pIShareConfig->SetScreenCaptureDownSampleMinHeight(TestConfig::i().m_nSceenCaptureDownSampleMin);
    }
    
    if(TestConfig::i().m_nMaxScreenCaptureFps>0){
        IShareConfig* pIShareConfig = m_pMediaConn->GetShareConfig(m_shareMid);
        if(pIShareConfig){
            pIShareConfig->SetScreenMaxCaptureFps(TestConfig::i().m_nMaxScreenCaptureFps);
            if(pIShareConfig->GetScreenMaxCaptureFps()!=TestConfig::i().m_nMaxScreenCaptureFps)
                CM_ERROR_TRACE("Error: GetScreenMaxCaptureFps != SetScreenMaxCaptureFps");
        }
    }
    
    if(TestConfig::i().m_nAppshareInitBw>0){
        IShareConfig* pIShareConfig = m_pMediaConn->GetShareConfig(m_shareMid);
        if(pIShareConfig){
            pIShareConfig->SetInitBandwidth(TestConfig::i().m_nAppshareInitBw);
        }

    }
    
}
void Endpoint::updateScreenFileCaptureFileName(){
    
    if(TestConfig::Instance().m_strScreenSharingCaptureFile.size()>0){
        if(m_pScreenFileCapturer==NULL){
            m_pScreenFileCapturer = new CScreenFileCapturer(TestConfig::Instance().m_strScreenSharingCaptureFile);
        }
        else
            m_pScreenFileCapturer->SetFilePath(TestConfig::i().m_strScreenSharingCaptureFile);
        if(m_pScreenFileCapturer){
            m_pScreenFileCapturer->SetCaptureSkipCount(TestConfig::i().m_nScreenSharingFileCaptureSkip);
            m_pScreenTrackSharer->SetExternalScreenCapturer(m_pScreenFileCapturer);
        }
    }
}
void Endpoint::addScreenSouce(IWmeScreenSource *pScreenSource)
{
	if (!pScreenSource)
		return;
	if (std::find(m_vctToShareScreenSource.begin(), m_vctToShareScreenSource.end(), pScreenSource) == m_vctToShareScreenSource.end()){
		m_vctToShareScreenSource.push_back(pScreenSource);
		pScreenSource->AddRef();
	}

	if (m_pScreenTrackSharer){
		if (TestConfig::Instance().m_handleSharedWindow)
			m_pScreenTrackSharer->AddSharedWindow(TestConfig::Instance().m_handleSharedWindow);
		m_pScreenTrackSharer->AddScreenSource(pScreenSource);
	}
	else{
        CM_ERROR_TRACE("Endpoint::addScreenSouce m_pScreenTrackSharer = NULL");
    }
}

void Endpoint::removeScreenSouce(IWmeScreenSource *pScreenSource)
{
	if (!pScreenSource)
		return;
	std::vector <IWmeScreenSource*>::iterator it = std::find(m_vctToShareScreenSource.begin(), m_vctToShareScreenSource.end(), pScreenSource);
	if (it != m_vctToShareScreenSource.end()){
		pScreenSource->Release();
		m_vctToShareScreenSource.erase(it);
	}

	if (m_pScreenTrackSharer){
		m_pScreenTrackSharer->RemoveScreenSource(pScreenSource);
	}
	else{
		CM_ERROR_TRACE("Endpoint::addScreenSouce m_pScreenTrackSharer = NULL");
	}
}

void Endpoint::setCamera(IWmeMediaDevice *pDevice)
{
    if (m_pVideoTrackCaller && pDevice)
        m_pVideoTrackCaller->SetCaptureDevice(pDevice);
}

void Endpoint::startSVS()
{
    if(m_pSvsInputPin == NULL && m_pVideoTrackCaller) {
        m_pVideoTrackCaller->GetExternalInputPin(&m_pSvsInputPin);
        m_svsTimer.ScheduleInThread(TT_MAIN, this, CCmTimeValue(0, 70000));
    }
}

WMERESULT Endpoint::setCamera(const char * pCurrCamera) {
	CM_INFO_TRACE("[NATIVE] setCamera, input = " << pCurrCamera);

    if(pCurrCamera == NULL) {
        m_svsTimer.Cancel();
        m_svsDelayAddTimer.Cancel();
        SAFE_RELEASE(m_pSvsInputPin);
        SAFE_RELEASE(m_pSvsInputPin2);
        auto firstCamera = m_cameraList.begin();
        if(m_pVideoTrackCaller) {
            return m_pVideoTrackCaller->SetCaptureDevice(firstCamera->second);
        }
        return WME_E_FAIL;
    }
    
	if(m_pVideoTrackCaller && pCurrCamera && m_cameraList.find(pCurrCamera)!=m_cameraList.end()) {
		return m_pVideoTrackCaller->SetCaptureDevice(m_cameraList[pCurrCamera]);
    }
    return WME_E_FAIL;
}

bool Endpoint::setAudioPlaybackDevice(const char *pDevName)
{
    CM_INFO_TRACE("[NATIVE] setAudioPlaybackDevice, input = "<< pDevName);

    WMERESULT ret = WME_S_OK;

    IWmeMediaEngine *mediaEngine = NULL;
    if( m_pMediaConn )
    mediaEngine = m_pMediaConn->GetEngine();
    if (mediaEngine==NULL) {
        CM_INFO_TRACE("[NATIVE] setAudioPlaybackDevice, create mediaengine failed ret = "<<ret);
        return false;
    }

    IWmeMediaDeviceEnumerator *pWmeMediaDeviceEnumerator = NULL;
    ret = mediaEngine->CreateMediaDeviceEnumerator(WmeMediaTypeAudio, WmeDeviceOut, &pWmeMediaDeviceEnumerator);
    if (pWmeMediaDeviceEnumerator) {
        IWmeMediaDevice *pDevice = NULL;
        int nDevCount = 0;
        pWmeMediaDeviceEnumerator->GetDeviceNumber(nDevCount);

        for (int i=0; i<nDevCount; i++) {
            CM_INFO_TRACE("[NATIVE] setAudioPlaybackDevie, index i = "<<i);
            IWmeMediaDevice *pDeviceTmp = NULL;
            pWmeMediaDeviceEnumerator->GetDevice(i, &pDeviceTmp);
            if (pDeviceTmp) {
                int szLen = 5120;
                char szDeviceName[5120];
                memset(szDeviceName, 0, sizeof(szDeviceName));
                pDeviceTmp->GetFriendlyName(szDeviceName, szLen);
                if (strcmp(szDeviceName, "Route_speaker") == 0) {
                    pDevice = pDeviceTmp;
                    if (m_pAudioTrackCaller) {
                        ret = m_pAudioTrackCaller->SetPlayoutDevice(pDevice);
                        CM_INFO_TRACE("[NATIVE] setAudioPlaybackDevie, ret="<<ret);
                    }
                    pDevice->Release();
                    break;
                }
                pDeviceTmp->Release();
            }
        }
    }
    else
    {
        CM_INFO_TRACE("[NATIVE] setAudioPlaybackDevice, create mediadevice enumerator failed ret = "<<ret);
        return false;
    }

    SAFE_RELEASE(mediaEngine);
    return ret ? false : true;
}

long Endpoint::GetAudioDeviceEnum(WmeDeviceInOutType type,std::vector<string>& inputdevice,std::vector<string>& outputdevice)
{
//    WmeDeviceInOutType devInOut;
    
    const int nMaxDeviceNum = 10;
	const int nMaxNameLen = 512;
	int nDeviceNum = 0;
	char szDeviceName[nMaxDeviceNum * nMaxNameLen];
	memset(szDeviceName, 0, sizeof(szDeviceName));
    
	IWmeMediaDevice * pDeviceItem[nMaxDeviceNum];
    
    IWmeMediaEngine* mediaEngine = NULL;
	WmeCreateMediaEngine(&mediaEngine);
    
    if(mediaEngine==NULL)
        return -1;
    
    IWmeMediaDeviceEnumerator * pWmeMediaDeviceEnumerator = NULL;
	mediaEngine->CreateMediaDeviceEnumerator(WmeMediaTypeAudio, type, &pWmeMediaDeviceEnumerator);
    
    if (pWmeMediaDeviceEnumerator)
    {
        WMERESULT res = WME_S_OK;
		int device_idx = 0;
		int32_t num = 0;
        //DeviceProperty dev_property;
		res = pWmeMediaDeviceEnumerator->GetDeviceNumber(num);
        
        if (WME_SUCCEEDED(res))
        {
			nDeviceNum = 0;
			while (num > 0 && device_idx < num && device_idx < nMaxDeviceNum)
            {
				IWmeMediaDevice * m_device = NULL;
				res = pWmeMediaDeviceEnumerator->GetDevice(device_idx, &m_device);
                
				if (WME_SUCCEEDED(res) && m_device)
                {                    
					int szLen = nMaxNameLen;
					res = m_device->GetFriendlyName(szDeviceName + nMaxNameLen * nDeviceNum, szLen);
					if (WME_SUCCEEDED(res))
                    {
						pDeviceItem[nDeviceNum] = m_device;
						nDeviceNum++;
					}
				}
                
				device_idx++;
			}
		}
        
        SAFE_RELEASE(pWmeMediaDeviceEnumerator);
    }
    
    for (int i = 0; i < nDeviceNum; i++)
    {
		char *p = szDeviceName + i * nMaxNameLen;
		printf("type is %d,Audio name is %s\n",type,p);
		std::string name = p;
        
        if (type == WmeDeviceIn)
            //m_vectorCaptureDevice.push_back(name);
            inputdevice.push_back(name);
        else if (type == WmeDeviceOut)
            //m_vectorPlayDevice.push_back(name);
            outputdevice.push_back(name);
            
		m_AudioDeviceList.insert(std::make_pair(name, pDeviceItem[i]));
		CM_INFO_TRACE("[NATIVE] getAudiodeviceList, pDeviceItem" << i << " = " << pDeviceItem[i]);
	}
    SAFE_RELEASE(mediaEngine);
    
    return 0;
}

std::map<std::string, IWmeMediaDevice *> Endpoint::getCameraList() {
	CM_INFO_TRACE("[NATIVE] getCameraList");

	map<string, IWmeMediaDevice *> cameraList; // Camera list to be returned
	const int nMaxDeviceNum = 10;
	const int nMaxNameLen = 512;
	int nDeviceNum = 0;
	char szDeviceName[nMaxDeviceNum * nMaxNameLen];
	memset(szDeviceName, 0, sizeof(szDeviceName));

	IWmeMediaDevice * pDeviceItem[nMaxDeviceNum];

	IWmeMediaEngine* mediaEngine = NULL;
	WmeCreateMediaEngine(&mediaEngine); // Create dummy media engine just to retrieve a camera list. TODO: Is this expensive?
    if(mediaEngine==NULL)return cameraList;
    
	IWmeMediaDeviceEnumerator * pWmeMediaDeviceEnumerator = NULL;
	mediaEngine->CreateMediaDeviceEnumerator(WmeMediaTypeVideo, WmeDeviceIn, &pWmeMediaDeviceEnumerator);
    
	if (pWmeMediaDeviceEnumerator) {
		WMERESULT res = WME_S_OK;
		int device_idx = 0;
		int32_t num = 0;
		res = pWmeMediaDeviceEnumerator->GetDeviceNumber(num);

		if (WME_SUCCEEDED(res)) {
			nDeviceNum = 0;
			while (num > 0 && device_idx < num && device_idx < nMaxDeviceNum) {
				IWmeMediaDevice * m_device = NULL;
				res = pWmeMediaDeviceEnumerator->GetDevice(device_idx, &m_device);

				if (WME_SUCCEEDED(res) && m_device) {

					int szLen = nMaxNameLen;
					res = m_device->GetFriendlyName(
							szDeviceName + nMaxNameLen * nDeviceNum, szLen);
					if (WME_SUCCEEDED(res)) {
						pDeviceItem[nDeviceNum] = m_device;
						nDeviceNum++;
					}
				}

				device_idx++;
			}
		}

		SAFE_RELEASE(pWmeMediaDeviceEnumerator);
	}

	for (int i = 0; i < nDeviceNum; i++) {
		char *p = szDeviceName + i * nMaxNameLen;
		printf("Camera %i %s\n", i, p);
		std::string name = p;
        
        IWmeVideoCapDevicePosition* pPosition = NULL;
        pDeviceItem[i]->QueryInterface(WMEIID_IWmeVideoCapDevicePosition, (void**)&pPosition);
        if (pPosition) // seems only available for ios/mac
        {
            WmeVideoCapDevicePosition pos = WME_VIDEO_CAP_DEVICE_POSITION_UNKNOWN;
            pPosition->GetPosition(&pos);
            pPosition->Release();
            pPosition = NULL;
            switch (pos) {
                case WME_VIDEO_CAP_DEVICE_POSITION_UNKNOWN:
                    break;
                case WME_VIDEO_CAP_DEVICE_POSITION_BACK:
                    name = "back";
                    break;
                case WME_VIDEO_CAP_DEVICE_POSITION_FRONT:
                    name = "front";
                    break;
                default:
                    break;
            }
        }
        
		cameraList.insert(std::make_pair(name, pDeviceItem[i]));
		CM_INFO_TRACE("[NATIVE] getCameraList, pDeviceItem" << i << " = " << pDeviceItem[i]);
	}
    SAFE_RELEASE(mediaEngine);
	return cameraList;
}

//sync status check
bool Endpoint::checkSyncStatus(const char* strResult, int32_t iRate)
{
    bool bCheckPass = false;
    
    if(m_pAudioTrackCallee)
    {
        unsigned int csiArray[8] = {0};
        unsigned int csiCount = 8;
        m_pAudioTrackCallee->GetCSI(csiArray, csiCount);
        if(csiCount != 1)
        {
            CM_INFO_TRACE("Endpoint::checkSyncStatus -- csiCount is not 1: csiCount = " << csiCount);
            return true;
        }
        uint32_t csi = csiArray[0];
     
        WmeSyncStatistics syncStats = {0};
        if (m_pMediaConn) {
            m_pMediaConn->GetSyncStatistics(csi, syncStats);
        }
        CM_INFO_TRACE("Endpoint::checkSyncStatus, get sync stats: bDecidedPlay = " << syncStats.bDecidedPlay << ", fDecideSyncPlayRatio = " << syncStats.fDecideSyncPlayRatio << ", fDecideDropRatio = " << syncStats.fDecideDropRatio << ", fDecideAheadPlayRatio = " << syncStats.fDecideAheadPlayRatio << ", fDecideNoSyncPlayRatio = " << syncStats.fDecideNoSyncPlayRatio << ", fDecideBufferRatio = " << syncStats.fDecideBufferRatio);
        
        float fCheckRatio = iRate / 100.0;
        float fStatsRatio = 0.0;
        float fTotalPlayRatio = syncStats.fDecideSyncPlayRatio + syncStats.fDecideDropRatio + syncStats.fDecideAheadPlayRatio + syncStats.fDecideNoSyncPlayRatio;
        if(fTotalPlayRatio > 0.0)
        {
            if(0 == strcmp("sync", strResult))
            {
                fStatsRatio = syncStats.fDecideSyncPlayRatio / fTotalPlayRatio;
            }
            else if(0 == strcmp("delay", strResult))
            {
                fStatsRatio = syncStats.fDecideDropRatio / fTotalPlayRatio;
            }
            else if(0 == strcmp("ahead", strResult))
            {
                fStatsRatio = syncStats.fDecideAheadPlayRatio / fTotalPlayRatio;
            }
            else if(0 == strcmp("nosync", strResult))
            {
                fStatsRatio = syncStats.fDecideNoSyncPlayRatio / fTotalPlayRatio;
            }
        }
        CM_INFO_TRACE("Endpoint::checkSyncStatus, check sync stats: strResult = " << strResult << ", fStatsRatio = " << fStatsRatio << ", fCheckRatio = " << fCheckRatio);
        if(fStatsRatio >= fCheckRatio)
        {
            bCheckPass = true;
        }
    }
    
    return bCheckPass;
}

// Calabash testing
Calabash * Endpoint::getCalabash() {
    return mCalabash;
}

WmeMediaStatus Endpoint::GetMediaStatus(WmeSessionType mediaType)
{
    WmeMediaStatus status = WmeMedia_Available;
    switch (mediaType) {
        case WmeSessionType_Audio:
            if(m_pAudioTrackCallee)
            {
                m_pAudioTrackCallee->GetMediaStatus(status);
            }
            break;
        case WmeSessionType_Video:
            if(m_pVideoTrackCallee[0])
            {
                m_pVideoTrackCallee[0]->GetMediaStatus(status);
            }
            break;
        case WmeSessionType_ScreenShare:
            if(m_pScreenTrackViewer)
            {
                m_pScreenTrackViewer->GetMediaStatus(status);
            }
            break;
        default:
            break;
    }
    return status;
}

void Endpoint::checkDirection(WmeSdpParsedInfo *arraySdp, int num)
{
    if(arraySdp)
    {
        for (int i = 0; i < num; i++) {
            switch (arraySdp[i].mediaType) {
                case WmeSessionType_Audio:
                    if(m_pAudioTrackCaller)
                    {
                        if((arraySdp[i].negotiatedRemoteDirection & WmeDirection_Recv) && !(m_audioRemoteDirection & WmeDirection_Recv)){
                            m_pAudioTrackCaller->Unmute();
                        }
                        else if(!(arraySdp[i].negotiatedRemoteDirection & WmeDirection_Recv) && (m_audioRemoteDirection & WmeDirection_Recv)){
                            m_pAudioTrackCaller->Mute();
                        }
                    }
                    m_audioRemoteDirection = arraySdp[i].negotiatedRemoteDirection;
                    break;
                case WmeSessionType_Video:
                    if(m_pVideoTrackCaller)
                    {
                        if((arraySdp[i].negotiatedRemoteDirection & WmeDirection_Recv) && !(m_videoRemoteDirection & WmeDirection_Recv)){
                            m_pVideoTrackCaller->Unmute();
                        }
                        else if(!(arraySdp[i].negotiatedRemoteDirection & WmeDirection_Recv) && (m_videoRemoteDirection & WmeDirection_Recv)){
                            m_pVideoTrackCaller->Mute();
                        }
                    }
                    m_videoRemoteDirection = arraySdp[i].negotiatedRemoteDirection;
                    break;
                default:
                    break;
            }
        }
    }
}

void Endpoint::ShowScreenSharePreview(void *winScreenPreview)
{
    // todo: rick xu, paramter to show preview
    m_winScreenPreview = winScreenPreview;
    
    if (m_pScreenTrackSharer)
    {
		if (winScreenPreview != NULL)
		{
#ifdef WP8
			m_pScreenTrackSharer->AddExternalRender((IWmeVideoRender*)winScreenPreview);
#else
			m_pScreenTrackSharer->AddRenderWindow(winScreenPreview);
			m_pScreenTrackSharer->SetRenderMode(TestConfig::Instance().m_eRenderModeScreenSharing);
#endif
		}
    }

}

void Endpoint::GetCpuUsage(WmeCpuUsage &cpuUsage)
{
	if (m_pMediaConn != NULL)
		m_pMediaConn->GetCPUStatistics(cpuUsage);
}

void Endpoint::GetMemoryUsage(WmeMemoryUsage &memoryUsage)
{
    if (m_pMediaConn != NULL)
        m_pMediaConn->GetMemoryStatistics(memoryUsage);
}

void Endpoint::GetNetworkMetrics(WmeAggregateNetworkMetricStats &stStat)
{
    if (m_pMediaConn != NULL)
        m_pMediaConn->GetNetworkMetrics(stStat);
}

int Endpoint::getCSICount()
{
    int count = 0;
    if(m_pAudioTrackCallee)
    {
        unsigned int csiArray[8] = {0};
        unsigned int csiCount = 8;
        m_pAudioTrackCallee->GetCSI(csiArray, csiCount);
        count = csiCount;
    }
    return count;
}

unsigned int Endpoint::getVideoCSI(WmeDirection direction)
{
    unsigned int csi = 0;
    unsigned int csiCount = 1;
    switch (direction) {
        case WmeDirection_Send:
            if(m_pVideoTrackCaller)
            {
                m_pVideoTrackCaller->GetCSI(&csi, csiCount);
            }
            break;
        case WmeDirection_Recv:
        {
            IWmeMediaTrackBase* pActive = FindTrackByVid(0);
            if(pActive) {
                pActive->GetCSI(&csi, csiCount);
                WmeMediaStatus mediaStatus = WmeMedia_Available;
                pActive->GetMediaStatus(mediaStatus);
                if(mediaStatus != WmeMedia_Available)
                    csi = m_possibleActiveCsi;
            }
        }
            break;
        default:
            break;
    }
    return csi;
}

void Endpoint::OnAvailableMediaChanged(unsigned long mid, int nCount)
{
    CM_INFO_TRACE_THIS("Endpoint::OnAvailableMediaChanged, mid=" << mid << "ncount=" << nCount);
}


void Endpoint::subscribeVideo(WmeVideoQualityType type, bool newTrack, WmeSubscribePolicy policy, unsigned int csi)
{
    if(newTrack)
    {
        if(m_pMediaConn)
        {
            int vid = -1;
            m_pMediaConn->Subscribe(VIDEO_MID, policy, type, vid, 255, csi);
        }
    }
    else
    {
        if(m_pVideoTrackCallee[0])
        {
            m_pVideoTrackCallee[0]->RequestVideo(type);
        }
    }
}

void Endpoint::unsubscribeVideo(unsigned int vid)
{
    if(m_pMediaConn)
    {
        m_pMediaConn->Unsubscribe(VIDEO_MID, vid);
    }
}

void Endpoint::StartExternalTrans()
{
    if(TestConfig::i().m_bTrain && m_pMediaConn)
    {
        if(TestConfig::i().m_bPlayback) {
            m_externalAudio.StartListen((int)m_audioMid, "0.0.0.0", TestConfig::i().m_nAudioPort);
            m_externalVideo.StartListen((int)m_videoMid, "0.0.0.0", TestConfig::i().m_nVideoPort);
        } else {
            WmeAudioConnectionStatistics audio = {0};
            m_pMediaConn->GetAudioStatistics(m_audioMid, audio);
            m_externalAudio.StartListen((int)m_audioMid, audio.connStat.localIp, audio.connStat.uLocalPort);

            WmeVideoConnectionStatistics video = {0};
            m_pMediaConn->GetVideoStatistics(m_videoMid, video);
            m_externalVideo.StartListen((int)m_videoMid, video.connStat.localIp, video.connStat.uLocalPort);
        }
    }
}

void Endpoint::ConnectExternalTrans()
{
    if(TestConfig::i().m_bTrain && m_pMediaConn)
    {
        WmeAudioConnectionStatistics audio = {0};
        m_pMediaConn->GetAudioStatistics(m_audioMid, audio);
        m_externalAudio.Connect(audio.connStat.remoteIp, audio.connStat.uRemotePort);
        
        WmeVideoConnectionStatistics video = {0};
        m_pMediaConn->GetVideoStatistics(m_videoMid, video);
        m_externalVideo.Connect(video.connStat.remoteIp, video.connStat.uRemotePort);
    }
}

void Endpoint::DisconnectExternalTrans()
{
    if(TestConfig::i().m_bTrain)
    {
        m_externalAudio.Disconnect();
        m_externalVideo.Disconnect();
    }
}

void Endpoint::SendEvent(const string &key, const string &value)
{
    if (key == "OnAudioTimeStampChanged") {
        if (m_pMediaConn) {
            Json::Reader r;
            Json::Value root;
            r.parse(value, root, true);
            uint32_t csi = root["csi"].asUInt();
            uint32_t timestamp = root["timestamp"].asUInt();
            m_pMediaConn->SyncWithAudioTimestamp(csi, timestamp);
        }
    }
}

void Endpoint::SetAutoSwitchDefaultMic(bool bFlag)
{
    if (m_pMediaConn)
        m_pMediaConn->GetAudioConfig(m_audioMid)->SetAutoSwitchDefaultMicrophoneDeviceFlag(bFlag);
}

void Endpoint::SetAutoSwitchDefaultSpeaker(bool bFlag)
{
    if (m_pMediaConn)
        m_pMediaConn->GetAudioConfig(m_audioMid)->SetAutoSwitchDefaultSpeakerDeviceFlag(bFlag);
}

//CTaIWmeStunTraceSink calss methods for TA
CTaIWmeStunTraceSink::CTaIWmeStunTraceSink()
{
    stunTraceRet = -1;
}

CTaIWmeStunTraceSink* CTaIWmeStunTraceSink::GetInstance()
{
    static CTaIWmeStunTraceSink instance;
    return &instance;
}

WMERESULT CTaIWmeStunTraceSink::OnResult(WmeStunTraceResult reason, const char* szDetail)
{
    stunTraceRet = reason;
    jsonRet = szDetail;
    return WME_S_OK;
}

//CTaIWmeTraceServerSink calss methods for TA
CTaIWmeTraceServerSink::CTaIWmeTraceServerSink()
{
    traceRet = -1;
}

CTaIWmeTraceServerSink* CTaIWmeTraceServerSink::GetInstance()
{
    static CTaIWmeTraceServerSink instance;
    return &instance;
}

WMERESULT CTaIWmeTraceServerSink::OnTraceServerResult(WmeStunTraceResult reason, const char* szDetail)
{
    traceRet = reason;
    jsonRet = szDetail;
    CM_INFO_TRACE("StunPing, result:" << szDetail);
    return WME_S_OK;
}
