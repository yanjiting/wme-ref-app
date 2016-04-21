#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "CmUtilMisc.h"
#include "wrtpapi.h"
#include "wrtpwmeapi.h"
#include "WMEInterfaceMock.h"
#include "testutil.h"

using namespace wrtp;
using namespace wme;

class CRTPSendingSinkMockForPSFBTest
    : public IRTPMediaTransport
{
public:
    CRTPSendingSinkMockForPSFBTest(IRTPSessionClient *recvSession, IRTPChannel *recvChannel)
        : m_recvSession(recvSession)
        , m_recvChannel(recvChannel)
        , m_rtpSendCount(0)
        , m_rtcpSendCount(0)
    {

    }

    virtual WMERESULT GetTransportType(WmeTransportType &eTransportType)
    {
        eTransportType = WmeTransportType_UDP;
        return WME_S_OK;
    }

    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage)
    {
        ++m_rtpSendCount;

        EXPECT_TRUE(nullptr != m_recvChannel);

        // forward the RTP packet to recving channel
        return m_recvChannel->ReceiveRTPPacket(pRTPPackage);
    }

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        if (nullptr == m_recvSession) {
            return 0;
        }
        ++m_rtcpSendCount;

        //EXPECT_TRUE(nullptr != m_recvSession);

        // forward the RTCP packet to recving session
        return m_recvSession->ReceiveRTCPPacket(pRTCPPackage);
    }

    uint32_t GetRTPSendCount()
    {
        return m_rtpSendCount;
    }
    uint32_t GetRTCPSendCount()
    {
        return m_rtcpSendCount;
    }

    void SetRTPSession(IRTPSessionClient *session)
    {
        m_recvSession = session;
    }

private:
    IRTPSessionClient  *m_recvSession;
    IRTPChannel        *m_recvChannel;

    uint32_t m_rtpSendCount;
    uint32_t m_rtcpSendCount;
};

// m_videoSendSession                       m_videoRecvSession
//                          RTP
// m_videoSendChannle  -------------->      m_videoRecvChannel
//                     <--------------
//                          RTCP(PLI)
//
// Topo:
// m_videoSendSession -> m_sendingSinkForSender -> m_videoRecvSession
//          ^                                                |
//          |___________ m_sendingSinkForRecver <------------|
class CPLIP2PRequestResponseTest
    : public ::testing::Test
    , public IRTPPayloadSpecificIndicationSink
{
public:
    CPLIP2PRequestResponseTest()
        : m_videoSendSession(nullptr)
        , m_videoSendChannel(nullptr)
        , m_videoRecvSession(nullptr)
        , m_videoRecvChannel(nullptr)
        , m_mediaSourceId(123)
        , m_streamId(1)
        , m_framemarkingURI("http://protocols.cisco.com/framemarking")
        , m_framemarkingId(2)
        , m_isPLIRequested(false)
    {
    }

    // functions from IRTPPayloadSpecificIndicationSink
    virtual int32_t OnPictureLossIndication(uint32_t mediaSourceId, uint32_t streamId, bool instant)
    {
        EXPECT_EQ(mediaSourceId, m_mediaSourceId);
        EXPECT_EQ(streamId, m_streamId);
        m_isPLIRequested = true;

        return 0;
    }

protected:
    virtual void SetUp()
    {

        InitSender();
        InitRecver();
        BindSenderRecver();
    }

    virtual void TearDown()
    {
        //{ fix crash issue
        m_sendingSinkForSender->SetRTPSession(nullptr);
        m_sendingSinkForRecver->SetRTPSession(nullptr);
        //}
        m_videoRecvChannel->Close();
        m_videoRecvChannel->DecreaseReference();
        m_videoRecvChannel = nullptr;

        m_videoRecvSession->DecreaseReference();
        m_videoRecvSession = nullptr;

        m_videoSendChannel->Close();
        m_videoSendChannel->DecreaseReference();
        m_videoSendChannel = nullptr;

        m_videoSendSession->DecreaseReference();
        m_videoSendSession = nullptr;

        m_sendingSinkForSender.reset();
        m_sendingSinkForRecver.reset();
    }

    void InitSender()
    {
        // create video sending session
        WRTPSessionParams sessionParams;
        sessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
        sessionParams.enableRTCP  = true;

        m_videoSendSession = WRTPCreateRTPSessionClient(sessionParams);
        ASSERT_TRUE(nullptr != m_videoSendSession);
        m_videoSendSession->IncreaseReference();

        int32_t ret = m_videoSendSession->UpdateRTPExtension(m_framemarkingURI, m_framemarkingId, wrtp::STREAM_INOUT);
        EXPECT_TRUE(0 == ret);
        m_videoSendSession->SetPayloadSpecificIndicationSink(this);

        ret = m_videoSendSession->RegisterPayloadType(60, 20, 90000);
        ASSERT_TRUE(0 == ret);

        ret = m_videoSendSession->SetMaxPacketSize(1200);
        ASSERT_TRUE(0 == ret);

        // create video sending channel
        WRTPChannelParams channelParams;

        m_videoSendChannel = m_videoSendSession->CreateRTPChannel(m_mediaSourceId, channelParams);
        ASSERT_TRUE(nullptr != m_videoSendChannel);
        m_videoSendChannel->IncreaseReference();
    }

    void InitRecver()
    {
        // create video recving session
        WRTPSessionParams sessionParams;
        sessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
        sessionParams.enableRTCP  = true;

        m_videoRecvSession = WRTPCreateRTPSessionClient(sessionParams);
        ASSERT_TRUE(nullptr != m_videoRecvSession);
        m_videoRecvSession->IncreaseReference();

        int32_t ret = m_videoRecvSession->UpdateRTPExtension(m_framemarkingURI, m_framemarkingId, wrtp::STREAM_INOUT);
        EXPECT_TRUE(0 == ret);

        ret = m_videoRecvSession->RegisterPayloadType(60, 20, 90000);
        ASSERT_TRUE(0 == ret);

        ret = m_videoRecvSession->SetMaxPacketSize(1200);
        ASSERT_TRUE(0 == ret);

        // create video recving channel
        WRTPChannelParams channelParams;

        m_videoRecvChannel = m_videoRecvSession->CreateRTPChannel(m_mediaSourceId, channelParams);
        ASSERT_TRUE(nullptr != m_videoRecvChannel);
        m_videoRecvChannel->IncreaseReference();
    }

    void BindSenderRecver()
    {
        int32_t ret = 0;
        // sending sink for sending session
        m_sendingSinkForSender.reset(new CRTPSendingSinkMockForPSFBTest(m_videoRecvSession, m_videoRecvChannel));
        ASSERT_TRUE(nullptr != m_sendingSinkForSender.get());
        ret = m_videoSendSession->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
        ASSERT_TRUE(0 == ret);
        ret = m_videoSendSession->SetMediaTransport(m_sendingSinkForSender.get());
        ASSERT_TRUE(0 == ret);

        // sending sink for recving session
        m_sendingSinkForRecver.reset(new CRTPSendingSinkMockForPSFBTest(m_videoSendSession, m_videoSendChannel));
        ASSERT_TRUE(nullptr != m_sendingSinkForRecver.get());
        ret = m_videoRecvSession->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
        ASSERT_TRUE(0 == ret);
        ret = m_videoRecvSession->SetMediaTransport(m_sendingSinkForRecver.get());
        ASSERT_TRUE(0 == ret);
    }

    void SendVideoData()
    {
        // init video data
        const int32_t videoDataLen = 1024;
        uint8_t videoData[videoDataLen] = {};

        char ch = 'a';
        for (int ii = 0;
                ii < videoDataLen;
                ++ii, ++ch) {
            if (ch > 'z') {
                ch = 'a';
            }
            videoData[ii] = ch;
        }

        // init media data structure
        WRTPMediaDataVideo mediaDataVideo;
        mediaDataVideo.timestamp        = 123;
        mediaDataVideo.sampleTimestamp  = 456;
        mediaDataVideo.marker           = 1;
        mediaDataVideo.size             = videoDataLen;
        mediaDataVideo.data             = videoData;
        //        mediaDataVideo.layerNumber      = 1;
        //        mediaDataVideo.layerIndex       = 0;
        mediaDataVideo.layerWidth       = 640;
        mediaDataVideo.layerHeight      = 360;
        mediaDataVideo.layerMapAll      = false;
        mediaDataVideo.frame_idc        = 0;
        mediaDataVideo.disposable       = false;
        mediaDataVideo.switchable       = true;
        mediaDataVideo.priority         = 1;
        mediaDataVideo.frameType        = 0;
        mediaDataVideo.DID              = m_streamId;
        mediaDataVideo.TID              = 0;
        mediaDataVideo.maxTID           = 1;
        mediaDataVideo.maxDID           = m_streamId;
        mediaDataVideo.codecType        = 60;

        ASSERT_TRUE(nullptr != m_videoSendChannel);
        m_videoSendChannel->SendMediaData(&mediaDataVideo);
    }

protected:
    IRTPSessionClient  *m_videoSendSession;
    IRTPChannel        *m_videoSendChannel;

    IRTPSessionClient *m_videoRecvSession;
    IRTPChannel        *m_videoRecvChannel;

    uint32_t m_mediaSourceId;
    uint32_t m_streamId;
    const char *m_framemarkingURI;
    uint8_t  m_framemarkingId;

    bool   m_isPLIRequested;

    std::unique_ptr<CRTPSendingSinkMockForPSFBTest> m_sendingSinkForSender;
    std::unique_ptr<CRTPSendingSinkMockForPSFBTest> m_sendingSinkForRecver;
};

TEST_F(CPLIP2PRequestResponseTest, Test_PLI_Request_when_no_rtp_packet_sent_then_return_error)
{
    CScopedTracer test_info;
    int32_t ret = m_videoRecvSession->IndicatePictureLoss(m_mediaSourceId, m_streamId);
    EXPECT_TRUE(0 != ret);

    EXPECT_FALSE(m_isPLIRequested);

    EXPECT_TRUE(m_sendingSinkForSender->GetRTPSendCount()   == 0);
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount()  == 0);
    EXPECT_TRUE(m_sendingSinkForRecver->GetRTPSendCount()   == 0);
    EXPECT_TRUE(m_sendingSinkForRecver->GetRTCPSendCount()  == 0);
}

TEST_F(CPLIP2PRequestResponseTest, Test_PLI_Request_after_rtp_packet_sent_then_return_success)
{
    CScopedTracer test_info;
    SendVideoData();

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);
    //::SleepMs(100);

    int32_t ret = m_videoRecvSession->IndicatePictureLoss(m_mediaSourceId, m_streamId);
    EXPECT_TRUE(0 == ret);

    EXPECT_TRUE(m_isPLIRequested);

    EXPECT_TRUE(m_sendingSinkForSender->GetRTPSendCount()   >  0);
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount()  >  0);
    EXPECT_TRUE(m_sendingSinkForRecver->GetRTPSendCount()   == 0);
    EXPECT_TRUE(m_sendingSinkForRecver->GetRTCPSendCount()  >  0);
}

TEST_F(CPLIP2PRequestResponseTest, Test_PLI_Request_after_rtp_packet_sent_but_no_set_PLI_sink_then_return_error)
{
    CScopedTracer test_info;
    m_videoSendSession->SetPayloadSpecificIndicationSink(nullptr);
    SendVideoData();

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);
    //::SleepMs(100);

    int32_t ret = m_videoRecvSession->IndicatePictureLoss(m_mediaSourceId, m_streamId);
    EXPECT_TRUE(0 == ret);

    EXPECT_FALSE(m_isPLIRequested);

    EXPECT_TRUE(m_sendingSinkForSender->GetRTPSendCount()   >  0);
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount()  >  0);
    EXPECT_TRUE(m_sendingSinkForRecver->GetRTPSendCount()   == 0);
    EXPECT_TRUE(m_sendingSinkForRecver->GetRTCPSendCount()  >  0);
}

TEST_F(CPLIP2PRequestResponseTest, Test_PLI_Request_after_rtp_packet_sent_but_request_wrong_source_then_return_error)
{
    CScopedTracer test_info;
    SendVideoData();

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);
    //::SleepMs(100);

    int32_t ret = m_videoRecvSession->IndicatePictureLoss(m_mediaSourceId + 1, m_streamId);
    EXPECT_TRUE(0 != ret);

    EXPECT_FALSE(m_isPLIRequested);

    EXPECT_TRUE(m_sendingSinkForSender->GetRTPSendCount()   >  0);
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount()  >  0);
    EXPECT_TRUE(m_sendingSinkForRecver->GetRTPSendCount()   == 0);
}

TEST_F(CPLIP2PRequestResponseTest, Test_PLI_Request_after_rtp_packet_sent_but_request_unknown_stream_then_return_success)
{
    CScopedTracer test_info;
    SendVideoData();

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);
    //::SleepMs(100);

    int32_t ret = m_videoRecvSession->IndicatePictureLoss(m_mediaSourceId, m_streamId + 1);
    EXPECT_TRUE(0 == ret);

    EXPECT_TRUE(m_isPLIRequested);

    EXPECT_TRUE(m_sendingSinkForSender->GetRTPSendCount()   >  0);
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount()  >  0);
    EXPECT_TRUE(m_sendingSinkForRecver->GetRTPSendCount()   == 0);
    EXPECT_TRUE(m_sendingSinkForRecver->GetRTCPSendCount()  >  0);
}

