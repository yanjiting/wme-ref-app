#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "wbxmock.h"
#include "wbxmockbase.h"
#include "wbxmockhelper.h"



#define private public
#define protected public


#include "CmOnePortICEImpl.h"

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



//CCmOnePortICEConn
class CCmOnePortICEConnTest : public testing::Test
{
public:
    CCmOnePortICEConnTest()
    {
        m_pConn = NULL;
    }

    virtual ~CCmOnePortICEConnTest()
    {
        /*
        if(NULL != m_pConn) 
        {
            delete m_pConn;
            m_pConn = NULL;
        }*/
        
    }

    
    // setup only once
    static void SetUpTestCase() 
    {
        //shared_resource_ = new ...;
        m_pOnePortMock = new MockCCmOnePortImpl();            
        m_pOnePortICE = CCmOnePortICEImplSingleT::Instance();
        m_pOnePortICE->m_pOnePort = dynamic_cast<CCmOnePortImpl*>(m_pOnePortMock);

        EXPECT_CALL(*m_pOnePortMock, AddReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(*m_pOnePortMock, ReleaseReference()).WillRepeatedly(testing::Return(1));
    }
    
    // tear down only once 
    static void TearDownTestCase() 
    {
        //delete shared_resource_;
        //shared_resource_ = NULL;

        //m_pOnePortICE->~CCmOnePortICEImpl();
        //SAFE_DELETE(m_pOnePortMock);
    }

    virtual void SetUp()
    {
       
        if (m_pConn == NULL) 
        {   
            m_pServerMock = new MockICmOnePortTransport();                  
           
            m_pUpperSinkMock = new MockICmTransportSink(); 
            m_pConn = new CCmOnePortICEConn(m_pServerMock, m_pOnePortICE);
            m_pConn->m_pUpperSink = m_pUpperSinkMock;
        }

   
        EXPECT_CALL(*m_pServerMock, Disconnect(_))
            .WillRepeatedly(Return(CM_OK));

        EXPECT_CALL(*m_pServerMock, AddReference()).WillRepeatedly(testing::Return(1));
        EXPECT_CALL(*m_pServerMock, ReleaseReference()).WillRepeatedly(testing::Return(1));
       
    }

    virtual void TearDown()
    {
        //SAFE_DELETE(m_pConn);   
        SAFE_RELEASE(m_pConn);     
        SAFE_DELETE(m_pUpperSinkMock);
        
        SAFE_DELETE(m_pServerMock);
        
    }

    MockICmOnePortTransport*        m_pServerMock;     
    static MockCCmOnePortImpl*             m_pOnePortMock;  
    static CCmOnePortICEImpl*              m_pOnePortICE ;
    MockICmTransportSink*           m_pUpperSinkMock; 

    CCmOnePortICEConn*              m_pConn;    

   
public:
    CmResult GetConnType_Mock(UINT32  &type) {type = PROTOCOL_TYPE_WEBSOCKET; return CM_OK;}
    CmResult GetRequestPath_MockSignal(CCmString& aPath) {aPath = "/"; return CM_OK;}
    CmResult GetRequestPath_MockData(CCmString& aPath) {aPath = " "; return CM_OK;}
    void OnServerCreation_Mock(ICmICETransport* aTransport) {SAFE_ADDREF(aTransport);}

    
};


MockCCmOnePortImpl*  CCmOnePortICEConnTest::m_pOnePortMock;  
CCmOnePortICEImpl*   CCmOnePortICEConnTest::m_pOnePortICE ;


typedef CM_THREAD_ID (ACmThread::*FUN_GetThreadId)();
typedef CM_THREAD_ID (*FUN_GetThreadId_MOCK)();

CM_THREAD_ID GetThreadId_Mock()
{
    return GetThreadSelfId();
}

//CCmOnePortICEConn::OnReceive
TEST_F(CCmOnePortICEConnTest, OnReceive_Signal)
{    
    FUN_GetThreadId f1 = (FUN_GetThreadId)&ACmThread::GetThreadId;
    FUN_GetThreadId_MOCK f2 = (FUN_GetThreadId_MOCK)&GetThreadId_Mock;
    wmMock(WM_FUN(f1), WM_FUN(f2));
    
    CM_ASSERTE(NULL != m_pConn);
    CCmMessageBlock mbZero((DWORD)0);
    
    EXPECT_CALL(*m_pServerMock, GetConnType(_))
        .Times(AtLeast(1))
        .WillRepeatedly(Invoke(this,&CCmOnePortICEConnTest::GetConnType_Mock));
    
    EXPECT_CALL(*m_pServerMock, GetRequestPath(_))
        .Times(AtLeast(1))
        .WillRepeatedly(Invoke(this,&CCmOnePortICEConnTest::GetRequestPath_MockSignal));
    
    EXPECT_CALL(*m_pUpperSinkMock, OnReceive(_,_,_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return());  

    //m_pConn->SetProtoHandle(&m_Proto);
    m_pConn->OnReceive(mbZero, (ICmTransport*)(m_pServerMock));


    wmUnmock(WM_FUN(f1));
}


TEST_F(CCmOnePortICEConnTest, OnReceive_Data)
{
    CM_ASSERTE(NULL != m_pConn);
    CCmMessageBlock mbZero((DWORD)0);
    //StartListen,      CmResult(ICmOnePortSink *aSink, const char *addrport));
    //
    
    EXPECT_CALL(*m_pOnePortMock, StartListen(_,_))
        .Times(AtLeast(1))
        .WillRepeatedly(Return(CM_OK));


    MockICmICESink*   pICESinkMock = new MockICmICESink();
    
    //m_pOnePortICE->m_pOnePort = m_pOnePortMock;
    
    //m_pOnePortICE->m_SinkMap[CTYPE_ONEPORT_DATA] = m_pOnePortMock;
    //CCmInetAddr addr("127.0.0.1",80);
    m_pOnePortICE->StartListen(pICESinkMock, CCmInetAddr("127.0.0.1",443), CTYPE_ONEPORT_DATA);


    
    //************************************************************************************************
    
    EXPECT_CALL(*m_pServerMock, GetConnType(_))
        .Times(AtLeast(1))
        .WillRepeatedly(Invoke(this,&CCmOnePortICEConnTest::GetConnType_Mock));
    
    EXPECT_CALL(*m_pServerMock, GetRequestPath(_))
        .Times(AtLeast(1))
        .WillRepeatedly(Invoke(this,&CCmOnePortICEConnTest::GetRequestPath_MockData));     



  /*  
    EXPECT_CALL(*pICESinkMock, OnServerCreation(_))
        .Times(AtLeast(1))
        .WillRepeatedly(Invoke(this,&CCmOnePortICEConnTest::OnServerCreation_Mock));   */ 
   
    m_pConn->OnReceive(mbZero, (ICmTransport*)(m_pServerMock));


    //*********************************************************************
    
	
    EXPECT_CALL(*m_pOnePortMock, StopListen(_))
           .Times(AtLeast(1))
           .WillRepeatedly(Return(CM_OK));


    m_pOnePortICE->StopListen(0, CTYPE_ONEPORT_DATA);
    
    SAFE_DELETE(pICESinkMock);
    
    //SAFE_RELEASE(m_pConn);       
}

//CCmOnePortICEConn::OnDisconnect
TEST_F(CCmOnePortICEConnTest, OnDisconnect_OKTest)
{
    FUN_GetThreadId f1 = (FUN_GetThreadId)&ACmThread::GetThreadId;
    FUN_GetThreadId_MOCK f2 = (FUN_GetThreadId_MOCK)&GetThreadId_Mock;
    wmMock(WM_FUN(f1), WM_FUN(f2));
    
    EXPECT_CALL(*m_pServerMock, Disconnect(_))
        .Times(AtLeast(1))
        .WillRepeatedly(testing::Return(CM_OK));
    EXPECT_CALL(*m_pUpperSinkMock, OnDisconnect(_, _))
        .Times(AtLeast(1))
        .WillRepeatedly(testing::Return());

    m_pConn->OnDisconnect(0, NULL);

    wmUnmock(WM_FUN(f1));
}

TEST_F(CCmOnePortICEConnTest, ParseStunMessage_OKTest)
{

    
    std::cout << "StunMessageTest::MessageHeader" << std::endl;
    IStunMessage m_stunmsg;
    m_stunmsg.Clear();
    m_stunmsg.SetClass(e_StunMsgClassRequest);
    m_stunmsg.SetMethod(e_StunMsgMethodBinding);
    
    StunTransactionID *transid = IStunMessage::GenerateTransactionID();
    EXPECT_TRUE(transid != NULL);
    
    m_stunmsg.SetTransactionID(transid);
   
   /* uint8_t buff[] = {0x1, 0x2, 0x4, 0x8, 0x16, 0x32};
    m_stunmsg.SetAttrMessageIntegrity(e_StunMsgAttrMessageIntegrity, 6, buff);*/

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

    m_pOnePortICE->SetClientPassword(user,cred);
    //    MOCK_METHOD2(SendData, CmResult(CCmMessageBlock &aData, CCmTransportParameter *aPara ) );
    EXPECT_CALL(*m_pServerMock, SendData(_,_))
        .Times(AtLeast(1))
        .WillRepeatedly(testing::Return(CM_OK));
     
    m_pConn->ParseStunMessage(msg_block);



}


