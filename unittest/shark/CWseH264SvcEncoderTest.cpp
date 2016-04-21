
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "SvcClientEngineMock.h"
#include "WseH264SvcEncoder.h"
#include "WseVideoSample.h"

class CWseH264SvcEncoderTest : public testing::Test
{
public:
	CWseH264SvcEncoderTest()
	{
		pEncoder = new CWseH264SvcEncoder();
	}

	virtual ~CWseH264SvcEncoderTest()
	{
		delete pEncoder;
	}

	virtual void SetUp()
	{
	
	}

	virtual void TearDown()
	{
		
	}

public:
	CWseH264SvcEncoder *pEncoder;
};

TEST_F(CWseH264SvcEncoderTest, Init)
{

	CWseEncodeParam param;
	memset(&param,0,sizeof(CWseEncodeParam));
    param.dwCodecType = videoCodecH264SVC;
	param.mode_num = 1;
	param.encode_width[0] = 1280;
	param.encode_height[0] = 720;
	param.encode_frame_rate[0] = 15;
	param.max_target_bitrate[0] = 64000;

	srand((unsigned int)time(NULL));
	param.max_bitrate[0] = rand();

	MockIWseEncoderSink pSink;
	EXPECT_EQ(WSE_S_OK,pEncoder->Init(&param,&pSink));

	WseVideoCodecType type = videoCodecUnknown;
	pEncoder->GetCodecType(type);
	EXPECT_EQ(videoCodecH264SVC, type);	

	pEncoder->SetInitCaptureFrameRate(30.0f);
}

TEST_F(CWseH264SvcEncoderTest, SetEncodeParam)
{
	CWseEncodeParam param;
	memset(&param,0,sizeof(CWseEncodeParam));
	param.mode_num = 1;
	param.encode_width[0] = 1280;
	param.encode_height[0] = 720;
	param.encode_frame_rate[0] = 15;
	param.max_target_bitrate[0] = 64000;

	srand((unsigned int)time(NULL));
	param.max_bitrate[0] = rand();

	MockIWseEncoderSink pSink;
	pEncoder->Init(&param,&pSink);

	param.encode_width[0] = 320;
	param.encode_height[0] = 180;
	EXPECT_EQ(WSE_S_OK,pEncoder->SetEncodeParam(&param));
	EXPECT_EQ(320,pEncoder->m_pSvcParam->encode_width[0]);
	EXPECT_EQ(180,pEncoder->m_pSvcParam->encode_height[0]);

	EXPECT_EQ(0, pEncoder->GetEncodeParam(&param));
}

TEST_F(CWseH264SvcEncoderTest, EncodeFrame)
{
	CWseEncodeParam param;
	memset(&param,0,sizeof(CWseEncodeParam));
	param.mode_num = 1;
	param.encode_width[0] = 1280;
	param.encode_height[0] = 720;
	param.encode_frame_rate[0] = 25;
	param.max_target_bitrate[0] = 64000;

	srand((unsigned int)time(NULL));
	param.max_bitrate[0] = rand();

	MockIWseEncoderSink pSink;
	pEncoder->Init(&param,&pSink);
    EXPECT_CALL(pSink, OnEncodeParamChanged(_,_,_)).WillRepeatedly(Return(0));
    EXPECT_CALL(pSink, OnEncoderIDRFrame(_)).WillRepeatedly(Return(0));

	param.encode_width[0] = 320;
	param.encode_height[0] = 240;
	pEncoder->SetEncodeParam(&param);
    
    CWseVideoSampleAllocator *pSampleAllocator = new CWseVideoSampleAllocator(64);
    pSampleAllocator->AddRef();
    IWseVideoSample *pSample = NULL;

	WseVideoFormat format;
	memset(&format,0,sizeof(WseVideoFormat));
	format.video_type = WseI420;
	format.width = 320;
	format.height = 180;
    pSampleAllocator->GetSample(format, &pSample);
	MockIWseDataSink pDataSink;
	pEncoder->SetDataSink(&pDataSink);
    EXPECT_CALL(pDataSink, OnOutputData(_)).WillRepeatedly(Return(0));
	EXPECT_EQ(WSE_S_OK,pEncoder->EncodeFrame(pSample));
	EXPECT_EQ(320, pEncoder->m_pSvcEncodingParam->sSpatialLayers[0].iVideoWidth);
	EXPECT_EQ(180, pEncoder->m_pSvcEncodingParam->sSpatialLayers[0].iVideoHeight);
	EXPECT_EQ(3, pEncoder->m_pSvcEncodingParam->iTemporalLayerNum);
	pSample->Release();

	format.width = 640;
	format.height = 480;
	param.encode_frame_rate[0] = 18;
	pEncoder->SetEncodeParam(&param);
    pSampleAllocator->GetSample(format, &pSample);
	EXPECT_EQ(WSE_S_OK, pEncoder->EncodeFrame(pSample));
	EXPECT_EQ(2, pEncoder->m_pSvcEncodingParam->iTemporalLayerNum);
    pSample->Release();

	WseVideoSourceInfoType sourceinfo;
	sourceinfo.nWidth = 1280;
	sourceinfo.nHeight = 720;
	sourceinfo.nFrameRate = 10;
	sourceinfo.nType = SourceType_Desktop_Sharing;
	EXPECT_EQ(0, pEncoder->SetOption(ENCODER_CONFIG_SOURCE_INFO, &sourceinfo));

	pEncoder->Init(&param,&pSink);
	param.encode_width[0] = 1280;
	param.encode_height[0] = 720;
	pEncoder->SetEncodeParam(&param);
	//pEncoder->SetDataSink(&pDataSink);
	format.width = 1280;
	format.height = 720;
    pSampleAllocator->GetSample(format, &pSample);
	EXPECT_EQ(WSE_S_OK, pEncoder->EncodeFrame(pSample));
    pSample->Release();
    
    pSampleAllocator->Release();
}

TEST_F(CWseH264SvcEncoderTest, EncodeKeyFrame)
{
	CWseEncodeParam param;
	memset(&param,0,sizeof(CWseEncodeParam));
	param.mode_num = 1;
	param.encode_width[0] = 1280;
	param.encode_height[0] = 720;
	param.encode_frame_rate[0] = 15;
	param.max_target_bitrate[0] = 64000;

	srand((unsigned int)time(NULL));
	param.max_bitrate[0] = rand();

	MockIWseEncoderSink pSink;
	pEncoder->Init(&param,&pSink);

	/*param.encode_width[0] = 320;
	param.encode_height[0] = 180;
	pEncoder->SetEncodeParam(&param);*/

	EXPECT_EQ(WSE_S_OK, pEncoder->EncodeKeyFrame());
	EXPECT_EQ(true, pEncoder->m_bGetKeyFrame);
}

TEST_F(CWseH264SvcEncoderTest, VideoTypeToVideoFormat)
{
	WseVideoType type;
	int format;

	type = WseUnknown;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(-1,format);

	type = WseRGB24;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatRGB | videoFormatVFlip,format);

	type = WseI420;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatI420,format);

	type = WseYUY2;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatYUY2,format);

	type = WseBGR24;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatBGR | videoFormatVFlip,format);

	type = WseYV12;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatYV12,format);

	type = WseRGB24Flip;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatRGB,format);

	type = WseBGR24Flip;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatBGR,format);

	type = WseRGBA32;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatRGBA | videoFormatVFlip,format);

	type = WseBGRA32;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatBGRA | videoFormatVFlip,format);

	type = WseARGB32;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatARGB | videoFormatVFlip,format);

	type = WseABGR32;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatABGR | videoFormatVFlip,format);

	type = WseRGBA32Flip;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatRGBA,format);

	type = WseBGRA32Flip;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatBGRA,format);

	type = WseARGB32Flip;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatARGB,format);

	type = WseABGR32Flip;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(videoFormatABGR,format);

	type = WseAVCStream;
	format = pEncoder->VideoTypeToVideoFormat(type);
	EXPECT_EQ(-1,format);
}

TEST_F(CWseH264SvcEncoderTest, LTR)
{
	CWseEncodeParam param;
	memset(&param,0,sizeof(CWseEncodeParam));
	param.mode_num = 1;
	param.encode_width[0] = 1280;
	param.encode_height[0] = 720;
	param.encode_frame_rate[0] = 15;
	param.max_target_bitrate[0] = 64000;

	srand((unsigned int)time(NULL));
	param.max_bitrate[0] = rand();

	MockIWseEncoderSink pSink;
	pEncoder->Init(&param,&pSink);


	SLTRRecoverRequest request;
	memset(&request, 0, sizeof(SLTRRecoverRequest));
	EXPECT_EQ(WSE_S_OK, pEncoder->SetLTRRecoverRequest(&request));

	SLTRMarkingFeedback feedback;
	memset(&feedback, 0, sizeof(SLTRMarkingFeedback));
	EXPECT_EQ(WSE_S_OK, pEncoder->SetLTRMarkingFeedback(&feedback));
}
TEST_F(CWseH264SvcEncoderTest, TrimVid){
    CWseEncodeParam cParam;
    memset(&cParam, 0, sizeof(cParam));
    cParam.mode_num = 1;
    cParam.encode_width[0] = 1280;
    cParam.encode_height[0] = 720;
    cParam.encode_frame_rate[0] = 15;
    cParam.max_target_bitrate[0] = 14000000;
    cParam.max_bitrate[0] = 15000000;
    cParam.vid_arrays[0].nNumVids = 1;
    cParam.vid_arrays[0].aVidArray[0] = 1;
    
    MockIWseEncoderSink pSink;
    pEncoder->Init(&cParam, &pSink);
    
    pEncoder->m_InputVideoFormat.video_type = WseI420;
    pEncoder->m_InputVideoFormat.width = 640;
    pEncoder->m_InputVideoFormat.height = 360;
    
    pEncoder->UpdateEncodeParam();     //try smaller resolution when encoding one layer.
    EXPECT_EQ(cParam.vid_arrays[0].nNumVids, pEncoder->m_pSvcParam->vid_arrays[0].nNumVids);
    EXPECT_EQ(cParam.vid_arrays[0].aVidArray[0], pEncoder->m_pSvcParam->vid_arrays[0].aVidArray[0]);
    
    cParam.mode_num = 2;
    cParam.encode_width[0] = 640;
    cParam.encode_height[0] = 360;
    cParam.encode_frame_rate[0] = 15;
    cParam.max_target_bitrate[0] = 10000000;
    cParam.max_bitrate[0] = 11000000;
    cParam.vid_arrays[0].nNumVids = 1;
    cParam.vid_arrays[0].aVidArray[0] = 2;
    cParam.encode_width[1] = 1280;
    cParam.encode_height[1] = 720;
    cParam.encode_frame_rate[1] = 15;
    cParam.max_target_bitrate[1] = 14000000;
    cParam.max_bitrate[1] = 15000000;
    cParam.vid_arrays[1].nNumVids = 1;
    cParam.vid_arrays[1].aVidArray[0] = 1;
    pEncoder->SetEncodeParam(&cParam);
    pEncoder->UpdateEncodeParam();          //try capture smaller resolution than required, to check trimVidArray function
    EXPECT_EQ((cParam.vid_arrays[0].nNumVids+cParam.vid_arrays[1].nNumVids), pEncoder->m_pSvcParam->vid_arrays[0].nNumVids);

    pEncoder->UpdateEncodeParam();          //try it again, to check whether trimVidArray propagate vid
    EXPECT_EQ((cParam.vid_arrays[0].nNumVids+cParam.vid_arrays[1].nNumVids), pEncoder->m_pSvcParam->vid_arrays[0].nNumVids);

    pEncoder->m_InputVideoFormat.width = 1280;
    pEncoder->m_InputVideoFormat.height = 720;

    pEncoder->UpdateEncodeParam();          //after captured resolution recovers, check trimed vid array recovers.
    EXPECT_EQ(cParam.vid_arrays[0].nNumVids, pEncoder->m_pSvcParam->vid_arrays[0].nNumVids);
    EXPECT_EQ(cParam.vid_arrays[1].nNumVids, pEncoder->m_pSvcParam->vid_arrays[1].nNumVids);
    

}
