//
//  VideoRecvFrameInfoTest.cpp
//  wrtpTest_mac
//
//  Created by Hank Peng on 8/24/15.
//  Copyright (c) 2015 cisco. All rights reserved.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "../testutil.h"

#include "wrtppacket.h"
#include "videorecvframe.h"

#include <vector>
#include <random>
#include <memory>
#include <functional>
#include <algorithm>

using namespace wrtp;

//////////////////////////////////////////////////////////////////////////////////
class CRTPVideoRecvFrameInfoTest : public ::testing::Test
{
public:
    CRTPVideoRecvFrameInfoTest()
    {
        Reset();
    }

    void OnMinSeqUpdated(uint16_t seq, bool marked)
    {
        m_minSeq    = seq;
        m_minMarked = marked;
    }

    void OnMaxSeqUpdated(uint16_t seq, bool marked)
    {
        m_maxSeq    = seq;
        m_maxMarked = marked;
    }

protected:
    virtual void SetUp()
    {
        Reset();
    }

    virtual void TearDown() {}

private:
    void Reset()
    {
        m_minSeq        = -1;
        m_minMarked     = false;
        m_maxSeq        = -1;
        m_maxMarked     = false;

        m_sequenceNumbers.assign({1, 2, 3, 4, 5, 6, 7, 8, 9, 10});

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(m_sequenceNumbers.begin(), m_sequenceNumbers.end(), g);
    }

protected:
    uint16_t      m_minSeq;
    bool        m_minMarked;

    uint16_t      m_maxSeq;
    bool        m_maxMarked;

    std::vector<uint16_t> m_sequenceNumbers;
};

TEST_F(CRTPVideoRecvFrameInfoTest, Out_of_Order_Without_Loss_Without_Marker_Test)
{
    CScopedTracer test_info;

    const uint32_t rtpTS = 100000;
    CRTPVideoRecvFrameInfo info(rtpTS);

    for (auto seq : m_sequenceNumbers) {
        char whichUpdated = info.InputSequenceNumber(seq, false);
        if(whichUpdated & CRTPVideoRecvFrameInfo::MIN_SEQ_UPDATED) {
            OnMinSeqUpdated(*info.GetMinSequence(), false);
        }
        if(whichUpdated & CRTPVideoRecvFrameInfo::MAX_SEQ_UPDATED) {
            OnMaxSeqUpdated(*info.GetMaxSequence(), false);
        }
    }

    ASSERT_EQ(1,        m_minSeq);
    ASSERT_EQ(false,    m_minMarked);
    ASSERT_EQ(10,       m_maxSeq);
    ASSERT_EQ(false,    m_maxMarked);

    ASSERT_EQ(rtpTS,    info.GetRTPTimestamp());
    ASSERT_TRUE(!info.GetStartSequence());
    ASSERT_TRUE(!info.GetEndSequence());
    ASSERT_EQ(1,        *info.GetMinSequence());
    ASSERT_EQ(10,       *info.GetMaxSequence());
}

TEST_F(CRTPVideoRecvFrameInfoTest, Out_of_Order_With_Loss_With_Marker_Test)
{
    CScopedTracer test_info;

    const uint32_t rtpTS = 100000;
    CRTPVideoRecvFrameInfo info(rtpTS);

    for (auto seq : m_sequenceNumbers) {
        if (seq % 3 == 0) {
            continue;
        }

        bool marked = (seq == 10);
        char whichUpdated = info.InputSequenceNumber(seq, marked);
        if(whichUpdated & CRTPVideoRecvFrameInfo::MIN_SEQ_UPDATED) {
            OnMinSeqUpdated(*info.GetMinSequence(), marked);
        }
        if(whichUpdated & CRTPVideoRecvFrameInfo::MAX_SEQ_UPDATED) {
            OnMaxSeqUpdated(*info.GetMaxSequence(), marked);
        }
    }

    ASSERT_EQ(1,        m_minSeq);
    ASSERT_EQ(false,    m_minMarked);
    ASSERT_EQ(10,       m_maxSeq);
    ASSERT_EQ(true,     m_maxMarked);

    ASSERT_EQ(rtpTS,    info.GetRTPTimestamp());
    ASSERT_TRUE(!info.GetStartSequence());
    ASSERT_TRUE(!!info.GetEndSequence());
    ASSERT_EQ(1,        *info.GetMinSequence());
    ASSERT_EQ(10,       *info.GetMaxSequence());
}


//////////////////////////////////////////////////////////////////////////////////
class CFrameBorderArbiterTest : public ::testing::Test
{
public:
    CFrameBorderArbiterTest() : m_info1(), m_info2(), m_info3(), m_info2Sequences() {}

protected:
    virtual void SetUp()
    {
        m_info1.reset(new CRTPVideoRecvFrameInfo(10));
        m_info2.reset(new CRTPVideoRecvFrameInfo(20));
        m_info3.reset(new CRTPVideoRecvFrameInfo(30));

        m_info2Sequences.assign({11, 13, 15, 16, 17, 18, 20});

        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(m_info2Sequences.begin(), m_info2Sequences.end(), g);
    }

    virtual void TearDown() {}

protected:
    std::unique_ptr<CRTPVideoRecvFrameInfo> m_info1;
    std::unique_ptr<CRTPVideoRecvFrameInfo> m_info2;
    std::unique_ptr<CRTPVideoRecvFrameInfo> m_info3;

    std::vector<uint16_t>                     m_info2Sequences;
};

TEST_F(CFrameBorderArbiterTest, No_PrevInfo_No_NextInfo_No_Marker)
{
    CScopedTracer test_info;

    CFrameBorderArbiter arbiter(m_info2.get(), nullptr, nullptr);
    for (auto seq : m_info2Sequences) {
        arbiter.InputSequenceOfCurrentFrame(seq, false);
    }

    ASSERT_EQ(11,   *m_info2->GetMinSequence());
    ASSERT_EQ(20,   *m_info2->GetMaxSequence());
    ASSERT_EQ(11,   *m_info2->GetStartSequence());
    ASSERT_TRUE(!m_info2->GetEndSequence());
}

TEST_F(CFrameBorderArbiterTest, No_PrevInfo_No_NextInfo_With_Marker)
{
    CScopedTracer test_info;

    CFrameBorderArbiter arbiter(m_info2.get(), nullptr, nullptr);
    for (auto seq : m_info2Sequences) {
        bool marked = (seq == 20);
        arbiter.InputSequenceOfCurrentFrame(seq, marked);
    }

    ASSERT_EQ(11,   *m_info2->GetMinSequence());
    ASSERT_EQ(20,   *m_info2->GetMaxSequence());
    ASSERT_EQ(11,   *m_info2->GetStartSequence());
    ASSERT_EQ(20,   *m_info2->GetEndSequence());
}

// 7, 8, 9, 10 | 11, ..., 20 | 21, 22, 23
TEST_F(CFrameBorderArbiterTest, PrevInfo_NextInfo_No_Gap_No_Marker)
{
    CScopedTracer test_info;

    std::random_device rd;
    std::mt19937 g(rd());

    std::vector<uint16_t> info1Sequence({7, 8, 9, 10});
    std::vector<uint16_t> info3Sequence({21, 22, 23});
    std::shuffle(info1Sequence.begin(), info1Sequence.end(), g);
    std::shuffle(info3Sequence.begin(), info3Sequence.end(), g);

    for (auto seq : info1Sequence) {
        m_info1->InputSequenceNumber(seq, false);
    }

    for (auto seq : info3Sequence) {
        m_info3->InputSequenceNumber(seq, false);
    }

    CFrameBorderArbiter arbiter(m_info2.get(), m_info1.get(), m_info3.get());
    for (auto seq : m_info2Sequences) {
        arbiter.InputSequenceOfCurrentFrame(seq, false);
    }

    ASSERT_EQ(10,  *m_info1->GetMaxSequence());
    ASSERT_EQ(10,  *m_info1->GetEndSequence());

    ASSERT_EQ(11, *m_info2->GetMinSequence());
    ASSERT_EQ(11, *m_info2->GetStartSequence());

    ASSERT_EQ(20, *m_info2->GetMaxSequence());
    ASSERT_EQ(20, *m_info2->GetEndSequence());

    ASSERT_EQ(21, *m_info3->GetMinSequence());
    ASSERT_EQ(21, *m_info3->GetStartSequence());

    ASSERT_EQ(23, *m_info3->GetMaxSequence());
}

// 7, 8, 9, 10' | 11, ..., 20' | 21, 22, 23'
TEST_F(CFrameBorderArbiterTest, PrevInfo_NextInfo_No_Gap_With_Marker)
{
    CScopedTracer test_info;

    std::random_device rd;
    std::mt19937 g(rd());

    std::vector<uint16_t> info1Sequence({7, 8, 9, 10});
    std::vector<uint16_t> info3Sequence({21, 22, 23});
    std::shuffle(info1Sequence.begin(), info1Sequence.end(), g);
    std::shuffle(info3Sequence.begin(), info3Sequence.end(), g);

    for (auto seq : info1Sequence) {
        bool marker = (seq == 10);
        m_info1->InputSequenceNumber(seq, marker);
    }

    for (auto seq : info3Sequence) {
        bool marker = (seq == 23);
        m_info3->InputSequenceNumber(seq, marker);
    }

    CFrameBorderArbiter arbiter(m_info2.get(), m_info1.get(), m_info3.get());
    for (auto seq : m_info2Sequences) {
        bool marker = (seq == 20);
        arbiter.InputSequenceOfCurrentFrame(seq, marker);
    }

    ASSERT_EQ(10,  *m_info1->GetMaxSequence());
    ASSERT_EQ(10,  *m_info1->GetEndSequence());

    ASSERT_EQ(11, *m_info2->GetMinSequence());
    ASSERT_EQ(11, *m_info2->GetStartSequence());

    ASSERT_EQ(20, *m_info2->GetMaxSequence());
    ASSERT_EQ(20, *m_info2->GetEndSequence());

    ASSERT_EQ(21, *m_info3->GetMinSequence());
    ASSERT_EQ(21, *m_info3->GetStartSequence());

    ASSERT_EQ(23, *m_info3->GetMaxSequence());
    ASSERT_EQ(23, *m_info3->GetEndSequence());
}

// 7, 8, 9, 10' | 11, ..., 20' | 21, 22, 23'
TEST_F(CFrameBorderArbiterTest, PrevInfo_NextInfo_No_Gap_With_Marker_Frame_Disorder)
{
    CScopedTracer test_info;

    std::random_device rd;
    std::mt19937 g(rd());

    std::vector<uint16_t> info1Sequence({7, 8, 9, 10});
    std::vector<uint16_t> info3Sequence({21, 22, 23});
    std::shuffle(info1Sequence.begin(), info1Sequence.end(), g);
    std::shuffle(info3Sequence.begin(), info3Sequence.end(), g);

    CFrameBorderArbiter arbiter1(m_info1.get(), nullptr, nullptr);
    for (auto seq : info1Sequence) {
        bool marker = (seq == 10);
        arbiter1.InputSequenceOfCurrentFrame(seq, marker);
    }
    ASSERT_EQ(10,  *m_info1->GetMaxSequence());
    ASSERT_EQ(10,  *m_info1->GetEndSequence());

    CFrameBorderArbiter arbiter3(m_info3.get(), m_info1.get(), nullptr);
    for (auto seq : info3Sequence) {
        bool marker = (seq == 23);
        m_info3->InputSequenceNumber(seq, marker);
    }
    ASSERT_EQ(23, *m_info3->GetMaxSequence());
    ASSERT_EQ(23, *m_info3->GetEndSequence());

    CFrameBorderArbiter arbiter2(m_info2.get(), m_info1.get(), m_info3.get());
    for (auto seq : m_info2Sequences) {
        bool marker = (seq == 20);
        arbiter2.InputSequenceOfCurrentFrame(seq, marker);
    }

    ASSERT_EQ(10,  *m_info1->GetMaxSequence());
    ASSERT_EQ(10,  *m_info1->GetEndSequence());

    ASSERT_EQ(11, *m_info2->GetMinSequence());
    ASSERT_EQ(11, *m_info2->GetStartSequence());

    ASSERT_EQ(20, *m_info2->GetMaxSequence());
    ASSERT_EQ(20, *m_info2->GetEndSequence());

    ASSERT_EQ(21, *m_info3->GetMinSequence());
    ASSERT_EQ(21, *m_info3->GetStartSequence());

    ASSERT_EQ(23, *m_info3->GetMaxSequence());
    ASSERT_EQ(23, *m_info3->GetEndSequence());
}


// 7, 8, 9, * | 11, ..., 20 | *, 22, 23
TEST_F(CFrameBorderArbiterTest, PrevInfo_NextInfo_1_Gap_With_Marker_Missing)
{
    CScopedTracer test_info;

    std::random_device rd;
    std::mt19937 g(rd());

    std::vector<uint16_t> info1Sequence({7, 8, 9});       // 10 is missing
    std::vector<uint16_t> info3Sequence({22, 23});        // 21 is missing
    std::shuffle(info1Sequence.begin(), info1Sequence.end(), g);
    std::shuffle(info3Sequence.begin(), info3Sequence.end(), g);

    CFrameBorderArbiter arbiter1(m_info1.get(), nullptr, nullptr);
    for (auto seq : info1Sequence) {
        arbiter1.InputSequenceOfCurrentFrame(seq, false);
    }

    CFrameBorderArbiter arbiter2(m_info2.get(), m_info1.get(), nullptr);
    for (auto seq : m_info2Sequences) {
        arbiter2.InputSequenceOfCurrentFrame(seq, false);
    }

    CFrameBorderArbiter arbiter3(m_info3.get(), m_info2.get(), nullptr);
    for (auto seq : info3Sequence) {
        arbiter3.InputSequenceOfCurrentFrame(seq, false);
    }

    ASSERT_EQ(7,  *m_info1->GetStartSequence());
    ASSERT_EQ(10, *m_info1->GetEndSequence());

    ASSERT_EQ(11, *m_info2->GetStartSequence());
    ASSERT_EQ(21, *m_info2->GetEndSequence());

    ASSERT_EQ(22, *m_info3->GetStartSequence());
}

// 7, 8, 9, * | 11, ..., 20 | *, 22, 23
TEST_F(CFrameBorderArbiterTest, PrevInfo_NextInfo_1_Gap_With_Marker_Missing_Frame_Disorder)
{
    CScopedTracer test_info;

    std::random_device rd;
    std::mt19937 g(rd());

    std::vector<uint16_t> info1Sequence({7, 8, 9});       // 10 is missing
    std::vector<uint16_t> info3Sequence({22, 23});        // 21 is missing
    std::shuffle(info1Sequence.begin(), info1Sequence.end(), g);
    std::shuffle(info3Sequence.begin(), info3Sequence.end(), g);

    CFrameBorderArbiter arbiter1(m_info1.get(), nullptr, nullptr);
    for (auto seq : info1Sequence) {
        arbiter1.InputSequenceOfCurrentFrame(seq, false);
    }

    CFrameBorderArbiter arbiter3(m_info3.get(), m_info1.get(), nullptr);
    for (auto seq : info3Sequence) {
        arbiter3.InputSequenceOfCurrentFrame(seq, false);
    }

    CFrameBorderArbiter arbiter2(m_info2.get(), m_info1.get(), m_info3.get());
    for (auto seq : m_info2Sequences) {
        arbiter2.InputSequenceOfCurrentFrame(seq, false);
    }

    ASSERT_EQ(7,  *m_info1->GetStartSequence());
    ASSERT_EQ(10, *m_info1->GetEndSequence());

    ASSERT_EQ(11, *m_info2->GetStartSequence());
    ASSERT_EQ(21, *m_info2->GetEndSequence());

    ASSERT_EQ(22, *m_info3->GetStartSequence());
}

// 7, 8, *, 10' | 11, ..., 20' | *, 22, 23'
TEST_F(CFrameBorderArbiterTest, PrevInfo_NextInfo_1_Gap_With_Marker)
{
    CScopedTracer test_info;

    std::random_device rd;
    std::mt19937 g(rd());

    std::vector<uint16_t> info1Sequence({7, 8, 10});          //  9 is missing
    std::vector<uint16_t> info3Sequence({22, 23});            // 21 is missing
    std::shuffle(info1Sequence.begin(), info1Sequence.end(), g);
    std::shuffle(info3Sequence.begin(), info3Sequence.end(), g);

    CFrameBorderArbiter arbiter1(m_info1.get(), nullptr, nullptr);
    for (auto seq : info1Sequence) {
        bool marker = (seq == 10);
        arbiter1.InputSequenceOfCurrentFrame(seq, marker);
    }
    ASSERT_EQ(7,  *m_info1->GetStartSequence());
    ASSERT_EQ(10, *m_info1->GetEndSequence());

    CFrameBorderArbiter arbiter2(m_info2.get(), m_info1.get(), nullptr);
    for (auto seq : m_info2Sequences) {
        bool marker = (seq == 20);
        arbiter2.InputSequenceOfCurrentFrame(seq, marker);
    }
    ASSERT_EQ(11, *m_info2->GetStartSequence());
    ASSERT_EQ(20, *m_info2->GetEndSequence());

    CFrameBorderArbiter arbiter3(m_info3.get(), m_info2.get(), nullptr);
    for (auto seq : info3Sequence) {
        bool marker = (seq == 23);
        arbiter3.InputSequenceOfCurrentFrame(seq, marker);
    }

    ASSERT_EQ(7,  *m_info1->GetStartSequence());
    ASSERT_EQ(10, *m_info1->GetEndSequence());

    ASSERT_EQ(11, *m_info2->GetStartSequence());
    ASSERT_EQ(20, *m_info2->GetEndSequence());

    ASSERT_EQ(21, *m_info3->GetStartSequence());
    ASSERT_EQ(23, *m_info3->GetEndSequence());
}

// 7, 8, *, 10' | 11, ..., 20' | *, 22, 23'
TEST_F(CFrameBorderArbiterTest, PrevInfo_NextInfo_1_Gap_With_Marker_Frame_Disorder)
{
    CScopedTracer test_info;

    std::random_device rd;
    std::mt19937 g(rd());

    std::vector<uint16_t> info1Sequence({7, 8, 10});          //  9 is missing
    std::vector<uint16_t> info3Sequence({22, 23});            // 21 is missing
    std::shuffle(info1Sequence.begin(), info1Sequence.end(), g);
    std::shuffle(info3Sequence.begin(), info3Sequence.end(), g);

    CFrameBorderArbiter arbiter1(m_info1.get(), nullptr, nullptr);
    for (auto seq : info1Sequence) {
        bool marker = (seq == 10);
        arbiter1.InputSequenceOfCurrentFrame(seq, marker);
    }
    ASSERT_EQ(7,  *m_info1->GetStartSequence());
    ASSERT_EQ(10, *m_info1->GetEndSequence());

    CFrameBorderArbiter arbiter3(m_info3.get(), m_info1.get(), nullptr);
    for (auto seq : info3Sequence) {
        bool marker = (seq == 23);
        arbiter3.InputSequenceOfCurrentFrame(seq, marker);
    }

    CFrameBorderArbiter arbiter2(m_info2.get(), m_info1.get(), m_info3.get());
    for (auto seq : m_info2Sequences) {
        bool marker = (seq == 20);
        arbiter2.InputSequenceOfCurrentFrame(seq, marker);
    }
    ASSERT_EQ(11, *m_info2->GetStartSequence());
    ASSERT_EQ(20, *m_info2->GetEndSequence());

    ASSERT_EQ(7,  *m_info1->GetStartSequence());
    ASSERT_EQ(10, *m_info1->GetEndSequence());

    ASSERT_EQ(11, *m_info2->GetStartSequence());
    ASSERT_EQ(20, *m_info2->GetEndSequence());

    ASSERT_EQ(21, *m_info3->GetStartSequence());
    ASSERT_EQ(23, *m_info3->GetEndSequence());
}

// 7, 8, *, 10' | 11, ..., 20' | *, 22, 23'
TEST_F(CFrameBorderArbiterTest, Packet_Random_Test)
{
    CScopedTracer test_info;

    std::random_device rd;
    std::mt19937 g(rd());

    std::vector<uint16_t> sequences{7, 8, 10, 11, 12, 13, 15, 17, 19, 20, 22, 23};
    std::shuffle(sequences.begin(), sequences.end(), g);

    CFrameBorderArbiter arbiter1(m_info1.get(), nullptr, m_info2.get());
    CFrameBorderArbiter arbiter2(m_info2.get(), m_info1.get(), m_info3.get());
    CFrameBorderArbiter arbiter3(m_info3.get(), m_info2.get(), nullptr);

    for (auto seq : sequences) {
        bool marker = (seq == 10) || (seq == 20) || (seq == 23);
        CFrameBorderArbiter *arbiter = &arbiter1;
        if (seq > 10 && seq <= 20) {
            arbiter = &arbiter2;
        } else if (seq > 20) {
            arbiter = &arbiter3;
        }

        arbiter->InputSequenceOfCurrentFrame(seq, marker);
    }

    ASSERT_EQ(11, *m_info2->GetStartSequence());
    ASSERT_EQ(20, *m_info2->GetEndSequence());

    ASSERT_EQ(7,  *m_info1->GetStartSequence());
    ASSERT_EQ(10, *m_info1->GetEndSequence());

    ASSERT_EQ(11, *m_info2->GetStartSequence());
    ASSERT_EQ(20, *m_info2->GetEndSequence());

    ASSERT_EQ(21, *m_info3->GetStartSequence());
    ASSERT_EQ(23, *m_info3->GetEndSequence());
}
