#include "gtest/gtest.h"
#include "gmock/gmock.h"
#define private public
#define protected public

#include "WmeInterface.h"
#include "WmeError.h"
#include "WmeTrack.h"

#include "wrtpmediaapi.h"
#include "WmeRemoteAudioTrackImp.h"

#if defined (WIN32)
#include <Windows.h>
#endif
using namespace wrtp;
using namespace wme;


class CWmeRemoteAudioTrackTest : public testing::Test
{
public:
	CWmeRemoteAudioTrackTest()
	{
		m_pTrack = NULL;
        
        
		WmeCreateMediaEngine(&m_pWmeEngine);
		if (m_pWmeEngine)
		{

			m_pWmeEngine->CreateRemoteAudioTrack((IWmeRemoteAudioTrack**)&m_pTrack);
			//m_pTrack->AddRef();
            
		}
        
        
	}
	virtual ~CWmeRemoteAudioTrackTest()
	{
		if (m_pTrack)
		{
			m_pTrack->Release();
			m_pTrack = NULL;
		}
		if (m_pWmeEngine)
		{
			m_pWmeEngine->Release();
			m_pWmeEngine = NULL;
		}
	}
    
	virtual void SetUp() {}
	virtual void TearDown() {}
public:
	IWmeMediaEngine* m_pWmeEngine;
	CWmeRemoteAudioTrack* m_pTrack;
	CWmeRemoteAudioTrack* m_pTrackArray[6];
};
TEST_F(CWmeRemoteAudioTrackTest, CreateRemoteAudioTrack)
{

	for (int i=0;i<6;i++)
	{
		
		EXPECT_EQ(WME_S_OK, m_pWmeEngine->CreateRemoteAudioTrack((IWmeRemoteAudioTrack**)&m_pTrackArray[i]));
		//m_pTrackArray[i]->AddRef();
	}
	EXPECT_NE(WME_S_OK, m_pWmeEngine->CreateRemoteAudioTrack((IWmeRemoteAudioTrack**)&m_pTrackArray[1]));
	m_pTrackArray[1]->Release();
	EXPECT_EQ(WME_S_OK, m_pWmeEngine->CreateRemoteAudioTrack((IWmeRemoteAudioTrack**)&m_pTrackArray[1]));
	//
	IWmeRemoteAudioTrack* pTrack;
	EXPECT_EQ(WME_E_FAIL, m_pWmeEngine->CreateRemoteAudioTrack(&pTrack));
	//
	for (int i=0;i<6;i++)
	{
		if(NULL != m_pTrackArray[i])
		{
			m_pTrackArray[i]->Release();
		}
	}
	EXPECT_EQ(WME_S_OK, m_pWmeEngine->CreateRemoteAudioTrack((IWmeRemoteAudioTrack**)&m_pTrackArray[1]));

}
TEST_F(CWmeRemoteAudioTrackTest, SendEvent)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if (m_pTrack) {
		EXPECT_EQ(WME_E_NOTIMPL, m_pTrack->SendEvent(WmeTrackEvent_UpLinkStat,NULL,0));
	}
}
TEST_F(CWmeRemoteAudioTrackTest, QueryInterface)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	void* pInterface = NULL;
    if (m_pTrack) {
        EXPECT_EQ(WME_S_OK, m_pTrack->QueryInterface(WMEIID_IWmeRemoteAudioTrack, &pInterface));
        EXPECT_NE(NULL, (long)pInterface);
    }
}
TEST_F(CWmeRemoteAudioTrackTest, SetOption)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if (m_pTrack) {
		//CID
		EXPECT_EQ(WME_E_INVALIDARG, m_pTrack->SetOption(WmeTrackOption_Cid,NULL,50));
		char pData[200];
		EXPECT_EQ(WME_S_OK, m_pTrack->SetOption(WmeTrackOption_Cid,(void*)pData,50));
		//
		EXPECT_EQ(WME_E_INVALIDARG, m_pTrack->SetOption(WmeTrackOption_Cid,(void*)pData,150));
		EXPECT_EQ(WME_E_INVALIDARG, m_pTrack->SetOption(WmeTrackOption_Cid,(void*)pData,0));
		//default
		EXPECT_EQ(WME_E_INVALIDARG, m_pTrack->SetOption(WmeTrackOption_EnableAAGC,(void*)pData,50));		
	}
}
TEST_F(CWmeRemoteAudioTrackTest,GetOption)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if (m_pTrack) {
		char pData[200];
		//CID
		EXPECT_EQ(WME_S_OK, m_pTrack->GetOption(WmeTrackOption_Cid,(void*)pData,200));
		EXPECT_EQ(WME_E_INVALIDARG, m_pTrack->GetOption(WmeTrackOption_Cid,NULL,200));
		EXPECT_EQ(WME_E_INVALIDARG, m_pTrack->GetOption(WmeTrackOption_Cid,(void*)pData,0));
		//default
		EXPECT_EQ(WME_E_INVALIDARG, m_pTrack->GetOption(WmeTrackOption_EnableAAGC,(void*)pData,50));		
	}
}

//TEST_F(CWmeRemoteAudioTrackTest, SetRTPChannel)
//{
//	EXPECT_NE(NULL, (long)m_pTrack);
//	wrtp::IRTPChannel* ori = NULL;
//
//	EXPECT_EQ(WME_S_OK, m_pTrack->SetRTPChannel(ori));
//}
TEST_F(CWmeRemoteAudioTrackTest, GetVoiceLevel)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if (m_pTrack) {
		uint32_t level;
		EXPECT_EQ(WME_S_OK, m_pTrack->GetVoiceLevel(level));
	}
}
TEST_F(CWmeRemoteAudioTrackTest, GetAudioPayloadType)
{
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if (m_pTrack) {
		WBXAE_AUDIO_PAYLOAD_TYPE audiotype = WBXAE_AUDIO_PAYLOAD_TYPE_NONE;
		//
		//EXPECT_EQ(WME_S_OK, m_pTrack->GetAudioPayloadType(WmeCodecType_G711_ULAW,audiotype));
		//EXPECT_EQ(WME_S_OK, m_pTrack->GetAudioPayloadType(WmeCodecType_G711_ALAW,audiotype));
		//EXPECT_EQ(WME_S_OK, m_pTrack->GetAudioPayloadType(WmeCodecType_ILBC,audiotype));
		//EXPECT_EQ(WME_S_OK, m_pTrack->GetAudioPayloadType(WmeCodecType_OPUS,audiotype));
		//EXPECT_EQ(WME_S_OK, m_pTrack->GetAudioPayloadType(WmeCodecType_G722,audiotype));
		//EXPECT_EQ(WME_S_OK, m_pTrack->GetAudioPayloadType(WmeCodecType_CNG,audiotype));
		//EXPECT_EQ(WME_S_OK, m_pTrack->GetAudioPayloadType(WmeCodecType_AVC,audiotype));
	}
}
TEST_F(CWmeRemoteAudioTrackTest, StartAndStop)
{
	EXPECT_NE(NULL, (long)m_pTrack);
    if (m_pTrack) {
        EXPECT_EQ(WME_S_OK, m_pTrack->Start());
        EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
    }
}
TEST_F(CWmeRemoteAudioTrackTest, OnRecvMediaData)
{
	EXPECT_NE(NULL, (long)m_pTrack);
	wrtp::WRTPMediaDataVoIP voipData;
    if (m_pTrack) {
        
        EXPECT_EQ(WME_S_OK, m_pTrack->Start());
        //
        int nSize = 50;
        unsigned char * pdata = new unsigned char[nSize];
        memset((void*)pdata,0,sizeof(pdata));

        voipData.type = 1;
#ifdef WIN32
        voipData.sampleTimestamp = GetTickCount();;
#else
        voipData.sampleTimestamp = time(NULL);
#endif
        voipData.marker = 0;
        voipData.size = nSize;
        voipData.data=pdata;;
        //	voipData.payloadType = 102;//iLBC
        //	voipData.encoding = "";
        voipData.sequenceNumber = 0;
        voipData.receiveTime = voipData.sampleTimestamp+100;
        voipData.ssrc = 123456;
        for(int i=0;i<10;i++)
        {
            voipData.sampleTimestamp += 30;
            voipData.sequenceNumber ++;
            voipData.receiveTime = voipData.sampleTimestamp+100;
            EXPECT_EQ(0,((wme::CWmeRemoteAudioTrack*)m_pTrack)->OnRecvMediaData(&voipData,NULL));
        }
        //
#ifdef WIN32
		Sleep(3000);
#endif
		CCmTimerWrapperID timerddd;
		m_pTrack->OnTimer(&timerddd);
  		m_pTrack->OnTimer(&timerddd);
		EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
		EXPECT_NE(WME_S_OK, m_pTrack->OnRecvMediaData(&voipData,NULL));
        delete pdata;
    }
}
TEST_F(CWmeRemoteAudioTrackTest,GetSyncPriority)
{
	int32_t result;
    if (m_pTrack) {
        wme::CWmeRemoteAudioTrack* pSync=(wme::CWmeRemoteAudioTrack*) m_pTrack;
        EXPECT_EQ(WME_S_OK, pSync->GetSyncPriority(result));
        EXPECT_EQ(0,(result));
    }
}
TEST_F(CWmeRemoteAudioTrackTest,SetSyncSource)
{
    if (m_pTrack) {
        wme::CWmeRemoteAudioTrack* pSync=(wme::CWmeRemoteAudioTrack*) m_pTrack;
        EXPECT_EQ(WME_S_OK, pSync->SetSyncSource(true));
    }
}
TEST_F(CWmeRemoteAudioTrackTest,SetSyncController)
{
    if (m_pTrack) {
        wme::CWmeRemoteAudioTrack* pSync=(wme::CWmeRemoteAudioTrack*) m_pTrack;
        EXPECT_EQ(WME_S_OK, pSync->SetSyncController(NULL));
    }
}
TEST_F(CWmeRemoteAudioTrackTest,GetRefNum)
{
    if (m_pTrack) {
        wme::CWmeRemoteAudioTrack* pSync=(wme::CWmeRemoteAudioTrack*) m_pTrack;
        EXPECT_EQ(2, pSync->GetRefNum());
    }
}
TEST_F(CWmeRemoteAudioTrackTest,GetPlayMixTrack)
{
    if (m_pTrack) {
        wme::IWmeRemoteAudioMixTrack* pMixerTrack=NULL;
        EXPECT_EQ(WME_S_OK,m_pTrack->GetPlayMixTrack(&pMixerTrack));
        uint32_t level;
        EXPECT_EQ(WME_S_OK,pMixerTrack->GetVoiceLevel(level));
		//
		WmeAudioRawFormat fmt;
		EXPECT_EQ(WME_E_NOTIMPL,pMixerTrack->GetPlayFormat(fmt));
		//
		WmeAudioStatistics state;
		EXPECT_EQ(WME_E_NOTIMPL,pMixerTrack->GetStatistics(state));
		//
        pMixerTrack->Release();
    }
}

TEST_F(CWmeRemoteAudioTrackTest,GetStatistics){
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if (m_pTrack) {
		WmeAudioStatistics stat_input;
		EXPECT_EQ(m_pTrack->GetStatistics(stat_input),WME_S_OK);
	}
}


TEST_F(CWmeRemoteAudioTrackTest,GetPlayFormat){
	EXPECT_NE(NULL, (long)m_pTrack);
	if (m_pTrack) {
		WmeAudioRawFormat formata;
		EXPECT_EQ(m_pTrack->GetPlayFormat(formata),WME_S_OK);
	}
}

//TEST_F(CWmeRemoteAudioTrackTest, OnRecvMediaData)
//{
//typedef int WRTPMediaDataIntegrityInfo ;
//	struct WRTPMediaDataMock
//{
//    WRTPMediaDataMock(): type(0), timestamp(0), sampleTimestamp(0), marker(0), size(0), data(NULL) {}
//
//    int type;
//    UINT32 timestamp;           // milliseconds, the clock tick when the media data is captured
//    UINT32 sampleTimestamp;     // used in RTP header timestamp field, with unit of 1/sample_clock_rate seconds.
//    UINT8 marker;
//    UINT32 size;
//    UINT8* data;
//};
//	class IMediaDataRecvSinkmock
//{
//public:
//    /** Call this function to notify media engine that a media data unit is arrived
//    * Parameters:
//    *  @Param[in] pData: the media data to be delivered to media engine
//    *  @Param[in] pIntegrityInfo: the integrity information for the received media data
//    * Return value:
//    *   0 is successful, otherwise failed
//    */
//    virtual INT32 OnRecvMediaData(/*RTP_IN*/ WRTPMediaDataMock* pData, /*RTP_IN*/ WRTPMediaDataIntegrityInfo* pIntegrityInfo) = 0;
//
//    virtual ~IMediaDataRecvSinkmock() {}
//};
//	EXPECT_NE(NULL, (long)m_pTrack);
//	IMediaDataRecvSinkmock* sink=(IMediaDataRecvSinkmock*)m_pTrack;
//	EXPECT_EQ((INT32)-1, sink->OnRecvMediaData(NULL,NULL));
//	//EXPECT_EQ(WME_S_OK, m_pTrack->Start());
//	////
//	//int nSize = 50;
//	//UINT8 * pdata = new UINT8[nSize];
//	//memset(pdata,0,sizeof(pdata));
//	//wrtp::WRTPMediaDataVoIP voipData;
//	//voipData.type = 1;
//	//voipData.sampleTimestamp = GetTickCount();;
//	//voipData.marker = 0;
//	//voipData.size = nSize;
//	//voipData.data=pdata;;
//	//voipData.payloadType = 102;//iLBC
//	//voipData.encoding = "";
//	//voipData.sequenceNumber = 0;
//	//voipData.receiveTime = voipData.sampleTimestamp+100;
//	//voipData.ssrc = 123456;
//	//for(int i=0;i<1000;i++)
//	//{
//	//	voipData.sampleTimestamp += 30;
//	//	voipData.sequenceNumber ++;
//	//	voipData.receiveTime = voipData.sampleTimestamp+100;
//	//	EXPECT_EQ(0,((wme::CWmeRemoteAudioTrack*)m_pTrack)->OnRecvMediaData(&voipData,NULL));
//	//}
//	////
//	//EXPECT_EQ(WME_S_OK, m_pTrack->Stop());
//	//delete pdata;
//
//}

TEST_F(CWmeRemoteAudioTrackTest,OnRefReleased){
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if (m_pTrack) {
		uint32_t ref = 0;
		EXPECT_EQ(WME_S_OK,m_pTrack->OnRefReleased(ref));
	}
}

TEST_F(CWmeRemoteAudioTrackTest,ExternalRenderer){
#ifndef IOS_SIMULATOR
	EXPECT_NE(NULL, (long)m_pTrack);
#endif
	if (m_pTrack) {
		EXPECT_NE(WME_S_OK,m_pTrack->AddExternalRenderer(NULL,NULL));
		//
		WmeAudioRawFormat renderFormat;
		IWmeMediaFileRenderer *pExternalRender;
		if(WME_S_OK == m_pWmeEngine->CreateMediaFileRenderer(&pExternalRender))
		{
			EXPECT_EQ(WME_S_OK,m_pTrack->AddExternalRenderer(pExternalRender,&renderFormat));
			uint32_t uTimestamp;
			WmeMediaFormatType eType = WmeMediaFormatAudioRaw;
			WmeAudioRawFormat fmt;
			unsigned char pData[320];
			uint32_t uLength = 320;
			EXPECT_EQ(WME_S_OK,m_pTrack->OnRequestData(uTimestamp,eType,&fmt,pData,uLength));
			//
			EXPECT_EQ(WME_S_OK,m_pTrack->RemoveExternalRenderer(pExternalRender));
			//
			EXPECT_NE(WME_S_OK,m_pTrack->OnRequestData(uTimestamp,eType,&fmt,pData,uLength));
		}
	}
}


TEST_F(CWmeRemoteAudioTrackTest,NotifyPlaybackTimestamp){
#ifndef IOS_SIMULATOR
    EXPECT_NE(NULL, (long)m_pTrack);
#endif
    if (m_pTrack) {
#ifdef MACOS // only enable it in Mac now
        EXPECT_EQ(WME_S_OK, m_pTrack->NotifyPlaybackTimestamp(0));
//       m_pTrack->Init();
//        m_pTrack->Uninit();
        EXPECT_EQ(WME_S_OK, m_pTrack->NotifyPlaybackTimestamp(-1));
#endif
    }
}


