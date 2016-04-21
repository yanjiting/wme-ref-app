#ifndef _WME_SCREEN_TRACK_TEST_H_
#define _WME_SCREEN_TRACK_TEST_H_

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WmeEngine.h"
#include "WmeInterface.h"
#include "WseUtil.h"
#include "share_define.h"

//==================================================================================
//UT Util Intefaces
void* CreateWindowX();
void DestroyWindowX(void* window);
void ShowWindowX(void* window);
void LaunchOneApp();

typedef WBXResult (*ThreadRunFunction)(WBXLpvoid);
#define THREAD_HEART_BEAT_TIME 50
WBXLpvoid CreateThreadX(ThreadRunFunction funCallback, WBXLpvoid pObj);
void DestroyThreadX(WBXLpvoid pThread);
void SleepX(WBXInt32 nMs);

class CWmeScreenShareTestBase : public testing::Test{
    
public:
	CWmeScreenShareTestBase(){
		m_pEngine = NULL;
		m_pScreenShareSession= NULL;
        
        
		m_pWmeScreenSourceEnumeratorDesktop = NULL;
		m_pWmeScreenSourceEnumeratorApp = NULL;
		m_pIWmeScreenSourceDesktop = NULL;
		m_pIWmeScreenSourceApp = NULL;

        
	}
	virtual ~CWmeScreenShareTestBase(){
        
	}
    
	void SetUp()
	{
		m_pEngine = NULL;
		wme::WmeCreateMediaEngine(&m_pEngine);
		//
		if(m_pEngine)
			m_pEngine->CreateMediaSession(wme::WmeSessionType_ScreenShare, &m_pScreenShareSession);
        _LaunchOneApp();
        
	}
	void TearDown()
	{
        SAFE_RELEASE(m_pIWmeScreenSourceDesktop);
		SAFE_RELEASE(m_pIWmeScreenSourceApp);
		SAFE_RELEASE(m_pWmeScreenSourceEnumeratorDesktop);
		SAFE_RELEASE(m_pWmeScreenSourceEnumeratorApp);
        
		//
		SAFE_RELEASE(m_pScreenShareSession);
		SAFE_RELEASE(m_pEngine);
        _ExitOneApp();
	}
    
    
    void RefreshScreenSource(){
		if(m_pEngine==NULL) return ;
        
		if(m_pWmeScreenSourceEnumeratorDesktop==NULL)
			m_pEngine->CreateScreenSourceEnumerator(&m_pWmeScreenSourceEnumeratorDesktop ,wme::WmeScreenSourceTypeDesktop);
		if(m_pWmeScreenSourceEnumeratorApp==NULL)
			m_pEngine->CreateScreenSourceEnumerator(&m_pWmeScreenSourceEnumeratorApp ,wme::WmeScreenSourceTypeApplication);
        
		if(m_pWmeScreenSourceEnumeratorDesktop){
			m_pWmeScreenSourceEnumeratorDesktop->Refresh();
			SAFE_RELEASE(m_pIWmeScreenSourceDesktop);
			int32_t nCount=0;
			m_pWmeScreenSourceEnumeratorDesktop->GetNumber(nCount);
			if(nCount>0)
				m_pWmeScreenSourceEnumeratorDesktop->GetSource(0,&m_pIWmeScreenSourceDesktop);
		}
        
		if(m_pWmeScreenSourceEnumeratorApp){
			m_pWmeScreenSourceEnumeratorApp->Refresh();
			SAFE_RELEASE(m_pIWmeScreenSourceApp);

			int32_t nCount=0;
			m_pWmeScreenSourceEnumeratorApp->GetNumber(nCount);
			if(nCount>0)
				m_pWmeScreenSourceEnumeratorApp->GetSource(0,&m_pIWmeScreenSourceApp);
		}
        
	}
    
    void _LaunchOneApp() {
        LaunchOneApp();
    }
    
    void _ExitOneApp() {
        
    }
protected:
	wme::IWmeMediaEngine* m_pEngine;
	wme::IWmeMediaSession* m_pScreenShareSession;
    
	//screen source
	wme::IWmeScreenSourceEnumerator * m_pWmeScreenSourceEnumeratorDesktop;
	wme::IWmeScreenSourceEnumerator * m_pWmeScreenSourceEnumeratorApp;
	wme::IWmeScreenSource *m_pIWmeScreenSourceDesktop;
	wme::IWmeScreenSource *m_pIWmeScreenSourceApp;
};

class CWmeScreenShareTrackTestBase : public CWmeScreenShareTestBase
{
public:
	CWmeScreenShareTrackTestBase(){
		m_hMockWnd= NULL;
		m_bRenderNow = false;
	}
	virtual ~CWmeScreenShareTrackTestBase(){
	}

	void SetUp()
	{
        CWmeScreenShareTestBase::SetUp();
	}
	void TearDown()
	{
		CWmeScreenShareTestBase::TearDown();
	}

public:
	void CreateMockWindow()
	{
		m_hMockWnd = CreateWindowX();
		m_bRenderNow = false;
	}
	void DestroyMockWindow()
	{
		DestroyWindowX(m_hMockWnd);
		m_hMockWnd = NULL;
		m_bRenderNow = false;
	}
protected:
	//
	void* m_hMockWnd;
	bool m_bRenderNow;
};




#endif