#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "rtcppacket.h"
#include "testutil.h"

#include <vector>

using namespace wrtp;
namespace wrtp
{
CRTCPPacket *DecodeRTCPPacket(CCmByteStreamNetwork &is, CCmMessageBlock &mb);
};

class CRTPFBPacketTEST : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        expect_ssrc = 123;
        expect_ssrcSrc = 234;
        expect_fciLength = 10;
        for (uint16_t i = 0; i < expect_fciLength; ++i) {
            expect_fci.push_back(i + 100);
        }
    }

    // virtual void TearDown() {}

    uint32_t expect_ssrc;
    uint32_t expect_ssrcSrc;
    uint16_t expect_fciLength;
    std::vector<uint32_t> expect_fci;
};

TEST_F(CRTPFBPacketTEST, CRTPFBPacket)
{
    CScopedTracer test_info;
    CRTPFBPacket fbPacketEncode(expect_ssrc, expect_ssrcSrc, (uint16_t)expect_fciLength*4, (uint8_t *)&*expect_fci.begin());
    CCmMessageBlock mbEncode(fbPacketEncode.CalcEncodeSize());
    CCmByteStreamNetwork os(mbEncode);

    fbPacketEncode.Encode(os, mbEncode);
    fbPacketEncode.m_fci = nullptr;

    CRTCPPacket *packet = DecodeRTCPPacket(os, mbEncode);
    EXPECT_TRUE(packet != nullptr);
    CPSFBPacket *fb_packet = (CPSFBPacket *)packet;
    //CRTPFBPacket fbPacketDecode;
    //fbPacketDecode.Decode(os, mbEncode);

    EXPECT_EQ(RTCP_PT_RTPFB, fb_packet->GetType());
    EXPECT_EQ(expect_ssrc, fb_packet->m_ssrc);
    EXPECT_EQ(expect_ssrcSrc, fb_packet->m_ssrcSrc);
    EXPECT_EQ(expect_fciLength*4, fb_packet->m_fciLength);
    for (uint16_t i = 0; i < expect_fciLength; ++i) {
        EXPECT_EQ(100+i, ((uint32_t *)fb_packet->m_fci)[i]);
    }
    fb_packet->ReleaseReference();
}


