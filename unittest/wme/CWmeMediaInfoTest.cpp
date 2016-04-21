#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
#include "WmeEngine.h"
#include "WmeVideoMediaInfoGetter.h"

#define MAX_VIDEO_WIDTH		1280
#define MAX_VIDEO_HEIGHT	720
#define MAX_VIDEO_FPS		30
#define MAX_VIDEO_SPACIAL_LAYER	4

#ifdef WIN32
#include "windows.h"
#endif

void OutputDebugPrintf(const char * strOutputString,...)
{
	char strBuffer[4096]={0};
	va_list vlArgs;
	va_start(vlArgs,strOutputString);
#ifdef WIN32
	_vsnprintf(strBuffer,sizeof(strBuffer)-1,strOutputString,vlArgs);
#else
	vsnprintf(strBuffer,sizeof(strBuffer)-1,strOutputString,vlArgs);
#endif
	va_end(vlArgs);
#ifdef WIN32
	OutputDebugStringA(strBuffer);        //windows API
#else
    printf(strBuffer);
#endif
}


using namespace wme;

class CWmeMediaInfoTest : public testing::Test
{
public:
	CWmeMediaInfoTest()
	{
		WmeCreateMediaEngine(&m_pWmeEngine);

		if (m_pWmeEngine)
		{
			m_pWmeEngine->CreateMediaInfoGetter(WmeMediaTypeVideo, &m_pVideoGetter);
			m_pWmeEngine->CreateMediaInfoGetter(WmeMediaTypeAudio, &m_pAudioGetter);
		}
	}

	virtual ~CWmeMediaInfoTest()
	{
		if (m_pVideoGetter)
		{
			m_pVideoGetter->Release();
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
	IWmeMediaInfoGetter* m_pVideoGetter;
	IWmeMediaInfoGetter* m_pAudioGetter;

};

TEST_F(CWmeMediaInfoTest, AboutVideo)
{
	EXPECT_NE(NULL, (long)m_pVideoGetter);
	
	WmeMediaInfo_VideoMax vMax;
	vMax.maxWidth = MAX_VIDEO_WIDTH;
	vMax.maxHeight = MAX_VIDEO_HEIGHT;
	vMax.maxFrameRate = MAX_VIDEO_FPS;
	vMax.maxLayerNum = MAX_VIDEO_SPACIAL_LAYER;
	EXPECT_EQ(WME_S_OK, m_pVideoGetter->SetConfig(WmeWIC_VideoMax, &vMax, sizeof(vMax)));

	int index = 0;
	IWmeMediaInfo* p = NULL;
	while (WME_S_OK == m_pVideoGetter->GetMediaInfo(index, &p))
	{
		WmeMediaType type = WmeMediaTypeVideo;
		EXPECT_EQ(WME_S_OK, p->GetMediaType(type));
		int32_t cap_num = 0;
		EXPECT_EQ(WME_S_OK, p->GetCapabilityNumber(cap_num));
		WmeVideoMediaCapability cap;
//		memset(&cap, 0, sizeof(cap));
		int i = 0;
		while (WME_S_OK == p->GetMediaCapabilities(i, &cap, sizeof(cap)))
		{
			//TODO: print cap
			OutputDebugPrintf("debug output: cap param(%d,%d,%d,%d,%d),frame(%d,: %d,%d,%d,%d), bitrate(%d) \r\n",cap.width, cap.height, cap.profile_level_id, cap.max_mbps, cap.max_fs,
				cap.frame_layer_number, cap.frame_layer[0], cap.frame_layer[1], cap.frame_layer[2], cap.frame_layer[3], cap.min_bitrate_bps);
			i++;
            EXPECT_NE(0, cap.min_bitrate_bps);
		}

		index++;
		p->Release();
	}

}
TEST_F(CWmeMediaInfoTest, AboutAudio)
{
	EXPECT_NE(NULL, (long)m_pAudioGetter);
	EXPECT_EQ(WME_E_NOTIMPL, m_pAudioGetter->SetConfig(WmeWIC_VideoMax, NULL, sizeof(char)));
	int32_t iNumbe=-1;
	EXPECT_EQ(WME_S_OK,m_pAudioGetter->GetInfoNumber(iNumbe));
	IWmeMediaInfo* p = NULL;
	for (int32_t i=0;i<iNumbe;i++)
	{
		m_pAudioGetter->GetMediaInfo(i,&p);
		WmeMediaType type = WmeMediaTypeVideo;
		EXPECT_EQ(WME_S_OK, p->GetMediaType(type));
		EXPECT_EQ(WmeMediaTypeAudio,type);
		int32_t cap_num = 0;
		EXPECT_EQ(WME_S_OK, p->GetCapabilityNumber(cap_num));
		WmeAudioMediaCapability cap;
		memset(&cap, 0, sizeof(WmeAudioMediaCapability));
		for (int32_t j=0;j<cap_num;j++)
		{
			EXPECT_EQ(WME_S_OK,p->GetMediaCapabilities(j, &cap, sizeof(WmeAudioMediaCapability)));

		}
	}



}
