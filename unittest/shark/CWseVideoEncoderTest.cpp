
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WseVideoEncoder.h"
#include "SvcClientEngineMock.h"

// The CWseVideoEncoder class had become a virtual base class
/*
class CWseVideoEncoderTest : public testing::Test
{
public:
	CWseVideoEncoderTest()
	{
		pEncoder = new CWseVideoEncoder();
        EXPECT_CALL(pDataSink, OnOutputData(_)).WillRepeatedly(Return(0));
	}

	virtual ~CWseVideoEncoderTest()
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
	CWseVideoEncoder *pEncoder;
    MockIWseDataSink pDataSink;
};

TEST_F(CWseVideoEncoderTest, Init)
{

	CWseEncodeParam param;
	memset(&param,0,sizeof(CWseEncodeParam));
	IWseEncoderSink* pSink = NULL;
	int num = 1;

	pEncoder->m_CodecMode = WSE_CODEC_MODE_SVC;
	param.dwCodecType = videoCodecH263;
	EXPECT_EQ(WSE_E_INVALID_PARAMS,pEncoder->Init(&param,pSink,num));
	param.dwCodecType = videoCodecH264AVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));
	param.dwCodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));
	param.dwCodecType = videoCodecVp8;
#ifdef ANDROID
	EXPECT_EQ(WSE_E_NOTIMPL, pEncoder->Init(&param, pSink, num));
#else
	EXPECT_EQ(WSE_E_FAIL, pEncoder->Init(&param, pSink, num));
#endif
//	EXPECT_EQ(1, pEncoder->GetCodecNum());
	
	pEncoder->m_CodecMode = WSE_CODEC_MODE_SVC_AVC;
	param.dwCodecType = videoCodecH263;
	EXPECT_EQ(WSE_E_INVALID_PARAMS,pEncoder->Init(&param,pSink,num));
	param.dwCodecType = videoCodecH264AVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));
	param.dwCodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));
	param.dwCodecType = videoCodecVp8;
#ifdef ANDROID
	EXPECT_EQ(WSE_E_NOTIMPL, pEncoder->Init(&param, pSink, num));
#else
	EXPECT_EQ(WSE_E_FAIL, pEncoder->Init(&param, pSink, num));
#endif
//	EXPECT_EQ(2, pEncoder->GetCodecNum());

	pEncoder->m_CodecMode = WSE_CODEC_MODE_AVC;
	param.dwCodecType = videoCodecH263;
	EXPECT_EQ(WSE_E_INVALID_PARAMS,pEncoder->Init(&param,pSink,num));
	param.dwCodecType = videoCodecH264AVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));
	param.dwCodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));
	param.dwCodecType = videoCodecVp8;
#ifdef ANDROID
	EXPECT_EQ(WSE_E_NOTIMPL, pEncoder->Init(&param, pSink, num));
#else
	EXPECT_EQ(WSE_E_FAIL, pEncoder->Init(&param, pSink, num));
#endif
//	EXPECT_EQ(4, pEncoder->GetCodecNum());
}

TEST_F(CWseVideoEncoderTest, SetEncodeParam)
{
	CWseEncodeParam param;
	memset(&param,0,sizeof(CWseEncodeParam));
	param.mode_num = 1;
	IWseEncoderSink* pSink = NULL;
	int num = 1;

	pEncoder->m_CodecMode = WSE_CODEC_MODE_SVC;
	param.dwCodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));
	EXPECT_EQ(WSE_S_OK, pEncoder->SetEncodeParam(&param, num));

	pEncoder->m_CodecMode = WSE_CODEC_MODE_SVC_AVC;
	param.dwCodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));
	EXPECT_EQ(WSE_S_OK, pEncoder->SetEncodeParam(&param, num));
	param.mode_num = 3;
	EXPECT_EQ(WSE_S_OK, pEncoder->SetEncodeParam(&param, num));
//	EXPECT_EQ(2, param.mode_num);

	pEncoder->m_CodecMode = WSE_CODEC_MODE_AVC;
	param.dwCodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));
	EXPECT_EQ(WSE_S_OK, pEncoder->SetEncodeParam(&param, num));

	EXPECT_EQ(WSE_S_OK, pEncoder->GetEncodeParam(&param, num));
}

TEST_F(CWseVideoEncoderTest, OnOutputData)
{
	CWseEncodeParam param;
	memset(&param,0,sizeof(CWseEncodeParam));
	IWseEncoderSink* pSink = NULL;
	int num = 1;

	CWseEncoderOutput output;
	memset(&output, 0, sizeof(CWseEncoderOutput));
	output.mode_idx = 1;

	pEncoder->m_CodecMode = WSE_CODEC_MODE_SVC;
	param.dwCodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));
	pEncoder->SetDataSink(&pDataSink);
	param.mode_num = 1;
	EXPECT_EQ(WSE_S_OK, pEncoder->SetEncodeParam(&param, num));
	output.stream_id = 0;
	EXPECT_EQ(WSE_S_OK, pEncoder->OnOutputData(output));
	EXPECT_EQ(15,output.stream_id);
	param.mode_num = 2;
	EXPECT_EQ(WSE_S_OK, pEncoder->SetEncodeParam(&param, num));
	output.stream_id = 0;
	EXPECT_EQ(WSE_S_OK, pEncoder->OnOutputData(output));
	EXPECT_EQ(14,output.stream_id);
	param.mode_num = 3;
	EXPECT_EQ(WSE_S_OK, pEncoder->SetEncodeParam(&param, num));
	output.stream_id = 0;
	EXPECT_EQ(WSE_S_OK, pEncoder->OnOutputData(output));
	EXPECT_EQ(2,output.stream_id);
	param.mode_num = 4;
	EXPECT_EQ(WSE_S_OK, pEncoder->SetEncodeParam(&param, num));
	output.stream_id = 0;
	EXPECT_EQ(WSE_S_OK, pEncoder->OnOutputData(output));
	EXPECT_EQ(2,output.stream_id);


	pEncoder->m_CodecMode = WSE_CODEC_MODE_SVC_AVC;
	param.dwCodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));
	pEncoder->SetDataSink(&pDataSink);
	EXPECT_EQ(WSE_S_OK, pEncoder->OnOutputData(output));

	pEncoder->m_CodecMode = WSE_CODEC_MODE_AVC;
	param.dwCodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));
	pEncoder->SetDataSink(&pDataSink);
	param.mode_num = 1;
	EXPECT_EQ(WSE_S_OK, pEncoder->SetEncodeParam(&param, num));
	output.stream_id = 0;
	EXPECT_EQ(WSE_S_OK, pEncoder->OnOutputData(output));
	EXPECT_EQ(15,output.stream_id);
	param.mode_num = 2;
	EXPECT_EQ(WSE_S_OK, pEncoder->SetEncodeParam(&param, num));
	output.stream_id = 0;
	EXPECT_EQ(WSE_S_OK, pEncoder->OnOutputData(output));
	EXPECT_EQ(14,output.stream_id);
	param.mode_num = 3;
	EXPECT_EQ(WSE_S_OK, pEncoder->SetEncodeParam(&param, num));
	output.stream_id = 0;
	EXPECT_EQ(WSE_S_OK, pEncoder->OnOutputData(output));
	EXPECT_EQ(2,output.stream_id);
	param.mode_num = 4;
	EXPECT_EQ(WSE_S_OK, pEncoder->SetEncodeParam(&param, num));
	output.stream_id = 0;
	EXPECT_EQ(WSE_S_OK, pEncoder->OnOutputData(output));
	EXPECT_EQ(2,output.stream_id);
}

TEST_F(CWseVideoEncoderTest, Option)
{
	CWseEncodeParam param;
	memset(&param,0,sizeof(CWseEncodeParam));
	IWseEncoderSink* pSink = NULL;
	int num = 1;

	pEncoder->m_CodecMode = WSE_CODEC_MODE_SVC;
	param.dwCodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));

	ENCODER_CONFIG opt;
	opt = ENCODER_CONFIG_UNKNOWN;
	EXPECT_EQ(-1, pEncoder->SetOption(opt, NULL));

	opt = ENCODER_CONFIG_REALTIME_APP;
	WseAppType appvalue = WSE_REALTIME_APP;
	EXPECT_EQ(0, pEncoder->SetOption(opt, &appvalue));

	opt = ENCODER_CONFIG_SOURCE_INFO;
	WseVideoSourceInfoType sourceinfo;
	sourceinfo.nType = SourceType_UNKNOWN;
	EXPECT_EQ(0, pEncoder->SetOption(opt, &sourceinfo));
	sourceinfo.nType = SourceType_Meeting_SVC; 
	EXPECT_EQ(0, pEncoder->SetOption(opt, &sourceinfo));
	sourceinfo.nType = SourceType_Video_Sharing;
	EXPECT_EQ(0, pEncoder->SetOption(opt, &sourceinfo));
	sourceinfo.nType = SourceType_Desktop_Sharing;
	EXPECT_EQ(0, pEncoder->SetOption(opt, &sourceinfo));

	EXPECT_EQ(0, pEncoder->GetOption(opt, &sourceinfo));
}

TEST_F(CWseVideoEncoderTest, LTR)
{
	SLTRRecoverRequest request;
	memset(&request, 0, sizeof(SLTRRecoverRequest));
	EXPECT_EQ(WSE_S_OK, pEncoder->SetLTRRecoverRequest(&request));

	SLTRMarkingFeedback feedback;
	memset(&feedback, 0, sizeof(SLTRMarkingFeedback));
	EXPECT_EQ(WSE_S_OK, pEncoder->SetLTRMarkingFeedback(&feedback));

	CWseEncodeParam param;
	memset(&param,0,sizeof(CWseEncodeParam));
	IWseEncoderSink* pSink = NULL;
	int num = 1;

	param.dwCodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));

	pEncoder->SetDataSink(&pDataSink);

	EXPECT_EQ(WSE_S_OK, pEncoder->SetLTRRecoverRequest(&request));

	EXPECT_EQ(WSE_S_OK, pEncoder->SetLTRMarkingFeedback(&feedback));
}

TEST_F(CWseVideoEncoderTest, IWseDataSinkMethod)
{
	CWseEncodeParam param;
	memset(&param,0,sizeof(CWseEncodeParam));
	IWseEncoderSink* pSink = NULL;
	int num = 1;

	param.dwCodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK, pEncoder->Init(&param, pSink, num));

	CWseEncoderOutput output;
	memset(&output, 0, sizeof(CWseEncoderOutput));

	pEncoder->OnFrameEncoded(output);
	pEncoder->OnEncodeParamChanged(NULL);
	pEncoder->OnInputFrameRateChanged(0);

	pEncoder->SetDataSink(&pDataSink);

	pEncoder->OnFrameEncoded(output);
	pEncoder->OnEncodeParamChanged(NULL);
	pEncoder->OnInputFrameRateChanged(0);
}
*/
