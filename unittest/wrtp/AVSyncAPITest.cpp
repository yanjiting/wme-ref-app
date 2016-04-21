#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "CmThreadManager.h"
#include "wrtpapi.h"
#include "rtputils.h"
#include "wrtpwmeapi.h"
#include "WMEInterfaceMock.h"
#include "rtpsessionconfig.h"
#include "testutil.h"
#include "CmByteStream.h"

using namespace wrtp;
using namespace wme;


#if 1
//annotated by Joyce
class CRTPSendingVideoSinkMock:public IRTPMediaTransport
{
public:
    CRTPSendingVideoSinkMock(IRTPSessionClient *recvSession, IRTPChannel *recvChannel, bool dropRTCP = false)
        : m_recvSession(recvSession)
        , m_recvChannel(recvChannel)
        , m_dropRTCP(dropRTCP)
        , m_rtpSendCount(0)
        , m_rtcpSendCount(0)
        , m_Mutex()
        , m_ssrc(0)
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
        if (!m_recvChannel) {
            return WRTP_ERR_NOERR;
        }
        ++ m_rtpSendCount;

        // forward the RTP packet to recving channel
        CWMEMediaPackage *pPacket = (CWMEMediaPackage *)pRTPPackage;
        int32_t *pData = (int32_t *)pRTPPackage;
        unsigned char *pBuff;
        pPacket->GetDataPointer(&pBuff);
        if (pBuff != nullptr) {
            m_ssrc = *(((int32_t *)pBuff) + 2);
            CCmHostNetworkConvertorNormal::Swap(m_ssrc);
        }
        uint32_t ts(0);
        pPacket->GetTimestamp(ts);

        return m_recvChannel->ReceiveRTPPacket(pRTPPackage);
    }

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        // Just discard the RTCP packets
        if (m_dropRTCP) {
            return WRTP_ERR_NOERR;
        }

        if (!m_recvSession) {

            return WRTP_ERR_NOERR;
        }

        // forward the RTCP packet to recving session

        int32_t ret = m_recvSession->ReceiveRTCPPacket(pRTCPPackage);
        EXPECT_EQ(WRTP_ERR_NOERR, ret);

        RecursiveMutexGuard g(m_Mutex);
        ++ m_rtcpSendCount;

        return WRTP_ERR_NOERR;
    }

    uint32_t GetRTPSendCount() const  { return m_rtpSendCount; }
    uint32_t GetRTCPSendCount()
    {
        RecursiveMutexGuard g(m_Mutex);
        return m_rtcpSendCount;
    }
    uint32_t GetSSRC()
    {
        return m_ssrc;
    }

    void SetDropRTCPFlag(bool toDrop)
    {
        m_dropRTCP = toDrop;
    }

protected:
private:
    IRTPSessionClient  *m_recvSession;
    IRTPChannel        *m_recvChannel;
    bool                m_dropRTCP;

    uint32_t              m_rtpSendCount;
    uint32_t              m_rtcpSendCount;
    uint32_t              m_ssrc;
    RecursiveMutex  m_Mutex;
};

class CMediaDataRecvSinkMock:public IMediaDataRecvSink
{
public:
    CMediaDataRecvSinkMock(IRTPSessionClient *recvSession, IRTPChannel *recvChannel, uint32_t timestamp, uint32_t sampleTimestamp)
        : m_recvSession(recvSession)
        , m_recvChannel(recvChannel)
        , m_timestamp(timestamp)
        , m_sampleTimestamp(sampleTimestamp)
        , m_firstRTPPacketReceived(false)
    {
    }

    virtual int32_t OnRecvMediaData(WRTPMediaData *pData, WRTPMediaDataIntegrityInfo *pIntegrityInfo)
    {
        if (m_firstRTPPacketReceived) {
            EXPECT_TRUE(nullptr != pData);
            EXPECT_TRUE(nullptr != pIntegrityInfo);

            EXPECT_EQ(0 != pData->timestamp, pData->timestampRecovered);
            if (0 != m_timestamp) {
                if (!((m_timestamp == pData->timestamp)||(m_timestamp-1 == pData->timestamp)||(m_timestamp+1 == pData->timestamp))) {
                    EXPECT_EQ(m_timestamp, pData->timestamp);
                }
            }
            if (0 != m_sampleTimestamp) {
                EXPECT_EQ(m_sampleTimestamp, pData->sampleTimestamp);
            }
        }

        m_firstRTPPacketReceived = true;

        return 0;
    }

protected:

private:
    uint32_t              m_timestamp;
    uint32_t              m_sampleTimestamp;

    IRTPSessionClient  *m_recvSession;
    IRTPChannel        *m_recvChannel;
    bool                m_firstRTPPacketReceived;
};

class CAVSyncTest
    :public ::testing::Test
{
public:
    CAVSyncTest(): m_SendSession(nullptr)
        , m_SendChannel(nullptr)
        , m_RecvSession(nullptr)
        , m_RecvChannel(nullptr)
        , m_mediaSourceId(123)
        , m_streamId(1)
        , m_framemarkingURI("http://protocols.cisco.com/framemarking")
        , m_framemarkingId(2)
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

        m_RecvChannel->Close();
        m_RecvChannel->DecreaseReference();
        m_RecvChannel = nullptr;

        m_RecvSession->DecreaseReference();
        m_RecvSession = nullptr;

        m_SendChannel->Close();
        m_SendChannel->DecreaseReference();
        m_SendChannel = nullptr;

        m_SendSession->DecreaseReference();
        m_SendSession = nullptr;

        m_sendingSinkForSender.reset();
        m_DataRecvSink.reset();

    }
    void InitSender(WRTPSessionType sessionType)
    {
        // create video sending session
        WRTPSessionParams sessionParams;
        sessionParams.sessionType = sessionType;
        sessionParams.enableRTCP  = true;

        m_SendSession = WRTPCreateRTPSessionClient(sessionParams);
        ASSERT_TRUE(nullptr != m_SendSession);
        m_SendSession->IncreaseReference();

        int32_t ret = 0;
        if (sessionType == RTP_SESSION_WEBEX_VIDEO || sessionType == RTP_SESSION_WEBEX_AS) {
            ret = m_SendSession->UpdateRTPExtension(m_framemarkingURI, m_framemarkingId, wrtp::STREAM_INOUT);
            EXPECT_TRUE(0 == ret);

            ret = m_SendSession->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
            ASSERT_TRUE(0 == ret);
        }

        if (sessionType == RTP_SESSION_WEBEX_VOIP) {
            ret = m_SendSession->RegisterPayloadType(CODEC_TYPE_AUDIO_TEST, PAYLOAD_TYPE_AUDIO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);
            ASSERT_TRUE(0 == ret);
        }

        // create video sending channel
        WRTPChannelParams channelParams;

        m_SendChannel = m_SendSession->CreateRTPChannel(m_mediaSourceId, channelParams);
        ASSERT_TRUE(nullptr != m_SendChannel);
        m_SendChannel->IncreaseReference();
    }

    void InitRecver(WRTPSessionType sessionType, uint32_t secondTimestamp, uint32_t sencondSampleTimestamp)
    {
        // create video recving session
        WRTPSessionParams sessionParams;
        sessionParams.sessionType = sessionType;
        sessionParams.enableRTCP  = true;

        m_RecvSession = WRTPCreateRTPSessionClient(sessionParams);
        ASSERT_TRUE(nullptr != m_RecvSession);
        m_RecvSession->IncreaseReference();

        int32_t ret = 0;

        if (sessionType == RTP_SESSION_WEBEX_VIDEO || sessionType == RTP_SESSION_WEBEX_AS) {
            ret = m_RecvSession->UpdateRTPExtension(m_framemarkingURI, m_framemarkingId, wrtp::STREAM_INOUT);
            EXPECT_TRUE(0 == ret);

            ret = m_RecvSession->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
            ASSERT_TRUE(0 == ret);
        }

        if (sessionType == RTP_SESSION_WEBEX_VOIP) {
            ret = m_RecvSession->RegisterPayloadType(CODEC_TYPE_AUDIO_TEST, PAYLOAD_TYPE_AUDIO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);
            ASSERT_TRUE(0 == ret);
        }

        // create video recving channel
        WRTPChannelParams channelParams;

        m_RecvChannel = m_RecvSession->CreateRTPChannel(m_mediaSourceId, channelParams);
        ASSERT_TRUE(nullptr != m_RecvChannel);
        m_RecvChannel->IncreaseReference();

        m_DataRecvSink.reset(new CMediaDataRecvSinkMock(m_RecvSession, m_RecvChannel, secondTimestamp, sencondSampleTimestamp));
        m_RecvChannel->SetMediaDataRecvSink(m_DataRecvSink.get());
    }

    void BindSender()
    {
        int32_t ret = 0;
        // sending sink for sending session
        m_sendingSinkForSender.reset(new CRTPSendingVideoSinkMock(m_RecvSession, m_RecvChannel));
        ASSERT_TRUE(nullptr != m_sendingSinkForSender);
        ret = m_SendSession->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
        EXPECT_TRUE(0 == ret);
        ret = m_SendSession->SetMediaTransport(m_sendingSinkForSender.get());
        EXPECT_TRUE(0 == ret);
    }

    void TriggerRTCPSend(IWmeMediaPackage *pRTPPackage)
    {
        ASSERT_TRUE(nullptr != m_SendChannel);
        ASSERT_TRUE(nullptr != m_sendingSinkForSender.get());
        ASSERT_TRUE(nullptr != pRTPPackage);
        m_sendingSinkForSender->SendRTCPPacket(pRTPPackage);
    }

    void SendData(WRTPMediaData *mediaData)
    {
        ASSERT_TRUE(nullptr != m_SendChannel);
        ASSERT_TRUE(nullptr != mediaData);
        //send first RTP packet
        m_SendChannel->SendMediaData(mediaData);
    }

protected:
    IRTPSessionClient        *m_SendSession;
    IRTPChannel              *m_SendChannel;

    IRTPSessionClient        *m_RecvSession;
    IRTPChannel              *m_RecvChannel;

    uint32_t                    m_mediaSourceId;
    const char               *m_framemarkingURI;
    uint8_t                     m_framemarkingId;

    std::unique_ptr<CRTPSendingVideoSinkMock> m_sendingSinkForSender;
    std::unique_ptr<CMediaDataRecvSinkMock> m_DataRecvSink;

public:
    uint32_t                    m_streamId;
};

TEST_F(CAVSyncTest, Test_cname)
{
    CScopedTracer test_info;
    string cname = "jimmqu@cisco.com";
    ///////Create a video stream and get SSRC///////////
    InitSender(RTP_SESSION_WEBEX_VIDEO);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, 0, 0);
    m_SendSession->SetLocalCName(cname.c_str());

    wrtp::CNameBlock nb1;
    m_SendSession->GetLocalCName(nb1);
    EXPECT_TRUE(cname == nb1.cname);
    BindSender();

#ifdef ANDROID
    CNTPTime ntp = CNTPTime::Now();
    uint32_t timestamp = ntp.ToClockTime().ToMilliseconds();
#else
    uint32_t timestamp = TickNowMS();
#endif

    WRTPMediaData *firstVideoData = CreateVideoData(timestamp, 456, 1024, m_streamId);
    WRTP_INFOTRACE("Send first packet:" << timestamp << ",456\n");
    SleepMs(100);
    SendData(firstVideoData);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (m_sendingSinkForSender->GetRTCPSendCount() > 0));
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount() > 0);

    uint32_t ssrc = m_sendingSinkForSender->GetSSRC();

    //////Create a SDES packet///////////////
    CCompoundPacket comp;
    CSDESPacket sdesp;

    sdesp.sdesTrunks.resize(1);
    sdesp.sdesTrunks[0].ssrc = ssrc;
    sdesp.sdesTrunks[0].sdesItems.resize(1);
    sdesp.sdesTrunks[0].sdesItems[0].type = SDES_TYPE_CNAME;
    sdesp.sdesTrunks[0].sdesItems[0].length = cname.length();
    sdesp.sdesTrunks[0].sdesItems[0].content = (uint8_t *)cname.c_str();
    comp.AddRtcpPacket(&sdesp);

    uint32_t encodeSize = comp.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize + MAX_RTCP_PACKET_RESERVED_LEN);
    CCmByteStreamNetwork os(mb);
    BOOL ret = comp.Encode(os, mb);
    comp.packets.clear();
    sdesp.sdesTrunks.clear();
    IRTPMediaPackage *mediaPackage = new CWMEMediaPackage(RTPMediaFormatRTCP, 0, mb.GetTopLevelLength());
    mediaPackage->AddRef();
    mediaPackage->SetTimestamp(6789);
    unsigned char *buffer = nullptr;
    ret = mediaPackage->GetDataPointer(&buffer);
    memcpy(buffer, mb.GetTopLevelReadPtr(), mb.GetTopLevelLength());
    mediaPackage->SetDataLength(mb.GetTopLevelLength());


    TriggerRTCPSend(mediaPackage);

    wrtp::CNameBlock nb2;
    m_RecvSession->QueryRemoteCName(ssrc, nb2);
    EXPECT_TRUE(cname == nb2.cname);
}

TEST_F(CAVSyncTest, Test_send_video_with_timestamp_and_sampleTimestamp_return_right_timestamp1)
{
    CScopedTracer test_info;

#ifdef ANDROID
    CNTPTime ntp = CNTPTime::Now();
    uint32_t timestamp = ntp.ToClockTime().ToMilliseconds();
#else
    uint32_t timestamp = TickNowMS();
#endif

    InitSender(RTP_SESSION_WEBEX_VIDEO);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, timestamp+1, 546);
    BindSender();

    //SleepMs(100);
    WRTP_TEST_TRIGGER_ON_TIMER(20, 5);

    WRTPMediaData *firstVideoData = CreateVideoData(timestamp, 456, 1024, m_streamId);
    WRTP_INFOTRACE("Send first packet:"<<timestamp<<",456\n");
    SendData(firstVideoData);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (m_sendingSinkForSender->GetRTCPSendCount() > 0));
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount() > 0);

    WRTPMediaData *secondVideoData = CreateVideoData(timestamp+1, 546, 1024, m_streamId);
    WRTP_INFOTRACE("Send second packet:"<<timestamp+1<<",546\n");
    SendData(secondVideoData);

    //::SleepMs(100);
    DestroyMediaData(firstVideoData);
    DestroyMediaData(secondVideoData);
}

TEST_F(CAVSyncTest, Test_send_video_with_timestamp_and_sampleTimestamp_roll_back_return_right_timestamp)
{
    CScopedTracer test_info;
#ifdef ANDROID
    CNTPTime ntp = CNTPTime::Now();
    uint32_t timestamp = ntp.ToClockTime().ToMilliseconds();
#else
    uint32_t timestamp = TickNowMS();
#endif
    InitSender(RTP_SESSION_WEBEX_VIDEO);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, timestamp, 456);
    BindSender();

    ::SleepMs(100);
    WRTPMediaData *firstVideoData = CreateVideoData(timestamp+1, 546, 1024, m_streamId);
    WRTP_INFOTRACE("Send first packet:"<<(timestamp+1)<<",546\n");
    SendData(firstVideoData);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (m_sendingSinkForSender->GetRTCPSendCount() > 0));
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount() > 0);

    WRTPMediaData *secondVideoData = CreateVideoData(timestamp, 456, 1024, m_streamId);
    WRTP_INFOTRACE("Send second packet:"<<timestamp<<",456\n");
    SendData(secondVideoData);

    //::SleepMs(100);
    DestroyMediaData(firstVideoData);
    DestroyMediaData(secondVideoData);
}

#if 0 // Disabled by Hank Peng, as the engine must provide the two timestamp now in order to support SVS of T30
TEST_F(CAVSyncTest, DISABLED_Test_send_video_with_timestamp_without_sampleTimestamp_return_right_timestamp)
{
    CScopedTracer test_info;
#ifdef ANDROID
    CNTPTime ntp = CNTPTime::Now();
    uint32_t timestamp = ntp.ToClockTime().ToMilliseconds();
#else
    uint32_t timestamp = TickNowMS();
#endif
    InitSender(RTP_SESSION_WEBEX_VIDEO);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, timestamp+1, 0);
    BindSender();

    ::SleepMs(100);
    WRTPMediaData *firstVideoData = CreateVideoData(timestamp, 0, 1024, m_streamId);
    SendData(firstVideoData);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (m_sendingSinkForSender->GetRTCPSendCount() > 0));
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount() > 0);

    WRTPMediaData *secondVideoData = CreateVideoData(timestamp+1, 0, 1024, m_streamId);
    SendData(secondVideoData);

    //::SleepMs(100);
    DestroyMediaData(firstVideoData);
    DestroyMediaData(secondVideoData);
}

TEST_F(CAVSyncTest, DISABLED_Test_send_video_with_timestamp_without_sampleTimestamp_roll_back_return_right_timestamp)
{
    CScopedTracer test_info;
#ifdef ANDROID
    CNTPTime ntp = CNTPTime::Now();
    uint32_t timestamp = ntp.ToClockTime().ToMilliseconds();
#else
    uint32_t timestamp = TickNowMS();
#endif
    InitSender(RTP_SESSION_WEBEX_VIDEO);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, timestamp, 0);
    BindSender();

    ::SleepMs(100);
    WRTPMediaData *firstVideoData = CreateVideoData(timestamp+1, 0, 1024, m_streamId);
    SendData(firstVideoData);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (m_sendingSinkForSender->GetRTCPSendCount() > 0));
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount() > 0);

    WRTPMediaData *secondVideoData = CreateVideoData(timestamp, 0, 1024, m_streamId);
    SendData(secondVideoData);

    //::SleepMs(100);
    DestroyMediaData(firstVideoData);
    DestroyMediaData(secondVideoData);
}
#endif

TEST_F(CAVSyncTest, Test_send_video_without_timestamp_with_sampleTimestamp_return_right_timestamp)
{
    CScopedTracer test_info;
    InitSender(RTP_SESSION_WEBEX_VIDEO);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, 0, 546);
    BindSender();

    ::SleepMs(100);
    WRTPMediaData *firstVideoData = CreateVideoData(0, 456, 1024, m_streamId);
    SendData(firstVideoData);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (m_sendingSinkForSender->GetRTCPSendCount() > 0));
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount() > 0);

    WRTPMediaData *secondVideoData = CreateVideoData(0, 546, 1024, m_streamId);
    SendData(secondVideoData);

    //::SleepMs(100);
    DestroyMediaData(firstVideoData);
    DestroyMediaData(secondVideoData);
}

TEST_F(CAVSyncTest, Test_send_video_without_timestamp_with_sampleTimestamp_roll_back_return_right_timestamp)
{

    CScopedTracer test_info;
    InitSender(RTP_SESSION_WEBEX_VIDEO);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, 0, 456);
    BindSender();

    ::SleepMs(100);
    WRTPMediaData *firstVideoData = CreateVideoData(0, 546, 1024, m_streamId);
    SendData(firstVideoData);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (m_sendingSinkForSender->GetRTCPSendCount() > 0));
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount() > 0);

    WRTPMediaData *secondVideoData = CreateVideoData(0, 456, 1024, m_streamId);
    SendData(secondVideoData);

    //::SleepMs(100);
    DestroyMediaData(firstVideoData);
    DestroyMediaData(secondVideoData);
}

TEST_F(CAVSyncTest, Test_send_audio_with_timestamp_and_sampleTimestamp_return_right_timestamp)
{
    CScopedTracer test_info;
#ifdef ANDROID
    CNTPTime ntp = CNTPTime::Now();
    uint32_t timestamp = ntp.ToClockTime().ToMilliseconds();
#else
    uint32_t timestamp = TickNowMS();
#endif
    InitSender(RTP_SESSION_WEBEX_VOIP);
    InitRecver(RTP_SESSION_WEBEX_VOIP, timestamp+1, 504);
    BindSender();

    ::SleepMs(100);
    WRTPMediaData *firstAudioData = CreateVoIPData(timestamp, 456, 1024);
    SendData(firstAudioData);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (m_sendingSinkForSender->GetRTCPSendCount() > 0));
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount() > 0);

    WRTPMediaData *secondAudioData = CreateVoIPData(timestamp+1, 504, 1024);
    SendData(secondAudioData);

    //::SleepMs(100);
    DestroyMediaData(firstAudioData);
    DestroyMediaData(secondAudioData);
}

TEST_F(CAVSyncTest, Test_send_audio_with_timestamp_and_sampleTimestamp_roll_back_return_right_timestamp)
{
    CScopedTracer test_info;
#ifdef ANDROID
    CNTPTime ntp = CNTPTime::Now();
    uint32_t timestamp = ntp.ToClockTime().ToMilliseconds();
#else
    uint32_t timestamp = TickNowMS();
#endif
    InitSender(RTP_SESSION_WEBEX_VOIP);
    InitRecver(RTP_SESSION_WEBEX_VOIP, timestamp, 456);
    BindSender();

    ::SleepMs(100);
    WRTPMediaData *firstAudioData = CreateVoIPData(timestamp+1, 504, 1024);
    SendData(firstAudioData);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (m_sendingSinkForSender->GetRTCPSendCount() > 0));
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount() > 0);

    WRTPMediaData *secondAudioData = CreateVoIPData(timestamp, 456, 1024);
    SendData(secondAudioData);

    //::SleepMs(100);
    DestroyMediaData(firstAudioData);
    DestroyMediaData(secondAudioData);
}

#if 0 // Disabled by Hank Peng, as the engine must provide the two timestamp now in order to support SVS of T30
TEST_F(CAVSyncTest, DISABLED_Test_send_audio_with_timestamp_without_sampleTimestamp_return_right_timestamp)
{
    CScopedTracer test_info;
#ifdef ANDROID
    CNTPTime ntp = CNTPTime::Now();
    uint32_t timestamp = ntp.ToClockTime().ToMilliseconds();
#else
    uint32_t timestamp = TickNowMS();
#endif
    InitSender(RTP_SESSION_WEBEX_VOIP);
    InitRecver(RTP_SESSION_WEBEX_VOIP, timestamp+1, 0);
    BindSender();

    ::SleepMs(100);
    WRTPMediaData *firstAudioData = CreateVoIPData(timestamp, 0, 1024);
    SendData(firstAudioData);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (m_sendingSinkForSender->GetRTCPSendCount() > 0));
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount() > 0);

    WRTPMediaData *secondAudioData = CreateVoIPData(timestamp+1, 0, 1024);
    SendData(secondAudioData);

    //::SleepMs(100);
    DestroyMediaData(firstAudioData);
    DestroyMediaData(secondAudioData);
}

TEST_F(CAVSyncTest, DISABLED_Test_send_audio_with_timestamp_without_sampleTimestamp_roll_back_return_right_timestamp)
{
    CScopedTracer test_info;
#ifdef ANDROID
    CNTPTime ntp = CNTPTime::Now();
    uint32_t timestamp = ntp.ToClockTime().ToMilliseconds();
#else
    uint32_t timestamp = TickNowMS();
#endif
    InitSender(RTP_SESSION_WEBEX_VOIP);
    InitRecver(RTP_SESSION_WEBEX_VOIP, timestamp, 0);
    BindSender();

    ::SleepMs(100);
    WRTPMediaData *firstAudioData = CreateVoIPData(timestamp+1, 0, 1024);
    SendData(firstAudioData);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (m_sendingSinkForSender->GetRTCPSendCount() > 0));
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount() > 0);

    WRTPMediaData *secondAudioData = CreateVoIPData(timestamp, 0, 1024);
    SendData(secondAudioData);

    //::SleepMs(100);
    DestroyMediaData(firstAudioData);
    DestroyMediaData(secondAudioData);
}
#endif

TEST_F(CAVSyncTest, Test_send_audio_without_timestamp_with_sampleTimestamp_return_right_timestamp)
{
    CScopedTracer test_info;
    InitSender(RTP_SESSION_WEBEX_VOIP);
    InitRecver(RTP_SESSION_WEBEX_VOIP, 0, 504);
    BindSender();

    ::SleepMs(100);
    WRTPMediaData *firstAudioData = CreateVoIPData(0, 456, 1024);
    SendData(firstAudioData);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (m_sendingSinkForSender->GetRTCPSendCount() > 0));
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount() > 0);

    WRTPMediaData *secondAudioData = CreateVoIPData(0, 504, 1024);
    SendData(secondAudioData);

    //::SleepMs(100);
    DestroyMediaData(firstAudioData);
    DestroyMediaData(secondAudioData);
}

TEST_F(CAVSyncTest, Test_send_audio_without_timestamp_with_sampleTimestamp_roll_back_return_right_timestamp)
{
    CScopedTracer test_info;
    InitSender(RTP_SESSION_WEBEX_VOIP);
    InitRecver(RTP_SESSION_WEBEX_VOIP, 0, 456);
    BindSender();

    ::SleepMs(100);
    WRTPMediaData *firstAudioData = CreateVoIPData(0, 504, 1024);
    SendData(firstAudioData);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (m_sendingSinkForSender->GetRTCPSendCount() > 0));
    EXPECT_TRUE(m_sendingSinkForSender->GetRTCPSendCount() > 0);

    WRTPMediaData *secondAudioData = CreateVoIPData(0, 456, 1024);
    SendData(secondAudioData);

    //::SleepMs(100);
    DestroyMediaData(firstAudioData);
    DestroyMediaData(secondAudioData);
}
#endif

class CRapidAVSyncTest : public CAVSyncTest
{
public:
    void InitSender(WRTPSessionType sessionType)
    {
        CAVSyncTest::InitSender(sessionType);
        int32_t ret = m_SendSession->UpdateRTPExtension(RapidSyncExtURI, RapidSyncExtId, wrtp::STREAM_INOUT);
        ASSERT_TRUE(WRTP_SUCCEEDED(ret));
    }

    void InitRecver(WRTPSessionType sessionType, uint32_t secondTimestamp, uint32_t secondSampleTimestamp)
    {
        CAVSyncTest::InitRecver(sessionType, secondTimestamp, secondSampleTimestamp);
        int32_t ret = m_RecvSession->UpdateRTPExtension(RapidSyncExtURI, RapidSyncExtId, wrtp::STREAM_IN);
        ASSERT_TRUE(WRTP_SUCCEEDED(ret));
    }

    void BindSender()
    {
        CAVSyncTest::BindSender();
        m_sendingSinkForSender->SetDropRTCPFlag(true);
    }

private:
    const char *RapidSyncExtURI     = "urn:ietf:params:rtp-hdrext:ntp-64";
    const uint8_t RapidSyncExtId      = 5;
};

TEST_F(CRapidAVSyncTest, Test_send_video_with_timestamp_and_sampleTimestamp_return_right_timestamp2)
{
    CScopedTracer test_info;

#ifdef ANDROID
    CNTPTime ntp = CNTPTime::Now();
    uint32_t timestamp = ntp.ToClockTime().ToMilliseconds();
#else
    uint32_t timestamp = TickNowMS();
#endif

    InitSender(RTP_SESSION_WEBEX_VIDEO);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, timestamp+1, 546);
    BindSender();

    WRTPMediaData *firstVideoData = CreateVideoData(timestamp, 456, 1024, m_streamId);
    WRTP_INFOTRACE("Send first packet:"<<timestamp<<",456\n");
    SendData(firstVideoData);

    WRTPMediaData *secondVideoData = CreateVideoData(timestamp+1, 546, 1024, m_streamId);
    WRTP_INFOTRACE("Send second packet:"<<timestamp+1<<",546\n");
    SendData(secondVideoData);

    DestroyMediaData(firstVideoData);
    DestroyMediaData(secondVideoData);
}

TEST_F(CRapidAVSyncTest, Test_send_audio_with_timestamp_and_sampleTimestamp_return_right_timestamp)
{
    CScopedTracer test_info;
#ifdef ANDROID
    CNTPTime ntp = CNTPTime::Now();
    uint32_t timestamp = ntp.ToClockTime().ToMilliseconds();
#else
    uint32_t timestamp = TickNowMS();
#endif
    InitSender(RTP_SESSION_WEBEX_VOIP);
    InitRecver(RTP_SESSION_WEBEX_VOIP, timestamp+1, 504);
    BindSender();

    WRTPMediaData *firstAudioData = CreateVoIPData(timestamp, 456, 1024);
    SendData(firstAudioData);

    WRTPMediaData *secondAudioData = CreateVoIPData(timestamp+1, 504, 1024);
    SendData(secondAudioData);

    DestroyMediaData(firstAudioData);
    DestroyMediaData(secondAudioData);
}

TEST_F(CRapidAVSyncTest, Test_send_sharing_with_timestamp_and_sampleTimestamp_return_right_timestamp)
{
    CScopedTracer test_info;

#ifdef ANDROID
    CNTPTime ntp = CNTPTime::Now();
    uint32_t timestamp = ntp.ToClockTime().ToMilliseconds();
#else
    uint32_t timestamp = TickNowMS();
#endif

    InitSender(RTP_SESSION_WEBEX_AS);
    InitRecver(RTP_SESSION_WEBEX_AS, timestamp+1, 546);
    BindSender();

    WRTPMediaData *firstVideoData = CreateVideoData(timestamp, 456, 1024, m_streamId);
    WRTP_INFOTRACE("Send first packet:"<<timestamp<<",456\n");
    SendData(firstVideoData);

    WRTPMediaData *secondVideoData = CreateVideoData(timestamp+1, 546, 1024, m_streamId);
    WRTP_INFOTRACE("Send second packet:"<<timestamp+1<<",546\n");
    SendData(secondVideoData);

    DestroyMediaData(firstVideoData);
    DestroyMediaData(secondVideoData);
}