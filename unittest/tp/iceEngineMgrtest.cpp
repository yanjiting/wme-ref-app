#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "iceEngineCheckMgr.h"
#include "iceEngineMgr.h"
#include "Mock_IIceEngineSink.h"

class CIceEngineMgrTest : public testing::Test
{
public:
	
	CIceEngineMgrTest():m_pIceEngineMgr(NULL)
	{
	}

	virtual ~CIceEngineMgrTest()
	{
	}


	virtual void SetUp()
	{
		m_pIceEngineMgr = new CIceEngineMgr();
	}

	virtual void TearDown()
	{
		delete m_pIceEngineMgr;
		m_pIceEngineMgr = NULL;
	}

protected:
	CIceEngineMgr *m_pIceEngineMgr;
};

TEST_F(CIceEngineMgrTest,Init)
{
	IIceEngineSink *pIceEngineSink = NULL;
	IceEngineInfo icerEngineInfo;
	bool bRet = m_pIceEngineMgr->Init(pIceEngineSink,icerEngineInfo);
	EXPECT_TRUE(bRet);
}


TEST_F(CIceEngineMgrTest,AddRemoteCandidates)
{
	bool bRet = m_pIceEngineMgr->AddRemoteCandidates(NULL, 0, true);
	EXPECT_FALSE(bRet);

	int nSize = 1;
	const IceEngineCandidate **ppCan = new const IceEngineCandidate*[nSize];

	bRet = m_pIceEngineMgr->AddRemoteCandidates(ppCan, 0,true);
	EXPECT_FALSE(bRet);

	for(int i = 0 ; i < nSize ; i++)
	{
		ppCan[i] = NULL;
	}
	bRet = m_pIceEngineMgr->AddRemoteCandidates(ppCan, nSize,true);
	EXPECT_FALSE(bRet);

	for(int i = 0 ; i < nSize; i++)
	{
		ppCan[i] =  new IceEngineCandidate();
	}

	bRet = m_pIceEngineMgr->AddRemoteCandidates(ppCan, nSize,false);
	EXPECT_TRUE(bRet);
	delete [] ppCan;
}


TEST_F(CIceEngineMgrTest,Start)
{
	int nSize = 1;
	const IceEngineCandidate **ppCan = new const IceEngineCandidate*[nSize];
	for(int i = 0 ; i < nSize; i++)
	{
		ppCan[i] =  new IceEngineCandidate();
	}
	m_pIceEngineMgr->AddRemoteCandidates(ppCan, nSize,true);
	bool bRet = m_pIceEngineMgr->Start();
	EXPECT_FALSE(bRet);

	bRet = m_pIceEngineMgr->Start();
	EXPECT_FALSE(bRet);
	delete [] ppCan;
}

TEST_F(CIceEngineMgrTest, Stop)
{
	//Do nothing now
}

TEST_F(CIceEngineMgrTest, Destroy)
{
	//Do nothing now
}