#include "rtpsecinterface.h"
#include "srtp.h"
#include "crypto_types.h"
#include "secstrategywrapper.h"

using namespace wrtp;

class CSRTPWrapper//ciscosrtp wrapper
{
public:
    CSRTPWrapper(bool protector)
        : m_srtp(NULL)
        , m_protector(protector)
    {
        srtp_err_status_t err = srtp_create(&m_srtp, NULL);
        EXPECT_TRUE(srtp_err_status_ok == err);
    }

    ~CSRTPWrapper()
    {
        if (m_srtp) {
            srtp_err_status_t err = srtp_dealloc(m_srtp);
            m_srtp = NULL;

            EXPECT_TRUE(srtp_err_status_ok == err);
        }
    }

    void InitWithParam(CryptoSuiteType type, uint8_t key[], uint32_t keyLength, SecurityStreamType streamType, srtp_sec_serv_t sec_serv)
    {
        switch (type) {
            case CST_NULL_CIPHER_HMAC_SHA1_80: {
                InitWithNULLPolicy(key, keyLength, streamType, sec_serv);
            }
            break;
            case CST_AES_CM_128_HMAC_SHA1_80: {
                InitWithStandardPolicy(key, keyLength, streamType, sec_serv);
            }
            break;
            default:
                printf("InitWithParam:unkown CryptoSuiteType\n");
        }

    }

private:
    void InitWithNULLPolicy(uint8_t key[], uint32_t keyLength, SecurityStreamType streamType, srtp_sec_serv_t sec_serv)
    {
        srtp_policy_t srtpPolicy = {};
        srtpPolicy.ssrc.type    = m_protector ? ssrc_any_outbound : ssrc_any_inbound;
        srtpPolicy.ssrc.value   = 0;
        //srtpPolicy.ekt          = NULL;
        srtpPolicy.window_size  = 128;
        srtpPolicy.allow_repeat_tx = 1;

        srtpPolicy.key          = key;
        srtpPolicy.next         = NULL;

        srtp_crypto_policy_t &rtp = srtpPolicy.rtp;
        rtp.cipher_type     = SRTP_NULL_CIPHER;
        rtp.cipher_key_len  = 30;
        rtp.auth_type       = SRTP_HMAC_SHA1;
        rtp.auth_key_len    = 20;
        rtp.auth_tag_len    = 10;
        rtp.sec_serv        = sec_serv;

        srtp_crypto_policy_t &rtcp = srtpPolicy.rtcp;
        rtcp = rtp;

        srtp_err_status_t err = srtp_add_stream(m_srtp, &srtpPolicy);
        ASSERT_TRUE(srtp_err_status_ok == err);
    }

    void InitWithStandardPolicy(uint8_t key[], uint32_t keyLength, SecurityStreamType streamType, srtp_sec_serv_t sec_serv)
    {
        srtp_policy_t srtpPolicy = {};
        srtpPolicy.ssrc.type    = m_protector ? ssrc_any_outbound : ssrc_any_inbound;
        srtpPolicy.ssrc.value   = 0;
        //srtpPolicy.ekt     = NULL;
        srtpPolicy.window_size  = 128;
        srtpPolicy.allow_repeat_tx = 1;

        srtpPolicy.key          = key;
        srtpPolicy.next         = NULL;

        srtp_crypto_policy_t &rtp = srtpPolicy.rtp;
        rtp.cipher_type     = SRTP_AES_128_ICM;
        rtp.cipher_key_len  = 30;
        rtp.auth_type       = SRTP_HMAC_SHA1;
        rtp.auth_key_len    = 20;
        rtp.auth_tag_len    = 10;
        rtp.sec_serv        = sec_serv;

        srtp_crypto_policy_t &rtcp = srtpPolicy.rtcp;
        rtcp = rtp;

        srtp_err_status_t err = srtp_add_stream(m_srtp, &srtpPolicy);
        ASSERT_TRUE(srtp_err_status_ok == err);
    }


public:
    int32_t ProtectRTP(uint8_t *rtpHeader, uint32_t &packetLength)
    {
        EXPECT_TRUE(m_protector);
        EXPECT_TRUE(NULL != m_srtp);
        srtp_err_status_t err = srtp_protect(m_srtp, rtpHeader, (int *)&packetLength);

        return (srtp_err_status_ok == err) ? WRTP_ERR_NOERR : WRTP_ERR_SRTP_PROTECT_FAILED;
    }

    int32_t UnprotectRTP(uint8_t *rtpHeader, uint32_t &packetLength)
    {
        EXPECT_TRUE(!m_protector);
        EXPECT_TRUE(NULL != m_srtp);
        srtp_err_status_t err = srtp_unprotect(m_srtp, rtpHeader, (int *)&packetLength);

        return (srtp_err_status_ok == err) ? WRTP_ERR_NOERR : WRTP_ERR_SRTP_UNPROTECT_FAILED;
    }

    int32_t ProtectRTCP(uint8_t *rtcpHeader, uint32_t &packetLength)
    {
        EXPECT_TRUE(m_protector);
        EXPECT_TRUE(NULL != m_srtp);
        srtp_err_status_t err = srtp_protect_rtcp(m_srtp, rtcpHeader, (int *)&packetLength);

        return (srtp_err_status_ok == err) ? WRTP_ERR_NOERR : WRTP_ERR_SRTP_PROTECT_FAILED;
    }

    int32_t UnprotectRTCP(uint8_t *rtcpHeader, uint32_t &packetLength)
    {
        EXPECT_TRUE(!m_protector);
        EXPECT_TRUE(NULL != m_srtp);
        srtp_err_status_t err = srtp_unprotect_rtcp(m_srtp, rtcpHeader, (int *)&packetLength);

        return (srtp_err_status_ok == err) ? WRTP_ERR_NOERR : WRTP_ERR_SRTP_UNPROTECT_FAILED;
    }

private:
    srtp_t m_srtp;
    bool m_protector;
};