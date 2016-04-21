
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "SvcClientEngineMock.h"
#include "WseH264SvcDecoder.h"
#include "WseVideoSample.h"

class CWseH264SvcDecoderTest : public testing::Test
{
public:
	CWseH264SvcDecoderTest()
	{
		pDecoder = new CWseH264SvcDecoder( new CWseVideoSampleAllocator(64));
	}

	virtual ~CWseH264SvcDecoderTest()
	{
		delete pDecoder;
	}

	virtual void SetUp()
	{
	
	}

	virtual void TearDown()
	{
		
	}

public:
	CWseH264SvcDecoder *pDecoder;
};

TEST_F(CWseH264SvcDecoderTest, Init)
{
	WseVideoType type;
	WseVideoCodecType CodecType;
	IWseVideoDecoderSink* pSink = NULL;
    EVideoFormatType option;

	
	
	type = WseI420;
	CodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK,pDecoder->Init(type,CodecType,WseSoftwareVideoCodec,pSink));

    EXPECT_EQ(WSE_S_OK,pDecoder->Uninit());
	
	//type = WseI420;
	//CodecType = videoCodecH264TP;
	//EXPECT_EQ(WSE_S_OK,pDecoder->Init(type,CodecType,pSink));	
	//EXPECT_EQ(WSE_S_OK,pDecoder->Unint());

}
TEST_F(CWseH264SvcDecoderTest, IsKeyFrameLost)
{
	WseVideoType type;
	WseVideoCodecType CodecType;
	IWseVideoDecoderSink* pSink = NULL;
    long hr;
	
	type = WseI420;
	CodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK,pDecoder->Init(type,CodecType,WseSoftwareVideoCodec,pSink));

	EXPECT_EQ(WSE_S_OK,pDecoder->SetP2Pmode(true));
	hr = dsErrorFree;
    EXPECT_EQ(NO_RECOVERY_REQUSET,pDecoder->IsKeyFrameLost(hr));

	hr = dsRefLost;
    EXPECT_EQ(IDR_RECOVERY_REQUEST,pDecoder->IsKeyFrameLost(hr));

    hr = dsNoParamSets;
    EXPECT_EQ(IDR_RECOVERY_REQUEST,pDecoder->IsKeyFrameLost(hr));

    EXPECT_EQ(WSE_S_OK,pDecoder->Uninit());
}
TEST_F(CWseH264SvcDecoderTest, LTRRecoveryRequest)
{
	WseVideoType type;
	WseVideoCodecType CodecType;
	MockIWseVideoDecoderSink pSink;
    long hr;
	
	type = WseI420;
	CodecType = videoCodecH264SVC;
    EXPECT_CALL(pSink, OnKeyFrameLost()).WillRepeatedly(Return(0));
    EXPECT_CALL(pSink, OnLTRrecoveryRequest(NotNull())).WillRepeatedly(Return(0));
	EXPECT_EQ(WSE_S_OK,pDecoder->Init(type,CodecType,WseSoftwareVideoCodec,&pSink));

    hr = dsNoParamSets;
	EXPECT_EQ(WSE_S_OK,pDecoder->LTRRecoveryRequest(hr));

	EXPECT_EQ(WSE_S_OK,pDecoder->SetP2Pmode(true));
	hr = dsNoParamSets;
    EXPECT_EQ(WSE_S_OK,pDecoder->LTRRecoveryRequest(hr));

	EXPECT_EQ(WSE_S_OK,pDecoder->SetP2Pmode(true));
	hr = dsRefLost;
    EXPECT_EQ(WSE_S_OK,pDecoder->LTRRecoveryRequest(hr));

	EXPECT_EQ(WSE_S_OK,pDecoder->Uninit());
}


TEST_F(CWseH264SvcDecoderTest, LTRMarkFeedback)
{
	WseVideoType type;
	WseVideoCodecType CodecType;
	MockIWseVideoDecoderSink pSink;
	long hr;

	type = WseI420;
	CodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK,pDecoder->Init(type,CodecType,WseSoftwareVideoCodec,&pSink));

	hr = dsErrorFree;
	EXPECT_EQ(WSE_S_OK,pDecoder->LTRMarkFeedback(hr));

	EXPECT_EQ(WSE_S_OK,pDecoder->Uninit());
}

TEST_F(CWseH264SvcDecoderTest, DecodeFrame)
{
	WseVideoType type;
	WseVideoCodecType CodecType;
	MockIWseVideoDecoderSink pSink;
	unsigned long uiTimeStamp;
	unsigned char* src;
	int iLen;
	
	type = WseI420;
	CodecType = videoCodecH264SVC;
	EXPECT_EQ(WSE_S_OK,pDecoder->Init(type,CodecType,WseSoftwareVideoCodec,&pSink));

	uiTimeStamp = 0;
    src = new UINT8[10];
	memset(src,0,10*sizeof(UINT8));
	iLen = 10;

	EXPECT_EQ(WSE_S_FALSE,pDecoder->DecodeFrame(uiTimeStamp, src, iLen));

	EXPECT_EQ(WSE_S_OK,pDecoder->Uninit());
}
