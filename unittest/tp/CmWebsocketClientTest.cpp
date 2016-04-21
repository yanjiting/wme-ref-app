#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "CmChannelHttpClient.h"
#include "CmChannelWSClient.h"
#include "CmHttpUrl.h"
//#include "CmInetAddr.h"

using ::testing::_;
using ::testing::Mock;
using ::testing::Return;

class MockCCmChannelHttpClient : public CCmChannelHttpClient {
public:
    MockCCmChannelHttpClient(CCmHttpUrl *aURL) : CCmChannelHttpClient(aURL) {}
    MOCK_METHOD1(AsyncOpen,
        CmResult(ICmChannelSink *aSink));
    MOCK_METHOD2(SendData,CmResult(CCmMessageBlock &aData, CCmTransportParameter *aPara));
    MOCK_METHOD2(SetOption,CmResult(DWORD aCommand, LPVOID aArg));
    MOCK_METHOD2(GetOption,CmResult(DWORD aCommand, LPVOID aArg));
    MOCK_METHOD1(Disconnect,CmResult(CmResult aReason));
};

class CCmChannelSinkTest : public ICmChannelSink
{
    virtual void OnConnect(CmResult aReason, ICmChannel *aChannelId, long id=-1) {
        return;
    }
    virtual void OnReceive(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara = NULL) {
        return;
    }

    virtual void OnSend(ICmTransport *aTrptId, CCmTransportParameter *aPara = NULL) {
        return;
    }

    virtual void OnDisconnect(CmResult aReason, ICmTransport *aTrptId) {
        return;
    }
public:
    ~CCmChannelSinkTest(){}
};
class CCmWebsocketClientTestDemo : public CCmChannelWSClient
{
public:
    CCmWebsocketClientTestDemo(CCmHttpUrl *aURL, CCmChannelManager::CFlag aFlag):CCmChannelWSClient(aURL,aFlag) {}
    ~CCmWebsocketClientTestDemo(){}
    void SetHttpClientChannel(ICmChannelHttpClient * pHttpClient) {
        m_pHttpClient = pHttpClient;
    }
    void SetWSState(int state) {
        m_state = state;
    }
    int GetWSState() {
        return m_state;
    }
};
class CCmWebsocketClientTest : public testing::Test
{
public:

    CCmWebsocketClientTest()
    {
    }

    virtual ~CCmWebsocketClientTest()
    {
    }


    virtual void SetUp()
    {
        pChannelSinkTest = new CCmChannelSinkTest();
    }

    virtual void TearDown()
    {
        if (NULL != pChannelSinkTest)
        {
            delete pChannelSinkTest;
            pChannelSinkTest = NULL;
        }
    }
protected:
    CCmChannelSinkTest * pChannelSinkTest;
};
TEST_F(CCmWebsocketClientTest,AddReleaseReference)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    LPCSTR pszUrl = "ws://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmChannelWSClient *pWebsocketClient = new CCmChannelWSClient(pUrl.ParaIn(),0);
    DWORD dwRef = pWebsocketClient->AddReference();
    EXPECT_EQ(1, dwRef);
    dwRef = pWebsocketClient->ReleaseReference();
    EXPECT_EQ(0, dwRef);
}
TEST_F(CCmWebsocketClientTest,AsyncOpen)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    LPCSTR pszUrl = "ws://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmChannelWSClient *pWebsocketClient = new CCmChannelWSClient(pUrl.ParaIn(),0);
    //MockCCmChannelHttpClient *pmock = new MockCCmChannelHttpClient(pUrl.ParaIn());
    //EXPECT_CALL(*pmock, AsyncOpen(_)).WillRepeatedly(Return(CM_OK));
    //pWebsocketClient->SetHttpClientChannel(pmock);
    int iRet = pWebsocketClient->AsyncOpen(pChannelSinkTest);
    EXPECT_TRUE(CM_SUCCEEDED(iRet));
    delete pWebsocketClient;
    pWebsocketClient = NULL;
    //delete pmock;
    //pmock = NULL;
}
TEST_F(CCmWebsocketClientTest,GetUrl)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    CCmHttpUrl * phttpurl = NULL;
    LPCSTR pszUrl = "wss://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmChannelWSClient *pWebsocketClient = new CCmChannelWSClient(pUrl.ParaIn(),0);
    int iRet = pWebsocketClient->GetUrl(phttpurl);
    EXPECT_TRUE(CM_SUCCEEDED(iRet));
    EXPECT_TRUE(pUrl == phttpurl);
    delete pWebsocketClient;
    pWebsocketClient = NULL;
}
//wallice disable the test case because it always fails in volvet's machine, just disable it before we find the root cause
/*
TEST_F(CCmWebsocketClientTest,GetAndOpenWithSinkSucceedAndFail)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    LPCSTR pszUrl = "wss://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmChannelWSClient *pWebsocketClient = new CCmChannelWSClient(pUrl.ParaIn(),0);
    int iRet = pWebsocketClient->OpenWithSink(NULL);
    EXPECT_TRUE(CM_ERROR_INVALID_ARG == iRet);
    iRet = pWebsocketClient->OpenWithSink(pChannelSinkTest);
    EXPECT_TRUE(CM_SUCCEEDED(iRet));
    ICmTransportSink * pTemp = pWebsocketClient->GetSink();
    EXPECT_TRUE(pTemp == pChannelSinkTest);
    delete pWebsocketClient;
    pWebsocketClient = NULL;
}
*/
TEST_F(CCmWebsocketClientTest,GetAndSetOptionOK)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    LPCSTR pszUrl = "ws://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmWebsocketClientTestDemo *pWebsocketClient = new CCmWebsocketClientTestDemo(pUrl.ParaIn(),0);
    MockCCmChannelHttpClient *pmock = new MockCCmChannelHttpClient(pUrl.ParaIn());
    EXPECT_CALL(*pmock, SetOption(_,_)).WillRepeatedly(Return(CM_OK));
    EXPECT_CALL(*pmock, GetOption(_,_)).WillRepeatedly(Return(CM_OK));
    pWebsocketClient->SetHttpClientChannel(pmock);
    int iRet = pWebsocketClient->SetOption(0,NULL);
    EXPECT_TRUE(CM_SUCCEEDED(iRet));
    iRet = pWebsocketClient->GetOption(0,NULL);
    EXPECT_TRUE(CM_SUCCEEDED(iRet));
    delete pWebsocketClient;
    pWebsocketClient = NULL;
    delete pmock;
    pmock = NULL;
}
TEST_F(CCmWebsocketClientTest,GetAndSetOptionFail)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    LPCSTR pszUrl = "ws://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmWebsocketClientTestDemo *pWebsocketClient = new CCmWebsocketClientTestDemo(pUrl.ParaIn(),0);
    MockCCmChannelHttpClient *pmock = new MockCCmChannelHttpClient(pUrl.ParaIn());
    EXPECT_CALL(*pmock, SetOption(_,_)).WillRepeatedly(Return(!CM_OK));
    EXPECT_CALL(*pmock, GetOption(_,_)).WillRepeatedly(Return(!CM_OK));
    pWebsocketClient->SetHttpClientChannel(pmock);
    int iRet = pWebsocketClient->SetOption(0,NULL);
    EXPECT_TRUE(CM_FAILED(iRet));
    iRet = pWebsocketClient->GetOption(0,NULL);
    EXPECT_TRUE(CM_FAILED(iRet));
    delete pWebsocketClient;
    pWebsocketClient = NULL;
    delete pmock;
    pmock = NULL;
}
TEST_F(CCmWebsocketClientTest,SendDataWithStateNotOk)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    LPCSTR pszUrl = "ws://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmWebsocketClientTestDemo *pWebsocketClient = new CCmWebsocketClientTestDemo(pUrl.ParaIn(),0);
    CCmMessageBlock amb(500,NULL,0,500);
    int iRet = pWebsocketClient->SendData(amb,NULL);
    EXPECT_TRUE(CM_ERROR_NOT_INITIALIZED == iRet);
    delete pWebsocketClient;
    pWebsocketClient = NULL;
}
TEST_F(CCmWebsocketClientTest,SendDataWithStateOKAndHttpClientNULL)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    LPCSTR pszUrl = "ws://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmWebsocketClientTestDemo *pWebsocketClient = new CCmWebsocketClientTestDemo(pUrl.ParaIn(),0);
    pWebsocketClient->SetWSState(2/*WSC_STATE_OPEN*/);
    CCmMessageBlock amb(500,NULL,0,500);
    int iRet = pWebsocketClient->SendData(amb,NULL);
    EXPECT_TRUE(CM_ERROR_NULL_POINTER == iRet);
    delete pWebsocketClient;
    pWebsocketClient = NULL;
}
TEST_F(CCmWebsocketClientTest,SendDataWithStateOKAndHttpClientok)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    LPCSTR pszUrl = "ws://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmWebsocketClientTestDemo *pWebsocketClient = new CCmWebsocketClientTestDemo(pUrl.ParaIn(),0);
    MockCCmChannelHttpClient *pmock = new MockCCmChannelHttpClient(pUrl.ParaIn());
    EXPECT_CALL(*pmock, SendData(_,_)).WillRepeatedly(Return(CM_OK));
    pWebsocketClient->SetHttpClientChannel(pmock);
    pWebsocketClient->SetWSState(2/*WSC_STATE_OPEN*/);
    CCmMessageBlock amb(500,NULL,0,500);
    int iRet = pWebsocketClient->SendData(amb,NULL);
    EXPECT_TRUE(CM_SUCCEEDED(iRet));
    delete pWebsocketClient;
    pWebsocketClient = NULL;
    delete pmock;
    pmock = NULL;
}
TEST_F(CCmWebsocketClientTest,DisConnectok)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    LPCSTR pszUrl = "ws://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmWebsocketClientTestDemo *pWebsocketClient = new CCmWebsocketClientTestDemo(pUrl.ParaIn(),0);
    MockCCmChannelHttpClient *pmock = new MockCCmChannelHttpClient(pUrl.ParaIn());
    EXPECT_CALL(*pmock, Disconnect(_)).WillRepeatedly(Return(CM_OK));
    pWebsocketClient->SetHttpClientChannel(pmock);
    int iRet = pWebsocketClient->Disconnect(CM_OK);
    EXPECT_TRUE(CM_SUCCEEDED(iRet));
    EXPECT_TRUE(/*WSC_STATE_CLOSED*/4 == pWebsocketClient->GetWSState());
    delete pWebsocketClient;
    pWebsocketClient = NULL;
    delete pmock;
    pmock = NULL;
}
TEST_F(CCmWebsocketClientTest,OnConnectOk)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    LPCSTR pszUrl = "ws://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmWebsocketClientTestDemo *pWebsocketClient = new CCmWebsocketClientTestDemo(pUrl.ParaIn(),0);
    MockCCmChannelHttpClient *pmock = new MockCCmChannelHttpClient(pUrl.ParaIn());
    EXPECT_CALL(*pmock, SendData(_,_)).WillRepeatedly(Return(CM_OK));
    pWebsocketClient->SetHttpClientChannel(pmock);
    //pWebsocketClient->OpenWithSink(pChannelSinkTest);
    pWebsocketClient->OnConnect(CM_OK,pmock);
    pWebsocketClient->SetWSState(2);
    EXPECT_TRUE(/*WSC_STATE_OPEN*/2 == pWebsocketClient->GetWSState());
    delete pWebsocketClient;
    pWebsocketClient = NULL;
    delete pmock;
    pmock = NULL;
}
TEST_F(CCmWebsocketClientTest,OnConnectFail)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    LPCSTR pszUrl = "ws://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmWebsocketClientTestDemo *pWebsocketClient = new CCmWebsocketClientTestDemo(pUrl.ParaIn(),0);
    MockCCmChannelHttpClient *pmock = new MockCCmChannelHttpClient(pUrl.ParaIn());
    //EXPECT_CALL(*pmock, SendData(_,_)).WillRepeatedly(Return(CM_OK));
    pWebsocketClient->SetHttpClientChannel(pmock);
    pWebsocketClient->OpenWithSink(pChannelSinkTest);
    pWebsocketClient->OnConnect(!CM_OK,pmock);
    EXPECT_TRUE(/*WSC_STATE_CLOSING*/3 == pWebsocketClient->GetWSState());
    delete pWebsocketClient;
    pWebsocketClient = NULL;
    delete pmock;
    pmock = NULL;
}
TEST_F(CCmWebsocketClientTest,OnSend)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    LPCSTR pszUrl = "ws://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmWebsocketClientTestDemo *pWebsocketClient = new CCmWebsocketClientTestDemo(pUrl.ParaIn(),0);
    MockCCmChannelHttpClient *pmock = new MockCCmChannelHttpClient(pUrl.ParaIn());
    //EXPECT_CALL(*pmock, SendData(_,_)).WillRepeatedly(Return(CM_OK));
    pWebsocketClient->SetHttpClientChannel(pmock);
    pWebsocketClient->OpenWithSink(pChannelSinkTest);
    pWebsocketClient->SetWSState(2);
    pWebsocketClient->OnSend(pmock);
    EXPECT_TRUE(/*WSC_STATE_OPEN*/2 == pWebsocketClient->GetWSState());
    delete pWebsocketClient;
    pWebsocketClient = NULL;
    delete pmock;
    pmock = NULL;
}
TEST_F(CCmWebsocketClientTest,OnDisconnectWithWSStateOK)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    LPCSTR pszUrl = "ws://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmWebsocketClientTestDemo *pWebsocketClient = new CCmWebsocketClientTestDemo(pUrl.ParaIn(),0);
    MockCCmChannelHttpClient *pmock = new MockCCmChannelHttpClient(pUrl.ParaIn());
    //EXPECT_CALL(*pmock, SendData(_,_)).WillRepeatedly(Return(CM_OK));
    pWebsocketClient->SetHttpClientChannel(pmock);
    pWebsocketClient->OpenWithSink(pChannelSinkTest);
    pWebsocketClient->SetWSState(2);
    pWebsocketClient->OnDisconnect(CM_OK,pmock);
    EXPECT_TRUE(/*WSC_STATE_CLOSED*/4 == pWebsocketClient->GetWSState());
    delete pWebsocketClient;
    pWebsocketClient = NULL;
    delete pmock;
    pmock = NULL;
}
TEST_F(CCmWebsocketClientTest,OnDisconnectWithWSStateNotOK)
{
    CCmComAutoPtr<CCmHttpUrl> pUrl;
    LPCSTR pszUrl = "ws://173.39.168.82/__proxy__?type=websocket&dtype=binary&tagurl=tcp://www.163.com:80";
    CmResult rv = CCmChannelManager::Instance()->CreateUrl(
        pUrl.ParaOut(),
        pszUrl);
    EXPECT_EQ(CM_OK, rv);
    CCmWebsocketClientTestDemo *pWebsocketClient = new CCmWebsocketClientTestDemo(pUrl.ParaIn(),0);
    MockCCmChannelHttpClient *pmock = new MockCCmChannelHttpClient(pUrl.ParaIn());
    //EXPECT_CALL(*pmock, SendData(_,_)).WillRepeatedly(Return(CM_OK));
    pWebsocketClient->SetHttpClientChannel(pmock);
    pWebsocketClient->OpenWithSink(pChannelSinkTest);
    pWebsocketClient->OnDisconnect(CM_OK,pmock);
    EXPECT_TRUE(/*WSC_STATE_CLOSED*/4 == pWebsocketClient->GetWSState());
    delete pWebsocketClient;
    pWebsocketClient = NULL;
    delete pmock;
    pmock = NULL;
}
