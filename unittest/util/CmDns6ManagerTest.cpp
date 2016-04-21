#include "CmDns6Manager.h"
#include "gtest/gtest.h"

class CCmDns6Observer : public ICmObserver
{
    virtual void OnObserve(LPCSTR aTopic, LPVOID aData = NULL) {
        return;
    }
};
class CCmDns6ManagerTest : public testing::Test
{
public:
    CCmDns6ManagerTest() {
    }

    virtual ~CCmDns6ManagerTest() {
    }

    virtual void SetUp() {
        m_pDns6Manger = CCmDns6Manager::Instance();
        m_pDns6Observer = new CCmDns6Observer();
        m_pDns6Record = new CCmDns6Record("localhost");

    }

    virtual void TearDown() {
        if (NULL != m_pDns6Observer) {
            delete m_pDns6Observer;
            m_pDns6Observer = NULL;
        }
        if (NULL != m_pDns6Record) {
            delete m_pDns6Record;
            m_pDns6Record = NULL;
        }
    }
protected:
    CCmDns6Manager * m_pDns6Manger;
    CCmDns6Observer* m_pDns6Observer;
    CCmDns6Record * m_pDns6Record;
};

TEST_F(CCmDns6ManagerTest,GetDns6MangerInstanceOK)
{
    CCmDns6Manager * pDns6Manger = CCmDns6Manager::Instance();
    ASSERT_TRUE(pDns6Manger);
    EXPECT_TRUE(m_pDns6Manger == pDns6Manger);
}
TEST_F(CCmDns6ManagerTest,Dns6ShutDownOK)
{
    CmResult rv = m_pDns6Manger->Shutdown();
    EXPECT_TRUE(CM_SUCCEEDED(rv));
}
TEST_F(CCmDns6ManagerTest,Dns6ASyncResolvehasNoObserverOK)
{
    CCmComAutoPtr<CCmDns6Record> pDns6Record;
    CmResult rv = m_pDns6Manger->AsyncResolve(pDns6Record.ParaOut(),"localhost",NULL,TRUE);
    EXPECT_TRUE(CM_ERROR_WOULD_BLOCK == rv);
}
TEST_F(CCmDns6ManagerTest,Dns6CancelResolveFail)
{
    CmResult rv = m_pDns6Manger->CancelResolve(m_pDns6Observer);
    EXPECT_TRUE(CM_FAILED(rv));
}
TEST_F(CCmDns6ManagerTest,Dns6ASyncResolvehasObserverOK)
{
    CCmComAutoPtr<CCmDns6Record> pDns6Record;

    CmResult rv = m_pDns6Manger->AsyncResolve(pDns6Record.ParaOut(),"www.baidu.com",m_pDns6Observer);
    EXPECT_TRUE(CM_ERROR_WOULD_BLOCK == rv);
}
TEST_F(CCmDns6ManagerTest,Dns6CancelResolveOK)
{
	CCmComAutoPtr<CCmDns6Record> pDns6Record;
	CmResult rv = m_pDns6Manger->AsyncResolve(pDns6Record.ParaOut(),"TestFail",m_pDns6Observer,TRUE);
	EXPECT_TRUE(CM_ERROR_WOULD_BLOCK == rv);
    rv = m_pDns6Manger->CancelResolve(m_pDns6Observer);
    EXPECT_TRUE(CM_SUCCEEDED(rv));
}
#if defined(CM_WIN32) || defined(CM_PORT_CLIENT)
TEST_F(CCmDns6ManagerTest,Dns6AsyncResolveFormCacheFail)
{
	CCmComAutoPtr<CCmDns6Record> pDns6Record;
	CmResult rv = m_pDns6Manger->AsyncResolve(pDns6Record.ParaOut(),"TestFail",m_pDns6Observer);
	EXPECT_TRUE(CM_ERROR_WOULD_BLOCK == rv);
}
#endif
TEST_F(CCmDns6ManagerTest,Dns6ASyncResolveSearchFromCache)
{
    CCmComAutoPtr<CCmDns6Record> pDns6Record;

    CmResult rv = m_pDns6Manger->AsyncResolve(pDns6Record.ParaOut(),"localhost",m_pDns6Observer);
    EXPECT_TRUE(CM_OK == rv || CM_ERROR_WOULD_BLOCK == rv);
}
TEST_F(CCmDns6ManagerTest,Dns6RefreshHostOK)
{
    CmResult rv = m_pDns6Manger->RefreshHost("localhost");
    EXPECT_TRUE(CM_ERROR_WOULD_BLOCK == rv);
}
TEST_F(CCmDns6ManagerTest,DNS6RecordGetAddrFail)
{
    CCmDns6Record Dns6Record("localhost");
    int family = 23;
    int len= 0;
    struct sockaddr_storage * pTest = Dns6Record.GetSockAddr(family,&len);
    EXPECT_TRUE(NULL == pTest);
}
/*
#ifdef CM_PORT_CLIENT
TEST_F(CCmDns6ManagerTest,Dnsgethostbyname_dns_r6OK)
{
	char szBuffer[1024] = {0};
	int nError = -1;
	struct hostent *pheResultBuf =reinterpret_cast<struct hostent *>(szBuffer);
	CmResult rv = gethostbyname_dns_r6("loaclhost",
		pheResultBuf,
		szBuffer + sizeof(hostent),
		sizeof(szBuffer) - sizeof(hostent),
		&pheResultBuf,
		&nError, 28);
	EXPECT_TRUE(CM_SUCCEEDED(rv));
}
TEST_F(CCmDns6ManagerTest,Dnsgethostbyname_dns_r6Fail)
{
	char szBuffer[1024] = {0};
	int nError = -1;
	struct hostent *pheResultBuf =reinterpret_cast<struct hostent *>(szBuffer);
	CmResult rv = gethostbyname_dns_r6("www.baidu.com",
		pheResultBuf,
		szBuffer + sizeof(hostent),
		sizeof(szBuffer) - sizeof(hostent),
		&pheResultBuf,
		&nError, 28);
	EXPECT_TRUE(CM_FAILED(rv));
}
#endif
*/
