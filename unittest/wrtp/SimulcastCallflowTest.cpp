//
//  SimulcastCallflowTest.cpp
//  wrtpTest_mac
//
//  Created by hankpeng on 1/7/14.
//  Copyright (c) 2014 cisco. All rights reserved.
//

#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "CmThreadInterface.h"
#include "CmThreadManager.h"
#include "wrtpapi.h"
#include "rtputils.h"
#include "wrtpwmeapi.h"
#include "WMEInterfaceMock.h"
#include "rtpsessionconfig.h"
#include "testutil.h"
#include "rtcppacket.h"
#include "rtpsessionclient.h"

#include "CmThread.h"


#include <string>
#include <list>
#include <utility>
#include <vector>

using namespace wrtp;
using namespace wme;
using namespace std;

enum {
    RTCP_SCR = 0,
    RTCP_SCA,
    RTCP_SCA_ACK,
    RTCP_OTHERS,
    RTCP_ALL,
    RTCP_TYPE_COUNT
};
// Transport mock to forward the RTP and RTCP packet to the receiver; the data forward will be in a new thread
class CSimulcastRTPMediaTransportMock : public IRTPMediaTransport
{

private:
    class CDataForwardEvent : public ICmEvent
    {
    public:
        CDataForwardEvent(IRTPSessionClient  *recvSession, IWmeMediaPackage *rtcpPackage)
            : m_recvSession(recvSession), m_dataPackage(rtcpPackage)
        {
            m_recvSession->IncreaseReference();
            m_dataPackage->AddRef();
        }

        CDataForwardEvent(IRTPChannel *recvChannel, IWmeMediaPackage *rtpPackage)
            : m_recvChannel(recvChannel), m_dataPackage(rtpPackage)
        {
            m_recvChannel->IncreaseReference();
            m_dataPackage->AddRef();
        }

        virtual CmResult OnEventFire()
        {
            int32_t ret = 0;
            if (m_recvSession) {
                ret = m_recvSession->ReceiveRTCPPacket(m_dataPackage);
                m_recvSession->DecreaseReference();
            } else if (m_recvChannel) {
                ret = m_recvChannel->ReceiveRTPPacket(m_dataPackage);
                m_recvChannel->DecreaseReference();

            }

            m_dataPackage->Release();

            EXPECT_TRUE(WME_SUCCEEDED(ret));
            return ret;
        }

    private:
        IRTPSessionClient  *m_recvSession;
        IRTPChannel        *m_recvChannel;
        IWmeMediaPackage   *m_dataPackage;
    };

public:
    CSimulcastRTPMediaTransportMock(IRTPSessionClient *recvSession, IRTPChannel *recvChannel)
        : m_recvSession(recvSession)
        , m_recvChannel(recvChannel)
        , m_copyRTPMessageBlock(nullptr)
        , m_copyRTCPMessageBlock(nullptr)
        , m_rtcpFiltered(0)
        , m_rtpSendCount(0)
        , m_mutex()
        , m_networkThread(nullptr)
    {
        memset(m_rtcpSendCount, 0, RTCP_TYPE_COUNT*sizeof(uint32_t));
        m_recvSession->IncreaseReference();
        m_recvChannel->IncreaseReference();

        CmResult ret = ::CreateUserTaskThread("network", m_networkThread, TF_JOINABLE, TRUE, TT_UNKNOWN);
        EXPECT_TRUE(CM_SUCCEEDED(ret));

    }

    ~CSimulcastRTPMediaTransportMock()
    {
        if (nullptr != m_copyRTPMessageBlock) {
            delete m_copyRTPMessageBlock;
            m_copyRTPMessageBlock = nullptr;
        }
        if (nullptr != m_copyRTCPMessageBlock) {
            delete m_copyRTCPMessageBlock;
            m_copyRTCPMessageBlock = nullptr;
        }

        m_recvChannel->DecreaseReference();
        m_recvSession->DecreaseReference();

        m_networkThread->Stop();
        CmResult ret = m_networkThread->Join();
        EXPECT_TRUE(CM_SUCCEEDED(ret));
    }

    virtual WMERESULT GetTransportType(WmeTransportType &eTransportType)
    {
        eTransportType = WmeTransportType_UDP;
        return WME_S_OK;
    }

    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage)
    {
        {
            RecursiveMutexGuard g(m_mutex);
            ++ m_rtpSendCount;
        }

        CopyPacketAsMessageBlock(&m_copyRTPMessageBlock,pRTPPackage);

        CmResult ret = CM_OK;
        ret = m_networkThread->GetEventQueue()->PostEvent(new CDataForwardEvent(m_recvSession, pRTPPackage));
        EXPECT_TRUE(CM_SUCCEEDED(ret));

        return ret;
    }

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
    {
        {
            RecursiveMutexGuard g(m_mutex);
            ++ m_rtcpSendCount[RTCP_ALL];
        }

        CmResult ret = CM_OK;
        uint8_t packetType = GetRTCPPacketType(pRTCPPackage);
        if (RTCP_OTHERS != packetType) {//copy scr,sca,scaack
            WRTP_INFOTRACE("CSimulcastRTPMediaTransportMock::SendRTCPPacket: packetType=" << packetType);
            CopyPacketAsMessageBlock(&m_copyRTCPMessageBlock, pRTCPPackage);
        }

        if (!IsRTCPFiltered(packetType)) {
            ret = m_networkThread->GetEventQueue()->PostEvent(new CDataForwardEvent(m_recvSession, pRTCPPackage));
            EXPECT_TRUE(CM_SUCCEEDED(ret));
        }
        return ret;
    }

    uint32_t GetRTPSendCount()
    {
        RecursiveMutexGuard g(m_mutex);
        return m_rtpSendCount;
    }
    uint32_t GetRTCPSentCount(uint8_t packetType = RTCP_ALL)
    {
        RecursiveMutexGuard g(m_mutex);
        return m_rtcpSendCount[packetType];
    }

    void CopyPacketAsMessageBlock(CCmMessageBlock **copyMessageBlock,IWmeMediaPackage *pRTPPackage)
    {
        if (nullptr != *copyMessageBlock) {
            delete *copyMessageBlock;
            *copyMessageBlock = nullptr;
        }
        unsigned char *pBuffer = nullptr;
        uint32_t bufferOffset = 0;
        uint32_t bufferLen = 0;
        WMERESULT wmeRet = pRTPPackage->GetDataPointer(&pBuffer);
        ASSERT_FALSE(WME_FAILED(wmeRet) || nullptr == pBuffer);
        wmeRet = pRTPPackage->GetDataOffset(bufferOffset);
        ASSERT_FALSE(WME_FAILED(wmeRet));
        wmeRet = pRTPPackage->GetDataLength(bufferLen);
        ASSERT_FALSE(WME_FAILED(wmeRet) || 0 == bufferLen);
        CCmMessageBlock mb(bufferLen-bufferOffset, (LPCSTR)pBuffer+bufferOffset, 0, bufferLen-bufferOffset);
        *copyMessageBlock = new CCmMessageBlock(mb.GetTopLevelLength());
        memcpy((*copyMessageBlock)->GetTopLevelWritePtr(), mb.GetTopLevelReadPtr(), mb.GetTopLevelLength());
        (*copyMessageBlock)->AdvanceChainedWritePtr(mb.GetTopLevelLength());
        ASSERT_TRUE(nullptr != *copyMessageBlock);
    }

    CCmMessageBlock *GetRTPMessageBlock()
    {
        return m_copyRTPMessageBlock;
    }

    CCmMessageBlock *GetRTCPMessageBlock()
    {
        return m_copyRTCPMessageBlock;
    }

    void SetFilter(uint8_t packetType)
    {
        switch (packetType) {
            case RTCP_SCR: {
                m_rtcpFiltered |= 0x01;
                break;
            }
            case RTCP_SCA: {
                m_rtcpFiltered |= 0x02;
                break;
            }
            case RTCP_SCA_ACK: {
                m_rtcpFiltered |= 0x04;
                break;
            }

            default:
                break;
        }
    }

    void ResetFilter()
    {
        m_rtcpFiltered = 0;
    }
    bool IsRTCPFiltered(uint8_t packetType)
    {
        switch (packetType) {
            case RTCP_SCR: {
                return (m_rtcpFiltered & 0x01) != 0;
                break;
            }
            case RTCP_SCA: {
                return (m_rtcpFiltered & 0x02) != 0;
                break;
            }
            case RTCP_SCA_ACK: {
                return (m_rtcpFiltered & 0x04) != 0;
                break;
            }
            default:
                return true;//RTCP_OTHERS
                break;
        }
    }

    uint8_t GetRTCPPacketType(IWmeMediaPackage *pRTCPPackage)
    {
        unsigned char *pBuffer = nullptr;
        uint32_t       bufferOffset = 0;
        uint32_t       bufferLen    = 0;
        int32_t ret = pRTCPPackage->GetDataPointer(&pBuffer);
        EXPECT_EQ(WRTP_ERR_NOERR, ret);
        ret = pRTCPPackage->GetDataLength(bufferLen);
        EXPECT_EQ(WRTP_ERR_NOERR, ret);
        ret = pRTCPPackage->GetDataOffset(bufferOffset);
        EXPECT_EQ(WRTP_ERR_NOERR, ret);

        CCmMessageBlock mb(bufferLen, (LPCSTR)pBuffer+bufferOffset, 0, bufferLen);
        CCmByteStreamNetwork is(mb);
        CRTCPPacket *rtcpPacket = nullptr;
        RTCPHeader   header;

        EXPECT_TRUE(DecodeRTCPHeader(is, header) == TRUE);
        switch (header.type) {
            case RTCP_PT_PSFB: {
                rtcpPacket = new CPSFBPacket();
                rtcpPacket->SetRTCPHeader(header);
                EXPECT_TRUE(rtcpPacket->Decode(is, mb) == TRUE);
                CPSFBPacket *psfbPacket = (CPSFBPacket *)rtcpPacket;
                if (memcmp(psfbPacket->m_fci, "MSTR", 4) == 0) {
                    uint16_t len = psfbPacket->GetFCILength();
                    CRTCPFciBaseMsg subscribingPacket;
                    ret = subscribingPacket.DecodeFrom(psfbPacket->GetFCIData(), len);
                    EXPECT_EQ(WRTP_ERR_NOERR, ret);
                    RecursiveMutexGuard g(m_mutex);
                    switch (subscribingPacket.GetFeedbackType()) {
                        case RTCP_SUBSCRIBING_TYPE_SCR: {
                            ++m_rtcpSendCount[RTCP_SCR];
                            return RTCP_SCR;
                        }
                        case RTCP_SUBSCRIBING_TYPE_SCA: {
                            ++m_rtcpSendCount[RTCP_SCA];
                            return RTCP_SCA;
                        }
                        case RTCP_SUBSCRIBING_TYPE_SCAACK: {
                            ++m_rtcpSendCount[RTCP_SCA_ACK];
                            return RTCP_SCA_ACK;
                        }
                    }
                }
            }
            default: {
                RecursiveMutexGuard g(m_mutex);
                ++m_rtcpSendCount[RTCP_OTHERS];
                return RTCP_OTHERS;
            }
        }

    }


private:
    IRTPSessionClient  *m_recvSession;
    IRTPChannel        *m_recvChannel;

    uint32_t              m_rtpSendCount;
    uint32_t              m_rtcpSendCount[RTCP_TYPE_COUNT];
    uint8_t               m_rtcpFiltered;
    RecursiveMutex  m_mutex;

    ACmThread          *m_networkThread;
    CCmMessageBlock    *m_copyRTPMessageBlock;
    CCmMessageBlock    *m_copyRTCPMessageBlock;
};

// IRTPSimulcastRequestCallback mock
class CRTPSimulcastRequestCallbackMock : public IRTPSimulcastRequestCallback
{
public:
    CRTPSimulcastRequestCallbackMock()
        : m_availableChannels(0)
        , m_maxAdjacentSources(0)
        , m_isCallback(false) {}

    virtual void OnAnnounce(const SubsessionChannelAnnounce &announce)
    {
        RecursiveMutexGuard g(m_mutex);
        m_isCallback = true;
        return;
    }

    bool IsCallback()
    {
        RecursiveMutexGuard g(m_mutex);
        return m_isCallback;
    }
private:
    uint8_t m_availableChannels;
    uint8_t m_maxAdjacentSources;
    RecursiveMutex  m_mutex;
    bool  m_isCallback;
};


typedef list< pair<ActiveSpeakerRequestTrait, CodecCapability> >    ActiveSpeakerRequestList;
typedef list< pair<SelectedSourceRequestTrait, CodecCapability> >   SelectedSourceRequestList;
typedef list< pair<NonePolicyRequestTrait, CodecCapability> >       NonePolicyRequestList;

class CSubsessionChannelIdChecker
{
public:
    CSubsessionChannelIdChecker(uint8_t id) : m_id(id) {}

    // T would be pair<ActiveSpeakerRequestTrait, CodecCapability>, pair<SelectedSourceRequestTrait, CodecCapability> or pair<NonePolicyRequestTrait, CodecCapability>
    template <typename T>
    bool operator()(const T &t)
    {
        return (m_id == t.first.subsessionChannelId);
    }

private:
    uint8_t m_id;
};

// IRTPSimulcastResponseCallback mock
class CRTPSimulcastResponseCallbackMock : public IRTPSimulcastResponseCallback
{
public:
    CRTPSimulcastResponseCallbackMock()
        : m_activeSpeakerRequests()
        , m_selectedSourceRequests()
        , m_nonePolicyRequests()
    {}

    virtual int32_t OnSubscribe(const SubsessionChannelRequest *requests, uint8_t requestCount)
    {
        m_activeSpeakerRequests.clear();
        m_selectedSourceRequests.clear();
        m_nonePolicyRequests.clear();

        for (uint8_t ii = 0; ii < requestCount; ++ii) {
            const SubsessionChannelRequest &request = requests[ii];
            switch (request.policyType) {
                case POLICY_TYPE_NONE_POLICY:
                    m_nonePolicyRequests.push_back(make_pair(request.nonePolicyTrait, request.codecCapability));
                    break;

                case POLICY_RECEIVER_SELECTED_SOURCE:
                    m_selectedSourceRequests.push_back(make_pair(request.selectedSourceTrait, request.codecCapability));
                    break;

                case POLICY_TYPE_ACTIVE_SPEAKER:
                    m_activeSpeakerRequests.push_back(make_pair(request.activeSpeakerTrait, request.codecCapability));
                    break;
            }
        }

        return WRTP_ERR_NOERR;
    }

    virtual int32_t QuerySubsessionChannelId(const MediaProperty &property, uint8_t *ids, RTP_INOUT uint8_t &idCount)
    {
        vector<uint8_t> vids;
        for (ActiveSpeakerRequestList::const_iterator iter = m_activeSpeakerRequests.begin(); iter != m_activeSpeakerRequests.end(); ++iter) {
            if (CanBeSatisfiedBy(property, (*iter).second)) {
                vids.push_back((*iter).first.subsessionChannelId);
            }
        }

        for (SelectedSourceRequestList::const_iterator iter = m_selectedSourceRequests.begin(); iter != m_selectedSourceRequests.end(); ++iter) {
            if (CanBeSatisfiedBy(property, (*iter).second)) {
                vids.push_back((*iter).first.subsessionChannelId);
            }
        }

        for (NonePolicyRequestList::const_iterator iter = m_nonePolicyRequests.begin(); iter != m_nonePolicyRequests.end(); ++iter) {
            if (CanBeSatisfiedBy(property, (*iter).second)) {
                vids.push_back((*iter).first.subsessionChannelId);
            }
        }

        uint8_t count = idCount;
        if (count > vids.size()) {
            count = vids.size();
        }

        for (uint8_t ii = 0; ii < count; ++ii) {
            ids[ii] = vids[ii];
        }
        idCount = count;

        return WRTP_ERR_NOERR;
    }

    const ActiveSpeakerRequestList     &GetActiveSpeaderRequests() const { return m_activeSpeakerRequests; }
    const SelectedSourceRequestList    &GetSelectedSourceRequests() const { return m_selectedSourceRequests; }
    const NonePolicyRequestList        &GetNonePolicyRequests() const { return m_nonePolicyRequests; }

private:
    bool CanBeSatisfiedBy(const MediaProperty &property, const CodecCapability &codecCap)
    {
        switch (codecCap.type) {
            case REQUEST_CODEC_TYPE_COMMON:
                if (property.payloadType == codecCap.commonCodecCap.payloadType) {
                    return true;
                }
                break;

            case REQUEST_CODEC_TYPE_H264:
                if (property.payloadType == codecCap.h264CodecCap.payloadType) {
                    return true;
                }
                break;
        }

        return false;
    }
private:
    ActiveSpeakerRequestList    m_activeSpeakerRequests;
    SelectedSourceRequestList   m_selectedSourceRequests;
    NonePolicyRequestList       m_nonePolicyRequests;
};

#define SAFE_RELEASE(p)         \
    do {                            \
        if (!p) break;              \
        p->DecreaseReference();     \
        p = nullptr;                   \
    } while(0)

void Assert_EQ(const ActiveSpeakerRequestTrait &lhs, const ActiveSpeakerRequestTrait &rhs)
{
    ASSERT_EQ(lhs.subsessionChannelId,  rhs.subsessionChannelId);
    ASSERT_EQ(lhs.sourceId,             rhs.sourceId);
    ASSERT_EQ(lhs.bitrate,              rhs.bitrate);

    ASSERT_EQ(lhs.priority,             rhs.priority);
    ASSERT_EQ(lhs.groupingAdjacencyId,  rhs.groupingAdjacencyId);
    ASSERT_EQ(lhs.duplicationFlag,      rhs.duplicationFlag);
}

void Assert_EQ(const SelectedSourceRequestTrait &lhs, const SelectedSourceRequestTrait &rhs)
{
    ASSERT_EQ(lhs.subsessionChannelId,  rhs.subsessionChannelId);
    ASSERT_EQ(lhs.sourceId,             rhs.sourceId);
    ASSERT_EQ(lhs.bitrate,              rhs.bitrate);

    ASSERT_EQ(lhs.captureSourceId,      rhs.captureSourceId);
}

void Assert_EQ(const NonePolicyRequestTrait &lhs, const NonePolicyRequestTrait &rhs)
{
    ASSERT_EQ(lhs.subsessionChannelId,  rhs.subsessionChannelId);
    ASSERT_EQ(lhs.sourceId,             rhs.sourceId);
    ASSERT_EQ(lhs.bitrate,              rhs.bitrate);
}

void Assert_EQ(const CommonCodecCapability &lhs, const CommonCodecCapability &rhs)
{
    ASSERT_EQ(lhs.payloadType,          rhs.payloadType);
}

void Assert_EQ(const H264CodecCapability &lhs, const H264CodecCapability &rhs)
{
    ASSERT_EQ(lhs.payloadType,          rhs.payloadType);
    ASSERT_EQ(lhs.maxMBPS,              rhs.maxMBPS);
    ASSERT_EQ(lhs.maxFS,                rhs.maxFS);
    ASSERT_EQ(lhs.maxFPS,               rhs.maxFPS);
    ASSERT_EQ(lhs.temporalLayerCount,   rhs.temporalLayerCount);
    ASSERT_LE(lhs.temporalLayerCount, MAX_TEMPORAL_LAYERS);
    for (uint8_t ii = 0; ii < lhs.temporalLayerCount; ++ii) {
        ASSERT_EQ(lhs.temporalLayers[ii], rhs.temporalLayers[ii]);
    }
}

void Assert_EQ(const CodecCapability &lhs, const CodecCapability &rhs)
{
    ASSERT_EQ(lhs.type,                 rhs.type);
    switch (lhs.type) {
        case REQUEST_CODEC_TYPE_COMMON:
            Assert_EQ(lhs.commonCodecCap, rhs.commonCodecCap);
            break;

        case REQUEST_CODEC_TYPE_H264:
            Assert_EQ(lhs.h264CodecCap, rhs.h264CodecCap);
            break;
    }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Simulcast Response Callback             Simulcast Request Callback
//          |                                             |
//      RTP Sender                                  RTP Receiver
//          |                                             |
//          |--------------->forward transport------>-----|
//          |-------<--------backward transport<----------|
/////////////////////////////////////////////////////////////////////////////////////////////////////
#define SIMULCAST_SOURCE_ID_1 1
#define SIMULCAST_SOURCE_ID_2 2
//CODEC_TYPE_2/PAYLOAD_TYPE_2/CLOCKRATE_2 test unsubscribed codec type
#define CODEC_TYPE_2          77
#define PAYLOAD_TYPE_2        66
#define CLOCKRATE_2           9000
class CSimulcastRequestTest : public ::testing::Test
{
public:
    CSimulcastRequestTest()
        : m_sendSession(nullptr)
        , m_sendChannel(nullptr)
        , m_recvSession(nullptr)
        , m_recvChannel(nullptr)
        , m_vidExtURI("http://protocols.cisco.com/virtualid")
        , m_vidExtId(1)
        , m_channelId(100)
        , m_forwardTransport()
        , m_reverseTransport()
        , m_simulcastRequestCallback()
        , m_simulcastResponseCallback()
    {}

    ~CSimulcastRequestTest() {}

protected:
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {
        // teardown the receiver side
        m_recvSession->SetMediaTransport(nullptr);
        m_reverseTransport.reset();

        m_recvSession->SetSimulcastResponseCallback(nullptr);
        m_simulcastResponseCallback.reset();

        SAFE_RELEASE(m_recvChannel);
        SAFE_RELEASE(m_recvSession);

        // teardown the sender side
        m_sendSession->SetMediaTransport(nullptr);
        m_forwardTransport.reset();

        m_sendSession->SetSimulcastRequestCallback(nullptr);
        m_simulcastRequestCallback.reset();


        SAFE_RELEASE(m_sendChannel);
        SAFE_RELEASE(m_sendSession);
    }

    void InitSessionChannel(WRTPSessionType sessionType, bool sender)
    {
        CodecType codecType = (sessionType == RTP_SESSION_WEBEX_VOIP) ? CODEC_TYPE_AUDIO_TEST : CODEC_TYPE_VIDEO_TEST;
        uint8_t payloadType   = (sessionType == RTP_SESSION_WEBEX_VOIP) ? PAYLOAD_TYPE_AUDIO_TEST : PAYLOAD_TYPE_VIDEO_TEST;
        uint32_t clockRate    = (sessionType == RTP_SESSION_WEBEX_VOIP) ? DEFAULT_AUDIO_CAPTURE_CLOCK_RATE : DEFAULT_VIDIO_CAPTURE_CLOCK_RATE;

        IRTPSessionClient *&session = sender ? m_sendSession : m_recvSession;
        IRTPChannel *&channel       = sender ? m_sendChannel : m_recvChannel;

        // create session
        WRTPSessionParams sessionParams;
        sessionParams.sessionType   = sessionType;
        sessionParams.enableRTCP    = true;

        session = WRTPCreateRTPSessionClient(sessionParams);
        ASSERT_TRUE(nullptr != session);
        session->IncreaseReference();

        // register VID RTP header extension
        int32_t ret = session->UpdateRTPExtension(m_vidExtURI.c_str(), m_vidExtId, wrtp::STREAM_INOUT);
        ASSERT_TRUE(WRTP_SUCCEEDED(ret));

        // register payload type
        ret = session->RegisterPayloadType(codecType, payloadType, clockRate);
        ASSERT_TRUE(WRTP_SUCCEEDED(ret));

        // set package allocator
        ret = session->SetMediaPackageAllocator(CWmeMediaPackageAllocatorMock::GetInstance());
        ASSERT_TRUE(WRTP_SUCCEEDED(ret));

        // create channel
        WRTPChannelParams channelParams;

        channel = session->CreateRTPChannel(m_channelId, channelParams);
        ASSERT_TRUE(nullptr != channel);
        channel->IncreaseReference();
    }

    void BindSenderReceiver()
    {
        // transport
        m_forwardTransport.reset(new CSimulcastRTPMediaTransportMock(m_recvSession, m_recvChannel));
        m_sendSession->SetMediaTransport(m_forwardTransport.get());

        m_reverseTransport.reset(new CSimulcastRTPMediaTransportMock(m_sendSession, m_sendChannel));
        m_recvSession->SetMediaTransport(m_reverseTransport.get());

        // simulcast callback
        m_simulcastRequestCallback.reset(new CRTPSimulcastRequestCallbackMock());
        m_recvSession->SetSimulcastRequestCallback(m_simulcastRequestCallback.get());

        m_simulcastResponseCallback.reset(new CRTPSimulcastResponseCallbackMock());
        m_sendSession->SetSimulcastResponseCallback(m_simulcastResponseCallback.get());

        // enable SCR
        m_sendSession->EnableCiscoSCR(true);
        m_recvSession->EnableCiscoSCR(true);

        // configure simulcast request
        int32_t ret = WRTP_ERR_NOERR;
        ret = m_sendSession->SetMaxSubsessionChannels(5, wrtp::STREAM_INOUT);
        ASSERT_TRUE(WRTP_SUCCEEDED(ret));
        ret = m_recvSession->SetMaxSubsessionChannels(5, wrtp::STREAM_INOUT);
        ASSERT_TRUE(WRTP_SUCCEEDED(ret));

        uint8_t sources[] = {SIMULCAST_SOURCE_ID_1, SIMULCAST_SOURCE_ID_2};
        for (uint8_t ii = 0; ii < sizeof(sources)/sizeof(sources[0]); ++ii) {
            ret = m_sendSession->RegisterPolicyId(sources[ii], POLICY_RECEIVER_SELECTED_SOURCE, 11, wrtp::STREAM_INOUT);
            ASSERT_TRUE(WRTP_SUCCEEDED(ret));

            ret = m_recvSession->RegisterPolicyId(sources[ii], POLICY_RECEIVER_SELECTED_SOURCE, 11, wrtp::STREAM_INOUT);
            ASSERT_TRUE(WRTP_SUCCEEDED(ret));

            ret = m_sendSession->RegisterPolicyId(sources[ii], POLICY_TYPE_ACTIVE_SPEAKER, 22, wrtp::STREAM_INOUT);
            ASSERT_TRUE(WRTP_SUCCEEDED(ret));

            ret = m_recvSession->RegisterPolicyId(sources[ii], POLICY_TYPE_ACTIVE_SPEAKER, 22, wrtp::STREAM_INOUT);
            ASSERT_TRUE(WRTP_SUCCEEDED(ret));
        }
        //        m_sendSession->g
    }

    void CheckVIDs(CCmMessageBlock *mb, VidVector &vids)
    {
        ASSERT_TRUE(nullptr != mb);
        CRTPPacketAuto packet(new CRTPPacket());
        CmResult ret = packet->Unpack(*mb);
        ASSERT_EQ(WRTP_ERR_NOERR, ret);

        uint8_t id = 1;// VID extension:id = 1;
        uint8_t *vidp = nullptr;
        uint8_t vidLen = 0;

        ret = packet->GetExtensionElement(id, vidp, vidLen);
        ASSERT_EQ(WRTP_ERR_NOERR, ret);

        //find vid
        EXPECT_EQ(vids.size(), vidLen);
        for (VidVector::iterator it = vids.begin(); it != vids.end(); ++it) {
            int ii = 0;
            for (; ii < vidLen; ++ii) {
                if (vidp[ii] == *it) {
                    break;
                }
            }
            EXPECT_NE(vidLen, ii);
        }
    }
#define RTCP_HEADER_LEN (1+1+2+4+4) //flag+type+length+ssrc+ssrcSrc
#define FCI_LEN (4+2+2) //feedbackIdentifier+messageTypeVersion+SeqNumber
    void ChangeSCASequenceNumber(CCmMessageBlock *rtcpMb, int16_t requestSeqAdvance)
    {
        uint8_t   rtcpType = (uint8_t)*(rtcpMb->GetTopLevelReadPtr()+1);
        ASSERT_EQ(RTCP_PT_PSFB, rtcpType);
        //        uint32_t *pFeedbackIdentifier = (uint32_t*)(rtcpMb->GetTopLevelReadPtr()+RTCP_HEADER_LEN);
        //        uint32_t feedbackIdentifier = ntohl(*pFeedbackIdentifier);
        uint16_t messageType = ntohs(*(uint16_t *)(rtcpMb->GetTopLevelReadPtr()+RTCP_HEADER_LEN+4));
        ASSERT_EQ(((2<<4)|(0x1)), messageType);//sca
        uint16_t *requestSeq = (uint16_t *)(rtcpMb->GetTopLevelReadPtr()+RTCP_HEADER_LEN+FCI_LEN);
        uint16_t hostRequestSeq = ntohs(*requestSeq);
        hostRequestSeq += requestSeqAdvance;
        uint16_t netRequestSeq = htons(hostRequestSeq);
        memcpy(requestSeq, &netRequestSeq, sizeof(uint16_t));

    }

    void ChangeSCAACKSequenceNumber(CCmMessageBlock *rtcpMb, int16_t requestAckSeqAdvance)
    {
        uint8_t   rtcpType = (uint8_t)*(rtcpMb->GetTopLevelReadPtr()+1);
        ASSERT_EQ(RTCP_PT_PSFB, rtcpType);

        uint16_t messageType = ntohs(*(uint16_t *)(rtcpMb->GetTopLevelReadPtr()+RTCP_HEADER_LEN+4));
        ASSERT_EQ(((3<<4)|(0x1)), messageType);//sca-ack
        uint16_t *requestAckSeq = (uint16_t *)(rtcpMb->GetTopLevelReadPtr()+RTCP_HEADER_LEN+4+2);
        uint16_t hostRequestSeq = ntohs(*requestAckSeq);
        hostRequestSeq += requestAckSeqAdvance;
        uint16_t netRequestSeq = htons(hostRequestSeq);
        memcpy(requestAckSeq, &netRequestSeq, sizeof(uint16_t));

    }

    void TransformMessageBlockToRTPMediaPackage(IRTPMediaPackage **mediaPackage, CCmMessageBlock *mb)
    {
        EXPECT_TRUE(!(*mediaPackage));
        uint32_t dataSize = mb->GetTopLevelLength();
        *mediaPackage = new CWMEMediaPackage(RTPMediaFormatRTCP, 0, dataSize);
        (*mediaPackage)->AddRef();
        ASSERT_TRUE(*mediaPackage != nullptr);
        (*mediaPackage)->SetTimestamp(6789);
        unsigned char *buffer = nullptr;
        WMERESULT ret = (*mediaPackage)->GetDataPointer(&buffer);
        ASSERT_TRUE(buffer != nullptr);
        ASSERT_EQ(WRTP_ERR_NOERR, ret);
        memcpy(buffer, mb->GetTopLevelReadPtr(), dataSize);
        (*mediaPackage)->SetDataLength(dataSize);
    }

    void SendSCAAnnounce(bool reliable)
    {
        if (m_sendSession) {
            SubsessionChannelAnnounce announce;
            int32_t ret = m_sendSession->Announce(announce, reliable);
            EXPECT_TRUE(WRTP_SUCCEEDED(ret));
        }

    }
protected:
    IRTPSessionClient      *m_sendSession;
    IRTPChannel            *m_sendChannel;

    IRTPSessionClient      *m_recvSession;
    IRTPChannel            *m_recvChannel;

    const string            m_vidExtURI;
    const uint8_t             m_vidExtId;
    const uint32_t            m_channelId;

    CCmSharedPtr<CSimulcastRTPMediaTransportMock>   m_forwardTransport;
    CCmSharedPtr<CSimulcastRTPMediaTransportMock>   m_reverseTransport;

    CCmSharedPtr<CRTPSimulcastRequestCallbackMock>  m_simulcastRequestCallback;
    CCmSharedPtr<CRTPSimulcastResponseCallbackMock> m_simulcastResponseCallback;
};

#define NETWORK_TRANSMISSION_INTERVAL 500

//--------------------------------Empty Request--------------------------------------
TEST_F(CSimulcastRequestTest, Audio_Empty_Request_Test)
{
    CScopedTracer test_info;

    // initialize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, false);
    BindSenderReceiver();

    int32_t ret = 0;
    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    // check the request
    const NonePolicyRequestList &nonePolicyRequests = m_simulcastResponseCallback->GetNonePolicyRequests();
    ASSERT_EQ(0, nonePolicyRequests.size());

    const SelectedSourceRequestList &selectedRequests = m_simulcastResponseCallback->GetSelectedSourceRequests();
    ASSERT_EQ(0, selectedRequests.size());

    const ActiveSpeakerRequestList &activeRequests = m_simulcastResponseCallback->GetActiveSpeaderRequests();
    ASSERT_EQ(0, activeRequests.size());
}

TEST_F(CSimulcastRequestTest, Vedio_Empty_Request_Test)
{
    CScopedTracer test_info;

    // initialize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    int32_t ret = 0;
    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    // check the request
    const NonePolicyRequestList &nonePolicyRequests = m_simulcastResponseCallback->GetNonePolicyRequests();
    ASSERT_EQ(0, nonePolicyRequests.size());

    const SelectedSourceRequestList &selectedRequests = m_simulcastResponseCallback->GetSelectedSourceRequests();
    ASSERT_EQ(0, selectedRequests.size());

    const ActiveSpeakerRequestList &activeRequests = m_simulcastResponseCallback->GetActiveSpeaderRequests();
    ASSERT_EQ(0, activeRequests.size());
}

//--------------------------------None Policy--------------------------------------
TEST_F(CSimulcastRequestTest, Audio_None_Policy_Request_Test)
{
    CScopedTracer test_info;

    // initialize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, false);
    BindSenderReceiver();

    // request info
    NonePolicyRequestTrait trait;
    trait.subsessionChannelId   = 1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 64*1024*8;

    CodecCapability codecCap;
    codecCap.type                       = REQUEST_CODEC_TYPE_COMMON;
    codecCap.commonCodecCap.payloadType = PAYLOAD_TYPE_AUDIO_TEST;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    // check the request
    const NonePolicyRequestList &requests = m_simulcastResponseCallback->GetNonePolicyRequests();
    ASSERT_EQ(1, requests.size());
    for (NonePolicyRequestList::const_iterator iter = requests.begin(); iter != requests.end(); ++iter) {
        Assert_EQ((*iter).first, trait);
        Assert_EQ((*iter).second, codecCap);
    }
}

TEST_F(CSimulcastRequestTest, Video_None_Policy_Request_Test)
{
    CScopedTracer test_info;

    // initialize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    // request info
    NonePolicyRequestTrait trait;
    trait.subsessionChannelId   = 1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 64*1024*8;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    H264CodecCapability &h264   = codecCap.h264CodecCap;
    h264.payloadType            = PAYLOAD_TYPE_VIDEO_TEST;
    h264.maxMBPS                = 1024;
    h264.maxFS                  = 240;
    h264.maxFPS                 = 30;
    h264.temporalLayerCount     = 3;
    h264.temporalLayers[0]      = 750;
    h264.temporalLayers[1]      = 1500;
    h264.temporalLayers[2]      = 3000;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    // check the request
    const NonePolicyRequestList &requests = m_simulcastResponseCallback->GetNonePolicyRequests();
    ASSERT_EQ(1, requests.size());
    for (NonePolicyRequestList::const_iterator iter = requests.begin(); iter != requests.end(); ++iter) {
        Assert_EQ((*iter).first, trait);
        Assert_EQ((*iter).second, codecCap);
    }
}


//--------------------------------Selected Source Policy--------------------------------------
TEST_F(CSimulcastRequestTest, Audio_Selected_Source_Request_Test)
{
    CScopedTracer test_info;

    // initialize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, false);
    BindSenderReceiver();

    // request info
    SelectedSourceRequestTrait trait;
    trait.subsessionChannelId   = 1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 64*1024*8;
    trait.captureSourceId       = 111222;

    CodecCapability codecCap;
    codecCap.type                       = REQUEST_CODEC_TYPE_COMMON;
    codecCap.commonCodecCap.payloadType = PAYLOAD_TYPE_AUDIO_TEST;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    // check the request
    const SelectedSourceRequestList &requests = m_simulcastResponseCallback->GetSelectedSourceRequests();
    ASSERT_EQ(1, requests.size());
    for (SelectedSourceRequestList::const_iterator iter = requests.begin(); iter != requests.end(); ++iter) {
        Assert_EQ((*iter).first, trait);
        Assert_EQ((*iter).second, codecCap);
    }
}

TEST_F(CSimulcastRequestTest, Video_Selected_Source_Request_Test)
{
    CScopedTracer test_info;

    // initialize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    // request info
    // request info
    SelectedSourceRequestTrait trait;
    trait.subsessionChannelId   = 1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 64*1024*8;
    trait.captureSourceId       = 111222;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    H264CodecCapability &h264   = codecCap.h264CodecCap;
    h264.payloadType            = PAYLOAD_TYPE_VIDEO_TEST;
    h264.maxMBPS                = 1024;
    h264.maxFS                  = 240;
    h264.maxFPS                 = 30;
    h264.temporalLayerCount     = 3;
    h264.temporalLayers[0]      = 750;
    h264.temporalLayers[1]      = 1500;
    h264.temporalLayers[2]      = 3000;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    // check the request
    const SelectedSourceRequestList &requests = m_simulcastResponseCallback->GetSelectedSourceRequests();
    ASSERT_EQ(1, requests.size());
    for (SelectedSourceRequestList::const_iterator iter = requests.begin(); iter != requests.end(); ++iter) {
        Assert_EQ((*iter).first, trait);
        Assert_EQ((*iter).second, codecCap);
    }
}

//--------------------------------Active Speaker Policy--------------------------------------
TEST_F(CSimulcastRequestTest, Audio_Active_Speaker_Request_Test)
{
    CScopedTracer test_info;

    // initialize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, false);
    BindSenderReceiver();

    // request info
    ActiveSpeakerRequestTrait trait;
    trait.subsessionChannelId   = 1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 64*1024*8;
    trait.priority              = 12;
    trait.groupingAdjacencyId   = 1;
    trait.duplicationFlag       = true;

    CodecCapability codecCap;
    codecCap.type                       = REQUEST_CODEC_TYPE_COMMON;
    codecCap.commonCodecCap.payloadType = PAYLOAD_TYPE_AUDIO_TEST;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    // check the request
    const ActiveSpeakerRequestList &requests = m_simulcastResponseCallback->GetActiveSpeaderRequests();
    ASSERT_EQ(1, requests.size());
    for (ActiveSpeakerRequestList::const_iterator iter = requests.begin(); iter != requests.end(); ++iter) {
        Assert_EQ((*iter).first, trait);
        Assert_EQ((*iter).second, codecCap);
    }
}

TEST_F(CSimulcastRequestTest, Video_Active_Speaker_Request_Test)
{
    CScopedTracer test_info;

    // initialize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    // request info
    ActiveSpeakerRequestTrait trait;
    trait.subsessionChannelId   = 1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 64*1024*8;
    trait.priority              = 10;
    trait.groupingAdjacencyId   = 1;
    trait.duplicationFlag       = true;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    H264CodecCapability &h264   = codecCap.h264CodecCap;
    h264.payloadType            = PAYLOAD_TYPE_VIDEO_TEST;
    h264.maxMBPS                = 1024;
    h264.maxFS                  = 240;
    h264.maxFPS                 = 30;
    h264.temporalLayerCount     = 3;
    h264.temporalLayers[0]      = 750;
    h264.temporalLayers[1]      = 1500;
    h264.temporalLayers[2]      = 3000;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    // check the request
    const ActiveSpeakerRequestList &activeSpeakerRequests = m_simulcastResponseCallback->GetActiveSpeaderRequests();
    ASSERT_EQ(1, activeSpeakerRequests.size());
    for (ActiveSpeakerRequestList::const_iterator iter = activeSpeakerRequests.begin(); iter != activeSpeakerRequests.end(); ++iter) {
        Assert_EQ((*iter).first, trait);
        Assert_EQ((*iter).second, codecCap);
    }
}

//--------------------------------Compound Policy--------------------------------------
TEST_F(CSimulcastRequestTest, Audio_Compound_Requests_Test)
{
    CScopedTracer test_info;

    // initialize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, false);
    BindSenderReceiver();

    CodecCapability codecCap;
    codecCap.type                       = REQUEST_CODEC_TYPE_COMMON;
    codecCap.commonCodecCap.payloadType = PAYLOAD_TYPE_AUDIO_TEST;

    int32_t ret = 0;

    // None Policy request info
    NonePolicyRequestTrait noneTrait;
    noneTrait.subsessionChannelId   = 1;
    noneTrait.sourceId              = SIMULCAST_SOURCE_ID_1;
    noneTrait.bitrate               = 64*1024*8;

    ret = m_recvSession->Subscribe(noneTrait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    // Selected Source request info
    SelectedSourceRequestTrait selectedTrait;
    selectedTrait.subsessionChannelId   = 2;
    selectedTrait.sourceId              = SIMULCAST_SOURCE_ID_1;
    selectedTrait.bitrate               = 64*1024*8;
    selectedTrait.captureSourceId       = 111222;

    ret = m_recvSession->Subscribe(selectedTrait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    // Active Policy request info
    ActiveSpeakerRequestTrait activeTrait;
    activeTrait.subsessionChannelId   = 3;
    activeTrait.sourceId              = SIMULCAST_SOURCE_ID_2;
    activeTrait.bitrate               = 64*1024*8;
    activeTrait.priority              = 12;
    activeTrait.groupingAdjacencyId   = 1;
    activeTrait.duplicationFlag       = false;

    ret = m_recvSession->Subscribe(activeTrait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));


    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    //----------------------------------------------------------------------------------
    // check the request
    const NonePolicyRequestList &nonePolicyRequests = m_simulcastResponseCallback->GetNonePolicyRequests();
    ASSERT_EQ(1, nonePolicyRequests.size());
    for (NonePolicyRequestList::const_iterator iter = nonePolicyRequests.begin(); iter != nonePolicyRequests.end(); ++iter) {
        Assert_EQ((*iter).first, noneTrait);
        Assert_EQ((*iter).second, codecCap);
    }

    const SelectedSourceRequestList &selectedRequests = m_simulcastResponseCallback->GetSelectedSourceRequests();
    ASSERT_EQ(1, selectedRequests.size());
    for (SelectedSourceRequestList::const_iterator iter = selectedRequests.begin(); iter != selectedRequests.end(); ++iter) {
        Assert_EQ((*iter).first, selectedTrait);
        Assert_EQ((*iter).second, codecCap);
    }

    const ActiveSpeakerRequestList &activeRequests = m_simulcastResponseCallback->GetActiveSpeaderRequests();
    ASSERT_EQ(1, activeRequests.size());
    for (ActiveSpeakerRequestList::const_iterator iter = activeRequests.begin(); iter != activeRequests.end(); ++iter) {
        Assert_EQ((*iter).first, activeTrait);
        Assert_EQ((*iter).second, codecCap);
    }
}

TEST_F(CSimulcastRequestTest, Video_Compound_Requests_Test)
{
    CScopedTracer test_info;

    // initialize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    H264CodecCapability &h264   = codecCap.h264CodecCap;
    h264.payloadType            = PAYLOAD_TYPE_VIDEO_TEST;
    h264.maxMBPS                = 1024;
    h264.maxFS                  = 240;
    h264.maxFPS                 = 30;
    h264.temporalLayerCount     = 3;
    h264.temporalLayers[0]      = 750;
    h264.temporalLayers[1]      = 1500;
    h264.temporalLayers[2]      = 3000;

    int32_t ret = 0;

    // None Policy request info
    NonePolicyRequestTrait noneTrait;
    noneTrait.subsessionChannelId   = 1;
    noneTrait.sourceId              = SIMULCAST_SOURCE_ID_1;
    noneTrait.bitrate               = 64*1024*8;

    ret = m_recvSession->Subscribe(noneTrait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    // Selected Source request info
    SelectedSourceRequestTrait selectedTrait;
    selectedTrait.subsessionChannelId   = 2;
    selectedTrait.sourceId              = SIMULCAST_SOURCE_ID_1;
    selectedTrait.bitrate               = 64*1024*8;
    selectedTrait.captureSourceId       = 111222;

    ret = m_recvSession->Subscribe(selectedTrait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    // Active Policy request info
    ActiveSpeakerRequestTrait activeTrait;
    activeTrait.subsessionChannelId   = 3;
    activeTrait.sourceId              = SIMULCAST_SOURCE_ID_2;
    activeTrait.bitrate               = 64*1024*8;
    activeTrait.priority              = 12;
    activeTrait.groupingAdjacencyId   = 1;
    activeTrait.duplicationFlag       = true;

    ret = m_recvSession->Subscribe(activeTrait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));


    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    //----------------------------------------------------------------------------------
    // check the request
    const NonePolicyRequestList &nonePolicyRequests = m_simulcastResponseCallback->GetNonePolicyRequests();
    ASSERT_EQ(1, nonePolicyRequests.size());
    for (NonePolicyRequestList::const_iterator iter = nonePolicyRequests.begin(); iter != nonePolicyRequests.end(); ++iter) {
        Assert_EQ((*iter).first, noneTrait);
        Assert_EQ((*iter).second, codecCap);
    }

    const SelectedSourceRequestList &selectedRequests = m_simulcastResponseCallback->GetSelectedSourceRequests();
    ASSERT_EQ(1, selectedRequests.size());
    for (SelectedSourceRequestList::const_iterator iter = selectedRequests.begin(); iter != selectedRequests.end(); ++iter) {
        Assert_EQ((*iter).first, selectedTrait);
        Assert_EQ((*iter).second, codecCap);
    }

    const ActiveSpeakerRequestList &activeRequests = m_simulcastResponseCallback->GetActiveSpeaderRequests();
    ASSERT_EQ(1, activeRequests.size());
    for (ActiveSpeakerRequestList::const_iterator iter = activeRequests.begin(); iter != activeRequests.end(); ++iter) {
        Assert_EQ((*iter).first, activeTrait);
        Assert_EQ((*iter).second, codecCap);
    }
}

//--------------------------------Subscribe then unsubscribe--------------------------------------
TEST_F(CSimulcastRequestTest, Audio_Subscribe_Then_Unsubscribe_Requests_Test)
{
    CScopedTracer test_info;

    // initialize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, false);
    BindSenderReceiver();

    CodecCapability codecCap;
    codecCap.type                       = REQUEST_CODEC_TYPE_COMMON;
    codecCap.commonCodecCap.payloadType = PAYLOAD_TYPE_AUDIO_TEST;

    int32_t ret = 0;

    // None Policy request info
    NonePolicyRequestTrait noneTrait;
    noneTrait.subsessionChannelId   = 1;
    noneTrait.sourceId              = SIMULCAST_SOURCE_ID_1;
    noneTrait.bitrate               = 64*1024*8;

    ret = m_recvSession->Subscribe(noneTrait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    // Selected Source request info
    SelectedSourceRequestTrait selectedTrait;
    selectedTrait.subsessionChannelId   = 2;
    selectedTrait.sourceId              = SIMULCAST_SOURCE_ID_1;
    selectedTrait.bitrate               = 64*1024*8;
    selectedTrait.captureSourceId       = 111222;

    ret = m_recvSession->Subscribe(selectedTrait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    // Active Policy request info
    ActiveSpeakerRequestTrait activeTrait;
    activeTrait.subsessionChannelId   = 3;
    activeTrait.sourceId              = SIMULCAST_SOURCE_ID_2;
    activeTrait.bitrate               = 64*1024*8;
    activeTrait.priority              = 12;
    activeTrait.groupingAdjacencyId   = 1;
    activeTrait.duplicationFlag       = false;

    ret = m_recvSession->Subscribe(activeTrait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));


    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    // ------------1.Unsubscribe---------------
    ret = m_recvSession->Unsubscribe(noneTrait.subsessionChannelId);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    //----------------------------------------------------------------------------------
    // 1. check the request
    {
        const NonePolicyRequestList &nonePolicyRequests = m_simulcastResponseCallback->GetNonePolicyRequests();
        ASSERT_EQ(0, nonePolicyRequests.size());

        const SelectedSourceRequestList &selectedRequests = m_simulcastResponseCallback->GetSelectedSourceRequests();
        ASSERT_EQ(1, selectedRequests.size());
        for (SelectedSourceRequestList::const_iterator iter = selectedRequests.begin(); iter != selectedRequests.end(); ++iter) {
            Assert_EQ((*iter).first, selectedTrait);
            Assert_EQ((*iter).second, codecCap);
        }

        const ActiveSpeakerRequestList &activeRequests = m_simulcastResponseCallback->GetActiveSpeaderRequests();
        ASSERT_EQ(1, activeRequests.size());
        for (ActiveSpeakerRequestList::const_iterator iter = activeRequests.begin(); iter != activeRequests.end(); ++iter) {
            Assert_EQ((*iter).first, activeTrait);
            Assert_EQ((*iter).second, codecCap);
        }
    }

    // ------------2.Unsubscribe---------------
    ret = m_recvSession->Unsubscribe(selectedTrait.subsessionChannelId);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    //----------------------------------------------------------------------------------
    // 2. check the request
    {
        const NonePolicyRequestList &nonePolicyRequests = m_simulcastResponseCallback->GetNonePolicyRequests();
        ASSERT_EQ(0, nonePolicyRequests.size());

        const SelectedSourceRequestList &selectedRequests = m_simulcastResponseCallback->GetSelectedSourceRequests();
        ASSERT_EQ(0, selectedRequests.size());

        const ActiveSpeakerRequestList &activeRequests = m_simulcastResponseCallback->GetActiveSpeaderRequests();
        ASSERT_EQ(1, activeRequests.size());
        for (ActiveSpeakerRequestList::const_iterator iter = activeRequests.begin(); iter != activeRequests.end(); ++iter) {
            Assert_EQ((*iter).first, activeTrait);
            Assert_EQ((*iter).second, codecCap);
        }
    }

    // ------------3.Unsubscribe---------------
    ret = m_recvSession->Unsubscribe(activeTrait.subsessionChannelId);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    //----------------------------------------------------------------------------------
    // 3. check the request
    {
        const NonePolicyRequestList &nonePolicyRequests = m_simulcastResponseCallback->GetNonePolicyRequests();
        ASSERT_EQ(0, nonePolicyRequests.size());

        const SelectedSourceRequestList &selectedRequests = m_simulcastResponseCallback->GetSelectedSourceRequests();
        ASSERT_EQ(0, selectedRequests.size());

        const ActiveSpeakerRequestList &activeRequests = m_simulcastResponseCallback->GetActiveSpeaderRequests();
        ASSERT_EQ(0, activeRequests.size());
    }
}


TEST_F(CSimulcastRequestTest, Multiple_Request_Test)
{
    CScopedTracer test_info;

    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    vector<SubsessionChannelRequest> requests;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    H264CodecCapability &h264   = codecCap.h264CodecCap;
    h264.payloadType            = PAYLOAD_TYPE_VIDEO_TEST;
    h264.maxMBPS                = 1024;
    h264.maxFS                  = 240;
    h264.maxFPS                 = 30;
    h264.temporalLayerCount     = 3;
    h264.temporalLayers[0]      = 750;
    h264.temporalLayers[1]      = 1500;
    h264.temporalLayers[2]      = 3000;

    int32_t ret = 0;

    // None Policy request info
    NonePolicyRequestTrait noneTrait;
    noneTrait.subsessionChannelId   = 1;
    noneTrait.sourceId              = SIMULCAST_SOURCE_ID_1;
    noneTrait.bitrate               = 64*1024*8;

    SubsessionChannelRequest request;
    request.policyType      = POLICY_TYPE_NONE_POLICY;
    request.nonePolicyTrait = noneTrait;
    request.codecCapability = codecCap;
    requests.push_back(request);


    // Selected Source request info
    SelectedSourceRequestTrait selectedTrait;
    selectedTrait.subsessionChannelId   = 2;
    selectedTrait.sourceId              = SIMULCAST_SOURCE_ID_1;
    selectedTrait.bitrate               = 64*1024*8;
    selectedTrait.captureSourceId       = 111222;

    request.policyType              = POLICY_RECEIVER_SELECTED_SOURCE;
    request.selectedSourceTrait     = selectedTrait;
    request.codecCapability         = codecCap;
    requests.push_back(request);

    // Active Policy request info
    ActiveSpeakerRequestTrait activeTrait;
    activeTrait.subsessionChannelId   = 3;
    activeTrait.sourceId              = SIMULCAST_SOURCE_ID_2;
    activeTrait.bitrate               = 64*1024*8;
    activeTrait.priority              = 12;
    activeTrait.groupingAdjacencyId   = 1;
    activeTrait.duplicationFlag       = true;

    request.policyType              = POLICY_TYPE_ACTIVE_SPEAKER;
    request.activeSpeakerTrait      = activeTrait;
    request.codecCapability         = codecCap;
    requests.push_back(request);


    ret = m_recvSession->Subscribe(&requests[0], requests.size());
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    //----------------------------------------------------------------------------------
    // check the request
    const NonePolicyRequestList &nonePolicyRequests = m_simulcastResponseCallback->GetNonePolicyRequests();
    ASSERT_EQ(1, nonePolicyRequests.size());
    for (NonePolicyRequestList::const_iterator iter = nonePolicyRequests.begin(); iter != nonePolicyRequests.end(); ++iter) {
        Assert_EQ((*iter).first, noneTrait);
        Assert_EQ((*iter).second, codecCap);
    }

    const SelectedSourceRequestList &selectedRequests = m_simulcastResponseCallback->GetSelectedSourceRequests();
    ASSERT_EQ(1, selectedRequests.size());
    for (SelectedSourceRequestList::const_iterator iter = selectedRequests.begin(); iter != selectedRequests.end(); ++iter) {
        Assert_EQ((*iter).first, selectedTrait);
        Assert_EQ((*iter).second, codecCap);
    }

    const ActiveSpeakerRequestList &activeRequests = m_simulcastResponseCallback->GetActiveSpeaderRequests();
    ASSERT_EQ(1, activeRequests.size());
    for (ActiveSpeakerRequestList::const_iterator iter = activeRequests.begin(); iter != activeRequests.end(); ++iter) {
        Assert_EQ((*iter).first, activeTrait);
        Assert_EQ((*iter).second, codecCap);
    }

}

//-----------------------------Active Speaker and Send RTP with right VID----------
#define SUBSESSION_CHANNEL_ID_1 6
#define SUBSESSION_CHANNEL_ID_2 8
TEST_F(CSimulcastRequestTest, Audio_Active_Speaker_Request_and_Send_RTP)
{
    CScopedTracer test_info;

    // initailize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, false);
    BindSenderReceiver();

    VidVector vids;
    vids.push_back(SUBSESSION_CHANNEL_ID_1);

    // request info
    ActiveSpeakerRequestTrait trait;
    trait.subsessionChannelId   = SUBSESSION_CHANNEL_ID_1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 32*1024*8;
    trait.priority              = 7;
    trait.groupingAdjacencyId   = 4;
    trait.duplicationFlag       = true;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_COMMON;
    codecCap.commonCodecCap.payloadType = PAYLOAD_TYPE_AUDIO_TEST;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    // send a voip packet with request codecType
    WRTPMediaData *voipDate = CreateVoIPData(133, 234, 57, 1, CODEC_TYPE_AUDIO_TEST);
    uint8_t vid = trait.subsessionChannelId;
    voipDate->vidCount = 1;
    voipDate->vidArray = &vid;

    ret = m_sendChannel->SendMediaData(voipDate);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (nullptr != m_forwardTransport->GetRTPMessageBlock()));
    EXPECT_TRUE(nullptr != m_forwardTransport->GetRTPMessageBlock());

    CCmMessageBlock *mb = m_forwardTransport->GetRTPMessageBlock();
    CheckVIDs(mb, vids);
}

TEST_F(CSimulcastRequestTest, Audio_Active_Speaker_Request_and_Send_RTP_with_wrong_codecType)
{
    CScopedTracer test_info;

    // initailize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, false);
    BindSenderReceiver();

    // Add by Joyce,this codec type used for test unsubscribed codec type
    int32_t ret = m_sendSession->RegisterPayloadType(CODEC_TYPE_2, PAYLOAD_TYPE_2, CLOCKRATE_2);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);
    ret = m_recvSession->RegisterPayloadType(CODEC_TYPE_2, PAYLOAD_TYPE_2, CLOCKRATE_2);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);

    // request info
    ActiveSpeakerRequestTrait trait;
    trait.subsessionChannelId   = SUBSESSION_CHANNEL_ID_1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 32*1024*8;
    trait.priority              = 7;
    trait.groupingAdjacencyId   = 4;
    trait.duplicationFlag       = false;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_COMMON;
    codecCap.commonCodecCap.payloadType = PAYLOAD_TYPE_AUDIO_TEST;

    ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    // send a voip packet with request codecType
    WRTPMediaData *voipDate = CreateVoIPData(133, 234, 57, 1, CODEC_TYPE_2);
    ret = m_sendChannel->SendMediaData(voipDate);
    ASSERT_NE(WRTP_ERR_NOERR, ret);
}

TEST_F(CSimulcastRequestTest, Video_Active_Speaker_Request_and_Send_RTP)
{
    CScopedTracer test_info;

    // initailize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    VidVector vids;
    vids.push_back(SUBSESSION_CHANNEL_ID_1);
    // request info
    ActiveSpeakerRequestTrait trait;
    trait.subsessionChannelId   = SUBSESSION_CHANNEL_ID_1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 32*1024*8;
    trait.priority              = 7;
    trait.groupingAdjacencyId   = 4;
    trait.duplicationFlag       = false;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    codecCap.h264CodecCap.maxMBPS = 8000;
    codecCap.h264CodecCap.maxFS   = 60*4*4*4;
    codecCap.h264CodecCap.maxFPS  = (uint16_t)(8*1024*8);
    codecCap.h264CodecCap.payloadType = PAYLOAD_TYPE_VIDEO_TEST;
    codecCap.h264CodecCap.temporalLayerCount = 0;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    // send a video packet with request codecType
    WRTPMediaData *videoDate = CreateVideoData(133, 234, 57, 2, 1, CODEC_TYPE_VIDEO_TEST);
    uint8_t vid = trait.subsessionChannelId;
    videoDate->vidCount = 1;
    videoDate->vidArray = &vid;

    //::SleepMs(100);
    ret = m_sendChannel->SendMediaData(videoDate);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);
    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    //WRTP_TEST_WAIT_UNTIL(10, 30, (nullptr != m_forwardTransport->GetRTPMessageBlock()));
    EXPECT_TRUE(nullptr != m_forwardTransport->GetRTPMessageBlock());

    CCmMessageBlock *mb = m_forwardTransport->GetRTPMessageBlock();
    CheckVIDs(mb, vids);
}

TEST_F(CSimulcastRequestTest, Video_Active_Speaker_Request_and_Send_RTP_with_wrong_codecType)
{
    CScopedTracer test_info;

    // initailize the session/channel
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    // Add by Joyce,this codec type used for test unsubscribed codec type
    int32_t ret = m_sendSession->RegisterPayloadType(CODEC_TYPE_2, PAYLOAD_TYPE_2, CLOCKRATE_2);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);
    ret = m_recvSession->RegisterPayloadType(CODEC_TYPE_2, PAYLOAD_TYPE_2, CLOCKRATE_2);
    ASSERT_EQ(WRTP_ERR_NOERR, ret);

    // request info
    ActiveSpeakerRequestTrait trait;
    trait.subsessionChannelId   = SUBSESSION_CHANNEL_ID_1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 32*1024*8;
    trait.priority              = 7;
    trait.groupingAdjacencyId   = 4;
    trait.duplicationFlag       = false;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    codecCap.h264CodecCap.maxMBPS = 8000;
    codecCap.h264CodecCap.maxFS   = 60*4*4*4;
    codecCap.h264CodecCap.maxFPS  = (uint16_t)(8*1024*8);
    codecCap.h264CodecCap.payloadType = PAYLOAD_TYPE_VIDEO_TEST;
    codecCap.h264CodecCap.temporalLayerCount = 0;

    ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    // send a video packet with request codecType
    WRTPMediaData *videoData = CreateVideoData(133, 234, 57, 2, 1, CODEC_TYPE_2);
    ret = m_sendChannel->SendMediaData(videoData);
    ASSERT_NE(WRTP_ERR_NOERR, ret);
}

//-----------------------------Selected Receiver and Send RTP with right VID----------
#define CAPTURE_ID_1 8749
TEST_F(CSimulcastRequestTest, Audio_Selected_Receiver_Request_and_Send_RTP)
{
    CScopedTracer test_info;

    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, false);
    BindSenderReceiver();

    VidVector vids;
    vids.push_back(SUBSESSION_CHANNEL_ID_1);

    SelectedSourceRequestTrait trait;
    trait.subsessionChannelId   = SUBSESSION_CHANNEL_ID_1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 32*1024*8;
    trait.captureSourceId       = CAPTURE_ID_1;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_COMMON;
    codecCap.commonCodecCap.payloadType = PAYLOAD_TYPE_AUDIO_TEST;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    WRTPMediaData *voipDate = CreateVoIPData(122, 234, 57, 1, CODEC_TYPE_AUDIO_TEST);
    uint8_t vid = trait.subsessionChannelId;
    voipDate->vidCount = 1;
    voipDate->vidArray = &vid;

    ret = m_sendChannel->SendMediaData(voipDate);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);
    //WRTP_TEST_WAIT_UNTIL(10, 30, (nullptr != m_forwardTransport->GetRTPMessageBlock()));
    EXPECT_TRUE(nullptr != m_forwardTransport->GetRTPMessageBlock());

    CCmMessageBlock *mb = m_forwardTransport->GetRTPMessageBlock();
    CheckVIDs(mb, vids);
}

TEST_F(CSimulcastRequestTest, Audio_Selected_Receiver_Request_and_Send_RTP_with_wrong_codecType)
{
    CScopedTracer test_info;

    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VOIP, false);
    BindSenderReceiver();

    SelectedSourceRequestTrait trait;
    trait.subsessionChannelId   = SUBSESSION_CHANNEL_ID_1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 32*1024*8;
    trait.captureSourceId       = CAPTURE_ID_1;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_COMMON;
    codecCap.commonCodecCap.payloadType = PAYLOAD_TYPE_AUDIO_TEST;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    WRTPMediaData *voipDate = CreateVoIPData(122, 234, 57, 1, CODEC_TYPE_2);
    ret = m_sendChannel->SendMediaData(voipDate);
    ASSERT_NE(WRTP_ERR_NOERR, ret);
}

TEST_F(CSimulcastRequestTest, Video_Selected_Receiver_Request_and_Send_RTP)
{
    CScopedTracer test_info;

    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    VidVector vids;
    vids.push_back(SUBSESSION_CHANNEL_ID_1);

    SelectedSourceRequestTrait trait;
    trait.subsessionChannelId   = SUBSESSION_CHANNEL_ID_1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 32*1024*8;
    trait.captureSourceId       = CAPTURE_ID_1;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    codecCap.h264CodecCap.maxMBPS = 8000;
    codecCap.h264CodecCap.maxFS   = 60*4*4*4;
    codecCap.h264CodecCap.maxFPS  = (uint16_t)(8*1024*8);
    codecCap.h264CodecCap.payloadType = PAYLOAD_TYPE_VIDEO_TEST;
    codecCap.h264CodecCap.temporalLayerCount = 0;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    WRTPMediaData *videoDate = CreateVideoData(122, 234, 57, 2, 1, CODEC_TYPE_VIDEO_TEST);
    uint8_t vid = trait.subsessionChannelId;
    videoDate->vidCount = 1;
    videoDate->vidArray = &vid;

    ret = m_sendChannel->SendMediaData(videoDate);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));
    WRTP_TEST_TRIGGER_ON_TIMER(100, 1);

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);
    //WRTP_TEST_WAIT_UNTIL(10, 30, (nullptr != m_forwardTransport->GetRTPMessageBlock()));
    EXPECT_TRUE(nullptr != m_forwardTransport->GetRTPMessageBlock());

    CCmMessageBlock *mb = m_forwardTransport->GetRTPMessageBlock();
    CheckVIDs(mb, vids);
}

TEST_F(CSimulcastRequestTest, Video_Selected_Receiver_Request_and_Send_RTP_with_wrong_codecType)
{
    CScopedTracer test_info;

    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    VidVector vids;
    vids.push_back(SUBSESSION_CHANNEL_ID_1);

    SelectedSourceRequestTrait trait;
    trait.subsessionChannelId   = SUBSESSION_CHANNEL_ID_1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 32*1024*8;
    trait.captureSourceId       = CAPTURE_ID_1;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    codecCap.h264CodecCap.maxMBPS = 8000;
    codecCap.h264CodecCap.maxFS   = 60*4*4*4;
    codecCap.h264CodecCap.maxFPS  = (uint16_t)(8*1024*8);
    codecCap.h264CodecCap.payloadType = PAYLOAD_TYPE_VIDEO_TEST;
    codecCap.h264CodecCap.temporalLayerCount = 0;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    WRTPMediaData *videoDate = CreateVideoData(122, 234, 57, 2, 1, CODEC_TYPE_2);
    ret = m_sendChannel->SendMediaData(videoDate);
    ASSERT_NE(WRTP_ERR_NOERR, ret);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Simulcast Response Callback             Simulcast Request Callback
//          |                                             |
//      RTP Sender                                  RTP Receiver
//          |                                             |
//          |----RTP/SCA---->forward transport------>-----|
//          |-------<------backward transport<-SCR/SCAACK-|
/////////////////////////////////////////////////////////////////////////

//---------------------test retransmission logic-----------------------------------
TEST_F(CSimulcastRequestTest, SCR_Retransmission_Not_Received_SCA)
{
    CScopedTracer test_info;

    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    m_forwardTransport->SetFilter(RTCP_SCA);//disable reverse transport to send sca

    VidVector vids;
    vids.push_back(SUBSESSION_CHANNEL_ID_1);

    SelectedSourceRequestTrait trait;
    trait.subsessionChannelId   = SUBSESSION_CHANNEL_ID_1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 32*1024*8;
    trait.captureSourceId       = CAPTURE_ID_1;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    codecCap.h264CodecCap.maxMBPS = 8000;
    codecCap.h264CodecCap.maxFS   = 60*4*4*4;
    codecCap.h264CodecCap.maxFPS  = (uint16_t)(8*1024*8);
    codecCap.h264CodecCap.payloadType = PAYLOAD_TYPE_VIDEO_TEST;
    codecCap.h264CodecCap.temporalLayerCount = 0;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL*2);
    WRTP_TEST_TRIGGER_ON_TIMER(NETWORK_TRANSMISSION_INTERVAL, 2);

    EXPECT_LT(1, m_reverseTransport->GetRTCPSentCount(RTCP_SCR));
}

TEST_F(CSimulcastRequestTest, SCR_Retransmission_Received_right_SCA_Then_Stop_Retrans)
{
    CScopedTracer test_info;

    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    m_forwardTransport->SetFilter(RTCP_SCA);//disable forward transport to send sca

    VidVector vids;
    vids.push_back(SUBSESSION_CHANNEL_ID_1);

    SelectedSourceRequestTrait trait;
    trait.subsessionChannelId   = SUBSESSION_CHANNEL_ID_1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_2;
    trait.bitrate               = 32*1024*8;
    trait.captureSourceId       = CAPTURE_ID_1;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    codecCap.h264CodecCap.maxMBPS = 8000;
    codecCap.h264CodecCap.maxFS   = 60*4*4*4;
    codecCap.h264CodecCap.maxFPS  = (uint16_t)(8*1024*8);
    codecCap.h264CodecCap.payloadType = PAYLOAD_TYPE_VIDEO_TEST;
    codecCap.h264CodecCap.temporalLayerCount = 0;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL*2);
    WRTP_TEST_TRIGGER_ON_TIMER(NETWORK_TRANSMISSION_INTERVAL, 2);

    EXPECT_LT(1, m_reverseTransport->GetRTCPSentCount(RTCP_SCR));
    m_forwardTransport->ResetFilter();
    SendSCAAnnounce(false);
    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL*2);
    WRTP_TEST_TRIGGER_ON_TIMER(NETWORK_TRANSMISSION_INTERVAL, 2);

    uint32_t scrSentCount = m_reverseTransport->GetRTCPSentCount(RTCP_SCR);
    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL*2);
    WRTP_TEST_TRIGGER_ON_TIMER(NETWORK_TRANSMISSION_INTERVAL, 2);

    EXPECT_EQ(scrSentCount, m_reverseTransport->GetRTCPSentCount(RTCP_SCR));
}

TEST_F(CSimulcastRequestTest, SCR_Retransmission_Received_wrong_SCA_Then_Keep_Retrans)
{
    CScopedTracer test_info;

    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    m_forwardTransport->SetFilter(RTCP_SCA);//disable forward transport to send sca

    VidVector vids;
    vids.push_back(SUBSESSION_CHANNEL_ID_1);

    SelectedSourceRequestTrait trait;
    trait.subsessionChannelId   = SUBSESSION_CHANNEL_ID_1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_2;
    trait.bitrate               = 32*1024*8;
    trait.captureSourceId       = CAPTURE_ID_1;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    codecCap.h264CodecCap.maxMBPS = 8000;
    codecCap.h264CodecCap.maxFS   = 60*4*4*4;
    codecCap.h264CodecCap.maxFPS  = (uint16_t)(8*1024*8);
    codecCap.h264CodecCap.payloadType = PAYLOAD_TYPE_VIDEO_TEST;
    codecCap.h264CodecCap.temporalLayerCount = 0;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL*2);
    WRTP_TEST_TRIGGER_ON_TIMER(NETWORK_TRANSMISSION_INTERVAL, 2);

    EXPECT_LT(1, m_reverseTransport->GetRTCPSentCount(RTCP_SCR));

    //send wrong sca
    SendSCAAnnounce(false);
    CCmMessageBlock *rtcpMb = m_forwardTransport->GetRTCPMessageBlock();
    ASSERT_TRUE(rtcpMb != nullptr);
    ChangeSCASequenceNumber(rtcpMb, 1);
    IRTPMediaPackage *package = nullptr;
    TransformMessageBlockToRTPMediaPackage(&package, rtcpMb);
    m_recvSession->ReceiveRTCPPacket(package);
    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);
    WRTP_TEST_TRIGGER_ON_TIMER(NETWORK_TRANSMISSION_INTERVAL, 1);

    uint32_t scrSentCount = m_reverseTransport->GetRTCPSentCount(RTCP_SCR);
    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL*2);
    WRTP_TEST_TRIGGER_ON_TIMER(NETWORK_TRANSMISSION_INTERVAL, 2);

    EXPECT_LT(scrSentCount, m_reverseTransport->GetRTCPSentCount(RTCP_SCR));
    EXPECT_TRUE(!m_simulcastRequestCallback->IsCallback());

}

TEST_F(CSimulcastRequestTest, SCA_Retransmission_Not_Received_SCAACK)
{
    CScopedTracer test_info;

    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    CRTPSessionClient *sessionClient = (CRTPSessionClient *)(m_sendSession);
    CSimulcastResponserSharedPtr responser = (sessionClient->GetSessionContext()->GetSimulcastResponser()).lock();
    ASSERT_TRUE(nullptr !=responser);

    m_reverseTransport->SetFilter(RTCP_SCA_ACK);//disable reverse transport to send sca-ack

    VidVector vids;
    vids.push_back(SUBSESSION_CHANNEL_ID_1);

    SelectedSourceRequestTrait trait;
    trait.subsessionChannelId   = SUBSESSION_CHANNEL_ID_1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 32*1024*8;
    trait.captureSourceId       = CAPTURE_ID_1;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    codecCap.h264CodecCap.maxMBPS = 8000;
    codecCap.h264CodecCap.maxFS   = 60*4*4*4;
    codecCap.h264CodecCap.maxFPS  = (uint16_t)(8*1024*8);
    codecCap.h264CodecCap.payloadType = PAYLOAD_TYPE_VIDEO_TEST;
    codecCap.h264CodecCap.temporalLayerCount = 0;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    SendSCAAnnounce(true);

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL*4);
    //ACmThread *thread = ::GetThread(TT_MAIN);
    //CCmTimerQueueBase *timer_queue = (CCmTimerQueueBase *)thread->GetTimerQueue();
    //timer_queue->CheckExpire();

    WRTP_TEST_TRIGGER_ON_TIMER(NETWORK_TRANSMISSION_INTERVAL, 4);


    EXPECT_LT(1, m_forwardTransport->GetRTCPSentCount(RTCP_SCA));

}

TEST_F(CSimulcastRequestTest, SCA_Retransmission_Received_right_SCAACK_Then_Stop_Retrans)
{
    CScopedTracer test_info;

    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    CRTPSessionClient *sessionClient = (CRTPSessionClient *)(m_sendSession);
    CSimulcastResponserSharedPtr responser = (sessionClient->GetSessionContext()->GetSimulcastResponser()).lock();
    ASSERT_TRUE(nullptr !=responser);

    m_reverseTransport->SetFilter(RTCP_SCA_ACK);//disable reverse transport to send sca-ack

    VidVector vids;
    vids.push_back(SUBSESSION_CHANNEL_ID_1);

    SelectedSourceRequestTrait trait;
    trait.subsessionChannelId   = SUBSESSION_CHANNEL_ID_1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 32*1024*8;
    trait.captureSourceId       = CAPTURE_ID_1;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    codecCap.h264CodecCap.maxMBPS = 8000;
    codecCap.h264CodecCap.maxFS   = 60*4*4*4;
    codecCap.h264CodecCap.maxFPS  = (uint16_t)(8*1024*8);
    codecCap.h264CodecCap.payloadType = PAYLOAD_TYPE_VIDEO_TEST;
    codecCap.h264CodecCap.temporalLayerCount = 0;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    SendSCAAnnounce(true);

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL*2);
    //ACmThread *thread = ::GetThread(TT_MAIN);
    //CCmTimerQueueBase *timer_queue = (CCmTimerQueueBase *)thread->GetTimerQueue();
    //timer_queue->CheckExpire();
    WRTP_TEST_TRIGGER_ON_TIMER(NETWORK_TRANSMISSION_INTERVAL, 2);

    EXPECT_LT(1, m_forwardTransport->GetRTCPSentCount(RTCP_SCA));
    m_reverseTransport->ResetFilter();

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL*2);
    //timer_queue->CheckExpire();
    WRTP_TEST_TRIGGER_ON_TIMER(NETWORK_TRANSMISSION_INTERVAL, 2);

    uint32_t scaSentCount = m_forwardTransport->GetRTCPSentCount(RTCP_SCA);
    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL*2);

    //timer_queue->CheckExpire();
    WRTP_TEST_TRIGGER_ON_TIMER(NETWORK_TRANSMISSION_INTERVAL, 2);
    EXPECT_EQ(scaSentCount, m_forwardTransport->GetRTCPSentCount(RTCP_SCA));

}

TEST_F(CSimulcastRequestTest, SCA_Retransmission_Received_wrong_SCAACK_Then_Keep_Retrans)
{
    CScopedTracer test_info;

    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, true);
    InitSessionChannel(RTP_SESSION_WEBEX_VIDEO, false);
    BindSenderReceiver();

    CRTPSessionClient *sessionClient = (CRTPSessionClient *)(m_sendSession);
    CSimulcastResponserSharedPtr responser = (sessionClient->GetSessionContext()->GetSimulcastResponser()).lock();
    ASSERT_TRUE(nullptr !=responser);

    m_reverseTransport->SetFilter(RTCP_SCA_ACK);//disable reverse transport to send sca-ack

    VidVector vids;
    vids.push_back(SUBSESSION_CHANNEL_ID_1);

    SelectedSourceRequestTrait trait;
    trait.subsessionChannelId   = SUBSESSION_CHANNEL_ID_1;
    trait.sourceId              = SIMULCAST_SOURCE_ID_1;
    trait.bitrate               = 32*1024*8;
    trait.captureSourceId       = CAPTURE_ID_1;

    CodecCapability codecCap;
    codecCap.type               = REQUEST_CODEC_TYPE_H264;
    codecCap.h264CodecCap.maxMBPS = 8000;
    codecCap.h264CodecCap.maxFS   = 60*4*4*4;
    codecCap.h264CodecCap.maxFPS  = (uint16_t)(8*1024*8);
    codecCap.h264CodecCap.payloadType = PAYLOAD_TYPE_VIDEO_TEST;
    codecCap.h264CodecCap.temporalLayerCount = 0;

    int32_t ret = m_recvSession->Subscribe(trait, codecCap);
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    ret = m_recvSession->SendRequest();
    ASSERT_TRUE(WRTP_SUCCEEDED(ret));

    SendSCAAnnounce(true);

    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL*2);

    //WRTP_TEST_TRIGGER_ON_TIMER(NETWORK_TRANSMISSION_INTERVAL, 2);

    uint32_t count = m_forwardTransport->GetRTCPSentCount(RTCP_SCA);

    EXPECT_EQ(1, m_forwardTransport->GetRTCPSentCount(RTCP_SCA));

    //send wrong sca-ack
    CCmMessageBlock *rtcpMb = m_reverseTransport->GetRTCPMessageBlock();
    ASSERT_TRUE(rtcpMb != nullptr);
    ChangeSCAACKSequenceNumber(rtcpMb, 1);
    IRTPMediaPackage *package = nullptr;
    TransformMessageBlockToRTPMediaPackage(&package, rtcpMb);
    m_recvSession->ReceiveRTCPPacket(package);
    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL);

    WRTP_TEST_TRIGGER_ON_TIMER(NETWORK_TRANSMISSION_INTERVAL, 1);

    uint32_t scaSentCount = m_forwardTransport->GetRTCPSentCount(RTCP_SCA);
    //::SleepMs(NETWORK_TRANSMISSION_INTERVAL*2);

    WRTP_TEST_TRIGGER_ON_TIMER(NETWORK_TRANSMISSION_INTERVAL, 2);

    count = m_forwardTransport->GetRTCPSentCount(RTCP_SCA);
    EXPECT_LT(scaSentCount, m_forwardTransport->GetRTCPSentCount(RTCP_SCA));

    //    printf("m_reverseTransport->GetRTCPSentCount(RTCP_SCR):%u\n",m_reverseTransport->GetRTCPSentCount(RTCP_SCR));
    //    printf("m_reverseTransport->GetRTCPSentCount(RTCP_SCA):%u\n",m_reverseTransport->GetRTCPSentCount(RTCP_SCA));
    //    printf("m_reverseTransport->GetRTCPSentCount(RTCP_SCA_ACK):%u\n",m_reverseTransport->GetRTCPSentCount(RTCP_SCA_ACK));
    //    printf("m_forwardTransport->GetRTCPSentCount(RTCP_SCR):%u\n",m_forwardTransport->GetRTCPSentCount(RTCP_SCR));
    //    printf("m_forwardTransport->GetRTCPSentCount(RTCP_SCA):%u\n",m_forwardTransport->GetRTCPSentCount(RTCP_SCA));
    //    printf("m_forwardTransport->GetRTCPSentCount(RTCP_SCA_ACK):%u\n",m_forwardTransport->GetRTCPSentCount(RTCP_SCA_ACK));

}