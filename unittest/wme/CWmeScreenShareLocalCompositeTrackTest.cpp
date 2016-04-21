#include "CWmeScreenTrackTestBase.h"
 
#if defined(MACOS) || defined(WIN32)

//======================================================================
//CWmeSceeen_LocalCompositeScreenShareTrackTest
class CWmeSceeen_LocalCompositeScreenShareTrackTest : public CWmeScreenShareTrackTestBase
{
public:
	CWmeSceeen_LocalCompositeScreenShareTrackTest(){}
	virtual ~CWmeSceeen_LocalCompositeScreenShareTrackTest(){}


	void SetUp()
	{
		CWmeScreenShareTrackTestBase::SetUp();
		if(m_pEngine)
			m_pEngine->CreateLocalCompositeScreenShareTrack( &m_pWmeLocalCompositeScreenShareTrack);
	}
	void TearDown()
	{
		//
		SAFE_RELEASE(m_pWmeLocalCompositeScreenShareTrack);
		CWmeScreenShareTrackTestBase::TearDown();
	}

protected:
	wme::IWmeLocalCompositeScreenShareTrack *m_pWmeLocalCompositeScreenShareTrack;
};

TEST_F(CWmeSceeen_LocalCompositeScreenShareTrackTest, CreateLocalCompositeScreenShareTrack)
{
	wme::IWmeLocalCompositeScreenShareTrack *pWmeLocalCompositeScreenShareTrack = NULL;

	EXPECT_EQ(WME_S_OK, m_pEngine->CreateLocalCompositeScreenShareTrack( &pWmeLocalCompositeScreenShareTrack));
	ASSERT_TRUE(pWmeLocalCompositeScreenShareTrack!=NULL);

	wme::WmeTrackType eWmeTrackType = wme::WmeTrackType_Uknown;
	EXPECT_EQ(WME_S_OK, pWmeLocalCompositeScreenShareTrack->GetTrackType(eWmeTrackType));
	EXPECT_EQ(wme::WmeTrackType_Composite, eWmeTrackType);
	pWmeLocalCompositeScreenShareTrack->Release();

	EXPECT_NE(WME_S_OK, m_pEngine->CreateLocalCompositeScreenShareTrack( NULL));
}

TEST_F(CWmeSceeen_LocalCompositeScreenShareTrackTest, GetBasicTrackTypeNumber)
{
	ASSERT_TRUE(m_pWmeLocalCompositeScreenShareTrack!=NULL);
	int32_t nTypeNumber =0;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalCompositeScreenShareTrack->GetBasicTrackTypeNumber(nTypeNumber));
	EXPECT_EQ(2, nTypeNumber);
}

TEST_F(CWmeSceeen_LocalCompositeScreenShareTrackTest, GetBasicTrackType)
{
	ASSERT_TRUE(m_pWmeLocalCompositeScreenShareTrack!=NULL);
	wme::WmeTrackType eWmeTrackType = wme::WmeTrackType_Uknown;
	EXPECT_EQ(WME_S_OK, m_pWmeLocalCompositeScreenShareTrack->GetBasicTrackType(0,eWmeTrackType));
	EXPECT_EQ(wme::WmeTrackType_Video, eWmeTrackType);
	EXPECT_EQ(WME_S_OK, m_pWmeLocalCompositeScreenShareTrack->GetBasicTrackType(1,eWmeTrackType));
	EXPECT_EQ(wme::WmeTrackType_Data, eWmeTrackType);
}


//======================================================================
//CWmeScreen_RemoteCompositeScreenShareTrackTest
class CWmeScreen_RemoteCompositeScreenShareTrackTest : public CWmeScreenShareTrackTestBase
{
public:
	CWmeScreen_RemoteCompositeScreenShareTrackTest(){
        m_pWmeRemoteCompositeScreenShareTrack = NULL;
    }
	virtual ~CWmeScreen_RemoteCompositeScreenShareTrackTest(){}


	void SetUp()
	{
		CWmeScreenShareTrackTestBase::SetUp();
		if(m_pEngine)
			m_pEngine->CreateRemoteCompositeScreenShareTrack( &m_pWmeRemoteCompositeScreenShareTrack);
	}
	void TearDown()
	{
		//
		SAFE_RELEASE(m_pWmeRemoteCompositeScreenShareTrack);
		CWmeScreenShareTrackTestBase::TearDown();
	}

protected:
	wme::IWmeRemoteCompositeScreenShareTrack *m_pWmeRemoteCompositeScreenShareTrack;
};

TEST_F(CWmeScreen_RemoteCompositeScreenShareTrackTest, CreateRemoteCompositeScreenShareTrack)
{
	wme::IWmeRemoteCompositeScreenShareTrack *pWmeRemoteCompositeScreenShareTrack = NULL;

	EXPECT_EQ(WME_S_OK, m_pEngine->CreateRemoteCompositeScreenShareTrack( &pWmeRemoteCompositeScreenShareTrack));
	ASSERT_TRUE(pWmeRemoteCompositeScreenShareTrack!=NULL);

	wme::WmeTrackType eWmeTrackType = wme::WmeTrackType_Uknown;
	EXPECT_EQ(WME_S_OK, pWmeRemoteCompositeScreenShareTrack->GetTrackType(eWmeTrackType));
	EXPECT_EQ(wme::WmeTrackType_Composite, eWmeTrackType);
	pWmeRemoteCompositeScreenShareTrack->Release();

	EXPECT_NE(WME_S_OK, m_pEngine->CreateRemoteCompositeScreenShareTrack( NULL));
}

TEST_F(CWmeScreen_RemoteCompositeScreenShareTrackTest, GetBasicTrackTypeNumber)
{
	ASSERT_TRUE(m_pWmeRemoteCompositeScreenShareTrack!=NULL);
	int32_t nTypeNumber =0;
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteCompositeScreenShareTrack->GetBasicTrackTypeNumber(nTypeNumber));
	EXPECT_EQ(2, nTypeNumber);
}

TEST_F(CWmeScreen_RemoteCompositeScreenShareTrackTest, GetBasicTrackType)
{
	ASSERT_TRUE(m_pWmeRemoteCompositeScreenShareTrack!=NULL);
	wme::WmeTrackType eWmeTrackType = wme::WmeTrackType_Uknown;
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteCompositeScreenShareTrack->GetBasicTrackType(0,eWmeTrackType));
	EXPECT_EQ(wme::WmeTrackType_Video, eWmeTrackType);
	EXPECT_EQ(WME_S_OK, m_pWmeRemoteCompositeScreenShareTrack->GetBasicTrackType(1,eWmeTrackType));
	EXPECT_EQ(wme::WmeTrackType_Data, eWmeTrackType);
}

#endif //defined(MACOS) || defined(WIN32)