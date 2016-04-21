#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
#include "WmeError.h"
#include "WmeTrack.h"
#include "WmeSyncBox.h"


using namespace wme;

class CWmeMediaSyncBoxTest : public testing::Test
{
public:
	CWmeMediaSyncBoxTest()
	{
		WmeCreateMediaEngine(&m_pWmeEngine);

		if (m_pWmeEngine)
		{
			m_pWmeEngine->CreateMediaSyncBox(&m_pWmeSyncBox);
		}
	}

	virtual ~CWmeMediaSyncBoxTest()
	{
		if (m_pWmeSyncBox)
		{
			m_pWmeSyncBox->Release();
		}
		if (m_pWmeEngine)
		{
			m_pWmeEngine->Release();
		}
	}

	virtual void SetUp() {}
	virtual void TearDown() {}
public:
	//member
	IWmeMediaEngine* m_pWmeEngine;
	IWmeMediaSyncBox* m_pWmeSyncBox;
};

TEST_F(CWmeMediaSyncBoxTest, AboutLabel)
{
	EXPECT_NE(NULL, (long)m_pWmeSyncBox);
	uint32_t label = 123;

	EXPECT_EQ(WME_S_OK, m_pWmeSyncBox->SetLabel(label));
	EXPECT_EQ(WME_S_OK, m_pWmeSyncBox->GetLabel(label));
}

TEST_F(CWmeMediaSyncBoxTest, AboutAddRemoveFind)
{
	EXPECT_NE(NULL, (long)m_pWmeSyncBox);

	IWmeLocalVideoTrack* pV_1 = NULL;

#ifdef IOS_SIMULATOR
	EXPECT_NE(WME_S_OK, m_pWmeEngine->CreateLocalVideoTrack(& pV_1));
#else
	EXPECT_EQ(WME_S_OK, m_pWmeEngine->CreateLocalVideoTrack(& pV_1));
#endif
    if (pV_1) {
        pV_1->SetTrackLabel(1);
    }

	IWmeLocalAudioTrack* pA_1 = NULL;
	EXPECT_EQ(WME_S_OK, m_pWmeEngine->CreateLocalAudioTrack(& pA_1));
    if (pA_1) {
        pA_1->SetTrackLabel(2);
    }

	IWmeRemoteVideoTrack* pV_2 = NULL;
	EXPECT_EQ(WME_S_OK, m_pWmeEngine->CreateRemoteVideoTrack(& pV_2));
    if (pV_2) {
        pV_2->SetTrackLabel(5);
    }

	IWmeRemoteAudioTrack* pA_2 = NULL;
	EXPECT_EQ(WME_S_OK, m_pWmeEngine->CreateRemoteAudioTrack(& pA_2));
    if (pA_2) {
        pA_2->SetTrackLabel(6);
    }

	/*bool found = false;
	IWmeMediaTrack* pT = NULL;*/

	EXPECT_NE(WME_S_OK, m_pWmeSyncBox->AddTrack(pV_1));
	EXPECT_EQ(WME_S_OK, m_pWmeSyncBox->AddTrack(pV_2));
	/*EXPECT_EQ(WME_S_OK, m_pWmeSyncBox->FindTrack(&found, &pT, 1, WmeTrackType_Video));
	EXPECT_EQ(true, found);*/

	
	/*EXPECT_EQ(WME_S_OK, m_pWmeSyncBox->FindTrack(&found, &pT, 2, WmeTrackType_Audio));
	EXPECT_EQ(false, found);*/
	EXPECT_NE(WME_S_OK, m_pWmeSyncBox->AddTrack(pA_1));
	EXPECT_EQ(WME_S_OK, m_pWmeSyncBox->AddTrack(pA_2));
	/*EXPECT_EQ(WME_S_OK, m_pWmeSyncBox->FindTrack(&found, &pT, 2, WmeTrackType_Audio));
	EXPECT_EQ(true, found);*/

	EXPECT_NE(WME_S_OK, m_pWmeSyncBox->RemoveTrack(pA_1));
	EXPECT_EQ(WME_S_OK, m_pWmeSyncBox->RemoveTrack(pA_2));
	EXPECT_NE(WME_S_OK, m_pWmeSyncBox->RemoveTrack(pV_1));
	EXPECT_EQ(WME_S_OK, m_pWmeSyncBox->RemoveTrack(pV_2));


	if (pV_1)
	{
		pV_1->Release();
	}
	if (pV_2)
	{
		pV_2->Release();
	}
	if (pA_1)
	{
		pA_1->Release();
	}
	if (pA_2)
	{
		pA_2->Release();
	}
}

TEST_F(CWmeMediaSyncBoxTest, GetStatistics)
{
    EXPECT_NE(NULL, (long)m_pWmeSyncBox);
    
    WmeSyncStatistics stSyncStatistics;
    EXPECT_EQ(WME_S_OK, m_pWmeSyncBox->GetStatistics(stSyncStatistics));
}
