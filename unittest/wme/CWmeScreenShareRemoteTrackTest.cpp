#include "CWmeScreenTrackTestBase.h"

#include "WmeInterface.h"
#include "wrtpclientapi.h"

#if defined (WIN32)
#include <Windows.h>	
#include <tchar.h>
#endif

#define OPEN_SCREEN_VIEWER_UT	0

static const uint32_t s_uTrackLabel = 123456;

static bool g_ExitThread = true;
static WBXResult RenderThreadWork(WBXLpvoid param);


class CWmeScreen_RemoteScreenShareTrackTest : public CWmeScreenShareTrackTestBase,
										public wme::IWmeVideoRenderObserver,
										public wme::IWmeRemoteVideoTrackObserver
{
public:
	CWmeScreen_RemoteScreenShareTrackTest()
	{
		m_pWmeRemoteScreenShareTrack = NULL;
		m_pThread = NULL;
	}

	virtual ~CWmeScreen_RemoteScreenShareTrackTest()
	{
		if(m_pThread)
			DestroyThreadX(m_pThread);
		m_pThread = NULL;
	}

	virtual void SetUp() {
		CWmeScreenShareTrackTestBase::SetUp();
		if(m_pEngine)
			m_pEngine->CreateRemoteScreenShareTrack(&m_pWmeRemoteScreenShareTrack);
	}

	virtual void TearDown() {
		SAFE_RELEASE(m_pWmeRemoteScreenShareTrack);
		CWmeScreenShareTrackTestBase::TearDown();
	}


	// IWmeMediaEventObserver interface
	virtual WMERESULT OnWillAddByNotifier(wme::IWmeMediaEventNotifier* pNotifer, const wme::WMEIID & ulObserverID) { return WME_S_OK; }
	virtual WMERESULT OnDidRemoveByNotifier(wme::IWmeMediaEventNotifier* pNotifer, const wme::WMEIID & ulObserverID) { return WME_S_OK; }

	// IWmeVideoRenderObserver interface
	virtual WMERESULT OnRequestedDC(wme::IWmeMediaEventNotifier *pNotifier, uint32_t uRequestedID)
	{
		m_bRenderNow = true;
		return WME_S_OK;
	}

	// IWmeRemoteVideoTrackObserver interface
	virtual WMERESULT OnKeyFrameLost(/*in*/uint32_t uLabel, /*in*/uint32_t uDID)
	{
		return WME_S_OK;
	}

	virtual WMERESULT OnRenderBlocked(/*in*/uint32_t uLabel, /*in*/bool bBlocked)
	{
		return WME_S_OK;
	}

	virtual WMERESULT OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight)
	{
		return WME_S_OK;
	}

	virtual WMERESULT OnMediaDataInputSuspend(uint32_t uLabel, bool bSuspensionStart)
	{
		return WME_S_OK;
	}

	//


	virtual void RenderMockWindow()
	{
		if (m_bRenderNow)
		{
#ifdef WIN32
			HWND hWnd = (HWND)m_hMockWnd;
			RECT Rect;
			::GetClientRect(hWnd,&Rect);
			HDC hdc = ::GetDC(hWnd);

			if (m_pWmeRemoteScreenShareTrack)
			{
				wme::WmeWindowLessRenderInfo renderInfo;
				renderInfo.hDC = hdc;
				renderInfo.stRect.origin.x = Rect.left;
				renderInfo.stRect.origin.y = Rect.top;
				renderInfo.stRect.size.width = Rect.right - Rect.left;
				renderInfo.stRect.size.height = Rect.bottom - Rect.top;
				renderInfo.bReDraw = false;
				WMERESULT res = m_pWmeRemoteScreenShareTrack->SendEvent(wme::WmeTrackEvent_RenderWithDC, &renderInfo, sizeof(wme::WmeWindowLessRenderInfo));
			}
#endif

			m_bRenderNow = false;
		}
	}

	virtual void CreateRenderThread()
	{
		CreateMockWindow();
		g_ExitThread = false;
		m_pThread = CreateThreadX(RenderThreadWork,this);

		if(m_hMockWnd)
		{
			ShowWindowX(m_hMockWnd);
			SleepX(500);	//show there is a window
		}
	}

	virtual void DestroyRenderThread()
	{
		g_ExitThread = true;
		if(m_pThread)
			DestroyThreadX(m_pThread);
		m_pThread = NULL;
		SleepX(100);

		DestroyMockWindow();
	}

public:
	//member
	wme::IWmeRemoteScreenShareTrack *m_pWmeRemoteScreenShareTrack;
	WBXLpvoid m_pThread;
};

static WBXResult RenderThreadWork(WBXLpvoid param)
{
	CWmeScreen_RemoteScreenShareTrackTest* p = (CWmeScreen_RemoteScreenShareTrackTest*)param;
	while (g_ExitThread == false)
	{
		p->RenderMockWindow();
		SleepX(10);
	}
	return 0;
}


TEST_F(CWmeScreen_RemoteScreenShareTrackTest, GetTrackType)
{
	wme::WmeTrackType eTrackType = wme::WmeTrackType_Uknown;

	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetTrackType(eTrackType));
	EXPECT_EQ(wme::WmeTrackType_Video, eTrackType);
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, GetTrackRole)
{
	wme::WmeTrackRole eTrackRole = wme::WmeTrackRole_Unknown;

	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetTrackRole(eTrackRole));
	EXPECT_EQ(wme::WmeTrackRole_Remote, eTrackRole);
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, GetTrackLabel)
{
	uint32_t uTrackLabel = 0;

	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetTrackLabel(uTrackLabel));
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, SetTrackLabel)
{
	uint32_t uTrackLabel = s_uTrackLabel;

	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetTrackLabel(uTrackLabel));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetTrackLabel(uTrackLabel));
	EXPECT_EQ(s_uTrackLabel, uTrackLabel);
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, SetTrackEnabled)
{
	bool bEnabled = true;

	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetTrackEnabled(bEnabled));
	bEnabled = false;
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetTrackEnabled(bEnabled));
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, GetTrackEnabled)
{
	bool bEnabled = true;

	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetTrackEnabled(bEnabled));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetTrackEnabled(bEnabled));
	EXPECT_EQ(true, bEnabled);

	bEnabled = false;
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetTrackEnabled(bEnabled));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetTrackEnabled(bEnabled));
	EXPECT_EQ(false, bEnabled);
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, GetTrackState)
{
	wme::WmeTrackState eTrackState = wme::WmeTrackState_Unknown;

	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetTrackState(eTrackState));
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, SetCodec)
{
	wme::IWmeMediaCodec *pCodec = NULL;
	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetCodec(pCodec));

	wme::IWmeMediaCodecEnumerator *pVideoCodecEnumerator = NULL;
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaCodecEnumerator(wme::WmeMediaTypeVideo, &pVideoCodecEnumerator));
	if(NULL != pVideoCodecEnumerator)
	{
		int32_t iNumber = 0;
		EXPECT_EQ(WME_S_OK, pVideoCodecEnumerator->GetNumber(iNumber));
		EXPECT_EQ(WME_S_OK, pVideoCodecEnumerator->GetCodec(0, &pCodec));
		EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetCodec(pCodec));

		pCodec->Release();
		pVideoCodecEnumerator->Release();
	}
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, GetCodec)
{
	wme::IWmeMediaCodec *pCodec = NULL;

	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetCodec(&pCodec));
	pCodec->Release();

	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetCodec(NULL));
}


TEST_F(CWmeScreen_RemoteScreenShareTrackTest, SendEvent_RenderingDisplayChanged)
{
	EXPECT_NE(NULL, (long)m_pWmeRemoteScreenShareTrack);
	if(NULL != m_pWmeRemoteScreenShareTrack)
	{
		wme::WmeTrackEvent eTrackEvent = wme::WmeTrackEvent_RenderingDisplayChanged;

		void* pRenderWindow =  CreateWindowX();
        if (pRenderWindow==NULL)  return ; //System dont support CreatWindowX , skip these cases

		EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddRenderWindow(pRenderWindow, NULL));
		EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->Start());
		EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->SendEvent(eTrackEvent, NULL, 0));
		EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->Stop());
		EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveRenderWindow(pRenderWindow));
		DestroyWindowX(pRenderWindow);
	}
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, SendEvent_RenderingPositionChanged)
{
	EXPECT_NE(NULL, (long)m_pWmeRemoteScreenShareTrack);
	if(NULL != m_pWmeRemoteScreenShareTrack)
	{
		wme::WmeTrackEvent eTrackEvent = wme::WmeTrackEvent_RenderingPositionChanged;

		void* pRenderWindow = CreateWindowX();
        if (pRenderWindow==NULL)  return ; //System dont support CreatWindowX , skip these cases

		EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddRenderWindow(pRenderWindow, NULL));
		EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->Start());
		EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->SendEvent(eTrackEvent, pRenderWindow, sizeof(void*)));
		EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->Stop());
		EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveRenderWindow(pRenderWindow));
		DestroyWindowX(pRenderWindow);

		//wrong case:
		EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->SendEvent(eTrackEvent, &pRenderWindow, sizeof(void*)));
		EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->SendEvent(eTrackEvent, &pRenderWindow, 0));
		EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->SendEvent(eTrackEvent, NULL, sizeof(void*)));
	}
}

#if defined(WIN32) 
//Windowless is just for windows.
TEST_F(CWmeScreen_RemoteScreenShareTrackTest, SetOption_VideoWindowLessRender)
{
	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_VideoWindowLessRender;

	bool bWindowLessRenderEnabled = true;
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetOption(eTrackOption, &bWindowLessRenderEnabled, sizeof(bool)));

	bWindowLessRenderEnabled = false;
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetOption(eTrackOption, &bWindowLessRenderEnabled, sizeof(bool)));

	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetOption(eTrackOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetOption(eTrackOption, &bWindowLessRenderEnabled, 0));
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, GetOption_VideoWindowLessRender)
{
	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_VideoWindowLessRender;
    
	bool bWindowLessRenderEnabled = true;
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetOption(eTrackOption, &bWindowLessRenderEnabled, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetOption(eTrackOption, &bWindowLessRenderEnabled, sizeof(bool)));
	EXPECT_EQ(true, bWindowLessRenderEnabled);
    
	bWindowLessRenderEnabled = false;
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetOption(eTrackOption, &bWindowLessRenderEnabled, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetOption(eTrackOption, &bWindowLessRenderEnabled, sizeof(bool)));
	EXPECT_EQ(false, bWindowLessRenderEnabled);
    
	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetOption(eTrackOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetOption(eTrackOption, &bWindowLessRenderEnabled, 0));
}
#endif

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, SetOption_StaticPerformanceProfile)
{
	// Not support for local screen share track

	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_StaticPerformanceProfile;

	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetOption(eTrackOption, NULL, 0));
}


TEST_F(CWmeScreen_RemoteScreenShareTrackTest, GetOption_StaticPerformanceProfile)
{
	// Not support for local screen share track

	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_StaticPerformanceProfile;

	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetOption(eTrackOption, NULL, 0));
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, GetOption_Bandwidth)
{
	// Not support for local screen share track

	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_Bandwidth;

	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetOption(eTrackOption, NULL, 0));
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, Start)
{
	CreateMockWindow();
    if(m_hMockWnd==NULL) return;//system don't implement createwindowx , need not run this cases;
    
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddRenderWindow(m_hMockWnd, NULL));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->Start());
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveRenderWindow(m_hMockWnd));
	DestroyMockWindow();
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, Stop)
{
	CreateMockWindow();
    if(m_hMockWnd==NULL) return;//system don't implement createwindowx , need not run this cases;


	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddRenderWindow(m_hMockWnd, NULL));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->Stop());

	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->Start());
	SleepX(1000);
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->Stop());

	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveRenderWindow(m_hMockWnd));

	DestroyMockWindow();
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, SetRTPChannel)
{
	wme::IWmeMediaSession *pMediaSession = NULL;
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_ScreenShare, &pMediaSession));
	if(NULL != pMediaSession)
	{
		EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetTrackLabel(300));
		EXPECT_EQ(WME_S_OK, pMediaSession->AddTrack(m_pWmeRemoteScreenShareTrack));
		EXPECT_EQ(WME_S_OK, pMediaSession->RemoveTrack(m_pWmeRemoteScreenShareTrack));
        
        pMediaSession->Release();
    }
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, AddRenderWindow)
{
	CreateMockWindow();
    if(m_hMockWnd==NULL) return;//system don't implement createwindowx , need not run this cases;


	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddRenderWindow(m_hMockWnd, NULL));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveRenderWindow(m_hMockWnd));

	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddRenderWindow(NULL, NULL));

	DestroyMockWindow();
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, RemoveRenderWindow)
{
	CreateMockWindow();
    if(m_hMockWnd==NULL) return;//system don't implement createwindowx , need not run this cases;

	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddRenderWindow(m_hMockWnd, NULL));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveRenderWindow(m_hMockWnd));

	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveRenderWindow(NULL));

	DestroyMockWindow();
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, AddExternalRenderer)
{
	// Not support for local screen share track

	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddExternalRenderer(NULL, NULL));
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, RemoveExternalRenderer)
{
	// Not support for local screen share track

	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveExternalRenderer(NULL));
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, GetStatistics)
{
	// Not support for local screen share track

	wme::WmeVideoStatistics stStat;

	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->GetStatistics(stStat));
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, AddObserver)
{
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddObserver(wme::WMEIID_IWmeRemoteVideoTrackObserver, dynamic_cast<wme::IWmeRemoteVideoTrackObserver*>(this)));

	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddObserver(wme::WMEIID_IWmeVideoRenderObserver, NULL));
	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddObserver(wme::WMEIID_IWmeRemoteVideoTrackObserver, NULL));
}

TEST_F(CWmeScreen_RemoteScreenShareTrackTest, RemoveObserver)
{
	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this)));
	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeRemoteVideoTrackObserver, dynamic_cast<wme::IWmeRemoteVideoTrackObserver*>(this)));

	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddObserver(wme::WMEIID_IWmeRemoteVideoTrackObserver, dynamic_cast<wme::IWmeRemoteVideoTrackObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeRemoteVideoTrackObserver, dynamic_cast<wme::IWmeRemoteVideoTrackObserver*>(this)));

	// Not support WMERESULT RemoveObserver(IWmeMediaEventObserver *pObserver) interface
#if 0
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddObserver(wme::WMEIID_IWmeRemoteVideoTrackObserver, dynamic_cast<wme::IWmeRemoteVideoTrackObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveObserver(dynamic_cast<wme::IWmeVideoRenderObserver*>(this)));

	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddObserver(wme::WMEIID_IWmeRemoteVideoTrackObserver, dynamic_cast<wme::IWmeRemoteVideoTrackObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveObserver(dynamic_cast<wme::IWmeRemoteVideoTrackObserver*>(this)));
#endif

	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeVideoRenderObserver, NULL));
	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeRemoteVideoTrackObserver, NULL));
	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveObserver(NULL));
}




#ifdef WIN32 //windowless mode is for window only
TEST_F(CWmeScreen_RemoteScreenShareTrackTest, SendEvent_RenderWithDC)
{
	wme::WmeTrackEvent eTrackEvent = wme::WmeTrackEvent_RenderWithDC;

	wme::WmeWindowLessRenderInfo stRenderInfo = {NULL, {{0, 0}, {0, 0}}, false};
	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->SendEvent(eTrackEvent, &stRenderInfo, sizeof(wme::WmeWindowLessRenderInfo)));

	stRenderInfo.hDC = ::GetDC((HWND)m_hMockWnd);
	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->SendEvent(eTrackEvent, &stRenderInfo, sizeof(wme::WmeWindowLessRenderInfo)));

	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->SendEvent(eTrackEvent, &stRenderInfo, 0));
	EXPECT_NE(WME_S_OK, m_pWmeRemoteScreenShareTrack->SendEvent(eTrackEvent, NULL, sizeof(wme::WmeWindowLessRenderInfo))); 

	wme::WmeTrackOption eTrackOption =  wme::WmeTrackOption_VideoWindowLessRender;
	bool bWindowLessRenderEnabled = true;
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->SetOption(eTrackOption, &bWindowLessRenderEnabled, sizeof(bool)));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->AddObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this)));
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->Start());
	CreateRenderThread();

	SleepX(8000);	//wait 8 seconds

	DestroyRenderThread();
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->Stop());
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteScreenShareTrack->RemoveObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this)));
}
#endif