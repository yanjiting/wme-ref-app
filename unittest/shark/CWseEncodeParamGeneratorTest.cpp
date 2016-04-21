#include <time.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WseEncodeParamGenerator.h"

using namespace shark;

class CWseEncodeParamGeneratorTest : public testing::Test
{
public:
	virtual void SetUp()
	{
	}

	virtual void TearDown()
	{
		delete pEncodeParamGenerator;
	}

public:
	CWseBaseEncodeParamGenerator* pEncodeParamGenerator;
    bool TestOneKindSubscription(const WseVideoSubscribeMultiStream* pSubscribedStr);
};

bool TestOneSetting(CWseBaseEncodeParamGenerator* pEncodeParamGenerator, WseVideoEncoderCapability& sVideoEncoderCapability, int& iMaxLevel)
{
  const int kLevel10MBPS = 1485;
  const int kLevel10FS = 99;
  const int kLevel10BR = 64000;

	int iReturn = pEncodeParamGenerator->SetMaxVideoEncodeCapability(&sVideoEncoderCapability);
	EXPECT_TRUE(WSE_S_OK == iReturn);

	//check generated param
	CWseEncodeParam param;
	int iAvailableModeNum = pEncodeParamGenerator->GetModeNum();
	for (int i=0; i<iAvailableModeNum; i++)
	{
		int iCurrentMax = pEncodeParamGenerator->GetMaxLevel(i);
		int iCurrentMin = pEncodeParamGenerator->GetMinLevel(i);
		EXPECT_TRUE(iCurrentMax>0);
		//loop over all the setting to make sure there is no exceeding
		for (int k=iCurrentMax;k>=iCurrentMin;k--)
		{
			pEncodeParamGenerator->GetEncodeParam(&param,k,LAYSELECT_TOP);
			unsigned int uMBWidth  = pEncodeParamGenerator->GetMBSize(param.encode_width[i]);
			unsigned int uMBHeight =  pEncodeParamGenerator->GetMBSize(param.encode_height[i]);
			EXPECT_TRUE( WSE_MAX(sVideoEncoderCapability.uMaxMBPS,kLevel10MBPS) >= uMBWidth*uMBHeight*param.encode_frame_rate[i] )
            <<"uMaxMBPS = "<<sVideoEncoderCapability.uMaxMBPS
            <<", uMBWidth = "<<uMBWidth
            <<", uMBHeight = "<<uMBHeight
            <<", encode_frame_rate[i] = "<<param.encode_frame_rate[i];
			EXPECT_TRUE( WSE_MAX(sVideoEncoderCapability.uMaxFS,kLevel10FS) >= uMBWidth*uMBHeight )
            <<"uMaxFS = "<<sVideoEncoderCapability.uMaxFS
            <<", uMBWidth = "<<uMBWidth
            <<", uMBHeight = "<<uMBHeight;
			EXPECT_TRUE( WSE_MAX(sVideoEncoderCapability.uMaxBitRate,kLevel10BR) >= param.max_target_bitrate[i] )
            <<"uMaxBitRate = "<<sVideoEncoderCapability.uMaxBitRate
            <<", max_target_bitrate[i] = "<<param.max_target_bitrate[i];
		}
		if (iMaxLevel<iCurrentMax)
			iMaxLevel = iCurrentMax;
	}
	return true;
}

TEST_F(CWseEncodeParamGeneratorTest, GenerateParamList_SVC)
{
	//create
	pEncodeParamGenerator = new CWseEncodeParamSVC();
	ASSERT_TRUE(pEncodeParamGenerator!=NULL);

	//set constraint
	WseVideoEncoderCapability sVideoEncoderCapability;
	srand((unsigned int)time(NULL));
	unsigned int c_uLevelIdx = rand()%H264_LEVEL_NUMBER;
	unsigned int c_uLevelIdc = aH264LevelLimits[c_uLevelIdx].uLevelIdc;
	sVideoEncoderCapability.uProfileLevelID = 0x42E000 + (c_uLevelIdc&0x00FF);
	sVideoEncoderCapability.uMaxBitRate = WSE_MAX(rand(), aH264LevelLimits[c_uLevelIdx].uMaxBR);
	sVideoEncoderCapability.uMaxFPS = (rand()%60);
	sVideoEncoderCapability.uMaxFS = WSE_MAX(rand(), aH264LevelLimits[c_uLevelIdx].uMaxFS);
	sVideoEncoderCapability.uMaxMBPS = WSE_MAX(rand(), aH264LevelLimits[c_uLevelIdx].uMaxMBPS);
	sVideoEncoderCapability.uMaxNalUnitSize = rand();

	//launch test
	int iMaxLevel = 0;
	EXPECT_TRUE (TestOneSetting(pEncodeParamGenerator, sVideoEncoderCapability, iMaxLevel));
}

TEST_F(CWseEncodeParamGeneratorTest, GenerateParamList_SVC_ThreeTimesNShape)
{
	//create
	pEncodeParamGenerator = new CWseEncodeParamSVC();
	ASSERT_TRUE(pEncodeParamGenerator!=NULL);	
	int iMaxLevel = 0, iOldMaxLevel=0;
	
	WseVideoEncoderCapability sVideoEncoderCapability;
	srand((unsigned int)time(NULL));
	unsigned int c_uLevelIdx = rand()%(H264_LEVEL_NUMBER/2);
	unsigned int uLevelIdc = aH264LevelLimits[c_uLevelIdx].uLevelIdc;
	sVideoEncoderCapability.uMaxBitRate = WSE_MAX(rand(), aH264LevelLimits[c_uLevelIdx].uMaxBR);
	sVideoEncoderCapability.uMaxFPS = (rand()%60);
	sVideoEncoderCapability.uMaxFS = WSE_MAX(rand(), aH264LevelLimits[c_uLevelIdx].uMaxFS);
	sVideoEncoderCapability.uMaxMBPS = WSE_MAX(rand(), aH264LevelLimits[c_uLevelIdx].uMaxMBPS);
	sVideoEncoderCapability.uMaxNalUnitSize = rand();
	//set constraint
	sVideoEncoderCapability.uProfileLevelID = 0x42E000 + (uLevelIdc&0x00FF);
	//launch test
	EXPECT_TRUE (TestOneSetting(pEncodeParamGenerator, sVideoEncoderCapability, iMaxLevel));
	iOldMaxLevel = iMaxLevel;
	
	//set constraint
	c_uLevelIdx = (rand()%(H264_LEVEL_NUMBER/2) + c_uLevelIdx)%H264_LEVEL_NUMBER;
	uLevelIdc = aH264LevelLimits[c_uLevelIdx].uLevelIdc;
	sVideoEncoderCapability.uProfileLevelID = 0x42E000 + (uLevelIdc&0x00FF);
	//launch test
	EXPECT_TRUE (TestOneSetting(pEncodeParamGenerator, sVideoEncoderCapability, iMaxLevel));
	EXPECT_TRUE(iOldMaxLevel<=iMaxLevel);
	iOldMaxLevel = iMaxLevel;

	//set constraint
	c_uLevelIdx = (c_uLevelIdx/2)%H264_LEVEL_NUMBER;
	uLevelIdc = aH264LevelLimits[c_uLevelIdx].uLevelIdc;
	sVideoEncoderCapability.uProfileLevelID = 0x42E000 + (uLevelIdc&0x00FF);
	//launch test
	EXPECT_TRUE (TestOneSetting(pEncodeParamGenerator, sVideoEncoderCapability, iMaxLevel));	
	EXPECT_TRUE(iOldMaxLevel>=iMaxLevel);
}

TEST_F(CWseEncodeParamGeneratorTest, GenerateParamList_SVC_ThreeTimesVShape)
{
	//create
	pEncodeParamGenerator = new CWseEncodeParamSVC();
	ASSERT_TRUE(pEncodeParamGenerator!=NULL);	
	int iMaxLevel = 0, iOldMaxLevel=0;

	WseVideoEncoderCapability sVideoEncoderCapability;
	srand((unsigned int)time(NULL));
	unsigned int c_uLevelIdx = (rand()%(H264_LEVEL_NUMBER/2) + H264_LEVEL_NUMBER/2)%H264_LEVEL_NUMBER;
	unsigned int uLevelIdc = aH264LevelLimits[c_uLevelIdx].uLevelIdc;
	sVideoEncoderCapability.uMaxBitRate = WSE_MAX(rand(), aH264LevelLimits[c_uLevelIdx].uMaxBR);
	sVideoEncoderCapability.uMaxFPS = (rand()%60);
	sVideoEncoderCapability.uMaxFS = WSE_MAX(rand(), aH264LevelLimits[c_uLevelIdx].uMaxFS);
	sVideoEncoderCapability.uMaxMBPS = WSE_MAX(rand(), aH264LevelLimits[c_uLevelIdx].uMaxMBPS);
	sVideoEncoderCapability.uMaxNalUnitSize = rand();
	//set constraint
	sVideoEncoderCapability.uProfileLevelID = 0x42E000 + (uLevelIdc&0x00FF);
	//launch test
	EXPECT_TRUE (TestOneSetting(pEncodeParamGenerator, sVideoEncoderCapability, iMaxLevel));	
	iOldMaxLevel = iMaxLevel;

	//set constraint
	c_uLevelIdx = rand()%(H264_LEVEL_NUMBER/2) ;
	uLevelIdc = aH264LevelLimits[c_uLevelIdx].uLevelIdc;
	sVideoEncoderCapability.uProfileLevelID = 0x42E000 + (uLevelIdc&0x00FF);
	//launch test
	EXPECT_TRUE (TestOneSetting(pEncodeParamGenerator, sVideoEncoderCapability, iMaxLevel));
	EXPECT_TRUE(iOldMaxLevel>=iMaxLevel);
	iOldMaxLevel = iMaxLevel;

	//set constraint
	c_uLevelIdx = ((c_uLevelIdx + H264_LEVEL_NUMBER)/2)%H264_LEVEL_NUMBER;
	uLevelIdc = aH264LevelLimits[c_uLevelIdx].uLevelIdc;
	sVideoEncoderCapability.uProfileLevelID = 0x42E000 + (uLevelIdc&0x00FF);
	//launch test
	EXPECT_TRUE (TestOneSetting(pEncodeParamGenerator, sVideoEncoderCapability, iMaxLevel));
	EXPECT_TRUE(iOldMaxLevel<=iMaxLevel);
}

TEST_F(CWseEncodeParamGeneratorTest, SetMaxVideoEncodeCapability)
{
	//create
	pEncodeParamGenerator = new CWseEncodeParamSVC();
	ASSERT_TRUE(pEncodeParamGenerator!=NULL);
	WseVideoEncoderCapability sVideoEncoderCapability;
	long iReturn;

	//level check
	sVideoEncoderCapability.uProfileLevelID = 0x42E000 + 0x000F;
	iReturn = pEncodeParamGenerator->SetMaxVideoEncodeCapability(&sVideoEncoderCapability);
	EXPECT_TRUE(WSE_E_FAIL == iReturn);

	//profile check
	sVideoEncoderCapability.uProfileLevelID = 0x4EE000 + 0x000A;
	iReturn = pEncodeParamGenerator->SetMaxVideoEncodeCapability(&sVideoEncoderCapability);
	EXPECT_TRUE(WSE_E_FAIL == iReturn);

	//profile/level check
	sVideoEncoderCapability.uProfileLevelID = 0x4EE000 + 0x0030;
	iReturn = pEncodeParamGenerator->SetMaxVideoEncodeCapability(&sVideoEncoderCapability);
	EXPECT_TRUE(WSE_E_FAIL == iReturn);

	//profile/level check
	sVideoEncoderCapability.uProfileLevelID = 0x64E000 + 0x000A;
	iReturn = pEncodeParamGenerator->SetMaxVideoEncodeCapability(&sVideoEncoderCapability);
	EXPECT_TRUE(WSE_S_OK == iReturn);
}
bool  CWseEncodeParamGeneratorTest::TestOneKindSubscription(const WseVideoSubscribeMultiStream* pSubscribedStr)
{
    unsigned int nNumberOfSubscription = pSubscribedStr->uNumStreams;
    EXPECT_TRUE(nNumberOfSubscription<=4 && nNumberOfSubscription>=1);
    //create
	pEncodeParamGenerator = new CWseEncodeParamMultStream();
	EXPECT_TRUE(pEncodeParamGenerator!=NULL);
    
//    long iRet = pEncodeParamGenerator->SetSubscription(nNumberOfSubscription, subscription);
    long iRet = pEncodeParamGenerator->SetSubscribedStreams(pSubscribedStr);
    EXPECT_TRUE(WSE_S_OK == iRet);
    
    EXPECT_TRUE(pEncodeParamGenerator->GetTotalLevelNum()>0);
    

    CWseEncodeParam paramCur;
    CWseEncodeParam paramPrev;
    int nTotalBrCur=0;
    int nTotalBrPrev=0;
    pEncodeParamGenerator->GetEncodeParam(&paramPrev, 0, LAYSELECT_ALL);
    for (int j=0; j<paramPrev.mode_num; j++) {
        nTotalBrPrev += paramPrev.max_target_bitrate[j];
    }
    EXPECT_TRUE(nTotalBrPrev>0);
    
	for (int i=1; i<pEncodeParamGenerator->GetTotalLevelNum(); i++)
    {
		//loop over all the setting to make sure there is no exceeding
        pEncodeParamGenerator->GetEncodeParam(&paramCur,i,LAYSELECT_ALL);
        
        nTotalBrCur = 0;
		for (int j=0; j<paramCur.mode_num; j++) {
            EXPECT_TRUE(paramCur.encode_frame_rate[j]>=14.9f);
            if (pSubscribedStr->stStream[j].uMaxFS) {
                EXPECT_TRUE(paramCur.encode_width[j]*paramCur.encode_height[j]/256<=pSubscribedStr->stStream[j].uMaxFS)<<" "<<i;
            }
            if (pSubscribedStr->stStream[j].uMaxFPS) {
                EXPECT_TRUE(paramCur.encode_frame_rate[j]<=pSubscribedStr->stStream[j].uMaxFPS);
            }
            if (pSubscribedStr->stStream[j].uMaxBitRate) {
                EXPECT_TRUE(paramCur.max_target_bitrate[j]<=pSubscribedStr->stStream[j].uMaxBitRate);
            }
            nTotalBrCur += paramCur.max_target_bitrate[j];
        }
        EXPECT_TRUE(nTotalBrCur>0);
        EXPECT_TRUE(nTotalBrCur>=nTotalBrPrev);
        nTotalBrPrev = nTotalBrCur;
        
        paramPrev = paramCur;
    }

    EXPECT_TRUE(pEncodeParamGenerator->GetDefaultLevel(0)>=0);
    for (int i=1; i<pEncodeParamGenerator->GetModeNum(); i++) {
        EXPECT_TRUE(pEncodeParamGenerator->GetDefaultLevel(i-1) <= pEncodeParamGenerator->GetDefaultLevel(i));
    }
    
    return true;
}
TEST_F(CWseEncodeParamGeneratorTest, GenerateParamList_MultiStream_Four)
{
//    WseVideoSubscription subscription[4] = {{videoCodecH264SVC, 1280, 720, 30, 0}, {videoCodecH264SVC, 640, 360, 30, 0}, {videoCodecH264SVC, 160, 90, 30, 0}, {videoCodecH264SVC, 320, 180, 30, 0}};
//    unsigned int nNumberOfSubscription = 4;
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 4;

    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x000a;
    sSubscribedStr.stStream[0].uMaxMBPS = 1800;
    sSubscribedStr.stStream[0].uMaxFS = 99;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 396;
    sSubscribedStr.stStream[0].uMaxBitRate = 64000;
    
    sSubscribedStr.stStream[1].uProfileLevelID =  0x42E000 + 0x000c;
    sSubscribedStr.stStream[1].uMaxMBPS = 7200;
    sSubscribedStr.stStream[1].uMaxFS = 396;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 2376;
    sSubscribedStr.stStream[1].uMaxBitRate = 200000;
    
    sSubscribedStr.stStream[2].uProfileLevelID =  0x42E000 + 0x0016;
    sSubscribedStr.stStream[2].uMaxMBPS = 27600;
    sSubscribedStr.stStream[2].uMaxFS = 1620;
    sSubscribedStr.stStream[2].uMaxFPS = 30;
    sSubscribedStr.stStream[2].uMaxDPB = 8100;
    sSubscribedStr.stStream[2].uMaxBitRate = 700000;
    
    sSubscribedStr.stStream[3].uProfileLevelID =  0x42E000 + 0x0020;
    sSubscribedStr.stStream[3].uMaxMBPS = 216000;
    sSubscribedStr.stStream[3].uMaxFS = 3600;
    sSubscribedStr.stStream[3].uMaxFPS = 30;
    sSubscribedStr.stStream[3].uMaxDPB = 20480;
    sSubscribedStr.stStream[3].uMaxBitRate = 2000000;

    
    EXPECT_TRUE(TestOneKindSubscription(&sSubscribedStr));
}
TEST_F(CWseEncodeParamGeneratorTest, GenerateParamList_MultiStream_FPS_Min15)
{
    //    WseVideoSubscription subscription[4] = {{videoCodecH264SVC, 1280, 720, 30, 0}, {videoCodecH264SVC, 640, 360, 30, 0}, {videoCodecH264SVC, 160, 90, 30, 0}, {videoCodecH264SVC, 320, 180, 30, 0}};
    //    unsigned int nNumberOfSubscription = 4;
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 4;
    
    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x000a;
    sSubscribedStr.stStream[0].uMaxMBPS = 1800;
    sSubscribedStr.stStream[0].uMaxFS = 99;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 396;
    sSubscribedStr.stStream[0].uMaxBitRate = 64000;
    
    sSubscribedStr.stStream[1].uProfileLevelID =  0x42E000 + 0x000c;
    sSubscribedStr.stStream[1].uMaxMBPS = 7200;
    sSubscribedStr.stStream[1].uMaxFS = 396;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 2376;
    sSubscribedStr.stStream[1].uMaxBitRate = 200000;
    
    sSubscribedStr.stStream[2].uProfileLevelID =  0x42E000 + 0x0016;
    sSubscribedStr.stStream[2].uMaxMBPS = 27600;
    sSubscribedStr.stStream[2].uMaxFS = 1620;
    sSubscribedStr.stStream[2].uMaxFPS = 30;
    sSubscribedStr.stStream[2].uMaxDPB = 8100;
    sSubscribedStr.stStream[2].uMaxBitRate = 700000;
    
    sSubscribedStr.stStream[3].uProfileLevelID =  0x42E000 + 0x0020;
    sSubscribedStr.stStream[3].uMaxMBPS = 216000;
    sSubscribedStr.stStream[3].uMaxFS = 3600;
    sSubscribedStr.stStream[3].uMaxFPS = 30;
    sSubscribedStr.stStream[3].uMaxDPB = 20480;
    sSubscribedStr.stStream[3].uMaxBitRate = 2000000;
    
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxFPS = 15;
    sSubscribedStr.stStream[2].uMaxFPS = 30;
    sSubscribedStr.stStream[3].uMaxFPS = 22;
    
    WseVideoSubscribeMultiStream *pSubscribedStr = &sSubscribedStr;
    unsigned int nNumberOfSubscription = pSubscribedStr->uNumStreams;
    EXPECT_TRUE(nNumberOfSubscription<=4 && nNumberOfSubscription>=1);
    //create
    pEncodeParamGenerator = new CWseEncodeParamMultStream();
    EXPECT_TRUE(pEncodeParamGenerator!=NULL);
    
    long iRet = pEncodeParamGenerator->SetSubscribedStreams(pSubscribedStr);
    EXPECT_TRUE(WSE_S_OK == iRet);
    
    EXPECT_TRUE(pEncodeParamGenerator->GetTotalLevelNum()>0);
    
    
    CWseEncodeParam paramCur;
    CWseEncodeParam paramPrev;
    int nTotalBrCur=0;
    int nTotalBrPrev=0;
    pEncodeParamGenerator->GetEncodeParam(&paramPrev, 0, LAYSELECT_ALL);
    for (int j=0; j<paramPrev.mode_num; j++) {
        nTotalBrPrev += paramPrev.max_target_bitrate[j];
    }
    EXPECT_TRUE(nTotalBrPrev>0);
    
    for (int i=1; i<pEncodeParamGenerator->GetTotalLevelNum(); i++)
    {
        //loop over all the setting to make sure there is no exceeding
        pEncodeParamGenerator->GetEncodeParam(&paramCur,i,LAYSELECT_ALL);
        
        nTotalBrCur = 0;
        for (int j=0; j<paramCur.mode_num; j++) {
            EXPECT_TRUE(paramCur.encode_frame_rate[j]>=14.9f);
            if (pSubscribedStr->stStream[j].uMaxFS) {
                EXPECT_TRUE(paramCur.encode_width[j]*paramCur.encode_height[j]/256<=pSubscribedStr->stStream[j].uMaxFS);
            }
            if (paramCur.mode_num==4) {
                EXPECT_TRUE(static_cast<unsigned int>(paramCur.encode_frame_rate[0]+0.5)==30);
                EXPECT_TRUE(static_cast<unsigned int>(paramCur.encode_frame_rate[1]+0.5)==15);
                EXPECT_TRUE(static_cast<unsigned int>(paramCur.encode_frame_rate[2]+0.5)==30);
                EXPECT_TRUE(static_cast<unsigned int>(paramCur.encode_frame_rate[3]+0.5)==15);
            }
            EXPECT_TRUE(paramCur.encode_frame_rate[j]<=pSubscribedStr->stStream[j].uMaxFPS);
            
            if (pSubscribedStr->stStream[j].uMaxBitRate) {
                EXPECT_TRUE(paramCur.max_target_bitrate[j]<=pSubscribedStr->stStream[j].uMaxBitRate);
            }
            nTotalBrCur += paramCur.max_target_bitrate[j];
        }

        EXPECT_TRUE(nTotalBrCur>0);
        EXPECT_TRUE(nTotalBrCur>=nTotalBrPrev);
        nTotalBrPrev = nTotalBrCur;
        
        paramPrev = paramCur;
    }
}
TEST_F(CWseEncodeParamGeneratorTest, GenerateParamList_MultiStream_FPS_GT15)
{
    //    WseVideoSubscription subscription[4] = {{videoCodecH264SVC, 1280, 720, 30, 0}, {videoCodecH264SVC, 640, 360, 30, 0}, {videoCodecH264SVC, 160, 90, 30, 0}, {videoCodecH264SVC, 320, 180, 30, 0}};
    //    unsigned int nNumberOfSubscription = 4;
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 4;
    
    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x000a;
    sSubscribedStr.stStream[0].uMaxMBPS = 1800;
    sSubscribedStr.stStream[0].uMaxFS = 99;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 396;
    sSubscribedStr.stStream[0].uMaxBitRate = 64000;
    
    sSubscribedStr.stStream[1].uProfileLevelID =  0x42E000 + 0x000c;
    sSubscribedStr.stStream[1].uMaxMBPS = 7200;
    sSubscribedStr.stStream[1].uMaxFS = 396;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 2376;
    sSubscribedStr.stStream[1].uMaxBitRate = 200000;
    
    sSubscribedStr.stStream[2].uProfileLevelID =  0x42E000 + 0x0016;
    sSubscribedStr.stStream[2].uMaxMBPS = 27600;
    sSubscribedStr.stStream[2].uMaxFS = 1620;
    sSubscribedStr.stStream[2].uMaxFPS = 30;
    sSubscribedStr.stStream[2].uMaxDPB = 8100;
    sSubscribedStr.stStream[2].uMaxBitRate = 700000;
    
    sSubscribedStr.stStream[3].uProfileLevelID =  0x42E000 + 0x0020;
    sSubscribedStr.stStream[3].uMaxMBPS = 216000;
    sSubscribedStr.stStream[3].uMaxFS = 3600;
    sSubscribedStr.stStream[3].uMaxFPS = 30;
    sSubscribedStr.stStream[3].uMaxDPB = 20480;
    sSubscribedStr.stStream[3].uMaxBitRate = 2000000;
    
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxFPS = 24;
    sSubscribedStr.stStream[2].uMaxFPS = 30;
    sSubscribedStr.stStream[3].uMaxFPS = 22;
    
    WseVideoSubscribeMultiStream *pSubscribedStr = &sSubscribedStr;
    unsigned int nNumberOfSubscription = pSubscribedStr->uNumStreams;
    EXPECT_TRUE(nNumberOfSubscription<=4 && nNumberOfSubscription>=1);
    //create
    pEncodeParamGenerator = new CWseEncodeParamMultStream();
    EXPECT_TRUE(pEncodeParamGenerator!=NULL);
    
    long iRet = pEncodeParamGenerator->SetSubscribedStreams(pSubscribedStr);
    EXPECT_TRUE(WSE_S_OK == iRet);
    
    EXPECT_TRUE(pEncodeParamGenerator->GetTotalLevelNum()>0);
    
    
    CWseEncodeParam paramCur;
    CWseEncodeParam paramPrev;
    int nTotalBrCur=0;
    int nTotalBrPrev=0;
    pEncodeParamGenerator->GetEncodeParam(&paramPrev, 0, LAYSELECT_ALL);
    for (int j=0; j<paramPrev.mode_num; j++) {
        nTotalBrPrev += paramPrev.max_target_bitrate[j];
    }
    EXPECT_TRUE(nTotalBrPrev>0);
    
    for (int i=1; i<pEncodeParamGenerator->GetTotalLevelNum(); i++)
    {
        //loop over all the setting to make sure there is no exceeding
        pEncodeParamGenerator->GetEncodeParam(&paramCur,i,LAYSELECT_ALL);
        
        nTotalBrCur = 0;
        for (int j=0; j<paramCur.mode_num; j++) {
            EXPECT_TRUE(paramCur.encode_frame_rate[j]>=14.9f);
            if (pSubscribedStr->stStream[j].uMaxFS) {
                EXPECT_TRUE(paramCur.encode_width[j]*paramCur.encode_height[j]/256<=pSubscribedStr->stStream[j].uMaxFS);
            }

            EXPECT_TRUE(paramCur.encode_frame_rate[j]<=22.1f);
            
            if (pSubscribedStr->stStream[j].uMaxBitRate) {
                EXPECT_TRUE(paramCur.max_target_bitrate[j]<=pSubscribedStr->stStream[j].uMaxBitRate);
            }
            nTotalBrCur += paramCur.max_target_bitrate[j];
        }
        
        EXPECT_TRUE(nTotalBrCur>0);
        EXPECT_TRUE(nTotalBrCur>=nTotalBrPrev);
        nTotalBrPrev = nTotalBrCur;
        
        paramPrev = paramCur;
    }
}
TEST_F(CWseEncodeParamGeneratorTest, GenerateParamList_MultiStream_One)
{
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x0020;
    sSubscribedStr.stStream[0].uMaxMBPS = 216000;
    sSubscribedStr.stStream[0].uMaxFS = 3600;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 20480;
    sSubscribedStr.stStream[0].uMaxBitRate = 2000000;
    EXPECT_TRUE(TestOneKindSubscription(&sSubscribedStr));
    
    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x0016;
    sSubscribedStr.stStream[0].uMaxMBPS = 20250;
    sSubscribedStr.stStream[0].uMaxFS = 1620;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 8100;
    sSubscribedStr.stStream[0].uMaxBitRate = 700000;
    EXPECT_TRUE(TestOneKindSubscription(&sSubscribedStr));

    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x000c;
    sSubscribedStr.stStream[0].uMaxMBPS = 6000;
    sSubscribedStr.stStream[0].uMaxFS = 396;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 2376;
    sSubscribedStr.stStream[0].uMaxBitRate = 200000;
    EXPECT_TRUE(TestOneKindSubscription(&sSubscribedStr));

    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x000a;
    sSubscribedStr.stStream[0].uMaxMBPS = 1485;
    sSubscribedStr.stStream[0].uMaxFS = 99;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 396;
    sSubscribedStr.stStream[0].uMaxBitRate = 64000;
    EXPECT_TRUE(TestOneKindSubscription(&sSubscribedStr));
}
TEST_F(CWseEncodeParamGeneratorTest, GenerateParamList_MultiStream_Two)
{
    
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 2;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
   
    sSubscribedStr.stStream[1].uProfileLevelID = 0x42E000 + 0x0020;
    sSubscribedStr.stStream[1].uMaxMBPS = 216000;
    sSubscribedStr.stStream[1].uMaxFS = 3600;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 20480;
    sSubscribedStr.stStream[1].uMaxBitRate = 2000000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42E000 + 0x000c;
    sSubscribedStr.stStream[0].uMaxMBPS = 6000;
    sSubscribedStr.stStream[0].uMaxFS = 396;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 2376;
    sSubscribedStr.stStream[0].uMaxBitRate = 200000;
    EXPECT_TRUE(TestOneKindSubscription(&sSubscribedStr));
}

TEST_F(CWseEncodeParamGeneratorTest, GenerateParamList_SVS)
{
    //create
    pEncodeParamGenerator = new CWseEncodeParamSVS();
    ASSERT_TRUE(pEncodeParamGenerator!=NULL);
    
    //set constraint
    WseVideoEncoderCapability sVideoEncoderCapability;
    srand((unsigned int)time(NULL));
    unsigned int c_uLevelIdx = rand()%H264_LEVEL_NUMBER;
    unsigned int c_uLevelIdc = aH264LevelLimits[c_uLevelIdx].uLevelIdc;
    sVideoEncoderCapability.uProfileLevelID = 0x42E000 + (c_uLevelIdc&0x00FF);
    sVideoEncoderCapability.uMaxBitRate = WSE_MAX(rand(), aH264LevelLimits[c_uLevelIdx].uMaxBR);
    sVideoEncoderCapability.uMaxFPS = (rand()%60);
    sVideoEncoderCapability.uMaxFS = WSE_MAX(rand(), aH264LevelLimits[c_uLevelIdx].uMaxFS);
    sVideoEncoderCapability.uMaxMBPS = WSE_MAX(rand(), aH264LevelLimits[c_uLevelIdx].uMaxMBPS);
    sVideoEncoderCapability.uMaxNalUnitSize = rand();
    
    //launch test
    int iMaxLevel = 0;
    pEncodeParamGenerator->SetSourceResolution(1280, 720, 30);
    
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x0020;
    sSubscribedStr.stStream[0].uMaxMBPS = 216000;
    sSubscribedStr.stStream[0].uMaxFS = 3600;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 20480;
    sSubscribedStr.stStream[0].uMaxBitRate = 2000000;
    EXPECT_TRUE(WSE_S_OK == pEncodeParamGenerator->SetSubscribedStreams(&sSubscribedStr));
    
    //EXPECT_TRUE (TestOneSetting(pEncodeParamGenerator, sVideoEncoderCapability, iMaxLevel));
}

