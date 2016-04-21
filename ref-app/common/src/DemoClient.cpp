#include "DemoClient.h"
#include "DemoParameters.h"
#include "CmThreadManager.h"
#include "CmThread.h"
#ifdef CUCUMBER_ENABLE
#include "pesq.h"
#endif

#undef min
#undef max

#ifdef ANDROID
#include "wmenative-jni.h"
#endif //ANDROID

#if defined(WIN32) || defined(MACOS) || defined(ANDROID)
#define ENABLE_SCREEN_SHARE 1
#endif

#define SCREEN_SHARE_USE_TCP

const WmeCodecPayloadTypeMap kAudioCodecMap[] = 
{
    {WmeCodecType_G711_ULAW, 0, 8000},
    {WmeCodecType_G711_ALAW, 8, 8000},
    {WmeCodecType_ILBC, 97, 8000},
    {WmeCodecType_OPUS, 101, 48000},
    {WmeCodecType_G722, 9, 16000},
    {WmeCodecType_CNG, WmeCodecType_CNG, 0}
};

const WmeCodecPayloadTypeMap kVideoCodecMap[] = 
{
    {WmeCodecType_AVC, 97, 90000},
    //{WmeCodecType_SVC, 98, 90000},
    {WmeCodecType_SVC, 101, 90000},
    {WmeCodecType_HEVC, 105, 90000},
    {WmeCodecType_VP8, 113, 90000}
};

uint8_t mapToPayloadType(WmeCodecType codec_type)
{
	//search video 
	int videoCodecMapLen = sizeof(kVideoCodecMap) / sizeof (WmeCodecPayloadTypeMap);
    for (int k=0; k < videoCodecMapLen; k++) {
		if (kVideoCodecMap[k].eCodecType == codec_type)
		{
			return kVideoCodecMap[k].uPayloadType;
		}
    }

	//search audio
	int audioCodecMapLen = sizeof(kAudioCodecMap) / sizeof (WmeCodecPayloadTypeMap);
    for (int k=0; k < audioCodecMapLen; k++) {
		if (kAudioCodecMap[k].eCodecType == codec_type)
		{
			return kAudioCodecMap[k].uPayloadType;
		}
    }

	return 0;
}

WmeCodecType  mapToCodecType( uint8_t uPayload, bool isAudio)
{
	//search video 
    if(!isAudio)
    {
	    int videoCodecMapLen = sizeof(kVideoCodecMap) / sizeof (WmeCodecPayloadTypeMap);
        for (int k=0; k < videoCodecMapLen; k++) {
		    if (kVideoCodecMap[k].uPayloadType  == uPayload)
		    {
			    return kVideoCodecMap[k].eCodecType;
		    }
        }
    }
    else
    {
        	//search audio
	    int audioCodecMapLen = sizeof(kAudioCodecMap) / sizeof (WmeCodecPayloadTypeMap);
        for (int k=0; k < audioCodecMapLen; k++) {
		    if (kAudioCodecMap[k].uPayloadType == uPayload)
		    {
			    return kAudioCodecMap[k].eCodecType;
		    }
        }
    }
	return WmeCodecType_Unknown;
}

#if 0
static const char pRTPExtension_Time[] = "Remote Time";
static const int nRTPExtension_Time = 118;
int32_t AddTimeToRTPExtension(const char* sExtURI, void* pData, uint8_t pExtBuffer[], uint32_t &uLength)
{
	if(!sExtURI)
	{
		return WME_E_FAIL;
	}
    
	if(strcmp(sExtURI, pRTPExtension_Time) != 0)
	{
		return WME_E_FAIL;
	}
    
    struct timeval current;
    gettimeofday(&current, NULL);
	if(uLength >= sizeof(current))
	{
		memcpy_s(pExtBuffer, uLength, &current, sizeof(current));
		uLength = sizeof(current);
		return WME_S_OK;
	}
    
	return WME_E_FAIL;
}

static unsigned int g_nCounter = 0;
int32_t GetTimeFromRTPExtension(const char* sExtURI, uint8_t pExtBuffer[], uint32_t uLength, void* pData)
{
    
	if(!sExtURI)
	{
		return WME_E_FAIL;
	}
    
	if(strcmp(sExtURI, pRTPExtension_Time) != 0)
	{
		return WME_E_FAIL;
	}
    
	struct timeval current;
    gettimeofday(&current, NULL);
    
	if(uLength >= sizeof(current))
	{
		memcpy_s(&current, sizeof(current), pExtBuffer, sizeof(current));
		uLength = sizeof(current);
        
		if(g_nCounter%1000 == 0)
		{
			CM_INFO_TRACE("GetTimeFromRTPExtension, remote time is "
                          << current.tv_sec << "-"
                          << current.tv_usec);
		}
        
		g_nCounter++;
		return WME_S_OK;
	}
    
	return WME_E_FAIL;
}
#endif

//
// class DemoClient
//

DemoClient::DemoClient(backUISink * pBackUISink)
{
    /// global for video and audio
	m_pWmeEngine				= NULL;
	m_pWmeSyncBox				= NULL;
    m_pNetworkIndicator         = NULL;
    m_pBackUISink				= pBackUISink;

    /// for audio sender and receiver
    m_pAudioSender              = NULL;
    m_pAudioReceiver            = NULL;
    m_pAudioTransport           = NULL;
    m_pAudioManager             = NULL;

    m_pWmeAudioSession          = NULL;
	m_bStartAudioSending        = false;
    m_bEnableMyAudio            = true;
    m_uAudioPorts[0]            = DEMO_P2P_AUDIO_DATA_PORT;
    m_uAudioPorts[1]            = DEMO_P2P_AUDIO_CRTL_PORT;
    m_AudioCodecType            = WmeCodecType_OPUS;    ///default OPUS

    /// for video sender and receiver
    m_pVideoPreview             = NULL;
    m_pVideoSender              = NULL;
    m_pVideoReceiver            = NULL;
    m_pVideoTransport           = NULL;
    m_pVideoManager             = NULL;

    m_pWmeVideoSession          = NULL;
    m_bStartVideoSending        = false;
    m_bEnableMyVideo            = true;
    m_uVideoPorts[0]            = DEMO_P2P_VIDEO_DATA_PORT;
    m_uVideoPorts[1]            = DEMO_P2P_VIDEO_CRTL_PORT;
	m_VideoCodecType            = WmeCodecType_SVC;	///default SVC
	m_bEnableQoS				= false;

    /// for DeskShare sender and receiver // whsu
//  m_pDeskSharePreview         = NULL;
    m_pDeskShareSender          = NULL;
    m_pDeskShareReceiver        = NULL;
    m_pDeskShareTransport       = NULL;
    m_pShareSourceManager       = NULL;
	m_uDeskSharePorts[0]        = DEMO_P2P_SCREEN_VIDEO_DATA_PORT; // whsu 2
    m_uDeskSharePorts[1]        = DEMO_P2P_SCREEN_VIDEO_CRTL_PORT;

    m_pWmeDeskShareSession      = NULL;
    m_bStartDeskShareSending    = false;
    m_bEnableMyDeskShare        = true;

	m_strVideoInputFileName = "";
	memset(&m_sVideoRawFormat, 0, sizeof(m_sVideoRawFormat));
	m_sVideoRawFormat.eRawType = WmeVideoUnknown;
	m_bUseVideoInputFile = FALSE;
    m_strVideoOutputFileName = "";
    m_bUseVideoOutputFile = FALSE;
    
	m_bEnableSendingFilterFeedback = true;

    m_strAudioInputFileName = "";
    memset(&m_sAudioRawFormat, 0, sizeof(WmeAudioRawFormat));
    m_bUseAudioInputFile = FALSE;
    
    m_bUseAudioOutputFile = FALSE;
    m_strAudioOuputFileName = "";
}

DemoClient::~DemoClient()
{
}

void DemoClient::SetUISink(backUISink * pBackUISink)
{
	m_pBackUISink = pBackUISink;
}

long DemoClient::Init(WmeTraceLevel level)
{
	long retData = WME_S_OK;
    
    CmUtilInit();
    
#ifdef WIN32
	CCmThreadManager theThreadManager;
	theThreadManager.InitMainThread(0, NULL);
#endif

    /// init WME
	retData = WmeInit(0);
	if (WME_S_OK != retData)
	{
		CM_ERROR_TRACE_THIS("DemoClient::Init, WmeInit failed!");
		return WME_S_FALSE;
	}

    /// for trace level setting
    //SetTraceMaxLevel(level);

#ifdef ANDROID
	//hard code for DEMO
	char path[] = "/data/data/com.cisco.wmeAndroid/lib";
	WmeSetMediaEngineOption(WmeOption_WorkPath, (void*)path, strlen(path));
	WmeTraceLevel trace_level = WME_TRACE_LEVEL_DETAIL;
	WmeSetMediaEngineOption(WmeOption_TraceMaxLevel, &trace_level, sizeof(trace_level));
#endif

// whsu 2
	//char path[] = "./";
	//WmeSetMediaEngineOption(WmeOption_DataDumpPath, (void*)path, strlen(path));
	//bool bDataDumped = true;
	//WmeSetMediaEngineOption(WmeOption_DataDumpFlag, &bDataDumped, sizeof(bDataDumped));

    /// for media engine
	WmeCreateMediaEngine(&m_pWmeEngine);
	if (NULL == m_pWmeEngine)
	{
		CM_ERROR_TRACE_THIS("DemoClient::Init, failed to create wme engine!");
		return WME_S_FALSE;
	}

    /// for media syncbox
	retData = m_pWmeEngine->CreateMediaSyncBox(&m_pWmeSyncBox);
	if(WME_S_OK != retData)
	{
		CM_ERROR_TRACE_THIS("DemoClient::Init, create MediaSyncBox failed! error = " << retData << ", ");
		return WME_S_FALSE;
	}
    
    retData = m_pWmeEngine->CreateNetworkIndicator(&m_pNetworkIndicator);
    if(WME_S_OK != retData)
	{
		CM_ERROR_TRACE_THIS("DemoClient::Init, create NeworkIndicator failed! error = " << retData << ", ");
		return WME_S_FALSE;
	}

	/// for audio session
	if(WME_S_OK != CreateAudioSession())
	{
		CM_ERROR_TRACE_THIS("DemoClient::Init, create audio session failed!");
		return WME_S_FALSE;
	}
    
    /// for video session
	if(WME_S_OK != CreateVideoSession())
	{
		CM_ERROR_TRACE_THIS("DemoClient::Init, create video session failed!");
		return WME_S_FALSE;
	}

    /// for audio manager
    if (CreateAudioManager() != WME_S_OK) {
		CM_ERROR_TRACE_THIS("DemoClient::Init, fail to CreateAudioManager");
        return WME_S_FALSE;
    }
    
    /// for video manager
    if (CreateVideoManager() != WME_S_OK) {
		CM_ERROR_TRACE_THIS("DemoClient::Init, fail to CreateVideoManager");
        return WME_S_FALSE;
    }
    
#ifdef ENABLE_SCREEN_SHARE

	/// for DeskShare session // whsu
	if(WME_S_OK != CreateDeskShareSession())
	{
		CM_ERROR_TRACE_THIS("DemoClient::Init, create DS session failed!");
		return WME_S_FALSE;
	}
#ifndef ANDROID
    /// for DeskShare manager
    if (CreateDeskShareSourceManager() != WME_S_OK) {
		CM_ERROR_TRACE_THIS("DemoClient::Init, fail to CreateVideoManager");
        return WME_S_FALSE;
    }
#endif
    
#endif

	return WME_S_OK;
}

long DemoClient::UnInit()
{
	long retData = WME_S_OK;

	DeleteMediaClient(DEMO_MEDIA_AUDIO);
    DeleteMediaClient(DEMO_MEDIA_VIDEO);
	DeleteMediaClient(DEMO_MEDIA_DESKSHARE); // whsu

	DeleteAudioSession();
	DeleteVideoSession();
	DeleteDeskShareSession(); // whsu

    DeleteAudioManager();
    DeleteVideoManager();
	DeleteDeskShareSourceManager(); // whsu

    SAFE_RELEASE(m_pWmeSyncBox);
	SAFE_RELEASE(m_pWmeEngine);
    
	retData = WmeUninit();
	if (WME_S_OK != retData)
	{
		CM_INFO_TRACE_THIS("Error in WmeUninit");
        retData = WME_S_FALSE;
	}

	m_pBackUISink = NULL;
    
    CmUtilCleanup();
    
	return retData;
}

#if defined(ANDROID) || defined(IOS)
void DemoClient::HeartBeat()
{
    ACmThread *pMain = CCmThreadManager::Instance()->GetThread(TT_MAIN);
    if(pMain != NULL)
    {
        pMain->OnThreadRun();
    }
}

void DemoClient::InitTPThread()
{
    CCmThreadManager theThreadManager;
    theThreadManager.InitMainThread(0, NULL);
}
#endif

void DemoClient::SetTraceMaxLevel(WmeTraceLevel level)
{
    /// for trace level setting
    if(level < WME_TRACE_LEVEL_NOTRACE)
        level = WME_TRACE_LEVEL_NOTRACE;
    else if(level >  WME_TRACE_LEVEL_ALL)
        level = WME_TRACE_LEVEL_ALL; 
    WmeSetTraceMaxLevel(level);
}

void DemoClient::SetDumpDataEnabled(unsigned int uDataDumpFlag)
{
    WmeSetMediaEngineOption(WmeOption_DataDumpFlag, (void *)&uDataDumpFlag, sizeof(unsigned int));
}

void DemoClient::SetDumpDataPath(const char *path)
{
    WmeSetMediaEngineOption(WmeOption_DataDumpPath, (void *)path, strlen(path));
}


long DemoClient::StartMediaSending(DEMO_MEDIA_TYPE mtype)
{
	CM_INFO_TRACE_THIS("DemoClient::StartMediaSending, begin mtype="<<mtype);

    AutoLock theGuard(m_Mutex);

    if (mtype == DEMO_MEDIA_VIDEO)
    {
        if (m_bStartVideoSending)
            return WME_S_FALSE;

        if(!m_bEnableMyVideo)
        {
            return WME_S_OK;
        }

        if (!m_pVideoSender) {
            return WME_S_FALSE;
        }
            
        m_pVideoSender->SetOptionForSendingData(true);
        m_bStartVideoSending = true;
    }
	else if (mtype == DEMO_MEDIA_DESKSHARE) // whsu
    {
        if (m_bStartDeskShareSending)
            return WME_S_FALSE;

        if(!m_bEnableMyDeskShare)
        {
            return WME_S_OK;
        }

        if (!m_pDeskShareSender) {
            return WME_S_FALSE;
        }
            
        m_pDeskShareSender->SetOptionForSendingData(true);
        m_bStartDeskShareSending = true;
    }
    else
    {
    	if (m_bStartAudioSending)
            return WME_S_FALSE;
        
        if(!m_bEnableMyAudio)
        {
            return WME_S_OK;
        }
        
        if (!m_pAudioSender) {
            return WME_S_FALSE;
        }

        m_pAudioSender->SetOptionForSendingData(true);
        m_bStartAudioSending = true;
    }

	return WME_S_OK;

}

bool DemoClient::IsMediaSending(DEMO_MEDIA_TYPE mtype)
{
    switch (mtype) {
        case DEMO_MEDIA_VIDEO:
            return m_bStartVideoSending;
            break;
        case DEMO_MEDIA_AUDIO:
            return m_bStartAudioSending;
            break;
        case DEMO_MEDIA_DESKSHARE:
            return m_bStartDeskShareSending;
        default:
            break;
    }
    return false;
}


long DemoClient::StopMediaSending(DEMO_MEDIA_TYPE mtype)
{
    AutoLock theGuard(m_Mutex);

    if (mtype == DEMO_MEDIA_VIDEO)
    {
        if (!m_bStartVideoSending)
            return WME_S_FALSE;

        if (!m_pVideoSender) {
            return WME_S_FALSE;
		}
        
        m_pVideoSender->SetOptionForSendingData(false);
		m_bStartVideoSending = false;
    }
    else if (mtype == DEMO_MEDIA_DESKSHARE)  //whsu
    {
        if (!m_bStartDeskShareSending)
            return WME_S_FALSE;

        if (!m_pDeskShareSender) {
            return WME_S_FALSE;
		}
        
        m_pDeskShareSender->SetOptionForSendingData(false);
		m_bStartDeskShareSending = false;
    }
    else if (mtype == DEMO_MEDIA_AUDIO)
    {
    	if (!m_bStartAudioSending)
            return WME_S_FALSE;
        
        if (!m_pAudioSender) {
            return WME_S_FALSE;
        }
        
        m_pAudioSender->SetOptionForSendingData(false);
        m_bStartAudioSending = false;
    }

	return WME_S_OK;
}

long DemoClient::StopRenderView(DEMO_TRACK_TYPE ttype)
{
    AutoLock theGuard(m_Mutex);

    long ret = WME_E_FAIL;
    VideoSettings *pSettings = GetVideoSettings(ttype);
	if (pSettings) {
        ret = pSettings->RemoveRenderer();
    }
    
	return ret;
}

//long DemoClient::StopDeskShareRenderView(DEMO_TRACK_TYPE ttype) // whsu
//{
//    AutoLock theGuard(m_Mutex);
//
//    long ret = WME_E_FAIL;
//    VideoSettings *pSettings = GetDeskShareSettings(ttype);
//	if (pSettings) {
//        ret = pSettings->RemoveRenderer();
//    }
//    
//	return ret;
//}


//////////////////////////////////////////////////////////////////////////
// call from high UI

bool DemoClient::GetMediaPorts(DEMO_MEDIA_TYPE mtype, unsigned short &uRtpPort, unsigned short &uRtcpPort)
{
    if (DEMO_MEDIA_AUDIO == mtype) {
        uRtpPort = m_uAudioPorts[0];
        uRtcpPort = m_uAudioPorts[1];
    }else if(DEMO_MEDIA_VIDEO == mtype) {
        uRtpPort = m_uVideoPorts[0];
        uRtcpPort = m_uVideoPorts[1];
	}else if(DEMO_MEDIA_DESKSHARE == mtype){ // whsu 2
		uRtpPort = m_uDeskSharePorts[0];
        uRtcpPort = m_uDeskSharePorts[1];
    }
    return true;
}

long DemoClient::InitHost(DEMO_MEDIA_TYPE mtype)
{
    AutoLock theGuard(m_Mutex);

	if (DEMO_MEDIA_AUDIO == mtype)
	{
        InitHost(mtype, m_uAudioPorts[0], m_uAudioPorts[1]);
	}
	else if (DEMO_MEDIA_VIDEO == mtype)
	{
        InitHost(mtype, m_uVideoPorts[0], m_uVideoPorts[1]);
	}
	else if (DEMO_MEDIA_DESKSHARE == mtype) //whsu 2
	{
		InitHost(mtype, m_uDeskSharePorts[0], m_uDeskSharePorts[1]);
	}

	return WME_S_OK;
}

long DemoClient::InitHost(DEMO_MEDIA_TYPE mtype, unsigned short uRtpPort, unsigned short uRtcpPort)
{
    AutoLock theGuard(m_Mutex);

    if (DEMO_MEDIA_AUDIO == mtype)
    {
        CreateAudioTransport(m_pAudioTransport);
        returnv_if_fail(m_pAudioTransport != NULL, WME_E_FAIL);

        if (m_pAudioReceiver)
            m_pAudioTransport->SetReceiver(m_pAudioReceiver);
        if (m_pAudioSender)
            m_pAudioSender->SetTransport(m_pAudioTransport);
        m_pAudioTransport->InitHost(TRUE, uRtpPort, CONN_UDP, uRtcpPort, CONN_UDP);
    }
    else if (DEMO_MEDIA_VIDEO == mtype)
    {
        CreateVideoTransport(m_pVideoTransport);
        returnv_if_fail(m_pVideoTransport != NULL, WME_E_FAIL);

        if (m_pVideoReceiver)
            m_pVideoTransport->SetReceiver(m_pVideoReceiver);
        if (m_pVideoSender)
            m_pVideoSender->SetTransport(m_pVideoTransport);
        m_pVideoTransport->InitHost(TRUE, uRtpPort, CONN_UDP, uRtcpPort, CONN_UDP);
    }
	else if (DEMO_MEDIA_DESKSHARE == mtype) // whsu
    {
        CreateDeskShareTransport();
        returnv_if_fail(m_pDeskShareTransport != NULL, WME_E_FAIL);

        if (m_pDeskShareReceiver)
            m_pDeskShareTransport->SetReceiver(m_pDeskShareReceiver);
        if (m_pDeskShareSender)
            m_pDeskShareSender->SetTransport(m_pDeskShareTransport);
#if defined(SCREEN_SHARE_USE_TCP)
        m_pDeskShareTransport->InitHost(TRUE, uRtpPort, CONN_TCP, uRtcpPort, CONN_TCP);
#else
        m_pDeskShareTransport->InitHost(TRUE, uRtpPort, CONN_UDP, uRtcpPort, CONN_UDP);
#endif
    }

    return WME_S_OK;
}


long DemoClient::InitHost(DEMO_MEDIA_TYPE mtype, const char* pMyName, const char *szJingleIP, 
		int nJinglePort, const char *szStunIP, int nStunPort)
{
#if 0
    AutoLock theGuard(m_Mutex);

	if (DEMO_MEDIA_AUDIO == mtype)
    {
        returnv_if_fail(m_pAudioTransport != NULL, WME_E_FAIL);
        m_pAudioTransport->InitHost(pMyName, szJingleIP, nJinglePort, szStunIP, nStunPort);
    }
	else if (DEMO_MEDIA_VIDEO == mtype)
    {
        returnv_if_fail(m_pVideoTransport != NULL, WME_E_FAIL);
        m_pVideoTransport->InitHost(pMyName, szJingleIP, nJinglePort, szStunIP, nStunPort);
    }
#endif
	return WME_S_OK;
}

long DemoClient::ConnectRemote(DEMO_MEDIA_TYPE mtype, char *pIPAddress)
{
    AutoLock theGuard(m_Mutex);

	if (DEMO_MEDIA_AUDIO == mtype)
	{
        ConnectRemote(mtype, pIPAddress, m_uAudioPorts[0], m_uAudioPorts[1]);
	}
	else if (DEMO_MEDIA_VIDEO == mtype)
	{
        ConnectRemote(mtype, pIPAddress, m_uVideoPorts[0], m_uVideoPorts[1]);
	}
	else if (DEMO_MEDIA_DESKSHARE == mtype) // whsu 2
	{
		ConnectRemote(mtype, pIPAddress, m_uDeskSharePorts[0], m_uDeskSharePorts[1]);
        //returnv_if_fail(m_pDeskShareTransport != NULL, WME_E_FAIL);
		//m_pDeskShareTransport->ConnectRemote(pIPAddress, DEMO_P2P_VIDEO_DATA_PORT, CONN_UDP, DEMO_P2P_VIDEO_CRTL_PORT, CONN_UDP);
	}


	return WME_S_OK;
}

long DemoClient::ConnectRemote(DEMO_MEDIA_TYPE mtype, char *pIPAddress, unsigned short uRtpPort, unsigned short uRtcpPort)
{
    AutoLock theGuard(m_Mutex);

	if (DEMO_MEDIA_AUDIO == mtype)
	{
		CreateAudioTransport(m_pAudioTransport);
        returnv_if_fail(m_pAudioTransport != NULL, WME_E_FAIL);
		
		if (m_pAudioReceiver)
			m_pAudioTransport->SetReceiver(m_pAudioReceiver);
		if (m_pAudioSender)
			m_pAudioSender->SetTransport(m_pAudioTransport);
		m_pAudioTransport->ConnectRemote(pIPAddress, uRtpPort, CONN_UDP, uRtcpPort, CONN_UDP);
	}
	else if (DEMO_MEDIA_VIDEO == mtype)
	{
		CreateVideoTransport(m_pVideoTransport);
        returnv_if_fail(m_pVideoTransport != NULL, WME_E_FAIL);

		if (m_pVideoReceiver)
			m_pVideoTransport->SetReceiver(m_pVideoReceiver);
		if (m_pVideoSender)
			m_pVideoSender->SetTransport(m_pVideoTransport);
		m_pVideoTransport->ConnectRemote(pIPAddress, uRtpPort, CONN_UDP, uRtcpPort, CONN_UDP);
	}
	// whsu 2
	else if (DEMO_MEDIA_DESKSHARE == mtype)
	{
		CreateDeskShareTransport();
        returnv_if_fail(m_pDeskShareTransport != NULL, WME_E_FAIL);

		if (m_pDeskShareReceiver)
			m_pDeskShareTransport->SetReceiver(m_pDeskShareReceiver);
		if (m_pDeskShareSender)
			m_pDeskShareSender->SetTransport(m_pDeskShareTransport);
        
#if defined(SCREEN_SHARE_USE_TCP)
		m_pDeskShareTransport->ConnectRemote(pIPAddress, uRtpPort, CONN_TCP , uRtcpPort, CONN_TCP);
#else
        m_pDeskShareTransport->ConnectRemote(pIPAddress, uRtpPort, CONN_UDP , uRtcpPort, CONN_UDP);
#endif
	}

	return WME_S_OK;
}

long DemoClient::ConnectRemote(DEMO_MEDIA_TYPE mtype, const char* pMyName, const char * pHostName, const char *szJingleIP,
		int nJinglePort, const char *szStunIP, int nStunPort)
{
#if 0
    AutoLock theGuard(m_Mutex);

	if (DEMO_MEDIA_AUDIO == mtype)
    {
        returnv_if_fail(m_pAudioTransport != NULL, WME_E_FAIL);
        m_pAudioTransport->ConnectRemote(pMyName, pHostName, szJingleIP, nJinglePort, szStunIP, nStunPort);
    }
	else if (DEMO_MEDIA_VIDEO == mtype)
    {
        returnv_if_fail(m_pVideoTransport != NULL, WME_E_FAIL);
        m_pVideoTransport->ConnectRemote(pMyName, pHostName, szJingleIP, nJinglePort, szStunIP, nStunPort);
    }
	else if (DEMO_MEDIA_DESKSHARE == mtype) // whsu
    {
        returnv_if_fail(m_pDeskShareTransport != NULL, WME_E_FAIL);
        m_pDeskShareTransport->ConnectRemote(pMyName, pHostName, szJingleIP, nJinglePort, szStunIP, nStunPort);
    }
#endif
	return WME_S_OK;
}

long DemoClient::ConnectFile(DEMO_MEDIA_TYPE eMediaTyep, const char *fileName, const char *sourceIP, unsigned short sourcePort, const char *destinationIP, unsigned short destinationPort)
{
    AutoLock theGuard(m_Mutex);
    
    if (DEMO_MEDIA_AUDIO == eMediaTyep)
	{
		CreateAudioTransport(m_pAudioTransport);
        returnv_if_fail(m_pAudioTransport != NULL, WME_E_FAIL);
		
		if (m_pAudioReceiver)
			m_pAudioTransport->SetReceiver(m_pAudioReceiver);
		if (m_pAudioSender)
			m_pAudioSender->SetTransport(m_pAudioTransport);
		m_pAudioTransport->ConnectFile(fileName, sourceIP, sourcePort, destinationIP, destinationPort);
	}
	else if (DEMO_MEDIA_VIDEO == eMediaTyep)
	{
		CreateVideoTransport(m_pVideoTransport);
        returnv_if_fail(m_pVideoTransport != NULL, WME_E_FAIL);
        
		if (m_pVideoReceiver)
			m_pVideoTransport->SetReceiver(m_pVideoReceiver);
		if (m_pVideoSender)
			m_pVideoSender->SetTransport(m_pVideoTransport);
		m_pVideoTransport->ConnectFile(fileName, sourceIP, sourcePort, destinationIP, destinationPort);
	}
	else if (DEMO_MEDIA_DESKSHARE == eMediaTyep)
	{
		CreateDeskShareTransport();
        returnv_if_fail(m_pDeskShareTransport != NULL, WME_E_FAIL);
        
		if (m_pDeskShareReceiver)
			m_pDeskShareTransport->SetReceiver(m_pDeskShareReceiver);
		if (m_pDeskShareSender)
			m_pDeskShareSender->SetTransport(m_pDeskShareTransport);
        
		m_pDeskShareTransport->ConnectFile(fileName, sourceIP, sourcePort, destinationIP, destinationPort);
	}
    
	return WME_S_OK;
}

long DemoClient::DisConnect(DEMO_MEDIA_TYPE mtype)
{
    AutoLock theGuard(m_Mutex);

    CM_INFO_TRACE_THIS("DemoClient::DisConnect, begin mtype="<<mtype);
    if (DEMO_MEDIA_AUDIO == mtype)
    {
        m_bStartAudioSending = false;
		DeleteAudioTransport();
    }
	else if (DEMO_MEDIA_VIDEO == mtype)
    {
        m_bStartVideoSending = false;
		DeleteVideoTransport();
    }
	else if (DEMO_MEDIA_DESKSHARE == mtype) // whsu
    {
        m_bStartDeskShareSending = false;
		DeleteDeskShareTransport();
        //if (m_pDeskShareTransport)
        //    m_pDeskShareTransport->StopTransport();
    }

	return WME_S_OK;
}

long DemoClient::SetRenderView(DEMO_TRACK_TYPE ttype, void * handle, bool windowless)
{
    AutoLock theGuard(m_Mutex);

    long ret = WME_E_FAIL;
    VideoSettings *pSettings = GetVideoSettings(ttype);
	if (pSettings) {
        if( handle )
            ret = pSettings->SetRenderer(handle, windowless);
        else
            ret = pSettings->RemoveRenderer();
    }
    return ret;
}

//long DemoClient::SetDeskShareRenderView(DEMO_TRACK_TYPE ttype, void * handle, bool windowless) // whsu
//{
//    AutoLock theGuard(m_Mutex);
//
//    long ret = WME_E_FAIL;
//    VideoSettings *pSettings = GetVideoSettings(ttype);//GetDeskShareSettings(ttype);
//	if (pSettings) {
//        ret = pSettings->SetRenderer(handle, windowless);
//    }
//    return ret;
//}

long DemoClient::SetRenderAspectRatioSameWithSource(DEMO_TRACK_TYPE ttype, bool keepsame)
{
    AutoLock theGuard(m_Mutex);

    long ret = WME_E_FAIL;
    VideoSettings *pSettings = GetVideoSettings(ttype);
	if (pSettings) {
		ret = pSettings->SetRenderAspectRatioSameWithSource(keepsame);
	}
    return ret;
}

long DemoClient::SetRenderMode(DEMO_TRACK_TYPE ttype, WmeTrackRenderScalingModeType eMode)
{
    AutoLock theGuard(m_Mutex);

    long ret = WME_E_FAIL;
    VideoSettings *pSettings = GetVideoSettings(ttype);
	if (pSettings) {
		ret = pSettings->SetRenderMode(eMode);
	}

    return ret;
}

long DemoClient::OnRenderWindowPositionChanged(DEMO_TRACK_TYPE ttype, void * render)
{
    AutoLock theGuard(m_Mutex);

    long ret = WME_E_FAIL;
    VideoSettings *pSettings = GetVideoSettings(ttype);
    if (pSettings) {
        ret = pSettings->OnRenderWindowPositionChanged(render);
    }
    return ret;
}

long DemoClient::OnRenderingDisplayChanged(DEMO_TRACK_TYPE ttype)
{
    AutoLock theGuard(m_Mutex);

    long ret = WME_E_FAIL;
    VideoSettings *pSettings = GetVideoSettings(ttype);
    if (pSettings) {
        ret = pSettings->OnRenderingDisplayChanged();
    }
    return ret;
}
#ifdef WIN32
long DemoClient::OnRenderWindowClosed(DEMO_WINDOW_TYPE type)
{
	DEMO_TRACK_TYPE ttype = DEMO_LOCAL_TRACK;
	if(DEMO_WINDOW_SENDER == type)
	{
		ttype = DEMO_LOCAL_TRACK;
	}
	else if(DEMO_WINDOW_RECEIVER == type)
	{
		ttype = DEMO_REMOTE_TRACK;
	}
	else if(DEMO_WINDOW_PREVIEW == type)
	{
		ttype = DEMO_PREVIEW_TRACK;
	}
	else if(DEMO_WINDOW_DESKSHARING == type) // whsu
	{
		ttype = DEMO_REMOTE_DESKTOP_SHARE_TRACK;
	}
	return StopRenderView(ttype);
}
#endif
long DemoClient::StartMediaTrack(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE track)
{
    CM_INFO_TRACE_THIS("DemoClient::StartMediaTrack, mtype="<<mtype<<", track="<<track);

    AutoLock theGuard(m_Mutex);

	WMERESULT ret = WME_E_FAIL;
    MediaClient *pClient = GetMediaClient(mtype, track);
    if (pClient) {
        ret = pClient->StartTrack();
    }

	return ret;
}

long DemoClient::StopMediaTrack(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE track)
{
    CM_INFO_TRACE_THIS("DemoClient::StopMediaTrack, mtype="<<mtype<<", track="<<track);

    AutoLock theGuard(m_Mutex);

	WMERESULT ret = WME_E_FAIL;
    MediaClient *pClient = GetMediaClient(mtype, track);
    if (pClient) {
        ret = pClient->StopTrack();
    }

	return ret;
}

long DemoClient::SetVideoQuality(DEMO_TRACK_TYPE ttype, WmeVideoQualityType quality)
{
	CM_INFO_TRACE_THIS("DemoClient::SetVideoQuality, ttype="<<ttype<<", quality="<<quality);
    long ret = WME_E_FAIL;
    VideoSettings *pSettings = GetVideoSettings(ttype);
    if (pSettings) {
        ret = pSettings->SetVideoQuality(quality);
    }
    return ret;
}

long DemoClient::SetStaticPerformance(WmePerformanceProfileType perf)
{
	CM_INFO_TRACE_THIS("DemoClient::SetStaticPerformance, perf="<<perf);
	long ret = WME_E_FAIL;
	if (m_pVideoSender)
	{
		ret = m_pVideoSender->SetStaticPerformance(perf);
	}
	return ret;
}

long DemoClient::GetCapability(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype, WmeMediaBaseCapability *pMC)
{
    long ret = WME_E_FAIL;
    MediaClient *pClient = GetMediaClient(mtype, ttype);
    if (pClient) {
        ret = pClient->GetCapability(pMC);
    }
    return ret;
}

/////////////////////////
long DemoClient::SetCamera(DEMO_TRACK_TYPE ttype, IWmeMediaDevice* pCamera)
{
    AutoLock theGuard(m_Mutex);

    long ret = WME_E_FAIL;
    VideoSettings *pSettings = GetVideoSettings(ttype);
	if (pSettings) {
        ret = pSettings->SetDevice(pCamera);
    }

	return ret;
}

IWmeScreenSource * DemoClient::GetOneScreenSource(){
    if(m_pShareSourceManager)
        return m_pShareSourceManager->GetOneScreenSource();
    return NULL;
}

long DemoClient::SetScreenSource(IWmeScreenSource * pIWmeScreenSource)
{
    AutoLock theGuard(m_Mutex);
    
    long ret = WME_E_FAIL;
    VideoSettings *pSettings = GetVideoSettings(DEMO_LOCAL_DESKTOP_SHARE_TRACK);
	if (pSettings) {
        ret = pSettings->SetScreenSource(pIWmeScreenSource);
    }
    
	return ret;
    
}
long DemoClient::SetMic(IWmeMediaDevice* pMic)
{
    returnv_if_fail(m_pAudioManager != NULL, WME_E_FAIL);
    return m_pAudioManager->SetDevice(pMic, AUDIO_DEV_MIC);
}

long DemoClient::SetSpeaker(IWmeMediaDevice* pSpeaker)
{
    returnv_if_fail(m_pAudioManager != NULL, WME_E_FAIL);
    return m_pAudioManager->SetDevice(pSpeaker, AUDIO_DEV_SPEAKER);
}

long DemoClient::GetCaptureDevice(DEMO_TRACK_TYPE ttype, DEMO_DEV_TYPE dtype, IWmeMediaDevice** ppDev)
{
	long ret = WME_E_FAIL;

	returnv_if_fail(ppDev != NULL, WME_E_INVALIDARG);

	if(dtype == DEV_TYPE_CAMERA)
	{
		VideoSettings *pSettings = GetVideoSettings(ttype);
		if (pSettings) {
			ret = pSettings->GetDevice(ppDev);
		}

		return ret;
	}

	if(dtype == DEV_TYPE_MIC || dtype == DEV_TYPE_SPEAKER)
	{
		returnv_if_fail(m_pAudioManager != NULL, WME_E_FAIL);

		if(dtype == DEV_TYPE_MIC)
		{
			ret = m_pAudioManager->GetDevice(ppDev, AUDIO_DEV_MIC);
		}
		else if (dtype == DEV_TYPE_SPEAKER)
		{
			ret = m_pAudioManager->GetDevice(ppDev, AUDIO_DEV_SPEAKER);
		}
		return ret;

	}
}



long DemoClient::GetDeviceList(DEMO_MEDIA_TYPE mtype, DEMO_DEV_TYPE dtype, DevicePropertyList& dpList)
{
    long ret = WME_E_FAIL;
    MediaManager *pManager = GetMediaManager(mtype);
    if (pManager) {
        ret = pManager->GetMediaDevices(dtype, dpList);
    }
    return ret;
}

long DemoClient::ClearDeviceList(DEMO_MEDIA_TYPE mtype, DevicePropertyList& dpList)
{
    long ret = WME_E_FAIL;
    MediaManager *pManager = GetMediaManager(mtype);
    if (pManager) {
        ret = pManager->ClearMediaDevices(dpList);
    }
    return ret;
}

long DemoClient::GetMediaCapabilities(DEMO_MEDIA_TYPE mType, void* mcList)
{
    long ret = WME_E_FAIL;
    MediaManager *pManager = GetMediaManager(mType);
    if (pManager) {
        ret = pManager->GetMediaCapabilities(mcList);
    }
	
	return ret;
}

long DemoClient::SetMediaCapability(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype, WmeMediaBaseCapability *mc)
{
    AutoLock theGuard(m_Mutex);
    long ret = WME_E_FAIL;
    MediaClient *pClient = GetMediaClient(mtype, ttype);
    if (pClient) {
        ret = pClient->SetCapability(mc);
    }

	return ret;
}

long DemoClient::GetMediaCapability(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype, WmeMediaBaseCapability *pMc)
{
	
	if(!pMc)
		return WME_E_FAIL;

	AutoLock theGuard(m_Mutex);


	long ret = WME_E_FAIL;
	MediaClient *pClient = GetMediaClient(mtype, ttype);
	if (pClient) {
		ret = pClient->GetCapability(pMc);
	}

	return ret;
}

long DemoClient::SetMediaCodec(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype, WmeCodecType ctype)
{
    long ret = WME_E_FAIL;
    IWmeMediaCodec *pCodec = NULL;
    MediaManager *pManager = GetMediaManager(mtype);
    if (pManager) {
        ret = pManager->QueryMediaCodec(ctype, pCodec);
    }

    AutoLock theGuard(m_Mutex);
    
    MediaClient *pClient = GetMediaClient(mtype, ttype);
    if (pClient && pCodec) {
        ret = pClient->SetTrackCodec(pCodec);
    }
    
	if (mtype == DEMO_MEDIA_VIDEO)
	{
		m_VideoCodecType = ctype;
	}
	else if (mtype == DEMO_MEDIA_AUDIO)
	{
		m_AudioCodecType = ctype;
	}
    return ret;
}

long DemoClient::GetMediaCodec(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype, WmeCodecType &ctype)
{
	long ret = WME_E_FAIL;
	IWmeMediaCodec *pCodec = NULL;

	AutoLock theGuard(m_Mutex);

	MediaClient *pClient = GetMediaClient(mtype, ttype);
	if (pClient) {
		ret = pClient->GetTrackCodec(&pCodec);
	}

	if (pCodec)
	{
		ret = pCodec->GetCodecType(ctype);
		pCodec->Release();
		pCodec = NULL;
	}

	return ret;
}

long DemoClient::SetMediaSession(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype, WmeCodecType codec)
{
    AutoLock theGuard(m_Mutex);

    long ret = WME_E_FAIL;
    MediaClient *pClient = GetMediaClient(mtype, ttype);
    if (pClient) {
        ret = pClient->SetRtpSession(mapToPayloadType(codec), 0);
    }
    
    return ret;
}

long DemoClient::GetDeviceCapabilities(DEMO_DEV_TYPE dtype, IWmeMediaDevice *dev, DeviceCapabilityList &dcList)
{
    returnv_if_fail(dev != NULL, WME_E_INVALIDARG);
    
    long ret = WME_E_FAIL;
    
    WmeDeviceCapability dc;
    if (dtype == DEV_TYPE_CAMERA) {
        dc.eType = WmeDeviceCapabilityVideo;
        dc.iSize = sizeof(WmeCameraCapability);
        
        for (int k=0; true; k++) {
            dc.pCapalibity = new WmeCameraCapability;
            if (dev->GetCapabilities(k, &dc) != WME_S_OK) {
                break;
            }
            dcList.push_back(dc);
        }
        ret = WME_S_OK;
    }
    return ret;
}

long DemoClient::ClearDeviceCapabilities(DEMO_DEV_TYPE dtype, DeviceCapabilityList &dcList)
{
    if (dcList.empty()) {
        return WME_S_OK;
    }
    
    long ret = WME_S_OK;
    if (dtype == DEV_TYPE_CAMERA) {
        for(int k=0; k < dcList.size(); k++) {
            WmeDeviceCapability *pDC = &(dcList.at(k));
            WmeCameraCapability *pCC = (WmeCameraCapability *)pDC->pCapalibity;
            SAFE_DELETE(pCC);
        }
        dcList.clear();
    }
    return ret;
}

long DemoClient::SetCameraCapability(DEMO_TRACK_TYPE ttype, WmeDeviceCapability *pDC)
{
    AutoLock theGuard(m_Mutex);
    
    returnv_if_fail(pDC != NULL, WME_E_FAIL);
    returnv_if_fail(pDC->pCapalibity != NULL, WME_E_FAIL);
    WmeCameraCapability *pCC = (WmeCameraCapability *)pDC->pCapalibity;

    long ret = WME_E_FAIL;
    VideoSettings *pSettings = GetVideoSettings(ttype);
    if(pSettings) {
        WmeVideoRawFormat format = {pCC->type, static_cast<int32_t>(pCC->width), static_cast<int32_t>(pCC->height), pCC->MaxFPS, 0};
        ret = pSettings->SetCaptureCapability(format);
    }
    
    return ret;
}


void DemoClient::EnableMyMedia(DEMO_MEDIA_TYPE mtype, bool bEnable)
{
    if (mtype == DEMO_MEDIA_VIDEO)
        m_bEnableMyVideo = bEnable;
    else if (mtype == DEMO_MEDIA_DESKSHARE) // whsu
        m_bEnableMyDeskShare = bEnable;
    else
        m_bEnableMyAudio = bEnable;
}

void DemoClient::EnableQoS(bool bEnable)
{
	m_bEnableQoS = bEnable;

	if(m_pWmeAudioSession)
	{
		m_pWmeAudioSession->SetOption(WmeSessionOption_EnableQOS, &bEnable, sizeof(bool));
	}

	if(m_pWmeVideoSession)
	{
		m_pWmeVideoSession->SetOption(WmeSessionOption_EnableQOS, &bEnable, sizeof(bool));
	}
}


int DemoClient::ProcessDeviceAdded(DevicePropertyList &dpList, WmeEventDataDeviceChanged &changeEvent)
{
	int nCount = dpList.size();
	for(int i=0; i<nCount; i++)
	{
		if(dpList.at(i).dev->IsSameDevice(changeEvent.szUniqueName, changeEvent.iNameLen) == WME_S_OK)
		{
			if(m_pBackUISink)
			{
				m_pBackUISink->OnDeviceChanged(&(dpList.at(i)), changeEvent);
			}
			return WME_S_OK;
		}
	}

	return WME_E_FAIL;
}

int DemoClient::ProcessDeviceDeleted(DeviceProperty &dp, WmeEventDataDeviceChanged &changeEvent)
{
	if(m_pBackUISink)
	{
		m_pBackUISink->OnDeviceChanged(&dp, changeEvent);
	}

	return WME_S_OK;;
}

int DemoClient::ProcessDefaultDeviceUpdated(DeviceProperty &dp, WmeEventDataDeviceChanged &changeEvent)
{
	if(m_pBackUISink)
	{
		m_pBackUISink->OnDeviceChanged(&dp, changeEvent);
	}

	return WME_S_OK;;
}

long DemoClient::OnDeviceChanged(DEMO_MEDIA_TYPE mtype, DEMO_DEV_TYPE dtype, WmeEventDataDeviceChanged &changeEvent)
{
#ifdef IOS
    if ((dtype == DEV_TYPE_SPEAKER) ||
        (dtype == DEV_TYPE_MIC)){
        if(m_pBackUISink)
        {
            m_pBackUISink->OnDeviceChanged(NULL, changeEvent);
        }
        return WME_S_OK;
    }
#endif
	if(changeEvent.iType == WmeDeviceRemoved)
	{
		DeviceProperty dp;
		memset(&dp, 0, sizeof(DeviceProperty));
		dp.dev_type = dtype;
		strncpy(dp.unique_name, changeEvent.szUniqueName, std::min((int32_t)sizeof(dp.unique_name), changeEvent.iNameLen));
		dp.unique_name_len = changeEvent.iNameLen;
		return ProcessDeviceDeleted(dp, changeEvent);
	}
	else if(changeEvent.iType == WmeDefaultDeviceChanged)
	{
		DeviceProperty dp;
		memset(&dp, 0, sizeof(DeviceProperty));
		dp.dev_type = dtype;
		strncpy(dp.unique_name, changeEvent.szUniqueName, std::min((int32_t)sizeof(dp.unique_name), changeEvent.iNameLen));
		dp.unique_name_len = changeEvent.iNameLen;
		return ProcessDefaultDeviceUpdated(dp, changeEvent);	
	}

	DevicePropertyList dpList;
	long ret = GetDeviceList(mtype, dtype, dpList);
	if(ret == WME_S_OK)
	{
		ret = ProcessDeviceAdded(dpList, changeEvent);
	}
    ClearDeviceList(mtype, dpList);
	return ret;
}

long DemoClient::OnAudioInChanged(WmeEventDataDeviceChanged &changeEvent)
{
    return OnDeviceChanged(DEMO_MEDIA_AUDIO, DEV_TYPE_MIC, changeEvent);
}

long DemoClient::OnAudioOutChanged(WmeEventDataDeviceChanged &changeEvent)
{
    return OnDeviceChanged(DEMO_MEDIA_AUDIO, DEV_TYPE_SPEAKER, changeEvent);
}

long DemoClient::OnVideoInChanged(WmeEventDataDeviceChanged &changeEvent)
{
    return OnDeviceChanged(DEMO_MEDIA_VIDEO, DEV_TYPE_CAMERA, changeEvent);
}

long DemoClient::OnVolumeChanged(WmeEventDataVolumeChanged &stChangeEvent)
{
    if (m_pBackUISink) {
        m_pBackUISink->OnVolumeChange(stChangeEvent);
    }
    return WME_S_OK;
}

long DemoClient::GetAudioVolume(DEMO_DEV_TYPE dtype, int &nVol)
{
    returnv_if_fail(m_pAudioManager != NULL, WME_E_FAIL);

    long ret = WME_E_FAIL;
    if (dtype == DEV_TYPE_MIC){
        ret = m_pAudioManager->GetVolume(nVol, AUDIO_DEV_MIC);
    }else if (dtype == DEV_TYPE_SPEAKER) {
        ret = m_pAudioManager->GetVolume(nVol, AUDIO_DEV_SPEAKER);
    }
    return ret;
}

void DemoClient::SetAudioVolume(DEMO_DEV_TYPE dtype, int nVol)
{
    return_if_fail(m_pAudioManager != NULL);
    
    long ret = WME_E_FAIL;
    if (dtype == DEV_TYPE_MIC){
        ret = m_pAudioManager->SetVolume(nVol, AUDIO_DEV_MIC);
    }else if (dtype == DEV_TYPE_SPEAKER) {
        ret = m_pAudioManager->SetVolume(nVol, AUDIO_DEV_SPEAKER);
    }
}

void DemoClient::MuteAudio(DEMO_DEV_TYPE dtype, bool bmute)
{
    return_if_fail(m_pAudioManager != NULL);

    long ret = WME_E_FAIL;
    if (dtype == DEV_TYPE_MIC){
        ret = m_pAudioManager->SetMute(bmute, AUDIO_DEV_MIC);
    }else if (dtype == DEV_TYPE_SPEAKER) {
        ret = m_pAudioManager->SetMute(bmute, AUDIO_DEV_SPEAKER);
    }
}

void DemoClient::SetAudioOutType(DEMO_AUDIO_OUT_TYPE nType)
{
	//SPEAKER:1
	//VOICE:4
    return_if_fail(m_pAudioManager != NULL);
    m_pAudioManager->SetAudioOutType((int)nType);
}

bool DemoClient::IsAudioMute(DEMO_DEV_TYPE dtype)
{
    returnv_if_fail(m_pAudioManager != NULL, false);

    bool bmute = false;
    long ret = WME_E_FAIL;
    if (dtype == DEV_TYPE_MIC){
        ret = m_pAudioManager->GetMute(bmute, AUDIO_DEV_MIC);
    }else if (dtype == DEV_TYPE_SPEAKER) {
        ret = m_pAudioManager->GetMute(bmute, AUDIO_DEV_SPEAKER);
    }
    
    return bmute;
}

void DemoClient::GetVoiceLevel(unsigned int &level)
{
    if (m_pAudioReceiver) {
        m_pAudioReceiver->GetVoiceLevel(level);
    }
}

void DemoClient::DeleteMediaClient(DEMO_MEDIA_TYPE mtype)
{
    DeleteMediaClient(mtype, DEMO_LOCAL_TRACK);
    DeleteMediaClient(mtype, DEMO_REMOTE_TRACK);
    DeleteMediaClient(mtype, DEMO_PREVIEW_TRACK);
}

void DemoClient::DeleteMediaClient(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype)
{
	CM_INFO_TRACE_THIS("DemoClient::DeleteMediaClient, mtype="<<mtype<<", ttype="<<ttype);
    AutoLock theGuard(m_Mutex);

    if (ttype == DEMO_PREVIEW_TRACK) {
        if (mtype == DEMO_MEDIA_VIDEO)
            DeleteVideoPreview();
        return;
    }

    if (mtype == DEMO_MEDIA_VIDEO)
    {
        if(ttype == DEMO_LOCAL_TRACK) {
            DeleteVideoSender();
        }else if (ttype == DEMO_REMOTE_TRACK) {
            if (m_pVideoTransport) {
                m_pVideoTransport->SetReceiver(NULL);
            }
            DeleteVideoReceiver();
        }

    }
    else if (mtype == DEMO_MEDIA_AUDIO)
    {
        if(ttype == DEMO_LOCAL_TRACK) {
            DeleteAudioSender();
        }else if (ttype == DEMO_REMOTE_TRACK) {
            if (m_pAudioTransport) {
                m_pAudioTransport->SetReceiver(NULL);
            }
            DeleteAudioReceiver();
        }

    }
	else if (mtype == DEMO_MEDIA_DESKSHARE)// whsu
    {
		if(ttype == DEMO_LOCAL_TRACK) {
			DeleteDeskShareSender();
		}else if (ttype == DEMO_REMOTE_TRACK) {
			if (m_pDeskShareTransport) {
                m_pDeskShareTransport->SetReceiver(NULL);
            }
			DeleteDeskShareReceiver();
		}
    }
}

long DemoClient::CreateMediaClient(DEMO_MEDIA_TYPE mtype)
{
    CreateMediaClient(mtype, DEMO_LOCAL_TRACK);
    CreateMediaClient(mtype, DEMO_REMOTE_TRACK);
    CreateMediaClient(mtype, DEMO_PREVIEW_TRACK);
    return WME_S_OK;
}

long DemoClient::CreateMediaClient(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype)
{
	CM_INFO_TRACE_THIS("DemoClient::CreateMediaClient, mtype="<<mtype<<", ttype="<<ttype);

    AutoLock theGuard(m_Mutex);

    if (ttype == DEMO_PREVIEW_TRACK) {
        if (mtype == DEMO_MEDIA_VIDEO)
            CreateVideoPreview();
        return WME_S_OK;
    }

	WMERESULT ret = WME_S_OK;
    if (mtype == DEMO_MEDIA_VIDEO)
    {
        if(ttype == DEMO_LOCAL_TRACK) {
            CreateVideoSender();
        }else if (ttype == DEMO_REMOTE_TRACK) {
            CreateVideoReceiver();
        }
    }
    else if (mtype == DEMO_MEDIA_AUDIO)
    {
        if(ttype == DEMO_LOCAL_TRACK) {
            CreateAudioSender();
        }else if (ttype == DEMO_REMOTE_TRACK) {
            CreateAudioReceiver();
        }
    }
	else if (mtype == DEMO_MEDIA_DESKSHARE) // whsu 2 
    {
		CreateDeskShareTransport();
        if(ttype == DEMO_LOCAL_TRACK) {
            CreateDeskShareSender();
        }else if (ttype == DEMO_REMOTE_TRACK) {
            CreateDeskShareReceiver();
        }
    }

	return ret;
}

long DemoClient::GetVideoStatistics(WmeSessionStatistics &wmeVideoStatistics)
{
    if (m_pWmeVideoSession) {
        return m_pWmeVideoSession->GetStatistics(wmeVideoStatistics);
    }
    return WME_E_FAIL;
}

long DemoClient::GetAudioStatistics(WmeSessionStatistics &wmeVideoStatistics)
{
    if (m_pWmeAudioSession) {
        return m_pWmeAudioSession->GetStatistics(wmeVideoStatistics);
    }
    return WME_E_FAIL;
}

long DemoClient::GetVideoStatistics(DEMO_TRACK_TYPE ttype, WmeVideoStatistics &wmeStatistics)
{
	VideoSettings *pSettings = GetVideoSettings(ttype);
	if (pSettings) {
		return pSettings->GetStatistics(wmeStatistics);
	}
	return WME_E_FAIL;
}

long DemoClient::GetAudioStatistics(DEMO_TRACK_TYPE ttype, WmeAudioStatistics &wmeStatistics)
{
	AudioSettings *pSettings = GetAudioSettings(ttype);
	if (pSettings) {
		return pSettings->GetStatistics(wmeStatistics);
	}
	return WME_E_FAIL;
}

long DemoClient::GetSyncStatistics(WmeSyncStatistics &wmeSyncStatistics)
{
    if(m_pWmeSyncBox)
    {
        return m_pWmeSyncBox->GetStatistics(wmeSyncStatistics);
    }
    return WME_E_FAIL;
}

long DemoClient::GetNetworkIndex(WmeNetworkIndex& idx, WmeNetworkDirection d)
{
    if(m_pWmeEngine)
    {
        idx = m_pNetworkIndicator->GetNetworkIndex(d);
        return WME_S_OK;
    }
    return WME_E_FAIL;
}

/// for audio options
long DemoClient::EnableEC(bool bEnable)
{
	AutoLock theGuard(m_Mutex);

	long ret = WME_E_FAIL;
	AudioSettings *pSettings = GetAudioSettings(DEMO_LOCAL_TRACK);
	if (pSettings) {
		ret = pSettings->EnableEC(bEnable);
	}

	return ret;
}

long DemoClient::EnableVAD(bool bEnable)
{
	AutoLock theGuard(m_Mutex);

	long ret = WME_E_FAIL;
	AudioSettings *pSettings = GetAudioSettings(DEMO_LOCAL_TRACK);
	if (pSettings) {
		ret = pSettings->EnableVAD(bEnable);
	}

	return ret;
}

long DemoClient::EnableNS(bool bEnable)
{
	AutoLock theGuard(m_Mutex);

	long ret = WME_E_FAIL;
	AudioSettings *pSettings = GetAudioSettings(DEMO_LOCAL_TRACK);
	if (pSettings) {
		ret = pSettings->EnableNS(bEnable);
	}

	return ret;
}

long DemoClient::EnableAGC(bool bEnable)
{
	AutoLock theGuard(m_Mutex);

	long ret = WME_E_FAIL;
	AudioSettings *pSettings = GetAudioSettings(DEMO_LOCAL_TRACK);
	if (pSettings) {
		ret = pSettings->EnableAGC(bEnable);
	}

	return ret;
}

long DemoClient::EnableAutoDropData(bool bEnable)
{
	AutoLock theGuard(m_Mutex);

	long ret = WME_E_FAIL;
	AudioSettings *pSettings = GetAudioSettings(DEMO_LOCAL_TRACK);
	if (pSettings) {
		ret = pSettings->EnableAutoDropData(bEnable);
	}

	return ret;
}

long DemoClient::EnableDecoderMosaic(bool bEnable)
{
	AutoLock theGuard(m_Mutex);

	long ret = WME_E_FAIL;
	if (m_pVideoReceiver) {
		ret = m_pVideoReceiver->EnableDecoderMosaic(bEnable);
	}

	return ret;
}

/// for IMediaClientSink
void DemoClient::OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight)
{
    if (m_pBackUISink) {
        m_pBackUISink->OnDecodeSizeChanged(uLabel, uWidth, uHeight);
    }
	
#ifdef ANDROID
	//javaSetVideoDecodeResolution_STAT(uWidth, uHeight);
#endif	//ANDROID
}

void DemoClient::OnEndOfStream(DEMO_MEDIA_TYPE eType)
{
    CM_INFO_TRACE_THIS("DemoClient::OnEndOfStream");
    if (m_pBackUISink) {
        m_pBackUISink->OnEndOfStream(eType);
    }
}

////////////////////////////////////////////
/// for IMediaTransportSink

void DemoClient::OnConnected(bool bSuccess, ConnSinkType stype) 
{
    DEMO_MEDIA_TYPE mtype;
    if (stype == CONN_SINK_AUDIO)
        mtype = DEMO_MEDIA_AUDIO;
    else if (stype == CONN_SINK_VIDEO)
        mtype = DEMO_MEDIA_VIDEO;
    else if (stype == CONN_SINK_DESKSHARE) // whsu
        mtype = DEMO_MEDIA_DESKSHARE;

    CM_INFO_TRACE_THIS("DemoClient::OnConnected, bSuccess="<<bSuccess<<", sinkType="<<stype);
	if(bSuccess) {
        if(mtype == DEMO_MEDIA_DESKSHARE){
            StartMediaSending(mtype);
        }
        if (m_pBackUISink) {
            m_pBackUISink->connectRemoteOKFromClient(mtype);
        }
		return;
    }

    AutoLock theGuard(m_Mutex);

    if (m_pBackUISink)
        m_pBackUISink->connectRemoteFailureFromClient(mtype);
    //DeleteMediaClient(mtype);
}

void DemoClient::OnDisConnected(ConnSinkType stype) 
{
    AutoLock theGuard(m_Mutex);

    CM_INFO_TRACE_THIS("DemoClient::OnDisConnected, stype = "<<stype);
    if (stype == CONN_SINK_AUDIO) {
	    m_bStartAudioSending = false;
        
        CM_INFO_TRACE_THIS("DemoClient::OnDisConnected");
#ifdef CUCUMBER_ENABLE
        //sleep(10000);
        //DeleteAudioReceiver();
        //DeleteMediaClient(DEMO_MEDIA_AUDIO);
#endif
        if (m_pBackUISink)
		    m_pBackUISink->networkDisconnectFromClient(DEMO_MEDIA_AUDIO);
    }else if (stype == CONN_SINK_VIDEO){
	    m_bStartVideoSending = false;
        if (m_pBackUISink)
		    m_pBackUISink->networkDisconnectFromClient(DEMO_MEDIA_VIDEO);
    }else if (stype == CONN_SINK_DESKSHARE){  // whus
        StopMediaSending(DEMO_MEDIA_DESKSHARE);
        if (m_pBackUISink)
		    m_pBackUISink->networkDisconnectFromClient(DEMO_MEDIA_DESKSHARE);
		//DeleteMediaClient(DEMO_MEDIA_VIDEO);
    }
}

/// manage media manager of audio/video
MediaManager *DemoClient::GetMediaManager(DEMO_MEDIA_TYPE mtype)
{
    MediaManager *pManager = NULL;
    if(mtype == DEMO_MEDIA_AUDIO) {
        pManager = m_pAudioManager;
    }else if(mtype == DEMO_MEDIA_VIDEO) {
        pManager = m_pVideoManager;
    }else if(mtype == DEMO_MEDIA_DESKSHARE) { // whus
       pManager = m_pShareSourceManager;
	}

    return pManager;
}

/// manage media client of audio/video's sender/receiver
MediaClient *DemoClient::GetMediaClient(DEMO_MEDIA_TYPE mtype, DEMO_TRACK_TYPE ttype)
{
    MediaClient *pClient = NULL;
    if(mtype == DEMO_MEDIA_AUDIO) {
        switch(ttype) {
        case DEMO_LOCAL_TRACK:
            pClient = m_pAudioSender;
            break;
        case DEMO_REMOTE_TRACK:
            pClient = m_pAudioReceiver;
            break;
        }
    }else if (mtype == DEMO_MEDIA_VIDEO) {
        switch(ttype) {
        case DEMO_LOCAL_TRACK:
            pClient = m_pVideoSender;
            break;
        case DEMO_REMOTE_TRACK:
            pClient = m_pVideoReceiver;
            break;
        case DEMO_PREVIEW_TRACK:
            pClient = m_pVideoPreview;
            break;
        }
    }else if (mtype == DEMO_MEDIA_DESKSHARE) { // whsu
        switch(ttype) {
		case DEMO_LOCAL_DESKTOP_SHARE_TRACK: 
            pClient = m_pDeskShareSender;
            break;
		case DEMO_REMOTE_DESKTOP_SHARE_TRACK:
            pClient = m_pDeskShareReceiver;
            break;
//        case DEMO_PREVIEW_TRACK:
//            pClient = m_pVideoPreview;
 //           break;
        }
    }
    return pClient;
}

/// manage video client of video's preview/sender/receiver
VideoSettings *DemoClient::GetVideoSettings(DEMO_TRACK_TYPE ttype)
{
    VideoSettings *pSettings = NULL;
    switch(ttype) {
        case DEMO_LOCAL_TRACK:
            pSettings = m_pVideoSender;
            break;
        case DEMO_REMOTE_TRACK:
            pSettings = m_pVideoReceiver;
            break;
        case DEMO_PREVIEW_TRACK:
            pSettings = m_pVideoPreview;
            break;
		case DEMO_LOCAL_DESKTOP_SHARE_TRACK:  // whsu
            pSettings = m_pDeskShareSender;
            break;
		case DEMO_REMOTE_DESKTOP_SHARE_TRACK:
            pSettings = m_pDeskShareReceiver;
            break;
    }
    return pSettings;
}

/// manage audio client of audio's sender/receiver
AudioSettings *DemoClient::GetAudioSettings(DEMO_TRACK_TYPE ttype)
{
	AudioSettings *pSettings = NULL;
	switch(ttype) {
		case DEMO_LOCAL_TRACK:
			pSettings = m_pAudioSender;
			break;
	}
	return pSettings;
}

/////////////////////////////////////////////
/// for audio transport/sender/receiver
///

long DemoClient::CreateAudioSession()
{
    returnv_if_fail(m_pWmeEngine != NULL, WME_E_FAIL);

    if (m_pWmeAudioSession != NULL) {
        CM_INFO_TRACE_THIS("DemoClient::CreateAudioSession, m_pWmeAudioSession is not NULL!");
        return WME_S_OK;
    }

    long ret = m_pWmeEngine->CreateMediaSession(WmeSessionType_Audio, &m_pWmeAudioSession);
    if(WME_S_OK != ret)
    {
        CM_ERROR_TRACE_THIS("DemoClient::CreateAudioSession, create audio session failed!");
        return WME_S_FALSE;
    }

    bool bEnable = true;
    m_pWmeAudioSession->SetOption(WmeSessionOption_EnableRTCP, &bEnable, sizeof(bool));

	bEnable = m_bEnableQoS;
    m_pWmeAudioSession->SetOption(WmeSessionOption_EnableQOS, &bEnable, sizeof(bool));


    int audioCodecMapLen = sizeof(kAudioCodecMap) / sizeof (WmeCodecPayloadTypeMap);
    for (int k=0; k < audioCodecMapLen; k++) {
        m_pWmeAudioSession->SetOption(WmeSessionOption_CodecPayloadTypeMap, (void *)&(kAudioCodecMap[k]), sizeof(WmeCodecPayloadTypeMap));
    }

    return WME_S_OK;
}

long DemoClient::DeleteAudioSession()
{
    SAFE_RELEASE(m_pWmeAudioSession);
    return WME_S_OK;
}

long DemoClient::CreateAudioTransport(MediaTransport * &transport)
{
    returnv_if_fail(m_pWmeAudioSession != NULL, WME_E_FAIL);

    if (transport) {
        CM_INFO_TRACE_THIS("DemoClient::CreateAudioTransport, transport has existed!");
        return WME_S_OK;
    }
    
    transport = new MediaTransport();
    returnv_if_fail(transport != NULL, WME_E_FAIL);
    transport->SetSink(this, CONN_SINK_AUDIO);
    transport->SetSession(m_pWmeAudioSession);

    return WME_S_OK;
}

long DemoClient::DeleteAudioTransport()
{
	if (m_pAudioTransport)
		m_pAudioTransport->StopTransport();
    SAFE_DELETE(m_pAudioTransport);
    return WME_S_OK;
}

long DemoClient::CreateAudioManager()
{
    returnv_if_fail(m_pWmeEngine != NULL, WME_E_FAIL);
    
    if (m_pAudioManager) {
        CM_INFO_TRACE_THIS("DemoClient::CreateVideoManager, m_pVideoManager has existed!");
        return WME_S_OK;
    }

    m_pAudioManager = new AudioManager();
    returnv_if_fail(m_pAudioManager != NULL, WME_E_FAIL);
    m_pAudioManager->SetMediaEngine(m_pWmeEngine);
    m_pAudioManager->SetSink(this);

    if(m_pAudioManager->Init() != WME_S_OK) {
        CM_INFO_TRACE_THIS("DemoClient::CreateAudioManager, fail to m_pAudioManager->Init()!");
        return WME_E_FAIL;
    }

    return WME_S_OK;
}

long DemoClient::DeleteAudioManager()
{
    SAFE_DELETE(m_pAudioManager);
    return WME_S_OK;
}

long DemoClient::CreateAudioSender()
{
    returnv_if_fail(m_pWmeEngine != NULL, WME_E_FAIL);
    returnv_if_fail(m_pWmeAudioSession != NULL, WME_E_FAIL);

    if (m_pAudioSender) {
        CM_INFO_TRACE_THIS("DemoClient::CreateAudioSender, m_pAudioSender has existed!");
        return WME_S_OK;
    }

    m_pAudioSender = new AudioSender();
    returnv_if_fail(m_pAudioSender != NULL, WME_E_FAIL);
    m_pAudioSender->SetMediaEngine(m_pWmeEngine);
    m_pAudioSender->SetMediaSession(m_pWmeAudioSession);
    m_pAudioSender->SetSink(this);

    /// set MediaTransport into Sender(local track)
    if (m_pAudioTransport)
        m_pAudioSender->SetTransport(m_pAudioTransport);


    //sequence! before create track
    m_pAudioSender->SetAudioSource(m_bUseAudioInputFile);
    if (m_bUseAudioInputFile)
        m_pAudioSender->SetAudioInputFile(m_strAudioInputFileName.c_str(), &m_sAudioRawFormat);


    /// create local track
    m_pAudioSender->CreateTrack();

    /// default settings
	SetMediaCodec(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK, m_AudioCodecType);
    m_pAudioSender->SetRtpSession(mapToPayloadType(m_AudioCodecType), 0);

    return WME_S_OK;
}

long DemoClient::DeleteAudioSender()
{
    if(m_pAudioSender) {
        m_pAudioSender->DeleteTrack();
        SAFE_DELETE(m_pAudioSender);
    }
    return WME_S_OK;
}

long DemoClient::CreateAudioReceiver()
{
    returnv_if_fail(m_pWmeEngine != NULL, WME_E_FAIL);
    returnv_if_fail(m_pWmeAudioSession != NULL, WME_E_FAIL);
    returnv_if_fail(m_pWmeSyncBox != NULL, WME_E_FAIL);

    if (m_pAudioReceiver) {
        CM_INFO_TRACE_THIS("DemoClient::CreateAudioReceiver, m_pAudioReceiver has existed!");
        return WME_S_OK;
    }

    m_pAudioReceiver = new AudioReceiver();
    returnv_if_fail(m_pAudioReceiver != NULL, WME_E_FAIL);
    m_pAudioReceiver->SetMediaEngine(m_pWmeEngine);
    m_pAudioReceiver->SetMediaSession(m_pWmeAudioSession);
    m_pAudioReceiver->SetMediaSyncBox(m_pWmeSyncBox);

    /// set Receiver(remote track) into MediaTransport
    if (m_pAudioTransport)
        m_pAudioTransport->SetReceiver(m_pAudioReceiver);
    
    
    m_pAudioReceiver->SetAudioTarget(m_bUseAudioOutputFile);
    if (m_bUseAudioOutputFile) {
        m_pAudioReceiver->SetAudioOutputFile(m_strAudioOuputFileName.c_str());
    }
    

    /// create remote track
    m_pAudioReceiver->CreateTrack();

    /// default settings
	SetMediaCodec(DEMO_MEDIA_AUDIO, DEMO_REMOTE_TRACK, m_AudioCodecType);
    m_pAudioReceiver->SetRtpSession(mapToPayloadType(m_AudioCodecType), 0);

    return WME_S_OK;
}

long DemoClient::DeleteAudioReceiver()
{
    if(m_pAudioReceiver) {
        m_pAudioReceiver->DeleteTrack();
        SAFE_DELETE(m_pAudioReceiver);
    }
    return WME_S_OK;
}

/////////////////////////////////////////////
/// for video transport/sender/receiver/previewer
///

long DemoClient::CreateVideoSession()
{
    returnv_if_fail(m_pWmeEngine != NULL, WME_E_FAIL);

    if (m_pWmeVideoSession != NULL) {
        CM_INFO_TRACE_THIS("DemoClient::CreateVideoSession, m_pWmeVideoSession is not NULL!");
        return WME_S_OK;
    }

    long ret = m_pWmeEngine->CreateMediaSession(WmeSessionType_Video, &m_pWmeVideoSession);
    if(WME_S_OK != ret)
    {
        CM_ERROR_TRACE_THIS("DemoClient::CreateVideoSession, create video session failed! error = " << ret);
        return WME_S_FALSE;
    }
    
    bool bEnable = true;
    m_pWmeVideoSession->SetOption(WmeSessionOption_EnableRTCP, &bEnable, sizeof(bool));

    bEnable = m_bEnableQoS;
    m_pWmeVideoSession->SetOption(WmeSessionOption_EnableQOS, &bEnable, sizeof(bool));
	
	//DisableSendingFilterFeedback();
	if (m_bEnableSendingFilterFeedback == false)
	{
		bool enable = false;
		m_pWmeVideoSession->SetOption(WmeSessionOption_EnableSendingFilterFeedback, &enable, sizeof(enable));
	}
	
    WmeRTPExtension rtpExtension;
    rtpExtension.sExtURI = (char *)"http://protocols.cisco.com/framemarking";
    rtpExtension.uExtID = 1;
    m_pWmeVideoSession->SetOption(WmeSessionOption_RTPExtension, &rtpExtension, sizeof(WmeRTPExtension));

	rtpExtension.sExtURI = (char *)"urn:ietf:params:rtp-hdrext:toffset";
	rtpExtension.uExtID = 2;
	m_pWmeVideoSession->SetOption(WmeSessionOption_RTPExtension, &rtpExtension, sizeof(WmeRTPExtension));


#if 0
    m_pWmeVideoSession->RegisterRTPExtensionProcessFunc(pRTPExtension_Time, AddTimeToRTPExtension, GetTimeFromRTPExtension);
    WmeRTPExtension myRTPExtension;
    myRTPExtension.sExtURI = pRTPExtension_Time;
    myRTPExtension.uExtID = nRTPExtension_Time;
    m_pWmeVideoSession->SetOption(WmeSessionOption_RTPExtension, &myRTPExtension, sizeof(WmeRTPExtension));
#endif
    
    int videoCodecMapLen = sizeof(kVideoCodecMap) / sizeof (WmeCodecPayloadTypeMap);
    for (int k=0; k < videoCodecMapLen; k++) {
        m_pWmeVideoSession->SetOption(WmeSessionOption_CodecPayloadTypeMap, (void *)&(kVideoCodecMap[k]), sizeof(WmeCodecPayloadTypeMap));
    }
    return WME_S_OK;
}

long DemoClient::DeleteVideoSession()
{
    SAFE_RELEASE(m_pWmeVideoSession);
    return WME_S_OK;
}

long DemoClient::CreateVideoTransport(MediaTransport * &transport)
{
    returnv_if_fail(m_pWmeVideoSession != NULL, WME_E_FAIL);

    if (transport) {
        CM_INFO_TRACE_THIS("DemoClient::CreateVideoTransport, transport has existed!");
        return WME_S_OK;
    }
    
    transport = new MediaTransport();
	returnv_if_fail(transport != NULL, WME_E_FAIL);
	transport->SetSink(this, CONN_SINK_VIDEO);
    transport->SetSession(m_pWmeVideoSession);

    return WME_S_OK;
}

long DemoClient::DeleteVideoTransport()
{
	if (m_pVideoTransport)
		m_pVideoTransport->StopTransport();
    SAFE_DELETE(m_pVideoTransport);
    return WME_S_OK;
}

long DemoClient::DeleteDeskShareTransport() // whsu
{
	if (m_pDeskShareTransport)
		m_pDeskShareTransport->StopTransport();
    SAFE_DELETE(m_pDeskShareTransport);
    return WME_S_OK;
}

long DemoClient::CreateVideoManager()
{
    returnv_if_fail(m_pWmeEngine != NULL, WME_E_FAIL);
    
    if (m_pVideoManager) {
        CM_INFO_TRACE_THIS("DemoClient::CreateVideoManager, m_pVideoManager has existed!");
        return WME_S_OK;
    }

    m_pVideoManager = new VideoManager();
    returnv_if_fail(m_pVideoManager != NULL, WME_E_FAIL);
    m_pVideoManager->SetMediaEngine(m_pWmeEngine);
    m_pVideoManager->SetSink(this);

    if(m_pVideoManager->Init() != WME_S_OK) {
        CM_ERROR_TRACE_THIS("DemoClient::CreateVideoManager, fail to m_pVideoManager->Init()!");
        return WME_E_FAIL;
    }

    return WME_S_OK;
}

long DemoClient::DeleteVideoManager()
{
    SAFE_DELETE(m_pVideoManager);
    return WME_S_OK;
}

long DemoClient::CreateVideoSender()
{
    returnv_if_fail(m_pWmeEngine != NULL, WME_E_FAIL);
    returnv_if_fail(m_pWmeVideoSession != NULL, WME_E_FAIL);

    if (m_pVideoSender) {
        CM_INFO_TRACE_THIS("DemoClient::CreateVideoSender, m_pVideoSender has existed!");
        return WME_S_OK;
    }

    m_pVideoSender = new VideoSender();
    returnv_if_fail(m_pVideoSender != NULL, WME_E_FAIL);
    m_pVideoSender->SetMediaEngine(m_pWmeEngine);
    m_pVideoSender->SetMediaSession(m_pWmeVideoSession);
    m_pVideoSender->SetSink(this);

    /// set MediaTransport into Sender(local track)
    if (m_pVideoTransport)
        m_pVideoSender->SetTransport(m_pVideoTransport);

	m_pVideoSender->SetVideoSource(m_bUseVideoInputFile);

	if(m_bUseVideoInputFile)
	{
		m_pVideoSender->SetVideoFile(m_strVideoInputFileName.c_str(), &m_sVideoRawFormat);
	}

    /// create local track
    if(m_pVideoSender->CreateTrack() != WME_S_OK) {
        CM_ERROR_TRACE_THIS("DemoClient::CreateVideoSender, fail to createTrack");
        return WME_E_FAIL;
    }

    /// default settings
	SetMediaCodec(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK, m_VideoCodecType);
    m_pVideoSender->SetDefaultSettings();
    m_pVideoSender->SetRtpSession(mapToPayloadType(m_VideoCodecType), 0);

    return WME_S_OK;
}

long DemoClient::DeleteVideoSender()
{
    if(m_pVideoSender) {
        m_pVideoSender->DeleteTrack();
        SAFE_DELETE(m_pVideoSender);
    }
    return WME_S_OK;
}

long DemoClient::CreateVideoReceiver()
{
    returnv_if_fail(m_pWmeEngine != NULL, WME_E_FAIL);
    returnv_if_fail(m_pWmeVideoSession != NULL, WME_E_FAIL);
    returnv_if_fail(m_pWmeSyncBox != NULL, WME_E_FAIL);

    if (m_pVideoReceiver) {
        CM_INFO_TRACE_THIS("DemoClient::CreateVideoReceiver, m_pVideoReceiver has existed!");
        return WME_S_OK;
    }

    m_pVideoReceiver = new VideoReceiver();
    returnv_if_fail(m_pVideoReceiver != NULL, WME_E_FAIL);
    m_pVideoReceiver->SetSink(this);  /// set sink to recv the events from remote track
    m_pVideoReceiver->SetMediaEngine(m_pWmeEngine);
    m_pVideoReceiver->SetMediaSession(m_pWmeVideoSession);
    m_pVideoReceiver->SetMediaSyncBox(m_pWmeSyncBox);

    /// set Receiver(remote track) into MediaTransport
    if (m_pVideoTransport)
        m_pVideoTransport->SetReceiver(m_pVideoReceiver);
    
    
    m_pVideoReceiver->SetVideoTarget(m_bUseVideoOutputFile);
    
	if(m_bUseVideoOutputFile)
	{
		m_pVideoReceiver->SetVideoFile(m_strVideoOutputFileName.c_str());
	}
    

    /// create remote track
    if(m_pVideoReceiver->CreateTrack() != WME_S_OK){
        CM_ERROR_TRACE_THIS("DemoClient::CreateVideoReceiver, fail to CreateTrack!");
        return WME_E_FAIL;
    }

    /// default settings
	//SetMediaCodec(DEMO_MEDIA_VIDEO, DEMO_REMOTE_TRACK, m_VideoCodecType); //don't call it, it will recreate video listen channel
    m_pVideoReceiver->SetRtpSession(mapToPayloadType(m_VideoCodecType), 0);

    return WME_S_OK;
}

long DemoClient::DeleteVideoReceiver()
{
    if(m_pVideoReceiver) {
        m_pVideoReceiver->DeleteTrack();
        SAFE_DELETE(m_pVideoReceiver);
    }
    return WME_S_OK;
}

long DemoClient::CreateVideoPreview()
{
	CM_INFO_TRACE_THIS("DemoClient::CreateVideoPreview");
    returnv_if_fail(m_pWmeEngine != NULL, WME_E_FAIL);

    if (m_pVideoPreview) {
        CM_INFO_TRACE_THIS("DemoClient::CreateVideoPreview, m_pVideoPreview has existed!");
        return WME_S_OK;
    }

    m_pVideoPreview = new VideoPreview();
    returnv_if_fail(m_pVideoPreview != NULL, WME_E_FAIL);
    m_pVideoPreview->SetMediaEngine(m_pWmeEngine);

    /// create preview track
    if(m_pVideoPreview->CreateTrack() != WME_S_OK) {
        CM_ERROR_TRACE_THIS("DemoClient::CreateVideoPreview, fail to CreateTrack!");
        return WME_E_FAIL;
    }

    return WME_S_OK;
}

long DemoClient::DeleteVideoPreview()
{
	CM_INFO_TRACE_THIS("DemoClient::DeleteVideoPreview");
    if (m_pVideoPreview) {
        m_pVideoPreview->DeleteTrack();
        SAFE_DELETE(m_pVideoPreview);
    }
    return WME_S_OK;
}

long DemoClient::SetVideoInputFile(const char* pFileName, WmeVideoRawFormat *pFormat)
{
	if(!pFileName || !pFormat || strlen(pFileName) == 0 || pFormat->eRawType == WmeVideoUnknown)
	{
		CM_ERROR_TRACE_THIS("DemoClient::SetVideoInputFile, invalid video file name or video format!");
		return WME_E_INVALIDARG;
	}

	CM_INFO_TRACE_THIS("DemoClient::SetVideoInputFile, video file:" << pFileName 
		<< ", nWidth = " << pFormat->iWidth
		<< ", nHeight = " << pFormat->iHeight
		<< ", format = " << pFormat->eRawType
		<< ", ");

	m_strVideoInputFileName = pFileName;
	m_sVideoRawFormat = *pFormat;

	if(m_pVideoSender)
		return m_pVideoSender->SetVideoFile(pFileName, pFormat);

	return WME_E_FAIL;
}

void DemoClient::SetVideoSource(int nSource)
{
	if(nSource == 1)
		m_bUseVideoInputFile = TRUE;    //1 is file
	else
		m_bUseVideoInputFile = FALSE;   //0 is camera
    
	if(m_pVideoSender)
		return m_pVideoSender->SetVideoSource(nSource);
}

long DemoClient::SetVideoOutputFile(const char* pFileName)
{
    if(!pFileName || strlen(pFileName) == 0)
	{
		CM_ERROR_TRACE_THIS("DemoClient::SetVideoOutputFile, invalid video file name!");
		return WME_E_INVALIDARG;
	}
    
	CM_INFO_TRACE_THIS("DemoClient::SetVideoOutputFile, video file:" << pFileName);
    
	m_strVideoOutputFileName = pFileName;
    
	if(m_pVideoReceiver)
		return m_pVideoReceiver->SetVideoFile(pFileName);
    
	return WME_E_FAIL;

}

void DemoClient::SetVideoTarget(int nTarget)
{
    if(nTarget == 1)
		m_bUseVideoOutputFile = false;    //1 is screen
	else
		m_bUseVideoOutputFile = true;   //0 is file
    
	if(m_pVideoReceiver)
		return m_pVideoReceiver->SetVideoTarget(m_bUseVideoOutputFile);
}

long DemoClient::SetAudioInputFile(const char* pFileName, WmeAudioRawFormat *pFormat)
{
    if(!pFileName || !pFormat || strlen(pFileName) == 0)
    {
        CM_ERROR_TRACE_THIS("DemoClient::SetAudioInputFile, invalid audio file name or audio format!");
        return WME_E_INVALIDARG;
    }

    m_strAudioInputFileName = pFileName;
    m_sAudioRawFormat = *pFormat;

    if(m_pAudioSender)
        return m_pAudioSender->SetAudioInputFile(pFileName, pFormat);

    return WME_S_OK;
}

long DemoClient::SetAudioOutputFile(const char* pFileName)
{
    if(!pFileName || strlen(pFileName) == 0)
    {
        CM_ERROR_TRACE_THIS("DemoClient::SetAudioOutputFile, invalid audio file name!");
        return WME_E_INVALIDARG;
    }
    
    m_strAudioOuputFileName = pFileName;
    
    if(m_pAudioReceiver)
        return m_pAudioReceiver->SetAudioOutputFile(pFileName);
    
    return WME_S_OK;
}
#ifdef CUCUMBER_ENABLE
long DemoClient::SetAudioFileRenderSink(IWmeMediaFileRenderSink* pSink)
{
    if(!m_pAudioReceiver)
        return 0;
    return m_pAudioReceiver->SetFileRenderSink(pSink);
}

float DemoClient::CheckAudioOutputFile(char* pAudioOutputFileName, char* pAudioRefFileName)
{
   
    CM_INFO_TRACE("DemoClient::CheckAudioOutputFile Get_pesq_mos, audio ref file = "<<pAudioRefFileName
                  <<", audio deg file = "<<pAudioOutputFileName);
    
    float fRet;
    int n = Get_pesq_mos(pAudioRefFileName, pAudioOutputFileName, 16000, fRet);
    
    CM_INFO_TRACE("DemoClient::CheckAudioOutputFile Get_pesq_mos n = "<< fRet);
    return fRet;
}

bool DemoClient::CheckVideoOutputFile()
{
    return true;
}

bool DemoClient::AddVideoFileRenderSink()
{
#ifdef ENABLE_ZBAR
    CM_INFO_TRACE("DemoClient::AddVideoFileRenderSink m_pVideoReceiver = "<< m_pVideoReceiver);
    if(!m_pVideoReceiver)
        return false;
    IWmeMediaFileRenderSink* pSink = new CWMEMediaFileRenderSink();
    m_pVideoReceiver->AddFileRenderSink(pSink);
#endif
    return true;
}
#endif

void DemoClient::SetAudioSource(int nSource)
{
    if(nSource == 1)    //1 is file
        m_bUseAudioInputFile = TRUE;
    else                //0 is mic
        m_bUseAudioInputFile = FALSE;

    if(m_pAudioSender)
        return m_pAudioSender->SetAudioSource(nSource);
}

void DemoClient::SetAudioTarget(int nTarget)
{
    if(nTarget == 1)    //1 is speaker
        m_bUseAudioOutputFile = false;
    else                //0 is file
        m_bUseAudioOutputFile = true;
    
    if(m_pAudioReceiver)
        return m_pAudioReceiver->SetAudioTarget(m_bUseAudioOutputFile);
}

long DemoClient::DisableSendingFilterFeedback()
{
	if (m_pWmeVideoSession != NULL)
	{
		bool enable = false;
		m_pWmeVideoSession->SetOption(WmeSessionOption_EnableSendingFilterFeedback, &enable, sizeof(enable));
	}
	m_bEnableSendingFilterFeedback = false;
	return WME_S_OK;
}

void DemoClient::SetSpeakerMute(bool bEnable)
{
    MuteAudio(DEV_TYPE_SPEAKER,bEnable);
}

void DemoClient::SetMicMute(bool bEnable)
{
    MuteAudio(DEV_TYPE_MIC,bEnable);
}

void DemoClient::SetQoSMaxLossRatio(float maxLossRatio)
{
    WmeSetMediaEngineOption(WmeOption_QoSMaxLossRatio, &maxLossRatio, sizeof(maxLossRatio));
}

void DemoClient::SetQoSMinBandwidth(int minBandwidth)
{
    unsigned int uMinBandwidth = (unsigned int)minBandwidth;
    WmeSetMediaEngineOption(WmeOption_QoSMinBandwidth, &uMinBandwidth, sizeof(uMinBandwidth));
}

void DemoClient::SetInitialBandwidth(int initBandwidth)
{
    unsigned int uInitBandwidth = (unsigned int)initBandwidth;
    if (m_pWmeVideoSession != NULL)
	{
		m_pWmeVideoSession->SetOption(WmeSessionOption_InitialBandwidth, &uInitBandwidth, sizeof(uInitBandwidth));
	}
}

//////////////////////////////////////////////////////////////////////////////// yhu2
// whsu
long DemoClient::CreateDeskShareSession()
{
    returnv_if_fail(m_pWmeEngine != NULL, WME_E_FAIL);

    if (m_pWmeDeskShareSession != NULL) {
        CM_INFO_TRACE_THIS("DemoClient::CreateDSSession, m_pWmeDSSession is not NULL!");
        return WME_S_OK;
    }

    long ret = m_pWmeEngine->CreateMediaSession(WmeSessionType_ScreenShare, &m_pWmeDeskShareSession);
    if(WME_S_OK != ret)
    {
        CM_ERROR_TRACE_THIS("DemoClient::CreateDSSession, create DS session failed! error = " << ret);
        return WME_S_FALSE;
    }
    
    bool bEnable = true;
    m_pWmeDeskShareSession->SetOption(WmeSessionOption_EnableRTCP, &bEnable, sizeof(bool));
    WmePacketizationMode eWmePacketizationMode = WmePacketizationMode_1;
    m_pWmeDeskShareSession->SetOption(WmeSessionOption_PacketizationMode, &eWmePacketizationMode, sizeof(WmePacketizationMode));
#ifdef ENABLE_QOS
//    bEnable = true;
//    m_pWmeDeskShareSession->SetOption(WmeSessionOption_EnableQOS, &bEnable, sizeof(bool));
#endif
    
    WmeRTPExtension rtpExtension;
    rtpExtension.sExtURI = (char *)"http://protocols.cisco.com/framemarking";
    rtpExtension.uExtID = 1;
    m_pWmeDeskShareSession->SetOption(WmeSessionOption_RTPExtension, &rtpExtension, sizeof(WmeRTPExtension));

#if 0
    m_pWmeVideoSession->RegisterRTPExtensionProcessFunc(pRTPExtension_Time, AddTimeToRTPExtension, GetTimeFromRTPExtension);
    WmeRTPExtension myRTPExtension;
    myRTPExtension.sExtURI = pRTPExtension_Time;
    myRTPExtension.uExtID = nRTPExtension_Time;
    m_pWmeVideoSession->SetOption(WmeSessionOption_RTPExtension, &myRTPExtension, sizeof(WmeRTPExtension));
#endif
    
    int videoCodecMapLen = sizeof(kVideoCodecMap) / sizeof (WmeCodecPayloadTypeMap);
    for (int k=0; k < videoCodecMapLen; k++) {
        m_pWmeDeskShareSession->SetOption(WmeSessionOption_CodecPayloadTypeMap, (void *)&(kVideoCodecMap[k]), sizeof(WmeCodecPayloadTypeMap));
    }
    return WME_S_OK;
}

long DemoClient::DeleteDeskShareSession()
{
    SAFE_RELEASE(m_pWmeDeskShareSession);
    return WME_S_OK;
}

long DemoClient::CreateDeskShareTransport()
{
    returnv_if_fail(m_pWmeDeskShareSession != NULL, WME_E_FAIL);

    if (m_pDeskShareTransport) {
        CM_INFO_TRACE_THIS("DemoClient::CreateDeskShareTransport, m_pDeskShareTransport has existed!");
        return WME_S_OK;
    }
    
    m_pDeskShareTransport = new MediaTransport();
    returnv_if_fail(m_pDeskShareTransport != NULL, WME_E_FAIL);
    m_pDeskShareTransport->SetSink(this, CONN_SINK_DESKSHARE);
    m_pDeskShareTransport->SetSession(m_pWmeDeskShareSession);

    return WME_S_OK;
}
long DemoClient::CreateDeskShareSender()
{
    returnv_if_fail(m_pWmeEngine != NULL, WME_E_FAIL);
    returnv_if_fail(m_pDeskShareTransport != NULL, WME_E_FAIL);
    returnv_if_fail(m_pWmeDeskShareSession != NULL, WME_E_FAIL);

    if (m_pDeskShareSender) {
        CM_INFO_TRACE_THIS("DemoClient::CreateDeskShareSender, m_pDeskShareSender has existed!");
        return WME_S_OK;
    }

    m_pDeskShareSender = new DeskShareSender();
    returnv_if_fail(m_pDeskShareSender != NULL, WME_E_FAIL);
    m_pDeskShareSender->SetMediaEngine(m_pWmeEngine);
    m_pDeskShareSender->SetMediaSession(m_pWmeDeskShareSession);

    /// set MediaTransport into Sender(local track)
    m_pDeskShareSender->SetTransport(m_pDeskShareTransport);

    /// create local track
    if(m_pDeskShareSender->CreateTrack() != WME_S_OK) {
        CM_ERROR_TRACE_THIS("DemoClient::CreateDeskShareSender, fail to createTrack");
        return WME_E_FAIL;
    }

    /// default settings
    //if (m_pDeskShareManager) {
    //    IWmeMediaCodec *pCodec = NULL;
    //    m_pDeskShareManager->QueryMediaCodec(WmeCodecType_SVC, pCodec);
    //    if (pCodec) {
    //        m_pDeskShareSender->SetTrackCodec(pCodec);
    //    }
    //}
    m_pDeskShareSender->SetDefaultSettings();
    m_pDeskShareSender->SetRtpSession(WmeCodecType_AVC, 0);

    return WME_S_OK;
}

long DemoClient::DeleteDeskShareSender()
{
    if(m_pDeskShareSender) {
        m_pDeskShareSender->DeleteTrack();
        SAFE_DELETE(m_pDeskShareSender);
    }
    return WME_S_OK;
}

long DemoClient::CreateDeskShareReceiver()
{
    returnv_if_fail(m_pWmeEngine != NULL, WME_E_FAIL);
    returnv_if_fail(m_pWmeDeskShareSession != NULL, WME_E_FAIL);
    returnv_if_fail(m_pWmeSyncBox != NULL, WME_E_FAIL);
    returnv_if_fail(m_pDeskShareTransport != NULL, WME_E_FAIL);

    if (m_pDeskShareReceiver) {
        CM_INFO_TRACE_THIS("DemoClient::CreateDeskShareReceiver, m_pDeskShareReceiver has existed!");
        return WME_S_OK;
    }

    m_pDeskShareReceiver = new DeskShareReceiver();
    returnv_if_fail(m_pDeskShareReceiver != NULL, WME_E_FAIL);
    m_pDeskShareReceiver->SetMediaEngine(m_pWmeEngine);
    m_pDeskShareReceiver->SetMediaSession(m_pWmeDeskShareSession);
    m_pDeskShareReceiver->SetMediaSyncBox(m_pWmeSyncBox);
    m_pDeskShareReceiver->SetSink(this);

    /// set Receiver(remote track) into MediaTransport
    m_pDeskShareTransport->SetReceiver(m_pDeskShareReceiver);

    /// create remote track
    if(m_pDeskShareReceiver->CreateTrack() != WME_S_OK){
        CM_ERROR_TRACE_THIS("DemoClient::CreateDeskShareReceiver, fail to CreateTrack!");
        return WME_E_FAIL;
    }

    /// default settings
    //if (m_pDeskShareManager) {
    //    IWmeMediaCodec *pCodec = NULL;
    //    m_pDeskShareManager->QueryMediaCodec(WmeCodecType_SVC, pCodec);
    //    if (pCodec) {
    //        m_pDeskShareSender->SetTrackCodec(pCodec);
    //    }
    //}
    m_pDeskShareReceiver->SetRtpSession(WmeCodecType_SVC, 0);

    return WME_S_OK;
}

long DemoClient::DeleteDeskShareReceiver()
{
    if(m_pDeskShareReceiver) {
        m_pDeskShareReceiver->DeleteTrack();
        SAFE_DELETE(m_pDeskShareReceiver);
    }
    return WME_S_OK;
}

long DemoClient::CreateDeskShareSourceManager()
{
    
    returnv_if_fail(m_pWmeEngine != NULL, WME_E_FAIL);
    
    if (m_pShareSourceManager) {
        CM_INFO_TRACE_THIS("DemoClient::CreateDeskShareSourceManager, m_pShareSourceManager has existed!");
        return WME_S_OK;
    }
    
    m_pShareSourceManager = new ShareSourceManager();
    returnv_if_fail(m_pShareSourceManager != NULL, WME_E_FAIL);
    m_pShareSourceManager->SetMediaEngine(m_pWmeEngine);
    m_pShareSourceManager->SetSink(this);
    
    if(m_pShareSourceManager->Init() != WME_S_OK) {
        CM_ERROR_TRACE_THIS("DemoClient::CreateDeskShareSourceManager, fail to m_pShareSourceManager->Init()!");
        return WME_E_FAIL;
    }
    return WME_S_OK;
}
long DemoClient::DeleteDeskShareSourceManager()
{
    SAFE_DELETE(m_pShareSourceManager);
    return WME_S_OK;
}


long DemoClient::GetScreenStatistics(DEMO_TRACK_TYPE ttype,
                                     WmeScreenShareStatistics &wmeScreenShareStatistics)
{
    return WME_S_OK;
}


long DemoClient::GetScreenStatistics(WmeSessionStatistics &wmeStatistics)
{
    return WME_S_OK;
}

