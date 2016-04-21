#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
#include "WmeSession.h"

class MockImeMediaTransport: public wme::IWmeMediaTransport
{
public:
	MockImeMediaTransport(wme::WmeTransportType eTransportType)
	: m_eTransportType(eTransportType), m_pMediaSession(NULL), m_pMediaTrack(NULL)
	{
	}
    
	~MockImeMediaTransport()
	{
		if(m_pMediaSession)
		{
			m_pMediaSession->Release();
			m_pMediaSession = NULL;
		}
        
		if(m_pMediaTrack)
		{
			m_pMediaTrack->Release();
			m_pMediaTrack = NULL;
		}
	}
    
	WMERESULT GetTransportType(wme::WmeTransportType &eTransportType)
	{
		eTransportType = m_eTransportType;
		return WME_S_OK;
	}
    
	WMERESULT SendRTPPacket(wme::IWmeMediaPackage *pRTPPackage)
	{
		if(NULL == m_pMediaSession || NULL == m_pMediaTrack)
		{
			return WME_E_FAIL;
		}
        
		return m_pMediaSession->ReceiveRTPPacket(m_pMediaTrack, pRTPPackage);
	}
    
	WMERESULT SendRTCPPacket(wme::IWmeMediaPackage *pRTCPPackage)
	{
		if(NULL == m_pMediaSession)
		{
			return WME_E_FAIL;
		}
        
		return m_pMediaSession->ReceiveRTCPPacket(pRTCPPackage);
	}
    
	WMERESULT SetMediaSession(wme::IWmeMediaSession *pSession)
	{
		if(m_pMediaSession)
		{
			m_pMediaSession->Release();
			m_pMediaSession = NULL;
		}
        
		m_pMediaSession = pSession;
		if(m_pMediaSession)
		{
			m_pMediaSession->AddRef();
		}
        
		return WME_S_OK;
	}
    
	WMERESULT SetMediaTrack(wme::IWmeMediaTrack *pTrack)
	{
		if(m_pMediaTrack)
		{
			m_pMediaTrack->Release();
			m_pMediaTrack = NULL;
		}
        
		m_pMediaTrack = pTrack;
		if(m_pMediaTrack)
		{
			m_pMediaTrack->AddRef();
		}
        
		return WME_S_OK;
	}
    
protected:
	wme::WmeTransportType	m_eTransportType;
	wme::IWmeMediaSession	*m_pMediaSession;
	wme::IWmeMediaTrack		*m_pMediaTrack;
};

static char s_sAudioExtURI[] = "urn:ietf:params:rtp-hdrext:ssrc-audio-level";
static char s_sVideoExtURI[] = "http://protocols.cisco.com/framemarking";
static char s_sExtURI[1024] = "";
int32_t MockBuiltinRTPExtensionIterFunc(const char* sExtURI)
{
	if(NULL != sExtURI && strlen(sExtURI) < 1024)
	{
#ifdef WIN32
		strcpy_s(s_sExtURI, 1024, sExtURI);
#else
		strncpy(s_sExtURI, sExtURI, 1024);
#endif
	}
	return 0;
}

static uint8_t s_pMasterKeySalt[30];
static uint32_t s_uMasterKeySaltLength = 30;
static uint8_t s_pEKTKey[16];
static uint32_t s_uEKTKeyLength = 16;

static uint32_t s_uOpusClockRate = 48000;
static uint32_t s_uAvcClockRate = 90000;
static uint32_t s_uSvcClockRate = 90000;

int32_t MockExtensionBuildFunc(const char* sExtURI, /*wrtp::WRTPMediaData*/void* pData, uint8_t pExtBuffer[], uint32_t &uLength)
{
	return 0;
}

int32_t MockExtensionParseFunc(const char* sExtURI, uint8_t pExtBuffer[], uint32_t uLength, /*wrtp::WRTPMediaData*/void* pData)
{
	return 0;
}

class CWmeMediaSessionTest : public testing::Test
{
public:
	CWmeMediaSessionTest()
	{
		m_pEngine = NULL;
		m_pAudioSession = NULL;
		m_pVideoSession = NULL;
		m_pScreenSession = NULL;
		
        m_pLocalVideoTrack = NULL;
        m_pRemoteVideoTrack = NULL;
        m_pLocalAudioTrack = NULL;
        m_pRemoteAudioTrack = NULL;
        m_pLocalScreenTrack = NULL;
        m_pRemoteScreenTrack = NULL;
        
		if(WME_S_OK == wme::WmeCreateMediaEngine(&m_pEngine) && m_pEngine)
		{
			m_pEngine->CreateMediaSession(wme::WmeSessionType_Audio, &m_pAudioSession);
			m_pEngine->CreateMediaSession(wme::WmeSessionType_Video, &m_pVideoSession);
			m_pEngine->CreateMediaSession(wme::WmeSessionType_ScreenShare, &m_pScreenSession);
#ifndef IOS_SIMULATOR
			m_pEngine->CreateLocalVideoTrack(&m_pLocalVideoTrack);
			m_pEngine->CreateRemoteVideoTrack(&m_pRemoteVideoTrack);

			m_pEngine->CreateLocalAudioTrack(&m_pLocalAudioTrack);
			m_pEngine->CreateRemoteAudioTrack(&m_pRemoteAudioTrack);
#endif
            m_pEngine->CreateLocalScreenShareTrack(&m_pLocalScreenTrack);
			m_pEngine->CreateRemoteScreenShareTrack(&m_pRemoteScreenTrack);

		}
	}
    
	virtual ~CWmeMediaSessionTest()
	{
		if(m_pLocalVideoTrack)
		{
			m_pLocalVideoTrack->Release();
			m_pLocalVideoTrack = NULL;
		}
        
		if(m_pRemoteVideoTrack)
		{
			m_pRemoteVideoTrack->Release();
			m_pRemoteVideoTrack = NULL;
		}
        
		if(m_pLocalAudioTrack)
		{
			m_pLocalAudioTrack->Release();
			m_pLocalAudioTrack = NULL;
		}
        
		if(m_pRemoteAudioTrack)
		{
			m_pRemoteAudioTrack->Release();
			m_pRemoteAudioTrack = NULL;
		}
        
		if(m_pLocalScreenTrack)
		{
			m_pLocalScreenTrack->Release();
			m_pLocalScreenTrack = NULL;
		}
        
		if(m_pRemoteScreenTrack)
		{
			m_pRemoteScreenTrack->Release();
			m_pRemoteScreenTrack = NULL;
		}
		
		if(m_pVideoSession)
		{
			m_pVideoSession->Release();
			m_pVideoSession = NULL;
		}
        
		if(m_pAudioSession)
		{
			m_pAudioSession->Release();
			m_pAudioSession = NULL;
		}
        
		if(m_pScreenSession)
		{
			m_pScreenSession->Release();
			m_pScreenSession = NULL;
		}
		
		if(m_pEngine)
		{
			m_pEngine->Release();
			m_pEngine = NULL;
		}
	}
    
	virtual void SetUp() {}
	virtual void TearDown() {}
public:
	//member
	wme::IWmeMediaEngine* m_pEngine;
	wme::IWmeMediaSession* m_pVideoSession;
	wme::IWmeMediaSession* m_pAudioSession;
	wme::IWmeMediaSession* m_pScreenSession;
	wme::IWmeLocalVideoTrack *m_pLocalVideoTrack;
	wme::IWmeRemoteVideoTrack *m_pRemoteVideoTrack;
	wme::IWmeLocalAudioTrack *m_pLocalAudioTrack;
	wme::IWmeRemoteAudioTrack *m_pRemoteAudioTrack;
	wme::IWmeLocalScreenShareTrack *m_pLocalScreenTrack;
	wme::IWmeRemoteScreenShareTrack *m_pRemoteScreenTrack;
};

TEST_F(CWmeMediaSessionTest, CreateMediaSession)
{
	wme::IWmeMediaSession *pMediaSession = NULL;
    
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_Audio, &pMediaSession));
	pMediaSession->Release();
    
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_Video, &pMediaSession));
	pMediaSession->Release();
    
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_ScreenShare, &pMediaSession));
	pMediaSession->Release();
    
	EXPECT_NE(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_Audio, NULL));
	EXPECT_NE(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_Video, NULL));
	EXPECT_NE(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_ScreenShare, NULL));
    
}

TEST_F(CWmeMediaSessionTest, GetSessionType)
{
	wme::WmeSessionType eType;
    
	EXPECT_EQ(WME_S_OK, m_pAudioSession->GetSessionType(eType));
	EXPECT_EQ(wme::WmeSessionType_Audio, eType);
    
	EXPECT_EQ(WME_S_OK, m_pVideoSession->GetSessionType(eType));
	EXPECT_EQ(wme::WmeSessionType_Video, eType);
    
	EXPECT_EQ(WME_S_OK, m_pScreenSession->GetSessionType(eType));
	EXPECT_EQ(wme::WmeSessionType_ScreenShare, eType);
}

TEST_F(CWmeMediaSessionTest, SetOption_EnableRTCP)
{
	wme::WmeSessionOption eOption;
    
	// test WmeSessionOption_EnableRTCP
	eOption = wme::WmeSessionOption_EnableRTCP;
	bool EnableRTCP = true;
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &EnableRTCP, sizeof(bool)));
	EnableRTCP = false;
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, &EnableRTCP, 0));
    
	EnableRTCP = true;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &EnableRTCP, sizeof(bool)));
	EnableRTCP = false;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, &EnableRTCP, 0));
    
	EnableRTCP = true;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &EnableRTCP, sizeof(bool)));
	EnableRTCP = false;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, &EnableRTCP, 0));
}

TEST_F(CWmeMediaSessionTest, SetOption_EnableQOS)
{
	wme::WmeSessionOption eOption;
    
	// test WmeSessionOption_EnableQOS
	eOption = wme::WmeSessionOption_EnableQOS;
	bool EnableQOS = true;
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EnableQOS = false;
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, &EnableQOS, 0));
    
	EnableQOS = true;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EnableQOS = false;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, &EnableQOS, 0));
    
	EnableQOS = true;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EnableQOS = false;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, &EnableQOS, 0));
}

TEST_F(CWmeMediaSessionTest, SetOption_RTPFecInfo)
{
	wme::WmeSessionOption eOption;
    /*
     // test WmeSessionOption_RTPFecInfo
     eOption = wme::WmeSessionOption_RTPFecInfo;
     wme::WmeRsFssi stRsFssi = {10, 6};
     wme::WmeRTPFecIdGroup stRTPFecIdGroup[2] = {{1001, 2001}, {1002, 2002}};
     wme::WmeRTPFecInfo stRTPFecInfo = {1234567890, 111, wme::WmeRTPFecNone, 60, 10, stRTPFecIdGroup, 2, NULL};
     EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &stRTPFecInfo, sizeof(stRTPFecInfo)));
     stRTPFecInfo.eFecType = wme::WmeRTPFecRs;
     stRTPFecInfo.pExtraInfo = &stRsFssi;
     EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &stRTPFecInfo, sizeof(stRTPFecInfo)));
     stRTPFecInfo.eFecType = wme::WmeRTPFecXor;
     stRTPFecInfo.pExtraInfo = NULL;
     EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &stRTPFecInfo, sizeof(stRTPFecInfo)));
     EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, NULL, sizeof(stRTPFecInfo)));
     EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, &stRTPFecInfo, 0));
     
     stRTPFecInfo.eFecType = wme::WmeRTPFecNone;
     stRTPFecInfo.pExtraInfo = NULL;
     EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &stRTPFecInfo, sizeof(stRTPFecInfo)));
     stRTPFecInfo.eFecType = wme::WmeRTPFecRs;
     stRTPFecInfo.pExtraInfo = &stRsFssi;
     EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &stRTPFecInfo, sizeof(stRTPFecInfo)));
     stRTPFecInfo.eFecType = wme::WmeRTPFecXor;
     stRTPFecInfo.pExtraInfo = NULL;
     EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &stRTPFecInfo, sizeof(stRTPFecInfo)));
     EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, NULL, sizeof(stRTPFecInfo)));
     EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, &stRTPFecInfo, 0));
     */
}

TEST_F(CWmeMediaSessionTest, SetOption_RTPExtension)
{
	wme::WmeSessionOption eOption;
    
	// test WmeSessionOption_RTPExtension
	eOption = wme::WmeSessionOption_RTPExtension;
	wme::WmeRTPExtension stExtersion = {NULL, 0};
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, &stExtersion, sizeof(wme::WmeRTPExtension)));
	EXPECT_EQ(WME_S_OK, m_pAudioSession->EnumBuiltinRTPHeaderExtensions(MockBuiltinRTPExtensionIterFunc));
	stExtersion.sExtURI = s_sExtURI;
	stExtersion.uExtID = 1;
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &stExtersion, sizeof(wme::WmeRTPExtension)));
	stExtersion.sExtURI = s_sAudioExtURI;
	stExtersion.uExtID = 2;
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &stExtersion, sizeof(wme::WmeRTPExtension)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, NULL, sizeof(wme::WmeRTPExtension)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, &stExtersion, 0));
    
	stExtersion.sExtURI = NULL;
	stExtersion.uExtID = 0;
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, &stExtersion, sizeof(wme::WmeRTPExtension)));
	EXPECT_EQ(WME_S_OK, m_pVideoSession->EnumBuiltinRTPHeaderExtensions(MockBuiltinRTPExtensionIterFunc));
	stExtersion.sExtURI = s_sExtURI;
	stExtersion.uExtID = 1;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &stExtersion, sizeof(wme::WmeRTPExtension)));
	stExtersion.sExtURI = s_sVideoExtURI;
	stExtersion.uExtID = 2;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &stExtersion, sizeof(wme::WmeRTPExtension)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, NULL, sizeof(wme::WmeRTPExtension)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, &stExtersion, 0));
    
	stExtersion.sExtURI = NULL;
	stExtersion.uExtID = 0;
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, &stExtersion, sizeof(wme::WmeRTPExtension)));
	EXPECT_EQ(WME_S_OK, m_pScreenSession->EnumBuiltinRTPHeaderExtensions(MockBuiltinRTPExtensionIterFunc));
	stExtersion.sExtURI = s_sExtURI;
	stExtersion.uExtID = 1;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &stExtersion, sizeof(wme::WmeRTPExtension)));
	stExtersion.sExtURI = s_sVideoExtURI;
	stExtersion.uExtID = 2;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &stExtersion, sizeof(wme::WmeRTPExtension)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, NULL, sizeof(wme::WmeRTPExtension)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, &stExtersion, 0));
}

TEST_F(CWmeMediaSessionTest, SetOption_CodecPayloadTypeMap)
{
	wme::WmeSessionOption eOption;
    
	// test WmeSessionOption_CodecPayloadTypeMap
	eOption = wme::WmeSessionOption_CodecPayloadTypeMap;
	wme::WmeCodecPayloadTypeMap stCodecPayloadTypeMap = {wme::WmeCodecType_OPUS, 1, s_uOpusClockRate};
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));
    
	stCodecPayloadTypeMap.eCodecType = wme::WmeCodecType_SVC;
	stCodecPayloadTypeMap.uPayloadType = 98;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));
    
	stCodecPayloadTypeMap.eCodecType = wme::WmeCodecType_AVC;
	stCodecPayloadTypeMap.uPayloadType = 96;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));
}


TEST_F(CWmeMediaSessionTest, SetOption_CodecPayloadTypeMap_Ext_Default)
{
	wme::WmeSessionOption eOption;

	// test WmeSessionOption_CodecPayloadTypeMap
	eOption = wme::WmeSessionOption_CodecPayloadTypeMap_Ext;
	wme::WmeCodecPayloadTypeMap_Ext stCodecPayloadTypeMap = { { wme::WmeCodecType_OPUS, 1, s_uOpusClockRate }, wme::STREAM_OUT };
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));

	stCodecPayloadTypeMap.wmecptMap.eCodecType = wme::WmeCodecType_SVC;
	stCodecPayloadTypeMap.wmecptMap.uPayloadType = 98;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));

	stCodecPayloadTypeMap.wmecptMap.eCodecType = wme::WmeCodecType_AVC;
	stCodecPayloadTypeMap.wmecptMap.uPayloadType = 96;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));
}

TEST_F(CWmeMediaSessionTest, SetOption_CodecPayloadTypeMap_Ext_INOUT)
{
	wme::WmeSessionOption eOption;

	// test WmeSessionOption_CodecPayloadTypeMap
	eOption = wme::WmeSessionOption_CodecPayloadTypeMap_Ext;
	wme::WmeCodecPayloadTypeMap_Ext stCodecPayloadTypeMap = {{wme::WmeCodecType_OPUS, 1, s_uOpusClockRate},wme::STREAM_INOUT};
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));

	stCodecPayloadTypeMap.wmecptMap.eCodecType = wme::WmeCodecType_SVC;
	stCodecPayloadTypeMap.wmecptMap.uPayloadType = 98;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));

	stCodecPayloadTypeMap.wmecptMap.eCodecType = wme::WmeCodecType_AVC;
	stCodecPayloadTypeMap.wmecptMap.uPayloadType = 96;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));
}


TEST_F(CWmeMediaSessionTest, SetOption_CodecPayloadTypeMap_Ext_IN)
{
	wme::WmeSessionOption eOption;

	// test WmeSessionOption_CodecPayloadTypeMap
	eOption = wme::WmeSessionOption_CodecPayloadTypeMap_Ext;
	wme::WmeCodecPayloadTypeMap_Ext stCodecPayloadTypeMap = {{wme::WmeCodecType_OPUS, 1, s_uOpusClockRate},wme::STREAM_IN};
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));

	stCodecPayloadTypeMap.wmecptMap.eCodecType = wme::WmeCodecType_SVC;
	stCodecPayloadTypeMap.wmecptMap.uPayloadType = 98;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));

	stCodecPayloadTypeMap.wmecptMap.eCodecType = wme::WmeCodecType_AVC;
	stCodecPayloadTypeMap.wmecptMap.uPayloadType = 96;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));
}


TEST_F(CWmeMediaSessionTest, SetOption_CodecPayloadTypeMap_Ext_OUT)
{
	wme::WmeSessionOption eOption;

	// test WmeSessionOption_CodecPayloadTypeMap
	eOption = wme::WmeSessionOption_CodecPayloadTypeMap_Ext;
	wme::WmeCodecPayloadTypeMap_Ext stCodecPayloadTypeMap = { { wme::WmeCodecType_OPUS, 1, s_uOpusClockRate }, wme::STREAM_OUT };
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));

	stCodecPayloadTypeMap.wmecptMap.eCodecType = wme::WmeCodecType_SVC;
	stCodecPayloadTypeMap.wmecptMap.uPayloadType = 98;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));

	stCodecPayloadTypeMap.wmecptMap.eCodecType = wme::WmeCodecType_AVC;
	stCodecPayloadTypeMap.wmecptMap.uPayloadType = 96;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap_Ext)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, &stCodecPayloadTypeMap, 0));
}

TEST_F(CWmeMediaSessionTest, SetOption_SecurityConfigurationp)
{
	wme::WmeSessionOption eOption;
    
	// test WmeSessionOption_SecurityConfiguration
	eOption = wme::WmeSessionOption_SecurityConfiguration;
	wme::WmeEKTConfiguration stEKTConfiguration = {wme::WmeEKTCipherType_AESKW_128, s_pEKTKey, s_uEKTKeyLength, 1};
	wme::WmeSecurityConfiguration stInboundSecurityConfiguration = {wme::WmeSecurityDirection_InBound,
        wme::WmeCryptoSuiteType_AES_CM_128_HMAC_SHA1_80,
        s_pMasterKeySalt,
        s_uMasterKeySaltLength,
        wme::WmeSecurityService_All,
        wme::WmeSecurityService_All,
        wme::WmeSecurityFECOrder_SRTP_FEC,
        NULL};
	wme::WmeSecurityConfiguration stOutboundSecurityConfiguration = {wme::WmeSecurityDirection_OutBound,
        wme::WmeCryptoSuiteType_AES_CM_128_HMAC_SHA1_80,
        s_pMasterKeySalt,
        s_uMasterKeySaltLength,
        wme::WmeSecurityService_All,
        wme::WmeSecurityService_All,
        wme::WmeSecurityFECOrder_SRTP_FEC,
        NULL};
#ifdef ENABLE_SECURITY
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &stInboundSecurityConfiguration, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &stOutboundSecurityConfiguration, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &stInboundSecurityConfiguration, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &stOutboundSecurityConfiguration, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &stInboundSecurityConfiguration, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &stOutboundSecurityConfiguration, sizeof(wme::WmeSecurityConfiguration)));
    
	stInboundSecurityConfiguration.pEKTConfiguration = &stEKTConfiguration;
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &stInboundSecurityConfiguration, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &stOutboundSecurityConfiguration, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &stInboundSecurityConfiguration, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &stOutboundSecurityConfiguration, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &stInboundSecurityConfiguration, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &stOutboundSecurityConfiguration, sizeof(wme::WmeSecurityConfiguration)));
#endif
	
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, NULL, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->SetOption(eOption, &stInboundSecurityConfiguration, 0));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, NULL, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(eOption, &stInboundSecurityConfiguration, 0));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, NULL, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->SetOption(eOption, &stInboundSecurityConfiguration, 0));
}

TEST_F(CWmeMediaSessionTest, SetOption_InitialBandwidth)
{
    uint32_t uQueriedBandwidth = 0;
    EXPECT_EQ(WME_S_OK, m_pVideoSession->GetOption(wme::WmeSessionOption_InitialBandwidth, &uQueriedBandwidth, sizeof(uQueriedBandwidth)));
    
    
	uint32_t uBandwidth = 8*1024*1024;  //bytes per second
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_InitialBandwidth, &uBandwidth, sizeof(uBandwidth)));
    EXPECT_EQ(WME_S_OK, m_pVideoSession->GetOption(wme::WmeSessionOption_InitialBandwidth, &uQueriedBandwidth, sizeof(uQueriedBandwidth)));
    EXPECT_EQ(uBandwidth, uQueriedBandwidth);
    
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_InitialBandwidth, NULL, sizeof(uBandwidth)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_InitialBandwidth, &uBandwidth, 0));
}

TEST_F(CWmeMediaSessionTest, SetOption_MaxPacketSize)
{
	uint32_t uMaxPacketSize = 1200;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_MaxPacketSize, &uMaxPacketSize, sizeof(uMaxPacketSize)));
    
	uMaxPacketSize = 0;
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_MaxPacketSize, &uMaxPacketSize, sizeof(uMaxPacketSize)));
    
	uMaxPacketSize = 1200;
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_MaxPacketSize, NULL, sizeof(uMaxPacketSize)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_MaxPacketSize, &uMaxPacketSize, 0));
}

TEST_F(CWmeMediaSessionTest, SetOption_PacketizationMode)
{
	wme::WmePacketizationMode eMode = wme::WmePacketizationMode_0;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_PacketizationMode, &eMode, sizeof(eMode)));
	eMode = wme::WmePacketizationMode_1;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_PacketizationMode, &eMode, sizeof(eMode)));
    
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_PacketizationMode, NULL, sizeof(eMode)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_PacketizationMode, &eMode, 0));
}

TEST_F(CWmeMediaSessionTest, GetOption_EnableRTCP)
{
	wme::WmeSessionOption eOption;
    
	// test WmeSessionOption_EnableRTCP
	eOption = wme::WmeSessionOption_EnableRTCP;
	bool EnableRTCP = true;
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pAudioSession->GetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_EQ(EnableRTCP, true);
	EnableRTCP = false;
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pAudioSession->GetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_EQ(EnableRTCP, false);
	EXPECT_NE(WME_S_OK, m_pAudioSession->GetOption(eOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->GetOption(eOption, &EnableRTCP, 0));
    
	EnableRTCP = true;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pVideoSession->GetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_EQ(EnableRTCP, true);
	EnableRTCP = false;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pVideoSession->GetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_EQ(EnableRTCP, false);
	EXPECT_NE(WME_S_OK, m_pVideoSession->GetOption(eOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->GetOption(eOption, &EnableRTCP, 0));
    
	EnableRTCP = true;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pScreenSession->GetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_EQ(EnableRTCP, true);
	EnableRTCP = false;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pScreenSession->GetOption(eOption, &EnableRTCP, sizeof(bool)));
	EXPECT_EQ(EnableRTCP, false);
	EXPECT_NE(WME_S_OK, m_pScreenSession->GetOption(eOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->GetOption(eOption, &EnableRTCP, 0));
}

TEST_F(CWmeMediaSessionTest, SetOption_MaxSendingDelay)
{
    wme::WmeSessionOption eOption;
    
    // test WmeSessionOption_MaxSendingDelay
    eOption = wme::WmeSessionOption_MaxSendingDelay;
    uint32_t maxDelay = 1500;
    EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &maxDelay, sizeof(uint32_t)));
}

TEST_F(CWmeMediaSessionTest, GetOption_EnableQOS)
{
	wme::WmeSessionOption eOption;
    
	// test WmeSessionOption_EnableQOS
	eOption = wme::WmeSessionOption_EnableQOS;
	bool EnableQOS = true;
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pAudioSession->GetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(EnableQOS, true);
	EnableQOS = false;
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pAudioSession->GetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(EnableQOS, false);
	EXPECT_EQ(WME_S_OK, m_pAudioSession->GetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->GetOption(eOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->GetOption(eOption, &EnableQOS, 0));
    
	EnableQOS = true;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pVideoSession->GetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(EnableQOS, true);
	EnableQOS = false;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pVideoSession->GetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(EnableQOS, false);
	EXPECT_EQ(WME_S_OK, m_pVideoSession->GetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->GetOption(eOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->GetOption(eOption, &EnableQOS, 0));
    
	EnableQOS = true;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pScreenSession->GetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(EnableQOS, true);
	EnableQOS = false;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pScreenSession->GetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(EnableQOS, false);
	EXPECT_EQ(WME_S_OK, m_pScreenSession->GetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->GetOption(eOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->GetOption(eOption, &EnableQOS, 0));
}

TEST_F(CWmeMediaSessionTest, GetOption_RTPFecInfo)
{
	wme::WmeSessionOption eOption;
    /*
     // test WmeSessionOption_RTPFecInfo
     eOption = wme::WmeSessionOption_RTPFecInfo;
     wme::WmeRTPFecInfo stRTPFecInfo = {0};
     EXPECT_NE(WME_S_OK, m_pVideoSession->GetOption(eOption, NULL, sizeof(stRTPFecInfo)));
     EXPECT_NE(WME_S_OK, m_pVideoSession->GetOption(eOption, &stRTPFecInfo, 0));
     
     EXPECT_NE(WME_S_OK, m_pScreenSession->GetOption(eOption, NULL, sizeof(stRTPFecInfo)));
     EXPECT_NE(WME_S_OK, m_pScreenSession->GetOption(eOption, &stRTPFecInfo, 0));
     */
}

TEST_F(CWmeMediaSessionTest, GetOption_RTPExtension)
{
	wme::WmeSessionOption eOption;
    
	// test WmeSessionOption_RTPExtension
	eOption = wme::WmeSessionOption_RTPExtension;
	wme::WmeRTPExtension stExtersion = {NULL, 0};
	EXPECT_NE(WME_S_OK, m_pAudioSession->GetOption(eOption, NULL, sizeof(wme::WmeRTPExtension)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->GetOption(eOption, &stExtersion, 0));
    
	EXPECT_NE(WME_S_OK, m_pVideoSession->GetOption(eOption, NULL, sizeof(wme::WmeRTPExtension)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->GetOption(eOption, &stExtersion, 0));
    
	EXPECT_NE(WME_S_OK, m_pScreenSession->GetOption(eOption, NULL, sizeof(wme::WmeRTPExtension)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->GetOption(eOption, &stExtersion, 0));
}

TEST_F(CWmeMediaSessionTest, GetOption_CodecPayloadTypeMap)
{
	wme::WmeSessionOption eOption;
    
	// test WmeSessionOption_CodecPayloadTypeMap
	eOption = wme::WmeSessionOption_CodecPayloadTypeMap;
	wme::WmeCodecPayloadTypeMap stCodecPayloadTypeMap = {wme::WmeCodecType_Unknown, 0, 0};
	EXPECT_NE(WME_S_OK, m_pAudioSession->GetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->GetOption(eOption, &stCodecPayloadTypeMap, 0));
    
	EXPECT_NE(WME_S_OK, m_pVideoSession->GetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->GetOption(eOption, &stCodecPayloadTypeMap, 0));
    
	EXPECT_NE(WME_S_OK, m_pScreenSession->GetOption(eOption, NULL, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->GetOption(eOption, &stCodecPayloadTypeMap, 0));
}

TEST_F(CWmeMediaSessionTest, GetOption_SecurityConfiguration)
{
	wme::WmeSessionOption eOption;
    
	// test WmeSessionOption_SecurityConfiguration
	eOption = wme::WmeSessionOption_SecurityConfiguration;
	wme::WmeSecurityConfiguration stSecurityConfiguration;
    
	EXPECT_NE(WME_S_OK, m_pAudioSession->GetOption(eOption, NULL, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_NE(WME_S_OK, m_pAudioSession->GetOption(eOption, &stSecurityConfiguration, 0));
    
	EXPECT_NE(WME_S_OK, m_pVideoSession->GetOption(eOption, NULL, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_NE(WME_S_OK, m_pVideoSession->GetOption(eOption, &stSecurityConfiguration, 0));
    
	EXPECT_NE(WME_S_OK, m_pScreenSession->GetOption(eOption, NULL, sizeof(wme::WmeSecurityConfiguration)));
	EXPECT_NE(WME_S_OK, m_pScreenSession->GetOption(eOption, &stSecurityConfiguration, 0));
}

TEST_F(CWmeMediaSessionTest, AddTrack)
{
    
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pLocalVideoTrack);
    EXPECT_NE(NULL, (long)m_pRemoteVideoTrack);
    EXPECT_NE(NULL, (long)m_pLocalAudioTrack);
    EXPECT_NE(NULL, (long)m_pRemoteAudioTrack);
    EXPECT_NE(NULL, (long)m_pLocalScreenTrack);
    EXPECT_NE(NULL, (long)m_pRemoteScreenTrack);
    
#endif
    
    if (m_pLocalVideoTrack) {
        m_pLocalVideoTrack->SetTrackLabel(100);
    }
    if (m_pRemoteVideoTrack) {
        m_pRemoteVideoTrack->SetTrackLabel(101);
    }
    if (m_pLocalAudioTrack) {
        m_pLocalAudioTrack->SetTrackLabel(200);
    }
    if (m_pRemoteAudioTrack) {
        m_pRemoteAudioTrack->SetTrackLabel(201);
    }
    if (m_pLocalScreenTrack) {
        m_pLocalScreenTrack->SetTrackLabel(300);
    }
    if (m_pRemoteScreenTrack) {
        m_pRemoteScreenTrack->SetTrackLabel(301);
    }
	
	wme::WmeCodecPayloadTypeMap stVideoCodecPayloadTypeMap = {wme::WmeCodecType_SVC, 98, s_uSvcClockRate};
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stVideoCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    
	wme::WmeCodecPayloadTypeMap stAudioCodecPayloadTypeMap = {wme::WmeCodecType_OPUS, 1, s_uOpusClockRate};
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stAudioCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    
	wme::WmeCodecPayloadTypeMap stScreenCodecPayloadTypeMap = {wme::WmeCodecType_AVC, 96, s_uAvcClockRate};
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stScreenCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    if (m_pLocalVideoTrack) {
        EXPECT_EQ(WME_S_OK, m_pVideoSession->AddTrack(m_pLocalVideoTrack));
    }
    if (m_pRemoteVideoTrack) {
        EXPECT_EQ(WME_S_OK, m_pVideoSession->AddTrack(m_pRemoteVideoTrack));
    }
	EXPECT_NE(WME_S_OK, m_pVideoSession->AddTrack(NULL));
	
    if (m_pLocalAudioTrack) {
        EXPECT_EQ(WME_S_OK, m_pAudioSession->AddTrack(m_pLocalAudioTrack));
    }
    if (m_pRemoteAudioTrack) {
        EXPECT_EQ(WME_S_OK, m_pAudioSession->AddTrack(m_pRemoteAudioTrack));
    }
	EXPECT_NE(WME_S_OK, m_pAudioSession->AddTrack(NULL));
    if (m_pLocalScreenTrack) {
        EXPECT_EQ(WME_S_OK, m_pScreenSession->AddTrack(m_pLocalScreenTrack));
    }
    if (m_pRemoteScreenTrack) {
        EXPECT_EQ(WME_S_OK, m_pScreenSession->AddTrack(m_pRemoteScreenTrack));
    }
	EXPECT_NE(WME_S_OK, m_pScreenSession->AddTrack(NULL));
}

TEST_F(CWmeMediaSessionTest, RemoveTrack)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pLocalVideoTrack);
    EXPECT_NE(NULL, (long)m_pRemoteVideoTrack);
    EXPECT_NE(NULL, (long)m_pLocalAudioTrack);
    EXPECT_NE(NULL, (long)m_pRemoteAudioTrack);
    EXPECT_NE(NULL, (long)m_pLocalScreenTrack);
    EXPECT_NE(NULL, (long)m_pRemoteScreenTrack);
#endif
    
    if (m_pLocalVideoTrack) {
        m_pLocalVideoTrack->SetTrackLabel(100);
    }
    if (m_pRemoteVideoTrack) {
        m_pRemoteVideoTrack->SetTrackLabel(101);
    }
    if (m_pLocalAudioTrack) {
        m_pLocalAudioTrack->SetTrackLabel(200);
    }
    if (m_pRemoteAudioTrack) {
        m_pRemoteAudioTrack->SetTrackLabel(201);
    }
    if (m_pLocalScreenTrack) {
        m_pLocalScreenTrack->SetTrackLabel(300);
    }
    if (m_pRemoteScreenTrack) {
        m_pRemoteScreenTrack->SetTrackLabel(301);
    }
    
	wme::WmeCodecPayloadTypeMap stVideoCodecPayloadTypeMap = {wme::WmeCodecType_SVC, 98, s_uSvcClockRate};
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stVideoCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    
	wme::WmeCodecPayloadTypeMap stAudioCodecPayloadTypeMap = {wme::WmeCodecType_OPUS, 1, s_uOpusClockRate};
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stAudioCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    
	wme::WmeCodecPayloadTypeMap stScreenCodecPayloadTypeMap = {wme::WmeCodecType_AVC, 96, s_uAvcClockRate};
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stScreenCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    if (m_pLocalVideoTrack) {
        EXPECT_EQ(WME_S_OK, m_pVideoSession->AddTrack(m_pLocalVideoTrack));
    }
    if (m_pRemoteVideoTrack) {
        EXPECT_EQ(WME_S_OK, m_pVideoSession->AddTrack(m_pRemoteVideoTrack));
    }
    if (m_pLocalVideoTrack) {
        EXPECT_EQ(WME_S_OK, m_pVideoSession->RemoveTrack(m_pLocalVideoTrack));
    }
    if (m_pRemoteVideoTrack) {
        EXPECT_EQ(WME_S_OK, m_pVideoSession->RemoveTrack(m_pRemoteVideoTrack));
    }
	EXPECT_NE(WME_S_OK, m_pVideoSession->RemoveTrack(NULL));
    if (m_pLocalAudioTrack) {
        EXPECT_EQ(WME_S_OK, m_pAudioSession->AddTrack(m_pLocalAudioTrack));
    }
    if (m_pRemoteAudioTrack) {
        EXPECT_EQ(WME_S_OK, m_pAudioSession->AddTrack(m_pRemoteAudioTrack));
    }
    if (m_pLocalAudioTrack) {
        EXPECT_EQ(WME_S_OK, m_pAudioSession->RemoveTrack(m_pLocalAudioTrack));
    }
    if (m_pRemoteAudioTrack) {
        EXPECT_EQ(WME_S_OK, m_pAudioSession->RemoveTrack(m_pRemoteAudioTrack));
    }
	EXPECT_NE(WME_S_OK, m_pAudioSession->RemoveTrack(NULL));
    if (m_pLocalScreenTrack) {
        EXPECT_EQ(WME_S_OK, m_pScreenSession->AddTrack(m_pLocalScreenTrack));
    }
    if (m_pRemoteScreenTrack) {
        EXPECT_EQ(WME_S_OK, m_pScreenSession->AddTrack(m_pRemoteScreenTrack));
    }
    if (m_pLocalScreenTrack) {
        EXPECT_EQ(WME_S_OK, m_pScreenSession->RemoveTrack(m_pLocalScreenTrack));
    }
    if (m_pRemoteScreenTrack) {
        EXPECT_EQ(WME_S_OK, m_pScreenSession->RemoveTrack(m_pRemoteScreenTrack));
    }
    
	EXPECT_NE(WME_S_OK, m_pScreenSession->RemoveTrack(NULL));
}

TEST_F(CWmeMediaSessionTest, SetTransport)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pLocalVideoTrack);
    EXPECT_NE(NULL, (long)m_pRemoteVideoTrack);
    EXPECT_NE(NULL, (long)m_pLocalAudioTrack);
    EXPECT_NE(NULL, (long)m_pRemoteAudioTrack);
    EXPECT_NE(NULL, (long)m_pLocalScreenTrack);
    EXPECT_NE(NULL, (long)m_pRemoteScreenTrack);
#endif
    
    if (m_pLocalVideoTrack) {
        m_pLocalVideoTrack->SetTrackLabel(100);
    }
    if (m_pRemoteVideoTrack) {
        m_pRemoteVideoTrack->SetTrackLabel(100);
    }
    if (m_pLocalAudioTrack) {
        m_pLocalAudioTrack->SetTrackLabel(200);
    }
    if (m_pRemoteAudioTrack) {
        m_pRemoteAudioTrack->SetTrackLabel(200);
    }
    if (m_pLocalScreenTrack) {
        m_pLocalScreenTrack->SetTrackLabel(300);
    }
    if (m_pRemoteScreenTrack) {
        m_pRemoteScreenTrack->SetTrackLabel(300);
    }
    
	wme::IWmeMediaSession	*m_pAudioReceiveSession = NULL;
	wme::IWmeMediaSession	*m_pVideoReceiveSession = NULL;
	wme::IWmeMediaSession	*m_pScreenReceiveSession = NULL;
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_Audio, &m_pAudioReceiveSession));
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_Video, &m_pVideoReceiveSession));
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_ScreenShare, &m_pScreenReceiveSession));
    
	wme::WmeCodecPayloadTypeMap stVideoCodecPayloadTypeMap = {wme::WmeCodecType_SVC, 98, s_uSvcClockRate};
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stVideoCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_EQ(WME_S_OK, m_pVideoReceiveSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stVideoCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    
	wme::WmeCodecPayloadTypeMap stAudioCodecPayloadTypeMap = {wme::WmeCodecType_OPUS, 1, s_uOpusClockRate};
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stAudioCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_EQ(WME_S_OK, m_pAudioReceiveSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stAudioCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    
	wme::WmeCodecPayloadTypeMap stScreenCodecPayloadTypeMap = {wme::WmeCodecType_AVC, 96, s_uAvcClockRate};
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stScreenCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_EQ(WME_S_OK, m_pScreenReceiveSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stScreenCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    
    if (m_pLocalVideoTrack) {
        EXPECT_EQ(WME_S_OK, m_pVideoSession->AddTrack(m_pLocalVideoTrack));
    }
    if (m_pRemoteVideoTrack) {
        EXPECT_EQ(WME_S_OK, m_pVideoReceiveSession->AddTrack(m_pRemoteVideoTrack));
    }
    if (m_pLocalAudioTrack) {
        EXPECT_EQ(WME_S_OK, m_pAudioSession->AddTrack(m_pLocalAudioTrack));
    }
    if (m_pRemoteAudioTrack) {
        EXPECT_EQ(WME_S_OK, m_pAudioReceiveSession->AddTrack(m_pRemoteAudioTrack));
    }
    if (m_pLocalScreenTrack) {
        EXPECT_EQ(WME_S_OK, m_pScreenSession->AddTrack(m_pLocalScreenTrack));
    }
    if (m_pRemoteScreenTrack) {
        EXPECT_EQ(WME_S_OK, m_pScreenReceiveSession->AddTrack(m_pRemoteScreenTrack));
    }
    
    if (m_pLocalVideoTrack) {
        m_pLocalVideoTrack->Start();
    }
    if (m_pRemoteVideoTrack) {
        m_pRemoteVideoTrack->Start();
    }
    if (m_pLocalAudioTrack) {
        m_pLocalAudioTrack->Start();
    }
    if (m_pRemoteAudioTrack) {
        m_pRemoteAudioTrack->Start();
    }
    if (m_pLocalScreenTrack) {
        m_pLocalScreenTrack->Start();
    }
    if (m_pRemoteScreenTrack) {
        m_pRemoteScreenTrack->Start();
    }
	
	MockImeMediaTransport audioTransport_UDP(wme::WmeTransportType_UDP), audioTransport_TCP(wme::WmeTransportType_TCP);
	MockImeMediaTransport videoTransport_UDP(wme::WmeTransportType_UDP), videoTransport_TCP(wme::WmeTransportType_TCP);
	MockImeMediaTransport screenTransport_UDP(wme::WmeTransportType_UDP), screenTransport_TCP(wme::WmeTransportType_TCP);
	
	// test UDP transport
	audioTransport_UDP.SetMediaSession(m_pAudioReceiveSession);
	audioTransport_UDP.SetMediaTrack(m_pRemoteAudioTrack);
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetTransport(&audioTransport_UDP));
    
	videoTransport_UDP.SetMediaSession(m_pVideoReceiveSession);
	videoTransport_UDP.SetMediaTrack(m_pRemoteVideoTrack);
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetTransport(&videoTransport_UDP));
    
	screenTransport_UDP.SetMediaSession(m_pScreenReceiveSession);
	screenTransport_UDP.SetMediaTrack(m_pRemoteScreenTrack);
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetTransport(&screenTransport_UDP));
    
#ifdef WIN32
	Sleep(1000);
#else
	usleep(1000000);
#endif
	
	// test TCP transport
	audioTransport_TCP.SetMediaSession(m_pAudioReceiveSession);
	audioTransport_TCP.SetMediaTrack(m_pRemoteAudioTrack);
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetTransport(&audioTransport_TCP));
    
	videoTransport_TCP.SetMediaSession(m_pVideoReceiveSession);
	videoTransport_TCP.SetMediaTrack(m_pRemoteVideoTrack);
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetTransport(&videoTransport_TCP));
    
	screenTransport_TCP.SetMediaSession(m_pScreenReceiveSession);
	screenTransport_TCP.SetMediaTrack(m_pRemoteScreenTrack);
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetTransport(&screenTransport_TCP));
    
#ifdef WIN32
	Sleep(1000);
#else
	usleep(1000000);
#endif
    if (m_pLocalVideoTrack) {
        m_pLocalVideoTrack->Stop();
    }
    if (m_pRemoteVideoTrack) {
        m_pRemoteVideoTrack->Stop();
    }
    if (m_pLocalAudioTrack) {
        m_pLocalAudioTrack->Stop();
    }
    if (m_pRemoteAudioTrack) {
        m_pRemoteAudioTrack->Stop();
    }
    if (m_pLocalScreenTrack) {
        m_pLocalScreenTrack->Stop();
    }
    if (m_pRemoteScreenTrack) {
        m_pRemoteScreenTrack->Stop();
    }
    
	// test NULL transport
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetTransport(NULL));
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetTransport(NULL));
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetTransport(NULL));
    
	m_pAudioReceiveSession->Release();
	m_pVideoReceiveSession->Release();
	m_pScreenReceiveSession->Release();
}

TEST_F(CWmeMediaSessionTest, ReceiveRTPPacket)
{
	unsigned char RTPPack[1024];
	RTPPack[0] = 0x80;
    
#ifndef IOS_SIMULATOR
    EXPECT_NE(NULL, (long)m_pRemoteVideoTrack);
    EXPECT_NE(NULL, (long)m_pRemoteAudioTrack);
    EXPECT_NE(NULL, (long)m_pRemoteScreenTrack);
#endif
    
    if (m_pRemoteVideoTrack) {
        m_pRemoteVideoTrack->SetTrackLabel(100);
    }
    if (m_pRemoteAudioTrack) {
        m_pRemoteAudioTrack->SetTrackLabel(200);
    }
    if (m_pRemoteScreenTrack) {
        m_pRemoteScreenTrack->SetTrackLabel(300);
    }
    
	wme::WmeCodecPayloadTypeMap stVideoCodecPayloadTypeMap = {wme::WmeCodecType_SVC, 98, s_uSvcClockRate};
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stVideoCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    
	wme::WmeCodecPayloadTypeMap stAudioCodecPayloadTypeMap = {wme::WmeCodecType_OPUS, 1, s_uOpusClockRate};
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stAudioCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    
	wme::WmeCodecPayloadTypeMap stScreenCodecPayloadTypeMap = {wme::WmeCodecType_AVC, 96, s_uAvcClockRate};
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stScreenCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    
        if (m_pRemoteVideoTrack) {
	EXPECT_EQ(WME_S_OK, m_pVideoSession->AddTrack(m_pRemoteVideoTrack));
        }
        if (m_pRemoteAudioTrack) {
	EXPECT_EQ(WME_S_OK, m_pAudioSession->AddTrack(m_pRemoteAudioTrack));
        }
        if (m_pRemoteScreenTrack) {
	EXPECT_EQ(WME_S_OK, m_pScreenSession->AddTrack(m_pRemoteScreenTrack));
        }
        if (m_pRemoteAudioTrack) {
            // set the payload type
            RTPPack[1] = 1;
	EXPECT_EQ(WME_S_OK, m_pAudioSession->ReceiveRTPPacket(m_pRemoteAudioTrack, RTPPack, 1024));
        }
        if (m_pRemoteVideoTrack) {
            // set the payload type
            RTPPack[1] = 98;
	EXPECT_EQ(WME_S_OK, m_pVideoSession->ReceiveRTPPacket(m_pRemoteVideoTrack, RTPPack, 1024));
        }
        if (m_pRemoteScreenTrack) {
            // set the payload type
            RTPPack[1] = 96;
	EXPECT_EQ(WME_S_OK, m_pScreenSession->ReceiveRTPPacket(m_pRemoteScreenTrack, RTPPack, 1024));
        }
        if (m_pRemoteAudioTrack) {
	EXPECT_NE(WME_S_OK, m_pAudioSession->ReceiveRTPPacket(m_pRemoteAudioTrack, NULL, 1024));
        }
        if (m_pRemoteVideoTrack) {
	EXPECT_NE(WME_S_OK, m_pVideoSession->ReceiveRTPPacket(m_pRemoteVideoTrack, NULL, 1024));
        }
        if (m_pRemoteScreenTrack) {
	EXPECT_NE(WME_S_OK, m_pScreenSession->ReceiveRTPPacket(m_pRemoteScreenTrack, NULL, 1024));
        }
        if (m_pRemoteAudioTrack) {
	EXPECT_NE(WME_S_OK, m_pAudioSession->ReceiveRTPPacket(m_pRemoteAudioTrack, RTPPack, 0));
        }
        if (m_pRemoteVideoTrack) {
	EXPECT_NE(WME_S_OK, m_pVideoSession->ReceiveRTPPacket(m_pRemoteVideoTrack, RTPPack, 0));
        }
        if (m_pRemoteScreenTrack) {
	EXPECT_NE(WME_S_OK, m_pScreenSession->ReceiveRTPPacket(m_pRemoteScreenTrack, RTPPack, 0));
        }
    
	EXPECT_NE(WME_S_OK, m_pAudioSession->ReceiveRTPPacket(NULL, RTPPack, 1024));
	EXPECT_NE(WME_S_OK, m_pVideoSession->ReceiveRTPPacket(NULL, RTPPack, 1024));
	EXPECT_NE(WME_S_OK, m_pScreenSession->ReceiveRTPPacket(NULL, RTPPack, 1024));
}

TEST_F(CWmeMediaSessionTest, ReceiveRTCPPacket)
{
	unsigned char RTCPPack[1024];
	RTCPPack[0] = 0xA0;
	RTCPPack[1] = 0xC9;
	RTCPPack[2] = 0x00;
	RTCPPack[3] = 0xFF;
    
	EXPECT_EQ(WME_S_OK, m_pAudioSession->ReceiveRTCPPacket(RTCPPack, 1024));
	EXPECT_EQ(WME_S_OK, m_pVideoSession->ReceiveRTCPPacket(RTCPPack, 1024));
	EXPECT_EQ(WME_S_OK, m_pScreenSession->ReceiveRTCPPacket(RTCPPack, 1024));
    
	EXPECT_NE(WME_S_OK, m_pAudioSession->ReceiveRTCPPacket(NULL, 1024));
	EXPECT_NE(WME_S_OK, m_pVideoSession->ReceiveRTCPPacket(NULL, 1024));
	EXPECT_NE(WME_S_OK, m_pScreenSession->ReceiveRTCPPacket(NULL, 1024));
    
	EXPECT_NE(WME_S_OK, m_pAudioSession->ReceiveRTCPPacket(RTCPPack, 0));
	EXPECT_NE(WME_S_OK, m_pVideoSession->ReceiveRTCPPacket(RTCPPack, 0));
	EXPECT_NE(WME_S_OK, m_pScreenSession->ReceiveRTCPPacket(RTCPPack, 0));
}

TEST_F(CWmeMediaSessionTest, IndicatePictureLoss)
{
	EXPECT_NE(WME_S_OK, m_pVideoSession->IndicatePictureLoss(100, 0));
#ifndef IOS_SIMULATOR
    ASSERT_NE(NULL, (long)m_pLocalVideoTrack);
    ASSERT_NE(NULL, (long)m_pRemoteVideoTrack);
#endif
    
    if (!m_pLocalVideoTrack || !m_pRemoteVideoTrack) {
        return;
    }
    
    m_pLocalVideoTrack->SetTrackLabel(100);
    m_pRemoteVideoTrack->SetTrackLabel(100);
    
	wme::IWmeMediaSession	*m_pVideoReceiveSession = NULL;
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_Video, &m_pVideoReceiveSession));
    
	wme::WmeCodecPayloadTypeMap stCodecPayloadTypeMap = {wme::WmeCodecType_SVC, 98, s_uSvcClockRate};
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_EQ(WME_S_OK, m_pVideoReceiveSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    EXPECT_EQ(WME_S_OK, m_pVideoSession->AddTrack(m_pLocalVideoTrack));
	EXPECT_EQ(WME_S_OK, m_pVideoReceiveSession->AddTrack(m_pRemoteVideoTrack));

	MockImeMediaTransport videoTransport_UDP(wme::WmeTransportType_UDP);
	videoTransport_UDP.SetMediaSession(m_pVideoReceiveSession);
	videoTransport_UDP.SetMediaTrack(m_pRemoteVideoTrack);
 
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetTransport(&videoTransport_UDP));

    m_pLocalVideoTrack->Start();
    m_pRemoteVideoTrack->Start();

    
#ifdef WIN32
	Sleep(2000);
#else
	usleep(1500*1000);
#endif
    //Commented out the assert by weichen2, need to refine this case to not depend on the camera.
    m_pVideoReceiveSession->IndicatePictureLoss(100, 0);
	//EXPECT_EQ(WME_S_OK, m_pVideoReceiveSession->IndicatePictureLoss(100, 0));
    if (m_pLocalVideoTrack) {
        m_pLocalVideoTrack->Stop();
    }
    if (m_pRemoteVideoTrack) {
        m_pRemoteVideoTrack->Stop();
    }
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetTransport(NULL));
    
    EXPECT_EQ(WME_S_OK, m_pVideoSession->RemoveTrack(m_pLocalVideoTrack));
    EXPECT_EQ(WME_S_OK, m_pVideoReceiveSession->RemoveTrack(m_pRemoteVideoTrack));
    
    m_pVideoReceiveSession->Release();
}

TEST_F(CWmeMediaSessionTest, EnableQOS)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pLocalVideoTrack);
    EXPECT_NE(NULL, (long)m_pRemoteVideoTrack);
    EXPECT_NE(NULL, (long)m_pLocalAudioTrack);
    EXPECT_NE(NULL, (long)m_pRemoteAudioTrack);
    EXPECT_NE(NULL, (long)m_pLocalScreenTrack);
    EXPECT_NE(NULL, (long)m_pRemoteScreenTrack);
#else
    return;
#endif
    
    if (m_pLocalVideoTrack) {
        m_pLocalVideoTrack->SetTrackLabel(100);
    }
    if (m_pRemoteVideoTrack) {
        m_pRemoteVideoTrack->SetTrackLabel(100);
    }
    if (m_pLocalAudioTrack) {
        m_pLocalAudioTrack->SetTrackLabel(200);
    }
    if (m_pRemoteAudioTrack) {
        m_pRemoteAudioTrack->SetTrackLabel(200);
    }
    if (m_pLocalScreenTrack) {
        m_pLocalScreenTrack->SetTrackLabel(300);
    }
    if (m_pRemoteScreenTrack) {
        m_pRemoteScreenTrack->SetTrackLabel(300);
    }
    
    
	wme::IWmeMediaSession	*m_pAudioReceiveSession = NULL;
	wme::IWmeMediaSession	*m_pVideoReceiveSession = NULL;
	wme::IWmeMediaSession	*m_pScreenReceiveSession = NULL;
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_Audio, &m_pAudioReceiveSession));
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_Video, &m_pVideoReceiveSession));
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_ScreenShare, &m_pScreenReceiveSession));
    
	wme::WmeSessionOption eOption = wme::WmeSessionOption_EnableQOS;
	bool EnableQOS = true;
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pAudioReceiveSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pVideoReceiveSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pScreenReceiveSession->SetOption(eOption, &EnableQOS, sizeof(bool)));
    
	wme::WmeCodecPayloadTypeMap stVideoCodecPayloadTypeMap = {wme::WmeCodecType_SVC, 98, s_uSvcClockRate};
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stVideoCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_EQ(WME_S_OK, m_pVideoReceiveSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stVideoCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    
	wme::WmeCodecPayloadTypeMap stAudioCodecPayloadTypeMap = {wme::WmeCodecType_OPUS, 1, s_uOpusClockRate};
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stAudioCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_EQ(WME_S_OK, m_pAudioReceiveSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stAudioCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
    
	wme::WmeCodecPayloadTypeMap stScreenCodecPayloadTypeMap = {wme::WmeCodecType_AVC, 96, s_uAvcClockRate};
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stScreenCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
	EXPECT_EQ(WME_S_OK, m_pScreenReceiveSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stScreenCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));
	if (m_pLocalVideoTrack) {
        EXPECT_EQ(WME_S_OK, m_pVideoSession->AddTrack(m_pLocalVideoTrack));
    }
    
	EXPECT_EQ(WME_S_OK, m_pVideoReceiveSession->AddTrack(m_pRemoteVideoTrack));
	
	EXPECT_EQ(WME_S_OK, m_pAudioSession->AddTrack(m_pLocalAudioTrack));
	EXPECT_EQ(WME_S_OK, m_pAudioReceiveSession->AddTrack(m_pRemoteAudioTrack));
	
	EXPECT_EQ(WME_S_OK, m_pScreenSession->AddTrack(m_pLocalScreenTrack));
	EXPECT_EQ(WME_S_OK, m_pScreenReceiveSession->AddTrack(m_pRemoteScreenTrack));
    
	MockImeMediaTransport audioTransport_UDP(wme::WmeTransportType_UDP), audioTransport_TCP(wme::WmeTransportType_TCP);
	MockImeMediaTransport videoTransport_UDP(wme::WmeTransportType_UDP), videoTransport_TCP(wme::WmeTransportType_TCP);
	MockImeMediaTransport screenTransport_UDP(wme::WmeTransportType_UDP), screenTransport_TCP(wme::WmeTransportType_TCP);
    
	// test UDP transport
	audioTransport_UDP.SetMediaSession(m_pAudioReceiveSession);
	audioTransport_UDP.SetMediaTrack(m_pRemoteAudioTrack);
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetTransport(&audioTransport_UDP));
    
	videoTransport_UDP.SetMediaSession(m_pVideoReceiveSession);
	videoTransport_UDP.SetMediaTrack(m_pRemoteVideoTrack);
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetTransport(&videoTransport_UDP));
    
	screenTransport_UDP.SetMediaSession(m_pScreenReceiveSession);
	screenTransport_UDP.SetMediaTrack(m_pRemoteScreenTrack);
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetTransport(&screenTransport_UDP));
    
    if (m_pLocalVideoTrack) {
        m_pLocalVideoTrack->Start();
    }
    if (m_pRemoteVideoTrack) {
        m_pRemoteVideoTrack->Start();
    }
    if (m_pLocalAudioTrack) {
        m_pLocalAudioTrack->Start();
    }
    if (m_pRemoteAudioTrack) {
        m_pRemoteAudioTrack->Start();
    }
    if (m_pLocalScreenTrack) {
        m_pLocalScreenTrack->Start();
    }
    if (m_pRemoteScreenTrack) {
        m_pRemoteScreenTrack->Start();
    }
#ifdef WIN32
	Sleep(5000);
#else
	usleep(1500*1000);
#endif
    
    if (m_pLocalVideoTrack) {
        m_pLocalVideoTrack->Stop();
    }
    if (m_pRemoteVideoTrack) {
        m_pRemoteVideoTrack->Stop();
    }
    if (m_pLocalAudioTrack) {
        m_pLocalAudioTrack->Stop();
    }
    if (m_pRemoteAudioTrack) {
        m_pRemoteAudioTrack->Stop();
    }
    if (m_pLocalScreenTrack) {
        m_pLocalScreenTrack->Stop();
    }
    if (m_pRemoteScreenTrack) {
        m_pRemoteScreenTrack->Stop();
    }
    
	// test TCP transport
	audioTransport_TCP.SetMediaSession(m_pAudioReceiveSession);
	audioTransport_TCP.SetMediaTrack(m_pRemoteAudioTrack);
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetTransport(&audioTransport_TCP));
    
	videoTransport_TCP.SetMediaSession(m_pVideoReceiveSession);
	videoTransport_TCP.SetMediaTrack(m_pRemoteVideoTrack);
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetTransport(&videoTransport_TCP));
    
	screenTransport_TCP.SetMediaSession(m_pScreenReceiveSession);
	screenTransport_TCP.SetMediaTrack(m_pRemoteScreenTrack);
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetTransport(&screenTransport_TCP));
    
    if (m_pLocalVideoTrack) {
        m_pLocalVideoTrack->Start();
    }
    if (m_pRemoteVideoTrack) {
        m_pRemoteVideoTrack->Start();
    }
    if (m_pLocalAudioTrack) {
        m_pLocalAudioTrack->Start();
    }
    if (m_pRemoteAudioTrack) {
        m_pRemoteAudioTrack->Start();
    }
    if (m_pLocalScreenTrack) {
        m_pLocalScreenTrack->Start();
    }
    if (m_pRemoteScreenTrack) {
        m_pRemoteScreenTrack->Start();
    }
#ifdef WIN32
	Sleep(5000);
#else
	usleep(1500*1000);
#endif
    if (m_pLocalVideoTrack) {
        m_pLocalVideoTrack->Stop();
    }
    if (m_pRemoteVideoTrack) {
        m_pRemoteVideoTrack->Stop();
    }
    if (m_pLocalAudioTrack) {
        m_pLocalAudioTrack->Stop();
    }
    if (m_pRemoteAudioTrack) {
        m_pRemoteAudioTrack->Stop();
    }
    if (m_pLocalScreenTrack) {
        m_pLocalScreenTrack->Stop();
    }
    if (m_pRemoteScreenTrack) {
        m_pRemoteScreenTrack->Stop();
    }
    
	EXPECT_EQ(WME_S_OK, m_pAudioSession->SetTransport(NULL));
	EXPECT_EQ(WME_S_OK, m_pVideoSession->SetTransport(NULL));
	EXPECT_EQ(WME_S_OK, m_pScreenSession->SetTransport(NULL));
    
	EXPECT_EQ(WME_S_OK, m_pVideoSession->RemoveTrack(m_pLocalVideoTrack));
	EXPECT_EQ(WME_S_OK, m_pVideoReceiveSession->RemoveTrack(m_pRemoteVideoTrack));
	
	EXPECT_EQ(WME_S_OK, m_pAudioSession->RemoveTrack(m_pLocalAudioTrack));
	EXPECT_EQ(WME_S_OK, m_pAudioReceiveSession->RemoveTrack(m_pRemoteAudioTrack));
	
	EXPECT_EQ(WME_S_OK, m_pScreenSession->RemoveTrack(m_pLocalScreenTrack));
	EXPECT_EQ(WME_S_OK, m_pScreenReceiveSession->RemoveTrack(m_pRemoteScreenTrack));
    
	m_pAudioReceiveSession->Release();
	m_pVideoReceiveSession->Release();
	m_pScreenReceiveSession->Release();
}

TEST_F(CWmeMediaSessionTest, EnumBuiltinRTPHeaderExtensions)
{
	EXPECT_EQ(WME_S_OK, m_pAudioSession->EnumBuiltinRTPHeaderExtensions(MockBuiltinRTPExtensionIterFunc));
	EXPECT_NE(WME_S_OK, m_pAudioSession->EnumBuiltinRTPHeaderExtensions(NULL));
    
	EXPECT_EQ(WME_S_OK, m_pVideoSession->EnumBuiltinRTPHeaderExtensions(MockBuiltinRTPExtensionIterFunc));
	EXPECT_NE(WME_S_OK, m_pVideoSession->EnumBuiltinRTPHeaderExtensions(NULL));
    
	EXPECT_EQ(WME_S_OK, m_pScreenSession->EnumBuiltinRTPHeaderExtensions(MockBuiltinRTPExtensionIterFunc));
	EXPECT_NE(WME_S_OK, m_pScreenSession->EnumBuiltinRTPHeaderExtensions(NULL));
}

TEST_F(CWmeMediaSessionTest, GetStatistics)
{
	wme::WmeSessionStatistics stSessionStat;
    
	EXPECT_EQ(WME_S_OK, m_pAudioSession->GetStatistics(stSessionStat));
	EXPECT_EQ(WME_S_OK, m_pVideoSession->GetStatistics(stSessionStat));
	EXPECT_EQ(WME_S_OK, m_pScreenSession->GetStatistics(stSessionStat));
}
