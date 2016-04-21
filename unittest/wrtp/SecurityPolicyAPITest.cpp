#define videoDataLen 10
#define audioDataLen 10
#define MAX_PACKET_SIZE 1200

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "wrtpwmeapi.h"
#include "CmThreadManager.h"
#include "wrtpapi.h"
#include "rtputils.h"
#include "srtpwrapper.h"
#include "WMEInterfaceMock.h"
#include "testutil.h"

using namespace wrtp;
using namespace wme;

struct Extension {
    uint8_t key;
};

uint8_t videoSendIndex = 0;
uint8_t videoRecvIndex = 0;
uint8_t audioSendIndex = 0;
uint8_t audioRecvIndex = 0;

CryptoSuiteType senderCryptoSuite = CST_NULL_CIPHER_HMAC_SHA1_80;
CryptoSuiteType recverCryptoSuite = CST_NULL_CIPHER_HMAC_SHA1_80;

class CRTPSendingSinkMock:public IRTPMediaTransport
{
public:
    CRTPSendingSinkMock(IRTPSessionClient *recvSession, IRTPChannel *recvChannel)
        : m_recvSession(recvSession)
        , m_recvChannel(recvChannel)
    {

    }

    ~CRTPSendingSinkMock()
    {
    }

    void Reset()
    {
        m_recvSession = nullptr;
        m_recvChannel = nullptr;
    }

    virtual WMERESULT GetTransportType(WmeTransportType &eTransportType)
    {
        eTransportType = WmeTransportType_UDP;
        return WME_S_OK;
    }

    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage)
    {
        CCmMessageBlock *mb = nullptr;
        ConvertMediaPackageToMessageBlock(pRTPPackage, mb);
        EXPECT_TRUE(mb != nullptr);
        CCmMessageBlock &rtpPacket = *mb;

        if (!m_recvChannel) {
            return WRTP_ERR_NOERR;
        }

        // forward the RTP packet to recving channel
        IWmeMediaPackage *rtpPackage = nullptr;
        ConvertMessageBlockToMediaPackage(rtpPacket, &rtpPackage, true);
        int32_t ret = m_recvChannel->ReceiveRTPPacket(rtpPackage);
        EXPECT_EQ(0, ret);

        mb->DestroyChained();
        mb = nullptr;
        WME_SAFE_RELEASE(rtpPackage);

        return ret;
    }

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        if (!m_recvSession) {
            return WRTP_ERR_NOERR;
        }

        // forward the RTCP packet to recving session
        int32_t ret = m_recvSession->ReceiveRTCPPacket(pRTCPPackage);
        // if(senderCryptoSuite == recverCryptoSuite) {
        //    EXPECT_EQ(WRTP_ERR_NOERR, ret);
        //} else {
        //     EXPECT_EQ(-1, ret);
        // }
        return ret;
    }

protected:
private:
    IRTPSessionClient  *m_recvSession;
    IRTPChannel        *m_recvChannel;

};

class CMDataRecvSinkMock:public IMediaDataRecvSink
{
public:
    CMDataRecvSinkMock(IRTPSessionClient *recvSession, IRTPChannel *recvChannel)
        : m_recvSession(recvSession)
        , m_recvChannel(recvChannel)
    {
    }

    virtual int32_t OnRecvMediaData(WRTPMediaData *pData, WRTPMediaDataIntegrityInfo *pIntegrityInfo)
    {

        if (pData->type == MEDIA_DATA_VOIP) {
            if (senderCryptoSuite == recverCryptoSuite) {
                EXPECT_EQ(audioDataLen, pData->size);
                Extension *ext = (Extension *)(pData->data);
                EXPECT_EQ(audioRecvIndex++, ext->key);
                char ch = 'a';
                for (int i = 1; i < audioDataLen; ++i) {
                    EXPECT_EQ(ch, pData->data[i]);
                }
                //audioRecvIndex++;
            } else {
                bool isSame = true;
                char ch = 'a';
                for (int i = 1; i < audioDataLen; ++i) {
                    if (pData->data[i] != ch) {
                        isSame = false;
                        break;
                    }
                }
                EXPECT_FALSE(isSame);
            }
        } else if (pData->type == MEDIA_DATA_VIDEO) {
            if (senderCryptoSuite == recverCryptoSuite) {
                EXPECT_EQ(videoDataLen, pData->size);
                Extension *ext = (Extension *)(pData->data);
                EXPECT_EQ(videoRecvIndex++, ext->key);
                char ch = 'v';
                for (int i = 1; i < videoDataLen; ++i) {
                    EXPECT_EQ(ch, pData->data[i]);
                }
            } else {
                bool isSame = true;
                char ch = 'v';
                for (int i = 1; i < videoDataLen; ++i) {
                    if (pData->data[i] != ch) {
                        isSame = false;
                        break;
                    }
                }
                EXPECT_FALSE(isSame);
            }
        }

        return 0;
    }

protected:

private:
    IRTPSessionClient  *m_recvSession;
    IRTPChannel        *m_recvChannel;
};

class CSecurityPolicyTest:public ::testing::Test
{
public:
    CSecurityPolicyTest():m_SendSession(nullptr)
        , m_SendChannel(nullptr)
        , m_RecvSession(nullptr)
        , m_RecvChannel(nullptr)
        , m_streamId(1)
        , m_mediaSourceId(32767)
        , m_framemarkingURI("http://protocols.cisco.com/framemarking")
        , m_framemarkingId(2)
    {

    }

protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
        m_SendSession->SetMediaTransport(nullptr);
        m_RecvSession->SetMediaTransport(nullptr);
        m_sendingSinkForSender->Reset();

        m_RecvChannel->Close();
        m_RecvChannel->DecreaseReference();
        m_RecvChannel = nullptr;

        m_RecvSession->DecreaseReference();
        m_RecvSession = nullptr;

        m_SendChannel->Close();
        m_SendChannel->DecreaseReference();
        m_SendChannel = nullptr;

        m_SendSession->DecreaseReference();
        m_SendSession = nullptr;

        m_sendingSinkForSender.reset();

        m_DataRecvSink.reset();
    }

    void InitSender(WRTPSessionType sessionType, const SecurityConfiguration &configuration)
    {
        // create video sending session
        WRTPSessionParams sessionParams;
        sessionParams.sessionType = sessionType;
        sessionParams.enableRTCP = true;

        m_SendSession = WRTPCreateRTPSessionClient(sessionParams);
        ASSERT_TRUE(nullptr != m_SendSession);
        ASSERT_EQ(WRTP_ERR_NOERR, m_SendSession->SetMaxPacketSize(MAX_PACKET_SIZE));
        m_SendSession->IncreaseReference();
        EXPECT_EQ(m_SendSession->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance()), 0);

        // set securityPolicy
        int32_t ret = m_SendSession->SetOutboundSecurityConfiguration(configuration);
        if (configuration.masterKeySalt == nullptr || configuration.masterKeySaltLength < 30) {
            ASSERT_EQ(WRTP_ERR_INVALID_PARAM, ret);
        } else {
            ASSERT_EQ(WRTP_ERR_NOERR, ret);
        }

        if (sessionType != RTP_SESSION_WEBEX_VOIP) {
            ret = m_SendSession->UpdateRTPExtension(m_framemarkingURI, m_framemarkingId, wrtp::STREAM_INOUT);
            EXPECT_TRUE(0 == ret);
        }

        ret = m_SendSession->RegisterPayloadType(60, 20, 90000);
        ASSERT_TRUE(0 == ret);
        ret = m_SendSession->RegisterPayloadType(50, 10, 48000);
        ASSERT_TRUE(0 == ret);

        // create video sending channel
        WRTPChannelParams channelParams;

        m_SendChannel = m_SendSession->CreateRTPChannel(m_mediaSourceId, channelParams);
        ASSERT_TRUE(nullptr != m_SendChannel);
        m_SendChannel->IncreaseReference();
    }

    void InitRecver(WRTPSessionType sessionType, const SecurityConfiguration &configuration)
    {
        // create video recving session
        WRTPSessionParams sessionParams;
        sessionParams.sessionType = sessionType;
        sessionParams.enableRTCP  = true;

        m_RecvSession = WRTPCreateRTPSessionClient(sessionParams);
        ASSERT_TRUE(nullptr != m_RecvSession);
        ASSERT_EQ(WRTP_ERR_NOERR, m_RecvSession->SetMaxPacketSize(MAX_PACKET_SIZE));
        m_RecvSession->IncreaseReference();
        EXPECT_EQ(m_RecvSession->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance()), 0);

        // set securityPolicy
        int32_t ret = m_RecvSession->SetInboundSecurityConfiguration(configuration);
        if (configuration.masterKeySalt == nullptr || configuration.masterKeySaltLength < 30) {
            ASSERT_EQ(WRTP_ERR_INVALID_PARAM, ret);
        } else {
            ASSERT_EQ(WRTP_ERR_NOERR, ret);
        }

        if (sessionType != RTP_SESSION_WEBEX_VOIP) {
            ret = m_RecvSession->UpdateRTPExtension(m_framemarkingURI, m_framemarkingId, wrtp::STREAM_INOUT);
            EXPECT_TRUE(0 == ret);
        }

        ret = m_RecvSession->RegisterPayloadType(60, 20, 90000);
        ASSERT_TRUE(0 == ret);
        ret = m_RecvSession->RegisterPayloadType(50, 10, 48000);
        ASSERT_TRUE(0 == ret);

        // create video recving channel
        WRTPChannelParams channelParams;

        m_RecvChannel = m_RecvSession->CreateRTPChannel(m_mediaSourceId, channelParams);
        ASSERT_TRUE(nullptr != m_RecvChannel);
        m_RecvChannel->IncreaseReference();

        m_DataRecvSink = std::unique_ptr<CMDataRecvSinkMock>(new CMDataRecvSinkMock(m_RecvSession, m_RecvChannel));
        m_RecvChannel->SetMediaDataRecvSink(m_DataRecvSink.get());
    }

    void BindSender()
    {
        m_sendingSinkForSender = std::unique_ptr<CRTPSendingSinkMock>(new CRTPSendingSinkMock(m_RecvSession, m_RecvChannel));
        ASSERT_TRUE(nullptr != m_sendingSinkForSender.get());
        WMERESULT ret = m_SendSession->SetMediaTransport(m_sendingSinkForSender.get());
        EXPECT_TRUE(WME_SUCCEEDED(ret));
    }

    void FillAndSendVoIPData(WRTPMediaDataVoIP *mediaDataAudio)
    {
        ASSERT_TRUE(nullptr != mediaDataAudio);
        // init audio data
        uint8_t *audioData = new uint8_t [audioDataLen];

        for (int ii = 0; ii < audioDataLen; ++ii) {
            audioData[ii] = 'a';
        }

        //audioSendIndex++;
        Extension *ext = (Extension *)audioData;
        ext->key = audioSendIndex++;

        mediaDataAudio->type = MEDIA_DATA_VOIP;
        mediaDataAudio->data = audioData;
        mediaDataAudio->size = audioDataLen;
        mediaDataAudio->marker = 0;
        mediaDataAudio->codecType = 60;

        // send audioData
        SendData(mediaDataAudio);

        // release memory
        delete [] audioData;
        audioData = nullptr;
    }

    void FillAndSendVideoData(WRTPMediaDataVideo *mediaDataVideo)
    {
        ASSERT_TRUE(nullptr != mediaDataVideo);
        // init video data
        uint8_t *videoData = new uint8_t [videoDataLen];

        for (int ii = 0; ii < videoDataLen; ++ii) {
            videoData[ii] = 'v';
        }

        Extension *ext = (Extension *)videoData;
        ext->key = videoSendIndex++;

        // init media data structure
        mediaDataVideo->type             = MEDIA_DATA_VIDEO;
        mediaDataVideo->marker           = 1;
        mediaDataVideo->size             = videoDataLen;
        mediaDataVideo->data             = videoData;
        //        mediaDataVideo->layerNumber      = 1;
        //        mediaDataVideo->layerIndex       = 0;
        mediaDataVideo->layerWidth       = 640;
        mediaDataVideo->layerHeight      = 360;
        mediaDataVideo->layerMapAll      = false;
        mediaDataVideo->frame_idc        = 0;
        mediaDataVideo->disposable       = false;
        mediaDataVideo->switchable       = true;
        mediaDataVideo->priority         = 1;
        mediaDataVideo->frameType        = 0;
        mediaDataVideo->DID              = m_streamId;
        mediaDataVideo->TID              = 0;
        mediaDataVideo->maxTID           = 1;
        mediaDataVideo->maxDID           = m_streamId;
        mediaDataVideo->codecType        = 50;

        // send videoData
        SendData(mediaDataVideo);

        // release memory
        delete [] videoData;
        videoData = nullptr;
    }

    void SendData(WRTPMediaData *mediaData)
    {
        ASSERT_TRUE(nullptr != m_SendChannel);
        ASSERT_TRUE(nullptr != mediaData);

        int32_t ret = m_SendChannel->SendMediaData(mediaData);
        ASSERT_EQ(0, ret);
    }

protected:
    IRTPSessionClient        *m_SendSession;
    IRTPChannel              *m_SendChannel;

    IRTPSessionClient        *m_RecvSession;
    IRTPChannel              *m_RecvChannel;

    uint32_t                    m_streamId;
    uint32_t                    m_mediaSourceId;
    const char               *m_framemarkingURI;
    uint8_t                     m_framemarkingId;

    std::unique_ptr<CRTPSendingSinkMock>      m_sendingSinkForSender;
    std::unique_ptr<CMDataRecvSinkMock>       m_DataRecvSink;
};

static SecurityConfiguration InitCryptoConfiguration(CryptoSuiteType type)
{
    SecurityConfiguration configuration   = {};
    configuration.cryptoSuiteType       = type;
    configuration.masterKeySalt         = nullptr;
    configuration.masterKeySaltLength   = 0;
    configuration.rtpSecurityService    = SEC_SERVICE_CONF_AUTH;
    configuration.rtcpSecurityService   = SEC_SERVICE_CONF_AUTH;
    configuration.fecOrder              = ORDER_SRTP_FEC;
    configuration.ekt                   = nullptr;

    return configuration;
}

#if 1
TEST_F(CSecurityPolicyTest, DefaultSecurityPolicy_video)
{
    CScopedTracer test_info;
    uint8_t keys[30] = {};
    int32_t ret = WRTPGetCryptoRandom(keys, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    SecurityConfiguration configuration = InitCryptoConfiguration(CST_NULL_CIPHER_HMAC_SHA1_80);
    configuration.masterKeySalt         = keys;
    configuration.masterKeySaltLength   = 30;
    InitSender(RTP_SESSION_WEBEX_VIDEO, configuration);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, configuration);
    BindSender();

    videoSendIndex = 0;
    videoRecvIndex = 0;
    WRTPMediaDataVideo videoData;
    for (int i = 0; i < 5; ++i) {
        FillAndSendVideoData(&videoData);
        //::SleepMs(300);
    }

}
#endif

TEST_F(CSecurityPolicyTest, DefaultSecurityPolicy_audio)
{
    CScopedTracer test_info;
    uint8_t keys[30] = {};
    int32_t ret = WRTPGetCryptoRandom(keys, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    SecurityConfiguration configuration = InitCryptoConfiguration(CST_NULL_CIPHER_HMAC_SHA1_80);
    configuration.masterKeySalt         = keys;
    configuration.masterKeySaltLength   = 30;
    InitSender(RTP_SESSION_WEBEX_VOIP, configuration);
    InitRecver(RTP_SESSION_WEBEX_VOIP, configuration);
    BindSender();

    audioSendIndex = 0;
    audioRecvIndex = 0;
    WRTPMediaDataVoIP audioData;
    for (int i = 0; i < 5; ++i) {
        FillAndSendVoIPData(&audioData);
        //::SleepMs(100);
    }

}

TEST_F(CSecurityPolicyTest, CST_AES_CM_128_HMAC_SHA1_80_SecurityPolicy_video)
{
    CScopedTracer test_info;
    uint8_t keys[30] = {};
    int32_t ret = WRTPGetCryptoRandom(keys, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    SecurityConfiguration configuration = InitCryptoConfiguration(CST_AES_CM_128_HMAC_SHA1_80);
    configuration.masterKeySalt       = keys;
    configuration.masterKeySaltLength = 30;
    InitSender(RTP_SESSION_WEBEX_VIDEO, configuration);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, configuration);
    BindSender();

    videoSendIndex = 0;
    videoRecvIndex = 0;
    WRTPMediaDataVideo videoData;
    for (int i = 0; i < 5; ++i) {
        FillAndSendVideoData(&videoData);
        //::SleepMs(300);
    }

}

TEST_F(CSecurityPolicyTest, CST_AES_CM_128_HMAC_SHA1_80_SecurityPolicy_audio)
{
    CScopedTracer test_info;
    uint8_t keys[30] = {};
    int32_t ret = WRTPGetCryptoRandom(keys, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    SecurityConfiguration configuration = InitCryptoConfiguration(CST_AES_CM_128_HMAC_SHA1_80);
    configuration.masterKeySalt       = keys;
    configuration.masterKeySaltLength = 30;
    InitSender(RTP_SESSION_WEBEX_VOIP, configuration);
    InitRecver(RTP_SESSION_WEBEX_VOIP, configuration);
    BindSender();

    audioSendIndex = 0;
    audioRecvIndex = 0;
    WRTPMediaDataVoIP audioData;
    for (int i = 0; i < 5; ++i) {
        FillAndSendVoIPData(&audioData);
        //::SleepMs(100);
    }

}

TEST_F(CSecurityPolicyTest, CST_AES_CM_256_HMAC_SHA1_80_SecurityPolicy_video)
{
    CScopedTracer test_info;
    uint8_t keys[46] = {};
    int32_t ret = WRTPGetCryptoRandom(keys, 46);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    SecurityConfiguration configuration = InitCryptoConfiguration(CST_AES_CM_256_HMAC_SHA1_80);
    configuration.masterKeySalt       = keys;
    configuration.masterKeySaltLength = 46;
    InitSender(RTP_SESSION_WEBEX_VIDEO, configuration);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, configuration);
    BindSender();

    videoSendIndex = 0;
    videoRecvIndex = 0;
    WRTPMediaDataVideo videoData;
    for (int i = 0; i < 5; ++i) {
        FillAndSendVideoData(&videoData);
        //::SleepMs(100);
    }

}

TEST_F(CSecurityPolicyTest, CST_AES_CM_256_HMAC_SHA1_80_SecurityPolicy_audio)
{
    CScopedTracer test_info;
    uint8_t keys[46] = {};
    int32_t ret = WRTPGetCryptoRandom(keys, 46);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    SecurityConfiguration configuration = InitCryptoConfiguration(CST_AES_CM_256_HMAC_SHA1_80);
    configuration.masterKeySalt       = keys;
    configuration.masterKeySaltLength = 46;
    InitSender(RTP_SESSION_WEBEX_VOIP, configuration);
    InitRecver(RTP_SESSION_WEBEX_VOIP, configuration);
    BindSender();

    videoSendIndex = 0;
    videoRecvIndex = 0;
    WRTPMediaDataVoIP audioData;
    for (int i = 0; i < 5; ++i) {
        FillAndSendVoIPData(&audioData);
        //::SleepMs(100);
    }

}

TEST_F(CSecurityPolicyTest, NULL_masterKeySalt_CST_NULL_CIPHER_HMAC_SHA1_80_SecurityPolicy_video)
{
    CScopedTracer test_info;
    SecurityConfiguration configuration = InitCryptoConfiguration(CST_NULL_CIPHER_HMAC_SHA1_80);
    configuration.masterKeySaltLength = 30;
    InitSender(RTP_SESSION_WEBEX_VIDEO, configuration);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, configuration);
    BindSender();
}

TEST_F(CSecurityPolicyTest, NULL_masterKeySalt_CST_NULL_CIPHER_HMAC_SHA1_80_SecurityPolicy_audio)
{
    CScopedTracer test_info;
    SecurityConfiguration configuration = InitCryptoConfiguration(CST_NULL_CIPHER_HMAC_SHA1_80);
    configuration.masterKeySaltLength = 30;
    InitSender(RTP_SESSION_WEBEX_VOIP, configuration);
    InitRecver(RTP_SESSION_WEBEX_VOIP, configuration);
    BindSender();
}

TEST_F(CSecurityPolicyTest, NULL_masterKeySalt_CST_AES_CM_128_HMAC_SHA1_80_SecurityPolicy_video)
{
    CScopedTracer test_info;
    SecurityConfiguration configuration = InitCryptoConfiguration(CST_AES_CM_128_HMAC_SHA1_80);
    configuration.masterKeySaltLength = 30;
    InitSender(RTP_SESSION_WEBEX_VIDEO, configuration);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, configuration);
    BindSender();

}

TEST_F(CSecurityPolicyTest, NULL_masterKeySalt_CST_AES_CM_128_HMAC_SHA1_80_SecurityPolicy_audio)
{
    CScopedTracer test_info;
    SecurityConfiguration configuration = InitCryptoConfiguration(CST_AES_CM_128_HMAC_SHA1_80);
    configuration.masterKeySaltLength = 30;
    InitSender(RTP_SESSION_WEBEX_VOIP, configuration);
    InitRecver(RTP_SESSION_WEBEX_VOIP, configuration);
    BindSender();
}

TEST_F(CSecurityPolicyTest, ShortLen_masterKeySaltLength_CST_NULL_CIPHER_HMAC_SHA1_80_SecurityPolicy_video)
{
    CScopedTracer test_info;
    uint8_t keys[30] = {};
    int32_t ret = WRTPGetCryptoRandom(keys, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    SecurityConfiguration configuration = InitCryptoConfiguration(CST_NULL_CIPHER_HMAC_SHA1_80);
    configuration.masterKeySalt         = keys;
    configuration.masterKeySaltLength   = 10;
    InitSender(RTP_SESSION_WEBEX_VIDEO, configuration);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, configuration);
    BindSender();

    videoSendIndex = 0;
    videoRecvIndex = 0;
    WRTPMediaDataVideo videoData;
    FillAndSendVideoData(&videoData);

    //::SleepMs(100);
}

TEST_F(CSecurityPolicyTest, ShortLen_masterKeySaltLength_CST_NULL_CIPHER_HMAC_SHA1_80_SecurityPolicy_audio)
{
    CScopedTracer test_info;
    uint8_t keys[30] = {};
    int32_t ret = WRTPGetCryptoRandom(keys, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    SecurityConfiguration configuration = InitCryptoConfiguration(CST_NULL_CIPHER_HMAC_SHA1_80);
    configuration.masterKeySalt        = keys;
    configuration.masterKeySaltLength  = 10;
    InitSender(RTP_SESSION_WEBEX_VOIP, configuration);
    InitRecver(RTP_SESSION_WEBEX_VOIP, configuration);
    BindSender();

    audioSendIndex = 0;
    audioRecvIndex = 0;
    WRTPMediaDataVoIP audioData;
    FillAndSendVoIPData(&audioData);

    //::SleepMs(100);
}

TEST_F(CSecurityPolicyTest, ShortLen_masterSaltKeyLength_CST_AES_CM_128_HMAC_SHA1_80_SecurityPolicy_video)
{
    CScopedTracer test_info;
    uint8_t keys[30] = {};
    int32_t ret = WRTPGetCryptoRandom(keys, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    SecurityConfiguration configuration = InitCryptoConfiguration(CST_AES_CM_128_HMAC_SHA1_80);
    configuration.masterKeySalt        = keys;
    configuration.masterKeySaltLength  = 10;
    InitSender(RTP_SESSION_WEBEX_VIDEO, configuration);
    InitRecver(RTP_SESSION_WEBEX_VIDEO, configuration);
    BindSender();

    videoSendIndex = 0;
    videoRecvIndex = 0;
    WRTPMediaDataVideo videoData;
    FillAndSendVideoData(&videoData);

    //::SleepMs(100);
}

TEST_F(CSecurityPolicyTest, ShortLen_masterSaltKeyLength_CST_AES_CM_128_HMAC_SHA1_80_SecurityPolicy_audio)
{
    CScopedTracer test_info;
    uint8_t keys[30] = {};
    int32_t ret = WRTPGetCryptoRandom(keys, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    SecurityConfiguration configuration = InitCryptoConfiguration(CST_AES_CM_128_HMAC_SHA1_80);
    configuration.masterKeySalt        = keys;
    configuration.masterKeySaltLength  = 10;
    InitSender(RTP_SESSION_WEBEX_VOIP, configuration);
    InitRecver(RTP_SESSION_WEBEX_VOIP, configuration);
    BindSender();

    audioSendIndex = 0;
    audioRecvIndex = 0;
    WRTPMediaDataVoIP audioData;
    FillAndSendVoIPData(&audioData);

    //::SleepMs(100);
}

TEST_F(CSecurityPolicyTest, SenderCryptoSuite_CST_NULL_CIPHER_HMAC_SHA1_80_and_RecvCryptoSuite_CST_NULL_CIPHER_HMAC_SHA1_80_video)
{
    CScopedTracer test_info;
    uint8_t keys[30] = {};
    int32_t ret = WRTPGetCryptoRandom(keys, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    senderCryptoSuite = CST_NULL_CIPHER_HMAC_SHA1_80;
    SecurityConfiguration senderConfiguration = InitCryptoConfiguration(senderCryptoSuite);
    senderConfiguration.masterKeySalt        = keys;
    senderConfiguration.masterKeySaltLength  = 30;
    InitSender(RTP_SESSION_WEBEX_VIDEO, senderConfiguration);

    recverCryptoSuite = CST_NULL_CIPHER_HMAC_SHA1_80;
    SecurityConfiguration recverConfiguration = InitCryptoConfiguration(recverCryptoSuite);
    recverConfiguration.masterKeySalt        = keys;
    recverConfiguration.masterKeySaltLength  = 30;
    InitRecver(RTP_SESSION_WEBEX_VIDEO, recverConfiguration);
    BindSender();

    videoSendIndex = 0;
    videoRecvIndex = 0;
    WRTPMediaDataVideo videoData;
    FillAndSendVideoData(&videoData);

    //::SleepMs(100);
}

TEST_F(CSecurityPolicyTest, SenderCryptoSuite_CST_NULL_CIPHER_HMAC_SHA1_80_and_RecvCryptoSuite_CST_NULL_CIPHER_HMAC_SHA1_80_audio)
{
    CScopedTracer test_info;
    uint8_t keys[30] = {};
    int32_t ret = WRTPGetCryptoRandom(keys, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    senderCryptoSuite = CST_NULL_CIPHER_HMAC_SHA1_80;
    SecurityConfiguration senderConfiguration = InitCryptoConfiguration(senderCryptoSuite);
    senderConfiguration.masterKeySalt        = keys;
    senderConfiguration.masterKeySaltLength  = 30;
    InitSender(RTP_SESSION_WEBEX_VOIP, senderConfiguration);

    recverCryptoSuite = CST_NULL_CIPHER_HMAC_SHA1_80;
    SecurityConfiguration recverConfiguration = InitCryptoConfiguration(recverCryptoSuite);
    recverConfiguration.masterKeySalt        = keys;
    recverConfiguration.masterKeySaltLength  = 30;
    InitRecver(RTP_SESSION_WEBEX_VOIP, recverConfiguration);
    BindSender();

    audioSendIndex = 0;
    audioRecvIndex = 0;
    WRTPMediaDataVoIP audioData;
    FillAndSendVoIPData(&audioData);

    //::SleepMs(100);
}

TEST_F(CSecurityPolicyTest, SenderCryptoSuite_CST_AES_CM_128_HMAC_SHA1_80_and_RecvCryptoSuite_CST_AES_CM_128_HMAC_SHA1_80_video)
{
    CScopedTracer test_info;
    uint8_t keys[30] = {};
    int32_t ret = WRTPGetCryptoRandom(keys, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    senderCryptoSuite = CST_AES_CM_128_HMAC_SHA1_80;
    SecurityConfiguration senderConfiguration = InitCryptoConfiguration(senderCryptoSuite);
    senderConfiguration.masterKeySalt        = keys;
    senderConfiguration.masterKeySaltLength  = 30;
    InitSender(RTP_SESSION_WEBEX_VIDEO, senderConfiguration);

    recverCryptoSuite = CST_AES_CM_128_HMAC_SHA1_80;
    SecurityConfiguration recverConfiguration = InitCryptoConfiguration(recverCryptoSuite);
    recverConfiguration.masterKeySalt        = keys;
    recverConfiguration.masterKeySaltLength  = 30;
    InitRecver(RTP_SESSION_WEBEX_VIDEO, recverConfiguration);
    BindSender();

    videoSendIndex = 0;
    videoRecvIndex = 0;
    WRTPMediaDataVideo videoData;
    FillAndSendVideoData(&videoData);

    //::SleepMs(100);
}

TEST_F(CSecurityPolicyTest, SenderCryptoSuite_CST_AES_CM_128_HMAC_SHA1_80_and_RecvCryptoSuite_CST_AES_CM_128_HMAC_SHA1_80_audio)
{
    CScopedTracer test_info;
    uint8_t keys[30] = {};
    int32_t ret = WRTPGetCryptoRandom(keys, 30);
    ASSERT_TRUE(ret == WRTP_ERR_NOERR);

    senderCryptoSuite = CST_AES_CM_128_HMAC_SHA1_80;
    SecurityConfiguration senderConfiguration = InitCryptoConfiguration(senderCryptoSuite);
    senderConfiguration.masterKeySalt        = keys;
    senderConfiguration.masterKeySaltLength  = 30;
    InitSender(RTP_SESSION_WEBEX_VOIP, senderConfiguration);

    recverCryptoSuite = CST_AES_CM_128_HMAC_SHA1_80;
    SecurityConfiguration recverConfiguration = InitCryptoConfiguration(recverCryptoSuite);
    recverConfiguration.masterKeySalt        = keys;
    recverConfiguration.masterKeySaltLength  = 30;
    InitRecver(RTP_SESSION_WEBEX_VOIP, recverConfiguration);
    BindSender();

    audioSendIndex = 0;
    audioRecvIndex = 0;
    WRTPMediaDataVoIP audioData;
    FillAndSendVoIPData(&audioData);

    //::SleepMs(100);
}
