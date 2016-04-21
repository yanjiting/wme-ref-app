//
//  RTPVideoRecvFrameManagerTest.cpp
//  wrtpTest_mac
//
//  Created by Hank Peng on 8/25/15.
//  Copyright (c) 2015 cisco. All rights reserved.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../testutil.h"

#include "wrtppacket.h"
#include "videorecvframe.h"

#include <array>
#include <vector>
#include <random>
#include <memory>
#include <functional>
#include <algorithm>

using namespace wrtp;

class CRTPVideoRecvFrameManagerTest : public ::testing::Test
{
public:
    CRTPVideoRecvFrameManagerTest()
        : m_frameManager()
        , m_maxFrameCountWithinManager(10)
        , m_poppedFrames()
    {}

    void HandlePoppedFrame(CRTPVideoRecvFrameUniquePtr &&frame)
    {
        //std::cout << frame->GetFramePacketInfo() << std::endl;
        m_poppedFrames.push_back(std::move(frame));
    }

protected:
    virtual void SetUp()
    {
        auto frameHandler = [this](CRTPVideoRecvFrameUniquePtr &&frame) { HandlePoppedFrame(std::move(frame));  };
        m_frameManager.reset(new CRTPVideoRecvFrameManager(m_maxFrameCountWithinManager, frameHandler));
        m_poppedFrames.clear();
    }

    virtual void TearDown() {}

protected:
    std::unique_ptr<CRTPVideoRecvFrameManager>  m_frameManager;
    const uint32_t                                m_maxFrameCountWithinManager;
    std::vector<CRTPVideoRecvFrameUniquePtr>     m_poppedFrames;
};


struct RtpTS_Seq {
    uint32_t rtpTS;
    uint16_t seq;
};

TEST_F(CRTPVideoRecvFrameManagerTest, Single_Complete_Frame_Test)
{
    CScopedTracer test_info;

    std::vector<uint16_t> sequences;
    const uint16_t MinSeq = 100;
    const uint16_t MaxSeq = 120;
    for (uint16_t ii = MinSeq; ii <= MaxSeq; ++ii) {
        sequences.push_back(ii);
    }

    std::random_device rd;
    std::mt19937 g(rd());
    // Skip the first seqeunce in case that 100 would be the last one
    // The case will fail if 100 is the last
    std::shuffle(std::next(sequences.begin()), sequences.end(), g);

    uint32_t nowTickMS    = 10000;
    uint32_t rtpTS        = 20000;
    for (auto seq : sequences) {
        CRTPPacketAuto rtp(new CRTPPacket());
        rtp->SetSequenceNumber(seq);
        rtp->SetTimestamp(rtpTS);
        if (seq == MaxSeq) {
            rtp->SetMarker();
        }
        m_frameManager->PushPacket(std::move(rtp), nowTickMS);
    }

    ASSERT_EQ(1, m_frameManager->GetPoppedFrameCount());
    ASSERT_EQ(1, m_frameManager->GetPoppedFrameCount(FramePopCause::FRAME_POP_COMPLETE));

    ASSERT_EQ(0, m_frameManager->GetDroppedPacketCount());
    ASSERT_EQ(MaxSeq - MinSeq + 1, m_frameManager->GetProcessedPacketCount());

    ASSERT_EQ(1, m_poppedFrames.size());
    const auto &frame = m_poppedFrames.front();

    ASSERT_EQ(rtpTS, frame->GetRTPTimestamp());
    ASSERT_TRUE(frame->IsFrameComplete());

    auto& frameInfo = frame->GetFrameInfo();
    ASSERT_EQ(rtpTS, frameInfo->GetRTPTimestamp());

    ASSERT_EQ(MinSeq, *frameInfo->GetMinSequence());
    ASSERT_EQ(MaxSeq, *frameInfo->GetMaxSequence());

    ASSERT_EQ(MinSeq, *frameInfo->GetStartSequence());
    ASSERT_EQ(MaxSeq, *frameInfo->GetEndSequence());
}

TEST_F(CRTPVideoRecvFrameManagerTest, Two_Complete_Frames_Without_Marker_Test)
{
    CScopedTracer test_info;

    const uint32_t RtpTS1 = 30 * 90;
    const uint32_t RtpTS2 = 10 * 90;
    const uint32_t RtpTS3 = 20 * 90;

    // 8, 9, 10 | 11, 12, 13 | 14, 15, 16
    std::vector<RtpTS_Seq> pktInfos{
        {RtpTS1,  9},       // 1
        {RtpTS1,  8},       // 1

        {RtpTS2, 11},       // 2
        {RtpTS2, 13},       // 2

        {RtpTS1, 10},       // 1

        {RtpTS3, 16},       // 3

        {RtpTS2, 12},       // 2

        {RtpTS3, 15},       // 3
        {RtpTS3, 14},       // 3
    };

    // Feed in the RTP packets
    uint32_t nowTickMS = 123456;
    for (const auto &info : pktInfos) {
        CRTPPacketAuto rtp(new CRTPPacket());
        rtp->SetSequenceNumber(info.seq);
        rtp->SetTimestamp(info.rtpTS);
        m_frameManager->PushPacket(std::move(rtp), nowTickMS);
    }

    // Check the manager statistics
    ASSERT_EQ(2, m_frameManager->GetPoppedFrameCount());
    ASSERT_EQ(2, m_frameManager->GetPoppedFrameCount(FramePopCause::FRAME_POP_COMPLETE));

    ASSERT_EQ(0, m_frameManager->GetDroppedPacketCount());
    ASSERT_EQ(pktInfos.size(), m_frameManager->GetProcessedPacketCount());

    ASSERT_EQ(2, m_poppedFrames.size());

    // Check the first popped frame
    auto &frame = m_poppedFrames[0];


    ASSERT_EQ(RtpTS1, frame->GetRTPTimestamp());
    ASSERT_TRUE(frame->IsFrameComplete());

    auto frameInfo = frame->GetFrameInfo();
    ASSERT_EQ(RtpTS1, frameInfo->GetRTPTimestamp());

    ASSERT_EQ(8,  *frameInfo->GetMinSequence());
    ASSERT_EQ(10, *frameInfo->GetMaxSequence());

    ASSERT_EQ(8,  *frameInfo->GetStartSequence());
    ASSERT_EQ(10, *frameInfo->GetEndSequence());

    // Check the second popped frame
    auto &frame2 = m_poppedFrames[1];


    ASSERT_EQ(RtpTS2, frame2->GetRTPTimestamp());
    ASSERT_TRUE(frame2->IsFrameComplete());

    frameInfo = frame2->GetFrameInfo();
    ASSERT_EQ(RtpTS2, frameInfo->GetRTPTimestamp());

    ASSERT_EQ(11, *frameInfo->GetMinSequence());
    ASSERT_EQ(13, *frameInfo->GetMaxSequence());

    ASSERT_EQ(11, *frameInfo->GetStartSequence());
    ASSERT_EQ(13, *frameInfo->GetEndSequence());
}

TEST_F(CRTPVideoRecvFrameManagerTest, Random_Complete_Frames_Marker_Test)
{
    CScopedTracer test_info;

    std::vector<RtpTS_Seq> basePktInfos{
        //{1, 10},
        {1, 11},
        {1, 12},
        //{1, 13},

        {2, 14},
        {2, 15},
        {2, 16},
        //{2, 17},
        {2, 18},

        {3, 19},
        //{3, 20},
        {3, 21},

        {4, 22},
        //{4, 23},

        {5, 24},
        {5, 25},
        //{5, 26},
        //{5, 27}
    };
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(basePktInfos.begin(), basePktInfos.end(), g);
    basePktInfos.insert(basePktInfos.begin(), {1, 10});

    std::vector<RtpTS_Seq> extPktInfos{
        {1, 13},
        {2, 17},
        {3, 20},
        {4, 23},
        {5, 26},
        {5, 27}
    };

    uint32_t nowTickMS = 12345;
    for (const auto &info : basePktInfos) {
        CRTPPacketAuto rtp(new CRTPPacket());
        rtp->SetSequenceNumber(info.seq);
        rtp->SetTimestamp(info.rtpTS);
        m_frameManager->PushPacket(std::move(rtp), nowTickMS);
    }
    ASSERT_EQ(0, m_frameManager->GetPoppedFrameCount());


    for (const auto &info : extPktInfos) {
        CRTPPacketAuto rtp(new CRTPPacket());
        bool marker = (info.seq == 27);
        rtp->SetSequenceNumber(info.seq);
        rtp->SetTimestamp(info.rtpTS);
        if (marker) {
            rtp->SetMarker();
        }
        m_frameManager->PushPacket(std::move(rtp), nowTickMS);
    }

    // Check the manager statistics
    ASSERT_EQ(5, m_frameManager->GetPoppedFrameCount());
    ASSERT_EQ(5, m_frameManager->GetPoppedFrameCount(FramePopCause::FRAME_POP_COMPLETE));

    ASSERT_EQ(0, m_frameManager->GetDroppedPacketCount());
    ASSERT_EQ(basePktInfos.size() + extPktInfos.size(), m_frameManager->GetProcessedPacketCount());

    ASSERT_EQ(5, m_poppedFrames.size());
}

TEST_F(CRTPVideoRecvFrameManagerTest, Incomplete_Frames_Pop_Due_To_Push_Test)
{
    CScopedTracer test_info;

    // 8, 9, 10 | 11, 12, 13 | 14, 15, 16 | 17, 18, 19
    std::vector<RtpTS_Seq> pktInfos{
        {1, 8},
        //{1, 9},
        {1, 10},

        //{?, 11},
        {2, 12},
        //{2, 13},

        {3, 14},
        {3, 15},
        {3, 16},

        {4, 17},
        {4, 18},
        {4, 19}
    };

    // Feed in the RTP packets
    uint32_t nowTickMS = 123456;
    for (const auto &info : pktInfos) {
        CRTPPacketAuto rtp(new CRTPPacket());
        rtp->SetSequenceNumber(info.seq);
        rtp->SetTimestamp(info.rtpTS);
        m_frameManager->PushPacket(std::move(rtp), nowTickMS);
    }

    // Check the manager statistics
    ASSERT_EQ(3, m_frameManager->GetPoppedFrameCount());
    ASSERT_EQ(1, m_frameManager->GetPoppedFrameCount(FramePopCause::FRAME_POP_COMPLETE));
    ASSERT_EQ(2, m_frameManager->GetPoppedFrameCount(FramePopCause::FRAME_POP_PUSHED));

    ASSERT_EQ(0, m_frameManager->GetDroppedPacketCount());
    ASSERT_EQ(pktInfos.size(), m_frameManager->GetProcessedPacketCount());

    ASSERT_EQ(3, m_poppedFrames.size());

    // Check the first popped frame
    auto &frame = m_poppedFrames[0];


    ASSERT_EQ(1, frame->GetRTPTimestamp());
    ASSERT_TRUE(!frame->IsFrameComplete());

    auto &frameInfo = frame->GetFrameInfo();
    ASSERT_EQ(1, frameInfo->GetRTPTimestamp());

    ASSERT_EQ(8,  *frameInfo->GetStartSequence());
    ASSERT_EQ(11, *frameInfo->GetEndSequence());

    // Check the second popped frame
    auto &frame2 = m_poppedFrames[1];

    ASSERT_EQ(2, frame2->GetRTPTimestamp());
    ASSERT_TRUE(!frame2->IsFrameComplete());

    auto &frameInfo2 = frame2->GetFrameInfo();
    ASSERT_EQ(2, frameInfo2->GetRTPTimestamp());

    ASSERT_EQ(12, *frameInfo2->GetStartSequence());
    ASSERT_EQ(13, *frameInfo2->GetEndSequence());

    // Check the third popped frame
    auto &frame3 = m_poppedFrames[2];

    ASSERT_EQ(3, frame3->GetRTPTimestamp());
    ASSERT_TRUE(frame3->IsFrameComplete());

    auto &frameInfo3 = frame3->GetFrameInfo();
    ASSERT_EQ(3, frameInfo3->GetRTPTimestamp());

    ASSERT_EQ(14, *frameInfo3->GetStartSequence());
    ASSERT_EQ(16, *frameInfo3->GetEndSequence());
}

struct RtpTS_Seq_Marker {
    uint32_t rtpTS;
    uint16_t seq;
    bool     marker;
};

TEST_F(CRTPVideoRecvFrameManagerTest, Front_Insert_Frame_Test)
{
    CScopedTracer test_info;
    
    // 8, 9, 10 | 11, 12, 13 | 14, 15, 16
    std::vector<RtpTS_Seq_Marker> pktInfos{
        {1, 8,  false},
        {1, 9,  false},
        {1, 10, true},
        
        {3, 14, false},
        {3, 16, true},
        
        {2, 11, false},
        {2, 13, true},
        
        {2, 12, false},
        
        {3, 15, false},
    };
    
    // Feed in the RTP packets
    uint32_t nowTickMS = 123456;
    for (const auto &info : pktInfos) {
        CRTPPacketAuto rtp(new CRTPPacket());
        rtp->SetSequenceNumber(info.seq);
        rtp->SetTimestamp(info.rtpTS);
        if (info.marker) {
            rtp->SetMarker();
        }
        m_frameManager->PushPacket(std::move(rtp), nowTickMS);
    }

    // Check the manager statistics
    ASSERT_EQ(3, m_frameManager->GetPoppedFrameCount());
    ASSERT_EQ(3, m_frameManager->GetPoppedFrameCount(FramePopCause::FRAME_POP_COMPLETE));
    
    ASSERT_EQ(0, m_frameManager->GetDroppedPacketCount());
    ASSERT_EQ(pktInfos.size(), m_frameManager->GetProcessedPacketCount());
    
    ASSERT_EQ(3, m_poppedFrames.size());
    
    // Check the first popped frame: [8, 9, 10]
    auto &frame = m_poppedFrames[0];
    
    
    ASSERT_EQ(1, frame->GetRTPTimestamp());
    ASSERT_TRUE(frame->IsFrameComplete());
    
    auto &frameInfo = frame->GetFrameInfo();
    ASSERT_EQ(1, frameInfo->GetRTPTimestamp());
    
    ASSERT_EQ(8,  *frameInfo->GetStartSequence());
    ASSERT_EQ(10, *frameInfo->GetEndSequence());
    
    // Check the second popped frame: [11, 12, 13]
    auto &frame2 = m_poppedFrames[1];
    
    ASSERT_EQ(2, frame2->GetRTPTimestamp());
    ASSERT_TRUE(frame2->IsFrameComplete());
    
    auto &frameInfo2 = frame2->GetFrameInfo();
    ASSERT_EQ(2, frameInfo2->GetRTPTimestamp());
    
    ASSERT_EQ(11, *frameInfo2->GetStartSequence());
    ASSERT_EQ(13, *frameInfo2->GetEndSequence());
    
    // Check the third popped frame: [14, 15, 16]
    auto &frame3 = m_poppedFrames[2];
    
    ASSERT_EQ(3, frame3->GetRTPTimestamp());
    ASSERT_TRUE(frame3->IsFrameComplete());
    
    auto &frameInfo3 = frame3->GetFrameInfo();
    ASSERT_EQ(3, frameInfo3->GetRTPTimestamp());
    
    ASSERT_EQ(14, *frameInfo3->GetStartSequence());
    ASSERT_EQ(16, *frameInfo3->GetEndSequence());
}
