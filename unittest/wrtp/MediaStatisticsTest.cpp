#include "gmock/gmock.h"
#include "gtest/gtest.h"
#define private public

using ::testing::_;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::StrictMock;
using namespace std;

#include <string>
#include "rtcphandler.h"
#include "testutil.h"
#include "rtpsessionconfig.h"
#include "rtpsessionclient.h"

RTPPacketRecvInfo MakeRTPPacketRecvInfo(uint32_t ssrc, uint16_t sequence, uint32_t length, uint32_t timestamp, uint32_t mariTImestamp = -1,
                                        uint32_t timeOffset = 0, uint32_t clockRate = 0, bool bFec = false, bool bFecRecovered = false);

class MediaStatisticsTest : public testing::Test
{
public:
    MediaStatisticsTest() {}

protected:
    //will be called before running every case
    virtual void SetUp()
    {
        WRTPSessionParams sessionParams;
        sessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
        sessionParams.enableRTCP = false;
        m_session =  dynamic_cast<CRTPSessionClient *>(WRTPCreateRTPSessionClient(sessionParams));
        ASSERT_TRUE(m_session != nullptr);
        m_session->IncreaseReference();
    }
    //will be called after running every case
    virtual void TearDown()
    {
        m_session->DecreaseReference();
    }

    //will be called before running the first case
    static void SetUpTestCase()
    {
    }

    //will be called after running the last case
    static void TearDownTestCase()
    {
    }

    CRTPSessionClient *m_session;
};

class CRTCPHandlerSinkMockMediaStats : public IRTCPHandlerSink
{
public:
    CRTCPHandlerSinkMockMediaStats()
    {
        m_rtcpPacket = nullptr;
    }

    ~CRTCPHandlerSinkMockMediaStats()
    {
        if (m_rtcpPacket) {
            m_rtcpPacket->DestroyChained();
            m_rtcpPacket = nullptr;
        }
    }

    virtual int32_t OnSendRTCPPacket(RTP_IN CCmMessageBlock &rtcpPacket)
    {
        m_rtcpPacket = rtcpPacket.DuplicateChained();
        return 0;
    }

    virtual int32_t OnPictureLossIndication(uint32_t ssrc, bool instant)
    {
        return 0;
    }

    // functions inherited from IRTPStreamStateObserver
    virtual int32_t OnTimeSync(uint32_t ssrc, const CNTPTime &ntpTimestamp, uint32_t rtpTimestamp)
    {
        return 0;
    }

    virtual void OnRTCPStreamLeaveIndication(uint32_t ssrc, eStreamDirection dir) {}

    virtual void OnReceivedSequenceIndication(uint32_t ssrc, uint32_t xseq) {};
    virtual void OnLossRatioIndication(float lossRatio) {};
    virtual CRTPStreamPtr GetStreamBySSRC(uint32_t ssrc, eStreamDirection dir)
    {
        CRTPStreamPtr tempPtr;
        return tempPtr;
    }
    virtual uint32_t GetNalNumToListenChannel()
    {
        return 0;
    }
    virtual void OnNetworkControlledInformation(AdjustmentState state, const AggregateMetric &netMetric, uint32_t bandwidth,uint32_t totalBandwidth) {}
    virtual void OnRTCPBandwidthUpdated(uint32_t bandwidth) { };
    virtual void OnStreamBandwidthUpdated(uint32_t ssrc, uint32_t bandwidth) {};

    virtual bool GetInboundFECEnabled() {return false;}
    virtual bool GetOutboundFECEnabled() {return false;}

    virtual void NotifyMariRecvStats(const NetworkMetric &networkmetric) {}
    virtual void NotifyMariSendStats(const NetworkMetricEx &networkmetric) {}

    virtual void OnRTCPByeReceived(uint32_t ssrc) {}
public:
    CCmMessageBlock *m_rtcpPacket;
};

class CRTCPHandlerFoMediaStatsTest : public CRTCPHandler
{
public:
    CRTCPHandlerFoMediaStatsTest() : CRTCPHandler(RTPSessionContextSharedPtr(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO))) {}
    CRTCPHandlerFoMediaStatsTest(const RTPSessionContextSharedPtr &sessionConfig) : CRTCPHandler(sessionConfig) {}
    ~CRTCPHandlerFoMediaStatsTest() {}

    uint32_t GetSSRC()
    {
        return m_ssrc;
    }

    string GetCName()
    {
        return m_cname;
    }

    void TrySendRTCPReport()
    {
        CRTCPHandler::TrySendRTCPReport();
    }
};

TEST_F(MediaStatisticsTest, Test_MediaStatistics_Common_Test_Small_Loss_Gap)
{
    CScopedTracer test_info;
    
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc = 0x11223344;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMockMediaStats sink;
    std::unique_ptr<CRTCPHandlerFoMediaStatsTest> handler(new CRTCPHandlerFoMediaStatsTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9527, 1022, 0x22334455);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9530, 1020, 0x22335456);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9529, 1024, 0x22336457);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9528, 1024, 0x22337458);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9531, 1024, 0x22338459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9538, 1024, 0x22339459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9539, 1024, 0x22340459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9541, 1024, 0x22331459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9542, 1024, 0x22332459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9549, 1024, 0x22333459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9550, 1024, 0x22334459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9551, 1024, 0x22335459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->TrySendRTCPReport(); // refresh the session statistics

    SessStats sta;
    handler->GetSessionStats(sta);
    EXPECT_EQ(0.52f, sta.inStats.netStats.lossRatio);
    EXPECT_EQ(0.52f, sta.inStats.netStats.fecResidualLoss);
    EXPECT_EQ(12282, sta.inStats.netStats.recvBytes);
    EXPECT_EQ(12, sta.inStats.netStats.recvPackets);
    EXPECT_EQ(98256, sta.inStats.netStats.recvBitRate);

    EXPECT_EQ(2, sta.inStats.netStats.outOfOrderCount);
    EXPECT_EQ(13, sta.inStats.netStats.lostCount);
    EXPECT_EQ(2, sta.inStats.netStats.errorCount);
}

TEST_F(MediaStatisticsTest, Test_MediaStatistics_Common_Test_Big_Loss_Gap)
{
    CScopedTracer test_info;
    
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc = 0x11223344;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMockMediaStats sink;
    std::unique_ptr<CRTCPHandlerFoMediaStatsTest> handler(new CRTCPHandlerFoMediaStatsTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 0, 1024, 0x22334455);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 8, 1024, 0x22335456);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9, 1024, 0x22336457);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 10, 1024, 0x22337458);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 20, 1024, 0x22338459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 30, 1024, 0x22339459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 40, 1024, 0x22340459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 50, 1024, 0x22331459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 60, 1024, 0x22332459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 70, 1024, 0x22333459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 80, 1024, 0x22334459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 10001, 1024, 0x22335459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->TrySendRTCPReport(); // refresh the session statistics

    SessStats sta;
    handler->GetSessionStats(sta);
    EXPECT_EQ(12288, sta.inStats.netStats.recvBytes);
    EXPECT_EQ(12, sta.inStats.netStats.recvPackets);
    EXPECT_EQ(98304, sta.inStats.netStats.recvBitRate);

    EXPECT_EQ(0, sta.inStats.netStats.outOfOrderCount);
    EXPECT_EQ(9990, sta.inStats.netStats.lostCount);
    EXPECT_EQ(0, sta.inStats.netStats.errorCount);
}

TEST_F(MediaStatisticsTest, Test_MediaStatistics_Common_Test_Big_OOO_Gap)
{
    CScopedTracer test_info;
    
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc = 0x11223344;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMockMediaStats sink;
    std::unique_ptr<CRTCPHandlerFoMediaStatsTest> handler(new CRTCPHandlerFoMediaStatsTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 0, 1024, 0x22334455);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 1, 1024, 0x22335456);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 5, 1024, 0x22336457);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 2, 1024, 0x22337458);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 3, 1024, 0x22338459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 4, 1024, 0x22339459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 8, 1024, 0x22340459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 8, 1024, 0x22331459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 8, 1024, 0x22332459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 80, 1024, 0x22333459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 7999, 1024, 0x22334459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 80, 1024, 0x22335459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->TrySendRTCPReport(); // refresh the session statistics

    SessStats sta;
    handler->GetSessionStats(sta);
    EXPECT_EQ(12288, sta.inStats.netStats.recvBytes);
    EXPECT_EQ(12, sta.inStats.netStats.recvPackets);
    EXPECT_EQ(98304, sta.inStats.netStats.recvBitRate);

    //This is a abnormal case, duplicateCount logic can't count the last seq 80 packet, because the diff is out of window
    EXPECT_EQ(2, sta.inStats.netStats.duplicateCount);
    EXPECT_EQ(4, sta.inStats.netStats.outOfOrderCount);
    EXPECT_EQ(7988, sta.inStats.netStats.lostCount);
    EXPECT_EQ(6, sta.inStats.netStats.errorCount);
}

TEST_F(MediaStatisticsTest, Test_MediaStatistics_Common_Test_Small_OOO_Gap)
{
    CScopedTracer test_info;
    
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc = 0x11223344;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMockMediaStats sink;
    std::unique_ptr<CRTCPHandlerFoMediaStatsTest> handler(new CRTCPHandlerFoMediaStatsTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 0, 1024, 0x22334455);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 200, 1024, 0x22335456);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 100, 1024, 0x22336457);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 150, 1024, 0x22337458);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 200, 1024, 0x22338459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 1, 1024, 0x22339459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 3, 1024, 0x22340459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 2, 1024, 0x22331459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 4, 1024, 0x22332459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 80, 1024, 0x22333459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 70, 1024, 0x22334459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 80, 1024, 0x22335459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->TrySendRTCPReport(); // refresh the session statistics

    SessStats sta;
    handler->GetSessionStats(sta);
    EXPECT_EQ(12288, sta.inStats.netStats.recvBytes);
    EXPECT_EQ(12, sta.inStats.netStats.recvPackets);
    EXPECT_EQ(98304, sta.inStats.netStats.recvBitRate);

    EXPECT_EQ(2, sta.inStats.netStats.duplicateCount);
    EXPECT_EQ(9, sta.inStats.netStats.outOfOrderCount);
    EXPECT_EQ(189, sta.inStats.netStats.lostCount);
    EXPECT_EQ(11, sta.inStats.netStats.errorCount);
}

TEST_F(MediaStatisticsTest, Test_MediaStatistics_Common_Test_Multi_Streams)
{
    CScopedTracer test_info;
    
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc = 0x11223344;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    uint32_t ssrc2 = 0x11223355;
    CRTPStreamPtr streamPtr2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMockMediaStats sink;
    std::unique_ptr<CRTCPHandlerFoMediaStatsTest> handler(new CRTCPHandlerFoMediaStatsTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 0, 1024, 0x22334455);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 200, 1024, 0x22335456);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 100, 1024, 0x22336457);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 150, 1024, 0x22337458);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 200, 1024, 0x22338459);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 1, 1024, 0x22339459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 3, 1024, 0x22340459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 2, 1024, 0x22331459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 4, 1024, 0x22332459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 80, 1024, 0x22333459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 70, 1024, 0x22334459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 80, 1024, 0x22335459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->TrySendRTCPReport(); // refresh the session statistics

    SessStats sta;
    handler->GetSessionStats(sta);
    EXPECT_EQ(12288, sta.inStats.netStats.recvBytes);
    EXPECT_EQ(12, sta.inStats.netStats.recvPackets);
    EXPECT_EQ(98304, sta.inStats.netStats.recvBitRate);

    EXPECT_EQ(2, sta.inStats.netStats.duplicateCount);
    EXPECT_EQ(4, sta.inStats.netStats.outOfOrderCount);
    EXPECT_EQ(269, sta.inStats.netStats.lostCount);
    EXPECT_EQ(2, sta.inStats.netStats.duplicateCount);
    EXPECT_EQ(6, sta.inStats.netStats.errorCount);

    handler->OnStreamDestroyed(0x11223355, STREAM_OUT);
    handler->TrySendRTCPReport(); // refresh the session statistics

    handler->GetSessionStats(sta);
    EXPECT_EQ(12288, sta.inStats.netStats.recvBytes);
    EXPECT_EQ(12, sta.inStats.netStats.recvPackets);
    EXPECT_EQ(98304, sta.inStats.netStats.recvBitRate);

    EXPECT_EQ(2, sta.inStats.netStats.duplicateCount);
    EXPECT_EQ(4, sta.inStats.netStats.outOfOrderCount);
    EXPECT_EQ(269, sta.inStats.netStats.lostCount);
    EXPECT_EQ(2, sta.inStats.netStats.duplicateCount);
    EXPECT_EQ(6, sta.inStats.netStats.errorCount);

}


TEST_F(MediaStatisticsTest, Test_MediaStatistics_Common_Test_FEC_Recover)
{
    CScopedTracer test_info;
    
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc = 0x11223344;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    uint32_t ssrc2 = 0x11223355;
    CRTPStreamPtr streamPtr2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    const RTPSessionContextSharedPtr &sessionConfig = m_session->GetSessionContext();
    CRTCPHandlerSinkMockMediaStats sink;
    std::unique_ptr<CRTCPHandlerFoMediaStatsTest> handler(new CRTCPHandlerFoMediaStatsTest(sessionConfig));
    handler->SetSink(&sink);
    CStreamMonitorSharedPtr streamMonitor = sessionConfig->GetStreamMonitor();

    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 0, 1024, 0x22334455);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    
    ++streamPtr->GetRecvStats().m_fecRecover;
    streamMonitor->NotifyRTPPacketRecovered(MakeRTPPacketRecvInfo(0x11223344, 1, 1024, 0x22334455, -1, 0, 0, false, true));
    
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 2, 1024, 0x22335456);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    
    ++streamPtr->GetRecvStats().m_fecRecover;
    streamMonitor->NotifyRTPPacketRecovered(MakeRTPPacketRecvInfo(0x11223344, 3, 1024, 0x22334455, -1, 0, 0, false, true));
    
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 4, 1024, 0x22336457);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    
    ++streamPtr->GetRecvStats().m_fecRecover;
    streamMonitor->NotifyRTPPacketRecovered(MakeRTPPacketRecvInfo(0x11223344, 5, 1024, 0x22334455, -1, 0, 0, false, true));
    
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 7, 1024, 0x22337458);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 1, 1024, 0x22339459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    
    ++streamPtr2->GetRecvStats().m_fecRecover;
    streamMonitor->NotifyRTPPacketRecovered(MakeRTPPacketRecvInfo(0x11223355, 2, 1024, 0x22334455, -1, 0, 0, false, true));
    
    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 3, 1024, 0x22340459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 5, 1024, 0x22331459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 8, 1024, 0x22332459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->TrySendRTCPReport(); // refresh the session statistics

    SessStats sessSta;
    handler->GetSessionStats(sessSta);
    EXPECT_EQ(4, sessSta.inStats.netStats.fecRecoveredPackets);
    EXPECT_EQ(0.5f, sessSta.inStats.netStats.lossRatio);
    EXPECT_EQ(0.25f, sessSta.inStats.netStats.fecResidualLoss);

    StreamInStats streamSta;
    streamMonitor->GetRemoteStats(0x11223344, streamSta, handler->GetSessionContext());
    EXPECT_EQ(3, streamSta.netStats.fecRecoveredPackets);
    EXPECT_EQ(0.5f, streamSta.netStats.lossRatio);
    EXPECT_EQ(0.125f, streamSta.netStats.fecResidualLoss);

    streamMonitor->GetRemoteStats(0x11223355, streamSta, handler->GetSessionContext());
    EXPECT_EQ(1, streamSta.netStats.fecRecoveredPackets);
    EXPECT_EQ(0.5f, streamSta.netStats.lossRatio);
    EXPECT_EQ(0.375f, streamSta.netStats.fecResidualLoss);

}

TEST_F(MediaStatisticsTest, Test_MediaStatistics_Common_Test_Drop_Count)
{
    CScopedTracer test_info;
    
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc = 0x11223344;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    uint32_t ssrc2 = 0x11223355;
    CRTPStreamPtr streamPtr2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    const RTPSessionContextSharedPtr &sessionConfig = m_session->GetSessionContext();
    CRTCPHandlerSinkMockMediaStats sink;
    std::unique_ptr<CRTCPHandlerFoMediaStatsTest> handler(new CRTCPHandlerFoMediaStatsTest(sessionConfig));
    handler->SetSink(&sink);
    CStreamMonitorSharedPtr streamMonitor = sessionConfig->GetStreamMonitor();

    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 0, 1024, 0x22334455);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 2, 1024, 0x22335456);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 4, 1024, 0x22336457);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 7, 1024, 0x22337458);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    streamPtr->GetRecvStats().m_dropCount += 2;
    streamMonitor->NotifyDropCount(0x11223344, 2);

    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 1, 1024, 0x22339459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 3, 1024, 0x22340459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    streamPtr2->GetRecvStats().m_dropCount += 1;
    streamMonitor->NotifyDropCount(0x11223355, 1);
    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 5, 1024, 0x22331459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223355, 8, 1024, 0x22332459);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->TrySendRTCPReport(); // refresh the session statistics

    SessStats sessSta;
    handler->GetSessionStats(sessSta);
    EXPECT_EQ(5, sessSta.inStats.netStats.processedPackages);
    EXPECT_EQ(0.5f, sessSta.inStats.netStats.lossRatio);
    EXPECT_EQ(0.375f, sessSta.inStats.netStats.rtpDropRatio);

    StreamInStats streamSta;
    streamMonitor->GetRemoteStats(0x11223344, streamSta, handler->GetSessionContext());
    EXPECT_EQ(2, streamSta.netStats.processedPackages);
    EXPECT_EQ(0.5f, streamSta.netStats.lossRatio);
    EXPECT_EQ(0.5f, streamSta.netStats.rtpDropRatio);

    streamMonitor->GetRemoteStats(0x11223355, streamSta, handler->GetSessionContext());
    EXPECT_EQ(3, streamSta.netStats.processedPackages);
    EXPECT_EQ(0.5f, streamSta.netStats.lossRatio);
    EXPECT_EQ(0.25f, streamSta.netStats.rtpDropRatio);

}

TEST_F(MediaStatisticsTest, Test_MediaStatistics_Common_Test_Lost_Count_When_Sequence_Rollback)
{
    CScopedTracer test_info;
    
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc = 0x11223344;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    const RTPSessionContextSharedPtr &sessionConfig = m_session->GetSessionContext();
    CRTCPHandlerSinkMockMediaStats sink;
    std::unique_ptr<CRTCPHandlerFoMediaStatsTest> handler(new CRTCPHandlerFoMediaStatsTest(sessionConfig));
    handler->SetSink(&sink);
    CStreamMonitorSharedPtr streamMonitor = sessionConfig->GetStreamMonitor();

    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 65532, 1024, 0x22334455);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 65533, 1024, 0x22335456);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 65534, 1024, 0x22336457);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 65535, 1024, 0x22337458);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->TrySendRTCPReport();

    SessStats sessSta;
    handler->GetSessionStats(sessSta);
    EXPECT_EQ(4, sessSta.inStats.netStats.recvPackets);
    EXPECT_EQ(0, sessSta.inStats.netStats.lostCount);

    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 10, 1024, 0x22337468);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->TrySendRTCPReport();

    handler->GetSessionStats(sessSta);
    EXPECT_EQ(5, sessSta.inStats.netStats.recvPackets);
    EXPECT_EQ(10, sessSta.inStats.netStats.lostCount);

}
