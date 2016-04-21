#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "rtpsessionclient.h"
#include "rtputils.h"
#include "mmframemgr.h"
#include "qosapimock.h"
#include "WMEInterfaceMock.h"
#include "wrtpwmeapi.h"
#include "rtptime.h"
#include "rtpheaderext.h"
#include "testutil.h"
#include "fragmentop.h"

#include <thread>

using namespace testing;
using namespace wrtp;
using namespace wme;


#define TEST_MEDIA_DATA_SIZE    10240

class CRTPSessionClientAPITEST : public ::testing::Test
{
public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

public:
    WRTPSessionParams m_rtpSessionParams;
    WRTPChannelParams m_rtpChannelParams;
};


class MediaEngineMockForSessionAPI : public ISendingFilterSink
{
public:
    MediaEngineMockForSessionAPI()
        : m_msid(0)
        , m_result(FILTER_RESULT_UNKNOWN)
        , m_totalLen(0)
        , m_infoData(nullptr)
        , m_infoDataLen(0)
        , m_msid2(0)
        , m_currentBufferWithBytes(0)
        , m_totalBufferWithBytes(0)
        , m_statusChange(false)
    {

    }
    virtual ~MediaEngineMockForSessionAPI()
    {
        if (m_infoData) {
            delete[] m_infoData;
            m_infoData = nullptr;
        }
    }
public:
    virtual void OnSendingFiltered(const StreamIdInfo &streamId, FilterResult result, uint32_t totalLen, uint8_t *infoData, uint32_t infoDataLen)
    {
        m_msid = streamId.trackId;
        m_result = result;
        m_totalLen = totalLen;
        m_infoDataLen = infoDataLen;

        if (m_infoData) {
            delete[] m_infoData;
            m_infoData = nullptr;
        }
        m_infoData = new uint8_t[m_infoDataLen];
        ASSERT_TRUE(nullptr != m_infoData);
        memcpy(m_infoData, infoData, infoDataLen);
    }

    virtual void OnSendingStatusChanged(const StreamIdInfo &streamId, uint32_t currentBufferWithBytes, uint32_t totalBufferWithBytes)
    {
        m_statusChange = true;
        m_msid2 = streamId.trackId;
        m_currentBufferWithBytes = currentBufferWithBytes;
        m_totalBufferWithBytes = totalBufferWithBytes;
    }

public:
    uint32_t m_msid;
    FilterResult m_result;
    uint32_t m_totalLen;
    uint8_t *m_infoData;
    uint32_t m_infoDataLen;

    uint32_t m_msid2;
    uint32_t m_currentBufferWithBytes;
    uint32_t m_totalBufferWithBytes;
    bool   m_statusChange;
};

class CRTPSendingSink : public IRTPMediaTransport
{
public:
    CRTPSendingSink()
        : m_firstRtpPacket(nullptr)
        , m_rtpPacket(nullptr)
        , m_rtpPacketSendResult(0)
        , m_rtcpPacket(nullptr)
        , m_rtpSendCount(0)
    {

    }

    ~CRTPSendingSink()
    {
        if (m_rtpPacket) {
            m_rtpPacket->DestroyChained();
        }

        if (m_firstRtpPacket) {
            m_firstRtpPacket->DestroyChained();
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

        if (nullptr == m_firstRtpPacket) {
            m_firstRtpPacket = m_rtpPacket->DuplicateChained();
        }

        ++m_rtpSendCount;

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
    CCmMessageBlock *m_firstRtpPacket;
    CCmMessageBlock *m_rtpPacket;
    int32_t m_rtpPacketSendResult;
    CCmMessageBlock *m_rtcpPacket;
    int32_t m_rtpSendCount;
};

class CRTPSendingSinkAS : public IRTPMediaTransport
{
public:
    CRTPSendingSinkAS()
    : m_done(false)
    , m_packetCount(0)
    , m_firstSequence(0)
    {}
    
public:
    virtual WMERESULT GetTransportType(WmeTransportType &eTransportType)
    {
        eTransportType = WmeTransportType_UDP;
        return WME_S_OK;
    }
    
    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage)
    {
        EXPECT_TRUE(pRTPPackage != nullptr);
        
        CCmMessageBlock* mb = nullptr;
        ConvertMediaPackageToMessageBlock(pRTPPackage, mb);
        WMERESULT ret = WME_S_OK;
        if (mb) {
            if (m_packetCount == 0) {
                auto ptr = mb->GetTopLevelReadPtr();
                m_firstSequence = (ptr[2] << 8) | ptr[3];
                ret = WME_S_FALSE;
            } else if (m_packetCount == 1) {
                auto ptr = mb->GetTopLevelReadPtr();
                uint16_t seq = (ptr[2] << 8) | ptr[3];
                EXPECT_EQ(m_firstSequence, seq);
                m_done = true;
            }
            mb->DestroyChained();
        }
        ++m_packetCount;
        
        return ret;
    }
    
    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        return WME_S_OK;
    }
    
public:
    bool        m_done;
    uint32_t    m_packetCount;
    uint16_t    m_firstSequence;
};

class MediaTransportMock : public IWmeMediaTransport
{
public:
    MediaTransportMock() {}
    virtual ~MediaTransportMock() {}

    virtual WMERESULT GetTransportType(WmeTransportType &eTransportType)
    {
        return WME_S_OK;
    }

    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage)
    {
        return WME_S_OK;
    }

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        return WME_S_OK;
    }
};

class NetworkStatusObserverMockForAPITest : public INetworkStatusObserver
{
public:
    NetworkStatusObserverMockForAPITest()
    {
        m_allocatedBandwidth = 0;
    }

    virtual void OnAdjustment(const NetworkStatus &status)
    {
        m_allocatedBandwidth = status.allocatedBandwidth;
    }

    virtual void NotifyStreamBandwidth(const StreamIdInfo &streamId, uint32_t bandwidthInBytePerSec) {}

    uint32_t m_allocatedBandwidth;
};

class CMediaSessionFilterMockForAPITest: public IMediaSessionFilter
{
public:
    virtual int32_t QueryRequirement(QUERY_TYPE eType, uint32_t &bandwidth)
    {
        return 0;
    }
};

void SendASMediaDataAndCheckResult(IRTPChannel *channel, CRTPSendingSink &sendingSink, uint8_t frameMarkingID)
{
    WRTPMediaDataVideo mediaDataVideo;
    string strData = "abcd";

    mediaDataVideo.type = MEDIA_DATA_VIDEO;
    mediaDataVideo.timestamp = CRTPTicker::Now().ToMilliseconds();
    mediaDataVideo.codecType = CODEC_TYPE_VIDEO_TEST;
    mediaDataVideo.sampleTimestamp = 0;
    mediaDataVideo.marker = true;
    mediaDataVideo.size = strData.length();
    mediaDataVideo.data = (uint8_t *)strData.c_str();

    // Video-specific parameters
    //    mediaDataVideo.layerNumber = 1;
    //    mediaDataVideo.layerIndex = 0;
    mediaDataVideo.layerWidth = 1024;
    mediaDataVideo.layerHeight = 768;
    mediaDataVideo.layerMapAll = true;

    mediaDataVideo.frame_idc = 12345;

    mediaDataVideo.priority = 1;
    mediaDataVideo.disposable = true;
    mediaDataVideo.switchable = true;
    mediaDataVideo.frameType = 0;
    mediaDataVideo.DID = 1;
    mediaDataVideo.TID = 2;
    mediaDataVideo.maxTID = 7;
    mediaDataVideo.maxDID = 2;

    EXPECT_EQ(0, channel->SendMediaData(&mediaDataVideo));
    //::SleepMs(100);
    WRTP_TEST_TRIGGER_ON_TIMER(20, 3);
    ASSERT_TRUE(sendingSink.m_rtpPacket != nullptr);

    CRTPPacketLazy rtpPacketLazy;
    rtpPacketLazy.Bind(*sendingSink.m_rtpPacket);
    EXPECT_EQ(1, rtpPacketLazy.GetCSRCCount());
    EXPECT_EQ(channel->GetChannelId(), rtpPacketLazy.GetCSRC(0));

    uint32_t payloadLength = rtpPacketLazy.GetPayloadLength();
    EXPECT_EQ(0, memcmp(mediaDataVideo.data, rtpPacketLazy.GetPayloadData(), payloadLength));
    EXPECT_TRUE(rtpPacketLazy.HasExtension());

    uint8_t *elementData = nullptr, elementLength = 0;
    CmResult ret = rtpPacketLazy.GetExtensionElement(frameMarkingID, elementData, elementLength);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);
}

TEST_F(CRTPSessionClientAPITEST, Test_Create_AS_Client_RTP_Session_and_Send_Media_Data)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_AS;
    m_rtpSessionParams.enableRTCP = false;
    CRTPPacketizationMock packetizer;
    uint32_t asChannelId = 777;
    uint8_t frameMarkingID = 3;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);

    session->IncreaseReference();

    session->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_FrameMarking), frameMarkingID, wrtp::STREAM_INOUT);
    CRTPSendingSink rtpSendingSink;
    EXPECT_EQ(session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance()), 0);
    EXPECT_EQ(session->SetMediaTransport(&rtpSendingSink), 0);

    int32_t ret = session->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
    ASSERT_TRUE(0 == ret);

    m_rtpChannelParams.pPacketizer = &packetizer;

    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(asChannelId, m_rtpChannelParams);
    ASSERT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();

    EXPECT_EQ(rtpSendChannel->GetChannelId(), asChannelId);

    EXPECT_EQ(rtpSendChannel->GetPacketizationOperator(), &packetizer);

    SendASMediaDataAndCheckResult(rtpSendChannel, rtpSendingSink, frameMarkingID);

    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();

    session->DecreaseReference();
}

TEST_F(CRTPSessionClientAPITEST, Test_RTP_Interface_For_WME_One_NAL)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP = false;
    CRTPPacketizationMock packetizer;
    uint32_t asChannelId = 777;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    session->IncreaseReference();

    MediaEngineMockForSessionAPI meMock;
    session->SetSendingFilterSink(&meMock);

    int32_t ret = session->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
    ASSERT_TRUE(0 == ret);

    m_rtpChannelParams.pPacketizer = &packetizer;
    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(asChannelId, m_rtpChannelParams);
    ASSERT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();

    EXPECT_EQ(asChannelId, rtpSendChannel->GetChannelId());

    EXPECT_EQ(&packetizer, rtpSendChannel->GetPacketizationOperator());

    uint8_t buf[TEST_MEDIA_DATA_SIZE] = {0};

    for (int i = 0; i < TEST_MEDIA_DATA_SIZE; ++i) {
        buf[i] = i;
    }

    WRTPMediaDataVideo mdv1;
    mdv1.type = MEDIA_DATA_VIDEO;
    mdv1.data = buf;
    mdv1.size = sizeof(buf);
    mdv1.timestamp = 0x88776655;
    mdv1.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv1.disposable = false;
    mdv1.switchable = false;
    mdv1.priority = 3;
    mdv1.frameType = 2;
    mdv1.DID = 2;
    mdv1.TID = 1;
    mdv1.maxTID = 3;
    mdv1.maxDID = 2;
    mdv1.frame_idc = 5;
    mdv1.marker = 1;

    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv1));

    uint32_t dataLength = sizeof(buf);
    FilterVideoDataInfo *infoData = (FilterVideoDataInfo *)meMock.m_infoData;
    ASSERT_TRUE(nullptr != infoData);

    EXPECT_EQ(mdv1.DID, infoData->DID);
    EXPECT_EQ(mdv1.frame_idc, infoData->frame_idc);
    EXPECT_EQ(mdv1.priority, infoData->priority);
    EXPECT_EQ(dataLength, infoData->dataLength);

    EXPECT_EQ(asChannelId, meMock.m_msid);
    EXPECT_EQ(FILTER_RESULT_PASSED, meMock.m_result);
    EXPECT_EQ(sizeof(FilterVideoDataInfo), meMock.m_infoDataLen);

    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();
    session->DecreaseReference();
}


TEST_F(CRTPSessionClientAPITEST, Test_RTP_Interface_For_WME_Multi_NAL)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP = false;
    CRTPPacketizationMock packetizer;
    uint32_t asChannelId = 777;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    session->IncreaseReference();

    MediaEngineMockForSessionAPI meMock;
    session->SetSendingFilterSink(&meMock);

    int32_t ret = session->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
    ASSERT_TRUE(0 == ret);

    m_rtpChannelParams.pPacketizer = &packetizer;
    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(asChannelId, m_rtpChannelParams);
    ASSERT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();

    EXPECT_EQ(asChannelId, rtpSendChannel->GetChannelId());

    EXPECT_EQ(&packetizer, rtpSendChannel->GetPacketizationOperator());
    uint8_t buf[TEST_MEDIA_DATA_SIZE] = {0};

    for (int i = 0; i < TEST_MEDIA_DATA_SIZE; ++i) {
        buf[i] = i;
    }

    WRTPMediaDataVideo mdv1;
    mdv1.type = MEDIA_DATA_VIDEO;
    mdv1.data = buf;
    mdv1.size = sizeof(buf);
    mdv1.timestamp = 0x88776655;
    mdv1.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv1.disposable = false;
    mdv1.switchable = false;
    mdv1.priority = 3;
    mdv1.frameType = 2;
    mdv1.DID = 2;
    mdv1.TID = 1;
    mdv1.maxTID = 3;
    mdv1.maxDID = 2;
    mdv1.frame_idc = 5;

    WRTPMediaDataVideo mdv2;
    mdv2.type = MEDIA_DATA_VIDEO;
    mdv2.data = buf;
    mdv2.size = sizeof(buf);
    mdv2.timestamp = 0x88776675;
    mdv2.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv2.disposable = false;
    mdv2.switchable = false;
    mdv2.priority = 3;
    mdv2.frameType = 2;
    mdv2.DID = 2;
    mdv2.TID = 1;
    mdv2.maxTID = 3;
    mdv2.maxDID = 2;
    mdv2.frame_idc = 5;
    mdv2.marker = 1;

    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv1));
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));

    uint32_t dataLength = sizeof(buf) + sizeof(buf);
    FilterVideoDataInfo *infoData = (FilterVideoDataInfo *)meMock.m_infoData;
    ASSERT_TRUE(nullptr != infoData);

    EXPECT_EQ(mdv1.DID, infoData->DID);
    EXPECT_EQ(mdv1.frame_idc, infoData->frame_idc);
    EXPECT_EQ(mdv1.priority, infoData->priority);
    EXPECT_EQ(dataLength, infoData->dataLength);

    EXPECT_EQ(asChannelId, meMock.m_msid);
    EXPECT_EQ(FILTER_RESULT_PASSED, meMock.m_result);
    EXPECT_EQ(sizeof(FilterVideoDataInfo), meMock.m_infoDataLen);

    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();
    session->DecreaseReference();
}
#if 0
//annotate by Joyce---Fix it later(time reference)
TEST_F(CRTPSessionClientAPITEST, Test_RTP_Interface_For_AS_Buffer_Status_Changed)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_AS;
    m_rtpSessionParams.enableRTCP = false;
    MockFragmentOperatorForClientAPITest fragmentOperator;
    uint32_t asChannelId = 777;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    session->IncreaseReference();

    MediaEngineMockForSessionAPI meMock;
    session->SetSendingFilterSink(&meMock);

    MediaTransportMock transportMock;
    session->SetMediaTransport(&transportMock);

    uint32_t ret = session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    ASSERT_TRUE(0 == ret);

    ret = session->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
    ASSERT_TRUE(0 == ret);

    int32_t cmRet = session->SetMaxPacketSize(1200);
    EXPECT_EQ(WRTP_ERR_NOERR, cmRet);

    m_rtpChannelParams.fecType = RTP_FEC_NONE;
    m_rtpChannelParams.pPacketizer = &packetizer;
    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(asChannelId, m_rtpChannelParams);
    ASSERT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();

    EXPECT_EQ(asChannelId, rtpSendChannel->GetChannelId());

    EXPECT_EQ(&packetizer, rtpSendChannel->GetPacketizationOperator());

    WRTPMediaDataVideo mediaDataVideo;
    string strData = "abcd";

    mediaDataVideo.type = MEDIA_DATA_VIDEO;
    mediaDataVideo.timestamp = CRTPTicker::Now().ToMilliseconds();
    mediaDataVideo.codecType = CODEC_TYPE_VIDEO_TEST;
    mediaDataVideo.sampleTimestamp = 0;
    mediaDataVideo.marker = true;
    mediaDataVideo.size = strData.length();
    mediaDataVideo.data = (uint8_t *)strData.c_str();

    // Video-specific parameters
    mediaDataVideo.layerNumber = 1;
    mediaDataVideo.layerIndex = 0;
    mediaDataVideo.layerWidth = 1024;
    mediaDataVideo.layerHeight = 768;
    mediaDataVideo.layerMapAll = true;

    mediaDataVideo.frame_idc = 12345;

    mediaDataVideo.priority = 1;
    mediaDataVideo.disposable = true;
    mediaDataVideo.switchable = true;
    mediaDataVideo.frameType = 0;
    mediaDataVideo.DID = 1;
    mediaDataVideo.TID = 2;
    mediaDataVideo.maxTID = 7;

    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mediaDataVideo));

    WRTP_TEST_WAIT_UNTIL(20, 50, (true == meMock.m_statusChange));

    EXPECT_EQ(asChannelId, meMock.m_msid2);
    EXPECT_EQ(meMock.m_totalLen, meMock.m_currentBufferWithBytes);

    const int count = 10;
    for (int i=0; i<count; ++i) { //send date and decrease buffer size
        if (meMock.m_currentBufferWithBytes == 0) {
            break;
        }
        ::SleepMs(100);
    }

    EXPECT_EQ(0, meMock.m_currentBufferWithBytes);

    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();
    session->DecreaseReference();
}
#endif

TEST_F(CRTPSessionClientAPITEST, Test_Set_Initial_Bandwidth_Invalid_Param)
{
    CScopedTracer test_info;
    NetworkStatusObserverMockForAPITest observer;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP = true;
    m_rtpSessionParams.networkStatusObserver = &observer;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    session->IncreaseReference();

    int32_t ret = session->SetInitialBandwidth(0);
    EXPECT_NE(WRTP_ERR_NOERR, ret);

    session->DecreaseReference();
}

TEST_F(CRTPSessionClientAPITEST, Test_Set_Initial_Bandwidth_And_Notify_QoS_Off)
{
    CScopedTracer test_info;
    NetworkStatusObserverMockForAPITest observer;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP = true;
    m_rtpSessionParams.networkStatusObserver = &observer;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    session->IncreaseReference();

    uint32_t initBandwidth = 1.5*1024*1024;
    int32_t ret = session->SetInitialBandwidth(initBandwidth);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);
    EXPECT_EQ((int32_t)(initBandwidth*WRTP_PAYLOAD_BW_RATIO), observer.m_allocatedBandwidth);

    session->DecreaseReference();
}

TEST_F(CRTPSessionClientAPITEST, Test_Set_Initial_Bandwidth_And_Notify_QoS_On)
{
    CScopedTracer test_info;
    NetworkStatusObserverMockForAPITest observer;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP = true;
    m_rtpSessionParams.networkStatusObserver = &observer;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    session->IncreaseReference();

    CMediaSessionFilterMockForAPITest filter;
    QoSParameter params;
    char peerId[] = "this is an id";
    params.peerID = (uint8_t *)peerId;
    params.lengthOfPeerID = sizeof(peerId);
    params.mediaSessionFilter = &filter;

    ASSERT_EQ(WRTP_ERR_NOERR, session->EnableQoS(params));

    uint32_t initBandwidth = 1.5*1024*1024;
    int32_t ret = session->SetInitialBandwidth(initBandwidth);
    EXPECT_EQ(WRTP_ERR_NOERR, ret);
    EXPECT_EQ((int32_t)(initBandwidth*WRTP_PAYLOAD_BW_RATIO), observer.m_allocatedBandwidth);

    session->DecreaseReference();
}

struct RtpPacketInfo {
    RtpPacketInfo()
        : m_rtpBuff(nullptr)
    {}

    ~RtpPacketInfo()
    {}

    CCmMessageBlock *m_rtpBuff;
    CCmSharedPtr<CRTPPacket> m_rtpPacket;
};

class CRTPSendingSinkForFec : public IRTPMediaTransport
{
public:
    CRTPSendingSinkForFec()
    {}

    ~CRTPSendingSinkForFec()
    {
        for (uint32_t i = 0; i < m_rtpPackets.size(); ++i) {
            if (nullptr != m_rtpPackets[i].m_rtpBuff) {
                m_rtpPackets[i].m_rtpBuff->DestroyChained();
                m_rtpPackets[i].m_rtpBuff = nullptr;
            }
        }
        for (uint32_t i = 0; i < m_rtcpPackets.size(); ++i) {
            if (nullptr != m_rtcpPackets[i].m_rtpBuff) {
                m_rtcpPackets[i].m_rtpBuff->DestroyChained();
                m_rtcpPackets[i].m_rtpBuff = nullptr;
            }
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

        RtpPacketInfo tmpRtpInfo;
        ConvertMediaPackageToMessageBlock(pRTPPackage, tmpRtpInfo.m_rtpBuff);
        pRTPPackage = nullptr;
        CCmSharedPtr<CRTPPacket> tmpPacket(new CRTPPacket());
        CmResult rv = tmpPacket->Unpack(*tmpRtpInfo.m_rtpBuff);
        EXPECT_EQ(WRTP_ERR_NOERR, rv);
        tmpRtpInfo.m_rtpPacket = tmpPacket;

        if (111 == tmpPacket->GetPayloadType()) {
            m_fecPackets.push_back(tmpRtpInfo);
        } else {
            m_rtpPackets.push_back(tmpRtpInfo);
        }

        return WME_S_OK;
    }

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        EXPECT_TRUE(pRTCPPackage != nullptr);

        RtpPacketInfo tmpRtpInfo;
        ConvertMediaPackageToMessageBlock(pRTCPPackage, tmpRtpInfo.m_rtpBuff);
        pRTCPPackage = nullptr;
        m_rtcpPackets.push_back(tmpRtpInfo);

        return WME_S_OK;
    }

public:
    vector<RtpPacketInfo> m_rtpPackets;
    vector<RtpPacketInfo> m_fecPackets;
    // Only use the m_rtpBuff
    vector<RtpPacketInfo> m_rtcpPackets;
};

struct WRTPMediaDataVideoInfo {
    WRTPMediaDataVideo m_mediaData;
    vector<uint8_t> m_buff;
};
class CMediaDataRecvSinkMockForFec:public IMediaDataRecvSink
{
public:
    CMediaDataRecvSinkMockForFec() {}


    virtual int32_t OnRecvMediaData(WRTPMediaData *pData, WRTPMediaDataIntegrityInfo *pIntegrityInfo)
    {
        EXPECT_TRUE(nullptr!=pData);
        EXPECT_TRUE(nullptr!=pIntegrityInfo);
        if (nullptr == pData || nullptr == pIntegrityInfo) {
            return 0;
        }
        WRTPMediaDataVideo *input = (WRTPMediaDataVideo *)pData;
        WRTPMediaDataVideoInfo tmp;
        tmp.m_mediaData = *input;
        tmp.m_buff.resize(input->size);
        tmp.m_mediaData.data = nullptr;
        memcpy(&*tmp.m_buff.begin(), input->data, input->size);
        m_mediaDatas.push_back(tmp);

        return 0;
    }

    vector<WRTPMediaDataVideoInfo> m_mediaDatas;
};

TEST_F(CRTPSessionClientAPITEST, Test_Video_Fec_Send)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP = false;
    CRTPPacketizationMock packetizer;
    // ChannelId must is 0, it stands for session level
    uint32_t channelId = 0;
    uint8_t frameMarkingID = 3;
    uint8_t timeoffsetID = 4;
    uint8_t fecPt = RTP_PAYLOAD_TYPE_MARI_FEC;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    session->IncreaseReference();
    session->SetDynamicFecScheme(wrtp::LOSS_RATIO_WITH_PRIORITY);

    // To keep the backward compatibility
    session->SetPacketizationMode(PACKETIZATION_MODE1);
    session->EnableSendingAggregationPacket(false);
    session->SetMaxPacketSize(1400);

    session->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_MARITimestamp), frameMarkingID, wrtp::STREAM_INOUT);
    session->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_TOFFSET), timeoffsetID, wrtp::STREAM_INOUT);
    CRTPSendingSinkForFec rtpSendingSink;
    session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    session->SetMediaTransport(&rtpSendingSink);

    // Register fec codecinfo
    int32_t ret = session->RegisterPayloadType(CODEC_TYPE_MARI_FEC, fecPt, DEFAULT_MARI_FEC_CAPTURE_CLOCK_RATE);
    // Enable fec
    MariFECInfo fecInfo;
    ret = session->RegisterFECInfo(fecInfo);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);
    // No rtcp, update loss ratio manually
    ((CRTPSessionClient *)session)->OnLossRatioIndication(0.5);

    MediaEngineMockForSessionAPI meMock;
    session->SetSendingFilterSink(&meMock);

    ret = session->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);

    m_rtpChannelParams.pPacketizer = &packetizer;
    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(channelId, m_rtpChannelParams);
    ASSERT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();


    uint8_t buf[TEST_MEDIA_DATA_SIZE] = {0};
    for (int i = 0; i < TEST_MEDIA_DATA_SIZE; ++i) {
        buf[i] = i;
    }

    WRTPMediaDataVideo mdv1;
    mdv1.type = MEDIA_DATA_VIDEO;
    mdv1.data = buf;
    mdv1.size = sizeof(buf);
    mdv1.timestamp = 0x88776655;
    mdv1.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv1.disposable = false;
    mdv1.switchable = false;
    mdv1.priority = 0;
    mdv1.frameType = 2;
    mdv1.DID = 2;
    mdv1.TID = 1;
    mdv1.maxTID = 3;
    mdv1.maxDID = 2;
    mdv1.frame_idc = 5;
    mdv1.marker = 1;

    WRTPMediaDataVideo mdv2;
    mdv2.type = MEDIA_DATA_VIDEO;
    mdv2.data = buf;
    mdv2.size = sizeof(buf);
    mdv2.timestamp = 0x88776675;
    mdv2.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv2.disposable = false;
    mdv2.switchable = false;
    mdv2.priority = 0;
    mdv2.frameType = 2;
    mdv2.DID = 2;
    mdv2.TID = 1;
    mdv2.maxTID = 3;
    mdv2.maxDID = 2;
    mdv2.frame_idc = 5;
    mdv2.marker = 1;

    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv1));
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));
    mdv2.timestamp = 0x88776690;
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));

    WRTP_TEST_TRIGGER_ON_TIMER(100, 12);
    mdv2.timestamp = 0x88776790;
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));
    // Must generate at least one FEC, src count is 8+8+8
    ASSERT_GT(rtpSendingSink.m_fecPackets.size(), 0);

    EXPECT_EQ(PAYLOAD_TYPE_VIDEO_TEST, rtpSendingSink.m_rtpPackets[0].m_rtpPacket->GetPayloadType());

    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();
    session->DecreaseReference();
}

TEST_F(CRTPSessionClientAPITEST, Test_Video_Fec_Send_Zero_Overhead)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP = false;
    CRTPPacketizationMock packetizer;
    // ChannelId must is 0, it stands for session level
    uint32_t channelId = 0;
    uint8_t frameMarkingID = 3;
    uint8_t timeoffsetID = 4;
    uint8_t fecPt = RTP_PAYLOAD_TYPE_MARI_FEC;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    session->IncreaseReference();
    session->SetDynamicFecScheme(wrtp::ONLY_LOSS_RATIO);
    session->SetMaxFecOverhead(0);

    // To keep the backward compatibility
    session->SetPacketizationMode(PACKETIZATION_MODE1);
    session->EnableSendingAggregationPacket(false);
    session->SetMaxPacketSize(1400);

    session->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_MARITimestamp), frameMarkingID, wrtp::STREAM_INOUT);
    session->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_TOFFSET), timeoffsetID, wrtp::STREAM_INOUT);
    CRTPSendingSinkForFec rtpSendingSink;
    session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    session->SetMediaTransport(&rtpSendingSink);

    // Register fec codecinfo
    int32_t ret = session->RegisterPayloadType(CODEC_TYPE_MARI_FEC, fecPt, DEFAULT_MARI_FEC_CAPTURE_CLOCK_RATE);
    // Enable fec
    MariFECInfo fecInfo;
    ret = session->RegisterFECInfo(fecInfo);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);
    // No rtcp, update loss ratio manually
    ((CRTPSessionClient *)session)->OnLossRatioIndication(0.5);

    MediaEngineMockForSessionAPI meMock;
    session->SetSendingFilterSink(&meMock);

    ret = session->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);

    m_rtpChannelParams.pPacketizer = &packetizer;
    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(channelId, m_rtpChannelParams);
    ASSERT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();


    uint8_t buf[TEST_MEDIA_DATA_SIZE] = {0};
    for (int i = 0; i < TEST_MEDIA_DATA_SIZE; ++i) {
        buf[i] = i;
    }

    WRTPMediaDataVideo mdv1;
    mdv1.type = MEDIA_DATA_VIDEO;
    mdv1.data = buf;
    mdv1.size = sizeof(buf);
    mdv1.timestamp = 0x88776655;
    mdv1.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv1.disposable = false;
    mdv1.switchable = false;
    mdv1.priority = 0;
    mdv1.frameType = 2;
    mdv1.DID = 2;
    mdv1.TID = 1;
    mdv1.maxTID = 3;
    mdv1.maxDID = 2;
    mdv1.frame_idc = 5;
    mdv1.marker = 1;

    WRTPMediaDataVideo mdv2;
    mdv2.type = MEDIA_DATA_VIDEO;
    mdv2.data = buf;
    mdv2.size = sizeof(buf);
    mdv2.timestamp = 0x88776675;
    mdv2.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv2.disposable = false;
    mdv2.switchable = false;
    mdv2.priority = 0;
    mdv2.frameType = 2;
    mdv2.DID = 2;
    mdv2.TID = 1;
    mdv2.maxTID = 3;
    mdv2.maxDID = 2;
    mdv2.frame_idc = 5;
    mdv2.marker = 1;

    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv1));
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));
    mdv2.timestamp = 0x88776690;
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));

    WRTP_TEST_TRIGGER_ON_TIMER(100, 12);
    mdv2.timestamp = 0x88776790;
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));
    // Must generate at least one FEC, src count is 8+8+8
    EXPECT_EQ(rtpSendingSink.m_fecPackets.size(), 0);

    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();
    session->DecreaseReference();
}

TEST_F(CRTPSessionClientAPITEST, Test_Video_Fec_Recv)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP = false;
    CRTPPacketizationMock packetizer;
    // ChannelId must is 0, it stands for session level
    uint32_t channelId = 0;
    uint8_t frameMarkingID = 3;
    uint8_t timeoffsetID = 4;
    uint8_t fecPt = RTP_PAYLOAD_TYPE_MARI_FEC;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    session->IncreaseReference();
    session->SetDynamicFecScheme(wrtp::LOSS_RATIO_WITH_PRIORITY);

    // To keep the backward compatibility
    session->SetPacketizationMode(PACKETIZATION_MODE1);
    session->EnableSendingAggregationPacket(false);
    session->SetMaxPacketSize(1400);

    session->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_FrameMarking), frameMarkingID, wrtp::STREAM_INOUT);
    session->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_TOFFSET), timeoffsetID, wrtp::STREAM_INOUT);
    CRTPSendingSinkForFec rtpSendingSink;
    session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    session->SetMediaTransport(&rtpSendingSink);

    // Register fec codecinfo
    int32_t ret = session->RegisterPayloadType(CODEC_TYPE_MARI_FEC, fecPt, DEFAULT_MARI_FEC_CAPTURE_CLOCK_RATE);
    // Enable fec
    MariFECInfo fecInfo;
    ret = session->RegisterFECInfo(fecInfo);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);
    // No rtcp, update loss ratio manually
    ((CRTPSessionClient *)session)->OnLossRatioIndication(0.5);

    MediaEngineMockForSessionAPI meMock;
    session->SetSendingFilterSink(&meMock);

    ret = session->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);

    m_rtpChannelParams.pPacketizer = &packetizer;
    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(channelId, m_rtpChannelParams);
    ASSERT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();
    CRTPPacketizationMock packetizerMock;
    rtpSendChannel->SetPacketizationOperator(&packetizerMock);

    uint8_t buf[TEST_MEDIA_DATA_SIZE] = {0};
    for (int i = 0; i < TEST_MEDIA_DATA_SIZE; ++i) {
        buf[i] = i+1;
    }

    WRTPMediaDataVideo mdv1;
    mdv1.type = MEDIA_DATA_VIDEO;
    mdv1.data = buf;
    mdv1.size = sizeof(buf);
    mdv1.timestamp = 0x88776655;
    mdv1.sampleTimestamp = 1234560;
    mdv1.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv1.disposable = false;
    mdv1.switchable = false;
    mdv1.priority = 0;
    mdv1.frameType = 2;
    mdv1.DID = 2;
    mdv1.TID = 1;
    mdv1.maxTID = 3;
    mdv1.maxDID = 2;
    mdv1.frame_idc = 5;
    mdv1.marker = 1;

    WRTPMediaDataVideo mdv2;
    mdv2.type = MEDIA_DATA_VIDEO;
    mdv2.data = buf;
    mdv2.size = sizeof(buf);
    mdv2.timestamp = 0x88776675;
    mdv2.sampleTimestamp = 1234560+90;
    mdv2.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv2.disposable = false;
    mdv2.switchable = false;
    mdv2.priority = 0;
    mdv2.frameType = 3;
    mdv2.DID = 2;
    mdv2.TID = 1;
    mdv2.maxTID = 3;
    mdv2.maxDID = 2;
    mdv2.frame_idc = 5;
    mdv2.marker = 1;

    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv1));
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));
    mdv2.timestamp = 0x88776690;
    mdv2.sampleTimestamp += 90;
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));

    WRTP_TEST_TRIGGER_ON_TIMER(100, 12);
    mdv2.timestamp = 0x88776790;
    mdv2.sampleTimestamp += 90;
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));
    // Must generate at least one FEC, src count is 8+8+8
    ASSERT_GT(rtpSendingSink.m_fecPackets.size(), 0);

    CMediaDataRecvSinkMockForFec mediaDataRecvSink;
    IRTPChannel *rtpRecvChannel =  session->CreateRTPChannel(1, m_rtpChannelParams);
    ASSERT_TRUE(rtpRecvChannel != nullptr);
    rtpRecvChannel->IncreaseReference();
    rtpRecvChannel->SetMediaDataRecvSink(&mediaDataRecvSink);
    rtpRecvChannel->SetPacketizationOperator(&packetizerMock);

    // First recv one fec, so fec manager can setup group info
    IRTPMediaPackage *rtpPackage = nullptr;
    ConvertMessageBlockToMediaPackage(*(rtpSendingSink.m_fecPackets[0].m_rtpBuff), &rtpPackage, true);
    CmResult rv = rtpRecvChannel->ReceiveRTPPacket(rtpPackage);
    WME_SAFE_RELEASE(rtpPackage);
    ASSERT_EQ(WRTP_ERR_NOERR, rv);

    // discard index 4 src packets
    for (uint32_t i = 0; i < rtpSendingSink.m_rtpPackets.size(); ++i) {
        if (4 == i) {
            continue;
        }

        IRTPMediaPackage *rtpPackage = nullptr;
        ConvertMessageBlockToMediaPackage(*(rtpSendingSink.m_rtpPackets[i].m_rtpBuff), &rtpPackage, true);
        CmResult rv = rtpRecvChannel->ReceiveRTPPacket(rtpPackage);
        WME_SAFE_RELEASE(rtpPackage);
        ASSERT_EQ(WRTP_ERR_NOERR, rv);
    }

    ASSERT_EQ(4, mediaDataRecvSink.m_mediaDatas.size());
    EXPECT_EQ(mdv1.type, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.type);
    EXPECT_EQ(mdv1.size, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.size);
    EXPECT_EQ(mdv1.codecType, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.codecType);
    EXPECT_EQ(mdv1.sampleTimestamp, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.sampleTimestamp);
    EXPECT_EQ(mdv1.frameType, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.frameType);
    EXPECT_EQ(mdv1.TID, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.TID);
    EXPECT_EQ(mdv1.marker, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.marker);
    EXPECT_EQ(0, memcmp(&*(mediaDataRecvSink.m_mediaDatas[0].m_buff.begin()), mdv1.data, mdv1.size));

    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();
    rtpRecvChannel->Close();
    rtpRecvChannel->DecreaseReference();
    session->DecreaseReference();
}

TEST_F(CRTPSessionClientAPITEST, Test_Video_Fec_Recv_Order_Fec_Srtp)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP = false;
    CRTPPacketizationMock packetizer;
    // ChannelId must is 0, it stands for session level
    uint32_t channelId = 0;
    uint8_t frameMarkingID = 3;
    uint8_t timeoffsetID = 4;
    uint8_t fecPt = RTP_PAYLOAD_TYPE_MARI_FEC;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    session->IncreaseReference();
    session->SetDynamicFecScheme(wrtp::LOSS_RATIO_WITH_PRIORITY);

    // To keep the backward compatibility
    session->SetPacketizationMode(PACKETIZATION_MODE1);
    session->EnableSendingAggregationPacket(false);
    session->SetMaxPacketSize(1400);

    session->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_FrameMarking), frameMarkingID, wrtp::STREAM_INOUT);
    session->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_TOFFSET), timeoffsetID, wrtp::STREAM_INOUT);
    CRTPSendingSinkForFec rtpSendingSink;
    session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    session->SetMediaTransport(&rtpSendingSink);

    // Register fec codecinfo
    int32_t ret = session->RegisterPayloadType(CODEC_TYPE_MARI_FEC, fecPt, DEFAULT_MARI_FEC_CAPTURE_CLOCK_RATE);
    // Enable fec
    MariFECInfo fecInfo;
    ret = session->RegisterFECInfo(fecInfo);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);
    // No rtcp, update loss ratio manually
    ((CRTPSessionClient *)session)->OnLossRatioIndication(0.5);

    MediaEngineMockForSessionAPI meMock;
    session->SetSendingFilterSink(&meMock);

    ret = session->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
    ret = session->SetSrtpFecOrder(wrtp::ORDER_FEC_SRTP);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);

    const bool toTest256Key         = false;    // if true, test CST_AES_CM_256_HMAC_SHA1_80; otherwise, test CST_AES_CM_128_HMAC_SHA1_80

    WRTP_WARNTRACE("WRTPCreateRTPSessionClient: hard-code key to test SRTP, toTest256Key = " << toTest256Key);

    uint8_t key[]                    = "1234567890abcdef" "1234567890abcdef" "1234567890abcdef";

    SecurityConfiguration inOutConf    = {};
    inOutConf.cryptoSuiteType          = toTest256Key ? CST_AES_CM_256_HMAC_SHA1_80 : CST_AES_CM_128_HMAC_SHA1_80;
    inOutConf.masterKeySalt            = key;
    inOutConf.masterKeySaltLength      = toTest256Key ? 46 : 30;
    inOutConf.rtpSecurityService       = SEC_SERVICE_CONF_AUTH;
    inOutConf.rtcpSecurityService      = SEC_SERVICE_CONF_AUTH;
    inOutConf.fecOrder                 = ORDER_SRTP_FEC;
    inOutConf.ekt                      = nullptr;

    ret = session->SetInboundSecurityConfiguration(inOutConf);
    if (ret) {
        WRTP_WARNTRACE("WRTPCreateRTPSessionClient(): Failed to set inbound security, " << WRTP_ERROR_CODE(ret));
    }
    ret = session->SetOutboundSecurityConfiguration(inOutConf);
    if (ret) {
        WRTP_WARNTRACE("WRTPCreateRTPSessionClient(): Failed to set outbound security, " << WRTP_ERROR_CODE(ret));
    }

    m_rtpChannelParams.pPacketizer = &packetizer;
    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(channelId, m_rtpChannelParams);
    ASSERT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();

    CRTPPacketizationMock packetizerMock;
    rtpSendChannel->SetPacketizationOperator(&packetizerMock);


    uint8_t buf[TEST_MEDIA_DATA_SIZE] = {0};
    for (int i = 0; i < TEST_MEDIA_DATA_SIZE; ++i) {
        buf[i] = i+1;
    }

    WRTPMediaDataVideo mdv1;
    mdv1.type = MEDIA_DATA_VIDEO;
    mdv1.data = buf;
    mdv1.size = sizeof(buf);
    mdv1.timestamp = 0x88776655;
    mdv1.sampleTimestamp = 123456;
    mdv1.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv1.disposable = false;
    mdv1.switchable = false;
    mdv1.priority = 0;
    mdv1.frameType = 2;
    mdv1.DID = 2;
    mdv1.TID = 1;
    mdv1.maxTID = 3;
    mdv1.maxDID = 2;
    mdv1.frame_idc = 5;
    mdv1.marker = 1;

    WRTPMediaDataVideo mdv2;
    mdv2.type = MEDIA_DATA_VIDEO;
    mdv2.data = buf;
    mdv2.size = sizeof(buf);
    mdv2.timestamp = 0x88776675;
    mdv2.sampleTimestamp = 123456 + 90;
    mdv2.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv2.disposable = false;
    mdv2.switchable = false;
    mdv2.priority = 0;
    mdv2.frameType = 3;
    mdv2.DID = 2;
    mdv2.TID = 1;
    mdv2.maxTID = 3;
    mdv2.maxDID = 2;
    mdv2.frame_idc = 5;
    mdv2.marker = 1;

    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv1));
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));
    mdv2.sampleTimestamp = 0x88776690;
    mdv2.sampleTimestamp += 90;
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));

    WRTP_TEST_TRIGGER_ON_TIMER(100, 12);
    mdv2.timestamp = 0x88776790;
    mdv2.sampleTimestamp += 90;
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));
    // Must generate at least one FEC, src count is 8+8+8
    ASSERT_GT(rtpSendingSink.m_fecPackets.size(), 0);

    CMediaDataRecvSinkMockForFec mediaDataRecvSink;
    IRTPChannel *rtpRecvChannel =  session->CreateRTPChannel(1, m_rtpChannelParams);
    ASSERT_TRUE(rtpRecvChannel != nullptr);
    rtpRecvChannel->IncreaseReference();
    rtpRecvChannel->SetMediaDataRecvSink(&mediaDataRecvSink);
    rtpRecvChannel->SetPacketizationOperator(&packetizerMock);

    // First recv one fec, so fec manager can setup group info
    IRTPMediaPackage *rtpPackage = nullptr;
    ConvertMessageBlockToMediaPackage(*(rtpSendingSink.m_fecPackets[0].m_rtpBuff), &rtpPackage, true);
    CmResult rv = rtpRecvChannel->ReceiveRTPPacket(rtpPackage);
    WME_SAFE_RELEASE(rtpPackage);
    ASSERT_EQ(WRTP_ERR_NOERR, rv);

    // discard index 4src packets
    for (uint32_t i = 0; i < rtpSendingSink.m_rtpPackets.size(); ++i) {
        if (4 == i) {
            continue;
        }

        IRTPMediaPackage *rtpPackage = nullptr;
        ConvertMessageBlockToMediaPackage(*(rtpSendingSink.m_rtpPackets[i].m_rtpBuff), &rtpPackage, true);
        CmResult rv = rtpRecvChannel->ReceiveRTPPacket(rtpPackage);
        WME_SAFE_RELEASE(rtpPackage);
        ASSERT_EQ(WRTP_ERR_NOERR, rv);
    }

    ASSERT_EQ(4, mediaDataRecvSink.m_mediaDatas.size());
    EXPECT_EQ(mdv1.type, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.type);
    EXPECT_EQ(mdv1.size, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.size);
    EXPECT_EQ(mdv1.codecType, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.codecType);
    EXPECT_EQ(mdv1.sampleTimestamp, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.sampleTimestamp);
    EXPECT_EQ(mdv1.frameType, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.frameType);
    EXPECT_EQ(mdv1.TID, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.TID);
    EXPECT_EQ(mdv1.marker, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.marker);
    EXPECT_EQ(0, memcmp(&*(mediaDataRecvSink.m_mediaDatas[0].m_buff.begin()), mdv1.data, mdv1.size));

    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();
    rtpRecvChannel->Close();
    rtpRecvChannel->DecreaseReference();
    session->DecreaseReference();
}

TEST_F(CRTPSessionClientAPITEST, Test_Video_Fec_Recv_Order_Srtp_Fec)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP = false;
    CRTPPacketizationMock packetizer;
    // ChannelId must is 0, it stands for session level
    uint32_t channelId = 0;
    uint8_t frameMarkingID = 3;
    uint8_t timeoffsetID = 4;
    uint8_t fecPt = RTP_PAYLOAD_TYPE_MARI_FEC;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    session->IncreaseReference();
    session->SetDynamicFecScheme(wrtp::LOSS_RATIO_WITH_PRIORITY);

    // To keep the backward compatibility
    session->SetPacketizationMode(PACKETIZATION_MODE1);
    session->EnableSendingAggregationPacket(false);
    session->SetMaxPacketSize(1400);

    session->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_FrameMarking), frameMarkingID, wrtp::STREAM_INOUT);
    session->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_TOFFSET), timeoffsetID, wrtp::STREAM_INOUT);
    CRTPSendingSinkForFec rtpSendingSink;
    session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    session->SetMediaTransport(&rtpSendingSink);

    // Register fec codecinfo
    int32_t ret = session->RegisterPayloadType(CODEC_TYPE_MARI_FEC, fecPt, DEFAULT_MARI_FEC_CAPTURE_CLOCK_RATE);
    // Enable fec
    MariFECInfo fecInfo;
    ret = session->RegisterFECInfo(fecInfo);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);
    // No rtcp, update loss ratio manually
    ((CRTPSessionClient *)session)->OnLossRatioIndication(0.5);

    MediaEngineMockForSessionAPI meMock;
    session->SetSendingFilterSink(&meMock);

    ret = session->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
    ret = session->SetSrtpFecOrder(wrtp::ORDER_SRTP_FEC);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);

    const bool toTest256Key         = false;    // if true, test CST_AES_CM_256_HMAC_SHA1_80; otherwise, test CST_AES_CM_128_HMAC_SHA1_80

    WRTP_WARNTRACE("WRTPCreateRTPSessionClient: hard-code key to test SRTP, toTest256Key = " << toTest256Key);

    uint8_t key[]                    = "1234567890abcdef" "1234567890abcdef" "1234567890abcdef";

    SecurityConfiguration inOutConf    = {};
    inOutConf.cryptoSuiteType          = toTest256Key ? CST_AES_CM_256_HMAC_SHA1_80 : CST_AES_CM_128_HMAC_SHA1_80;
    inOutConf.masterKeySalt            = key;
    inOutConf.masterKeySaltLength      = toTest256Key ? 46 : 30;
    inOutConf.rtpSecurityService       = SEC_SERVICE_CONF_AUTH;
    inOutConf.rtcpSecurityService      = SEC_SERVICE_CONF_AUTH;
    inOutConf.fecOrder                 = ORDER_SRTP_FEC;
    inOutConf.ekt                      = nullptr;

    ret = session->SetInboundSecurityConfiguration(inOutConf);
    if (ret) {
        WRTP_WARNTRACE("WRTPCreateRTPSessionClient(): Failed to set inbound security, " << WRTP_ERROR_CODE(ret));
    }
    ret = session->SetOutboundSecurityConfiguration(inOutConf);
    if (ret) {
        WRTP_WARNTRACE("WRTPCreateRTPSessionClient(): Failed to set outbound security, " << WRTP_ERROR_CODE(ret));
    }

    m_rtpChannelParams.pPacketizer = &packetizer;
    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(channelId, m_rtpChannelParams);
    ASSERT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();
    CRTPPacketizationMock packetizerMock;
    rtpSendChannel->SetPacketizationOperator(&packetizerMock);

    uint8_t buf[TEST_MEDIA_DATA_SIZE] = {0};
    for (int i = 0; i < TEST_MEDIA_DATA_SIZE; ++i) {
        buf[i] = i+1;
    }

    WRTPMediaDataVideo mdv1;
    mdv1.type = MEDIA_DATA_VIDEO;
    mdv1.data = buf;
    mdv1.size = sizeof(buf);
    mdv1.timestamp = 0x88776655;
    mdv1.sampleTimestamp = 123456;
    mdv1.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv1.disposable = false;
    mdv1.switchable = false;
    mdv1.priority = 0;
    mdv1.frameType = 2;
    mdv1.DID = 2;
    mdv1.TID = 1;
    mdv1.maxTID = 3;
    mdv1.maxDID = 2;
    mdv1.frame_idc = 5;
    mdv1.marker = 1;

    WRTPMediaDataVideo mdv2;
    mdv2.type = MEDIA_DATA_VIDEO;
    mdv2.data = buf;
    mdv2.size = sizeof(buf);
    mdv2.timestamp = 0x88776675;
    mdv2.sampleTimestamp = 123456 + 90;
    mdv2.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv2.disposable = false;
    mdv2.switchable = false;
    mdv2.priority = 0;
    mdv2.frameType = 3;
    mdv2.DID = 2;
    mdv2.TID = 1;
    mdv2.maxTID = 3;
    mdv2.maxDID = 2;
    mdv2.frame_idc = 5;
    mdv2.marker = 1;

    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv1));
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));
    mdv2.timestamp = 0x88776690;
    mdv2.sampleTimestamp += 90;
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));

    WRTP_TEST_TRIGGER_ON_TIMER(100, 12);
    mdv2.timestamp = 0x88776790;
    mdv2.sampleTimestamp += 90;
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));
    // Must generate at least one FEC, src count is 8+8+8
    ASSERT_GT(rtpSendingSink.m_fecPackets.size(), 0);

    CMediaDataRecvSinkMockForFec mediaDataRecvSink;
    IRTPChannel *rtpRecvChannel =  session->CreateRTPChannel(1, m_rtpChannelParams);
    ASSERT_TRUE(rtpRecvChannel != nullptr);
    rtpRecvChannel->IncreaseReference();
    rtpRecvChannel->SetMediaDataRecvSink(&mediaDataRecvSink);
    rtpRecvChannel->SetPacketizationOperator(&packetizerMock);

    // First recv one fec, so fec manager can setup group info
    IRTPMediaPackage *rtpPackage = nullptr;
    ConvertMessageBlockToMediaPackage(*(rtpSendingSink.m_fecPackets[0].m_rtpBuff), &rtpPackage, true);
    CmResult rv = rtpRecvChannel->ReceiveRTPPacket(rtpPackage);
    WME_SAFE_RELEASE(rtpPackage);
    ASSERT_EQ(WRTP_ERR_NOERR, rv);

    // discard index 4 src packets
    for (uint32_t i = 0; i < rtpSendingSink.m_rtpPackets.size(); ++i) {
        if (4 == i) {
            continue;
        }

        IRTPMediaPackage *rtpPackage = nullptr;
        ConvertMessageBlockToMediaPackage(*(rtpSendingSink.m_rtpPackets[i].m_rtpBuff), &rtpPackage, true);
        CmResult rv = rtpRecvChannel->ReceiveRTPPacket(rtpPackage);
        WME_SAFE_RELEASE(rtpPackage);
        ASSERT_EQ(WRTP_ERR_NOERR, rv);
    }

    ASSERT_EQ(4, mediaDataRecvSink.m_mediaDatas.size());
    EXPECT_EQ(mdv1.type, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.type);
    EXPECT_EQ(mdv1.size, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.size);
    EXPECT_EQ(mdv1.codecType, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.codecType);
    EXPECT_EQ(mdv1.sampleTimestamp, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.sampleTimestamp);
    EXPECT_EQ(mdv1.frameType, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.frameType);
    EXPECT_EQ(mdv1.TID, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.TID);
    EXPECT_EQ(mdv1.marker, mediaDataRecvSink.m_mediaDatas[0].m_mediaData.marker);
    EXPECT_EQ(0, memcmp(&*(mediaDataRecvSink.m_mediaDatas[0].m_buff.begin()), mdv1.data, mdv1.size));

    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();
    rtpRecvChannel->Close();
    rtpRecvChannel->DecreaseReference();
    session->DecreaseReference();
}

///////////////////////////////////////////////////////////////////////////////////////
// Test Channel.UpdateCSI
TEST_F(CRTPSessionClientAPITEST, Test_RTP_Interface_For_Video_UpdateCSI)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP = false;
    CRTPPacketizationMock packetizer;
    uint32_t channelId = 777;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    session->IncreaseReference();

    CRTPSendingSink rtpSendingSink;
    EXPECT_EQ(session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance()), 0);
    EXPECT_EQ(session->SetMediaTransport(&rtpSendingSink), 0);
    int32_t ret = session->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
    ASSERT_TRUE(0 == ret);

    m_rtpChannelParams.pPacketizer = &packetizer;
    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(channelId, m_rtpChannelParams);
    ASSERT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();

    EXPECT_EQ(channelId, rtpSendChannel->GetChannelId());
    EXPECT_EQ(&packetizer, rtpSendChannel->GetPacketizationOperator());

    uint8_t buf[512] = {0};
    for (int i = 0; i < sizeof(buf); ++i) {
        buf[i] = i;
    }

    WRTPMediaDataVideo mdv1;
    mdv1.type = MEDIA_DATA_VIDEO;
    mdv1.data = buf;
    mdv1.size = sizeof(buf);
    mdv1.timestamp = 0x88776655;
    mdv1.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv1.disposable = false;
    mdv1.switchable = false;
    mdv1.priority = 3;
    mdv1.frameType = 2;
    mdv1.DID = 2;
    mdv1.TID = 1;
    mdv1.maxTID = 3;
    mdv1.maxDID = 2;
    mdv1.frame_idc = 5;
    mdv1.marker = 1;

    WRTPMediaDataVideo mdv2;
    mdv2.type = MEDIA_DATA_VIDEO;
    mdv2.data = buf;
    mdv2.size = sizeof(buf);
    mdv2.timestamp = 0x88776675;
    mdv2.codecType = CODEC_TYPE_VIDEO_TEST;
    mdv2.disposable = false;
    mdv2.switchable = false;
    mdv2.priority = 3;
    mdv2.frameType = 2;
    mdv2.DID = 2;
    mdv2.TID = 1;
    mdv2.maxTID = 3;
    mdv2.maxDID = 2;
    mdv2.frame_idc = 5;
    mdv2.marker = 1;

    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv1));
    ASSERT_TRUE(rtpSendingSink.m_rtpPacket != nullptr);
    CRTPPacket pp1;
    EXPECT_EQ(WRTP_ERR_NOERR, pp1.Unpack(*rtpSendingSink.m_firstRtpPacket));
    EXPECT_TRUE(pp1.GetCSRCCount() > 0);
    EXPECT_EQ(channelId, pp1.GetCSRC(0));

    session->UpdateCSI(channelId, 888);
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));
    ASSERT_TRUE(rtpSendingSink.m_rtpPacket != nullptr);
    CRTPPacket pp2;
    EXPECT_EQ(WRTP_ERR_NOERR, pp2.Unpack(*rtpSendingSink.m_rtpPacket));
    EXPECT_TRUE(pp2.GetCSRCCount() > 0);
    EXPECT_EQ(888, pp2.GetCSRC(0));

    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();
    session->DecreaseReference();
}

TEST_F(CRTPSessionClientAPITEST, Test_RTP_Interface_For_Audio_UpdateCSI)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VOIP;
    m_rtpSessionParams.enableRTCP = false;
    uint32_t channelId = 777;

    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    session->IncreaseReference();

    CRTPSendingSink rtpSendingSink;
    EXPECT_EQ(session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance()), 0);
    EXPECT_EQ(session->SetMediaTransport(&rtpSendingSink), 0);
    int32_t ret = session->RegisterPayloadType(CODEC_TYPE_AUDIO_TEST, PAYLOAD_TYPE_AUDIO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);
    ASSERT_TRUE(0 == ret);

    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(channelId, m_rtpChannelParams);
    ASSERT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();

    EXPECT_EQ(channelId, rtpSendChannel->GetChannelId());

    uint8_t buf[512] = {0};
    for (int i = 0; i < sizeof(buf); ++i) {
        buf[i] = i;
    }

    WRTPMediaDataVoIP mdv1;
    mdv1.type = MEDIA_DATA_VOIP;
    mdv1.data = buf;
    mdv1.size = sizeof(buf);
    mdv1.timestamp = 0x88776655;
    mdv1.codecType = CODEC_TYPE_AUDIO_TEST;
    mdv1.marker = 1;

    WRTPMediaDataVoIP mdv2;
    mdv2.type = MEDIA_DATA_VOIP;
    mdv2.data = buf;
    mdv2.size = sizeof(buf);
    mdv2.timestamp = 0x88776675;
    mdv2.codecType = CODEC_TYPE_AUDIO_TEST;
    mdv2.marker = 1;

    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv1));
    ASSERT_TRUE(rtpSendingSink.m_rtpPacket != nullptr);
    CRTPPacket pp1;
    EXPECT_EQ(WRTP_ERR_NOERR, pp1.Unpack(*rtpSendingSink.m_firstRtpPacket));
    EXPECT_TRUE(pp1.GetCSRCCount() > 0);
    EXPECT_EQ(channelId, pp1.GetCSRC(0));

    session->UpdateCSI(channelId, 888);
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mdv2));
    ASSERT_TRUE(rtpSendingSink.m_rtpPacket != nullptr);
    CRTPPacket pp2;
    EXPECT_EQ(WRTP_ERR_NOERR, pp2.Unpack(*rtpSendingSink.m_rtpPacket));
    EXPECT_TRUE(pp2.GetCSRCCount() > 0);
    EXPECT_EQ(888, pp2.GetCSRC(0));

    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();
    session->DecreaseReference();
}

extern "C" CmResult do_CreateUserTaskThread(const char *name, ACmThread *&aThread, TFlag aFlag, BOOL bWithTimerQueue, TType aType);
class CRTPSessionClientAPITEST_RealThread : public ::testing::Test
{
public:
    virtual void SetUp()
    {
        ::setCreateUserTaskThread(do_CreateUserTaskThread);
    }
    
    virtual void TearDown()
    {
        ::setCreateUserTaskThread(CreateFakeThread);
    }
    
public:
    WRTPSessionParams m_rtpSessionParams;
    WRTPChannelParams m_rtpChannelParams;
};

class CRTPSendingSinkForMTSending : public IRTPMediaTransport
{
public:
    CRTPSendingSinkForMTSending()
    {
        for (auto& c : packetCounts) {
            c = 0;
        }
    }
    
    ~CRTPSendingSinkForMTSending()
    {
        
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
        
        CCmMessageBlock *mb = nullptr;
        ConvertMediaPackageToMessageBlock(pRTPPackage, mb);
        
        CRTPPacketLazy rtpPacketLazy;
        rtpPacketLazy.Bind(*mb);
        EXPECT_EQ(1, rtpPacketLazy.GetCSRCCount());

        EXPECT_TRUE(rtpPacketLazy.HasExtension());
        uint8_t *elementData = nullptr, elementLength = 0;
        CmResult ret = rtpPacketLazy.GetExtensionElement(2, elementData, elementLength);
        EXPECT_EQ(WRTP_ERR_NOERR, ret);
        EXPECT_EQ(1, elementLength);
        EXPECT_TRUE(elementData[0] < 3);
        ++packetCounts[elementData[0]];
        mb->DestroyChained();
        
        return WME_S_OK;
    }
    
    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        return WME_S_OK;
    }
    
public:
    uint32_t packetCounts[3];
};

void SendTwoSecondsMediaData(IRTPChannel *channel, CRTPSendingSinkForMTSending &sendingSink, uint8_t frameMarkingID, uint8_t DID, uint8_t* buf, uint32_t len)
{
    uint32_t timestamp = 0x88776655;
    uint8_t vidArray[1];
    vidArray[0] = DID;
    for (int i=0; i < 100; ++i) {
        timestamp += 1800;
        WRTPMediaDataVideo mdv1;
        mdv1.type = MEDIA_DATA_VIDEO;
        mdv1.data = (uint8_t*)buf;
        mdv1.size = len > 1024 ? 1024 : len;
        mdv1.timestamp = timestamp;
        mdv1.codecType = CODEC_TYPE_VIDEO_TEST;
        mdv1.disposable = false;
        mdv1.switchable = false;
        mdv1.priority = 3;
        mdv1.frameType = 2;
        mdv1.DID = DID;
        mdv1.TID = 1;
        mdv1.maxTID = 3;
        mdv1.maxDID = 3;
        mdv1.frame_idc = 5;
        mdv1.marker = 0;
        mdv1.vidArray = vidArray;
        mdv1.vidCount = 1;
        
        WRTPMediaDataVideo mdv2;
        mdv2.type = MEDIA_DATA_VIDEO;
        mdv2.data = (uint8_t*)buf;
        mdv2.size = len;
        mdv2.timestamp = timestamp;
        mdv2.codecType = CODEC_TYPE_VIDEO_TEST;
        mdv2.disposable = false;
        mdv2.switchable = false;
        mdv2.priority = 3;
        mdv2.frameType = 2;
        mdv2.DID = DID;
        mdv2.TID = 1;
        mdv2.maxTID = 3;
        mdv2.maxDID = 3;
        mdv2.frame_idc = 5;
        mdv2.marker = 0;
        mdv2.vidArray = vidArray;
        mdv2.vidCount = 1;
        
        WRTPMediaDataVideo mdv3;
        mdv3.type = MEDIA_DATA_VIDEO;
        mdv3.data = (uint8_t*)buf;
        mdv3.size = len;
        mdv3.timestamp = timestamp;
        mdv3.codecType = CODEC_TYPE_VIDEO_TEST;
        mdv3.disposable = false;
        mdv3.switchable = false;
        mdv3.priority = 3;
        mdv3.frameType = 2;
        mdv3.DID = DID;
        mdv3.TID = 1;
        mdv3.maxTID = 3;
        mdv3.maxDID = 3;
        mdv3.frame_idc = 5;
        mdv3.marker = 1;
        mdv3.vidArray = vidArray;
        mdv3.vidCount = 1;
        
        EXPECT_EQ(0, channel->SendMediaData(&mdv1));
        EXPECT_EQ(0, channel->SendMediaData(&mdv2));
        EXPECT_EQ(0, channel->SendMediaData(&mdv3));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    uint32_t totalPackets = 100*5;
    uint32_t waitCount = 500;
    bool got_it = false;
    while (waitCount-- > 0) {
        if (sendingSink.packetCounts[DID] >= totalPackets) {
            got_it = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(got_it);
    EXPECT_EQ(totalPackets, sendingSink.packetCounts[DID]);
}

void SendMediaDataInMultiThreads(IRTPChannel *channel, CRTPSendingSinkForMTSending &sendingSink, uint8_t frameMarkingID)
{
    uint8_t buf[1800] = {0};
    for (int i = 0; i < sizeof(buf); ++i) {
        buf[i] = i;
    }
    uint32_t buf_len = sizeof(buf);
    std::thread tr1 = std::thread([channel, frameMarkingID, buf, buf_len, &sendingSink]{
        SendTwoSecondsMediaData(channel, sendingSink, frameMarkingID, 0, (uint8_t*)buf, buf_len);
    });
    std::thread tr2 = std::thread([channel, frameMarkingID, buf, buf_len, &sendingSink]{
        SendTwoSecondsMediaData(channel, sendingSink, frameMarkingID, 1, (uint8_t*)buf, buf_len);
    });
    std::thread tr3 = std::thread([channel, frameMarkingID, buf, buf_len, &sendingSink]{
        SendTwoSecondsMediaData(channel, sendingSink, frameMarkingID, 2, (uint8_t*)buf, buf_len);
    });
    
    if (tr1.joinable()) {
        tr1.join();
    }
    if (tr2.joinable()) {
        tr2.join();
    }
    if (tr3.joinable()) {
        tr3.join();
    }
}

TEST_F(CRTPSessionClientAPITEST_RealThread, Test_RTP_Interface_For_WME_Send_Media_Data_in_Multi_Threads)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    m_rtpSessionParams.enableRTCP = true;
    CRTPPacketizationMock packetizer;
    uint32_t channelId = 777;
    uint8_t frameMarkingID = 3;
    uint8_t virtualID = 2;
    
    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    
    session->IncreaseReference();
    session->EnableCiscoSCR(true);
    session->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_FrameMarking), frameMarkingID, wrtp::STREAM_INOUT);
    session->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_VID), virtualID, wrtp::STREAM_INOUT);
    CRTPSendingSinkForMTSending rtpSendingSink;
    EXPECT_EQ(session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance()), 0);
    EXPECT_EQ(session->SetMediaTransport(&rtpSendingSink), 0);
    session->SetPacketizationMode(PACKETIZATION_MODE1);
    
    int32_t ret = session->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
    ASSERT_TRUE(0 == ret);
    
    m_rtpChannelParams.pPacketizer = &packetizer;
    
    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(channelId, m_rtpChannelParams);
    ASSERT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();
    
    EXPECT_EQ(rtpSendChannel->GetChannelId(), channelId);
    EXPECT_EQ(rtpSendChannel->GetPacketizationOperator(), &packetizer);
    
    SendMediaDataInMultiThreads(rtpSendChannel, rtpSendingSink, frameMarkingID);
    
    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();
    
    session->DecreaseReference();
}

TEST_F(CRTPSessionClientAPITEST_RealThread, Test_AS_Send_Failure_and_Resend)
{
    CScopedTracer test_info;
    m_rtpSessionParams.sessionType = RTP_SESSION_WEBEX_AS;
    m_rtpSessionParams.enableRTCP = false;
    CRTPPacketizationMock packetizer;
    uint32_t asChannelId = 777;
    uint8_t frameMarkingID = 3;
    
    IRTPSessionClient *session =  WRTPCreateRTPSessionClient(m_rtpSessionParams);
    ASSERT_TRUE(session != nullptr);
    
    session->IncreaseReference();
    
    session->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_FrameMarking), frameMarkingID, wrtp::STREAM_INOUT);
    CRTPSendingSinkAS rtpSendingSink;
    EXPECT_EQ(session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance()), 0);
    EXPECT_EQ(session->SetMediaTransport(&rtpSendingSink), 0);
    
    int32_t ret = session->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
    ASSERT_TRUE(0 == ret);
    
    m_rtpChannelParams.pPacketizer = &packetizer;
    
    IRTPChannel *rtpSendChannel =  session->CreateRTPChannel(asChannelId, m_rtpChannelParams);
    ASSERT_TRUE(rtpSendChannel != nullptr);
    rtpSendChannel->IncreaseReference();
    
    EXPECT_EQ(rtpSendChannel->GetChannelId(), asChannelId);
    EXPECT_EQ(rtpSendChannel->GetPacketizationOperator(), &packetizer);
    
    WRTPMediaDataVideo mediaDataVideo;
    string strData = "abcd";
    
    mediaDataVideo.type = MEDIA_DATA_VIDEO;
    mediaDataVideo.timestamp = CRTPTicker::Now().ToMilliseconds();
    mediaDataVideo.codecType = CODEC_TYPE_VIDEO_TEST;
    mediaDataVideo.sampleTimestamp = 0;
    mediaDataVideo.marker = true;
    mediaDataVideo.size = strData.length();
    mediaDataVideo.data = (uint8_t *)strData.c_str();
    
    // Video-specific parameters
    //    mediaDataVideo.layerNumber = 1;
    //    mediaDataVideo.layerIndex = 0;
    mediaDataVideo.layerWidth = 1024;
    mediaDataVideo.layerHeight = 768;
    mediaDataVideo.layerMapAll = true;
    
    mediaDataVideo.frame_idc = 12345;
    
    mediaDataVideo.priority = 1;
    mediaDataVideo.disposable = true;
    mediaDataVideo.switchable = true;
    mediaDataVideo.frameType = 0;
    mediaDataVideo.DID = 1;
    mediaDataVideo.TID = 2;
    mediaDataVideo.maxTID = 7;
    mediaDataVideo.maxDID = 2;
    
    EXPECT_EQ(0, rtpSendChannel->SendMediaData(&mediaDataVideo));
    
    uint32_t waitCount = 500;
    bool got_it = false;
    while (waitCount-- > 0) {
        if (rtpSendingSink.m_done) {
            got_it = true;
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    EXPECT_TRUE(got_it);
    
    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();
    
    session->DecreaseReference();
}
