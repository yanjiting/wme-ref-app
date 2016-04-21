#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "iceEngineCheckMgr.h"
#include "iceEngineMgr.h"

class CIceEngineCheckTest : public testing::Test, public CIceEngineCheckMgrSink
{
public:

	virtual void OnCheckCompleted(bool bSuccess){};

	virtual bool OnValidTransport( const ICmTransport *pTransport, int iComponentId, eIceEngineTransType transType, eIceEngineType engineType,uint64_t transportPriority){return true;};

	virtual void OnDiscoverPeerRefOrTrigger(const CCmInetAddr &local, const CCmInetAddr &remote, ICmAcceptorConnectorId *aRequestId){};

	virtual void OnRoleChanged(eIceRole role){};

	CIceEngineCheckTest():m_pCheck(NULL)
	{
	}

	virtual ~CIceEngineCheckTest()
	{
	}


	virtual void SetUp()
	{
		eIceEngineTransType transtypes[] = {IE_TRANSPORT_TYPE_UDP,IE_TRANSPORT_TYPE_UDP,IE_TRANSPORT_TYPE_WEBSOCKET,IE_TRANSPORT_TYPE_WEBSOCKET}; 
		eIceEngineType engintypes[] = {IE_ENGINE_RTP,IE_ENGINE_RTCP,IE_ENGINE_RTP,IE_ENGINE_RTCP};

		for(int i = 0;i< 4;i++)
		{
			CCmInetAddr remote("www.google.com",80);
			IceEngineCandidate *pCand = new IceEngineCandidate();
			pCand->m_addr = remote;
			pCand->m_wsUrl = "wss://google.com:443";
			pCand->m_relIpAddr = remote;
			pCand->m_transType = transtypes[i];
			pCand->m_engineType = engintypes[i];
			pCand->m_candidateType = ICE_CANDIDATE_TYPE_HOST;
			pCand->m_iComponentId = 1;
			pCand->m_iPriority = 100;
			pCand->m_strFoundation = "100";
			m_remoteCandidates.push_back(pCand);
		}
		//IceEngineCandidate *  m_pLocalCand;
		m_pLocalCand = new IceEngineCandidate();
		m_pLocalCand->m_iComponentId = 1;
		m_pLocalCand->m_iPriority = 100;
		m_pLocalCand->m_strFoundation = "100";
		m_localCandidates.push_back(m_pLocalCand);

		m_pCheckMgr = new CIceEngineCheckMgr(
			this, 
			ICE_NOMINATION_MODE_AGGRESSIVE,
			ICE_ROLE_CONTROLLING,
			rand64bits(),
			""
			);

		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false);
			}
		}

		m_pCheck = m_pCheckMgr->GetEngineCheck(0);

	}

	virtual void TearDown()
	{
		for (Candidates::iterator it = m_localCandidates.begin(); it != m_localCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				delete(*it);
			}
		}
		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				delete(*it);
			}
		}
		m_localCandidates.clear();
		m_remoteCandidates.clear();
		delete m_pCheckMgr;
	}

protected:
	CIceEngineCheck *m_pCheck;
	CIceEngineCheckMgr *m_pCheckMgr;
	Candidates m_localCandidates;
	Candidates m_remoteCandidates;
	IceEngineCandidate *  m_pLocalCand;
};






TEST_F(CIceEngineCheckTest, SetTransport_FALSE)
{

	try
	{
		m_pCheck->SetTransport(NULL,false);
	}
	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}

}

TEST_F(CIceEngineCheckTest, OnConnect)
{

	try
	{
		//m_pCheck->OnConnect(CM_OK,(ICmChannel *)(NULL));
	}
	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}

}


TEST_F(CIceEngineCheckTest, SetTransport_TRUE)
{

	try
	{
		m_pCheck->SetTransport(NULL,true);
	}
	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}

}


/*

// ICmTransportSink
virtual void OnReceive(
					   CCmMessageBlock &aData,
					   ICmTransport *aTrptId,
					   CCmTransportParameter *aPara = NULL);

virtual void OnSend(
					ICmTransport *aTrptId,
					CCmTransportParameter *aPara = NULL);

virtual void OnDisconnect(
						  CmResult aReason,
						  ICmTransport *aTrptId);

// ICmAcceptorConnectorSink
virtual void OnConnectIndication(
								 CmResult aReason,
								 ICmTransport *aTrpt,
								 ICmAcceptorConnectorId *aRequestId);*/




