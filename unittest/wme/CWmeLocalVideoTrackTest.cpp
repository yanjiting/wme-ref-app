#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
#include "WmeError.h"
#include "WmeTrack.h"
#include "WmeVideoCodecImp.h"

#if defined (WIN32)
#include "WmeLocalVideoTrackImp.h"
#include <Windows.h>
#include <tchar.h>
#endif

using namespace wme;

static bool CompareEncodeCapability(WmeVideoEncodeCapability &left, WmeVideoEncodeCapability &right)
{
    if(memcmp(&left, &right, sizeof(WmeVideoEncodeCapability)))
    {
        return false;
    }
    
    return true;
}

static bool CompareEncodeConfiguraion(WmeVideoEncodeConfiguraion &left, WmeVideoEncodeConfiguraion &right)
{
    if(memcmp(&left, &right, sizeof(WmeVideoEncodeConfiguraion)))
    {
        return false;
    }
 
    return true;
}

static void WmeTestSleep(uint32_t uMillisecond)
{
#if defined(WIN32)
    Sleep(uMillisecond);
#else
    usleep(uMillisecond * 1000);
#endif
}

class CWmeLocalVideoTrackTest : public testing::Test, public IWmeVideoRenderObserver, public IWmeMediaCaptureObserver,
public IWmeLocalVideoTrackObserver
{
public:
	CWmeLocalVideoTrackTest()
	{
		m_pTrack = NULL;
		
		WmeSetTraceMaxLevel(WME_TRACE_LEVEL_INFO);
        
		WmeCreateMediaEngine(&m_pWmeEngine);
		if (m_pWmeEngine)
		{
			m_pWmeEngine->CreateLocalVideoTrack(&m_pTrack);
		}
        
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
	virtual ~CWmeLocalVideoTrackTest()
	{
		if (m_pTrack)
		{
			m_pTrack->Release();
			m_pTrack = NULL;
		}
		if (m_pWmeEngine)
		{
			m_pWmeEngine->Release();
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
    
	WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID) { return WME_S_OK; }
	WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID) { return WME_S_OK; }
    
	//for IWmeMediaCaptureObserver
	virtual WMERESULT OnStartedCapture(IWmeMediaEventNotifier *pNotifier, WMERESULT eStartedResult)
	{
		return WME_S_OK;
	}
	virtual WMERESULT OnUpdateCaptureResolution(IWmeMediaEventNotifier *pNotifier, uint32_t uWidth, uint32_t uHeight)
	{
		return WME_S_OK;
	}
	virtual WMERESULT OnUpdateCaptureFPS(IWmeMediaEventNotifier *pNotifier, uint32_t uFPS)
	{
		return WME_S_OK;
	}
    virtual WMERESULT OnCameraRuntimeDie(IWmeMediaEventNotifier *pNotifier)
    {
        return WME_S_OK;
    }
    
	//for IWmeLocalVideoTrackObserver
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
    
	virtual void SetUp() {}
	virtual void TearDown() {}

public:
	//member
	IWmeMediaEngine* m_pWmeEngine;
	IWmeLocalVideoTrack* m_pTrack;
#ifdef WIN32
	HWND m_monoWnd;
	bool m_bRenderNow;
#endif
};

#ifdef WIN32
static bool g_ExitThread = true;
static DWORD WINAPI threadWork(LPVOID param)
{
	CWmeLocalVideoTrackTest* p = (CWmeLocalVideoTrackTest*)param;
	while (g_ExitThread == false)
	{
		p->wndProc();
		WmeTestSleep(10);
	}
	return 0;
}
#endif

/// The WmeLocalVideoTrack can't be support on iOS simulator
#ifndef IOS_SIMULATOR

TEST_F(CWmeLocalVideoTrackTest, QueryInterface)
{
    EXPECT_NE(NULL, (long)m_pTrack);
	void* pInterface = NULL;
    if (m_pTrack) {
        EXPECT_EQ(WME_S_OK, m_pTrack->QueryInterface(WMEIID_IWmeLocalVideoTrack, &pInterface));
	    EXPECT_NE(NULL, (long)pInterface);
    }
}

TEST_F(CWmeLocalVideoTrackTest, StartAndStop)
{
    EXPECT_NE(NULL, (long)m_pTrack);
    if (NULL != m_pTrack) {
        bool send = true;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_SendingData, &send, sizeof(bool)));
        
#if defined (WIN32)
        g_ExitThread = false;
        HANDLE hThread = CreateThread(NULL, 0, threadWork, this, 0, NULL);
        
        EXPECT_NE(NULL, (long)m_monoWnd);
        ShowWindow(m_monoWnd, SW_SHOW);
        UpdateWindow(m_monoWnd);
        WmeTestSleep(500);	//show there is a window
        void* p1 = m_monoWnd;
        EXPECT_EQ(WME_S_OK, m_pTrack->AddRenderWindow(p1, NULL));
#endif
        
        WmeVideoQualityType type = WmeVideoQuality_SLD;	//test, ziyzhang
        EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoQuality, &type, sizeof(WmeVideoQualityType)));
        
#ifdef WIN32
        bool useWindowLess = true;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoWindowLessRender, &useWindowLess, sizeof(bool)));
#endif
        
        EXPECT_EQ(WME_S_OK, m_pTrack->Start());
        
        
        EXPECT_EQ(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeVideoRenderObserver, this));
        
        WmeTestSleep(5000);	//wait seconds
        
        WmeVideoEncodeConfiguraion veCfg;
        EXPECT_EQ(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_VideoEncodeConfiguraion, &veCfg, sizeof(veCfg)));
        
        EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
        
        EXPECT_EQ(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeVideoRenderObserver, this));
        
#ifdef WIN32
        useWindowLess = false;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoWindowLessRender, &useWindowLess, sizeof(bool)));
#endif
        
#ifdef WIN32
        EXPECT_EQ(WME_S_OK, m_pTrack->RemoveRenderWindow(p1));
        
        g_ExitThread = true;
        WmeTestSleep(100);
#endif
    }
}

TEST_F(CWmeLocalVideoTrackTest, GetTrackType)
{
    EXPECT_NE(NULL, (long)m_pTrack);
	WmeTrackType type = WmeTrackType_Uknown;
    if (m_pTrack) {
	    EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackType(type));
	    EXPECT_EQ(WmeTrackType_Video, type);
    }
}

TEST_F(CWmeLocalVideoTrackTest, AboutLabel)
{
    EXPECT_NE(NULL, (long)m_pTrack);
    if (m_pTrack) {
	    uint32_t ori = 0x1234;
	    EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackLabel(ori));
	    uint32_t real = 0xffff;
	    EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackLabel(real));
	    EXPECT_EQ(ori, real);
    }
}

TEST_F(CWmeLocalVideoTrackTest, AboutEnable)
{
    EXPECT_NE(NULL, (long)m_pTrack);
    if (m_pTrack) {
        bool ori = true;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackEnabled(ori));
        bool real = false;
        EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackEnabled(real));
        EXPECT_EQ(ori, real);
    }
}

TEST_F(CWmeLocalVideoTrackTest, AboutState)
{
    EXPECT_NE(NULL, (long)m_pTrack);
    if (m_pTrack) {
        WmeTrackState ori = WmeTrackState_Unknown;
        EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackState(ori));
        EXPECT_NE(WmeTrackState_Unknown, ori);
    }
}

//TEST_F(CWmeLocalVideoTrackTest, SetRTPChannel)
//{
//	EXPECT_NE(NULL, (long)m_pTrack);
//	wrtp::IRTPChannel* ori = NULL;
//	EXPECT_EQ(WME_S_OK, m_pTrack->SetRTPChannel(ori));
//	int i = 0;
//}

TEST_F(CWmeLocalVideoTrackTest, SendEvent)
{
    EXPECT_NE(NULL, (long)m_pTrack);
	/*EVENT starts here*/
	//WmeEvent_UpLinkStat
    if (m_pTrack) {
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
TEST_F(CWmeLocalVideoTrackTest, SendEvent_RenderingDisplayChanged)
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
        //	WmeTestSleep(1000);	//show there is a window
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
TEST_F(CWmeLocalVideoTrackTest, SendEvent_RenderingPositionChanged)
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
        //	WmeTestSleep(1000);	//show there is a window
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

TEST_F(CWmeLocalVideoTrackTest, SetOption)
{
    EXPECT_NE(NULL, (long)m_pTrack);
    if(NULL != m_pTrack)
    {
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
    }
}

TEST_F(CWmeLocalVideoTrackTest, SetOption_VideoEncodeIdrPeriod)
{
    EXPECT_NE(NULL, (long)m_pTrack);
	//WmeTrackOption_VideoEncodeIdrPeriod
    if(NULL != m_pTrack){
		//right case:
		uint32_t uIdrPeriod = 60;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeIdrPeriod, &uIdrPeriod, sizeof(uIdrPeriod)));
		//wrong case:
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeIdrPeriod, &uIdrPeriod, 0));
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeIdrPeriod, NULL, sizeof(uIdrPeriod)));
	}
}

TEST_F(CWmeLocalVideoTrackTest, SetOption_MaxPayloadSize)
{
    EXPECT_NE(NULL, (long)m_pTrack);
	//WmeTrackOption_MaxPayloadSize
    if(NULL != m_pTrack){
		//right case:
		uint32_t uMaxPayloadSize = 1000;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_MaxPayloadSize, &uMaxPayloadSize, sizeof(uMaxPayloadSize)));
		//wrong case:
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_MaxPayloadSize, &uMaxPayloadSize, 0));
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_MaxPayloadSize, NULL, sizeof(uMaxPayloadSize)));
	}
}

TEST_F(CWmeLocalVideoTrackTest, SetOption_MaxEncodeCapability)
{
    EXPECT_NE(NULL, (long)m_pTrack);
	if(NULL != m_pTrack){
        //right case:
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
        
        sCap.uMaxNalUnitSize = 1400;
        sCap.uNumVids = 1;
        sCap.uVids[0] = 0;
        sCap.uMaxBitRate = 1500000;
        sCap.uMaxFS = 3600;
        sCap.uMaxFPS = 30;
        sCap.uMaxDPB = 0;
        sCap.uMaxMBPS = 108000;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeMaxCapability, &sCap, sizeof(sCap)));
        
        //wrong case:
        sCap.uProfileLevelID = 0;
        sCap.uNumVids = 1;
        sCap.uVids[0] = 0;
        sCap.uMaxBitRate = 1500000;
        sCap.uMaxFS = 3600;
        sCap.uMaxFPS = 30;
        sCap.uMaxDPB = 0;
        sCap.uMaxMBPS = 108000;
        //EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeMaxCapability, &sCap, sizeof(sCap)));
        
        EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeMaxCapability, &sCap, 0));
        EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeMaxCapability, NULL, sizeof(sCap)));
    }
}

TEST_F(CWmeLocalVideoTrackTest, GetOption_VideoEncodeConfiguraion)
{
    EXPECT_NE(NULL, (long)m_pTrack);
    if(NULL != m_pTrack)
    {
        //right case:
        WmeVideoEncodeConfiguraion veCfg = {0};
        EXPECT_EQ(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_VideoEncodeConfiguraion, &veCfg, sizeof(veCfg)));
        
        WmeVideoEncodeConfiguraion setCfg;
        setCfg.iMaxFrameRate = 16;
        setCfg.iMaxWidth = 480;
        setCfg.iMaxHeight = 360;
        setCfg.iMaxSpacialLayer = 2;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeConfiguraion, &setCfg, sizeof(setCfg)));
        EXPECT_EQ(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_VideoEncodeConfiguraion, &veCfg, sizeof(veCfg)));
        EXPECT_EQ(true, CompareEncodeConfiguraion(setCfg, veCfg));
        
        //invalid arguments case:
        EXPECT_NE(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_VideoEncodeConfiguraion, &veCfg, 0));
        EXPECT_NE(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_VideoEncodeConfiguraion, NULL, sizeof(veCfg)));
    }
}

TEST_F(CWmeLocalVideoTrackTest, GetOption_MaxEncodeCapability)
{
    EXPECT_NE(NULL, (long)m_pTrack);
	if(NULL != m_pTrack){
        //wrong case:
        WmeVideoEncodeCapability sCap = {0};
        EXPECT_NE(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_VideoEncodeMaxCapability, &sCap, sizeof(sCap)));
        
        //right case:
        WmeVideoEncodeCapability sSetCap = {0};
		memset(&sSetCap, 0, sizeof(sSetCap));
		memset(&sCap, 0, sizeof(sCap));
        sSetCap.uProfileLevelID = 0x42E014;
        sSetCap.uMaxNalUnitSize = 1400;
        sSetCap.uMaxFPS = 30;
        sSetCap.uMaxBitRate = 1500000;
        sSetCap.uMaxFS = 3600;
        sSetCap.uMaxDPB = 0;
        sSetCap.uMaxMBPS = 108000;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoEncodeMaxCapability, &sSetCap, sizeof(sSetCap)));
        EXPECT_EQ(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_VideoEncodeMaxCapability, &sCap, sizeof(sCap)));
        EXPECT_EQ(0x42E014, sCap.uProfileLevelID);
        EXPECT_EQ(1400, sCap.uMaxNalUnitSize);
        EXPECT_EQ(30, sCap.uMaxFPS);
        
        //invalid arguments case:
        EXPECT_NE(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_VideoEncodeMaxCapability, &sCap, 0));
        EXPECT_NE(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_VideoEncodeMaxCapability, NULL, sizeof(sCap)));
    }
}
TEST_F(CWmeLocalVideoTrackTest, SetOption_VideoMultiStreamSubscribe)
{
    EXPECT_NE(NULL, (long)m_pTrack);
    if(NULL != m_pTrack){
        //wrong case:
        WmeVideoSubscribeMultiStream sSubscribedStr;
        memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
        sSubscribedStr.uNumStreams = 4;
        
        // 90p default maxbr is 64k
        sSubscribedStr.sStream[0].uProfileLevelID = 0x42E000 + 0x000a;
        sSubscribedStr.sStream[0].uMaxMBPS = 1800;
        sSubscribedStr.sStream[0].uMaxFS = 60;
        sSubscribedStr.sStream[0].uMaxFPS = 30;
        sSubscribedStr.sStream[0].uMaxDPB = 180;
        sSubscribedStr.sStream[0].uMaxBitRate = 64*1000;
        sSubscribedStr.sStream[0].uNumVids = 1;
        sSubscribedStr.sStream[0].uVids[0] = 0;
        
        
        // 180p default maxbr is 768k
        sSubscribedStr.sStream[1].uProfileLevelID = 0x42E000 + 0x000c;
        sSubscribedStr.sStream[1].uMaxMBPS = 7200;
        sSubscribedStr.sStream[1].uMaxFS = 240;
        sSubscribedStr.sStream[1].uMaxFPS = 24;
        sSubscribedStr.sStream[1].uMaxDPB = 891;
        sSubscribedStr.sStream[1].uMaxBitRate = 768*1000;
        sSubscribedStr.sStream[1].uNumVids = 2;
        sSubscribedStr.sStream[1].uVids[0] = 1;
        sSubscribedStr.sStream[1].uVids[1] = 2;
        
        // 360p default maxbr is 2M
        sSubscribedStr.sStream[2].uProfileLevelID = 0x42E000 + 0x001e;
        sSubscribedStr.sStream[2].uMaxMBPS = 27600;
        sSubscribedStr.sStream[2].uMaxFS = 920;
        sSubscribedStr.sStream[2].uMaxFPS = 30;
        sSubscribedStr.sStream[2].uMaxDPB = 2760;
        sSubscribedStr.sStream[2].uMaxBitRate = 2000*1000;
        sSubscribedStr.sStream[2].uNumVids = 3;
        sSubscribedStr.sStream[2].uVids[0] = 3;
        sSubscribedStr.sStream[2].uVids[1] = 4;
        sSubscribedStr.sStream[2].uVids[2] = 5;

        
        // 720p default maxbr is 4M
        sSubscribedStr.sStream[3].uProfileLevelID = 0x42E000 + 0x001f;
        sSubscribedStr.sStream[3].uMaxMBPS = 108000;
        sSubscribedStr.sStream[3].uMaxFS = 3840;
        sSubscribedStr.sStream[3].uMaxFPS = 22;
        sSubscribedStr.sStream[3].uMaxDPB = 11520;
        sSubscribedStr.sStream[3].uMaxBitRate = 4000*1000;
        sSubscribedStr.sStream[3].uNumVids = 4;
        sSubscribedStr.sStream[3].uVids[0] = 6;
        sSubscribedStr.sStream[3].uVids[1] = 7;
        sSubscribedStr.sStream[3].uVids[2] = 8;
        sSubscribedStr.sStream[3].uVids[3] = 9;

        
        EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoMultiStreamSubscribe, &sSubscribedStr, sizeof(sSubscribedStr)));
        
        //right case:
        WmeVideoSubscribeMultiStream sGottenMulStr;
        
        EXPECT_EQ(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_VideoMultiStreamSubscribe, &sGottenMulStr, sizeof(sSubscribedStr)));
        
        EXPECT_EQ (sGottenMulStr.uNumStreams, 4);
        
        EXPECT_EQ (sGottenMulStr.sStream[0].uProfileLevelID,  0x42E000 + 0x000a);
        EXPECT_EQ (sGottenMulStr.sStream[1].uProfileLevelID,  0x42E000 + 0x000c);
        EXPECT_EQ (sGottenMulStr.sStream[2].uProfileLevelID,  0x42E000 + 0x001e);
        EXPECT_EQ (sGottenMulStr.sStream[3].uProfileLevelID,  0x42E000 + 0x001f);
        
        EXPECT_GE (sGottenMulStr.sStream[0].uMaxFPS, 22);
        EXPECT_GE (sGottenMulStr.sStream[1].uMaxFPS, 22);
        EXPECT_GE (sGottenMulStr.sStream[2].uMaxFPS, 22);
        EXPECT_GE (sGottenMulStr.sStream[3].uMaxFPS, 22);
        
        EXPECT_EQ(sGottenMulStr.sStream[0].uNumVids, 1);
        EXPECT_EQ(sGottenMulStr.sStream[1].uNumVids, 2);
        EXPECT_EQ(sGottenMulStr.sStream[2].uNumVids, 3);
        EXPECT_EQ(sGottenMulStr.sStream[3].uNumVids, 4);
        
        EXPECT_EQ(sGottenMulStr.sStream[0].uVids[0], 0);
        EXPECT_EQ(sGottenMulStr.sStream[1].uVids[0], 1);
        EXPECT_EQ(sGottenMulStr.sStream[1].uVids[1], 2);
        EXPECT_EQ(sGottenMulStr.sStream[2].uVids[0], 3);
        EXPECT_EQ(sGottenMulStr.sStream[2].uVids[1], 4);
        EXPECT_EQ(sGottenMulStr.sStream[2].uVids[2], 5);
        EXPECT_EQ(sGottenMulStr.sStream[3].uVids[0], 6);
        EXPECT_EQ(sGottenMulStr.sStream[3].uVids[1], 7);
        EXPECT_EQ(sGottenMulStr.sStream[3].uVids[2], 8);
        EXPECT_EQ(sGottenMulStr.sStream[3].uVids[3], 9);
    }
}
TEST_F(CWmeLocalVideoTrackTest, AboutPreviewWindows_simple)
{
    EXPECT_NE(NULL, (long)m_pTrack);
    
    if(NULL != m_pTrack)
    {
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
        WmeTestSleep(1000);	//show there is a window
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
    }
}

TEST_F(CWmeLocalVideoTrackTest, StaticObserver)
{
    return; // disable for temp solution of dynamic_cast crash
    
    EXPECT_NE(NULL, (long)m_pTrack);
    if(NULL != m_pTrack) {
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
}

TEST_F(CWmeLocalVideoTrackTest, AboutCaptureDevice)
{
	IWmeMediaDeviceEnumerator* pDeviceEnum = NULL;
    EXPECT_NE(NULL, (long)m_pTrack);
	if(NULL != m_pTrack) {
        IWmeMediaDevice*	pDevice = NULL;
        EXPECT_EQ(WME_S_OK, m_pTrack->GetCaptureDevice(&pDevice));
        EXPECT_NE(NULL, (long)pDevice);
        EXPECT_EQ(WME_S_OK, m_pTrack->SetCaptureDevice(pDevice));
        pDevice->Release();
        pDevice = NULL;
        
        m_pWmeEngine->CreateMediaDeviceEnumerator(WmeMediaTypeVideo, WmeDeviceIn, &pDeviceEnum);
        EXPECT_NE(NULL, (long)pDeviceEnum);
        
        int32_t num = 0;
        EXPECT_EQ(WME_S_OK, pDeviceEnum->GetDeviceNumber(num));
        
        int device_idx = num;
        if (num > 0)
        {
            device_idx = num - 1;
        }
        if (device_idx < num)
        {
            //WmeRefPointer<IWmeMediaDevice> rfDevice;
            
            EXPECT_EQ(WME_S_OK, pDeviceEnum->GetDevice(device_idx, &pDevice));
            
            EXPECT_EQ(WME_S_OK, m_pTrack->SetCaptureDevice(pDevice));
            if (pDevice) // for pDeviceEnum->GetDevice
            {
                pDevice->Release();
            }
            
            EXPECT_EQ(WME_S_OK, m_pTrack->GetCaptureDevice(&pDevice));
            if (pDevice) // for m_pTrack->GetCaptureDevice
            {
                pDevice->Release();
            }
        }
        pDeviceEnum->Release();
        pDeviceEnum = NULL;
        
        EXPECT_NE(WME_S_OK, m_pTrack->SetCaptureDevice(NULL));
    }
}

TEST_F(CWmeLocalVideoTrackTest, AboutCaptureFormat)
{
	EXPECT_NE(NULL, (long)m_pTrack);
    
    if (NULL != m_pTrack) {
        EXPECT_NE(WME_S_OK, m_pTrack->SetCaptureFormat(NULL));
        
        WmeVideoRawFormat format;
        memset(&format, 0, sizeof(WmeVideoRawFormat));
        EXPECT_NE(WME_S_OK, m_pTrack->SetCaptureFormat(&format));
        
        IWmeMediaDeviceEnumerator* pDeviceEnum = NULL;
        EXPECT_EQ(WME_S_OK, m_pWmeEngine->CreateMediaDeviceEnumerator(WmeMediaTypeVideo, WmeDeviceIn, &pDeviceEnum));
        IWmeMediaDevice* pDevice = NULL;
        EXPECT_EQ(WME_S_OK, pDeviceEnum->GetDevice(0, &pDevice));
        if(NULL != pDevice)
        {
            EXPECT_EQ(WME_S_OK, m_pTrack->SetCaptureDevice(pDevice));
            WmeCameraCapability stCameraCapability;
            WmeDeviceCapability stCapability = {WmeDeviceCapabilityVideo, sizeof(WmeCameraCapability), &stCameraCapability};
            // check at least one format
            EXPECT_EQ(WME_S_OK, pDevice->GetCapabilities(0, &stCapability));
            format.eRawType = stCameraCapability.type;
            format.iWidth = stCameraCapability.width;
            format.iHeight = stCameraCapability.height;
            format.fFrameRate = stCameraCapability.MaxFPS;
            format.uTimestamp = 0;
            EXPECT_EQ(WME_S_OK, m_pTrack->SetCaptureFormat(&format));
            // check all other formats
            for (int i = 1; WME_S_OK == pDevice->GetCapabilities(i, &stCapability); i++) {
                format.eRawType = stCameraCapability.type;
                format.iWidth = stCameraCapability.width;
                format.iHeight = stCameraCapability.height;
                format.fFrameRate = stCameraCapability.MaxFPS;
                format.uTimestamp = 0;
                EXPECT_EQ(WME_S_OK, m_pTrack->SetCaptureFormat(&format));
            }
            pDevice->Release();
        }
        
        EXPECT_EQ(WME_S_OK, m_pTrack->Start());
        
        WmeTestSleep(100);
        
        EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
        
        EXPECT_EQ(WME_S_OK, m_pTrack->GetCaptureFormat(format));
    }
}

TEST_F(CWmeLocalVideoTrackTest, ChangeCodec)
{
	EXPECT_NE(NULL, (long)m_pTrack);
    if (NULL != m_pTrack) {
        IWmeMediaCodecEnumerator* codec_enum = NULL;
        EXPECT_EQ(WME_S_OK, m_pWmeEngine->CreateMediaCodecEnumerator(WmeMediaTypeVideo, &codec_enum));
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
        
        WmeTestSleep(1000);	//wait seconds
        
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
}

TEST_F(CWmeLocalVideoTrackTest, RuntimeObservers)
{
	EXPECT_NE(NULL, (long)m_pTrack);
    if (NULL != m_pTrack) {
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
}

#endif  //#ifndef IOS_SIMULATOR
