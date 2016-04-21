//
//  RTPVideoRecvFrameTest.cpp
//  wrtpTest_mac
//
//  Created by Hank Peng on 8/24/15.
//  Copyright (c) 2015 cisco. All rights reserved.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../testutil.h"

#include "rtputils.h"
#include "wrtppacket.h"
#include "videorecvframe.h"

#include <vector>
#include <random>
#include <chrono>
#include <memory>
#include <functional>
#include <algorithm>

using namespace wrtp;

class CRTPVideoRecvFrameTest : public ::testing::Test
{
public:
    CRTPVideoRecvFrameTest() : m_packetMgr("CVideoRecvFramePacket"), m_frameMgr("CRTPVideoRecvFrame") {}
protected:
    virtual void SetUp() {}

    virtual void TearDown() {}

protected:
    CVideoRecvFramePacket::ObjectManager    m_packetMgr;
    CRTPVideoRecvFrame::ObjectManager       m_frameMgr;
};

TEST_F(CRTPVideoRecvFrameTest, Empty_Frame_Test)
{
    CScopedTracer test_info;

    const uint32_t rtpTS = 12345;
    CRecvFrameUniquePtr frame(m_frameMgr.GetObjectUniquePtr());
    frame->Init(rtpTS);
    ASSERT_EQ(rtpTS, frame->GetRTPTimestamp());

    //auto range = frame->GetMinMaxSeqRange();
    //ASSERT_TRUE(isSequenceGT(range.first, range.second));

    uint32_t nowTickMS    = TickNowMS();
    FramePopCause cause;
    bool popping        = frame->CanPopout(nowTickMS, cause);
    ASSERT_FALSE(popping);

    bool complete       = frame->IsFrameComplete();
    ASSERT_FALSE(complete);
}

TEST_F(CRTPVideoRecvFrameTest, Complete_Frame_Test)
{
    CScopedTracer test_info;

    const uint32_t rtpTS      = 12345;
    uint32_t nowTick          = 10000;
    CRecvFrameUniquePtr frame(m_frameMgr.GetObjectUniquePtr());
    frame->Init(rtpTS);

    const uint16_t sequenceBase   = 100;
    const uint32_t packetCount    = 10;
    for (int ii = 0; ii < packetCount; ++ii) {
        CRTPPacketAuto rtp(new CRTPPacket());
        uint16_t sequence             = sequenceBase + ii;
        bool marked                 = (ii == packetCount - 1) ? true : false;
        
        CRecvPacketUniquePtr pkt(m_packetMgr.GetObjectUniquePtr());
        pkt->Init(std::move(rtp), sequence, rtpTS, marked);
        nowTick += ii;
        frame->PushPacket(std::move(pkt), nowTick);
    }

    //auto range = frame.GetMinMaxSeqRange();
    uint16_t minSeq = frame->GetMinSequence();
    uint16_t maxSeq = frame->GetMaxSequence();
    ASSERT_EQ(sequenceBase, minSeq);
    ASSERT_EQ(sequenceBase + packetCount - 1, maxSeq);

    bool complete = frame->IsFrameComplete();
    ASSERT_TRUE(complete);

    bool popping = false;
    FramePopCause cause;
    nowTick += 1;
    popping = frame->CanPopout(nowTick, cause);
    ASSERT_TRUE(popping);
    ASSERT_TRUE(FramePopCause::FRAME_POP_COMPLETE == cause);
}

TEST_F(CRTPVideoRecvFrameTest, Incomplete_Frame_Test)
{
    CScopedTracer test_info;

    const uint32_t rtpTS      = 12345;
    uint32_t nowTick          = 10000;
    CRecvFrameUniquePtr frame(m_frameMgr.GetObjectUniquePtr());
    frame->Init(rtpTS);

    const uint16_t sequenceBase   = 100;
    const uint32_t packetCount    = 10;
    for (int ii = 0; ii < packetCount; ++ii) {
        uint16_t sequence             = sequenceBase + ii;
        if (sequence % 3 == 0) {
            continue;
        }

        CRTPPacketAuto rtp(new CRTPPacket());
        bool marked                 = (ii == packetCount - 1) ? true : false;
        CRecvPacketUniquePtr pkt(m_packetMgr.GetObjectUniquePtr());
        pkt->Init(std::move(rtp), sequence, rtpTS, marked);

        nowTick += ii;
        frame->PushPacket(std::move(pkt), nowTick);
    }

    //auto range = frame.GetMinMaxSeqRange();
    uint16_t minSeq = frame->GetMinSequence();
    uint16_t maxSeq = frame->GetMaxSequence();
    ASSERT_EQ(sequenceBase, minSeq);
    ASSERT_EQ(sequenceBase + packetCount - 1, maxSeq);

    bool complete = frame->IsFrameComplete();
    ASSERT_FALSE(complete);

    bool popping = false;
    FramePopCause cause;
    nowTick += frame->GetMaxIntervalSinceLastPkt() - 1;
    popping = frame->CanPopout(nowTick, cause);
    ASSERT_FALSE(popping);

    nowTick += frame->GetMaxIntervalSinceFirstPkt();
    popping = frame->CanPopout(nowTick, cause);
    ASSERT_TRUE(popping);
    ASSERT_TRUE(FramePopCause::FRAME_POP_TIMEOUT == cause);
}

TEST_F(CRTPVideoRecvFrameTest, Packet_Reorder_Test)
{
    CScopedTracer test_info;

    const uint32_t rtpTS      = 12345;
    uint32_t nowTick          = 10000;
    CRecvFrameUniquePtr frame(m_frameMgr.GetObjectUniquePtr());
    frame->Init(rtpTS);

    std::vector<uint16_t> sequences;
    // Generate 100 random numbers
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator((unsigned int)seed);
    std::uniform_int_distribution<uint16_t> distribution(1, 200);
    for (uint16_t ii = 0; ii < 100; ++ii) {
        sequences.push_back(distribution(generator));
    }

    for (auto seq : sequences) {
        CRTPPacketAuto rtp(new CRTPPacket());
        CRecvPacketUniquePtr pkt(m_packetMgr.GetObjectUniquePtr());
        pkt->Init(std::move(rtp), seq, rtpTS, false);
        frame->PushPacket(std::move(pkt), nowTick);
    }

    // Check the packet sequence
    std::sort(sequences.begin(), sequences.end());
    auto last = std::unique(sequences.begin(), sequences.end());
    sequences.erase(last, sequences.end());

    auto& processPackets = frame->GetRtpPackets();
    ASSERT_EQ(sequences.size(), processPackets.size());

    auto iter1 = sequences.begin();
    auto iter2 = processPackets.begin();
    while (iter1 != sequences.end()) {
        ASSERT_EQ(*iter1, (*iter2)->GetSequence());
        ++ iter1;
        ++ iter2;
    }
}

TEST_F(CRTPVideoRecvFrameTest, Packet_Reorder_With_Sequence_Rollover_Test)
{
    CScopedTracer test_info;

    const uint32_t rtpTS      = 12345;
    uint32_t nowTick          = 10000;
    CRecvFrameUniquePtr frame(m_frameMgr.GetObjectUniquePtr());
    frame->Init(rtpTS);

    std::vector<uint16_t> sequences;
    // Generate 100 random numbers
    auto seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator((unsigned int)seed);
    std::uniform_int_distribution<uint16_t> distribution(1, 200);
    for (uint16_t ii = 0; ii < 100; ++ii) {
        sequences.push_back(distribution(generator));
    }

    const uint16_t Offset = (65535 - 100);

    for (auto seq : sequences) {
        CRTPPacketAuto rtp(new CRTPPacket());
        // Make sequence roll over by adding an offset
        CRecvPacketUniquePtr pkt(m_packetMgr.GetObjectUniquePtr());
        pkt->Init(std::move(rtp), seq + Offset, rtpTS, false);
        frame->PushPacket(std::move(pkt), nowTick);
    }

    // Check the packet sequence
    std::sort(sequences.begin(), sequences.end());
    auto last = std::unique(sequences.begin(), sequences.end());
    sequences.erase(last, sequences.end());

    // Make sequence roll over by adding an offset
    std::for_each(sequences.begin(), sequences.end(), [Offset](uint16_t &seq) { seq += Offset; });

    auto& processPackets = frame->GetRtpPackets();
    ASSERT_EQ(sequences.size(), processPackets.size());

    auto iter1 = sequences.begin();
    auto iter2 = processPackets.begin();
    while (iter1 != sequences.end()) {
        ASSERT_EQ(*iter1, (*iter2)->GetSequence());
        ++ iter1;
        ++ iter2;
    }
}