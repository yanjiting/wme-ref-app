#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "wrtpapi.h"
#include "rtpchannel.h"
#include "rtpsessionclient.h"
#include "testutil.h"

using namespace wrtp;

void MakeRTPPacketHeader(uint32_t csi, uint32_t ssrc, uint16_t seq, uint32_t ts, uint8_t pt, bool marker, uint8_t *buf);

TEST(RTPChannelFactoryMethodTest, Test_create_with_null_session_then_return_null_channel)
{
    CScopedTracer test_info;
    uint32_t channelId = 123;
    WRTPChannelParams channelParams;

    CRTPSessionClient *sessionClient = nullptr;
    CRTPChannelPtr channel;
    channel = CreateRTPChannelFactoryMethod(sessionClient, channelId, channelParams);
    ASSERT_TRUE(nullptr == channel);
}

TEST(RTPChannelFactoryMethodTest, Test_create_with_VoIP_session_then_return_VoIP_channel)
{
    CScopedTracer test_info;
    // create a VoIP session
    WRTPSessionParams sessionParams;
    sessionParams.sessionType = RTP_SESSION_WEBEX_VOIP;
    sessionParams.enableRTCP = true;

    IRTPSessionClient *session = nullptr;
    session =  WRTPCreateRTPSessionClient(sessionParams);
    ASSERT_TRUE(nullptr != session);

    session->IncreaseReference();

    // down cast
    CRTPSessionClient *sessionClient = (CRTPSessionClient *)(session);
    ASSERT_TRUE(nullptr != sessionClient);

    // create a VoIP channel
    uint32_t channelId = 123;
    WRTPChannelParams channelParams;

    CRTPChannelPtr channel = nullptr;
    channel = CreateRTPChannelFactoryMethod(sessionClient, channelId, channelParams);
    ASSERT_TRUE(nullptr != channel);

    channel->IncreaseReference();

    CRTPChannelVoIP *voipChannel = (CRTPChannelVoIP *)(channel.get());
    ASSERT_TRUE(nullptr != voipChannel);

    channel->Close();
    channel->DecreaseReference();

    session->DecreaseReference();
}

TEST(RTPChannelFactoryMethodTest, Test_create_with_Video_session_then_return_Video_channel)
{
    CScopedTracer test_info;
    // create a Video session
    WRTPSessionParams sessionParams;
    sessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    sessionParams.enableRTCP = true;

    IRTPSessionClient *session = nullptr;
    session =  WRTPCreateRTPSessionClient(sessionParams);
    ASSERT_TRUE(nullptr != session);

    session->IncreaseReference();

    // down cast
    CRTPSessionClient *sessionClient = (CRTPSessionClient *)(session);
    ASSERT_TRUE(nullptr != sessionClient);

    // create a Video channel
    uint32_t channelId = 123;
    WRTPChannelParams channelParams;

    CRTPChannelPtr channel = nullptr;
    channel = CreateRTPChannelFactoryMethod(sessionClient, channelId, channelParams);
    ASSERT_TRUE(nullptr != channel);

    channel->IncreaseReference();
    CRTPChannelVideo *voipChannel = (CRTPChannelVideo *)(channel.get());
    ASSERT_TRUE(nullptr != voipChannel);

    channel->Close();
    channel->DecreaseReference();

    session->DecreaseReference();
}

TEST(RTPChannelFactoryMethodTest, Test_create_with_AS_session_then_return_AS_channel)
{
    CScopedTracer test_info;
    // create a AS session
    WRTPSessionParams sessionParams;
    sessionParams.sessionType = RTP_SESSION_WEBEX_AS;
    sessionParams.enableRTCP = true;

    IRTPSessionClient *session = nullptr;
    session =  WRTPCreateRTPSessionClient(sessionParams);
    ASSERT_TRUE(nullptr != session);

    session->IncreaseReference();

    // down cast
    CRTPSessionClient *sessionClient = (CRTPSessionClient *)(session);
    ASSERT_TRUE(nullptr != sessionClient);

    // create a AS channel
    uint32_t channelId = 123;
    WRTPChannelParams channelParams;

    CRTPChannelPtr channel = nullptr;
    channel = CreateRTPChannelFactoryMethod(sessionClient, channelId, channelParams);
    ASSERT_TRUE(nullptr != channel);

    channel->IncreaseReference();
    CRTPChannelAS *asChannel = (CRTPChannelAS *)(channel.get());
    ASSERT_TRUE(nullptr != asChannel);

    channel->Close();
    channel->DecreaseReference();

    session->DecreaseReference();
}

#define CODEC_TYPE_AUDIO_TEST   101
#define PAYLOAD_TYPE_AUDIO_TEST 101
#define CODEC_TYPE_VIDEO_TEST   98
#define PAYLOAD_TYPE_VIDEO_TEST 98

#ifdef ENABLE_SSRC_COLLISION_HANDLING
TEST(RTPChannelSSRCCollisionTest, Test_VOIP_Session_SSRC_Collision)
{
    CScopedTracer test_info;
    // create a VOIP session
    WRTPSessionParams sessionParams;
    sessionParams.sessionType = RTP_SESSION_WEBEX_VOIP;
    sessionParams.enableRTCP = true;

    IRTPSessionClient *voipSession = nullptr;
    voipSession =  WRTPCreateRTPSessionClient(sessionParams);
    ASSERT_TRUE(nullptr != voipSession);

    voipSession->IncreaseReference();

    voipSession->RegisterPayloadType(CODEC_TYPE_AUDIO_TEST, PAYLOAD_TYPE_AUDIO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);

    // create a Void channel
    uint32_t channelId = 123;
    WRTPChannelParams channelParams;
    channelParams.fecType       = RTP_FEC_NONE;
    channelParams.pFragmentOp   = nullptr;

    IRTPChannel *channel = nullptr;
    channel = voipSession->CreateRTPChannel(channelId, channelParams);
    ASSERT_TRUE(nullptr != channel);

    channel->IncreaseReference();
    CRTPChannelVoIP *voipChannel = (CRTPChannelVoIP *)(channel);
    ASSERT_TRUE(nullptr != voipChannel);

    uint8_t buf[1024] = {0};
    memset(buf, 0xFF, 1024);

    WRTPMediaDataVoIP mda;
    mda.type = MEDIA_DATA_VOIP;
    mda.data = buf;
    mda.size = sizeof(buf);
    mda.timestamp = 0x88776655;
    mda.codecType = CODEC_TYPE_AUDIO_TEST;
    mda.marker = 0;
    channel->SendMediaData(&mda);

    CRTPStreamPtr outStream = voipChannel->GetStream(mda.codecType);
    ASSERT_TRUE(nullptr != outStream);
    uint32_t ssrc = outStream->GetSSRC();

    uint8_t inBuf[1024] = {0};
    memset(buf, 0xDE, 1024);
    MakeRTPPacketHeader(channelId, ssrc, 555, 99884, PAYLOAD_TYPE_AUDIO_TEST, true, inBuf);
    CCmMessageBlock mb(sizeof(inBuf), (char *)inBuf, 0, sizeof(inBuf));
    voipChannel->RecvRTPPacket(mb);
    EXPECT_TRUE(outStream->GetSSRCCollisionFlag());
    outStream.reset();

    channel->SendMediaData(&mda);
    CRTPStreamPtr outStream2 = voipChannel->GetStream(mda.codecType);
    ASSERT_TRUE(nullptr != outStream2);
    uint32_t ssrc2 = outStream2->GetSSRC();

    outStream2.reset();
    EXPECT_NE(ssrc, ssrc2);

    channel->Close();
    channel->DecreaseReference();

    voipSession->DecreaseReference();
}

TEST(RTPChannelSSRCCollisionTest, Test_Video_Session_SSRC_Collision)
{
    CScopedTracer test_info;
    // create a Video session
    WRTPSessionParams sessionParams;
    sessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    sessionParams.enableRTCP = true;

    IRTPSessionClient *videoSession = nullptr;
    videoSession =  WRTPCreateRTPSessionClient(sessionParams);
    ASSERT_TRUE(nullptr != videoSession);

    videoSession->IncreaseReference();

    videoSession->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);

    // create a Void channel
    uint32_t channelId = 123;
    WRTPChannelParams channelParams;
    channelParams.fecType       = RTP_FEC_NONE;
    channelParams.pFragmentOp   = nullptr;

    IRTPChannel *channel = nullptr;
    channel = videoSession->CreateRTPChannel(channelId, channelParams);
    ASSERT_TRUE(nullptr != channel);

    channel->IncreaseReference();
    CRTPChannelVideo *videoChannel = (CRTPChannelVideo *)(channel);
    ASSERT_TRUE(nullptr != videoChannel);

    uint8_t buf[1024] = {0};
    memset(buf, 0xFF, 1024);

    WRTPMediaDataVideo mdv;
    mdv.type = MEDIA_DATA_VIDEO;
    mdv.data = buf;
    mdv.size = sizeof(buf);
    mdv.timestamp = 0x88776655;
    mdv.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv.marker = 1;
    mdv.disposable = false;
    mdv.switchable = false;
    mdv.priority = 3;
    mdv.frameType = 2;
    mdv.DID = 2;
    mdv.TID = 1;
    mdv.maxTID = 3;
    mdv.maxDID = 2;
    channel->SendMediaData(&mdv);

    CRTPStreamPtr outStream = videoChannel->GetStream(mdv.DID);
    ASSERT_TRUE(nullptr != outStream);
    uint32_t ssrc = outStream->GetSSRC();

    uint8_t inBuf[1024] = {0};
    memset(buf, 0xDE, 1024);
    MakeRTPPacketHeader(channelId, ssrc, 555, 99884, PAYLOAD_TYPE_VIDEO_TEST, true, inBuf);
    CCmMessageBlock mb(sizeof(inBuf), (char *)inBuf, 0, sizeof(inBuf));
    videoChannel->RecvRTPPacket(mb);
    EXPECT_TRUE(outStream->GetSSRCCollisionFlag());
    outStream.reset();

    channel->SendMediaData(&mdv);
    CRTPStreamPtr outStream2 = videoChannel->GetStream(mdv.DID);
    ASSERT_TRUE(nullptr != outStream2);
    uint32_t ssrc2 = outStream2->GetSSRC();
    outStream2.reset();
    EXPECT_NE(ssrc, ssrc2);

    channel->Close();
    channel->DecreaseReference();

    videoSession->DecreaseReference();
}

#if 0
TEST(RTPChannelSSRCCollisionTest, Test_Mixed_Session_SSRC_Collision)
{
    CScopedTracer test_info;
    // create a VOIP session
    WRTPSessionParams voipSessionParams;
    voipSessionParams.sessionType = RTP_SESSION_WEBEX_VOIP;
    voipSessionParams.enableRTCP = true;

    IRTPSessionClient *voipSession = nullptr;
    voipSession =  WRTPCreateRTPSessionClient(voipSessionParams);
    ASSERT_TRUE(nullptr != voipSession);
    voipSession->IncreaseReference();

    voipSession->RegisterPayloadType(CODEC_TYPE_AUDIO_TEST, PAYLOAD_TYPE_AUDIO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);

    // create a Voip channel
    uint32_t voipChannelId = 123;
    WRTPChannelParams voipChannelParams;
    voipChannelParams.fecType       = RTP_FEC_NONE;
    voipChannelParams.pFragmentOp   = nullptr;

    IRTPChannel *channel = nullptr;
    channel = voipSession->CreateRTPChannel(voipChannelId, voipChannelParams);
    ASSERT_TRUE(nullptr != channel);

    channel->IncreaseReference();
    CRTPChannelVoIP *voipChannel = (CRTPChannelVoIP *)(channel);
    ASSERT_TRUE(nullptr != voipChannel);

    // create a Video session
    WRTPSessionParams videoSessionParams;
    videoSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    videoSessionParams.enableRTCP = true;

    IRTPSessionClient *videoSession = nullptr;
    videoSession =  WRTPCreateRTPSessionClient(videoSessionParams);
    ASSERT_TRUE(nullptr != videoSession);

    videoSession->IncreaseReference();

    videoSession->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);

    // create a Void channel
    uint32_t videoChannelId = 456;
    WRTPChannelParams videoChannelParams;
    videoChannelParams.fecType       = RTP_FEC_NONE;
    videoChannelParams.pFragmentOp   = nullptr;

    channel = videoSession->CreateRTPChannel(videoChannelId, videoChannelParams);
    ASSERT_TRUE(nullptr != channel);

    channel->IncreaseReference();
    CRTPChannelVideo *videoChannel = (CRTPChannelVideo *)(channel);
    ASSERT_TRUE(nullptr != videoChannel);

    uint8_t buf[1024] = {0};
    memset(buf, 0xFF, 1024);

    WRTPMediaDataVideo mdv;
    mdv.type = MEDIA_DATA_VIDEO;
    mdv.data = buf;
    mdv.size = sizeof(buf);
    mdv.timestamp = 0x88776655;
    mdv.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv.marker = 1;
    mdv.disposable = false;
    mdv.switchable = false;
    mdv.priority = 3;
    mdv.frameType = 2;
    mdv.DID = 2;
    mdv.TID = 1;
    mdv.maxTID = 3;
    mdv.maxDID = 2;
    videoChannel->SendMediaData(&mdv);

    CRTPStreamPtr outStream = videoChannel->GetStream(mdv.DID);
    ASSERT_TRUE(nullptr != outStream);
    uint32_t ssrc = outStream->GetSSRC();

    uint8_t inBuf[1024] = {0};
    memset(buf, 0xDE, 1024);
    MakeRTPPacketHeader(voipChannelId, ssrc, 555, 99884, PAYLOAD_TYPE_AUDIO_TEST, true, inBuf);
    CCmMessageBlock mb(sizeof(inBuf), (char *)inBuf, 0, sizeof(inBuf));
    voipChannel->RecvRTPPacket(mb);
    EXPECT_TRUE(outStream->GetSSRCCollisionFlag());
    outStream.reset();

    videoChannel->SendMediaData(&mdv);
    CRTPStreamPtr outStream2 = videoChannel->GetStream(mdv.DID);
    ASSERT_TRUE(nullptr != outStream2);
    uint32_t ssrc2 = outStream2->GetSSRC();
    outStream2.reset();
    EXPECT_NE(ssrc, ssrc2);

    voipChannel->Close();
    voipChannel->DecreaseReference();

    videoChannel->Close();
    videoChannel->DecreaseReference();

    voipSession->DecreaseReference();

    videoSession->DecreaseReference();
}
#endif
#endif
void MakeRTPPacketHeader(uint32_t csi, uint32_t ssrc, uint16_t seq, uint32_t ts, uint8_t pt, bool marker, uint8_t *buf)
{
    buf[0] = (2 << 6)|1; // version & count
    buf[1] = 0x80|pt; // marker & payload type
    *(uint16_t *)(buf + 2) = htons(seq);
    *(uint32_t *)(buf + 4) = htonl(ts); // timestamp
    *(uint32_t *)(buf + 8) = htonl(ssrc); // ssrc
    *(uint32_t *)(buf + 12) = htonl(csi); // csi
}


