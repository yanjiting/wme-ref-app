//
//  VideoRecvFramePacketTest.cpp
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

using namespace wrtp;

class CVideoRecvFramePacketTest : public ::testing::Test
{
public:
    CVideoRecvFramePacketTest() : m_packetMgr("CVideoRecvFramePacket") {}
protected:

    virtual void SetUp() {}
    virtual void TearDown() {}
    
    CVideoRecvFramePacket::ObjectManager    m_packetMgr;
};


TEST_F(CVideoRecvFramePacketTest, Data_Member_Acess_Test)
{
    CScopedTracer test_info;

    CRTPPacketAuto pktPtr(new CRTPPacket());
    uint16_t seq      = 1000;
    uint32_t rtpTS    = 90000;
    bool marked     = true;

    CRecvPacketUniquePtr packet(m_packetMgr.GetObjectUniquePtr());
    packet->Init(std::move(pktPtr), seq, rtpTS, marked);
    ASSERT_TRUE(!!(packet->GetRtpPacket()));
    ASSERT_EQ(marked,   packet->IsMarked());
    ASSERT_EQ(rtpTS,    packet->GetRTPTimestamp());

    uint32_t capTick  = 12000;
    uint32_t sendTick = 12010;
    uint32_t recvTick = 12080;

    packet->SetCaptureTick(capTick);
    ASSERT_EQ(capTick,  packet->GetCaptureTick());

    packet->SetSendingTick(sendTick);
    ASSERT_EQ(sendTick, packet->GetSendingTick());

    packet->SetRecvingTick(recvTick);
    ASSERT_EQ(recvTick, packet->GetRecvingTick());
}
