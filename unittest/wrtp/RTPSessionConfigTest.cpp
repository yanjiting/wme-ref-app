#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "rtpsessionconfig.h"
#include "testutil.h"

using namespace wrtp;
class CRTPSessionContextTest : public ::testing::Test
{
public:
    CRTPSessionContextTest() : m_sessionContext() {}
    ~CRTPSessionContextTest() {}

protected:
    virtual void SetUp()
    {
        m_sessionContext.reset(new CRTPSessionContext(RTP_SESSION_WEBEX_VIDEO));
    }

    virtual void TearDown()
    {

    }

protected:
    RTPSessionContextSharedPtr m_sessionContext;
};

//////////////////////////////////////////////////////
// Session bandwidth test
TEST_F(CRTPSessionContextTest, Test_GetSessionBandwidth_return_error_if_not_set)
{
    CScopedTracer test_info;
    uint32_t bandwidth = 0;
    int32_t ret = m_sessionContext->GetOutboundConfig()->GetSessionBandwidth(bandwidth);
    ASSERT_TRUE(ret != WRTP_ERR_NOERR);
}

TEST_F(CRTPSessionContextTest, Test_SetSessionBandwidth_return_eror_if_bandwidth_is_0)
{
    CScopedTracer test_info;
    uint32_t bandwidth = 0;
    int32_t ret = m_sessionContext->GetOutboundConfig()->SetSessionBandwidth(bandwidth);
    ASSERT_TRUE(ret != WRTP_ERR_NOERR);
}

TEST_F(CRTPSessionContextTest, Test_Set_GetSessionBandwidth_return_sucess_if_bandwidth_is_ok)
{
    CScopedTracer test_info;
    uint32_t originalBandwidth = 1024*1024; // 1M bytes/sec
    int32_t ret = m_sessionContext->GetOutboundConfig()->SetSessionBandwidth(originalBandwidth);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    uint32_t bandwidth = 0;
    ret = m_sessionContext->GetOutboundConfig()->GetSessionBandwidth(bandwidth);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    ASSERT_EQ(originalBandwidth, bandwidth);
}

//////////////////////////////////////////////////////
// Codec info test

TEST_F(CRTPSessionContextTest, Test_RegisterCodecInfo_return_success_if_codec_info_is_ok)
{
    CScopedTracer test_info;
    CCodecInfo audioCodecInfo(1, 101, 48000);
    CCodecInfo videoCodecInfo(2, 98, 90000);

    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
}


TEST_F(CRTPSessionContextTest, Test_RegisterCodecInfo_return_success_if_just_change_the_payload_type)
{
    CScopedTracer test_info;
    CCodecInfo videoCodecInfo(2, 98, 90000);

    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    CCodecInfo queriedCodecInfo;
    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(videoCodecInfo.codecType,     queriedCodecInfo.codecType);
    EXPECT_EQ(videoCodecInfo.payloadType,   queriedCodecInfo.payloadType);
    EXPECT_EQ(videoCodecInfo.clockRate,     queriedCodecInfo.clockRate);

    // change the payload type
    videoCodecInfo.payloadType = 99;

    ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(videoCodecInfo.codecType,     queriedCodecInfo.codecType);
    EXPECT_EQ(videoCodecInfo.payloadType,   queriedCodecInfo.payloadType);
    EXPECT_EQ(videoCodecInfo.clockRate,     queriedCodecInfo.clockRate);
}

TEST_F(CRTPSessionContextTest, Test_RegisterCodecInfo_return_error_if_change_the_clockrate)
{
    CScopedTracer test_info;
    CCodecInfo audioCodecInfo(1, 101, 48000);
    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    // re-register should be ok
    ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    // re-register after modifing the clockrate
    audioCodecInfo.clockRate = 8000;
    ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);
}

TEST_F(CRTPSessionContextTest, Test_Query_return_sucess_if_codec_info_registered)
{
    CScopedTracer test_info;
    const CodecType codec     = 2;
    const uint8_t payloadType   = 98;
    const uint32_t clockRate    = 90000;
    const CCodecInfo videoCodecInfo(codec, payloadType, clockRate);

    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    CCodecInfo queriedCodecInfo;
    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(codec, queriedCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(codec,        queriedCodecInfo.codecType);
    EXPECT_EQ(payloadType,  queriedCodecInfo.payloadType);
    EXPECT_EQ(clockRate,    queriedCodecInfo.clockRate);

    uint8_t queriedPayloadType = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryPayloadType(codec, queriedPayloadType);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(payloadType, queriedPayloadType);

    uint32_t queriedClockRate = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryClockRate(codec, queriedClockRate);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(clockRate, queriedClockRate);

    ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    CodecType queriedCodecType = 0;
    ret = m_sessionContext->GetInboundConfig()->QueryCodecTypeByPayloadType(payloadType, queriedCodecType);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(codec, queriedCodecType);

}

TEST_F(CRTPSessionContextTest, Test_Query_return_error_if_codec_not_registered)
{
    CScopedTracer test_info;
    const CodecType codec     = 2;
    const uint8_t payloadType   = 98;
    const uint32_t clockRate    = 90000;
    const CCodecInfo videoCodecInfo(codec, payloadType, clockRate);

    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    const CodecType unknownCodec    = 20;
    const uint8_t unknownPayloadType  = 101;

    CCodecInfo queriedCodecInfo;
    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(unknownCodec, queriedCodecInfo);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);

    uint8_t queriedPayloadType = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryPayloadType(unknownCodec, queriedPayloadType);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);

    uint32_t queriedClockRate = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryClockRate(unknownCodec, queriedClockRate);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);


    ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    CodecType queriedCodecType = 0;
    ret = m_sessionContext->GetInboundConfig()->QueryCodecTypeByPayloadType(unknownPayloadType, queriedCodecType);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);

}
//////////////////////////////////////////////////////
// Codec info test with _STREAM_INOUT direction

TEST_F(CRTPSessionContextTest, Test_RegisterCodecInfo_STREAM_INOUT_return_success_if_codec_info_is_ok)
{
    CScopedTracer test_info;
    CCodecInfo audioCodecInfo(1, 101, 48000);
    CCodecInfo videoCodecInfo(2, 98, 90000);

    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
}


TEST_F(CRTPSessionContextTest, Test_RegisterCodecInfo_STREAM_INOUT_return_success_if_just_change_the_payload_type)
{
    CScopedTracer test_info;
    CCodecInfo videoCodecInfo(2, 98, 90000);

    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    CCodecInfo queriedCodecInfo;
    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(videoCodecInfo.codecType,     queriedCodecInfo.codecType);
    EXPECT_EQ(videoCodecInfo.payloadType,   queriedCodecInfo.payloadType);
    EXPECT_EQ(videoCodecInfo.clockRate,     queriedCodecInfo.clockRate);

    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(videoCodecInfo.codecType,     queriedCodecInfo.codecType);
    EXPECT_EQ(videoCodecInfo.payloadType,   queriedCodecInfo.payloadType);
    EXPECT_EQ(videoCodecInfo.clockRate,     queriedCodecInfo.clockRate);

    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(videoCodecInfo.codecType,     queriedCodecInfo.codecType);
    EXPECT_EQ(videoCodecInfo.payloadType,   queriedCodecInfo.payloadType);
    EXPECT_EQ(videoCodecInfo.clockRate,     queriedCodecInfo.clockRate);

    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(videoCodecInfo.codecType,     queriedCodecInfo.codecType);
    EXPECT_EQ(videoCodecInfo.payloadType,   queriedCodecInfo.payloadType);
    EXPECT_EQ(videoCodecInfo.clockRate,     queriedCodecInfo.clockRate);



    // change the payload type
    videoCodecInfo.payloadType = 99;

    ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(videoCodecInfo.codecType,     queriedCodecInfo.codecType);
    EXPECT_EQ(videoCodecInfo.payloadType,   queriedCodecInfo.payloadType);
    EXPECT_EQ(videoCodecInfo.clockRate,     queriedCodecInfo.clockRate);

    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(videoCodecInfo.codecType,     queriedCodecInfo.codecType);
    EXPECT_EQ(videoCodecInfo.payloadType,   queriedCodecInfo.payloadType);
    EXPECT_EQ(videoCodecInfo.clockRate,     queriedCodecInfo.clockRate);

    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(videoCodecInfo.codecType,     queriedCodecInfo.codecType);
    EXPECT_EQ(videoCodecInfo.payloadType,   queriedCodecInfo.payloadType);
    EXPECT_EQ(videoCodecInfo.clockRate,     queriedCodecInfo.clockRate);

    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(videoCodecInfo.codecType,     queriedCodecInfo.codecType);
    EXPECT_EQ(videoCodecInfo.payloadType,   queriedCodecInfo.payloadType);
    EXPECT_EQ(videoCodecInfo.clockRate,     queriedCodecInfo.clockRate);
}

TEST_F(CRTPSessionContextTest, Test_RegisterCodecInfo_STREAM_INOUT_return_error_if_change_the_clockrate)
{
    CScopedTracer test_info;
    CCodecInfo audioCodecInfo(1, 101, 48000);
    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    // re-register should be ok
    ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    // re-register after modifing the clockrate
    audioCodecInfo.clockRate = 8000;
    ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);
}

TEST_F(CRTPSessionContextTest, Test_Query_STREAM_INOUT_return_sucess_if_codec_info_registered)
{
    CScopedTracer test_info;
    const CodecType codec     = 2;
    const uint8_t payloadType   = 98;
    const uint32_t clockRate    = 90000;
    const CCodecInfo videoCodecInfo(codec, payloadType, clockRate);

    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    CCodecInfo queriedCodecInfo;
    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(codec, queriedCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(codec,        queriedCodecInfo.codecType);
    EXPECT_EQ(payloadType,  queriedCodecInfo.payloadType);
    EXPECT_EQ(clockRate,    queriedCodecInfo.clockRate);


    uint8_t queriedPayloadType = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryPayloadType(codec, queriedPayloadType);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(payloadType, queriedPayloadType);

    uint32_t queriedClockRate = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryClockRate(codec, queriedClockRate);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(clockRate, queriedClockRate);

    ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    CodecType queriedCodecType = 0;
    ret = m_sessionContext->GetInboundConfig()->QueryCodecTypeByPayloadType(payloadType, queriedCodecType);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(codec, queriedCodecType);

}

TEST_F(CRTPSessionContextTest, Test_Query_STREAM_INOUT_return_error_if_codec_not_registered)
{
    CScopedTracer test_info;
    const CodecType codec     = 2;
    const uint8_t payloadType   = 98;
    const uint32_t clockRate    = 90000;
    const CCodecInfo videoCodecInfo(codec, payloadType, clockRate);

    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    const CodecType unknownCodec    = 20;
    const uint8_t unknownPayloadType  = 101;

    CCodecInfo queriedCodecInfo;
    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(unknownCodec, queriedCodecInfo);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);

    uint8_t queriedPayloadType = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryPayloadType(unknownCodec, queriedPayloadType);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);

    uint32_t queriedClockRate = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryClockRate(unknownCodec, queriedClockRate);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);

    CodecType queriedCodecType = 0;

    ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    ret = m_sessionContext->GetInboundConfig()->QueryCodecTypeByPayloadType(unknownPayloadType, queriedCodecType);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);

}


//////////////////////////////////////////////////////
// Codec info test with _STREAM_IN direction

TEST_F(CRTPSessionContextTest, Test_RegisterCodecInfo_STREAM_IN_return_success_if_codec_info_is_ok)
{
    CScopedTracer test_info;
    CCodecInfo audioCodecInfo(1, 101, 48000);
    CCodecInfo videoCodecInfo(2, 98, 90000);

    int32_t ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
}


TEST_F(CRTPSessionContextTest, Test_RegisterCodecInfo_STREAM_IN_return_success_if_just_change_the_payload_type)
{
    CScopedTracer test_info;
    CCodecInfo videoCodecInfo(2, 98, 90000);

    int32_t ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    CCodecInfo queriedCodecInfo;

    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(WRTP_ERR_UNKNOWN_CODEC_TYPE, ret);

    // change the payload type
    videoCodecInfo.payloadType = 99;

    ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(WRTP_ERR_UNKNOWN_CODEC_TYPE, ret);


    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(WRTP_ERR_UNKNOWN_CODEC_TYPE, ret);
}

TEST_F(CRTPSessionContextTest, Test_RegisterCodecInfo_STREAM_IN_return_error_if_change_the_clockrate)
{
    CScopedTracer test_info;
    CCodecInfo audioCodecInfo(1, 101, 48000);
    int32_t ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    // re-register should be ok
    ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    // re-register after modifing the clockrate
    audioCodecInfo.clockRate = 8000;
    ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);
}

TEST_F(CRTPSessionContextTest, Test_Query_STREAM_IN_return_sucess_if_codec_info_registered)
{
    CScopedTracer test_info;
    const CodecType codec     = 2;
    const uint8_t payloadType   = 98;
    const uint32_t clockRate    = 90000;
    const CCodecInfo videoCodecInfo(codec, payloadType, clockRate);

    int32_t ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    CCodecInfo queriedCodecInfo;
    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(codec, queriedCodecInfo);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);


    uint8_t queriedPayloadType = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryPayloadType(codec, queriedPayloadType);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);

    uint32_t queriedClockRate = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryClockRate(codec, queriedClockRate);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);


    CodecType queriedCodecType = 0;

    ret = m_sessionContext->GetInboundConfig()->QueryCodecTypeByPayloadType(payloadType, queriedCodecType);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(codec, queriedCodecType);

}

TEST_F(CRTPSessionContextTest, Test_Query_STREAM_IN_return_error_if_codec_not_registered)
{
    CScopedTracer test_info;
    const CodecType codec     = 2;
    const uint8_t payloadType   = 98;
    const uint32_t clockRate    = 90000;
    const CCodecInfo videoCodecInfo(codec, payloadType, clockRate);

    int32_t ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    const CodecType unknownCodec    = 20;
    const uint8_t unknownPayloadType  = 101;

    CCodecInfo queriedCodecInfo;
    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(unknownCodec, queriedCodecInfo);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);

    uint8_t queriedPayloadType = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryPayloadType(unknownCodec, queriedPayloadType);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);

    uint32_t queriedClockRate = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryClockRate(unknownCodec, queriedClockRate);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);

    CodecType queriedCodecType = 0;
    ret = m_sessionContext->GetInboundConfig()->QueryCodecTypeByPayloadType(unknownPayloadType, queriedCodecType);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);

}

//////////////////////////////////////////////////////
// Codec info test with _STREAM_OUT direction

TEST_F(CRTPSessionContextTest, Test_RegisterCodecInfo_STREAM_OUT_return_success_if_codec_info_is_ok)
{
    CScopedTracer test_info;
    CCodecInfo audioCodecInfo(1, 101, 48000);
    CCodecInfo videoCodecInfo(2, 98, 90000);

    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    ret = m_sessionContext->GetInboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
}


TEST_F(CRTPSessionContextTest, Test_RegisterCodecInfo_STREAM_OUT_return_success_if_just_change_the_payload_type)
{
    CScopedTracer test_info;
    CCodecInfo videoCodecInfo(2, 98, 90000);

    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    CCodecInfo queriedCodecInfo;
    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(videoCodecInfo.codecType,     queriedCodecInfo.codecType);
    EXPECT_EQ(videoCodecInfo.payloadType,   queriedCodecInfo.payloadType);
    EXPECT_EQ(videoCodecInfo.clockRate,     queriedCodecInfo.clockRate);

    // change the payload type
    videoCodecInfo.payloadType = 99;

    ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(videoCodecInfo.codecType, queriedCodecInfo);
    EXPECT_EQ(videoCodecInfo.codecType,     queriedCodecInfo.codecType);
    EXPECT_EQ(videoCodecInfo.payloadType,   queriedCodecInfo.payloadType);
    EXPECT_EQ(videoCodecInfo.clockRate,     queriedCodecInfo.clockRate);


}

TEST_F(CRTPSessionContextTest, Test_RegisterCodecInfo_STREAM_OUT_return_error_if_change_the_clockrate)
{
    CScopedTracer test_info;
    CCodecInfo audioCodecInfo(1, 101, 48000);
    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    // re-register should be ok
    ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    // re-register after modifing the clockrate
    audioCodecInfo.clockRate = 8000;
    ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(audioCodecInfo);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);
}

TEST_F(CRTPSessionContextTest, Test_Query_STREAM_OUT_return_sucess_if_codec_info_registered)
{
    CScopedTracer test_info;
    const CodecType codec     = 2;
    const uint8_t payloadType   = 98;
    const uint32_t clockRate    = 90000;
    const CCodecInfo videoCodecInfo(codec, payloadType, clockRate);

    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    CCodecInfo queriedCodecInfo;
    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(codec, queriedCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(codec,        queriedCodecInfo.codecType);
    EXPECT_EQ(payloadType,  queriedCodecInfo.payloadType);
    EXPECT_EQ(clockRate,    queriedCodecInfo.clockRate);


    uint8_t queriedPayloadType = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryPayloadType(codec, queriedPayloadType);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(payloadType, queriedPayloadType);


    uint32_t queriedClockRate = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryClockRate(codec, queriedClockRate);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);
    EXPECT_EQ(clockRate, queriedClockRate);


    CodecType queriedCodecType = 0;
    ret = m_sessionContext->GetInboundConfig()->QueryCodecTypeByPayloadType(payloadType, queriedCodecType);
    EXPECT_EQ(ret,WRTP_ERR_UNKNOWN_PAYLOAD_TYPE);

}

TEST_F(CRTPSessionContextTest, Test_Query_STREAM_OUT_return_error_if_codec_not_registered)
{
    CScopedTracer test_info;
    const CodecType codec     = 2;
    const uint8_t payloadType   = 98;
    const uint32_t clockRate    = 90000;
    const CCodecInfo videoCodecInfo(codec, payloadType, clockRate);

    int32_t ret = m_sessionContext->GetOutboundConfig()->RegisterCodecInfo(videoCodecInfo);
    EXPECT_TRUE(ret == WRTP_ERR_NOERR);

    const CodecType unknownCodec    = 20;
    const uint8_t unknownPayloadType  = 101;

    CCodecInfo queriedCodecInfo;
    ret = m_sessionContext->GetOutboundConfig()->QueryCodecInfo(unknownCodec, queriedCodecInfo);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);


    uint8_t queriedPayloadType = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryPayloadType(unknownCodec, queriedPayloadType);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);


    uint32_t queriedClockRate = 0;
    ret = m_sessionContext->GetOutboundConfig()->QueryClockRate(unknownCodec, queriedClockRate);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);

    CodecType queriedCodecType = 0;
    ret = m_sessionContext->GetInboundConfig()->QueryCodecTypeByPayloadType(unknownPayloadType, queriedCodecType);
    EXPECT_TRUE(ret != WRTP_ERR_NOERR);


}

TEST_F(CRTPSessionContextTest, Test_SetGetMariEncoderManager)
{
    FecLevelSharedPtr fecLevelHandle(new DYNAMIC_FEC_LEVEL_FUNC());
    MariFECInfo fecInfo;
    CCmSharedPtr<CMariEncoderManager> encoder(new CMariEncoderManager(fecInfo, 8000, 3, fecLevelHandle));
    m_sessionContext->GetOutboundConfig()->SetMariEncoderManager(encoder);
    CCmSharedPtr<CMariEncoderManager> ret = m_sessionContext->GetOutboundConfig()->GetMariEncoderManager();
    if (ret) {
        ASSERT_TRUE(true);
    } else {
        ASSERT_TRUE(false);
    }
}

TEST_F(CRTPSessionContextTest, Test_SetGetMariDecoderManager)
{
    RECV_RECOVERED_PACKET recvHandle;
    MariFECInfo fecInfo;
    CCmSharedPtr<CMariDecoderManager> decoder(new CMariDecoderManager(fecInfo, 8000, recvHandle));
    m_sessionContext->GetInboundConfig()->SetMariDecoderManager(decoder);
    CCmSharedPtr<CMariDecoderManager> ret = m_sessionContext->GetInboundConfig()->GetMariDecoderManager();
    if (ret) {
        ASSERT_TRUE(true);
    } else {
        ASSERT_TRUE(false);
    }
}

TEST_F(CRTPSessionContextTest, Test_In_SetSrtpFecOrder)
{
    int32_t rv = m_sessionContext->GetInboundConfig()->SetSrtpFecOrder(wrtp::ORDER_SRTP_FEC, true);
    ASSERT_EQ((int32_t)WRTP_ERR_NOERR, rv);
    wrtp::SRTPFecOrder getOrder = m_sessionContext->GetInboundConfig()->GetSrtpFecOrder();
    EXPECT_EQ(wrtp::ORDER_SRTP_FEC, getOrder);
}

TEST_F(CRTPSessionContextTest, Test_Out_SetSrtpFecOrder)
{
    int32_t rv = m_sessionContext->GetOutboundConfig()->SetSrtpFecOrder(wrtp::ORDER_SRTP_FEC, true);
    ASSERT_EQ((int32_t)WRTP_ERR_NOERR, rv);
    wrtp::SRTPFecOrder getOrder = m_sessionContext->GetOutboundConfig()->GetSrtpFecOrder();
    EXPECT_EQ(wrtp::ORDER_SRTP_FEC, getOrder);
}

TEST_F(CRTPSessionContextTest, Test_In_SetSrtpFecOrder_Mari_Srtp)
{
    int32_t rv = m_sessionContext->GetInboundConfig()->SetSrtpFecOrder(wrtp::ORDER_SRTP_FEC, true);
    ASSERT_EQ((int32_t)WRTP_ERR_NOERR, rv);
    rv = m_sessionContext->GetInboundConfig()->SetSrtpFecOrder(wrtp::ORDER_FEC_SRTP, false);
    ASSERT_EQ((int32_t)WRTP_ERR_NOERR, rv);
    wrtp::SRTPFecOrder getOrder = m_sessionContext->GetInboundConfig()->GetSrtpFecOrder();
    EXPECT_EQ(wrtp::ORDER_SRTP_FEC, getOrder);
}

TEST_F(CRTPSessionContextTest, Test_In_SetSrtpFecOrder_Srtp_Mari)
{
    int32_t rv = m_sessionContext->GetInboundConfig()->SetSrtpFecOrder(wrtp::ORDER_SRTP_FEC, false);
    ASSERT_EQ((int32_t)WRTP_ERR_NOERR, rv);
    rv = m_sessionContext->GetInboundConfig()->SetSrtpFecOrder(wrtp::ORDER_FEC_SRTP, true);
    ASSERT_EQ((int32_t)WRTP_ERR_NOERR, rv);
    wrtp::SRTPFecOrder getOrder = m_sessionContext->GetInboundConfig()->GetSrtpFecOrder();
    EXPECT_EQ(wrtp::ORDER_FEC_SRTP, getOrder);
}

TEST_F(CRTPSessionContextTest, Test_Out_SetSrtpFecOrder_Mari_Srtp)
{
    int32_t rv = m_sessionContext->GetOutboundConfig()->SetSrtpFecOrder(wrtp::ORDER_SRTP_FEC, true);
    ASSERT_EQ((int32_t)WRTP_ERR_NOERR, rv);
    rv = m_sessionContext->GetOutboundConfig()->SetSrtpFecOrder(wrtp::ORDER_FEC_SRTP, false);
    ASSERT_EQ((int32_t)WRTP_ERR_NOERR, rv);
    wrtp::SRTPFecOrder getOrder = m_sessionContext->GetOutboundConfig()->GetSrtpFecOrder();
    EXPECT_EQ(wrtp::ORDER_SRTP_FEC, getOrder);
}

TEST_F(CRTPSessionContextTest, Test_Out_SetSrtpFecOrder_Srtp_Mari)
{
    int32_t rv = m_sessionContext->GetOutboundConfig()->SetSrtpFecOrder(wrtp::ORDER_SRTP_FEC, false);
    ASSERT_EQ((int32_t)WRTP_ERR_NOERR, rv);
    rv = m_sessionContext->GetOutboundConfig()->SetSrtpFecOrder(wrtp::ORDER_FEC_SRTP, true);
    ASSERT_EQ((int32_t)WRTP_ERR_NOERR, rv);
    wrtp::SRTPFecOrder getOrder = m_sessionContext->GetOutboundConfig()->GetSrtpFecOrder();
    EXPECT_EQ(wrtp::ORDER_FEC_SRTP, getOrder);
}


