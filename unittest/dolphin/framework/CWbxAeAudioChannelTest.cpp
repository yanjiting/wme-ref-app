/*****************************************************************************************
*Generate by AUT Tool on 2011-01-04
*class:CWbxAeAudioChannelTest
********************************************************************************************/

#ifndef WINVER                  // Specifies that the minimum required platform is Windows XP.
#define WINVER 0x0500           // Change this to the appropriate value to target other versions of Windows.
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
//#include "wbxmock.h"
#include "CmMutex.h"
#include "CmEventQueueBase.h"

#define private public
#define protected public

#include "WbxAeAudioChannel.h"



using testing::Return;
using testing::_;
using testing::Invoke;
using testing::ReturnRef;
using testing::Matcher;


class CWbxAeAudioChannelTest : public testing::Test
{
public:
    CWbxAeAudioChannelTest()
    {
    }

    virtual ~CWbxAeAudioChannelTest()
    {
    }

    virtual void SetUp()
    {
		WBXWAVEFORMAT waveFormat;
		waveFormat.dwSize = sizeof(WBXWAVEFORMAT);
		waveFormat.nSamplesPerSec = WBXAE_DEFAULT_SAMPLE_RATE;
		m_pChannel = new CWbxAeAudioChannel(1,waveFormat);
    }

    virtual void TearDown()
    {
		delete m_pChannel;
    }

protected:
	CWbxAeAudioChannel * m_pChannel;
};

TEST_F(CWbxAeAudioChannelTest, Start)
{
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->Start());
	EXPECT_EQ(AUDIO_CHANNEL_START,m_pChannel->m_lState);
}

TEST_F(CWbxAeAudioChannelTest, Stop)
{
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->Stop());
	EXPECT_EQ(AUDIO_CHANNEL_IDLE,m_pChannel->m_lState);
}

TEST_F(CWbxAeAudioChannelTest, Pause)
{
	m_pChannel->SetState(AUDIO_CHANNEL_IDLE);
	EXPECT_EQ(WBXAE_ERROR_CHANNEL_STATE_MISTAKE,m_pChannel->Pause());

	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->Start());
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->Pause());
}

TEST_F(CWbxAeAudioChannelTest, UnPause)
{
	m_pChannel->SetState(AUDIO_CHANNEL_IDLE);
	EXPECT_EQ(WBXAE_ERROR_CHANNEL_STATE_MISTAKE,m_pChannel->UnPause());

	m_pChannel->SetState(AUDIO_CHANNEL_PAUSE);
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->UnPause());
}

TEST_F(CWbxAeAudioChannelTest, GetState)
{
	CHANNEL_STATE stat = m_pChannel->GetState();
	EXPECT_EQ(stat,m_pChannel->m_lState);
}

TEST_F(CWbxAeAudioChannelTest, SetEncryptor)
{
	IWbxAe_Encryptor * pEncryptor = NULL;
	m_pChannel->SetEncryptor(pEncryptor);
	EXPECT_TRUE(pEncryptor == m_pChannel->m_pEncryptor);
}

TEST_F(CWbxAeAudioChannelTest, SetState)
{
	CHANNEL_STATE lState = AUDIO_CHANNEL_PAUSE;
	m_pChannel->SetState(lState);
	EXPECT_EQ(lState,m_pChannel->m_lState);
}

TEST_F(CWbxAeAudioChannelTest, SetCodecInst)
{
	WBX_CodecInst  codecInst;
	unsigned int dwSupportedNum=0;
	WbxAEGetSupportedCodecNum(dwSupportedNum);
	EXPECT_TRUE(dwSupportedNum > 0);
	EnumerateWbxAECodec(codecInst,dwSupportedNum-1);


	EXPECT_EQ(WBXAE_ERROR_INVALID_CHANNEL_TYPE,m_pChannel->SetCodecInst(codecInst));
	EXPECT_TRUE(m_pChannel->m_pAudioCodec == NULL);
}

TEST_F(CWbxAeAudioChannelTest, GetType)
{
	EXPECT_EQ(WBXAE_AUDIO_CHANNEL_BASE,m_pChannel->GetType());
}

TEST_F(CWbxAeAudioChannelTest, IsSameCodecInst)
{

	WBX_CodecInst  codecInst;
	unsigned int dwSupportedNum=0;
	WbxAEGetSupportedCodecNum(dwSupportedNum);
	EXPECT_TRUE(dwSupportedNum > 0);
	EnumerateWbxAECodec(codecInst,dwSupportedNum-2);

	EXPECT_TRUE(!m_pChannel->IsSameCodecInst(codecInst));
	m_pChannel->m_CodecType.pltype = codecInst.pltype;
	EXPECT_TRUE(!m_pChannel->IsSameCodecInst(codecInst));

}
TEST_F(CWbxAeAudioChannelTest, SetMediaType)
{
	WBXWAVEFORMAT waveFormat;
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->SetMediaType(waveFormat));

	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->Start());
	EXPECT_EQ(WBXAE_ERROR_CHANNEL_ALREADY_START,m_pChannel->SetMediaType(waveFormat));

}


