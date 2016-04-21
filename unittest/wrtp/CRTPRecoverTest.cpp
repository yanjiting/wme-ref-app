//
//  CRTPRecoverTest.cpp
//  wrtpTest_mac
//
//  Created by juntang on 9/28/14.
//  Copyright (c) 2014 cisco. All rights reserved.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "wrtpdefines.h"
#include "wrtpclientapi.h"
#include "rtputils.h"
#include "wrtppacket.h"
#include "testutil.h"
#include "rtpsessionconfig.h"

#define private public
#include "rtprecover.h"


using namespace wrtp;
using namespace wme;

class CRTPRecoverSinkMock : public CRTPRecoverSink
{
public:
    
    CRTPRecoverSinkMock()
    {
        m_pktCount = 0;
        m_isOrder = true;
        m_lastSeq = 0;
    }
    
    ~CRTPRecoverSinkMock()
    {
        
    }
    
    virtual void OnOutput(CRTPPacketAuto && pkt)
    {
        if (m_lastSeq == 0)
            m_lastSeq = pkt->GetSequenceNumber();
        else {
            if ((int16_t)(m_lastSeq - pkt->GetSequenceNumber()) >= 0)
                m_isOrder = false;
            
            m_lastSeq = pkt->GetSequenceNumber();
        }
        ++m_pktCount;
    }
    
    virtual void FlushLastFrame(TICK_COUNT_TYPE nowTickMS) {}
public:
    int m_pktCount;
    bool m_isOrder;
    uint16_t m_lastSeq;
};

class CRTPRecoveryTEST : public ::testing::Test
{
public:
    CRTPRecoveryTEST() : m_context(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO)) {}
protected:
    virtual void SetUp() {
    }
    virtual void TearDown() {
    }
    
protected:
    const RTPSessionContextSharedPtr m_context;
};

TEST_F(CRTPRecoveryTEST, CRTPRecoveryOrderWithoutLoss)
{
    CRTPRecoverSinkMock sinkMock;
    
    CCmSharedPtr<CRTPRecover> rtpRecover(new CRTPRecover(m_context->GetContextTag(), &sinkMock, 1));
    
    CRTPPacketAuto pkt(new CRTPPacket());
    
    int i, baseSeq = 1300;
    for (i = 0; i < 10; ++i) {
        TICK_COUNT_TYPE curTick = TickNowMS();
        
        pkt->SetSequenceNumber(baseSeq++);
        rtpRecover->SortRTPPacket(std::move(pkt), pkt->GetSequenceNumber(), curTick, false);
    }
    
    EXPECT_EQ(sinkMock.m_pktCount, i);
    EXPECT_EQ(sinkMock.m_isOrder, true);
}

TEST_F(CRTPRecoveryTEST, CRTPRecoveryOrderWithLoss)
{
    CRTPRecoverSinkMock sinkMock;
    
    CCmSharedPtr<CRTPRecover> rtpRecover(new CRTPRecover(m_context->GetContextTag(), &sinkMock, 1));
    
    //CRTPPacketAuto pkt(new CRTPPacket());
    
    int i, baseSeq = 1300;
    for (i = 0; i < 10; ++i) {
        CRTPPacketAuto pkt(new CRTPPacket());
        TICK_COUNT_TYPE curTick = TickNowMS();
        if (i == 3) {
            baseSeq++;
            continue;
        }
        pkt->SetSequenceNumber(baseSeq++);
        rtpRecover->SortRTPPacket(std::move(pkt), pkt->GetSequenceNumber(), curTick, false);
        //SleepMs(20);
    }
    
    WRTP_TEST_TRIGGER_ON_TIMER(600, 1);
    EXPECT_EQ(sinkMock.m_pktCount, i-1);
    EXPECT_EQ(sinkMock.m_isOrder, true);
}

TEST_F(CRTPRecoveryTEST, CRTPRecoveryDisorderWithouLoss)
{
    CRTPRecoverSinkMock sinkMock;
    
    CCmSharedPtr<CRTPRecover> rtpRecover(new CRTPRecover(m_context->GetContextTag(), &sinkMock, 1));
    
    CRTPPacketAuto pkt1(new CRTPPacket());
    TICK_COUNT_TYPE curTick = TickNowMS();
    pkt1->SetSequenceNumber(1300);
    rtpRecover->SortRTPPacket(std::move(pkt1), pkt1->GetSequenceNumber(), curTick, false);
    
    CRTPPacketAuto pkt4(new CRTPPacket());
    curTick = TickNowMS();
    pkt4->SetSequenceNumber(1303);
    rtpRecover->SortRTPPacket(std::move(pkt4), pkt4->GetSequenceNumber(), curTick, false);
    
    CRTPPacketAuto pkt5(new CRTPPacket());
    curTick = TickNowMS();
    pkt5->SetSequenceNumber(1304);
    rtpRecover->SortRTPPacket(std::move(pkt5), pkt5->GetSequenceNumber(), curTick, false);
    
    CRTPPacketAuto pkt2(new CRTPPacket());
    curTick = TickNowMS();
    pkt2->SetSequenceNumber(1302);
    rtpRecover->SortRTPPacket(std::move(pkt2), pkt2->GetSequenceNumber(), curTick, false);
    
    CRTPPacketAuto pkt3(new CRTPPacket());
    curTick = TickNowMS();
    pkt3->SetSequenceNumber(1301);
    rtpRecover->SortRTPPacket(std::move(pkt3), pkt3->GetSequenceNumber(), curTick, false);
    
    EXPECT_EQ(sinkMock.m_pktCount, 5);
    EXPECT_EQ(sinkMock.m_isOrder, true);
}

TEST_F(CRTPRecoveryTEST, CRTPRecoveryDisorderWithLoss)
{
    CRTPRecoverSinkMock sinkMock;
    
    CCmSharedPtr<CRTPRecover> rtpRecover(new CRTPRecover(m_context->GetContextTag(), &sinkMock, 1));
    
    CRTPPacketAuto pkt1(new CRTPPacket());
    TICK_COUNT_TYPE curTick = TickNowMS();
    pkt1->SetSequenceNumber(1300);
    rtpRecover->SortRTPPacket(std::move(pkt1), pkt1->GetSequenceNumber(), curTick, false);
    
    CRTPPacketAuto pkt3(new CRTPPacket());
    curTick = TickNowMS();
    pkt3->SetSequenceNumber(1302);
    rtpRecover->SortRTPPacket(std::move(pkt3), pkt3->GetSequenceNumber(), curTick, false);
    
    CRTPPacketAuto pkt2(new CRTPPacket());
    curTick = TickNowMS();
    pkt2->SetSequenceNumber(1301);
    rtpRecover->SortRTPPacket(std::move(pkt2), pkt2->GetSequenceNumber(), curTick, false);
    
    CRTPPacketAuto pkt5(new CRTPPacket());
    curTick = TickNowMS();
    pkt5->SetSequenceNumber(1304);
    rtpRecover->SortRTPPacket(std::move(pkt5), pkt5->GetSequenceNumber(), curTick, false);
    
    WRTP_TEST_TRIGGER_ON_TIMER(600, 1);
    
    EXPECT_EQ(sinkMock.m_pktCount, 4);
    EXPECT_EQ(sinkMock.m_isOrder, true);
}