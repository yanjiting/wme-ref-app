#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public
#define protected public

#include "WmeInterface.h"

#include "WmeLocalAudioTrackImp.h"
#include "WmeCodec.h"

#if defined (WIN32)
#include <Windows.h>
#include <tchar.h>
#endif

using namespace wme;

class CWmeLocalAudioTrackTest : public testing::Test
{
public:
	CWmeLocalAudioTrackTest()
	{
		m_pTrack = NULL;
		WmeCreateMediaEngine(&m_pWmeEngine);
		if (m_pWmeEngine)
		{
#ifndef IOS_SIMULATOR
			m_pWmeEngine->CreateLocalAudioTrack((IWmeLocalAudioTrack**)&m_pTrack);
			m_pTrack->AddRef();
#endif
		}
	}
	virtual ~CWmeLocalAudioTrackTest()
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
	CWmeLocalAudioTrack* m_pTrack;	
};

TEST_F(CWmeLocalAudioTrackTest, StartAndStop)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
    if (m_pTrack) {
		EXPECT_EQ(WME_S_OK, m_pTrack->Start());
		EXPECT_EQ(WME_S_OK, m_pTrack->Start());
		EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
		EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
    }
}
TEST_F(CWmeLocalAudioTrackTest, SendEvent)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
    
    if (m_pTrack) {
        WmeUpLinkNetStat Value;
        EXPECT_EQ(WME_E_NOTIMPL, m_pTrack->SendEvent(WmeTrackEvent_UpLinkStat, &Value, sizeof(WmeUpLinkNetStat)));
        //
        EXPECT_EQ(WME_E_NOTIMPL, m_pTrack->SendEvent(WmeTrackEvent_ForceKeyFrame, &Value, sizeof(WmeUpLinkNetStat)));
        //
		EXPECT_EQ(WME_S_OK, m_pTrack->SendEvent(WmeTrackEvent_AudioCaptureDataError, &Value, sizeof(WmeUpLinkNetStat)));
		//
		EXPECT_EQ(WME_E_NOTIMPL, m_pTrack->SendEvent(WmeTrackEvent_ServerFilterInfo, &Value, sizeof(WmeUpLinkNetStat)));
    }
}
TEST_F(CWmeLocalAudioTrackTest, GetTrackType)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
    if (m_pTrack){
        WmeTrackType type = WmeTrackType_Uknown;
        EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackType(type));
        EXPECT_EQ(WmeTrackType_Audio, type);
	}
}

TEST_F(CWmeLocalAudioTrackTest, AboutLabel)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
    if (m_pTrack) {
        uint32_t ori = 0x1234;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackLabel(ori));
        uint32_t real = 0xffff;
        EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackLabel(real));
        EXPECT_EQ(ori, real);
    }
}

TEST_F(CWmeLocalAudioTrackTest, AboutEnable)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
    if (m_pTrack) {
        bool ori = true;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetTrackEnabled(ori));
        bool real = false;
        EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackEnabled(real));
        EXPECT_EQ(ori, real);
    }
}

TEST_F(CWmeLocalAudioTrackTest, AboutState)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
    if (m_pTrack) {
        
        WmeTrackState ori = WmeTrackState_Unknown;
        EXPECT_EQ(WME_S_OK, m_pTrack->GetTrackState(ori));
        EXPECT_NE(WmeTrackState_Unknown, ori);
    }
}

TEST_F(CWmeLocalAudioTrackTest, AboutCodec)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if (m_pTrack) {
        IWmeMediaCodec* ori = NULL;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetCodec(ori));
        IWmeMediaCodec* real = NULL;
        EXPECT_EQ(WME_S_OK, m_pTrack->GetCodec(&real));
        EXPECT_EQ((long)ori, (long)real);
    }
}

TEST_F(CWmeLocalAudioTrackTest, SetRTPChannel)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	wrtp::IRTPChannel* ori = NULL;
    if (NULL != m_pTrack) {
        EXPECT_EQ(WME_S_OK, m_pTrack->SetRTPChannel(ori));
    }
	int i = 0;
}

TEST_F(CWmeLocalAudioTrackTest, SetOption)
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
		//DAGC
		bEnable = false;
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_EnableAAGC, (void*)(&bEnable), sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG, m_pTrack->SetOption(WmeTrackOption_EnableDAGC, (void*)(&bEnable), 0));
		EXPECT_EQ(WME_E_INVALIDARG, m_pTrack->SetOption(WmeTrackOption_EnableDAGC, NULL, sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG, m_pTrack->SetOption(WmeTrackOption_EnableDAGC, NULL, 0));
	
        float db = 0;
        EXPECT_EQ(WME_E_NOTIMPL, m_pTrack->SetOption(WmeTrackOption_NoiseIndication, (void*)(&db), sizeof(float)));
        EXPECT_EQ(WME_E_NOTIMPL, m_pTrack->SetOption(WmeTrackOption_VoiceLevelIndication, (void*)(&db), sizeof(float)));
        EXPECT_EQ(WME_E_NOTIMPL, m_pTrack->SetOption(WmeTrackOption_AECIndication, (void*)(&db), sizeof(float)));
 
#if defined(MACOS)
        
        WmeAudioAecType aectype = WmeAudioAecTypeAlpha;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_ECType, (void*)(&aectype), sizeof(WmeAudioAecType)));
        
        aectype = WmeAudioAecTypeWme;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_ECType, (void*)(&aectype), sizeof(WmeAudioAecType)));
       

        aectype = WmeAudioAecTypeNone;
        EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_ECType, (void*)(&aectype), sizeof(WmeAudioAecType)));

        aectype = WmeAudioAecTypeTc;
        EXPECT_EQ(WME_E_NOTIMPL, m_pTrack->SetOption(WmeTrackOption_ECType, (void*)(&aectype), sizeof(WmeAudioAecType)));
       

        aectype = WmeAudioAecTypeBuildin;
        EXPECT_EQ(WME_E_NOTIMPL, m_pTrack->SetOption(WmeTrackOption_ECType, (void*)(&aectype), sizeof(WmeAudioAecType)));
#endif
        
    }
}

TEST_F(CWmeLocalAudioTrackTest, GetOption)
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

		//DAGC
		bEnable = true;
		EXPECT_EQ(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_EnableDAGC, (void*)(&bEnable), sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG, m_pTrack->GetOption(WmeTrackOption_EnableDAGC, (void*)(&bEnable), 0));
		EXPECT_EQ(WME_E_INVALIDARG, m_pTrack->GetOption(WmeTrackOption_EnableDAGC, NULL, sizeof(bool)));
		EXPECT_EQ(WME_E_INVALIDARG, m_pTrack->GetOption(WmeTrackOption_EnableDAGC, NULL, 0));
        
        float db = 0;
        EXPECT_EQ(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_NoiseIndication, (void*)(&db), sizeof(float)));
        EXPECT_EQ(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_VoiceLevelIndication, (void*)(&db), sizeof(float)));
        EXPECT_EQ(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_AECIndication, (void*)(&db), sizeof(float)));
    }
}

TEST_F(CWmeLocalAudioTrackTest, QueryInterface)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if (m_pTrack) {
        void* pInterface = NULL;
        EXPECT_EQ(WME_S_OK, m_pTrack->QueryInterface(WMEIID_IWmeLocalAudioTrack, &pInterface));
        EXPECT_NE(NULL, (long)pInterface);
    }
}
TEST_F(CWmeLocalAudioTrackTest, SetCodec)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
    if (m_pTrack) {
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
			if (pCodec != NULL)
			{
				wme::WmeCodecType codecType;
				if (pCodec->GetCodecType(codecType) == WME_S_OK)
				{
					if (codecType == WBXAE_AUDIO_PAYLOAD_TYPE_OPUS)
					{
						uint32_t codecBandWidth = 0;
						EXPECT_EQ(WME_S_OK, pCodec->GetCodecBandwidth(codecBandWidth));
						EXPECT_EQ(m_pTrack->m_CodecInst.rate, codecBandWidth);
					}
				}
			}
            pCodec->Release();
        }
        pAudioCodecEnu->Release();
        pAudioCodecEnu = NULL;
    }
}
TEST_F(CWmeLocalAudioTrackTest,GetStatistics){
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
    if (m_pTrack) {
        WmeAudioStatistics stat_input;
        EXPECT_EQ(WME_S_OK,m_pTrack->GetStatistics(stat_input));
    }
}
TEST_F(CWmeLocalAudioTrackTest,ResetStatistic){
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if (m_pTrack) {
		//m_pTrack->ResetStatistic();
	}
}

TEST_F(CWmeLocalAudioTrackTest,GetCaptureFormat){
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if (m_pTrack) {
		WmeAudioRawFormat fmt;
		EXPECT_EQ(WME_E_NOTIMPL,m_pTrack->GetCaptureFormat(fmt));
	}
}
TEST_F(CWmeLocalAudioTrackTest,GetCaptureDevice){
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if (m_pTrack) {
		IWmeMediaDevice* pDevice = NULL;
		EXPECT_EQ(WME_E_NOTIMPL,m_pTrack->GetCaptureDevice(&pDevice));
	}
}
//TEST_F(CWmeLocalAudioTrackTest,StartTimer){
//#ifndef IOS_SIMULATOR
//	EXPECT_NE(NULL, (long)m_pTrack);
//#endif
//	if (m_pTrack) {
//		EXPECT_EQ(CM_OK,((CWmeLocalAudioTrack*)m_pTrack)->StartTimer());
//		EXPECT_EQ(CM_OK,((CWmeLocalAudioTrack*)m_pTrack)->StartTimer());
//	}
//}
//TEST_F(CWmeLocalAudioTrackTest,StopTimer){
//#ifndef IOS_SIMULATOR
//	EXPECT_NE(NULL, (long)m_pTrack);
//#endif
//	if (m_pTrack) {
//		EXPECT_EQ(CM_OK,((CWmeLocalAudioTrack*)m_pTrack)->StopTimer());
//		EXPECT_EQ(CM_OK,((CWmeLocalAudioTrack*)m_pTrack)->StopTimer());
//	}
//}

TEST_F(CWmeLocalAudioTrackTest,SetRtpChannel){
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if (m_pTrack) {
		EXPECT_EQ(CM_OK,m_pTrack->SetRTPChannel(NULL));
	}
}

TEST_F(CWmeLocalAudioTrackTest, SetAudioProc)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
    if (m_pTrack) {
        bool bSwitchON, bSwitchOFF,bSwitchResult;
        bSwitchON= true;
        bSwitchOFF = false;
        
        // AGC
        m_pTrack->SetOption(WmeTrackOption_EnableAAGC,&bSwitchON,sizeof(bool));
        m_pTrack->GetOption(WmeTrackOption_EnableAAGC,&bSwitchResult,sizeof(bool));
#if defined(CM_MACOS) || defined(ANDROID)
      EXPECT_NE(bSwitchON,bSwitchResult);
#else
      EXPECT_EQ(bSwitchON,bSwitchResult);
#endif
      
        m_pTrack->SetOption(WmeTrackOption_EnableAAGC,&bSwitchOFF,sizeof(bool));
        m_pTrack->GetOption(WmeTrackOption_EnableAAGC,&bSwitchResult,sizeof(bool));
        EXPECT_EQ(bSwitchOFF,bSwitchResult);
        
        //AEC
        m_pTrack->SetOption(WmeTrackOption_EnableEC,&bSwitchON,sizeof(bool));
        m_pTrack->GetOption(WmeTrackOption_EnableEC,&bSwitchResult,sizeof(bool));
        EXPECT_EQ(bSwitchON,bSwitchResult);
        
        m_pTrack->SetOption(WmeTrackOption_EnableEC,&bSwitchOFF,sizeof(bool));
        m_pTrack->GetOption(WmeTrackOption_EnableEC,&bSwitchResult,sizeof(bool));
        EXPECT_EQ(bSwitchOFF,bSwitchResult);
        
        //VAD
        m_pTrack->SetOption(WmeTrackOption_EnableVAD,&bSwitchON,sizeof(bool));
        m_pTrack->GetOption(WmeTrackOption_EnableVAD,&bSwitchResult,sizeof(bool));
        EXPECT_EQ(bSwitchON,bSwitchResult);
        
        m_pTrack->SetOption(WmeTrackOption_EnableVAD,&bSwitchOFF,sizeof(bool));
        m_pTrack->GetOption(WmeTrackOption_EnableVAD,&bSwitchResult,sizeof(bool));
        EXPECT_EQ(bSwitchOFF,bSwitchResult);
        
        //NS
        m_pTrack->SetOption(WmeTrackOption_EnableNS,&bSwitchON,sizeof(bool));
        m_pTrack->GetOption(WmeTrackOption_EnableNS,&bSwitchResult,sizeof(bool));
        EXPECT_EQ(bSwitchON,bSwitchResult);
        
        m_pTrack->SetOption(WmeTrackOption_EnableNS,&bSwitchOFF,sizeof(bool));
        m_pTrack->GetOption(WmeTrackOption_EnableNS,&bSwitchResult,sizeof(bool));
        EXPECT_EQ(bSwitchOFF,bSwitchResult);
        
        //DropSeconds
        m_pTrack->SetOption(WmeTrackOption_EnableDropSeconds,&bSwitchON,sizeof(bool));
        m_pTrack->GetOption(WmeTrackOption_EnableDropSeconds,&bSwitchResult,sizeof(bool));
        EXPECT_EQ(bSwitchON,bSwitchResult);
        
        m_pTrack->SetOption(WmeTrackOption_EnableDropSeconds,&bSwitchOFF,sizeof(bool));
        m_pTrack->GetOption(WmeTrackOption_EnableDropSeconds,&bSwitchResult,sizeof(bool));
        EXPECT_EQ(bSwitchOFF,bSwitchResult);
    }
}

TEST_F(CWmeLocalAudioTrackTest,TransportSendPacket){
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
	EXPECT_NE(NULL, (long)(m_pTrack->m_pRecordTransport));
#endif
	if (m_pTrack) {
		EXPECT_EQ(-1,m_pTrack->m_pRecordTransport->SendPacket(0,NULL,0));
		EXPECT_EQ(0,m_pTrack->m_pRecordTransport->SendRTCPPacket(0,NULL,0));
		//
		char pData[100];
		EXPECT_EQ(-1,m_pTrack->m_pRecordTransport->SendPacket(0,pData,100));
	}
}

TEST_F(CWmeLocalAudioTrackTest,TransportSetRTPChnnel){
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
	EXPECT_NE(NULL, (long)(m_pTrack->m_pRecordTransport));
#endif
	if (m_pTrack) {
//		m_pTrack->m_pRecordTransport->SetRTPChnnel(NULL);
//		m_pTrack->m_pRecordTransport->SetRTPChnnel(NULL);
	}
}
