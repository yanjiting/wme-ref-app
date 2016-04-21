#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "rtputils.h"
#include "secstrategyclientwrapper.h"
#include "rtcppacket.h"
#include "testutil.h"

#include <memory>

using namespace wrtp;

class CSecurityStrategyClientWrapperTEST: public::testing::TestWithParam<CryptoSuiteType>
{
public:
    CSecurityStrategyClientWrapperTEST()
        : m_strategySendObj(nullptr)
        , m_strategyRecvObj(nullptr)
        , m_statistics(new CSRTPStatistics())
    {
    };
    ~CSecurityStrategyClientWrapperTEST() {};

    CRRPacket *GetBasicRTCPPacket()   //
    {
        CRRPacket *srPacket = new CRRPacket();
        srPacket->ssrc = 0x12345678;

        RRBlock rrBlock;
        rrBlock.ssrc = 0x3579;
        rrBlock.flost = 0x21;
        rrBlock.plost = 0x4567;
        rrBlock.xseq = 0x23456;
        rrBlock.jitter = 0x17;
        rrBlock.lsr = 0x9876;
        rrBlock.dlsr = 0x76543;
        srPacket->rrBlocks.push_back(rrBlock);

        return srPacket;
    }

    static SecurityConfiguration InitCryptoConfiguration(CryptoSuiteType type, uint8_t *masterKeySalt, uint32_t masterKeySaltLength)
    {
        SecurityConfiguration configuration   = {};
        configuration.cryptoSuiteType       = type;
        configuration.masterKeySalt         = masterKeySalt;
        configuration.masterKeySaltLength   = masterKeySaltLength;
        configuration.rtpSecurityService    = SEC_SERVICE_CONF_AUTH;
        configuration.rtcpSecurityService   = SEC_SERVICE_CONF_AUTH;
        configuration.fecOrder              = ORDER_SRTP_FEC;
        configuration.ekt                   = nullptr;

        return configuration;
    }

protected:
    virtual void SetUp()
    {
        SetUpSendSecStrategy();
        SetUpRecvSecStrategy();
    }
    virtual void TearDown()
    {
        if (nullptr != m_strategySendObj) {
            delete m_strategySendObj;
            m_strategySendObj = nullptr;
        }

        if (nullptr != m_strategyRecvObj) {
            delete m_strategyRecvObj;
            m_strategyRecvObj = nullptr;
        }
    }

    void SetUpSendSecStrategy()
    {
        if (nullptr != m_strategySendObj) {
            delete m_strategySendObj;
            m_strategySendObj = nullptr;
        }
        m_strategySendObj = new CSecurityStrategyClientWrapper(m_statistics);
    }

    void SetUpRecvSecStrategy()
    {
        if (nullptr != m_strategyRecvObj) {
            delete m_strategyRecvObj;
            m_strategyRecvObj = nullptr;
        }
        m_strategyRecvObj = new CSecurityStrategyClientWrapper(m_statistics);
    }
protected:
    ISecurityStrategy *m_strategySendObj;
    ISecurityStrategy *m_strategyRecvObj;
    CSRTPStatisticsSharedPtr m_statistics;
};

static int GetKeySaltLength(CryptoSuiteType type)
{
    if (type == CST_AES_CM_128_HMAC_SHA1_80) {
        return 30;
    } else if (type == CST_AES_CM_256_HMAC_SHA1_80) {
        return 46;
    } else {
        return 30;
    }

}

//use param test:
//0 means CST_NULL_CIPHER_HMAC_SHA1_80,
//1 means CST_AES_CM_128_HMAC_SHA1_80,
//2 means CST_AES_CM_256_HMAC_SHA1_80,
//3 means CST_AES_CM_128_HMAC_SHA1_32

INSTANTIATE_TEST_CASE_P(TestWithCryptoSuiteType, CSecurityStrategyClientWrapperTEST, testing::Values(CST_NULL_CIPHER_HMAC_SHA1_80, CST_AES_CM_128_HMAC_SHA1_80, CST_AES_CM_256_HMAC_SHA1_80, CST_AES_CM_128_HMAC_SHA1_32));

static uint32_t GetTagLen(CryptoSuiteType type)
{
    return (type == CST_AES_CM_128_HMAC_SHA1_32 ? 4 : 10);
}

#define TAG_LEN() GetTagLen(GetParam())

//Init
TEST_P(CSecurityStrategyClientWrapperTEST, Init_when_masterKeySalt_is_NULL_direction_is_sending_then_return_WRTP_ERR_INVALID_PARAM)
{
    CScopedTracer test_info;
    SetUpRecvSecStrategy();

    SecurityConfiguration configuration = InitCryptoConfiguration(CST_NULL_CIPHER_HMAC_SHA1_80, nullptr, 0);
    int32_t ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_INVALID_PARAM, ret);
}
TEST_P(CSecurityStrategyClientWrapperTEST, Init_when_masterKeySaltLength_less_than_30_direction_is_sending_then_return_WRTP_ERR_INVALID_PARAM)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    uint8_t keySalt[30] = {};
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, 29);
    int32_t ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_INVALID_PARAM, ret);
}

TEST_P(CSecurityStrategyClientWrapperTEST, Init_when_config_is_right_direction_is_sending_then_return_WRTP_ERR_NOERR)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, Init_when_masterKeySalt_is_NULL_direction_is_recving_then_return_WRTP_ERR_INVALID_PARAM)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    SecurityConfiguration configuration = InitCryptoConfiguration(type, nullptr, 0);
    int32_t ret = m_strategyRecvObj->Init(configuration, STREAM_IN);
    EXPECT_EQ(WRTP_ERR_INVALID_PARAM, ret);
}

TEST_P(CSecurityStrategyClientWrapperTEST, Init_when_masterKeySaltLength_less_than_30_direction_is_recving_then_return_WRTP_ERR_INVALID_PARAM)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    uint8_t keySalt[30] = {};
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, 29);
    int32_t ret = m_strategyRecvObj->Init(configuration, STREAM_IN);
    EXPECT_EQ(WRTP_ERR_INVALID_PARAM, ret);
}
TEST_P(CSecurityStrategyClientWrapperTEST, Init_when_config_is_right_direction_is_recving_then_return_WRTP_ERR_NOERR)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    ret = m_strategyRecvObj->Init(configuration, STREAM_IN);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, Init_when_masterKeySalt_is_NULL_direction_is_unknown_then_return_WRTP_ERR_INVALID_PARAM)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    SecurityConfiguration configuration = InitCryptoConfiguration(type, nullptr, 0);
    int32_t ret = m_strategySendObj->Init(configuration, STREAM_NONE);
    EXPECT_EQ(WRTP_ERR_INVALID_PARAM, ret);
}

TEST_P(CSecurityStrategyClientWrapperTEST, Init_when_masterKeySaltLength_less_than_30_direction_is_unknown_then_return_WRTP_ERR_INVALID_PARAM)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    uint8_t keySalt[30] = {};
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, 29);
    int32_t ret = m_strategySendObj->Init(configuration, STREAM_NONE);
    EXPECT_EQ(WRTP_ERR_INVALID_PARAM, ret);
}

TEST_P(CSecurityStrategyClientWrapperTEST, Init_when_config_is_right_direction_is_unknown_then_return_WRTP_ERR_SRTP_STREAM_ADD_ERROR)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    ret = m_strategySendObj->Init(configuration, STREAM_NONE);
    EXPECT_EQ(WRTP_ERR_SRTP_STREAM_ADD_ERROR, ret);

    delete keySalt;
}

//protect/unprotect
TEST_P(CSecurityStrategyClientWrapperTEST, ProtectRTP_UnProtectRTP_when_config_is_right_then_return_correct_rtppacket)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    //procedure  rtp packet
    CCmMessageBlock mb;
    const uint8_t dataLen = 100;
    uint8_t dataBuffer[dataLen] = {0};
    CCmMessageBlock payloadMb(dataLen);
    for (int i = 0; i < dataLen; i++) {
        dataBuffer[i] = i;
    }

    memcpy((void *)payloadMb.GetTopLevelReadPtr(), dataBuffer, dataLen);
    payloadMb.AdvanceTopLevelWritePtr(dataLen);
    CRTPPacket *rtpPacketPtr = GetBasicRTPPacket(77, 88, 99, 55, payloadMb);

    CmResult cmRet = rtpPacketPtr->Pack(mb);
    EXPECT_TRUE(CM_SUCCEEDED(cmRet));

    //protect rtp packet
    uint32_t oldPacketLength1 = mb.GetTopLevelLength();

    CCmMessageBlock mbSrc(oldPacketLength1);
    memcpy((void *)mbSrc.GetTopLevelReadPtr(), (void *)mb.GetTopLevelReadPtr(), oldPacketLength1);
    mbSrc.AdvanceTopLevelWritePtr(oldPacketLength1);

    EXPECT_EQ(rtpPacketPtr->CalcPacketLength(), oldPacketLength1);
    uint32_t newPacketLength1 = oldPacketLength1;
    ret = m_strategySendObj->ProtectRTP((uint8_t *)mb.GetTopLevelReadPtr(), newPacketLength1);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);

    cmRet = mb.AdvanceTopLevelWritePtr(newPacketLength1 - oldPacketLength1);
    ASSERT_TRUE(CM_SUCCEEDED(cmRet));

    ASSERT_EQ(oldPacketLength1 + TAG_LEN(), newPacketLength1);

    //Init receiving side
    ret = m_strategyRecvObj->Init(configuration, STREAM_IN);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    //unprotect rtp packet
    uint32_t oldPacketLength2 = mb.GetTopLevelLength();
    uint32_t newPacketLength2 = oldPacketLength2;
    ret = m_strategyRecvObj->UnprotectRTP((uint8_t *)mb.GetTopLevelReadPtr(), newPacketLength2);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);

    ASSERT_EQ(oldPacketLength2 - TAG_LEN(), newPacketLength2);
    EXPECT_EQ(0, memcmp((uint8_t *)mbSrc.GetTopLevelReadPtr(), (uint8_t *)mb.GetTopLevelReadPtr(), newPacketLength2));
    cmRet = mb.RetreatTopLevelWritePtr(oldPacketLength2 - newPacketLength2);
    ASSERT_TRUE(CM_SUCCEEDED(cmRet));

    delete rtpPacketPtr;
    delete keySalt;
}

//ProtectRTCP/UnprotectRTCP
TEST_P(CSecurityStrategyClientWrapperTEST, ProtectRTCP_UnProtectRTCP_when_config_is_right_then_return_correct_rtcppacket)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    //procedure  rtcp packet
    CRRPacket *rrPacket = GetBasicRTCPPacket();
    uint32_t encodeSize = rrPacket->CalcEncodeSize();
    CCmMessageBlock mb(encodeSize+MAX_RTCP_PACKET_RESERVED_LEN);
    CCmByteStreamNetwork os(mb);
    ASSERT_EQ(TRUE,rrPacket->Encode(os, mb));

    //protect rtp packet
    uint32_t oldPacketLength1 = mb.GetTopLevelLength();

    CCmMessageBlock mbSrc(oldPacketLength1);
    memcpy((void *)mbSrc.GetTopLevelReadPtr(), (void *)mb.GetTopLevelReadPtr(), oldPacketLength1);
    mbSrc.AdvanceTopLevelWritePtr(oldPacketLength1);

    uint32_t newPacketLength1 = oldPacketLength1;
    ret = m_strategySendObj->ProtectRTCP((uint8_t *)mb.GetTopLevelReadPtr(), newPacketLength1);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);
    ASSERT_EQ(oldPacketLength1 + 4 + TAG_LEN(), newPacketLength1);
    CmResult cmRet = mb.AdvanceTopLevelWritePtr(newPacketLength1 - oldPacketLength1);
    ASSERT_TRUE(CM_SUCCEEDED(cmRet));

    //Init receiving side
    ret = m_strategyRecvObj->Init(configuration, STREAM_IN);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    //unprotect rtp packet
    uint32_t oldPacketLength2 = mb.GetTopLevelLength();
    uint32_t newPacketLength2 = oldPacketLength2;
    ret = m_strategyRecvObj->UnprotectRTCP((uint8_t *)mb.GetTopLevelReadPtr(), newPacketLength2);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);
    ASSERT_EQ(oldPacketLength2 - 4 - TAG_LEN(), newPacketLength2);
    cmRet = mb.RetreatTopLevelWritePtr(oldPacketLength2 - newPacketLength2);
    ASSERT_TRUE(CM_SUCCEEDED(cmRet));

    EXPECT_EQ(0, memcmp((uint8_t *)mbSrc.GetTopLevelReadPtr(), (uint8_t *)mb.GetTopLevelReadPtr(), newPacketLength2));

    delete rrPacket;
    rrPacket = nullptr;

    delete keySalt;
}

//GetMaxTagLength
TEST_F(CSecurityStrategyClientWrapperTEST, GetMaxTagLength_CST_NULL_CIPHER_HMAC_SHA1_80_rtp_then_return_correct_value)
{
    CScopedTracer test_info;
    uint8_t keySalt[30] = {};
    int32_t ret = WRTPGetCryptoRandom(keySalt, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(CST_NULL_CIPHER_HMAC_SHA1_80, keySalt, 30);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    EXPECT_EQ(10, m_strategySendObj->GetMaxTagLength(SEC_STREAM_RTP));
    EXPECT_EQ(14, m_strategySendObj->GetMaxTagLength(SEC_STREAM_RTCP));
    EXPECT_EQ(14, m_strategySendObj->GetMaxTagLength(SEC_STREAM_RTP_RTCP));
}

//AddStream
TEST_F(CSecurityStrategyClientWrapperTEST, AddStream_not_Init_then_return_error)
{
    CScopedTracer test_info;
    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONF_AUTH;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_SRTP_INVALID_HANDLER, m_strategySendObj->AddStream(ssrc, service, streamType));
}

TEST_P(CSecurityStrategyClientWrapperTEST, AddStream_SEC_SERVICE_CONF_AUTH_Init_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONF_AUTH;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, AddStream_SEC_SERVICE_CONFIDENTIALITY_Init_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONFIDENTIALITY;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, AddStream_SEC_SERVICE_AUTHENTICATION_Init_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_AUTHENTICATION;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    delete keySalt;
}

//RemoveStream
TEST_F(CSecurityStrategyClientWrapperTEST, RemoveStream_not_Init_then_return_error)
{
    CScopedTracer test_info;
    uint32_t ssrc = 123;
    EXPECT_EQ(WRTP_ERR_SRTP_INVALID_HANDLER, m_strategySendObj->RemoveStream(ssrc));
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_CONF_AUTH_STREAM_RTP_and_remove_wrong_stream_then_return_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONF_AUTH;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_SRTP_STREAM_REMOVE_ERROR, m_strategySendObj->RemoveStream(100));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_CONFIDENTIALITY_STREAM_RTP_and_remove_wrong_stream_then_return_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONFIDENTIALITY;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_SRTP_STREAM_REMOVE_ERROR, m_strategySendObj->RemoveStream(100));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_AUTHENTICATION_STREAM_RTP_and_remove_wrong_stream_then_return_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_AUTHENTICATION;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_SRTP_STREAM_REMOVE_ERROR, m_strategySendObj->RemoveStream(100));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_CONF_AUTH_STREAM_RTP_and_remove_first_stream_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONF_AUTH;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->RemoveStream(ssrc));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_CONFIDENTIALITY_STREAM_RTP_and_remove_first_stream_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONFIDENTIALITY;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->RemoveStream(ssrc));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_AUTHENTICATION_STREAM_RTP_and_remove_first_stream_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_AUTHENTICATION;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->RemoveStream(ssrc));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_CONF_AUTH_STREAM_RTP_and_remove_middle_stream_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONF_AUTH;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(220, service, streamType));
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->RemoveStream(ssrc));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_CONFIDENTIALITY_STREAM_RTP_and_remove_middle_stream_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONFIDENTIALITY;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(220, service, streamType));
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->RemoveStream(ssrc));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_AUTHENTICATION_STREAM_RTP_and_remove_middle_stream_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_AUTHENTICATION;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(220, service, streamType));
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->RemoveStream(ssrc));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_CONF_AUTH_STREAM_RTCP_and_remove_wrong_stream_then_return_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONF_AUTH;
    SecurityStreamType streamType = SEC_STREAM_RTCP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_SRTP_STREAM_REMOVE_ERROR, m_strategySendObj->RemoveStream(100));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_CONFIDENTIALITY_STREAM_RTCP_and_remove_wrong_stream_then_return_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONFIDENTIALITY;
    SecurityStreamType streamType = SEC_STREAM_RTCP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_SRTP_STREAM_REMOVE_ERROR, m_strategySendObj->RemoveStream(100));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_AUTHENTICATION_STREAM_RTCP_and_remove_wrong_stream_then_return_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_AUTHENTICATION;
    SecurityStreamType streamType = SEC_STREAM_RTCP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_SRTP_STREAM_REMOVE_ERROR, m_strategySendObj->RemoveStream(100));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_CONF_AUTH_STREAM_RTCP_and_remove_first_stream_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONF_AUTH;
    SecurityStreamType streamType = SEC_STREAM_RTCP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->RemoveStream(ssrc));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_CONFIDENTIALITY_STREAM_RTCP_and_remove_first_stream_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONFIDENTIALITY;
    SecurityStreamType streamType = SEC_STREAM_RTCP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->RemoveStream(ssrc));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_AUTHENTICATION_STREAM_RTCP_and_remove_first_stream_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_AUTHENTICATION;
    SecurityStreamType streamType = SEC_STREAM_RTCP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->RemoveStream(ssrc));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_CONF_AUTH_STREAM_RTCP_and_remove_middle_stream_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONF_AUTH;
    SecurityStreamType streamType = SEC_STREAM_RTCP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(220, service, streamType));
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->RemoveStream(ssrc));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_CONFIDENTIALITY_STREAM_RTCP_and_remove_middle_stream_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_CONFIDENTIALITY;
    SecurityStreamType streamType = SEC_STREAM_RTCP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(220, service, streamType));
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->RemoveStream(ssrc));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, RemoveStream_Init_SEC_SERVICE_AUTHENTICATION_STREAM_RTCP_and_remove_middle_stream_then_return_no_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    uint32_t ssrc = 123;
    SecurityService service = SEC_SERVICE_AUTHENTICATION;
    SecurityStreamType streamType = SEC_STREAM_RTCP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(220, service, streamType));
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->RemoveStream(ssrc));

    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, Init_with_auth_conf_AddStream_with_auth_then_return_correct_rtppacket)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    //procedure  rtp packet
    CCmMessageBlock mb;
    const uint8_t dataLen = 100;
    uint8_t dataBuffer[dataLen] = {0};
    CCmMessageBlock payloadMb(dataLen);
    for (int i = 0; i < dataLen; i++) {
        dataBuffer[i] = i;
    }

    memcpy((void *)payloadMb.GetTopLevelReadPtr(), dataBuffer, dataLen);
    payloadMb.AdvanceTopLevelWritePtr(dataLen);
    CRTPPacket *rtpPacketPtr = GetBasicRTPPacket(77, 88, 99, 55, payloadMb);
    CmResult cmRet = rtpPacketPtr->Pack(mb);
    EXPECT_TRUE(CM_SUCCEEDED(cmRet));
    uint32_t ssrc = rtpPacketPtr->GetSSRC();

    //add a auth only stream
    SecurityService service = SEC_SERVICE_AUTHENTICATION;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    //protect rtp packet
    uint32_t oldPacketLength1 = mb.GetTopLevelLength();

    EXPECT_EQ(rtpPacketPtr->CalcPacketLength(), oldPacketLength1);
    uint32_t newPacketLength1 = oldPacketLength1;
    ret = m_strategySendObj->ProtectRTP((uint8_t *)mb.GetTopLevelReadPtr(), newPacketLength1);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);

    ASSERT_EQ(oldPacketLength1 + TAG_LEN(), newPacketLength1);

    cmRet = mb.AdvanceTopLevelWritePtr(newPacketLength1 - oldPacketLength1);
    ASSERT_TRUE(CM_SUCCEEDED(cmRet));

    CCmMessageBlock mbProtected(newPacketLength1);
    memcpy((void *)mbProtected.GetTopLevelReadPtr(), (void *)mb.GetTopLevelReadPtr(), newPacketLength1);
    mbProtected.AdvanceTopLevelWritePtr(newPacketLength1);

    //Init receiving side
    ret = m_strategyRecvObj->Init(configuration, STREAM_IN);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    //add a auth only stream
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategyRecvObj->AddStream(ssrc, service, streamType));

    //unprotect rtp packet
    uint32_t oldPacketLength2 = mb.GetTopLevelLength();
    uint32_t newPacketLength2 = oldPacketLength2;
    ret = m_strategyRecvObj->UnprotectRTP((uint8_t *)mb.GetTopLevelReadPtr(), newPacketLength2);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);

    ASSERT_EQ(oldPacketLength2 - TAG_LEN(), newPacketLength2);
    EXPECT_TRUE(0 == memcmp((uint8_t *)mbProtected.GetTopLevelReadPtr(), (uint8_t *)mb.GetTopLevelReadPtr(), newPacketLength2));
    cmRet = mb.RetreatTopLevelWritePtr(oldPacketLength2 - newPacketLength2);
    ASSERT_TRUE(CM_SUCCEEDED(cmRet));

    delete rtpPacketPtr;
    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, Init_with_auth_conf_AddStream_with_conf_then_return_correct_rtppacket)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    uint8_t *keySalt = new uint8_t [keySaltLen];
    int32_t ret = WRTPGetCryptoRandom(keySalt, keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt, keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    //procedure  rtp packet
    CCmMessageBlock mb;
    const uint8_t dataLen = 100;
    uint8_t dataBuffer[dataLen] = {0};
    CCmMessageBlock payloadMb(dataLen);
    for (int i = 0; i < dataLen; i++) {
        dataBuffer[i] = i;
    }

    memcpy((void *)payloadMb.GetTopLevelReadPtr(), dataBuffer, dataLen);
    payloadMb.AdvanceTopLevelWritePtr(dataLen);
    CRTPPacket *rtpPacketPtr = GetBasicRTPPacket(77, 88, 99, 55, payloadMb);

    CmResult cmRet = rtpPacketPtr->Pack(mb);
    EXPECT_TRUE(CM_SUCCEEDED(cmRet));
    uint32_t ssrc = rtpPacketPtr->GetSSRC();


    //add a auth only stream
    SecurityService service = SEC_SERVICE_AUTHENTICATION;
    SecurityStreamType streamType = SEC_STREAM_RTP;
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategySendObj->AddStream(ssrc, service, streamType));

    //protect rtp packet
    uint32_t oldPacketLength1 = mb.GetTopLevelLength();

    CCmMessageBlock mbSrc(oldPacketLength1);
    memcpy((void *)mbSrc.GetTopLevelReadPtr(), (void *)mb.GetTopLevelReadPtr(), oldPacketLength1);
    mbSrc.AdvanceTopLevelWritePtr(oldPacketLength1);

    EXPECT_EQ(rtpPacketPtr->CalcPacketLength(), oldPacketLength1);
    uint32_t newPacketLength1 = oldPacketLength1;
    ret = m_strategySendObj->ProtectRTP((uint8_t *)mb.GetTopLevelReadPtr(), newPacketLength1);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);
    cmRet = mb.AdvanceTopLevelWritePtr(newPacketLength1 - oldPacketLength1);
    ASSERT_TRUE(CM_SUCCEEDED(cmRet));

    ASSERT_EQ(oldPacketLength1 + TAG_LEN(), newPacketLength1);

    //Init receiving side
    ret = m_strategyRecvObj->Init(configuration, STREAM_IN);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    //add a auth only stream
    EXPECT_EQ(WRTP_ERR_NOERR, m_strategyRecvObj->AddStream(ssrc, service, streamType));

    //unprotect rtp packet
    uint32_t oldPacketLength2 = mb.GetTopLevelLength();
    uint32_t newPacketLength2 = oldPacketLength2;
    ret = m_strategyRecvObj->UnprotectRTP((uint8_t *)mb.GetTopLevelReadPtr(), newPacketLength2);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);

    ASSERT_EQ(oldPacketLength2 - TAG_LEN(), newPacketLength2);
    EXPECT_TRUE(0 == memcmp((uint8_t *)mbSrc.GetTopLevelReadPtr(), (uint8_t *)mb.GetTopLevelReadPtr(), newPacketLength2));
    cmRet = mb.RetreatTopLevelWritePtr(oldPacketLength2 - newPacketLength2);
    ASSERT_TRUE(CM_SUCCEEDED(cmRet));


    delete rtpPacketPtr;
    delete keySalt;
}

TEST_P(CSecurityStrategyClientWrapperTEST, Init_with_none_sevice_return_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();

    int keySaltLen = GetKeySaltLength(type);
    std::unique_ptr<uint8_t[]> keySalt(new uint8_t[keySaltLen]);
    int32_t ret = WRTPGetCryptoRandom(keySalt.get(), keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    SecurityConfiguration configuration = {};
    configuration.cryptoSuiteType       = type;
    configuration.masterKeySalt         = keySalt.get();
    configuration.masterKeySaltLength   = keySaltLen;
    configuration.rtpSecurityService    = SEC_SERVICE_CONF_AUTH;
    configuration.rtcpSecurityService   = SEC_SERVICE_NONE;
    configuration.fecOrder              = ORDER_SRTP_FEC;
    configuration.ekt                   = nullptr;

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_NE(WRTP_ERR_NOERR, ret);

    configuration.rtpSecurityService    = SEC_SERVICE_NONE;
    configuration.rtcpSecurityService   = SEC_SERVICE_CONF_AUTH;
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_NE(WRTP_ERR_NOERR, ret);
}

TEST_P(CSecurityStrategyClientWrapperTEST, Init_with_auth_conf_AddStream_none_service_return_error)
{
    CScopedTracer test_info;
    CryptoSuiteType type = GetParam();
    SetUpRecvSecStrategy();

    int keySaltLen = GetKeySaltLength(type);
    std::unique_ptr<uint8_t[]> keySalt(new uint8_t[keySaltLen]);
    int32_t ret = WRTPGetCryptoRandom(keySalt.get(), keySaltLen);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);
    SecurityConfiguration configuration = InitCryptoConfiguration(type, keySalt.get(), keySaltLen);

    //Init sending side
    ret = m_strategySendObj->Init(configuration, STREAM_OUT);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);

    //add a none service stream
    EXPECT_NE(WRTP_ERR_NOERR, m_strategySendObj->AddStream(100, SEC_SERVICE_NONE, SEC_STREAM_RTP));
    EXPECT_NE(WRTP_ERR_NOERR, m_strategySendObj->AddStream(101, SEC_SERVICE_NONE, SEC_STREAM_RTCP));
}