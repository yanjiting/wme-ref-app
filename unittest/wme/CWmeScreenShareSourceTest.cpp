#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeEngine.h"
#include "WmeInterface.h"
#include "WseUtil.h"

#include "WmeScreenSource.h"
#include "WmeAsScreenSourceFilter.h"
#include "WmeAsScreenSourceEnumerator.h"
#include "../src/wbxbase_reference_impl.h"

//Windows DLL just export limited interface ,cannot test internal class
//#if defined(MACOS) || defined(WIN32)
#ifdef defined(MACOS) 

const int g_nFakeWmeScreenSourceCount = 5;

class CFakeShareSource: public IShareSource, public CWbxReference{
public:
    IMPLEMENT_WBX_REFERENCE();
    
	virtual WBXInt32 GetSourceUniqueId() {return 0;};
	virtual ShareSourceType GetSourceType() {return SST_UNKNOWN;};
	virtual WBXBool IsShared() {return WBXFalse;};
    virtual WBXRect GetSourceRect() {return NULL;};
};

class CFakeShareSourceEnum : public IShareSourceEnumerator
{
public:
    CFakeShareSourceEnum()
    {
        m_pIshareSource = new CFakeShareSource;
        m_pIshareSource->AddRef();
    }
    
    virtual ~CFakeShareSourceEnum()
    {
        SAFE_RELEASE(m_pIshareSource);
    }
    
    //for IShareSourceEnumerator
	virtual WBXResult GetNumber(WBXInt32 &iNumber)
    {
        iNumber = g_nFakeWmeScreenSourceCount;
        return WBX_SUCCESS;
    }
    
	virtual WBXResult GetSource(WBXInt32 iIndex, IShareSource **ppSource)
    {
        *ppSource = m_pIshareSource;
        return WBX_SUCCESS;
    }
    
	virtual WBXVoid  SetFilter(IShareSourceFilter *pFilter){return;}
    virtual WBXVoid   Refresh() {return;}
    
    // for IShareSourceSinkMgr
    virtual WBXResult AddSink(IShareSourceEnumSink *pIShareSourceEnumSink){return WBX_ERR_NO_IMPLEMENT;}
	virtual WBXResult RemoveSink(IShareSourceEnumSink *pIShareSourceEnumSink){return WBX_ERR_NO_IMPLEMENT;}
	virtual WBXResult CleanSink(){return WBX_ERR_NO_IMPLEMENT;}
private:
    //std::vector<IShareSource*>   m_vctShareSource;
    IShareSource * m_pIshareSource;
};

class CFakeWmeAsScreenSourceEnum: public wme::CWmeAsScreenSourceEnumerator
{
public:
    CFakeWmeAsScreenSourceEnum(wme::WmeScreenSourceType wmeScreenSourceType)
                :wme::CWmeAsScreenSourceEnumerator(wmeScreenSourceType)
    {
        if(m_pIShareSourceEnumerator!=WBXNull)
            delete m_pIShareSourceEnumerator;
		wme::CWmeAsScreenSourceEnumerator::Cleanup();
        m_pIShareSourceEnumerator = new CFakeShareSourceEnum;
        wme::CWmeAsScreenSource *pSource = NULL;
        for (int i = 0; i < g_nFakeWmeScreenSourceCount; i++)
        {
            pSource = new wme::CWmeAsScreenSource(NULL);
            m_mapSource.insert(std::map<int, wme::CWmeAsScreenSource*>::value_type(i, pSource));
        }
    }
    
    virtual ~CFakeWmeAsScreenSourceEnum()
    {
        for (std::map<int, wme::CWmeAsScreenSource*>::iterator iter = m_mapSource.begin();
             iter != m_mapSource.end(); ++iter)
		{
			delete (*iter).second;
		}
		m_mapSource.clear();
        if(m_pIShareSourceEnumerator!=NULL)
            delete m_pIShareSourceEnumerator;
        m_pIShareSourceEnumerator = NULL;
    }
    
};

class CWmeScreenShareSourceTestBase : public testing::Test,
								public wme::IWmeScreenSourceEnumeratorEventObserver
{
public:
	CWmeScreenShareSourceTestBase(){
		m_pEngine = NULL;
		m_pWmeScreenSourceEnumerator = NULL;
	}
	virtual ~CWmeScreenShareSourceTestBase(){
	}

	void SetUp()
	{
		//
		wme::WmeCreateMediaEngine(&m_pEngine);
		//if(m_pEngine!=NULL)
		//	m_pEngine->CreateScreenSourceEnumerator(&m_pWmeScreenSourceEnumerator);
        m_pWmeScreenSourceEnumerator = new CFakeWmeAsScreenSourceEnum(wme::WmeScreenSourceTypeApplication);
	}
	void TearDown()
	{
		//
		SAFE_RELEASE(m_pEngine);
		SAFE_RELEASE(m_pWmeScreenSourceEnumerator);
	}

	//IWmeScreenSourceEnumeratorEventObserver	
	virtual WMERESULT OnScreenSourceChanged(wme::IWmeMediaEventNotifier *pNotifier, 
											wme::WmeScreenSourceEvent event,
											wme::IWmeScreenSource *pScreenSource)
	{
			return WME_S_OK;
	}
	//IWmeMediaEventObserver
	virtual WMERESULT OnWillAddByNotifier(wme::IWmeMediaEventNotifier* pNotifer, const wme::WMEIID & ulObserverID){return WME_S_OK;}
	virtual WMERESULT OnDidRemoveByNotifier(wme::IWmeMediaEventNotifier* pNotifer, const wme::WMEIID & ulObserverID){return WME_S_OK;}
protected:
	wme::IWmeMediaEngine* m_pEngine;
	wme::IWmeScreenSourceEnumerator *m_pWmeScreenSourceEnumerator;
};


TEST_F(CWmeScreenShareSourceTestBase,CreateScreenSourceEnumerator )
{
	ASSERT_TRUE(m_pEngine!=NULL);
	wme::IWmeScreenSourceEnumerator *pWmeScreenSourceEnumerator = NULL;

	EXPECT_EQ(WME_S_OK, m_pEngine->CreateScreenSourceEnumerator(&pWmeScreenSourceEnumerator,wme::WmeScreenSourceTypeApplication));
	ASSERT_TRUE(pWmeScreenSourceEnumerator!=NULL);
	pWmeScreenSourceEnumerator->Release();
}

TEST_F(CWmeScreenShareSourceTestBase,GetNumber )
{
	ASSERT_TRUE(m_pWmeScreenSourceEnumerator!=NULL);
    
    int32_t iNumber;
    EXPECT_EQ(WME_S_OK, m_pWmeScreenSourceEnumerator->GetNumber(iNumber));
    EXPECT_EQ(g_nFakeWmeScreenSourceCount, iNumber);
 }

TEST_F(CWmeScreenShareSourceTestBase,GetSource )
{
	ASSERT_TRUE(m_pWmeScreenSourceEnumerator!=NULL);

    wme::IWmeScreenSource * pSource = NULL;
    EXPECT_EQ(WME_E_FAIL, m_pWmeScreenSourceEnumerator->GetSource(-1, &pSource));
    EXPECT_EQ(WME_E_FAIL, m_pWmeScreenSourceEnumerator->GetSource(g_nFakeWmeScreenSourceCount, &pSource));
    for(int i=0; i<g_nFakeWmeScreenSourceCount; i++){
        EXPECT_EQ(WME_S_OK, m_pWmeScreenSourceEnumerator->GetSource(i, &pSource));
        EXPECT_TRUE(pSource  != NULL);
    }
}

TEST_F(CWmeScreenShareSourceTestBase,Refresh )
{
	ASSERT_TRUE(m_pWmeScreenSourceEnumerator!=NULL);
    EXPECT_EQ(WME_S_OK, m_pWmeScreenSourceEnumerator->Refresh());
}

TEST_F(CWmeScreenShareSourceTestBase,SetFilter )
{
	ASSERT_TRUE(m_pWmeScreenSourceEnumerator!=NULL);
    wme::IWmeScreenSourceFilter *pFilter = new wme::CWmeAsScreenSourceFilter;
    EXPECT_EQ(WME_E_FAIL, m_pWmeScreenSourceEnumerator->SetFilter(pFilter));
}

TEST_F(CWmeScreenShareSourceTestBase,AddObserver )
{
	ASSERT_TRUE(m_pWmeScreenSourceEnumerator!=NULL);
	//TBD
}


TEST_F(CWmeScreenShareSourceTestBase,RemoveObserver )
{
	ASSERT_TRUE(m_pWmeScreenSourceEnumerator!=NULL);
	//TBD
}

#endif