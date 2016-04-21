/*
 * iceEngineTransporttest.cpp
 *
 *  Created on: Oct 15, 2013
 *      Author: haijutan
 */
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "iceEngineTransport.h"
#include "Mock_CmConnectionInterface.h"
#include "Mock_CmHttpInterface.h"

using ::testing::_;
using ::testing::Mock;
using ::testing::Return;

class CIceEngineTransportTest : public testing::Test
{
public:

	CIceEngineTransportTest()
	{
	}

	virtual ~CIceEngineTransportTest()
	{
	}


	virtual void SetUp()
	{

	}

	virtual void TearDown()
	{

	}
};

TEST_F(CIceEngineTransportTest,AddReleaseReference)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	DWORD dwRef = pIceEngineTransport->AddReference();
	EXPECT_EQ(1, dwRef);
	dwRef = pIceEngineTransport->ReleaseReference();
	EXPECT_EQ(0, dwRef);
}

TEST_F(CIceEngineTransportTest,OpenWithSink)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CmResult ret = pIceEngineTransport->OpenWithSink(NULL);
	EXPECT_EQ(CM_OK, ret);
	pIceEngineTransport->ReleaseReference();
}

TEST_F(CIceEngineTransportTest,GetSink)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	pIceEngineTransport->OpenWithSink(NULL);
	ICmTransportSink* pSink = pIceEngineTransport->GetSink();
	EXPECT_TRUE(NULL == pSink);
	pIceEngineTransport->ReleaseReference();
}


TEST_F(CIceEngineTransportTest,SendData_Uninitialized)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_TOTAL,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CCmMessageBlock aData;
	CmResult ret = pIceEngineTransport->SendData(aData);
	EXPECT_EQ(CM_ERROR_NOT_INITIALIZED, ret);
	pIceEngineTransport->ReleaseReference();
}

TEST_F(CIceEngineTransportTest,SendData_UDPTCP)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	MockICmTransport *pMockICmTransport = new MockICmTransport();
	EXPECT_CALL(*pMockICmTransport, SendData(_,_)).WillRepeatedly(Return(CM_ERROR_FAILURE));
	EXPECT_CALL(*pMockICmTransport, GetOption(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmTransport, OpenWithSink(_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmTransport, Disconnect(_)).WillRepeatedly(Return(CM_OK));
	CCmMessageBlock aData;
	pIceEngineTransport->SetTransport(pMockICmTransport,true);
	CmResult ret = pIceEngineTransport->SendData(aData);
	EXPECT_EQ(CM_ERROR_FAILURE, ret);

	EXPECT_CALL(*pMockICmTransport, SendData(_,_)).WillRepeatedly(Return(CM_OK));
	ret = pIceEngineTransport->SendData(aData);
	EXPECT_EQ(CM_OK, ret);
	pIceEngineTransport->ReleaseReference();
	if(pMockICmTransport)
	{
		delete pMockICmTransport;
		pMockICmTransport = NULL;
	}
}

TEST_F(CIceEngineTransportTest,SendData_WebSocket)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	MockICmChannel *pMockICmChannel = new MockICmChannel();
	EXPECT_CALL(*pMockICmChannel, SendData(_,_)).WillRepeatedly(Return(CM_ERROR_FAILURE));
	EXPECT_CALL(*pMockICmChannel, Disconnect(_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmChannel, GetOption(_,_)).WillRepeatedly(Return(CM_OK));
	CCmMessageBlock aData;
	pIceEngineTransport->SetTransport(pMockICmChannel,true);
	CmResult ret = pIceEngineTransport->SendData(aData);
	EXPECT_EQ(CM_ERROR_FAILURE, ret);

	EXPECT_CALL(*pMockICmChannel, SendData(_,_)).WillRepeatedly(Return(CM_OK));
	ret = pIceEngineTransport->SendData(aData);
	EXPECT_EQ(CM_OK, ret);
	pIceEngineTransport->ReleaseReference();
	if(pMockICmChannel)
	{
		delete pMockICmChannel;
		pMockICmChannel = NULL;
	}
}

TEST_F(CIceEngineTransportTest,Disconnect_Uninitialized)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_TOTAL,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CmResult ret = pIceEngineTransport->Disconnect(CM_ERROR_NOT_INITIALIZED);
	EXPECT_EQ(CM_ERROR_NOT_INITIALIZED, ret);
	pIceEngineTransport->ReleaseReference();
}

TEST_F(CIceEngineTransportTest,Disconnect_UDPTCP)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	MockICmTransport *pMockICmTransport = new MockICmTransport();
	EXPECT_CALL(*pMockICmTransport, SendData(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmTransport, GetOption(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmTransport, OpenWithSink(_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmTransport, Disconnect(_)).WillRepeatedly(Return(CM_OK));
	pIceEngineTransport->SetTransport(pMockICmTransport,true);
	CmResult ret = pIceEngineTransport->Disconnect(CM_OK);
	EXPECT_EQ(CM_OK, ret);
	pIceEngineTransport->ReleaseReference();
	if(pMockICmTransport)
	{
		delete pMockICmTransport;
		pMockICmTransport = NULL;
	}
}

TEST_F(CIceEngineTransportTest,Disconnect_WebSocket)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	MockICmChannel *pMockICmChannel = new MockICmChannel();
	EXPECT_CALL(*pMockICmChannel, Disconnect(_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmChannel, GetOption(_,_)).WillRepeatedly(Return(CM_OK));
	pIceEngineTransport->SetTransport(pMockICmChannel,true);
	CmResult ret = pIceEngineTransport->Disconnect(CM_OK);
	EXPECT_EQ(CM_OK, ret);
	pIceEngineTransport->ReleaseReference();
	if(pMockICmChannel)
	{
		delete pMockICmChannel;
		pMockICmChannel = NULL;
	}
}

TEST_F(CIceEngineTransportTest,SetOption_Uninitialized_TotalType)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_TOTAL,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CmResult ret = pIceEngineTransport->SetOption(1,NULL);
	EXPECT_EQ(CM_ERROR_NOT_INITIALIZED,ret);
	pIceEngineTransport->ReleaseReference();
}

TEST_F(CIceEngineTransportTest,SetOption_Uninitialized_UDPTCP)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CmResult ret = pIceEngineTransport->SetOption(1,NULL);
	EXPECT_EQ(CM_ERROR_NOT_INITIALIZED,ret);
	pIceEngineTransport->ReleaseReference();
}

TEST_F(CIceEngineTransportTest,SetOption_Uninitialized_WebSocket)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CmResult ret = pIceEngineTransport->SetOption(1,NULL);
	EXPECT_EQ(CM_ERROR_NOT_INITIALIZED,ret);
	pIceEngineTransport->ReleaseReference();
}

TEST_F(CIceEngineTransportTest,SetOption_UDPTCP)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	MockICmTransport *pMockICmTransport = new MockICmTransport();
	EXPECT_CALL(*pMockICmTransport, SetOption(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmTransport, GetOption(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmTransport, OpenWithSink(_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmTransport, Disconnect(_)).WillRepeatedly(Return(CM_OK));
	pIceEngineTransport->SetTransport(pMockICmTransport,true);
	CmResult ret = pIceEngineTransport->SetOption(1,NULL);
	EXPECT_EQ(CM_OK,ret);
	pIceEngineTransport->ReleaseReference();
	if(pMockICmTransport)
	{
		delete pMockICmTransport;
		pMockICmTransport = NULL;
	}
}

TEST_F(CIceEngineTransportTest,SetOption_WebSocket)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	MockICmChannel *pMockICmChannel = new MockICmChannel();
	EXPECT_CALL(*pMockICmChannel, SetOption(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmChannel, GetOption(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmChannel, Disconnect(_)).WillRepeatedly(Return(CM_OK));
	pIceEngineTransport->SetTransport(pMockICmChannel,true);
	CmResult ret = pIceEngineTransport->SetOption(1,NULL);
	EXPECT_EQ(CM_OK,ret);
	pIceEngineTransport->ReleaseReference();
	if(pMockICmChannel)
	{
		delete pMockICmChannel;
		pMockICmChannel = NULL;
	}
}

TEST_F(CIceEngineTransportTest,GetOption_Uninitialized_TotalType)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_TOTAL,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CmResult ret = pIceEngineTransport->GetOption(1,NULL);
	EXPECT_EQ(CM_ERROR_NOT_INITIALIZED,ret);
	pIceEngineTransport->ReleaseReference();
}

TEST_F(CIceEngineTransportTest,GetOption_Uninitialized_UDPTCP)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CmResult ret = pIceEngineTransport->GetOption(1,NULL);
	EXPECT_EQ(CM_ERROR_NOT_INITIALIZED,ret);
	pIceEngineTransport->ReleaseReference();
}

TEST_F(CIceEngineTransportTest,GetOption_Uninitialized_WebSocket)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CmResult ret = pIceEngineTransport->GetOption(1,NULL);
	EXPECT_EQ(CM_ERROR_NOT_INITIALIZED,ret);
	pIceEngineTransport->ReleaseReference();
}

TEST_F(CIceEngineTransportTest,GetOption_UDPTCP)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	MockICmTransport *pMockICmTransport = new MockICmTransport();
	EXPECT_CALL(*pMockICmTransport, GetOption(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmTransport, OpenWithSink(_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmTransport, Disconnect(_)).WillRepeatedly(Return(CM_OK));
	pIceEngineTransport->SetTransport(pMockICmTransport,true);
	CmResult ret = pIceEngineTransport->GetOption(1,NULL);
	EXPECT_EQ(CM_OK,ret);
	pIceEngineTransport->ReleaseReference();
	if(pMockICmTransport)
	{
		delete pMockICmTransport;
		pMockICmTransport = NULL;
	}
}

TEST_F(CIceEngineTransportTest,GetOption_WebSocket)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	MockICmChannel *pMockICmChannel = new MockICmChannel();
	EXPECT_CALL(*pMockICmChannel, GetOption(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmChannel, Disconnect(_)).WillRepeatedly(Return(CM_OK));
	pIceEngineTransport->SetTransport(pMockICmChannel,true);
	CmResult ret = pIceEngineTransport->GetOption(1,NULL);
	EXPECT_EQ(CM_OK,ret);
	pIceEngineTransport->ReleaseReference();
	if(pMockICmChannel)
	{
		delete pMockICmChannel;
		pMockICmChannel = NULL;
	}
}

TEST_F(CIceEngineTransportTest,OnTimer)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	pIceEngineTransport->OnTimer(NULL);
	pIceEngineTransport->ReleaseReference();
}

TEST_F(CIceEngineTransportTest,SendStunResponse)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	StunTransactionID* id = new StunTransactionID();
	pIceEngineTransport->SendStunResponse(e_StunMsgSuccess,id);
	pIceEngineTransport->ReleaseReference();
	if(id)
	{
		delete id;
		id = NULL;
	}
}

TEST_F(CIceEngineTransportTest,SendStunRequest)
{
	IceEngineCandidate *cand = new IceEngineCandidate();
	cand->m_transType = IE_TRANSPORT_TYPE_TOTAL;
	CIceEngineCheck *pCheck = new CIceEngineCheck(1, NULL, cand,cand);
	pCheck->SetNominated();
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(pCheck,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	pIceEngineTransport->SendStunRequest();
	pIceEngineTransport->ReleaseReference();
	if(pCheck)
	{
		delete pCheck;
		pCheck = NULL;
	}
	if(cand)
	{
		delete cand;
		cand = NULL;
	}
}

TEST_F(CIceEngineTransportTest,SetTransport_UDPTCP)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	MockICmTransport *pMockICmTransport = new MockICmTransport();
	EXPECT_CALL(*pMockICmTransport, SetOption(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmTransport, GetOption(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmTransport, OpenWithSink(_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmTransport, Disconnect(_)).WillRepeatedly(Return(CM_OK));
	pIceEngineTransport->SetTransport(pMockICmTransport,true);
	pIceEngineTransport->ReleaseReference();
	if(pMockICmTransport)
	{
		delete pMockICmTransport;
		pMockICmTransport = NULL;
	}
}


TEST_F(CIceEngineTransportTest,SetTransport_WebSocket)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	MockICmChannel *pMockICmChannel = new MockICmChannel();
	EXPECT_CALL(*pMockICmChannel, SetOption(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmChannel, GetOption(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmChannel, Disconnect(_)).WillRepeatedly(Return(CM_OK));
	pIceEngineTransport->SetTransport(pMockICmChannel,true);
	pIceEngineTransport->ReleaseReference();
	if(pMockICmChannel)
	{
		delete pMockICmChannel;
		pMockICmChannel = NULL;
	}
}

TEST_F(CIceEngineTransportTest,OnChannelConnectionFail)
{
	IceEngineCandidate *cand = new IceEngineCandidate();
	cand->m_transType = IE_TRANSPORT_TYPE_TOTAL;
	CIceEngineCheck *pCheck = new CIceEngineCheck(1, NULL, cand,cand);
	pCheck->SetNominated();
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(pCheck,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	pIceEngineTransport->OnChannelConnectionFail(CM_OK);
	pIceEngineTransport->ReleaseReference();
	if(pCheck)
	{
		delete pCheck;
		pCheck = NULL;
	}
	if(cand)
	{
		delete cand;
		cand = NULL;
	}
}

TEST_F(CIceEngineTransportTest,ConnectAsWebSocket)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CmResult ret = pIceEngineTransport->ConnectAsWebSocket("wrongurl");
	EXPECT_EQ(CM_ERROR_FAILURE, ret);
	ret = pIceEngineTransport->ConnectAsWebSocket("wss://www.example.com/audio");
	EXPECT_EQ(CM_OK,ret);
	pIceEngineTransport->ReleaseReference();
}

TEST_F(CIceEngineTransportTest, OnReceive_ErrorDecode)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	MockICmTransportSink *pTransportSink = new MockICmTransportSink();
	pIceEngineTransport->OpenWithSink(pTransportSink);
	EXPECT_CALL(*pTransportSink, OnReceive(_,_,_));
	CCmMessageBlock aData;
	pIceEngineTransport->OnReceive(aData,NULL);
	pIceEngineTransport->ReleaseReference();
	if(pTransportSink)
	{
		delete pTransportSink;
		pTransportSink = NULL;
	}
}

TEST_F(CIceEngineTransportTest, OnReceive_NoIntegrity)
{
	CIceEngineCheckMgr *pCheckMgr = new CIceEngineCheckMgr(NULL,ICE_NOMINATION_MODE_AGGRESSIVE,ICE_ROLE_CONTROLLED,1,"shortermCred");
	IceEngineCandidate *cand = new IceEngineCandidate();
	cand->m_transType = IE_TRANSPORT_TYPE_TOTAL;
	CIceEngineCheck *pCheck = new CIceEngineCheck(1,pCheckMgr,cand,cand);
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(pCheck,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	IStunMessage m_stunmsg;
	m_stunmsg.Clear();

	uint32_t length;
    const char *buffer = m_stunmsg.Encode(length);
    EXPECT_TRUE(length > 0);
    LOG_STUNBUFF(buffer, 20);
	CCmMessageBlock msg_block(length,buffer,CCmMessageBlock::DONT_DELETE,length);
	pIceEngineTransport->OnReceive(msg_block,NULL);
	pIceEngineTransport->ReleaseReference();
		if(pCheck)
	{
		delete pCheck;
		pCheck = NULL;
	}
	if(cand)
	{
		delete cand;
		cand = NULL;
	}
	if(pCheckMgr)
	{
		delete pCheckMgr;
		pCheckMgr = NULL;
	}
}

TEST_F(CIceEngineTransportTest, CheckMsgIntegrity_WrongCred)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	IStunMessage m_stunmsg;
	m_stunmsg.Clear();

	uint8_t buff[] = {0x1, 0x2, 0x4, 0x8, 0x16, 0x32};
    m_stunmsg.SetAttrMessageIntegrity(e_StunMsgAttrMessageIntegrity, 6, buff);

	uint32_t length;
    const char *buffer = m_stunmsg.Encode(length);
    EXPECT_TRUE(length > 0);
    LOG_STUNBUFF(buffer, 20);

    CCmMessageBlock msg_block(length,buffer,CCmMessageBlock::DONT_DELETE,length);
	pIceEngineTransport->OnReceive(msg_block,NULL);
	pIceEngineTransport->ReleaseReference();
}

TEST_F(CIceEngineTransportTest, ParseStunMessage_Request_Controlling)
{	
	CIceEngineCheckMgr *pCheckMgr = new CIceEngineCheckMgr(NULL,ICE_NOMINATION_MODE_AGGRESSIVE,ICE_ROLE_CONTROLLED,1,"shortermCred");
	IceEngineCandidate *cand = new IceEngineCandidate();
	cand->m_transType = IE_TRANSPORT_TYPE_TOTAL;
	CIceEngineCheck *pCheck = new CIceEngineCheck(1,pCheckMgr,cand,cand);
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(pCheck,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	IStunMessage m_stunmsg;
	m_stunmsg.Clear();
	m_stunmsg.SetClass(e_StunMsgClassRequest);
    m_stunmsg.SetMethod(e_StunMsgMethodBinding);
	m_stunmsg.SetAttrEmpty(e_StunMsgAttrUseCandidate);
	StunTransactionID *transid = IStunMessage::GenerateTransactionID();
	m_stunmsg.SetTransactionID(transid);
	m_stunmsg.SetAttrString(e_StunMsgAttrUserName,"username");
    m_stunmsg.SetAttrUInt64(e_StunMsgAttrIceControlling, 0x1234567887654321);
	uint32_t length;
    const char *buffer = m_stunmsg.Encode(length);
    EXPECT_TRUE(length > 0);
    LOG_STUNBUFF(buffer, 20);
    CCmMessageBlock msg_block(length,buffer,CCmMessageBlock::DONT_DELETE,length);

	pIceEngineTransport->OnReceive(msg_block,NULL);
	pIceEngineTransport->ReleaseReference();
	if(pCheck)
	{
		delete pCheck;
		pCheck = NULL;
	}
	if(cand)
	{
		delete cand;
		cand = NULL;
	}
	if(pCheckMgr)
	{
		delete pCheckMgr;
		pCheckMgr = NULL;
	}
}

TEST_F(CIceEngineTransportTest, ParseStunMessage_Request_Controlled)
{
	CIceEngineCheckMgr *pCheckMgr = new CIceEngineCheckMgr(NULL,ICE_NOMINATION_MODE_AGGRESSIVE,ICE_ROLE_CONTROLLING,1,"shortermCred");
	IceEngineCandidate *cand = new IceEngineCandidate();
	cand->m_transType = IE_TRANSPORT_TYPE_TOTAL;
	CIceEngineCheck *pCheck = new CIceEngineCheck(1,pCheckMgr,cand,cand);
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(pCheck,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	IStunMessage m_stunmsg;
	m_stunmsg.Clear();
	m_stunmsg.SetClass(e_StunMsgClassRequest);
    m_stunmsg.SetMethod(e_StunMsgMethodBinding);
	m_stunmsg.SetAttrEmpty(e_StunMsgAttrUseCandidate);
	StunTransactionID *transid = IStunMessage::GenerateTransactionID();
	m_stunmsg.SetTransactionID(transid);
	m_stunmsg.SetAttrString(e_StunMsgAttrUserName,"username");
    m_stunmsg.SetAttrUInt64(e_StunMsgAttrIceControlled, 0x1234567887654321);
	uint32_t length;
    const char *buffer = m_stunmsg.Encode(length);
    EXPECT_TRUE(length > 0);
    LOG_STUNBUFF(buffer, 20);
    CCmMessageBlock msg_block(length,buffer,CCmMessageBlock::DONT_DELETE,length);

	pIceEngineTransport->OnReceive(msg_block,NULL);
	pIceEngineTransport->ReleaseReference();
	if(pCheck)
	{
		delete pCheck;
		pCheck = NULL;
	}
	if(cand)
	{
		delete cand;
		cand = NULL;
	}
	if(pCheckMgr)
	{
		delete pCheckMgr;
		pCheckMgr = NULL;
	}
}

TEST_F(CIceEngineTransportTest, ParseStunMessage_Response_MissTransaction)
{
	CIceEngineCheckMgr *pCheckMgr = new CIceEngineCheckMgr(NULL,ICE_NOMINATION_MODE_AGGRESSIVE,ICE_ROLE_CONTROLLING,1,"shortermCred");
	IceEngineCandidate *cand = new IceEngineCandidate();
	cand->m_transType = IE_TRANSPORT_TYPE_TOTAL;
	CIceEngineCheck *pCheck = new CIceEngineCheck(1,pCheckMgr,cand,cand);
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(pCheck,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	IStunMessage m_stunmsg;
	m_stunmsg.Clear();
	m_stunmsg.SetClass(e_StunMsgClassSuccessResponse);
    m_stunmsg.SetMethod(e_StunMsgMethodBinding);
	m_stunmsg.SetAttrEmpty(e_StunMsgAttrUseCandidate);
	StunTransactionID *transid = IStunMessage::GenerateTransactionID();
	m_stunmsg.SetTransactionID(transid);
	m_stunmsg.SetAttrString(e_StunMsgAttrUserName,"username");
    m_stunmsg.SetAttrUInt64(e_StunMsgAttrIceControlled, 0x1234567887654321);
	uint32_t length;
    const char *buffer = m_stunmsg.Encode(length);
    EXPECT_TRUE(length > 0);
    LOG_STUNBUFF(buffer, 20);
    CCmMessageBlock msg_block(length,buffer,CCmMessageBlock::DONT_DELETE,length);
	pIceEngineTransport->OnReceive(msg_block,NULL);
	pIceEngineTransport->ReleaseReference();
	if(pCheck)
	{
		delete pCheck;
		pCheck = NULL;
	}
	if(cand)
	{
		delete cand;
		cand = NULL;
	}
	if(pCheckMgr)
	{
		delete pCheckMgr;
		pCheckMgr = NULL;
	}
}


TEST_F(CIceEngineTransportTest, ParseStunMessage_ErrorMethod)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	IStunMessage m_stunmsg;
	m_stunmsg.Clear();
	m_stunmsg.SetClass(e_StunMsgClassRequest);
    m_stunmsg.SetMethod(e_StunMsgMethodSend);
	uint32_t length;
    const char *buffer = m_stunmsg.Encode(length);
    EXPECT_TRUE(length > 0);
    LOG_STUNBUFF(buffer, 20);
    CCmMessageBlock msg_block(length,buffer,CCmMessageBlock::DONT_DELETE,length);
	pIceEngineTransport->OnReceive(msg_block,NULL);
	pIceEngineTransport->ReleaseReference();
}


TEST_F(CIceEngineTransportTest, OnReceive_Success)
{	
	CIceEngineCheckMgr *pCheckMgr = new CIceEngineCheckMgr(NULL,ICE_NOMINATION_MODE_AGGRESSIVE,ICE_ROLE_CONTROLLING,1,"shortermCred");
	IceEngineCandidate *cand = new IceEngineCandidate();
	cand->m_transType = IE_TRANSPORT_TYPE_TOTAL;
	CIceEngineCheck *pCheck = new CIceEngineCheck(1,pCheckMgr,cand,cand);
	pCheck->SetState(P2P_ICE_CHECK_STATE_IN_PROGRESS);
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(pCheck,IE_TRANSPORT_TYPE_WEBSOCKET,1,1,"clientCred", "", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	IStunMessage m_stunmsg;
	m_stunmsg.Clear();
	m_stunmsg.SetClass(e_StunMsgClassRequest);
    m_stunmsg.SetMethod(e_StunMsgMethodBinding);
	m_stunmsg.SetAttrEmpty(e_StunMsgAttrUseCandidate);
	StunTransactionID *transid = IStunMessage::GenerateTransactionID();
	m_stunmsg.SetTransactionID(transid);
	m_stunmsg.SetAttrString(e_StunMsgAttrUserName,"username");
    m_stunmsg.SetAttrUInt64(e_StunMsgAttrIceControlling, 0x1234567887654321);
	uint32_t length;
    const char *buffer = m_stunmsg.Encode(length);
    EXPECT_TRUE(length > 0);
    LOG_STUNBUFF(buffer, 20);
    CCmMessageBlock msg_block(length,buffer,CCmMessageBlock::DONT_DELETE,length);

	pIceEngineTransport->OnReceive(msg_block,NULL);
	pIceEngineTransport->ReleaseReference();
	if(pCheck)
	{
		delete pCheck;
		pCheck = NULL;
	}
	if(cand)
	{
		delete cand;
		cand = NULL;
	}
	if(pCheckMgr)
	{
		delete pCheckMgr;
		pCheckMgr = NULL;
	}
}

TEST_F(CIceEngineTransportTest, OnSend)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	pIceEngineTransport->OnSend(NULL,NULL);
	pIceEngineTransport->ReleaseReference();
}

TEST_F(CIceEngineTransportTest, OnDisconnect)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	pIceEngineTransport->OnDisconnect(CM_OK,NULL);
	pIceEngineTransport->ReleaseReference();
}


TEST_F(CIceEngineTransportTest, CCmChannelSink_OnConnect_NoChannel)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CCmChannelSink *pChannelSink = new CCmChannelSink(pIceEngineTransport);
	pChannelSink->OnConnect(CM_OK,NULL);
	pIceEngineTransport->ReleaseReference();
	if(pChannelSink)
	{
		delete pChannelSink;
		pChannelSink = NULL;
	}
}

TEST_F(CIceEngineTransportTest, CCmChannelSink_OnConnect_HasChannel)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CCmChannelSink *pChannelSink = new CCmChannelSink(pIceEngineTransport);
	MockICmChannel *pMockICmChannel = new MockICmChannel();
	EXPECT_CALL(*pMockICmChannel, SetOption(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmChannel, GetOption(_,_)).WillRepeatedly(Return(CM_OK));
	EXPECT_CALL(*pMockICmChannel, Disconnect(_)).WillRepeatedly(Return(CM_OK));
	pChannelSink->OnConnect(CM_OK,pMockICmChannel);
	pIceEngineTransport->ReleaseReference();
	if(pChannelSink)
	{
		delete pChannelSink;
		pChannelSink = NULL;
	}
	if(pMockICmChannel)
	{
		delete pMockICmChannel;
		pMockICmChannel = NULL;
	}
}

TEST_F(CIceEngineTransportTest, CCmChannelSink_OnReceive)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CCmChannelSink *pChannelSink = new CCmChannelSink(pIceEngineTransport);
	CCmMessageBlock aData;
	pChannelSink->OnReceive(aData,NULL);
	pIceEngineTransport->ReleaseReference();
	if(pChannelSink)
	{
		delete pChannelSink;
		pChannelSink = NULL;
	}
}

TEST_F(CIceEngineTransportTest, CCmChannelSink_OnSend)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CCmChannelSink *pChannelSink = new CCmChannelSink(pIceEngineTransport);
	pChannelSink->OnSend(NULL,NULL);
	pIceEngineTransport->ReleaseReference();
	if(pChannelSink)
	{
		delete pChannelSink;
		pChannelSink = NULL;
	}
}

TEST_F(CIceEngineTransportTest, CCmChannelSink_OnDisconnect)
{
	CIceEngineTransport *pIceEngineTransport = new CIceEngineTransport(NULL,IE_TRANSPORT_TYPE_UDP,1,1,"clientCred", "serverCred", "username",ICE_ROLE_CONTROLLING,ICE_NOMINATION_MODE_REGULAR);
	pIceEngineTransport->AddReference();
	CCmChannelSink *pChannelSink = new CCmChannelSink(pIceEngineTransport);
	pChannelSink->OnDisconnect(CM_OK,NULL);
	pIceEngineTransport->ReleaseReference();
	if(pChannelSink)
	{
		delete pChannelSink;
		pChannelSink = NULL;
	}
}