#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
#include "WmeError.h"
#include "WmeTrack.h"

#if defined (WIN32)
#include "WmeVideoPreviewTrackImp.h"
#include <Windows.h>	
#include <tchar.h>
#endif

using namespace wme;

static void WmeTestSleep(uint32_t uMillisecond)
{
#if defined(WIN32)
    Sleep(uMillisecond);
#else
    usleep(uMillisecond * 1000);
#endif
}

class CWmeVideoPreviewTrackTest : public testing::Test, public IWmeVideoRenderObserver
{
public:
	CWmeVideoPreviewTrackTest()
	{
		m_pTrack = NULL;

		WmeSetTraceMaxLevel(WME_TRACE_LEVEL_INFO);

		WmeCreateMediaEngine(&m_pWmeEngine);
		if (m_pWmeEngine)
		{
			m_pWmeEngine->CreateVideoPreviewTrack(&m_pTrack);
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
		m_monoWnd = CreateWindow(_T("RenderWindow"),L"MotoX",WS_OVERLAPPEDWINDOW,50,50,320,240,NULL,NULL,hInstance,NULL); 

		m_bRenderNow = false;
#endif

	}
	virtual ~CWmeVideoPreviewTrackTest()
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
				int i  = 0;
			}
			
			m_bRenderNow = false;
		}
		
	}
#endif

	virtual void SetUp() {}
	virtual void TearDown() {}

	WMERESULT OnRequestedDC(IWmeMediaEventNotifier *pNotifier, uint32_t uRequestedID) {
#ifdef WIN32
		m_bRenderNow = true;
#endif
		return WME_S_OK;
	}

	WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID) { return WME_S_OK; }
	WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID) { return WME_S_OK; }
    
public:
	//member
	IWmeMediaEngine* m_pWmeEngine;
	IWmeVideoPreviewTrack* m_pTrack;
#ifdef WIN32
	HWND m_monoWnd;
	bool m_bRenderNow;
#endif
};



#ifdef WIN32
static bool g_ExitThread = true;
static DWORD WINAPI threadWork(LPVOID param)
{
	CWmeVideoPreviewTrackTest* p = (CWmeVideoPreviewTrackTest*)param;
	while (g_ExitThread == false)
	{
		p->wndProc();
		WmeTestSleep(10);
	}
	return 0;
}
#endif

/// The WmeVideoPreviewTrack can't be support on iOS simulator
#ifndef IOS_SIMULATOR

TEST_F(CWmeVideoPreviewTrackTest, AboutOption)
{
    EXPECT_NE(NULL, (long)m_pTrack);
	if (m_pTrack)
	{
		bool async = true;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_CaptureStartAsync, &async, sizeof(async)));
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_CaptureStartAsync, NULL, sizeof(async)));

		EXPECT_EQ(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_CaptureStartAsync, &async, sizeof(async)));
		EXPECT_NE(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_CaptureStartAsync, NULL, sizeof(async)));
	}
}

TEST_F(CWmeVideoPreviewTrackTest, SetOption_VideoQuality)
{
    EXPECT_NE(NULL, (long)m_pTrack);
    if (m_pTrack)
	{
		WmeVideoQualityType eVideoQualityType = WmeVideoQuality_SLD;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoQuality, &eVideoQualityType, sizeof(WmeVideoQualityType)));
		eVideoQualityType = WmeVideoQuality_LD;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoQuality, &eVideoQualityType, sizeof(WmeVideoQualityType)));
		eVideoQualityType = WmeVideoQuality_SD;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoQuality, &eVideoQualityType, sizeof(WmeVideoQualityType)));
		eVideoQualityType = WmeVideoQuality_HD_720P;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoQuality, &eVideoQualityType, sizeof(WmeVideoQualityType)));

		EXPECT_EQ(WME_S_OK, m_pTrack->Start());
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoQuality, &eVideoQualityType, sizeof(WmeVideoQualityType)));
		EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
		
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoQuality, NULL, sizeof(WmeVideoQualityType)));
		EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoQuality, &eVideoQualityType, 0));
	}
}

TEST_F(CWmeVideoPreviewTrackTest, StartAndStop)
{
	EXPECT_NE(NULL, (long)m_pTrack);
    if (NULL != m_pTrack)
    {
#ifdef WIN32
        g_ExitThread = false;
        HANDLE hThread = CreateThread(NULL, 0, threadWork, this, 0, NULL);
        
        EXPECT_NE(NULL, (long)m_monoWnd);
        ShowWindow(m_monoWnd, SW_SHOW);
        UpdateWindow(m_monoWnd);
        WmeTestSleep(500);	//show there is a window
        void* p1 = m_monoWnd;
        EXPECT_EQ(WME_S_OK, m_pTrack->AddRenderWindow(p1, NULL));
        
        bool useWindowLess = true;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoWindowLessRender, &useWindowLess, sizeof(bool)));
#endif
        
        EXPECT_EQ(WME_S_OK, m_pTrack->Start());
        
        
        EXPECT_EQ(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeVideoRenderObserver, this));
        
        WmeTestSleep(6000);	//wait seconds
        
        EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
        
        EXPECT_EQ(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeVideoRenderObserver, this));
        
#ifdef WIN32
        useWindowLess = false;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoWindowLessRender, &useWindowLess, sizeof(bool)));
        
        EXPECT_EQ(WME_S_OK, m_pTrack->RemoveRenderWindow(p1));
        
        g_ExitThread = true;
        WmeTestSleep(100);
#endif
    }
}

TEST_F(CWmeVideoPreviewTrackTest, AboutCaptureDevice)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	EXPECT_NE(NULL, (long)m_pWmeEngine);

    if (NULL != m_pTrack) {
        IWmeMediaDevice*	pDevice = NULL;
        EXPECT_EQ(WME_S_OK, m_pTrack->GetCaptureDevice(&pDevice));
        EXPECT_NE(NULL, (long)pDevice);
        EXPECT_EQ(WME_S_OK, m_pTrack->SetCaptureDevice(pDevice));
        pDevice->Release();
        pDevice = NULL;
        
        IWmeMediaDeviceEnumerator* pDeviceEnum = NULL;
        if (m_pWmeEngine)
        {
            EXPECT_EQ(WME_S_OK, m_pWmeEngine->CreateMediaDeviceEnumerator(WmeMediaTypeVideo, WmeDeviceIn, &pDeviceEnum));
        }
        
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
            
            EXPECT_EQ(WME_S_OK, pDeviceEnum->GetDevice(device_idx, &pDevice));
            EXPECT_NE(NULL, (long)pDevice);
            
            
            EXPECT_EQ(WME_S_OK, m_pTrack->SetCaptureDevice(pDevice));
            
            EXPECT_EQ(WME_S_OK, m_pTrack->Start());
            
            IWmeMediaDevice* p = NULL;
            EXPECT_EQ(WME_S_OK, m_pTrack->GetCaptureDevice(&p));
            EXPECT_NE(NULL, (long)p);
            p->Release();
            
            WmeTestSleep(100);
            
            EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
            pDevice->Release();
        }
        
        if (pDeviceEnum)
        {
            pDeviceEnum->Release();
            pDeviceEnum = NULL;
        }
    }
}

TEST_F(CWmeVideoPreviewTrackTest, AboutCaptureFormat)
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

/// The case is only valid in Windows platform
#ifdef WIN32
TEST_F(CWmeVideoPreviewTrackTest, SendEvent_RenderingDisplayChanged)
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
TEST_F(CWmeVideoPreviewTrackTest, SendEvent_RenderingPositionChanged)
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

#endif  //#ifndef IOS_SIMULATOR
