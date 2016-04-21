#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "CmThreadManager.h"
#include "wrtpapi.h"
#include "rtputils.h"
#include "wrtpclientapi.h"
#include "wrtppacket.h"

#include "WMEInterfaceMock.h"
#include "rtpsessionclient.h"
#include "testutil.h"

using namespace wrtp;
using namespace wme;

class CWmeCodecPayloadTypeMediaTransportMock : public IWmeMediaTransport
{
public:
    CWmeCodecPayloadTypeMediaTransportMock(IRTPSessionClient *recvSession, IRTPChannel *recvChannel)
        : m_recvSession(recvSession)
        , m_recvChannel(recvChannel)
        , m_payloadType(0)
    {

    }


    virtual ~CWmeCodecPayloadTypeMediaTransportMock()
    {

    }

    void Reset()
    {
        m_recvSession = nullptr;
        m_recvChannel = nullptr;
    }

    virtual WMERESULT GetTransportType(WmeTransportType &eTransportType)
    {
        eTransportType = WmeTransportType_UDP;
        return WME_S_OK;
    }

    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage)
    {
        EXPECT_TRUE(nullptr != pRTPPackage);
        if (!m_recvChannel) {
            return WRTP_ERR_NOERR;
        }

        unsigned char *pBuffer = nullptr;
        uint32_t bufferOffset = 0;
        uint32_t bufferLen = 0;
        WMERESULT wmeRet;
        wmeRet = pRTPPackage->GetDataPointer(&pBuffer);
        EXPECT_FALSE(WME_FAILED(wmeRet));
        EXPECT_TRUE(nullptr != pBuffer);

        wmeRet = pRTPPackage->GetDataOffset(bufferOffset);
        EXPECT_FALSE(WME_FAILED(wmeRet));

        wmeRet = pRTPPackage->GetDataLength(bufferLen);
        EXPECT_FALSE(WME_FAILED(wmeRet));
        EXPECT_TRUE(0 != bufferLen);

        CCmMessageBlock mb(bufferLen, (LPCSTR)pBuffer+bufferOffset, 0, bufferLen);

        CRTPPacketLazy rtpPacketLazy;
        EXPECT_EQ(WRTP_ERR_NOERR, rtpPacketLazy.Bind(mb));

        EXPECT_EQ(m_payloadType,rtpPacketLazy.GetPayloadType());
        // forward the RTP packet to recving channel

        m_recvChannel->ReceiveRTPPacket(pRTPPackage);
        return WME_S_OK;
    }

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        return WME_S_OK;
    }

    void SetPayloadType(uint8_t payloadType)
    {
        m_payloadType = payloadType;
    }
private:
    IRTPSessionClient  *m_recvSession;
    IRTPChannel        *m_recvChannel;
    uint8_t m_payloadType;
};

class CWmeMediaDataRecvSinkMock:public IMediaDataRecvSink
{
public:
    CWmeMediaDataRecvSinkMock(IRTPSessionClient *recvSession, IRTPChannel *recvChannel)
        : m_recvSession(recvSession)
        , m_recvChannel(recvChannel)
        ,m_codeType(0) {}

    virtual int32_t OnRecvMediaData(WRTPMediaData *pData, WRTPMediaDataIntegrityInfo *pIntegrityInfo)
    {
        EXPECT_TRUE(nullptr != pData);
        EXPECT_TRUE(nullptr != pIntegrityInfo);

        EXPECT_EQ(m_codeType, pData->codecType);
        return 0;
    }

    void SetCodecType(uint32_t codecType)
    {
        m_codeType = codecType;
    }
private:
    IRTPSessionClient  *m_recvSession;
    IRTPChannel        *m_recvChannel;
    uint32_t              m_codeType;
};

class CWmeCodecPayloadTypeAPITest
    :public ::testing::Test
{
public:
    CWmeCodecPayloadTypeAPITest(): m_SendSession(nullptr)
        , m_SendChannel(nullptr)
        , m_RecvSession(nullptr)
        , m_RecvChannel(nullptr)
        , m_mediaSourceId(123)
        , m_streamId(1)
        , m_framemarkingURI("http://protocols.cisco.com/framemarking")
        , m_framemarkingId(2)
        , m_codecType(0)
        , m_payloadType(0)
    {
    }

protected:
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {
        m_SendSession->SetMediaTransport(nullptr);
        m_RecvSession->SetMediaTransport(nullptr);
        m_sendingSinkForSender->Reset();

        if (m_RecvChannel) {
            m_RecvChannel->Close();
            m_RecvChannel->DecreaseReference();
            m_RecvChannel = nullptr;
        }

        if (m_RecvSession) {
            m_RecvSession->DecreaseReference();
            m_RecvSession = nullptr;
        }

        if (m_SendChannel) {
            m_SendChannel->Close();
            m_SendChannel->DecreaseReference();
            m_SendChannel = nullptr;
        }

        if (m_SendSession) {
            m_SendSession->DecreaseReference();
            m_SendSession = nullptr;
        }

        m_sendingSinkForSender.reset();
        m_receivingSinkForReceiver.reset();
    }
    void InitSender(WRTPSessionType sessionType)
    {
        // create video sending session
        WRTPSessionParams sessionParams;
        sessionParams.sessionType = sessionType;
        sessionParams.enableRTCP  = false;

        m_SendSession = WRTPCreateRTPSessionClient(sessionParams);
        ASSERT_TRUE(nullptr != m_SendSession);
        m_SendSession->IncreaseReference();

        int32_t ret = 0;
        if (sessionType == RTP_SESSION_WEBEX_VIDEO || sessionType == RTP_SESSION_WEBEX_AS) {
            ret = m_SendSession->UpdateRTPExtension(m_framemarkingURI, m_framemarkingId, wrtp::STREAM_INOUT);
            EXPECT_TRUE(0 == ret);
        }

        ret = m_SendSession->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
        ASSERT_TRUE(0 == ret);

        ret = m_SendSession->RegisterPayloadType(101, 21, 90000);
        ASSERT_TRUE(0 == ret);
        ret = m_SendSession->RegisterPayloadType(102, 22, 90000);
        ASSERT_TRUE(0 == ret);
        ret = m_SendSession->RegisterPayloadType(103, 23, 90000);
        ASSERT_TRUE(0 == ret);

        // create video sending channel
        WRTPChannelParams channelParams;

        m_SendChannel = m_SendSession->CreateRTPChannel(m_mediaSourceId, channelParams);
        ASSERT_TRUE(nullptr != m_SendChannel);
        m_SendChannel->IncreaseReference();
    }

    void InitRecver(WRTPSessionType sessionType)
    {
        // create video recving session
        WRTPSessionParams sessionParams;
        sessionParams.sessionType = sessionType;
        sessionParams.enableRTCP  = false;

        m_RecvSession = WRTPCreateRTPSessionClient(sessionParams);
        ASSERT_TRUE(nullptr != m_RecvSession);
        m_RecvSession->IncreaseReference();

        int32_t ret = 0;
        if (sessionType == RTP_SESSION_WEBEX_VIDEO || sessionType == RTP_SESSION_WEBEX_AS) {
            ret = m_RecvSession->UpdateRTPExtension(m_framemarkingURI, m_framemarkingId, wrtp::STREAM_INOUT);
            EXPECT_TRUE(0 == ret);
        }

        ret = m_RecvSession->RegisterPayloadType(101, 21, 90000);
        ASSERT_TRUE(0 == ret);
        ret = m_RecvSession->RegisterPayloadType(102, 22, 90000);
        ASSERT_TRUE(0 == ret);
        ret = m_RecvSession->RegisterPayloadType(103, 23, 90000);
        ASSERT_TRUE(0 == ret);

        // create video recving channel
        WRTPChannelParams channelParams;

        m_RecvChannel = m_RecvSession->CreateRTPChannel(m_mediaSourceId, channelParams);
        ASSERT_TRUE(nullptr != m_RecvChannel);
        m_RecvChannel->IncreaseReference();

        m_receivingSinkForReceiver.reset(new CWmeMediaDataRecvSinkMock(m_RecvSession, m_RecvChannel));
        m_RecvChannel->SetMediaDataRecvSink(m_receivingSinkForReceiver.get());
    }

    void BindSender()
    {
        int32_t ret = 0;
        // sending sink for sending session
        m_sendingSinkForSender.reset(new CWmeCodecPayloadTypeMediaTransportMock(m_RecvSession, m_RecvChannel));
        ASSERT_TRUE(nullptr != m_sendingSinkForSender.get());
        ret = m_SendSession->SetMediaTransport(m_sendingSinkForSender.get());
        EXPECT_TRUE(0 == ret);
    }

    void SetCodecTypePayloadType(uint32_t codecType, uint8_t payloadType)
    {
        m_codecType = codecType;
        m_payloadType = payloadType;
        m_receivingSinkForReceiver->SetCodecType(codecType);
        m_sendingSinkForSender->SetPayloadType(payloadType);
    }

    void FillVoIPData(WRTPMediaDataVoIP *mediaDataAudio)
    {
        ASSERT_TRUE(nullptr != mediaDataAudio);
        uint8_t buf[1024];
        mediaDataAudio->type = MEDIA_DATA_VOIP;
        mediaDataAudio->data = buf;
        mediaDataAudio->size = sizeof(buf);
        mediaDataAudio->timestamp = 123;
        mediaDataAudio->sampleTimestamp = 456;
        mediaDataAudio->marker = 0;
        mediaDataAudio->codecType = m_codecType;
    }

    void FillVideoData(WRTPMediaDataVideo *mediaDataVideo)
    {
        //m_mediaDataVideo = mediaDataVideo;
        ASSERT_TRUE(nullptr != mediaDataVideo);
        // init video data
        const int32_t videoDataLen = 1024;
        uint8_t videoData[videoDataLen] = {};

        char ch = 'a';
        for (int ii = 0; ii < videoDataLen; ++ii, ++ch) {
            if (ch > 'z') {
                ch = 'a';
            }
            videoData[ii] = ch;
        }

        // init media data structure
        mediaDataVideo->timestamp        = 123;
        mediaDataVideo->sampleTimestamp  = 456;
        mediaDataVideo->marker           = 1;
        mediaDataVideo->size             = videoDataLen;
        mediaDataVideo->data             = videoData;
        //        mediaDataVideo->layerNumber      = 1;
        //        mediaDataVideo->layerIndex       = 0;
        mediaDataVideo->layerWidth       = 640;
        mediaDataVideo->layerHeight      = 360;
        mediaDataVideo->layerMapAll      = false;
        mediaDataVideo->frame_idc        = 0;
        mediaDataVideo->disposable       = false;
        mediaDataVideo->switchable       = true;
        mediaDataVideo->priority         = 1;
        mediaDataVideo->frameType        = 0;
        mediaDataVideo->DID              = m_streamId;
        mediaDataVideo->TID              = 0;
        mediaDataVideo->maxTID           = 1;
        mediaDataVideo->codecType        = m_codecType;
    }

    void SendVideoData(WRTPMediaDataVideo *mediaDataVideo)
    {
        ASSERT_TRUE(nullptr != m_SendChannel);
        ASSERT_TRUE(nullptr != mediaDataVideo);
        m_SendChannel->SendMediaData(mediaDataVideo);
    }

    void SendVoIPData(WRTPMediaDataVoIP *mediaDataAudio)
    {
        ASSERT_TRUE(nullptr != m_SendChannel);
        ASSERT_TRUE(nullptr != mediaDataAudio);
        m_SendChannel->SendMediaData(mediaDataAudio);
    }

protected:
    IRTPSessionClient        *m_SendSession;
    IRTPChannel              *m_SendChannel;

    IRTPSessionClient        *m_RecvSession;
    IRTPChannel              *m_RecvChannel;

    uint32_t                    m_streamId;
    uint32_t                    m_mediaSourceId;
    const char               *m_framemarkingURI;
    uint8_t                     m_framemarkingId;

    std::unique_ptr<CWmeCodecPayloadTypeMediaTransportMock> m_sendingSinkForSender;
    std::unique_ptr<CWmeMediaDataRecvSinkMock> m_receivingSinkForReceiver;
    uint8_t m_payloadType;
    uint32_t m_codecType;
private:
};

TEST_F(CWmeCodecPayloadTypeAPITest, Test_send_one_voip_data_with_codecType_then_recv_right_codecType)
{
    CScopedTracer test_info;
    InitSender(RTP_SESSION_WEBEX_VOIP);
    InitRecver(RTP_SESSION_WEBEX_VOIP);
    BindSender();
    SetCodecTypePayloadType(101, 21);

    WRTPMediaDataVoIP firstVoIPData;
    FillVoIPData(&firstVoIPData);
    SendVoIPData(&firstVoIPData);

    //::SleepMs(100);
}

TEST_F(CWmeCodecPayloadTypeAPITest, Test_send_multi_voip_data_with_codecType_then_recv_right_codecType)
{
    CScopedTracer test_info;
    InitSender(RTP_SESSION_WEBEX_VOIP);
    InitRecver(RTP_SESSION_WEBEX_VOIP);
    BindSender();

    SetCodecTypePayloadType(101, 21);
    WRTPMediaDataVoIP firstVoIPData;
    FillVoIPData(&firstVoIPData);
    SendVoIPData(&firstVoIPData);

    SetCodecTypePayloadType(102, 22);
    WRTPMediaDataVoIP secondVoIPData;
    FillVoIPData(&secondVoIPData);
    SendVoIPData(&secondVoIPData);

    SetCodecTypePayloadType(103, 23);
    WRTPMediaDataVoIP thirdVoIPData;
    FillVoIPData(&thirdVoIPData);
    SendVoIPData(&thirdVoIPData);

    //::SleepMs(100);
}

TEST_F(CWmeCodecPayloadTypeAPITest, Test_send_one_video_data_with_codecType_then_recv_right_codecType)
{
    CScopedTracer test_info;
    InitSender(RTP_SESSION_WEBEX_VIDEO);
    InitRecver(RTP_SESSION_WEBEX_VIDEO);
    BindSender();

    SetCodecTypePayloadType(101, 21);
    WRTPMediaDataVideo firstVideoData;
    FillVideoData(&firstVideoData);
    SendVideoData(&firstVideoData);

    //::SleepMs(100);
}

TEST_F(CWmeCodecPayloadTypeAPITest, Test_send_multi_video_data_with_codecType_then_recv_right_codecType)
{
    CScopedTracer test_info;
    InitSender(RTP_SESSION_WEBEX_VIDEO);
    InitRecver(RTP_SESSION_WEBEX_VIDEO);
    BindSender();

    SetCodecTypePayloadType(101, 21);
    WRTPMediaDataVideo firstVideoData;
    FillVideoData(&firstVideoData);
    SendVideoData(&firstVideoData);

    //::SleepMs(100);

    SetCodecTypePayloadType(102, 22);
    WRTPMediaDataVideo SecondVideoData;
    FillVideoData(&SecondVideoData);
    SendVideoData(&SecondVideoData);

    //::SleepMs(100);

    SetCodecTypePayloadType(103, 23);
    WRTPMediaDataVideo thirdVideoData;
    FillVideoData(&thirdVideoData);
    FillVideoData(&thirdVideoData);

    //::SleepMs(100);
}

TEST_F(CWmeCodecPayloadTypeAPITest, Test_RegisterPayloadType)
{
    CScopedTracer test_info;
    InitSender(RTP_SESSION_WEBEX_VIDEO);
    InitRecver(RTP_SESSION_WEBEX_VIDEO);
    BindSender();

    CRTPSessionClient *sessionClient = (CRTPSessionClient *)(m_SendSession);
    ASSERT_TRUE(nullptr != sessionClient);

    int32_t ret = m_SendSession->RegisterPayloadType(50, 10, 48000);
    ASSERT_TRUE(0 == ret);

    uint32_t clockRate = 0;
    EXPECT_TRUE(0 != sessionClient->GetSessionContext()->GetOutboundConfig()->QueryClockRate(60, clockRate));

    EXPECT_TRUE(0 == sessionClient->GetSessionContext()->GetOutboundConfig()->QueryClockRate(50, clockRate));
    EXPECT_EQ(48000, clockRate);
}