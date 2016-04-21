/*****************************************************************************************
*Generate by AUT Tool on 2011-01-04
*class:CWbxAeRecordChannelTest
********************************************************************************************/

#ifndef WINVER                  // Specifies that the minimum required platform is Windows XP.
#define WINVER 0x0500           // Change this to the appropriate value to target other versions of Windows.
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
//#include "wbxmock.h"
#include "WbxAeAudioChannel.h"
#define private public
#define protected public

using testing::Return;
using testing::_;
using testing::Invoke;
using testing::ReturnRef;
using testing::Matcher;

class CWbxAeRecordChannelTest : public testing::Test
{
public:
    CWbxAeRecordChannelTest()
    {
    }

    virtual ~CWbxAeRecordChannelTest()
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
		m_pChannel = new CWbxAeRecordChannel(MAX_PLAYBACK_CHANNEL_NUM +1,waveFormat);
    }

    virtual void TearDown()
    {
		delete m_pChannel;
    }

protected:
	CWbxAeRecordChannel * m_pChannel;
};

TEST_F(CWbxAeRecordChannelTest, OnCaptureData)
{
	CWbxAeMediaBlock * p1 = new CWbxAeMediaBlock(WBX_MEMORY_BLOCK_SIZE);
	//p1->AddRef(); 

	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->Stop());
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->OnCaptureData(p1));
	
	//Set Codec
	WBX_CodecInst  codecInst;
	DWORD dwSupportedNum=0;
	WbxAEGetSupportedCodecNum(dwSupportedNum);
	EXPECT_TRUE(dwSupportedNum > 0);
	for(int i = 0; i < dwSupportedNum; i++)
	{
		EnumerateWbxAECodec(codecInst,i);
		if (codecInst.pltype == WBXAE_AUDIO_PAYLOAD_TYPE_SILK_24K)
		{
			break;
		}

	}
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->Stop());
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->SetSendCodec(codecInst));

	//Start and set
	DWORD dwSize= m_pChannel->m_dwFrameSize / 2;
	p1->AdvanceWritePtr(dwSize);
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->Start());
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->OnCaptureData(p1));

	CWbxAeMediaBlock * p2 = new CWbxAeMediaBlock(WBX_MEMORY_BLOCK_SIZE);
	p2->AdvanceWritePtr(dwSize);
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->OnCaptureData(p2));

	EXPECT_EQ(0,m_pChannel->m_MBlist.m_CaptureDataList.size());

}

TEST_F(CWbxAeRecordChannelTest, SetTranport)
{
	IWbxAeTransport *pTransport = NULL;
	m_pChannel->SetTranport(pTransport);
	EXPECT_TRUE(pTransport==m_pChannel->m_pTranport);
}

TEST_F(CWbxAeRecordChannelTest, SetSendCodec)
{
	WBX_CodecInst  codecInst;
	DWORD dwSupportedNum=0;
	WbxAEGetSupportedCodecNum(dwSupportedNum);
	EXPECT_TRUE(dwSupportedNum > 0);
	EnumerateWbxAECodec(codecInst,dwSupportedNum-1);

	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->Start());
	EXPECT_EQ(WBXAE_ERROR_CHANNEL_ALREADY_START,m_pChannel->SetSendCodec(codecInst));

	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->Stop());
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->SetSendCodec(codecInst));

	EXPECT_TRUE(m_pChannel->IsSameCodecInst(codecInst));

}

TEST_F(CWbxAeRecordChannelTest, GetSendCodec)
{
	WBX_CodecInst  codecInst;
	DWORD dwSupportedNum=0;
	WbxAEGetSupportedCodecNum(dwSupportedNum);
	EXPECT_TRUE(dwSupportedNum > 0);
	EnumerateWbxAECodec(codecInst,dwSupportedNum-1);

	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->Stop());
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->SetSendCodec(codecInst));

	WBX_CodecInst codecInst2;
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->GetSendCodec(codecInst2));

	EXPECT_TRUE(m_pChannel->IsSameCodecInst(codecInst2));
	
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->GetSendCodec(m_pChannel->m_CodecType));

}

TEST_F(CWbxAeRecordChannelTest, Pause)
{
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->Start());

	CWbxAeMediaBlock * p1 = new CWbxAeMediaBlock(WBX_MEMORY_BLOCK_SIZE);
	p1->AdvanceWritePtr(100);
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->OnCaptureData(p1));
	EXPECT_EQ(1,m_pChannel->m_MBlist.m_CaptureDataList.size());

	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->Pause());
	EXPECT_EQ(1,m_pChannel->m_MBlist.m_CaptureDataList.size());	
	m_pChannel->m_MBlist.CleanList();
}

TEST_F(CWbxAeRecordChannelTest, GetType)
{
	EXPECT_EQ(WBXAE_AUDIO_CHANNEL_RECORD,m_pChannel->GetType());
}

TEST_F(CWbxAeRecordChannelTest, DoEnccode)
{
	BYTE * pSrc = NULL;
	DWORD dwSrcLen = 0;
	BYTE * pDest = new BYTE [200];
	DWORD dwDestLen = 99;
	DWORD dwResult = 0;
	EXPECT_EQ(WBXAE_ERROR_NULL_POINTER,m_pChannel->DoEnccode(pSrc,dwSrcLen,pDest,dwDestLen,dwResult));

	pSrc = new BYTE[100];
	dwSrcLen = 100;
	EXPECT_EQ(WBXAE_ERROR_MEMORY_OVERWRITE,m_pChannel->DoEnccode(pSrc,dwSrcLen,pDest,dwDestLen,dwResult));

	dwDestLen = 200;
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->DoEnccode(pSrc,dwSrcLen,pDest,dwDestLen,dwResult));

	EXPECT_EQ(dwSrcLen,dwResult);

	delete []pSrc;
	delete []pDest;
}


TEST_F(CWbxAeRecordChannelTest, BuildRtpHead)
{
	BYTE * pSrc = new BYTE [200];
	DWORD dwSrcLen = 200;
	RTP_Frame * pRtpPacket = NULL;
	DWORD dwResult = 0;

	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->BuildRtpHead(pSrc,dwSrcLen,pRtpPacket,dwResult));
	EXPECT_TRUE(pRtpPacket != NULL);
	EXPECT_EQ(dwResult, dwSrcLen + RTP_HEAD_SIZE);
	if(pRtpPacket)
		delete pRtpPacket;
	
	m_pChannel->m_wRTPSequence=0xffff;
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->BuildRtpHead(pSrc,dwSrcLen,pRtpPacket,dwResult));
	EXPECT_TRUE(pRtpPacket != NULL);
	EXPECT_EQ(dwResult, dwSrcLen + RTP_HEAD_SIZE);
	if(pRtpPacket)
		delete pRtpPacket;
	delete pSrc;
}

TEST_F(CWbxAeRecordChannelTest, SetSenderNetWorkSatus)
{
	WBX_CodecInst  codecInst;
	DWORD dwSupportedNum=0;
	WbxAEGetSupportedCodecNum(dwSupportedNum);
	EXPECT_TRUE(dwSupportedNum > 0);
	EnumerateWbxAECodec(codecInst,dwSupportedNum-1);

	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->Stop());
	EXPECT_EQ(WBXAE_SUCCESS,m_pChannel->SetSendCodec(codecInst));

	WBXAE_QOS_NETWORK_PARAM  qosNetWork;
	qosNetWork.bandwidth = 100;
	qosNetWork.packetLossrate = 10;

	m_pChannel->SetSenderNetWorkSatus(&qosNetWork);


}
