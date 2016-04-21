
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "SvcClientEngineMock.h"
#include "WseEncodeControl.h"

class CWseEncodeControllerTest : public testing::Test
{
public:
	CWseEncodeControllerTest()
	{
		pEncodeController =  (CWseEncodeController*)CreateEncodeController(EncodeController_Adaptor_Based);
	}

	virtual ~CWseEncodeControllerTest()
	{
        pEncodeController->Release();
	}

	virtual void SetUp()
	{

	}

	virtual void TearDown()
	{

	}

public:
	CWseEncodeController *pEncodeController;
};

TEST_F(CWseEncodeControllerTest, Init)
{
	WseVideoSourceInfoType SourceInfoType;
	SourceInfoType.nType = SourceType_Video_Sharing;
	SourceInfoType.nWidth = 352;
	SourceInfoType.nHeight = 288;
	SourceInfoType.nFrameRate = 15;
	EXPECT_EQ(WSE_S_OK, pEncodeController->Init(&SourceInfoType));

	SourceInfoType.nType = SourceType_Meeting_Camera;
	SourceInfoType.nWidth = 160;
	SourceInfoType.nHeight = 90;
	SourceInfoType.nFrameRate = 30;
	EXPECT_EQ(WSE_E_INVALIDARG, pEncodeController->SetSourceInfo(&SourceInfoType));
	EXPECT_EQ(WSE_E_FAIL, pEncodeController->Init(&SourceInfoType));//SourceInfo just for video sharing and desktop sharing
    EXPECT_EQ(WSE_S_OK, pEncodeController->Init());

	SourceInfoType.nType = SourceType_Desktop_Sharing;
	SourceInfoType.nWidth = 1440;
	SourceInfoType.nHeight = 900;
	SourceInfoType.nFrameRate = 10;
	EXPECT_EQ(WSE_S_OK, pEncodeController->SetSourceInfo(&SourceInfoType));
}

TEST_F(CWseEncodeControllerTest, UpdateUpLinkStat)
{
	WseVideoSourceInfoType SourceInfoType;
	SourceInfoType.nType = SourceType_Desktop_Sharing;
	SourceInfoType.nWidth = 1600;
	SourceInfoType.nHeight = 900;
	SourceInfoType.nFrameRate = 10;
	EXPECT_EQ(WSE_S_OK, pEncodeController->Init(&SourceInfoType));
	
	TUpLinkNetStat stat;
	memset(&stat, 0, sizeof(TUpLinkNetStat));
	stat.dwEvaluateBandwidth = 800000;
	stat.nCongestStatus = 8;

	EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));

	//TODO: input and output parameters match
}

TEST_F(CWseEncodeControllerTest, GetCaptureCapability)
{
    const int nCapNum = 6;
    const int n360pIdx = 3;
    WseCameraCapability sCapabilities[nCapNum] = {
        { WseI420,  160,  90, 10, 30 },
        { WseI420,  320,  180, 10, 30 },
        { WseI420,  480,  270, 10, 30 },
        { WseI420,  640,  360, 10, 30 },
        { WseI420,  960,  540, 10, 30 },
        { WseI420,  1280, 720, 10, 30 },
    };
    
    WseCameraCapability  sCapParam;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->Init());
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetCaptureCameraCapability(nCapNum, sCapabilities));
    pEncodeController->SetMeetingVideoQuality(Meeting_Video_Quality_SD);
    EXPECT_EQ(WSE_S_OK, pEncodeController->GetCaptureCapability(nCapNum, sCapabilities, &sCapParam));
    EXPECT_EQ(0, memcmp(&sCapParam, &sCapabilities[n360pIdx], sizeof(sCapParam)));
}
TEST_F(CWseEncodeControllerTest, SetSubscribeMultistream_SVS)
{
    
    WseVideoSourceInfoType SourceInfoType;
    SourceInfoType.nType = SourceType_Video_Sharing;
    SourceInfoType.nWidth = 352;
    SourceInfoType.nHeight = 288;
    SourceInfoType.nFrameRate = 15;
    EXPECT_EQ(WSE_S_OK, pEncodeController->Init(&SourceInfoType));
   
    //Test: SetSourceInfo first then SetOption
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 1350000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42E000 + 0x0020;
    
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 2;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetSourceInfo(&SourceInfoType));

    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeController::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    CWseEncodeParam cEncCfg;
    pEncodeController->GetEncodeConfiguration(&cEncCfg);
    EXPECT_EQ(1, cEncCfg.mode_num);
    EXPECT_EQ(1, cEncCfg.vid_arrays[0].nNumVids);
    EXPECT_EQ(2, cEncCfg.vid_arrays[0].aVidArray[0]);
    
    
    //Test: sucbscribe 2 streams
    sSubscribedStr.uNumStreams = 2;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxBitRate = 536000;
    sSubscribedStr.stStream[1].uProfileLevelID = 0x42E000 + 0x0016;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 3;
    sSubscribedStr.stStream[1].uNumVids = 2;
    sSubscribedStr.stStream[1].uVids[0] = 4;
    sSubscribedStr.stStream[1].uVids[1] = 5;
    
    SourceInfoType.nType = SourceType_Video_Sharing;
    SourceInfoType.nWidth = 352;
    SourceInfoType.nHeight = 288;
    SourceInfoType.nFrameRate = 15;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeController::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    EXPECT_EQ(WSE_S_OK, pEncodeController->GetEncodeConfiguration(&cEncCfg));
    EXPECT_EQ(2, cEncCfg.mode_num);
    EXPECT_EQ(352, cEncCfg.encode_width[1]);
    EXPECT_EQ(288, cEncCfg.encode_height[1]);
    EXPECT_EQ(1, cEncCfg.vid_arrays[0].nNumVids);
    EXPECT_EQ(3, cEncCfg.vid_arrays[0].aVidArray[0]);
    EXPECT_EQ(2, cEncCfg.vid_arrays[1].nNumVids);
    EXPECT_EQ(4, cEncCfg.vid_arrays[1].aVidArray[0]);
    EXPECT_EQ(5, cEncCfg.vid_arrays[1].aVidArray[1]);
    
    //Test: SetOption first then SetSourceInfo
    SourceInfoType.nType = SourceType_Video_Sharing;
    SourceInfoType.nWidth = 320;
    SourceInfoType.nHeight = 480;
    SourceInfoType.nFrameRate = 15;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetSourceInfo(&SourceInfoType));
    EXPECT_EQ(WSE_S_OK, pEncodeController->GetEncodeConfiguration(&cEncCfg));
    EXPECT_EQ(320, cEncCfg.encode_width[1]);
    EXPECT_EQ(480, cEncCfg.encode_height[1]);
    EXPECT_EQ(2, cEncCfg.mode_num);
    EXPECT_EQ(1, cEncCfg.vid_arrays[0].nNumVids);
    EXPECT_EQ(3, cEncCfg.vid_arrays[0].aVidArray[0]);
    EXPECT_EQ(2, cEncCfg.vid_arrays[1].nNumVids);
    EXPECT_EQ(4, cEncCfg.vid_arrays[1].aVidArray[0]);
    EXPECT_EQ(5, cEncCfg.vid_arrays[1].aVidArray[1]);
    
    
    //Test: subscribe lower layer
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFS = 60;
    sSubscribedStr.stStream[0].uMaxBitRate = 536000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42E000 + 0x0016;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 6;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeController::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    EXPECT_EQ(WSE_S_OK, pEncodeController->GetEncodeConfiguration(&cEncCfg));
    EXPECT_EQ(1, cEncCfg.mode_num);
    EXPECT_EQ(1, cEncCfg.vid_arrays[0].nNumVids);
    EXPECT_EQ(6, cEncCfg.vid_arrays[0].aVidArray[0]);
    EXPECT_EQ(160, cEncCfg.encode_width[0]);
    
    
    //Test: subscribe up layer
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFS = 920;
    sSubscribedStr.stStream[0].uMaxBitRate = 536000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42E000 + 0x0016;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 7;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeController::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    EXPECT_EQ(WSE_S_OK, pEncodeController->GetEncodeConfiguration(&cEncCfg));
    EXPECT_EQ(1, cEncCfg.mode_num);
    EXPECT_EQ(1, cEncCfg.vid_arrays[0].nNumVids);
    EXPECT_EQ(7, cEncCfg.vid_arrays[0].aVidArray[0]);
    EXPECT_EQ(320, cEncCfg.encode_width[0]);
    
    
    //Test: subscribe test as user case
    SourceInfoType.nType = SourceType_Video_Sharing;
    SourceInfoType.nWidth = 640;
    SourceInfoType.nHeight = 480;
    SourceInfoType.nFrameRate = 30;
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFS = 920;
    sSubscribedStr.stStream[0].uMaxBitRate = 573000;
    sSubscribedStr.stStream[0].uProfileLevelID = 4325406;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 8;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetSourceInfo(&SourceInfoType));
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeController::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    EXPECT_EQ(WSE_S_OK, pEncodeController->GetEncodeConfiguration(&cEncCfg));
    EXPECT_EQ(1, cEncCfg.mode_num);
    EXPECT_EQ(1, cEncCfg.vid_arrays[0].nNumVids);
    
}
TEST_F(CWseEncodeControllerTest, SetSubscribeMultistream_SVSLower)
{
    
    WseVideoSourceInfoType SourceInfoType;
    SourceInfoType.nType = SourceType_Video_Sharing;
    SourceInfoType.nWidth = 352;
    SourceInfoType.nHeight = 288;
    SourceInfoType.nFrameRate = 15;
    EXPECT_EQ(WSE_S_OK, pEncodeController->Init(&SourceInfoType));
    

    
    //Test: subscribe test as user case
    SourceInfoType.nType = SourceType_Video_Sharing;
    SourceInfoType.nWidth = 640;
    SourceInfoType.nHeight = 480;
    SourceInfoType.nFrameRate = 30;
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));

    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFS = 920;
    sSubscribedStr.stStream[0].uMaxBitRate = 573000;
    sSubscribedStr.stStream[0].uProfileLevelID = 4325406;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 8;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetSourceInfo(&SourceInfoType));
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeController::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    
    CWseEncodeParam cEncCfg;
    EXPECT_EQ(WSE_S_OK, pEncodeController->GetEncodeConfiguration(&cEncCfg));
    
    EXPECT_EQ(1, cEncCfg.mode_num);
    EXPECT_EQ(1, cEncCfg.vid_arrays[0].nNumVids);
    

    //============= subscribe maxFS=240 ===============
    SourceInfoType.nType = SourceType_Video_Sharing;
    SourceInfoType.nWidth = 640;
    SourceInfoType.nHeight = 360;
    SourceInfoType.nFrameRate = 30;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetSourceInfo(&SourceInfoType));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFS = 240;
    sSubscribedStr.stStream[0].uMaxBitRate = 328000;
    sSubscribedStr.stStream[0].uProfileLevelID = 4325406;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 8;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeController::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->GetEncodeConfiguration(&cEncCfg));
    EXPECT_EQ(180, cEncCfg.encode_height[0]);

}
class CWseEncodeControllerSimulTest : public testing::Test
{
public:
    CWseEncodeControllerSimulTest()
    {
        pEncodeController =  (CWseEncodeControllerSimul*)CreateEncodeController(EncodeController_Simulcast);
    }
    
    virtual ~CWseEncodeControllerSimulTest()
    {
        pEncodeController->Release();
    }
    
    virtual void SetUp()
    {
        
    }
    
    virtual void TearDown()
    {
        
    }
    
public:
    CWseEncodeControllerSimul *pEncodeController;
};

TEST_F(CWseEncodeControllerSimulTest, SetSubscribeMultistream)
{
    WseMeetingVideoQualityType eMtQuality = Meeting_Video_Quality_HD_720P;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_MEETING_VIDEO_QUALITY, &eMtQuality));
    
    WsePerformanceDetectionAlert eDynamicPerf = Warning_alert_Green;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_DYNAMIC_PERFORMANCE, &eDynamicPerf));
    
//    int nCapNum;
//    WseCameraCapability caps;
//    pEncodeController->GetCaptureCapability(nCapNum, &caps);
    
    //0: 720P, 1: 360P. Encode control will exchange the order to 0: 360p, 1: 720p.
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 1350000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42E000 + 0x0020;
    sSubscribedStr.stStream[0].uMaxFS = 3600;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 18000;
    sSubscribedStr.stStream[0].uMaxMBPS = 108000;

    sSubscribedStr.stStream[0].uNumVids = 4;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    sSubscribedStr.stStream[0].uVids[1] = 1;
    sSubscribedStr.stStream[0].uVids[2] = 2;
    sSubscribedStr.stStream[0].uVids[3] = 3;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    
    sSubscribedStr.uNumStreams = 2;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxBitRate = 536000;
    sSubscribedStr.stStream[1].uProfileLevelID = 0x42E000 + 0x0016;
    sSubscribedStr.stStream[1].uMaxFS = 920;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 8100;
    sSubscribedStr.stStream[1].uMaxMBPS = 20250;
    sSubscribedStr.stStream[1].uNumVids = 2;
    sSubscribedStr.stStream[1].uVids[0] = 4;
    sSubscribedStr.stStream[1].uVids[1] = 5;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    
    CWseEncodeParam cEncParam;
    pEncodeController->GetEncodeConfiguration(&cEncParam);

    EXPECT_EQ(2, cEncParam.mode_num);
    //Pay attention to index, as 360p will be arranged before 720p. When checking uNumVids, exchange the index.
    EXPECT_EQ(sSubscribedStr.stStream[1].uNumVids, cEncParam.vid_arrays[0].nNumVids);
    for (int i=0; i<sSubscribedStr.stStream[1].uNumVids; i++) {
        EXPECT_EQ(sSubscribedStr.stStream[1].uVids[i], cEncParam.vid_arrays[0].aVidArray[i]);
    }
    EXPECT_EQ(sSubscribedStr.stStream[0].uNumVids, cEncParam.vid_arrays[1].nNumVids);
    for (int i=0; i<sSubscribedStr.stStream[0].uNumVids; i++) {
        EXPECT_EQ(sSubscribedStr.stStream[0].uVids[i], cEncParam.vid_arrays[1].aVidArray[i]);
    }
    
    //degrade 360P to 180P based on bandwidth;
    unsigned long dwEvaluatedBandwidth = 180000;
    pEncodeController->UpdateLinksStat(0, cEncParam.vid_arrays[0].nNumVids, &cEncParam.vid_arrays[0].aVidArray[0], dwEvaluatedBandwidth);
    CWseEncodeParam cEncParamDegraded;
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(2, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(720, cEncParamDegraded.encode_height[1]);
    
    //degrade 720p to 180p based on bandwidth;
    dwEvaluatedBandwidth = 200000;
    pEncodeController->UpdateLinksStat(1, cEncParam.vid_arrays[1].nNumVids, &cEncParam.vid_arrays[1].aVidArray[0], dwEvaluatedBandwidth);
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    
}

#ifdef ANDROID
TEST_F(CWseEncodeControllerTest, SetConstrainedEncodeControl)
{
    //======== Test 3 layers at most (no 90p) for android grafika encoder
    pEncodeController->SetConstrainedEncodeControl(true);
    
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 2;
    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x000a;
    sSubscribedStr.stStream[0].uMaxMBPS = 1800;
    sSubscribedStr.stStream[0].uMaxFS = 99;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 396;
    sSubscribedStr.stStream[0].uMaxBitRate = 64000;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    
    sSubscribedStr.stStream[1].uProfileLevelID =  0x42E000 + 0x000c;
    sSubscribedStr.stStream[1].uMaxMBPS = 7200;
    sSubscribedStr.stStream[1].uMaxFS = 396;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 2376;
    sSubscribedStr.stStream[1].uMaxBitRate = 200000;
    sSubscribedStr.stStream[1].uNumVids = 1;
    sSubscribedStr.stStream[1].uVids[1] = 1;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeController::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    CWseEncodeParam cEncParamDegraded;
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(2, cEncParamDegraded.vid_arrays[0].nNumVids);
    
    //degrade to 180p based on bandwidth;
    TUpLinkNetStat stat;
    memset(&stat, 0, sizeof(TUpLinkNetStat));
    stat.dwEvaluateBandwidth = 64000;
    stat.nCongestStatus = 8;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    
    //========== Test 4 layers as original logic for android software encoder
    pEncodeController->SetConstrainedEncodeControl(false);
    
    stat.dwEvaluateBandwidth = 500000;
    stat.nCongestStatus = 8;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(2, cEncParamDegraded.mode_num);
    EXPECT_EQ(90, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[1]);
    EXPECT_EQ(1, cEncParamDegraded.vid_arrays[0].nNumVids);
    EXPECT_EQ(1, cEncParamDegraded.vid_arrays[1].nNumVids);
    
    //degrade 90p based on bandwidth;
    memset(&stat, 0, sizeof(TUpLinkNetStat));
    stat.dwEvaluateBandwidth = 64000;
    stat.nCongestStatus = 8;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(90, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(2, cEncParamDegraded.vid_arrays[0].nNumVids);
}
TEST_F(CWseEncodeControllerTest, SetConstrainedEncodeControl_270p540p)
{
    //======== Test 4 layers at most (no 90p) for android grafika encoder
    pEncodeController->SetConstrainedEncodeControl(true);
    
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 4;
    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x000a;
    sSubscribedStr.stStream[0].uMaxMBPS = 1800;
    sSubscribedStr.stStream[0].uMaxFS = 99;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 396;
    sSubscribedStr.stStream[0].uMaxBitRate = 64000;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    
    sSubscribedStr.stStream[1].uProfileLevelID =  0x42E000 + 0x000c;
    sSubscribedStr.stStream[1].uMaxMBPS = 7200;
    sSubscribedStr.stStream[1].uMaxFS = 396;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 2376;
    sSubscribedStr.stStream[1].uMaxBitRate = 200000;
    sSubscribedStr.stStream[1].uNumVids = 1;
    sSubscribedStr.stStream[1].uVids[1] = 1;
    
    sSubscribedStr.stStream[2].uMaxFPS = 30;
    sSubscribedStr.stStream[2].uMaxBitRate = 800000;
    sSubscribedStr.stStream[2].uProfileLevelID = 0x42E000 + 0x0014;
    sSubscribedStr.stStream[2].uMaxFS = 920;
    sSubscribedStr.stStream[2].uMaxFPS = 30;
    sSubscribedStr.stStream[2].uMaxDPB = 0;
    sSubscribedStr.stStream[2].uMaxMBPS = 27600;
    sSubscribedStr.stStream[2].uNumVids = 1;
    sSubscribedStr.stStream[2].uVids[0] = 2;
    
    sSubscribedStr.stStream[3].uMaxFPS = 30;
    sSubscribedStr.stStream[3].uMaxBitRate = 2000000;
    sSubscribedStr.stStream[3].uProfileLevelID =  0x42E000 + 0x0020;
    sSubscribedStr.stStream[3].uMaxFS = 3600;
    sSubscribedStr.stStream[3].uMaxFPS = 30;
    sSubscribedStr.stStream[3].uMaxDPB = 0;
    sSubscribedStr.stStream[3].uMaxMBPS = 108000;
    sSubscribedStr.stStream[3].uNumVids = 1;
    sSubscribedStr.stStream[3].uVids[0] = 3;

    TUpLinkNetStat stat;
    memset(&stat, 0, sizeof(TUpLinkNetStat));
    stat.dwEvaluateBandwidth = 4000000;    //bandwidth is enough for 90+180+360+720, merge 90+180
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeController::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    CWseEncodeParam cEncParamDegraded;
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(3, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(360, cEncParamDegraded.encode_height[1]);
    EXPECT_EQ(720, cEncParamDegraded.encode_height[2]);
    EXPECT_EQ(2, cEncParamDegraded.vid_arrays[0].nNumVids);
    EXPECT_EQ(1, cEncParamDegraded.vid_arrays[1].nNumVids);
    EXPECT_EQ(1, cEncParamDegraded.vid_arrays[2].nNumVids);

    stat.dwEvaluateBandwidth = 1740000;    //bandwidth is enough for 90+180+360+576, merge 90+180, 360+576
    stat.nCongestStatus = 8;
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(2, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(360, cEncParamDegraded.encode_height[1]);
    EXPECT_EQ(2, cEncParamDegraded.vid_arrays[0].nNumVids);
    EXPECT_EQ(2, cEncParamDegraded.vid_arrays[1].nNumVids);

    stat.dwEvaluateBandwidth = 59000;    //uplink is very low, merge all
    stat.nCongestStatus = 8;
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(4, cEncParamDegraded.vid_arrays[0].nNumVids);

    //======== Test 90p + 360p for android grafika encoder
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 2;
    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x000a;
    sSubscribedStr.stStream[0].uMaxMBPS = 1800;
    sSubscribedStr.stStream[0].uMaxFS = 99;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 396;
    sSubscribedStr.stStream[0].uMaxBitRate = 64000;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxBitRate = 800000;
    sSubscribedStr.stStream[1].uProfileLevelID = 0x42E000 + 0x0014;
    sSubscribedStr.stStream[1].uMaxFS = 920;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 0;
    sSubscribedStr.stStream[1].uMaxMBPS = 27600;
    sSubscribedStr.stStream[1].uNumVids = 1;
    sSubscribedStr.stStream[1].uVids[0] = 1;
    
    stat.dwEvaluateBandwidth = 400000;    //bandwidth is enough for 90+270, merge to 180p
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeController::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(2, cEncParamDegraded.vid_arrays[0].nNumVids);

    stat.dwEvaluateBandwidth = 590000;    //bandwidth is enough for 90+360, change 90 to 180
    stat.nCongestStatus = 8;
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(2, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(360, cEncParamDegraded.encode_height[1]);
    EXPECT_EQ(1, cEncParamDegraded.vid_arrays[0].nNumVids);
    EXPECT_EQ(1, cEncParamDegraded.vid_arrays[1].nNumVids);
    
    
    //======== Test 90p + 360p + 720p for android grafika encoder
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 3;
    sSubscribedStr.stStream[0].uProfileLevelID =  0x42E000 + 0x000a;
    sSubscribedStr.stStream[0].uMaxMBPS = 1800;
    sSubscribedStr.stStream[0].uMaxFS = 99;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 396;
    sSubscribedStr.stStream[0].uMaxBitRate = 64000;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxBitRate = 800000;
    sSubscribedStr.stStream[1].uProfileLevelID = 0x42E000 + 0x0014;
    sSubscribedStr.stStream[1].uMaxFS = 920;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 0;
    sSubscribedStr.stStream[1].uMaxMBPS = 27600;
    sSubscribedStr.stStream[1].uNumVids = 1;
    sSubscribedStr.stStream[1].uVids[0] = 1;
    
    sSubscribedStr.stStream[2].uMaxFPS = 30;
    sSubscribedStr.stStream[2].uMaxBitRate = 2000000;
    sSubscribedStr.stStream[2].uProfileLevelID =  0x42E000 + 0x0020;
    sSubscribedStr.stStream[2].uMaxFS = 3600;
    sSubscribedStr.stStream[2].uMaxFPS = 30;
    sSubscribedStr.stStream[2].uMaxDPB = 0;
    sSubscribedStr.stStream[2].uMaxMBPS = 108000;
    sSubscribedStr.stStream[2].uNumVids = 1;
    sSubscribedStr.stStream[2].uVids[0] = 2;
    stat.dwEvaluateBandwidth = 400000;    //bandwidth is enough for 90+270, merge to 180p
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeController::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(3, cEncParamDegraded.vid_arrays[0].nNumVids);
    
    stat.dwEvaluateBandwidth = 590000;    //bandwidth is enough for 90+360, change 90 to 180, and merge 720 vids with 360p's
    stat.nCongestStatus = 8;
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(2, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(360, cEncParamDegraded.encode_height[1]);
    EXPECT_EQ(1, cEncParamDegraded.vid_arrays[0].nNumVids);
    EXPECT_EQ(2, cEncParamDegraded.vid_arrays[1].nNumVids);
    
    stat.dwEvaluateBandwidth = 1500000;    //bandwidth is enough for 90+360+576, change 90 to 180, and merge 720 with 360p
    stat.nCongestStatus = 8;
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(2, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(360, cEncParamDegraded.encode_height[1]);
    EXPECT_EQ(1, cEncParamDegraded.vid_arrays[0].nNumVids);
    EXPECT_EQ(2, cEncParamDegraded.vid_arrays[1].nNumVids);

    stat.dwEvaluateBandwidth = 4000000;    //bandwidth is enough for 90+360+720, change 90 to 180
    stat.nCongestStatus = 8;
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(3, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(360, cEncParamDegraded.encode_height[1]);
    EXPECT_EQ(720, cEncParamDegraded.encode_height[2]);
    EXPECT_EQ(1, cEncParamDegraded.vid_arrays[0].nNumVids);
    EXPECT_EQ(1, cEncParamDegraded.vid_arrays[1].nNumVids);
    EXPECT_EQ(1, cEncParamDegraded.vid_arrays[2].nNumVids);

}
TEST_F(CWseEncodeControllerSimulTest, SetConstrainedEncodeControl)
{
    //======== Test 3 layers at most (no 90p) for android grafika encoder
    pEncodeController->SetConstrainedEncodeControl(true);

    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 2;
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
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    CWseEncodeParam cEncParamDegraded;
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);

    //degrade 720p to 180p based on bandwidth;
    TUpLinkNetStat stat;
    memset(&stat, 0, sizeof(TUpLinkNetStat));
    stat.dwEvaluateBandwidth = 64000;
    stat.nCongestStatus = 8;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    
    //========== Test 4 layers as original logic for android software encoder
    pEncodeController->SetConstrainedEncodeControl(false);
    
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 2;
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
    stat.dwEvaluateBandwidth = 500000;
    stat.nCongestStatus = 8;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(2, cEncParamDegraded.mode_num);
    EXPECT_EQ(90, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[1]);
    
    //degrade 90p based on bandwidth;
    memset(&stat, 0, sizeof(TUpLinkNetStat));
    stat.dwEvaluateBandwidth = 64000;
    stat.nCongestStatus = 8;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(90, cEncParamDegraded.encode_height[0]);
    
}

TEST_F(CWseEncodeControllerSimulTest, SetSubscribeMultistream_270p540p)
{
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    
    //Test 270p downgrade to 180p;
    sSubscribedStr.uNumStreams = 2;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 230000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42E000 + 0x0014;
    sSubscribedStr.stStream[0].uMaxFS = 920;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 0;
    sSubscribedStr.stStream[0].uMaxMBPS = 27600;
    
    sSubscribedStr.stStream[0].uNumVids = 4;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    sSubscribedStr.stStream[0].uVids[1] = 1;
    sSubscribedStr.stStream[0].uVids[2] = 2;
    sSubscribedStr.stStream[0].uVids[3] = 3;
    
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxBitRate = 300000;
    sSubscribedStr.stStream[1].uProfileLevelID =  0x42E000 + 0x0020;
    sSubscribedStr.stStream[1].uMaxFS = 3600;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 0;
    sSubscribedStr.stStream[1].uMaxMBPS = 108000;
    sSubscribedStr.stStream[1].uNumVids = 2;
    sSubscribedStr.stStream[1].uVids[0] = 4;
    sSubscribedStr.stStream[1].uVids[1] = 5;
    pEncodeController->SetConstrainedEncodeControl(true);
    TUpLinkNetStat stat;
    memset(&stat, 0, sizeof(TUpLinkNetStat));
    stat.dwEvaluateBandwidth = 2000000;
    stat.nCongestStatus = 8;
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    CWseEncodeParam cEncParamDegraded;
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(6, cEncParamDegraded.vid_arrays[0].nNumVids);
    

    sSubscribedStr.stStream[0].uMaxBitRate = 600000;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxBitRate = 1000000;
    sSubscribedStr.stStream[1].uProfileLevelID =  0x42E000 + 0x0020;
    sSubscribedStr.stStream[1].uMaxFS = 3600;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 0;
    sSubscribedStr.stStream[1].uMaxMBPS = 108000;
    sSubscribedStr.stStream[1].uNumVids = 2;
    sSubscribedStr.stStream[1].uVids[0] = 4;
    sSubscribedStr.stStream[1].uVids[1] = 5;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(360, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(6, cEncParamDegraded.vid_arrays[0].nNumVids);

}
TEST_F(CWseEncodeControllerSimulTest, SetSubscribeMultistream_test)
{
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    
    //Test 90p;
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 135000;
    sSubscribedStr.stStream[0].uProfileLevelID = 4325407;
    sSubscribedStr.stStream[0].uMaxFS = 396;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 0;
    sSubscribedStr.stStream[0].uMaxMBPS = 3000;
    
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    pEncodeController->SetConstrainedEncodeControl(true);
    TUpLinkNetStat stat;
    memset(&stat, 0, sizeof(TUpLinkNetStat));
    stat.dwEvaluateBandwidth = 1724280;
    stat.nCongestStatus = 8;
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    CWseEncodeParam cEncParamDegraded;
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(135000, cEncParamDegraded.max_bitrate[0]);
    EXPECT_EQ(1, cEncParamDegraded.vid_arrays[0].nNumVids);
    
    //Test 270p downgrade to 180p;
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 300000;
    sSubscribedStr.stStream[0].uProfileLevelID = 4325407;
    sSubscribedStr.stStream[0].uMaxFS = 396;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 0;
    sSubscribedStr.stStream[0].uMaxMBPS = 7200;
    
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(300000, cEncParamDegraded.max_bitrate[0]);
    EXPECT_EQ(1, cEncParamDegraded.vid_arrays[0].nNumVids);
}

#endif

TEST_F(CWseEncodeControllerSimulTest, SetSubscribeMultistream_360720p)
{
    //0: 720P, 1: 360P. Encode control will exchange the order to 0: 360p, 1: 720p.
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 2;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 1000000;
    sSubscribedStr.stStream[0].uProfileLevelID = 4382740;
    sSubscribedStr.stStream[0].uMaxFS = 920;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 0;
    sSubscribedStr.stStream[0].uMaxMBPS = 27600;
    
    sSubscribedStr.stStream[0].uNumVids = 4;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    sSubscribedStr.stStream[0].uVids[1] = 1;
    sSubscribedStr.stStream[0].uVids[2] = 2;
    sSubscribedStr.stStream[0].uVids[3] = 3;
    
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxBitRate = 1500000;
    sSubscribedStr.stStream[1].uProfileLevelID =  0x42E000 + 0x0020;
    sSubscribedStr.stStream[1].uMaxFS = 3600;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 0;
    sSubscribedStr.stStream[1].uMaxMBPS = 108000;
    sSubscribedStr.stStream[1].uNumVids = 2;
    sSubscribedStr.stStream[1].uVids[0] = 4;
    sSubscribedStr.stStream[1].uVids[1] = 5;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    CWseEncodeParam cEncParam;
    pEncodeController->GetEncodeConfiguration(&cEncParam);
    
    EXPECT_EQ(2, cEncParam.mode_num);

}

TEST_F(CWseEncodeControllerSimulTest, SetSubscribeMultistream_errorcode)
{
    WseMeetingVideoQualityType eMtQuality = Meeting_Video_Quality_HD_720P;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_MEETING_VIDEO_QUALITY, &eMtQuality));
    
    WsePerformanceDetectionAlert eDynamicPerf = Warning_alert_Green;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_DYNAMIC_PERFORMANCE, &eDynamicPerf));
    
    
    //0: 720P, 1: 360P; 360p@3fps is invalid, we should only encode 720p.
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 2;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 1350000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42E000 + 0x0020;
    sSubscribedStr.stStream[0].uMaxFS = 3600;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 18000;
    sSubscribedStr.stStream[0].uMaxMBPS = 108000;
    
    sSubscribedStr.stStream[0].uNumVids = 4;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    sSubscribedStr.stStream[0].uVids[1] = 1;
    sSubscribedStr.stStream[0].uVids[2] = 2;
    sSubscribedStr.stStream[0].uVids[3] = 3;
    
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxBitRate = 536000;
    sSubscribedStr.stStream[1].uProfileLevelID = 0x42E000 + 0x0016;
    sSubscribedStr.stStream[1].uMaxFS = 920;
    sSubscribedStr.stStream[1].uMaxFPS = 3;
    sSubscribedStr.stStream[1].uMaxDPB = 8100;
    sSubscribedStr.stStream[1].uMaxMBPS = 20250;
    sSubscribedStr.stStream[1].uNumVids = 2;
    sSubscribedStr.stStream[1].uVids[0] = 4;
    sSubscribedStr.stStream[1].uVids[1] = 5;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    EXPECT_EQ(true, sSubscribedStr.stStream[0].bSucceeded);
    EXPECT_EQ(false, sSubscribedStr.stStream[1].bSucceeded);
    
    CWseEncodeParam cEncParam;
    pEncodeController->GetEncodeConfiguration(&cEncParam);
    
    EXPECT_EQ(1, cEncParam.mode_num);
    //Pay attention to index, as 360p will be arranged before 720p. When checking uNumVids, exchange the index.
    EXPECT_EQ(sSubscribedStr.stStream[0].uNumVids, cEncParam.vid_arrays[0].nNumVids);
    for (int i=0; i<sSubscribedStr.stStream[0].uNumVids; i++) {
        EXPECT_EQ(sSubscribedStr.stStream[0].uVids[i], cEncParam.vid_arrays[0].aVidArray[i]);
    }
    
    EXPECT_EQ(720, cEncParam.encode_height[0]);
}
TEST_F(CWseEncodeControllerSimulTest, SetSubscribeMultistream_everysub)
{
    WseMeetingVideoQualityType eMtQuality = Meeting_Video_Quality_HD_720P;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_MEETING_VIDEO_QUALITY, &eMtQuality));
    
    WsePerformanceDetectionAlert eDynamicPerf = Warning_alert_Green;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_DYNAMIC_PERFORMANCE, &eDynamicPerf));
    
     //0: 720P, 1: 360P; 360p@3fps is invalid, we should only encode 720p.
    CWseEncodeParam cEncParam;
    WseVideoSubscribeMultiStream sSubscribedStr;
    
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 1000000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42e00b;
    sSubscribedStr.stStream[0].uMaxFS = 396;
    sSubscribedStr.stStream[0].uMaxMBPS = 3000;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    EXPECT_EQ(true, sSubscribedStr.stStream[0].bSucceeded);
    pEncodeController->GetEncodeConfiguration(&cEncParam);
    //EXPECT_EQ(90, cEncParam.encode_height[0]);

    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 1000000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42e00c;
    sSubscribedStr.stStream[0].uMaxFS = 396;
    sSubscribedStr.stStream[0].uMaxMBPS = 7200;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    EXPECT_EQ(true, sSubscribedStr.stStream[0].bSucceeded);
    pEncodeController->GetEncodeConfiguration(&cEncParam);
    EXPECT_EQ(180, cEncParam.encode_height[0]);
    
    
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 1000000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42e014;
    sSubscribedStr.stStream[0].uMaxFS = 920;
    sSubscribedStr.stStream[0].uMaxMBPS = 27600;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    EXPECT_EQ(true, sSubscribedStr.stStream[0].bSucceeded);
    pEncodeController->GetEncodeConfiguration(&cEncParam);
    EXPECT_EQ(360, cEncParam.encode_height[0]);
    
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 2000000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42e01f;
    sSubscribedStr.stStream[0].uMaxFS = 3600;
    sSubscribedStr.stStream[0].uMaxMBPS = 108000;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    EXPECT_EQ(true, sSubscribedStr.stStream[0].bSucceeded);
    pEncodeController->GetEncodeConfiguration(&cEncParam);
    EXPECT_EQ(720, cEncParam.encode_height[0]);
    
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFPS = 24;
    sSubscribedStr.stStream[0].uMaxBitRate = 1000000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42e00c;
    sSubscribedStr.stStream[0].uMaxFS = 396;
    sSubscribedStr.stStream[0].uMaxMBPS = 3000;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    EXPECT_EQ(true, sSubscribedStr.stStream[0].bSucceeded);
    pEncodeController->GetEncodeConfiguration(&cEncParam);
    //EXPECT_EQ(90, cEncParam.encode_height[0]);
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFPS = 24;
    sSubscribedStr.stStream[0].uMaxBitRate = 1000000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42e00c;
    sSubscribedStr.stStream[0].uMaxFS = 396;
    sSubscribedStr.stStream[0].uMaxMBPS = 7200;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    EXPECT_EQ(true, sSubscribedStr.stStream[0].bSucceeded);
    pEncodeController->GetEncodeConfiguration(&cEncParam);
    EXPECT_EQ(180, cEncParam.encode_height[0]);
    
    
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFPS = 24;
    sSubscribedStr.stStream[0].uMaxBitRate = 1000000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42e01f;
    sSubscribedStr.stStream[0].uMaxFS = 920;
    sSubscribedStr.stStream[0].uMaxMBPS = 27600;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    EXPECT_EQ(true, sSubscribedStr.stStream[0].bSucceeded);
    pEncodeController->GetEncodeConfiguration(&cEncParam);
    EXPECT_EQ(360, cEncParam.encode_height[0]);
    
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFPS = 24;
    sSubscribedStr.stStream[0].uMaxBitRate = 2000000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42e01f;
    sSubscribedStr.stStream[0].uMaxFS = 3600;
    sSubscribedStr.stStream[0].uMaxMBPS = 108000;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    EXPECT_EQ(true, sSubscribedStr.stStream[0].bSucceeded);
    pEncodeController->GetEncodeConfiguration(&cEncParam);
    EXPECT_EQ(720, cEncParam.encode_height[0]);
}

TEST_F(CWseEncodeControllerSimulTest, SetSubscribeMultistream_TestBW2Smoothsend)
{
    //0: 720P, 1: 360P. Encode control will exchange the order to 0: 360p, 1: 720p.
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetConstrainedEncodeControl(false));
    
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 2;

    sSubscribedStr.stStream[0].uMaxBitRate = 1000000;
    sSubscribedStr.stStream[0].uProfileLevelID = 4382740;
    sSubscribedStr.stStream[0].uMaxFS = 920;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 0;
    sSubscribedStr.stStream[0].uMaxMBPS = 27600;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 0;


    sSubscribedStr.stStream[1].uMaxBitRate = 1500000;
    sSubscribedStr.stStream[1].uProfileLevelID =  0x42E000 + 0x0020;
    sSubscribedStr.stStream[1].uMaxFS = 3600;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 0;
    sSubscribedStr.stStream[1].uMaxMBPS = 108000;
    sSubscribedStr.stStream[1].uNumVids = 1;
    sSubscribedStr.stStream[1].uVids[0] = 1;

    WseMeetingVideoQualityType eMtQuality = Meeting_Video_Quality_HD_720P;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_MEETING_VIDEO_QUALITY, &eMtQuality));
    
    WsePerformanceDetectionAlert eDynamicPerf = Warning_alert_Green;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_DYNAMIC_PERFORMANCE, &eDynamicPerf));
    
    //degrade 720p to 180p based on bandwidth;
    TUpLinkNetStat stat;
    memset(&stat, 0, sizeof(TUpLinkNetStat));
    stat.dwEvaluateBandwidth = 4000000;
    stat.nCongestStatus = 8;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));

    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    CWseEncodeParam cEncParam;
    pEncodeController->GetEncodeConfiguration(&cEncParam);
    
    EXPECT_EQ(2, cEncParam.mode_num);
    EXPECT_GE(1000000, cEncParam.bandwidth_to_smoothsend[0]);
    EXPECT_GE(1500000, cEncParam.bandwidth_to_smoothsend[1]);
    
    //when uplink is not enough for 2 layers
    stat.dwEvaluateBandwidth = 800000;
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    pEncodeController->GetEncodeConfiguration(&cEncParam);
    EXPECT_EQ(1, cEncParam.mode_num);
    EXPECT_GE(800000, cEncParam.bandwidth_to_smoothsend[0]);

    //When uplink is enough for 2, but each is small than downlink
    stat.dwEvaluateBandwidth = 1500000;
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    pEncodeController->GetEncodeConfiguration(&cEncParam);
    EXPECT_EQ(2, cEncParam.mode_num);
    EXPECT_LE(stat.dwEvaluateBandwidth, cEncParam.bandwidth_to_smoothsend[0]+cEncParam.bandwidth_to_smoothsend[1]);

    //720 downlinks is 600k, downgrade to 360p, and bandwith_to_smoothsend should follow this
    sSubscribedStr.stStream[1].uMaxBitRate = 500000;
    sSubscribedStr.stStream[0].uMaxBitRate = 1000000;
    stat.dwEvaluateBandwidth = 1500000; //Already update
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    pEncodeController->GetEncodeConfiguration(&cEncParam);
    EXPECT_EQ(1, cEncParam.mode_num);
    EXPECT_EQ(500000, cEncParam.bandwidth_to_smoothsend[0]);

}
TEST_F(CWseEncodeControllerSimulTest, GetMinMaxLevelBitrates)
{
    //0: 720P, 1: 360P. Encode control will exchange the order to 0: 360p, 1: 720p.
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetConstrainedEncodeControl(false));
    
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    sSubscribedStr.uNumStreams = 3;
    
    sSubscribedStr.stStream[0].uMaxBitRate = 1000000;
    sSubscribedStr.stStream[0].uProfileLevelID = 0x42e00c;
    sSubscribedStr.stStream[0].uMaxFS = 396;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 0;
    sSubscribedStr.stStream[0].uMaxMBPS = 7200;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 2;
    
    sSubscribedStr.stStream[1].uMaxBitRate = 1000000;
    sSubscribedStr.stStream[1].uProfileLevelID = 4382740;
    sSubscribedStr.stStream[1].uMaxFS = 920;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 0;
    sSubscribedStr.stStream[1].uMaxMBPS = 27600;
    sSubscribedStr.stStream[1].uNumVids = 1;
    sSubscribedStr.stStream[1].uVids[0] = 0;
    
    
    sSubscribedStr.stStream[2].uMaxBitRate = 1500000;
    sSubscribedStr.stStream[2].uProfileLevelID =  0x42E000 + 0x0020;
    sSubscribedStr.stStream[2].uMaxFS = 3600;
    sSubscribedStr.stStream[2].uMaxFPS = 30;
    sSubscribedStr.stStream[2].uMaxDPB = 0;
    sSubscribedStr.stStream[2].uMaxMBPS = 108000;
    sSubscribedStr.stStream[2].uNumVids = 1;
    sSubscribedStr.stStream[2].uVids[0] = 1;
    
    WseMeetingVideoQualityType eMtQuality = Meeting_Video_Quality_SD;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_MEETING_VIDEO_QUALITY, &eMtQuality));
    
    WsePerformanceDetectionAlert eDynamicPerf = Warning_alert_Green;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_DYNAMIC_PERFORMANCE, &eDynamicPerf));
    
    //degrade 720p to 180p based on bandwidth;
    TUpLinkNetStat stat;
    memset(&stat, 0, sizeof(TUpLinkNetStat));
    stat.dwEvaluateBandwidth = 4000000;
    stat.nCongestStatus = 8;
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    
    unsigned long dwMinBitrate, dwMaxBitrate;
    EXPECT_EQ(WSE_S_OK, pEncodeController->GetMinMaxLevelBitrate(dwMinBitrate, dwMaxBitrate));
    EXPECT_EQ(901998, dwMaxBitrate); //90p is 177999, 180p is 312999, 360p is 588999, 720p is 1779000
    
    eMtQuality = Meeting_Video_Quality_HD_720P;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_MEETING_VIDEO_QUALITY, &eMtQuality));
    EXPECT_EQ(WSE_S_OK, pEncodeController->GetMinMaxLevelBitrate(dwMinBitrate, dwMaxBitrate));
    EXPECT_EQ(2680998, dwMaxBitrate);
}

TEST_F(CWseEncodeControllerSimulTest, SetSubscribeMultistream_Disable90p)
{
    WseVideoSubscribeMultiStream sSubscribedStr;
    memset(&sSubscribedStr, 0, sizeof(sSubscribedStr));
    
    //Test 90p and 180p;
    sSubscribedStr.uNumStreams = 2;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 135000;
    sSubscribedStr.stStream[0].uProfileLevelID = 4325407;
    sSubscribedStr.stStream[0].uMaxFS = 396;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 0;
    sSubscribedStr.stStream[0].uMaxMBPS = 3000;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxBitRate = 300000;
    sSubscribedStr.stStream[1].uProfileLevelID = 4325407;
    sSubscribedStr.stStream[1].uMaxFS = 396;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 0;
    sSubscribedStr.stStream[1].uMaxMBPS = 7200;
    sSubscribedStr.stStream[1].uNumVids = 1;
    sSubscribedStr.stStream[1].uVids[0] = 1;
    pEncodeController->SetConstrainedEncodeControl(false);
    pEncodeController->SetDisable90p(true);
    
    TUpLinkNetStat stat;
    memset(&stat, 0, sizeof(TUpLinkNetStat));
    stat.dwEvaluateBandwidth = 1724280;
    stat.nCongestStatus = 8;
    EXPECT_EQ(WSE_S_OK, pEncodeController->UpdateUpLinkStat(&stat, 0));
    
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    CWseEncodeParam cEncParamDegraded;
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(135000, cEncParamDegraded.max_bitrate[0]);
    EXPECT_EQ(2, cEncParamDegraded.vid_arrays[0].nNumVids);
    
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxBitRate = 300000;
    sSubscribedStr.stStream[1].uProfileLevelID = 0x42E000 + 0x0020;;
    sSubscribedStr.stStream[1].uMaxFS = 920;
    sSubscribedStr.stStream[1].uMaxFPS = 30;
    sSubscribedStr.stStream[1].uMaxDPB = 0;
    sSubscribedStr.stStream[1].uMaxMBPS = 27600;
    sSubscribedStr.stStream[1].uNumVids = 1;
    sSubscribedStr.stStream[1].uVids[0] = 1;
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(135000, cEncParamDegraded.max_bitrate[0]);
    EXPECT_EQ(2, cEncParamDegraded.vid_arrays[0].nNumVids);
    
    //Test only 90p
    sSubscribedStr.uNumStreams = 1;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxBitRate = 135000;
    sSubscribedStr.stStream[0].uProfileLevelID = 4325407;
    sSubscribedStr.stStream[0].uMaxFS = 396;
    sSubscribedStr.stStream[0].uMaxFPS = 30;
    sSubscribedStr.stStream[0].uMaxDPB = 0;
    sSubscribedStr.stStream[0].uMaxMBPS = 3000;
    sSubscribedStr.stStream[0].uNumVids = 1;
    sSubscribedStr.stStream[0].uVids[0] = 0;
    EXPECT_EQ(WSE_S_OK, pEncodeController->SetOption(CWseEncodeControllerSimul::ENCODECONTROL_SUB_MULTI_STREAM, &sSubscribedStr));
    pEncodeController->GetEncodeConfiguration(&cEncParamDegraded);
    EXPECT_EQ(1, cEncParamDegraded.mode_num);
    EXPECT_EQ(180, cEncParamDegraded.encode_height[0]);
    EXPECT_EQ(135000, cEncParamDegraded.max_bitrate[0]);
    EXPECT_EQ(1, cEncParamDegraded.vid_arrays[0].nNumVids);


}
