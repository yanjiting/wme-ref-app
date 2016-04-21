#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "CWmeScreenTrackTestBase.h"
#include "WmeAsScreenCaptureEgnine.h"

//Windows DLL just export limited interface ,cannot test internal class
//#if defined(MACOS) || defined(WIN32)
#if defined(MACOS) 

using namespace wme;

class CWmeScreenCaptureEngineTest : public CWmeScreenShareTestBase
                                    , public IWmeScreenCaptureEngineEventObserver
                                    , public IWmeAsScreenCaptureEngineDeliverer
{
public:
	CWmeScreenCaptureEngineTest()
	{
        m_pWmeScreenCaptureEngine = new CWmeAsScreenCaptureEngine(this);
		if(m_pWmeScreenCaptureEngine!=NULL)
			m_pWmeScreenCaptureEngine->AddRef();
        
		m_pWmeScreenCaptureEngine->AddObserver(WMEIID_IWmeScreenCaptureEngineEventObserver,(IWmeScreenCaptureEngineEventObserver*)this);
        
        m_eWmeExternalCapturerStatus = WmeExternalCapturerStatusUnknown;
        m_stCaptureSize.width = 0;
        m_stCaptureSize.height = 0;
        m_nCapturedFrameCount = 0;
    }

	virtual ~CWmeScreenCaptureEngineTest()
	{
		if(NULL != m_pWmeScreenCaptureEngine)
		{
            m_pWmeScreenCaptureEngine->RemoveObserver(WMEIID_IWmeScreenCaptureEngineEventObserver,(IWmeScreenCaptureEngineEventObserver*)this);
			m_pWmeScreenCaptureEngine->Release();
			m_pWmeScreenCaptureEngine = NULL;
		}
	}

	virtual void SetUp() {
        CWmeScreenShareTestBase::SetUp();
        RefreshScreenSource();
    }
	virtual void TearDown() {
        CWmeScreenShareTestBase::TearDown();
    }
    
    //IWmeScreenCaptureEngineEventObserver
    virtual WMERESULT OnCaptureStatusChanged(IWmeMediaEventNotifier *pNotifier, WmeExternalCapturerStatus eCapturerStatus){
        m_eWmeExternalCapturerStatus = eCapturerStatus;
        return WME_S_OK;
    }
    virtual WMERESULT OnCaptureDisplaySizeChanged(IWmeMediaEventNotifier *pNotifier, WmeSize &stCaptureSize){
        m_stCaptureSize = stCaptureSize;
        return WME_S_OK;

    }
    virtual WMERESULT OnCaptureData(IWmeMediaEventNotifier *pNotifier,IWmeMediaPackage *pIWmeMediaPackage){
        if(pIWmeMediaPackage)
            m_nCapturedFrameCount ++ ;
        return WME_S_OK;
    }
    virtual WMERESULT OnError(IWmeMediaEventNotifier *pNotifier, uint32_t nErrorCode){
        return WME_S_OK;
    }
    // IWmeMediaEventObserver interface
	virtual WMERESULT OnWillAddByNotifier(wme::IWmeMediaEventNotifier* pNotifer, const wme::WMEIID & ulObserverID) { return WME_S_OK; }
	virtual WMERESULT OnDidRemoveByNotifier(wme::IWmeMediaEventNotifier* pNotifer, const wme::WMEIID & ulObserverID) { return WME_S_OK; }

    //IWmeAsScreenCaptureEngineDeliverer
    virtual WMERESULT DeliverImage(shark::IWseVideoSample* pSample) {
        if(pSample)
            m_nCapturedFrameCount ++ ;
        return WME_S_OK;
    }
public:
	//member
	IWmeScreenCaptureEngine* m_pWmeScreenCaptureEngine;
    WmeExternalCapturerStatus m_eWmeExternalCapturerStatus;
    WmeSize m_stCaptureSize;
    int m_nCapturedFrameCount;
};


TEST_F(CWmeScreenCaptureEngineTest, Start_Stop_Desktop)
{
    m_eWmeExternalCapturerStatus = WmeExternalCapturerStatusUnknown;
    EXPECT_NE(WME_S_OK, m_pWmeScreenCaptureEngine->Start());
    m_pWmeScreenCaptureEngine->AddScreenSource(m_pIWmeScreenSourceDesktop);
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->Start());
    EXPECT_EQ(WmeExternalCapturerStatusStarted, m_eWmeExternalCapturerStatus);
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->Stop());
    EXPECT_EQ(WmeExternalCapturerStatusStopped, m_eWmeExternalCapturerStatus);
}

TEST_F(CWmeScreenCaptureEngineTest, Start_Stop_App)
{
    m_eWmeExternalCapturerStatus = WmeExternalCapturerStatusUnknown;
    EXPECT_NE(WME_S_OK, m_pWmeScreenCaptureEngine->Start());
    m_pWmeScreenCaptureEngine->AddScreenSource(m_pIWmeScreenSourceApp);
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->Start());
    EXPECT_EQ(WmeExternalCapturerStatusStarted, m_eWmeExternalCapturerStatus);
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->Stop());
    EXPECT_EQ(WmeExternalCapturerStatusStopped, m_eWmeExternalCapturerStatus);
}
TEST_F(CWmeScreenCaptureEngineTest, Pause_Resume)
{
    
    EXPECT_NE(WME_S_OK, m_pWmeScreenCaptureEngine->Pause());
    EXPECT_NE(WmeExternalCapturerStatusPaused, m_eWmeExternalCapturerStatus);
    EXPECT_NE(WME_S_OK, m_pWmeScreenCaptureEngine->Resume());
    EXPECT_NE(WmeExternalCapturerStatusStarted, m_eWmeExternalCapturerStatus);
    
    m_eWmeExternalCapturerStatus = WmeExternalCapturerStatusUnknown;
    m_pWmeScreenCaptureEngine->AddScreenSource(m_pIWmeScreenSourceApp);
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->Start());
    EXPECT_EQ(WmeExternalCapturerStatusStarted, m_eWmeExternalCapturerStatus);
    
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->Pause());
    EXPECT_EQ(WmeExternalCapturerStatusPaused, m_eWmeExternalCapturerStatus);

    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->Resume());
    EXPECT_EQ(WmeExternalCapturerStatusStarted, m_eWmeExternalCapturerStatus);
    
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->Stop());
    EXPECT_EQ(WmeExternalCapturerStatusStopped, m_eWmeExternalCapturerStatus);
    
}

TEST_F(CWmeScreenCaptureEngineTest, GetCapturerStatus)
{
    WmeExternalCapturerStatus eWmeExternalCapturerStatus = WmeExternalCapturerStatusUnknown;
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->GetCapturerStatus(eWmeExternalCapturerStatus));
    EXPECT_EQ(WmeExternalCapturerStatusStopped, eWmeExternalCapturerStatus);
    
    m_pWmeScreenCaptureEngine->AddScreenSource(m_pIWmeScreenSourceApp);
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->Start());
    eWmeExternalCapturerStatus = WmeExternalCapturerStatusUnknown;
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->GetCapturerStatus(eWmeExternalCapturerStatus));
    EXPECT_EQ(WmeExternalCapturerStatusStarted, eWmeExternalCapturerStatus);
    
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->Pause());
    eWmeExternalCapturerStatus = WmeExternalCapturerStatusUnknown;
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->GetCapturerStatus(eWmeExternalCapturerStatus));
    EXPECT_EQ(WmeExternalCapturerStatusPaused, eWmeExternalCapturerStatus);
    
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->Resume());
    eWmeExternalCapturerStatus = WmeExternalCapturerStatusUnknown;
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->GetCapturerStatus(eWmeExternalCapturerStatus));
    EXPECT_EQ(WmeExternalCapturerStatusStarted, eWmeExternalCapturerStatus);

    
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->Stop());
    eWmeExternalCapturerStatus = WmeExternalCapturerStatusUnknown;
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->GetCapturerStatus(eWmeExternalCapturerStatus));
    EXPECT_EQ(WmeExternalCapturerStatusStopped, eWmeExternalCapturerStatus);
}

TEST_F(CWmeScreenCaptureEngineTest, AddScreenSource)
{
    EXPECT_NE(WME_S_OK, m_pWmeScreenCaptureEngine->AddScreenSource(NULL));
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->AddScreenSource(m_pIWmeScreenSourceApp));
}

TEST_F(CWmeScreenCaptureEngineTest, RemoveScreenSource)
{
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->RemoveScreenSource(NULL));
    EXPECT_EQ(WME_S_OK, m_pWmeScreenCaptureEngine->RemoveScreenSource(m_pIWmeScreenSourceApp));
}

TEST_F(CWmeScreenCaptureEngineTest, SetExternalCapturer)
{
    //TBD for iOS content capturer
}

TEST_F(CWmeScreenCaptureEngineTest, SetExternalInputter)
{
    //TBD for iOS content capturer
}

#endif //#if defined(MACOS) || defined(WIN32)
