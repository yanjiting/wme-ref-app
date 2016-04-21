#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "wrtpclientapi.h"
#include "fragmentop.h"
#include "rtpsmartptr.h"
#include "WMEInterfaceMock.h"
#include "rtpsessionconfig.h"
#include "testutil.h"
#include <vector>

using namespace std;
using namespace wrtp;


class CRTPPacketizationMockTest : public ::testing::Test
{
public:
    CRTPPacketizationMockTest() {}

protected:
    virtual void SetUp()
    {
        m_packetizationOperator.reset(new CRTPPacketizationMock());
    }

    virtual void TearDown()
    {
        m_packetizationOperator.reset();
    }

    std::unique_ptr<CRTPPacketizationMock> m_packetizationOperator;
};

TEST_F(CRTPPacketizationMockTest, Aggregation_Test)
{
    CScopedTracer test_info;
    uint8_t nal0[] = {PHT_SINGLE, 0, 0, '1', '2', '3'};
    uint8_t nal1[] = {PHT_SINGLE, 0, 0, 'a', 'b', 'c', 'd'};
    uint8_t nal2[] = {PHT_SINGLE, 0, 0, '1', '2', '3'};

    PAYLOAD_SET_LENGTH(nal0, sizeof(nal0));
    PAYLOAD_SET_LENGTH(nal1, sizeof(nal1));
    PAYLOAD_SET_LENGTH(nal2, sizeof(nal2));

    // construct the aggregated packet
    const uint32_t AggregatedLength = RTP_PAYLOAD_HEADER_LEN + sizeof(nal0) + sizeof(nal1) + sizeof(nal2);
    uint8_t aggregatedNal[AggregatedLength] = {PHT_AGGREGATED};
    PAYLOAD_SET_COUNT(aggregatedNal, 3);
    uint8_t *datap = aggregatedNal + RTP_PAYLOAD_HEADER_LEN;
    memcpy(datap, nal0, sizeof(nal0));
    datap += sizeof(nal0);
    memcpy(datap, nal1, sizeof(nal1));
    datap += sizeof(nal1);
    memcpy(datap, nal2, sizeof(nal2));

    FragmentBuffer nals[3] = {};
    nals[0].bufferLength    = sizeof(nal0);
    nals[0].fragmentLength  = sizeof(nal0);
    nals[0].pBuffer         = nal0;

    nals[1].bufferLength    = sizeof(nal1);
    nals[1].fragmentLength  = sizeof(nal1);
    nals[1].pBuffer         = nal1;

    nals[2].bufferLength    = sizeof(nal2);
    nals[2].fragmentLength  = sizeof(nal2);
    nals[2].pBuffer         = nal2;


    // Aggregate test
    uint8_t aggregatedBuffer[128] = {};
    uint32_t length = sizeof(aggregatedBuffer);
    int32_t ret = m_packetizationOperator->Aggregate(nals, 3, aggregatedBuffer, length);

    ASSERT_TRUE(ret == 0);
    ASSERT_EQ(sizeof(aggregatedNal), length);
    ASSERT_TRUE(0 == memcmp(aggregatedNal, aggregatedBuffer, length));

    // NAL count check
    uint32_t count = m_packetizationOperator->GetContainedNALCount(aggregatedBuffer, length);
    ASSERT_EQ(3, count);

    // De-aggregate test
    for (uint32_t ii = 0; ii < 3; ++ii) {
        nals[ii].fragmentLength = 0;
    }

    ret = m_packetizationOperator->Deaggregate(aggregatedBuffer, length, nals, 3);
    ASSERT_TRUE(ret == 0);

    // Re-aggregate test
    ret = m_packetizationOperator->Aggregate(nals, 3, aggregatedBuffer, length);

    ASSERT_TRUE(ret == 0);
    ASSERT_EQ(sizeof(aggregatedNal), length);
    ASSERT_TRUE(0 == memcmp(aggregatedNal, aggregatedBuffer, length));
}

TEST_F(CRTPPacketizationMockTest, Fragmentation_Test)
{
    CScopedTracer test_info;
    uint8_t singleNal[] = {PHT_SINGLE, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8};
    RTP_PAYLOAD_INIT(singleNal, sizeof(singleNal));
    const uint32_t maxSize = 7;

    FragmentBuffer nals[3] = {};
    for (uint32_t ii = 0; ii < 3; ++ii) {
        nals[ii].bufferLength = 10;
        nals[ii].fragmentLength = 0;
        nals[ii].pBuffer = new uint8_t[10];
    }

    uint32_t count = m_packetizationOperator->GetFragmentCount(sizeof(singleNal), maxSize);
    ASSERT_EQ(3, count);

    // Fragment test
    int32_t ret = m_packetizationOperator->Fragment(singleNal, sizeof(singleNal), nals, 3);
    ASSERT_EQ(0, ret);

    uint8_t fu0[] = {PHT_FU_START, 0, 0, 0, 0, 0, 1};
    uint8_t fu1[] = {PHT_FU_MID,   0, 0, 2, 3, 4, 5};
    uint8_t fu2[] = {PHT_FU_END,   0, 0, 6, 7, 8};

    PAYLOAD_SET_LENGTH(fu0, sizeof(fu0));
    PAYLOAD_SET_LENGTH(fu0 + RTP_PAYLOAD_HEADER_LEN, sizeof(singleNal));
    PAYLOAD_SET_LENGTH(fu1, sizeof(fu1));
    PAYLOAD_SET_LENGTH(fu2, sizeof(fu2));

    ASSERT_EQ(sizeof(fu0), nals[0].fragmentLength);
    ASSERT_EQ(0, memcmp(fu0, nals[0].pBuffer, sizeof(fu0)));

    ASSERT_EQ(sizeof(fu1), nals[1].fragmentLength);
    ASSERT_EQ(0, memcmp(fu1, nals[1].pBuffer, sizeof(fu1)));

    ASSERT_EQ(sizeof(fu2), nals[2].fragmentLength);
    ASSERT_EQ(0, memcmp(fu2, nals[2].pBuffer, sizeof(fu2)));

    // Fu type test
    ASSERT_TRUE(m_packetizationOperator->IsStartFu(nals[0].pBuffer, nals[0].fragmentLength));
    ASSERT_FALSE(m_packetizationOperator->IsEndFu(nals[0].pBuffer, nals[0].fragmentLength));
    ASSERT_FALSE(m_packetizationOperator->IsStartFu(nals[1].pBuffer, nals[1].fragmentLength));
    ASSERT_FALSE(m_packetizationOperator->IsEndFu(nals[1].pBuffer, nals[1].fragmentLength));
    ASSERT_FALSE(m_packetizationOperator->IsStartFu(nals[2].pBuffer, nals[2].fragmentLength));
    ASSERT_TRUE(m_packetizationOperator->IsEndFu(nals[2].pBuffer, nals[2].fragmentLength));


    uint8_t defragmentedNAL[128] = {};
    uint32_t length = sizeof(defragmentedNAL);

    // De-fragment test
    ret = m_packetizationOperator->Defragment(nals, 3, defragmentedNAL, length);
    ASSERT_EQ(0, ret);
    ASSERT_EQ(sizeof(singleNal), length);
    ASSERT_TRUE(0 == memcmp(singleNal, defragmentedNAL, length));

    // release the memory
    for (uint32_t ii = 0; ii < 3; ++ii) {
        delete [] nals[ii].pBuffer;
    }

}

class CVideoDecorderMock : public IMediaDataRecvSink
{
public:
    CVideoDecorderMock() : m_nalRecvCount(0), m_nals(), m_markerFalgs() {}

    virtual int32_t OnRecvMediaData(/*RTP_IN*/ WRTPMediaData *pData, /*RTP_IN*/ WRTPMediaDataIntegrityInfo *pIntegrityInfo)
    {
        EXPECT_TRUE(pData != nullptr);
        EXPECT_EQ((WRTPDataType)MEDIA_DATA_VIDEO, pData->type);
        EXPECT_LE(m_nalRecvCount, m_nals.size());

        FragmentBuffer &curNal = m_nals[m_nalRecvCount];
        EXPECT_EQ(curNal.fragmentLength, pData->size);
        EXPECT_EQ(0, memcmp(curNal.pBuffer, pData->data, pData->size));

        ++m_nalRecvCount;

        WRTPMediaDataVideo *mdv = reinterpret_cast<WRTPMediaDataVideo *>(pData);
        m_markerFalgs.push_back(mdv->marker);

        return 0;
    }

    void RegisterSingleNAL(const FragmentBuffer &nal)
    {
        m_nals.push_back(nal);
    }

    uint32_t GetNalRecvCount() { return m_nalRecvCount; }

    const vector<uint8_t> GetMarkerFlags() { return m_markerFalgs; }

private:
    uint32_t m_nalRecvCount;
    vector<FragmentBuffer> m_nals;
    vector<uint8_t> m_markerFalgs;
};

class CRTPPacketTransportMock
    : public IRTPMediaTransport
{
public:
    CRTPPacketTransportMock()
        : m_sendRTPPackets()
    {
    }

    ~CRTPPacketTransportMock()
    {
        for (vector<IWmeMediaPackage *>::iterator iter = m_sendRTPPackets.begin(); iter != m_sendRTPPackets.end(); ++iter) {
            (*iter)->Release();
        }
        m_sendRTPPackets.clear();
    }

    virtual WMERESULT GetTransportType(WmeTransportType &eTransportType)
    {
        eTransportType = WmeTransportType_UDP;
        return WME_S_OK;
    }

    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage)
    {
        pRTPPackage->AddRef();
        m_sendRTPPackets.push_back(pRTPPackage);

        return 0;
    }

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        return 0;
    }

    const vector<IWmeMediaPackage *> &GetSendRTPPackets()
    {
        return m_sendRTPPackets;
    }

private:
    vector<IWmeMediaPackage *> m_sendRTPPackets;
};

#define CODEC_TYPE_VIDEO_TEST   98
#define PAYLOAD_TYPE_VIDEO_TEST 98

class CVideoPacketizationTest : public ::testing::Test
{
public:
    CVideoPacketizationTest()
        : m_rtpVideoSession(nullptr)
        , m_rtpVideoChannel(nullptr)
        , m_packetizer()
    {}

protected:
    virtual void SetUp()
    {
        // init RTP session
        WRTPSessionParams sessionParam;
        sessionParam.sessionType = RTP_SESSION_WEBEX_VIDEO;
        sessionParam.enableRTCP  = true;
        m_rtpVideoSession = WRTPCreateRTPSessionClient(sessionParam);

        ASSERT_TRUE(nullptr != m_rtpVideoSession);
        m_rtpVideoSession->IncreaseReference();

        uint32_t ret = m_rtpVideoSession->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
        ASSERT_TRUE(0 == ret);

        ret = m_rtpVideoSession->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);
        ASSERT_TRUE(0 == ret);

        // init RTP channel
        WRTPChannelParams channelParam;
        m_rtpVideoChannel = m_rtpVideoSession->CreateRTPChannel(1, channelParam);

        ASSERT_TRUE(nullptr != m_rtpVideoChannel);
        m_rtpVideoChannel->IncreaseReference();

        m_videoDecoder.reset(new CVideoDecorderMock());
        m_rtpVideoChannel->SetMediaDataRecvSink(m_videoDecoder.get());
        m_rtpVideoChannel->SetPacketizationOperator(&m_packetizer);
        m_rtpVideoChannel->SetPacketizationOperator(&m_packetizer);

        // set transport mock
        m_transportMock.reset(new CRTPPacketTransportMock());
        m_rtpVideoSession->SetMediaTransport(m_transportMock.get());

    }

    virtual void TearDown()
    {
        if (m_rtpVideoChannel) {
            m_rtpVideoChannel->Close();
            m_rtpVideoChannel->DecreaseReference();
            m_rtpVideoChannel = nullptr;
        }

        if (m_rtpVideoSession) {
            m_rtpVideoSession->DecreaseReference();
            m_rtpVideoSession = nullptr;
        }

        m_videoDecoder.reset();
        m_transportMock.reset();
    }

protected:
    IRTPSessionClient  *m_rtpVideoSession;
    IRTPChannel        *m_rtpVideoChannel;
    std::unique_ptr<CVideoDecorderMock> m_videoDecoder;
    std::unique_ptr<CRTPPacketTransportMock> m_transportMock;
    CRTPPacketizationMock m_packetizer;
};

IRTPMediaPackage *CaseToMediaPackage(CRTPPacket &packet)
{
    // serialize
    CCmMessageBlock rtpMB((DWORD)0);
    packet.Pack(rtpMB);

    IRTPMediaPackage *rtpPackage = nullptr;
    ConvertMessageBlockToMediaPackage(rtpMB, &rtpPackage, true);

    return rtpPackage;
}

void ConstructRTPPacket(CRTPPacket &packet, const uint8_t *payload, uint32_t length)
{
    packet.AddCSRC(1);
    packet.SetPayloadType(PAYLOAD_TYPE_VIDEO_TEST);
    CCmMessageBlock mb(length);
    memcpy((void *)mb.GetTopLevelReadPtr(), payload, length);
    mb.AdvanceTopLevelWritePtr(length);
    packet.SetPayload(mb);
}


TEST_F(CVideoPacketizationTest, depacketization_RTP_packet_when_received_single_nal_packet)
{
    CScopedTracer test_info;
    m_rtpVideoSession->SetPacketizationMode(PACKETIZATION_MODE1);

    // construct the aggregated RTP payload
    uint8_t nal0[] = {PHT_SINGLE, 0, 0, '1', '2', '3'};

    PAYLOAD_SET_LENGTH(nal0, sizeof(nal0));

    FragmentBuffer nalBuffer;
    nalBuffer.bufferLength    = sizeof(nal0);
    nalBuffer.fragmentLength  = sizeof(nal0);
    nalBuffer.pBuffer         = nal0;

    // register the source NALs
    m_videoDecoder->RegisterSingleNAL(nalBuffer);

    // construct an RTP packet
    CRTPPacket packet;
    ConstructRTPPacket(packet, nal0, sizeof(nal0));
    packet.SetSequenceNumber(101);
    packet.SetMarker(true);

    IRTPMediaPackage *rtpPackage = CaseToMediaPackage(packet);
    int32_t ret = m_rtpVideoChannel->ReceiveRTPPacket(rtpPackage);
    EXPECT_EQ(0, ret);
    delete rtpPackage;
    rtpPackage = nullptr;

    EXPECT_EQ(1, m_videoDecoder->GetNalRecvCount());
}

TEST_F(CVideoPacketizationTest, deggregation_RTP_packet_when_received_an_aggregated_packet)
{
    CScopedTracer test_info;
    m_rtpVideoSession->SetPacketizationMode(PACKETIZATION_MODE1);

    // construct the aggregated RTP payload
    uint8_t nal0[] = {PHT_SINGLE, 0, 0, '1', '2', '3'};
    uint8_t nal1[] = {PHT_SINGLE, 0, 0, 'a', 'b', 'c', 'd'};
    uint8_t nal2[] = {PHT_SINGLE, 0, 0, '1', '2', '3'};

    PAYLOAD_SET_LENGTH(nal0, sizeof(nal0));
    PAYLOAD_SET_LENGTH(nal1, sizeof(nal1));
    PAYLOAD_SET_LENGTH(nal2, sizeof(nal2));

    // construct the aggregated packet
    FragmentBuffer nals[3] = {};
    nals[0].bufferLength    = sizeof(nal0);
    nals[0].fragmentLength  = sizeof(nal0);
    nals[0].pBuffer         = nal0;

    nals[1].bufferLength    = sizeof(nal1);
    nals[1].fragmentLength  = sizeof(nal1);
    nals[1].pBuffer         = nal1;

    nals[2].bufferLength    = sizeof(nal2);
    nals[2].fragmentLength  = sizeof(nal2);
    nals[2].pBuffer         = nal2;

    // aggregate
    uint8_t aggregatedBuffer[128] = {};
    uint32_t length = sizeof(aggregatedBuffer);
    int32_t ret = m_packetizer.Aggregate(nals, 3, aggregatedBuffer, length);
    ASSERT_EQ(0, ret);

    // register the source NALs
    m_videoDecoder->RegisterSingleNAL(nals[0]);
    m_videoDecoder->RegisterSingleNAL(nals[1]);
    m_videoDecoder->RegisterSingleNAL(nals[2]);

    // construct an RTP packet
    CRTPPacket packet;
    ConstructRTPPacket(packet, aggregatedBuffer, length);
    packet.SetSequenceNumber(101);
    packet.SetMarker(true);

    IRTPMediaPackage *rtpPackage = CaseToMediaPackage(packet);
    ret = m_rtpVideoChannel->ReceiveRTPPacket(rtpPackage);
    EXPECT_EQ(0, ret);
    delete rtpPackage;
    rtpPackage = nullptr;

    EXPECT_EQ(3, m_videoDecoder->GetNalRecvCount());

    vector<uint8_t> markerFlags = m_videoDecoder->GetMarkerFlags();
    for (uint32_t ii = 0; ii < markerFlags.size(); ++ii) {
        if (ii != markerFlags.size() - 1) {
            EXPECT_EQ(0, markerFlags[ii]);
        } else {
            EXPECT_EQ(1, markerFlags[ii]);
        }
    }
}

TEST_F(CVideoPacketizationTest, defragmentation_RTP_packet_when_received_FU_A_packets)
{
    CScopedTracer test_info;
    m_rtpVideoSession->SetPacketizationMode(PACKETIZATION_MODE1);

    // the full NAL unit
    uint8_t singleNal[] = {PHT_SINGLE, 0, 0, 1, 2, 3, 4, 5, 6, 7, 8};
    RTP_PAYLOAD_INIT(singleNal, sizeof(singleNal));

    // fragmented FU-A
    uint8_t fu0[] = {PHT_FU_START, 0, 0, 0, 0, 0, 1};
    uint8_t fu1[] = {PHT_FU_MID,   0, 0, 2, 3, 4, 5};
    uint8_t fu2[] = {PHT_FU_END,   0, 0, 6, 7, 8};

    PAYLOAD_SET_LENGTH(fu0, sizeof(fu0));
    PAYLOAD_SET_LENGTH(fu0 + RTP_PAYLOAD_HEADER_LEN, sizeof(singleNal));
    PAYLOAD_SET_LENGTH(fu1, sizeof(fu1));
    PAYLOAD_SET_LENGTH(fu2, sizeof(fu2));

    // register the source NALs
    FragmentBuffer nalBuffer;
    nalBuffer.bufferLength    = sizeof(singleNal);
    nalBuffer.fragmentLength  = sizeof(singleNal);
    nalBuffer.pBuffer         = singleNal;
    m_videoDecoder->RegisterSingleNAL(nalBuffer);

    // construct the FU-A RTP packets
    CRTPPacket packets[3] = {};
    ConstructRTPPacket(packets[0], fu0, sizeof(fu0));
    packets[0].SetSequenceNumber(100);
    ConstructRTPPacket(packets[1], fu1, sizeof(fu1));
    packets[1].SetSequenceNumber(101);
    ConstructRTPPacket(packets[2], fu2, sizeof(fu2));
    packets[2].SetSequenceNumber(102);
    packets[2].SetMarker(true);

    for (uint32_t ii = 0; ii < 3; ++ii) {
        IRTPMediaPackage *rtpPackage = CaseToMediaPackage(packets[ii]);
        int32_t ret = m_rtpVideoChannel->ReceiveRTPPacket(rtpPackage);
        EXPECT_EQ(0, ret);

        delete rtpPackage;
        rtpPackage = nullptr;
    }

    EXPECT_EQ(1, m_videoDecoder->GetNalRecvCount());
}

TEST_F(CVideoPacketizationTest, aggregate_RTP_packet_when_send_short_NALs)
{
    CScopedTracer test_info;
    m_rtpVideoSession->SetPacketizationMode(PACKETIZATION_MODE1);

    const uint32_t NALCount = 3;
    uint8_t nal0[] = {PHT_SINGLE, 0, 0, '1', '2', '3'};
    uint8_t nal1[] = {PHT_SINGLE, 0, 0, 'a', 'b', 'c', 'd'};
    uint8_t nal2[] = {PHT_SINGLE, 0, 0, '1', '2', '3'};

    struct NALData {
        uint8_t *dataptr;
        uint16_t length;
    };
    NALData nals[NALCount] = { {nal0, sizeof(nal0)}, {nal1, sizeof(nal1)}, {nal2, sizeof(nal2)}};
    for (int ii = 0; ii < NALCount; ++ii) {
        PAYLOAD_SET_LENGTH(nals[ii].dataptr, nals[ii].length);
    }

    // aggregate
    // construct the aggregated packet
    FragmentBuffer nalBufers[NALCount] = {};
    for (int ii = 0; ii < NALCount; ++ii) {
        nalBufers[ii].bufferLength    = nals[ii].length;
        nalBufers[ii].fragmentLength  = nals[ii].length;
        nalBufers[ii].pBuffer         = nals[ii].dataptr;
    }

    uint8_t aggregatedBuffer[128] = {};
    uint32_t aggLength = sizeof(aggregatedBuffer);
    int32_t ret = m_packetizer.Aggregate(nalBufers, NALCount, aggregatedBuffer, aggLength);
    ASSERT_EQ(0, ret);

    m_rtpVideoSession->EnableSendingAggregationPacket(true);
    for (int ii = 0; ii < NALCount; ++ii) {
        WRTPMediaDataVideo md;
        md.type             = MEDIA_DATA_VIDEO;
        md.timestamp        = 1234;
        md.sampleTimestamp  = 5678;
        md.codecType        = CODEC_TYPE_VIDEO_TEST;
        md.marker           = (ii != NALCount - 1) ? 0 : 1;
        md.size             = nals[ii].length;
        md.data             = nals[ii].dataptr;
        //        md.layerNumber      = 1;
        //        md.layerIndex       = 0;
        md.layerWidth       = 160;
        md.layerHeight      = 90;
        md.layerMapAll      = false;
        md.frame_idc        = 1;
        md.disposable       = false;
        md.switchable       = false;
        md.priority         = 1;
        md.frameType        = 0;
        md.DID              = 0;
        md.TID              = 0;
        md.maxTID           = 1;

        int32_t ret = m_rtpVideoChannel->SendMediaData(&md);
        EXPECT_EQ(0, ret);
    }

    /*
    for (int ii = 0; ii < 10; ++ii) {
        if (m_transportMock->GetSendRTPPackets().empty()) {
            ::SleepMs(20);
        } else {
            break;
        }
    }
     */

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    const vector<IWmeMediaPackage *> &sendPackets = m_transportMock->GetSendRTPPackets();
    ASSERT_EQ(1, sendPackets.size());
    IWmeMediaPackage *package = sendPackets.front();

    CCmMessageBlock *mb = nullptr;
    ConvertMediaPackageToMessageBlock(package, mb);

    CRTPPacket rtpPacket;
    EXPECT_EQ(0, rtpPacket.Unpack(*mb));

    EXPECT_EQ(aggLength, rtpPacket.GetPayloadLength());
    EXPECT_EQ(0, memcmp(aggregatedBuffer, rtpPacket.GetPayloadData(), aggLength));

    mb->DestroyChained();
    mb = nullptr;
}

TEST_F(CVideoPacketizationTest, combined_single_aggregate_fragement_RTP_packet_Test)
{
    CScopedTracer test_info;
    m_rtpVideoSession->SetPacketizationMode(PACKETIZATION_MODE1);

    // nal0: single
    // nal1 + nal2: aggregated
    // nal3: fragmented
    const uint32_t NALCount = 4;
    uint8_t nal0[] = {PHT_SINGLE, 0, 0, '1', '2', '3'};
    uint8_t nal1[] = {PHT_SINGLE, 0, 0, 'a', 'b', 'c', 'd'};
    uint8_t nal2[] = {PHT_SINGLE, 0, 0, '1', '2', '3'};
    uint8_t nal3[1500] = {PHT_SINGLE, 0, 0, 'a', 'b', 'c', 'd', 'e', 'f'};

    struct NALData {
        uint8_t *dataptr;
        uint16_t length;
    };
    NALData nals[NALCount] = { {nal0, sizeof(nal0)}, {nal1, sizeof(nal1)}, {nal2, sizeof(nal2)}, {nal3, sizeof(nal3)}};
    for (int ii = 0; ii < NALCount; ++ii) {
        PAYLOAD_SET_LENGTH(nals[ii].dataptr, nals[ii].length);
    }

    m_rtpVideoSession->EnableSendingAggregationPacket(true);
    for (int ii = 0; ii < NALCount; ++ii) {
        WRTPMediaDataVideo md;
        md.type             = MEDIA_DATA_VIDEO;
        md.timestamp        = 1234;
        md.sampleTimestamp  = 5678;
        md.codecType        = CODEC_TYPE_VIDEO_TEST;
        md.marker           = (ii != NALCount -1) ? 0 : 1;
        md.size             = nals[ii].length;
        md.data             = nals[ii].dataptr;
        //        md.layerNumber      = 1;
        //        md.layerIndex       = 0;
        md.layerWidth       = 160;
        md.layerHeight      = 90;
        md.layerMapAll      = false;
        md.frame_idc        = 1;
        md.disposable       = false;
        md.switchable       = false;
        md.priority         = 1;
        md.frameType        = 0;
        md.DID              = 0;
        md.TID              = (ii == 0) ? 0 : 1; // the TID makes nal1 and nal2 can be aggregated
        md.maxTID           = 2;

        int32_t ret = m_rtpVideoChannel->SendMediaData(&md);
        EXPECT_EQ(0, ret);
    }

    /*
    for (int ii = 0; ii < 10; ++ii) {
        if (m_transportMock->GetSendRTPPackets().empty()) {
            ::SleepMs(20);
        } else {
            break;
        }
    }
     */

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    ::SleepMs(100);
    const vector<IWmeMediaPackage *> &sendPackets = m_transportMock->GetSendRTPPackets();
    ASSERT_EQ(4, sendPackets.size());
}