#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "mmframemgr.h"
#include "testutil.h"
#include "rtpsessionconfig.h"

using namespace wrtp;

///////////////////////////////////////////////////////////////////////////////
class CFragmentUnitTest : public ::testing::Test
{
public:
    CFragmentUnitTest() : m_bufferSize(1024), m_fragment()
    {
    }

protected:
    virtual void SetUp()
    {
        FragmentContainer fragments;
        m_mgr.GetFragments(1, fragments, 1024, nullptr);
        ASSERT_TRUE(!fragments.empty());
        m_fragment = std::move(fragments[0]);
        ASSERT_TRUE(!!m_fragment);
    }

    virtual void TearDown()
    {
        m_fragment.reset();
    }

protected:
    uint32_t m_bufferSize;
    CMMFrameManager m_mgr;
    CFragmentUniquePtr m_fragment;
};

TEST_F(CFragmentUnitTest, Test_GetPacketLength_EmptyFragment)
{
    CScopedTracer test_info;
    // one csrc, one timeoffset ext, one MARI timestamp
    EXPECT_EQ(RTP_FIX_HEADER_LENGTH+4+RTP_FIX_HEADER_EXTENSION_LENGTH+4+8, m_fragment->GetPacketLength());
}

TEST_F(CFragmentUnitTest, Test_Get_Set_Marker)
{
    CScopedTracer test_info;
    uint8_t marker = m_fragment->GetMarker();
    EXPECT_EQ(0, marker);

    m_fragment->SetMarker(1);
    marker = m_fragment->GetMarker();
    EXPECT_EQ(1, marker);
}

TEST_F(CFragmentUnitTest, Test_Buffer_and_Size)
{
    CScopedTracer test_info;
    EXPECT_TRUE(nullptr != m_fragment->GetDataBuffer());
    EXPECT_TRUE(m_bufferSize == m_fragment->GetBufferSize());
}

TEST_F(CFragmentUnitTest, Test_Data_Size)
{
    CScopedTracer test_info;
    EXPECT_TRUE(0 == m_fragment->GetDataSize());

    uint32_t dataSize = m_bufferSize / 2;
    m_fragment->SetDataSize(dataSize);
    EXPECT_EQ(dataSize, m_fragment->GetDataSize());
}


TEST_F(CFragmentUnitTest, Test_CopyDataIn_underflow_then_return_sucess)
{
    CScopedTracer test_info;
    char *data = const_cast<char *>("hello");
    uint32_t dataSize = strlen(data) + 1;

    int32_t ret = m_fragment->CopyDataIn(reinterpret_cast<uint8_t *>(data), dataSize);
    EXPECT_TRUE(0 == ret);

    uint8_t *dataPtr = m_fragment->GetDataBuffer();
    //EXPECT_STREQ(data, reinterpret_cast<const char*>(dataPtr));

    int len2 = m_fragment->GetDataSize();
    int len1 = dataSize;
    ASSERT_EQ(len1, len2);
    EXPECT_EQ(0, memcmp(data, dataPtr, len1));
}

TEST_F(CFragmentUnitTest, Test_CopyDataIn_overflow_then_return_failure)
{
    CScopedTracer test_info;
    uint32_t dataLen = m_bufferSize + 1;
    std::unique_ptr<uint8_t[]> data(new uint8_t[dataLen]);
    int32_t ret = m_fragment->CopyDataIn(data.get(), dataLen);
    EXPECT_TRUE(0 != ret);

}

TEST_F(CFragmentUnitTest, Test_Get_Container)
{
    CScopedTracer test_info;
    EXPECT_EQ(nullptr, m_fragment->GetContainer());
}

///////////////////////////////////////////////////////////////////////////////
class CNALUnitTest : public ::testing::Test
{
public:
    CNALUnitTest() : m_nal()
    {
    }

protected:
    virtual void SetUp()
    {
        m_nal = m_mgr.GetNALUnit(nullptr);
    }

    virtual void TearDown()
    {
        m_nal.reset();
    }

protected:
    CMMFrameManager m_mgr;
    CNALUniquePtr m_nal;
};

TEST_F(CNALUnitTest, Test_Create_Single_Fragment)
{
    CScopedTracer test_info;
    EXPECT_TRUE(m_nal->IsEmpty());

    uint32_t dataSize = 1024;
    auto &fragments = m_nal->CreateFragment(dataSize, 1, &m_mgr);
    auto &fu = fragments[0];
    ASSERT_TRUE(!!fu);
    EXPECT_EQ(dataSize, fu->GetBufferSize());
    EXPECT_EQ(0, fu->GetDataSize());
    EXPECT_FALSE(m_nal->IsEmpty());

    m_nal->SetMarker(1);
    EXPECT_EQ(1, fu->GetMarker());

    CCmMessageBlock* mb = nullptr;
    CRTPPacketMetaInfo pktInfo;
    bool ret = m_nal->Front(mb, pktInfo);
    EXPECT_TRUE(ret);
    m_nal->PopFront();
    EXPECT_TRUE(m_nal->IsEmpty());
}

TEST_F(CNALUnitTest, Test_Create_Multiple_Fragments)
{
    CScopedTracer test_info;
    EXPECT_TRUE(m_nal->IsEmpty());

    uint32_t dataSize = 1024;
    const uint32_t count = 4;
    auto& fragments = m_nal->CreateFragment(dataSize, count, &m_mgr);
    ASSERT_TRUE(fragments.size() == count);

    m_nal->SetMarker(1);
    for (int ii = 0; ii < count-1; ++ii) {
        EXPECT_EQ(0, fragments[ii]->GetMarker());
    }
    EXPECT_EQ(1, fragments[count-1]->GetMarker());
}

TEST_F(CNALUnitTest, Test_GetMaxFragmentSize)
{
    CScopedTracer test_info;

    auto& fragments = m_nal->CreateFragment(234, 2, &m_mgr);
    ASSERT_TRUE(fragments.size()==2);
    EXPECT_TRUE(!!fragments[0]);
    EXPECT_TRUE(!!fragments[1]);

    uint8_t data[234];
    fragments[0]->CopyDataIn(data, 123);
    fragments[1]->CopyDataIn(data, 234);

    m_nal->GetTotalPacketLength();
    EXPECT_EQ(234+RTP_FIX_HEADER_LENGTH+4+RTP_FIX_HEADER_EXTENSION_LENGTH+4+8, m_nal->GetMaxFragmentSize());
}

///////////////////////////////////////////////////////////////////////////////
class CFrameUnitTest : public ::testing::Test
{
public:
    CFrameUnitTest()
        : m_channelId(1)
        , m_ssrc(123)
        , m_payloadType(99)
        , m_clockRate(90000)
        , m_timeoffsetExtId(3)
        , m_captureTimestamp(800)
        , m_sampleTimestamp(1234)
        , m_sessionContext(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO))
    {
    }

protected:
    virtual void SetUp()
    {
        CCodecInfo codecInfo(1, 99, 90000);
        m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(codecInfo);
        uint8_t *payload = (uint8_t *)m_payloads;
        for (int i = 0; i < 3*300; ++i) {
            *payload = i;
            ++payload;
        }
    }

    virtual void TearDown()
    {
    }
    
    CSendFramePtr CreateTestFrame()
    {
        CSendFramePtr frame = ConstructFrameUnit(m_channelId, m_ssrc, m_payloadType, m_clockRate,
                                     m_timeoffsetExtId, m_captureTimestamp, m_sampleTimestamp, 0, m_sessionContext);
        EXPECT_TRUE(!!frame);
        return frame;
    }

protected:
    uint32_t  m_channelId;
    uint32_t  m_ssrc;
    uint8_t   m_payloadType;
    uint32_t  m_clockRate;
    uint8_t   m_timeoffsetExtId;

    uint32_t  m_captureTimestamp;
    uint32_t  m_sampleTimestamp;
    uint8_t       m_payloads[3][300];
    RTPSessionContextSharedPtr m_sessionContext;
    CMMFrameManager m_mgr;
};

TEST_F(CFrameUnitTest, Test_Internal_Status)
{
    CScopedTracer test_info;
    CSendFramePtr frame = CreateTestFrame();
    auto& metaInfo = frame->GetMediaMetaInfo();
    EXPECT_EQ(m_channelId,          metaInfo.GetChannelId());
    EXPECT_EQ(m_ssrc,               frame->GetSSRC());
    EXPECT_EQ(m_payloadType,        metaInfo.GetPayloadType());
    EXPECT_EQ(m_clockRate,          frame->GetClockRate());
    EXPECT_EQ(m_captureTimestamp,   metaInfo.GetCaptureTickMS());
    EXPECT_EQ(m_sampleTimestamp,    metaInfo.GetRTPTimestamp());
    EXPECT_TRUE(frame->IsEmpty());
}

TEST_F(CFrameUnitTest, Test_Create_NAL)
{
    CScopedTracer test_info;
    CSendFramePtr frame = CreateTestFrame();
    EXPECT_TRUE(frame->IsEmpty());

    auto &nal = frame->CreateNAL();
    EXPECT_TRUE(!!nal);
    EXPECT_FALSE(frame->IsEmpty());
    //EXPECT_EQ(1, frame->GetNALs().size());
}

TEST_F(CFrameUnitTest, Test_Sequence)
{
    CScopedTracer test_info;
    CSendFramePtr frame = CreateTestFrame();
    EXPECT_TRUE(frame->IsEmpty());

    auto &nal = frame->CreateNAL();
    EXPECT_TRUE(!!nal);
    EXPECT_FALSE(frame->IsEmpty());
    //EXPECT_EQ(1, m_frame->GetNALs().size());

    auto& fragments = nal->CreateFragment(234, 3, &m_mgr);
    ASSERT_TRUE(fragments.size() == 3);

    fragments[0]->CopyDataIn(m_payloads[0], 123);
    fragments[1]->CopyDataIn(m_payloads[1], 234);
    fragments[2]->CopyDataIn(m_payloads[2], 200);

    for (int i = 0; i < 3; ++i) {
        EXPECT_FALSE(frame->IsEmpty());
        CCmMessageBlock* mb = nullptr;
        CRTPPacketMetaInfo pktInfo;
        bool ret = frame->Front(mb, pktInfo);
        EXPECT_TRUE(ret);
        ASSERT_EQ(3, pktInfo.GetPacketsOfFrame());
        ASSERT_EQ(i, pktInfo.GetPacketIndex());
        frame->PopFront();
    }
}

#define MAX_RTP_PACKET_LEN 258

TEST_F(CFrameUnitTest, Test_FrameUnit_Front_Normal)
{
    CScopedTracer test_info;
    CSendFramePtr frame = CreateTestFrame();
    ASSERT_TRUE(frame->IsEmpty());

    auto &nal = frame->CreateNAL();
    ASSERT_FALSE(frame->IsEmpty());
    //ASSERT_EQ(1, frame->GetNALs().size());

    auto& fragments = nal->CreateFragment(234, 2, &m_mgr);
    ASSERT_TRUE(fragments.size() == 2);

    fragments[0]->CopyDataIn(m_payloads[0], 123);
    fragments[1]->CopyDataIn(m_payloads[1], 234);

    nal->GetTotalPacketLength();
    ASSERT_EQ(MAX_RTP_PACKET_LEN, nal->GetMaxFragmentSize());

    for (int i = 0; i < 2; ++i) {
        ASSERT_FALSE(frame->IsEmpty());
        CCmMessageBlock* mb = nullptr;
        CRTPPacketMetaInfo pktInfo;
        bool ret = frame->Front(mb, pktInfo);
        EXPECT_TRUE(ret);
        ASSERT_EQ(2, pktInfo.GetPacketsOfFrame());
        ASSERT_EQ(i, pktInfo.GetPacketIndex());

        frame->PopFront();
    }
}

TEST_F(CFrameUnitTest, Test_FrameUnit_Front_Normal_Empty)
{
    CScopedTracer test_info;
    CSendFramePtr frame = CreateTestFrame();
    ASSERT_TRUE(frame->IsEmpty());

    auto &nal = frame->CreateNAL();
    ASSERT_FALSE(frame->IsEmpty());
    //ASSERT_EQ(1, frame->GetNALs().size());

    auto& fragments = nal->CreateFragment(234, 2, &m_mgr);
    ASSERT_TRUE(fragments.size() == 2);

    fragments[0]->CopyDataIn(m_payloads[0], 123);
    fragments[1]->CopyDataIn(m_payloads[1], 234);

    nal->GetTotalPacketLength();
    ASSERT_EQ(MAX_RTP_PACKET_LEN, nal->GetMaxFragmentSize());

    CCmMessageBlock* mb = nullptr;
    CRTPPacketMetaInfo pktInfo;
    for (int i = 0; i < 2; ++i) {
        ASSERT_FALSE(frame->IsEmpty());
        bool ret = frame->Front(mb, pktInfo);
        EXPECT_TRUE(ret);
        ASSERT_EQ(2, pktInfo.GetPacketsOfFrame());
        ASSERT_EQ(i, pktInfo.GetPacketIndex());

        frame->PopFront();
    }
    ASSERT_EQ(234+12+4+4+4, pktInfo.GetPacketLength());
    EXPECT_FALSE(frame->Front(mb, pktInfo));
    EXPECT_TRUE(frame->IsEmpty());
}
