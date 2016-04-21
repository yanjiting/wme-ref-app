/*****************************************************************************************
*Generate by AUT Tool on 2011-01-06
*class:CWbxAJBPolicyTest
********************************************************************************************/
#include "gtest/gtest.h"
#include "gmock/gmock.h"
//#include "wbxmock.h"

#include "WbxAeAJBPolicy.h"
#include "WbxAeJitterPolicy.h"
#include "WseRtpPacket.h"
#define private public
#define protected public
using testing::Return;
using testing::_;
using testing::Invoke;
using testing::ReturnRef;
using testing::Matcher;

class CWbxAJBPolicyTest : public testing::Test
{
public:
	CWbxAJBPolicyTest()
	{
	}

	virtual ~CWbxAJBPolicyTest()
	{
	}

	virtual void SetUp()
	{
		m_pAJBPolicy= new CWbxAJBPolicy();
	}

	virtual void TearDown()
	{
		delete m_pAJBPolicy;

	}

protected:
	CWbxAJBPolicy * m_pAJBPolicy;
};

TEST_F(CWbxAJBPolicyTest, CreateDestroy)
{
	IWbxJitterPolicy* pPolicy;
	EXPECT_NE(WBXAE_SUCCESS, CreateIWbxJitterPolicy(NULL));
	EXPECT_NE(WBXAE_SUCCESS, DestoryIWbxJitterPolicy(NULL));
	EXPECT_EQ(WBXAE_SUCCESS, CreateIWbxJitterPolicy(&pPolicy));
	EXPECT_EQ(WBXAE_SUCCESS, DestoryIWbxJitterPolicy(pPolicy));
}
TEST_F(CWbxAJBPolicyTest, JitterPolicy)
{

	int ssrc = 99999;

	unsigned int playtime = 0 ;
	for ( int i =0 ; i < 100; i ++)
	{
		if ((i + 1)% 5 == 0)
		{

			EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->JitterPolicy(ssrc, 80, (i + 10 ) * 20, i + 1 , 160* (i + 1), playtime));
			EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->JitterPolicy(ssrc,80, (i + 1 )* 20 + 10, i, 160* i, playtime));
			i++;
		}
		else
		{
			EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->JitterPolicy(ssrc,80, i * 20, i, 160* i, playtime));
		}

	}

	for (int i = 101; i < 200; i ++)
	{
		EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->JitterPolicy(ssrc,80, i * 20, i, 160* i, playtime));

	}

	for ( int i =201 ; i < 500; i ++)
	{

		if ((i + 1)% 5 == 0)
		{

			EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->JitterPolicy(ssrc,80, (i + 10 ) * 20, i + 1 , 160* (i + 1), playtime));
			EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->JitterPolicy(ssrc,80, (i + 1 )* 20 + 10, i, 160* i, playtime));
			i++;
		}
		else
		{
			EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->JitterPolicy(ssrc,80, i * 20, i, 160* i, playtime));
		}

	}
	EXPECT_NE(WBXAE_SUCCESS, m_pAJBPolicy->JitterPolicy(ssrc,0, 20, 1, 160, playtime));

	EXPECT_NE(WBXAE_SUCCESS, m_pAJBPolicy->JitterPolicy(ssrc,800, 20, 1, 160, playtime));
}

TEST_F(CWbxAJBPolicyTest, GetJitterInformation)
{
	WbxAEAJBStatistics jitterStastics;
	int ssrc  = 9999;
	unsigned int playtime = 0 ;
	for ( int i =0 ; i < 100; i ++)
	{
		if ((i + 1)% 5 == 0)
		{
			EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->JitterPolicy(ssrc,80, (i + 1) * 20, i + 1, 160*( i + 1) , playtime));
			EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->JitterPolicy(ssrc,80, i * 20, i, 160* i, playtime));
			i++;
		}
		else
		{
			EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->JitterPolicy(ssrc,80, i * 20, i, 160* i, playtime));
		
		}

	}
	EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->JitterPolicy(ssrc,80, 103 * 20,103, 160* 103, playtime));


	m_pAJBPolicy->GetJitterInformation(jitterStastics);
	m_pAJBPolicy->Reset();
	m_pAJBPolicy->GetJitterInformation(jitterStastics);

}

TEST_F(CWbxAJBPolicyTest, SetDelay)
{
	EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->SetDelay(0, 0));
	EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->SetDelay(0, 200));
	EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->SetDelay(-1, 200));
	EXPECT_EQ(WBXAE_SUCCESS, m_pAJBPolicy->SetDelay(200, 1000));
	EXPECT_NE(WBXAE_SUCCESS, m_pAJBPolicy->SetDelay(100, 20000));

	EXPECT_NE(WBXAE_SUCCESS, m_pAJBPolicy->SetDelay(20000, 20000));
}

TEST_F(CWbxAJBPolicyTest, Reset)
{
	m_pAJBPolicy->Reset();
}
