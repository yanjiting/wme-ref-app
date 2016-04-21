#include <gmock/gmock.h>
#include <gtest/gtest.h>


#include "Mock_CmOnePortInterface.h"
#include "Mock_CmOnePortImpl.h"
#include "Mock_CmHttpInterface.h"
#include "Mock_CmConnectionInterface.h"
#include "Mock_CmThreadInterface.h"
#include "Mock_CmBaseHTTPExProtoHandle.h"

#include "CmRestHandle.h"

class MockCmRestHandle : public CmRestHandle 
{
public:
    MockCmRestHandle()
    {
        m_EventQueue = NULL; 
    }
public:
    void SetEventQueue(ICmEventQueue* pEventQueue)
    {
        m_EventQueue = pEventQueue;
    }
    
    virtual ICmEventQueue* GetEventQueue()
    {
        return m_EventQueue;
    }
    
protected:
    ICmEventQueue*  m_EventQueue;
};



class CmResttHandleTest : public testing::Test
{
public:
    CmResttHandleTest()
    {
        m_pRestHandle = NULL;
        m_HttpServer = NULL;
    }

    virtual ~CmResttHandleTest()
    {
        Reset();
    }

    virtual void SetUp()
    {
        if(NULL == m_pRestHandle)
        {
            m_pRestHandle = new MockCmRestHandle();
            m_pRestHandle->AddReference();
        }
        
        if(NULL == m_HttpServer)
        {
            m_HttpServer = new MockICmChannelHttpServer();
        }
        
        m_pRestHandle->SetEventQueue(&m_EventQueue);
        
        EXPECT_CALL(*m_HttpServer, AddReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(*m_HttpServer, ReleaseReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(*m_HttpServer, Disconnect(testing::_)).WillRepeatedly(testing::Return(CM_OK));
        
        
        EXPECT_CALL(*m_HttpServer, GetRequestHeader(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_OK));
        
        
        EXPECT_CALL(*m_HttpServer, SetOrAddResponseHeader(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_OK));
        
        EXPECT_CALL(*m_HttpServer, SetResponseStatus(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_OK));
        
        EXPECT_CALL(*m_HttpServer, SendData(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_OK));
        
        EXPECT_CALL(*m_HttpServer, OpenWithSink(testing::_))
            .WillRepeatedly(testing::Return(CM_OK));

        EXPECT_CALL(*m_HttpServer, SetOption(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_OK));

        EXPECT_CALL(*m_HttpServer, GetOption(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_OK));

        //EXPECT_CALL(*m_HttpServer, OnReceive(testing::_, testing::_))
        //    .WillRepeatedly(testing::Return(CM_OK));

        EXPECT_CALL(m_OnePortSink, OnServerCreation(testing::_)).Times(testing::AnyNumber());
        
        EXPECT_CALL(m_HTTPExProtoSink, OnConnect(testing::_, testing::_)).Times(testing::AnyNumber());
        
        EXPECT_CALL(m_HTTPExProtoSink, OnDisconnect(testing::_, testing::_)).Times(testing::AnyNumber());
        
        EXPECT_CALL(m_DataTransportSink, OnReceive(testing::_, testing::_, testing::_)).Times(testing::AnyNumber());
        
        EXPECT_CALL(m_DataTransportSink, OnSend(testing::_, testing::_)).Times(testing::AnyNumber());
        
        EXPECT_CALL(m_DataTransportSink, OnDisconnect(testing::_, testing::_)).Times(testing::AnyNumber());
        
        EXPECT_CALL(m_EventQueue, PostEvent(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_OK));
    }

    virtual void TearDown()
    {
        Reset();
    }

protected:
    void Reset()
    {
        if(NULL != m_pRestHandle)
        {
            m_pRestHandle->ReleaseReference();
            m_pRestHandle = NULL;
        }
        
        if(NULL != m_HttpServer)
        {
            delete m_HttpServer;
            m_HttpServer = NULL;
        }
    }

protected:
    MockCmRestHandle*                       m_pRestHandle;
    MockICmBaseHTTPExProtoHandleSink        m_HTTPExProtoSink;  

    MockICmOnePortSink                      m_OnePortSink;
    MockICmTransportSink                    m_DataTransportSink;
    MockICmChannelHttpServer*               m_HttpServer;
    MockICmEventQueue                       m_EventQueue;
};


TEST_F(CmResttHandleTest, OnDisconnectTest)
{
    ASSERT_TRUE(NULL != m_pRestHandle);
    
    EXPECT_TRUE(CM_OK == m_pRestHandle->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));
    EXPECT_TRUE(CM_OK == m_pRestHandle->OpenWithSink(&m_DataTransportSink));
}

TEST_F(CmResttHandleTest, InitializeTest)
{
    ASSERT_TRUE(NULL != m_pRestHandle);
    EXPECT_TRUE(CM_ERROR_NULL_POINTER == m_pRestHandle->Initialize(NULL, NULL));
    
    EXPECT_TRUE(CM_OK == m_pRestHandle->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));
    EXPECT_TRUE(CM_ERROR_ALREADY_INITIALIZED == m_pRestHandle->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));
}

TEST_F(CmResttHandleTest, HandshakeTest)
{
    ASSERT_TRUE(NULL != m_pRestHandle);
    
    CCmString aHeader="just_test";
    CCmString aValue;
    
    UINT32  type = 0;
    EXPECT_TRUE(CM_OK == m_pRestHandle->GetConnType(type));
    EXPECT_EQ(PROTOCOL_TYPE_REST, type);
    
    EXPECT_TRUE(CM_OK == m_pRestHandle->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));
    
    EXPECT_TRUE(CM_ERROR_FAILURE == m_pRestHandle->SetOrAddResponseHeader(aHeader, aValue));
    EXPECT_TRUE(CM_ERROR_FAILURE == m_pRestHandle->GetRequestHeader(aHeader, aValue));
    
    const char szBuf[] = "hello world";
    CCmMessageBlock aMessage(100);
    aMessage.Write(szBuf, strlen(szBuf));
    EXPECT_TRUE(CM_OK == m_pRestHandle->Handshake(m_HttpServer, &aMessage));
    
    
    EXPECT_TRUE(NULL == m_pRestHandle->GetSink());
    EXPECT_TRUE(CM_OK == m_pRestHandle->OpenWithSink(&m_DataTransportSink));
    EXPECT_TRUE(&m_DataTransportSink == m_pRestHandle->GetSink());
    
    
    EXPECT_TRUE(CM_OK == m_pRestHandle->SetOrAddResponseHeader(aHeader, aValue));
    EXPECT_TRUE(CM_OK == m_pRestHandle->GetRequestHeader(aHeader, aValue));
    
    DWORD aCommand = 100;
    DWORD aArg = 50;
    EXPECT_TRUE(CM_OK == m_pRestHandle->SetOption(aCommand, &aArg));
    EXPECT_TRUE(CM_OK == m_pRestHandle->GetOption(aCommand, &aArg));
}

TEST_F(CmResttHandleTest, InteractiveTest)
{
    ASSERT_TRUE(NULL != m_pRestHandle);
    
    EXPECT_TRUE(CM_OK == m_pRestHandle->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));
    
    const char szBuf3[] = "hello world";
    CCmMessageBlock aMessage(100);
    aMessage.Write(szBuf3, strlen(szBuf3));
    EXPECT_TRUE(CM_OK == m_pRestHandle->Handshake(m_HttpServer, &aMessage));
    
    EXPECT_TRUE(CM_OK == m_pRestHandle->OpenWithSink(&m_DataTransportSink));
    
    
    char szBuf[] = "hello world";
    UINT8  uFirstByte = 0x81;
    UINT8  uSecondByte = strlen(szBuf);
    CCmMessageBlock aHead(2);
    CCmMessageBlock aData(strlen(szBuf));

    aHead.Write(&uFirstByte, sizeof(UINT8));
    aHead.Write(&uSecondByte, sizeof(UINT8));
    
    aData.Write(szBuf, strlen(szBuf));
    aHead.Append(&aData);
    
    CRestEvent objEvent1(NULL, NULL);
    objEvent1.OnEventFire();
    
    CRestEvent objEvent2(m_pRestHandle, NULL);
    objEvent2.OnEventFire();
    
    CRestEvent objEvent3(m_pRestHandle, &aHead);
    objEvent3.OnEventFire();
    
    // m_pRestHandle->OnReceive(aHead, m_HttpServer, NULL);
    
    char szBuf2[] = "response";
    uSecondByte = strlen(szBuf2);
    CCmMessageBlock aHead2(2);
    CCmMessageBlock aData2(strlen(szBuf2));
    
    aHead2.Write(&uFirstByte, sizeof(UINT8));
    aHead2.Write(&uSecondByte, sizeof(UINT8));
    aData2.Write(szBuf2, strlen(szBuf2));
    aHead2.Append(&aData2);
    
    EXPECT_TRUE(CM_OK == m_pRestHandle->SendData(aHead2));
    m_pRestHandle->OnSend(m_HttpServer, NULL);
    m_pRestHandle->OnDisconnect(CM_OK, NULL);
    m_pRestHandle->OnDisconnect(CM_OK, m_HttpServer);
}