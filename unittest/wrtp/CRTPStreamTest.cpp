#include "gmock/gmock.h"
#include "gtest/gtest.h"
#define private public
#include "rtpstream.h"
#include "rtpchannel.h"
#include "rtpsessionclient.h"
#include "mmframemgr.h"
#include "videostream.h"
#include "fragmentop.h"
#include "wrtpwmeapi.h"
#include "WMEInterfaceMock.h"
#include "testutil.h"
#include "sendcontrol.h"

#include <vector>

#define MAX_PACKET_SIZE     1400
#define TEST_MEDIA_DATA_SIZE    10240

#define MAX_PAYLOAD_SIZE    1200
#define FRAME_UNIT_LEN_SMALL (640 * 1024)
#define FRAME_UNIT_LEN_LARGE (2 * FRAME_UNIT_LEN_SMALL)

#define BANDWIDTH_RATE_SMALL  (1024 * 1024)
#define BANDWIDTH_RATE_MIDDLE (2 * BANDWIDTH_RATE_SMALL)
#define BANDWIDTH_RATE_LARGE  (6 * BANDWIDTH_RATE_SMALL)

#define MAX_DELAY_SMALL   100  //  100ms
#define MAX_DELAY_LARGE   1200 //  1200ms


using namespace wrtp;
using namespace wme;
typedef std::vector<CCmMessageBlock *>   MBVECTOR;

class IRTPMediaTransportMock : public IRTPMediaTransport
{
public:
    IRTPMediaTransportMock()
    {
        m_channel = nullptr;
        m_index = 0;
        m_recvCount = 0;
        m_payloadLength = 0;
        memset(m_payloadHash, 0, 32);
    }

    ~IRTPMediaTransportMock()
    {
        for (uint32_t i = 0; i < m_mbs.size(); ++i) {
            m_mbs[i]->DestroyChained();
        }
        m_mbs.clear();
    }

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
        EXPECT_TRUE(mb != nullptr);
        OnRecvRTPPacket(pRTPPackage, mb);
        m_mbs.push_back(mb);
        EXPECT_TRUE(m_channel != nullptr);
        m_channel->ReceiveRTPPacket(pRTPPackage);
        ++m_recvCount;
        return WME_S_OK;
    }

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        return WME_S_OK;
    }

    void SetRTPChannel(CRTPChannel *channel)
    {
        m_channel = channel;
    }

    virtual uint32_t GetRecvCount()
    {
        return m_recvCount;
    }

    uint32_t GetPacketSize()
    {
        return package_size;
    }

    virtual uint32_t OnRecvRTPPacket(IWmeMediaPackage *rtpPackage, CCmMessageBlock *mb)
    {
        /*
        WMERESULT wmeRet = WME_S_OK;
        if (nullptr == rtpPackage) {
            return WRTP_ERR_NULL_POINTER;
        }
        RTPMediaFormatType formatType = (RTPMediaFormatType)-1;
        wmeRet = rtpPackage->GetFormatType(formatType);
        if (WME_FAILED(wmeRet)) {
            return WRTP_ERR_WME_MP_FAILED;
        }
        if (RTPMediaFormatRTP != formatType) {
            return WRTP_ERR_INVALID_PACKAGE;
        }

        unsigned char *pBuffer = nullptr;
        uint32_t bufferOffset = 0;
        uint32_t bufferLen = 0;
        wmeRet = rtpPackage->GetDataPointer(&pBuffer);
        if (WME_FAILED(wmeRet) || nullptr == pBuffer) {
            return WRTP_ERR_WME_MP_FAILED;
        }
        wmeRet = rtpPackage->GetDataOffset(bufferOffset);
        if (WME_FAILED(wmeRet)) {
            return WRTP_ERR_WME_MP_FAILED;
        }
        wmeRet = rtpPackage->GetDataLength(bufferLen);
        if (WME_FAILED(wmeRet) || 0 == bufferLen) {
            return WRTP_ERR_WME_MP_FAILED;
        }

        CCmMessageBlock mb(bufferLen, (LPCSTR)pBuffer + bufferOffset, 0, bufferLen);
        */

        CRTPPacketAuto packet(new CRTPPacket());
        if (!packet) {
            return WRTP_ERR_OUT_OF_MEMORY;
        }

        CmResult ret = packet->Unpack(*mb);
        if (WRTP_ERR_NOERR != ret) {
            return WRTP_ERR_INVALID_PACKAGE;
        }

        if (RTP_SESSION_WEBEX_VOIP == m_channel->GetSession()->GetSessionContext()->GetSessionType()) {
            memcpy(m_packagePayload + m_payloadLength, packet->GetPayloadData(), packet->GetPayloadLength());
            m_payloadLength += packet->GetPayloadLength();
        } else {
            //Need know what the first byte of a video package is, andy 01/28/2014
            memcpy(m_packagePayload + m_payloadLength, packet->GetPayloadData()+RTP_PAYLOAD_HEADER_LEN, packet->GetPayloadLength()-RTP_PAYLOAD_HEADER_LEN);
            m_payloadLength += packet->GetPayloadLength()-RTP_PAYLOAD_HEADER_LEN;
        }

        return WRTP_ERR_NOERR;
    }

    //add three method to caculate the hash of the payload, payload data and payload length, by andy, 01/26/2014
    virtual uint32_t GetPayloadLength()
    {
        return m_payloadLength;
    }

    virtual uint8_t *GetPayloadBuffer()
    {
        return m_packagePayload;
    }

    virtual void InitPayloadBuffer()
    {
        m_payloadLength = 0;
        memset(m_packagePayload, 0, 204800);
        m_mbs.clear();
    }

    MBVECTOR m_mbs;

    uint32_t m_payloadLength;
    uint32_t m_payloadIndex;
    uint8_t m_packagePayload[204800];
    uint8_t m_payloadHash[32];
    //add end

    CRTPChannel *m_channel;
    uint32_t m_index;
    uint32_t m_recvCount;
    uint32_t package_size;
};

class CRTPSessionClientMockForAudioStream : public CRTPSessionClientAudio
{
public:
    CRTPSessionClientMockForAudioStream(WRTPSessionParams &param)
        : CRTPSessionClientAudio(param)
    {
        RegisterPayloadType(CODEC_TYPE_AUDIO_TEST, PAYLOAD_TYPE_AUDIO_TEST, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE);
    }
};

class CRTPSessionClientMockForVideo : public CRTPSessionClientVideo
{
public:
    CRTPSessionClientMockForVideo(WRTPSessionParams &param)
        : CRTPSessionClientVideo(param)
    {
        RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
    }
};

class CRTPChannelVideoMock : public CRTPChannelVideo
{
public:
    CRTPChannelVideoMock(CRTPSessionClientMockForVideo *session, const RTPSessionContextSharedPtr &sessionConfig, uint32_t msid, WRTPChannelParams &params)
        : CRTPChannelVideo(session, sessionConfig, msid, params)
    {
        m_count = 0;
        m_data = nullptr;
        m_size = 0;
    }
    virtual ~CRTPChannelVideoMock()
    {
        if (m_data) {
            delete[] m_data;
            m_data = nullptr;
        }
        m_size = 0;
    }
    virtual void OnRecvMediaData(WRTPMediaData *md, WRTPMediaDataIntegrityInfo *mdii)
    {
        if (md->data && md->size > 0) {
            m_size = md->size;
            m_data = new uint8_t[m_size];
            memcpy(m_data, md->data, m_size);
        }
    }

    /*
    int32_t SendFrameUnit(CFrameUnit* frame) {
        NALContainer::const_iterator nalIT = frame->m_nals.begin();
        for (; nalIT != frame->m_nals.end(); ++nalIT) {
            m_count += (*nalIT)->GetFragments().size();
        }

        return CRTPChannelVideo::SendFrameUnit(frame);
    }
    */

    uint32_t GetCount()
    {
        return m_count;
    }
    uint32_t m_count;
    uint8_t *m_data;
    uint32_t m_size;
};

class CRTPChannelVoIPMock : public CRTPChannelVoIP
{
public:
    CRTPChannelVoIPMock(CRTPSessionClientMockForAudioStream *session, const RTPSessionContextSharedPtr &sessionConfig, uint32_t msid, WRTPChannelParams &params)
        : CRTPChannelVoIP(session, sessionConfig, msid, params)
    {
        m_data = nullptr;
        m_size = 0;
    }
    virtual ~CRTPChannelVoIPMock()
    {
        if (m_data) {
            delete[] m_data;
            m_data = nullptr;
        }
        m_size = 0;
    }
    virtual void OnRecvMediaData(WRTPMediaData *md, WRTPMediaDataIntegrityInfo *mdii)
    {
        if (md->data && md->size > 0) {
            m_size = md->size;
            m_data = new uint8_t[m_size];
            memcpy(m_data, md->data, m_size);
        }
    }
    uint8_t *m_data;
    uint32_t m_size;
};

class IRTPSendingSinkMockDisorder1 : public IRTPMediaTransportMock
{
public:
    virtual int32_t OnSendRTPPacket(RTP_IN CCmMessageBlock &mb)
    {
        EXPECT_TRUE(m_channel != nullptr);
        CRTPPacketAuto packet(new CRTPPacket());
        EXPECT_FALSE(!packet);
        packet->Unpack(mb);
        if (3 == m_index) {
            packet->SetSequenceNumber(packet->GetSequenceNumber()+3);
        }
        if (6 == m_index) {
            packet->SetSequenceNumber(packet->GetSequenceNumber()-3);
        }
        ++m_index;
        CCmMessageBlock mb1((DWORD)0);
        packet->Pack(mb1);
        m_channel->HandleReceivedPacket(mb1);
        ++m_recvCount;
        return 0;
    }
};

class IRTPSendingSinkMockDisorder2 : public IRTPMediaTransportMock
{
public:
    virtual int32_t OnSendRTPPacket(RTP_IN CCmMessageBlock &mb)
    {
        EXPECT_TRUE(m_channel != nullptr);
        CRTPPacketAuto packet(new CRTPPacket());
        EXPECT_FALSE(!packet);
        packet->Unpack(mb);
        if (0 == m_index) {
            packet->SetSequenceNumber(packet->GetSequenceNumber()+5);
            packet->GetPayloadData()[0] = 0;
        }
        if (5 == m_index) {
            packet->SetSequenceNumber(packet->GetSequenceNumber()-5);
            packet->GetPayloadData()[0] = 1;
        }
        ++m_index;
        CCmMessageBlock mb1((DWORD)0);
        packet->Pack(mb1);
        m_channel->HandleReceivedPacket(mb1);
        ++m_recvCount;
        return 0;
    }
};

class IRTPSendingSinkMockDisorder3 : public IRTPMediaTransportMock
{
public:
    virtual int32_t OnSendRTPPacket(RTP_IN CCmMessageBlock &mb)
    {
        EXPECT_TRUE(m_channel != nullptr);
        CRTPPacketAuto packet(new CRTPPacket());
        EXPECT_FALSE(!packet);
        packet->Unpack(mb);
        if (4 == m_index) {
            packet->SetSequenceNumber(packet->GetSequenceNumber()+3);
            packet->GetPayloadData()[0] = 2;
        }
        if (7 == m_index) {
            packet->SetSequenceNumber(packet->GetSequenceNumber()-3);
            packet->GetPayloadData()[0] = 0;
        }
        ++m_index;
        CCmMessageBlock mb1((DWORD)0);
        packet->Pack(mb1);
        m_channel->HandleReceivedPacket(mb1);
        ++m_recvCount;
        return 0;
    }
};

class CRTPStreamTEST : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }
};

TEST_F(CRTPStreamTEST, CRTPStreamSendMediaDataVOIP)
{
    CScopedTracer test_info;
    const int PAYLOAD_LENGTH = 1024;
    IRTPMediaTransportMock rtpSink;
    WRTPSessionParams param;
    param.sessionType = RTP_SESSION_WEBEX_VOIP;
    param.enableRTCP = true;
    CRTPSessionClientMockForAudioStream *sessMock = new CRTPSessionClientMockForAudioStream(param);
    sessMock->IncreaseReference();
    sessMock->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    sessMock->SetMediaTransport(&rtpSink);
    WRTPChannelParams params;
    CRTPChannelVoIPMock *chanMock = new CRTPChannelVoIPMock(sessMock, sessMock->GetSessionContext(), 123, params);
    CRTPChannelPtr chanPtr(chanMock);
    chanMock->IncreaseReference();
    CRTPStreamPtr stream(sessMock->CreateRTPStream(12345678, DEFAULT_AUDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_INOUT));
    stream->SetChannel(chanPtr);
    rtpSink.SetRTPChannel(chanMock);

    uint8_t buf[PAYLOAD_LENGTH] = {0};
    memset(buf, 0xFF, PAYLOAD_LENGTH);

    rtpSink.InitPayloadBuffer();

    WRTPMediaDataVoIP mda;
    mda.type = MEDIA_DATA_VOIP;
    mda.data = buf;
    mda.size = sizeof(buf);
    mda.timestamp = 0x88776655;
    mda.codecType = CODEC_TYPE_AUDIO_TEST;
    mda.marker = 0;
    stream->SendMediaData(&mda);

    while (rtpSink.GetRecvCount() < 1) {
        ::SleepMs(10);
    }

    uint32_t length = rtpSink.GetPayloadLength();
    int re = memcmp(buf, rtpSink.GetPayloadBuffer(), length);

    //verify receive data
    EXPECT_EQ(length, PAYLOAD_LENGTH);
    EXPECT_EQ(re, 0);

    chanMock->Close();
    chanMock->DecreaseReference();
    stream.reset();
    sessMock->DecreaseReference();
}

TEST_F(CRTPStreamTEST, CRTPStreamSendMediaDataVIDEO)
{
    CScopedTracer test_info;
    IRTPMediaTransportMock rtpSink;
    WRTPSessionParams param;
    param.sessionType = RTP_SESSION_WEBEX_VIDEO;
    param.enableRTCP = true;
    CRTPSessionClientMockForVideo *sessMock = new CRTPSessionClientMockForVideo(param);
    sessMock->IncreaseReference();
    sessMock->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    sessMock->SetMediaTransport(&rtpSink);
    sessMock->SetMaxPacketSize(MAX_PACKET_SIZE);
    CRTPPacketizationMock fragOp;
    WRTPChannelParams params;
    params.pPacketizer = &fragOp;
    CRTPChannelVideoMock *chanMock = new CRTPChannelVideoMock(sessMock, sessMock->GetSessionContext(), 123, params);
    CRTPChannelPtr chanPtr(chanMock);
    chanMock->IncreaseReference();
    CRTPStreamPtr stream(sessMock->CreateRTPStream(12345678, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_INOUT));
    stream->SetChannel(chanPtr);
    rtpSink.SetRTPChannel(chanMock);

    uint8_t buf[TEST_MEDIA_DATA_SIZE];
    for (int i = 0; i < TEST_MEDIA_DATA_SIZE; ++i) {
        buf[i] = 0xFF;
    }
    RTP_PAYLOAD_INIT(buf, TEST_MEDIA_DATA_SIZE);

    rtpSink.InitPayloadBuffer();

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
    stream->SendMediaData(&mdv);

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    uint32_t maxPayloadSize = sessMock->GetSessionContext()->GetOutboundConfig()->GetPayloadMaxAppromateSize();
    uint32_t packetCount = (mdv.size + maxPayloadSize - 1)/maxPayloadSize;
    while (rtpSink.GetRecvCount() < packetCount) {
        ::SleepMs(10);
    }

    // verify data
    uint32_t length = rtpSink.GetPayloadLength();
    int re = memcmp(buf, rtpSink.GetPayloadBuffer(), length);

    //verify receive data
    EXPECT_EQ(length, TEST_MEDIA_DATA_SIZE);
    EXPECT_EQ(re, 0);

    chanMock->Close();
    chanMock->DecreaseReference();
    stream.reset();
    sessMock->DecreaseReference();
}

TEST_F(CRTPStreamTEST, CRTPStreamRecvRTPPacketVIDEODisorder1)
{
    CScopedTracer test_info;
    IRTPSendingSinkMockDisorder1 rtpSink;
    WRTPSessionParams param;
    param.sessionType = RTP_SESSION_WEBEX_VIDEO;
    param.enableRTCP = true;
    CRTPSessionClientMockForVideo *sessMock = new CRTPSessionClientMockForVideo(param);
    sessMock->IncreaseReference();
    sessMock->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    sessMock->SetMediaTransport(&rtpSink);
    sessMock->SetMaxPacketSize(MAX_PACKET_SIZE);
    CRTPPacketizationMock fragOp;
    WRTPChannelParams params;
    params.pPacketizer = &fragOp;
    CRTPChannelVideoMock *chanMock = new CRTPChannelVideoMock(sessMock, sessMock->GetSessionContext(), 123, params);
    CRTPChannelPtr chanPtr(chanMock);
    chanMock->IncreaseReference();
    CRTPStreamPtr stream(sessMock->CreateRTPStream(12345678, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_INOUT));
    stream->SetChannel(chanPtr);
    rtpSink.SetRTPChannel(chanMock);

    uint8_t buf[TEST_MEDIA_DATA_SIZE];
    for (int i = 0; i < TEST_MEDIA_DATA_SIZE; ++i) {
        buf[i] = 0xFF;
    }
    RTP_PAYLOAD_INIT(buf, TEST_MEDIA_DATA_SIZE);

    rtpSink.InitPayloadBuffer();

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
    stream->SendMediaData(&mdv);

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    uint32_t maxPayloadSize = sessMock->GetSessionContext()->GetOutboundConfig()->GetPayloadMaxAppromateSize();
    uint32_t packetCount = (mdv.size + maxPayloadSize - 1)/maxPayloadSize;
    while (rtpSink.GetRecvCount() < packetCount) {
        ::SleepMs(10);
    }

    // verify data
    uint32_t length = rtpSink.GetPayloadLength();
    int re = memcmp(buf, rtpSink.GetPayloadBuffer(), length);

    //verify receive data
    EXPECT_EQ(length, TEST_MEDIA_DATA_SIZE);
    EXPECT_EQ(re, 0);

    chanMock->Close();
    chanMock->DecreaseReference();
    stream.reset();
    sessMock->DecreaseReference();
}

TEST_F(CRTPStreamTEST, CRTPStreamRecvRTPPacketVIDEODisorder2)
{
    CScopedTracer test_info;
    IRTPSendingSinkMockDisorder2 rtpSink;
    WRTPSessionParams param;
    param.sessionType = RTP_SESSION_WEBEX_VIDEO;
    param.enableRTCP = true;
    CRTPSessionClientMockForVideo *sessMock = new CRTPSessionClientMockForVideo(param);
    sessMock->IncreaseReference();
    sessMock->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    sessMock->SetMediaTransport(&rtpSink);
    sessMock->SetMaxPacketSize(MAX_PACKET_SIZE);
    CRTPPacketizationMock fragOp;
    WRTPChannelParams params;
    params.pPacketizer = &fragOp;
    CRTPChannelVideoMock *chanMock = new CRTPChannelVideoMock(sessMock, sessMock->GetSessionContext(), 123, params);
    CRTPChannelPtr chanPtr(chanMock);
    chanMock->IncreaseReference();
    CRTPStreamPtr stream(sessMock->CreateRTPStream(12345678, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_INOUT));
    stream->SetChannel(chanPtr);
    rtpSink.SetRTPChannel(chanMock);

    uint8_t buf[TEST_MEDIA_DATA_SIZE];
    for (int i = 0; i < TEST_MEDIA_DATA_SIZE; ++i) {
        buf[i] = 0xFF;
    }
    RTP_PAYLOAD_INIT(buf, TEST_MEDIA_DATA_SIZE);

    rtpSink.InitPayloadBuffer();

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
    stream->SendMediaData(&mdv);

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    uint32_t maxPayloadSize = sessMock->GetSessionContext()->GetOutboundConfig()->GetPayloadMaxAppromateSize();
    uint32_t packetCount = (mdv.size + maxPayloadSize - 1)/maxPayloadSize;
    while (rtpSink.GetRecvCount() < packetCount) {
        ::SleepMs(10);
    }

    // verify data
    uint32_t length = rtpSink.GetPayloadLength();
    int re = memcmp(buf, rtpSink.GetPayloadBuffer(), length);

    //verify receive data
    EXPECT_EQ(length, TEST_MEDIA_DATA_SIZE);
    EXPECT_EQ(re, 0);

    chanMock->Close();
    chanMock->DecreaseReference();
    stream.reset();
    sessMock->DecreaseReference();
}

TEST_F(CRTPStreamTEST, CRTPStreamRecvRTPPacketVIDEODisorder3)
{
    CScopedTracer test_info;
    IRTPSendingSinkMockDisorder3 rtpSink;
    WRTPSessionParams param;
    param.sessionType = RTP_SESSION_WEBEX_VIDEO;
    param.enableRTCP = true;
    CRTPSessionClientMockForVideo *sessMock = new CRTPSessionClientMockForVideo(param);
    sessMock->IncreaseReference();
    sessMock->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    sessMock->SetMediaTransport(&rtpSink);
    sessMock->SetMaxPacketSize(MAX_PACKET_SIZE);
    CRTPPacketizationMock fragOp;
    WRTPChannelParams params;
    params.pPacketizer = &fragOp;
    CRTPChannelVideoMock *chanMock = new CRTPChannelVideoMock(sessMock, sessMock->GetSessionContext(), 123, params);
    CRTPChannelPtr chanPtr(chanMock);
    chanMock->IncreaseReference();
    CRTPStreamPtr stream(sessMock->CreateRTPStream(12345678, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_INOUT));
    stream->SetChannel(chanPtr);
    rtpSink.SetRTPChannel(chanMock);

    uint8_t buf[TEST_MEDIA_DATA_SIZE];
    for (int i = 0; i < TEST_MEDIA_DATA_SIZE; ++i) {
        buf[i] = 0xFF;
    }
    RTP_PAYLOAD_INIT(buf, TEST_MEDIA_DATA_SIZE);

    rtpSink.InitPayloadBuffer();

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
    stream->SendMediaData(&mdv);

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    uint32_t maxPayloadSize = sessMock->GetSessionContext()->GetOutboundConfig()->GetPayloadMaxAppromateSize();
    uint32_t packetCount = (mdv.size + maxPayloadSize - 1)/maxPayloadSize;
    while (rtpSink.GetRecvCount() < packetCount) {
        ::SleepMs(10);
    }

    // verify data
    uint32_t length = rtpSink.GetPayloadLength();
    int re = memcmp(buf, rtpSink.GetPayloadBuffer(), length);

    //verify receive data
    EXPECT_EQ(length, TEST_MEDIA_DATA_SIZE);
    EXPECT_EQ(re, 0);

    chanMock->Close();
    chanMock->DecreaseReference();
    stream.reset();
    sessMock->DecreaseReference();
}

typedef std::vector<CRTPPacket *>   RTPPACKETVECTOR;
enum {
    TESTCASE_NONE       = 0,
    TESTCASE_NORMAL,
    TESTCASE_DISORDER,
    TESTCASE_DISORDER2,
    TESTCASE_INTERLEAVED,
};

class IRTPSendingSinkMockAll : public IRTPMediaTransportMock
{
public:
    ~IRTPSendingSinkMockAll()
    {
        for (uint32_t i = 0; i < m_mbs.size(); ++i) {
            m_mbs[i]->DestroyChained();
        }
        m_mbs.clear();
    }

    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage)
    {
        EXPECT_TRUE(pRTPPackage != nullptr);
        CCmMessageBlock *mb = nullptr;
        ConvertMediaPackageToMessageBlock(pRTPPackage, mb);
        EXPECT_TRUE(mb != nullptr);

        m_mbs.push_back(mb);
        ++m_recvCount;
        return 0;
    }
};

class CRTPVideoStreamMockAll : public CVideoStream
{
public:
    CRTPVideoStreamMockAll(CRTPSessionBase *session, uint32_t ssrc, uint32_t clockRate, wrtp::eStreamDirection dir)
        : CVideoStream(session, ssrc, clockRate, dir)
    {
    }
};

class CRTPChannelMockAll : public CRTPChannelVideo
{
public:
    CRTPChannelMockAll(CRTPSessionClientMockForVideo *session, const RTPSessionContextSharedPtr &sessionConfig, uint32_t msid, WRTPChannelParams &params)
        : CRTPChannelVideo(session, sessionConfig, msid, params)
        , m_sessionContext(sessionConfig)
    {
        m_testCase = 0;
        m_fragmentIndex = 0;
        m_count = 0;
        m_data = nullptr;
        m_size = 0;
    }
    ~CRTPChannelMockAll()
    {
        if (m_data) {
            delete[] m_data;
            m_data = nullptr;
        }
        m_size = 0;
    }
    virtual void OnRecvMediaData(WRTPMediaData *md, WRTPMediaDataIntegrityInfo *mdii)
    {
        if (TESTCASE_NORMAL == m_testCase) {
            if (1 == m_fragmentIndex) {
                EXPECT_EQ(mdii->startSequence, m_p1[0]);
                EXPECT_EQ(mdii->packetCount, m_p1.size());
            } else if (2 == m_fragmentIndex) {
                EXPECT_EQ(mdii->startSequence, m_p2[0]);
                EXPECT_EQ(mdii->packetCount, m_p2.size());
            }
        } else if (TESTCASE_DISORDER == m_testCase) {
            if (1 == m_fragmentIndex) {
                EXPECT_EQ(mdii->startSequence, m_startSeq1);
                EXPECT_EQ(mdii->packetCount, m_p1.size());
                m_fragmentIndex = 2; // prepare to receive fragment 2
            } else if (2 == m_fragmentIndex) {
                EXPECT_EQ(mdii->startSequence, m_startSeq2);
                EXPECT_EQ(mdii->packetCount, m_p2.size());
            }
        } else if (TESTCASE_DISORDER2 == m_testCase) {
            if (1 == m_fragmentIndex) {
                EXPECT_EQ(mdii->startSequence, m_startSeq1);
                EXPECT_EQ(mdii->packetCount, m_p1.size());
            } else if (2 == m_fragmentIndex) {
                EXPECT_EQ(mdii->startSequence, m_startSeq2);
                EXPECT_EQ(mdii->packetCount, m_p2.size());
                m_fragmentIndex = 1; // prepare to receive fragment 1
            }
        } else if (TESTCASE_INTERLEAVED == m_testCase) {
            if (1 == m_fragmentIndex) {
                EXPECT_EQ(mdii->startSequence, m_startSeq1);
                EXPECT_EQ(mdii->packetCount, m_p1.size());
                m_fragmentIndex = 2; // prepare to receive fragment 2
            } else if (2 == m_fragmentIndex) {
                EXPECT_EQ(mdii->startSequence, m_startSeq2);
                EXPECT_EQ(mdii->packetCount, m_p2.size());
            }
        }
        if (m_data) {
            delete[] m_data;
            m_data = nullptr;
        }
        m_size = 0;
        if (md->data && md->size > 0) {
            m_size = md->size;
            m_data = new uint8_t[m_size];
            memcpy(m_data, md->data, m_size);
        }
    }

    CRTPStream *CreateRTPStream(uint32_t ssrc, uint32_t clockRate)
    {
        return new CRTPVideoStreamMockAll(GetSession(), ssrc, clockRate, wrtp::STREAM_INOUT);
    }

    /*
    int32_t SendFrameUnit(CFrameUnit* frame) {
        NALContainer::iterator nalIT = frame->m_nals.begin();
        for (; nalIT != frame->m_nals.end(); ++nalIT) {
            m_count += (*nalIT)->GetFragments().size();
        }

    //        return CRTPChannelVideo::SendFrameUnit(frame);
    }
    */

    void CleanChannelMockData()
    {
        if (m_data) {
            delete[] m_data;
            m_data = nullptr;
        }
        m_size = 0;
    }

    uint32_t GetCount()
    {
        return m_count;
    }
    uint32_t m_count;

    uint8_t m_testCase;
    uint32_t m_fragmentIndex;
    uint16_t      m_startSeq1;
    uint16_t      m_startSeq2;
    std::vector<uint16_t>  m_p1;
    std::vector<uint16_t>  m_p2;
    uint8_t             *m_data;
    uint32_t             m_size;
    RTPSessionContextSharedPtr m_sessionContext;
};

class CRTPStreamAllTEST : public ::testing::Test
{
protected:
    CRTPStreamAllTEST()
    {
        m_sessMock = nullptr;
        m_chanMock = nullptr;
    }
    virtual void SetUp()
    {
        WRTPSessionParams param;
        param.sessionType = RTP_SESSION_WEBEX_VIDEO;
        param.enableRTCP = true;
        m_sessMock = new CRTPSessionClientMockForVideo(param);
        m_sessMock->IncreaseReference();
        m_sessMock->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
        m_sessMock->SetMediaTransport(&m_rtpSink);
        m_sessMock->SetMaxPacketSize(MAX_PACKET_SIZE);
        WRTPChannelParams params;
        params.pPacketizer = &m_fragOp;

        m_chanMock = new CRTPChannelMockAll(m_sessMock, m_sessMock->GetSessionContext(), 123, params);
        m_chanMock->SetPacketizationOperator(&m_fragOp);
        m_chanMock->IncreaseReference();
        m_chanPtr.reset(m_chanMock);
        m_stream = m_sessMock->CreateRTPStream(12345678, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_INOUT);
        m_stream->SetChannel(m_chanPtr);
        m_rtpSink.InitPayloadBuffer();
    }
    virtual void TearDown()
    {
        m_chanMock->Close();
        m_chanMock->DecreaseReference();
        m_chanMock = nullptr;
        m_stream.reset();
        m_sessMock->DecreaseReference();
        m_sessMock = nullptr;
    }

protected:
    IRTPSendingSinkMockAll m_rtpSink;
    CRTPPacketizationMock m_fragOp;
    CRTPSessionClientMockForVideo *m_sessMock;
    CRTPChannelMockAll *m_chanMock;
    CRTPChannelPtr      m_chanPtr;
    CRTPStreamPtr       m_stream;
};

TEST_F(CRTPStreamAllTEST, CRTPStreamTest_normal_case)
{
    CScopedTracer test_info;
    uint8_t buf[TEST_MEDIA_DATA_SIZE];
    //memset(buf, 'n', sizeof(buf));
    for (int j = 0; j < TEST_MEDIA_DATA_SIZE; ++j) {
        buf[j] = j;
    }

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
    m_stream->SendMediaData(&mdv);

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    uint32_t maxPayloadSize = m_sessMock->GetSessionContext()->GetOutboundConfig()->GetPayloadMaxAppromateSize();
    uint32_t packetCount = (mdv.size + maxPayloadSize - 1)/maxPayloadSize;
    while (m_rtpSink.GetRecvCount() < packetCount) {
        ::SleepMs(10);
    }

    ASSERT_FALSE(m_rtpSink.m_mbs.empty());

    uint16_t seq = 0;
    uint32_t i = 0;
    AUTOPACKETVEC p1;
    AUTOPACKETVEC p2;

    //////////////////////////////////////////////////////////////////////////
    // test case 1: normal case
    m_chanMock->m_testCase = TESTCASE_NORMAL;

    // prepare RTP packet
    uint32_t rtpTS1 = 100*90;
    uint32_t rtpTS2 = rtpTS1 + 30*90;
    for (i = 0; i <  m_rtpSink.m_mbs.size(); ++i) {
        // generate RTP packets
        CRTPPacketAuto packet(new CRTPPacket());
        packet->Unpack(*m_rtpSink.m_mbs[i]);
        packet->SetTimestamp(rtpTS1);
        p1.emplace_back(std::move(packet));
        m_chanMock->m_p1.push_back(p1[i]->GetSequenceNumber());

        packet.reset(new CRTPPacket());
        packet->Unpack(*m_rtpSink.m_mbs[i]);
        packet->SetTimestamp(rtpTS2);
        p2.emplace_back(std::move(packet));
    }
    ASSERT_TRUE(p1.back()->HasMarker());
    ASSERT_TRUE(p2.back()->HasMarker());

    seq = p1[p1.size()-1]->GetSequenceNumber();
    for (i = 0; i < p2.size(); ++i) {
        // update sequence for packets2
        p2[i]->SetSequenceNumber(seq+i+1);
        m_chanMock->m_p2.push_back(p2[i]->GetSequenceNumber());
    }
    m_chanMock->m_startSeq1 = p1[0]->GetSequenceNumber();
    m_chanMock->m_startSeq2 = p2[0]->GetSequenceNumber();

    m_chanMock->m_fragmentIndex = 1;
    for (i = 0; i < p1.size(); ++i) {
        // notify fragment 1
        RTPPacketRecvInfo info;
        info.length = p1[i]->GetUnpackLength();
        info.ssrc = p1[i]->GetSSRC();
        info.sequence = p1[i]->GetSequenceNumber();
        info.timestamp = p1[i]->GetTimestamp();
        m_stream->RecvRTPPacket(std::move(p1[i]), m_chanPtr, info.fecRecovered);
    }

    // verify data
    EXPECT_EQ(m_chanMock->m_size, TEST_MEDIA_DATA_SIZE);
    ASSERT_TRUE(m_chanMock->m_data != nullptr);
    EXPECT_EQ(0, memcmp(buf, m_chanMock->m_data, TEST_MEDIA_DATA_SIZE));

    if (m_chanMock->m_data) {
        delete[] m_chanMock->m_data;
        m_chanMock->m_data = nullptr;
        m_chanMock->m_size = 0;
    }
    m_chanMock->m_fragmentIndex = 2;
    for (i = 0; i < p2.size(); ++i) {
        // notify fragment 2
        RTPPacketRecvInfo info;
        info.length = p2[i]->GetUnpackLength();
        info.ssrc = p2[i]->GetSSRC();
        info.sequence = p2[i]->GetSequenceNumber();
        info.timestamp = p2[i]->GetTimestamp();
        m_stream->RecvRTPPacket(std::move(p2[i]), m_chanPtr, info.fecRecovered);
    }
    // verify data
    EXPECT_EQ(m_chanMock->m_size, TEST_MEDIA_DATA_SIZE);
    ASSERT_TRUE(m_chanMock->m_data != nullptr);
    EXPECT_EQ(0, memcmp(buf, m_chanMock->m_data, TEST_MEDIA_DATA_SIZE));
}

TEST_F(CRTPStreamAllTEST, CRTPStreamTest_packet_interleaved)
{
    CScopedTracer test_info;
    uint8_t buf[TEST_MEDIA_DATA_SIZE];
    //memset(buf, 'n', sizeof(buf));
    for (int j = 0; j < TEST_MEDIA_DATA_SIZE; ++j) {
        buf[j] = j;
    }

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
    m_stream->SendMediaData(&mdv);

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    uint32_t maxPayloadSize = m_sessMock->GetSessionContext()->GetOutboundConfig()->GetPayloadMaxAppromateSize();
    uint32_t packetCount = (mdv.size + maxPayloadSize - 1)/maxPayloadSize;
    while (m_rtpSink.GetRecvCount() < packetCount) {
        ::SleepMs(10);
    }

    ASSERT_FALSE(m_rtpSink.m_mbs.empty());

    uint16_t seq = 0;
    uint32_t i = 0;
    AUTOPACKETVEC p1;
    AUTOPACKETVEC p2;

    // test case 4:
    m_chanMock->CleanChannelMockData();
    m_chanMock->m_testCase = TESTCASE_INTERLEAVED;
    // prepare RTP packet
    uint32_t rtpTS1 = 100*90;
    uint32_t rtpTS2 = rtpTS1 + 30*90;
    for (i = 0; i <  m_rtpSink.m_mbs.size(); ++i) {
        // generate RTP packets
        CRTPPacketAuto packet(new CRTPPacket());
        packet->Unpack(*m_rtpSink.m_mbs[i]);
        packet->SetTimestamp(rtpTS1);
        p1.emplace_back(std::move(packet));
        m_chanMock->m_p1.push_back(p1[i]->GetSequenceNumber());

        packet.reset(new CRTPPacket());
        packet->Unpack(*m_rtpSink.m_mbs[i]);
        packet->SetTimestamp(rtpTS2);
        p2.emplace_back(std::move(packet));
    }
    ASSERT_TRUE(p1.back()->HasMarker());
    ASSERT_TRUE(p2.back()->HasMarker());

    seq = p1[p1.size()-1]->GetSequenceNumber();
    for (i = 0; i < p2.size(); ++i) {
        // update sequence for packets2
        p2[i]->SetSequenceNumber(seq+i+1);
        m_chanMock->m_p2.push_back(p2[i]->GetSequenceNumber());
    }
    m_chanMock->m_startSeq1 = p1[0]->GetSequenceNumber();
    m_chanMock->m_startSeq2 = p2[0]->GetSequenceNumber();

    m_chanMock->m_fragmentIndex = 1;

    WRTP_INFOTRACE("Array Start to Receive interleave packet");

    for (i = 0; i < p1.size(); ++i) {
        // interleaved notification
        RTPPacketRecvInfo info;
        info.length = p1[i]->GetUnpackLength();
        info.ssrc = p1[i]->GetSSRC();
        info.sequence = p1[i]->GetSequenceNumber();
        info.timestamp = p1[i]->GetTimestamp();
        m_stream->RecvRTPPacket(std::move(p1[i]), m_chanPtr, info.fecRecovered);
        if (i == 7) {
            // data 1 should be ready
            // verify data 1
            EXPECT_EQ(m_chanMock->m_size, TEST_MEDIA_DATA_SIZE);
            ASSERT_TRUE(m_chanMock->m_data != nullptr);
            EXPECT_EQ(0, memcmp(buf, m_chanMock->m_data, TEST_MEDIA_DATA_SIZE));
            if (m_chanMock->m_data) {
                delete[] m_chanMock->m_data;
                m_chanMock->m_data = nullptr;
                m_chanMock->m_size = 0;
            }
        }
        info.length = p2[i]->GetUnpackLength();
        info.ssrc = p2[i]->GetSSRC();
        info.sequence = p2[i]->GetSequenceNumber();
        info.timestamp = p2[i]->GetTimestamp();
        m_stream->RecvRTPPacket(std::move(p2[i]), m_chanPtr, info.fecRecovered);
    }
    // verify data 2
    EXPECT_EQ(m_chanMock->m_size, TEST_MEDIA_DATA_SIZE);
    ASSERT_TRUE(m_chanMock->m_data != nullptr);
    EXPECT_EQ(0, memcmp(buf, m_chanMock->m_data, TEST_MEDIA_DATA_SIZE));
}

TEST_F(CRTPStreamAllTEST, CRTPStreamTest_corner_case_recv_end_fragment)
{
    CScopedTracer test_info;
    uint8_t buf[TEST_MEDIA_DATA_SIZE];
    //memset(buf, 'n', sizeof(buf));
    for (int j = 0; j < TEST_MEDIA_DATA_SIZE; ++j) {
        buf[j] = j;
    }

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
    m_stream->SendMediaData(&mdv);

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    uint32_t maxPayloadSize = m_sessMock->GetSessionContext()->GetOutboundConfig()->GetPayloadMaxAppromateSize();
    uint32_t packetCount = (mdv.size + maxPayloadSize - 1)/maxPayloadSize;
    while (m_rtpSink.GetRecvCount() < packetCount) {
        ::SleepMs(10);
    }

    ASSERT_FALSE(m_rtpSink.m_mbs.empty());

    uint16_t seq = 0;
    uint32_t i = 0;
    AUTOPACKETVEC p1;
    AUTOPACKETVEC p2;

    //////////////////////////////////////////////////////////////////////////
    // test case 1: normal case
    m_chanMock->CleanChannelMockData();
    m_chanMock->m_testCase = TESTCASE_NORMAL;

    // prepare RTP packet
    uint32_t rtpTS1 = 100*90;
    uint32_t rtpTS2 = rtpTS1 + 30*90;
    for (i = 0; i <  m_rtpSink.m_mbs.size(); ++i) {
        // generate RTP packets
        CRTPPacketAuto packet(new CRTPPacket());
        packet->Unpack(*m_rtpSink.m_mbs[i]);
        packet->SetTimestamp(rtpTS1);
        p1.emplace_back(std::move(packet));
        m_chanMock->m_p1.push_back(p1[i]->GetSequenceNumber());

        packet.reset(new CRTPPacket());
        packet->Unpack(*m_rtpSink.m_mbs[i]);
        packet->SetTimestamp(rtpTS2);
        p2.emplace_back(std::move(packet));
    }
    ASSERT_TRUE(p1.back()->HasMarker());
    ASSERT_TRUE(p2.back()->HasMarker());

    seq = p1[p1.size()-1]->GetSequenceNumber();
    for (i = 0; i < p2.size(); ++i) {
        // update sequence for packets2
        p2[i]->SetSequenceNumber(seq+i+1);
        m_chanMock->m_p2.push_back(p2[i]->GetSequenceNumber());
    }
    m_chanMock->m_startSeq1 = p1[0]->GetSequenceNumber();
    m_chanMock->m_startSeq2 = p2[0]->GetSequenceNumber();

    m_chanMock->m_fragmentIndex = 1;
    RTPPacketRecvInfo info;
    for (i = 0; i < p1.size() - 1; ++i) {
        // notify fragment 1
        info.length = p1[i]->GetUnpackLength();
        info.ssrc = p1[i]->GetSSRC();
        info.sequence = p1[i]->GetSequenceNumber();
        info.timestamp = p1[i]->GetTimestamp();
        m_stream->RecvRTPPacket(std::move(p1[i]), m_chanPtr, info.fecRecovered);
    }
    for (i = 0; i < p2.size() - 1; ++i) {
        // notify fragment 2
        info.length = p2[i]->GetUnpackLength();
        info.ssrc = p2[i]->GetSSRC();
        info.sequence = p2[i]->GetSequenceNumber();
        info.timestamp = p2[i]->GetTimestamp();
        m_stream->RecvRTPPacket(std::move(p2[i]), m_chanPtr, info.fecRecovered);
    }
    info.length = p1[p1.size() - 1]->GetUnpackLength();
    info.ssrc = p1[p1.size() - 1]->GetSSRC();
    info.sequence = p1[p1.size() - 1]->GetSequenceNumber();
    info.timestamp = p1[p1.size() - 1]->GetTimestamp();
    m_stream->RecvRTPPacket(std::move(p1[p1.size() - 1]), m_chanPtr, info.fecRecovered);
    // verify data
    EXPECT_EQ(m_chanMock->m_size, TEST_MEDIA_DATA_SIZE);
    ASSERT_TRUE(m_chanMock->m_data != nullptr);
    EXPECT_EQ(0, memcmp(buf, m_chanMock->m_data, TEST_MEDIA_DATA_SIZE));
    if (m_chanMock->m_data) {
        delete[] m_chanMock->m_data;
        m_chanMock->m_data = nullptr;
        m_chanMock->m_size = 0;
    }

    m_chanMock->m_fragmentIndex = 2;
    info.length = p2[p2.size() - 1]->GetUnpackLength();
    info.ssrc = p2[p2.size() - 1]->GetSSRC();
    info.sequence = p2[p2.size() - 1]->GetSequenceNumber();
    info.timestamp = p2[p2.size() - 1]->GetTimestamp();
    m_stream->RecvRTPPacket(std::move(p2[p2.size() - 1]), m_chanPtr, info.fecRecovered);
    // verify data
    EXPECT_EQ(m_chanMock->m_size, TEST_MEDIA_DATA_SIZE);
    ASSERT_TRUE(m_chanMock->m_data != nullptr);
    EXPECT_EQ(0, memcmp(buf, m_chanMock->m_data, TEST_MEDIA_DATA_SIZE));
}

TEST_F(CRTPStreamAllTEST, CRTPStreamTest_corner_case_begin_end_fragment_disorder)
{
    CScopedTracer test_info;
    uint8_t buf[TEST_MEDIA_DATA_SIZE];
    //memset(buf, 'n', sizeof(buf));
    for (int j = 0; j < TEST_MEDIA_DATA_SIZE; ++j) {
        buf[j] = j;
    }

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
    m_stream->SendMediaData(&mdv);

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    uint32_t maxPayloadSize = m_sessMock->GetSessionContext()->GetOutboundConfig()->GetPayloadMaxAppromateSize();
    uint32_t packetCount = (mdv.size + maxPayloadSize - 1)/maxPayloadSize;
    while (m_rtpSink.GetRecvCount() < packetCount) {
        ::SleepMs(10);
    }

    ASSERT_FALSE(m_rtpSink.m_mbs.empty());

    uint16_t seq = 0;
    uint32_t i = 0;
    AUTOPACKETVEC p1;
    AUTOPACKETVEC p2;

    //////////////////////////////////////////////////////////////////////////
    // test case 1: normal case
    m_chanMock->CleanChannelMockData();
    m_chanMock->m_testCase = TESTCASE_NORMAL;

    // prepare RTP packet
    uint32_t rtpTS1 = 100*90;
    uint32_t rtpTS2 = rtpTS1 + 30*90;
    for (i = 0; i <  m_rtpSink.m_mbs.size(); ++i) {
        // generate RTP packets
        CRTPPacketAuto packet(new CRTPPacket());
        packet->Unpack(*m_rtpSink.m_mbs[i]);
        packet->SetTimestamp(rtpTS1);
        p1.emplace_back(std::move(packet));
        m_chanMock->m_p1.push_back(p1[i]->GetSequenceNumber());

        packet.reset(new CRTPPacket());
        packet->Unpack(*m_rtpSink.m_mbs[i]);
        packet->SetTimestamp(rtpTS2);
        p2.emplace_back(std::move(packet));
    }
    ASSERT_TRUE(p1.back()->HasMarker());
    ASSERT_TRUE(p2.back()->HasMarker());

    seq = p1[p1.size()-1]->GetSequenceNumber();
    for (i = 0; i < p2.size(); ++i) {
        // update sequence for packets2
        p2[i]->SetSequenceNumber(seq+i+1);
        m_chanMock->m_p2.push_back(p2[i]->GetSequenceNumber());
    }
    m_chanMock->m_startSeq1 = p1[0]->GetSequenceNumber();
    m_chanMock->m_startSeq2 = p2[0]->GetSequenceNumber();

    m_chanMock->m_fragmentIndex = 1;
    RTPPacketRecvInfo info;
    WRTP_INFOTRACE("Array start NAL1");
    for (i = 0; i < p1.size() - 1; ++i) {
        // notify fragment 1
        info.length = p1[i]->GetUnpackLength();
        info.ssrc = p1[i]->GetSSRC();
        info.sequence = p1[i]->GetSequenceNumber();
        info.timestamp = p1[i]->GetTimestamp();
        m_stream->RecvRTPPacket(std::move(p1[i]), m_chanPtr, info.fecRecovered);
    }
    // send begin fragment later
    WRTP_INFOTRACE("Array start NAL2");
    for (i = 1; i < p2.size() - 1; ++i) {
        // notify fragment 2
        info.length = p2[i]->GetUnpackLength();
        info.ssrc = p2[i]->GetSSRC();
        info.sequence = p2[i]->GetSequenceNumber();
        info.timestamp = p2[i]->GetTimestamp();
        m_stream->RecvRTPPacket(std::move(p2[i]), m_chanPtr, info.fecRecovered);
    }
    info.length = p1[p1.size() - 1]->GetUnpackLength();
    info.ssrc = p1[p1.size() - 1]->GetSSRC();
    info.sequence = p1[p1.size() - 1]->GetSequenceNumber();
    info.timestamp = p1[p1.size() - 1]->GetTimestamp();
    m_stream->RecvRTPPacket(std::move(p1[p1.size() - 1]), m_chanPtr, info.fecRecovered);
    // verify data
    EXPECT_EQ(m_chanMock->m_size, TEST_MEDIA_DATA_SIZE);
    ASSERT_TRUE(m_chanMock->m_data != nullptr);
    EXPECT_EQ(0, memcmp(buf, m_chanMock->m_data, TEST_MEDIA_DATA_SIZE));
    if (m_chanMock->m_data) {
        delete[] m_chanMock->m_data;
        m_chanMock->m_data = nullptr;
        m_chanMock->m_size = 0;
    }

    m_chanMock->m_fragmentIndex = 2;
    info.length = p2[p2.size() - 1]->GetUnpackLength();
    info.ssrc = p2[p2.size() - 1]->GetSSRC();
    info.sequence = p2[p2.size() - 1]->GetSequenceNumber();
    info.timestamp = p2[p2.size() - 1]->GetTimestamp();
    m_stream->RecvRTPPacket(std::move(p2[p2.size() - 1]), m_chanPtr, info.fecRecovered);
    info.length = p2[0]->GetUnpackLength();
    info.ssrc = p2[0]->GetSSRC();
    info.sequence = p2[0]->GetSequenceNumber();
    info.timestamp = p2[0]->GetTimestamp();
    m_stream->RecvRTPPacket(std::move(p2[0]), m_chanPtr, info.fecRecovered);
    // verify data
    EXPECT_EQ(m_chanMock->m_size, TEST_MEDIA_DATA_SIZE);
    ASSERT_TRUE(m_chanMock->m_data != nullptr);
    EXPECT_EQ(0, memcmp(buf, m_chanMock->m_data, TEST_MEDIA_DATA_SIZE));
}

TEST_F(CRTPStreamTEST, Test_SendMediaDataSS)
{
    CScopedTracer test_info;
    IRTPMediaTransportMock rtpSink;
    WRTPSessionParams param;
    param.sessionType = RTP_SESSION_WEBEX_VIDEO;
    param.enableRTCP = true;
    CRTPSessionClientMockForVideo *sessMock = new CRTPSessionClientMockForVideo(param);
    sessMock->IncreaseReference();
    sessMock->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
    sessMock->SetMediaTransport(&rtpSink);
    sessMock->SetMaxPacketSize(MAX_PACKET_SIZE);
    CRTPPacketizationMock fragOp;
    WRTPChannelParams params;
    params.pPacketizer = &fragOp;
    CRTPChannelVideoMock *chanMock = new CRTPChannelVideoMock(sessMock, sessMock->GetSessionContext(), 123, params);
    CRTPChannelPtr chanPtr(chanMock);
    chanMock->IncreaseReference();
    CRTPStreamPtr stream(sessMock->CreateRTPStream(12345678, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE, wrtp::STREAM_INOUT));
    stream->SetChannel(chanPtr);
    rtpSink.SetRTPChannel(chanMock);

    //{
    uint8_t buf[10240];
    memset(buf, 0xFF, sizeof(buf));
    RTP_PAYLOAD_INIT(buf, sizeof(buf));

    uint32_t maxPayloadSize = sessMock->GetSessionContext()->GetOutboundConfig()->GetPayloadMaxAppromateSize();
    uint32_t packetCount = 0;
    for (int i = 0; i < 5; ++i) {
        WRTPMediaDataVideo mdv;
        mdv.type = MEDIA_DATA_VIDEO;
        mdv.data = buf;
        mdv.size = sizeof(buf);
        mdv.timestamp = 0x88776655;
        mdv.codecType = CODEC_TYPE_VIDEO_TEST;
        mdv.marker = 4==i;
        mdv.disposable = false;
        mdv.switchable = false;
        mdv.priority = 3;
        mdv.frameType = 2;
        mdv.DID = 2;
        mdv.TID = 1;
        mdv.maxTID = 3;
        packetCount += (mdv.size + maxPayloadSize - 1)/maxPayloadSize;
        stream->SendMediaData(&mdv);
    }

    uint8_t tmp_buf[51200] = {0};
    uint8_t *datap = tmp_buf;
    for (uint32_t ii = 0; ii < 5; ++ii) {
        memcpy(datap, buf, sizeof(buf));
        datap += sizeof(buf);
    }

    WRTP_TEST_TRIGGER_ON_TIMER(100, 5);

    while (rtpSink.GetRecvCount() < packetCount) {
        ::SleepMs(10);
    }

    // verify data
    uint32_t length = rtpSink.GetPayloadLength();
    int re = memcmp(tmp_buf, rtpSink.GetPayloadBuffer(), length);

    //verify receive data
    EXPECT_EQ(length, 51200);
    EXPECT_EQ(re, 0);


    chanMock->Close();
    chanMock->DecreaseReference();
    stream.reset();
    sessMock->DecreaseReference();
}
