/*****************************************************************************************
*Generate by AUT Tool on 2010-12-29
*class:CWbxAudioEngineImplTest
********************************************************************************************/
// #ifndef WINVER                  // Specifies that the minimum required platform is Windows XP.
// #define WINVER 0x0500           // Change this to the appropriate value to target other versions of Windows.
// #endif
#include "WbxAeDefine.h"
#include "CmStdCpp.h"

#include "gtest/gtest.h"
#include "gmock/gmock.h"
//#include "wbxmock.h"

#define private public
#define protected public
#include "WbxAudioEngineImpl.h"

using testing::Return;
using testing::_;
using testing::Invoke;
using testing::ReturnRef;
using testing::Matcher;

#ifndef IOS_SIMULATOR

class CWbxAudioEngineImplTest : public testing::Test
{
public:
    CWbxAudioEngineImplTest()
    {
    }

    virtual ~CWbxAudioEngineImplTest()
    {
    }

    virtual void SetUp()
    {
		m_pAudioEng = new CWbxAudioEngineImpl();
    }

    virtual void TearDown()
    {
		delete m_pAudioEng;
    }

protected:
	CWbxAudioEngineImpl * m_pAudioEng;
};


TEST_F(CWbxAudioEngineImplTest, GetDeviceEnumeratorInst)
{
	AeResult rv;
	IWBXDeviceEnumerator * pDeviceEnu = NULL;
	rv = m_pAudioEng->GetDeviceEnumeratorInst(pDeviceEnu);
	EXPECT_EQ(rv,WBXAE_SUCCESS);
	//EXPECT_TRUE(pDeviceEnu);
	EXPECT_TRUE(pDeviceEnu == m_pAudioEng->m_pDeviceEnum);

}

TEST_F(CWbxAudioEngineImplTest, SetAudioEngineSink)
{
	IWbxAudioEngineSink* infoSink = NULL;
	AeResult rv = m_pAudioEng->SetAudioEngineSink(infoSink);
	EXPECT_EQ(rv,WBXAE_SUCCESS);
	EXPECT_TRUE(infoSink==m_pAudioEng->m_pAESink);
}

TEST_F(CWbxAudioEngineImplTest, Init)
{
	AeResult rv = m_pAudioEng->Init();
	EXPECT_EQ(rv,WBXAE_SUCCESS);
	EXPECT_TRUE(m_pAudioEng->m_pAQE != NULL);
	EXPECT_TRUE(m_pAudioEng->m_pCaptureEng != NULL);
	EXPECT_TRUE(m_pAudioEng->m_pPlaybackEng != NULL);
	EXPECT_TRUE(m_pAudioEng->m_pVoiceMixer != NULL);

}

TEST_F(CWbxAudioEngineImplTest, Terminate)
{
    m_pAudioEng->Init();
	AeResult rv = m_pAudioEng->Terminate();
	EXPECT_EQ(rv,WBXAE_SUCCESS);
	EXPECT_TRUE(m_pAudioEng->m_pAQE == NULL);
	EXPECT_TRUE(m_pAudioEng->m_pCaptureEng == NULL);
	EXPECT_TRUE(m_pAudioEng->m_pPlaybackEng == NULL);
	EXPECT_TRUE(m_pAudioEng->m_pVoiceMixer == NULL);
}

TEST_F(CWbxAudioEngineImplTest, SetTraceFileName)
{
}

TEST_F(CWbxAudioEngineImplTest, SetTraceFilter)
{
}

TEST_F(CWbxAudioEngineImplTest, SendDeviceNotification)
{
    m_pAudioEng->Init();
    AudioDeviceNotification tempNotification = HeadsetPlugin;
    AeResult rv = m_pAudioEng->SendDeviceNotification(tempNotification,0,0);
    EXPECT_EQ(rv,WBXAE_ERROR_BASE);
    
    m_pAudioEng->Init();
    rv = m_pAudioEng->SendDeviceNotification(tempNotification,0,0);
    EXPECT_EQ(rv,WBXAE_ERROR_BASE);
    
    int channelID = m_pAudioEng->CreateRecordChannel();
    rv =  m_pAudioEng->StartSend(channelID);
    
    rv = m_pAudioEng->SendDeviceNotification(tempNotification,0,0);
    EXPECT_EQ(rv,WBXAE_SUCCESS);
    
    m_pAudioEng->DeleteRecordChannel(channelID);
    for(int i =0;i < MAX_RECORD_CHANNEL_NUM;i++)
    {
        if(m_pAudioEng->m_pRecordChannel[i])
        {
            delete m_pAudioEng->m_pRecordChannel[i];
            m_pAudioEng->m_pRecordChannel[i] = NULL;
        }
    }
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, SetRecordDevice)
{
	WBXAEDEVICEID pDeviceID;
#ifdef WIN32
	memset(&pDeviceID,0,sizeof(WBXAEDEVICEID));
#else
#endif
	AeResult rv = m_pAudioEng->SetRecordDevice(&pDeviceID);
	EXPECT_NE(rv, WBXAE_SUCCESS);

#ifdef WIN32
	IWBXDeviceEnumerator* pDeviceEnumerator = NULL;
	m_pAudioEng->GetDeviceEnumeratorInst(pDeviceEnumerator);


    int numberofDev = pDeviceEnumerator->GetNumOfMicrophones();
	if (numberofDev >= 1)
	{
		pDeviceEnumerator->GetSysDefaultMicrophone(pDeviceID);
	}
#endif
    m_pAudioEng->Init();
	rv = m_pAudioEng->SetRecordDevice(&pDeviceID);
	EXPECT_EQ(rv,WBXAE_SUCCESS);  
	m_pAudioEng->Terminate();

}

TEST_F(CWbxAudioEngineImplTest, SetPlaybackDevice)
{
	WBXAEDEVICEID pDeviceID;
	//memset(&pDeviceID,0,sizeof(WBXAEDEVICEID));
	pDeviceID.flow = WBXAE_DEVICE_RENDER;
	AeResult rv = m_pAudioEng->SetPlaybackDevice(&pDeviceID);
	//EXPECT_EQ(rv,WBXAE_ERROR_GET_SPEAKER_VOLUECTL);  ////this logic is different between android platform and others
    m_pAudioEng->Init();
	rv = m_pAudioEng->SetPlaybackDevice(&pDeviceID);
	//EXPECT_EQ(rv,WBXAE_ERROR_GET_SPEAKER_VOLUECTL);  ////this logic is different between android platform and others
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, MaxNumOfRecordChannels)
{
    m_pAudioEng->Init();
	int iNum = 0;
	iNum = m_pAudioEng->MaxNumOfRecordChannels();
	EXPECT_EQ(iNum,MAX_RECORD_CHANNEL_NUM);
}

TEST_F(CWbxAudioEngineImplTest, CreateRecordChannel)
{
    m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreateRecordChannel();
	int iSeq = m_pAudioEng->GetRecordChannelSeq(channelID);
	EXPECT_TRUE(iSeq >=0 && iSeq < MAX_RECORD_CHANNEL_NUM);
	EXPECT_TRUE(m_pAudioEng->m_pRecordChannel[iSeq]!=NULL);

	//test create another Record channel

	int channelID2 = INVALID_CHANNEL_ID;
	for(int i =0;i < MAX_RECORD_CHANNEL_NUM;i++)
	{
		channelID2= m_pAudioEng->CreateRecordChannel();
	}
	//EXPECT_EQ(channelID2,INVALID_CHANNEL_ID);

	m_pAudioEng->DeleteRecordChannel(channelID);
	for(int i =0;i < MAX_RECORD_CHANNEL_NUM;i++)
	{
		if(m_pAudioEng->m_pRecordChannel[i])
		{
			delete m_pAudioEng->m_pRecordChannel[i];
			m_pAudioEng->m_pRecordChannel[i] = NULL;
		}
	}
}

TEST_F(CWbxAudioEngineImplTest, DeleteRecordChannel)
{
    m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreateRecordChannel();
	int iSeq = m_pAudioEng->GetRecordChannelSeq(channelID);
	m_pAudioEng->DeleteRecordChannel(channelID);

	//After delete, this channel should NULL
	EXPECT_TRUE(iSeq >=0 && iSeq < MAX_RECORD_CHANNEL_NUM);
	EXPECT_TRUE(m_pAudioEng->m_pRecordChannel[iSeq]==NULL);

	//test invalid channelID
	channelID = INVALID_CHANNEL_ID;
	AeResult rv= m_pAudioEng->DeleteRecordChannel(channelID);
	EXPECT_EQ(rv,WBXAE_ERROR_INVALID_CHANNELID);
	
}

TEST_F(CWbxAudioEngineImplTest, PauseRecordChannel)
{
	m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreateRecordChannel();
	int iSeq = m_pAudioEng->GetRecordChannelSeq(channelID);
	EXPECT_EQ(m_pAudioEng->PauseRecordChannel(channelID),WBXAE_ERROR_CHANNEL_STATE_MISTAKE);
	EXPECT_EQ(m_pAudioEng->m_pRecordChannel[iSeq]->GetState(), AUDIO_CHANNEL_IDLE);

	m_pAudioEng->StartSend(channelID);
	EXPECT_EQ(m_pAudioEng->PauseRecordChannel(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pRecordChannel[iSeq]->GetState(), AUDIO_CHANNEL_PAUSE);
    
#ifdef MAC_IOS
    usleep(100 *1000);
#endif
    
	m_pAudioEng->DeleteRecordChannel(channelID);
	m_pAudioEng->Terminate();

}

TEST_F(CWbxAudioEngineImplTest, UnPauseRecordChannel)
{
	m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreateRecordChannel();
	m_pAudioEng->StartSend(channelID);
	int iSeq = m_pAudioEng->GetRecordChannelSeq(channelID);
	EXPECT_EQ(m_pAudioEng->PauseRecordChannel(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pRecordChannel[iSeq]->GetState(), AUDIO_CHANNEL_PAUSE);

	EXPECT_EQ(m_pAudioEng->UnPauseRecordChannel(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pRecordChannel[iSeq]->GetState(), AUDIO_CHANNEL_START);

	//unpause a channel which in start status, invalid
	EXPECT_EQ(m_pAudioEng->UnPauseRecordChannel(channelID),WBXAE_ERROR_CHANNEL_STATE_MISTAKE);
	EXPECT_EQ(m_pAudioEng->m_pRecordChannel[iSeq]->GetState(), AUDIO_CHANNEL_START);

	m_pAudioEng->DeleteRecordChannel(channelID);
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, MaxNumOfPlaybackChannels)
{
    m_pAudioEng->Init();
	int iNum = 0;
	iNum = m_pAudioEng->MaxNumOfPlaybackChannels();
	EXPECT_EQ(iNum,MAX_PLAYBACK_CHANNEL_NUM);
}

TEST_F(CWbxAudioEngineImplTest, CreatePlaybackChannel)
{
    m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreatePlaybackChannel();
	int iSeq = m_pAudioEng->GetPlaybackChannelSeq(channelID);
	EXPECT_TRUE(iSeq >=0 && iSeq < MAX_PLAYBACK_CHANNEL_NUM);
	EXPECT_TRUE(m_pAudioEng->m_pPlaybackChannel[iSeq]!=NULL);

	//test create another playback channel
	int channelID2 = INVALID_CHANNEL_ID;
	for(int i =0;i < MAX_PLAYBACK_CHANNEL_NUM;i++)
	{
	 channelID2= m_pAudioEng->CreatePlaybackChannel();
	}
	EXPECT_EQ(channelID2,INVALID_CHANNEL_ID);

	m_pAudioEng->DeletePlaybackChannel(channelID);
	for(int i =0;i < MAX_PLAYBACK_CHANNEL_NUM;i++)
	{
		if(m_pAudioEng->m_pPlaybackChannel[i])
		{
			delete m_pAudioEng->m_pPlaybackChannel[i];
			m_pAudioEng->m_pPlaybackChannel[i] = NULL;
		}
	}
}

TEST_F(CWbxAudioEngineImplTest, DeletePlaybackChannel)
{
    m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreatePlaybackChannel();
	int iSeq = m_pAudioEng->GetPlaybackChannelSeq(channelID);
	m_pAudioEng->DeletePlaybackChannel(channelID);

	//After delete, this channel should NULL
	EXPECT_TRUE(iSeq >=0 && iSeq < MAX_PLAYBACK_CHANNEL_NUM);
	EXPECT_TRUE(m_pAudioEng->m_pPlaybackChannel[iSeq]==NULL);

	//test invalid channelID
	channelID = INVALID_CHANNEL_ID;
	AeResult rv= m_pAudioEng->DeletePlaybackChannel(channelID);
	EXPECT_EQ(rv,WBXAE_ERROR_INVALID_CHANNELID);
}

TEST_F(CWbxAudioEngineImplTest, PausePlaybackChannel)
{
	m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreatePlaybackChannel();
	int iSeq = m_pAudioEng->GetPlaybackChannelSeq(channelID);
	EXPECT_EQ(m_pAudioEng->PausePlaybackChannel(channelID),WBXAE_ERROR_CHANNEL_STATE_MISTAKE);
	EXPECT_EQ(m_pAudioEng->m_pPlaybackChannel[iSeq]->GetState(), AUDIO_CHANNEL_IDLE);

	//test After start.
	m_pAudioEng->StartPlayback(channelID);
	EXPECT_EQ(m_pAudioEng->PausePlaybackChannel(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pPlaybackChannel[iSeq]->GetState(), AUDIO_CHANNEL_PAUSE);

	m_pAudioEng->DeletePlaybackChannel(channelID);
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, UnPausePlaybackChannel)
{
	m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreatePlaybackChannel();
	m_pAudioEng->StartPlayback(channelID);
	int iSeq = m_pAudioEng->GetPlaybackChannelSeq(channelID);
	EXPECT_EQ(m_pAudioEng->PausePlaybackChannel(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pPlaybackChannel[iSeq]->GetState(), AUDIO_CHANNEL_PAUSE);

	EXPECT_EQ(m_pAudioEng->UnPausePlaybackChannel(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pPlaybackChannel[iSeq]->GetState(), AUDIO_CHANNEL_START);

	//test channel in start status
	EXPECT_EQ(m_pAudioEng->UnPausePlaybackChannel(channelID),WBXAE_ERROR_CHANNEL_STATE_MISTAKE);
	EXPECT_EQ(m_pAudioEng->m_pPlaybackChannel[iSeq]->GetState(), AUDIO_CHANNEL_START);

	m_pAudioEng->DeletePlaybackChannel(channelID);
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, SetSendTransport)
{
	m_pAudioEng->Init();
	IWbxAeTransport *pTranport = NULL;
	int channelID = m_pAudioEng->CreateRecordChannel();
	int iSeq = m_pAudioEng->GetRecordChannelSeq(channelID);

	m_pAudioEng->SetSendTransport(channelID,pTranport);
	EXPECT_TRUE(pTranport== m_pAudioEng->m_pRecordChannel[iSeq]->m_pTranport);

	m_pAudioEng->DeleteRecordChannel(channelID);
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, StartPlayback)
{
	//create;
	m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreatePlaybackChannel();
	int iSeq = m_pAudioEng->GetPlaybackChannelSeq(channelID);

	EXPECT_EQ(m_pAudioEng->StartPlayback(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pPlaybackChannel[iSeq]->GetState(), AUDIO_CHANNEL_START);
	EXPECT_EQ(TRUE,m_pAudioEng->m_bPlaybackStarted);

	//test invalid case
	EXPECT_EQ(m_pAudioEng->StartPlayback(INVALID_CHANNEL_ID),WBXAE_ERROR_INVALID_CHANNELID);

	//clean;
	m_pAudioEng->DeletePlaybackChannel(channelID);
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, StopPlayback)
{
	//create and start ;
	m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreatePlaybackChannel();
	int iSeq = m_pAudioEng->GetPlaybackChannelSeq(channelID);

	EXPECT_EQ(m_pAudioEng->StartPlayback(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pPlaybackChannel[iSeq]->GetState(), AUDIO_CHANNEL_START);
	EXPECT_EQ(TRUE,m_pAudioEng->m_bPlaybackStarted);

	//Stop it
	EXPECT_EQ(m_pAudioEng->StopPlayback(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pPlaybackChannel[iSeq]->GetState(), AUDIO_CHANNEL_IDLE);
	//EXPECT_EQ(FALSE,m_pAudioEng->m_bPlaybackStarted);

	//restart channel and create another channel;
	EXPECT_EQ(m_pAudioEng->StartPlayback(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pPlaybackChannel[iSeq]->GetState(), AUDIO_CHANNEL_START);
	EXPECT_EQ(TRUE,m_pAudioEng->m_bPlaybackStarted);

	int channelID2 = m_pAudioEng->CreatePlaybackChannel();
	int iSeq2 = m_pAudioEng->GetPlaybackChannelSeq(channelID2);
	EXPECT_EQ(m_pAudioEng->StartPlayback(channelID2),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pPlaybackChannel[iSeq2]->GetState(), AUDIO_CHANNEL_START);
	EXPECT_EQ(TRUE,m_pAudioEng->m_bPlaybackStarted);

	//stop 1 channel, Playback engine should not stopped.
	EXPECT_EQ(m_pAudioEng->StopPlayback(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pPlaybackChannel[iSeq]->GetState(), AUDIO_CHANNEL_IDLE);
	EXPECT_EQ(TRUE,m_pAudioEng->m_bPlaybackStarted);

	//stop channel 2,Playback engine should stoped.
	EXPECT_EQ(m_pAudioEng->StopPlayback(channelID2),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pPlaybackChannel[iSeq2]->GetState(), AUDIO_CHANNEL_IDLE);
	//EXPECT_EQ(FALSE,m_pAudioEng->m_bPlaybackStarted);

	EXPECT_EQ(m_pAudioEng->StopPlayback(INVALID_CHANNEL_ID),WBXAE_ERROR_INVALID_CHANNELID);

	//clean;
	m_pAudioEng->DeletePlaybackChannel(channelID);
	m_pAudioEng->DeletePlaybackChannel(channelID2);
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, StartSend)
{
	//create;
	m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreateRecordChannel();
	int iSeq = m_pAudioEng->GetRecordChannelSeq(channelID);

	EXPECT_EQ(m_pAudioEng->StartSend(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pRecordChannel[iSeq]->GetState(), AUDIO_CHANNEL_START);
	EXPECT_EQ(TRUE,m_pAudioEng->m_bCaptureStarted);

	//test invalid case
	EXPECT_EQ(m_pAudioEng->StartSend(INVALID_CHANNEL_ID),WBXAE_ERROR_INVALID_CHANNELID);

	//clean;
	m_pAudioEng->DeleteRecordChannel(channelID);
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, StopSend)
{
	//create and start ;
	m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreateRecordChannel();
	int iSeq = m_pAudioEng->GetRecordChannelSeq(channelID);

	EXPECT_EQ(m_pAudioEng->StartSend(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pRecordChannel[iSeq]->GetState(), AUDIO_CHANNEL_START);
	EXPECT_EQ(TRUE,m_pAudioEng->m_bCaptureStarted);

	//Stop it
	EXPECT_EQ(m_pAudioEng->StopSend(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pRecordChannel[iSeq]->GetState(), AUDIO_CHANNEL_IDLE);
	EXPECT_EQ(FALSE,m_pAudioEng->m_bCaptureStarted);

	//restart channel 
	EXPECT_EQ(m_pAudioEng->StartSend(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pRecordChannel[iSeq]->GetState(), AUDIO_CHANNEL_START);
	EXPECT_EQ(TRUE,m_pAudioEng->m_bCaptureStarted);

	//stop 1 channel, Playback engine should stopped.
	EXPECT_EQ(m_pAudioEng->StopSend(channelID),WBXAE_SUCCESS);
	EXPECT_EQ(m_pAudioEng->m_pRecordChannel[iSeq]->GetState(), AUDIO_CHANNEL_IDLE);
	EXPECT_EQ(FALSE,m_pAudioEng->m_bCaptureStarted);

	EXPECT_EQ(m_pAudioEng->StopSend(INVALID_CHANNEL_ID),WBXAE_ERROR_INVALID_CHANNELID);

	//clean;
	m_pAudioEng->DeleteRecordChannel(channelID);
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, SetNSStatus)
{
	BOOL enable = TRUE;
	WBX_NSLevel level = WBXAE_NS_LOW;
    m_pAudioEng->Init();
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetNSStatus(enable,level));
}

TEST_F(CWbxAudioEngineImplTest, GetNSStatus)
{
	m_pAudioEng->Init();
	bool enable = TRUE;
	WBX_NSLevel level = WBXAE_NS_LOW;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetNSStatus(enable,level));
	enable = FALSE;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetNSStatus(enable,level));
	EXPECT_EQ(true,enable);
	EXPECT_EQ(WBXAE_NS_LOW,level);

	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, SetMicAGCStatus)
{
	m_pAudioEng->Init();
	bool enable = true;
	const WBX_AGCMode mode = WBXAE_AGC_DIGITAL;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetMicAGCStatus(enable,mode));

	const WBX_AGCMode mode2 = WBXAE_AGC_ANALOG;
#if defined(MAC_IOS) || defined(ANDROID)
	EXPECT_NE(WBXAE_SUCCESS,m_pAudioEng->SetMicAGCStatus(enable,mode2));
#else
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetMicAGCStatus(enable,mode2));
#endif
  enable = false;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetMicAGCStatus(enable,mode2));

    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetMicAGCStatus)
{
  
	m_pAudioEng->Init();
	bool enable = true;
	const WBX_AGCMode mode = WBXAE_AGC_DIGITAL;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetMicAGCStatus(enable,mode));
	enable = false;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetMicAGCStatus(enable,mode));
	EXPECT_EQ(true,enable);

	const WBX_AGCMode mode2 = WBXAE_AGC_ANALOG;
#if defined(MAC_IOS) || defined(ANDROID)
	EXPECT_NE(WBXAE_SUCCESS,m_pAudioEng->SetMicAGCStatus(enable,mode2));
#else
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetMicAGCStatus(enable,mode2));
#endif

	enable = false;
  
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetMicAGCStatus(enable,mode2));
  
#if defined(MAC_IOS) || defined(ANDROID)
	EXPECT_NE(true,enable);
#else
	EXPECT_EQ(true,enable);
#endif


	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, SetSpeakerDVolumeScale)
{
    m_pAudioEng->Init();
	unsigned int volumeScale=50;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetSpeakerDVolumeScale(volumeScale));

	volumeScale=301;
	EXPECT_EQ(WBXAE_ERROR_INVALID_VALUE,m_pAudioEng->SetSpeakerDVolumeScale(volumeScale));
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetSpeakerDVolumeScale)
{
	m_pAudioEng->Init();
	unsigned int volumeScale=50;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetSpeakerDVolumeScale(volumeScale));

	volumeScale = 100;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetSpeakerDVolumeScale(volumeScale));
	EXPECT_EQ(50,volumeScale);
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, SetSpeakerAGCStatus)
{
	BOOL enable = FALSE;
    m_pAudioEng->Init();
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetSpeakerAGCStatus(enable));
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetSpeakerAGCStatus)
{
	m_pAudioEng->Init();
	bool enable = FALSE;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetSpeakerAGCStatus(enable));

	enable = TRUE;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetSpeakerAGCStatus(enable));
	EXPECT_EQ(false,enable);
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, SetECStatus)
{
	BOOL enable = FALSE;
    m_pAudioEng->Init();
	WBX_ECmode mode = WBXAE_EC_NONE;
	int nResult = m_pAudioEng->SetECStatus(enable,mode);
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetECStatus)
{
	m_pAudioEng->Init();
	bool enable = false;
	WBX_ECmode mode = WBXAE_EC_NONE;
	int nResult = m_pAudioEng->SetECStatus(enable,mode);
	if(WBXAE_SUCCESS == nResult)
	{
		enable=true;
		EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetECStatus(enable,mode));
		EXPECT_EQ(false,enable);
		EXPECT_EQ(WBXAE_EC_NONE,mode);
	}
	enable = true;
	nResult = m_pAudioEng->SetECStatus(enable,mode);
	if(WBXAE_SUCCESS == nResult)
	{
		enable=false;
		EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetECStatus(enable,mode));
		EXPECT_EQ(true,enable);
		EXPECT_EQ(WBXAE_EC_NONE,mode);
	}
	m_pAudioEng->Terminate();
}


TEST_F(CWbxAudioEngineImplTest, GetVersion)
{
    m_pAudioEng->Init();
	int Len= strlen(WBXAE_LIB_VERSION);
	unsigned int BufLen = Len-1;
	char * strVersion = new char[BufLen];
	EXPECT_EQ(WBXAE_ERROR_BUFFER_SIZE_LIMIT,m_pAudioEng->GetVersion(strVersion,BufLen));

	delete []strVersion;
	BufLen = Len +1;
	strVersion = new char[BufLen];
	
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetVersion(strVersion,BufLen));
	strVersion[Len] = 0;
	EXPECT_EQ(0,strcmp(strVersion,WBXAE_LIB_VERSION));
	EXPECT_EQ(BufLen,Len);
    m_pAudioEng->Terminate();
	delete []strVersion;
}

TEST_F(CWbxAudioEngineImplTest, GetLastError)
{
    m_pAudioEng->Init();
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetLastError());
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetCurrentPlaybackDelay)
{
	m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreatePlaybackChannel();
	int iSeq = m_pAudioEng->GetPlaybackChannelSeq(channelID);
	
	//not start, should error
	unsigned int lDelay=0;
	EXPECT_EQ(WBXAE_ERROR_CHANNEL_STATE_MISTAKE,m_pAudioEng->GetCurrentPlaybackDelay(channelID,lDelay));

	m_pAudioEng->StartPlayback(channelID);
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetCurrentPlaybackDelay(channelID,lDelay));

	EXPECT_EQ(WBXAE_ERROR_INVALID_CHANNELID,m_pAudioEng->GetCurrentPlaybackDelay(INVALID_CHANNEL_ID,lDelay));

	m_pAudioEng->DeletePlaybackChannel(channelID);
	m_pAudioEng->Terminate();

}

TEST_F(CWbxAudioEngineImplTest, GetCurrentCaptureDelay)
{
	m_pAudioEng->Init();
	unsigned int lDelay=0;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetCurrentCaptureDelay(INVALID_CHANNEL_ID,lDelay));

	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetNumberOfCodecs)
{
    m_pAudioEng->Init();
	int Num = m_pAudioEng->GetNumberOfCodecs();
	EXPECT_TRUE(Num>0);
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetCodec)
{
    m_pAudioEng->Init();
	WBX_CodecInst codeInst;
	//memset(&codeInst,0,sizeof(WBX_CodecInst));
	int Num = m_pAudioEng->GetNumberOfCodecs();
	for(int i=0;i< Num;i++)
	{
		EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetCodec(i,codeInst));
		EXPECT_TRUE(codeInst.plfreq > 0);
	}
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, SetSendCodec)
{
	m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreateRecordChannel();

	WBX_CodecInst codeInst;
	//memset(&codeInst,0,sizeof(WBX_CodecInst));
	int Num = m_pAudioEng->GetNumberOfCodecs();
	EXPECT_TRUE(Num>0);

	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetCodec(0,codeInst));
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetSendCodec(channelID,codeInst));

	EXPECT_EQ(WBXAE_ERROR_INVALID_CHANNELID,m_pAudioEng->SetSendCodec(INVALID_CHANNEL_ID,codeInst));

	m_pAudioEng->DeleteRecordChannel(channelID);
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetSendCodec)
{
	m_pAudioEng->Init();
	int channelID = m_pAudioEng->CreateRecordChannel();

	WBX_CodecInst codeInst;
	//memset(&codeInst,0,sizeof(WBX_CodecInst));
	int Num = m_pAudioEng->GetNumberOfCodecs();
	EXPECT_TRUE(Num>0);

	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetCodec(0,codeInst));
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetSendCodec(channelID,codeInst));

	WBX_CodecInst codeInst1;
	EXPECT_EQ(WBXAE_ERROR_INVALID_CHANNELID,m_pAudioEng->GetSendCodec(INVALID_CHANNEL_ID,codeInst1));
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetSendCodec(channelID,codeInst1));
	EXPECT_EQ(codeInst.pltype,codeInst1.pltype);

	m_pAudioEng->DeleteRecordChannel(channelID);
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetInputSpeechLevel)
{
	m_pAudioEng->Init();
	unsigned int level=0;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetInputSpeechLevel(level));
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetOutputSpeechLevel)
{
	m_pAudioEng->Init();
	unsigned int level=0;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetOutputSpeechLevel(level));
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, SetEncryptor)
{
    m_pAudioEng->Init();
	IWbxAe_Encryptor * pEncryptor = NULL;

	//Set Encryptor to playback channel
	int playbackChannelID = m_pAudioEng->GetPlaybackChannelID(MAX_PLAYBACK_CHANNEL_NUM-1);
	EXPECT_EQ(WBXAE_ERROR_INVALID_CHANNELID,m_pAudioEng->SetEncryptor(playbackChannelID,pEncryptor));

	playbackChannelID = m_pAudioEng->CreatePlaybackChannel();
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetEncryptor(playbackChannelID,pEncryptor));

	m_pAudioEng->StartPlayback(playbackChannelID);
	EXPECT_EQ(WBXAE_ERROR_CHANNEL_ALREADY_START,m_pAudioEng->SetEncryptor(playbackChannelID,pEncryptor));

	m_pAudioEng->DeletePlaybackChannel(playbackChannelID);

	//Set Encryptor to Record channel
	int recordChannelID = m_pAudioEng->GetRecordChannelID(MAX_RECORD_CHANNEL_NUM-1);
	EXPECT_EQ(WBXAE_ERROR_INVALID_CHANNELID,m_pAudioEng->SetEncryptor(recordChannelID,pEncryptor));

	recordChannelID = m_pAudioEng->CreateRecordChannel();
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetEncryptor(recordChannelID,pEncryptor));

	m_pAudioEng->StartSend(recordChannelID);
	EXPECT_EQ(WBXAE_ERROR_CHANNEL_ALREADY_START,m_pAudioEng->SetEncryptor(recordChannelID,pEncryptor));

	m_pAudioEng->DeleteRecordChannel(recordChannelID);
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, SetQosOption)
{
    m_pAudioEng->Init();
	WBXAE_QosType  type =WBXAE_QOS_SENDER_UPLINK_NETWORK;
	WBXAE_QOS_NETWORK_PARAM  qosNetWork;
	qosNetWork.bandwidth = 100;
	qosNetWork.packetLossrate = 10;
	m_pAudioEng->SetQosOption(type,&qosNetWork);

	type = WBXAE_QOS_RECEIVER_NETWORK;
	m_pAudioEng->SetQosOption(type,&qosNetWork);

	type = WBXAE_QOS_BASE;
	m_pAudioEng->SetQosOption(type,&qosNetWork);
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetQosOption)
{
}

TEST_F(CWbxAudioEngineImplTest, SetExternalMediaProcessing)
{
	m_pAudioEng->Init();
	WBXAE_ProcessingTypes type =WBXAE_PLAYBACK_PER_CHANNEL;
	bool benable = FALSE;
	IWbxMediaProcess *proccessObject = NULL;
	int channelID = m_pAudioEng->GetPlaybackChannelID(MAX_PLAYBACK_CHANNEL_NUM-1);

	EXPECT_EQ(WBXAE_ERROR_INVALID_CHANNELID,m_pAudioEng->SetExternalMediaProcessing(type,channelID,benable,proccessObject));

	channelID = m_pAudioEng->CreatePlaybackChannel();
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetExternalMediaProcessing(type,channelID,benable,proccessObject));

	m_pAudioEng->StartPlayback(channelID);
	EXPECT_EQ(WBXAE_ERROR_CHANNEL_ALREADY_START,m_pAudioEng->SetExternalMediaProcessing(type,channelID,benable,proccessObject));

	m_pAudioEng->DeletePlaybackChannel(channelID);

	type = WBXAE_PLAYBACK_ALL_CHANNELS_MIXED;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetExternalMediaProcessing(type,channelID,benable,proccessObject));

	type = WBXAE_RECORD_BEFORE_AQE;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetExternalMediaProcessing(type,channelID,benable,proccessObject));

	type = WBXAE_RECORD_BEFORE_ENCODER;
	EXPECT_EQ(WBXAE_ERROR_INVALID_CHANNELID,m_pAudioEng->SetExternalMediaProcessing(type,channelID,benable,proccessObject));

	type = (WBXAE_ProcessingTypes)0xfff;	
	EXPECT_EQ(WBXAE_ERROR_INVALID_VALUE,m_pAudioEng->SetExternalMediaProcessing(type,channelID,benable,proccessObject));

	type = WBXAE_RECORD_BEFORE_ENCODER;
	channelID = m_pAudioEng->CreateRecordChannel(0);
	m_pAudioEng->StartSend(channelID);
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetExternalMediaProcessing(type,channelID,benable,proccessObject));
    m_pAudioEng->StopSend(channelID);
    m_pAudioEng->DeleteRecordChannel(channelID);
	m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, SetErrorMessage)
{
    m_pAudioEng->Init();
 
	int errCode = 100;
	char * errorMessage= "test error" ;
	int channelID =0;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->SetErrorMessage(errCode,errorMessage,channelID));
	EXPECT_EQ(errCode,m_pAudioEng->m_iLastErrorCode);
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, SetQosInfo)
{
}

TEST_F(CWbxAudioEngineImplTest, AdjustMicVolume)
{
    m_pAudioEng->Init();
	int volume=0;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->AdjustMicVolume(volume));
    m_pAudioEng->Terminate();

}

TEST_F(CWbxAudioEngineImplTest, GetMicVolume)
{
    m_pAudioEng->Init();
	unsigned int volume=0;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->GetMicVolume(volume));
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, AdjustDSpeakerVolume)
{
    m_pAudioEng->Init();
	int volume=0;
	EXPECT_EQ(WBXAE_SUCCESS,m_pAudioEng->AdjustDSpeakerVolume(volume));
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetRecordChannelID)
{
    m_pAudioEng->Init();
	int seq=MAX_RECORD_CHANNEL_NUM-1;
	EXPECT_TRUE(INVALID_CHANNEL_ID!=m_pAudioEng->GetRecordChannelID(seq));

	seq = MAX_RECORD_CHANNEL_NUM +1;
	EXPECT_EQ(INVALID_CHANNEL_ID,m_pAudioEng->GetRecordChannelID(seq));
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetRecordChannelSeq)
{
    m_pAudioEng->Init();
	int seq=MAX_RECORD_CHANNEL_NUM-1;
	int channelID=m_pAudioEng->GetRecordChannelID(seq);

	EXPECT_EQ(seq,m_pAudioEng->GetRecordChannelSeq(channelID));
	EXPECT_EQ(INVALID_CHANNEL_ID,m_pAudioEng->GetRecordChannelSeq(INVALID_CHANNEL_ID));
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetPlaybackChannelID)
{
    m_pAudioEng->Init();
	int seq=MAX_PLAYBACK_CHANNEL_NUM-1;
	EXPECT_TRUE(INVALID_CHANNEL_ID!=m_pAudioEng->GetPlaybackChannelID(seq));

	seq = MAX_PLAYBACK_CHANNEL_NUM +1;
	EXPECT_EQ(INVALID_CHANNEL_ID,m_pAudioEng->GetPlaybackChannelID(seq));
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetPlaybackChannelSeq)
{
    m_pAudioEng->Init();
	int seq=MAX_PLAYBACK_CHANNEL_NUM-1;
	int channelID=m_pAudioEng->GetPlaybackChannelID(seq);

	EXPECT_EQ(seq,m_pAudioEng->GetPlaybackChannelSeq(channelID));
	EXPECT_EQ(INVALID_CHANNEL_ID,m_pAudioEng->GetPlaybackChannelSeq(INVALID_CHANNEL_ID));
    m_pAudioEng->Terminate();
}

TEST_F(CWbxAudioEngineImplTest, GetSource)
{
}

TEST_F(CWbxAudioEngineImplTest, GetFrameInterval)
{
}
TEST_F(CWbxAudioEngineImplTest, AlignSourceList)
{
}

TEST_F(CWbxAudioEngineImplTest, GetSoundEffectState4Device)
{
#if defined(WIN32) && !defined(WP8)
	WBXAEDEVICEID pDeviceID;
	memset(&pDeviceID, 0, sizeof(WBXAEDEVICEID));

	bool bChecked = false;
	AeResult rv = m_pAudioEng->GetSoundEffectState4Device(pDeviceID,bChecked);
	EXPECT_NE(rv, WBXAE_SUCCESS);

	IWBXDeviceEnumerator* pDeviceEnumerator = NULL;
	m_pAudioEng->GetDeviceEnumeratorInst(pDeviceEnumerator);
	int numberofDev = pDeviceEnumerator->GetNumOfMicrophones();
	if (numberofDev >= 1)
	{
		pDeviceEnumerator->GetSysDefaultMicrophone(pDeviceID);
	}
	rv = m_pAudioEng->GetSoundEffectState4Device(pDeviceID, bChecked);
	EXPECT_TRUE((rv == WBXAE_SUCCESS || rv == WBXAE_ERROR_KEYNOTFOUND));
	rv = m_pAudioEng->GetSoundEffectState4Device(pDeviceID, bChecked, 1);
	EXPECT_TRUE((rv == WBXAE_ERROR_NOTIMPL || rv == WBXAE_ERROR_KEYNOTFOUND));

	memset(&pDeviceID, 0, sizeof(WBXAEDEVICEID));
	numberofDev = pDeviceEnumerator->GetNumOfSpeakers();
	if (numberofDev >= 1)
	{
		pDeviceEnumerator->GetSysDefaultSpeaker(pDeviceID);
	}

	rv = m_pAudioEng->GetSoundEffectState4Device(pDeviceID, bChecked);
	EXPECT_TRUE((rv == WBXAE_SUCCESS || rv == WBXAE_ERROR_KEYNOTFOUND));
	rv = m_pAudioEng->GetSoundEffectState4Device(pDeviceID, bChecked, 1);
	EXPECT_TRUE((rv == WBXAE_ERROR_NOTIMPL || rv == WBXAE_ERROR_KEYNOTFOUND));
#endif
}
TEST_F(CWbxAudioEngineImplTest, SetGetOption)
{
#if defined(MX_IOS)
    bool bEnabled = false; // vpio is disabled
    
    SetAudioEngineOption(DolphinOption_IOSVPIOEnable, &bEnabled, sizeof(bEnabled));
    
    AudioAECAlgorithmType type = AudioAECAlgorithm_None;
    AeResult rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv == WBXAE_SUCCESS );
    
    AudioAECAlgorithmType gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_None );

    // not support  alpha aec
    type = AudioAECAlgorithm_Alpha;
    rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv != WBXAE_SUCCESS );
    
    gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_Webrtc );
    
    type = AudioAECAlgorithm_Webrtc;
    rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv == WBXAE_SUCCESS );
    
    gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_Webrtc );
    
    type = AudioAECAlgorithm_BuildIn;
    rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv == WBXAE_ERROR_NOTIMPL );
    
    gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_Webrtc );
    
    
    type = AudioAECAlgorithm_TC;
    rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv == WBXAE_SUCCESS );
    
    gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_TC );

#elif defined(ANDROID)
    AudioAECAlgorithmType type = AudioAECAlgorithm_None;
    AeResult rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv == WBXAE_SUCCESS );
    
    AudioAECAlgorithmType gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_None );
    
    type = AudioAECAlgorithm_Alpha;
    rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv == WBXAE_SUCCESS );
    
    gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_Alpha );
    
    type = AudioAECAlgorithm_Webrtc;
    rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv == WBXAE_SUCCESS );
    
    gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_Webrtc );
    
    type = AudioAECAlgorithm_BuildIn;
    rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv == WBXAE_ERROR_NOTIMPL );
    
    gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_Webrtc );
    
    
    type = AudioAECAlgorithm_TC;
    rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv == WBXAE_SUCCESS );
    
    gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_TC );

#elif defined(MACOS) || defined(WIN32)
    AudioAECAlgorithmType type = AudioAECAlgorithm_None;
    AeResult rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv == WBXAE_SUCCESS );
    
    AudioAECAlgorithmType gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_None );
    
    type = AudioAECAlgorithm_Alpha;
    rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv == WBXAE_SUCCESS );
 
    gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_Alpha );
    
    type = AudioAECAlgorithm_Webrtc;
    rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv == WBXAE_SUCCESS );

    gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_Webrtc );

    type = AudioAECAlgorithm_BuildIn;
    rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv == WBXAE_ERROR_NOTIMPL );
 
    gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_Webrtc );

    
    type = AudioAECAlgorithm_TC;
    rv = SetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(rv == WBXAE_ERROR_NOTIMPL );
    
    gettype = AudioAECAlgorithm_None;
    rv = GetAudioEngineOption(DolphinOption_AECAlgoType, &type, sizeof(type));
    EXPECT_TRUE(type == AudioAECAlgorithm_Webrtc );
#endif
}


#endif
