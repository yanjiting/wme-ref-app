
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WseVideoSample.h"
#include "SvcClientEngineMock.h"

namespace shark {
    extern unsigned long CalcBufferSize(WseVideoFormat &bufferFormat);
}

class CWseVideoSampleTest : public testing::Test
{
public:
	CWseVideoSampleTest()
	{
        pSampleAllocator = new CWseVideoSampleAllocator(64);
        pSampleAllocator->AddRef();
	}
    
	virtual ~CWseVideoSampleTest()
	{
        if(pSampleAllocator)
            pSampleAllocator->Release();
        pSampleAllocator = NULL;
	}
    
	virtual void SetUp()
	{
        
	}
    
	virtual void TearDown()
	{
		
	}
    
public:
	CWseVideoSampleAllocator *pSampleAllocator;
};

TEST_F(CWseVideoSampleTest, GetSize)
{
    CWseVideoSample *pSample = NULL;
    unsigned long ulSize=0, *pSize=NULL;
    unsigned long ulReservedSize = 0;
    unsigned long ulBufferSize=0;
    
    WseVideoFormat format = {WseI420, 160, 90, 0.0, 0};
    ulBufferSize = CalcBufferSize(format);
    pSample = pSampleAllocator->GetSample(format);
    EXPECT_EQ(WSE_E_INVALIDARG, pSample->GetSize(pSize));
    EXPECT_EQ(WSE_S_OK, pSample->GetSize(&ulSize));
    EXPECT_EQ(WSE_S_OK, pSample->GetReservedSize(ulReservedSize));
    EXPECT_EQ(ulBufferSize, (ulSize - ulReservedSize));
    pSample->Release();
    
    format.width = 320;
    format.height = 180;
    ulBufferSize = CalcBufferSize(format);
    pSample = pSampleAllocator->GetSample(format);
    EXPECT_EQ(WSE_S_OK, pSample->GetSize(&ulSize));
    EXPECT_EQ(WSE_S_OK, pSample->GetReservedSize(ulReservedSize));
    EXPECT_EQ(ulBufferSize, (ulSize - ulReservedSize));
    pSample->Release();
}

TEST_F(CWseVideoSampleTest, GetPointer)
{
    CWseVideoSample *pSample = NULL;
    unsigned char *pPointer = NULL;
    unsigned char **ppPointer = NULL;
    
    WseVideoFormat format = {WseI420, 160, 90, 0.0, 0};
    pSample = pSampleAllocator->GetSample(format);
    EXPECT_EQ(WSE_E_INVALIDARG, pSample->GetPointer(ppPointer));
    ppPointer = &pPointer;
    EXPECT_EQ(WSE_S_OK, pSample->GetPointer(ppPointer));
    pSample->Release();
}

TEST_F(CWseVideoSampleTest, SetGetDataPointer)
{
    CWseVideoSample *pSample = NULL;
    unsigned char *pPointer = NULL;
    unsigned char **ppPointer = NULL;
    
    WseVideoFormat format = {WseI420, 160, 90, 0.0, 0};
    pSample = pSampleAllocator->GetSample(format);
    EXPECT_EQ(WSE_E_INVALIDARG, pSample->GetDataPointer(ppPointer));
    ppPointer = &pPointer;
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPointer(ppPointer));
    
    unsigned char *pDataPointer = pPointer;
    pDataPointer -= 64;
    EXPECT_EQ(WSE_E_INVALIDARG, pSample->SetDataPointer(pDataPointer));
    pDataPointer += 64;
    EXPECT_EQ(WSE_S_OK, pSample->SetDataPointer(pDataPointer));
    EXPECT_EQ(pPointer, pDataPointer);
    pDataPointer += 50000;
    EXPECT_EQ(WSE_E_INVALIDARG, pSample->SetDataPointer(pDataPointer));
    pSample->Release();
}

TEST_F(CWseVideoSampleTest, SetGetDataLength)
{
    CWseVideoSample *pSample = NULL;
    unsigned long ulDataLength=0, *pDataLength=NULL;
    unsigned long ulBufferSize=0;
    
    WseVideoFormat format = {WseI420, 160, 90, 0.0, 0};
    ulBufferSize = CalcBufferSize(format);
    pSample = pSampleAllocator->GetSample(format);
    EXPECT_EQ(WSE_E_INVALIDARG, pSample->GetDataLength(pDataLength));
    EXPECT_EQ(WSE_S_OK, pSample->GetDataLength(&ulDataLength));
    EXPECT_EQ(ulBufferSize, ulDataLength);
    pSample->Release();
    
    pDataLength = &ulDataLength;
    format.width = 320;
    format.height = 180;
    ulBufferSize = CalcBufferSize(format);
    pSample = pSampleAllocator->GetSample(format);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataLength(pDataLength));
    EXPECT_EQ(ulBufferSize, ulDataLength);
    pSample->Release();
}

TEST_F(CWseVideoSampleTest, DataPlanar)
{
    CWseVideoSample *pSample = NULL;
    unsigned char *pPointer = NULL;
    unsigned char **ppPointer = NULL;
    
    WseVideoFormat format = {WseI420, 160, 90, 0.0, 0};
    pSample = pSampleAllocator->GetSample(format);
    EXPECT_EQ(WSE_E_INVALIDARG, pSample->GetDataPlanePointer(ppPointer, 0));
    ppPointer = &pPointer;
    EXPECT_EQ(WSE_E_INVALIDARG, pSample->GetDataPlanePointer(ppPointer, 4));
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlanePointer(ppPointer, 0));
    EXPECT_EQ(true, pPointer != NULL);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPointer(ppPointer));
    EXPECT_EQ(true, pPointer != NULL);
    
/*
    WseVideoFormat format;
    format.video_type = WseI420;
    format.width = 64;
    format.height = 16;
    format.frame_rate = 10.0f;
    format.time_stamp = 0;
    EXPECT_EQ(WSE_S_OK, pSample->SetVideoFormat(&format));
    
    unsigned char *pPlanarPointer = NULL;
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPointer(&pPointer));
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlanePointer(&pPlanarPointer, 0));
    EXPECT_EQ(pPointer, pPlanarPointer);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlanePointer(&pPlanarPointer, 1));
    EXPECT_EQ(pPointer+1024, pPlanarPointer);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlanePointer(&pPlanarPointer, 2));
    EXPECT_EQ(pPointer+1280, pPlanarPointer);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlanePointer(&pPlanarPointer, 3));
    EXPECT_EQ(NULL, pPlanarPointer);
    unsigned long uiStride=0;
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlaneStride(&uiStride, 0));
    EXPECT_EQ(64, uiStride);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlaneStride(&uiStride, 1));
    EXPECT_EQ(32, uiStride);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlaneStride(&uiStride, 2));
    EXPECT_EQ(32, uiStride);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlaneStride(&uiStride, 3));
    EXPECT_EQ(0, uiStride);
    
    format.video_type = WseRGB24;
    EXPECT_EQ(WSE_S_OK, pSample->SetVideoFormat(&format));
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlanePointer(&pPlanarPointer, 0));
    EXPECT_EQ(pPointer, pPlanarPointer);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlanePointer(&pPlanarPointer, 1));
    EXPECT_EQ(pPointer+1024, pPlanarPointer);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlanePointer(&pPlanarPointer, 2));
    EXPECT_EQ(pPointer+2048, pPlanarPointer);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlanePointer(&pPlanarPointer, 3));
    EXPECT_EQ(NULL, pPlanarPointer);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlaneStride(&uiStride, 0));
    EXPECT_EQ(64, uiStride);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlaneStride(&uiStride, 1));
    EXPECT_EQ(64, uiStride);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlaneStride(&uiStride, 2));
    EXPECT_EQ(64, uiStride);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlaneStride(&uiStride, 3));
    EXPECT_EQ(0, uiStride);
    
    format.video_type = WseRGBA32;
    EXPECT_EQ(WSE_S_OK, pSample->SetVideoFormat(&format));
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlanePointer(&pPlanarPointer, 0));
    EXPECT_EQ(pPointer, pPlanarPointer);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlanePointer(&pPlanarPointer, 1));
    EXPECT_EQ(pPointer+1024, pPlanarPointer);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlanePointer(&pPlanarPointer, 2));
    EXPECT_EQ(pPointer+2048, pPlanarPointer);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlanePointer(&pPlanarPointer, 3));
    EXPECT_EQ(pPointer+3072, pPlanarPointer);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlaneStride(&uiStride, 0));
    EXPECT_EQ(64, uiStride);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlaneStride(&uiStride, 1));
    EXPECT_EQ(64, uiStride);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlaneStride(&uiStride, 2));
    EXPECT_EQ(64, uiStride);
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlaneStride(&uiStride, 3));
    EXPECT_EQ(64, uiStride);
*/
    
    
    unsigned char *pPlanarPointer = pPointer;
    pPlanarPointer -= 64;
    EXPECT_EQ(WSE_E_INVALIDARG, pSample->SetDataPlanePointer(pPlanarPointer,0));
    pPlanarPointer += 64;
    EXPECT_EQ(WSE_E_INVALIDARG, pSample->SetDataPlanePointer(pPlanarPointer,-1));
    EXPECT_EQ(WSE_S_OK, pSample->SetDataPlanePointer(pPlanarPointer,0));
    pPlanarPointer = NULL;
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlanePointer(&pPlanarPointer,0));
    EXPECT_EQ(pPointer, pPlanarPointer);
    pPlanarPointer = pPointer + 50000;
    EXPECT_EQ(WSE_E_INVALIDARG, pSample->SetDataPlanePointer(pPlanarPointer,0));
    
    unsigned long ulStride = 0;
    EXPECT_EQ(WSE_S_OK, pSample->GetDataPlaneStride(&ulStride, 0));
    EXPECT_EQ(true, ulStride != 0);
    pSample->Release();
}

TEST_F(CWseVideoSampleTest, SetGetVideoFormat)
{
    WseVideoFormat format = {WseI420, 160, 120, 10.0f, 0};
    CWseVideoSample *pSample = NULL;
    pSample = pSampleAllocator->GetSample(format);
    
    EXPECT_EQ(WSE_E_INVALIDARG, pSample->SetVideoFormat(NULL));
    format.height = 90;
    EXPECT_EQ(WSE_S_OK, pSample->SetVideoFormat(&format));
    format.video_type = WseBGRA32;
    EXPECT_NE(WSE_S_OK, pSample->SetVideoFormat(&format));
    
    EXPECT_EQ(WSE_E_INVALIDARG, pSample->GetVideoFormat(NULL));
    WseVideoFormat format2;
    EXPECT_EQ(WSE_S_OK, pSample->GetVideoFormat(&format2));
    
    pSample->Release();
}
