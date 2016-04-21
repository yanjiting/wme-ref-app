#include "CmDnsManager.h"
#include "gtest/gtest.h"
#ifdef WIN32
#include "./ufmock/ufmock.h"
#pragma comment(lib,"Ws2_32.lib")
#endif

class CCmDnsObserver : public ICmObserver
{
    virtual void OnObserve(LPCSTR aTopic, LPVOID aData = NULL) {
        return;
    }
};
class CCmDnsManagerTest : public testing::Test
{
public:
    CCmDnsManagerTest() {
    }

    virtual ~CCmDnsManagerTest() {
    }

    virtual void SetUp() {
        m_pDnsManger = CCmDnsManager::Instance();
        m_pDnsObserver = new CCmDnsObserver();

    }

    virtual void TearDown() {
        if (NULL != m_pDnsObserver) {
            delete m_pDnsObserver;
            m_pDnsObserver = NULL;
        }
    }
protected:
    CCmDnsManager * m_pDnsManger;
    CCmDnsObserver* m_pDnsObserver;
    //CCmComAutoPtr<CCmDnsRecord> m_pDnsRecord;
};

TEST_F(CCmDnsManagerTest,GetDNSMangerInstanceOK)
{
    CCmDnsManager * pDnsManger = CCmDnsManager::Instance();
    ASSERT_TRUE(pDnsManger);
    EXPECT_TRUE(m_pDnsManger == pDnsManger);
}
TEST_F(CCmDnsManagerTest,DNSShutDownOK)
{
    CmResult rv = m_pDnsManger->Shutdown();
    EXPECT_TRUE(CM_SUCCEEDED(rv));
}
TEST_F(CCmDnsManagerTest,DNSSyncResolveOK)
{
    CCmComAutoPtr<CCmDnsRecord> pDnsRecord;
    CmResult rv = m_pDnsManger->SyncResolve(pDnsRecord.ParaOut(),"localhost");
    EXPECT_TRUE(CM_SUCCEEDED(rv));
}
TEST_F(CCmDnsManagerTest,DNSASyncResolvehasNoObserverOK)
{
    CCmComAutoPtr<CCmDnsRecord> pDnsRecord;
    CmResult rv = m_pDnsManger->AsyncResolve(pDnsRecord.ParaOut(),"localhost",NULL,TRUE);
    EXPECT_TRUE(CM_ERROR_WOULD_BLOCK == rv);
}
TEST_F(CCmDnsManagerTest,DNSCancelResolveFail)
{
    CmResult rv = m_pDnsManger->CancelResolve(m_pDnsObserver);
    EXPECT_TRUE(CM_FAILED(rv));
}
TEST_F(CCmDnsManagerTest,DNSASyncResolvehasObserverOK)
{
    CCmComAutoPtr<CCmDnsRecord> pDnsRecord;

    CmResult rv = m_pDnsManger->AsyncResolve(pDnsRecord.ParaOut(),"www.baidu.com",m_pDnsObserver);
    EXPECT_TRUE(CM_ERROR_WOULD_BLOCK == rv);
}
TEST_F(CCmDnsManagerTest,DNSCancelResolveOK)
{
	CCmComAutoPtr<CCmDnsRecord> pDnsRecord;
	CmResult rv = m_pDnsManger->AsyncResolve(pDnsRecord.ParaOut(),"AnyURL",m_pDnsObserver,TRUE);
	EXPECT_TRUE(CM_ERROR_WOULD_BLOCK == rv);
    rv = m_pDnsManger->CancelResolve(m_pDnsObserver);
    EXPECT_TRUE(CM_SUCCEEDED(rv));
}
TEST_F(CCmDnsManagerTest,DNSASyncResolveSearchFromCache)
{
    CCmComAutoPtr<CCmDnsRecord> pDnsRecord;

    CmResult rv = m_pDnsManger->AsyncResolve(pDnsRecord.ParaOut(),"localhost");
    EXPECT_TRUE(CM_OK == rv);
}
TEST_F(CCmDnsManagerTest,DNSGetLocalIpsOK)
{
    CCmComAutoPtr<CCmDnsRecord> pDnsRecord;
    CmResult rv = m_pDnsManger->GetLocalIps(pDnsRecord.ParaOut());
    EXPECT_TRUE(CM_SUCCEEDED(rv));
}
TEST_F(CCmDnsManagerTest,DNSRefreshHostOK)
{
    CmResult rv = m_pDnsManger->RefreshHost("localhost");
    EXPECT_TRUE(CM_ERROR_WOULD_BLOCK == rv);
}
TEST_F(CCmDnsManagerTest,DNSSyncResolveFail)
{
    CCmComAutoPtr<CCmDnsRecord> pDnsRecord;
#ifdef WIN32
    //UFMocker<hostent *> fmi;
    //fmi.mock((void*)(gethostbyname));
    //fmi.setReturnValue(NULL);
#endif
    CmResult rv = m_pDnsManger->SyncResolve(pDnsRecord.ParaOut(),"TestFail");
    EXPECT_TRUE(CM_FAILED(rv));
}
TEST_F(CCmDnsManagerTest,DNSAsyncResolveFormCacheFail)
{
	CCmComAutoPtr<CCmDnsRecord> pDnsRecord;
	CmResult rv = m_pDnsManger->AsyncResolve(pDnsRecord.ParaOut(),"TestFail",m_pDnsObserver);
	EXPECT_TRUE(CM_ERROR_WOULD_BLOCK == rv);
}
TEST_F(CCmDnsManagerTest,DNSAsyncResolveTwiceOK)
{
	CCmComAutoPtr<CCmDnsRecord> pDnsRecord;
	CmResult rv = m_pDnsManger->AsyncResolve(pDnsRecord.ParaOut(),"localhost",m_pDnsObserver,TRUE);
	EXPECT_TRUE(CM_ERROR_WOULD_BLOCK == rv);
	rv = m_pDnsManger->AsyncResolve(pDnsRecord.ParaOut(),"localhost",m_pDnsObserver,TRUE);
	EXPECT_TRUE(CM_ERROR_WOULD_BLOCK == rv);
}
#if defined(CM_WIN32) || defined(CM_PORT_CLIENT)
TEST_F(CCmDnsManagerTest,DNSSetAndGetNetTypeOK)
{
	m_pDnsManger->set_network_type(NET_TYPE_IPV4);
	int iType = m_pDnsManger->get_network_type();
	EXPECT_TRUE(NET_TYPE_IPV4 == iType);
}
#endif
