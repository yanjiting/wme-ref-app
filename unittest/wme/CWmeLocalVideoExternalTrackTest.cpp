#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "CWmeMediaSDKMock.h"
#include "WmeInterface.h"
#include "wrtpclientapi.h"

#include "WmeInterface.h"
#include "WmeError.h"
#include "WmeTrack.h"
#include "WmeVideoCodecImp.h"
#include "WmeLocalVideoExternalTrackImp.h"

#if defined (WIN32)
#include <Windows.h>	
#include <tchar.h>
#endif

using namespace wme;

static const uint32_t s_uTrackLabel = 123456;

static wrtp::FilterVideoDataInfo s_stVideoDataInfo;
static wme::WmeSendingFilterInfo s_stFilterInfo = {wme::WmeFilter_Passed, 1000,(uint8_t*)&s_stVideoDataInfo, sizeof(wrtp::FilterVideoDataInfo)};


static void WmeTestSleep(uint32_t uMillisecond)
{
#if defined(WIN32)
    Sleep(uMillisecond);
#else
    usleep(uMillisecond * 1000);
#endif
}

class CWmeLocalVideoExternalTrackTest :	public testing::Test,  
										public wme::IWmeVideoRenderObserver,
										public wme::IWmeLocalVideoTrackObserver
{
public:
	CWmeLocalVideoExternalTrackTest()
	{
		m_pEngine = NULL;
		m_pVideoSession = NULL;
		m_pTrack = NULL;

		if(WME_S_OK == wme::WmeCreateMediaEngine(&m_pEngine) && m_pEngine)
		{
			m_pEngine->CreateMediaSession(wme::WmeSessionType_Video, &m_pVideoSession);
			m_pEngine->CreateLocalVideoExternalTrack(&m_pTrack, Wme_SourceType_Video_Sharing);
		}

		// add observer
#if 0
		if(NULL != m_pTrack)
		{
			m_pTrack->AddObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this));
			m_pTrack->AddObserver(wme::WMEIID_IWmeLocalVideoTrackObserver, dynamic_cast<wme::IWmeLocalVideoTrackObserver*>(this));
		}
#endif

#ifdef WIN32
		HINSTANCE hInstance;
		hInstance=GetModuleHandle(NULL);
		WNDCLASS Render_WND;	
		Render_WND.cbClsExtra = 0;
		Render_WND.cbWndExtra = 0;
		Render_WND.hCursor = LoadCursor(hInstance, IDC_ARROW);		
		Render_WND.hIcon = LoadIcon(hInstance, IDI_APPLICATION);	
		Render_WND.lpszMenuName = NULL;								
		Render_WND.style = CS_HREDRAW | CS_VREDRAW;					
		Render_WND.hbrBackground = (HBRUSH)COLOR_WINDOW;			
		Render_WND.lpfnWndProc = DefWindowProc;				
		Render_WND.lpszClassName = _T("RenderWindow");				
		Render_WND.hInstance = hInstance;
		//see details @ http://blog.chinaunix.net/uid-13614124-id-3747923.html	
		RegisterClass(&Render_WND);
		m_monoWnd = CreateWindow(_T("RenderWindow"),L"Nexus",WS_OVERLAPPEDWINDOW,50,50,320,240,NULL,NULL,hInstance,NULL); 

		m_bRenderNow = false;
#endif
	}

	virtual ~CWmeLocalVideoExternalTrackTest()
	{
		// remove observer
		if(NULL != m_pTrack)
		{
			m_pTrack->RemoveObserver(wme::WMEIID_IWmeVideoRenderObserver, dynamic_cast<wme::IWmeVideoRenderObserver*>(this));
			m_pTrack->RemoveObserver(wme::WMEIID_IWmeLocalVideoTrackObserver, dynamic_cast<wme::IWmeLocalVideoTrackObserver*>(this));
		}

		if(m_pTrack)
		{
			m_pTrack->Release();
			m_pTrack = NULL;
		}

		if(m_pVideoSession)
		{
			m_pVideoSession->Release();
			m_pVideoSession = NULL;
		}

		if(m_pEngine)
		{
			m_pEngine->Release();
			m_pEngine = NULL;
		}

#ifdef WIN32
		if (m_monoWnd)
		{
			DestroyWindow(m_monoWnd);
		}
#endif
	}

#ifdef WIN32
	virtual void wndProc()
	{
		if (m_bRenderNow)
		{
			RECT Rect;
			::GetClientRect(m_monoWnd,&Rect);
			HDC hdc = ::GetDC(m_monoWnd);

			if (m_pTrack)
			{
				WmeWindowLessRenderInfo renderInfo;
				renderInfo.hDC = hdc;
				renderInfo.stRect.origin.x = Rect.left;
				renderInfo.stRect.origin.y = Rect.top;
				renderInfo.stRect.size.width = Rect.right - Rect.left;
				renderInfo.stRect.size.height = Rect.bottom - Rect.top;
				renderInfo.bReDraw = false;
				WMERESULT res = m_pTrack->SendEvent(WmeTrackEvent_RenderWithDC, &renderInfo, sizeof(WmeWindowLessRenderInfo));
				int i  = 0;
			}

			m_bRenderNow = false;
		}

	}
#endif

	WMERESULT OnRequestedDC(IWmeMediaEventNotifier *pNotifier, uint32_t uRequestedID) {
#ifdef WIN32
		m_bRenderNow = true;
#endif
		return WME_S_OK;
	}


	virtual void SetUp() {}
	virtual void TearDown() {}

	// IWmeMediaEventObserver interface
	virtual WMERESULT OnWillAddByNotifier(wme::IWmeMediaEventNotifier* pNotifer, const wme::WMEIID & ulObserverID) { return WME_S_OK; }
	virtual WMERESULT OnDidRemoveByNotifier(wme::IWmeMediaEventNotifier* pNotifer, const wme::WMEIID & ulObserverID) { return WME_S_OK; }

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
    virtual WMERESULT OnSelfviewSizeUpdate(WmeVideoSizeInfo *pSizeInfo)
    {
        return WME_S_OK;
    }

public:
	//member
	wme::IWmeMediaEngine* m_pEngine;
	wme::IWmeMediaSession* m_pVideoSession;
	wme::IWmeLocalVideoExternalTrack *m_pTrack;
#ifdef WIN32
	HWND m_monoWnd;
	bool m_bRenderNow;
#endif
};


#ifdef WIN32
static bool g_ExitThread = true;
static DWORD WINAPI threadWork(LPVOID param)
{
	CWmeLocalVideoExternalTrackTest* p = (CWmeLocalVideoExternalTrackTest*)param;
	while (g_ExitThread == false)
	{
		p->wndProc();
		WmeTestSleep(10);
	}
	return 0;
}
#endif

TEST_F(CWmeLocalVideoExternalTrackTest, QueryInterface)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	void* pInterface = NULL;
	EXPECT_EQ(WME_S_OK, m_pTrack->QueryInterface(WMEIID_IWmeLocalVideoExternalTrack, &pInterface));
	EXPECT_NE(NULL, (long)pInterface);
}

TEST_F(CWmeLocalVideoExternalTrackTest, StartAndStop)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	bool send = true;
	EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_SendingData, &send, sizeof(bool)));

#if defined (WIN32)       
	g_ExitThread = false;
	HANDLE hThread = CreateThread(NULL, 0, threadWork, this, 0, NULL);

	EXPECT_NE(NULL, (long)m_monoWnd);
	ShowWindow(m_monoWnd, SW_SHOW);
	UpdateWindow(m_monoWnd); 
	Sleep(500);	//show there is a window
	void* p1 = m_monoWnd;
	EXPECT_EQ(WME_S_OK, m_pTrack->AddRenderWindow(p1, NULL));

	WmeVideoQualityType type = WmeVideoQuality_SLD;	//test, ziyzhang
	EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoQuality, &type, sizeof(WmeVideoQualityType)));

	bool useWindowLess = true;
	EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoWindowLessRender, &useWindowLess, sizeof(bool)));

	EXPECT_EQ(WME_S_OK, m_pTrack->Start());


	EXPECT_EQ(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeVideoRenderObserver, this));

	Sleep(5000);	//wait seconds

	WmeVideoEncodeConfiguraion veCfg;
	EXPECT_EQ(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_VideoEncodeConfiguraion, &veCfg, sizeof(veCfg)));

	EXPECT_EQ(WME_S_OK, m_pTrack->Stop());

	EXPECT_EQ(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeVideoRenderObserver, this));

	useWindowLess = false;
	EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoWindowLessRender, &useWindowLess, sizeof(bool)));

	EXPECT_EQ(WME_S_OK, m_pTrack->RemoveRenderWindow(p1));

	g_ExitThread = true;
	Sleep(100);
#endif

}


TEST_F(CWmeLocalVideoExternalTrackTest, CreateLocalVideoExternalTrack)
{
	wme::IWmeLocalVideoExternalTrack *pLocalVideoExternalTrack = NULL;

	EXPECT_EQ(WME_S_OK, m_pEngine->CreateLocalVideoExternalTrack( &pLocalVideoExternalTrack, Wme_SourceType_Video_Sharing));
	pLocalVideoExternalTrack->Release();

	EXPECT_NE(WME_S_OK, m_pEngine->CreateLocalVideoExternalTrack(NULL, Wme_SourceType_Video_Sharing));
}


TEST_F(CWmeLocalVideoExternalTrackTest, GetTrackType)
{
	wme::WmeTrackType eTrackType = wme::WmeTrackType_Uknown;

	EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackType(eTrackType));
	EXPECT_EQ(wme::WmeTrackType_Video, eTrackType);
}

TEST_F(CWmeLocalVideoExternalTrackTest, AboutLabel)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	uint32_t ori = 0x1234;
	EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackLabel(ori));
	uint32_t real = 0xffff;
	EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackLabel(real));
	EXPECT_EQ(ori, real);
	int i = 0;
}

TEST_F(CWmeLocalVideoExternalTrackTest, AboutEnable)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	bool ori = true;
	EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackEnabled(ori));
	bool real = false;
	EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackEnabled(real));
	EXPECT_EQ(ori, real);
	int i = 0;
}

TEST_F(CWmeLocalVideoExternalTrackTest, AboutState)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	WmeTrackState ori = WmeTrackState_Unknown;
	EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackState(ori));
	EXPECT_NE(WmeTrackState_Unknown, ori);
	int i = 0;
}

TEST_F(CWmeLocalVideoExternalTrackTest, GetTrackRole)
{
	wme::WmeTrackRole eTrackRole = wme::WmeTrackRole_Unknown;

	EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackRole(eTrackRole));
	EXPECT_EQ(wme::WmeTrackRole_Local, eTrackRole);
}

TEST_F(CWmeLocalVideoExternalTrackTest, GetTrackLabel)
{
	uint32_t uTrackLabel = 0;

	EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackLabel(uTrackLabel));
}

TEST_F(CWmeLocalVideoExternalTrackTest, SetTrackLabel)
{
	uint32_t uTrackLabel = s_uTrackLabel;

	EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackLabel(uTrackLabel));
	EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackLabel(uTrackLabel));
	EXPECT_EQ(s_uTrackLabel, uTrackLabel);
}

TEST_F(CWmeLocalVideoExternalTrackTest, SetTrackEnabled)
{
	bool bEnabled = true;

	EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackEnabled(bEnabled));
	bEnabled = false;
	EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackEnabled(bEnabled));
}

TEST_F(CWmeLocalVideoExternalTrackTest, GetTrackEnabled)
{
	bool bEnabled = true;

	EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackEnabled(bEnabled));
	EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackEnabled(bEnabled));
	EXPECT_EQ(true, bEnabled);

	bEnabled = false;
	EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackEnabled(bEnabled));
	EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackEnabled(bEnabled));
	EXPECT_EQ(false, bEnabled);
}

TEST_F(CWmeLocalVideoExternalTrackTest, GetTrackState)
{
	wme::WmeTrackState eTrackState = wme::WmeTrackState_Unknown;

	EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackState(eTrackState));
}

TEST_F(CWmeLocalVideoExternalTrackTest, SendEvent)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	/*EVENT starts here*/
	//WmeEvent_UpLinkStat
	{
		//right case:
		WmeUpLinkNetStat stat = {{0, 0, 0, 0}, 2000000, 0x1/*Network_Bandwidth_Up*/};
		EXPECT_EQ(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_UpLinkStat, &stat, sizeof(WmeUpLinkNetStat)));
		//wrong case:
		EXPECT_NE(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_UpLinkStat, &stat, 0));
		EXPECT_NE(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_UpLinkStat, NULL, sizeof(WmeUpLinkNetStat)));
	}
	
}

/// The case is only valid in Windows platform
#ifdef WIN32
TEST_F(CWmeLocalVideoExternalTrackTest, SendEvent_RenderingDisplayChanged)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	if(NULL != m_pTrack)
	{
		void* pRenderWindow = NULL;

		//right case:
		HINSTANCE hInstance;
		hInstance=GetModuleHandle(NULL);
		WNDCLASS Render_WND;	
		Render_WND.cbClsExtra = 0;
		Render_WND.cbWndExtra = 0;
		Render_WND.hCursor = LoadCursor(hInstance, IDC_ARROW);		
		Render_WND.hIcon = LoadIcon(hInstance, IDI_APPLICATION);	
		Render_WND.lpszMenuName = NULL;								
		Render_WND.style = CS_HREDRAW | CS_VREDRAW;					
		Render_WND.hbrBackground = (HBRUSH)COLOR_WINDOW;			
		Render_WND.lpfnWndProc = DefWindowProc;				
		Render_WND.lpszClassName = _T("RenderWindow");				
		Render_WND.hInstance = hInstance;
		//see details @ http://blog.chinaunix.net/uid-13614124-id-3747923.html	
		RegisterClass(&Render_WND);
		HWND hwnd = CreateWindow(_T("RenderWindow"),L"RenderingDisplayChanged",WS_OVERLAPPEDWINDOW,0,0,320,240,NULL,NULL,hInstance,NULL);         	 
	//	ShowWindow(hwnd, SW_SHOW);
	//	UpdateWindow(hwnd); 
	//	Sleep(1000);	//show there is a window
		pRenderWindow = hwnd;
		EXPECT_EQ(WME_S_OK, m_pTrack->AddRenderWindow(pRenderWindow, NULL));
		EXPECT_EQ(WME_S_OK, m_pTrack->Start());
		EXPECT_EQ(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_RenderingDisplayChanged, NULL, 0));
		EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
		EXPECT_EQ(WME_S_OK, m_pTrack->RemoveRenderWindow(pRenderWindow));
		DestroyWindow(hwnd);

		//wrong case:
	//	EXPECT_NE(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_RenderingDisplayChanged, NULL, 0));	// If no window render, it still return WME_S_OK.
	}
}
#endif

/// The case is only valid in Windows platform
#ifdef WIN32
TEST_F(CWmeLocalVideoExternalTrackTest, SendEvent_RenderingPositionChanged)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	if(NULL != m_pTrack)
	{
		void* pRenderWindow = NULL;

		//right case:
		HINSTANCE hInstance;
		hInstance=GetModuleHandle(NULL);
		WNDCLASS Render_WND;	
		Render_WND.cbClsExtra = 0;
		Render_WND.cbWndExtra = 0;
		Render_WND.hCursor = LoadCursor(hInstance, IDC_ARROW);		
		Render_WND.hIcon = LoadIcon(hInstance, IDI_APPLICATION);	
		Render_WND.lpszMenuName = NULL;								
		Render_WND.style = CS_HREDRAW | CS_VREDRAW;					
		Render_WND.hbrBackground = (HBRUSH)COLOR_WINDOW;			
		Render_WND.lpfnWndProc = DefWindowProc;				
		Render_WND.lpszClassName = _T("RenderWindow");				
		Render_WND.hInstance = hInstance;
		//see details @ http://blog.chinaunix.net/uid-13614124-id-3747923.html	
		RegisterClass(&Render_WND);
		HWND hwnd = CreateWindow(_T("RenderWindow"),L"RenderingPositionChanged",WS_OVERLAPPEDWINDOW,0,0,320,240,NULL,NULL,hInstance,NULL);         	 
	//	ShowWindow(hwnd, SW_SHOW);
	//	UpdateWindow(hwnd); 
	//	Sleep(1000);	//show there is a window
		pRenderWindow = hwnd;
		EXPECT_EQ(WME_S_OK, m_pTrack->AddRenderWindow(pRenderWindow, NULL));
		EXPECT_EQ(WME_S_OK, m_pTrack->Start());
		EXPECT_EQ(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_RenderingPositionChanged, pRenderWindow, sizeof(void*)));
		EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
		EXPECT_EQ(WME_S_OK, m_pTrack->RemoveRenderWindow(pRenderWindow));
		DestroyWindow(hwnd);
		
		//wrong case:
		EXPECT_NE(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_RenderingPositionChanged, &pRenderWindow, sizeof(void*)));
		EXPECT_NE(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_RenderingPositionChanged, &pRenderWindow, 0));
		EXPECT_NE(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_RenderingPositionChanged, NULL, sizeof(void*)));
	}
}
#endif

TEST_F(CWmeLocalVideoExternalTrackTest, SetOption)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	/*OPTION starts here*/
	//WmeTrackOption_VideoQuality
	{
		//right case:
		WmeVideoQualityType type = WmeVideoQuality_SD;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoQuality, &type, sizeof(WmeVideoQualityType)));
		//wrong case:
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoQuality, &type, 0));
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoQuality, NULL, sizeof(WmeVideoQualityType)));
	}

	//WmeTrackOption_CabacSupport
	{
		bool bCabacSupport = true;
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncoderCabacSupport, NULL, sizeof(bool)));
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncoderCabacSupport, &bCabacSupport, 0));
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncoderCabacSupport, &bCabacSupport, sizeof(bool)));
	}

	//WmeTrackOption_StaticPerformanceProfile
	{
		//right case:
		WmePerformanceProfileType type = WmePerformanceProfileMedium;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_StaticPerformanceProfile, &type, sizeof(WmePerformanceProfileType)));
		//wrong case:
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_StaticPerformanceProfile, &type, 0));
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_StaticPerformanceProfile, NULL, sizeof(WmePerformanceProfileType)));
	}
	
	{
		WmeVideoEncodeConfiguraion param;
		param.iMaxFrameRate = 16;
		param.iMaxWidth = 300;
		param.iMaxHeight = 200;
		param.iMaxSpacialLayer = 2;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeConfiguraion, &param, sizeof(param)));
	}

	int i = 0;
}

TEST_F(CWmeLocalVideoExternalTrackTest, SetOption_VideoEncodeIdrPeriod)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	//WmeTrackOption_VideoEncodeIdrPeriod
	{
		//right case:
		uint32_t uIdrPeriod = 60;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeIdrPeriod, &uIdrPeriod, sizeof(uIdrPeriod)));
		//wrong case:
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeIdrPeriod, &uIdrPeriod, 0));
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeIdrPeriod, NULL, sizeof(uIdrPeriod)));
	}
}

TEST_F(CWmeLocalVideoExternalTrackTest, SetOption_MaxPayloadSize)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	//WmeTrackOption_MaxPayloadSize
	{
		//right case:
		uint32_t uMaxPayloadSize = 1000;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_MaxPayloadSize, &uMaxPayloadSize, sizeof(uMaxPayloadSize)));
		//wrong case:
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_MaxPayloadSize, &uMaxPayloadSize, 0));
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_MaxPayloadSize, NULL, sizeof(uMaxPayloadSize)));
	}
}

TEST_F(CWmeLocalVideoExternalTrackTest, SetOption_MaxEncodeCapability)
{
	EXPECT_NE(NULL, (long)m_pTrack);

	WmeVideoEncodeCapability sCap;
	sCap.uProfileLevelID = 0x42E014;
    sCap.uNumVids = 1;
    sCap.uVids[0] = 0;
    sCap.uMaxBitRate = 1500000;
    sCap.uMaxFS = 3600;
    sCap.uMaxFPS = 30;
    sCap.uMaxDPB = 0;
    sCap.uMaxMBPS = 108000;
    EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeMaxCapability, &sCap, sizeof(sCap)));
    
    sCap.uProfileLevelID = 0;
    
    //EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeMaxCapability, &sCap, sizeof(sCap)));
	EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeMaxCapability, &sCap, 0));
	EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeMaxCapability, NULL, sizeof(sCap)));
}

TEST_F(CWmeLocalVideoExternalTrackTest, GetOption)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	/*OPTION starts here*/
	//
	WmeVideoEncodeConfiguraion veCfg;
	EXPECT_EQ(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_VideoEncodeConfiguraion, &veCfg, sizeof(veCfg)));
	EXPECT_NE(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_VideoEncodeConfiguraion, &veCfg, 0));

	int i = 0;
}

TEST_F(CWmeLocalVideoExternalTrackTest, AboutPreviewWindows_simple)
{
	EXPECT_NE(NULL, (long)m_pTrack);

#if defined (WIN32)
	HINSTANCE hInstance;
	hInstance=GetModuleHandle(NULL);
	WNDCLASS Render_WND;	
	Render_WND.cbClsExtra = 0;
	Render_WND.cbWndExtra = 0;
	Render_WND.hCursor = LoadCursor(hInstance, IDC_ARROW);		
	Render_WND.hIcon = LoadIcon(hInstance, IDI_APPLICATION);	
	Render_WND.lpszMenuName = NULL;								
	Render_WND.style = CS_HREDRAW | CS_VREDRAW;					
	Render_WND.hbrBackground = (HBRUSH)COLOR_WINDOW;			
	Render_WND.lpfnWndProc = DefWindowProc;				
	Render_WND.lpszClassName = _T("RenderWindow");				
	Render_WND.hInstance = hInstance;
	//see details @ http://blog.chinaunix.net/uid-13614124-id-3747923.html	
	RegisterClass(&Render_WND);
	HWND hwnd = CreateWindow(_T("RenderWindow"),L"Zombie",WS_OVERLAPPEDWINDOW,0,0,320,240,NULL,NULL,hInstance,NULL);         	 
	ShowWindow(hwnd, SW_SHOW);
	UpdateWindow(hwnd); 
	Sleep(1000);	//show there is a window
	void* p1 = hwnd;
	EXPECT_EQ(WME_S_OK, m_pTrack->AddRenderWindow(p1, NULL));
	EXPECT_EQ(WME_S_OK, m_pTrack->RemoveRenderWindow(p1));
	DestroyWindow(hwnd);
#else 
	//Not windows platform, simple implement
	void* p1 = NULL;
	EXPECT_NE(WME_S_OK, m_pTrack->AddRenderWindow(p1, NULL));
	EXPECT_NE(WME_S_OK, m_pTrack->RemoveRenderWindow(p1));
#endif

	
	
	int i = 0;
}

TEST_F(CWmeLocalVideoExternalTrackTest, StaticObserver)
{
	return; // disable for temp solution of dynamic_cast crash
    
    EXPECT_NE(NULL, (long)m_pTrack);
	const WMEIID WMEIID_TEST_1 = 
	{ 0xbdb41451, 0x58e2, 0x498b, { 0xa4, 0x6b, 0xd9, 0x1e, 0x36, 0xaa, 0x48, 0x7f } };


	class ZombieObserver : public IWmeVideoRenderObserver
	{
	public:
		virtual WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID) {return WME_S_OK;}
		virtual WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID) {return WME_S_OK;}
		virtual WMERESULT OnRequestedDC(IWmeMediaEventNotifier *pNotifier, uint32_t uRequestedID) {return WME_S_OK;}
	};

	ZombieObserver zombie_1;
	ZombieObserver zombie_2;


	EXPECT_NE(WME_S_OK, m_pTrack->AddObserver(WMEIID_TEST_1, &zombie_1));
	EXPECT_EQ(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeVideoRenderObserver, &zombie_1));
	EXPECT_NE(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeVideoRenderObserver, &zombie_1));
	EXPECT_EQ(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeVideoRenderObserver, &zombie_2));


//#ifdef WIN32
//	((CWmeLocalVideoTrack*)m_pTrack)->RequestDC(0);
//#endif


	EXPECT_EQ(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeVideoRenderObserver, &zombie_1));
	EXPECT_EQ(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeVideoRenderObserver, &zombie_2));
	EXPECT_NE(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeVideoRenderObserver, &zombie_1));
}

TEST_F(CWmeLocalVideoExternalTrackTest, ChangeCodec)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	IWmeMediaCodecEnumerator* codec_enum = NULL;
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaCodecEnumerator(WmeMediaTypeVideo, &codec_enum));
	EXPECT_NE(NULL, (long)codec_enum);
	int nNumber = 0;
	codec_enum->GetNumber(nNumber);
	IWmeMediaCodec *pCodec = NULL;
    for(int i=0; i < nNumber; i++)
    {
		EXPECT_EQ(WME_S_OK,codec_enum->GetCodec(i, &pCodec));
		EXPECT_NE(NULL, (long)pCodec);
		WmeCodecType type = WmeCodecType_Unknown;
		pCodec->GetCodecType(type);
		if (type == WmeCodecType_AVC)	//target AVC
		{
			break;
		}
    }
	codec_enum->Release();
	codec_enum = NULL;
	
	if (pCodec != NULL)
	{
		EXPECT_EQ(WME_S_OK, m_pTrack->SetCodec(pCodec));
	}

	EXPECT_EQ(WME_S_OK, m_pTrack->Start());

#ifdef WIN32
	Sleep(1000);	//wait seconds
#else
	sleep(1);
#endif

	IWmeMediaCodec* pCodec_t = NULL;
	EXPECT_EQ(WME_S_OK, m_pTrack->GetCodec(&pCodec_t));
	EXPECT_NE(NULL, (long)pCodec_t);
	WmeCodecType type = WmeCodecType_Unknown;
	pCodec_t->GetCodecType(type);
	EXPECT_EQ(WmeCodecType_AVC, type);
	pCodec_t->Release();
	pCodec_t = NULL;

	EXPECT_EQ(WME_S_OK, m_pTrack->Stop());

	if (pCodec != NULL)
	{
		pCodec->Release();
		pCodec = NULL;
	}
}

TEST_F(CWmeLocalVideoExternalTrackTest, RuntimeObservers)
{
	EXPECT_NE(NULL, (long)m_pTrack);

	EXPECT_EQ(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeMediaCaptureObserver, this));
	EXPECT_EQ(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeLocalVideoTrackObserver, this));
	EXPECT_NE(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeMediaCaptureObserver, this));

	EXPECT_EQ(WME_S_OK, m_pTrack->Start());

	WmeTestSleep(1000*2);

	EXPECT_EQ(WME_S_OK, m_pTrack->Stop());


	EXPECT_EQ(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeMediaCaptureObserver, this));
	EXPECT_EQ(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeLocalVideoTrackObserver, this));
	EXPECT_NE(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeMediaCaptureObserver, this));
}

TEST_F(CWmeLocalVideoExternalTrackTest, SetOptionWmeTrackOption_VideoEncoderCabacSupport)
{
	wme::WmeTrackOption eTrackOption = wme::WmeTrackOption_VideoEncoderCabacSupport;
	bool bCabacSupport = true;
	EXPECT_NE(WME_S_OK,m_pTrack->SetOption(eTrackOption, NULL, sizeof(bool)));
	EXPECT_NE(WME_S_OK, m_pTrack->SetOption(eTrackOption, &bCabacSupport, 0));
	EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(eTrackOption, &bCabacSupport, sizeof(bool)));
	bCabacSupport = false;
	EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(eTrackOption, &bCabacSupport, sizeof(bool)));
}

TEST_F(CWmeLocalVideoExternalTrackTest, Start)
{
	EXPECT_EQ(WME_S_OK, m_pTrack->Start());
}

TEST_F(CWmeLocalVideoExternalTrackTest, Stop)
{
	EXPECT_EQ(WME_S_OK, m_pTrack->Stop());

	EXPECT_EQ(WME_S_OK, m_pTrack->Start());
	EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
}

TEST_F(CWmeLocalVideoExternalTrackTest, GetExternalInputter)
{
	EXPECT_NE(WME_S_OK, m_pTrack->GetExternalInputter(NULL));

	wme::IWmeExternalInputter *pVideoInputter = NULL;
	EXPECT_EQ(WME_S_OK, m_pTrack->GetExternalInputter(&pVideoInputter));
	EXPECT_NE(NULL, (long)pVideoInputter);
	if(NULL != pVideoInputter)
	{
		wme::WmeCodecPayloadTypeMap stVideoCodecPayloadTypeMap = {wme::WmeCodecType_SVC, 98, 90000};
		EXPECT_EQ(WME_S_OK, m_pVideoSession->SetOption(wme::WmeSessionOption_CodecPayloadTypeMap, &stVideoCodecPayloadTypeMap, sizeof(wme::WmeCodecPayloadTypeMap)));

		EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackLabel(s_uTrackLabel));
		EXPECT_EQ(WME_S_OK, m_pVideoSession->AddTrack(m_pTrack));
		EXPECT_EQ(WME_S_OK, m_pTrack->Start());

		uint32_t uTimestamp = 0;
		wme::WmeMediaFormatType eFormatType = wme::WmeMediaFormatVideoCodec;
		wme::WmeVideoCodecFormat stVideoCodecFormat = {};
        memset(&stVideoCodecFormat, 0, sizeof(wme::WmeVideoCodecFormat));
		stVideoCodecFormat.eCodecType = wme::WmeCodecType_SVC;
		stVideoCodecFormat.uTimestamp = 1000;
		stVideoCodecFormat.uSampleTimestamp = 90000;
		stVideoCodecFormat.uMarker = 1;
		stVideoCodecFormat.uPriority = 0;
		stVideoCodecFormat.uMaxTID = 4;
		stVideoCodecFormat.uTID = 3;
		stVideoCodecFormat.uDID = 0;
		const int iLen = 1024;
		unsigned char pData[1024];
		EXPECT_EQ(WME_S_OK, pVideoInputter->InputMediaData(uTimestamp, eFormatType, &stVideoCodecFormat, pData, iLen));

		EXPECT_NE(WME_S_OK, pVideoInputter->InputMediaData(uTimestamp, eFormatType, NULL, pData, iLen));
		EXPECT_NE(WME_S_OK, pVideoInputter->InputMediaData(uTimestamp, eFormatType, &stVideoCodecFormat, NULL, iLen));
		EXPECT_NE(WME_S_OK, pVideoInputter->InputMediaData(uTimestamp, eFormatType, &stVideoCodecFormat, pData, 0));
		EXPECT_NE(WME_S_OK, pVideoInputter->InputMediaData(uTimestamp, wme::WmeMediaFormatAudioCodec, &stVideoCodecFormat, pData, iLen));

//		MockWmeMediaPackage cMediaPackage;
//		EXPECT_EQ(WME_S_OK, pVideoInputter->InputMediaData(&cMediaPackage));
		EXPECT_NE(WME_S_OK, pVideoInputter->InputMediaData(NULL));

		EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
		EXPECT_NE(WME_S_OK, pVideoInputter->InputMediaData(uTimestamp, eFormatType, &stVideoCodecFormat, pData, iLen));

		EXPECT_EQ(WME_S_OK, m_pVideoSession->RemoveTrack(m_pTrack));
		EXPECT_NE(WME_S_OK, pVideoInputter->InputMediaData(uTimestamp, eFormatType, &stVideoCodecFormat, pData, iLen));
        
        pVideoInputter->Release();
	}
}

TEST_F(CWmeLocalVideoExternalTrackTest, SetRTPChannel)
{
	wme::IWmeMediaSession *pVideoSession = NULL;
	EXPECT_EQ(WME_S_OK, m_pEngine->CreateMediaSession(wme::WmeSessionType_Video, &pVideoSession));
	if(NULL != pVideoSession)
	{
		EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackLabel(s_uTrackLabel));
		EXPECT_EQ(WME_S_OK, pVideoSession->AddTrack(m_pTrack));
		EXPECT_EQ(WME_S_OK, pVideoSession->RemoveTrack(m_pTrack));
        
        pVideoSession->Release();
    }
}
