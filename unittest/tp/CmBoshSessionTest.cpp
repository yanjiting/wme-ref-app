
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Mock_CmConnectionInterface.h"
#include "Mock_CmOnePortInterface.h"
#include "Mock_CmBaseHTTPExProtoHandle.h"
#include "Mock_CmHttpInterface.h"

#include "CmBoshDataBuild.h"
#include "CmBoshBody.h"
#include "CmBoshTransaction.h"
#include "CmBoshSession.h"
#include "CmBoshProvider.h"


class CBoshSessionTest : public testing::Test
{
public:
    CBoshSessionTest()
    {
        m_pBoshSession = NULL;
    }

    virtual ~CBoshSessionTest()
    {
        if(NULL != m_pBoshSession)
        {
            m_pBoshSession->ReleaseReference();
            m_pBoshSession = NULL;
        }
    }

    virtual void SetUp()
    {
        if(NULL != m_pBoshSession)
        {
            m_pBoshSession->ReleaseReference();
            m_pBoshSession = NULL;
        }

        EXPECT_CALLHTTPExProtoHandle();

        m_strSid = CBoshProvider::GenerateSid();

        m_pBoshSession = new CBoshSession(m_strSid, &m_HTTPExHandle, &m_HTTPExProtoSink, &m_OnePortSink);
        m_pBoshSession->AddReference();
    }

    virtual void TearDown()
    {
    }

protected:
    void EXPECT_CALLHttpServer()
    {
        EXPECT_CALL(m_HttpServer, AddReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(m_HttpServer, ReleaseReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(m_HttpServer, OpenWithSink(testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer, Disconnect(testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer, SendData(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));

        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer, SetOrAddResponseHeader(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));

        EXPECT_CALL(m_HttpServer, SetOption(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer, GetOption(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
    
        EXPECT_CALL(m_HttpServer2, AddReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(m_HttpServer2, ReleaseReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(m_HttpServer2, OpenWithSink(testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer2, Disconnect(testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer2, SendData(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));

        EXPECT_CALL(m_HttpServer2, GetRequestHeader(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer2, SetOrAddResponseHeader(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));

        EXPECT_CALL(m_HttpServer2, SetOption(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer2, GetOption(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
    }

    void EXPECT_CALLOnePortSink()
    {
        EXPECT_CALL(m_OnePortSink, OnServerCreation(testing::_));
    }

    void EXPECT_CALLHTTPExProtoHandle()
    {
        EXPECT_CALL(m_HTTPExHandle, AddReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(m_HTTPExHandle, ReleaseReference()).WillRepeatedly(testing::Return(1));
    }

    void EXPECT_CALLHTTPExProtoSink()
    {
        EXPECT_CALL(m_HTTPExProtoSink, OnDisconnect(testing::_, testing::_));
    }

    void EXPECT_CALLTransportSink()
    {
        EXPECT_CALL(m_TransportSink, OnReceive(testing::_, testing::_, testing::_));
    }


protected:
    CBoshSession*                               m_pBoshSession;

    string                                      m_strSid;

    MockICmTransportSink                        m_TransportSink;
    MockICmBaseHTTPExProtoHandle                m_HTTPExHandle;
    MockICmBaseHTTPExProtoHandleSink            m_HTTPExProtoSink; 
    MockICmOnePortSink                          m_OnePortSink;
    MockICmChannelHttpServer                    m_HttpServer;
    MockICmChannelHttpServer                    m_HttpServer2;

};


TEST_F(CBoshSessionTest, ReferenceTest)
{

    ASSERT_TRUE(NULL != m_pBoshSession);

    DWORD dwRef1 = m_pBoshSession->GetReference();
    {
        int nTimes = 100;
        for(int i = 0; i < nTimes; i++)
        {
            m_pBoshSession->AddReference();
        }
        
        EXPECT_EQ(dwRef1 + nTimes, m_pBoshSession->GetReference());

        for(int i = 0; i < nTimes; i++)
        {
            m_pBoshSession->ReleaseReference();
        }
        
        EXPECT_EQ(dwRef1, m_pBoshSession->GetReference());
    }
    DWORD dwRef2 = m_pBoshSession->GetReference();

    EXPECT_EQ(dwRef1, dwRef2);
}

TEST_F(CBoshSessionTest, OpenWithSinkTest)
{
    ASSERT_TRUE(NULL != m_pBoshSession);

    EXPECT_TRUE(NULL == m_pBoshSession->GetSink());

    EXPECT_EQ(CM_OK, m_pBoshSession->OpenWithSink(&m_TransportSink));
    EXPECT_TRUE(&m_TransportSink == m_pBoshSession->GetSink());

    EXPECT_EQ(CM_OK, m_pBoshSession->OpenWithSink(NULL));
    EXPECT_TRUE(NULL == m_pBoshSession->GetSink());
}

//TEST_F(CBoshSessionTest, SendDataTestReturnOK)
//{
//    ASSERT_TRUE(NULL != m_pBoshSession);
//
//    char szBuf[512];
//    memset(szBuf, 'c', 511);
//    szBuf[511] = '\0';
//    CCmMessageBlock mbData(512, szBuf, CCmMessageBlock::DONT_DELETE, 512);
//
//    EXPECT_TRUE(CM_OK == m_pBoshSession->SendData(mbData));
//}

TEST_F(CBoshSessionTest, OptionTest)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();
 
    DWORD aCommand = 11;
    DWORD aArg = 12345;

    {
        CBoshTransaction*   pBoshTrans = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
        ASSERT_TRUE(NULL != pBoshTrans);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(pBoshTrans);

        CBoshTransaction*   pBoshTrans2 = new CBoshTransaction(&m_HttpServer2, m_pBoshSession);
        ASSERT_TRUE(NULL != pBoshTrans2);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans2(pBoshTrans2);

        EXPECT_TRUE(CM_OK != m_pBoshSession->SetOption(aCommand, (LPVOID)&aArg));
        EXPECT_TRUE(CM_OK != m_pBoshSession->GetOption(aCommand, (LPVOID)&aArg));

        EXPECT_EQ(CM_OK, pBoshTrans->Disconnect(CM_OK));
        EXPECT_EQ(CM_OK, pBoshTrans2->Disconnect(CM_OK));
    }
}

TEST_F(CBoshSessionTest, DisconnectTestNormal)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();
    EXPECT_CALLHTTPExProtoSink();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();
    {
        CBoshTransaction*   pBoshTrans = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
        ASSERT_TRUE(NULL != pBoshTrans);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(pBoshTrans);

        EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());

        EXPECT_EQ(CM_OK, m_pBoshSession->Disconnect(CM_OK));
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshSessionTest, DisconnectTestHTTPExSinkIsNull)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    m_pBoshSession->ReleaseReference();
    m_pBoshSession = NULL;

    m_pBoshSession = new CBoshSession(m_strSid, NULL, &m_HTTPExProtoSink, &m_OnePortSink);
    ASSERT_TRUE(NULL != m_pBoshSession);
    m_pBoshSession->AddReference();
 
    EXPECT_TRUE(CM_OK != m_pBoshSession->Disconnect(CM_OK));
}

TEST_F(CBoshSessionTest, GetConnTypeTest)
{
    ASSERT_TRUE(NULL != m_pBoshSession);

    UINT32 uType = 0;
    EXPECT_TRUE(CM_OK == m_pBoshSession->GetConnType(uType));
    EXPECT_EQ(PROTOCOL_TYPE_BOSH, uType);
}

TEST_F(CBoshSessionTest, GetRequestHeaderTest)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    {
        CBoshTransaction*   pBoshTrans = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
        ASSERT_TRUE(NULL != pBoshTrans);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(pBoshTrans);

        CBoshTransaction*   pBoshTrans2 = new CBoshTransaction(&m_HttpServer2, m_pBoshSession);
        ASSERT_TRUE(NULL != pBoshTrans2);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans2(pBoshTrans2);

        CCmString aHeader = "header";
        CCmString aValue = "just for test";
        EXPECT_TRUE(CM_OK != m_pBoshSession->GetRequestHeader(aHeader, aValue));

        EXPECT_EQ(CM_OK, pBoshTrans->Disconnect(CM_OK));
        EXPECT_EQ(CM_OK, pBoshTrans2->Disconnect(CM_OK));
    }
}

TEST_F(CBoshSessionTest, SetOrAddResponseHeaderTest)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    {
        CBoshTransaction*   pBoshTrans = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
        ASSERT_TRUE(NULL != pBoshTrans);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(pBoshTrans);

        CBoshTransaction*   pBoshTrans2 = new CBoshTransaction(&m_HttpServer2, m_pBoshSession);
        ASSERT_TRUE(NULL != pBoshTrans2);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans2(pBoshTrans2);


        CCmString aHeader = "header";
        CCmString aValue = "just for test";
        EXPECT_TRUE(CM_OK != m_pBoshSession->SetOrAddResponseHeader(aHeader, aValue));

        EXPECT_EQ(CM_OK, pBoshTrans->Disconnect(CM_OK));
        EXPECT_EQ(CM_OK, pBoshTrans2->Disconnect(CM_OK));
    }
}

TEST_F(CBoshSessionTest, RegisterTestNormalInput)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();
    {
        CBoshTransaction*   pBoshTrans = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
        ASSERT_TRUE(NULL != pBoshTrans);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(pBoshTrans);

        EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());
        EXPECT_EQ(CM_OK, pBoshTrans->Disconnect(CM_OK));
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshSessionTest, RegisterTestInvalidInput)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();
    {
        CBoshTransaction*   pBoshTrans = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
        ASSERT_TRUE(NULL != pBoshTrans);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(pBoshTrans);

        EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());

        EXPECT_TRUE(CM_OK != m_pBoshSession->Register(NULL, NULL));
        EXPECT_TRUE(CM_OK != m_pBoshSession->Register(&m_HttpServer, NULL));
        EXPECT_TRUE(CM_OK != m_pBoshSession->Register(&m_HttpServer, pBoshTrans));

        EXPECT_EQ(CM_OK, pBoshTrans->Disconnect(CM_OK));
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshSessionTest, UnRegisterTestInvalidInput)
{
    ASSERT_TRUE(NULL != m_pBoshSession);

    EXPECT_TRUE(CM_OK != m_pBoshSession->UnRegister(NULL));
    EXPECT_TRUE(CM_OK != m_pBoshSession->UnRegister(&m_HttpServer));
}

TEST_F(CBoshSessionTest, QueryByChannelHttpServerTestNormal)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();
    {
        CBoshTransaction*   pBoshTrans = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
        ASSERT_TRUE(NULL != pBoshTrans);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(pBoshTrans);

        EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());

        CBoshTransaction* pQuery = m_pBoshSession->QueryByChannelHttpServer(&m_HttpServer);
        EXPECT_EQ(pBoshTrans, pQuery);

        pQuery = m_pBoshSession->QueryByChannelHttpServer(NULL);
        EXPECT_TRUE(NULL == pQuery);

        pQuery = m_pBoshSession->QueryByChannelHttpServer(&m_HttpServer2);
        EXPECT_TRUE(NULL == pQuery);

        EXPECT_EQ(CM_OK, pBoshTrans->Disconnect(CM_OK));
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshSessionTest, PushRequestTestNormal)
{
    ASSERT_TRUE(NULL != m_pBoshSession);

    EXPECT_CALLHttpServer();
    EXPECT_CALLOnePortSink();
    EXPECT_CALLTransportSink();

    m_pBoshSession->OpenWithSink(&m_TransportSink);

    string strHandshake = CBoshDataBuild::XmlSessionCreateRequest();
    CBoshRequest*   pHandshake = new CBoshRequest(strHandshake);
    ASSERT_TRUE(NULL != pHandshake);
    CCmComAutoPtr<CBoshRequest> autoPtrHandshake(pHandshake);

    EXPECT_TRUE(CM_OK == m_pBoshSession->CreateSessionResponse(&m_HttpServer, pHandshake));

    string strBody = CBoshDataBuild::XmlRestartRequest(m_strSid);
    CBoshRequest*   pBoshRequest = new CBoshRequest(strBody);
    ASSERT_TRUE(NULL != pBoshRequest);
    CCmComAutoPtr<CBoshRequest> autoPtrRequest(pBoshRequest);

    EXPECT_TRUE(CM_OK == m_pBoshSession->PushRequest(&m_HttpServer, pBoshRequest));
}

TEST_F(CBoshSessionTest, PushRequestTestErrorInput)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    
    m_pBoshSession->OpenWithSink(&m_TransportSink);
    EXPECT_TRUE(CM_OK != m_pBoshSession->PushRequest(NULL, NULL));
    EXPECT_TRUE(CM_OK != m_pBoshSession->PushRequest(&m_HttpServer, NULL));
}

TEST_F(CBoshSessionTest, HandleRequestingPayloadsTestErrorInput)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    
    EXPECT_CALLHttpServer();

    m_pBoshSession->OpenWithSink(&m_TransportSink);

    string strBody = CBoshDataBuild::XmlRestartRequest(m_strSid);
    CBoshRequest*   pBoshRequest = new CBoshRequest(strBody);
    ASSERT_TRUE(NULL != pBoshRequest);
    CCmComAutoPtr<CBoshRequest> autoPtrRequest(pBoshRequest);

    EXPECT_TRUE(CM_OK != m_pBoshSession->PushRequest(&m_HttpServer, pBoshRequest));
}

TEST_F(CBoshSessionTest, CreateSessionResponseTestNormalInput)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();
    EXPECT_CALLOnePortSink();

    {
        string strBody = CBoshDataBuild::XmlSessionCreateRequest();
        CBoshRequest*   pBoshRequest = new CBoshRequest(strBody);
        ASSERT_TRUE(NULL != pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(pBoshRequest);

        EXPECT_EQ(CM_OK, m_pBoshSession->CreateSessionResponse(&m_HttpServer, pBoshRequest));
    }
}

TEST_F(CBoshSessionTest, CreateSessionResponseTestInvalidInput)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    {
        EXPECT_TRUE(CM_OK != m_pBoshSession->CreateSessionResponse(NULL, NULL));

        EXPECT_TRUE(CM_OK != m_pBoshSession->CreateSessionResponse(&m_HttpServer, NULL));

        string strBody = CBoshDataBuild::XmlSessionCreateRequest();
        CBoshRequest*   pBoshRequest = new CBoshRequest(strBody);
        ASSERT_TRUE(NULL != pBoshRequest);
        CCmComAutoPtr<CBoshRequest> autoPtrRequest(pBoshRequest);

        EXPECT_TRUE(CM_OK != m_pBoshSession->CreateSessionResponse(NULL, pBoshRequest));
    }
}