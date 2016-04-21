#include "gmock/gmock.h"
#include "gtest/gtest.h"
#define private public
#define protected public
#include "wrtpcommonapi.h"
#include "rtpsessionclient.h"
#include "qosapimock.h"
#include "testutil.h"
#include "qoshandler.h"

using namespace wrtp;
using namespace testing;

class CNetworkStatusObserver : public INetworkStatusObserver
{
public:
    CNetworkStatusObserver(): m_allocatedBandwidth(0)
        , m_rtt(0), m_queuingDelay(0), m_totalBandwidth(0), m_lossRate(0.0f), m_state(0) {}

    void OnAdjustment(const NetworkStatus &status)
    {
        m_allocatedBandwidth    = status.allocatedBandwidth;
        m_rtt                   = status.rtt;
        m_queuingDelay          = status.queuingDelay;
        m_totalBandwidth        = status.totalBandwidth;
        m_lossRate              = status.lossRate;
        m_state                 = status.state;
        return;
    }

    virtual void NotifyStreamBandwidth(const StreamIdInfo &streamId, uint32_t bandwidthInBytePerSec) {}

    uint32_t GetAllocatedBandwidth() const
    {
        return m_allocatedBandwidth;
    }
    uint32_t GetRTT() const
    {
        return m_rtt;
    }
    uint32_t GetQueuingDelay() const
    {
        return m_queuingDelay;
    }

    uint32_t GetTotalBandwidth() const
    {
        return m_totalBandwidth;
    }
    float GetLossRate() const
    {
        return m_lossRate;
    }
    uint32_t GetState() const
    {
        return m_state;
    }

private:
    uint32_t m_state;
    uint32_t m_allocatedBandwidth;
    uint32_t m_totalBandwidth;
    uint32_t m_queuingDelay;
    uint32_t m_rtt;
    float m_lossRate;
};
class CInformQoSAPITEST : public ::testing::Test
{
public:
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {

    }

public:
    WRTPSessionParams m_rtpSessionParams;
};

class CMediaSessionFilter:public IMediaSessionFilter
{
public:
    CMediaSessionFilter(uint32_t minBandwidth, uint32_t maxBandwidth): m_minBandwidth(minBandwidth), m_maxBandwidth(maxBandwidth) {}
    ~CMediaSessionFilter() {}

    virtual int32_t QueryRequirement(QUERY_TYPE eType, uint32_t &bandwidth)
    {
        switch (eType) {
            case QUERY_MAX_BW:
                bandwidth = m_maxBandwidth;
                break;

            case QUERY_MIN_BW:
                bandwidth = m_minBandwidth;
                break;

            default:
                break;
        }

        return 0;
    }

    void SetMinBandwidth(uint32_t bw) { m_minBandwidth = bw; }
    void SetMaxBandwidth(uint32_t bw) { m_maxBandwidth = bw; }

private:
    uint32_t m_minBandwidth;
    uint32_t m_maxBandwidth;
};


TEST_F(CInformQoSAPITEST, Test_EnableQoS_when_rtcp_not_enable_then_return_error)
{
    CScopedTracer test_info;
    //create rtp session
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = false;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    EXPECT_TRUE(session != nullptr);

    session->IncreaseReference();

    QoSParameter qosParam;
    int32_t ret = session->EnableQoS(qosParam);
    EXPECT_TRUE(0 != ret);

    session->DecreaseReference();
}

TEST_F(CInformQoSAPITEST, Test_EnableQoS_with_invalid_filter_then_return_error)
{
    CScopedTracer test_info;
    //create rtp session
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;

    CNetworkStatusObserver networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    EXPECT_TRUE(session != nullptr);

    session->IncreaseReference();

    const char *peerID = "peerID";

    QoSParameter qosParam;
    //qosParam.networkStatusObserver = &networkStatusObserver;
    qosParam.primaryPriority       = 1;
    qosParam.targetBandwidth       = 50*1024;     // 50kbytes/sec
    qosParam.transportType         = TRANSPORT_TYPE_UDP;
    qosParam.peerID                = reinterpret_cast<const uint8_t *>(peerID);
    qosParam.lengthOfPeerID        = strlen(peerID);
    qosParam.smoothSending         = true;

    int32_t ret = session->EnableQoS(qosParam);
    EXPECT_TRUE(0 != ret);

    session->DecreaseReference();
}

TEST_F(CInformQoSAPITEST, Test_EnableQoS_with_invalid_peer_id_then_return_error)
{
    CScopedTracer test_info;
    //create rtp session
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;

    CNetworkStatusObserver networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    EXPECT_TRUE(session != nullptr);

    session->IncreaseReference();

    QoSParameter qosParam;
    //qosParam.networkStatusObserver = &networkStatusObserver;
    CMediaSessionFilter    mediaSessionFilter(8*1024, 100*1024);
    qosParam.mediaSessionFilter = &mediaSessionFilter;

    int32_t ret = session->EnableQoS(qosParam);
    EXPECT_TRUE(0 != ret);

    session->DecreaseReference();
}

#if 0 // Joyce:discuss later
TEST_F(CInformQoSAPITEST, Test_EnableQoS_repeatedly_then_return_error)
{
    CScopedTracer test_info;
    // create rtp session
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;

    CNetworkStatusObserver networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    EXPECT_TRUE(session != nullptr);

    session->IncreaseReference();

    CMediaSessionFilter    mediaSessionFilter(8*1024, 100*1024);
    const char *peerID = "peerID";

    QoSParameter qosParam;
    //qosParam.networkStatusObserver  = &networkStatusObserver;
    qosParam.mediaSessionFilter     = &mediaSessionFilter;
    qosParam.primaryPriority        = 1;
    qosParam.targetBandwidth        = 50*1024;     // 50kbytes/sec
    qosParam.transportType          = TRANSPORT_TYPE_UDP;
    qosParam.peerID                 = reinterpret_cast<const uint8_t *>(peerID);
    qosParam.lengthOfPeerID         = strlen(peerID);
    qosParam.smoothSending          = true;

    int32_t ret = session->EnableQoS(qosParam);
    EXPECT_TRUE(0 == ret);

    ret = session->EnableQoS(qosParam);
    EXPECT_TRUE(0 != ret);

    ret = session->DisableQoS();
    EXPECT_TRUE(0 == ret);

    session->DecreaseReference();
}
#endif
TEST_F(CInformQoSAPITEST, Test_DisableQoS_then_return_success)
{
    CScopedTracer test_info;
    // create rtp session
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;

    CNetworkStatusObserver networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    EXPECT_TRUE(session != nullptr);

    session->IncreaseReference();

    CMediaSessionFilter    mediaSessionFilter(8*1024, 100*1024);
    const char *peerID = "peerID";

    QoSParameter qosParam;
    //qosParam.networkStatusObserver = &networkStatusObserver;
    qosParam.mediaSessionFilter    = &mediaSessionFilter;
    qosParam.primaryPriority       = 1;
    qosParam.targetBandwidth       = 50*1024;     // 50kbytes/sec
    qosParam.transportType         = TRANSPORT_TYPE_UDP;
    qosParam.peerID                = reinterpret_cast<const uint8_t *>(peerID);
    qosParam.lengthOfPeerID        = strlen(peerID);
    qosParam.smoothSending         = true;

    int32_t ret = session->EnableQoS(qosParam);
    EXPECT_TRUE(0 == ret);

    ret = session->DisableQoS();
    EXPECT_TRUE(0 == ret);

    session->DecreaseReference();
}

TEST_F(CInformQoSAPITEST, Test_QoS_OnNetworkControlledInformation)
{
    CScopedTracer test_info;
    // create rtp session
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;

    CNetworkStatusObserver networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    EXPECT_TRUE(session != nullptr);

    session->IncreaseReference();

    CMediaSessionFilter    mediaSessionFilter(8*1024, 100*1024);
    const char *peerID  = "peerID";

    QoSParameter qosParam;
    //qosParam.networkStatusObserver = &networkStatusObserver;
    qosParam.mediaSessionFilter    = &mediaSessionFilter;
    qosParam.peerID                = reinterpret_cast<const uint8_t *>(peerID);
    qosParam.lengthOfPeerID        = strlen(peerID);

    // successfully registered
    int32_t ret = session->EnableQoS(qosParam);
    EXPECT_TRUE(0 == ret);

    uint32_t bandwidth    = 60 * 1024;
    uint32_t rtt          = 50;
    uint32_t queuingDelay = 20;
    float lossRate      = 0.5f;

    AggregateMetric metric;
    metric.rtt       = rtt;
    metric.qdelay    = queuingDelay;
    metric.lossRatio = lossRate;
    // QoS callback
    CRTPSessionClient *sessionClient = (CRTPSessionClient *)(session);
    ASSERT_TRUE(sessionClient != nullptr);

    CRTCPHandlerSharedPtr rtcphandler = sessionClient->GetRTCPHandler();
    ASSERT_TRUE(rtcphandler != nullptr);

    rtcphandler->ConfigTMMBR(false, false);
    rtcphandler->OnNetworkControlledInformation(wqos::BANDWIDTH_HOLD, metric, bandwidth, bandwidth);

    // check filter data
    EXPECT_EQ(networkStatusObserver.GetAllocatedBandwidth(), (int32_t)(bandwidth*WRTP_PAYLOAD_BW_RATIO));
    EXPECT_EQ(networkStatusObserver.GetRTT(), rtt);
    EXPECT_EQ(networkStatusObserver.GetQueuingDelay(), queuingDelay);
    EXPECT_EQ(networkStatusObserver.GetTotalBandwidth(), bandwidth);
    EXPECT_EQ(networkStatusObserver.GetLossRate(), lossRate);
    EXPECT_EQ(networkStatusObserver.GetState(), wqos::BANDWIDTH_HOLD);

    session->DecreaseReference();
}

///////////////////////////////////////////////////////////////////////////////////////////
TEST_F(CInformQoSAPITEST, Test_Bandwidth_Query)
{
    uint32_t minBW = 100*1024;    // 100k bytes/sec
    uint32_t maxBW = 500*1024;    // 500k bytes/sec
    RTPSessionContextSharedPtr      sessionConfig(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO));
    CMediaSessionFilter             sessionFilter(minBW, maxBW);

    CMediaSessionBandwidthContact bandwidthContact(sessionConfig, &sessionFilter, 0);

    EXPECT_EQ(minBW, bandwidthContact.GetMinBandwidth());
    EXPECT_EQ(maxBW, bandwidthContact.GetMaxBandwidth());

    // update max-bw
    minBW = minBW / 2;
    maxBW = maxBW * 2;
    sessionFilter.SetMinBandwidth(minBW);
    sessionFilter.SetMaxBandwidth(maxBW);
    EXPECT_EQ(minBW, bandwidthContact.GetMinBandwidth());
    EXPECT_EQ(maxBW, bandwidthContact.GetMaxBandwidth());

    // set the max-bw of RTP session larger than max-bw of session filter
    uint32_t maxSessionBW = maxBW * 2;
    int32_t ret = sessionConfig->GetOutboundConfig()->SetMaxSessionBandwidth(maxSessionBW);
    EXPECT_TRUE(WRTP_SUCCEEDED(ret));

    EXPECT_EQ(maxBW, bandwidthContact.GetMaxBandwidth());

    // set the max-bw of RTP session less than max-bw of session filter
    maxSessionBW = maxBW / 2;
    ret = sessionConfig->GetOutboundConfig()->SetMaxSessionBandwidth(maxSessionBW);
    EXPECT_TRUE(WRTP_SUCCEEDED(ret));

    EXPECT_EQ(maxSessionBW, bandwidthContact.GetMaxBandwidth());

}
