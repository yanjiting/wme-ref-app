#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "rtpsessionconfig.h"
#include "fecencoder.h"
#include "fecdecoder.h"
#include "testutil.h"
#include "atdef.h"

using namespace wrtp;

class RtpPacket {
public:
    RtpPacket() : rtpBuff(nullptr) {}
    RtpPacket(CRTPPacketAuto&& pkt, CCmMessageBlock* buf) : rtp(std::move(pkt)), rtpBuff(buf) {}
    RtpPacket(RtpPacket&& l) {
        *this = std::move(l);
    }
    RtpPacket& operator= (RtpPacket&& l) {
        if (this != &l) {
            rtp = std::move(l.rtp);
            rtpBuff = l.rtpBuff;

            l.rtpBuff = nullptr;
        }
        return *this;
    }

    CRTPPacketAuto rtp;
    CCmMessageBlock *rtpBuff;
};
struct RtpBuff {
    RtpBuff()
        : csi(0)
        , ssrc(0)
        , seq(0)
        , rtpBuff(nullptr)
    {}

    uint32_t csi;
    uint32_t ssrc;
    uint16_t seq;
    CCmMessageBlock *rtpBuff;
};

class CMariFecManagerTest : public ::testing::Test
{
public:
    CRTPPacketAuto GetRTPMsgBlock(CCmMessageBlock &rtpPacketMb, uint16_t seq, uint32_t timestamp, uint8_t payloadType, uint32_t ssrc, uint16_t payloadLength, bool marker = false)
    {
        CCmMessageBlock payloadMb(payloadLength);
        InitBuffer((uint8_t *)payloadMb.GetTopLevelWritePtr(), payloadLength);
        payloadMb.AdvanceTopLevelWritePtr(payloadLength);
        CRTPPacket *rtpPacket = GetBasicRTPPacket(seq, timestamp, payloadType, ssrc, payloadMb);
        // set csi equal to ssrc
        rtpPacket->AddCSRC(ssrc);
        if (marker) {
            rtpPacket->SetMarker();
        }
        rtpPacket->Pack(rtpPacketMb);
        CRTPPacketAuto ret(rtpPacket);
        return ret;
    }

    uint8_t getFecLevel(uint8_t k, uint8_t p, uint32_t csi, uint32_t ssrc)
    {
        return k;
    }

    // recovered packet handler
    void recoveredPacket(uint32_t csi, uint32_t ssrc, uint16_t seq, CCmMessageBlock &rtpBuff)
    {
        RtpBuff p;
        p.csi = csi;
        p.ssrc = ssrc;
        p.seq = seq;
        p.rtpBuff = rtpBuff.DuplicateChained();
        m_recoveredPackets.push_back(p);
    }

    void GenerateSrc(uint16_t streamCount, uint16_t streamSrcPacketCount, vector<RtpPacket> &srcPackets)
    {
        for (uint16_t i = 0; i < streamCount; ++i) {
            uint16_t seq = (uint16_t)rand()%100000;
            uint32_t timestamp = (uint32_t)rand()%200000;
            uint32_t ssrc = (uint32_t)rand()%300000;
            for (uint16_t i = 0; i < streamSrcPacketCount; ++i) {
                uint16_t payloadLength = (uint16_t)rand()%1400;
                if (0 == payloadLength) {
                    payloadLength = 123;
                }
                CCmMessageBlock tmp;
                CRTPPacketAuto rtp = GetRTPMsgBlock(tmp, seq++, timestamp++, PAYLOAD_TYPE_VIDEO_TEST, ssrc, payloadLength, (streamSrcPacketCount-1 == i));
                srcPackets.emplace_back(std::move(rtp), tmp.DuplicateChained());
                //srcPackets.push_back(rtpPacket);
            }
        }
    }

    void GenerateFec(uint16_t streamCount, uint16_t streamSrcPacketCount, vector<RtpPacket> &srcPackets, vector<RtpPacket> &fecPackets)
    {
        MariFECInfo fecInfo;
        CMariEncoderManager encoderManager(fecInfo, m_clockRate, m_timeoffsetId, m_fecLevelHandle);
        GenerateSrc(streamCount, streamSrcPacketCount, srcPackets);

        for (uint32_t i = 0; i < srcPackets.size(); ++i) {
            bool rv = encoderManager.AddMediaPacket(srcPackets[i].rtp->GetCSRC(0), srcPackets[i].rtp->GetSSRC(), srcPackets[i].rtp->GetSequenceNumber(), *(srcPackets[i].rtpBuff), (streamSrcPacketCount-1 == i%streamSrcPacketCount));
            ASSERT_TRUE(rv);
        }

        for (uint16_t i = 0; i < streamCount*streamSrcPacketCount; ++i) {
            CCmMessageBlock tmp;
            bool result = encoderManager.FrontFecPacket(tmp);
            ASSERT_TRUE(result);
            CRTPPacketAuto rtp = CRTPPacketAuto(new CRTPPacket());
            CmResult rv = rtp->Unpack(tmp);
            ASSERT_EQ(WRTP_ERR_NOERR, rv);
            fecPackets.emplace_back(std::move(rtp), tmp.DuplicateChained());
            //fecPackets.push_back(fecPacket);
            encoderManager.PopFecPacket();
        }
    }

    void CheckRecoveredPacket(uint16_t srcIndex, RtpBuff recoveredPacket)
    {
        CRTPPacket rtpPacket;
        CmResult rv = rtpPacket.Unpack(*(recoveredPacket.rtpBuff));
        ASSERT_TRUE(WRTP_ERR_NOERR == rv);

        EXPECT_EQ(PAYLOAD_TYPE_VIDEO_TEST, rtpPacket.GetPayloadType());

        EXPECT_EQ(1, rtpPacket.GetCSRCCount());
        EXPECT_EQ(m_srcPackets[srcIndex].rtp->GetCSRC(0), rtpPacket.GetCSRC(0));
        EXPECT_EQ(m_srcPackets[srcIndex].rtp->GetCSRC(0), recoveredPacket.csi);

        EXPECT_EQ(m_srcPackets[srcIndex].rtp->GetSSRC(), rtpPacket.GetSSRC());
        EXPECT_EQ(m_srcPackets[srcIndex].rtp->GetSSRC(), recoveredPacket.ssrc);

        EXPECT_EQ(m_srcPackets[srcIndex].rtp->GetSequenceNumber(), rtpPacket.GetSequenceNumber());
        EXPECT_EQ(m_srcPackets[srcIndex].rtp->GetSequenceNumber(), recoveredPacket.seq);

        ASSERT_EQ(m_srcPackets[srcIndex].rtpBuff->GetTopLevelLength(), recoveredPacket.rtpBuff->GetTopLevelLength());
        memcmp(m_srcPackets[srcIndex].rtpBuff->GetTopLevelReadPtr(), recoveredPacket.rtpBuff->GetTopLevelReadPtr(), recoveredPacket.rtpBuff->GetTopLevelLength());
    }

protected:
    virtual void SetUp()
    {
        m_payloadType = 111;
        m_timeoffsetId = 3;
        m_clockRate = DEFAULT_MARI_FEC_CAPTURE_CLOCK_RATE;

        FecLevelSharedPtr tmp(new DYNAMIC_FEC_LEVEL_FUNC(bind(&CMariFecManagerTest::getFecLevel, this, PLACEHOLDER_NS::_1, PLACEHOLDER_NS::_2, PLACEHOLDER_NS::_3, PLACEHOLDER_NS::_4)));
        m_fecLevelHandle = tmp;

        MariFECInfo fecInfo;
        CCmSharedPtr<CMariEncoderManager> encoderManeger(new CMariEncoderManager(fecInfo, m_clockRate, m_timeoffsetId, m_fecLevelHandle));
        m_encoderManager = encoderManeger;

        CCmSharedPtr<CMariDecoderManager> decoderManager(new CMariDecoderManager(fecInfo, m_clockRate, bind(&CMariFecManagerTest::recoveredPacket, this, PLACEHOLDER_NS::_1, PLACEHOLDER_NS::_2, PLACEHOLDER_NS::_3, PLACEHOLDER_NS::_4)));
        m_decoderManager = decoderManager;
        m_srcPackets.clear();
        m_fecPackets.clear();
        m_recoveredPackets.clear();
    }

    virtual void TearDown()
    {
        for (vector<RtpBuff>::iterator it = m_recoveredPackets.begin(); it != m_recoveredPackets.end(); ++it) {
            if (it->rtpBuff) {
                it->rtpBuff->DestroyChained();
                it->rtpBuff = nullptr;
            }
        }
        for (vector<RtpPacket>::iterator it = m_srcPackets.begin(); it != m_srcPackets.end(); ++it) {
            if (it->rtpBuff) {
                it->rtpBuff->DestroyChained();
                it->rtpBuff = nullptr;
            }
        }
        for (vector<RtpPacket>::iterator it = m_fecPackets.begin(); it != m_fecPackets.end(); ++it) {
            if (it->rtpBuff) {
                it->rtpBuff->DestroyChained();
                it->rtpBuff = nullptr;
            }
        }
        m_srcPackets.clear();
        m_fecPackets.clear();
        m_recoveredPackets.clear();
    }

public:
    // For fec
    uint8_t m_payloadType;
    uint16_t m_timeoffsetId;
    uint32_t m_clockRate;

    vector<RtpPacket> m_srcPackets;
    vector<RtpPacket> m_fecPackets;
    vector<RtpBuff> m_recoveredPackets;

    CCmSharedPtr<CMariDecoderManager> m_decoderManager;
    CCmSharedPtr<CMariEncoderManager> m_encoderManager;
    FecLevelSharedPtr m_fecLevelHandle;
};

class CMariEncoderManagerTest : public CMariFecManagerTest
{
public:
    void CheckFecPacket(CCmMessageBlock &fecMb, uint32_t fecIndex = 0, bool resetChecker = false)
    {
        static bool firstPacket = true;
        static uint32_t lastSsrc = 0;
        static uint16_t lastSeq = 0;
        if (resetChecker) {
            firstPacket = true;
            lastSsrc = 0;
            lastSeq = 0;
        }

        CRTPPacket rtpPacket;
        CmResult rv = rtpPacket.Unpack(fecMb);
        ASSERT_TRUE(WRTP_ERR_NOERR == rv);
        EXPECT_EQ(1, rtpPacket.GetCSRCCount());
        EXPECT_EQ(m_srcPackets[fecIndex].rtp->GetCSRC(0), rtpPacket.GetCSRC(0));
        EXPECT_EQ(m_payloadType, rtpPacket.GetPayloadType());
        if (firstPacket) {
            lastSsrc = rtpPacket.GetSSRC();
            lastSeq  = rtpPacket.GetSequenceNumber();
            firstPacket = false;
        } else {
            EXPECT_EQ(lastSsrc, rtpPacket.GetSSRC());
            EXPECT_EQ(++lastSeq, rtpPacket.GetSequenceNumber());
        }
    }
};

TEST_F(CMariEncoderManagerTest, Invalid_fec_level_handle)
{
    CScopedTracer test_info;

    uint32_t srcCount = 15;
    GenerateSrc(1, srcCount, m_srcPackets);

    FecLevelSharedPtr emptyHandle;
    MariFECInfo fecInfo;
    CMariEncoderManager invalidManager(fecInfo, m_clockRate, m_timeoffsetId, emptyHandle);

    for (uint32_t i = 0; i < m_srcPackets.size(); ++i) {
        bool rv = m_encoderManager->AddMediaPacket(m_srcPackets[i].rtp->GetCSRC(0), m_srcPackets[i].rtp->GetSSRC(), m_srcPackets[i].rtp->GetSequenceNumber(), *(m_srcPackets[i].rtpBuff));
        ASSERT_TRUE(rv);
    }

    CCmMessageBlock fecMb;
    EXPECT_FALSE(m_encoderManager->FrontFecPacket(fecMb));
}

TEST_F(CMariEncoderManagerTest, Normal_test_single_stream)
{
    CScopedTracer test_info;

    uint32_t srcCount = 15;
    GenerateSrc(1, srcCount, m_srcPackets);
    for (uint32_t i = 0; i < m_srcPackets.size(); ++i) {
        bool rv = m_encoderManager->AddMediaPacket(m_srcPackets[i].rtp->GetCSRC(0), m_srcPackets[i].rtp->GetSSRC(), m_srcPackets[i].rtp->GetSequenceNumber(), *(m_srcPackets[i].rtpBuff), (i == srcCount-1));
        ASSERT_TRUE(rv);
    }

    for (uint32_t fecCount = 0; fecCount < m_srcPackets.size(); ++fecCount) {
        CCmMessageBlock fecMb;
        ASSERT_TRUE(m_encoderManager->FrontFecPacket(fecMb));
        CheckFecPacket(fecMb);
        m_encoderManager->PopFecPacket();
    }
    CCmMessageBlock fecMb;
    EXPECT_FALSE(m_encoderManager->FrontFecPacket(fecMb));
}

TEST_F(CMariEncoderManagerTest, Normal_test_multi_streams)
{
    CScopedTracer test_info;
    uint16_t streamCount = 3;
    uint16_t srcCountPerStream = 15;
    GenerateSrc(streamCount, srcCountPerStream, m_srcPackets);

    for (uint16_t i = 0; i < streamCount*srcCountPerStream; ++i) {
        bool rv = m_encoderManager->AddMediaPacket(m_srcPackets[i].rtp->GetCSRC(0), m_srcPackets[i].rtp->GetSSRC(), m_srcPackets[i].rtp->GetSequenceNumber(), *(m_srcPackets[i].rtpBuff), (srcCountPerStream-1 == i%srcCountPerStream));
        ASSERT_TRUE(rv);
    }

    for (uint16_t streamIndex = 0; streamIndex < streamCount; ++streamIndex) {
        for (uint16_t fecIndex = 0; fecIndex < srcCountPerStream; ++fecIndex) {
            CCmMessageBlock fecMb;
            ASSERT_TRUE(m_encoderManager->FrontFecPacket(fecMb));
            CheckFecPacket(fecMb, streamCount*srcCountPerStream-1, (0 == fecIndex));
            m_encoderManager->PopFecPacket();
        }
    }
    CCmMessageBlock fecMb;
    EXPECT_FALSE(m_encoderManager->FrontFecPacket(fecMb));
}

TEST_F(CMariEncoderManagerTest, Test_Pause_Resume)
{
    CScopedTracer test_info;

    uint32_t srcCount = 30;
    GenerateSrc(1, srcCount, m_srcPackets);
    for (uint32_t i = 0; i < 15; ++i) {
        bool rv = m_encoderManager->AddMediaPacket(m_srcPackets[i].rtp->GetCSRC(0), m_srcPackets[i].rtp->GetSSRC(), m_srcPackets[i].rtp->GetSequenceNumber(), *(m_srcPackets[i].rtpBuff), (i == 14));
        ASSERT_TRUE(rv);
    }
    m_encoderManager->Pause();
    CCmMessageBlock fecMb;
    ASSERT_FALSE(m_encoderManager->FrontFecPacket(fecMb));

    m_encoderManager->Resume();

    for (uint32_t i = 15; i < 30; ++i) {
        bool rv = m_encoderManager->AddMediaPacket(m_srcPackets[i].rtp->GetCSRC(0), m_srcPackets[i].rtp->GetSSRC(), m_srcPackets[i].rtp->GetSequenceNumber(), *(m_srcPackets[i].rtpBuff), (i == 29));
        ASSERT_TRUE(rv);
    }

    for (int fecCount = 0; fecCount < 15; ++fecCount) {
        CCmMessageBlock fecMb;
        ASSERT_TRUE(m_encoderManager->FrontFecPacket(fecMb));
        CheckFecPacket(fecMb, 0, (0 == fecCount));
        m_encoderManager->PopFecPacket();
    }
}

class CMariDecoderManagerTest : public CMariFecManagerTest
{
};

TEST_F(CMariDecoderManagerTest, Empty_Manager)
{
    CScopedTracer test_info;
    CRTPPacket rtp;
    rtp.SetSSRC(1);
    rtp.AddCSRC(2);
    rtp.SetPayloadType(111);
    CCmMessageBlock rtpPayload(1400);
    rtp.SetPayload(rtpPayload);
    CCmMessageBlock rtpBuff;
    rtp.Pack(rtpBuff);

    bool rv = m_decoderManager->ReceivePacket(rtpBuff);
    EXPECT_FALSE(rv);
}

TEST_F(CMariDecoderManagerTest, test_one_stream_lost_last_packet)
{
    CScopedTracer test_info;
    GenerateFec(1, 15, m_srcPackets, m_fecPackets);

    ASSERT_EQ(15, m_srcPackets.size());
    ASSERT_EQ(m_srcPackets.size(), m_fecPackets.size());
    m_decoderManager->ReceivePacket(*(m_fecPackets[0].rtpBuff));

    for (int i = 0; i < m_srcPackets.size(); ++i) {
        if (i == m_srcPackets.size()-1) {
            //lost last src packet
            continue;
        }
        m_decoderManager->ReceivePacket(*(m_srcPackets[i].rtpBuff));
    }
    m_decoderManager->ReceivePacket(*(m_fecPackets[2].rtpBuff));

    EXPECT_EQ(1, m_recoveredPackets.size());

    CheckRecoveredPacket(m_srcPackets.size()-1, m_recoveredPackets[0]);
}

TEST_F(CMariDecoderManagerTest, test_one_stream_lost_two_packets)
{
    CScopedTracer test_info;
    GenerateFec(1, 15, m_srcPackets, m_fecPackets);

    ASSERT_EQ(15, m_srcPackets.size());
    ASSERT_EQ(m_srcPackets.size(), m_fecPackets.size());
    m_decoderManager->ReceivePacket(*(m_fecPackets[0].rtpBuff));

    for (int i = 0; i < m_srcPackets.size(); ++i) {
        if ((0 == i) || (i == m_srcPackets.size()-1)) {
            //lost first and last src packet
            continue;
        }
        m_decoderManager->ReceivePacket(*(m_srcPackets[i].rtpBuff));
    }
    m_decoderManager->ReceivePacket(*(m_fecPackets[2].rtpBuff));

    EXPECT_EQ(2, m_recoveredPackets.size());

    CheckRecoveredPacket(0, m_recoveredPackets[0]);
    CheckRecoveredPacket(m_srcPackets.size()-1, m_recoveredPackets[1]);
}

TEST_F(CMariDecoderManagerTest, test_one_stream_repeat_packets)
{
    CScopedTracer test_info;
    GenerateFec(1, 15, m_srcPackets, m_fecPackets);

    ASSERT_EQ(15, m_srcPackets.size());
    ASSERT_EQ(m_srcPackets.size(), m_fecPackets.size());
    m_decoderManager->ReceivePacket(*(m_fecPackets[0].rtpBuff));

    for (int i = 0; i < m_srcPackets.size(); ++i) {
        if ((0 == i) || (i == m_srcPackets.size()-1)) {
            //lost first and last src packet
            continue;
        }
        m_decoderManager->ReceivePacket(*(m_srcPackets[i].rtpBuff));
    }
    // repeat recv one packet
    m_decoderManager->ReceivePacket(*(m_srcPackets[1].rtpBuff));
    m_decoderManager->ReceivePacket(*(m_fecPackets[2].rtpBuff));

    EXPECT_EQ(2, m_recoveredPackets.size());

    CheckRecoveredPacket(0, m_recoveredPackets[0]);
    CheckRecoveredPacket(m_srcPackets.size()-1, m_recoveredPackets[1]);
}

TEST_F(CMariDecoderManagerTest, test_one_stream_lost_all_packets)
{
    int srcCount = 15;
    CScopedTracer test_info;
    GenerateFec(1, srcCount, m_srcPackets, m_fecPackets);

    ASSERT_EQ(srcCount, m_srcPackets.size());
    ASSERT_EQ(m_srcPackets.size(), m_fecPackets.size());

    for (int i = 0; i < m_fecPackets.size(); ++i) {
        m_decoderManager->ReceivePacket(*(m_fecPackets[i].rtpBuff));
    }

    EXPECT_EQ(srcCount, m_recoveredPackets.size());

    for (int i = 0; i < m_fecPackets.size(); ++i) {
        CheckRecoveredPacket(i, m_recoveredPackets[i]);
    }
}

TEST_F(CMariDecoderManagerTest, test_multi_stream_lost_packets)
{
    CScopedTracer test_info;
    int streamCount = 3;
    int packetPerStream = 15;
    GenerateFec(streamCount, packetPerStream, m_srcPackets, m_fecPackets);

    ASSERT_EQ(streamCount*packetPerStream, m_srcPackets.size());
    ASSERT_EQ(m_srcPackets.size(), m_fecPackets.size());

    m_decoderManager->ReceivePacket(*(m_fecPackets[0].rtpBuff));
    m_decoderManager->ReceivePacket(*(m_fecPackets[packetPerStream].rtpBuff));
    m_decoderManager->ReceivePacket(*(m_fecPackets[packetPerStream*2].rtpBuff));

    for (int i = 0; i < m_srcPackets.size(); ++i) {
        if ((0 == i) || (3 == i) ||(i == m_srcPackets.size()-1)) {
            //lost first and last src packet
            continue;
        }
        m_decoderManager->ReceivePacket(*(m_srcPackets[i].rtpBuff));
    }
    m_decoderManager->ReceivePacket(*(m_fecPackets[2].rtpBuff));
    m_decoderManager->ReceivePacket(*(m_fecPackets[2+packetPerStream].rtpBuff));
    m_decoderManager->ReceivePacket(*(m_fecPackets[2+packetPerStream*2].rtpBuff));

    // The second stream will recover one packet
    ASSERT_EQ(4, m_recoveredPackets.size());

    CheckRecoveredPacket(29, m_recoveredPackets[0]);
    CheckRecoveredPacket(m_srcPackets.size()-1, m_recoveredPackets[1]);
    CheckRecoveredPacket(0, m_recoveredPackets[2]);
}

TEST_F(CMariDecoderManagerTest, test_multi_stream_lost_all_packets)
{
    CScopedTracer test_info;
    int streamCount = 3;
    int packetPerStream = 15;
    GenerateFec(streamCount, packetPerStream, m_srcPackets, m_fecPackets);

    ASSERT_EQ(streamCount*packetPerStream, m_srcPackets.size());
    ASSERT_EQ(m_srcPackets.size(), m_fecPackets.size());

    for (int i = 0; i < m_fecPackets.size(); ++i) {
        m_decoderManager->ReceivePacket(*(m_fecPackets[i].rtpBuff));
    }

    EXPECT_EQ(streamCount*packetPerStream, m_recoveredPackets.size());

    for (int i = 0; i < m_fecPackets.size(); ++i) {
        CheckRecoveredPacket(i, m_recoveredPackets[i]);
    }
}

TEST_F(CMariDecoderManagerTest, test_GetRecoveredCount)
{
    CScopedTracer test_info;
    int streamCount = 2;
    int packetPerStream = 15;
    GenerateFec(streamCount, packetPerStream, m_srcPackets, m_fecPackets);

    ASSERT_EQ(streamCount*packetPerStream, m_srcPackets.size());
    ASSERT_EQ(m_srcPackets.size(), m_fecPackets.size());

    m_decoderManager->ReceivePacket(*(m_fecPackets[0].rtpBuff));
    m_decoderManager->ReceivePacket(*(m_fecPackets[packetPerStream].rtpBuff));

    for (int i = 0; i < m_srcPackets.size(); ++i) {
        if ((0 == i) || (3 == i) ||(i == m_srcPackets.size()-1)) {
            //first stream lost two packets, second stream lost one packet
            continue;
        }
        m_decoderManager->ReceivePacket(*(m_srcPackets[i].rtpBuff));
    }
    m_decoderManager->ReceivePacket(*(m_fecPackets[2].rtpBuff));
    m_decoderManager->ReceivePacket(*(m_fecPackets[2+packetPerStream].rtpBuff));

    ASSERT_EQ(3, m_recoveredPackets.size());
    EXPECT_EQ(3, m_decoderManager->GetRecoveredCount());
}

TEST_F(CMariDecoderManagerTest, test_remove_decoder)
{
    CScopedTracer test_info;
    m_decoderManager->RemoveMariDecoder(123);
    CCmMessageBlock tmp;
    ASSERT_FALSE(m_decoderManager->ReceivePacket(tmp));
}

TEST_F(CMariDecoderManagerTest, test_remove_encoder)
{
    CScopedTracer test_info;
    m_encoderManager->RemoveMariEncoder(123);
    CCmMessageBlock tmp;
    ASSERT_TRUE(m_encoderManager->AddMediaPacket(12, 34, 56, tmp));
}