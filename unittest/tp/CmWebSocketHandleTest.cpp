
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "Mock_CmOnePortInterface.h"
#include "Mock_CmOnePortImpl.h"
#include "Mock_CmHttpInterface.h"
#include "Mock_CmConnectionInterface.h"
#include "Mock_CmBaseHTTPExProtoHandle.h"

#include "CmWebSocketHandle.h"

class CmWebSocketHandleTest : public testing::Test
{
public:
    CmWebSocketHandleTest()
    {
        m_pWSHandle = NULL;
    }

    virtual ~CmWebSocketHandleTest()
    {
        if(NULL != m_pWSHandle)
        {
            m_pWSHandle->ReleaseReference();
            m_pWSHandle = NULL;
        }
    }

    virtual void SetUp()
    {
        if(NULL == m_pWSHandle)
        {
            m_pWSHandle = new CmWebSocketHandle();
            m_pWSHandle->AddReference();
        }
        
        EXPECT_CALL(m_HttpServer, AddReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(m_HttpServer, ReleaseReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(m_HttpServer, Disconnect(testing::_)).WillRepeatedly(testing::Return(CM_OK));
        
        EXPECT_CALL(m_HttpServer, GetTcpTransport()).WillRepeatedly(testing::Return(&m_DataTransport));
        
        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_ERROR_FAILURE));
        
        ChromeEnv();
        
        EXPECT_CALL(m_HttpServer, SetOrAddResponseHeader(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_OK));
        
        EXPECT_CALL(m_HttpServer, SetResponseStatus(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_OK));
        
        EXPECT_CALL(m_HttpServer, SendData(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_OK));
        
        EXPECT_CALL(m_DataTransport, OpenWithSink(testing::_))
            .WillRepeatedly(testing::Return(CM_OK));
        
        EXPECT_CALL(m_DataTransport, SendData(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_OK));

        EXPECT_CALL(m_DataTransport, SetOption(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_OK));

        EXPECT_CALL(m_DataTransport, GetOption(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_OK));
        
        EXPECT_CALL(m_OnePortSink, OnServerCreation(testing::_))
            .Times(testing::AnyNumber());
        
        EXPECT_CALL(m_HTTPExProtoSink, OnConnect(testing::_, testing::_))
            .Times(testing::AnyNumber());
        
        EXPECT_CALL(m_HTTPExProtoSink, OnDisconnect(testing::_, testing::_))
            .Times(testing::AnyNumber());

        EXPECT_CALL(m_DataTransportSink, OnReceive(testing::_, testing::_, testing::_)).WillRepeatedly(
            testing::InvokeWithoutArgs(&m_DataTransportSink, &MockICmTransportSink::InvokeIncreaseRecvCount));

        m_DataTransportSink.SetRecvCount(0);

        EXPECT_CALL(m_DataTransportSink, OnSend(testing::_, testing::_))
            .Times(testing::AnyNumber());
        
        EXPECT_CALL(m_DataTransportSink, OnDisconnect(testing::_, testing::_))
            .Times(testing::AnyNumber());
    }
    
    virtual void TearDown()
    {
        if(NULL != m_pWSHandle)
        {
            m_pWSHandle->ReleaseReference();
            m_pWSHandle = NULL;
        }
    }

protected:
    void ChromeEnv()
    {
        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Connection"), testing::_))
            .WillRepeatedly(DoAll(testing::SetArgReferee<1>("Upgrade"), testing::Return(CM_OK)));
        
        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Upgrade"), testing::_))
            .WillRepeatedly(DoAll(testing::SetArgReferee<1>("websocket"), testing::Return(CM_OK)));
        
        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Sec-WebSocket-Key"), testing::_))
            .WillRepeatedly(DoAll(testing::SetArgReferee<1>("dGhlIHNhbXBsZSBub25jZQ=="), testing::Return(CM_OK)));
        
        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Sec-WebSocket-Version"), testing::_))
            .WillRepeatedly(DoAll(testing::SetArgReferee<1>("13"), testing::Return(CM_OK)));
        
        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Sec-WebSocket-Protocol"), testing::_))
            .WillRepeatedly(DoAll(testing::SetArgReferee<1>("chat, superchat"), testing::Return(CM_OK)));
    }
    
    void FireFoxEnv()
    {
        // GET / HTTP/1.1
        // Host: 10.224.54.42:8080
        // User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64; rv:19.0) Gecko/20100101 Firefox/19.0
        // Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8
        // Accept-Language: en-US,en;q=0.5
        // Accept-Encoding: gzip, deflate
        // Sec-WebSocket-Version: 13
        // Origin: null
        // Sec-WebSocket-Key: F5EwPCMVVtdvz4mmiClwOw==
        // Connection: keep-alive, Upgrade
        // Pragma: no-cache
        // Cache-Control: no-cache
        // Upgrade: websocket
        
        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Connection"), testing::_))
            .WillRepeatedly(DoAll(testing::SetArgReferee<1>(" keep-alive, Upgrade"), testing::Return(CM_OK)));
        
        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Upgrade"), testing::_))
            .WillRepeatedly(DoAll(testing::SetArgReferee<1>("websocket"), testing::Return(CM_OK)));
        
        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Sec-WebSocket-Key"), testing::_))
            .WillRepeatedly(DoAll(testing::SetArgReferee<1>("F5EwPCMVVtdvz4mmiClwOw=="), testing::Return(CM_OK)));
        
        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Sec-WebSocket-Version"), testing::_))
            .WillRepeatedly(DoAll(testing::SetArgReferee<1>(" 13"), testing::Return(CM_OK)));
        
        // EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Sec-WebSocket-Protocol"), testing::_))
            // .WillRepeatedly(DoAll(testing::SetArgReferee<1>("chat, superchat"), testing::Return(CM_OK)));
    }
    
    void HandshakeTest()
    {
        ASSERT_TRUE(NULL != m_pWSHandle);
        
        UINT32  type = 0;
        EXPECT_TRUE(CM_OK == m_pWSHandle->GetConnType(type));
        EXPECT_EQ(PROTOCOL_TYPE_WEBSOCKET, type);
        
        EXPECT_TRUE(CM_OK == m_pWSHandle->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));
        EXPECT_TRUE(CM_ERROR_NULL_POINTER == m_pWSHandle->Handshake(NULL));
        EXPECT_TRUE(CM_OK == m_pWSHandle->Handshake(&m_HttpServer));
        EXPECT_TRUE(CM_ERROR_ALREADY_INITIALIZED == m_pWSHandle->Handshake(&m_HttpServer));
        
        EXPECT_TRUE(NULL == m_pWSHandle->GetSink());
        EXPECT_TRUE(CM_OK == m_pWSHandle->OpenWithSink(&m_DataTransportSink));
        EXPECT_TRUE(&m_DataTransportSink == m_pWSHandle->GetSink());
        
        CCmString aHeader="just_test";
        CCmString aValue;
        EXPECT_TRUE(CM_ERROR_FAILURE == m_pWSHandle->SetOrAddResponseHeader(aHeader, aValue));
        EXPECT_TRUE(CM_ERROR_FAILURE == m_pWSHandle->GetRequestHeader(aHeader, aValue));
        
        DWORD aCommand = 100;
        DWORD aArg = 50;
        EXPECT_TRUE(CM_OK == m_pWSHandle->SetOption(aCommand, &aArg));
        EXPECT_TRUE(CM_OK == m_pWSHandle->GetOption(aCommand, &aArg));
    }
    
    void InteractiveTest()
    {
        ASSERT_TRUE(NULL != m_pWSHandle);
    
        EXPECT_TRUE(CM_OK == m_pWSHandle->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));
        EXPECT_TRUE(CM_OK == m_pWSHandle->Handshake(&m_HttpServer));
        
        EXPECT_TRUE(CM_OK == m_pWSHandle->OpenWithSink(&m_DataTransportSink));
        
        
        char szBuf[] = "hello world";
        UINT8  uFirstByte = 0x81;
        UINT8  uSecondByte = strlen(szBuf);
        CCmMessageBlock aHead(2);
        CCmMessageBlock aData(strlen(szBuf));

        aHead.Write(&uFirstByte, sizeof(UINT8));
        aHead.Write(&uSecondByte, sizeof(UINT8));
        
        aData.Write(szBuf, strlen(szBuf));
        aHead.Append(&aData);
        
        m_pWSHandle->OnReceive(aHead, &m_DataTransport, NULL);
        
        char szBuf2[] = "response";
        uSecondByte = strlen(szBuf2);
        CCmMessageBlock aHead2(2);
        CCmMessageBlock aData2(strlen(szBuf2));
        
        aHead2.Write(&uFirstByte, sizeof(UINT8));
        aHead2.Write(&uSecondByte, sizeof(UINT8));
        aData2.Write(szBuf2, strlen(szBuf2));
        aHead2.Append(&aData2);
        
        EXPECT_TRUE(CM_OK == m_pWSHandle->SendData(aHead2));
        m_pWSHandle->OnSend(&m_DataTransport, NULL);
        m_pWSHandle->OnDisconnect(CM_OK, NULL);
        m_pWSHandle->OnDisconnect(CM_OK, &m_DataTransport);
    }

    void InteractiveTestOneMsgOnePacket()
    {
        ASSERT_TRUE(NULL != m_pWSHandle);

        EXPECT_TRUE(CM_OK == m_pWSHandle->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));
        EXPECT_TRUE(CM_OK == m_pWSHandle->Handshake(&m_HttpServer));

        EXPECT_TRUE(CM_OK == m_pWSHandle->OpenWithSink(&m_DataTransportSink));
        m_DataTransportSink.SetRecvCount(0);

        char szBuf[] = "hello world";
        UINT8  uFirstByte = 0x81;
        UINT8  uSecondByte = strlen(szBuf);
        CCmMessageBlock aHead(2);
        CCmMessageBlock aData(strlen(szBuf));

        aHead.Write(&uFirstByte, sizeof(UINT8));
        aHead.Write(&uSecondByte, sizeof(UINT8));

        aData.Write(szBuf, strlen(szBuf));
        aHead.Append(&aData);

        m_pWSHandle->OnReceive(aHead, &m_DataTransport, NULL);

        EXPECT_EQ(1, m_DataTransportSink.GetRecvCount());

        char szBuf2[] = "response";
        uSecondByte = strlen(szBuf2);
        CCmMessageBlock aHead2(2);
        CCmMessageBlock aData2(strlen(szBuf2));

        aHead2.Write(&uFirstByte, sizeof(UINT8));
        aHead2.Write(&uSecondByte, sizeof(UINT8));
        aData2.Write(szBuf2, strlen(szBuf2));
        aHead2.Append(&aData2);

        EXPECT_TRUE(CM_OK == m_pWSHandle->SendData(aHead2));
        m_pWSHandle->OnSend(&m_DataTransport, NULL);
        m_pWSHandle->OnDisconnect(CM_OK, NULL);
        m_pWSHandle->OnDisconnect(CM_OK, &m_DataTransport);
    }

    void InteractiveTestOneMsgThreePackets()
    {
        ASSERT_TRUE(NULL != m_pWSHandle);

        EXPECT_TRUE(CM_OK == m_pWSHandle->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));
        EXPECT_TRUE(CM_OK == m_pWSHandle->Handshake(&m_HttpServer));

        EXPECT_TRUE(CM_OK == m_pWSHandle->OpenWithSink(&m_DataTransportSink));
        m_DataTransportSink.SetRecvCount(0);

        char szBuf[] = "hello world";
        UINT8  uFirstByte = 0x81;
        UINT8  uSecondByte = strlen(szBuf);
        CCmMessageBlock aHead(2);
        CCmMessageBlock aData(strlen(szBuf));

        aHead.Write(&uFirstByte, sizeof(UINT8));
        aHead.Write(&uSecondByte, sizeof(UINT8));

        aData.Write(szBuf, strlen(szBuf));
        aHead.Append(&aData);

        CCmMessageBlock *pMessage = aHead.DuplicateChained();
        ASSERT_TRUE(NULL != pMessage);
        pMessage->Append(aHead.DuplicateChained());
        pMessage->Append(aHead.DuplicateChained());

        m_pWSHandle->OnReceive(*pMessage, &m_DataTransport, NULL);

        EXPECT_EQ(3, m_DataTransportSink.GetRecvCount());
        pMessage->DestroyChained();
        pMessage = NULL;

        char szBuf2[] = "response";
        uSecondByte = strlen(szBuf2);
        CCmMessageBlock aHead2(2);
        CCmMessageBlock aData2(strlen(szBuf2));

        aHead2.Write(&uFirstByte, sizeof(UINT8));
        aHead2.Write(&uSecondByte, sizeof(UINT8));
        aData2.Write(szBuf2, strlen(szBuf2));
        aHead2.Append(&aData2);

        EXPECT_TRUE(CM_OK == m_pWSHandle->SendData(aHead2));
        m_pWSHandle->OnSend(&m_DataTransport, NULL);
        m_pWSHandle->OnDisconnect(CM_OK, NULL);
        m_pWSHandle->OnDisconnect(CM_OK, &m_DataTransport);
    }

    void InteractiveTestThreeMsgOnePacket()
    {
        ASSERT_TRUE(NULL != m_pWSHandle);

        EXPECT_TRUE(CM_OK == m_pWSHandle->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));
        EXPECT_TRUE(CM_OK == m_pWSHandle->Handshake(&m_HttpServer));

        EXPECT_TRUE(CM_OK == m_pWSHandle->OpenWithSink(&m_DataTransportSink));
        m_DataTransportSink.SetRecvCount(0);

        char szBuf[] = "hello world";
        char szBuf2[] = "abcdefgh---";
        char szBuf3[] = "0123456789";
        UINT8  uFirstByte = 0x81;
        UINT8  uSecondByte = strlen(szBuf) + strlen(szBuf2) + strlen(szBuf3) ;
        CCmMessageBlock aHead(2);
        CCmMessageBlock aData(strlen(szBuf));

        aHead.Write(&uFirstByte, sizeof(UINT8));
        aHead.Write(&uSecondByte, sizeof(UINT8));

        aData.Write(szBuf, strlen(szBuf));
        aHead.Append(&aData);

        m_pWSHandle->OnReceive(aHead, &m_DataTransport, NULL);
        EXPECT_EQ(0, m_DataTransportSink.GetRecvCount());

        CCmMessageBlock aData2(strlen(szBuf2));
        aData2.Write(szBuf2, strlen(szBuf2));
        m_pWSHandle->OnReceive(aData2, &m_DataTransport, NULL);
        EXPECT_EQ(0, m_DataTransportSink.GetRecvCount());

        CCmMessageBlock aData3(strlen(szBuf3));
        aData3.Write(szBuf3, strlen(szBuf3));
        m_pWSHandle->OnReceive(aData3, &m_DataTransport, NULL);
        EXPECT_EQ(1, m_DataTransportSink.GetRecvCount());

        char szSendBuffer[] = "response";
        uSecondByte = strlen(szSendBuffer);
        CCmMessageBlock aSendHead(2);
        CCmMessageBlock aSendData(strlen(szSendBuffer));

        aSendHead.Write(&uFirstByte, sizeof(UINT8));
        aSendHead.Write(&uSecondByte, sizeof(UINT8));
        aSendHead.Write(szSendBuffer, strlen(szSendBuffer));
        aSendHead.Append(&aSendData);

        EXPECT_TRUE(CM_OK == m_pWSHandle->SendData(aSendHead));
        m_pWSHandle->OnSend(&m_DataTransport, NULL);
        m_pWSHandle->OnDisconnect(CM_OK, NULL);
        m_pWSHandle->OnDisconnect(CM_OK, &m_DataTransport);
    }


protected:
    CmWebSocketHandle*                          m_pWSHandle;

    MockICmBaseHTTPExProtoHandleSink            m_HTTPExProtoSink;  
    MockICmOnePortSink                          m_OnePortSink;
    MockICmTransportSink                        m_DataTransportSink;
    
    MockICmTransport                            m_DataTransport;
    MockICmChannelHttpServer                    m_HttpServer;
};


TEST_F(CmWebSocketHandleTest, InitializeNullPointTest)
{
    ASSERT_TRUE(NULL != m_pWSHandle);
    EXPECT_TRUE(CM_ERROR_NULL_POINTER == m_pWSHandle->Initialize(NULL, NULL));
}

TEST_F(CmWebSocketHandleTest, InitializeTwoTimesTest)
{
    ASSERT_TRUE(NULL != m_pWSHandle);
    EXPECT_TRUE(CM_OK == m_pWSHandle->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));
    EXPECT_TRUE(CM_ERROR_ALREADY_INITIALIZED == m_pWSHandle->Initialize(&m_HTTPExProtoSink, &m_OnePortSink));
}

TEST_F(CmWebSocketHandleTest, HandshakeChromeTest)
{
    HandshakeTest();
}

TEST_F(CmWebSocketHandleTest, InteractiveChromeTest)
{
    InteractiveTest();
}

TEST_F(CmWebSocketHandleTest, HandshakeFireFoxTest)
{
    FireFoxEnv();
    HandshakeTest();
}

TEST_F(CmWebSocketHandleTest, InteractiveFireFoxTest)
{
    FireFoxEnv();
    InteractiveTest();
}

TEST_F(CmWebSocketHandleTest, InteractiveChromeTestOneMsgOnePacket)
{
    InteractiveTestOneMsgOnePacket();
}

TEST_F(CmWebSocketHandleTest, InteractiveFireFoxTestOneMsgOnePacket)
{
    FireFoxEnv();
    InteractiveTestOneMsgOnePacket();
}

TEST_F(CmWebSocketHandleTest, InteractiveChromeTestOneMsgThreePacket)
{
    InteractiveTestOneMsgThreePackets();
}

TEST_F(CmWebSocketHandleTest, InteractiveFireFoxTestOneMsgThreePacket)
{
    FireFoxEnv();
    InteractiveTestOneMsgThreePackets();
}

TEST_F(CmWebSocketHandleTest, InteractiveChromeTestThreeMsgOnePacket)
{
    InteractiveTestThreeMsgOnePacket();
}

TEST_F(CmWebSocketHandleTest, InteractiveFireFoxTestThreeMsgOnePacket)
{
    FireFoxEnv();
    InteractiveTestThreeMsgOnePacket();
}