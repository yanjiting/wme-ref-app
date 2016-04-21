/*****************************************************************************************
*Generate by AUT Tool on 2011-01-04
*class:CWbxAePlaybackChannelTest
********************************************************************************************/

#ifndef WINVER                  // Specifies that the minimum required platform is Windows XP.
#define WINVER 0x0500           // Change this to the appropriate value to target other versions of Windows.
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#define private public
#define protected public

//#include "wbxmock.h"
#include "WbxAeAudioChannel.h"


using testing::Return;
using testing::_;
using testing::Invoke;
using testing::ReturnRef;
using testing::Matcher;

#define CHANNEL_SSRC 1234

class CWbxAePlaybackChannelTest : public testing::Test
{
public:
    CWbxAePlaybackChannelTest()
    {
    }

    virtual ~CWbxAePlaybackChannelTest()
    {
    }

    virtual void SetUp()
    {
		WBXWAVEFORMAT waveFormat;
		waveFormat.dwSize = sizeof(WBXWAVEFORMAT);
		waveFormat.nSamplesPerSec = WBXAE_DEFAULT_SAMPLE_RATE;
		waveFormat.nChannels = WBXAE_AUDIO_SAMPLE_CHANNELS;
		waveFormat.wBitsPerSample = WBXAE_AUDIO_SAMPLE_SIZE * 8 *waveFormat.nChannels;
		waveFormat.nBlockAlign =waveFormat.wBitsPerSample / 8;
		m_pChannel = new CWbxAePlaybackChannel(1,waveFormat);
    }

    virtual void TearDown()
    {
		delete m_pChannel;
    }

protected:
	CWbxAePlaybackChannel * m_pChannel;
};

TEST_F(CWbxAePlaybackChannelTest, InputRtpData)
{
	RTP_Frame *pRtp = new RTP_Frame();
	pRtp->set_payload_size(m_pChannel->m_waveFormat.nSamplesPerSec/1000 * m_pChannel->m_waveFormat.nBlockAlign * 30);
	pRtp->set_payload_type(WBXAE_AUDIO_PAYLOAD_TYPE_SILK_24K);
	pRtp->set_sequence_number(1);
	pRtp->set_sync_source(CHANNEL_SSRC);
	pRtp->set_extension(false);
	
	m_pChannel->Start();
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->InputRtpData(pRtp->get_buffer(),pRtp->get_frame_size()));
	EXPECT_EQ(1, m_pChannel->m_pJitterBuffer->Size());

	CAudioJitterBuffer * pTemp = m_pChannel->m_pJitterBuffer;
	m_pChannel->m_pJitterBuffer = NULL;
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->InputRtpData(pRtp->get_buffer(),pRtp->get_frame_size()));
	m_pChannel->m_pJitterBuffer = pTemp;

	m_pChannel->m_pJitterBuffer->RemoveFirstPacket();
	if(pRtp)
		delete pRtp;
}
TEST_F(CWbxAePlaybackChannelTest,PrepareRawData)
{
	CAudioJitterBuffer * pTemp = m_pChannel->m_pJitterBuffer;
	m_pChannel->m_pJitterBuffer = NULL;
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->PrepareRawData(100));
	m_pChannel->m_pJitterBuffer = pTemp;

	//input and get 
	RTP_Frame *pRtp = new RTP_Frame();
	pRtp->set_payload_size(m_pChannel->m_waveFormat.nSamplesPerSec/1000 * m_pChannel->m_waveFormat.nBlockAlign * 30);
	pRtp->set_payload_type(WBXAE_AUDIO_PAYLOAD_TYPE_SILK_24K);
	pRtp->set_sequence_number(1);
	pRtp->set_sync_source(CHANNEL_SSRC);
	pRtp->set_extension(false);

	m_pChannel->Start();
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->InputRtpData(pRtp->get_buffer(),pRtp->get_frame_size()));
	EXPECT_EQ(1, m_pChannel->m_pJitterBuffer->Size());

	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->PrepareRawData(100));

}

TEST_F(CWbxAePlaybackChannelTest, GetRawData)
{
	RTP_Frame *pRtp = new RTP_Frame();
	pRtp->set_payload_size(m_pChannel->m_waveFormat.nSamplesPerSec/1000 * m_pChannel->m_waveFormat.nBlockAlign * 20);
	pRtp->set_payload_type(WBXAE_AUDIO_PAYLOAD_TYPE_SILK_24K);
	pRtp->set_sequence_number(1);
	pRtp->set_sync_source(CHANNEL_SSRC);
	pRtp->set_extension(false);

	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->InputRtpData(pRtp->get_buffer(),pRtp->get_frame_size()));

	delete pRtp;

	pRtp = new RTP_Frame();
	pRtp->set_payload_size(m_pChannel->m_waveFormat.nSamplesPerSec/1000 * m_pChannel->m_waveFormat.nBlockAlign * 20);
	pRtp->set_payload_type(WBXAE_AUDIO_PAYLOAD_TYPE_SILK_24K);
	pRtp->set_sequence_number(2);
	pRtp->set_sync_source(CHANNEL_SSRC);
	pRtp->set_extension(false);

	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->InputRtpData(pRtp->get_buffer(),pRtp->get_frame_size()));
	delete pRtp;

	BYTE * pSrc = NULL;
	CWbxAeMediaBlock * pMediaBlock = NULL;
	DWORD dwRequiredLen = m_pChannel->m_waveFormat.nSamplesPerSec/1000 * m_pChannel->m_waveFormat.nBlockAlign * 30;
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->GetRawData(pSrc,dwRequiredLen,pMediaBlock));

	if(pMediaBlock)
		pMediaBlock->Release();

	m_pChannel->m_pJitterBuffer->RemoveFirstPacket();
	m_pChannel->m_MBlist.CleanList();
}
TEST_F(CWbxAePlaybackChannelTest,GetRawDataBufferLen)
{
	DWORD tmp = m_pChannel->GetRawDataBufferLen();
}

TEST_F(CWbxAePlaybackChannelTest, SetMediaProcess)
{
	IWbxMediaProcess * pWbxMediaProcess = NULL;
	m_pChannel->SetMediaProcess(FALSE,pWbxMediaProcess);
	m_pChannel->SetMediaProcess(TRUE,pWbxMediaProcess);
}

TEST_F(CWbxAePlaybackChannelTest, GetType)
{
	EXPECT_EQ(WBXAE_AUDIO_CHANNEL_PLAYBACK,m_pChannel->GetType());
}

TEST_F(CWbxAePlaybackChannelTest, DoDecode)
{
	CWbxAERTPSample* pRtpSample = NULL;
	CWbxAeMediaBlock * pMediaBlock = NULL;
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->DoDecode(pRtpSample,pMediaBlock));

	BYTE * pTemp = new BYTE[100];
	pRtpSample = new CWbxAERTPSample(pTemp,100,0,true,0,0,0);
	m_pChannel->m_pAudioCodec = NULL;
	m_pChannel->DoDecode(pRtpSample,pMediaBlock);
}

TEST_F(CWbxAePlaybackChannelTest, DoDecodePlc)
{
	CWbxAeMediaBlock * pMediaBlock = NULL;
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->DoDecodePlc(pMediaBlock));
}

TEST_F(CWbxAePlaybackChannelTest, SetCodecByPayload)
{
	m_pChannel->SetCodecByPayload(m_pChannel->m_CodecType.pltype);
}

TEST_F(CWbxAePlaybackChannelTest, AlignSrcList)
{
	RTP_Frame *pRtp = new RTP_Frame();
	pRtp->set_payload_size(m_pChannel->m_waveFormat.nSamplesPerSec/1000 * m_pChannel->m_waveFormat.nBlockAlign * 30);
	pRtp->set_payload_type(WBXAE_AUDIO_PAYLOAD_TYPE_SILK_24K);
	pRtp->set_sequence_number(1);
	pRtp->set_sync_source(CHANNEL_SSRC);
	pRtp->set_extension(false);

	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->InputRtpData(pRtp->get_buffer(),pRtp->get_frame_size()));

	delete pRtp;

	BYTE * pSrc = NULL;
	CWbxAeMediaBlock * pMediaBlock = NULL;
	DWORD dwRequiredLen = m_pChannel->m_waveFormat.nSamplesPerSec/1000 * m_pChannel->m_waveFormat.nBlockAlign * 20;
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->GetRawData(pSrc,dwRequiredLen,pMediaBlock));

	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->AlignSrcList());
	EXPECT_EQ(0, m_pChannel->m_MBlist.GetDataLen());

}

TEST_F(CWbxAePlaybackChannelTest, GetChannelDelay)
{
	DWORD tmpDelay = m_pChannel->GetChannelDelay();

}

