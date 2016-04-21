/*****************************************************************************************
*Generate by AUT Tool on 2011-01-06
*class:CWbxAeVoiceMixerTest
********************************************************************************************/
#ifndef WINVER                  // Specifies that the minimum required platform is Windows XP.
#define WINVER 0x0500           // Change this to the appropriate value to target other versions of Windows.
#endif
#include "gtest/gtest.h"
#include "gmock/gmock.h"
//#include "wbxmock.h"

#define private public
#define protected public

#include "WbxAeVoiceMixer.h"
#include "WbxAudioEngineImpl.h"



using testing::Return;
using testing::_;
using testing::Invoke;
using testing::ReturnRef;
using testing::Matcher;

class CWbxAeVoiceMixerTest : public testing::Test
{
public:
    CWbxAeVoiceMixerTest()
    {
    }

    virtual ~CWbxAeVoiceMixerTest()
    {
    }

    virtual void SetUp()
    {
		m_pAe= new CWbxAudioEngineImpl();
        m_pAe->Init();
		WBXWAVEFORMAT waveFormat;
		waveFormat.dwSize = sizeof(WBXWAVEFORMAT);
		waveFormat.nSamplesPerSec = WBXAE_DEFAULT_SAMPLE_RATE;
		waveFormat.nChannels = WBXAE_AUDIO_SAMPLE_CHANNELS;
		waveFormat.wBitsPerSample = WBXAE_AUDIO_SAMPLE_SIZE * 8 *waveFormat.nChannels;
		waveFormat.nBlockAlign =waveFormat.wBitsPerSample / 8;
		
		m_pMixer = new CWbxAeVoiceMixer(m_pAe,waveFormat);
    }

    virtual void TearDown()
    {
		delete m_pMixer;
        m_pAe->Terminate();
        delete m_pAe;
		
    }

protected:
	CWbxAeVoiceMixer * m_pMixer;
    CWbxAudioEngineImpl * m_pAe;
};

TEST_F(CWbxAeVoiceMixerTest, SetMediaProcess)
{
	m_pMixer->SetMediaProcess(TRUE,NULL);
}

TEST_F(CWbxAeVoiceMixerTest, SetWaveFormat)
{
	WBXWAVEFORMAT waveFormat;
	waveFormat.dwSize = sizeof(WBXWAVEFORMAT);
	waveFormat.nSamplesPerSec = AE_SAMPLE_RATE_24K;
	waveFormat.nChannels =1;

	m_pMixer->SetWaveFormat(waveFormat);
	EXPECT_EQ(waveFormat.dwSize,m_pMixer->m_waveFormat.dwSize);
	EXPECT_EQ(waveFormat.nSamplesPerSec,m_pMixer->m_waveFormat.nSamplesPerSec);
	EXPECT_EQ(waveFormat.nChannels,m_pMixer->m_waveFormat.nChannels);
}
TEST_F(CWbxAeVoiceMixerTest, AddReceiver)
{
	m_pMixer->AddReceiver(NULL);
}

TEST_F(CWbxAeVoiceMixerTest, RemoveReceiver)
{
	m_pMixer->RemoveReceiver(NULL);
	CWBXAE_AQE *pTemp = new CWBXAE_AQE();
	m_pMixer->AddReceiver(pTemp);
	//m_pMixer->RemoveReceiver(pTemp);

	delete pTemp;

}

TEST_F(CWbxAeVoiceMixerTest, CleanReceiver)
{
	m_pMixer->CleanReceiver();
}

TEST_F(CWbxAeVoiceMixerTest, Start)
{
// 	m_pMixer->Start();
// 	m_pMixer->m_pThread = NULL;
// 	m_pMixer->Start();
}

TEST_F(CWbxAeVoiceMixerTest, Stop)
{
	m_pMixer->Stop();
}




TEST_F(CWbxAeVoiceMixerTest, MixStream)
{
	m_pMixer->MixStream(NULL,NULL,0,0);

	short* pDesAudioData = new short[100];
	short *pSrcAduioData[2];
	for(int i = 0;i<2;i++)
	{
		pSrcAduioData[i] = new short[100];
		memset(pSrcAduioData[i],i+2,100);
	}

	m_pMixer->MixStream(pDesAudioData,pSrcAduioData,2,2);

	delete []pDesAudioData;
	for(int i = 0;i<2;i++)
	{
		delete []pSrcAduioData[i];
	}



}

TEST_F(CWbxAeVoiceMixerTest,DoMix)
{
	m_pMixer->DoMix(0);

	m_pMixer->m_pMeidaBlock[0] = new CWbxAeMediaBlock(WBX_MEMORY_BLOCK_SIZE);
	m_pMixer->m_pMeidaBlock[0]->AdvanceWritePtr(100);
	m_pMixer->m_pMeidaBlock[0]->AddReference();
	m_pMixer->DoMix(2);

	m_pMixer->m_pReceiver = NULL;
	m_pMixer->DoMix(2);

	m_pMixer->DoMix(1);

}
