
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WseCommonTypes.h"
#include "WseVideoMediaInfo.h"
#include "WseVideoMediaInfoGetter.h"
#include "WseH264MediaInfo.h"

using namespace shark;

class CWseVideoMediaInfoTest : public testing::Test
{
public:
	CWseVideoMediaInfoTest()
	{
		pGetter = new CWseVideoMediaInfoGetter();
		if(pGetter)
		{
			pGetter->AddRef();
		}
	}

	virtual ~CWseVideoMediaInfoTest()
	{
		if(pGetter)
		{
			pGetter->Release();
			pGetter = NULL;
		}
	}

	virtual void SetUp()
	{

	}

	virtual void TearDown()
	{

	}

public:
	CWseVideoMediaInfoGetter *pGetter;
};

TEST_F(CWseVideoMediaInfoTest, CreateVideoMediaInfoGetter)
{
	int iMinIndex = 1, iMaxIndex = 4;
	IWseVideoMediaInfoGetter *pTempGetter = NULL;

	EXPECT_EQ(WSE_S_OK, CreateVideoMediaInfoGetter(1280, 720, 30, iMaxIndex, &pTempGetter));
	SAFE_RELEASE(pTempGetter);

	EXPECT_NE(WSE_S_OK, CreateVideoMediaInfoGetter(-1280, 720, 30, iMaxIndex, &pTempGetter));
	SAFE_RELEASE(pTempGetter);

	EXPECT_NE(WSE_S_OK, CreateVideoMediaInfoGetter(1280, -720, 30, iMaxIndex, &pTempGetter));
	SAFE_RELEASE(pTempGetter);

	EXPECT_NE(WSE_S_OK, CreateVideoMediaInfoGetter(1280, 720, -30, iMaxIndex, &pTempGetter));
	SAFE_RELEASE(pTempGetter);

	EXPECT_NE(WSE_S_OK, CreateVideoMediaInfoGetter(1280, 720, 30, iMinIndex - 1, &pTempGetter));
	SAFE_RELEASE(pTempGetter);

	EXPECT_EQ(WSE_E_INVALIDARG, CreateVideoMediaInfoGetter(1280, 720, 30, iMaxIndex, NULL));
}

TEST_F(CWseVideoMediaInfoTest, Init)
{
	int iMinIndex = 1, iMaxIndex = 4;
	int i = 0;
	
	EXPECT_EQ(FALSE, pGetter->Init(1280, 720, 30, iMinIndex - 1));
	EXPECT_EQ(0, pGetter->Uninit());

//	EXPECT_EQ(FALSE, pGetter->Init(1280, 720, iMaxIndex + 1));
//	EXPECT_EQ(0, pGetter->Uninit());

	EXPECT_EQ(FALSE, pGetter->Init(-1280, 720, 30, iMaxIndex));
	EXPECT_EQ(0, pGetter->Uninit());

	EXPECT_EQ(FALSE, pGetter->Init(1280, -720, 30, iMaxIndex));
	EXPECT_EQ(0, pGetter->Uninit());

	EXPECT_EQ(FALSE, pGetter->Init(1280, 720, -30, iMaxIndex));
	EXPECT_EQ(0, pGetter->Uninit());

	for(i = iMinIndex; i <= iMaxIndex; i++)
	{
		EXPECT_EQ(TRUE, pGetter->Init(1280, 720, 30, i));
		EXPECT_EQ(0, pGetter->Uninit());
	}

	for(i = iMinIndex; i <= iMaxIndex; i++)
	{
		EXPECT_EQ(TRUE, pGetter->Init(640, 360, 30, i));
		EXPECT_EQ(0, pGetter->Uninit());
	}

	for(i = iMinIndex; i <= iMaxIndex; i++)
	{
		EXPECT_EQ(TRUE, pGetter->Init(320, 180, 30, i));
		EXPECT_EQ(0, pGetter->Uninit());
	}

	for(i = iMinIndex; i <= iMaxIndex; i++)
	{
		EXPECT_EQ(TRUE, pGetter->Init(160, 90, 30, i));
		EXPECT_EQ(0, pGetter->Uninit());
	}
}

TEST_F(CWseVideoMediaInfoTest, SetMaxResolution)
{
	int iMinIndex = 1, iMaxIndex = 4;

	EXPECT_EQ(TRUE, pGetter->Init(160, 90, 30, iMinIndex));
	
	EXPECT_EQ(WSE_S_OK, pGetter->SetMaxResolution(1280, 720));
	EXPECT_EQ(WSE_S_OK, pGetter->SetMaxResolution(640, 360));
	EXPECT_EQ(WSE_S_OK, pGetter->SetMaxResolution(320, 180));
	EXPECT_EQ(WSE_S_OK, pGetter->SetMaxResolution(160, 90));

	EXPECT_EQ(WSE_E_INVALIDARG, pGetter->SetMaxResolution(-1280, 720));
	EXPECT_EQ(WSE_E_INVALIDARG, pGetter->SetMaxResolution(1280, -720));
}

TEST_F(CWseVideoMediaInfoTest, SetMaxLayerNumber)
{
	int iMinIndex = 1, iMaxIndex = 4;
	int i = 0;

	EXPECT_EQ(TRUE, pGetter->Init(1280, 720, 30, iMaxIndex));

	for(i = iMinIndex; i <= iMaxIndex; i++)
	{
		EXPECT_EQ(WSE_S_OK, pGetter->SetMaxLayerNumber(i));
	}

	EXPECT_EQ(WSE_E_INVALIDARG, pGetter->SetMaxLayerNumber(iMinIndex - 1));
//	EXPECT_EQ(WSE_E_INVALIDARG, pGetter->SetMaxLayerNumber(iMaxIndex + 1));
}

TEST_F(CWseVideoMediaInfoTest, SetMaxFrameRate)
{
	int iMinIndex = 1, iMaxIndex = 4;
	int i = 0;

	EXPECT_EQ(TRUE, pGetter->Init(1280, 720, 15, iMaxIndex));

	EXPECT_EQ(WSE_S_OK, pGetter->SetMaxFrameRate(30));
	EXPECT_EQ(WSE_E_INVALIDARG, pGetter->SetMaxFrameRate(-30));
}

TEST_F(CWseVideoMediaInfoTest, GetMediaInfo)
{
	int iMinIndex = 1, iMaxIndex = 4;
	int iMaxCodecNumber = 2, iMaxCapabilitiesNumber = 0;
	int i = 0, j = 0;
	
	EXPECT_EQ(TRUE, pGetter->Init(1280, 720, 30, iMaxIndex));
	
	IWseVideoMediaInfo *pMediaInfo = NULL;
	WseVideoCodecType eCodecType = videoCodecH264AVC;
	WseVideoMediaCapability stMediaCapabilities = {0};
	for(i = 0; ; i++)
	{
		if(i >= iMaxCodecNumber)
		{
			EXPECT_EQ(WSE_E_INVALIDARG, pGetter->GetMediaInfo(i, &pMediaInfo));
			break;
		}
		EXPECT_EQ(WSE_S_OK, pGetter->GetMediaInfo(i, &pMediaInfo));
		EXPECT_EQ(WSE_S_OK, pMediaInfo->GetCodecType(eCodecType));
		iMaxCapabilitiesNumber = 4;
		if(videoCodecH264SVC == eCodecType)
		{
			iMaxCapabilitiesNumber = iMaxIndex;		
		}
		for(j = 0; ; j++)
		{
			if(j >= iMaxCapabilitiesNumber)
			{
				EXPECT_EQ(WSE_E_INVALIDARG, pMediaInfo->GetMediaCapabilities(j, &stMediaCapabilities));
				break;
			}
			EXPECT_EQ(WSE_S_OK, pMediaInfo->GetMediaCapabilities(j, &stMediaCapabilities));
		}

		EXPECT_EQ(WSE_E_INVALIDARG, pMediaInfo->GetMediaCapabilities(-1, &stMediaCapabilities));
		EXPECT_EQ(WSE_E_INVALIDARG, pMediaInfo->GetMediaCapabilities(0, NULL));

		SAFE_RELEASE(pMediaInfo);
	}

	EXPECT_EQ(WSE_S_OK, pGetter->SetMaxFrameRate(15));
	for(i = 0; ; i++)
	{
		if(WSE_S_OK != pGetter->GetMediaInfo(i, &pMediaInfo))
		{
			break;
		}
		EXPECT_EQ(WSE_S_OK, pGetter->GetMediaInfo(i, &pMediaInfo));
		SAFE_RELEASE(pMediaInfo);
	}


	EXPECT_EQ(WSE_E_INVALIDARG, pGetter->GetMediaInfo(-1, &pMediaInfo));
	EXPECT_EQ(WSE_E_INVALIDARG, pGetter->GetMediaInfo(0, NULL));
}

TEST_F(CWseVideoMediaInfoTest, QueryInterface)
{
	int iMinIndex = 1, iMaxIndex = 4;
	EXPECT_EQ(TRUE, pGetter->Init(1280, 720, 30, iMaxIndex));

	IWseVideoMediaInfoGetter *pQueryGetter = NULL;

	EXPECT_EQ(WSE_S_OK, pGetter->QueryInterface(WSEIID_IWseVideoMediaInfoGetter, (void**)&pQueryGetter));
	SAFE_RELEASE(pQueryGetter);

	EXPECT_EQ(WSE_S_OK, pGetter->QueryInterface(JLIID_IJlUnknown, (void**)&pQueryGetter));
	SAFE_RELEASE(pQueryGetter);

	EXPECT_NE(WSE_S_OK, pGetter->QueryInterface(WSEIID_IWseVideoMediaInfo, (void**)&pQueryGetter));
	SAFE_RELEASE(pQueryGetter);

	EXPECT_NE(WSE_S_OK, pGetter->QueryInterface(WSEIID_IWseVideoMediaInfoGetter, NULL));

	IWseVideoMediaInfo *pMediaInfo = NULL;
	IWseVideoMediaInfo *pQueryMediaInfo = NULL;
	for(int i = 0; ; i++)
	{
		if(WSE_S_OK != pGetter->GetMediaInfo(i, &pMediaInfo))
		{
			break;
		}
		EXPECT_EQ(WSE_S_OK, pGetter->GetMediaInfo(i, &pMediaInfo));
		
		EXPECT_EQ(WSE_S_OK, pMediaInfo->QueryInterface(WSEIID_IWseVideoMediaInfo, (void**)&pQueryMediaInfo));
		SAFE_RELEASE(pQueryMediaInfo);

		EXPECT_EQ(WSE_S_OK, pMediaInfo->QueryInterface(JLIID_IJlUnknown, (void**)&pQueryMediaInfo));
		SAFE_RELEASE(pQueryMediaInfo);

		EXPECT_NE(WSE_S_OK, pMediaInfo->QueryInterface(WSEIID_IWseVideoMediaInfoGetter, (void**)&pQueryMediaInfo));
		SAFE_RELEASE(pQueryMediaInfo);

		EXPECT_NE(WSE_S_OK, pMediaInfo->QueryInterface(WSEIID_IWseVideoMediaInfo, NULL));
		
		SAFE_RELEASE(pMediaInfo);
	}
}

TEST_F(CWseVideoMediaInfoTest, GetH264SpatialLayerInfo)
{
	unsigned int resolutionArray[][2] = {1920, 1080,
										1280, 720,
										640, 360,
										320, 180,
										160, 90,
										640, 480,
										320, 240,
										352, 288,
										176, 144
										};
	int resolutionArrayNumber = sizeof(resolutionArray) / sizeof(unsigned int [2]);

	float frameRateArray[] =  {30, 25, 20, 15, 10, 5};
	int frameRateArrayNumber = sizeof(frameRateArray) / sizeof(float);

	WseH264SpatialLayerType typeArray[2] = {WSE_H264SL_SVC, WSE_H264SL_AVC};
	WseH264SpatialLayerFpsMode fpsModeArray[2] = {WSE_H264SL_FPS_FULL_MODE, WSE_H264SL_FPS_HALF_MODE};

	int iMinIndex = 1, iMaxIndex = 5;
	int i = 0, j = 0, k = 0, l = 0, m = 0, n= 0;
	EXPECT_EQ(TRUE, pGetter->Init(1280, 720, 30, iMaxIndex));

	int layerNumber = iMaxIndex;
	WseH264SpatialLayerInfo stLayerInfoArray[5];
	WseH264SpatialLayerParam stMaxLayerParam = {WSE_H264SL_SVC, 1280, 720, 30, WSE_H264SL_FPS_HALF_MODE};
	for(i = 0; i < resolutionArrayNumber; i++)
	{
		stMaxLayerParam.width = resolutionArray[i][0];
		stMaxLayerParam.height = resolutionArray[i][1];

		for(j = 0; j < frameRateArrayNumber; j++)
		{
			stMaxLayerParam.frame_rate = frameRateArray[j];

			for(k = 0; k < 2; k++)
			{
				stMaxLayerParam.type = typeArray[k];

				for(l = 0; l < 2; l++)
				{
					stMaxLayerParam.fps_mode = fpsModeArray[l];

					for(m = iMinIndex; m <= iMaxIndex; m++)
					{
						layerNumber = m;
						EXPECT_EQ(WSE_S_OK, GetH264SpatialLayerInfo(stMaxLayerParam, stLayerInfoArray, layerNumber));
					}
				}
			}
		}
	}

	layerNumber = iMaxIndex;
	EXPECT_EQ(WSE_E_INVALIDARG, GetH264SpatialLayerInfo(stMaxLayerParam, NULL, layerNumber));
	layerNumber = iMinIndex - 1;
	EXPECT_EQ(WSE_E_INVALIDARG, GetH264SpatialLayerInfo(stMaxLayerParam, stLayerInfoArray, layerNumber));
}
