
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WseVideoListenChannel.h"
#include "SvcClientEngineMock.h"
#include "WseVideoDecoderFactory.h"
#include "WseVideoSample.h"
#include "WseEngineImp.h"
#include "WseRtpMonitor.h"


#include "CWseUtil.h"

#define NOW   static_cast<unsigned long>(wse_ticker::now()/1000)


class CWseVideoListenChannelTest : public testing::Test
{
public:
	CWseVideoListenChannelTest()
	{
		pWseVideoSampleAllocator = new CWseVideoSampleAllocator(64);
        pWseVideoSampleAllocator->AddRef();
		pEngine = new CWseEngineImp();
		mMonitor = new CWseRtpMonitor(0, NULL, NULL);
		pChannel = new CWseVideoListenChannel(pEngine,0,mMonitor,WSE_REALTIME_APP,pWseVideoSampleAllocator);
        m_uiCSI = 2000;
		//pChannel->AddRef();
		//pChannel->StartReceive(WseAVCStream, videoCodecH264SVC, Wse_LCTM_Sync);
        EXPECT_CALL(pNotifySink, OnKeyFrameLost(_)).WillRepeatedly(Return(0));
        EXPECT_CALL(pNotifySink, OnDecodedFrame(_,_,_,_)).WillRepeatedly(Return(0));
        EXPECT_CALL(pNotifySink, OnListenParameterChanged(_,_,_)).WillRepeatedly(Return(0));
        EXPECT_CALL(pNotifySink, OnRenderBlocked(_,_)).WillRepeatedly(Return(0));
        EXPECT_CALL(pNotifySink, OnMediaDataInputInterrupt(_,_)).WillRepeatedly(Return(0));
	}

	virtual ~CWseVideoListenChannelTest()
	{
		delete mMonitor;
		delete pChannel;
		delete pEngine;
        pWseVideoSampleAllocator->Release();
	}

	virtual void SetUp()
	{

	}

	virtual void TearDown()
	{

	}

	static WSERESULT EnumVideoDeliverCallback_fn(IWseVideoDeliverer* pVideoDeliverer, void* lpContext)
	{
		return WSE_S_OK;
	}

public:
	CWseVideoListenChannel *pChannel;
	CWseEngineImp *pEngine;
	CWseRtpMonitor *mMonitor;
	IWseVideoSampleAllocator *pWseVideoSampleAllocator;
    MockIWseVideoListenChannelNotifySink pNotifySink;
    UINT32 m_uiCSI;
};



TEST_F(CWseVideoListenChannelTest, Initialize)
{
	CWseVideoListenChannel *pChannel2 = new CWseVideoListenChannel(NULL,0,mMonitor,WSE_REALTIME_APP,pWseVideoSampleAllocator);
	EXPECT_EQ(WSE_E_POINTER, pChannel2->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Sync));
	delete pChannel2;

	unsigned long srcID1, srcID2;
	srcID1 = 1000;
	srcID2 = 2000;
	EXPECT_EQ(WSE_S_OK, pChannel->SetSourceID(srcID1));
	srcID2 = pChannel->GetSourceID();
	EXPECT_EQ(srcID2, srcID2);

	EXPECT_EQ(WSE_S_OK,pChannel->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Sync));
}

TEST_F(CWseVideoListenChannelTest, SetGetParams)
{
	EXPECT_EQ(WSE_S_OK,pChannel->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Sync));

	unsigned long dwValue,size;
	void* pParam = NULL;
	size = 0;
	dwValue = true;
	pChannel->SetParams(WSE_P2P_PARAM, dwValue, pParam, size);
	pChannel->GetParams(WSE_P2P_PARAM, dwValue, pParam, size);
	EXPECT_EQ(true, dwValue != 0);
	dwValue = false;
	pChannel->SetParams(WSE_P2P_PARAM, dwValue, pParam, size);
	pChannel->GetParams(WSE_P2P_PARAM, dwValue, pParam, size);
	EXPECT_EQ(false, dwValue != 0);

	pChannel->GetParams(WSE_BLOCK_THRESDHOLD, dwValue, pParam, size);
	EXPECT_EQ(5000, dwValue);
	dwValue = 2000;
	pChannel->SetParams(WSE_BLOCK_THRESDHOLD, dwValue, pParam, size);
	pChannel->GetParams(WSE_BLOCK_THRESDHOLD, dwValue, pParam, size);
	EXPECT_EQ(2000, dwValue);
	pChannel->SetParams(WSE_BLOCK_STATUS, dwValue, pParam, size);
	pChannel->GetParams(WSE_BLOCK_THRESDHOLD, dwValue, pParam, size);
	EXPECT_EQ(-1, pChannel->m_iBlockStatus);

	dwValue = 10;
	pChannel->SetParams(WSE_STATISTIC_ONLY, dwValue, pParam, size);
}

TEST_F(CWseVideoListenChannelTest, RegisterComponent)
{
	EXPECT_EQ(WSE_S_OK,pChannel->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Sync));

	IWseDecoder *dec = WseVideoDecoderFactory::instance()->CreateVideoDecoder(WseI420, videoCodecH264SVC, WseSoftwareVideoCodec, NULL, pWseVideoSampleAllocator);

    void* pDecoder = NULL;
	pChannel->RegisterComponent(WSE_DECODER, dec, NULL);
	pChannel->GetComponent(WSE_DECODER, &pDecoder);
	EXPECT_EQ(dec, pDecoder);
	pChannel->SetComponentStatus(WSE_DECODER, true);
	EXPECT_EQ(WSE_S_OK, pChannel->UnRegisterComponent(WSE_DECODER, dec));

	MockIWseEncryptSink encrypt,encrypt2;
	pChannel->SetComponentStatus(WSE_CRYPTION, 1);
}

TEST_F(CWseVideoListenChannelTest, RecvNALData)
{
	EXPECT_EQ(WSE_S_OK,pChannel->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Sync));
	EXPECT_TRUE(pChannel->IsInSyncMode());
	pChannel->Start();

	wrtp::WRTPMediaDataVideo mediaData;
    wrtp::WRTPMediaDataIntegrityInfo dataIntegrityInfo;

	mediaData.data = new UINT8[10];
	mediaData.size = 10;
	mediaData.marker = 0;
	mediaData.timestamp = 1000;
	mediaData.type = wrtp::MEDIA_DATA_VIDEO;
	dataIntegrityInfo.packetCount = 0;
	dataIntegrityInfo.startSequence = 1000;
	mediaData.data[0] = 6;

	mediaData.timestamp ++;
    mediaData.csrcArray = &m_uiCSI;
    mediaData.lastDataOfFrame = true;
	dataIntegrityInfo.startSequence ++;
	EXPECT_EQ(WSE_S_OK,pChannel->OnRecvMediaData(&mediaData,&dataIntegrityInfo));
	mediaData.timestamp ++;
	dataIntegrityInfo.startSequence ++;
	CPacketElemNode packetnode = {WSE_NALU, mediaData.data, mediaData.size, mediaData.sampleTimestamp/90, mediaData.timestamp, mediaData.timestampRecovered, static_cast<unsigned short>(dataIntegrityInfo.startSequence), mediaData.priority, 0, 0, 0, NULL};
    int tmp = (pChannel->m_pReceivingBuffer == NULL)?0:1;
    EXPECT_NE(0, tmp);
	EXPECT_EQ(WSE_S_OK, pChannel->m_pReceivingBuffer->AddPacket(packetnode));
	mediaData.timestamp ++;
	dataIntegrityInfo.startSequence ++;
	EXPECT_EQ(WSE_S_OK, pChannel->m_pReceivingBuffer->AddPacket(packetnode));
	mediaData.timestamp ++;
	dataIntegrityInfo.startSequence -= 5;
	EXPECT_EQ(WSE_S_OK, pChannel->m_pReceivingBuffer->AddPacket(packetnode));

	EXPECT_EQ(WSE_S_OK,pChannel->Stop());
}


TEST_F(CWseVideoListenChannelTest, RecvNALData_AysncMode)
{
	EXPECT_EQ(WSE_S_OK,pChannel->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Split));
	EXPECT_FALSE(pChannel->IsInSyncMode());
	pChannel->Start();

	wrtp::WRTPMediaDataVideo mediaData;
    wrtp::WRTPMediaDataIntegrityInfo dataIntegrityInfo;

	mediaData.data = new UINT8[10];
    memset(mediaData.data, 0, 10*sizeof(UINT8));
	mediaData.size = 10;
	mediaData.marker = 0;
	mediaData.timestamp = 0;
    mediaData.csrcArray = &m_uiCSI;
	mediaData.type = wrtp::MEDIA_DATA_VIDEO;
    mediaData.lastDataOfFrame = true;
	dataIntegrityInfo.packetCount = 0;
	dataIntegrityInfo.startSequence = 0;
	CPacketElemNode packetnode = {WSE_NALU, mediaData.data, mediaData.size, mediaData.sampleTimestamp/90, mediaData.timestamp, mediaData.timestampRecovered, static_cast<unsigned short>(dataIntegrityInfo.startSequence), mediaData.priority, 0, 0, 0, NULL};
    int tmp = (pChannel->m_pReceivingBuffer == NULL)?0:1;
    EXPECT_NE(0, tmp );
	EXPECT_EQ(WSE_S_OK,pChannel->OnRecvMediaData(&mediaData,&dataIntegrityInfo));
	EXPECT_EQ(WSE_S_OK, pChannel->m_pReceivingBuffer->AddPacket(packetnode));

	EXPECT_EQ(WSE_S_OK,pChannel->Stop());
}

TEST_F(CWseVideoListenChannelTest, SetRTPChannel)
{

	EXPECT_EQ(WSE_S_OK,pChannel->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Sync));

	//EXPECT_EQ(WSE_S_OK, pSrcChannel->Initialize(WSE_REALTIME_APP));
	MockIRTPChannel cRTPChannel;
    EXPECT_CALL(cRTPChannel, IncreaseReference()).WillOnce(Return(0));
    EXPECT_CALL(cRTPChannel, DecreaseReference()).WillOnce(Return(0));
    EXPECT_CALL(cRTPChannel, SetPacketizationOperator(_)).WillRepeatedly(Return());
    EXPECT_CALL(cRTPChannel, SetMediaDataRecvSink(_)).WillRepeatedly(Return());
	EXPECT_EQ(WSE_S_OK, pChannel->SetRTPChannel((wrtp::IRTPChannel*)&cRTPChannel));
	EXPECT_EQ(WSE_S_OK, pChannel->SetRTPChannel(NULL));
}

TEST_F(CWseVideoListenChannelTest, TestVideoDeliver)
{
	EXPECT_EQ(WSE_S_OK,pChannel->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Sync));

	IWseVideoDeliverer *pDeliver = new MockIWseVideoDeliverer;
	EXPECT_EQ(WSE_S_OK, pChannel->AppendVideoDeliverer(pDeliver));
	EXPECT_EQ(WSE_S_OK, pChannel->EnumVideoDeliverer(EnumVideoDeliverCallback_fn, NULL));
	EXPECT_EQ(WSE_S_OK, pChannel->RemoveVideoDeliverer(pDeliver));
	delete pDeliver;

	IWseVideoSample* pSample = NULL;
	ASSERT_NO_FATAL_FAILURE(pChannel->DoDeliverImage(pSample));

	IAVSyncListenController* pSink = NULL;
	EXPECT_EQ(WSE_S_OK, pChannel->SetAVSyncListenController(pSink, AV_SYNC_ID_TYPE_SOURCE_ID));
	EXPECT_FALSE(pChannel->IsPlayControlEnabled());
	pChannel->m_bPlayControlEnabled = true;
	EXPECT_TRUE(pChannel->IsPlayControlEnabled());
	pChannel->m_pPlayControl = NULL;
	pChannel->m_bPlayControlEnabled = true;
	EXPECT_FALSE(pChannel->IsPlayControlEnabled());
	pChannel->m_bPlayControlEnabled = false;
	EXPECT_FALSE(pChannel->IsPlayControlEnabled());
}


TEST_F(CWseVideoListenChannelTest, TestQIInterfaces)
{
	void* ppv = NULL;
	EXPECT_EQ(WSE_S_OK, pChannel->QueryInterface(WSEIID_IWseVideoListenChannel, &ppv));
	EXPECT_EQ(WSE_S_OK, pChannel->QueryInterface(WSEIID_IWseVideoProvider, &ppv));
	EXPECT_EQ(JL_E_POINTER, pChannel->QueryInterface(WSEIID_IWseVideoListenChannel, NULL));
	EXPECT_EQ(JL_E_NOINTERFACE, pChannel->QueryInterface(WSEIID_IWseVideoSourceChannel, &ppv));
}

TEST_F(CWseVideoListenChannelTest, TestDecodeMode)
{
	EXPECT_EQ(WSE_S_OK,pChannel->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Sync));

	unsigned long dwSSRC = 1001;
	EXPECT_EQ(WSE_S_OK,pChannel->SetSourceID(dwSSRC));
	EXPECT_EQ(dwSSRC,pChannel->GetSourceID());
}



TEST_F(CWseVideoListenChannelTest, TestTimer)
{
    const static int MinLTRRecoverFrameRequestInterval = 500;
    const static int MinKeyFrameRequestInterval = 500;
	EXPECT_EQ(WSE_S_OK,pChannel->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Sync));

	EXPECT_EQ(WSE_S_OK, pChannel->SetListenChannelSink(&pNotifySink));

	// TODO:
	LTR_Recover_Request_s ltr_req;
	memset(&ltr_req, 0, sizeof(LTR_Recover_Request_s));
	EXPECT_EQ(WSE_E_POINTER, pChannel->OnLTRrecoveryRequest(NULL));
	pChannel->m_dwLastLTRRecoverRequest = 0;
	ASSERT_NO_FATAL_FAILURE(pChannel->OnLTRrecoveryRequest(&ltr_req));
	pChannel->m_dwLastLTRRecoverRequest = NOW;
	ASSERT_NO_FATAL_FAILURE(pChannel->OnLTRrecoveryRequest(&ltr_req));
	pChannel->m_dwLastLTRRecoverRequest = NOW - MinLTRRecoverFrameRequestInterval*2;
	ASSERT_NO_FATAL_FAILURE(pChannel->OnLTRrecoveryRequest(&ltr_req));

	LTR_Marking_Feedback_s ltr_fb;
	memset(&ltr_fb, 0, sizeof(LTR_Marking_Feedback_s));
	ASSERT_NO_FATAL_FAILURE(pChannel->OnLTRMarking(NULL));
	ASSERT_NO_FATAL_FAILURE(pChannel->OnLTRMarking(&ltr_fb));

	EXPECT_EQ(WSE_S_OK, pChannel->OnKeyFrameLost());
	pChannel->m_dwLastKFL = NOW - (MinKeyFrameRequestInterval/2);
	EXPECT_EQ(WSE_S_OK, pChannel->OnKeyFrameLost());
	pChannel->m_dwLastKFL = NOW - (MinKeyFrameRequestInterval*2);
	EXPECT_EQ(WSE_S_OK, pChannel->OnKeyFrameLost());

	CCmTimerWrapperID timer;
	ASSERT_NO_FATAL_FAILURE(pChannel->OnTimer(&timer));
	
    WseVideoFormat format = {WseI420, 360, 180, 0.0, 0};
    int len = format.width * format.height * 1.5;
	IWseVideoSample* pSample = NULL;
	EXPECT_EQ(WSE_S_OK, pWseVideoSampleAllocator->GetSample(format, &pSample));
	pSample->SetVideoFormat(&format);
	pSample->SetDataLength(len);

#if 0 // yongzxu: unsupport type of videoFormatAVC
	ASSERT_NO_FATAL_FAILURE(pChannel->OnDecoded(videoFormatAVC, 0, 180, 90, pSample));
#endif
	ASSERT_NO_FATAL_FAILURE(pChannel->OnDecoded(pSample, false));
	pChannel->m_iBlockStatus = 0;
	pChannel->m_dwBlockThreshold = 0;

	unsigned long nowTime = NOW;
	pChannel->m_ulTrafficStartingStatTime[0] = nowTime - 1001;
	pChannel->m_ulTrafficStartingStatTime[1] = nowTime - 1001;
	pChannel->m_ulTrafficStartingStatTime[2] = nowTime - 1001;
	pChannel->m_ulTrafficStartingStatTime[3] = nowTime - 1001;
	pChannel->m_ulTrafficStartingStatTime[4] = nowTime - 1001;
	ASSERT_NO_FATAL_FAILURE(pChannel->OnTimer(&timer));

	pChannel->m_bFpsChanged = true;
	ASSERT_NO_FATAL_FAILURE(pChannel->OnTimer(&timer));
}


TEST_F(CWseVideoListenChannelTest, TestOnDecoded)
{
	EXPECT_EQ(WSE_S_OK, pChannel->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Sync));

    WseVideoFormat format = {WseI420, 360, 180, 0.0, 0};
    int len = format.width * format.height * 1.5;
	IWseVideoSample* pSample = NULL;
	EXPECT_EQ(WSE_S_OK, pWseVideoSampleAllocator->GetSample(format, &pSample));
	pSample->SetVideoFormat(&format);
	pSample->SetDataLength(len);
    
	ASSERT_NO_FATAL_FAILURE(pChannel->OnDecoded(pSample, false));

	pChannel->m_iBlockStatus = 1;
    ASSERT_NO_FATAL_FAILURE(pChannel->OnDecoded(pSample, false));

	pChannel->m_FpsMonitor = NULL;
	ASSERT_NO_FATAL_FAILURE(pChannel->OnDecoded(pSample, false));
}

TEST_F(CWseVideoListenChannelTest, TestOnDecoded_Split)
{
    //for now we disable the Wse_LCTM_Split_PlaySyncAfterDecode mode which is the same as Wse_LCTM_Split.
	EXPECT_EQ(WSE_S_OK,pChannel->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Split));

    WseVideoFormat format = {WseI420, 360, 180, 0.0, 0};
    int len = format.width * format.height * 1.5;
	IWseVideoSample* pSample = NULL;
	EXPECT_EQ(WSE_S_OK, pWseVideoSampleAllocator->GetSample(format, &pSample));
	pSample->SetVideoFormat(&format);
	pSample->SetDataLength(len);
    
    ASSERT_NO_FATAL_FAILURE(pChannel->OnDecoded(pSample, false));
	ASSERT_NO_FATAL_FAILURE(pChannel->OnDecoded(pSample, false));
}

TEST_F(CWseVideoListenChannelTest, TestDecodeAndRender)
{
	EXPECT_EQ(WSE_S_OK, pChannel->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Split));
	pChannel->Start();

    const unsigned int MAX_FRAME_PACKET_SIZE = 64000;
	int size = MAX_FRAME_PACKET_SIZE+16;
	unsigned char *data = new unsigned char[size];
	CPacketElemNode packetnode = {WSE_NALU, data, static_cast<DWORD>(size), 0, 0, 0, 0, 0, 0, 0, 0, NULL};
	CPacketElemNode packetnode1 = {WSE_ENCODED_FRAME, data, static_cast<DWORD>(size), 0, 0, 0, 0, 0, 0, 0, 0, NULL};
    int tmp = (pChannel->m_pReceivingBuffer == NULL)?0:1;
    EXPECT_NE(0, tmp );
	EXPECT_EQ(WSE_S_OK, pChannel->m_pReceivingBuffer->AddPacket(packetnode));
	EXPECT_EQ(WSE_S_OK, pChannel->m_pReceivingBuffer->AddPacket(packetnode1));

	EXPECT_EQ(WSE_S_OK, pChannel->m_pReceivingBuffer->AddPacket(packetnode));
	EXPECT_EQ(WSE_S_OK, pChannel->m_pReceivingBuffer->AddPacket(packetnode1));

	EXPECT_EQ(WSE_S_OK, pChannel->m_pReceivingBuffer->AddPacket(packetnode));
	EXPECT_EQ(WSE_S_OK, pChannel->m_pReceivingBuffer->AddPacket(packetnode1));

	EXPECT_EQ(WSE_S_OK,pChannel->Stop());
}

TEST_F(CWseVideoListenChannelTest, TestClearOldData)
{
	EXPECT_EQ(WSE_S_OK, pChannel->ResetDecoderState());
	pChannel->m_bInitialized = true;
	EXPECT_EQ(WSE_S_OK, pChannel->ResetDecoderState());
}

TEST_F(CWseVideoListenChannelTest, TestSetRenderAspectRatio)
{
}


TEST_F(CWseVideoListenChannelTest, TestOnRecvMediaDataConditionals)
{
	wrtp::WRTPMediaData mediaData;
    wrtp::WRTPMediaDataIntegrityInfo dataIntegrityInfo;

	mediaData.data = NULL;
	mediaData.size = 0;
	mediaData.marker = 0;
	mediaData.timestamp = 0;
    mediaData.csrcArray = &m_uiCSI;
	mediaData.type = wrtp::MEDIA_DATA_VOIP;
	dataIntegrityInfo.packetCount = 0;
	dataIntegrityInfo.startSequence = 0;
    

    unsigned char nal[27] = {0x00, 0x00, 0x00, 0x01, 0x67, 0x42, 0xC0, 0x1F, 0x8C, 0x8D, 0x40, 0x28, 0x02, 0xD9, 0x00, 0xF0, 0x88, 0x46, 0xA0, 0x00, 0x00, 0x00, 0x01, 0x68, 0xCE, 0x3C, 0x80};


	pChannel->m_bInitialized = true;

    mediaData.data = nal;
	mediaData.size = 27;
	pChannel->m_bStartReceive = true;
    
    // fix 64bit overflow
	//EXPECT_EQ((INT32)WSE_E_POINTER, pChannel->OnRecvMediaData(&mediaData, NULL));
	//EXPECT_EQ((INT32)WSE_E_FAIL, pChannel->OnRecvMediaData(&mediaData, &dataIntegrityInfo));

	mediaData.type = wrtp::MEDIA_DATA_VIDEO;
	pChannel->m_lctm = Wse_LCTM_Split;
	EXPECT_EQ((INT32)WSE_S_OK, pChannel->OnRecvMediaData(&mediaData, &dataIntegrityInfo));
}

TEST_F(CWseVideoListenChannelTest, TestMiscInterfaces)
{
	EXPECT_EQ(WSE_S_OK,pChannel->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Sync));

	EXPECT_EQ(WSE_S_OK, pChannel->ForceProcessPendingData());
	EXPECT_EQ(WSE_S_OK, pChannel->Flush());

	EXPECT_EQ(WSE_S_OK, pChannel->Reset());
	EXPECT_EQ(WSE_S_OK, pChannel->ResetDecoderState());
	EXPECT_EQ(WSE_S_OK, pChannel->EnableSmoothPlay(true));
	EXPECT_EQ(WSE_S_OK, pChannel->ResetBlockStatus());

	unsigned long threshold = 100;
	EXPECT_EQ(WSE_S_OK, pChannel->SetBlockThreshold(threshold));
	EXPECT_EQ(threshold, pChannel->GetBlockThreshold());
	EXPECT_EQ(WSE_S_OK, pChannel->OnParameterChanged(WSE_LISTEN_CHANNEL_JITTER, 10));
	EXPECT_EQ(WSE_S_OK, pChannel->OnParameterChanged(WSE_LISTEN_CHANNEL_DELAY, 10));
	EXPECT_EQ(WSE_S_OK, pChannel->OnParameterChanged(WSE_LISTEN_CHANNEL_BANDWIDTH, 1024));
	EXPECT_EQ(WSE_S_OK, pChannel->OnParameterChanged(WSE_LISTEN_CHANNEL_LOSSRATE, 10));

	EXPECT_EQ(WSE_S_OK, pChannel->SetP2Pmode(true));
	EXPECT_EQ(true, pChannel->GetP2Pmode());

	EXPECT_EQ(WSE_S_OK, pChannel->OnMessage(CODEC_KEY_FRAME_LOST, NULL));

	void *ppv = NULL;
	EXPECT_EQ(WSE_S_OK, pChannel->GetComponent(WSE_CRYPTION, &ppv));

	pChannel->m_dwRenderFrameCount = 1;
	EXPECT_EQ(1, pChannel->GetRenderFrameCount());
	EXPECT_EQ(WSE_S_OK, pChannel->SetMaxRtpBufferSize(0));
	EXPECT_EQ(pChannel->m_dwCurrentRenderFrameTimeStamp, pChannel->GetCurrentRenderFrameTimeStamp());
}

TEST_F(CWseVideoListenChannelTest, TestVerifyDecodedFPS)
{
#if !defined(ANDROID) && !defined (WP8)
    static const float diffThreshHold = 0.2;
    static const int frameNum = 40;
    float fFPS = 0;
    
    
    EXPECT_EQ(WSE_S_OK, pChannel->Initialize(WseI420,videoCodecH264AVC,Wse_LCTM_Sync));
    pChannel->Start();
    
	EXPECT_EQ(WSE_S_OK, pChannel->SetListenChannelSink(&pNotifySink));
    
    WseVideoFormat format = {WseI420, 360, 180, 0.0, 0};
    int len = format.width * format.height * 1.5;
	IWseVideoSample* pSample = NULL;
	EXPECT_EQ(WSE_S_OK, pWseVideoSampleAllocator->GetSample(format, &pSample));
	pSample->SetVideoFormat(&format);
	pSample->SetDataLength(len);
    
    // expect 0 at firstly
    fFPS = pChannel->m_fRenderFps;
    EXPECT_EQ(0, fFPS);
    // test FPS 6, input 6 decoded sample
    for (int iDecodedNum = 0; iDecodedNum < frameNum; iDecodedNum++)
    {
    	ASSERT_NO_FATAL_FAILURE(pChannel->OnDecoded(pSample, false));
    }

#if defined (WIN32)
	WseSleep(2000);
#else
    WseSleep(2000*1000); // wait 2s
#endif
    ASSERT_NO_FATAL_FAILURE(pChannel->OnTimer(pChannel->m_pNotificationTimer));
    float fpsReal = frameNum*1000/(2000);
    fFPS = pChannel->m_fRenderFps;
    
    EXPECT_LE(fFPS, fpsReal*(1+diffThreshHold));
#endif
}


#if defined(IPHONEOS) && !defined(IOS_SIMULATOR)
#include "CWseResourceForUT.h"
#include <unistd.h>

static unsigned char* DetectStartCodePrefix (const unsigned char* kpBuf, int* pOffset, int iBufSize) {
    unsigned char* pBits = (unsigned char*)kpBuf;
    do {
        int iIdx = 0;
        while ((iIdx < iBufSize) && (! (*pBits))) {
            ++ pBits;
            ++ iIdx;
        }
        if (iIdx >= iBufSize) break;
        
        ++ iIdx;
        ++ pBits;
        
        if ((iIdx >= 3) && ((* (pBits - 1)) == 0x1)) {
            *pOffset = (unsigned int) (((unsigned long)pBits) - ((unsigned long)kpBuf));
            return pBits;
        }
        iBufSize -= iIdx;
    }while (1);
    return NULL;
}
/* TODO: redesign it as timer changed
TEST_F(CWseVideoListenChannelTest, testListenChannelSplitModePerformance)
{
    //Check whether this device iphone5 above
    if ((sDeviceInfo.pDeviceInfo[6] - '0')<5) {
        return;
    }
    EXPECT_EQ(WSE_S_OK, pChannel->Initialize(WseI420,videoCodecH264SVC,Wse_LCTM_Split));
    pChannel->Start();
    unsigned char* pBuf = new unsigned char[sResourceInfo.uiBitSreamForDecoderControlLen];
    unsigned char* pData = NULL;
    unsigned char* pDataNext = NULL;
    memcpy(pBuf, sResourceInfo.pBitStreamForDecoderControl, sResourceInfo.uiBitSreamForDecoderControlLen);
    int iOffset, iBufferSize;
    iBufferSize  = sResourceInfo.uiBitSreamForDecoderControlLen;
    
    unsigned char* bFrameData = NULL;
    unsigned int uiFrameDataLen = 0;
    unsigned int uiTimeStampGap = 0;
    unsigned int uiCSRC = 100;
    wrtp::WRTPMediaDataVideo sVideoFrameInfo;
    wrtp::WRTPMediaDataIntegrityInfo sDataIntegrityInfo;
    pData = DetectStartCodePrefix(pBuf, &iOffset, iBufferSize);
    unsigned int uiTime = 3; //unit second
    unsigned int uiFps = 30; //inputting fps

    uiTimeStampGap = (1000/uiFps)*90;
    unsigned long ulTime = NOW;
    for (int i =0; i<uiTime*uiFps; i++) {
        iBufferSize-=iOffset;
        pDataNext = DetectStartCodePrefix(pData, &iOffset, iBufferSize);
        if (pDataNext == NULL) {
            uiFrameDataLen = iBufferSize;
            bFrameData = pData;
            iBufferSize = sResourceInfo.uiBitSreamForDecoderControlLen;
            pData = DetectStartCodePrefix(pBuf, &iOffset, iBufferSize);
        }else{
            uiFrameDataLen = pDataNext-pData-4;
            bFrameData = pData;
            pData = pDataNext;
        }
        memset(&sVideoFrameInfo, 0, sizeof(sVideoFrameInfo));
        sVideoFrameInfo.data = bFrameData;
        sVideoFrameInfo.size = uiFrameDataLen;
        sVideoFrameInfo.sampleTimestamp = uiTimeStampGap*i;
        sVideoFrameInfo.type = wrtp::MEDIA_DATA_VIDEO;
        sVideoFrameInfo.csrcArray = &uiCSRC;
        ++sVideoFrameInfo.lastDataOfFrame = true;
        sDataIntegrityInfo.startSequence = i;
        pChannel->OnRecvMediaData(&sVideoFrameInfo, &sDataIntegrityInfo);
        usleep(1000*1000/uiFps);
    }
    ulTime = NOW - ulTime;
    float fFps = pChannel->m_dwRenderFrameCount*1000/ulTime;
    EXPECT_GE(fFps, 25);
    delete [] pBuf;
    
}

TEST_F(CWseVideoListenChannelTest, testListenChannelSyncModePerformance)
{
    //Check whether this device iphone5 above
    if ((sDeviceInfo.pDeviceInfo[6] - '0')<5) {
        return;
    }
    EXPECT_EQ(WSE_S_OK, pChannel->Initialize(WseI420,videoCodecH264SVC,Wse_LCTM_Sync));
    pChannel->Start();
    unsigned char* pBuf = new unsigned char[sResourceInfo.uiBitSreamForDecoderControlLen];
    unsigned char* pData = NULL;
    unsigned char* pDataNext = NULL;
    memcpy(pBuf, sResourceInfo.pBitStreamForDecoderControl, sResourceInfo.uiBitSreamForDecoderControlLen);
    int iOffset, iBufferSize;
    iBufferSize  = sResourceInfo.uiBitSreamForDecoderControlLen;
    
    unsigned char* bFrameData = NULL;
    unsigned int uiFrameDataLen = 0;
    unsigned int uiTimeStampGap = 0;
    unsigned int uiCSRC = 100;
    wrtp::WRTPMediaDataVideo sVideoFrameInfo;
    wrtp::WRTPMediaDataIntegrityInfo sDataIntegrityInfo;
    pData = DetectStartCodePrefix(pBuf, &iOffset, iBufferSize);
    unsigned int uiTime = 3; //unit second
    unsigned int uiFps = 50; //inputting fps
    
    uiTimeStampGap = (1000/uiFps)*90;
    unsigned long ulTime = NOW;
    for (int i =0; i<uiTime*uiFps; i++) {
        iBufferSize-=iOffset;
        pDataNext = DetectStartCodePrefix(pData, &iOffset, iBufferSize);
        if (pDataNext == NULL) {
            uiFrameDataLen = iBufferSize;
            bFrameData = pData;
            iBufferSize = sResourceInfo.uiBitSreamForDecoderControlLen;
            pData = DetectStartCodePrefix(pBuf, &iOffset, iBufferSize);
        }else{
            uiFrameDataLen = pDataNext-pData-4;
            bFrameData = pData;
            pData = pDataNext;
        }
        memset(&sVideoFrameInfo, 0, sizeof(sVideoFrameInfo));
        sVideoFrameInfo.data = bFrameData;
        sVideoFrameInfo.size = uiFrameDataLen;
        sVideoFrameInfo.sampleTimestamp = uiTimeStampGap*i;
        sVideoFrameInfo.type = wrtp::MEDIA_DATA_VIDEO;
        sVideoFrameInfo.csrcArray = &uiCSRC;
        ++sVideoFrameInfo.lastDataOfFrame = true;
        sDataIntegrityInfo.startSequence = i;
        pChannel->OnRecvMediaData(&sVideoFrameInfo, &sDataIntegrityInfo);
    }
    ulTime = NOW - ulTime;
    float fFps = pChannel->m_dwRenderFrameCount*1000/ulTime;
    pChannel->Stop();
    EXPECT_GE(fFps, 60);
    delete [] pBuf;
}
*/
#undef NOW

#endif
