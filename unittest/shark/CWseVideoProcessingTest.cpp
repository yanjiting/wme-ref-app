
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WseVideoProcessing.h"
#include "WseVideoSourceChannel.h"
#include "WseVideoSample.h"
#include "jlbaseimp.h"
//#include "wbxmock/wbxmock.h"

using namespace shark;
class CWseVideoProcessingTest : public testing::TestWithParam<WseResizeMethod>
{
public:
	CWseVideoProcessingTest()
	{
//		pProcessing = new CWseVideoProcessing();
	}

	virtual ~CWseVideoProcessingTest()
	{
	}

	virtual void SetUp()
	{
	
	}

	virtual void TearDown()
	{
		
	}

public:
	CWseVideoProcessing pProcessing;
};

TEST_F(CWseVideoProcessingTest, Init)
{
	EXPECT_EQ(FALSE, pProcessing.Init(WseRGB24,NULL));

	CWseVideoSampleAllocator *pSampleAllocator = new CWseVideoSampleAllocator(64);
    pSampleAllocator->AddRef();
	EXPECT_EQ(TRUE, pProcessing.Init(WseRGB24,pSampleAllocator));

	pProcessing.Uninit();
	EXPECT_EQ(WseUnknown, pProcessing.m_typeOutput);
	EXPECT_EQ(NULL, pProcessing.m_pVideoCSC);
    
    pSampleAllocator->Release();
}

TEST_F(CWseVideoProcessingTest, SetProcessParam)
{
	CWseVideoSampleAllocator *pSampleAllocator = new CWseVideoSampleAllocator(64);
    pSampleAllocator->AddRef();
	EXPECT_EQ(TRUE, pProcessing.Init(WseRGB24,pSampleAllocator));

	WseProcessingParam param;
	memset(&param,0,sizeof(WseProcessingParam));
	param.method = CROP_METHORD;
    EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.SetProcessParams(&param));

    param.out_format.video_type = WseI420;
    EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.SetProcessParams(&param));

    param.out_format.width = 160;
    EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.SetProcessParams(&param));
    
    param.out_format.height = 90;
    EXPECT_EQ(WSE_S_OK, pProcessing.SetProcessParams(&param));
    
	EXPECT_EQ(WseI420, pProcessing.m_typeOutput);
	EXPECT_EQ(160, pProcessing.m_ProcessingParam.out_format.width);
	EXPECT_EQ(90, pProcessing.m_ProcessingParam.out_format.height);
    
    pSampleAllocator->Release();
}

INSTANTIATE_TEST_CASE_P(ResizeMethod, CWseVideoProcessingTest, 
						testing::Values(CLIP_METHORD, STRETCH_METHORD, FIT_METHORD));

TEST_F(CWseVideoProcessingTest, Process_Crop)
{
	CWseVideoSampleAllocator *pSampleAllocator = new CWseVideoSampleAllocator(64);
    pSampleAllocator->AddRef();
	pProcessing.Init(WseRGB24,pSampleAllocator);

	IWseVideoSample *pS;
	unsigned char* pSrc;

	WseVideoFormat inFormat;
	memset(&inFormat,0,sizeof(WseVideoFormat));
	inFormat.video_type = WseRGB24;
	inFormat.width = 176;
	inFormat.height = 144;

	WseVideoFormat outFormat;
	memset(&outFormat,0,sizeof(WseVideoFormat));
	outFormat.video_type = WseI420;
	outFormat.width = 176;
	outFormat.height = 144;

	WseProcessingParam param;
	memset(&param,0,sizeof(WseProcessingParam));
	memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	param.method = CROP_METHORD;

	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
	EXPECT_EQ(WSE_S_OK, pProcessing.Process(pS));
	pS->GetDataPointer(&pSrc);
	//EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	pS->Release();

	inFormat.width = 320;
	inFormat.height = 240;
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
	EXPECT_EQ(WSE_S_OK, pProcessing.Process(pS));
	pS->GetDataPointer(&pSrc);
	//EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	pS->Release();

	inFormat.video_type = WseYUY2;
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	outFormat.video_type = WseRGB24;
	memcpy(&param.out_format, &outFormat, sizeof(WseVideoFormat));
	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
	EXPECT_EQ(WSE_S_OK, pProcessing.Process(pS));
	pS->GetDataPointer(&pSrc);
	//EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	pS->Release();

	inFormat.video_type = WseRGB24;
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	outFormat.video_type = WseYUY2;
	memcpy(&param.out_format, &outFormat, sizeof(WseVideoFormat));
	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
	EXPECT_EQ(WSE_S_OK, pProcessing.Process(pS));
	pS->GetDataPointer(&pSrc);
	//EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	pS->Release();

	//outFormat.video_type = WseBGRA32;
	//memcpy(&param.out_format, &outFormat, sizeof(WseVideoFormat));
	//pProcessing.SetProcessParams(&param);
	//pSampleAllocator->GetSample(inFormat,&pS);
	//pS->SetVideoFormat(&inFormat);
	//EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(pS));
	//pS->GetDataPointer(&pSrc);
	//EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	//pS->Release();

    pSampleAllocator->Release();
}

// This feature is currently not supported. [Disable by Sand]
/*
TEST_P(CWseVideoProcessingTest, Process_Clip_Stretch_Fit)
{
	CWseVideoSampleAllocator *pSampleAllocator = new CWseVideoSampleAllocator(64);
    pSampleAllocator->AddRef();
	pProcessing.Init(WseRGB24,pSampleAllocator);

	IWseVideoSample *pS;
	unsigned char* pSrc;

	WseVideoFormat inFormat;
	memset(&inFormat,0,sizeof(WseVideoFormat));
	inFormat.video_type = WseRGB24;
	inFormat.width = 160;
	inFormat.height = 120;

	WseVideoFormat outFormat;
	memset(&outFormat,0,sizeof(WseVideoFormat));
	outFormat.video_type = WseI420;
	outFormat.width = 160;
	outFormat.height = 120;

	WseProcessingParam param;
	memset(&param,0,sizeof(WseProcessingParam));
	memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	param.method = GetParam();

	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
	EXPECT_EQ(0, pProcessing.Process(pS));
	pS->GetDataPointer(&pSrc);
	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	pS->Release();

	inFormat.width = 320;
	inFormat.height = 240;
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
	EXPECT_EQ(0, pProcessing.Process(pS));
	pS->GetDataPointer(&pSrc);
	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	pS->Release();

	inFormat.width = 320;
	inFormat.height = 180;
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
	EXPECT_EQ(0, pProcessing.Process(pS));
	pS->GetDataPointer(&pSrc);
	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	pS->Release();

	inFormat.width = 352;
	inFormat.height = 288;
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
	EXPECT_EQ(0, pProcessing.Process(pS));
	pS->GetDataPointer(&pSrc);
	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	pS->Release();

	inFormat.width = 320;
	inFormat.height = 240;
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	param.out_format.video_type = WseUnknown;
	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(pS));
	pS->GetDataPointer(&pSrc);
	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	pS->Release();
 
    pSampleAllocator->Release();
}
*/

// This feature is currently not supported. [Disable by Sand]
/*
TEST_F(CWseVideoProcessingTest, Process_Center)
{
	CWseVideoSampleAllocator *pSampleAllocator = new CWseVideoSampleAllocator(64);
    pSampleAllocator->AddRef();
	pProcessing.Init(WseRGB24,pSampleAllocator);

	IWseVideoSample *pS;
	unsigned char* pSrc;

	WseVideoFormat inFormat;
	memset(&inFormat,0,sizeof(WseVideoFormat));
	inFormat.video_type = WseRGB24;
	inFormat.width = 176;
	inFormat.height = 144;

	WseVideoFormat outFormat;
	memset(&outFormat,0,sizeof(WseVideoFormat));
	outFormat.video_type = WseI420;
	outFormat.width = 320;
	outFormat.height = 180;

	WseProcessingParam param;
	memset(&param,0,sizeof(WseProcessingParam));
	memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	param.method = CENTER_METHORD;

	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
	EXPECT_EQ(0, pProcessing.Process(pS));
	pS->GetDataPointer(&pSrc);
	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	pS->Release();

	inFormat.width = 320;
	inFormat.height = 144;
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
	EXPECT_EQ(0, pProcessing.Process(pS));
	pS->GetDataPointer(&pSrc);
	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	pS->Release();

	inFormat.width = 176;
	inFormat.height = 180;
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
	EXPECT_EQ(0, pProcessing.Process(pS));
	pS->GetDataPointer(&pSrc);
	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	pS->Release();

	inFormat.width = 360;
	inFormat.height = 240;
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
	EXPECT_EQ(0, pProcessing.Process(pS));
	pS->GetDataPointer(&pSrc);
	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	pS->Release();

	inFormat.width = 176;
	inFormat.height = 144;
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	param.out_format.video_type = WseUnknown;
	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(pS));
	pS->GetDataPointer(&pSrc);
	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.Process(inFormat, pSrc,0, outFormat, NULL, 0));
	pS->Release();
 
    pSampleAllocator->Release();
}
*/

TEST_F(CWseVideoProcessingTest, VideoDeliverer)
{
	CWseVideoSampleAllocator *pSampleAllocator = new CWseVideoSampleAllocator(64);
    pSampleAllocator->AddRef();
	pProcessing.Init(WseRGB24,pSampleAllocator);

	CWseVideoSourceChannel *pSrcChannel = new CWseVideoSourceChannel(0, NULL);

	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.AppendVideoDeliverer(NULL));
	EXPECT_EQ(WSE_S_OK,pProcessing.AppendVideoDeliverer(pSrcChannel));
	EXPECT_EQ(1, pProcessing.m_listDeliverers.size());
	EXPECT_EQ(WSE_S_OK,pProcessing.AppendVideoDeliverer(pSrcChannel));
	EXPECT_EQ(1, pProcessing.m_listDeliverers.size());

	IWseVideoSample *pS;

	WseVideoFormat inFormat;
	memset(&inFormat,0,sizeof(WseVideoFormat));
	inFormat.video_type = WseRGB24;
	inFormat.width = 176;
	inFormat.height = 144;

	WseVideoFormat outFormat;
	memset(&outFormat,0,sizeof(WseVideoFormat));
	outFormat.video_type = WseI420;
	outFormat.width = 176;
	outFormat.height = 144;

	WseProcessingParam param;
	memset(&param,0,sizeof(WseProcessingParam));
	memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	param.method = CROP_METHORD;

	pProcessing.SetProcessParams(&param);
	pSampleAllocator->GetSample(inFormat,&pS);
	pS->SetVideoFormat(&inFormat);
    
    IWseVideoSample* pSample = NULL;
	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.DeliverImage(pSample));
	EXPECT_EQ(WSE_S_OK, pProcessing.DeliverImage(pS));

	EXPECT_EQ(WSE_E_INVALIDARG, pProcessing.RemoveVideoDeliverer(NULL));
	EXPECT_EQ(WSE_S_OK,pProcessing.RemoveVideoDeliverer(pSrcChannel));
	EXPECT_EQ(0, pProcessing.m_listDeliverers.size());
    
    pSampleAllocator->Release();
}

TEST_F(CWseVideoProcessingTest, CSC)
{
    CWseVideoSampleAllocator *pSampleAllocator = new CWseVideoSampleAllocator(64);
    pSampleAllocator->AddRef();
    
    CWseVideoColorspaceConverter *pVideoCSC = new CWseVideoColorspaceConverter();
    wsevp::IWseVP * pWelsVP = NULL;
	EXPECT_EQ(TRUE, pVideoCSC->Init(pWelsVP));

	WseVideoType type[7] = {WseRGB24, WseRGB24Flip, WseBGR24, WseBGR24Flip, WseI420, WseYV12, WseYUY2};
    
    IWseVideoSample *pSrcSample = NULL, *pDstSample = NULL;
    bool bSrcPlanar = false, bDstPlanar = false;
    unsigned long ulSrcCount = 0, ulDstCount = 0;
    unsigned char *pSrc[3] = {NULL}, *pDst[3] = {NULL};
    unsigned long ulSrcStride[3] = {0}, ulDstStride[3] = {0};
	unsigned long width = 1280, height = 720, bufferlen = 0, bufferlenNew = 0;
    WseVideoFormat  srcFormat = {WseI420, 1280, 720, 0.0, 0},
                    dstFormat = {WseI420, 1280, 720, 0.0, 0};
    for(int i=0; i<7; i++)
	{
		EXPECT_EQ(TRUE, pVideoCSC->Init(pWelsVP));
        dstFormat.video_type = type[i];
		EXPECT_EQ(true, pVideoCSC->GetInformation(dstFormat.video_type, width, height, dstFormat.width, dstFormat.height,bufferlenNew));
        pSampleAllocator->GetSample(dstFormat, &pDstSample);
        pDstSample->IsPlanar(&bDstPlanar);
        pDstSample->GetDataPlaneCount(&ulDstCount);
        if(true == bDstPlanar)
        {
            for(int m = 0; m < ulDstCount; m++)
            {
                pDstSample->GetDataPlanePointer(&pDst[m], m);
                pDstSample->GetDataPlaneStride(&ulDstStride[m], m);
            }
        }
        else
        {
            pDstSample->GetDataPointer(&pDst[0]);
            pDstSample->GetDataStride(&ulDstStride[0]);
        }
		for (int j=0; j<7; j++)
		{
            if (i == j) continue;
            
            srcFormat.video_type = type[j];
			EXPECT_EQ(true, pVideoCSC->GetInformation(srcFormat.video_type, width, height, srcFormat.width, srcFormat.height, bufferlen));
            pSampleAllocator->GetSample(srcFormat, &pSrcSample);
            pSrcSample->IsPlanar(&bSrcPlanar);
            pSrcSample->GetDataPlaneCount(&ulSrcCount);
            if(true == bSrcPlanar)
            {
                for(int n = 0; n < ulSrcCount; n++)
                {
                    pSrcSample->GetDataPlanePointer(&pSrc[n], n);
                    pSrcSample->GetDataPlaneStride(&ulSrcStride[n], n);
                }
            }
            else
            {
                pSrcSample->GetDataPointer(&pSrc[0]);
                pSrcSample->GetDataStride(&ulSrcStride[0]);
            }
            
            EXPECT_EQ(WSE_S_OK, pVideoCSC->DoConvert(type[j], srcFormat.width, srcFormat.height, pSrc, ulSrcStride, type[i], pDst, ulDstStride));
			EXPECT_EQ(WSE_S_OK, pVideoCSC->DoConvert(pSrcSample, pDstSample, 0));
            
            pSrcSample->Release();
		}
        
        pDstSample->Release();
	}

	delete pVideoCSC;
    
    pSampleAllocator->Release();
}


/// Test for all kinds of aspect ratio
#define CROPPED_NUM 6
typedef struct WseVideoFormatEx {
	WseVideoFormat format;
	float pixelBytes;
	WseSize croppedSize[CROPPED_NUM];
	unsigned long expectedResult[CROPPED_NUM];
}WseVideoFormatEx;

const WseVideoFormatEx kAllFormatsEx[] = {
	//	(a)input format	(b)pixel bytes (c)expeceted cropped output size (d) expected return result
	{	{WseRGB24, 640, 480, 0, 0},
		3,		
		{{0,0}, {640,480}, {360,480}, {640,360}, {270,480}, {1280,720}},
		{WSE_S_FALSE,WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK,static_cast<unsigned long>(WSE_E_INVALIDARG)}
	},
	{	{WseRGB24, 720, 576, 0, 0},	
		3,		
		{{0,0}, {720,540}, {432,576}, {720,405}, {324,576}, {1280,720}},
		{WSE_S_FALSE,WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK,static_cast<unsigned long>(WSE_E_INVALIDARG)}
	},
	{	{WseI420, 640, 480, 0, 0},		
		1.5,	
		{{0,0}, {640,480}, {360,480}, {640,360}, {270,480}, {1280,720}},
		{WSE_S_FALSE,WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK,static_cast<unsigned long>(WSE_E_INVALIDARG)}
	},
	{	{WseYUY2, 640, 480, 0, 0},		
		2,
		{{0,0}, {640,480}, {360,480}, {640,360}, {270,480}, {1280,720}},
		{WSE_S_FALSE,WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK,static_cast<unsigned long>(WSE_E_INVALIDARG)}
	}
};


#define ASPECT_RATIO_NUM	6
typedef struct WseVideoFormatEx2 {
	WseVideoFormat informat;
	float pixelBytes;
	WseVideoFormat outformats[ASPECT_RATIO_NUM];
	unsigned long expectedResult[ASPECT_RATIO_NUM];
}WseVideoFormatEx2;

const WseVideoFormatEx2 kAllFormatsEx2[] = {
	//	(a)input format	(b)pixel bytes (c)expected output format[color format and aspect ratio]	(d) expected return result
	{	{WseRGB24, 176, 144, 0, 0},	
		3,		
		{{WseI420,0,0,0,0}, {WseI420,4,3,0,0}, {WseI420,3,4,0,0},{WseI420,16,9,0,0},{WseI420,9,16,0,0},{WseI420,176,144,0,0}},
		{static_cast<unsigned long>(WSE_E_INVALIDARG),WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK}
	},
	{	{WseRGB24, 320, 240, 0, 0},	
		3,
		{{WseI420,0,0,0,0}, {WseI420,4,3,0,0}, {WseI420,3,4,0,0},{WseI420,16,9,0,0},{WseI420,9,16,0,0},{WseI420,320,240,0,0}},
		{static_cast<unsigned long>(WSE_E_INVALIDARG),WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK}
	},
	{	{WseYUY2, 320, 240, 0, 0},		
		2,
		{{WseRGB24,0,0,0,0}, {WseRGB24,4,3,0,0}, {WseRGB24,3,4,0,0},{WseRGB24,16,9,0,0},{WseRGB24,9,16,0,0},{WseRGB24,320,240,0,0}},
		{static_cast<unsigned long>(WSE_E_INVALIDARG),WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK}
	},
	{	{WseRGB24, 320, 240, 0, 0},	
		3,
		{{WseYUY2,0,0,0,0}, {WseYUY2,4,3,0,0}, {WseYUY2,3,4,0,0},{WseYUY2,16,9,0,0},{WseYUY2,9,16,0,0},{WseYUY2,320,240,0,0}},
		{static_cast<unsigned long>(WSE_E_INVALIDARG),WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK}
	},
	/*{	{WseRGB24, 320, 240, 0, 0},
		3,
		{{WseBGRA32,0,0,0,0}, {WseBGRA32,4,3,0,0}, {WseBGRA32,3,4,0,0},{WseBGRA32,16,9,0,0},{WseBGRA32,9,16,0,0},{WseBGRA32,320,240,0,0}},
		{static_cast<unsigned long>(WSE_E_INVALIDARG),static_cast<unsigned long>(WSE_E_INVALIDARG),static_cast<unsigned long>(WSE_E_INVALIDARG),static_cast<unsigned long>(WSE_E_INVALIDARG),static_cast<unsigned long>(WSE_E_INVALIDARG),static_cast<unsigned long>(WSE_E_INVALIDARG)}
	},*/
};

TEST_F(CWseVideoProcessingTest, Process_VideoCropper_Further)
{
	CWseVideoSampleAllocator *pSampleAllocator = new CWseVideoSampleAllocator(64);
    pSampleAllocator->AddRef();
	pProcessing.Init(WseRGB24,pSampleAllocator);

	IWseVideoSample *pS = NULL;
	unsigned char* pSrc = NULL;

	WseProcessingParam param;
	memset(&param,0,sizeof(WseProcessingParam));
	for (int iCase=0; iCase < 2; iCase++) {
		pProcessing.m_bLandscape = (iCase & 0x01);

		param.method = CROP_METHORD;
		for (int i=0; i < sizeof(kAllFormatsEx2) / sizeof(WseVideoFormatEx2); i++) {			
			param.in_format = kAllFormatsEx2[i].informat;			
			float pixelBytes = kAllFormatsEx2[i].pixelBytes;

			for (int j=0; j < ASPECT_RATIO_NUM; j++) {
				param.out_format = kAllFormatsEx2[i].outformats[j];
				unsigned long lret = pProcessing.SetProcessParams(&param);
                if (lret != WSE_S_OK) {
                    continue;
                }
				lret = pSampleAllocator->GetSample(param.in_format,&pS);
				if (lret != WSE_S_OK) {
					continue;
				}
				pS->SetVideoFormat(&param.in_format);
				EXPECT_EQ(kAllFormatsEx2[i].expectedResult[j], pProcessing.Process(pS));
				pS->Release();
				pS = NULL;
			}
		}
	}
    
    pSampleAllocator->Release();
}

// This feature is currently not supported. [Disable by Sand]
/*
///???: Downsample donot support width:height=4:3 output
const WseVideoFormatEx2 kAllFormatsEx3[] = {
	//	(a)input format	(b)pixel bytes (c)expected output format[color format and aspect ratio]	(d) expected return result
	{	{WseRGB24, 176, 144, 0, 0},	
		3,		
		{{WseI420,0,0,0,0}, {WseI420,8,6,0,0}, {WseI420,36,48,0,0},{WseI420,16,9,0,0},{WseI420,72,128,0,0},{WseI420,176,144,0,0}},
		{static_cast<unsigned long>(WSE_E_INVALIDARG),WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK}
	},
	{	{WseRGB24, 320, 240, 0, 0},	
		3,
		{{WseI420,0,0,0,0}, {WseI420,8,6,0,0}, {WseI420,36,48,0,0},{WseI420,16,9,0,0},{WseI420,72,128,0,0},{WseI420,320,240,0,0}},
		{static_cast<unsigned long>(WSE_E_INVALIDARG),WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK}
	},
	{	{WseYUY2, 320, 240, 0, 0},		
		2,
		{{WseI420,0,0,0,0}, {WseI420,8,6,0,0}, {WseI420,36,48,0,0},{WseI420,16,9,0,0},{WseI420,72,128,0,0},{WseI420,320,240,0,0}},
		{static_cast<unsigned long>(WSE_E_INVALIDARG),WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK,WSE_S_OK}
	},
	{	{WseRGB24, 320, 240, 0, 0},	
		3,
		{{WseYUY2,0,0,0,0}, {WseI420,8,6,0,0}, {WseYUY2,36,48,0,0},{WseI420,16,9,0,0},{WseYUY2,72,128,0,0},{WseI420,320,240,0,0}},
		{static_cast<unsigned long>(WSE_E_INVALIDARG),WSE_S_OK,static_cast<unsigned long>(WSE_E_INVALIDARG),WSE_S_OK,static_cast<unsigned long>(WSE_E_INVALIDARG),WSE_S_OK}
	},
	{	{WseRGB24, 320, 240, 0, 0},	
		3,
		{{WseI420,0,0,0,0}, {WseBGRA32,8,6,0,0}, {WseI420,36,48,0,0},{WseI420,16,9,0,0},{WseBGRA32,72,128,0,0},{WseI420,320,240,0,0}},
		{static_cast<unsigned long>(WSE_E_INVALIDARG),static_cast<unsigned long>(WSE_E_INVALIDARG),WSE_S_OK,WSE_S_OK,static_cast<unsigned long>(WSE_E_INVALIDARG),WSE_S_OK}
	},
};

TEST_F(CWseVideoProcessingTest, Process_VideoProcess_Further)
{
	CWseVideoSampleAllocator *pSampleAllocator = new CWseVideoSampleAllocator(64);
    pSampleAllocator->AddRef();
	pProcessing.Init(WseRGB24,pSampleAllocator);

	IWseVideoSample *pS = NULL;
	unsigned char* pSrc = NULL;

	WseProcessingParam param;
	memset(&param,0,sizeof(WseProcessingParam));
	for (int iCase=0; iCase < 2; iCase++) {
		pProcessing.m_bLandscape = (iCase & 0x01);

//        for (int iMethod=CROP_METHORD+1; iMethod <= CENTER_METHORD; iMethod++)
		for (int iMethod=CROP_METHORD+1; iMethod <= CROP_METHORD+1; iMethod++)
		{
			param.method = (WseResizeMethod)iMethod;
			for (int i=0; i < sizeof(kAllFormatsEx3) / sizeof(WseVideoFormatEx2); i++) {			
				param.in_format = kAllFormatsEx3[i].informat;				
				float pixelBytes = kAllFormatsEx3[i].pixelBytes;

				for (int j=0; j < ASPECT_RATIO_NUM; j++) {
					param.out_format = kAllFormatsEx3[i].outformats[j];
					unsigned long lret = pProcessing.SetProcessParams(&param);
                    if (lret != WSE_S_OK) {
                        continue;
                    }
					lret = pSampleAllocator->GetSample(param.in_format,&pS);
					if (lret != WSE_S_OK || pS == NULL) {
						continue;
					}
					pS->SetVideoFormat(&param.in_format);
					EXPECT_EQ(kAllFormatsEx3[i].expectedResult[j], pProcessing.Process(pS));
					pS->Release();
					pS = NULL;
				}
			}
		}
	}
 
    pSampleAllocator->Release();
}
*/

///////////////////////////UT for new video process function
void NV12YV12SrcDataFill(unsigned char* pData, unsigned int width, unsigned int height, unsigned int rotation, WseVideoType type)
{
    int i = 0, j = 0;
    unsigned char* pPixel = pData;
    unsigned int uiYSize = width * height;
    unsigned int widthC  = (width>>1);
    unsigned int heightC = (height>>1);
    unsigned char* pPixelC = NULL;
    if (0 == rotation)
    {
        for (i = 0; i < height; i++)   //Y set 1
        {
            memset(pPixel, 1, width);
            pPixel += width;
        }
        pPixel = pData + uiYSize;
        for (i = 0; i < heightC; i++)
        {
            for (j = 0; j < width; j+=2)
            {
                pPixel[j]   = 3;   //V set 3
                pPixel[j+1] = 2;   //U set 2
            }
            pPixel += width;
        }
    }
    else if (90 == rotation)
    {
        for (i = 0; i < height; i++)   //Y set 0 1 2 3.....width-1
        {
            for (j = 0; j < width; j++)
            {
                pPixel[j] = j;
            }
            pPixel += width;
        }
        pPixel = pData + uiYSize;
        if (WseNV12 == type)
        {
            for (i = 0; i < heightC; i++)
            {
                for (j = 0; j < width; j+=2)
                {
                    pPixel[j]   = (j>>1);   //V set 0 1 2 3....width-1
                    pPixel[j+1] = widthC-1-(j>>1);   //U set width-1....3 2 1 0
                }
                pPixel += width;
            }
        }
        else if (WseYV12 == type)
        {
            pPixelC = pPixel + (uiYSize>>2);
            for (i = 0; i < heightC; i++)
            {
                for (j = 0; j < widthC; j++)
                {
                    pPixel[j]   = j;   //V set 0 1 2 3....width-1
                    pPixelC[j] = widthC-1-j;   //U set width-1....3 2 1 0
                }
                pPixel  += widthC;
                pPixelC += widthC;
            }
        }
    }
    else if (180 == rotation)
    {
        for (i = 0; i < height; i++) //Y
        {
            for (j = 0; j < width; j++)
            {
                pPixel[j] = j;
            }
            pPixel += width;
        }
        pPixel = pData + uiYSize;
        if (WseNV12 == type)
        {
            for (i = 0; i < heightC; i++)
            {
                for (j = 0; j < width; j+=2)
                {
                    pPixel[j]   = (j>>1);
                    pPixel[j+1] = widthC-1-(j>>1);
                }
                pPixel += width;
            }
        }
        else if (WseYV12 == type)
        {
            pPixelC = pPixel + (uiYSize>>2);
            for (i = 0; i < heightC; i++)
            {
                for (j = 0; j < widthC; j++)
                {
                    pPixel[j]   = j;
                    pPixelC[j] = widthC-1-j;
                }
                pPixel += widthC;
                pPixelC += widthC;
            }
        }
    }
    else if (270 == rotation)
    {
        for (i = 0; i < height; i++)
        {
            for (j = 0; j < width; j++)
            {
                pPixel[j] = j;
            }
            pPixel += width;
        }
        pPixel = pData + uiYSize;
        if (WseNV12 == type)
        {
            for (i = 0; i < heightC; i++)
            {
                for (j = 0; j < width; j+=2)
                {
                    pPixel[j]   = (j>>1);
                    pPixel[j+1] = widthC-1-(j>>1);
                }
                pPixel += width;
            }
        }
        else if (WseYV12 == type)
        {
            pPixelC = pPixel + (uiYSize>>2);
            for (i = 0; i < heightC; i++)
            {
                for (j = 0; j < widthC; j++)
                {
                    pPixel[j]   = j;
                    pPixelC[j] = widthC-1-j;
                }
                pPixel += widthC;
                pPixelC += widthC;
            }
        }
    }
}
void I420DstDataFill(unsigned char* pData, unsigned int width, unsigned int height, unsigned int rotation)
{
    int i = 0, j = 0;
    unsigned char* pPixel  = pData;
    unsigned char* pPixelC = NULL;
    unsigned int uiYSize = width * height;
    unsigned int widthC  = (width>>1);
    unsigned int heightC = (height>>1);
    if (0 == rotation)
    {
        for (i = 0; i < height; i++) //Y set 1
        {
            memset(pPixel, 1, width);
            pPixel += width;
        }
        pPixel  = pData + uiYSize;
        pPixelC = pPixel + (uiYSize>>2);
        for (i = 0; i < heightC; i++)
        {
            memset(pPixel, 2, widthC);  //U set 2
            memset(pPixelC, 3, widthC); //V set 3
            pPixel  += widthC;
            pPixelC += widthC;
        }
    }
    else if (90 == rotation)
    {
        for (i = 0; i < height; i++)   //Y set 0 1 2 3 ....height-1
        {
            memset(pPixel, i, width);
            pPixel += width;
        }
        pPixel  = pData + uiYSize;
        pPixelC = pPixel + (uiYSize>>2);
        for (i = 0; i < heightC; i++)
        {
            memset(pPixel, heightC-1-i, widthC);  //U set height-1 ....3 2 1 0
            memset(pPixelC, i, widthC);        //V set 0 1 2 3....height-1

            pPixel += widthC;
            pPixelC+= widthC;
        }
    }
    else if (180 == rotation)
    {
        for (i = 0; i < height; i++)   //Y
        {
            for (j = 0; j < width; j++)
            {
                pPixel[j] = width-1-j;
            }
            pPixel += width;
        }
        pPixel  = pData + uiYSize;
        pPixelC = pPixel + (uiYSize>>2);
        for (i = 0; i < heightC; i++)
        {
            for (j = 0; j < widthC; j++)
            {
                pPixel[j]  = j;
                pPixelC[j] = widthC-1-j;
            }
            pPixel  += widthC;
            pPixelC += widthC;
        }
    }
    else if (270 == rotation)
    {
        for (i = 0; i < height; i++)
        {
            memset(pPixel, height-1-i, width);
            pPixel += width;
        }
        pPixel  = pData + uiYSize;
        pPixelC = pPixel + (uiYSize>>2);
        for (i = 0; i < heightC; i++)
        {
            memset(pPixel, i, widthC);
            memset(pPixelC, heightC-1-i, widthC);
            
            pPixel  += widthC;
            pPixelC += widthC;
        }
    }
}

void I420DstDataPadding(unsigned char* pData, unsigned int width, unsigned int height, unsigned int rotation, int paddingLenW, int paddingLenH)
{
    int i = 0, j = 0;
    unsigned char* pPixel  = pData;
    unsigned char* pPixelC = NULL;
    unsigned int uiYSize = width * height;
    unsigned int widthC  = (width>>1);
    unsigned int heightC = (height>>1);
    int paddingLenCW = paddingLenW/2;
    int paddingLenCH = paddingLenH/2;
    if (0 == rotation)
    {
        for (i = 0; i < height; i++) //Y set 0
        {
            memset(pPixel, 0, paddingLenW/2);
            memset(pPixel + width - paddingLenW/2, 0, paddingLenW/2);
            pPixel += width;
        }
        pPixel  = pData + uiYSize;
        pPixelC = pPixel + (uiYSize>>2);
        for (i = 0; i < heightC; i++)
        {
            memset(pPixel, 0x80, paddingLenCW/2);  //U set 128
            memset(pPixel + widthC - paddingLenCW/2, 0x80, paddingLenCW/2);
            memset(pPixelC, 0x80, paddingLenCW/2); //V set 128
            memset(pPixelC + widthC - paddingLenCW/2, 0x80, paddingLenCW/2);
            pPixel  += widthC;
            pPixelC += widthC;
        }
    }
    else if (270 == rotation)
    {
        for (i = 0; i < height; i++) //Y set 0
        {
            memset(pPixel, 0, paddingLenW/2);
            memset(pPixel + paddingLenW/2, height - 1 - i - 1 - paddingLenH/2, width - paddingLenW);
            memset(pPixel + width - paddingLenW/2, 0, paddingLenW/2);
            pPixel += width;
        }
        pPixel  = pData + uiYSize;
        pPixelC = pPixel + (uiYSize>>2);
        for (i = 0; i < heightC; i++)
        {
            memset(pPixel, 0x80, paddingLenCW/2);  //U set 128
            memset(pPixel + paddingLenCW/2, i + 1 - paddingLenCH/2, widthC - paddingLenCW);
            memset(pPixel + widthC - paddingLenCW/2, 0x80, paddingLenCW/2);
            memset(pPixelC, 0x80, paddingLenCW/2); //V set 128
            memset(pPixelC + paddingLenCW/2, heightC - 1 - i - paddingLenCH/2, widthC - paddingLenCW);
            memset(pPixelC + widthC - paddingLenCW/2, 0x80, paddingLenCW/2);
            pPixel  += widthC;
            pPixelC += widthC;
        }
    }
}

void CheckPixelRight(unsigned char* pSrc, unsigned char* pDst, unsigned int width, unsigned int height)
{
    unsigned int i = 0, j = 0;
    unsigned int uiYSize = width * height;
    unsigned int widthC  = (width>>1);
    unsigned int heightC = (height>>1);

    unsigned char* pSrcU = pSrc + uiYSize;
    unsigned char* pSrcV = pSrcU + (uiYSize>>2);
    unsigned char* pDstU = pDst + uiYSize;
    unsigned char* pDstV = pDstU + (uiYSize>>2);
    
    for (i = 0; i < height; i++)
    {
        for (j = 0; j < width; j++)
        {
            EXPECT_EQ(pDst[i*width+j], pSrc[i*width+j]);
        }
    }

    for (i = 0; i < heightC; i++)
    {
        for (j = 0; j < widthC; j++)
        {
            EXPECT_EQ(pDstU[i*widthC+j], pSrcU[i*widthC+j]);
            EXPECT_EQ(pDstV[i*widthC+j], pSrcV[i*widthC+j]);
        }
    }
}

class CWseSampleOutput : public IWseVideoDeliverer,
public CJlUnknown
{
public:
    IMPLEMENT_JLREFERENCE
	BEGIN_QI_HANDLER(CWseSampleOutput)
    WSE_QI_HANDLER(IWseVideoDeliverer)
	END_QI_HANDLER()
    
    CWseSampleOutput()
    {
        
    }
    virtual ~CWseSampleOutput()
    {
        
    }
    
    virtual WSERESULT DeliverImage(IWseVideoSample* pSample)
    {
        WseVideoFormat SrcFormat;
        memset(&SrcFormat, 0, sizeof(WseVideoFormat));
        unsigned char* pSrcData = NULL;
        pSample->GetDataPointer(&pSrcData);
        pSample->GetVideoFormat(&SrcFormat);
        
        CheckPixelRight(pSrcData, m_pDataAnchor, m_dstWidth, m_dstHeight);
        
        EXPECT_EQ(m_pDstFormat->video_type, SrcFormat.video_type);
        EXPECT_EQ(m_pDstFormat->width, SrcFormat.width);
        EXPECT_EQ(m_pDstFormat->height, SrcFormat.height);
        EXPECT_EQ(m_pDstFormat->frame_rate, SrcFormat.frame_rate);
        EXPECT_EQ(m_pDstFormat->time_stamp, SrcFormat.time_stamp);

        return WSE_S_OK;
    }
    virtual WSERESULT DeliverImage(VideoRawDataPack* pVideoPack)
    {
        return WSE_S_OK;
    }
    
    void SetParam(WseVideoFormat* pFormat, unsigned char* pData, unsigned int width, unsigned int height)
    {
        m_pDstFormat = pFormat;
        m_pDataAnchor = pData;
        m_dstWidth = width;
        m_dstHeight = height;
    }
    
public:
    WseVideoFormat* m_pDstFormat;
    unsigned char* m_pDataAnchor;
    unsigned int m_dstWidth;
    unsigned int m_dstHeight;
};


TEST_F(CWseVideoProcessingTest, Process_VideoProcess_crop_csc_rotation)
{
    
    //NV21: Android platform with Rotation
    //---------------case 0: NV21(160*120) + crop(160*90) + rotation(0)   + csc(I420) ->I420(160*90)
    CWseVideoSampleAllocator *pSampleAllocator = new CWseVideoSampleAllocator(64);
    pSampleAllocator->AddRef();
	pProcessing.Init(WseRGB24,pSampleAllocator);
    
    CWseSampleOutput* pSampleOut = new CWseSampleOutput();
    EXPECT_EQ(WSE_S_OK,pProcessing.AppendVideoDeliverer(pSampleOut));

	WseVideoFormat inFormat;
	memset(&inFormat,0,sizeof(WseVideoFormat));
	inFormat.video_type = WseNV21;
	inFormat.width  = 160;
	inFormat.height = 120;
    
	WseVideoFormat outFormat;
	memset(&outFormat,0,sizeof(WseVideoFormat));
	outFormat.video_type = WseI420;
	outFormat.width  = 160;
	outFormat.height = 90;
    
	WseProcessingParam param;
	memset(&param,0,sizeof(WseProcessingParam));
	memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	param.method = CROP_METHORD;
	pProcessing.SetProcessParams(&param);
    
    VideoRawDataPack video_pack;
    memset(&video_pack, 0, sizeof(VideoRawDataPack));
    memcpy(&video_pack.SrcFormat, &inFormat, sizeof(WseVideoFormat));
    unsigned char srcData[160*120*3] = {0};
    unsigned int uiSrcStride[3] = {160, 160, 160};
    
    unsigned char dstData[160*90+(160*90>>1)] = {0};
    
    unsigned int uiSrcYSize = inFormat.width * inFormat.height;
    unsigned int widthYSrc  = inFormat.width;
    unsigned int heightYSrc = inFormat.height;
    
    unsigned int widthYDst  = outFormat.width;
    unsigned int heightYDst = outFormat.height;
    
    NV12YV12SrcDataFill(srcData, widthYSrc, heightYSrc, 0, WseNV12);
    I420DstDataFill(dstData, widthYDst, heightYDst, 0);
    
    video_pack.pSrcData[0] = srcData;
    video_pack.pSrcData[1] = srcData + uiSrcYSize;
    video_pack.pSrcData[2] = video_pack.pSrcData[1] + 1;
    video_pack.uiSrcStride[0] = uiSrcStride[0];
    video_pack.uiSrcStride[1] = uiSrcStride[1];
    video_pack.uiSrcStride[2] = uiSrcStride[2];
    video_pack.uDataLen = uiSrcYSize + (uiSrcYSize>>1);
    pSampleOut->SetParam(&outFormat, dstData, widthYDst, heightYDst);
	EXPECT_EQ(0, pProcessing.Process(&video_pack));
    
    //---------------case 1: NV21(160*120) + crop(160*90) + rotation(90)  + csc(I420) ->I420(90*160)
	memset(&inFormat,0,sizeof(WseVideoFormat));
	inFormat.video_type = WseNV21;
	inFormat.width  = 160;
	inFormat.height = 120;
    memcpy(&video_pack.SrcFormat, &inFormat, sizeof(WseVideoFormat));
    video_pack.rotation = 90;
    
	memset(&outFormat,0,sizeof(WseVideoFormat));
	outFormat.video_type = WseI420;
	outFormat.width  = 90;
	outFormat.height = 160;
    
	memset(&param,0,sizeof(WseProcessingParam));
	memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	param.method = CROP_METHORD;
	pProcessing.SetProcessParams(&param);
    
    NV12YV12SrcDataFill(srcData, widthYSrc, heightYSrc, 90, WseNV12);
    I420DstDataFill(dstData, heightYDst, widthYDst, 90);
    
    video_pack.pSrcData[0] = srcData;
    video_pack.pSrcData[1] = srcData + uiSrcYSize;
    video_pack.pSrcData[2] = video_pack.pSrcData[1] + 1;
    video_pack.uiSrcStride[0] = uiSrcStride[0];
    video_pack.uiSrcStride[1] = uiSrcStride[1];
    video_pack.uiSrcStride[2] = uiSrcStride[2];
    video_pack.uDataLen = uiSrcYSize + (uiSrcYSize>>1);
    
    pSampleOut->SetParam(&outFormat, dstData, heightYDst, widthYDst);
	EXPECT_EQ(0, pProcessing.Process(&video_pack));
    
    //-----------case 2: NV21(160*120) + crop(160*90) + rotation(180) + csc(I420) ->I420(160*90)
    memset(&inFormat,0,sizeof(WseVideoFormat));
	inFormat.video_type = WseNV21;
	inFormat.width  = 160;
	inFormat.height = 120;
    memcpy(&video_pack.SrcFormat, &inFormat, sizeof(WseVideoFormat));
    video_pack.rotation = 180;
    
	memset(&outFormat,0,sizeof(WseVideoFormat));
	outFormat.video_type = WseI420;
	outFormat.width  = 160;
	outFormat.height = 90;
    
	memset(&param,0,sizeof(WseProcessingParam));
	memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	param.method = CROP_METHORD;
	pProcessing.SetProcessParams(&param);
    
    NV12YV12SrcDataFill(srcData, widthYSrc, heightYSrc, 180, WseNV12);
    I420DstDataFill(dstData, widthYDst, heightYDst, 180);
    
    video_pack.pSrcData[0] = srcData;
    video_pack.pSrcData[1] = srcData + uiSrcYSize;
    video_pack.pSrcData[2] = video_pack.pSrcData[1] + 1;
    video_pack.uiSrcStride[0] = uiSrcStride[0];
    video_pack.uiSrcStride[1] = uiSrcStride[1];
    video_pack.uiSrcStride[2] = uiSrcStride[2];
    video_pack.uDataLen = uiSrcYSize + (uiSrcYSize>>1);
    
    pSampleOut->SetParam(&outFormat, dstData, widthYDst, heightYDst);
	EXPECT_EQ(0, pProcessing.Process(&video_pack));
    
    //case 3: NV21(160*120) + crop(160*90) + rotation(270) + csc(I420) ->I420(90*160)
    memset(&inFormat,0,sizeof(WseVideoFormat));
	inFormat.video_type = WseNV21;
	inFormat.width  = 160;
	inFormat.height = 120;
    memcpy(&video_pack.SrcFormat, &inFormat, sizeof(WseVideoFormat));
    video_pack.rotation = 270;
    
	memset(&outFormat,0,sizeof(WseVideoFormat));
	outFormat.video_type = WseI420;
	outFormat.width  = 90;
	outFormat.height = 160;
    
	memset(&param,0,sizeof(WseProcessingParam));
	memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	param.method = CROP_METHORD;
	pProcessing.SetProcessParams(&param);
    
    //dst data filling
    NV12YV12SrcDataFill(srcData, widthYSrc, heightYSrc, 270, WseNV12);
    I420DstDataFill(dstData, heightYDst, widthYDst, 270);
    
    video_pack.pSrcData[0] = srcData;
    video_pack.pSrcData[1] = srcData + uiSrcYSize;
    video_pack.pSrcData[2] = video_pack.pSrcData[1] + 1;
    video_pack.uiSrcStride[0] = uiSrcStride[0];
    video_pack.uiSrcStride[1] = uiSrcStride[1];
    video_pack.uiSrcStride[2] = uiSrcStride[2];
    video_pack.uDataLen = uiSrcYSize + (uiSrcYSize>>1);
    
    pSampleOut->SetParam(&outFormat, dstData, heightYDst, widthYDst);
	EXPECT_EQ(0, pProcessing.Process(&video_pack));

    //YV12: Android platform with Rotation
    //---------------case 0: YV12(160*120) + crop(160*90) + rotation(0)   + csc(I420) ->I420(160*90)
    memset(&inFormat,0,sizeof(WseVideoFormat));
	inFormat.video_type = WseYV12;
	inFormat.width  = 160;
	inFormat.height = 120;
    memcpy(&video_pack.SrcFormat, &inFormat, sizeof(WseVideoFormat));
    video_pack.rotation = 0;
    
	memset(&outFormat,0,sizeof(WseVideoFormat));
	outFormat.video_type = WseI420;
	outFormat.width  = 160;
	outFormat.height = 90;
    
	memset(&param,0,sizeof(WseProcessingParam));
	memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	param.method = CROP_METHORD;
	pProcessing.SetProcessParams(&param);
    
    I420DstDataFill(srcData, widthYSrc, heightYSrc, 0); //src
    I420DstDataFill(dstData, widthYDst, heightYDst, 0); //dst
    video_pack.pSrcData[0] = srcData;
    video_pack.pSrcData[2] = srcData + uiSrcYSize;
    video_pack.pSrcData[1] = video_pack.pSrcData[2] + (uiSrcYSize>>2);
    video_pack.uiSrcStride[0] = uiSrcStride[0];
    video_pack.uiSrcStride[1] =
    video_pack.uiSrcStride[2] = (uiSrcStride[0]>>1);
    video_pack.uDataLen = uiSrcYSize + (uiSrcYSize>>1);
    
    pSampleOut->SetParam(&outFormat, dstData, widthYDst, heightYDst);
	EXPECT_EQ(0, pProcessing.Process(&video_pack));
    
    //---------------case 1: YV12(160*120) + crop(160*90) + rotation(90)  + csc(I420) ->I420(90*160)
    memset(&inFormat,0,sizeof(WseVideoFormat));
	inFormat.video_type = WseYV12;
	inFormat.width  = 160;
	inFormat.height = 120;
    memcpy(&video_pack.SrcFormat, &inFormat, sizeof(WseVideoFormat));
    video_pack.rotation = 90;
    
	memset(&outFormat,0,sizeof(WseVideoFormat));
	outFormat.video_type = WseI420;
	outFormat.width  = 90;
	outFormat.height = 160;
    
	memset(&param,0,sizeof(WseProcessingParam));
	memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	param.method = CROP_METHORD;
	pProcessing.SetProcessParams(&param);
    
    NV12YV12SrcDataFill(srcData, widthYSrc, heightYSrc, 90, WseYV12); //src
    I420DstDataFill(dstData, heightYDst, widthYDst, 90); //dst
    video_pack.pSrcData[0] = srcData;
    video_pack.pSrcData[1] = srcData + uiSrcYSize;
    video_pack.pSrcData[2] = video_pack.pSrcData[1] + (uiSrcYSize>>2);
    video_pack.uiSrcStride[0] = uiSrcStride[0];
    video_pack.uiSrcStride[1] =
    video_pack.uiSrcStride[2] = (uiSrcStride[0]>>1);
    video_pack.uDataLen = uiSrcYSize + (uiSrcYSize>>1);
    
    pSampleOut->SetParam(&outFormat, dstData, heightYDst, widthYDst);
	EXPECT_EQ(0, pProcessing.Process(&video_pack));

    //---------------case 2: YV12(160*120) + crop(160*90) + rotation(180) + csc(I420) ->I420(160*90)
    memset(&inFormat,0,sizeof(WseVideoFormat));
	inFormat.video_type = WseYV12;
	inFormat.width  = 160;
	inFormat.height = 120;
    memcpy(&video_pack.SrcFormat, &inFormat, sizeof(WseVideoFormat));
    video_pack.rotation = 180;
    
	memset(&outFormat,0,sizeof(WseVideoFormat));
	outFormat.video_type = WseI420;
	outFormat.width  = 160;
	outFormat.height = 90;
    
	memset(&param,0,sizeof(WseProcessingParam));
	memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	param.method = CROP_METHORD;
	pProcessing.SetProcessParams(&param);
    
    NV12YV12SrcDataFill(srcData, widthYSrc, heightYSrc, 180, WseYV12); //src
    I420DstDataFill(dstData, widthYDst, heightYDst, 180); //dst
    video_pack.pSrcData[0] = srcData;
    video_pack.pSrcData[1] = srcData + uiSrcYSize;
    video_pack.pSrcData[2] = video_pack.pSrcData[1] + (uiSrcYSize>>2);
    video_pack.uiSrcStride[0] = uiSrcStride[0];
    video_pack.uiSrcStride[1] =
    video_pack.uiSrcStride[2] = (uiSrcStride[0]>>1);
    video_pack.uDataLen = uiSrcYSize + (uiSrcYSize>>1);
    
    pSampleOut->SetParam(&outFormat, dstData, heightYDst, widthYDst);
	EXPECT_EQ(0, pProcessing.Process(&video_pack));
    
    //---------------case 3: YV12(160*120) + crop(160*90) + rotation(270) + csc(I420) ->I420(90*160)
    memset(&inFormat,0,sizeof(WseVideoFormat));
	inFormat.video_type = WseYV12;
	inFormat.width  = 160;
	inFormat.height = 120;
    memcpy(&video_pack.SrcFormat, &inFormat, sizeof(WseVideoFormat));
    video_pack.rotation = 270;
    
	memset(&outFormat,0,sizeof(WseVideoFormat));
	outFormat.video_type = WseI420;
	outFormat.width  = 90;
	outFormat.height = 160;
    
	memset(&param,0,sizeof(WseProcessingParam));
	memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
	memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
	param.method = CROP_METHORD;
	pProcessing.SetProcessParams(&param);
    
    NV12YV12SrcDataFill(srcData, widthYSrc, heightYSrc, 270, WseYV12); //src
    I420DstDataFill(dstData, heightYDst, widthYDst, 270); //dst
    video_pack.pSrcData[0] = srcData;
    video_pack.pSrcData[1] = srcData + uiSrcYSize;
    video_pack.pSrcData[2] = video_pack.pSrcData[1] + (uiSrcYSize>>2);
    video_pack.uiSrcStride[0] = uiSrcStride[0];
    video_pack.uiSrcStride[1] =
    video_pack.uiSrcStride[2] = (uiSrcStride[0]>>1);
    video_pack.uDataLen = uiSrcYSize + (uiSrcYSize>>1);
    
    pSampleOut->SetParam(&outFormat, dstData, heightYDst, widthYDst);
	EXPECT_EQ(0, pProcessing.Process(&video_pack));
    
    EXPECT_EQ(WSE_S_OK,pProcessing.RemoveVideoDeliverer(pSampleOut));
    
    pSampleAllocator->Release();
    
    //NV12: iOS platform without Rotation
    //case 1: NV12(160*120) + crop(160*90) + rotation(0) + csc(I420) ->I420(160*90)
    
    
    //BGRA32: iOS platform without Rotation
    //case 1: BGRA32(160*120) + crop(160*90) + rotation(0) + csc(I420) ->I420(160*90)
    
    //other video format such as ARGB32 BGRA32 YUY2 (Mac) and BGR24Flip BGR24 I420 YV12 YUY2 (Win).
    //crop(Yes) + rotation(No) + copy (no csc as these format for Mac or Win render)
    //case 1: BGRA32(160*120) + crop(160*90) + rotation(0) + copy ->BRGA32(160*90)
    //case 2: BGR24(160*120)  + crop(160*90) + rotation(0) + copy ->BGR23(160*90)
    //case 3: YUY2(160*120)   + crop(160*90) + rotation(0) + copy ->YUY2(160*90)
    
}

TEST_F(CWseVideoProcessingTest, Process_VideoProcess_force_landscape)
{
    //NV21: iOS platform without Rotation
    //---------------case 0: NV21(192*144) + crop(192*108) + rotation(0)   + csc(I420) ->I420(192*108)
    CWseVideoSampleAllocator *pSampleAllocator = new CWseVideoSampleAllocator(64);
    pSampleAllocator->AddRef();
    pProcessing.Init(WseRGB24,pSampleAllocator);
    bool bLandscape = true;
    pProcessing.SetOption(WSE_PROCESSING_OPTION_FORCE_LANDSCAPE, &bLandscape, sizeof(bLandscape));

    CWseSampleOutput* pSampleOut = new CWseSampleOutput();
    EXPECT_EQ(WSE_S_OK,pProcessing.AppendVideoDeliverer(pSampleOut));

    WseVideoFormat inFormat;
    memset(&inFormat,0,sizeof(WseVideoFormat));
    inFormat.video_type = WseNV21;
    inFormat.width  = 192;
    inFormat.height = 144;

    WseVideoFormat outFormat;
    memset(&outFormat,0,sizeof(WseVideoFormat));
    outFormat.video_type = WseI420;
    outFormat.width  = 192;
    outFormat.height = 108;

    WseProcessingParam param;
    memset(&param,0,sizeof(WseProcessingParam));
    memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
    memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
    param.method = CROP_METHORD;
    pProcessing.SetProcessParams(&param);

    VideoRawDataPack video_pack;
    memset(&video_pack, 0, sizeof(VideoRawDataPack));
    memcpy(&video_pack.SrcFormat, &inFormat, sizeof(WseVideoFormat));
    unsigned char srcData[192*144*3] = {0};
    unsigned int uiSrcStride[3] = {192, 192, 192};

    unsigned char dstData[192*144+(192*144>>1)] = {0};

    unsigned int uiSrcYSize = inFormat.width * inFormat.height;
    unsigned int widthYSrc  = inFormat.width;
    unsigned int heightYSrc = inFormat.height;

    unsigned int widthYDst  = outFormat.width;
    unsigned int heightYDst = outFormat.height;

    NV12YV12SrcDataFill(srcData, widthYSrc, heightYSrc, 0, WseNV12);
    I420DstDataFill(dstData, widthYDst, heightYDst, 0);

    video_pack.pSrcData[0] = srcData;
    video_pack.pSrcData[1] = srcData + uiSrcYSize;
    video_pack.pSrcData[2] = video_pack.pSrcData[1] + 1;
    video_pack.uiSrcStride[0] = uiSrcStride[0];
    video_pack.uiSrcStride[1] = uiSrcStride[1];
    video_pack.uiSrcStride[2] = uiSrcStride[2];
    video_pack.uDataLen = uiSrcYSize + (uiSrcYSize>>1);
    pSampleOut->SetParam(&outFormat, dstData, widthYDst, heightYDst);
    EXPECT_EQ(0, pProcessing.Process(&video_pack));

    //case 1: NV21(144*192) + crop(144*108) + rotation(0) + csc(I420) ->I420(192*108)
    memset(&inFormat,0,sizeof(WseVideoFormat));
    inFormat.video_type = WseNV21;
    inFormat.width  = 144;
    inFormat.height = 192;
    memcpy(&video_pack.SrcFormat, &inFormat, sizeof(WseVideoFormat));
    video_pack.rotation = 0;

    memset(&outFormat,0,sizeof(WseVideoFormat));
    outFormat.video_type = WseI420;
    outFormat.width  = 192;
    outFormat.height = 108;

    memset(&param,0,sizeof(WseProcessingParam));
    memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
    memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
    param.method = CROP_METHORD;
    pProcessing.SetProcessParams(&param);

    //dst data filling
    uiSrcYSize = inFormat.width * inFormat.height;
    widthYSrc  = inFormat.width;
    heightYSrc = inFormat.height;

    widthYDst  = outFormat.width;
    heightYDst = outFormat.height;
    NV12YV12SrcDataFill(srcData, widthYSrc, heightYSrc, 0, WseNV12);
    I420DstDataFill(dstData, widthYDst, heightYDst, 0);
    I420DstDataPadding(dstData, widthYDst, heightYDst, 0, widthYDst-widthYSrc, 0);

    video_pack.pSrcData[0] = srcData;
    video_pack.pSrcData[1] = srcData + uiSrcYSize;
    video_pack.pSrcData[2] = video_pack.pSrcData[1] + 1;
    video_pack.uiSrcStride[0] = inFormat.width;
    video_pack.uiSrcStride[1] = inFormat.width;
    video_pack.uiSrcStride[2] = inFormat.width;
    video_pack.uDataLen = uiSrcYSize + (uiSrcYSize>>1);

    pSampleOut->SetParam(&outFormat, dstData, widthYDst, heightYDst);
    EXPECT_EQ(0, pProcessing.Process(&video_pack));

    //NV21: android platform with Rotation
    //case 0: NV21(176*144) + crop(160*90) + rotation(0) + csc(I420) ->I420(160*90)
    memset(&inFormat,0,sizeof(WseVideoFormat));
    inFormat.video_type = WseNV21;
    inFormat.width  = 176;
    inFormat.height = 144;
    memcpy(&video_pack.SrcFormat, &inFormat, sizeof(WseVideoFormat));
    video_pack.rotation = 0;

    memset(&outFormat,0,sizeof(WseVideoFormat));
    outFormat.video_type = WseI420;
    outFormat.width  = 160;
    outFormat.height = 90;

    memset(&param,0,sizeof(WseProcessingParam));
    memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
    memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
    param.method = CROP_METHORD;
    pProcessing.SetProcessParams(&param);

    //dst data filling
    uiSrcYSize = inFormat.width * inFormat.height;
    widthYSrc  = inFormat.width;
    heightYSrc = inFormat.height;

    widthYDst  = outFormat.width;
    heightYDst = outFormat.height;
    NV12YV12SrcDataFill(srcData, widthYSrc, heightYSrc, 0, WseNV12);
    I420DstDataFill(dstData, widthYDst, heightYDst, 0);

    video_pack.pSrcData[0] = srcData;
    video_pack.pSrcData[1] = srcData + uiSrcYSize;
    video_pack.pSrcData[2] = video_pack.pSrcData[1] + 1;
    video_pack.uiSrcStride[0] = inFormat.width;
    video_pack.uiSrcStride[1] = inFormat.width;
    video_pack.uiSrcStride[2] = inFormat.width;
    video_pack.uDataLen = uiSrcYSize + (uiSrcYSize>>1);

    pSampleOut->SetParam(&outFormat, dstData, widthYDst, heightYDst);
    EXPECT_EQ(0, pProcessing.Process(&video_pack));

    //case 1: NV21(176*144) + crop(90*144) + rotation(270) + csc(I420) ->I420(160*90)
    memset(&inFormat,0,sizeof(WseVideoFormat));
    inFormat.video_type = WseNV21;
    inFormat.width  = 176;
    inFormat.height = 144;
    memcpy(&video_pack.SrcFormat, &inFormat, sizeof(WseVideoFormat));
    video_pack.rotation = 270;

    memset(&outFormat,0,sizeof(WseVideoFormat));
    outFormat.video_type = WseI420;
    outFormat.width  = 160;
    outFormat.height = 90;

    memset(&param,0,sizeof(WseProcessingParam));
    memcpy(&param.out_format,&outFormat,sizeof(WseVideoFormat));
    memcpy(&param.in_format, &inFormat, sizeof(WseVideoFormat));
    param.method = CROP_METHORD;
    pProcessing.SetProcessParams(&param);

    //dst data filling
    uiSrcYSize = inFormat.width * inFormat.height;
    widthYSrc  = inFormat.width;
    heightYSrc = inFormat.height;

    widthYDst  = outFormat.width;
    heightYDst = outFormat.height;
    NV12YV12SrcDataFill(srcData, widthYSrc, heightYSrc, 270, WseNV12);
    I420DstDataFill(dstData, widthYDst, heightYDst, 270);
    I420DstDataPadding(dstData, widthYDst, heightYDst, 270, widthYDst-heightYSrc, heightYDst-widthYSrc);

    video_pack.pSrcData[0] = srcData;
    video_pack.pSrcData[1] = srcData + uiSrcYSize;
    video_pack.pSrcData[2] = video_pack.pSrcData[1] + 1;
    video_pack.uiSrcStride[0] = inFormat.width;
    video_pack.uiSrcStride[1] = inFormat.width;
    video_pack.uiSrcStride[2] = inFormat.width;
    video_pack.uDataLen = uiSrcYSize + (uiSrcYSize>>1);

    pSampleOut->SetParam(&outFormat, dstData, widthYDst, heightYDst);
    EXPECT_EQ(0, pProcessing.Process(&video_pack));
    
    pSampleAllocator->Release();
}

