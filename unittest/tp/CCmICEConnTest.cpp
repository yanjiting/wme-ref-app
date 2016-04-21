#include <gmock/gmock.h>
#include <gtest/gtest.h>


#define private public
#define protected public


#include "CmICEImpl.h"

#include "Mock_CmOnePortInterface.h"

#include "Mock_CmOnePortImpl.h"

#include "Mock_CmConnectionInterface.h"

#include "Mock_ICmICE.h"

#include "CmInetAddr.h"



using ::testing::_;
using ::testing::Mock;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::Invoke;
using ::testing::NiceMock;


#define SAFE_DELETE(p)              {if(NULL != p) {delete p; p = NULL;}}
#define SAFE_DELETE_ARRAY(p)        {if(NULL != p) {delete [] p; p = NULL;}}

#define SAFE_RELEASE(p)              {if(NULL != p) {p->ReleaseReference();}}
#define SAFE_ADDREF(p)              {if(NULL != p) {p->AddReference();}}


//CCmOnePortICEImpl
class CCmICEConnTest : public testing::Test
{
public:
    CCmICEConnTest()
    :m_pConn(NULL)
    {        
    }

    virtual ~CCmICEConnTest()
    {     
    }

    virtual void SetUp()
    {
       
        if (m_pConn == NULL) 
        {  
            m_pServerMock = new MockICmTransport();      
            EXPECT_CALL(*m_pServerMock, AddReference()).WillRepeatedly(Return(1));
            EXPECT_CALL(*m_pServerMock, ReleaseReference()).WillRepeatedly(Return(1));
            //EXPECT_CALL(*m_pServerMock, Disconnect(_)).WillRepeatedly(Return(CM_OK));
            EXPECT_CALL(*m_pServerMock, OpenWithSink(_))
                        .Times(AtLeast(1))
                        .WillRepeatedly(Return(CM_OK));
            
            m_pICESinkMock = new MockICmICESink(); 
            m_pAcceptor = new CCmICEImpl();    
            m_pAcceptor->m_pSink= m_pICESinkMock;
            m_pUpperSinkMock = new MockICmTransportSink();             
            m_pConn = new CCmICEConn(m_pServerMock, m_pAcceptor);
            m_pConn->m_pUpperSink = m_pUpperSinkMock;
        }
      
    }

    virtual void TearDown()
    {     
        SAFE_RELEASE(m_pConn);
        SAFE_DELETE(m_pUpperSinkMock);
        //SAFE_DELETE(m_pAcceptor);
        SAFE_DELETE(m_pICESinkMock);
        SAFE_DELETE(m_pServerMock);
    }

       
    MockICmTransport* m_pServerMock;
    CCmICEImpl* m_pAcceptor;//let conn has one reference of acceptor
    MockICmTransportSink *m_pUpperSinkMock;
    MockICmICESink *m_pICESinkMock; 
    CCmICEConn *m_pConn;
    
public:

    
    void OnServerCreation_Mock(ICmICETransport* aTransport) {SAFE_ADDREF(aTransport);}
    
};


//CCmICEConn

TEST_F(CCmICEConnTest, OnReceive_OK)
{   
    CM_ASSERTE(NULL != m_pConn);
    IStunMessage m_stunmsg;
    m_stunmsg.Clear();
    m_stunmsg.SetClass(e_StunMsgClassRequest);
    m_stunmsg.SetMethod(e_StunMsgMethodBinding);
    
    StunTransactionID *transid = IStunMessage::GenerateTransactionID();
    EXPECT_TRUE(transid != NULL);
    
    m_stunmsg.SetTransactionID(transid);
    
    std::string user = "user_name";
    m_stunmsg.SetAttrString(e_StunMsgAttrUserName, user);
    
    std::string cred = "abc";
    uint8_t hmac[20] = {0};
    m_stunmsg.SetAttrMessageIntegrity(e_StunMsgAttrMessageIntegrity, 20, NULL);
    m_stunmsg.GetMsgIntegrityHMAC((const uint8_t *)cred.c_str(), cred.length(), hmac);
    m_stunmsg.SetAttrMessageIntegrity(e_StunMsgAttrMessageIntegrity, 20, hmac);
    
    uint32_t length;
    const char *buffer = m_stunmsg.Encode(length);
    EXPECT_TRUE(length > 0);
    LOG_STUNBUFF(buffer, 20);

    CCmMessageBlock msg_block(length,buffer,CCmMessageBlock::DONT_DELETE,length);


    m_pAcceptor->SetClientPassword(user,cred);
    
    EXPECT_CALL(*m_pICESinkMock, OnServerCreation(_))
        .Times(AtLeast(1))
        .WillRepeatedly(Invoke(this,&CCmICEConnTest::OnServerCreation_Mock));    

    EXPECT_CALL(*m_pServerMock, SendData(_,_))
        .Times(AtLeast(1))
        .WillRepeatedly(testing::Return(CM_OK));
    // 1st time 
    m_pConn->OnReceive(msg_block, m_pServerMock);

  
    // 2nd time 
    m_pConn->OnReceive(msg_block, m_pServerMock);


    // 3rd time 
    EXPECT_CALL(*m_pUpperSinkMock, OnReceive(_,_,_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return());  
    
    char type[2];
    type[0] = 0x01;//rtp data
    CCmMessageBlock msg_block2(sizeof(type),type,CCmMessageBlock::DONT_DELETE,sizeof(type));
    
    m_pConn->OnReceive(msg_block2, m_pServerMock);

}



