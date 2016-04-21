#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "CmReferenceControl.h"
#include "CmUtilMisc.h"

#include "wrtpclientapi.h"
#include "wrtppacket.h"
#include "rtpsessionconfig.h"

#include "fragmentop.h"
#include "WMEInterfaceMock.h"
#include "testutil.h"

using namespace wrtp;
using namespace wme;

/////////////////////////////////////////////////////////////////////////////////////////////////
#define BUFFER_COUNT 64

class CWmeMediaTransportMock : public IWmeMediaTransport
{
public:
    CWmeMediaTransportMock()
        : m_rtpCount(0)
        , m_rtcpCount(0)
    {
        for (uint32_t i = 0; i < BUFFER_COUNT; ++i) {
            m_rtpPackages[i] = nullptr;
        }
        for (uint32_t i = 0; i < BUFFER_COUNT; ++i) {
            m_rtcpPackages[i] = nullptr;
        }
    }

    virtual ~CWmeMediaTransportMock()
    {
        for (uint32_t i = 0; i < m_rtpCount; ++i) {
            WME_SAFE_RELEASE(m_rtpPackages[i]);
        }
        for (uint32_t i = 0; i < m_rtcpCount; ++i) {
            WME_SAFE_RELEASE(m_rtcpPackages[i]);
        }
    }

    virtual WMERESULT GetTransportType(WmeTransportType &eTransportType)
    {
        eTransportType = WmeTransportType_UDP;
        return WME_S_OK;
    }

    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage)
    {
        EXPECT_TRUE(nullptr != pRTPPackage);
        if (pRTPPackage) {
            if (m_rtpCount + 1 < BUFFER_COUNT) {
                WME_SAFE_ADDREF(pRTPPackage);
                m_rtpPackages[m_rtpCount] = pRTPPackage;
                ++m_rtpCount;
            } else {
                return WME_S_FALSE;
            }
        }

        return WME_S_OK;
    }

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        EXPECT_TRUE(nullptr != pRTCPPackage);
        if (pRTCPPackage) {
            if (m_rtcpCount + 1 < BUFFER_COUNT) {
                WME_SAFE_ADDREF(pRTCPPackage);
                m_rtcpPackages[m_rtcpCount] = pRTCPPackage;
                ++m_rtcpCount;
            } else {
                return WME_S_FALSE;
            }
        }

        return WME_S_OK;
    }

    uint32_t GetRTPCount()
    {
        return m_rtpCount;
    }

    IWmeMediaPackage *RefGetLatestRTPPackage(uint32_t index = 0) const
    {
        IWmeMediaPackage *ret = nullptr;
        if (index < m_rtpCount) {
            ret = m_rtpPackages[index];
        }

        WME_SAFE_ADDREF(ret);
        return ret;
    }

    uint32_t GetRTCPCount()
    {
        return m_rtcpCount;
    }

    IWmeMediaPackage *RefGetLatestRTCPPackage(uint32_t index = 0) const
    {
        IWmeMediaPackage *ret = nullptr;
        if (index < m_rtcpCount) {
            ret = m_rtcpPackages[index];
        }

        WME_SAFE_ADDREF(ret);
        return ret;
    }

private:
    uint32_t            m_rtpCount;
    IWmeMediaPackage *m_rtpPackages[BUFFER_COUNT];
    uint32_t            m_rtcpCount;
    IWmeMediaPackage *m_rtcpPackages[BUFFER_COUNT];
};

/////////////////////////////////////////////////////////////////////////////////////////////////
class CVoIPSenderTest : public ::testing::Test, public IMediaDataRecvSink
{
public:
    CVoIPSenderTest()
        : m_rtpSession(nullptr)
        , m_rtpChannel(nullptr)
        , m_channelId(12345)
        ,  m_payloadType(100)
        , m_timestamp(1000)
        , m_sampleTimestamp(1000)
        , m_mediaDataLen(40)
        , m_marker(1)
        , m_mediaReceived(false)
        , m_packetizerMock()
    {
    }

    virtual ~CVoIPSenderTest() {}

    virtual void SetUp()
    {
        // init RTP session
        WRTPSessionParams sessionParam;
        sessionParam.sessionType = RTP_SESSION_WEBEX_VOIP;
        sessionParam.enableRTCP  = true;
        m_rtpSession = WRTPCreateRTPSessionClient(sessionParam);

        ASSERT_TRUE(nullptr != m_rtpSession);
        m_rtpSession->IncreaseReference();

        uint32_t ret = m_rtpSession->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
        ASSERT_TRUE(0 == ret);

        m_mediaTransport.reset(new CWmeMediaTransportMock());
        ret = m_rtpSession->SetMediaTransport(m_mediaTransport.get());
        ASSERT_TRUE(0 == ret);

        ret = m_rtpSession->RegisterPayloadType(CODEC_TYPE_AUDIO_TEST, PAYLOAD_TYPE_AUDIO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);
        ASSERT_TRUE(0 == ret);

        // init RTP channel
        WRTPChannelParams channelParam;
        m_rtpChannel = m_rtpSession->CreateRTPChannel(m_channelId, channelParam);

        ASSERT_TRUE(nullptr != m_rtpChannel);
        m_rtpChannel->IncreaseReference();
        m_rtpChannel->SetPacketizationOperator(&m_packetizerMock);

        // set media recv sink
        m_rtpChannel->SetMediaDataRecvSink(this);
    }

    virtual void TearDown()
    {
        if (m_rtpChannel) {
            m_rtpChannel->Close();
            m_rtpChannel->DecreaseReference();
            m_rtpChannel = nullptr;
        }

        if (m_rtpSession) {
            m_rtpSession->DecreaseReference();
            m_rtpSession = nullptr;
        }

        m_mediaTransport.reset();

    }

    virtual int32_t OnRecvMediaData(/*RTP_IN*/ WRTPMediaData *pData, /*RTP_IN*/ WRTPMediaDataIntegrityInfo *pIntegrityInfo)
    {
        m_mediaReceived = true;
        CheckMediaData(pData);
        return 0;
    }

private:

    void CheckMediaData(WRTPMediaData *pData)
    {
        ASSERT_TRUE(nullptr != pData);
        ASSERT_EQ(pData->type, (WRTPDataType)MEDIA_DATA_VOIP);
        EXPECT_EQ(pData->sampleTimestamp, m_sampleTimestamp);
        EXPECT_EQ(pData->marker, m_marker);
        EXPECT_EQ(pData->size, m_mediaDataLen);
        EXPECT_EQ(1, pData->csrcCount);
        EXPECT_EQ(m_channelId, pData->csrcArray[0]);

    }

protected:
    IRTPSessionClient  *m_rtpSession;
    IRTPChannel        *m_rtpChannel;
    uint32_t              m_channelId;

    std::unique_ptr<CWmeMediaTransportMock> m_mediaTransport;

    uint8_t   m_payloadType;
    uint32_t  m_timestamp;
    uint32_t  m_sampleTimestamp;
    uint32_t  m_mediaDataLen;
    uint32_t  m_marker;

    bool    m_mediaReceived;
    CRTPPacketizationMock m_packetizerMock;
};


TEST_F(CVoIPSenderTest, Send_VoIP_Media_Data_then_Return_Success)
{
    CScopedTracer test_info;
    WRTPMediaData *md = CreateVoIPData(m_timestamp, m_sampleTimestamp, m_mediaDataLen, m_marker);
    m_rtpChannel->SendMediaData(md);
    DestroyMediaData(md);

    IWmeMediaPackage *package = m_mediaTransport->RefGetLatestRTPPackage();
    ASSERT_TRUE(nullptr != package);

    // get the address of RTP packet
    uint8_t *dataPointer = nullptr;
    WMERESULT wmeRes = package->GetDataPointer(&dataPointer);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRes));
    ASSERT_TRUE(nullptr != dataPointer);

    // get the length of RTP packet
    uint32_t packetLen = 0;
    wmeRes = package->GetDataLength(packetLen);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRes));
    ASSERT_TRUE(packetLen > 0);

    CRTPPacketLazy rtpPacket;
    CmResult cmRes = rtpPacket.Bind(dataPointer, packetLen);
    ASSERT_TRUE(CM_SUCCEEDED(cmRes));

    // check the packet field
    EXPECT_EQ(rtpPacket.GetTimestamp(), m_sampleTimestamp);
    EXPECT_EQ(rtpPacket.HasMarker(), m_marker);
    EXPECT_EQ(rtpPacket.GetPayloadLength(), m_mediaDataLen);

    WME_SAFE_RELEASE(package);
}

TEST_F(CVoIPSenderTest, Recv_VoIP_Media_Data_then_Return_Success)
{
    CScopedTracer test_info;
    WRTPMediaData *md = CreateVoIPData(m_timestamp, m_sampleTimestamp, m_mediaDataLen, m_marker);
    m_rtpChannel->SendMediaData(md);
    DestroyMediaData(md);

    IWmeMediaPackage *package = m_mediaTransport->RefGetLatestRTPPackage();
    ASSERT_TRUE(nullptr != package);

    CmResult cmRes = m_rtpChannel->ReceiveRTPPacket(package);
    EXPECT_TRUE(CM_SUCCEEDED(cmRes));
    EXPECT_TRUE(m_mediaReceived);

    WME_SAFE_RELEASE(package);
}

class CVideoSenderTest : public ::testing::Test, public IMediaDataRecvSink
{
public:
    CVideoSenderTest()
        : m_rtpSession(nullptr)
        , m_rtpChannel(nullptr)
        , m_channelId(12345)
        , m_payloadType(100)
        , m_did(4)
        , m_timestamp(1000)
        , m_sampleTimestamp(1000)
        , m_mediaDataLen(2000)
        , m_marker(1)
        , m_mediaReceived(false)
    {
    }

    virtual void SetUp()
    {
        // init RTP session
        WRTPSessionParams sessionParam;
        sessionParam.sessionType = RTP_SESSION_WEBEX_VIDEO;
        sessionParam.enableRTCP  = true;
        m_rtpSession = WRTPCreateRTPSessionClient(sessionParam);

        ASSERT_TRUE(nullptr != m_rtpSession);
        m_rtpSession->IncreaseReference();

        // To keep the backward compatibility
        m_rtpSession->SetPacketizationMode(PACKETIZATION_MODE1);
        m_rtpSession->EnableSendingAggregationPacket(false);
        m_rtpSession->SetMaxPacketSize(1400);

        uint32_t ret = m_rtpSession->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
        ASSERT_TRUE(0 == ret);

        m_mediaTransport.reset(new CWmeMediaTransportMock());
        ret = m_rtpSession->SetMediaTransport(m_mediaTransport.get());
        ASSERT_TRUE(0 == ret);

        ret = m_rtpSession->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
        ASSERT_TRUE(0 == ret);

        ret = m_rtpSession->SetMaxPacketSize(1400);
        ASSERT_TRUE(0 == ret);

        // init RTP channel
        WRTPChannelParams channelParam;
        // Don't fragment, just send little media package, so give garbage pointer
        channelParam.pPacketizer = &m_packetizerMock;
        m_rtpChannel = m_rtpSession->CreateRTPChannel(m_channelId, channelParam);

        ASSERT_TRUE(nullptr != m_rtpChannel);
        m_rtpChannel->IncreaseReference();
        m_rtpChannel->SetPacketizationOperator(&m_packetizerMock);

        // set media recv sink
        m_rtpChannel->SetMediaDataRecvSink(this);
    }

    virtual void TearDown()
    {
        if (m_rtpChannel) {
            m_rtpChannel->Close();
            m_rtpChannel->DecreaseReference();
            m_rtpChannel = nullptr;
        }

        if (m_rtpSession) {
            m_rtpSession->DecreaseReference();
            m_rtpSession = nullptr;
        }

        m_mediaTransport.reset();

    }

    virtual int32_t OnRecvMediaData(/*RTP_IN*/ WRTPMediaData *pData, /*RTP_IN*/ WRTPMediaDataIntegrityInfo *pIntegrityInfo)
    {
        m_mediaReceived = true;
        CheckMediaData(pData);
        return 0;
    }

private:

    void CheckMediaData(WRTPMediaData *pData)
    {
        ASSERT_TRUE(nullptr != pData);
        ASSERT_EQ(pData->type, (WRTPDataType)MEDIA_DATA_VIDEO);
        EXPECT_EQ(pData->sampleTimestamp, m_sampleTimestamp);
        EXPECT_EQ(pData->marker, m_marker);
        ASSERT_EQ(pData->size, m_mediaDataLen);
        EXPECT_EQ(0, memcmp(&*m_frameData.begin(), pData->data, pData->size));
    }

protected:
    IRTPSessionClient  *m_rtpSession;
    IRTPChannel        *m_rtpChannel;
    uint32_t              m_channelId;

    std::unique_ptr<CWmeMediaTransportMock> m_mediaTransport;

    uint8_t   m_payloadType;
    uint8_t   m_did;
    uint32_t  m_timestamp;
    uint32_t  m_sampleTimestamp;
    uint32_t  m_mediaDataLen;
    uint32_t  m_marker;

    bool    m_mediaReceived;
    CRTPPacketizationMock m_packetizerMock;
    vector<uint8_t> m_frameData;
};


TEST_F(CVideoSenderTest, Send_Video_Media_Data_then_Return_Success)
{
    CScopedTracer test_info;
    WRTPMediaData *md = CreateVideoData(m_timestamp, m_sampleTimestamp, m_mediaDataLen, m_marker, m_did);
    m_rtpChannel->SendMediaData(md);
    DestroyMediaData(md);

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    //::SleepMs(3000);
    IWmeMediaPackage *package = m_mediaTransport->RefGetLatestRTPPackage(1);
    ASSERT_TRUE(nullptr != package);

    // get the address of RTP packet
    uint8_t *dataPointer = nullptr;
    WMERESULT wmeRes = package->GetDataPointer(&dataPointer);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRes));
    ASSERT_TRUE(nullptr != dataPointer);

    // get the length of RTP packet
    uint32_t packetLen = 0;
    wmeRes = package->GetDataLength(packetLen);
    ASSERT_TRUE(WME_SUCCEEDED(wmeRes));
    ASSERT_TRUE(packetLen > 0);

    CRTPPacketLazy rtpPacket;
    CmResult cmRes = rtpPacket.Bind(dataPointer, packetLen);
    ASSERT_TRUE(CM_SUCCEEDED(cmRes));

    // check the packet field
    EXPECT_EQ(rtpPacket.GetTimestamp(), m_sampleTimestamp);
    EXPECT_EQ(rtpPacket.HasMarker(), m_marker);

    WME_SAFE_RELEASE(package);
}

TEST_F(CVideoSenderTest, Recv_Video_Media_Data_then_Return_Success)
{
    CScopedTracer test_info;
    m_frameData.resize(m_mediaDataLen);
    WRTPMediaData *md = CreateVideoData(m_timestamp, m_sampleTimestamp, m_mediaDataLen, m_marker, m_did);
    memcpy(&*m_frameData.begin(), md->data, m_mediaDataLen);
    m_rtpChannel->SendMediaData(md);
    DestroyMediaData(md);

    //::SleepMs(3000);
    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    IWmeMediaPackage *package = m_mediaTransport->RefGetLatestRTPPackage(0);
    ASSERT_TRUE(nullptr != package);
    CmResult cmRes = m_rtpChannel->ReceiveRTPPacket(package);
    EXPECT_TRUE(CM_SUCCEEDED(cmRes));
    package = m_mediaTransport->RefGetLatestRTPPackage(1);
    ASSERT_TRUE(nullptr != package);
    cmRes = m_rtpChannel->ReceiveRTPPacket(package);
    EXPECT_TRUE(CM_SUCCEEDED(cmRes));

    EXPECT_TRUE(m_mediaReceived);

    WME_SAFE_RELEASE(package);
}

class CRTCPSenderTest : public ::testing::Test, public IMediaDataRecvSink
{
public:
    CRTCPSenderTest()
        : m_rtpSession(nullptr)
        , m_rtpChannel(nullptr)
        , m_channelId(12345)
        , m_payloadType(100)
        , m_did(4)
        , m_timestamp(1000)
        , m_sampleTimestamp(1000)
        , m_mediaDataLen(2000)
        , m_marker(1)
        , m_mediaReceived(false)
        , m_packetizerMock()
    {
    }

    virtual void SetUp()
    {
        // init RTP session
        WRTPSessionParams sessionParam;
        sessionParam.sessionType = RTP_SESSION_WEBEX_VIDEO;
        sessionParam.enableRTCP  = true;
        m_rtpSession = WRTPCreateRTPSessionClient(sessionParam);

        ASSERT_TRUE(nullptr != m_rtpSession);
        m_rtpSession->IncreaseReference();

        // To keep the backward compatibility
        m_rtpSession->SetPacketizationMode(PACKETIZATION_MODE1);
        m_rtpSession->EnableSendingAggregationPacket(false);
        m_rtpSession->SetMaxPacketSize(1400);

        uint32_t ret = m_rtpSession->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
        ASSERT_TRUE(0 == ret);

        m_mediaTransport.reset(new CWmeMediaTransportMock());
        ret = m_rtpSession->SetMediaTransport(m_mediaTransport.get());
        ASSERT_TRUE(0 == ret);

        ret = m_rtpSession->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
        ASSERT_TRUE(0 == ret);

        // init RTP channel
        WRTPChannelParams channelParam;
        // Don't fragment, just send little media package, so give garbage pointer
        channelParam.pPacketizer = &m_packetizerMock;
        m_rtpChannel = m_rtpSession->CreateRTPChannel(m_channelId, channelParam);

        ASSERT_TRUE(nullptr != m_rtpChannel);
        m_rtpChannel->IncreaseReference();

        // set media recv sink
        m_rtpChannel->SetMediaDataRecvSink(this);
        m_rtpChannel->SetPacketizationOperator(&m_packetizerMock);
    }

    virtual void TearDown()
    {
        if (m_rtpChannel) {
            m_rtpChannel->Close();
            m_rtpChannel->DecreaseReference();
            m_rtpChannel = nullptr;
        }

        if (m_rtpSession) {
            m_rtpSession->DecreaseReference();
            m_rtpSession = nullptr;
        }

        m_mediaTransport.reset();

    }

    virtual int32_t OnRecvMediaData(/*RTP_IN*/ WRTPMediaData *pData, /*RTP_IN*/ WRTPMediaDataIntegrityInfo *pIntegrityInfo)
    {
        m_mediaReceived = true;
        CheckMediaData(pData);
        return 0;
    }

private:

    void CheckMediaData(WRTPMediaData *pData)
    {
        ASSERT_TRUE(nullptr != pData);
        ASSERT_EQ(pData->type, (WRTPDataType)MEDIA_DATA_VIDEO);
        EXPECT_EQ(pData->sampleTimestamp, m_sampleTimestamp);
        EXPECT_EQ(pData->marker, m_marker);
        ASSERT_EQ(pData->size, m_mediaDataLen);
        EXPECT_EQ(0, memcmp(&*m_frameData.begin(), pData->data, pData->size));
    }

protected:
    IRTPSessionClient  *m_rtpSession;
    IRTPChannel        *m_rtpChannel;
    uint32_t              m_channelId;

    std::unique_ptr<CWmeMediaTransportMock> m_mediaTransport;

    uint8_t   m_payloadType;
    uint8_t   m_did;
    uint32_t  m_timestamp;
    uint32_t  m_sampleTimestamp;
    uint32_t  m_mediaDataLen;
    uint32_t  m_marker;

    bool    m_mediaReceived;
    vector<uint8_t> m_frameData;
    CRTPPacketizationMock m_packetizerMock;
};

TEST_F(CRTCPSenderTest, RTCP_Send_Success)
{
    CScopedTracer test_info;
    WRTPMediaData *md = CreateVideoData(m_timestamp, m_sampleTimestamp, m_mediaDataLen, m_marker, m_did);
    m_rtpChannel->SendMediaData(md);
    DestroyMediaData(md);

    m_frameData.resize(m_mediaDataLen);
    md = CreateVideoData(m_timestamp, m_sampleTimestamp, m_mediaDataLen, m_marker, m_did);
    memcpy(&*m_frameData.begin(), md->data, m_mediaDataLen);
    m_rtpChannel->SendMediaData(md);
    DestroyMediaData(md);

    //::SleepMs(3000);
    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);
    CmResult cmRes;
    for (uint32_t i = 0; i < m_mediaTransport->GetRTPCount(); ++i) {
        cmRes = m_rtpChannel->ReceiveRTPPacket(m_mediaTransport->RefGetLatestRTPPackage(i));
        EXPECT_TRUE(CM_SUCCEEDED(cmRes));
    }
    EXPECT_TRUE(m_mediaTransport->GetRTCPCount() > 0);
}

TEST_F(CRTCPSenderTest, RTCP_Recv_Success)
{
    CScopedTracer test_info;
    WRTPMediaData *md = CreateVideoData(m_timestamp, m_sampleTimestamp, m_mediaDataLen, m_marker, m_did);
    m_rtpChannel->SendMediaData(md);
    DestroyMediaData(md);

    m_frameData.resize(m_mediaDataLen);
    md = CreateVideoData(m_timestamp, m_sampleTimestamp, m_mediaDataLen, m_marker, m_did);
    memcpy(&*m_frameData.begin(), md->data, m_mediaDataLen);
    m_rtpChannel->SendMediaData(md);
    DestroyMediaData(md);

    //::SleepMs(3000);
    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);
    CmResult cmRes;
    for (uint32_t i = 0; i < m_mediaTransport->GetRTPCount(); ++i) {
        cmRes = m_rtpChannel->ReceiveRTPPacket(m_mediaTransport->RefGetLatestRTPPackage(i));
        EXPECT_TRUE(CM_SUCCEEDED(cmRes));
    }
    EXPECT_TRUE(m_mediaTransport->GetRTCPCount() > 0);

    for (uint32_t i = 0; i < m_mediaTransport->GetRTCPCount(); ++i) {
        cmRes = m_rtpSession->ReceiveRTCPPacket(m_mediaTransport->RefGetLatestRTCPPackage(i));
        EXPECT_TRUE(CM_SUCCEEDED(cmRes));
    }
}
