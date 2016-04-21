
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Mock_CmConnectionInterface.h"
#include "Mock_CmOnePortInterface.h"
#include "Mock_CmBaseHTTPExProtoHandle.h"
#include "Mock_CmHttpInterface.h"

#include "CmBoshDataBuild.h"
#include "CmBoshSession.h"
#include "CmBoshTransaction.h"

class CBoshTransactionTest : 
    public testing::Test
{
public:
    CBoshTransactionTest()
    {
        m_pBoshTransaction = NULL;
        m_pBoshSession = NULL;
    }

    virtual ~CBoshTransactionTest()
    {

    }

    virtual void SetUp()
    {
        if(NULL != m_pBoshSession)
        {
            m_pBoshSession->ReleaseReference();
            m_pBoshSession = NULL;
        }

        m_pBoshSession = new CBoshSession(m_strSid, &m_HTTPExHandle, &m_HTTPExProtoSink, &m_OnePortSink);
        m_pBoshSession->AddReference();

    }

    virtual void TearDown()
    {
        if(NULL != m_pBoshSession)
        {
            m_pBoshSession->ReleaseReference();
            m_pBoshSession = NULL;
        }
    }

    void EXPECT_CALLHttpServer()
    {
        EXPECT_CALL(m_HttpServer, AddReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(m_HttpServer, ReleaseReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(m_HttpServer, OpenWithSink(testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer, Disconnect(testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer, SendData(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer, SetOrAddResponseHeader(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer, GetRequestMethod(testing::_)).WillRepeatedly(testing::Return(CM_OK));
    }

protected:
    CBoshSession*                               m_pBoshSession;
    CBoshTransaction*                           m_pBoshTransaction;

    string                                      m_strSid;

    MockICmTransportSink                        m_TransportSink;
    MockICmBaseHTTPExProtoHandle                m_HTTPExHandle;
    MockICmBaseHTTPExProtoHandleSink            m_HTTPExProtoSink; 
    MockICmOnePortSink                          m_OnePortSink;
    MockICmChannelHttpServer                    m_HttpServer;
};

TEST_F(CBoshTransactionTest, ConstructTestErrorInput)
{
    ASSERT_TRUE(NULL == m_pBoshTransaction);
    EXPECT_CALLHttpServer();
    {
        m_pBoshTransaction = new CBoshTransaction(&m_HttpServer, NULL);
        ASSERT_TRUE(NULL != m_pBoshTransaction);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

        string strBody = CBoshDataBuild::XmlPostDataRequest("sid");
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        m_pBoshTransaction->OnReceive(mbData, &m_HttpServer);
    }

    {
        m_pBoshTransaction = new CBoshTransaction(NULL, m_pBoshSession);
        ASSERT_TRUE(NULL != m_pBoshTransaction);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

        string strBody = CBoshDataBuild::XmlPostDataRequest("sid");
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        m_pBoshTransaction->OnReceive(mbData, NULL);
    }

    {
        m_pBoshTransaction = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
        ASSERT_TRUE(NULL != m_pBoshTransaction);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);
        m_pBoshTransaction->ReleaseReference();
    }
    m_pBoshTransaction = NULL;
}

TEST_F(CBoshTransactionTest, ReferenceTest)
{
    ASSERT_TRUE(NULL == m_pBoshTransaction);
    m_pBoshTransaction = new CBoshTransaction(NULL, NULL);
    ASSERT_TRUE(NULL != m_pBoshTransaction);
    CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

    DWORD dwRef1 = m_pBoshTransaction->GetReference();
    {
        int nTimes = 100;
        for(int i=0; i<nTimes; i++)
        {
            m_pBoshTransaction->AddReference();
        }

        EXPECT_EQ(dwRef1+nTimes, m_pBoshTransaction->GetReference());

        for(int i=0; i<nTimes; i++)
        {
            m_pBoshTransaction->ReleaseReference();
        }
        EXPECT_EQ(dwRef1, m_pBoshTransaction->GetReference());
    }
    DWORD dwRef2 = m_pBoshTransaction->GetReference();

    EXPECT_EQ(dwRef1, dwRef2);  
}

TEST_F(CBoshTransactionTest, OnReceiveTestNormal)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();

    ASSERT_TRUE(NULL == m_pBoshTransaction);
    m_pBoshTransaction = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
    ASSERT_TRUE(NULL != m_pBoshTransaction);

    EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());
    {
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

        string strBody = CBoshDataBuild::XmlPostDataRequest("sid");
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        m_pBoshTransaction->OnReceive(mbData, &m_HttpServer);
        
        EXPECT_EQ(CM_OK, m_pBoshTransaction->Disconnect(CM_OK));
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshTransactionTest, OnReceiveTestInvalidInput)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();

    ASSERT_TRUE(NULL == m_pBoshTransaction);
    m_pBoshTransaction = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
    ASSERT_TRUE(NULL != m_pBoshTransaction);

    EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());
    {
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

        string strBody = CBoshDataBuild::XmlPostDataRequest("SomeSID");
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        m_pBoshTransaction->OnReceive(mbData, NULL);

        EXPECT_EQ(CM_OK, m_pBoshTransaction->Disconnect(CM_OK));
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshTransactionTest, OnSendTestNormal)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();

    ASSERT_TRUE(NULL == m_pBoshTransaction);
    m_pBoshTransaction = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
    ASSERT_TRUE(NULL != m_pBoshTransaction);

    EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());
    {
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

        m_pBoshTransaction->OnSend(&m_HttpServer);

        EXPECT_EQ(CM_OK, m_pBoshTransaction->Disconnect(CM_OK));
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshTransactionTest, OnSendTestInvalidInput)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();

    ASSERT_TRUE(NULL == m_pBoshTransaction);
    m_pBoshTransaction = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
    ASSERT_TRUE(NULL != m_pBoshTransaction);

    EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());
    {
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

        m_pBoshTransaction->OnSend(NULL);

        EXPECT_EQ(CM_OK, m_pBoshTransaction->Disconnect(CM_OK));
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshTransactionTest, OnDisconnectTestNormal)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();

    ASSERT_TRUE(NULL == m_pBoshTransaction);
    m_pBoshTransaction = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
    ASSERT_TRUE(NULL != m_pBoshTransaction);

    EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());
    {
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

        m_pBoshTransaction->OnDisconnect(CM_OK, &m_HttpServer);
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshTransactionTest, OnDisconnectTestInvalidInput)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();

    ASSERT_TRUE(NULL == m_pBoshTransaction);
    m_pBoshTransaction = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
    ASSERT_TRUE(NULL != m_pBoshTransaction);

    EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());
    {
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

        m_pBoshTransaction->OnDisconnect(CM_OK, NULL);
        EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());
        m_pBoshTransaction->Disconnect(CM_OK);
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshTransactionTest, SendDataTestNormal)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();

    ASSERT_TRUE(NULL == m_pBoshTransaction);
    m_pBoshTransaction = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
    ASSERT_TRUE(NULL != m_pBoshTransaction);

    EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());
    {
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

        string strBody = CBoshDataBuild::XmlPostDataRequest("SomeSID");
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        EXPECT_EQ(CM_OK, m_pBoshTransaction->SendData(mbData));

        EXPECT_CALL(m_HttpServer, SendData(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_ERROR_FAILURE));
        EXPECT_TRUE(CM_OK != m_pBoshTransaction->SendData(mbData));

        EXPECT_EQ(CM_OK, m_pBoshTransaction->Disconnect(CM_OK));
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshTransactionTest, SendDataTestErrorHttpServerPointIsNull)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();
    

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();

    ASSERT_TRUE(NULL == m_pBoshTransaction);
    m_pBoshTransaction = new CBoshTransaction(NULL, m_pBoshSession);
    ASSERT_TRUE(NULL != m_pBoshTransaction);

    EXPECT_EQ(dwCount1, m_pBoshSession->GetTransactionCount());
    {
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

        string strBody = CBoshDataBuild::XmlPostDataRequest("SomeSID");
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        EXPECT_TRUE(CM_OK != m_pBoshTransaction->SendData(mbData));
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshTransactionTest, DisconnectTestNormal)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();

    ASSERT_TRUE(NULL == m_pBoshTransaction);
    m_pBoshTransaction = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
    ASSERT_TRUE(NULL != m_pBoshTransaction);
    
    EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());
    {
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);
        EXPECT_EQ(CM_OK, m_pBoshTransaction->Disconnect(CM_OK));
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshTransactionTest, DisconnectTestError)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();
    {
        ASSERT_TRUE(NULL == m_pBoshTransaction);
        m_pBoshTransaction = new CBoshTransaction(NULL, m_pBoshSession);
        ASSERT_TRUE(NULL != m_pBoshTransaction);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

        EXPECT_TRUE(CM_OK != m_pBoshTransaction->Disconnect(CM_OK));
    }
    m_pBoshTransaction = NULL;
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();
    EXPECT_EQ(dwCount1, dwCount2);

    {
        ASSERT_TRUE(NULL == m_pBoshTransaction);
        m_pBoshTransaction = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
        ASSERT_TRUE(NULL != m_pBoshTransaction);
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

        EXPECT_CALL(m_HttpServer, Disconnect(testing::_)).WillRepeatedly(testing::Return(CM_ERROR_FAILURE));
        EXPECT_TRUE(CM_OK != m_pBoshTransaction->Disconnect(CM_OK));
    }
    dwCount2 = m_pBoshSession->GetTransactionCount();
    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshTransactionTest, HandleHTTPOptionsTestNormal)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();

    ASSERT_TRUE(NULL == m_pBoshTransaction);
    m_pBoshTransaction = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
    ASSERT_TRUE(NULL != m_pBoshTransaction);

    EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());
    {
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

        EXPECT_CALL(m_HttpServer, GetRequestMethod(testing::_))
            .WillRepeatedly(DoAll(testing::SetArgReferee<0>("OPTIONS"), testing::Return(CM_OK)));

        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Origin"), testing::_))
            .WillRepeatedly(DoAll(testing::SetArgReferee<1>("null"), testing::Return(CM_OK)));

        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Access-Control-Request-Headers"), testing::_))
            .WillRepeatedly(DoAll(testing::SetArgReferee<1>("content-type"), testing::Return(CM_OK)));

        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Access-Control-Request-Method"), testing::_))
            .WillRepeatedly(DoAll(testing::SetArgReferee<1>("POST"), testing::Return(CM_OK)));

        string strBody = CBoshDataBuild::XmlPostDataRequest("sid");
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        m_pBoshTransaction->OnReceive(mbData, &m_HttpServer);

        EXPECT_EQ(CM_OK, m_pBoshTransaction->Disconnect(CM_OK));
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CBoshTransactionTest, HandleHTTPOptionsTestErrorInput)
{
    ASSERT_TRUE(NULL != m_pBoshSession);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshSession->GetTransactionCount();

    ASSERT_TRUE(NULL == m_pBoshTransaction);
    m_pBoshTransaction = new CBoshTransaction(&m_HttpServer, m_pBoshSession);
    ASSERT_TRUE(NULL != m_pBoshTransaction);

    EXPECT_EQ(dwCount1+1, m_pBoshSession->GetTransactionCount());
    {
        CCmComAutoPtr<CBoshTransaction> autoPtrBoshTrans(m_pBoshTransaction);

        EXPECT_CALL(m_HttpServer, GetRequestMethod(testing::_))
            .WillRepeatedly(DoAll(testing::SetArgReferee<0>("OPTIONS"), testing::Return(CM_OK)));

        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Origin"), testing::_))
            .WillRepeatedly(testing::Return(CM_ERROR_FAILURE));

        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Access-Control-Request-Headers"), testing::_))
            .WillRepeatedly(testing::Return(CM_ERROR_FAILURE));

        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Access-Control-Request-Method"), testing::_))
            .WillRepeatedly(testing::Return(CM_ERROR_FAILURE));

        string strBody = CBoshDataBuild::XmlPostDataRequest("sid");
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        m_pBoshTransaction->OnReceive(mbData, &m_HttpServer);

        EXPECT_EQ(CM_OK, m_pBoshTransaction->Disconnect(CM_OK));
    }
    DWORD dwCount2 = m_pBoshSession->GetTransactionCount();

    EXPECT_EQ(dwCount1, dwCount2);
}