#include "gmock/gmock.h"
#include "gtest/gtest.h"
#define private public
#include "rtpsessionclient.h"
#include "rtcphandler.h"
#include "rtpinternal.h"
#include "testutil.h"
#include "rtpsessionconfig.h"

#include "videostream.h"

using namespace wrtp;
using namespace std;

namespace wrtp
{
BOOL DecodeRTCPHeader(CCmByteStreamNetwork &is, RTCPHeader &header);
CRTCPPacket *DecodeRTCPPacket(CCmByteStreamNetwork &is, CCmMessageBlock &mb);
CXRBlock *DecodeXRBlock(CCmByteStreamNetwork &is, CCmMessageBlock &mb);
};

RTPPacketSendInfo MakeRTPPacketSentInfo(uint32_t ssrc, uint16_t sequence, uint32_t length, uint32_t rtpTimestamp,bool bFec = false)
{
    RTPPacketSendInfo info;
    info.ssrc = (ssrc);
    info.sequence = (sequence);
    info.packetLength = (length);
    info.rtpTimestamp = (rtpTimestamp);
    info.fecPacket = (bFec);

    return info;
}

RTPPacketRecvInfo MakeRTPPacketRecvInfo(uint32_t ssrc, uint16_t sequence, uint32_t length, uint32_t timestamp, uint32_t mariTimestamp = -1, uint32_t timeOffset = 0, uint32_t clockRate = 0, bool bFec = false, bool bFecRecovered = false)
{
    RTPPacketRecvInfo info;
    info.ssrc = ssrc;
    info.sequence = sequence;
    info.length = length;
    info.timestamp = timestamp;
    info.mariTimestamp = mariTimestamp;
    info.timeOffset = timeOffset;
    info.clockRate = clockRate;
    info.fecPacket = bFec;
    info.fecRecovered = bFecRecovered;

    return info;
}

class CRTCPHandlerTEST : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        WRTPSessionParams sessionParams;
        sessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
        sessionParams.enableRTCP = false;
        m_session =  dynamic_cast<CRTPSessionClient *>(WRTPCreateRTPSessionClient(sessionParams));
        ASSERT_TRUE(m_session != nullptr);
        m_session->IncreaseReference();
    }

    virtual void TearDown()
    {
        m_session->DecreaseReference();
    }

    CRTPSessionClient *m_session;
};

class CRTCPHandlerSinkMock : public IRTCPHandlerSink
{
public:
    CRTCPHandlerSinkMock()
    {
        m_rtcpPacket = nullptr;
    }

    ~CRTCPHandlerSinkMock()
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
    virtual void OnRTCPBandwidthUpdated(uint32_t bandwidth) {}
    virtual void NotifyStreamBandwidth(const StreamIdInfo &streamId, uint32_t bandwidthInBytePerSec) {}
    virtual void OnStreamBandwidthUpdated(uint32_t ssrc, uint32_t bandwidth) {}

    virtual bool GetInboundFECEnabled() {return false;}
    virtual bool GetOutboundFECEnabled() {return  false;}

    virtual void NotifyMariRecvStats(const NetworkMetric &networkmetric) {}
    virtual void NotifyMariSendStats(const NetworkMetricEx &networkmetric) {}

    virtual void OnRTCPByeReceived(uint32_t ssrc) {}
public:
    CCmMessageBlock *m_rtcpPacket;
};

class CRTCPHandlerForTest : public CRTCPHandler
{
public:
    CRTCPHandlerForTest(const RTPSessionContextSharedPtr &ctx) : CRTCPHandler(ctx)
    {
        m_rtcpReportFlag = FALSE;
    }
    ~CRTCPHandlerForTest() {}

    virtual void TrySendRTCPReport()
    {
        if (m_rtcpReportFlag) {
            return CRTCPHandler::TrySendRTCPReport();
        }
    }

    uint32_t GetSSRC()
    {
        return m_ssrc;
    }

    string GetCName()
    {
        return m_cname;
    }

    BOOL m_rtcpReportFlag;
};

//mock IQoSFeedbackGenerator
class IQoSFeedbackGeneratorMock : public IQoSFeedbackGenerator
{
public:
    IQoSFeedbackGeneratorMock()
    {
        networkmetricSample.qdelay = 10;
        networkmetricSample.receiveRate = 2000;
    }

    virtual FeedbackResult OnDataReceived(uint32_t now,
                                          uint32_t sourceId,
                                          uint16_t sequence,
                                          uint32_t length,
                                          uint32_t timestampTx,
                                          uint32_t timestampRx)
    {
        return FEEDBACK_ACTION;
    }

    virtual int32_t GetNetworkMetricFeedback(uint32_t now, PARAM_OUT NetworkMetric &networkMetric)
    {
        networkMetric = networkmetricSample;
        return 0;
    }

    virtual void SetLogPrefix(const std::string &) {}

    virtual ~IQoSFeedbackGeneratorMock() {}
public :
    NetworkMetric networkmetricSample;
};

class CQoSHandlerMock : public IQoSHandler
{
public:
    CQoSHandlerMock() : m_serviceId(1), m_feedbackGenerator(new IQoSFeedbackGeneratorMock())
    {
    }

    ~CQoSHandlerMock()
    {
        delete m_feedbackGenerator;
    }


    virtual bool IsInitialized()
    {
        return true;
    }
    virtual string GetDescStr()
    {
        return "Test";
    }
    virtual uint32_t GetServiceId()
    {
        return m_serviceId;
    }

    virtual void NotifyRTPPacketSend(const CRTPPacketSentInfo &info,bool sendFailed, bool bPadding) { }
    virtual FeedbackResult OnDataReceived(uint32_t now, uint32_t sourceId, uint16_t sequence, uint32_t length, uint32_t timestamp)
    {
        return m_feedbackGenerator->OnDataReceived(now, sourceId, sequence, length, timestamp, now);
    }
    virtual int32_t GetNetworkMetricFeedback(uint32_t now, PARAM_OUT NetworkMetric &networkMetric)
    {
        return m_feedbackGenerator->GetNetworkMetricFeedback(now, networkMetric);
    }

    virtual int32_t FeedbackNetworkMetrics(NetworkMetricEx &netMetric)
    {
        return WRTP_ERR_NOERR;
    }

    virtual int32_t SetAdjustmentPolicy(const AdjustmentPolicy &policy)
    {
        return WRTP_ERR_NOERR;
    }

private:
    uint32_t                  m_serviceId;
    IQoSFeedbackGenerator  *m_feedbackGenerator;

};

void CheckSDESPacketInRTCPReport(uint32_t ssrc, string cname, CSDESPacket *packet)
{
    EXPECT_EQ(ssrc, packet->sdesTrunks[0].ssrc);

    EXPECT_EQ((uint8_t)SDES_TYPE_CNAME, packet->sdesTrunks[0].sdesItems[0].type);
    EXPECT_EQ(cname.length(), packet->sdesTrunks[0].sdesItems[0].length);

    string content = string((char *)packet->sdesTrunks[0].sdesItems[0].content);
    EXPECT_STREQ(cname.c_str(), content.c_str());
}

TEST_F(CRTCPHandlerTEST, Test_NotifyRTPPacketSent_then_send_single_SR_without_RRBlock)
{
    CScopedTracer test_info;
    
    uint32_t ssrc = 0x11223344;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    
    CRTCPHandlerSinkMock sink;
    std::unique_ptr<CRTCPHandlerForTest> handler(new CRTCPHandlerForTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    streamPtr->GetSendStats().NotifyRTPPacketSent(MakeRTPPacketSentInfo(ssrc, 0x9527, 1022, 0x22334455), TickNowMS(), false);
    handler->NotifyRTPPacketSend(MakeRTPPacketSentInfo(ssrc, 0x9527, 1022, 0x22334455), TickNowMS());
    handler->m_rtcpReportFlag = TRUE;
    streamPtr->GetSendStats().NotifyRTPPacketSent(MakeRTPPacketSentInfo(ssrc, 0x9530, 1022, 0x22334455), TickNowMS(), false);
    handler->NotifyRTPPacketSend(MakeRTPPacketSentInfo(ssrc, 0x9530, 1022, 0x22334455), TickNowMS());

    CCmByteStreamNetwork is(*(sink.m_rtcpPacket));
    CSRPacket *srpacket = (CSRPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));

    EXPECT_TRUE(srpacket != nullptr);
    if (srpacket) {
        EXPECT_EQ(ssrc, srpacket->ssrc);
        EXPECT_EQ(2, srpacket->srInfo.count);
        EXPECT_EQ(2044, srpacket->srInfo.bytes);
        srpacket->ReleaseReference();
    }

    CSDESPacket *sdespacket = (CSDESPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(sdespacket != nullptr);
    if (sdespacket) {
        CheckSDESPacketInRTCPReport(handler->GetSSRC(), handler->GetCName(), sdespacket);
        sdespacket->ReleaseReference();
    }

    CRTCPPacket *packet = (CRTCPPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(packet == nullptr);
    if (packet) {
        packet->ReleaseReference();
    }
}

TEST_F(CRTCPHandlerTEST, Test_NotifyRTPPacketSent_then_send_compound_SR_without_RRBlock)
{
    CScopedTracer test_info;
    
    uint32_t ssrc1 = 0x11223344;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    uint32_t ssrc2 = 0x11223355;
    CRTPStreamPtr streamPtr2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    
    CRTCPHandlerSinkMock sink;
    std::unique_ptr<CRTCPHandlerForTest> handler(new CRTCPHandlerForTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    streamPtr->GetSendStats().NotifyRTPPacketSent(MakeRTPPacketSentInfo(ssrc1, 0x9527, 1022, 0x22334455), TickNowMS(), false);
    handler->NotifyRTPPacketSend(MakeRTPPacketSentInfo(ssrc1, 0x9527, 1022, 0x22334455), TickNowMS());
    streamPtr2->GetSendStats().NotifyRTPPacketSent(MakeRTPPacketSentInfo(ssrc2, 0x9530, 1033, 0x22334455), TickNowMS(), false);
    handler->NotifyRTPPacketSend(MakeRTPPacketSentInfo(ssrc2, 0x9530, 1033, 0x22334455), TickNowMS());
    handler->m_rtcpReportFlag = TRUE;
    streamPtr2->GetSendStats().NotifyRTPPacketSent(MakeRTPPacketSentInfo(ssrc2, 0x9535, 1044, 0x22334455), TickNowMS(), false);
    handler->NotifyRTPPacketSend(MakeRTPPacketSentInfo(ssrc2, 0x9535, 1044, 0x22334455), TickNowMS());

    CCmByteStreamNetwork is(*(sink.m_rtcpPacket));
    CSRPacket *srpacket = (CSRPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));

    EXPECT_TRUE(srpacket != nullptr);
    if (srpacket) {
        EXPECT_EQ(ssrc1, srpacket->ssrc);
        EXPECT_EQ(1, srpacket->srInfo.count);
        EXPECT_EQ(1022, srpacket->srInfo.bytes);
        srpacket->ReleaseReference();
    }

    srpacket = (CSRPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(srpacket != nullptr);
    if (srpacket) {
        EXPECT_EQ(ssrc2, srpacket->ssrc);
        EXPECT_EQ(2, srpacket->srInfo.count);
        EXPECT_EQ(2077, srpacket->srInfo.bytes);
        srpacket->ReleaseReference();
    }

    CSDESPacket *sdespacket = (CSDESPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(sdespacket != nullptr);
    if (sdespacket) {
        CheckSDESPacketInRTCPReport(handler->GetSSRC(), handler->GetCName(), sdespacket);
        sdespacket->ReleaseReference();
    }

    CRTCPPacket *packet = (CRTCPPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(packet == nullptr);
}

TEST_F(CRTCPHandlerTEST, Test_NotifyRTPPacketSent_then_send_single_RR_without_packet_lost)
{
    CScopedTracer test_info;
    
    uint32_t ssrc = 0x11223344;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMock sink;
    std::unique_ptr<CRTCPHandlerForTest> handler(new CRTCPHandlerForTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    RTPPacketRecvInfo info = MakeRTPPacketRecvInfo(ssrc, 9527, 1022, 0x22334455, -1, 0, 0, false);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(info, TickNowMS());
    handler->NotifyRTPPacketRecv(info, TickNowMS());
    info = MakeRTPPacketRecvInfo(ssrc, 9530, 1020, 0x22335456, -1, 0, 0, false);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(info, TickNowMS());
    handler->NotifyRTPPacketRecv(info, TickNowMS());
    info = MakeRTPPacketRecvInfo(ssrc, 9529, 1024, 0x22336457, -1, 0, 0, false);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(info, TickNowMS());
    handler->NotifyRTPPacketRecv(info, TickNowMS());
    info = MakeRTPPacketRecvInfo(ssrc, 9528, 1024, 0x22337458, -1, 0, 0, false);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(info, TickNowMS());
    handler->NotifyRTPPacketRecv(info, TickNowMS());
    handler->m_rtcpReportFlag = TRUE;
    info = MakeRTPPacketRecvInfo(ssrc, 9531, 1024, 0x22338459, -1, 0, 0, false);
    streamPtr->GetRecvStats().NotifyRTPPacketRecv(info, TickNowMS());
    handler->NotifyRTPPacketRecv(info, TickNowMS());

    handler->TrySendRTCPReport();

    CCmByteStreamNetwork is(*(sink.m_rtcpPacket));
    CRRPacket *rrpacket = (CRRPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));

    EXPECT_TRUE(rrpacket != nullptr);
    if (rrpacket) {
        EXPECT_EQ(handler->GetSSRC(), rrpacket->ssrc);

        EXPECT_EQ(ssrc, rrpacket->rrBlocks[0].ssrc);
        EXPECT_EQ(0, rrpacket->rrBlocks[0].flost);
        EXPECT_EQ(0, rrpacket->rrBlocks[0].plost);
        EXPECT_EQ(9531, rrpacket->rrBlocks[0].xseq);
        EXPECT_EQ(0, rrpacket->rrBlocks[0].jitter);
        EXPECT_EQ(0, rrpacket->rrBlocks[0].lsr);
        EXPECT_EQ(0, rrpacket->rrBlocks[0].dlsr);
        rrpacket->ReleaseReference();
    }

    CSDESPacket *sdespacket = (CSDESPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(sdespacket != nullptr);
    if (sdespacket) {
        CheckSDESPacketInRTCPReport(handler->GetSSRC(), handler->GetCName(), sdespacket);
        sdespacket->ReleaseReference();
    }

    CRTCPPacket *packet = (CRTCPPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(packet == nullptr);
}


TEST_F(CRTCPHandlerTEST, Test_NotifyRTPPacketSent_then_send_single_SR_single_RR)
{
    CScopedTracer test_info;
    
    uint32_t ssrc = 0x11223344;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr streamPtr2 = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMock sink;
    std::unique_ptr<CRTCPHandlerForTest> handler(new CRTCPHandlerForTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    RTPPacketSendInfo info = MakeRTPPacketSentInfo(ssrc, 9530, 1020, 0x22334455);
    streamPtr->GetSendStats().NotifyRTPPacketSent(info, TickNowMS(), false);
    handler->NotifyRTPPacketSend(info, TickNowMS());
    handler->m_rtcpReportFlag = TRUE;
    RTPPacketRecvInfo info2 = MakeRTPPacketRecvInfo(ssrc, 9531, 1024, 0x22334456, -1, 0, 0, false);
    streamPtr2->GetRecvStats().NotifyRTPPacketRecv(info2, TickNowMS());
    handler->NotifyRTPPacketRecv(info2, TickNowMS());

    handler->TrySendRTCPReport();
    CCmByteStreamNetwork is(*(sink.m_rtcpPacket));
    CSRPacket *srpacket = (CSRPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));

    EXPECT_TRUE(srpacket != nullptr);
    if (srpacket) {
        EXPECT_EQ(ssrc, srpacket->ssrc);
        EXPECT_EQ(1, srpacket->srInfo.count);
        EXPECT_EQ(1020, srpacket->srInfo.bytes);

        EXPECT_FALSE(srpacket->rrBlocks.empty());
        if (!srpacket->rrBlocks.empty()) {
            EXPECT_EQ(ssrc, srpacket->rrBlocks[0].ssrc);
            EXPECT_EQ(0, srpacket->rrBlocks[0].flost);
            EXPECT_EQ(0, srpacket->rrBlocks[0].plost);
            EXPECT_EQ(0, srpacket->rrBlocks[0].jitter);
            EXPECT_EQ(9531, srpacket->rrBlocks[0].xseq);
        }

        srpacket->ReleaseReference();
    }

    CSDESPacket *sdespacket = (CSDESPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(sdespacket != nullptr);
    if (sdespacket) {
        CheckSDESPacketInRTCPReport(handler->GetSSRC(), handler->GetCName(), sdespacket);
        sdespacket->ReleaseReference();
    }

    CRTCPPacket *packet = (CRTCPPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(packet == nullptr);
}

TEST_F(CRTCPHandlerTEST, Test_NotifyRTPPacketSent_then_send_single_SR_compound_RR)
{
    CScopedTracer test_info;
    
    RTPPacketSendInfo sendInfo;
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc1 = 0x11223344;
    CRTPStreamPtr sendStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    uint32_t ssrc2 = 0x11223345;
    CRTPStreamPtr sendStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMock sink;
    std::unique_ptr<CRTCPHandlerForTest> handler(new CRTCPHandlerForTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    sendInfo = MakeRTPPacketSentInfo(0x11223344, 9530, 1020, 0x22334455);
    sendStream1->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler->NotifyRTPPacketSend(sendInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9531, 1024, 0x22334456, -1, 0, 0, false);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9532, 1024, 0x22334457, -1, 0, 0, false);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 9531, 1024, 0x22334456, -1, 0, 0, false);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->m_rtcpReportFlag = TRUE;
    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 9532, 1024, 0x22334457, -1, 0, 0, false);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->TrySendRTCPReport();
    CCmByteStreamNetwork is(*(sink.m_rtcpPacket));
    CSRPacket *srpacket = (CSRPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));

    EXPECT_TRUE(srpacket != nullptr);
    if (srpacket) {
        EXPECT_EQ(0x11223344, srpacket->ssrc);
        EXPECT_EQ(1, srpacket->srInfo.count);
        EXPECT_EQ(1020, srpacket->srInfo.bytes);

        EXPECT_EQ(0x11223344, srpacket->rrBlocks[0].ssrc);
        EXPECT_EQ(0, srpacket->rrBlocks[0].flost);
        EXPECT_EQ(0, srpacket->rrBlocks[0].plost);
        EXPECT_EQ(0, srpacket->rrBlocks[0].jitter);
        EXPECT_EQ(9532, srpacket->rrBlocks[0].xseq);

        EXPECT_EQ(0x11223345, srpacket->rrBlocks[1].ssrc);
        EXPECT_EQ(0, srpacket->rrBlocks[1].flost);
        EXPECT_EQ(0, srpacket->rrBlocks[1].plost);
        EXPECT_EQ(0, srpacket->rrBlocks[1].jitter);
        EXPECT_EQ(9532, srpacket->rrBlocks[1].xseq);

        srpacket->ReleaseReference();
    }


    CSDESPacket *sdespacket = (CSDESPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(sdespacket != nullptr);
    if (sdespacket) {
        CheckSDESPacketInRTCPReport(handler->GetSSRC(), handler->GetCName(), sdespacket);
        sdespacket->ReleaseReference();
    }

    CRTCPPacket *packet = (CRTCPPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(packet == nullptr);
}

TEST_F(CRTCPHandlerTEST, Test_NotifyRTPPacketSent_then_compound_SR_compound_RR)
{
    CScopedTracer test_info;
    
    uint32_t ssrc1 = 0x11223344;
    CRTPStreamPtr sendStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    uint32_t ssrc2 = 0x11223345;
    CRTPStreamPtr sendStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMock sink;
    std::unique_ptr<CRTCPHandlerForTest> handler(new CRTCPHandlerForTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    RTPPacketSendInfo sendInfo = MakeRTPPacketSentInfo(0x11223344, 9530, 1020, 0x22334455);
    sendStream1->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler->NotifyRTPPacketSend(sendInfo, TickNowMS());
    sendInfo = MakeRTPPacketSentInfo(0x11223344, 9531, 1020, 0x22334455);
    sendStream1->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler->NotifyRTPPacketSend(sendInfo, TickNowMS());

    sendInfo = MakeRTPPacketSentInfo(0x11223345, 9530, 1020, 0x22334455);
    sendStream2->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler->NotifyRTPPacketSend(sendInfo, TickNowMS());
    sendInfo = MakeRTPPacketSentInfo(0x11223345, 9531, 1020, 0x22334455);
    sendStream2->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler->NotifyRTPPacketSend(sendInfo, TickNowMS());

    RTPPacketRecvInfo recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9531, 1024, 0x22334456, -1, 0, 0, false);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9532, 1024, 0x22334457, -1, 0, 0, false);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 9531, 1024, 0x22334456, -1, 0, 0, false);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->m_rtcpReportFlag = TRUE;
    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 9532, 1024, 0x22334457, -1, 0, 0, false);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->TrySendRTCPReport();
    CCmByteStreamNetwork is(*(sink.m_rtcpPacket));
    CRTCPPacket *packet = DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    ASSERT_TRUE(packet != nullptr);
    ASSERT_EQ(RTCP_PT_SR, packet->GetType());
    CSRPacket *srpacket = (CSRPacket *)packet;
    if (srpacket) {
        EXPECT_EQ(0x11223344, srpacket->ssrc);
        EXPECT_EQ(2, srpacket->srInfo.count);
        EXPECT_EQ(2040, srpacket->srInfo.bytes);

        EXPECT_EQ(0x11223344, srpacket->rrBlocks[0].ssrc);
        EXPECT_EQ(0, srpacket->rrBlocks[0].flost);
        EXPECT_EQ(0, srpacket->rrBlocks[0].plost);
        EXPECT_EQ(0, srpacket->rrBlocks[0].jitter);
        EXPECT_EQ(9532, srpacket->rrBlocks[0].xseq);

        EXPECT_EQ(0x11223345, srpacket->rrBlocks[1].ssrc);
        EXPECT_EQ(0, srpacket->rrBlocks[1].flost);
        EXPECT_EQ(0, srpacket->rrBlocks[1].plost);
        EXPECT_EQ(0, srpacket->rrBlocks[1].jitter);
        EXPECT_EQ(9532, srpacket->rrBlocks[1].xseq);

        srpacket->ReleaseReference();
    }

    packet = DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    ASSERT_TRUE(packet != nullptr);
    ASSERT_EQ(RTCP_PT_SR, packet->GetType());
    srpacket = (CSRPacket *)packet;
    if (srpacket) {
        EXPECT_EQ(0x11223345, srpacket->ssrc);
        EXPECT_EQ(2, srpacket->srInfo.count);
        EXPECT_EQ(2040, srpacket->srInfo.bytes);

        srpacket->ReleaseReference();
    }

    packet = DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    ASSERT_TRUE(packet != nullptr);
    ASSERT_EQ(RTCP_PT_SDES, packet->GetType());
    CSDESPacket *sdespacket = (CSDESPacket *)packet;
    if (sdespacket) {
        CheckSDESPacketInRTCPReport(handler->GetSSRC(), handler->GetCName(), sdespacket);
        sdespacket->ReleaseReference();
    }

    packet = (CRTCPPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(packet == nullptr);
}

TEST_F(CRTCPHandlerTEST, Test_NotifyRTPPacketSent_then_compound_SR_single_RR)
{
    CScopedTracer test_info;
    
    RTPPacketSendInfo sendInfo;
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc1 = 0x11223344;
    CRTPStreamPtr sendStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    uint32_t ssrc2 = 0x11223345;
    CRTPStreamPtr sendStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMock sink;
    std::unique_ptr<CRTCPHandlerForTest> handler(new CRTCPHandlerForTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    sendInfo = MakeRTPPacketSentInfo(0x11223344, 9530, 1020, 0x22334455);
    sendStream1->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler->NotifyRTPPacketSend(sendInfo, TickNowMS());
    sendInfo = MakeRTPPacketSentInfo(0x11223344, 9531, 1020, 0x22334455);
    sendStream1->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler->NotifyRTPPacketSend(sendInfo, TickNowMS());

    sendInfo = MakeRTPPacketSentInfo(0x11223345, 9530, 1020, 0x22334455);
    sendStream2->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler->NotifyRTPPacketSend(sendInfo, TickNowMS());
    sendInfo = MakeRTPPacketSentInfo(0x11223345, 9531, 1020, 0x22334455);
    sendStream2->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler->NotifyRTPPacketSend(sendInfo, TickNowMS());

    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9531, 1024, 0x22334456, -1, 0, 0, false);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->m_rtcpReportFlag = TRUE;
    recvInfo =MakeRTPPacketRecvInfo(0x11223344, 9532, 1024, 0x22334457, -1, 0, 0, false);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->TrySendRTCPReport();
    CCmByteStreamNetwork is(*(sink.m_rtcpPacket));
    CRTCPPacket *packet = DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    ASSERT_TRUE(packet != nullptr);
    ASSERT_EQ(RTCP_PT_SR, packet->GetType());
    CSRPacket *srpacket = (CSRPacket *)packet;
    if (srpacket) {
        EXPECT_EQ(0x11223344, srpacket->ssrc);
        EXPECT_EQ(2, srpacket->srInfo.count);
        EXPECT_EQ(2040, srpacket->srInfo.bytes);

        EXPECT_EQ(0x11223344, srpacket->rrBlocks[0].ssrc);
        EXPECT_EQ(0, srpacket->rrBlocks[0].flost);
        EXPECT_EQ(0, srpacket->rrBlocks[0].plost);
        EXPECT_EQ(0, srpacket->rrBlocks[0].jitter);
        EXPECT_EQ(9532, srpacket->rrBlocks[0].xseq);

        srpacket->ReleaseReference();
    }

    packet = DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    ASSERT_TRUE(packet != nullptr);
    ASSERT_EQ(RTCP_PT_SR, packet->GetType());
    srpacket = (CSRPacket *)packet;
    if (srpacket) {
        EXPECT_EQ(0x11223345, srpacket->ssrc);
        EXPECT_EQ(2, srpacket->srInfo.count);
        EXPECT_EQ(2040, srpacket->srInfo.bytes);

        srpacket->ReleaseReference();
    }

    packet = DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    ASSERT_TRUE(packet != nullptr);
    ASSERT_EQ(RTCP_PT_SDES, packet->GetType());
    CSDESPacket *sdespacket = (CSDESPacket *)packet;
    if (sdespacket) {
        CheckSDESPacketInRTCPReport(handler->GetSSRC(), handler->GetCName(), sdespacket);
        sdespacket->ReleaseReference();
    }

    packet = (CRTCPPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(packet == nullptr);
}

TEST_F(CRTCPHandlerTEST, Test_NotifyRTPPacketSent_then_send_compound_RR)
{
    CScopedTracer test_info;
    
    RTPPacketSendInfo sendInfo;
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc1 = 0x11223344;
    CRTPStreamPtr sendStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    uint32_t ssrc2 = 0x11223345;
    CRTPStreamPtr sendStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMock sink;
    std::unique_ptr<CRTCPHandlerForTest> handler(new CRTCPHandlerForTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9528, 1022, 0x22334455, -1, 0, 0, false);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9530, 1020, 0x22335456, -1, 0, 0, false);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9529, 1024, 0x22336457, -1, 0, 0, false);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 9530, 1024, 0x22337458, -1, 0, 0, false);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->m_rtcpReportFlag = TRUE;
    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 9531, 1024, 0x22338459, -1, 0, 0, false);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->TrySendRTCPReport();
    CCmByteStreamNetwork is(*(sink.m_rtcpPacket));
    CRRPacket *rrpacket = (CRRPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));

    EXPECT_TRUE(rrpacket != nullptr);
    if (rrpacket) {
        EXPECT_EQ(handler->GetSSRC(), rrpacket->ssrc);

        EXPECT_EQ(0x11223344, rrpacket->rrBlocks[0].ssrc);
        EXPECT_EQ(0, rrpacket->rrBlocks[0].flost);
        EXPECT_EQ(0, rrpacket->rrBlocks[0].plost);
        EXPECT_EQ(9530, rrpacket->rrBlocks[0].xseq);
        EXPECT_EQ(0, rrpacket->rrBlocks[0].jitter);

        EXPECT_EQ(0x11223345, rrpacket->rrBlocks[1].ssrc);
        EXPECT_EQ(0, rrpacket->rrBlocks[1].flost);
        EXPECT_EQ(0, rrpacket->rrBlocks[1].plost);
        EXPECT_EQ(9531, rrpacket->rrBlocks[1].xseq);
        EXPECT_EQ(0, rrpacket->rrBlocks[1].jitter);

        rrpacket->ReleaseReference();
    }

    CSDESPacket *sdespacket = (CSDESPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(sdespacket != nullptr);
    if (sdespacket) {
        CheckSDESPacketInRTCPReport(handler->GetSSRC(), handler->GetCName(), sdespacket);
        sdespacket->ReleaseReference();
    }

    CRTCPPacket *packet = (CRTCPPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(packet == nullptr);
}


TEST_F(CRTCPHandlerTEST, Test_NotifyRTPPacketSent_then_send_single_SR_compound_RR_with_lost)
{
    CScopedTracer test_info;
    
    RTPPacketSendInfo sendInfo;
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc1 = 0x11223344;
    CRTPStreamPtr sendStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    uint32_t ssrc2 = 0x11223345;
    CRTPStreamPtr sendStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMock sink;
    std::unique_ptr<CRTCPHandlerForTest> handler(new CRTCPHandlerForTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);


    sendInfo = MakeRTPPacketSentInfo(0x11223344, 9530, 1020, 0x22334455);
    sendStream1->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler->NotifyRTPPacketSend(sendInfo, TickNowMS());
    
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9531, 1024, 0x22334456);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9533, 1024, 0x22334457);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9535, 1024, 0x22334457);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 9538, 1024, 0x22334457);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 9531, 1024, 0x22334456);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 9534, 1024, 0x22334456);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 9537, 1024, 0x22334456);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->m_rtcpReportFlag = TRUE;
    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 9540, 1024, 0x22334457);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->TrySendRTCPReport();
    CCmByteStreamNetwork is(*(sink.m_rtcpPacket));
    CSRPacket *srpacket = (CSRPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));

    EXPECT_TRUE(srpacket != nullptr);
    if (srpacket) {
        EXPECT_EQ(0x11223344, srpacket->ssrc);
        EXPECT_EQ(1, srpacket->srInfo.count);
        EXPECT_EQ(1020, srpacket->srInfo.bytes);

        EXPECT_EQ(0x11223344, srpacket->rrBlocks[0].ssrc);
        EXPECT_EQ(128, srpacket->rrBlocks[0].flost);
        EXPECT_EQ(4, srpacket->rrBlocks[0].plost);
        EXPECT_EQ(0, srpacket->rrBlocks[0].jitter);
        EXPECT_EQ(9538, srpacket->rrBlocks[0].xseq);

        EXPECT_EQ(0x11223345, srpacket->rrBlocks[1].ssrc);
        EXPECT_EQ(153, srpacket->rrBlocks[1].flost);
        EXPECT_EQ(6, srpacket->rrBlocks[1].plost);
        EXPECT_EQ(0, srpacket->rrBlocks[1].jitter);
        EXPECT_EQ(9540, srpacket->rrBlocks[1].xseq);

        srpacket->ReleaseReference();
    }


    CSDESPacket *sdespacket = (CSDESPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(sdespacket != nullptr);
    if (sdespacket) {
        CheckSDESPacketInRTCPReport(handler->GetSSRC(), handler->GetCName(), sdespacket);
        sdespacket->ReleaseReference();
    }

    CRTCPPacket *packet = (CRTCPPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(packet == nullptr);
}

TEST_F(CRTCPHandlerTEST, Test_NotifyRTPPacketSent_then_send_single_SR_compound_RR_with_xseq)
{
    CScopedTracer test_info;
    
    RTPPacketSendInfo sendInfo;
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc1 = 0x11223344;
    CRTPStreamPtr sendStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    uint32_t ssrc2 = 0x11223345;
    CRTPStreamPtr sendStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMock sink;
    std::unique_ptr<CRTCPHandlerForTest> handler(new CRTCPHandlerForTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    sendInfo = MakeRTPPacketSentInfo(0x11223344, 9530, 1020, 0x22334455);
    sendStream1->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler->NotifyRTPPacketSend(sendInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 65530, 1024, 0x22334456);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 65532, 1024, 0x22334457);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 65534, 1024, 0x22334457);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo =MakeRTPPacketRecvInfo(0x11223344, 1, 1024, 0x22334457);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 65533, 1024, 0x22334456);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 65534, 1024, 0x22334456);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 1, 1024, 0x22334456);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->m_rtcpReportFlag = TRUE;
    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 65535, 1024, 0x22334457);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->TrySendRTCPReport();
    CCmByteStreamNetwork is(*(sink.m_rtcpPacket));
    CSRPacket *srpacket = (CSRPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));

    EXPECT_TRUE(srpacket != nullptr);
    if (srpacket) {
        EXPECT_EQ(0x11223344, srpacket->ssrc);
        EXPECT_EQ(1, srpacket->srInfo.count);
        EXPECT_EQ(1020, srpacket->srInfo.bytes);

        EXPECT_EQ(0x11223344, srpacket->rrBlocks[0].ssrc);
        EXPECT_EQ(128, srpacket->rrBlocks[0].flost);
        EXPECT_EQ(4, srpacket->rrBlocks[0].plost);
        EXPECT_EQ(0, srpacket->rrBlocks[0].jitter);
        EXPECT_EQ(65537, srpacket->rrBlocks[0].xseq);

        EXPECT_EQ(0x11223345, srpacket->rrBlocks[1].ssrc);
        EXPECT_EQ(51, srpacket->rrBlocks[1].flost);
        EXPECT_EQ(1, srpacket->rrBlocks[1].plost);
        EXPECT_EQ(0, srpacket->rrBlocks[1].jitter);
        EXPECT_EQ(65537, srpacket->rrBlocks[1].xseq);

        srpacket->ReleaseReference();
    }


    CSDESPacket *sdespacket = (CSDESPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(sdespacket != nullptr);
    if (sdespacket) {
        CheckSDESPacketInRTCPReport(handler->GetSSRC(), handler->GetCName(), sdespacket);
        sdespacket->ReleaseReference();
    }

    CRTCPPacket *packet = (CRTCPPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(packet == nullptr);
}


TEST_F(CRTCPHandlerTEST, Test_NotifyRTPPacketSent_then_send_single_SR_compound_RR_with_many_xseq)
{
    CScopedTracer test_info;
    
    RTPPacketSendInfo sendInfo;
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc1 = 0x11223344;
    CRTPStreamPtr sendStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    uint32_t ssrc2 = 0x11223345;
    CRTPStreamPtr sendStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMock sink;
    std::unique_ptr<CRTCPHandlerForTest> handler(new CRTCPHandlerForTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    sendInfo = MakeRTPPacketSentInfo(0x11223344, 9530, 1020, 0x22334455);
    sendStream1->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler->NotifyRTPPacketSend(sendInfo, TickNowMS());

    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 65534, 1024, 0x22334456);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 65535, 1024, 0x22334456);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 0, 1024, 0x22334456);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    int drop = 0;
    for (int i = 1; i <= 65535; i++) {
        if (i % 10 == 0) {
            drop ++;
            continue;
        }
        recvInfo = MakeRTPPacketRecvInfo(0x11223345, i, 1024, 0x22334456);
        recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
        handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    }
    handler->m_rtcpReportFlag = TRUE;
    recvInfo = MakeRTPPacketRecvInfo(0x11223345, 2, 1024, 0x22334457);
    recvStream2->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    drop += 2;

    handler->TrySendRTCPReport();
    CCmByteStreamNetwork is(*(sink.m_rtcpPacket));
    CSRPacket *srpacket = (CSRPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));

    EXPECT_TRUE(srpacket != nullptr);
    if (srpacket) {
        EXPECT_EQ(0x11223344, srpacket->ssrc);
        EXPECT_EQ(1, srpacket->srInfo.count);
        EXPECT_EQ(1020, srpacket->srInfo.bytes);

        EXPECT_EQ(0x11223345, srpacket->rrBlocks[0].ssrc);
        EXPECT_EQ(drop * 255 / 65540, srpacket->rrBlocks[0].flost);
        EXPECT_EQ(drop, srpacket->rrBlocks[0].plost);
        EXPECT_EQ(0, srpacket->rrBlocks[0].jitter);
        EXPECT_EQ(131074, srpacket->rrBlocks[0].xseq);

        srpacket->ReleaseReference();
    }


    CSDESPacket *sdespacket = (CSDESPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(sdespacket != nullptr);
    if (sdespacket) {
        CheckSDESPacketInRTCPReport(handler->GetSSRC(), handler->GetCName(), sdespacket);
        sdespacket->ReleaseReference();
    }

    CRTCPPacket *packet = (CRTCPPacket *)DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    EXPECT_TRUE(packet == nullptr);
}


BOOL ConvertReceiveRateForTest(uint32_t receiveRate, uint32_t &mantissa, uint8_t &exponent)
{
    for (int i=0; i<64; i++) {
        if (receiveRate <= ((uint32_t)262143 << i)) {
            mantissa = receiveRate >> i;
            exponent = i;
            return TRUE;
        }
    }

    return FALSE;
}

void MakeSRPacketWithoutRRBlock(uint32_t ssrc, CSRPacket &packet)
{
    packet.ssrc = ssrc;
    packet.srInfo.ntph = 0x96;
    packet.srInfo.ntpl = 0x5566;
    packet.srInfo.ts = 0x7788;
    packet.srInfo.count = 0x123;
    packet.srInfo.bytes = 0x556677;
}

TEST_F(CRTCPHandlerTEST, Test_NotifyRTPPacketReceived_with_qos)
{
    CScopedTracer test_info;
    
    RTPPacketSendInfo sendInfo;
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc1 = 0x11223344;
    CRTPStreamPtr sendStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    uint32_t ssrc2 = 0x11223345;
    CRTPStreamPtr sendStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    IQoSFeedbackGeneratorMock m_qosfbgm;
    CRTCPHandlerSinkMock sink;
    std::unique_ptr<CRTCPHandlerForTest> handler(new CRTCPHandlerForTest(m_session->GetSessionContext()));
    handler->SetSink(&sink);

    CSRPacket srPacket;
    MakeSRPacketWithoutRRBlock(0x11223344, srPacket);

    uint32_t encodeSize = srPacket.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize);
    CCmByteStreamNetwork os(mb);
    ASSERT_EQ(srPacket.Encode(os, mb), TRUE);

    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 1, 1024, 0x22334457, 0, 90000);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());
    sendInfo =MakeRTPPacketSentInfo(0x11223344, 9530, 1020, 0x22334455);
    sendStream1->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler->NotifyRTPPacketSend(sendInfo, TickNowMS());
    handler->RecvRTCPPacket(mb);
    handler->m_rtcpReportFlag = TRUE;
    recvInfo = MakeRTPPacketRecvInfo(0x11223344, 2, 1025, 0x22334457, 0, 90000);
    recvStream1->GetRecvStats().NotifyRTPPacketRecv(recvInfo, TickNowMS());
    handler->NotifyRTPPacketRecv(recvInfo, TickNowMS());

    handler->TrySendRTCPReport();
    CCmByteStreamNetwork is(*(sink.m_rtcpPacket));
    CRTCPPacket *rtcpPacket = DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    ASSERT_TRUE(rtcpPacket != nullptr);
    ASSERT_EQ(RTCP_PT_SR, rtcpPacket->GetType());
    CSRPacket *srpacket = (CSRPacket *)rtcpPacket;

    if (srpacket) {
        EXPECT_EQ(0x11223344, srpacket->ssrc);
        EXPECT_EQ(1, srpacket->srInfo.count);
        EXPECT_EQ(1020, srpacket->srInfo.bytes);

        EXPECT_EQ(0x11223344, srpacket->rrBlocks[0].ssrc);
        EXPECT_EQ(0, srpacket->rrBlocks[0].flost);
        EXPECT_EQ(0, srpacket->rrBlocks[0].plost);
        EXPECT_EQ(2, srpacket->rrBlocks[0].xseq);

        srpacket->ReleaseReference();
    }
}

TEST_F(CRTCPHandlerTEST, CRTCPHandlerNotifyRTPPacketSent)
{
    CScopedTracer test_info;
    CRTCPHandlerSinkMock sink;
    std::unique_ptr<CRTCPHandler> handler(new CRTCPHandler(RTPSessionContextSharedPtr(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO))));
    handler->SetSink(&sink);

    handler->NotifyRTPPacketSend(MakeRTPPacketSentInfo(0x11223344, 0x9527, 1022, 0x22334455, false), TickNowMS());
    handler->NotifyRTPPacketSend(MakeRTPPacketSentInfo(0x11223344, 0x9528, 1020, 0x22335455), TickNowMS());
    handler->NotifyRTPPacketSend(MakeRTPPacketSentInfo(0x11223344, 0x9529, 1024, 0x22336455), TickNowMS());
    handler->NotifyRTPPacketSend(MakeRTPPacketSentInfo(0x11223344, 0x9530, 1024, 0x22337455), TickNowMS());
    handler->NotifyRTPPacketSend(MakeRTPPacketSentInfo(0x11223344, 0x9531, 1024, 0x22338455), TickNowMS());
}

TEST_F(CRTCPHandlerTEST, CRTCPHandlerNotifyRTPPacketReceived)
{
    CScopedTracer test_info;
    CRTCPHandlerSinkMock sink;
    
    std::unique_ptr<CRTCPHandler> handler(new CRTCPHandler(RTPSessionContextSharedPtr(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO))));
    handler->SetSink(&sink);

    handler->NotifyRTPPacketRecv(MakeRTPPacketRecvInfo(0x11223344, 0x9527, 1022, 0x22334455), TickNowMS());
    handler->NotifyRTPPacketRecv(MakeRTPPacketRecvInfo(0x11223344, 0x9530, 1020, 0x22335455), TickNowMS());
    handler->NotifyRTPPacketRecv(MakeRTPPacketRecvInfo(0x11223344, 0x9529, 1024, 0x22336455), TickNowMS());
    handler->NotifyRTPPacketRecv(MakeRTPPacketRecvInfo(0x11223344, 0x9528, 1024, 0x22337455), TickNowMS());
    handler->NotifyRTPPacketRecv(MakeRTPPacketRecvInfo(0x11223344, 0x9531, 1024, 0x22338455), TickNowMS());
}

class CRTCPHandlerMock:public CRTCPHandler
{
public:
    CRTCPHandlerMock(const RTPSessionContextSharedPtr &ctx) : CRTCPHandler(ctx) {}

    virtual void SubmitFBNetworkMetric()
    {
        m_fbNetworkMetricMock = m_fbNetworkMetric;

    }
    virtual void SubmitFBNetworkMetricMock()
    {
        CRTCPHandler::SubmitFBNetworkMetric();
    }

    void TrySendRTCPReport()
    {
        CRTCPHandler::TrySendRTCPReport();
    }

public:
    NetworkMetricEx     m_fbNetworkMetricMock;
};


TEST_F(CRTCPHandlerTEST, CRTCPHandlerSR)
{
    CScopedTracer test_info;
    CRTCPHandlerSinkMock sink;
    CRTCPHandlerMock handler(m_session->GetSessionContext());
    handler.SetSink(&sink);

    CSRPacket packet;
    packet.ssrc = 0x12345678;
    packet.srInfo.ntph = 0x96;
    packet.srInfo.ntpl = 0x5566;
    packet.srInfo.ts = 0x7788;
    packet.srInfo.count = 0x123;
    packet.srInfo.bytes = 0x556677;

    RRBlock rrBlock;
    rrBlock.ssrc = 0x3579;
    rrBlock.flost = 0x21;
    rrBlock.plost = 0x4567;
    rrBlock.xseq = 0x23456;
    rrBlock.jitter = 0x17;
    rrBlock.lsr = 0x9876;
    rrBlock.dlsr = 0x76543;
    packet.rrBlocks.push_back(rrBlock);

    uint32_t encodeSize = packet.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize);
    CCmByteStreamNetwork os(mb);
    EXPECT_EQ(packet.Encode(os, mb), TRUE);

    RTPPacketSendInfo info;
    info.ssrc = 0x3579;
    handler.NotifyRTPPacketSend(info, TickNowMS());
    handler.RecvRTCPPacket(mb);

    //NetworkMetricEx m_fbNetworkMetricMock = handler.m_fbNetworkMetricMock;
    //float lossRatio = (float)(0x21*1.0/256);
    //EXPECT_EQ(lossRatio, m_fbNetworkMetricMock.lossRatio);
    EXPECT_TRUE(true);
}

TEST_F(CRTCPHandlerTEST, CRTCPHandlerCompoundPacket)
{
    CScopedTracer test_info;
    CRTCPHandlerSinkMock sink;
    CRTCPHandlerMock handler(m_session->GetSessionContext());
    handler.SetSink(&sink);

    CCompoundPacket packet;

    CSRPacket packet01;
    packet01.ssrc = 0x12345678;
    packet01.srInfo.ntph = 0x96;
    packet01.srInfo.ntpl = 0x5566;
    packet01.srInfo.ts = 0x7788;
    packet01.srInfo.count = 0x123;
    packet01.srInfo.bytes = 0x556677;

    RRBlock rrBlock;
    rrBlock.ssrc = 0x3579;
    rrBlock.flost = 0x21;
    rrBlock.plost = 0x4567;
    rrBlock.xseq = 0x23456;
    rrBlock.jitter = 0x17;
    rrBlock.lsr = 0x9876;
    rrBlock.dlsr = 0x76543;
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);

    CSDESPacket packet02;
    SDESTrunk trunk;
    trunk.ssrc = 0x12345678;
    SDESItem item;
    item.type = 1;
    item.content = (uint8_t *)"cname_test";
    item.length = strlen((char *)item.content);
    trunk.sdesItems.push_back(item);
    packet02.sdesTrunks.push_back(trunk);

    CBYEPacket packet03;
    packet03.ssrcs.push_back(0x12345678);
    packet03.ssrcs.push_back(0x23456789);
    packet03.ssrcs.push_back(0x34567890);
    packet03.ssrcs.push_back(0x45678901);
    packet03.ssrcs.push_back(0x56789012);
    packet03.ssrcs.push_back(0x67890123);
    packet03.reason = const_cast<char *>("I'm sad, so I leave");
    packet03.reasonLength = strlen(packet03.reason);

    packet.AddRtcpPacket(&packet01);
    packet.AddRtcpPacket(&packet02);
    packet.AddRtcpPacket(&packet03);

    uint32_t encodeSize = packet.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize);
    CCmByteStreamNetwork os(mb);
    EXPECT_EQ(packet.Encode(os, mb), TRUE);

    // clear packets
    packet.packets.clear();
    // we haven't alloc memory for BYE reason, so cleanup
    packet03.reason = nullptr;
    packet03.reasonLength = 0;
    // we haven't alloc memory for trunk items' content, so cleanup
    packet02.sdesTrunks.clear();

    RTPPacketSendInfo info;
    info.ssrc = 0x3579;
    handler.NotifyRTPPacketSend(info, TickNowMS());
    handler.RecvRTCPPacket(mb);

    //NetworkMetricEx m_fbNetworkMetricMock = handler.m_fbNetworkMetricMock;
    //float lossRatio = (float)(0x21*1.0/256);
    //EXPECT_EQ(lossRatio, m_fbNetworkMetricMock.lossRatio);
    EXPECT_TRUE(true);
}

TEST_F(CRTCPHandlerTEST, CRTCPHandlerCompoundPacket_RR)
{
    CScopedTracer test_info;
    CRTCPHandlerSinkMock sink;
    CRTCPHandlerMock handler(m_session->GetSessionContext());
    handler.SetSink(&sink);

    CCompoundPacket packet;

    CRRPacket packet01;
    packet01.ssrc = 0x12345678;


    RRBlock rrBlock;
    rrBlock.ssrc = 0x3579;
    rrBlock.flost = 0x21;
    rrBlock.plost = 0x4567;
    rrBlock.xseq = 0x23456;
    rrBlock.jitter = 0x17;
    rrBlock.lsr = 0x9876;
    rrBlock.dlsr = 0x76543;
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);
    packet01.rrBlocks.push_back(rrBlock);

    CSDESPacket packet02;
    SDESTrunk trunk;
    trunk.ssrc = 0x12345678;
    SDESItem item;
    item.type = 1;
    item.content = (uint8_t *)"cname_test";
    item.length = strlen((char *)item.content);
    trunk.sdesItems.push_back(item);
    packet02.sdesTrunks.push_back(trunk);

    CBYEPacket packet03;
    packet03.ssrcs.push_back(0x12345678);
    packet03.ssrcs.push_back(0x23456789);
    packet03.ssrcs.push_back(0x34567890);
    packet03.ssrcs.push_back(0x45678901);
    packet03.ssrcs.push_back(0x56789012);
    packet03.ssrcs.push_back(0x67890123);
    packet03.reason = const_cast<char *>("I'm sad, so I leave");
    packet03.reasonLength = strlen(packet03.reason);

    packet.AddRtcpPacket(&packet01);
    packet.AddRtcpPacket(&packet02);
    packet.AddRtcpPacket(&packet03);

    uint32_t encodeSize = packet.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize);
    CCmByteStreamNetwork os(mb);
    EXPECT_EQ(packet.Encode(os, mb), TRUE);

    // clear packets
    packet.packets.clear();
    // we haven't alloc memory for BYE reason, so cleanup
    packet03.reason = nullptr;
    packet03.reasonLength = 0;
    // we haven't alloc memory for trunk items' content, so cleanup
    packet02.sdesTrunks.clear();

    RTPPacketSendInfo info;
    info.ssrc = 0x3579;
    handler.NotifyRTPPacketSend(info, TickNowMS());
    handler.RecvRTCPPacket(mb);

    //NetworkMetricEx m_fbNetworkMetricMock = handler.m_fbNetworkMetricMock;
    //float lossRatio = (float)(0x21*1.0/256);
    //EXPECT_EQ(lossRatio, m_fbNetworkMetricMock.lossRatio);
    EXPECT_TRUE(true);
}

static void ConstuctCSRPacket(CSRPacket &packet,UINT ssrc,UINT ntph,UINT ntpl,UINT ts,UINT count,UINT bytes)
{
    packet.ssrc = ssrc;
    packet.srInfo.ntph = ntph;
    packet.srInfo.ntpl = ntpl;
    packet.srInfo.ts = ts;
    packet.srInfo.count = count;
    packet.srInfo.bytes = bytes;
}

static RRBlock &ConstuctRRBlock(RRBlock &rrBlock,uint32_t ssrc,uint8_t flost,uint32_t plost,uint32_t xseq,uint32_t jitter,uint32_t lsr,uint32_t dlsr)
{
    rrBlock.ssrc = ssrc;
    rrBlock.flost = flost;
    rrBlock.plost = plost;
    rrBlock.xseq = xseq;
    rrBlock.jitter = jitter;
    rrBlock.lsr = lsr;
    rrBlock.dlsr = dlsr;
    return rrBlock;
}

TEST_F(CRTCPHandlerTEST, RTCPHandlerSR_Statistic)
{
    CScopedTracer test_info;
    
    RTPPacketSendInfo sendInfo;
    RTPPacketRecvInfo recvInfo;
    uint32_t ssrc1 = 0x3579;
    CRTPStreamPtr sendStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream1 = m_session->CreateRTPStream(ssrc1, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    uint32_t ssrc2 = 0x3580;
    CRTPStreamPtr sendStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTPStreamPtr recvStream2 = m_session->CreateRTPStream(ssrc2, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_IN);
    
    CRTCPHandlerSinkMock sink;
    CRTCPHandlerMock handler(m_session->GetSessionContext());
    handler.SetSink(&sink);
    const int COUNT = 2;

    UINT uLostRatio = 0x21;
    UINT uJitter = 0x170000;

    CSRPacket packet;
    ConstuctCSRPacket(packet,0x12345678,0x96,0x5566,0x7788,0x123,0x556677);

    RRBlock rrBlock[COUNT];
    for (int i = 0; i<COUNT; i++) {
        packet.rrBlocks.push_back(ConstuctRRBlock(rrBlock[0], 0x3579,uLostRatio,0x4567,0x23456,uJitter,0x9876,0x76543));
    }

    packet.rrBlocks.push_back(ConstuctRRBlock(rrBlock[1], 0x3580,uLostRatio,0x4567,0x23456,uJitter,0x9876,0x76543));

    uint32_t encodeSize = packet.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize);
    CCmByteStreamNetwork os(mb);
    EXPECT_EQ(packet.Encode(os, mb), TRUE);

    sendInfo = MakeRTPPacketSentInfo(0x3579, 0x9531, 1024, 0x22338455);
    sendStream1->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler.NotifyRTPPacketSend(sendInfo, TickNowMS());
    sendInfo = MakeRTPPacketSentInfo(0x3580, 0x9531, 1024, 0x22338455);
    sendStream2->GetSendStats().NotifyRTPPacketSent(sendInfo, TickNowMS(), false);
    handler.NotifyRTPPacketSend(sendInfo, TickNowMS());

    handler.RecvRTCPPacket(mb);

    handler.TrySendRTCPReport(); // refresh the session statistics

    SessStats stats;
    handler.GetSessionStats(stats);
    EXPECT_FLOAT_EQ(stats.outStats.netStats.lossRatio, (float)(uLostRatio*1.0/256));
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CRTCPHandlerSinkWithParameterMock : public IRTCPHandlerSink
{
public:
    CRTCPHandlerSinkWithParameterMock()
    {
        m_rtcpPacket = nullptr;
    }

    ~CRTCPHandlerSinkWithParameterMock()
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
    virtual void OnRTCPBandwidthUpdated(uint32_t bandwidth) {}

    MOCK_METHOD0(GetFECEnabled, bool(void));

public:
    CCmMessageBlock *m_rtcpPacket;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// UT for MARI packet handling
class CRTCPHandlerSinkMockForMARI : public IRTCPHandlerSink
{
public:
    CRTCPHandlerSinkMockForMARI()
    {
        m_rtcpPacket = nullptr;
    }

    ~CRTCPHandlerSinkMockForMARI()
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
    virtual void OnRTCPBandwidthUpdated(uint32_t bandwidth) {}
    virtual void OnStreamBandwidthUpdated(uint32_t ssrc, uint32_t bandwidth) {}

    virtual bool GetInboundFECEnabled() { return false; }
    virtual bool GetOutboundFECEnabled() {return  false;}

    virtual void NotifyMariRecvStats(const NetworkMetric &networkmetric) {}
    virtual void NotifyMariSendStats(const NetworkMetricEx &networkmetric) {}

    virtual void OnRTCPByeReceived(uint32_t ssrc) {}
public:
    CCmMessageBlock *m_rtcpPacket;
};

class CRTCPHandlerMockForMARI : public CRTCPHandler
{
public:
    CRTCPHandlerMockForMARI() : CRTCPHandler(RTPSessionContextSharedPtr(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO)))
    {
        m_rtcpReportFlag = FALSE;
    }
    ~CRTCPHandlerMockForMARI() {}

    virtual void TrySendRTCPReport()
    {
        if (m_rtcpReportFlag) {
            return CRTCPHandler::TrySendRTCPReport();
        }
    }

    uint32_t GetSSRC()
    {
        return m_ssrc;
    }

    string GetCName()
    {
        return m_cname;
    }

    UT_VIRTUAL QoSSharedPtr GetQoSHandler()
    {
        return m_qosHandler;
    }

    BOOL m_rtcpReportFlag;
    QoSSharedPtr m_qosHandler;
};

class CQoSHandlerMockForMARI : public IQoSHandler
{
public:
    CQoSHandlerMockForMARI()
    {
        m_sourceId = 238973;
        m_lossRatio = 0.12f;
        m_qdelay = 45; // ms
        m_receiveRate = 232923;

        m_receivedFB = false;
    }
    virtual bool IsInitialized()
    {
        return true;
    }
    virtual string GetDescStr()
    {
        return "TestForMARI";
    }
    virtual uint32_t GetServiceId()
    {
        return 0;
    }
    virtual int32_t Pause()
    {
        return 0;
    }
    virtual int32_t Resume()
    {
        return 0;
    }
    virtual void NotifyRTPPacketSend(uint32_t tickNowMs, const RTPPacketSendInfo &info, bool sendFailed, bool bPadding) { }
    virtual FeedbackResult OnDataReceived(uint32_t now, uint32_t sourceId, uint16_t sequence, uint32_t length, uint32_t timestamp)
    {
        return FEEDBACK_ACTION;
    }
    virtual int32_t GetNetworkMetricFeedback(uint32_t now, NetworkMetric &networkMetric)
    {
        networkMetric.lossRatio = m_lossRatio;
        networkMetric.qdelay = m_qdelay*10; // 100us
        networkMetric.receiveRate = m_receiveRate;
        return WRTP_ERR_NOERR;
    }
    virtual int32_t FeedbackNetworkMetrics(uint32_t tickNowMs, NetworkMetricEx &netMetric)
    {
        m_receivedFB = true;
        m_receivedNM = netMetric;
        return WRTP_ERR_NOERR;
    }
    virtual int32_t SetAdjustmentPolicy(const AdjustmentPolicy &policy)
    {
        return WRTP_ERR_NOERR;
    }

    virtual int32_t GetBwDownEventStats(BwDownEventStat &stat)
    {
        return 0;
    };
    virtual int32_t GetSevereCongestionStats(uint32_t &count)
    {
        return 0;
    };

public:
    uint32_t m_sourceId;
    float m_lossRatio;
    uint16_t m_qdelay;
    uint32_t m_receiveRate;

    bool m_receivedFB;
    NetworkMetricEx m_receivedNM;
};

class CMediaSessionFilterMockForMARI: public IMediaSessionFilter
{
public:
    virtual int32_t QueryRequirement(QUERY_TYPE eType, uint32_t &bandwidth)
    {
        return 0;
    }
};

TEST_F(CRTCPHandlerTEST, Test_send_MARI_packet)
{
    CScopedTracer test_info;
    CRTCPHandlerSinkMockForMARI sink;
    std::unique_ptr<CRTCPHandlerMockForMARI> handler(new CRTCPHandlerMockForMARI());
    handler->SetSink(&sink);

    CMediaSessionFilterMockForMARI filter;
    QoSParameter params;
    char peerId[] = "this is an id";
    params.peerID = (uint8_t *)peerId;
    params.mediaSessionFilter = &filter;
    ASSERT_EQ(0, handler->EnableQoS(params));

    CQoSHandlerMockForMARI *qosHandlerMARI = new CQoSHandlerMockForMARI();
    handler->m_qosHandler.reset(qosHandlerMARI);

    CSRPacket srPacket;
    srPacket.ssrc = qosHandlerMARI->m_sourceId;
    srPacket.srInfo.ntph = 0x96;
    srPacket.srInfo.ntpl = 0x5566;
    srPacket.srInfo.ts = 0x22334457;
    srPacket.srInfo.count = 0x123;
    srPacket.srInfo.bytes = 0x556677;
    CCmMessageBlock mb(srPacket.CalcEncodeSize() + MAX_RTCP_PACKET_RESERVED_LEN);
    CCmByteStreamNetwork os(mb);
    srPacket.Encode(os, mb);
    handler->RecvRTCPPacket(mb);
    //handler.HandleSRPacket(&srPacket);

    uint32_t mariTimestamp = TickMsToMariTimestamp(TickNowMS());
    handler->NotifyRTPPacketRecv(MakeRTPPacketRecvInfo(qosHandlerMARI->m_sourceId, 1, 1024, 0x22334457, mariTimestamp, 0, 90000, false), TickNowMS());
    //handler.NotifyRTPPacketSend(MakeRTPPacketSentInfo(0x11223344, 9530, 1020, 0x22334455));
    //handler.NotifyRTPPacketRecv(0x11223344, 2, 1025, 0x22334457, 0, 90000, false);

    ASSERT_TRUE(sink.m_rtcpPacket != nullptr);
    CCmByteStreamNetwork is(*(sink.m_rtcpPacket));
    // now MARI packet is sent as reduced-size RTCP packet
    // decode RR packet
    /*CRTCPPacket* rtcpPacket = DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    ASSERT_TRUE(rtcpPacket != nullptr);
    ASSERT_EQ(RTCP_PT_RR, rtcpPacket->GetType());
    rtcpPacket->ReleaseReference();

    //decode SDES packet
    rtcpPacket = DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    ASSERT_TRUE(rtcpPacket != nullptr);
    ASSERT_EQ(RTCP_PT_SDES, rtcpPacket->GetType());
    rtcpPacket->ReleaseReference();
    */
    //decode MARI packet
    CRTCPPacket *rtcpPacket = DecodeRTCPPacket(is, *(sink.m_rtcpPacket));
    ASSERT_TRUE(rtcpPacket != nullptr);
    ASSERT_EQ(RTCP_PT_PSFB, rtcpPacket->GetType());
    CPSFBPacket *mariPacket = (CPSFBPacket *)rtcpPacket;
    ASSERT_TRUE(mariPacket != nullptr);
    float lossRatio = 0.0f;
    uint16_t qdelay = 0;
    uint32_t timestamp = 0;
    uint64_t receiveRate = 0;
    BOOL ret = DecodePsfbMARIPacket(*mariPacket, lossRatio, qdelay, timestamp, receiveRate);
    ASSERT_TRUE(ret == TRUE);
    EXPECT_EQ(uint8_t(qosHandlerMARI->m_lossRatio*256), uint8_t(lossRatio*256));
    EXPECT_EQ(qosHandlerMARI->m_qdelay, qdelay/10);
    EXPECT_EQ(qosHandlerMARI->m_receiveRate, receiveRate/8);
    EXPECT_EQ(mariTimestamp, timestamp);

    rtcpPacket->ReleaseReference();

    handler->DisableQoS();
}

TEST_F(CRTCPHandlerTEST, Test_receive_MARI_packet)
{
    CScopedTracer test_info;
    CRTCPHandlerSinkMockForMARI sink;
    std::unique_ptr<CRTCPHandlerMockForMARI> handler(new CRTCPHandlerMockForMARI());
    handler->SetSink(&sink);

    CMediaSessionFilterMockForMARI filter;
    QoSParameter params;
    char peerId[] = "this is an id";
    params.peerID = (uint8_t *)peerId;
    params.mediaSessionFilter = &filter;
    ASSERT_EQ(0, handler->EnableQoS(params));

    CQoSHandlerMockForMARI *qosHandlerMARI = new CQoSHandlerMockForMARI();
    handler->m_qosHandler.reset(qosHandlerMARI);

    CSRPacket srPacket;
    srPacket.ssrc = qosHandlerMARI->m_sourceId;
    srPacket.srInfo.ntph = 0x96;
    srPacket.srInfo.ntpl = 0x5566;
    srPacket.srInfo.ts = 0x22334457;
    srPacket.srInfo.count = 0x123;
    srPacket.srInfo.bytes = 0x556677;
    CCmMessageBlock mb(srPacket.CalcEncodeSize() + MAX_RTCP_PACKET_RESERVED_LEN);
    CCmByteStreamNetwork os(mb);
    srPacket.Encode(os, mb);
    handler->RecvRTCPPacket(mb);
    //handler.HandleSRPacket(&srPacket);

    uint32_t mariTimestamp = TickMsToMariTimestamp(TickNowMS());
    handler->NotifyRTPPacketRecv(MakeRTPPacketRecvInfo(qosHandlerMARI->m_sourceId, 1, 1024, 0x22334457, mariTimestamp, 0, 90000, false), TickNowMS());
    ASSERT_TRUE(sink.m_rtcpPacket != nullptr);
    CCmMessageBlock *dupPacket = sink.m_rtcpPacket->DuplicateChained();

    handler->RecvRTCPPacket(*sink.m_rtcpPacket);
    EXPECT_TRUE(qosHandlerMARI->m_receivedFB);

    handler->NotifyRTPPacketSend(MakeRTPPacketSentInfo(qosHandlerMARI->m_sourceId, 9530, 1020, 0x22334455), TickNowMS());
    handler->RecvRTCPPacket(*dupPacket);
    dupPacket->DestroyChained();

    EXPECT_TRUE(qosHandlerMARI->m_receivedFB);
    EXPECT_EQ(uint8_t(qosHandlerMARI->m_lossRatio*256), uint8_t(qosHandlerMARI->m_receivedNM.lossRatio*256));
    EXPECT_EQ(qosHandlerMARI->m_qdelay, MariTimestampToTickMs(qosHandlerMARI->m_receivedNM.qdelay));
    EXPECT_EQ(qosHandlerMARI->m_receiveRate, qosHandlerMARI->m_receivedNM.receiveRate);

    handler->DisableQoS();
}




