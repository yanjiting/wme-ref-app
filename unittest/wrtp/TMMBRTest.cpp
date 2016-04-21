#include "gmock/gmock.h"
#include "gtest/gtest.h"
#define private public
#include "rtcphandler.h"
#include "rtpinternal.h"
#include "testutil.h"
#include "rtpsessionconfig.h"
#include "rtpsessionclient.h"

using namespace wrtp;
using namespace std;

bool DecodeCompoundPacket(CCmMessageBlock &mb, CCompoundPacket &compoundPacket);

class CTMMBRTest : public ::testing::Test
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

class CRTCPHandlerSinkTMMBR : public IRTCPHandlerSink
{
public:
    CRTCPHandlerSinkTMMBR()
    {
        bandwidth = 0;
        streamBandwidth = 0;
        mbPacket = nullptr;
        tmmbr = nullptr;
        tmmbn = nullptr;
    }

    ~CRTCPHandlerSinkTMMBR()
    {
        if (mbPacket) {
            mbPacket->DestroyChained();
            mbPacket = nullptr;
        }
        if (tmmbr) {
            tmmbr->ReleaseReference();
            tmmbr = nullptr;
        }
        if (tmmbn) {
            tmmbn->ReleaseReference();
            tmmbn = nullptr;
        }
    }

    virtual int32_t OnSendRTCPPacket(RTP_IN CCmMessageBlock &rtcpPacket)
    {
        if (mbPacket) {
            mbPacket->DestroyChained();
            mbPacket = nullptr;
        }
        mbPacket = rtcpPacket.DuplicateChained();
        CCompoundPacket comp;
        if (DecodeCompoundPacket(rtcpPacket, comp)) {
            for (uint32_t i=0; i < comp.packets.size(); ++i) {
                CRTCPPacket *p1 = comp.packets[i];
                if (p1->GetType() != RTCP_PT_RTPFB) {
                    continue;
                }
                CRTPFBPacket *p2 = (CRTPFBPacket *)(p1);
                if (nullptr == p2) {
                    continue;
                }
                if (p2->GetFeedbackMessageType() == RTCP_RTPFB_TMMBR) {
                    if (tmmbr) {
                        tmmbr->ReleaseReference();
                        tmmbr = nullptr;
                    }
                    tmmbr = p2;
                    tmmbr->AddReference();
                } else if (p2->GetFeedbackMessageType() == RTCP_RTPFB_TMMBN) {
                    if (tmmbn) {
                        tmmbn->ReleaseReference();
                        tmmbn = nullptr;
                    }
                    tmmbn = p2;
                    tmmbn->AddReference();
                }
            }
        }

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

    virtual void OnReceivedSequenceIndication(uint32_t ssrc, uint32_t xseq) {}
    virtual void OnLossRatioIndication(float lossRatio) {};
    virtual CRTPStreamPtr GetStreamBySSRC(uint32_t ssrc, eStreamDirection dir)
    {
        return CRTPStreamPtr();
    }
    virtual uint32_t GetNalNumToListenChannel()
    {
        return 0;
    }
    virtual void OnNetworkControlledInformation(AdjustmentState state, const AggregateMetric &netMetric, uint32_t bandwidth,uint32_t totalBandwidth) {}
    virtual void OnRTCPBandwidthUpdated(uint32_t bw)
    {
        bandwidth = bw;
    }
    virtual void OnStreamBandwidthUpdated(uint32_t ssrc, uint32_t bandwidth)
    {
        streamBandwidth = bandwidth;
    }

    virtual bool GetInboundFECEnabled() {return false;}
    virtual bool GetOutboundFECEnabled() {return false;}

    virtual void NotifyMariRecvStats(const NetworkMetric &networkmetric) {}
    virtual void NotifyMariSendStats(const NetworkMetricEx &networkmetric) {}

    virtual void OnRTCPByeReceived(uint32_t ssrc) {}
public:
    uint32_t bandwidth;
    uint32_t streamBandwidth;
    CCmMessageBlock *mbPacket;
    CRTPFBPacket *tmmbr;
    CRTPFBPacket *tmmbn;
};

class CRTCPHandlerTMMBR : public CRTCPHandler
{
public:
    CRTCPHandlerTMMBR(const RTPSessionContextSharedPtr& ctx) : CRTCPHandler(ctx) {}
    ~CRTCPHandlerTMMBR() {}

    uint32_t GetSSRC()
    {
        return m_ssrc;
    }
};

TEST_F(CTMMBRTest, TestRecvTMMBR_not_for_us)
{
    CScopedTracer test_info;
    CRTCPHandlerSinkTMMBR sink;
    std::unique_ptr<CRTCPHandlerTMMBR> rtcp(new CRTCPHandlerTMMBR(m_session->GetSessionContext()));
    rtcp->SetSink(&sink);

    CRTPFBPacket packet;
    TMMBRVECTOR items;
    TMMBRTuple item;
    item.ssrc = 11111;
    item.maxTBR = 2*1024*1024;
    item.oh = 28;
    items.push_back(item);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items, packet));
    EXPECT_EQ(packet.m_fciLength, 8);
    items.clear();

    CCmMessageBlock mb(packet.CalcEncodeSize());
    CCmByteStreamNetwork os(mb);
    EXPECT_TRUE(!!packet.Encode(os, mb));

    rtcp->RecvRTCPPacket(mb);
    EXPECT_EQ(0, sink.bandwidth);
    EXPECT_NE(item.maxTBR/8, sink.bandwidth);
}

TEST_F(CTMMBRTest, TestRecvTMMBR_for_us)
{
    CScopedTracer test_info;
    CRTCPHandlerSinkTMMBR sink;
    std::unique_ptr<CRTCPHandlerTMMBR> rtcp(new CRTCPHandlerTMMBR(m_session->GetSessionContext()));
    rtcp->SetSink(&sink);
    rtcp->ConfigTMMBR(true, true);

    CRTPFBPacket packet;
    TMMBRVECTOR items;
    TMMBRTuple item;
    item.ssrc = rtcp->GetSSRC();
    item.maxTBR = 2*1024*1024;
    item.oh = 28;
    items.push_back(item);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items, packet));
    EXPECT_EQ(packet.m_fciLength, 8);
    items.clear();

    CCmMessageBlock mb(packet.CalcEncodeSize());
    CCmByteStreamNetwork os(mb);
    EXPECT_TRUE(!!packet.Encode(os, mb));

    rtcp->RecvRTCPPacket(mb);
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_NE(0, sink.bandwidth);
    EXPECT_EQ(item.maxTBR/8, sink.bandwidth);
#endif
    EXPECT_EQ(item.maxTBR/8, sink.streamBandwidth);
}

TEST_F(CTMMBRTest, TestRecvTMMBR_for_us_sender)
{
    CScopedTracer test_info;
    uint32_t ssrc = 111111;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    CRTCPHandlerSinkTMMBR sink;
    std::unique_ptr<CRTCPHandlerTMMBR> rtcp(new CRTCPHandlerTMMBR(m_session->GetSessionContext()));
    rtcp->SetSink(&sink);
    rtcp->ConfigTMMBR(true, true);

    RTPPacketSendInfo info;
    info.ssrc = (ssrc);
    info.packetLength = (1029);
    info.clockRate = (90000);
    info.rtpTimestamp = (38578353);
    info.captureTick = (238472);
    info.sequence = (3398);
    streamPtr->GetSendStats().NotifyRTPPacketSent(info, TickNowMS(), false);
    rtcp->NotifyRTPPacketSend(info, TickNowMS());

    CRTPFBPacket packet;
    TMMBRVECTOR items;
    TMMBRTuple item;
    item.ssrc = ssrc;
    item.maxTBR = 2*1024*1024;
    item.oh = 28;
    items.push_back(item);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items, packet));
    EXPECT_EQ(packet.m_fciLength, 8);
    items.clear();

    CCmMessageBlock mb(packet.CalcEncodeSize());
    CCmByteStreamNetwork os(mb);
    EXPECT_TRUE(!!packet.Encode(os, mb));

    rtcp->RecvRTCPPacket(mb);
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_NE(0, sink.bandwidth);
    EXPECT_EQ(item.maxTBR/8, sink.bandwidth);
#endif
    EXPECT_EQ(item.maxTBR/8, sink.streamBandwidth);
}

TEST_F(CTMMBRTest, TestRecvTMMBR_And_Send_TMMBN_selected)
{
    CScopedTracer test_info;
    uint32_t ssrc = 111111;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    uint32_t tmmbrSSRC = 123456;
    CRTCPHandlerSinkTMMBR sink;
    std::unique_ptr<CRTCPHandlerTMMBR> rtcp(new CRTCPHandlerTMMBR(m_session->GetSessionContext()));
    rtcp->SetSink(&sink);
    //rtcp.Stop(); // stop RTCP timer;
    rtcp->ConfigTMMBR(true, true);
    rtcp->UpdateQoSTBR(3*1024*1024);

    RTPPacketSendInfo info;
    info.ssrc = ssrc;
    info.packetLength = (1029);
    info.clockRate = (90000);
    info.rtpTimestamp = (38578353);
    info.captureTick = 238472;
    info.sequence = (3398);
    streamPtr->GetSendStats().NotifyRTPPacketSent(info, TickNowMS(), false);
    rtcp->NotifyRTPPacketSend(info, TickNowMS());

    CRTPFBPacket packet;
    TMMBRVECTOR items;
    TMMBRTuple item;
    item.ssrc = ssrc;
    item.maxTBR = 2*1024*1024;
    item.oh = 28;
    items.push_back(item);
    EXPECT_TRUE(!!EncodeTMMBRPacket(tmmbrSSRC, RTCP_RTPFB_TMMBR, items, packet));
    EXPECT_EQ(packet.m_fciLength, 8);
    items.clear();

    CCmMessageBlock mb(packet.CalcEncodeSize());
    CCmByteStreamNetwork os(mb);
    EXPECT_TRUE(!!packet.Encode(os, mb));

    rtcp->RecvRTCPPacket(mb);
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_NE(0, sink.bandwidth);
    EXPECT_EQ(item.maxTBR/8, sink.bandwidth);
#endif
    EXPECT_EQ(item.maxTBR/8, sink.streamBandwidth);

    /*
    int loop = 4;
    while(loop-- > 0){
        if(sink.tmmbn != nullptr){
            break;
        } else {
            ::SleepMs(250);
        }
    }
     */
    ASSERT_TRUE(sink.tmmbn != nullptr);
    CRTPFBPacket *tmmbnPacket = sink.tmmbn;
    ASSERT_EQ(rtcp->GetSSRC(), tmmbnPacket->m_ssrc);
    ASSERT_EQ(0, tmmbnPacket->m_ssrcSrc);
    ASSERT_EQ(8, tmmbnPacket->m_fciLength);

    items.clear();
    EXPECT_TRUE(!!DecodeTMMBRFCI(*tmmbnPacket, items));
    EXPECT_EQ(1, items.size());
    EXPECT_EQ(tmmbrSSRC, items[0].ssrc);
    EXPECT_EQ(item.maxTBR, items[0].maxTBR);
}

TEST_F(CTMMBRTest, TestRecvTMMBR_And_Send_TMMBN_not_selected)
{
    CScopedTracer test_info;
    uint32_t ssrc = 111111;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    uint32_t tmmbrSSRC = 123456;
    CRTCPHandlerSinkTMMBR sink;
    std::unique_ptr<CRTCPHandlerTMMBR> rtcp(new CRTCPHandlerTMMBR(m_session->GetSessionContext()));
    rtcp->SetSink(&sink);
    rtcp->ConfigTMMBR(true, true);
    rtcp->UpdateQoSTBR(1*1024*1024);

    RTPPacketSendInfo info;
    info.ssrc = (ssrc);
    info.packetLength = (1029);
    info.clockRate = (90000);
    info.rtpTimestamp = (38578353);
    info.captureTick = (238472);
    info.sequence = (3398);
    streamPtr->GetSendStats().NotifyRTPPacketSent(info, TickNowMS(), false);
    rtcp->NotifyRTPPacketSend(info, TickNowMS());

    CRTPFBPacket packet;
    TMMBRVECTOR items;
    TMMBRTuple item;
    item.ssrc = ssrc;
    item.maxTBR = 2*1024*1024;
    item.oh = 0;
    items.push_back(item);
    EXPECT_TRUE(!!EncodeTMMBRPacket(tmmbrSSRC, RTCP_RTPFB_TMMBR, items, packet));
    EXPECT_EQ(packet.m_fciLength, 8);
    items.clear();

    CCmMessageBlock mb(packet.CalcEncodeSize());
    CCmByteStreamNetwork os(mb);
    EXPECT_TRUE(!!packet.Encode(os, mb));

    rtcp->RecvRTCPPacket(mb);
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_NE(0, sink.bandwidth);
    EXPECT_EQ(1*1024*1024/8, sink.bandwidth);
#endif
    EXPECT_NE(0, sink.streamBandwidth);

    /*
    int loop = 4;
    while(loop-- > 0){
        if(sink.tmmbn != nullptr){
            break;
        } else {
            ::SleepMs(250);
        }
    }
     */

    ASSERT_TRUE(sink.tmmbn != nullptr);
    CRTPFBPacket *tmmbnPacket = sink.tmmbn;
    ASSERT_EQ(rtcp->GetSSRC(), tmmbnPacket->m_ssrc);
    ASSERT_EQ(0, tmmbnPacket->m_ssrcSrc);
    ASSERT_EQ(8, tmmbnPacket->m_fciLength);

    items.clear();
    EXPECT_TRUE(!!DecodeTMMBRFCI(*tmmbnPacket, items));
    EXPECT_EQ(1, items.size());
    EXPECT_EQ(rtcp->GetSSRC(), items[0].ssrc);
    EXPECT_EQ(1*1024*1024, items[0].maxTBR);
}

TEST_F(CTMMBRTest, TestSendTMMBN_when_QoS_bw_updated)
{
    CScopedTracer test_info;
    uint32_t ssrc = 111111;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    uint32_t tmmbrSSRC = 123456;
    CRTCPHandlerSinkTMMBR sink;
    std::unique_ptr<CRTCPHandlerTMMBR> rtcp(new CRTCPHandlerTMMBR(m_session->GetSessionContext()));
    rtcp->SetSink(&sink);
    rtcp->ConfigTMMBR(true, true);
    rtcp->UpdateQoSTBR(3*1024*1024);

    RTPPacketSendInfo info;
    info.ssrc = (ssrc);
    info.packetLength = (1029);
    info.clockRate = (90000);
    info.rtpTimestamp = (38578353);
    info.captureTick = (238472);
    info.sequence = (3398);
    streamPtr->GetSendStats().NotifyRTPPacketSent(info, TickNowMS(), false);
    rtcp->NotifyRTPPacketSend(info, TickNowMS());

    CRTPFBPacket packet;
    TMMBRVECTOR items;
    TMMBRTuple item;
    item.ssrc = ssrc;
    item.maxTBR = 2*1024*1024;
    item.oh = 0;
    items.push_back(item);
    EXPECT_TRUE(!!EncodeTMMBRPacket(tmmbrSSRC, RTCP_RTPFB_TMMBR, items, packet));
    EXPECT_EQ(packet.m_fciLength, 8);
    items.clear();

    CCmMessageBlock mb(packet.CalcEncodeSize());
    CCmByteStreamNetwork os(mb);
    EXPECT_TRUE(!!packet.Encode(os, mb));

    rtcp->RecvRTCPPacket(mb);
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_NE(0, sink.bandwidth);
    EXPECT_EQ(item.maxTBR/8, sink.bandwidth);
#endif
    EXPECT_NE(0, sink.streamBandwidth);
    /*
    int loop = 4;
    while(loop-- > 0){
    if(sink.tmmbn != nullptr){
        break;
    } else {
        ::SleepMs(250);
    }
    }
    */
    ASSERT_TRUE(sink.tmmbn != nullptr);
    CRTPFBPacket *tmmbnPacket = sink.tmmbn;
    ASSERT_EQ(rtcp->GetSSRC(), tmmbnPacket->m_ssrc);
    ASSERT_EQ(0, tmmbnPacket->m_ssrcSrc);
    ASSERT_EQ(8, tmmbnPacket->m_fciLength);

    items.clear();
    EXPECT_TRUE(!!DecodeTMMBRFCI(*tmmbnPacket, items));
    EXPECT_EQ(1, items.size());
    EXPECT_EQ(tmmbrSSRC, items[0].ssrc);
    EXPECT_EQ(item.maxTBR, items[0].maxTBR);

    sink.tmmbn->ReleaseReference();
    sink.tmmbn = nullptr;
    // update QoS bandwidth
    rtcp->UpdateQoSTBR(1*1024*1024);

    /*
    loop = 4;
    while(loop-- > 0){
        if(sink.tmmbn != nullptr){
            break;
        } else {
            ::SleepMs(250);
        }
    }
     */
    ASSERT_TRUE(sink.tmmbn != nullptr);
    tmmbnPacket = sink.tmmbn;
    ASSERT_EQ(rtcp->GetSSRC(), tmmbnPacket->m_ssrc);
    ASSERT_EQ(0, tmmbnPacket->m_ssrcSrc);
    ASSERT_EQ(8, tmmbnPacket->m_fciLength);

    items.clear();
    EXPECT_TRUE(!!DecodeTMMBRFCI(*tmmbnPacket, items));
    EXPECT_EQ(1, items.size());
    EXPECT_EQ(rtcp->GetSSRC(), items[0].ssrc);
    EXPECT_EQ(1*1024*1024, items[0].maxTBR);
}

TEST_F(CTMMBRTest, TestPacketRateStat_1)
{
    CScopedTracer test_info;
    PacketRateStats packetRateS;
    uint32_t tick = 12345;
    packetRateS.AddPackets(5, tick);
    //SleepMs(100);
    tick += 100;
    packetRateS.AddPackets(5, tick);
    //SleepMs(100);
    tick += 100;
    packetRateS.AddPackets(5, tick);
    //SleepMs(20);
    tick += 20;
    packetRateS.AddPackets(5, tick);
    //SleepMs(100);
    tick += 100;
    packetRateS.AddPackets(5, tick);
    //SleepMs(700);
    tick += 700;
    packetRateS.AddPackets(1, tick);

    EXPECT_EQ(25, packetRateS.GetPacketRate());
}

TEST_F(CTMMBRTest, TestPacketRateStat_3)
{
    CScopedTracer test_info;
    PacketRateStats packetRateS;
    uint32_t tick = 45678;
    packetRateS.AddPackets(30, tick);
    //SleepMs(1100);
    tick += 1100;
    packetRateS.AddPackets(1, tick);
    EXPECT_TRUE(packetRateS.GetPacketRate() <= 28);
    packetRateS.AddPackets(50, tick);
    //SleepMs(1020);
    tick += 1020;
    packetRateS.AddPackets(1, tick);
    EXPECT_TRUE(packetRateS.GetPacketRate() <= 29);
    packetRateS.AddPackets(80, tick);
    //SleepMs(1050);
    tick += 1050;
    packetRateS.AddPackets(1, tick);

    EXPECT_TRUE(packetRateS.GetPacketRate() <= 32);
}

TEST_F(CTMMBRTest, TestTMMBR_multi_tuples)
{
    CScopedTracer test_info;
    uint32_t ssrc = 111111;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    uint32_t tmmbrSSRC = 123456;
    CRTCPHandlerSinkTMMBR sink;
    std::unique_ptr<CRTCPHandlerTMMBR> rtcp(new CRTCPHandlerTMMBR(m_session->GetSessionContext()));
    rtcp->SetSink(&sink);
    rtcp->ConfigTMMBR(true, true);
    rtcp->SetOverhead(28);
    rtcp->UpdateQoSTBR(2*1024*1024);

    RTPPacketSendInfo info;
    info.ssrc = (ssrc);
    info.packetLength = (1029);
    info.clockRate = (90000);
    info.rtpTimestamp = (38578353);
    info.captureTick = (238472);
    info.sequence = (3398);
    streamPtr->GetSendStats().NotifyRTPPacketSent(info, TickNowMS(), false);
    rtcp->NotifyRTPPacketSend(info, TickNowMS());

    TMMBRVECTOR tuples;
    TMMBRTuple tuple;
    tuple.ssrc = ssrc;
    tuple.maxTBR = 2*1024*1024;
    tuple.oh = 26;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = (uint64_t)(1.5*1024*1024);
    tuple.oh = 40;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = (uint64_t)(2.5*1024*1024);
    tuple.oh = 28;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = 3*1024*1024;
    tuple.oh = 12;
    tuples.push_back(tuple);
    tuple.ssrc = ssrc;
    tuple.maxTBR = 1*1024*1024;
    tuple.oh = 15;
    tuples.push_back(tuple);
    tuple.ssrc = ssrc;
    tuple.maxTBR = (uint64_t)(1.2*1024*1024);
    tuple.oh = 32;
    tuples.push_back(tuple);

    CRTPFBPacket packet;
    EXPECT_TRUE(!!EncodeTMMBRPacket(tmmbrSSRC, RTCP_RTPFB_TMMBR, tuples, packet));
    EXPECT_EQ(packet.m_fciLength, 8*tuples.size());
    CCmMessageBlock mb(packet.CalcEncodeSize());
    CCmByteStreamNetwork os(mb);
    EXPECT_TRUE(!!packet.Encode(os, mb));

    rtcp->RecvRTCPPacket(mb);
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_NE(0, sink.bandwidth);
#endif
    EXPECT_NE(0, sink.streamBandwidth);

    /*
    int loop = 4;
    while(loop-- > 0){
        if(sink.tmmbn != nullptr){
            break;
        } else {
            ::SleepMs(250);
        }
    }
     */
    ASSERT_TRUE(sink.tmmbn != nullptr);
    CRTPFBPacket *tmmbnPacket = sink.tmmbn;
    ASSERT_EQ(rtcp->GetSSRC(), tmmbnPacket->m_ssrc);
    ASSERT_EQ(0, tmmbnPacket->m_ssrcSrc);
    ASSERT_EQ(8*rtcp->m_tmmbrState.GetBoundingSet().size(), tmmbnPacket->m_fciLength);

    TMMBRVECTOR tuples2;
    EXPECT_TRUE(!!DecodeTMMBRFCI(*tmmbnPacket, tuples2));
    EXPECT_EQ(rtcp->m_tmmbrState.GetBoundingSet().size(), tuples2.size());
}

TEST_F(CTMMBRTest, TestTMMBR_multi_tuples_and_update)
{
    CScopedTracer test_info;
    uint32_t ssrc = 111111;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    uint32_t tmmbrSSRC = 123456;
    CRTCPHandlerSinkTMMBR sink;
    std::unique_ptr<CRTCPHandlerTMMBR> rtcp(new CRTCPHandlerTMMBR(m_session->GetSessionContext()));
    rtcp->SetSink(&sink);
    rtcp->ConfigTMMBR(true, true);
    rtcp->SetOverhead(28);
    rtcp->UpdateQoSTBR(2*1024*1024);

    RTPPacketSendInfo info;
    info.ssrc = (ssrc);
    info.packetLength = (1029);
    info.clockRate = (90000);
    info.rtpTimestamp = (38578353);
    info.captureTick = (238472);
    info.sequence = (3398);
    streamPtr->GetSendStats().NotifyRTPPacketSent(info, TickNowMS(), false);
    rtcp->NotifyRTPPacketSend(info, TickNowMS());

    TMMBRVECTOR tuples;
    TMMBRTuple tuple;
    tuple.ssrc = ssrc;
    tuple.maxTBR = 2*1024*1024;
    tuple.oh = 26;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = (uint64_t)(1.5*1024*1024);
    tuple.oh = 40;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = (uint64_t)(2.5*1024*1024);
    tuple.oh = 29;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = 3*1024*1024;
    tuple.oh = 12;
    tuples.push_back(tuple);
    tuple.ssrc = ssrc;
    tuple.maxTBR = 1*1024*1024;
    tuple.oh = 15;
    tuples.push_back(tuple);
    tuple.ssrc = ssrc;
    tuple.maxTBR = (uint64_t)(1.2*1024*1024);
    tuple.oh = 32;
    tuples.push_back(tuple);

    CRTPFBPacket packet;
    EXPECT_TRUE(!!EncodeTMMBRPacket(tmmbrSSRC, RTCP_RTPFB_TMMBR, tuples, packet));
    EXPECT_EQ(packet.m_fciLength, 8*tuples.size());
    CCmMessageBlock mb(packet.CalcEncodeSize());
    CCmByteStreamNetwork os(mb);
    EXPECT_TRUE(!!packet.Encode(os, mb));

    rtcp->RecvRTCPPacket(mb);
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_NE(0, sink.bandwidth);
#endif
    EXPECT_NE(0, sink.streamBandwidth);

    /*
    int loop = 4;
    while(loop-- > 0){
        if(sink.tmmbn != nullptr){
            break;
        } else {
            ::SleepMs(250);
        }
    }
     */

    ASSERT_TRUE(sink.tmmbn != nullptr);
    CRTPFBPacket *tmmbnPacket = sink.tmmbn;
    ASSERT_EQ(rtcp->GetSSRC(), tmmbnPacket->m_ssrc);
    ASSERT_EQ(0, tmmbnPacket->m_ssrcSrc);
    ASSERT_EQ(8*rtcp->m_tmmbrState.GetBoundingSet().size(), tmmbnPacket->m_fciLength);

    TMMBRVECTOR tuples2;
    EXPECT_TRUE(!!DecodeTMMBRFCI(*tmmbnPacket, tuples2));
    EXPECT_EQ(rtcp->m_tmmbrState.GetBoundingSet().size(), tuples2.size());

    //////////////////////////////////////////////////////////////////////////
    tuples.clear();
    tuple.ssrc = ssrc;
    tuple.maxTBR = uint64_t(1.3*1024*1024);
    tuple.oh = 26;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = (uint64_t)(1.5*1024*1024);
    tuple.oh = 40;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = (uint64_t)(2.5*1024*1024);
    tuple.oh = 29;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.ssrc = ssrc;
    tuple.maxTBR = uint64_t(1.8*1024*1024);
    tuple.oh = 15;
    tuples.push_back(tuple);

    CRTPFBPacket packet2;
    EXPECT_TRUE(!!EncodeTMMBRPacket(tmmbrSSRC, RTCP_RTPFB_TMMBR, tuples, packet2));
    EXPECT_EQ(packet2.m_fciLength, 8*tuples.size());
    CCmMessageBlock mb2(packet2.CalcEncodeSize());
    CCmByteStreamNetwork os2(mb2);
    EXPECT_TRUE(!!packet2.Encode(os2, mb2));

    rtcp->RecvRTCPPacket(mb2);
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_NE(0, sink.bandwidth);
#endif
    EXPECT_NE(0, sink.streamBandwidth);
    /*
    loop = 4;
    while(loop-- > 0){
        if(sink.tmmbn != nullptr){
            break;
        } else {
            ::SleepMs(250);
        }
    }
     */
    ASSERT_TRUE(sink.tmmbn != nullptr);
    tmmbnPacket = sink.tmmbn;
    ASSERT_EQ(rtcp->GetSSRC(), tmmbnPacket->m_ssrc);
    ASSERT_EQ(0, tmmbnPacket->m_ssrcSrc);
    ASSERT_EQ(8*rtcp->m_tmmbrState.GetBoundingSet().size(), tmmbnPacket->m_fciLength);

    tuples2.clear();
    EXPECT_TRUE(!!DecodeTMMBRFCI(*tmmbnPacket, tuples2));
    EXPECT_EQ(rtcp->m_tmmbrState.GetBoundingSet().size(), tuples2.size());
}

TEST_F(CTMMBRTest, TestTMMBR_multi_tuples_and_increment)
{
    CScopedTracer test_info;
    uint32_t ssrc = 111111;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    uint32_t tmmbrSSRC = 123456;
    uint32_t tmmbrSSRC2 = 654321;
    CRTCPHandlerSinkTMMBR sink;
    std::unique_ptr<CRTCPHandlerTMMBR> rtcp(new CRTCPHandlerTMMBR(m_session->GetSessionContext()));
    rtcp->SetSink(&sink);
    rtcp->ConfigTMMBR(true, true);
    rtcp->SetOverhead(28);
    rtcp->UpdateQoSTBR(2*1024*1024);

    RTPPacketSendInfo info;
    info.ssrc = (ssrc);
    info.packetLength = (1029);
    info.clockRate = (90000);
    info.rtpTimestamp = (38578353);
    info.captureTick = (238472);
    info.sequence = (3398);
    streamPtr->GetSendStats().NotifyRTPPacketSent(info, TickNowMS(), false);
    rtcp->NotifyRTPPacketSend(info, TickNowMS());

    TMMBRVECTOR tuples;
    TMMBRTuple tuple;
    tuple.ssrc = ssrc;
    tuple.maxTBR = 2*1024*1024;
    tuple.oh = 26;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = (uint64_t)(1.5*1024*1024);
    tuple.oh = 40;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = (uint64_t)(2.5*1024*1024);
    tuple.oh = 29;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = 3*1024*1024;
    tuple.oh = 12;
    tuples.push_back(tuple);
    tuple.ssrc = ssrc;
    tuple.maxTBR = 1*1024*1024;
    tuple.oh = 15;
    tuples.push_back(tuple);
    tuple.ssrc = ssrc;
    tuple.maxTBR = (uint64_t)(1.2*1024*1024);
    tuple.oh = 32;
    tuples.push_back(tuple);

    CRTPFBPacket packet;
    EXPECT_TRUE(!!EncodeTMMBRPacket(tmmbrSSRC, RTCP_RTPFB_TMMBR, tuples, packet));
    EXPECT_EQ(packet.m_fciLength, 8*tuples.size());
    CCmMessageBlock mb(packet.CalcEncodeSize());
    CCmByteStreamNetwork os(mb);
    EXPECT_TRUE(!!packet.Encode(os, mb));

    rtcp->RecvRTCPPacket(mb);
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_NE(0, sink.bandwidth);
#endif
    EXPECT_NE(0, sink.streamBandwidth);

    /*
    int loop = 4;
    while(loop-- > 0){
        if(sink.tmmbn != nullptr){
            break;
        } else {
            ::SleepMs(250);
        }
    }
     */
    ASSERT_TRUE(sink.tmmbn != nullptr);
    CRTPFBPacket *tmmbnPacket = sink.tmmbn;
    ASSERT_EQ(rtcp->GetSSRC(), tmmbnPacket->m_ssrc);
    ASSERT_EQ(0, tmmbnPacket->m_ssrcSrc);
    ASSERT_EQ(8*rtcp->m_tmmbrState.GetBoundingSet().size(), tmmbnPacket->m_fciLength);

    TMMBRVECTOR tuples2;
    EXPECT_TRUE(!!DecodeTMMBRFCI(*tmmbnPacket, tuples2));
    EXPECT_EQ(rtcp->m_tmmbrState.GetBoundingSet().size(), tuples2.size());

    //////////////////////////////////////////////////////////////////////////
    tuples.clear();
    tuple.ssrc = ssrc;
    tuple.maxTBR = uint64_t(1.3*1024*1024);
    tuple.oh = 26;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = (uint64_t)(1.5*1024*1024);
    tuple.oh = 40;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = (uint64_t)(2.5*1024*1024);
    tuple.oh = 29;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.ssrc = ssrc;
    tuple.maxTBR = uint64_t(1.8*1024*1024);
    tuple.oh = 15;
    tuples.push_back(tuple);

    CRTPFBPacket packet2;
    EXPECT_TRUE(!!EncodeTMMBRPacket(tmmbrSSRC2, RTCP_RTPFB_TMMBR, tuples, packet2));
    EXPECT_EQ(packet2.m_fciLength, 8*tuples.size());
    CCmMessageBlock mb2(packet2.CalcEncodeSize());
    CCmByteStreamNetwork os2(mb2);
    EXPECT_TRUE(!!packet2.Encode(os2, mb2));

    rtcp->RecvRTCPPacket(mb2);
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_NE(0, sink.bandwidth);
#endif
    EXPECT_NE(0, sink.streamBandwidth);

    /*
    loop = 4;
    while(loop-- > 0){
        if(sink.tmmbn != nullptr){
            break;
        } else {
            ::SleepMs(250);
        }
    }
     */

    ASSERT_TRUE(sink.tmmbn != nullptr);
    tmmbnPacket = sink.tmmbn;
    ASSERT_EQ(rtcp->GetSSRC(), tmmbnPacket->m_ssrc);
    ASSERT_EQ(0, tmmbnPacket->m_ssrcSrc);
    ASSERT_EQ(8*rtcp->m_tmmbrState.GetBoundingSet().size(), tmmbnPacket->m_fciLength);

    tuples2.clear();
    EXPECT_TRUE(!!DecodeTMMBRFCI(*tmmbnPacket, tuples2));
    EXPECT_EQ(rtcp->m_tmmbrState.GetBoundingSet().size(), tuples2.size());
}

TEST_F(CTMMBRTest, TestTMMBR_multi_tuples_and_increment_and_iamowner)
{
    CScopedTracer test_info;
    uint32_t ssrc = 111111;
    CRTPStreamPtr streamPtr = m_session->CreateRTPStream(ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);
    uint32_t tmmbrSSRC = 123456;
    uint32_t tmmbrSSRC2 = 654321;
    CRTCPHandlerSinkTMMBR sink;
    std::unique_ptr<CRTCPHandlerTMMBR> rtcp(new CRTCPHandlerTMMBR(m_session->GetSessionContext()));
    rtcp->SetSink(&sink);
    rtcp->ConfigTMMBR(true, true);
    rtcp->SetOverhead(28);
    rtcp->UpdateQoSTBR(2*1024*1024);

    RTPPacketSendInfo info;
    info.ssrc = (ssrc);
    info.packetLength = (1029);
    info.clockRate = (90000);
    info.rtpTimestamp = (38578353);
    info.captureTick = (238472);
    info.sequence = (3398);
    streamPtr->GetSendStats().NotifyRTPPacketSent(info, TickNowMS(), false);
    rtcp->NotifyRTPPacketSend(info, TickNowMS());

    TMMBRVECTOR tuples;
    TMMBRTuple tuple;
    tuple.ssrc = ssrc;
    tuple.maxTBR = 2*1024*1024;
    tuple.oh = 26;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = (uint64_t)(1.5*1024*1024);
    tuple.oh = 40;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = (uint64_t)(2.5*1024*1024);
    tuple.oh = 29;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = 3*1024*1024;
    tuple.oh = 12;
    tuples.push_back(tuple);
    tuple.ssrc = ssrc;
    tuple.maxTBR = 1*1024*1024;
    tuple.oh = 15;
    tuples.push_back(tuple);
    tuple.ssrc = ssrc;
    tuple.maxTBR = (uint64_t)(1.2*1024*1024);
    tuple.oh = 32;
    tuples.push_back(tuple);

    CRTPFBPacket packet;
    EXPECT_TRUE(!!EncodeTMMBRPacket(tmmbrSSRC, RTCP_RTPFB_TMMBR, tuples, packet));
    EXPECT_EQ(packet.m_fciLength, 8*tuples.size());
    CCmMessageBlock mb(packet.CalcEncodeSize());
    CCmByteStreamNetwork os(mb);
    EXPECT_TRUE(!!packet.Encode(os, mb));

    rtcp->RecvRTCPPacket(mb);
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_NE(0, sink.bandwidth);
#endif
    EXPECT_NE(0, sink.streamBandwidth);

    /*
    int loop = 4;
    while(loop-- > 0){
        if(sink.tmmbn != nullptr){
            break;
        } else {
            ::SleepMs(250);
        }
    }
     */

    ASSERT_TRUE(sink.tmmbn != nullptr);
    CRTPFBPacket *tmmbnPacket = sink.tmmbn;
    ASSERT_EQ(rtcp->GetSSRC(), tmmbnPacket->m_ssrc);
    ASSERT_EQ(0, tmmbnPacket->m_ssrcSrc);
    ASSERT_EQ(8*rtcp->m_tmmbrState.GetBoundingSet().size(), tmmbnPacket->m_fciLength);

    TMMBRVECTOR tuples2;
    EXPECT_TRUE(!!DecodeTMMBRFCI(*tmmbnPacket, tuples2));
    EXPECT_EQ(rtcp->m_tmmbrState.GetBoundingSet().size(), tuples2.size());

    //////////////////////////////////////////////////////////////////////////
    rtcp->SetOverhead(39);
    rtcp->UpdateQoSTBR(uint64_t(1.2*1024*1024));

    tuples.clear();
    tuple.ssrc = ssrc;
    tuple.maxTBR = uint64_t(1.3*1024*1024);
    tuple.oh = 26;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = (uint64_t)(1.5*1024*1024);
    tuple.oh = 40;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.maxTBR = (uint64_t)(2.5*1024*1024);
    tuple.oh = 29;
    tuples.push_back(tuple);
    tuple.ssrc = rtcp->GetSSRC();
    tuple.ssrc = ssrc;
    tuple.maxTBR = uint64_t(1.8*1024*1024);
    tuple.oh = 15;
    tuples.push_back(tuple);

    CRTPFBPacket packet2;
    EXPECT_TRUE(!!EncodeTMMBRPacket(tmmbrSSRC2, RTCP_RTPFB_TMMBR, tuples, packet2));
    EXPECT_EQ(packet2.m_fciLength, 8*tuples.size());
    CCmMessageBlock mb2(packet2.CalcEncodeSize());
    CCmByteStreamNetwork os2(mb2);
    EXPECT_TRUE(!!packet2.Encode(os2, mb2));

    rtcp->RecvRTCPPacket(mb2);
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_NE(0, sink.bandwidth);
#endif
    EXPECT_NE(0, sink.streamBandwidth);

    /*
    loop = 4;
    while(loop-- > 0){
        if(sink.tmmbn != nullptr){
            break;
        } else {
            ::SleepMs(250);
        }
    }
     */

    ASSERT_TRUE(sink.tmmbn != nullptr);
    tmmbnPacket = sink.tmmbn;

    EXPECT_TRUE(rtcp->m_tmmbrState.IsOwner());
    ASSERT_EQ(rtcp->GetSSRC(), tmmbnPacket->m_ssrc);
    ASSERT_EQ(0, tmmbnPacket->m_ssrcSrc);
    ASSERT_EQ(8*rtcp->m_tmmbrState.GetBoundingSet().size(), tmmbnPacket->m_fciLength);

    tuples2.clear();
    EXPECT_TRUE(!!DecodeTMMBRFCI(*tmmbnPacket, tuples2));
    EXPECT_EQ(rtcp->m_tmmbrState.GetBoundingSet().size(), tuples2.size());
}

bool DecodeCompoundPacket(CCmMessageBlock &mb, CCompoundPacket &compoundPacket)
{
    CCmByteStreamNetwork is(mb);
    do {
        CRTCPPacket *packet = DecodeRTCPPacket(is, mb);
        if (nullptr == packet) {
            return false;
        }
        compoundPacket.AddRtcpPacket(packet);
    } while (mb.GetChainedLength() > 0);
    return true;
}
