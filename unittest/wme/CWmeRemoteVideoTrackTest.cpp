#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
#include "WmeError.h"
#include "WmeTrack.h"

#if defined (WIN32)
#include <Windows.h>
#include <tchar.h>
#endif

using namespace wme;

class CWmeRemoteVideoTrackTest : public testing::Test
{
public:
	CWmeRemoteVideoTrackTest()
	{
		m_pTrack = NULL;


		WmeCreateMediaEngine(&m_pWmeEngine);
		if (m_pWmeEngine)
		{
			m_pWmeEngine->CreateRemoteVideoTrack(&m_pTrack);
		}


	}
	virtual ~CWmeRemoteVideoTrackTest()
	{
		if (m_pTrack)
		{
			m_pTrack->Release();
			m_pTrack = NULL;
		}
		if (m_pWmeEngine)
		{
			m_pWmeEngine->Release();
			m_pWmeEngine = NULL;
		}
	}

	virtual void SetUp() {}
	virtual void TearDown() {}
public:
	//member
	IWmeMediaEngine* m_pWmeEngine;
	IWmeRemoteVideoTrack* m_pTrack;
};


TEST_F(CWmeRemoteVideoTrackTest, QueryInterface)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	void* pInterface = NULL;
	EXPECT_EQ(WME_S_OK, m_pTrack->QueryInterface(WMEIID_IWmeRemoteVideoTrack, &pInterface));
	EXPECT_NE(NULL, (long)pInterface);
	int i = 0;
}

TEST_F(CWmeRemoteVideoTrackTest, StartAndStop)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	EXPECT_EQ(WME_S_OK, m_pTrack->Start());
	EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
	int i = 0;
}

TEST_F(CWmeRemoteVideoTrackTest, GetTrackType)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	WmeTrackType type = WmeTrackType_Uknown;
	EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackType(type));
	EXPECT_EQ(WmeTrackType_Video, type);
	int i = 0;
}

TEST_F(CWmeRemoteVideoTrackTest, AboutLabel)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	uint32_t ori = 0x1234;
	EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackLabel(ori));
	uint32_t real = 0xffff;
	EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackLabel(real));
	EXPECT_EQ(ori, real);
	int i = 0;
}

TEST_F(CWmeRemoteVideoTrackTest, AboutEnable)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	bool ori = true;
	EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackEnabled(ori));
	bool real = false;
	EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackEnabled(real));
	EXPECT_EQ(ori, real);
	int i = 0;
}

TEST_F(CWmeRemoteVideoTrackTest, AboutState)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	WmeTrackState ori = WmeTrackState_Unknown;
	EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackState(ori));
	EXPECT_NE(WmeTrackState_Unknown, ori);
	int i = 0;
}

TEST_F(CWmeRemoteVideoTrackTest, AboutCodec)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	IWmeMediaCodec* ori = NULL;
	EXPECT_NE(WME_S_OK, m_pTrack->SetCodec(ori));
	IWmeMediaCodec* real = NULL;
	EXPECT_EQ(WME_S_OK, m_pTrack->GetCodec(&real));
	real->Release();

	wme::IWmeMediaCodecEnumerator *pVideoCodecEnumerator = NULL;
	EXPECT_EQ(WME_S_OK, m_pWmeEngine->CreateMediaCodecEnumerator(wme::WmeMediaTypeVideo, &pVideoCodecEnumerator));
	if(NULL != pVideoCodecEnumerator)
	{
		int32_t iNumber = 0;
		EXPECT_EQ(WME_S_OK, pVideoCodecEnumerator->GetNumber(iNumber));
		EXPECT_EQ(WME_S_OK, pVideoCodecEnumerator->GetCodec(0, &ori));
		EXPECT_EQ(WME_S_OK, m_pTrack->SetCodec(ori));
		EXPECT_EQ(WME_S_OK, m_pTrack->GetCodec(&real));
		EXPECT_EQ((long)ori, (long)real);
		ori->Release();
		real->Release();
	}

	int i = 0;
}

//TEST_F(CWmeRemoteVideoTrackTest, SetRTPChannel)
//{
//	EXPECT_NE(NULL, (long)m_pTrack);
//	wrtp::IRTPChannel* ori = NULL;
//	EXPECT_EQ(WME_S_OK, m_pTrack->SetRTPChannel(ori));
//	int i = 0;
//}


//Not support in runtime because of HWND 
TEST_F(CWmeRemoteVideoTrackTest, AboutPreviewWindows_simple)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	void* p1 = NULL;
	EXPECT_NE(WME_S_OK, m_pTrack->AddRenderWindow(p1, NULL));
	EXPECT_NE(WME_S_OK, m_pTrack->RemoveRenderWindow(p1));
	int i = 0;
}

TEST_F(CWmeRemoteVideoTrackTest, AboutObserver)
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
	class ZombieObserver1: public IWmeRemoteVideoTrackObserver
	{
	public:
		virtual WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID) {return WME_S_OK;}
		virtual WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID) {return WME_S_OK;}
		virtual WMERESULT OnKeyFrameLost(/*in*/uint32_t uLabel, /*in*/uint32_t uDID) {return WME_S_OK;}
		virtual WMERESULT OnRenderBlocked(/*in*/uint32_t uLabel, /*in*/bool bBlocked){return WME_S_OK;}
		virtual WMERESULT OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight) {return WME_S_OK;}
		virtual WMERESULT OnMediaDataInputSuspend(uint32_t uLabel, bool bSuspensionStart) {return WME_S_OK;}

	};

	ZombieObserver zombie_1;
	ZombieObserver zombie_2;
	ZombieObserver1 zombie_3;
	ZombieObserver1 zombie_4;


	EXPECT_NE(WME_S_OK, m_pTrack->AddObserver(WMEIID_TEST_1, &zombie_1));
	EXPECT_EQ(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeVideoRenderObserver, &zombie_1));
	EXPECT_NE(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeVideoRenderObserver, &zombie_1));
	EXPECT_EQ(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeVideoRenderObserver, &zombie_2));


	EXPECT_EQ(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeVideoRenderObserver, &zombie_1));
	EXPECT_EQ(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeVideoRenderObserver, &zombie_2));
	EXPECT_NE(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeVideoRenderObserver, &zombie_1));

	EXPECT_NE(WME_S_OK, m_pTrack->AddObserver(WMEIID_TEST_1, &zombie_3));
	EXPECT_EQ(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeRemoteVideoTrackObserver, &zombie_3));
	EXPECT_NE(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeRemoteVideoTrackObserver, &zombie_3));
	EXPECT_EQ(WME_S_OK, m_pTrack->AddObserver(WMEIID_IWmeRemoteVideoTrackObserver, &zombie_4));


	EXPECT_EQ(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeRemoteVideoTrackObserver, &zombie_3));
	EXPECT_EQ(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeRemoteVideoTrackObserver, &zombie_4));
	EXPECT_NE(WME_S_OK, m_pTrack->RemoveObserver(WMEIID_IWmeRemoteVideoTrackObserver, &zombie_3));


	int i = 0;
}

TEST_F(CWmeRemoteVideoTrackTest, AboutSetOptionSendEvent)
{
#ifdef WIN32
	bool bWindowlessRender = true;
	EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoWindowLessRender, &bWindowlessRender, sizeof(bWindowlessRender)));
	EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoWindowLessRender, &bWindowlessRender, sizeof(bWindowlessRender)));
	bWindowlessRender = false;
	EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoWindowLessRender, &bWindowlessRender, sizeof(bWindowlessRender)));
	EXPECT_NE(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoWindowLessRender, &bWindowlessRender, sizeof(bWindowlessRender)));

	WmeWindowLessRenderInfo WindowlessRenderInfo;
	EXPECT_NE(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_RenderWithDC, &WindowlessRenderInfo, sizeof(WmeWindowLessRenderInfo)));
	bWindowlessRender = true;
	EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_VideoWindowLessRender, &bWindowlessRender, sizeof(bWindowlessRender)));
	EXPECT_EQ(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_RenderWithDC, &WindowlessRenderInfo, sizeof(WmeWindowLessRenderInfo)));
#endif
}

TEST_F(CWmeRemoteVideoTrackTest, SendEvent_RenderingDisplayChanged)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	if(NULL != m_pTrack)
	{
		void* pRenderWindow = NULL;

		//right case:
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
#endif

		//wrong case:
		//	EXPECT_NE(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_RenderingDisplayChanged, NULL, 0));	// If no window render, it still return WME_S_OK.
	}
}

TEST_F(CWmeRemoteVideoTrackTest, SendEvent_RenderingPositionChanged)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	if(NULL != m_pTrack)
	{
		void* pRenderWindow = NULL;

		//right case:
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
#endif

		//wrong case:
		EXPECT_NE(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_RenderingPositionChanged, &pRenderWindow, sizeof(void*)));
		EXPECT_NE(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_RenderingPositionChanged, &pRenderWindow, 0));
		EXPECT_NE(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_RenderingPositionChanged, NULL, sizeof(void*)));
	}
}

