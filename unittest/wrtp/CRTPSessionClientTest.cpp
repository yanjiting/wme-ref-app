#include "gmock/gmock.h"
#include "gtest/gtest.h"
#define private public
#define protected public
#include "rtpsessionclient.h"
#include "rtputils.h"
#include "mmframemgr.h"
#include "qosapimock.h"
#include "wrtpwmeapi.h"
#include "rtpheaderext.h"
#include "WMEInterfaceMock.h"
#include "testutil.h"
#include "videostream.h"
#include "rtpchannel.h"

#include <memory>
#include <functional>

using namespace testing;
using namespace wrtp;
using namespace wme;


class CNetworkStatusObserverMock : public INetworkStatusObserver
{
public:
    CNetworkStatusObserverMock(): m_allocatedBandwidth(0)
        , m_rtt(0), m_queuingDelay(0), m_totalBandwidth(0), m_lossRate(0.0f), m_state(0), m_streamBitrate(0) {}

    void OnAdjustment(const NetworkStatus &status)
    {
        m_allocatedBandwidth    = status.allocatedBandwidth;
        m_rtt                   = status.rtt;
        m_queuingDelay          = status.queuingDelay;
        m_totalBandwidth        = status.totalBandwidth;
        m_lossRate              = status.lossRate;
        m_state                 = status.state;
        return;
    }

    virtual void NotifyStreamBandwidth(const StreamIdInfo &streamId, uint32_t bandwidthInBytePerSec)
    {
        m_streamBitrate = bandwidthInBytePerSec;
    }

    uint32_t GetAllocatedBandwidth() const
    {
        return m_allocatedBandwidth;
    }
    uint32_t GetRTT() const
    {
        return m_rtt;
    }
    uint32_t GetQueuingDelay() const
    {
        return m_queuingDelay;
    }

    uint32_t GetTotalBandwidth() const
    {
        return m_totalBandwidth;
    }
    float GetLossRate() const
    {
        return m_lossRate;
    }
    uint32_t GetState() const
    {
        return m_state;
    }
    uint32_t GetStreamBandwidth() const
    {
        return m_streamBitrate;
    }

private:
    uint32_t m_state;
    uint32_t m_allocatedBandwidth;
    uint32_t m_totalBandwidth;
    uint32_t m_queuingDelay;
    uint32_t m_rtt;
    float m_lossRate;
    uint32_t m_streamBitrate;
};

class CMediaSessionFilterMockForTest: public IMediaSessionFilter
{
public:
    virtual int32_t QueryRequirement(QUERY_TYPE eType, uint32_t &bandwidth)
    {
        return 0;
    }
};

class CRTPSessionClientTEST : public ::testing::Test
{
public:
    virtual void SetUp()
    {
        m_audioLevelID = 2;

        m_voipChannelId = 666;
        m_voipPayloadType = 12;
        m_voipFecType = RTP_FEC_NONE;

        InitMediaDataVoIP();
    }

    virtual void TearDown()
    {
        if (m_mediaDataVoIP.data) {
            delete [] m_mediaDataVoIP.data;
        }
    }

    void InitMediaDataVoIP()
    {
        m_mediaDataVoIP.type = MEDIA_DATA_VOIP;
        m_mediaDataVoIP.timestamp = 987654321;
        m_mediaDataVoIP.sampleTimestamp = 987654321;
        m_mediaDataVoIP.data = new uint8_t[12];
        memcpy(m_mediaDataVoIP.data, "ABCDabcdABCD", 12);
        m_mediaDataVoIP.size = 12;

        m_mediaDataVoIP.vad = true;
        m_mediaDataVoIP.level = 8;
        m_mediaDataVoIP.codecType = 50;
    }

public:
    WRTPSessionParams m_rtpSessionParams;
    WRTPChannelParams m_rtpChannelParams;
    WRTPMediaDataVoIP m_mediaDataVoIP;

    uint8_t m_audioLevelID;

    uint32_t m_voipChannelId;
    uint8_t m_voipPayloadType;
    WRTPFecType m_voipFecType;
};

class CRTPSendingVoIPSink : public IRTPMediaTransport
{
public:
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

        CRTPPacketLazy rtpPacketLazy;
        rtpPacketLazy.Bind(*mb);

        EXPECT_EQ(m_mediaDataVoIP.timestamp, rtpPacketLazy.GetTimestamp());
        EXPECT_EQ(1, rtpPacketLazy.GetCSRCCount());
        EXPECT_EQ(m_voipChannelId, rtpPacketLazy.GetCSRC(0));

        uint32_t payloadLength = rtpPacketLazy.GetPayloadLength();
        EXPECT_EQ(0, memcmp(m_mediaDataVoIP.data, rtpPacketLazy.GetPayloadData(), payloadLength));
        EXPECT_TRUE(rtpPacketLazy.HasExtension());

        uint8_t expect_elementData = m_mediaDataVoIP.level&0x7F;
        if (m_mediaDataVoIP.vad) {
            expect_elementData |= 0x80;
        }

        uint8_t *elementData = nullptr, elementLength = 0;
        CmResult ret = rtpPacketLazy.GetExtensionElement(m_audioLevelID, elementData, elementLength);
        EXPECT_EQ(WRTP_ERR_NOERR, ret);
        EXPECT_EQ(1, elementLength);
        EXPECT_EQ(expect_elementData, *elementData);

        mb->DestroyChained();
        mb = nullptr;

        return WME_S_OK;
    }

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        return WME_S_OK;
    }

    void SetRTPMediaParams(uint32_t voipChannelId, uint8_t audioLevelID, WRTPMediaDataVoIP mediaDataVoIP)
    {
        m_voipChannelId = voipChannelId;
        m_audioLevelID = audioLevelID;
        m_mediaDataVoIP = mediaDataVoIP;
    }
private:
    WRTPMediaDataVoIP m_mediaDataVoIP;
    uint8_t m_audioLevelID;
    uint32_t m_voipChannelId;
};

class CVoIPDataRecvSink : public IMediaDataRecvSink
{
public:
    int32_t OnRecvMediaData(RTP_IN WRTPMediaData *pData, RTP_IN WRTPMediaDataIntegrityInfo *pIntegrityInfo)
    {
        WRTPMediaDataVoIP *pVoIPData = (WRTPMediaDataVoIP *)pData;

        EXPECT_TRUE(pVoIPData != nullptr && pIntegrityInfo != nullptr);
        EXPECT_EQ(1, pIntegrityInfo->packetCount);
        EXPECT_EQ(0, pIntegrityInfo->lossFlag[0]);
        EXPECT_EQ(m_sequence, pIntegrityInfo->startSequence);

        EXPECT_EQ(m_mediaDataVoIP.sampleTimestamp, pVoIPData->sampleTimestamp);
        EXPECT_EQ(m_sequence, pVoIPData->sequenceNumber);
        EXPECT_EQ(m_ssrc, pVoIPData->ssrc);

        EXPECT_EQ(m_mediaDataVoIP.size, pVoIPData->size);
        EXPECT_EQ(0, memcmp(m_mediaDataVoIP.data, pVoIPData->data, m_mediaDataVoIP.size));

        EXPECT_EQ(1, pVoIPData->csrcCount);
        EXPECT_EQ(m_voipChannelId, pVoIPData->csrcArray[0]);

        EXPECT_EQ(m_mediaDataVoIP.vad, pVoIPData->vad);
        EXPECT_EQ(m_mediaDataVoIP.level, pVoIPData->level);

        return 0;
    }

    void SetRTPMediaParams(uint32_t voipChannelId, uint8_t audioLevelID, WRTPMediaDataVoIP mediaDataVoIP)
    {
        m_voipChannelId = voipChannelId;
        m_audioLevelID = audioLevelID;
        m_mediaDataVoIP = mediaDataVoIP;

        m_ssrc = 11;
        m_sequence = 12;
    }

    void BuildMockRecvRTPPacket(CCmMessageBlock &rtpPacket)
    {
        CRTPPacket packet;

        uint8_t elementData = m_mediaDataVoIP.level&0x7F;
        if (m_mediaDataVoIP.vad) {
            elementData |= 0x80;
        }

        packet.AddExtensionElement(m_audioLevelID, &elementData, sizeof(elementData));
        packet.SetMarker();
        packet.SetTimestamp(m_mediaDataVoIP.timestamp);
        packet.AddCSRC(m_voipChannelId);
        packet.SetSSRC(m_ssrc);
        packet.SetSequenceNumber(m_sequence);
        packet.SetPayloadType(PAYLOAD_TYPE_AUDIO_TEST);
        CCmMessageBlock mb(m_mediaDataVoIP.size, (LPCSTR)m_mediaDataVoIP.data);
        mb.AdvanceTopLevelWritePtr(m_mediaDataVoIP.size);
        packet.SetPayload(mb);
        packet.Pack(rtpPacket);
    }

private:
    WRTPMediaDataVoIP m_mediaDataVoIP;
    uint8_t m_audioLevelID;
    uint32_t m_voipChannelId;

    uint32_t m_ssrc;
    uint16_t m_sequence;
};

TEST_F(CRTPSessionClientTEST, Test_Create_Client_VoIP_RTP_Session_and_Send_VoIP_Media_Data)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VOIP;
    m_rtpSessionParams.enableRTCP = false;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    EXPECT_TRUE(session != nullptr);

    session->IncreaseReference();

    EXPECT_EQ(session->UpdateRTPExtension(GetAudioRTPHeaderExtName(RTPEXT_AudioLevel), m_audioLevelID, wrtp::STREAM_INOUT), 0);

    int32_t ret = session->RegisterPayloadType(50, 10, 48000);
    ret = session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    ASSERT_TRUE(0 == ret);

    CRTPSendingVoIPSink rtpSendingSink;
    EXPECT_TRUE(WME_SUCCEEDED(session->SetMediaTransport(&rtpSendingSink)));
    rtpSendingSink.SetRTPMediaParams(m_voipChannelId, m_audioLevelID, m_mediaDataVoIP);

    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(m_voipChannelId, m_rtpChannelParams);
    EXPECT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();

    EXPECT_EQ(rtpSendChannel->GetChannelId(), m_voipChannelId);
    EXPECT_EQ(rtpSendChannel->SendMediaData(&m_mediaDataVoIP),0);

    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();

    session->DecreaseReference();
}

TEST_F(CRTPSessionClientTEST, Test_Create_Client_VoIP_RTP_Session_and_Recv_VoIP_Media_Data)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VOIP;
    m_rtpSessionParams.enableRTCP = false;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    EXPECT_TRUE(session != nullptr);

    session->IncreaseReference();

    EXPECT_EQ(session->UpdateRTPExtension(GetAudioRTPHeaderExtName(RTPEXT_AudioLevel), m_audioLevelID, wrtp::STREAM_INOUT), 0);

    int32_t ret = session->RegisterPayloadType(50, 10, 48000);
    ret = session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    ASSERT_TRUE(0 == ret);

    IRTPChannel *rtpRecvChannel =  session->CreateRTPChannel(m_voipChannelId, m_rtpChannelParams);
    EXPECT_TRUE(rtpRecvChannel != nullptr);
    rtpRecvChannel->IncreaseReference();

    EXPECT_EQ(rtpRecvChannel->GetChannelId(), m_voipChannelId);

    CVoIPDataRecvSink voipDataRecvSink;
    rtpRecvChannel->SetMediaDataRecvSink(&voipDataRecvSink);
    voipDataRecvSink.SetRTPMediaParams(m_voipChannelId, m_audioLevelID, m_mediaDataVoIP);

    CCmMessageBlock rtpPacket((DWORD)0);
    voipDataRecvSink.BuildMockRecvRTPPacket(rtpPacket);

    IRTPMediaPackage *mediaPackage = nullptr;
    ConvertMessageBlockToMediaPackage(rtpPacket, &mediaPackage, true);
    rtpRecvChannel->ReceiveRTPPacket(mediaPackage);
    WME_SAFE_RELEASE(mediaPackage);

    rtpRecvChannel->Close();
    rtpRecvChannel->DecreaseReference();

    session->DecreaseReference();
}

TEST_F(CRTPSessionClientTEST, Test_SetOption_GetOption)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VOIP;
    m_rtpSessionParams.enableRTCP = false;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    EXPECT_TRUE(session != nullptr);

    session->IncreaseReference();

    int32_t ret = session->RegisterPayloadType(50, 10, 48000);
    ASSERT_TRUE(0 == ret);

    IRTPChannel *channel =  session->CreateRTPChannel(123, m_rtpChannelParams);
    EXPECT_TRUE(channel != nullptr);
    channel->IncreaseReference();

    channel->Close();
    channel->DecreaseReference();

    session->DecreaseReference();
}

TEST_F(CRTPSessionClientTEST, Test_Set_And_Get)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VOIP;
    m_rtpSessionParams.enableRTCP = false;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    EXPECT_TRUE(session != nullptr);

    session->IncreaseReference();

    EXPECT_EQ(session->UpdateRTPExtension(GetAudioRTPHeaderExtName(RTPEXT_AudioLevel), m_audioLevelID, wrtp::STREAM_INOUT), 0);

    int32_t ret = session->RegisterPayloadType(50, 10, 48000);
    ret = session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    ASSERT_TRUE(0 == ret);

    CRTPSendingVoIPSink rtpSendingSink;
    EXPECT_TRUE(WME_SUCCEEDED(session->SetMediaTransport(&rtpSendingSink)));
    rtpSendingSink.SetRTPMediaParams(m_voipChannelId, m_audioLevelID, m_mediaDataVoIP);

    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(m_voipChannelId, m_rtpChannelParams);
    EXPECT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();

    EXPECT_EQ(rtpSendChannel->GetChannelId(), m_voipChannelId);

    EXPECT_EQ(rtpSendChannel->SendMediaData(&m_mediaDataVoIP),0);

    rtpSendChannel->SetChannelId(1000);
    EXPECT_EQ(rtpSendChannel->GetChannelId(), 1000);

    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();

    session->DecreaseReference();
}

TEST_F(CRTPSessionClientTEST, Test_INetworkStatusObserver_API_when_QoSDisabled_and_TMMBRReceived)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;

    CNetworkStatusObserverMock networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(nullptr != session);
    session->IncreaseReference();
    CRTPSessionClient *client = (CRTPSessionClient *)(session);
    ASSERT_TRUE(nullptr != client);

    CRTCPHandlerSharedPtr handler = client->m_rtcpHandler;
    ASSERT_TRUE(nullptr != handler);
    handler->ConfigTMMBR(true, false);

    CRTPFBPacket packet1;
    TMMBRVECTOR items1;
    TMMBRTuple item1;
    item1.ssrc = handler->m_ssrc;
    item1.maxTBR = 2*1024*1024;
    item1.oh = 0;
    items1.push_back(item1);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items1, packet1));
    EXPECT_EQ(packet1.m_fciLength, 8);
    items1.clear();

    CCmMessageBlock mb1(packet1.CalcEncodeSize());
    CCmByteStreamNetwork os1(mb1);
    EXPECT_TRUE(!!packet1.Encode(os1, mb1));

    client->HandleRecvedRTCPPacket(mb1);

#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ((int32_t)(2*1024*1024/8*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());
#endif
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ(wqos::BANDWIDTH_UP, networkStatusObserver.GetState());
#endif

    CRTPFBPacket packet2;
    TMMBRVECTOR items2;
    TMMBRTuple item2;
    item2.ssrc = handler->m_ssrc;
    item2.maxTBR = 1024*1024;
    item2.oh = 0;
    items2.push_back(item2);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items2, packet2));
    EXPECT_EQ(packet2.m_fciLength, 8);
    items2.clear();

    CCmMessageBlock mb2(packet2.CalcEncodeSize());
    CCmByteStreamNetwork os2(mb2);
    EXPECT_TRUE(!!packet2.Encode(os2, mb2));

    client->HandleRecvedRTCPPacket(mb2);

#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ((int32_t)(1024*1024/8*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());
#endif
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ(wqos::BANDWIDTH_DOWN, networkStatusObserver.GetState());
#endif

    session->DecreaseReference();
}

TEST_F(CRTPSessionClientTEST, Test_INetworkStatusObserver_API_when_QoSEnabled_and_NoTMMBRReceived)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;

    CNetworkStatusObserverMock networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(nullptr != session);
    session->IncreaseReference();
    CRTPSessionClient *client = (CRTPSessionClient *)(session);
    ASSERT_TRUE(nullptr != client);

    CMediaSessionFilterMockForTest filter;
    QoSParameter params;
    char peerId[] = "this is an id";
    params.peerID = (uint8_t *)peerId;
    params.lengthOfPeerID = sizeof(peerId);
    params.mediaSessionFilter = &filter;

    ASSERT_EQ(WRTP_ERR_NOERR, session->EnableQoS(params));

    CRTCPHandlerSharedPtr handler = client->m_rtcpHandler;
    ASSERT_TRUE(nullptr != handler);

    uint32_t bandwidth = 0;
    AggregateMetric metric;
    metric.rtt       = 50;
    metric.qdelay    = 20;
    metric.lossRatio = 0.5f;

    handler->OnNetworkControlledInformation(wqos::BANDWIDTH_HOLD, metric, bandwidth, bandwidth);

    EXPECT_EQ(0, networkStatusObserver.GetAllocatedBandwidth());
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_DOWN, networkStatusObserver.GetState());
#else
    EXPECT_EQ(50, networkStatusObserver.GetRTT());
    EXPECT_EQ(20, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_NE(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_HOLD, networkStatusObserver.GetState());
#endif


    bandwidth = 2*1024*1024/8;

    handler->OnNetworkControlledInformation(wqos::BANDWIDTH_HOLD, metric, bandwidth, bandwidth);

    EXPECT_EQ((int32_t)(2*1024*1024/8*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_UP, networkStatusObserver.GetState());
#else
    EXPECT_EQ(50, networkStatusObserver.GetRTT());
    EXPECT_EQ(20, networkStatusObserver.GetQueuingDelay());
    EXPECT_NE(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_NE(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_HOLD, networkStatusObserver.GetState());
#endif

    session->DisableQoS();
    session->DecreaseReference();
}

TEST_F(CRTPSessionClientTEST, Test_INetworkStatusObserver_API_when_QoSEnabled_then_QoSDisabled_and_TMMBRReceived)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;

    CNetworkStatusObserverMock networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(nullptr != session);
    session->IncreaseReference();
    CRTPSessionClient *client = (CRTPSessionClient *)(session);
    ASSERT_TRUE(nullptr != client);

    CMediaSessionFilterMockForTest filter;
    QoSParameter params;
    char peerId[] = "this is an id";
    params.peerID = (uint8_t *)peerId;
    params.lengthOfPeerID = sizeof(peerId);
    params.mediaSessionFilter = &filter;
    params.targetBandwidth = 2*720*1024/8;

    ASSERT_EQ(WRTP_ERR_NOERR, session->EnableQoS(params));

    CRTCPHandlerSharedPtr handler = client->m_rtcpHandler;
    ASSERT_TRUE(nullptr != handler);

    IRTPChannel *channel =  session->CreateRTPChannel(1111, m_rtpChannelParams);
    ASSERT_TRUE(channel != nullptr);
    channel->IncreaseReference();
    CRTPStreamPtr stream = client->CreateRTPStream(handler->m_ssrc, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_OUT);

    CRTPFBPacket packet1;
    TMMBRVECTOR items1;
    TMMBRTuple item1;
    item1.ssrc = handler->m_ssrc;
    item1.maxTBR = 2*1024*1024;
    item1.oh = 0;
    items1.push_back(item1);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items1, packet1));
    EXPECT_EQ(packet1.m_fciLength, 8);
    items1.clear();

    CCmMessageBlock mb1(packet1.CalcEncodeSize());
    CCmByteStreamNetwork os1(mb1);
    EXPECT_TRUE(!!packet1.Encode(os1, mb1));

    client->HandleRecvedRTCPPacket(mb1);

    EXPECT_EQ((int32_t)(WRTP_SESSION_INITIAL_BANDWIDTH*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetStreamBandwidth());
#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_HOLD, networkStatusObserver.GetState());
#else
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_NE(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_NE(wqos::BANDWIDTH_HOLD, networkStatusObserver.GetState());
#endif

    uint32_t bandwidth = 100;
    AggregateMetric metric;
    metric.rtt       = 50;
    metric.qdelay    = 20;
    metric.lossRatio = 0.5f;

    handler->OnNetworkControlledInformation(wqos::BANDWIDTH_HOLD, metric, bandwidth, bandwidth);

#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ(100*WRTP_PAYLOAD_BW_RATIO, networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_DOWN, networkStatusObserver.GetState());
#else
    EXPECT_EQ(100*WRTP_PAYLOAD_BW_RATIO, networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(50, networkStatusObserver.GetRTT());
    EXPECT_EQ(20, networkStatusObserver.GetQueuingDelay());
    EXPECT_NE(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_NE(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_HOLD, networkStatusObserver.GetState());
#endif

    session->DisableQoS();

    EXPECT_EQ((int32_t)(WRTP_SESSION_INITIAL_BANDWIDTH*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_HOLD, networkStatusObserver.GetState());

    CRTPFBPacket packet2;
    TMMBRVECTOR items2;
    TMMBRTuple item2;
    item2.ssrc = handler->m_ssrc;
    item2.maxTBR = 1*1024*1024;
    item2.oh = 0;
    items2.push_back(item2);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items2, packet2));
    EXPECT_EQ(packet2.m_fciLength, 8);
    items2.clear();

    CCmMessageBlock mb2(packet2.CalcEncodeSize());
    CCmByteStreamNetwork os2(mb2);
    EXPECT_TRUE(!!packet2.Encode(os2, mb2));

    client->HandleRecvedRTCPPacket(mb2);

#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ((int32_t)(1*1024*1024/8*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_DOWN, networkStatusObserver.GetState());
#endif

    channel->Close();
    channel->DecreaseReference();
    stream.reset();
    session->DecreaseReference();
}

TEST_F(CRTPSessionClientTEST, Test_INetworkStatusObserver_API_when_QoSEnabled_and_TMMBRReceived_and_BandwidthIsBad)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;

    CNetworkStatusObserverMock networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(nullptr != session);
    session->IncreaseReference();
    CRTPSessionClient *client = (CRTPSessionClient *)(session);
    ASSERT_TRUE(nullptr != client);

    CMediaSessionFilterMockForTest filter;
    QoSParameter params;
    char peerId[] = "this is an id";
    params.peerID = (uint8_t *)peerId;
    params.lengthOfPeerID = sizeof(peerId);
    params.mediaSessionFilter = &filter;
    params.targetBandwidth = 2*720*1024/8;

    ASSERT_EQ(WRTP_ERR_NOERR, session->EnableQoS(params));

    CRTCPHandlerSharedPtr handler = client->m_rtcpHandler;
    ASSERT_TRUE(nullptr != handler);
    handler->ConfigTMMBR(true, false);

    CRTPFBPacket packet1;
    TMMBRVECTOR items1;
    TMMBRTuple item1;
    item1.ssrc = handler->m_ssrc;
    item1.maxTBR = 2*1024*1024;
    item1.oh = 0;
    items1.push_back(item1);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items1, packet1));
    EXPECT_EQ(packet1.m_fciLength, 8);
    items1.clear();

    CCmMessageBlock mb1(packet1.CalcEncodeSize());
    CCmByteStreamNetwork os1(mb1);
    EXPECT_TRUE(!!packet1.Encode(os1, mb1));

    client->HandleRecvedRTCPPacket(mb1);

#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ((uint32_t)(2*720*1024/8*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_HOLD, networkStatusObserver.GetState());
#endif

    uint32_t bandwidth = 0;
    AggregateMetric metric;
    metric.rtt       = 50;
    metric.qdelay    = 20;
    metric.lossRatio = 0.5f;

    handler->OnNetworkControlledInformation(wqos::BANDWIDTH_HOLD, metric, bandwidth, bandwidth);

#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ(0, networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_DOWN, networkStatusObserver.GetState());
#endif

    CRTPFBPacket packet2;
    TMMBRVECTOR items2;
    TMMBRTuple item2;
    item2.ssrc = handler->m_ssrc;
    item2.maxTBR = 1*1024*1024;
    item2.oh = 0;
    items2.push_back(item2);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items2, packet2));
    EXPECT_EQ(packet2.m_fciLength, 8);
    items2.clear();

    CCmMessageBlock mb2(packet2.CalcEncodeSize());
    CCmByteStreamNetwork os2(mb2);
    EXPECT_TRUE(!!packet2.Encode(os2, mb2));

    client->HandleRecvedRTCPPacket(mb2);

#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ(0, networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_HOLD, networkStatusObserver.GetState());
#endif

    bandwidth = (uint32_t)(float)(1.5*1024*1024/8);

    handler->OnNetworkControlledInformation(wqos::BANDWIDTH_HOLD, metric, bandwidth, bandwidth);

#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ((uint32_t)(1.5*1024*1024/8*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_UP, networkStatusObserver.GetState());
#endif

    session->DisableQoS();
    session->DecreaseReference();
}

TEST_F(CRTPSessionClientTEST, Test_INetworkStatusObserver_API_when_QoSEnabled_and_TMMBRReceived_and_BandwidthIsGood)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;

    CNetworkStatusObserverMock networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(nullptr != session);
    session->IncreaseReference();
    CRTPSessionClient *client = (CRTPSessionClient *)(session);
    ASSERT_TRUE(nullptr != client);

    CMediaSessionFilterMockForTest filter;
    QoSParameter params;
    char peerId[] = "this is an id";
    params.peerID = (uint8_t *)peerId;
    params.lengthOfPeerID = sizeof(peerId);
    params.mediaSessionFilter = &filter;
    params.targetBandwidth = 2*720*1024/8;

    ASSERT_EQ(WRTP_ERR_NOERR, session->EnableQoS(params));

    CRTCPHandlerSharedPtr handler = client->m_rtcpHandler;
    ASSERT_TRUE(nullptr != handler);
    handler->ConfigTMMBR(true, false);

    CRTPFBPacket packet1;
    TMMBRVECTOR items1;
    TMMBRTuple item1;
    item1.ssrc = handler->m_ssrc;
    item1.maxTBR = 2*1024*1024;
    item1.oh = 0;
    items1.push_back(item1);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items1, packet1));
    EXPECT_EQ(packet1.m_fciLength, 8);
    items1.clear();

    CCmMessageBlock mb1(packet1.CalcEncodeSize());
    CCmByteStreamNetwork os1(mb1);
    EXPECT_TRUE(!!packet1.Encode(os1, mb1));

    client->HandleRecvedRTCPPacket(mb1);

#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ((uint32_t)(2*720*1024/8*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_HOLD, networkStatusObserver.GetState());
#endif

    uint32_t bandwidth = 3 * 1024 * 1024 / 8;
    AggregateMetric metric;
    metric.rtt       = 50;
    metric.qdelay    = 20;
    metric.lossRatio = 0.5f;

    handler->OnNetworkControlledInformation(wqos::BANDWIDTH_HOLD, metric, bandwidth, bandwidth);

#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ((uint32_t)(3*1024*1024/8*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_UP, networkStatusObserver.GetState());
#endif

    CRTPFBPacket packet2;
    TMMBRVECTOR items2;
    TMMBRTuple item2;
    item2.ssrc = handler->m_ssrc;
    item2.maxTBR = 4*1024*1024;
    item2.oh = 0;
    items2.push_back(item2);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items2, packet2));
    EXPECT_EQ(packet2.m_fciLength, 8);
    items2.clear();

    CCmMessageBlock mb2(packet2.CalcEncodeSize());
    CCmByteStreamNetwork os2(mb2);
    EXPECT_TRUE(!!packet2.Encode(os2, mb2));

    client->HandleRecvedRTCPPacket(mb2);

#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ((uint32_t)(3*1024*1024/8*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_HOLD, networkStatusObserver.GetState());
#endif

    session->DisableQoS();
    session->DecreaseReference();
}

TEST_F(CRTPSessionClientTEST, Test_INetworkStatusObserver_API_when_QoSDisabled_and_not_set_initial_bandwidth)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;

    CNetworkStatusObserverMock networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(nullptr != session);
    session->IncreaseReference();
    CRTPSessionClient *client = (CRTPSessionClient *)(session);
    ASSERT_TRUE(nullptr != client);

    CRTCPHandlerSharedPtr handler = client->m_rtcpHandler;
    ASSERT_TRUE(nullptr != handler);
    handler->ConfigTMMBR(true, false);

    CRTPFBPacket packet1;
    TMMBRVECTOR items1;
    TMMBRTuple item1;
    item1.ssrc = handler->m_ssrc;
    item1.maxTBR = 3*1024*1024;
    item1.oh = 0;
    items1.push_back(item1);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items1, packet1));
    EXPECT_EQ(packet1.m_fciLength, 8);
    items1.clear();

    CCmMessageBlock mb1(packet1.CalcEncodeSize());
    CCmByteStreamNetwork os1(mb1);
    EXPECT_TRUE(!!packet1.Encode(os1, mb1));

    client->HandleRecvedRTCPPacket(mb1);

#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ((uint32_t)(2*1024*1024/8*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_UP, networkStatusObserver.GetState());
#endif

    CRTPFBPacket packet2;
    TMMBRVECTOR items2;
    TMMBRTuple item2;
    item2.ssrc = handler->m_ssrc;
    item2.maxTBR = 1*1024*1024;
    item2.oh = 0;
    items2.push_back(item2);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items2, packet2));
    EXPECT_EQ(packet2.m_fciLength, 8);
    items2.clear();

    CCmMessageBlock mb2(packet2.CalcEncodeSize());
    CCmByteStreamNetwork os2(mb2);
    EXPECT_TRUE(!!packet2.Encode(os2, mb2));

    client->HandleRecvedRTCPPacket(mb2);

#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ((uint32_t)(1*1024*1024/8*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_DOWN, networkStatusObserver.GetState());
#endif

    session->DisableQoS();
    session->DecreaseReference();
}

TEST_F(CRTPSessionClientTEST, Test_INetworkStatusObserver_API_when_QoSDisabled_and_set_initial_bandwidth)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;

    CNetworkStatusObserverMock networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(nullptr != session);
    session->IncreaseReference();
    CRTPSessionClient *client = (CRTPSessionClient *)(session);
    ASSERT_TRUE(nullptr != client);
    uint32_t initialBandwidth = (uint32_t)(float)(1.5*1024*1024/8);
    session->SetInitialBandwidth(initialBandwidth);

    CRTCPHandlerSharedPtr handler = client->m_rtcpHandler;
    ASSERT_TRUE(nullptr != handler);
    handler->ConfigTMMBR(true, false);

    CRTPFBPacket packet1;
    TMMBRVECTOR items1;
    TMMBRTuple item1;
    item1.ssrc = handler->m_ssrc;
    item1.maxTBR = 2*1024*1024;
    item1.oh = 0;
    items1.push_back(item1);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items1, packet1));
    EXPECT_EQ(packet1.m_fciLength, 8);
    items1.clear();

    CCmMessageBlock mb1(packet1.CalcEncodeSize());
    CCmByteStreamNetwork os1(mb1);
    EXPECT_TRUE(!!packet1.Encode(os1, mb1));

    client->HandleRecvedRTCPPacket(mb1);

    EXPECT_EQ((uint32_t)(initialBandwidth*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_HOLD, networkStatusObserver.GetState());

    CRTPFBPacket packet2;
    TMMBRVECTOR items2;
    TMMBRTuple item2;
    item2.ssrc = handler->m_ssrc;
    item2.maxTBR = 1*1024*1024;
    item2.oh = 0;
    items2.push_back(item2);
    EXPECT_TRUE(!!EncodeTMMBRPacket(123456, RTCP_RTPFB_TMMBR, items2, packet2));
    EXPECT_EQ(packet2.m_fciLength, 8);
    items2.clear();

    CCmMessageBlock mb2(packet2.CalcEncodeSize());
    CCmByteStreamNetwork os2(mb2);
    EXPECT_TRUE(!!packet2.Encode(os2, mb2));

    client->HandleRecvedRTCPPacket(mb2);

#ifndef ENABLE_SIMULCAST_BITRATE_CONTROL
    EXPECT_EQ((uint32_t)(1*1024*1024/8*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetRTT());
    EXPECT_EQ(0, networkStatusObserver.GetQueuingDelay());
    EXPECT_EQ(0, networkStatusObserver.GetTotalBandwidth());
    EXPECT_EQ(0, networkStatusObserver.GetLossRate());
    EXPECT_EQ(wqos::BANDWIDTH_DOWN, networkStatusObserver.GetState());
#endif

    session->DisableQoS();
    session->DecreaseReference();
}

TEST_F(CRTPSessionClientTEST, Test_INetworkStatusObserver_API_when_QoSDisabled_and_TMMBRDisable_and_not_set_initial_bandwidth)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;

    CNetworkStatusObserverMock networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(nullptr != session);
    session->IncreaseReference();
    CRTPSessionClient *client = (CRTPSessionClient *)(session);
    ASSERT_TRUE(nullptr != client);
    //uint32_t initialBandwidth = (uint32_t)(float)(1.5*1024*1024/8);
    //session->SetOption(0, WRTP_OPTION_SMOOTH_BANDWIDTH, &initialBandwidth, sizeof(initialBandwidth));

    CRTCPHandlerSharedPtr handler = client->m_rtcpHandler;
    ASSERT_TRUE(nullptr != handler);
    handler->ConfigTMMBR(false, false);

    session->DisableQoS();
    EXPECT_EQ((uint32_t)(float)(WRTP_SESSION_INITIAL_BANDWIDTH*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());

    session->DecreaseReference();
}

TEST_F(CRTPSessionClientTEST, Test_INetworkStatusObserver_API_when_QoSDisabled_and_TMMBRDisable_and_set_initial_bandwidth)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;

    CNetworkStatusObserverMock networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    IRTPSessionClient *session = WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(nullptr != session);
    session->IncreaseReference();
    CRTPSessionClient *client = (CRTPSessionClient *)(session);
    ASSERT_TRUE(nullptr != client);
    uint32_t initialBandwidth = (uint32_t)(float)(1.5*1024*1024/8);
    session->SetInitialBandwidth(initialBandwidth);

    CRTCPHandlerSharedPtr handler = client->m_rtcpHandler;
    ASSERT_TRUE(nullptr != handler);
    handler->ConfigTMMBR(false, false);

    session->DisableQoS();
    EXPECT_EQ((uint32_t)(float)(initialBandwidth*WRTP_PAYLOAD_BW_RATIO), networkStatusObserver.GetAllocatedBandwidth());

    session->DecreaseReference();
}

TEST_F(CRTPSessionClientTEST, Test_Removing_Stream_When_received_RTCP_BYE)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VOIP;
    m_rtpSessionParams.enableRTCP = true;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    EXPECT_TRUE(session != nullptr);

    session->IncreaseReference();

    CRTPSessionClient *client = (CRTPSessionClient *)(session);
    ASSERT_TRUE(nullptr != client);

    EXPECT_EQ(session->UpdateRTPExtension(GetAudioRTPHeaderExtName(RTPEXT_AudioLevel), m_audioLevelID, wrtp::STREAM_INOUT), 0);

    int32_t ret = session->RegisterPayloadType(CODEC_TYPE_AUDIO_TEST, PAYLOAD_TYPE_AUDIO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);
    ret = session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    ASSERT_TRUE(0 == ret);

    RTPSessionContextSharedPtr context = client->GetSessionContext();
    CStreamMonitorSharedPtr monitor = context->GetStreamMonitor();

    IRTPChannel *rtpRecvChannel =  session->CreateRTPChannel(m_voipChannelId, m_rtpChannelParams);
    EXPECT_TRUE(rtpRecvChannel != nullptr);
    rtpRecvChannel->IncreaseReference();

    EXPECT_EQ(rtpRecvChannel->GetChannelId(), m_voipChannelId);

    std::vector<CRTPStreamWeakPtr> streams;
    context->GetRemoteStreamMgr()->GetStreams(streams);
    EXPECT_EQ(0, streams.size());
    EXPECT_EQ(0, monitor->m_recvStreams.size());

    CVoIPDataRecvSink voipDataRecvSink;
    rtpRecvChannel->SetMediaDataRecvSink(&voipDataRecvSink);
    voipDataRecvSink.SetRTPMediaParams(m_voipChannelId, m_audioLevelID, m_mediaDataVoIP);

    CCmMessageBlock rtpPacket((DWORD)0);
    voipDataRecvSink.BuildMockRecvRTPPacket(rtpPacket);

    CRTPPacketLazy rtpPacketLazy;
    rtpPacketLazy.Bind(rtpPacket);
    uint32_t ssrc = rtpPacketLazy.GetSSRC();

    IRTPMediaPackage *mediaPackage = nullptr;
    ConvertMessageBlockToMediaPackage(rtpPacket, &mediaPackage, true);
    rtpRecvChannel->ReceiveRTPPacket(mediaPackage);
    WME_SAFE_RELEASE(mediaPackage);

    streams.clear();
    context->GetRemoteStreamMgr()->GetStreams(streams);
    EXPECT_EQ(1, streams.size());
    //EXPECT_EQ(1, monitor->m_recvStreams.size());

    CBYEPacket bye;
    bye.ssrcs.push_back(ssrc);
    bye.reason = const_cast<char *>("test bye packet");
    bye.reasonLength = strlen(bye.reason);

    CCompoundPacket packet;
    packet.AddRtcpPacket(&bye);

    uint32_t encodeSize = packet.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize);
    CCmByteStreamNetwork os(mb);
    EXPECT_EQ(packet.Encode(os, mb), TRUE);

    // clear packets
    packet.packets.clear();
    // we haven't alloc memory for BYE reason, so cleanup
    bye.reason = nullptr;
    bye.reasonLength = 0;

    mediaPackage = nullptr;
    ConvertMessageBlockToMediaPackage(mb, &mediaPackage, false);
    session->ReceiveRTCPPacket(mediaPackage);
    WME_SAFE_RELEASE(mediaPackage);

    streams.clear();
    context->GetRemoteStreamMgr()->GetStreams(streams);
    EXPECT_EQ(0, streams.size());
    //EXPECT_EQ(0, monitor->m_recvStreams.size());

    rtpRecvChannel->Close();
    rtpRecvChannel->DecreaseReference();

    session->DecreaseReference();
}

class CRTPSendingSinkForBYE : public IRTPMediaTransport
{
public:
    CRTPSendingSinkForBYE()
        : m_rtpPacket(nullptr)
        , m_rtcpPacket(nullptr)
    {

    }

    ~CRTPSendingSinkForBYE()
    {
        if (m_rtpPacket) {
            m_rtpPacket->DestroyChained();
        }

        if (m_rtcpPacket) {
            m_rtcpPacket->DestroyChained();
        }
    }

public:
    virtual WMERESULT GetTransportType(WmeTransportType &eTransportType)
    {
        eTransportType = WmeTransportType_UDP;
        return WME_S_OK;
    }

    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage)
    {
        EXPECT_TRUE(pRTPPackage != nullptr);

        if (m_rtpPacket) {
            m_rtpPacket->DestroyChained();
        }
        ConvertMediaPackageToMessageBlock(pRTPPackage, m_rtpPacket);
        pRTPPackage = nullptr;


        return WME_S_OK;
    }

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        EXPECT_TRUE(pRTCPPackage != nullptr);
        if (m_rtcpPacket) {
            m_rtcpPacket->DestroyChained();
        }
        ConvertMediaPackageToMessageBlock(pRTCPPackage, m_rtcpPacket);
        pRTCPPackage = nullptr;

        return WME_S_OK;
    }

public:
    CCmMessageBlock *m_rtpPacket;
    CCmMessageBlock *m_rtcpPacket;
};

TEST_F(CRTPSessionClientTEST, Test_Removing_SRTP_Stream_When_received_RTCP_BYE)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VOIP;
    m_rtpSessionParams.enableRTCP = true;

    IRTPSessionClient *outSession =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    EXPECT_TRUE(outSession != nullptr);
    outSession->IncreaseReference();

    // out session
    CRTPSessionClient *outClient = (CRTPSessionClient *)(outSession);
    ASSERT_TRUE(nullptr != outClient);
    CRTPSendingSinkForBYE sendingSink;
    outSession->SetMediaTransport(&sendingSink);
    EXPECT_EQ(outSession->UpdateRTPExtension(GetAudioRTPHeaderExtName(RTPEXT_AudioLevel), m_audioLevelID, wrtp::STREAM_INOUT), 0);
    int32_t ret = outSession->RegisterPayloadType(50, PAYLOAD_TYPE_AUDIO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);
    ret = outSession->RegisterPayloadType(51, PAYLOAD_TYPE_AUDIO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);
    ret = outSession->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    ASSERT_TRUE(0 == ret);

    RTPSessionContextSharedPtr outContext = outClient->GetSessionContext();
    CStreamMonitorSharedPtr outMonitor = outContext->GetStreamMonitor();

    // in session
    IRTPSessionClient *inSession =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    EXPECT_TRUE(inSession != nullptr);
    inSession->IncreaseReference();

    CRTPSessionClient *inClient = (CRTPSessionClient *)(inSession);
    ASSERT_TRUE(nullptr != inClient);
    EXPECT_EQ(inSession->UpdateRTPExtension(GetAudioRTPHeaderExtName(RTPEXT_AudioLevel), m_audioLevelID, wrtp::STREAM_INOUT), 0);
    ret = inSession->RegisterPayloadType(50, PAYLOAD_TYPE_AUDIO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);
    ret = inSession->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    ASSERT_TRUE(0 == ret);

    RTPSessionContextSharedPtr inContext = inClient->GetSessionContext();
    CStreamMonitorSharedPtr inMonitor = inContext->GetStreamMonitor();

    uint8_t key1[]                    = "1234567890abcdef" "1234567890abcdef" "1234567890abcdef";
    uint8_t key2[]                    = "fedcba0987654321" "fedcba0987654321" "fedcba0987654321";

    SecurityConfiguration conf1    = {};
    conf1.cryptoSuiteType          = CST_AES_CM_128_HMAC_SHA1_80;
    conf1.masterKeySalt            = key1;
    conf1.masterKeySaltLength      = 30;
    conf1.rtpSecurityService       = SEC_SERVICE_CONF_AUTH;
    conf1.rtcpSecurityService      = SEC_SERVICE_CONF_AUTH;
    conf1.fecOrder                 = ORDER_SRTP_FEC;
    conf1.ekt                      = nullptr;

    SecurityConfiguration conf2   = {};
    conf2.cryptoSuiteType         = CST_AES_CM_128_HMAC_SHA1_80;
    conf2.masterKeySalt           = key2;
    conf2.masterKeySaltLength     = 30;
    conf2.rtpSecurityService      = SEC_SERVICE_CONF_AUTH;
    conf2.rtcpSecurityService     = SEC_SERVICE_CONF_AUTH;
    conf2.fecOrder                = ORDER_SRTP_FEC;
    conf2.ekt                     = nullptr;

    ret = outSession->SetInboundSecurityConfiguration(conf1);
    if (ret) {
        WRTP_WARNTRACE("CRTPSessionClientTEST: Failed to set inbound security 1, " << WRTP_ERROR_CODE(ret));
    }
    ret = outSession->SetOutboundSecurityConfiguration(conf2);
    if (ret) {
        WRTP_WARNTRACE("CRTPSessionClientTEST: Failed to set outbound security 1, " << WRTP_ERROR_CODE(ret));
    }
    ret = inSession->SetInboundSecurityConfiguration(conf2);
    if (ret) {
        WRTP_WARNTRACE("CRTPSessionClientTEST: Failed to set inbound security 2, " << WRTP_ERROR_CODE(ret));
    }
    ret = inSession->SetOutboundSecurityConfiguration(conf1);
    if (ret) {
        WRTP_WARNTRACE("CRTPSessionClientTEST: Failed to set outbound security 2, " << WRTP_ERROR_CODE(ret));
    }

    IRTPChannel *outChannel =  outSession->CreateRTPChannel(m_voipChannelId, m_rtpChannelParams);
    EXPECT_TRUE(outChannel != nullptr);
    outChannel->IncreaseReference();
    EXPECT_EQ(outChannel->GetChannelId(), m_voipChannelId);

    IRTPChannel *inChannel =  inSession->CreateRTPChannel(m_voipChannelId, m_rtpChannelParams);
    EXPECT_TRUE(inChannel != nullptr);
    inChannel->IncreaseReference();
    EXPECT_EQ(inChannel->GetChannelId(), m_voipChannelId);

    std::vector<CRTPStreamWeakPtr> streams;
    inContext->GetRemoteStreamMgr()->GetStreams(streams);
    EXPECT_EQ(0, streams.size());
    //EXPECT_EQ(0, inMonitor->m_recvStreams.size());

    // 1
    outChannel->SendMediaData(&m_mediaDataVoIP);
    ASSERT_TRUE(nullptr != sendingSink.m_rtpPacket);
    CRTPPacketLazy rtpPacketLazy;
    rtpPacketLazy.Bind(*sendingSink.m_rtpPacket);
    uint32_t ssrc = rtpPacketLazy.GetSSRC();
    IRTPMediaPackage *mediaPackage = nullptr;
    ConvertMessageBlockToMediaPackage(*sendingSink.m_rtpPacket, &mediaPackage, true);
    inChannel->ReceiveRTPPacket(mediaPackage);
    WME_SAFE_RELEASE(mediaPackage);

    streams.clear();
    inContext->GetRemoteStreamMgr()->GetStreams(streams);
    EXPECT_EQ(1, streams.size());
    //EXPECT_EQ(1, inMonitor->m_recvStreams.size());

    // 2
    CBYEPacket bye;
    bye.ssrcs.push_back(ssrc);
    bye.reason = const_cast<char *>("test bye packet");
    bye.reasonLength = strlen(bye.reason);

    CCompoundPacket packet;
    packet.AddRtcpPacket(&bye);

    uint32_t encodeSize = packet.CalcEncodeSize();
    CCmMessageBlock mb(encodeSize);
    CCmByteStreamNetwork os(mb);
    EXPECT_EQ(packet.Encode(os, mb), TRUE);

    // clear packets
    packet.packets.clear();
    // we haven't alloc memory for BYE reason, so cleanup
    bye.reason = nullptr;
    bye.reasonLength = 0;

    outClient->SendRTCPPacket(mb);
    ASSERT_TRUE(nullptr != sendingSink.m_rtcpPacket);

    mediaPackage = nullptr;
    ConvertMessageBlockToMediaPackage(*sendingSink.m_rtcpPacket, &mediaPackage, false);
    inSession->ReceiveRTCPPacket(mediaPackage);
    WME_SAFE_RELEASE(mediaPackage);

    streams.clear();
    inContext->GetRemoteStreamMgr()->GetStreams(streams);
    EXPECT_EQ(0, streams.size());
    //EXPECT_EQ(0, inMonitor->m_recvStreams.size());

    // 3
    m_mediaDataVoIP.codecType = 51; // update codec type to use new ssrc
    outChannel->SendMediaData(&m_mediaDataVoIP);
    CRTPPacketLazy rtpPacketLazy2;
    rtpPacketLazy2.Bind(*sendingSink.m_rtpPacket);
    ssrc = rtpPacketLazy2.GetSSRC();
    mediaPackage = nullptr;
    ConvertMessageBlockToMediaPackage(*sendingSink.m_rtpPacket, &mediaPackage, true);
    inChannel->ReceiveRTPPacket(mediaPackage);
    WME_SAFE_RELEASE(mediaPackage);
    streams.clear();
    inContext->GetRemoteStreamMgr()->GetStreams(streams);
    EXPECT_EQ(1, streams.size());
    //EXPECT_EQ(1, inMonitor->m_recvStreams.size());

    outChannel->Close();
    outChannel->DecreaseReference();
    inChannel->Close();
    inChannel->DecreaseReference();

    outSession->DecreaseReference();
    inSession->DecreaseReference();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CRTPSessionClientMock_M1 : public CRTPSessionClientVideo
{
public:
    CRTPSessionClientMock_M1(WRTPSessionParams &param) : CRTPSessionClientVideo(param) {}
    MOCK_METHOD0(GetInboundFECEnabled, bool(void));
    MOCK_METHOD0(GetOutboundFECEnabled, bool(void));
};

RTPPacketSendInfo MakeRTPPacketSentInfo(uint32_t ssrc, uint16_t sequence, uint32_t length, uint32_t rtpTimestamp,bool bFec = false);
RTPPacketRecvInfo MakeRTPPacketRecvInfo(uint32_t ssrc, uint16_t sequence, uint32_t length, uint32_t timestamp, uint32_t mariTimestamp = -1,
                                        uint32_t timeOffset = 0, uint32_t clockRate = 0, bool bFec = false, bool bFecRecovered = false);
using ::testing::Return;
TEST_F(CRTPSessionClientTEST, GetRTCPStatsImp_FECLevel)
{
    CScopedTracer test_info;

    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;
    CNetworkStatusObserverMock networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    auto deleter = [](CRTPSessionClientMock_M1* p) { p-> DecreaseReference();};
    std::unique_ptr<CRTPSessionClientMock_M1, std::function<void(CRTPSessionClientMock_M1 *)>> session(new CRTPSessionClientMock_M1(m_rtpSessionParams), deleter);
    session->IncreaseReference();

    EXPECT_CALL(*session, GetInboundFECEnabled()).Times(4)
    .WillOnce(Return(false)).WillOnce(Return(true))
    .WillOnce(Return(false)).WillOnce(Return(true));
    EXPECT_CALL(*session, GetOutboundFECEnabled()).Times(4)
    .WillOnce(Return(false)).WillOnce(Return(true))
    .WillOnce(Return(false)).WillOnce(Return(true));

    //if there is no in/out stream, FECLevel must be disabled
    SessStats stats;
    session->GetSessionStats(stats);
    EXPECT_EQ(stats.inStats.fecStats.fecLevel,0)<<"stats.inStats.FECLevel is "<<stats.outStats.fecStats.fecLevel;
    EXPECT_EQ(stats.outStats.fecStats.fecLevel,0)<<"stats.outStats.FECLevel is "<<stats.outStats.fecStats.fecLevel;

    session->GetSessionStats(stats);
    EXPECT_EQ(stats.inStats.fecStats.fecLevel,0)<<"stats.inStats.FECLevel is "<<stats.outStats.fecStats.fecLevel;
    EXPECT_EQ(stats.outStats.fecStats.fecLevel,0)<<"stats.outStats.FECLevel is "<<stats.outStats.fecStats.fecLevel;

    //mock creating in/out stream
    session->NotifyRTPSend(MakeRTPPacketSentInfo(0x3579, 0x9531, 1024, 0x22338455), TickNowMS(), false, false);
    session->NotifyRTPRecv(MakeRTPPacketRecvInfo(0x11223344, 9527, 1022, 0x22334455), TickNowMS(), 0);

    session->GetSessionStats(stats);
    EXPECT_EQ(stats.inStats.fecStats.fecLevel,0)<<"stats.inStats.FECLevel is "<<stats.outStats.fecStats.fecLevel;
    EXPECT_EQ(stats.outStats.fecStats.fecLevel,0)<<"stats.outStats.FECLevel is "<<stats.outStats.fecStats.fecLevel;

    session->GetSessionStats(stats);
    EXPECT_GT(stats.inStats.fecStats.fecLevel,0)<<"stats.inStats.FECLevel is "<<stats.outStats.fecStats.fecLevel;
    EXPECT_GT(stats.outStats.fecStats.fecLevel,0)<<"stats.outStats.FECLevel is "<<stats.outStats.fecStats.fecLevel;
}

using ::testing::Return;
TEST_F(CRTPSessionClientTEST, GetRTCPStatsImp_FECPackages)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP  = true;
    CNetworkStatusObserverMock networkStatusObserver;
    m_rtpSessionParams.networkStatusObserver = &networkStatusObserver;

    auto deleter = [](CRTPSessionClientMock_M1* p) { p-> DecreaseReference();};
    std::unique_ptr<CRTPSessionClientMock_M1, std::function<void(CRTPSessionClientMock_M1 *)>> session(new CRTPSessionClientMock_M1(m_rtpSessionParams), deleter);
    session->IncreaseReference();

    EXPECT_CALL(*session, GetInboundFECEnabled()).Times(1)
    .WillRepeatedly(Return(true));
    EXPECT_CALL(*session, GetOutboundFECEnabled()).Times(1)
    .WillRepeatedly(Return(true));

    //mock creating in/out stream
    for (int i = 0; i < 4; i++) {
        session->NotifyRTPSend(MakeRTPPacketSentInfo(0x3579, 0x9531, 1024, 0x22338455), TickNowMS(), false, false);
    }

    for (int i = 0; i < 2; i++) {
        session->NotifyRTPSend(MakeRTPPacketSentInfo(0x2468, 0x9531, 1024, 0x22338455, true), TickNowMS(), false, false);
    }

    int nSecquence = 9527;
    for (int i = 0; i < 4; i++) {
        session->NotifyRTPRecv(MakeRTPPacketRecvInfo(0x11223344, nSecquence++ , 1022, 0x22334455, -1, 0, 0, false), TickNowMS(), 0);
    }

    for (int i = 0; i < 2; i++) {
        session->NotifyRTPRecv(MakeRTPPacketRecvInfo(0x55667788, nSecquence++ , 1022, 0x22334455, -1, 0, 0, true), TickNowMS(), 0);
    }

    SessStats stats;
    session->GetSessionStats(stats);
    EXPECT_EQ(stats.inStats.fecStats.fecRecvPackets,2);
    EXPECT_EQ(stats.inStats.fecStats.fecRecvBytes,2044);

    EXPECT_EQ(stats.inStats.netStats.recvPackets,4);
    EXPECT_EQ(stats.inStats.netStats.recvBytes,1022*4);

    EXPECT_EQ(stats.inStats.netStats.recvBitRate,1022*4*8);
    EXPECT_EQ(stats.inStats.fecStats.fecRecvBitRate,1022*2*8);

    EXPECT_EQ(stats.outStats.fecStats.fecSentPackets,2);
    EXPECT_EQ(stats.outStats.fecStats.fecSentBytes,2*1024);

    EXPECT_EQ(stats.outStats.netStats.sentPackets,4);
    EXPECT_EQ(stats.outStats.netStats.sentBytes,4*1024);

    EXPECT_EQ(stats.outStats.netStats.sentBitRate,1024*4*8);
    EXPECT_EQ(stats.outStats.fecStats.fecSendBitRate,1024*2*8);
}
