#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "wrtpclientapi.h"
#include "rtpsessionconfig.h"
#include "WMEInterfaceMock.h"
#include "testutil.h"

using namespace wrtp;

class CSessionConfigObserverMock : public IRTPSessionConfigObserver
{
public:
    CSessionConfigObserverMock() : m_maxPayloadSize(0), m_callbackTimes(0) {}

    virtual void OnMaxRTPPayloadSizeUpdated(uint32_t newMaxPayloadSize)
    {
        m_maxPayloadSize = newMaxPayloadSize;
        ++m_callbackTimes;
    }

    uint32_t GetMaxPayloadSize()  { return m_maxPayloadSize; }
    uint32_t GetCallbackTimes()   { return m_callbackTimes; }

private:
    uint32_t m_maxPayloadSize;
    uint32_t m_callbackTimes;
};

#define CODEC_TYPE_VIDEO_TEST   98
#define PAYLOAD_TYPE_VIDEO_TEST 98
class CSessionConfigObserverTest : public ::testing::Test
{
public:
    CSessionConfigObserverTest() : m_rtpVideoSession(nullptr) {}

protected:
    virtual void SetUp()
    {
        // init RTP session
        WRTPSessionParams sessionParam;
        sessionParam.sessionType = RTP_SESSION_WEBEX_VIDEO;
        sessionParam.enableRTCP  = true;
        m_rtpVideoSession = WRTPCreateRTPSessionClient(sessionParam);

        ASSERT_TRUE(nullptr != m_rtpVideoSession);
        m_rtpVideoSession->IncreaseReference();

        // To keep the backward compatibility
        m_rtpVideoSession->SetPacketizationMode(PACKETIZATION_MODE1);
        m_rtpVideoSession->EnableSendingAggregationPacket(false);
        m_rtpVideoSession->SetMaxPacketSize(1400);

        uint32_t ret = m_rtpVideoSession->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
        ASSERT_TRUE(0 == ret);

        ret = m_rtpVideoSession->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);
        ASSERT_TRUE(0 == ret);

    }

    virtual void TearDown()
    {
        if (m_rtpVideoSession) {
            m_rtpVideoSession->DecreaseReference();
            m_rtpVideoSession = nullptr;
        }
    }

protected:
    IRTPSessionClient  *m_rtpVideoSession;
};

TEST_F(CSessionConfigObserverTest, max_payload_size_updated_if_add_header_extension)
{
    CScopedTracer test_info;
    CSessionConfigObserverMock observer;

    uint32_t ret = m_rtpVideoSession->RegisterSessionConfigObserver(&observer);
    ASSERT_EQ(0, ret);

    uint32_t maxPayloadSize   = observer.GetMaxPayloadSize();
    uint32_t callbackTimes    = observer.GetCallbackTimes();
    EXPECT_EQ(1, callbackTimes);

    // Register a header extension
    char *extURI    = const_cast<char *>("http://protocols.cisco.com/virtualid");
    uint8_t extId     = 1;

    ret = m_rtpVideoSession->UpdateRTPExtension(extURI, extId, wrtp::STREAM_INOUT);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    callbackTimes = observer.GetCallbackTimes();
    EXPECT_EQ(2, callbackTimes);
    EXPECT_GT(maxPayloadSize, observer.GetMaxPayloadSize());
    maxPayloadSize = observer.GetMaxPayloadSize();

    // Register another header extension
    extURI    = const_cast<char *>("urn:ietf:params:rtp-hdrext:toffset");
    extId     = 2;

    ret = m_rtpVideoSession->UpdateRTPExtension(extURI, extId, wrtp::STREAM_INOUT);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    callbackTimes = observer.GetCallbackTimes();
    EXPECT_EQ(3, callbackTimes);
    EXPECT_GT(maxPayloadSize, observer.GetMaxPayloadSize());
    maxPayloadSize = observer.GetMaxPayloadSize();

    // unregister the observer
    ret = m_rtpVideoSession->UnregisterSessionConfigObserver(&observer);
    ASSERT_EQ(0, ret);

    // Register another header extension
    extURI    = const_cast<char *>("http://protocols.cisco.com/framemarking");
    extId     = 3;

    ret = m_rtpVideoSession->UpdateRTPExtension(extURI, extId, wrtp::STREAM_INOUT);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(callbackTimes, observer.GetCallbackTimes());
}

TEST_F(CSessionConfigObserverTest, max_payload_size_updated_if_change_max_RTP_packet_size)
{
    CScopedTracer test_info;
    CSessionConfigObserverMock observer;

    uint32_t ret = m_rtpVideoSession->RegisterSessionConfigObserver(&observer);
    ASSERT_EQ(0, ret);

    uint32_t maxPayloadSize   = observer.GetMaxPayloadSize();
    uint32_t callbackTimes    = observer.GetCallbackTimes();
    EXPECT_EQ(1, callbackTimes);

    // set max RTP packet size
    ret = m_rtpVideoSession->SetMaxPacketSize(1300);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    callbackTimes = observer.GetCallbackTimes();
    EXPECT_EQ(2, callbackTimes);
    EXPECT_GT(maxPayloadSize, observer.GetMaxPayloadSize());
    maxPayloadSize = observer.GetMaxPayloadSize();


    // set max RTP packet size
    ret = m_rtpVideoSession->SetMaxPacketSize(1200);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    callbackTimes = observer.GetCallbackTimes();
    EXPECT_EQ(3, callbackTimes);
    EXPECT_GT(maxPayloadSize, observer.GetMaxPayloadSize());
    maxPayloadSize = observer.GetMaxPayloadSize();

    // unregister the observer
    ret = m_rtpVideoSession->UnregisterSessionConfigObserver(&observer);
    ASSERT_EQ(0, ret);

    // set max RTP packet size
    ret = m_rtpVideoSession->SetMaxPacketSize(1100);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(callbackTimes, observer.GetCallbackTimes());
}

TEST_F(CSessionConfigObserverTest, max_payload_size_updated_if_set_outbound_SRTP)
{
    CScopedTracer test_info;
    CSessionConfigObserverMock observer;

    uint32_t ret = m_rtpVideoSession->RegisterSessionConfigObserver(&observer);
    ASSERT_EQ(0, ret);

    uint32_t maxPayloadSize   = observer.GetMaxPayloadSize();
    uint32_t callbackTimes    = observer.GetCallbackTimes();
    EXPECT_EQ(1, callbackTimes);

    // set outbound SRTP
    uint8_t keys[30] = {};
    ret = WRTPGetCryptoRandom(keys, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    SecurityConfiguration configuration;
    configuration.cryptoSuiteType       = CST_AES_CM_128_HMAC_SHA1_80;
    configuration.masterKeySalt         = nullptr;
    configuration.masterKeySaltLength   = 0;
    configuration.rtpSecurityService    = SEC_SERVICE_CONF_AUTH;
    configuration.rtcpSecurityService   = SEC_SERVICE_CONF_AUTH;
    configuration.fecOrder              = ORDER_SRTP_FEC;
    configuration.ekt                   = nullptr;
    configuration.masterKeySalt         = keys;
    configuration.masterKeySaltLength   = 30;

    ret = m_rtpVideoSession->SetOutboundSecurityConfiguration(configuration);
    ASSERT_EQ(0, ret);

    callbackTimes = observer.GetCallbackTimes();
    EXPECT_EQ(2, callbackTimes);
    EXPECT_GT(maxPayloadSize, observer.GetMaxPayloadSize());
    maxPayloadSize = observer.GetMaxPayloadSize();

    // unregister the observer
    ret = m_rtpVideoSession->UnregisterSessionConfigObserver(&observer);
    ASSERT_EQ(0, ret);

}

TEST_F(CSessionConfigObserverTest, max_payload_size_updated_in_mixed_cases)
{
    CScopedTracer test_info;
    CSessionConfigObserverMock observer;

    uint32_t ret = m_rtpVideoSession->RegisterSessionConfigObserver(&observer);
    ASSERT_EQ(0, ret);

    uint32_t maxPayloadSize   = observer.GetMaxPayloadSize();
    EXPECT_EQ(1, observer.GetCallbackTimes());

    // set max RTP packet size
    ret = m_rtpVideoSession->SetMaxPacketSize(1100);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    maxPayloadSize   = observer.GetMaxPayloadSize();
    EXPECT_EQ(2, observer.GetCallbackTimes());
    EXPECT_EQ(1100-12-4, maxPayloadSize);

    // Register a header extension
    char *extURI    = const_cast<char *>("http://protocols.cisco.com/virtualid");
    uint8_t extId     = 1;

    ret = m_rtpVideoSession->UpdateRTPExtension(extURI, extId, wrtp::STREAM_INOUT);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(3, observer.GetCallbackTimes());
    EXPECT_EQ(maxPayloadSize - 4 - 4, observer.GetMaxPayloadSize());
    maxPayloadSize   = observer.GetMaxPayloadSize();

    // set outbound SRTP
    uint8_t keys[30] = {};
    ret = WRTPGetCryptoRandom(keys, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    SecurityConfiguration configuration;
    configuration.cryptoSuiteType       = CST_AES_CM_128_HMAC_SHA1_80;
    configuration.masterKeySalt         = nullptr;
    configuration.masterKeySaltLength   = 0;
    configuration.rtpSecurityService    = SEC_SERVICE_CONF_AUTH;
    configuration.rtcpSecurityService   = SEC_SERVICE_CONF_AUTH;
    configuration.fecOrder              = ORDER_SRTP_FEC;
    configuration.ekt                   = nullptr;
    configuration.masterKeySalt         = keys;
    configuration.masterKeySaltLength   = 30;

    ret = m_rtpVideoSession->SetOutboundSecurityConfiguration(configuration);
    ASSERT_EQ(0, ret);

    EXPECT_EQ(4, observer.GetCallbackTimes());
    EXPECT_EQ(maxPayloadSize - 10, observer.GetMaxPayloadSize());
    maxPayloadSize = observer.GetMaxPayloadSize();
}