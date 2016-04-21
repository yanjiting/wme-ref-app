#include "CWmeScreenTrackTestBase.h"


#if defined(MACOS) || defined(WIN32)

#include "wrtpclientapi.h"
static const uint32_t s_uTrackLabel = 123456;
static wrtp::FilterVideoDataInfo s_stVideoDataInfo;
static wme::WmeSendingFilterInfo s_stFilterInfo = {wme::WmeFilter_Passed, 1000,(uint8_t*)&s_stVideoDataInfo, sizeof(wrtp::FilterVideoDataInfo)};

class CWmeScreen_LocalScreenShareTrackTest : public CWmeScreenShareTrackTestBase,
									public wme::IWmeScreenCaptureEngineEventObserver ,
									public wme::IWmeVideoRenderObserver,
									public wme::IWmeLocalVideoTrackObserver
{
public:
	CWmeScreen_LocalScreenShareTrackTest(){
		m_pWmeLocalScreenShareTrack = NULL;
		m_eCapturerStatus = wme::WmeExternalCapturerStatusUnknown;
		m_stCaptureSize.height	=0;
		m_stCaptureSize.width	=0;
	}
	~CWmeScreen_LocalScreenShareTrackTest(){
		SAFE_RELEASE(m_pWmeLocalScreenShareTrack);		
	}

	void SetUp()
	{
		CWmeScreenShareTrackTestBase::SetUp();
		if(m_pEngine){
			m_pEngine->CreateLocalScreenShareTrack( &m_pWmeLocalScreenShareTrack);
		}
	}
	void TearDown()
	{
		//
		SAFE_RELEASE(m_pWmeLocalScreenShareTrack);		
		CWmeScreenShareTrackTestBase::TearDown();
	}


	// IWmeMediaEventObserver interface
	virtual WMERESULT OnWillAddByNotifier(wme::IWmeMediaEventNotifier* pNotifer, const wme::WMEIID & ulObserverID) { return WME_S_OK; }
	virtual WMERESULT OnDidRemoveByNotifier(wme::IWmeMediaEventNotifier* pNotifer, const wme::WMEIID & ulObserverID) { return WME_S_OK; }
	// IWmeScreenCaptureEngineEventObserver interface
	virtual WMERESULT OnCaptureStatusChanged(wme::IWmeMediaEventNotifier *pNotifier, wme::WmeExternalCapturerStatus eCapturerStatus)
	{
		m_eCapturerStatus = eCapturerStatus;
		return WME_S_OK;
	}

	virtual WMERESULT OnCaptureDisplaySizeChanged(wme::IWmeMediaEventNotifier *pNotifier, wme::WmeSize &stCaptureSize)
	{
		m_stCaptureSize = stCaptureSize;
		return WME_S_OK;
	}
	virtual WMERESULT OnCaptureData(wme::IWmeMediaEventNotifier *pNotifier, wme::IWmeMediaPackage *pIWmeMediaPackage)
	{
		//m_stCaptureSize = stCaptureSize;
		return WME_S_OK;
	}

	virtual WMERESULT OnError(wme::IWmeMediaEventNotifier *pNotifier, uint32_t nErrorCode) { return WME_S_OK; }

	// IWmeVideoRenderObserver interface
	virtual WMERESULT OnRequestedDC(wme::IWmeMediaEventNotifier *pNotifier, uint32_t uRequestedID)
	{
		m_bRenderNow = true;
		return WME_S_OK;
	}

	// IWmeLocalVideoTrackObserver interface
	virtual WMERESULT OnRequiredFrameRateUpdate(float fRequiredFrameRate)
	{
		return WME_S_OK;
	}
	virtual WMERESULT OnEncodedResolutionUpdate(uint32_t uWidth, uint32_t uHeight)
	{
		return WME_S_OK;
	}
	virtual WMERESULT OnEncodedFrameRateUpdate(uint32_t uFPS)
	{
		return WME_S_OK;
	}
    virtual WMERESULT OnSelfviewSizeUpdate(wme::WmeVideoSizeInfo *pSizeInfo)
    {
        return WME_S_OK;
    }


protected:

	wme::IWmeLocalScreenShareTrack *m_pWmeLocalScreenShareTrack;

	wme::WmeExternalCapturerStatus m_eCapturerStatus;
	wme::WmeSize m_stCaptureSize;

};


TEST_F(CWmeScreen_LocalScreenShareTrackTest, GetTrackType)
{
	wme::WmeTrackType eTrackType = wme::WmeTrackType_Uknown;

	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetTrackType(eTrackType));
	EXPECT_EQ(wme::WmeTrackType_Video, eTrackType);
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, GetTrackRole)
{
	wme::WmeTrackRole eTrackRole = wme::WmeTrackRole_Unknown;

	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetTrackRole(eTrackRole));
	EXPECT_EQ(wme::WmeTrackRole_Local, eTrackRole);
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, GetTrackLabel)
{
	uint32_t uTrackLabel = 0;

	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetTrackLabel(uTrackLabel));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, SetTrackLabel)
{
	uint32_t uTrackLabel = s_uTrackLabel;

	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetTrackLabel(uTrackLabel));
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetTrackLabel(uTrackLabel));
	EXPECT_EQ(s_uTrackLabel, uTrackLabel);
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, SetTrackEnabled)
{
	bool bEnabled = true;

	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetTrackEnabled(bEnabled));
	bEnabled = false;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetTrackEnabled(bEnabled));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, GetOption_SendingData)
{
	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_SendingData;

	bool bSendingDataEnabled = true;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bSendingDataEnabled, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, &bSendingDataEnabled, sizeof(bool)));
	EXPECT_EQ(true, bSendingDataEnabled);

	bSendingDataEnabled = false;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bSendingDataEnabled, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, &bSendingDataEnabled, sizeof(bool)));
	EXPECT_EQ(false, bSendingDataEnabled);

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, &bSendingDataEnabled, 0));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, GetOption_CaptureStartAsync)
{
	// Not support for local screen share track

	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_CaptureStartAsync;

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, NULL, 0));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, GetOption_StaticPerformanceProfile)
{
	// Not support for local screen share track

	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_StaticPerformanceProfile;

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, NULL, 0));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, GetOption_Bandwidth)
{
	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_Bandwidth;

	wme::WmeMediaBandwidth stBandwidth = {0, 0};
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, &stBandwidth, sizeof(wme::WmeMediaBandwidth)));

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, NULL, sizeof(wme::WmeMediaBandwidth)));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, &stBandwidth, 0));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, GetTrackEnabled)
{
	bool bEnabled = true;

	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetTrackEnabled(bEnabled));
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetTrackEnabled(bEnabled));
	EXPECT_EQ(true, bEnabled);

	bEnabled = false;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetTrackEnabled(bEnabled));
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetTrackEnabled(bEnabled));
	EXPECT_EQ(false, bEnabled);
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, GetTrackState)
{
	wme::WmeTrackState eTrackState = wme::WmeTrackState_Unknown;

	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetTrackState(eTrackState));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, SetCodec)
{
	wme::IWmeMediaCodec *pCodec = NULL;
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SetCodec(pCodec));

	wme::IWmeMediaCodecEnumerator *pVideoCodecEnumerator = NULL;
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaCodecEnumerator(wme::WmeMediaTypeVideo, &pVideoCodecEnumerator));
	if(NULL != pVideoCodecEnumerator)
	{
		int32_t iNumber = 0;
		EXPECT_EQ(WME_S_OK, pVideoCodecEnumerator->GetNumber(iNumber));
		EXPECT_EQ(WME_S_OK, pVideoCodecEnumerator->GetCodec(0, &pCodec));
		EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetCodec(pCodec));

		pCodec->Release();
		pVideoCodecEnumerator->Release();
	}
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, GetCodec)
{
	wme::IWmeMediaCodec *pCodec = NULL;

	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetCodec(&pCodec));
	pCodec->Release();

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->GetCodec(NULL));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, SendEvent_UpLinkStat)
{
	wme::WmeTrackEvent eTrackEvent = wme::WmeTrackEvent_UpLinkStat;

	wme::WmeUpLinkNetStat stNetStat = {{0, 0, 0, 0}, 2000000, 0x1};
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SendEvent(eTrackEvent, &stNetStat, sizeof(wme::WmeUpLinkNetStat)));

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SendEvent(eTrackEvent, &stNetStat, 0));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SendEvent(eTrackEvent, NULL, sizeof(wme::WmeUpLinkNetStat))); 
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, SendEvent_ServerFilterInfo)
{
	wme::WmeTrackEvent eTrackEvent = wme::WmeTrackEvent_ServerFilterInfo;

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SendEvent(eTrackEvent, NULL, 0)); 
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, SendEvent_SendingFilterInfo)
{
	wme::WmeTrackEvent eTrackEvent = wme::WmeTrackEvent_SendingFilterInfo;

	wme::WmeSendingFilterInfo stFilterInfo = s_stFilterInfo;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SendEvent(eTrackEvent, &stFilterInfo, sizeof(wme::WmeSendingFilterInfo)));

	stFilterInfo = s_stFilterInfo;
	stFilterInfo.result = wme::WmeFilter_Dropped;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SendEvent(eTrackEvent, &stFilterInfo, sizeof(wme::WmeSendingFilterInfo)));

	stFilterInfo = s_stFilterInfo;
	stFilterInfo.result = wme::WmeFilter_Unknown;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SendEvent(eTrackEvent, &stFilterInfo, sizeof(wme::WmeSendingFilterInfo))); // Should it be EXPECT_NE?

	stFilterInfo = s_stFilterInfo;
	stFilterInfo.totalLen = 0;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SendEvent(eTrackEvent, &stFilterInfo, sizeof(wme::WmeSendingFilterInfo))); // Should it be EXPECT_NE?

	stFilterInfo = s_stFilterInfo;
	stFilterInfo.infoData = NULL;
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SendEvent(eTrackEvent, &stFilterInfo, sizeof(wme::WmeSendingFilterInfo)));

	stFilterInfo = s_stFilterInfo;
	stFilterInfo.infoDataLen = 0;
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SendEvent(eTrackEvent, &stFilterInfo, sizeof(wme::WmeSendingFilterInfo)));

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SendEvent(eTrackEvent, &s_stFilterInfo, 0));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SendEvent(eTrackEvent, 0, sizeof(wme::WmeSendingFilterInfo)));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, SendEvent_ForceKeyFrame)
{
	wme::WmeTrackEvent eTrackEvent = wme::WmeTrackEvent_ForceKeyFrame;

    bool bInstant = false;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SendEvent(eTrackEvent, &bInstant, sizeof(bInstant)));
}


TEST_F(CWmeScreen_LocalScreenShareTrackTest, SetOption_VideoQuality)
{
	// Not support for local screen share track

	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_VideoQuality;

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, NULL, 0));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, SetOption_VideoEncodeConfiguraion)
{
	// Not support for local screen share track

	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_VideoEncodeConfiguraion;

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, NULL, 0));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, SetOption_CabacSupport)
{
	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_VideoEncoderCabacSupport;

	bool bCabacSupport = true;
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bCabacSupport, 0));
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bCabacSupport, sizeof(bool)));
	bCabacSupport = false;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bCabacSupport, sizeof(bool)));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, SetOption_VideoEncodeLayerType)
{
	// Not support for local screen share track

	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_VideoEncodeLayerType;

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, NULL, 0));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, SetOption_VideoWindowLessRender)
{
	//windowless mode is windows platform only
#if defined (WIN32)       
	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_VideoWindowLessRender;

	bool bWindowLessRenderEnabled = true;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bWindowLessRenderEnabled, sizeof(bool)));

	bWindowLessRenderEnabled = false;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bWindowLessRenderEnabled, sizeof(bool)));

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bWindowLessRenderEnabled, 0));
#endif
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, AddObserver)
{

	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->AddObserver(wme::WMEIID_IWmeScreenCaptureEngineEventObserver, dynamic_cast<wme::IWmeScreenCaptureEngineEventObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->AddObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->AddObserver(wme::WMEIID_IWmeLocalVideoTrackObserver, dynamic_cast<wme::IWmeLocalVideoTrackObserver*>(this)));

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->AddObserver(wme::WMEIID_IWmeScreenCaptureEngineEventObserver, NULL));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->AddObserver(wme::WMEIID_IWmeVideoRenderObserver, NULL));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->AddObserver(wme::WMEIID_IWmeLocalVideoTrackObserver, NULL));

}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, RemoveObserver)
{

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeScreenCaptureEngineEventObserver, dynamic_cast<wme::IWmeScreenCaptureEngineEventObserver*>(this)));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this)));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeLocalVideoTrackObserver, dynamic_cast<wme::IWmeLocalVideoTrackObserver*>(this)));

	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->AddObserver(wme::WMEIID_IWmeScreenCaptureEngineEventObserver, dynamic_cast<wme::IWmeScreenCaptureEngineEventObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->AddObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->AddObserver(wme::WMEIID_IWmeLocalVideoTrackObserver, dynamic_cast<wme::IWmeLocalVideoTrackObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeScreenCaptureEngineEventObserver, dynamic_cast<wme::IWmeScreenCaptureEngineEventObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeLocalVideoTrackObserver, dynamic_cast<wme::IWmeLocalVideoTrackObserver*>(this)));


	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeScreenCaptureEngineEventObserver, NULL));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeVideoRenderObserver, NULL));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeLocalVideoTrackObserver, NULL));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->RemoveObserver(NULL));
}


TEST_F(CWmeScreen_LocalScreenShareTrackTest, AddExternalRenderer)
{
	// Not support for local screen share track

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->AddExternalRenderer(NULL, NULL));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, RemoveExternalRenderer)
{
	// Not support for local screen share track

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->RemoveExternalRenderer(NULL));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, GetStatistics)
{
	// Not support for local screen share track

	wme::WmeScreenShareStatistics stStat;

	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetStatistics(stStat));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, AddScreenSource)
{
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->AddScreenSource(NULL));

	//TBD

}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, RemoveScreenSource)
{
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->RemoveScreenSource(NULL));
}


TEST_F(CWmeScreen_LocalScreenShareTrackTest, GetOption_VideoQuality)
{
	// Not support for local screen share track

	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_VideoQuality;

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, NULL, 0));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, GetOption_VideoEncodeConfiguraion)
{
	// Not support for local screen share track

	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_VideoEncodeConfiguraion;

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, NULL, 0));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, GetOption_VideoEncodeLayerType)
{
	// Not support for local screen share track

	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_VideoEncodeLayerType;

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, NULL, 0));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, SetRTPChannel)
{
		EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetTrackLabel(300));
		EXPECT_EQ(WME_S_OK, m_pScreenShareSession->AddTrack(m_pWmeLocalScreenShareTrack));
		EXPECT_EQ(WME_S_OK, m_pScreenShareSession->RemoveTrack(m_pWmeLocalScreenShareTrack));
}

TEST_F(CWmeScreen_LocalScreenShareTrackTest, SetOption_StaticPerformanceProfile)
{
	// Not support for local screen share track

	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_StaticPerformanceProfile;

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, NULL, 0));
}

//=================================================================================
#include "WseMutex.h"

#if defined (WIN32)
#include <Windows.h>	
#include <tchar.h>
#endif//WIN32

#define OPEN_SCREEN_CAPTURE_UT		0
#define OPEN_RUN_FAIL				0
#define START_CAPTURE_WAIT_TIME		(THREAD_HEART_BEAT_TIME*4)


typedef enum
{
	MockWmeCaptureOperate_Unknown = 0,
	MockWmeCaptureOperate_Start,
	MockWmeCaptureOperate_Stop,
	MockWmeCaptureOperate_Pause,
	MockWmeCaptureOperate_Resume
}MockWmeCaptureOperateType;

static bool g_ExitCaptureThread = true;
static WBXResult  CaptuerThreadWork(WBXLpvoid param);

class CWmeScreen_LocalScreenShareTrackIntegrationTest : public CWmeScreen_LocalScreenShareTrackTest
{
public:
	CWmeScreen_LocalScreenShareTrackIntegrationTest()
	{
		m_eCapturerStatus = wme::WmeExternalCapturerStatusUnknown;
		
		m_pThread = NULL;
		
		m_eOperateType = MockWmeCaptureOperate_Unknown;
		m_bChangeNow = false;

	}

	virtual ~CWmeScreen_LocalScreenShareTrackIntegrationTest()
	{


	}

	virtual void SetUp() {
		CWmeScreen_LocalScreenShareTrackTest::SetUp();

		// add observer
		if(NULL != m_pWmeLocalScreenShareTrack)
		{
			m_pWmeLocalScreenShareTrack->AddObserver(wme::WMEIID_IWmeScreenCaptureEngineEventObserver, dynamic_cast<wme::IWmeScreenCaptureEngineEventObserver*>(this));
			m_pWmeLocalScreenShareTrack->AddObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this));
			m_pWmeLocalScreenShareTrack->AddObserver(wme::WMEIID_IWmeLocalVideoTrackObserver, dynamic_cast<wme::IWmeLocalVideoTrackObserver*>(this));
		}

		RefreshScreenSource();
	}
	virtual void TearDown() {

		// remove observer
		if(NULL != m_pWmeLocalScreenShareTrack)
		{
			m_pWmeLocalScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeScreenCaptureEngineEventObserver, dynamic_cast<wme::IWmeScreenCaptureEngineEventObserver*>(this));
			m_pWmeLocalScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this));
			m_pWmeLocalScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeLocalVideoTrackObserver, dynamic_cast<wme::IWmeLocalVideoTrackObserver*>(this));
		}

		CWmeScreen_LocalScreenShareTrackTest::TearDown();
	}
	

	virtual void ChangeCapturerStatus()
	{

		if(m_bChangeNow)
		{

			CWseMutexGuardT<CWseMutex> theGuard(m_TrackLock);
			if(NULL != m_pWmeLocalScreenShareTrack)
			{
				m_pWmeLocalScreenShareTrack->AddRef();

				bool bOperated = false;
				switch(m_eOperateType)
				{
				case MockWmeCaptureOperate_Start:
					{
						m_pWmeLocalScreenShareTrack->Start();
						bOperated = true;
					}
					break;

				case MockWmeCaptureOperate_Stop:
					{
						m_pWmeLocalScreenShareTrack->Stop();
						bOperated = true;
					}
					break;

				case MockWmeCaptureOperate_Pause:
					{
						bool bCaptuerPauseEnabled = true;
						m_pWmeLocalScreenShareTrack->SetOption(wme::WmeTrackOption_CapturePause, &bCaptuerPauseEnabled, sizeof(bool));
						bOperated = true;
					}
					break;

				case MockWmeCaptureOperate_Resume:
					{
						bool bCaptuerPauseEnabled = false;
						m_pWmeLocalScreenShareTrack->SetOption(wme::WmeTrackOption_CapturePause, &bCaptuerPauseEnabled, sizeof(bool));
						bOperated = true;
					}
					break;

				default:
					break;
				}

				m_pWmeLocalScreenShareTrack->Release();
			}
			
			m_bChangeNow = false;
		}
	}

	virtual void UpdateCaptureOperate(MockWmeCaptureOperateType eOperateType, uint32_t uSleepXTime)
	{
		m_eOperateType = eOperateType;
		m_bChangeNow = true;
		SleepX(uSleepXTime);
	}

	
	virtual void CreateCaptureThread(bool bAddSource = true)
	{
		if(bAddSource)
		{
			AddDesktopSource();
		}

		g_ExitCaptureThread = false;
		m_pThread = CreateThreadX(CaptuerThreadWork, this);
	}

	virtual void DestroyCaptureThread(bool bRemoveSource = true)
	{
		g_ExitCaptureThread = true;
		if(m_pThread)
			DestroyThreadX(m_pThread);
		m_pThread = NULL;

		SleepX(100);
	
		if(bRemoveSource)
		{
			RemoveDesktopSource();
		}
	}

	virtual WMERESULT AddDesktopSource()
	{
		WMERESULT wmeResult  = WME_S_OK;
		if(m_pWmeLocalScreenShareTrack && m_pIWmeScreenSourceDesktop)
			m_pWmeLocalScreenShareTrack->AddScreenSource(m_pIWmeScreenSourceDesktop);
		return wmeResult;
	}

	virtual WMERESULT RemoveDesktopSource()
	{
		WMERESULT wmeResult  = WME_S_OK;

		if(m_pWmeLocalScreenShareTrack && m_pIWmeScreenSourceDesktop)
			m_pWmeLocalScreenShareTrack->RemoveScreenSource(m_pIWmeScreenSourceDesktop);

		return wmeResult;
	}
public:
	CWseMutex	m_TrackLock;

	MockWmeCaptureOperateType m_eOperateType;
	bool m_bChangeNow;
	WBXLpvoid m_pThread;
};

static WBXResult  CaptuerThreadWork(WBXLpvoid param)
{
	CWmeScreen_LocalScreenShareTrackIntegrationTest* p = (CWmeScreen_LocalScreenShareTrackIntegrationTest*)param;
	if (g_ExitCaptureThread == false)
	{
		p->ChangeCapturerStatus();
	}
	return 0;
}
TEST_F(CWmeScreen_LocalScreenShareTrackIntegrationTest, Start)
{
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->Start());
	AddDesktopSource();
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->Start());
	m_pWmeLocalScreenShareTrack->Stop();
}

TEST_F(CWmeScreen_LocalScreenShareTrackIntegrationTest, Stop)
{
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->Stop());
    
	AddDesktopSource();
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->Start());
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->Stop());
}


TEST_F(CWmeScreen_LocalScreenShareTrackIntegrationTest, IsDesktopSharing)
{
    //Will be refactoring or remove this interface
}

TEST_F(CWmeScreen_LocalScreenShareTrackIntegrationTest, IsApplicationShared)
{
    //Will be refactoring or remove this interface
}

TEST_F(CWmeScreen_LocalScreenShareTrackIntegrationTest, GetSharedApplicationNumber)
{
	//Will be refactoring or remove this interface
}
TEST_F(CWmeScreen_LocalScreenShareTrackIntegrationTest, SetOption_CapturePause)
{
	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_CapturePause;

	bool bCaptuerPauseEnabled = true;
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bCaptuerPauseEnabled, 0));

	//CreateCaptureThread();
    
//	UpdateCaptureOperate(MockWmeCaptureOperate_Start, START_CAPTURE_WAIT_TIME);
//	EXPECT_EQ(wme::WmeExternalCapturerStatusStarted, m_eCapturerStatus);
    AddDesktopSource();
    m_pWmeLocalScreenShareTrack->Start();

	bCaptuerPauseEnabled = true;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bCaptuerPauseEnabled, sizeof(bool)));

	bCaptuerPauseEnabled = false;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bCaptuerPauseEnabled, sizeof(bool)));

//    UpdateCaptureOperate(MockWmeCaptureOperate_Stop, START_CAPTURE_WAIT_TIME);
    m_pWmeLocalScreenShareTrack->Stop();
    RemoveDesktopSource();
    
    
	//DestroyCaptureThread();
}

TEST_F(CWmeScreen_LocalScreenShareTrackIntegrationTest, SetOption_SendingData)
{	
	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_SendingData;

	bool bSendingDataEnabled = true;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bSendingDataEnabled, sizeof(bool)));
	bSendingDataEnabled = false;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bSendingDataEnabled, sizeof(bool)));

	//CreateCaptureThread();
    
	//UpdateCaptureOperate(MockWmeCaptureOperate_Start, START_CAPTURE_WAIT_TIME);
	//EXPECT_EQ(wme::WmeExternalCapturerStatusStarted, m_eCapturerStatus);
 
    AddDesktopSource();
    m_pWmeLocalScreenShareTrack->Start();
	bSendingDataEnabled = true;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bSendingDataEnabled, sizeof(bool)));
	bSendingDataEnabled = false;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bSendingDataEnabled, sizeof(bool)));
    
    m_pWmeLocalScreenShareTrack->Stop();
    
    RemoveDesktopSource();
	
    //UpdateCaptureOperate(MockWmeCaptureOperate_Stop, START_CAPTURE_WAIT_TIME);
    
	//DestroyCaptureThread();

	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->SetOption(eTrackOption, &bSendingDataEnabled, 0));
}


TEST_F(CWmeScreen_LocalScreenShareTrackIntegrationTest, SetOption_CaptureStartAsync)
{
	// Not support for local screen share track
}

TEST_F(CWmeScreen_LocalScreenShareTrackIntegrationTest, GetOption_CapturePause)
{
	// Not support for local screen share track
}
    

TEST_F(CWmeScreen_LocalScreenShareTrackIntegrationTest, GetOption_CaptureStatus)
{
	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_CaptureStatus;

	wme::WmeExternalCapturerStatus eCapturerStatus = wme::WmeExternalCapturerStatusUnknown;
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, NULL, sizeof(wme::WmeExternalCapturerStatus)));
	EXPECT_NE(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, &eCapturerStatus, 0));

//	CreateCaptureThread();

	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, &eCapturerStatus, sizeof(wme::WmeExternalCapturerStatus)));
	EXPECT_EQ(wme::WmeExternalCapturerStatusStopped, eCapturerStatus);

//	UpdateCaptureOperate(MockWmeCaptureOperate_Start, START_CAPTURE_WAIT_TIME);
    
    AddDesktopSource();
    m_pWmeLocalScreenShareTrack->Start();
    
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, &eCapturerStatus, sizeof(wme::WmeExternalCapturerStatus)));
	EXPECT_EQ(wme::WmeExternalCapturerStatusStarted, eCapturerStatus);

    bool bCaptuerPauseEnabled = true;
    m_pWmeLocalScreenShareTrack->SetOption(wme::WmeTrackOption_CapturePause, &bCaptuerPauseEnabled, sizeof(bool));
    
//	UpdateCaptureOperate(MockWmeCaptureOperate_Pause, START_CAPTURE_WAIT_TIME);
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, &eCapturerStatus, sizeof(wme::WmeExternalCapturerStatus)));
	EXPECT_EQ(wme::WmeExternalCapturerStatusPaused, eCapturerStatus);

//	UpdateCaptureOperate(MockWmeCaptureOperate_Resume, START_CAPTURE_WAIT_TIME);
    bCaptuerPauseEnabled = false;
    m_pWmeLocalScreenShareTrack->SetOption(wme::WmeTrackOption_CapturePause, &bCaptuerPauseEnabled, sizeof(bool));

	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, &eCapturerStatus, sizeof(wme::WmeExternalCapturerStatus)));
	EXPECT_EQ(wme::WmeExternalCapturerStatusStarted, eCapturerStatus);

//	UpdateCaptureOperate(MockWmeCaptureOperate_Stop, START_CAPTURE_WAIT_TIME);
    
    m_pWmeLocalScreenShareTrack->Stop();
    RemoveDesktopSource();
    
	EXPECT_EQ(WME_S_OK, m_pWmeLocalScreenShareTrack->GetOption(eTrackOption, &eCapturerStatus, sizeof(wme::WmeExternalCapturerStatus)));
	EXPECT_EQ(wme::WmeExternalCapturerStatusStopped, eCapturerStatus);

//	DestroyCaptureThread();
}

#endif //#if defined(MACOS) || defined(WIN32)