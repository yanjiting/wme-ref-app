#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WmeInterface.h"
#include "WmeError.h"
#include "WmeTrack.h"
#include "WmeLocalAudioExternalTrackImp.h"

#if defined (WIN32)
#include "WmeAudioCodecEnumerator.h"
#include "WmeCodec.h"
#include <Windows.h>	
#include <tchar.h>
#endif

using namespace wme;

class CWmeLocalAudioExternalTrackTest : public testing::Test
{
public:
	CWmeLocalAudioExternalTrackTest()
	{
		m_pTrack = NULL;
		WmeCreateMediaEngine(&m_pWmeEngine);
		if (m_pWmeEngine)
		{
#ifndef IOS_SIMULATOR
			m_pWmeEngine->CreateLocalAudioExternalTrack((IWmeLocalAudioExternalTrack**)&m_pTrack);		
#endif
		}
	}
	virtual ~CWmeLocalAudioExternalTrackTest()
	{
		if (m_pTrack)
		{
			m_pTrack->Release();
			m_pTrack = NULL;
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
	CWmeLocalAudioExternalTrack* m_pTrack;
};

TEST_F(CWmeLocalAudioExternalTrackTest, SetCodec)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		IWmeMediaCodecEnumerator* pAudioCodecEnu;
		int32_t nResult =  m_pWmeEngine->CreateMediaCodecEnumerator(WmeMediaTypeAudio,((IWmeMediaCodecEnumerator **)&pAudioCodecEnu));
		int nNum=0;
		pAudioCodecEnu->GetNumber(nNum);
		IWmeMediaCodec* pCodec = NULL;
		for(int i=0;i<nNum;i++)
		{
			pCodec = NULL;
			pAudioCodecEnu->GetCodec(i,((IWmeMediaCodec**)&pCodec));
			EXPECT_EQ(WME_S_OK, m_pTrack->SetCodec(pCodec));
			pCodec->Release();
		}
		pAudioCodecEnu->Release();
		pAudioCodecEnu = NULL;
	}
}

TEST_F(CWmeLocalAudioExternalTrackTest, StartAndStop)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		EXPECT_EQ(WME_S_OK, m_pTrack->Start());
		EXPECT_EQ(WME_S_OK, m_pTrack->Start());
		EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
		EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
	}
}

TEST_F(CWmeLocalAudioExternalTrackTest, GetTrackType)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		WmeTrackType type = WmeTrackType_Uknown;
		EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackType(type));
		EXPECT_EQ(WmeTrackType_Audio, type);
	}
}

TEST_F(CWmeLocalAudioExternalTrackTest, AboutLabel)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		uint32_t ori = 0x1234;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackLabel(ori));
		uint32_t real = 0xffff;
		EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackLabel(real));
		EXPECT_EQ(ori, real);
	}
}

TEST_F(CWmeLocalAudioExternalTrackTest, AboutEnable)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		bool ori = true;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackEnabled(ori));
		bool real = false;
		EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackEnabled(real));
		EXPECT_EQ(ori, real);
	}
}

TEST_F(CWmeLocalAudioExternalTrackTest, AboutState)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		WmeTrackState ori = WmeTrackState_Unknown;
		EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackState(ori));
		EXPECT_NE(WmeTrackState_Unknown, ori);
	}
}

TEST_F(CWmeLocalAudioExternalTrackTest, AboutCodec)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		IWmeMediaCodec* ori = NULL;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetCodec(ori));
		IWmeMediaCodec* real = NULL;
		EXPECT_EQ(WME_S_OK, m_pTrack->GetCodec(&real));
	}	
}

TEST_F(CWmeLocalAudioExternalTrackTest, GetVoiceLevel)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		uint32_t level;
		EXPECT_EQ(WME_S_OK, m_pTrack->GetVoiceLevel(level));
	}	
}

TEST_F(CWmeLocalAudioExternalTrackTest,SendEvent )
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		EXPECT_EQ(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_ForceKeyFrame,NULL,0));
	}	
}
TEST_F(CWmeLocalAudioExternalTrackTest, GetExternalInputter)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		/*OPTION starts here*/
		IWmeExternalInputter* p=NULL;
		EXPECT_EQ(WME_S_OK, m_pTrack->GetExternalInputter(&p));
        if (p != NULL)
        {
            p->Release();
        }
		EXPECT_EQ(WME_E_INVALIDARG, m_pTrack->GetExternalInputter(NULL));
	}
}
TEST_F(CWmeLocalAudioExternalTrackTest, InputMediaData)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		/*OPTION starts here*/
		IWmeExternalInputter* p=NULL;
		EXPECT_EQ(WME_S_OK, m_pTrack->GetExternalInputter(&p));
		EXPECT_NE(NULL, (long)p);
		IWmeMediaPackage* package=NULL;
		EXPECT_EQ(WME_E_NOTIMPL, p->InputMediaData(package));
        p->Release();
	}
}
TEST_F(CWmeLocalAudioExternalTrackTest, InputMediaData2)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		/*OPTION starts here*/
		IWmeExternalInputter* p=NULL;
		EXPECT_EQ(WME_S_OK, m_pTrack->GetExternalInputter(&p));
		EXPECT_NE(NULL, (long)p);

		EXPECT_EQ(WME_E_INVALIDARG, p->InputMediaData( 0, WmeMediaFormatAudioRaw, NULL, NULL, 0));
		//
		unsigned char data[320];
		WmeAudioRawFormat fmt;
		fmt.eRawType = WmePCM;
		fmt.iBitsPerSample = 16;
		fmt.iChannels =1;
		fmt.iSampleRate = 16000;
		EXPECT_EQ(WME_S_OK,p->InputMediaData(0,WmeMediaFormatAudioRaw,&fmt,data,320));
		EXPECT_EQ(WME_S_OK,p->InputMediaData(0,WmeMediaFormatAudioRaw,&fmt,data,320));
		EXPECT_EQ(WME_S_OK,p->InputMediaData(0,WmeMediaFormatAudioRaw,&fmt,data,320));
		//
		//fmt.iSampleRate =8000;
		//EXPECT_EQ(WME_S_OK,p->InputMediaData(0,WmeMediaFormatAudioRaw,&fmt,data,320));
	}
}


TEST_F(CWmeLocalAudioExternalTrackTest, SetOption)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	/*OPTION starts here*/
	//
	if(m_pTrack){
		bool bEnable = true;
		//AAGC
		bEnable = true;
#if defined(CM_MACOS) || defined(ANDROID)
		EXPECT_NE(WME_S_OK,m_pTrack->SetOption(WmeTrackOption_EnableAAGC,(void*)(&bEnable),sizeof(bool)));
#else
		EXPECT_EQ(WME_S_OK,m_pTrack->SetOption(WmeTrackOption_EnableAAGC,(void*)(&bEnable),sizeof(bool)));
#endif
		
    bEnable = false;
		EXPECT_EQ(WME_S_OK,m_pTrack->SetOption(WmeTrackOption_EnableAAGC,(void*)(&bEnable),sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableAAGC,(void*)(&bEnable),0));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableAAGC,NULL,sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableAAGC,NULL,0));
		//EC
		bEnable = true;
		EXPECT_EQ(WME_S_OK,m_pTrack->SetOption(WmeTrackOption_EnableEC,(void*)(&bEnable),sizeof(bool)));
		bEnable = false;
		EXPECT_EQ(WME_S_OK,m_pTrack->SetOption(WmeTrackOption_EnableEC,(void*)(&bEnable),sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableEC,(void*)(&bEnable),0));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableEC,NULL,sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableEC,NULL,0));
		//VAD
		bEnable = true;
		EXPECT_EQ(WME_S_OK,m_pTrack->SetOption(WmeTrackOption_EnableVAD,(void*)(&bEnable),sizeof(bool)));
		bEnable = false;
		EXPECT_EQ(WME_S_OK,m_pTrack->SetOption(WmeTrackOption_EnableVAD,(void*)(&bEnable),sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableVAD,(void*)(&bEnable),0));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableVAD,NULL,sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableVAD,NULL,0));
		//NS
		bEnable = true;
		EXPECT_EQ(WME_S_OK,m_pTrack->SetOption(WmeTrackOption_EnableNS,(void*)(&bEnable),sizeof(bool)));
		bEnable = false;
		EXPECT_EQ(WME_S_OK,m_pTrack->SetOption(WmeTrackOption_EnableNS,(void*)(&bEnable),sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableNS,(void*)(&bEnable),0));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableNS,NULL,sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableNS,NULL,0));
		//drop seconds
		bEnable = true;
		EXPECT_EQ(WME_S_OK,m_pTrack->SetOption(WmeTrackOption_EnableDropSeconds,(void*)(&bEnable),sizeof(bool)));
		bEnable = false;
		EXPECT_EQ(WME_S_OK,m_pTrack->SetOption(WmeTrackOption_EnableDropSeconds,(void*)(&bEnable),sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableDropSeconds,(void*)(&bEnable),0));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableDropSeconds,NULL,sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_EnableDropSeconds,NULL,0));
		//band width
		WmeMediaBandwidth bandwidth;
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_Bandwidth,(void*)(&bandwidth),sizeof(WmeMediaBandwidth)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_Bandwidth,(void*)(&bandwidth),0));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_Bandwidth,NULL,0));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_Bandwidth,NULL,sizeof(WmeMediaBandwidth)));
		//
		char name_cid[200];
		EXPECT_EQ(WME_S_OK,m_pTrack->SetOption(WmeTrackOption_Cid,(void*)name_cid,50));		
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_Cid,(void*)name_cid,150));		
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_Cid,(void*)name_cid,0));		
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_Cid,(void*)name_cid,-20));		
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_Cid,(void*)NULL,0));		
		//default
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->SetOption(WmeTrackOption_CapturePause,(void*)(&bEnable),sizeof(bool)));
		//
	}
}

TEST_F(CWmeLocalAudioExternalTrackTest, GetOption)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	/*OPTION starts here*/
	//
	if(m_pTrack){
		bool bEnable = true;
		//AAGC
		EXPECT_EQ(WME_S_OK,m_pTrack->GetOption(WmeTrackOption_EnableAAGC,(void*)(&bEnable),sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableAAGC,(void*)(&bEnable),0));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableAAGC,NULL,sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableAAGC,NULL,0));
		//EC
		EXPECT_EQ(WME_S_OK,m_pTrack->GetOption(WmeTrackOption_EnableEC,(void*)(&bEnable),sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableEC,(void*)(&bEnable),0));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableEC,NULL,sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableEC,NULL,0));
		//VAD
		EXPECT_EQ(WME_S_OK,m_pTrack->GetOption(WmeTrackOption_EnableVAD,(void*)(&bEnable),sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableVAD,(void*)(&bEnable),0));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableVAD,NULL,sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableVAD,NULL,0));
		//NS
		EXPECT_EQ(WME_S_OK,m_pTrack->GetOption(WmeTrackOption_EnableNS,(void*)(&bEnable),sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableNS,(void*)(&bEnable),0));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableNS,NULL,sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableNS,NULL,0));
		//drop seconds
		EXPECT_EQ(WME_S_OK,m_pTrack->GetOption(WmeTrackOption_EnableDropSeconds,(void*)(&bEnable),sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableDropSeconds,(void*)(&bEnable),0));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableDropSeconds,NULL,sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_EnableDropSeconds,NULL,0));
		//band width
		WmeMediaBandwidth bandwidth;
		EXPECT_EQ(WME_S_OK,m_pTrack->GetOption(WmeTrackOption_Bandwidth,(void*)(&bandwidth),sizeof(WmeMediaBandwidth)));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_Bandwidth,(void*)(&bandwidth),0));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_Bandwidth,NULL,0));
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_Bandwidth,NULL,sizeof(WmeMediaBandwidth)));
		//default
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_CapturePause,(void*)(&bEnable),sizeof(bool)));
		//
		char name_cid[127];
		EXPECT_EQ(WME_S_OK,m_pTrack->GetOption(WmeTrackOption_Cid,(void*)name_cid,sizeof(name_cid)));		
		EXPECT_EQ(WME_E_INVALIDARG,m_pTrack->GetOption(WmeTrackOption_Cid,(void*)name_cid,0));		
	}
}

TEST_F(CWmeLocalAudioExternalTrackTest, QueryInterface)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		void* pInterface = NULL;
		EXPECT_EQ(WME_S_OK, m_pTrack->QueryInterface(WMEIID_IWmeLocalAudioExternalTrack, &pInterface));
		EXPECT_NE(NULL, (long)pInterface);
	}
}
TEST_F(CWmeLocalAudioExternalTrackTest, SetRTPChannel)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
	//	EXPECT_EQ(WME_S_OK, m_pTrack->SetRTPChannel(NULL));
	}
}
TEST_F(CWmeLocalAudioExternalTrackTest, IsFormatSupport)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
//		WBXWAVEFORMAT fmt;
//		EXPECT_EQ(false, m_pTrack->IsFormatSupport(fmt));
	}
}
TEST_F(CWmeLocalAudioExternalTrackTest, GetDefaultFormat)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
//		WBXWAVEFORMAT fmt;
//		EXPECT_EQ(WBXAE_SUCCESS, m_pTrack->GetDefaultFormat(&fmt));
	}
}
TEST_F(CWmeLocalAudioExternalTrackTest, OpenDevice)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		WBXWAVEFORMAT fmt;
		EXPECT_EQ(WBXAE_CE_ERROR_AUDIO_CAPTURE_PLAYBACK_BADFORMAT, m_pTrack->OpenDevice(NULL));
		EXPECT_EQ(WBXAE_SUCCESS, m_pTrack->OpenDevice(&fmt));		
	}
}
TEST_F(CWmeLocalAudioExternalTrackTest, CloseDevice)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		EXPECT_EQ(WBXAE_SUCCESS, m_pTrack->CloseDevice());
	}
}

TEST_F(CWmeLocalAudioExternalTrackTest, StartCapture)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		EXPECT_EQ(WBXAE_SUCCESS, m_pTrack->StartCapture());
	}
}

TEST_F(CWmeLocalAudioExternalTrackTest, StopCapture)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		EXPECT_EQ(WBXAE_SUCCESS, m_pTrack->StopCapture());
	}
}

TEST_F(CWmeLocalAudioExternalTrackTest, SetDevice)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		WBXAEDEVICEID deviceID;
		EXPECT_EQ(WBXAE_SUCCESS, m_pTrack->SetDevice(&deviceID));
		EXPECT_EQ(WBXAE_CE_ERROR_AUDIO_CAPTURE_PLAYBACK_DEVICESET, m_pTrack->SetDevice(NULL));		
	}
}

TEST_F(CWmeLocalAudioExternalTrackTest, IsDeviceSet)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		WBXAEDEVICEID deviceID;
		EXPECT_EQ(false, m_pTrack->IsDeviceSet(&deviceID));
	}
}
TEST_F(CWmeLocalAudioExternalTrackTest, SetSink)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if(NULL != m_pTrack)
	{
		EXPECT_EQ(WBXAE_SUCCESS, m_pTrack->SetSink(NULL));
	}
}