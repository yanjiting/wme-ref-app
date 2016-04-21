#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "iceEngineCheckMgr.h"
#include "iceEngineMgr.h"

class CIceEngineCheckMgrTest : public testing::Test, public CIceEngineCheckMgrSink
{
public:
	
	CIceEngineCheckMgrTest():m_pCheckMgr(NULL)
	{
	}

	virtual ~CIceEngineCheckMgrTest()
	{
	}

	virtual void OnCheckCompleted(bool bSuccess){};

	virtual bool OnValidTransport( const ICmTransport *pTransport, int iComponentId,eIceEngineTransType transType, eIceEngineType engineType, uint64_t transportPriority){return true;};

	virtual void OnDiscoverPeerRefOrTrigger(const CCmInetAddr &local, const CCmInetAddr &remote, ICmAcceptorConnectorId *aRequestId){};

	virtual void OnRoleChanged(eIceRole role){};
	

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
	CIceEngineCheckMgr *m_pCheckMgr;
	Candidates m_localCandidates;
	Candidates m_remoteCandidates;
	IceEngineCandidate *  m_pLocalCand;
};


TEST_F(CIceEngineCheckMgrTest, TestAddCandidatePair_invalidParameter)
{

	try
	{
		EXPECT_TRUE(m_pCheckMgr->AddCandidatePair(100,NULL,NULL,true) == false);

		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				EXPECT_TRUE(m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false) == true);
			}
		}
	}

	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}

}


TEST_F(CIceEngineCheckMgrTest, TestAddCandidatePair)
{

	try
	{
		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				EXPECT_TRUE(m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false) == true);
			}
		}
	}

	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}

}

TEST_F(CIceEngineCheckMgrTest, PairsCount)
{

	try
	{
		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				EXPECT_TRUE(m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false) == true);
			}
		}
		EXPECT_TRUE(m_pCheckMgr->PairsCount() == 4);
	}

	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}

}


TEST_F(CIceEngineCheckMgrTest, WaitingPairsCount)
{

	try
	{
		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				EXPECT_TRUE(m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false) == true);
			}
		}
		EXPECT_TRUE(m_pCheckMgr->WaitingPairsCount() == 0);
	}

	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}

}


TEST_F(CIceEngineCheckMgrTest, InProgressPairCount)
{
	try
	{
		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false);
			}
		}
		EXPECT_TRUE(m_pCheckMgr->InProgressPairCount() == 0);
	}

	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}
}



TEST_F(CIceEngineCheckMgrTest, WaitingPairsCount_start)
{

	try
	{
		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false);
			}
		}
		m_pCheckMgr->Start();
	}

	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}

}



TEST_F(CIceEngineCheckMgrTest, OnValidPair)
{
	try
	{
		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false);
			}
		}
		CIceEngineCheck * pCheck = m_pCheckMgr->GetEngineCheck(0);
		m_pCheckMgr->OnValidPair(pCheck);
	}

	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}
}


TEST_F(CIceEngineCheckMgrTest, OnValidPair_NULL)
{
	try
	{
		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false);
			}
		}
		m_pCheckMgr->OnValidPair(NULL);
	}

	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}
}

TEST_F(CIceEngineCheckMgrTest, EndCandidate)
{
	try
	{
		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false);
			}
		}
		m_pCheckMgr->EndCandidate();
	}

	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}
}


TEST_F(CIceEngineCheckMgrTest, Reset)
{
	try
	{
		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false);
			}
		}
		m_pCheckMgr->Reset();
	}

	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}
}

TEST_F(CIceEngineCheckMgrTest, SetWaitingPairs)
{
	try
	{
		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false);
			}
		}
		EXPECT_TRUE(m_pCheckMgr->SetWaitingPairs() >= 0);
	}

	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}
}


TEST_F(CIceEngineCheckMgrTest, OnCompleted)
{
	try
	{
		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false);
			}
		}
		CIceEngineCheck * pCheck = m_pCheckMgr->GetEngineCheck(0);
		ICmTransport* transport = NULL;
		if(pCheck != NULL)
		{
			 transport = pCheck->GetTransport();
		}
		m_pCheckMgr->OnCompleted(pCheck,transport,P2P_ICE_CHECK_STATE_WAITING,0,IE_TRANSPORT_TYPE_UDP, IE_ENGINE_RTP,100,0);
	}

	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}
}



TEST_F(CIceEngineCheckMgrTest, OnRoleChanged)
{
	try
	{
		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false);
			}
		}

		m_pCheckMgr->OnRoleChanged(ICE_ROLE_CONTROLLING);
	}

	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}
}

TEST_F(CIceEngineCheckMgrTest, OnTimer)
{
	try
	{
		for (Candidates::iterator it = m_remoteCandidates.begin(); it != m_remoteCandidates.end(); ++it)
		{
			if((*it) != NULL)
			{
				m_pCheckMgr->AddCandidatePair(100,m_pLocalCand,(*it),false);
			}
		}

		CCmTimerWrapperID timer;
		m_pCheckMgr->OnTimer(&timer);
	}

	catch (...)
	{
		EXPECT_TRUE(CM_OK == CM_ERROR_FAILURE);
	}
}



