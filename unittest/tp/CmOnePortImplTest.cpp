#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "CmOnePortImpl.h"
#include "Mock_CmBaseHTTPExProtoHandle.h"
#include "Mock_CmHttpInterface.h"
#include "Mock_CmOnePortImpl.h"
//#include "Mock_CmChannelHttpServerAcceptor.h"

class CCmOnePortConnTest : public testing::Test
{
public:
    CCmOnePortConnTest()
    {
        m_pConn = NULL;
    }

    virtual ~CCmOnePortConnTest()
    {
        /*
        if(NULL != m_pConn) 
        {
        delete m_pConn;
        m_pConn = NULL;
        }
        */
    }

    virtual void SetUp()
    {
        if (m_pConn == NULL) 
        {
            m_pConn = new MockCCmOnePortConn(&m_HttpServer, &m_OnePort);
        }

        EXPECT_CALL(m_HttpServer, OpenWithSink(testing::_))
            .WillRepeatedly(testing::Return(CM_OK));

        EXPECT_CALL(m_HttpServer, Disconnect(testing::_))
            .WillRepeatedly(testing::Return(CM_OK));

        EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::_, testing::_))
            .WillRepeatedly(testing::Return(CM_ERROR_FAILURE));

        //GetRequestHeader


        EXPECT_CALL(m_OnePort, OnConnect(testing::_, testing::_))
            .Times(testing::AnyNumber());

        EXPECT_CALL(m_Proto, AddReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(m_Proto, ReleaseReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(m_Proto, Initialize(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
        EXPECT_CALL(m_Proto, Handshake(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
    }

    virtual void TearDown()
    {
        /*
        if(NULL != m_pConn) 
        {
        delete m_pConn;
        m_pConn = NULL;
        }
        */
    }

protected:

    MockICmChannelHttpServer        m_HttpServer;
    MockICmBaseHTTPExProtoHandle    m_Proto;
    MockCCmOnePortImpl              m_OnePort;
    MockCCmOnePortConn*             m_pConn;    
};


TEST_F(CCmOnePortConnTest, OnReceiveOKTest)
{
    CM_ASSERTE(NULL != m_pConn);

    CCmMessageBlock mbZero((DWORD)0);
    EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Connection"), testing::_))
        .WillRepeatedly(DoAll(testing::SetArgReferee<1>("Upgrade"), testing::Return(CM_OK)));

    m_pConn->SetProtoHandle(&m_Proto);
    m_pConn->OnReceive(mbZero, &m_HttpServer, NULL);

}

TEST_F(CCmOnePortConnTest, OnReceiveFail1Test)
{
    CM_ASSERTE(NULL != m_pConn);

    CCmMessageBlock mbZero((DWORD)0);
    EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Connection"), testing::_))
        .WillRepeatedly(DoAll(testing::SetArgReferee<1>("Upgrade"), testing::Return(CM_OK)));

    m_pConn->SetProtoHandle(&m_Proto);

    EXPECT_CALL(m_Proto, Initialize(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_ERROR_FAILURE));
    m_pConn->OnReceive(mbZero, &m_HttpServer, NULL);

}

TEST_F(CCmOnePortConnTest, OnReceiveFail2Test)
{
    CM_ASSERTE(NULL != m_pConn);

    CCmMessageBlock mbZero((DWORD)0);
    EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Connection"), testing::_))
        .WillRepeatedly(DoAll(testing::SetArgReferee<1>("Upgrade"), testing::Return(CM_OK)));

    m_pConn->SetProtoHandle(&m_Proto);

    EXPECT_CALL(m_Proto, Initialize(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
    EXPECT_CALL(m_Proto, Handshake(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_ERROR_FAILURE));
    m_pConn->OnReceive(mbZero, &m_HttpServer, NULL);

}


TEST_F(CCmOnePortConnTest, OnReceiveRestOKTest)
{
    CM_ASSERTE(NULL != m_pConn);

    CCmMessageBlock mbZero((DWORD)0);


    EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Connection"), testing::_))
        .WillRepeatedly(DoAll(testing::SetArgReferee<1>(""), testing::Return(CM_OK)));

    m_pConn->SetProtoHandle(&m_Proto);
    m_pConn->OnReceive(mbZero, &m_HttpServer, NULL);

}


TEST_F(CCmOnePortConnTest, OnReceiveRestFail1Test)
{
    CM_ASSERTE(NULL != m_pConn);

    CCmMessageBlock mbZero((DWORD)0);


    EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Connection"), testing::_))
        .WillRepeatedly(DoAll(testing::SetArgReferee<1>(""), testing::Return(CM_OK)));

    m_pConn->SetProtoHandle(&m_Proto);


    EXPECT_CALL(m_Proto, Initialize(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_ERROR_FAILURE));
    m_pConn->OnReceive(mbZero, &m_HttpServer, NULL);

}


TEST_F(CCmOnePortConnTest, OnReceiveRestFail2Test)
{
    CM_ASSERTE(NULL != m_pConn);

    CCmMessageBlock mbZero((DWORD)0);


    EXPECT_CALL(m_HttpServer, GetRequestHeader(testing::Eq("Connection"), testing::_))
        .WillRepeatedly(DoAll(testing::SetArgReferee<1>(""), testing::Return(CM_OK)));

    m_pConn->SetProtoHandle(&m_Proto);


    EXPECT_CALL(m_Proto, Initialize(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_OK));
    EXPECT_CALL(m_Proto, Handshake(testing::_, testing::_)).WillRepeatedly(testing::Return(CM_ERROR_FAILURE));
    m_pConn->OnReceive(mbZero, &m_HttpServer, NULL);

}


class CCmOnePortImplTest : public testing::Test
{
public:

    CCmOnePortImplTest()
    {
        m_pOnePort = NULL;
    }

    virtual ~CCmOnePortImplTest()
    {
        if(NULL != m_pOnePort) {
            destroy_one_port(m_pOnePort);
            m_pOnePort = NULL;
        }
    }


    virtual void SetUp()
    {
        if (m_pOnePort == NULL) {
            m_pOnePort = (CCmOnePortImpl *)create_one_port();
        }
    }


    virtual void TearDown()
    {
        if(NULL != m_pOnePort) {
            destroy_one_port(m_pOnePort);
            m_pOnePort = NULL;
        }
    } 

protected:
    CCmOnePortImpl *m_pOnePort;
    //Mock_CmChannelHttpServerAcceptor m_server_acceptor;
};

TEST_F(CCmOnePortImplTest, StopListenTest)
{
    ASSERT_TRUE(NULL != m_pOnePort);

    //EXPECT_CALL(m_server_acceptor, StopListen(testing::_)).WillRepeatedly(testing::Return(CM_OK));

    //m_pOnePort->StopListen(CM_OK);		

}

TEST_F(CCmOnePortImplTest, OnServerCreationTest)
{
    ASSERT_TRUE(NULL != m_pOnePort);
    m_pOnePort->OnServerCreation(NULL);
}

TEST_F(CCmOnePortImplTest, ProtHandleListTest)
{
    ASSERT_TRUE(NULL != m_pOnePort);

    ICmBaseHTTPExProtoHandle *prot_handle = create_basehttpex_proto_handle(PROTOCOL_TYPE_REST);
    ASSERT_TRUE(NULL != prot_handle);

    m_pOnePort->OnConnect(CM_OK, prot_handle);
    EXPECT_TRUE(m_pOnePort->GetHandleNum() == 1);

    m_pOnePort->OnDisconnect(CM_OK, prot_handle);
    EXPECT_TRUE(m_pOnePort->GetHandleNum() == 0);    
}

