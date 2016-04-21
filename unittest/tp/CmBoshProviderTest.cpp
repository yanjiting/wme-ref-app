
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "CmThread.h"
#include "CmThreadManager.h"
#include "CmEventQueueBase.h"

#include "Mock_CmConnectionInterface.h"
#include "Mock_CmOnePortInterface.h"
#include "Mock_CmBaseHTTPExProtoHandle.h"
#include "Mock_CmHttpInterface.h"

#include "CmBoshConstStringDefine.h"
#include "CmEventTimer.h"
#include "CmBoshDataBuild.h"
#include "CmBoshBody.h"
#include "CmBoshTransaction.h"
#include "CmBoshSession.h"

#include "CmBoshProvider.h"


class CheckProvider
{
public:
    CheckProvider()
    {
        m_nOnSendCalledCount = 0;
    }

public:
    CmResult InvokeHTTPSend(CCmMessageBlock& aData, CCmTransportParameter* pParam)
    {
        return CM_OK;
    }

    void InvokeOnSend(ICmTransport *aTrptId, CCmTransportParameter *aPara)
    {
        m_nOnSendCalledCount++;
    }

    int GetOnSendCalledCount()
    {
        return m_nOnSendCalledCount;
    }

protected:
    int                 m_nOnSendCalledCount;
};


class CmBoshProviderTest : 
    public testing::Test
    , public ICmEventTimerSink
{
public:
    CmBoshProviderTest()
    {
        m_pBoshProvider = NULL;
    }

    virtual ~CmBoshProviderTest()
    {
        if(NULL != m_pBoshProvider)
        {
            m_pBoshProvider->ReleaseReference();
            m_pBoshProvider = NULL;
        }
    }

    virtual void SetUp()
    {
        m_strSessionId = "";
        CCmThreadManager* pThreadManage = CCmThreadManager::Instance();
        if(NULL != pThreadManage)
        {
            delete pThreadManage;
            pThreadManage = NULL;
        }
        
        m_bExistEventQueue = FALSE;
        if(NULL != m_pBoshProvider)
        {
            m_pBoshProvider->ReleaseReference();
            m_pBoshProvider = NULL;
        }

        EXPECT_CALLHTTPExProtoHandle();

        m_pBoshProvider = new CBoshProvider();
        m_pBoshProvider->AddReference();
    }

    virtual void TearDown()
    {
        CCmThreadManager* pThreadManage = CCmThreadManager::Instance();
        if(NULL != pThreadManage)
        {
            delete pThreadManage;
            pThreadManage = NULL;
        }
    }

    //{{{ICmEventTimerSink
public:
    virtual void OnEvent(
        UINT32 u32EventID)
    {
        if(20 == u32EventID)
        {
            ICmOnePortTransport* pOnePortTransport = m_OnePortSink.GetOnePortTransport();
            if(NULL == pOnePortTransport)
            {
                return ;
            }

            CBoshSession*   pBoshSession = dynamic_cast<CBoshSession*>(pOnePortTransport);

            string strData = "signal data";
            CCmMessageBlock signalData(strData.size(), strData.c_str(), 0, strData.size()); 
            EXPECT_TRUE(CM_OK != pBoshSession->SendData(signalData));

            string strRetrieve = CBoshDataBuild::XmlRetrieveDataRequest(m_strSessionId);
            CCmMessageBlock mbRetrieve2(strRetrieve.size(), strRetrieve.c_str(), 0, strRetrieve.size());
            EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbRetrieve2));

            EXPECT_TRUE(CM_OK == pBoshSession->SendData(signalData));
        }

        if(14 == u32EventID)
        {//
            string strRetrieve2 = CBoshDataBuild::XmlRetrieveDataRequest(m_strSessionId);
            CCmMessageBlock mbRetrieve2(strRetrieve2.size(), strRetrieve2.c_str(), 0, strRetrieve2.size());
            EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbRetrieve2));

            string strRetrieve3 = CBoshDataBuild::XmlRetrieveDataRequest(m_strSessionId);
            CCmMessageBlock mbRetrieve3(strRetrieve3.size(), strRetrieve3.c_str(), 0, strRetrieve3.size());
            EXPECT_TRUE(CM_OK != m_pBoshProvider->Handshake(&m_HttpServer, &mbRetrieve3));
        }

        if(13 == u32EventID)
        {
            ACmThread* pThread = CCmThreadManager::Instance()->GetThread(TT_MAIN);
            if(NULL != pThread)
            {
                pThread->Stop();
                m_bExistEventQueue = TRUE;
            }

        }
    }
    //}}}ICmEventTimerSink

protected:
    void EXPECT_CALLHttpServer()
    {
        EXPECT_CALL(m_HttpServer, AddReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(m_HttpServer, ReleaseReference()).WillRepeatedly(testing::Return(1));

        EXPECT_CALL(m_HttpServer, OpenWithSink(testing::_)).WillRepeatedly(
            testing::DoAll(testing::IgnoreResult(testing::Invoke(&m_HttpServer, &MockICmChannelHttpServer::InvokeOpenWithSink)),
            testing::Return(CM_OK)));

        EXPECT_CALL(m_HttpServer, Disconnect(testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer, SendData(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));

        //EXPECT_CALL(m_HttpServer, SendData(testing::_, testing::_)).WillRepeatedly(
        //    testing::DoAll(testing::IgnoreResult(testing::Invoke(&m_Check, &CheckProvider::InvokeHTTPSend)),
        //    testing::Return(CM_OK)));


        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_HttpServer, SetOrAddResponseHeader(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
    }

    void EXPECT_CALLOnePortSink()
    {
        EXPECT_CALL(m_OnePortSink, OnServerCreation(testing::_)).WillRepeatedly(
            testing::Invoke(&m_OnePortSink, &MockICmOnePortSink::InvokeOnServerCreation));
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
        EXPECT_CALL(m_TransportSink, OnDisconnect(testing::_, testing::_)).WillRepeatedly(testing::Return());
        EXPECT_CALL(m_TransportSink, OnReceive(testing::_, testing::_, testing::_)).WillRepeatedly(testing::Return());
    }

protected:
    BOOL                                        m_bExistEventQueue;
    string                                      m_strSessionId;
    CBoshProvider*                              m_pBoshProvider;
    CCmTimeValue                                m_TimeValue;

    CheckProvider                               m_Check;

    MockICmBaseHTTPExProtoHandle                m_HTTPExHandle;
    MockICmBaseHTTPExProtoHandleSink            m_HTTPExProtoSink; 
    MockICmOnePortSink                          m_OnePortSink;
    MockICmChannelHttpServer                    m_HttpServer;
    MockICmTransportSink                        m_TransportSink;
};

TEST_F(CmBoshProviderTest, ReferenceTest)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);

    DWORD dwRef1 = m_pBoshProvider->GetReference();
    {
        DWORD dwCount = 1;
        int nTimes = 100;
        for(int i=0; i<nTimes; i++)
        {
            m_pBoshProvider->AddReference();
            dwCount++;
        }

        EXPECT_EQ(dwCount, m_pBoshProvider->GetReference());

        for(int i=0; i<nTimes; i++)
        {
            m_pBoshProvider->ReleaseReference();
            dwCount--;
        }
    }
    DWORD dwRef2 = m_pBoshProvider->GetReference();

    EXPECT_EQ(dwRef1, dwRef2);
}

TEST_F(CmBoshProviderTest, InitializeTestInvalidInput)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);

    EXPECT_TRUE(CM_OK != m_pBoshProvider->Initialize(NULL, NULL));

    EXPECT_TRUE(CM_OK != m_pBoshProvider->Initialize(&m_HTTPExProtoSink, NULL));
    EXPECT_TRUE(CM_OK != m_pBoshProvider->Initialize(NULL, &m_OnePortSink));
}

TEST_F(CmBoshProviderTest, InitializeTestNormalInput)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);

    EXPECT_TRUE(CM_OK == m_pBoshProvider->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));
}

TEST_F(CmBoshProviderTest, HandshakeTestNormalInput)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);
    EXPECT_CALLHttpServer();
    EXPECT_CALLOnePortSink();

    DWORD dwCount1 = m_pBoshProvider->GetCount();
    {
        EXPECT_TRUE(CM_OK == m_pBoshProvider->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));

        string strBody = CBoshDataBuild::XmlSessionCreateRequest();
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbData));

        ICmTransportSink* pSink = m_HttpServer.InvokeGetSink();
        EXPECT_TRUE(NULL != pSink);

        pSink->OnDisconnect(CM_OK, &m_HttpServer);
    }
    DWORD dwCount2 = m_pBoshProvider->GetCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CmBoshProviderTest, HandshakeTestInvalidInput)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);
    EXPECT_CALLHttpServer();

    DWORD dwCount1 = m_pBoshProvider->GetCount();
    {
        EXPECT_TRUE(CM_OK != m_pBoshProvider->Handshake(NULL, NULL));

        EXPECT_TRUE(CM_OK != m_pBoshProvider->Handshake(&m_HttpServer, NULL));

        string strBody = CBoshDataBuild::XmlSessionCreateRequest();
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 
        EXPECT_TRUE(CM_OK != m_pBoshProvider->Handshake(NULL, &mbData));

        string strBody2 = CBoshDataBuild::XmlInvalidBoshData();
        CCmMessageBlock mbData2(strBody2.size(), strBody2.c_str(), 0, strBody2.size()); 
        EXPECT_TRUE(CM_OK != m_pBoshProvider->Handshake(&m_HttpServer, &mbData2));
    }
    DWORD dwCount2 = m_pBoshProvider->GetCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CmBoshProviderTest, RegisterTestNormalInput)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);

    DWORD dwCount1 = m_pBoshProvider->GetCount();
    {
        string strSid = m_pBoshProvider->GenerateSid();
        CBoshSession *pSession = new CBoshSession(strSid, &m_HTTPExHandle, &m_HTTPExProtoSink, &m_OnePortSink);
        ASSERT_TRUE(NULL != pSession);
        CCmComAutoPtr<CBoshSession> autoPtrSession(pSession);

        CBoshSession *pFindSession = m_pBoshProvider->QueryBySid(strSid);
        EXPECT_EQ(pSession, pFindSession);

        EXPECT_EQ(dwCount1+1, m_pBoshProvider->GetCount());
    }
    DWORD dwCount2 = m_pBoshProvider->GetCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CmBoshProviderTest, RegisterTestInvalidInput)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);

    DWORD dwCount1 = m_pBoshProvider->GetCount();
    {
        string strSid = m_pBoshProvider->GenerateSid();
        CBoshSession *pSession = new CBoshSession(strSid, &m_HTTPExHandle, &m_HTTPExProtoSink, &m_OnePortSink);
        ASSERT_TRUE(NULL != pSession);
        CCmComAutoPtr<CBoshSession> autoPtrSession(pSession);

        CBoshSession *pFindSession = m_pBoshProvider->QueryBySid(strSid);
        EXPECT_EQ(pSession, pFindSession);

        EXPECT_EQ(dwCount1+1, m_pBoshProvider->GetCount());

        EXPECT_TRUE(CM_OK != m_pBoshProvider->Register("", NULL));

        string strSid2 = m_pBoshProvider->GenerateSid();
        EXPECT_TRUE(CM_OK != m_pBoshProvider->Register(strSid2, NULL));

        CBoshSession *pSession2 = new CBoshSession(strSid, NULL, NULL, NULL);
        ASSERT_TRUE(NULL != pSession2);
        CCmComAutoPtr<CBoshSession> autoPtrSession2(pSession2);

        EXPECT_TRUE(CM_OK != m_pBoshProvider->Register(strSid, pSession2));
    }
    DWORD dwCount2 = m_pBoshProvider->GetCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CmBoshProviderTest, RegisterTestExistentItem)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);

    DWORD dwCount1 = m_pBoshProvider->GetCount();
    {
        string strSid = m_pBoshProvider->GenerateSid();
        CBoshSession *pSession = new CBoshSession(strSid, &m_HTTPExHandle, &m_HTTPExProtoSink, &m_OnePortSink);
        ASSERT_TRUE(NULL != pSession);
        CCmComAutoPtr<CBoshSession> autoPtrSession(pSession);

        CBoshSession* pFind = m_pBoshProvider->QueryBySid(strSid);
        EXPECT_EQ(pSession, pFind);

        EXPECT_EQ(dwCount1+1, m_pBoshProvider->GetCount());

        CBoshSession* pSession2 = new CBoshSession(strSid, &m_HTTPExHandle, &m_HTTPExProtoSink, &m_OnePortSink);
        ASSERT_TRUE(NULL != pSession2);
        CCmComAutoPtr<CBoshSession> autoPtrSession2(pSession2);
    }
    DWORD dwCount2 = m_pBoshProvider->GetCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CmBoshProviderTest, UnRegisterTestInvalidInput)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);

    EXPECT_TRUE(CM_OK != m_pBoshProvider->UnRegister(""));
}

TEST_F(CmBoshProviderTest, UnRegisterTestInexistentItem)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);

    string strSid = m_pBoshProvider->GenerateSid();

    EXPECT_TRUE(NULL == m_pBoshProvider->QueryBySid(strSid));
    EXPECT_TRUE(CM_OK != m_pBoshProvider->UnRegister(strSid));
}

TEST_F(CmBoshProviderTest, GetCountTest)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);

    DWORD dwCount1 = m_pBoshProvider->GetCount();
    {
        string strSid = m_pBoshProvider->GenerateSid();
        CBoshSession *pSession = new CBoshSession(strSid, &m_HTTPExHandle, &m_HTTPExProtoSink, &m_OnePortSink);

        ASSERT_TRUE(NULL != pSession);
        CCmComAutoPtr<CBoshSession> autoPtrSession(pSession);

        EXPECT_EQ(dwCount1+1, m_pBoshProvider->GetCount());
    }
    DWORD dwCount2 = m_pBoshProvider->GetCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CmBoshProviderTest, GenerateSidTest)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);

    BOOL bRepeat = FALSE;
    {
        typedef std::map<string, int>               mapSidInt;
        typedef std::map<string, int>::iterator     mapSidInt_IT;

        mapSidInt mapSid;
        for(int i=0; i<50000; i++)
        {
            string strSid = m_pBoshProvider->GenerateSid();

            mapSidInt_IT itFind = mapSid.find(strSid);
            if(mapSid.end() != itFind)
            {
                bRepeat = TRUE;
                break;
            }
            else
            {
                mapSid.insert(std::make_pair(strSid, 1));
            }
        }
    }

    EXPECT_TRUE(!bRepeat);
}

TEST_F(CmBoshProviderTest, APITestClientTerminate)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);
    EXPECT_CALLHttpServer();
    EXPECT_CALLOnePortSink();
    EXPECT_CALLTransportSink();

    DWORD dwCount1 = m_pBoshProvider->GetCount();
    {
        EXPECT_TRUE(CM_OK == m_pBoshProvider->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));

        string strBody = CBoshDataBuild::XmlSessionCreateRequest();
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbData));
        EXPECT_EQ(dwCount1+1, m_pBoshProvider->GetCount());

        ICmTransportSink* pSink = m_HttpServer.InvokeGetSink();
        EXPECT_TRUE(NULL != pSink);

        ICmOnePortTransport* pOnePortTransport = m_OnePortSink.GetOnePortTransport();
        EXPECT_TRUE(NULL != pOnePortTransport);
        ASSERT_TRUE(NULL != pOnePortTransport);

        CBoshSession*   pBoshSession = dynamic_cast<CBoshSession*>(pOnePortTransport);
        pBoshSession->OpenWithSink(&m_TransportSink);

        string strTerminate = CBoshDataBuild::XmlTerminateDataRequest(pBoshSession->GetSid());
        CCmMessageBlock mbTerminate(strTerminate.size(), strTerminate.c_str(), 0, strTerminate.size()); 
        EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbTerminate));
    }
    DWORD dwCount2 = m_pBoshProvider->GetCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CmBoshProviderTest, APITestTPTransportTerminate)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);
    EXPECT_CALLHttpServer();
    EXPECT_CALLOnePortSink();

    DWORD dwCount1 = m_pBoshProvider->GetCount();
    {
        EXPECT_TRUE(CM_OK == m_pBoshProvider->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));

        string strBody = CBoshDataBuild::XmlSessionCreateRequest();
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbData));

        ICmTransportSink* pSink = m_HttpServer.InvokeGetSink();
        EXPECT_TRUE(NULL != pSink);

        ICmOnePortTransport* pOnePortTransport = m_OnePortSink.GetOnePortTransport();
        EXPECT_TRUE(NULL != pOnePortTransport);
        ASSERT_TRUE(NULL != pOnePortTransport);

        CBoshSession*   pBoshSession = dynamic_cast<CBoshSession*>(pOnePortTransport);
        pBoshSession->OpenWithSink(&m_TransportSink);

        CBoshTransaction* pTransaction = pBoshSession->QueryByChannelHttpServer(&m_HttpServer);
        ASSERT_TRUE(NULL != pTransaction);
        pTransaction->OnDisconnect(CM_OK , &m_HttpServer);
    }
    DWORD dwCount2 = m_pBoshProvider->GetCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CmBoshProviderTest, APITestAppServerTerminate)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);
    EXPECT_CALLHttpServer();
    EXPECT_CALLOnePortSink();
    EXPECT_CALLHTTPExProtoSink();

    DWORD dwCount1 = m_pBoshProvider->GetCount();
    {
        EXPECT_TRUE(CM_OK == m_pBoshProvider->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));

        string strBody = CBoshDataBuild::XmlSessionCreateRequest();
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbData));

        ICmTransportSink* pSink = m_HttpServer.InvokeGetSink();
        EXPECT_TRUE(NULL != pSink);

        ICmOnePortTransport* pOnePortTransport = m_OnePortSink.GetOnePortTransport();
        EXPECT_TRUE(NULL != pOnePortTransport);
        ASSERT_TRUE(NULL != pOnePortTransport);

        CBoshSession*   pBoshSession = dynamic_cast<CBoshSession*>(pOnePortTransport);
        pBoshSession->OpenWithSink(&m_TransportSink);

        pBoshSession->Disconnect(CM_OK);
    }
    DWORD dwCount2 = m_pBoshProvider->GetCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CmBoshProviderTest, APITestTransmittingPayloads)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);
    EXPECT_CALLHttpServer();
    EXPECT_CALLOnePortSink();
    EXPECT_CALLTransportSink();

    DWORD dwCount1 = m_pBoshProvider->GetCount();
    {
        EXPECT_TRUE(CM_OK == m_pBoshProvider->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));

        string strBody = CBoshDataBuild::XmlSessionCreateRequest();
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbData));

        ICmTransportSink* pSink = m_HttpServer.InvokeGetSink();
        EXPECT_TRUE(NULL != pSink);

        ICmOnePortTransport* pOnePortTransport = m_OnePortSink.GetOnePortTransport();
        EXPECT_TRUE(NULL != pOnePortTransport);
        ASSERT_TRUE(NULL != pOnePortTransport);

        CBoshSession*   pBoshSession = dynamic_cast<CBoshSession*>(pOnePortTransport);
        pBoshSession->OpenWithSink(&m_TransportSink);

        string strPayloads = CBoshDataBuild::XmlPostDataRequest(pBoshSession->GetSid());

        string strTerminate = CBoshDataBuild::XmlTerminateDataRequest(pBoshSession->GetSid());
        CCmMessageBlock mbTerminate(strTerminate.size(), strTerminate.c_str(), 0, strTerminate.size()); 
        EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbTerminate));
    }
    DWORD dwCount2 = m_pBoshProvider->GetCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

TEST_F(CmBoshProviderTest, APITestInteractive)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);
    EXPECT_CALLHttpServer();
    EXPECT_CALLOnePortSink();
    EXPECT_CALLTransportSink();
    
    ACmThread* pThread = CCmThreadManager::Instance()->GetThread(TT_MAIN);
    ASSERT_TRUE(NULL != pThread);

    DWORD dwCount1 = m_pBoshProvider->GetCount();
    {
        EXPECT_TRUE(CM_OK == m_pBoshProvider->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));

        string strBody = CBoshDataBuild::XmlSessionCreateRequest();
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbData));

        ICmTransportSink* pSink = m_HttpServer.InvokeGetSink();
        EXPECT_TRUE(NULL != pSink);

        ICmOnePortTransport* pOnePortTransport = m_OnePortSink.GetOnePortTransport();
        EXPECT_TRUE(NULL != pOnePortTransport);
        ASSERT_TRUE(NULL != pOnePortTransport);

        CBoshSession*   pBoshSession = dynamic_cast<CBoshSession*>(pOnePortTransport);
        pBoshSession->OpenWithSink(&m_TransportSink);

        string strPayloads = CBoshDataBuild::XmlPostDataRequest(pBoshSession->GetSid());
        CCmMessageBlock mbPayloads(strPayloads.size(), strPayloads.c_str(), 0, strPayloads.size());
        EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbPayloads));

        //string strRetrieve = CBoshDataBuild::XmlRetrieveDataRequest(pBoshSession->GetSid());
        //CCmMessageBlock mbRetrieve(strRetrieve.size(), strRetrieve.c_str(), 0, strRetrieve.size());
        //EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbRetrieve));

        ICmEventTimerSink* pTimerSink = dynamic_cast<ICmEventTimerSink*>(this);
        CCmEventTimer eventTimer(pTimerSink);

        m_TimeValue.SetByTotalMsec(60000);
        eventTimer.Schedule(13, m_TimeValue, 1);

#ifndef WIN32
        while(!m_bExistEventQueue)
#endif
        {
            pThread->OnThreadRun();
        }

        EXPECT_TRUE(CM_OK == eventTimer.CancelAll());

        string strTerminate = CBoshDataBuild::XmlTerminateDataRequest(pBoshSession->GetSid());
        CCmMessageBlock mbTerminate(strTerminate.size(), strTerminate.c_str(), 0, strTerminate.size()); 
        EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbTerminate));
    }
    DWORD dwCount2 = m_pBoshProvider->GetCount();

    EXPECT_EQ(dwCount1, dwCount2);
}

 TEST_F(CmBoshProviderTest, APITestRidAvailableNotify)
 {
     ASSERT_TRUE(NULL != m_pBoshProvider);
     EXPECT_CALLHttpServer();
     EXPECT_CALLOnePortSink();
     EXPECT_CALLTransportSink();

     EXPECT_CALL(m_TransportSink, OnSend(testing::_, testing::_)).WillRepeatedly(
         testing::DoAll(testing::Invoke(&m_Check, &CheckProvider::InvokeOnSend),
         testing::Return()));

     ACmThread* pThread = CCmThreadManager::Instance()->GetThread(TT_MAIN);
     ASSERT_TRUE(NULL != pThread);

     DWORD dwCount1 = m_pBoshProvider->GetCount();
     {
         EXPECT_TRUE(CM_OK == m_pBoshProvider->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));

         string strBody = CBoshDataBuild::XmlSessionCreateRequest();
         CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

         EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbData));

         ICmTransportSink* pSink = m_HttpServer.InvokeGetSink();
         EXPECT_TRUE(NULL != pSink);

         ICmOnePortTransport* pOnePortTransport = m_OnePortSink.GetOnePortTransport();
         EXPECT_TRUE(NULL != pOnePortTransport);
         ASSERT_TRUE(NULL != pOnePortTransport);

         CBoshSession*   pBoshSession = dynamic_cast<CBoshSession*>(pOnePortTransport);
         pBoshSession->OpenWithSink(&m_TransportSink);

         string strData = "signal data";
         CCmMessageBlock signalData(strData.size(), strData.c_str(), 0, strData.size()); 
         EXPECT_TRUE(CM_OK != pBoshSession->SendData(signalData));

         m_strSessionId = pBoshSession->GetSid();

         string strRetrieve = CBoshDataBuild::XmlRetrieveDataRequest(m_strSessionId);
         CCmMessageBlock mbRetrieve(strRetrieve.size(), strRetrieve.c_str(), 0, strRetrieve.size());
         EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbRetrieve));

         EXPECT_TRUE(CM_OK == pBoshSession->SendData(signalData));

         ICmEventTimerSink* pTimerSink = dynamic_cast<ICmEventTimerSink*>(this);
         CCmEventTimer eventTimer(pTimerSink);

         m_TimeValue.SetByTotalMsec(100);
         eventTimer.Schedule(20, m_TimeValue, 1);

         CCmTimeValue timeValue2;
         timeValue2.SetByTotalMsec(2000);
         eventTimer.Schedule(13, timeValue2, 1);


 #ifndef WIN32
         while(!m_bExistEventQueue)
 #endif
         {
             pThread->OnThreadRun();
         }

         EXPECT_TRUE(CM_OK == eventTimer.CancelAll());

         string strTerminate = CBoshDataBuild::XmlTerminateDataRequest(pBoshSession->GetSid());
         CCmMessageBlock mbTerminate(strTerminate.size(), strTerminate.c_str(), 0, strTerminate.size()); 
         EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbTerminate));

         int nCount = m_Check.GetOnSendCalledCount();
         EXPECT_EQ(2 , nCount);
     }
     DWORD dwCount2 = m_pBoshProvider->GetCount();

     EXPECT_EQ(dwCount1, dwCount2);
 }

TEST_F(CmBoshProviderTest, APITestTFRRPolicy)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);
    EXPECT_CALLHttpServer();
    EXPECT_CALLOnePortSink();
    EXPECT_CALLTransportSink();
    
    ACmThread* pThread = CCmThreadManager::Instance()->GetThread(TT_MAIN);
    ASSERT_TRUE(NULL != pThread);

    DWORD dwCount1 = m_pBoshProvider->GetCount();
    {
        EXPECT_TRUE(CM_OK == m_pBoshProvider->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));

        string strBody = CBoshDataBuild::XmlSessionCreateRequest();
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbData));

        ICmTransportSink* pSink = m_HttpServer.InvokeGetSink();
        EXPECT_TRUE(NULL != pSink);

        ICmOnePortTransport* pOnePortTransport = m_OnePortSink.GetOnePortTransport();
        EXPECT_TRUE(NULL != pOnePortTransport);
        ASSERT_TRUE(NULL != pOnePortTransport);

        CBoshSession*   pBoshSession = dynamic_cast<CBoshSession*>(pOnePortTransport);
        pBoshSession->OpenWithSink(&m_TransportSink);

        string strRetrieve = CBoshDataBuild::XmlRetrieveDataRequest(pBoshSession->GetSid());
        CCmMessageBlock mbRetrieve(strRetrieve.size(), strRetrieve.c_str(), 0, strRetrieve.size());
        EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbRetrieve));

        string strRetrieve2 = CBoshDataBuild::XmlRetrieveDataRequest(pBoshSession->GetSid());
        CCmMessageBlock mbRetrieve2(strRetrieve2.size(), strRetrieve2.c_str(), 0, strRetrieve2.size());
        EXPECT_TRUE(CM_OK != m_pBoshProvider->Handshake(&m_HttpServer, &mbRetrieve2));

        ICmEventTimerSink* pTimerSink = dynamic_cast<ICmEventTimerSink*>(this);
        CCmEventTimer eventTimer(pTimerSink);

        m_TimeValue.SetByTotalMsec(500);
        eventTimer.Schedule(13, m_TimeValue, 1);

#ifndef WIN32
        while(!m_bExistEventQueue)
#endif
        {
            pThread->OnThreadRun();
        }

        EXPECT_TRUE(CM_OK == eventTimer.CancelAll());
    }
    DWORD dwCount2 = m_pBoshProvider->GetCount();

    EXPECT_EQ(dwCount1, dwCount2);
}


TEST_F(CmBoshProviderTest, APITestTMSRRPolicy)
{
    ASSERT_TRUE(NULL != m_pBoshProvider);
    EXPECT_CALLHttpServer();
    EXPECT_CALLOnePortSink();
    EXPECT_CALLTransportSink();
    
    ACmThread* pThread = CCmThreadManager::Instance()->GetThread(TT_MAIN);
    ASSERT_TRUE(NULL != pThread);

    DWORD dwCount1 = m_pBoshProvider->GetCount();
    {
        EXPECT_TRUE(CM_OK == m_pBoshProvider->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));

        string strBody = CBoshDataBuild::XmlSessionCreateRequest();
        CCmMessageBlock mbData(strBody.size(), strBody.c_str(), 0, strBody.size()); 

        EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbData));

        ICmTransportSink* pSink = m_HttpServer.InvokeGetSink();
        EXPECT_TRUE(NULL != pSink);

        ICmOnePortTransport* pOnePortTransport = m_OnePortSink.GetOnePortTransport();
        EXPECT_TRUE(NULL != pOnePortTransport);
        ASSERT_TRUE(NULL != pOnePortTransport);

        CBoshSession*   pBoshSession = dynamic_cast<CBoshSession*>(pOnePortTransport);
        pBoshSession->OpenWithSink(&m_TransportSink);

        m_strSessionId = pBoshSession->GetSid();

        string strRetrieve = CBoshDataBuild::XmlRetrieveDataRequest(m_strSessionId);
        CCmMessageBlock mbRetrieve(strRetrieve.size(), strRetrieve.c_str(), 0, strRetrieve.size());
        EXPECT_TRUE(CM_OK == m_pBoshProvider->Handshake(&m_HttpServer, &mbRetrieve));

        string strRetrieve1 = strRetrieve;
        CCmMessageBlock mbRetrieve1(strRetrieve1.size(), strRetrieve1.c_str(), 0, strRetrieve1.size());
        //exist rid
        EXPECT_TRUE(CM_OK != m_pBoshProvider->Handshake(&m_HttpServer, &mbRetrieve1));

        ICmEventTimerSink* pTimerSink = dynamic_cast<ICmEventTimerSink*>(this);
        CCmEventTimer eventTimer(pTimerSink);

        m_TimeValue.SetByTotalMsec(8000);
        eventTimer.Schedule(13, m_TimeValue, 1);
        
        // > polling(5s);
        long lSecond = 6;
        eventTimer.Schedule(14, CCmTimeValue(lSecond), 1);
#ifndef WIN32
        while(!m_bExistEventQueue)
#endif
        {
            pThread->OnThreadRun();
        }

        EXPECT_TRUE(CM_OK == eventTimer.CancelAll());
    }
    DWORD dwCount2 = m_pBoshProvider->GetCount();

    EXPECT_EQ(dwCount1, dwCount2);
}