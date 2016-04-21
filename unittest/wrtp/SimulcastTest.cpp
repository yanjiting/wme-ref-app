#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "simulcastrequestmsg.h"
#include "testutil.h"
#include <algorithm>
#include <stdio.h>
#define private public
using namespace wrtp;

class CSubsessionChannelRequestMsgTest:public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }

    //SCR format
    struct RTCPFCI {
        uint32_t identifier;
        uint16_t msgTypeVersion;
        uint16_t seqNum;
    };

    struct RequestHeader {
        uint8_t vid;
        uint8_t sourceId;
        uint16_t requestLen;
        uint32_t bitrate;
        uint16_t policyType;
        uint16_t policyInfoLen;
    };

    //policy specific info
    struct PolicySpecificInfoActive {
        uint8_t priority;
        uint8_t groupAdj;
        bool  dupFlag;
        //uint16_t reserved;
    };

    struct PolicySpecificInfoSelect {
        uint32_t csi;
    };

    //payload section
    struct PayloadSectionHeader {
        uint8_t payloadType;
        uint8_t reserved;
        uint16_t payloadInfoLen;
    };

    struct PayloadSpecificInfo {
        uint32_t maxMBPS;
        uint16_t maxFS;
        uint16_t maxFPS;
        uint8_t temporalLayerNum;
        uint16_t temporalLayer[MAX_TEMPORAL_LAYERS];
    };

    struct PayloadSection {
        PayloadSectionHeader header;
        PayloadSpecificInfo  specificInfo;
        PayloadSection       *next;
    };

    union SpecificInfo {
        PolicySpecificInfoActive activeSpecificInfo;
        PolicySpecificInfoSelect selectSpecificInfo;
    };
    struct RequestInfo {
        RequestHeader               requestHeader;
        SpecificInfo                *specificInfo;
        PayloadSection              *payloadSection;
        RequestInfo           *next;
    };

    struct SCRPacket {
        RTCPFCI             fci;
        RequestInfo   request;
    };

    void ReleaseMemory(RequestInfo *request)
    {
        RequestInfo *currentRequest = nullptr;
        while (nullptr != request) {
            if (nullptr != request->specificInfo) {
                delete request->specificInfo;
                request->specificInfo = nullptr;
            }
            if (nullptr != request->payloadSection) {
                delete request->payloadSection;
                request->payloadSection = nullptr;
            }
            request = request->next;
            if (nullptr != currentRequest) {
                delete currentRequest;
                currentRequest = nullptr;
            }
            currentRequest = request;


        }
    }

    void CompareDecodeRTCPFCI(RTCPFCI *fci, CSubsessionChannelRequestMsg *msg)
    {
        EXPECT_EQ((fci->msgTypeVersion)>>4, msg->GetFeedbackType());
        EXPECT_EQ(fci->seqNum, msg->GetSequenceNumber());

    }

    void CompareDecodeRTCPFCI(CSubsessionChannelRequestMsg *encodeMsg, CSubsessionChannelRequestMsg *decodeMsg)
    {
        EXPECT_EQ(encodeMsg->GetFeedbackType(), decodeMsg->GetFeedbackType());
        //EXPECT_EQ(encodeMsg->GetRecvSequenceNumber(), decodeMsg->GetRecvSequenceNumber());//Encode create a new seqNum
    }

    void CompareDecodeSCR(RequestInfo *request, CSubsessionChannelRequestMsg *msg, RequestIdPolicyMap &idPollicyMap)
    {
        PayloadSection    *payloadSection = nullptr;
        while (nullptr != request) {
            RequestIdPolicyMap::iterator itPayloadType = idPollicyMap.find(request->requestHeader.policyType);
            ASSERT_TRUE(itPayloadType != idPollicyMap.end());
            SCRRequestPolicyType policyType = itPayloadType->second;

            //check request header
            SimulcastRequestInfoMap::iterator itVid = msg->GetRequestInfoMap().find(request->requestHeader.vid);
            ASSERT_TRUE(itVid != msg->GetRequestInfoMap().end());
            CSubsessionChannelRequestBlockPtr requestBlockPtr = itVid->second;
            EXPECT_EQ(request->requestHeader.vid, requestBlockPtr->GetSubsessionChannelId());

            switch (policyType) {
                case wrtp::POLICY_TYPE_ACTIVE_SPEAKER: {
                    const ActiveSpeakerRequestTrait &activeTrait= requestBlockPtr->GetActiveSpeakerRequestTrait();
                    EXPECT_EQ(request->requestHeader.vid, activeTrait.subsessionChannelId);
                    EXPECT_EQ(request->requestHeader.sourceId, activeTrait.sourceId);
                    EXPECT_EQ(request->requestHeader.bitrate, activeTrait.bitrate);

                    PolicySpecificInfoActive *activeInfo = (PolicySpecificInfoActive *)request->specificInfo;
                    EXPECT_EQ(activeInfo->priority, activeTrait.priority);
                    EXPECT_EQ(activeInfo->groupAdj, activeTrait.groupingAdjacencyId);
                    EXPECT_EQ(activeInfo->dupFlag, activeTrait.duplicationFlag);
                    break;
                }
                case wrtp::POLICY_RECEIVER_SELECTED_SOURCE: {
                    const SelectedSourceRequestTrait &selectTrait = requestBlockPtr->GetSelectedSourceRequestTrait();
                    EXPECT_EQ(request->requestHeader.vid, selectTrait.subsessionChannelId);
                    EXPECT_EQ(request->requestHeader.sourceId, selectTrait.sourceId);
                    EXPECT_EQ(request->requestHeader.bitrate, selectTrait.bitrate);
                    PolicySpecificInfoSelect *selectInfo = (PolicySpecificInfoSelect *)request->specificInfo;
                    EXPECT_EQ(selectInfo->csi, selectTrait.captureSourceId);
                    break;
                }
                case wrtp::POLICY_TYPE_NONE_POLICY: {
                    const NonePolicyRequestTrait &nonePolicyTrait = requestBlockPtr->GetNonePolicyRequestTrait();
                    EXPECT_EQ(request->requestHeader.vid, nonePolicyTrait.subsessionChannelId);
                    EXPECT_EQ(request->requestHeader.sourceId, nonePolicyTrait.sourceId);
                    EXPECT_EQ(request->requestHeader.bitrate, nonePolicyTrait.bitrate);
                    break;
                }
                default:
                    break;
            }

            //payload section
            payloadSection = request->payloadSection;
            while (nullptr != payloadSection) {
                const CodecCapability &codecCapability = requestBlockPtr->GetCodecCapability();

                if (payloadSection->header.payloadInfoLen != 0) {
                    EXPECT_EQ(REQUEST_CODEC_TYPE_H264, codecCapability.type);
                    EXPECT_EQ(payloadSection->header.payloadType, codecCapability.h264CodecCap.payloadType);
                    EXPECT_EQ(payloadSection->specificInfo.maxMBPS, codecCapability.h264CodecCap.maxMBPS);
                    EXPECT_EQ(payloadSection->specificInfo.maxFS, codecCapability.h264CodecCap.maxFS);
                    EXPECT_EQ(payloadSection->specificInfo.maxFPS, codecCapability.h264CodecCap.maxFPS);
                    EXPECT_EQ(payloadSection->specificInfo.temporalLayerNum, codecCapability.h264CodecCap.temporalLayerCount);
                    for (int ii = 0; ii < payloadSection->specificInfo.temporalLayerNum; ++ii) {
                        EXPECT_EQ(payloadSection->specificInfo.temporalLayer[ii], codecCapability.h264CodecCap.temporalLayers[ii]);
                    }
                } else {
                    EXPECT_EQ(REQUEST_CODEC_TYPE_COMMON, codecCapability.type);
                    EXPECT_EQ(payloadSection->header.payloadType, codecCapability.commonCodecCap.payloadType);
                    EXPECT_EQ(0, codecCapability.h264CodecCap.maxMBPS);
                    EXPECT_EQ(0, codecCapability.h264CodecCap.maxFS);
                    EXPECT_EQ(0, codecCapability.h264CodecCap.maxFPS);
                    EXPECT_EQ(0, codecCapability.h264CodecCap.temporalLayerCount);
                    EXPECT_EQ(0, codecCapability.h264CodecCap.temporalLayerCount);
                    for (int ii = 0; ii < MAX_TEMPORAL_LAYERS; ++ii) {
                        EXPECT_EQ(0, codecCapability.h264CodecCap.temporalLayers[ii]);
                    }
                }
                payloadSection = payloadSection->next;
            }
            request = request->next;
        }
    }

    void CompareDecodeSCR(CSubsessionChannelRequestMsg *encodeMsg, CSubsessionChannelRequestMsg *decodeMsg)
    {
        for (SimulcastRequestInfoMap::iterator itEncode = encodeMsg->GetRequestInfoMap().begin(); itEncode != encodeMsg->GetRequestInfoMap().end(); ++itEncode) {
            SimulcastRequestInfoMap::iterator itDecode = decodeMsg->GetRequestInfoMap().find(itEncode->first);
            ASSERT_TRUE(itDecode != decodeMsg->GetRequestInfoMap().end());
            CSubsessionChannelRequestBlockPtr encodeBlockPtr = itEncode->second;
            CSubsessionChannelRequestBlockPtr decodeBlockPtr = itDecode->second;

            EXPECT_EQ(encodeBlockPtr->GetSubsessionChannelId(), decodeBlockPtr->GetSubsessionChannelId());
            ASSERT_EQ(encodeBlockPtr->GetPolicyType(), decodeBlockPtr->GetPolicyType());
            SCRRequestPolicyType policyType = encodeBlockPtr->GetPolicyType();

            switch (policyType) {
                case wrtp::POLICY_TYPE_ACTIVE_SPEAKER: {
                    const ActiveSpeakerRequestTrait &encodeActiveTrait = encodeBlockPtr->GetActiveSpeakerRequestTrait();
                    const ActiveSpeakerRequestTrait &decodeActiveTrait = decodeBlockPtr->GetActiveSpeakerRequestTrait();
                    EXPECT_EQ(encodeActiveTrait.subsessionChannelId, decodeActiveTrait.subsessionChannelId);
                    EXPECT_EQ(encodeActiveTrait.sourceId, decodeActiveTrait.sourceId);
                    EXPECT_EQ(encodeActiveTrait.bitrate, decodeActiveTrait.bitrate);
                    EXPECT_EQ(encodeActiveTrait.priority, decodeActiveTrait.priority);
                    EXPECT_EQ(encodeActiveTrait.groupingAdjacencyId, decodeActiveTrait.groupingAdjacencyId);
                    EXPECT_EQ(encodeActiveTrait.duplicationFlag, decodeActiveTrait.duplicationFlag);
                    break;
                }
                case wrtp::POLICY_RECEIVER_SELECTED_SOURCE: {
                    const SelectedSourceRequestTrait &encodeSelectTrait = encodeBlockPtr->GetSelectedSourceRequestTrait();
                    const SelectedSourceRequestTrait &decodeSelectTrait = decodeBlockPtr->GetSelectedSourceRequestTrait();
                    EXPECT_EQ(encodeSelectTrait.subsessionChannelId, decodeSelectTrait.subsessionChannelId);
                    EXPECT_EQ(encodeSelectTrait.sourceId, decodeSelectTrait.sourceId);
                    EXPECT_EQ(encodeSelectTrait.bitrate, decodeSelectTrait.bitrate);
                    EXPECT_EQ(encodeSelectTrait.captureSourceId, decodeSelectTrait.captureSourceId);
                    break;
                }
                case wrtp::POLICY_TYPE_NONE_POLICY: {
                    const NonePolicyRequestTrait &encodeNonePolicyTrait = encodeBlockPtr->GetNonePolicyRequestTrait();
                    const NonePolicyRequestTrait &decodeNonePolicyTrait = decodeBlockPtr->GetNonePolicyRequestTrait();
                    EXPECT_EQ(encodeNonePolicyTrait.subsessionChannelId, decodeNonePolicyTrait.subsessionChannelId);
                    EXPECT_EQ(encodeNonePolicyTrait.sourceId, decodeNonePolicyTrait.sourceId);
                    EXPECT_EQ(encodeNonePolicyTrait.bitrate, decodeNonePolicyTrait.bitrate);
                    break;
                }
                default:
                    break;
            }

            //payload section
            {
                const CodecCapability &encodeCodecCapability = encodeBlockPtr->GetCodecCapability();
                const CodecCapability &decodeCodecCapability = decodeBlockPtr->GetCodecCapability();

                EXPECT_EQ(encodeCodecCapability.type, decodeCodecCapability.type);
                switch (encodeCodecCapability.type) {
                    case wrtp::REQUEST_CODEC_TYPE_COMMON: {
                        EXPECT_EQ(encodeCodecCapability.commonCodecCap.payloadType, decodeCodecCapability.commonCodecCap.payloadType);
                        break;
                    }
                    case wrtp::REQUEST_CODEC_TYPE_H264: {
                        EXPECT_EQ(encodeCodecCapability.h264CodecCap.payloadType, decodeCodecCapability.h264CodecCap.payloadType);
                        EXPECT_EQ(encodeCodecCapability.h264CodecCap.maxMBPS, decodeCodecCapability.h264CodecCap.maxMBPS);
                        EXPECT_EQ(encodeCodecCapability.h264CodecCap.maxFS, decodeCodecCapability.h264CodecCap.maxFS);
                        EXPECT_EQ(encodeCodecCapability.h264CodecCap.maxFPS, decodeCodecCapability.h264CodecCap.maxFPS);
                        EXPECT_EQ(encodeCodecCapability.h264CodecCap.temporalLayerCount, decodeCodecCapability.h264CodecCap.temporalLayerCount);
                        for (int ii = 0; ii < encodeCodecCapability.h264CodecCap.
                                temporalLayerCount; ++ii) {
                            EXPECT_EQ(encodeCodecCapability.h264CodecCap.temporalLayers[ii], decodeCodecCapability.h264CodecCap.temporalLayers[ii]);
                        }
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    }

    void EncodeRTCPFCI(RTCPFCI *fci, CCmByteStreamNetwork &os)
    {
        os << fci->identifier;
        os << fci->msgTypeVersion;
        os << fci->seqNum;
        ASSERT_TRUE(os.IsGood() == TRUE);
    }
    void EncodeRequest(RequestInfo *request, CCmByteStreamNetwork &os, RequestIdPolicyMap &idPollicyMap)
    {
        PayloadSection    *payloadSection = nullptr;

        while (nullptr != request) {
            RequestIdPolicyMap::iterator it = idPollicyMap.find(request->requestHeader.policyType);
            ASSERT_TRUE(it != idPollicyMap.end());
            SCRRequestPolicyType policyType = it->second;

            os << request->requestHeader.vid;
            os << request->requestHeader.sourceId;
            os << request->requestHeader.requestLen;
            os << request->requestHeader.bitrate;
            os << request->requestHeader.policyType;
            os << request->requestHeader.policyInfoLen;

            switch (policyType) {
                case POLICY_TYPE_ACTIVE_SPEAKER: {
                    PolicySpecificInfoActive *activeInfo = (PolicySpecificInfoActive *)request->specificInfo;
                    os << activeInfo->priority;
                    os << activeInfo->groupAdj;
                    uint16_t dupReserved = 0;
                    if (activeInfo->dupFlag) {
                        dupReserved = 1 << 15;
                    }
                    os << dupReserved;
                    break;
                }
                case POLICY_RECEIVER_SELECTED_SOURCE: {
                    PolicySpecificInfoSelect *selectInfo = (PolicySpecificInfoSelect *)request->specificInfo;
                    os << (uint32_t)(selectInfo->csi);
                    break;
                }
                case POLICY_TYPE_NONE_POLICY: {
                    break;
                }
                default:
                    break;
            }

            //payload section
            payloadSection = request->payloadSection;
            while (nullptr != payloadSection) {
                os << payloadSection->header.payloadType;
                os << payloadSection->header.reserved;
                os << payloadSection->header.payloadInfoLen;
                if (payloadSection->header.payloadInfoLen != 0) {
                    os << payloadSection->specificInfo.maxMBPS;
                    os << payloadSection->specificInfo.maxFS;
                    os << payloadSection->specificInfo.maxFPS;
                    os << payloadSection->specificInfo.temporalLayerNum;
                    for (int layerNum = 0; layerNum < payloadSection->specificInfo.temporalLayerNum; ++layerNum) {
                        os << payloadSection->specificInfo.temporalLayer[layerNum];
                    }
                    uint8_t paddingLen = LENGTH_OF_PADDING(1+2*payloadSection->specificInfo.temporalLayerNum, 4);
                    for (int i=0; i<paddingLen; ++i) {
                        os << (uint8_t)0;
                    }
                }

                payloadSection = payloadSection->next;
            }

            request = request->next;
        }

        ASSERT_TRUE(os.IsGood() == TRUE);
    }


} ;

#define RTP_UT_STRUCT_INIT(x) ( memset( (x), 0, sizeof(*(x)) ) )

TEST_F(CSubsessionChannelRequestMsgTest, Decode_SCR_one_AS_request)
{
    CScopedTracer test_info;
    uint16_t packetLen = 8;
    RequestIdPolicyMap receiverIdPolicyMap;
    SimulcastSource_RequestIdPolicyMap simulcastReceiverIdPolicyMap;
    SCRPacket scrPacket;
    RTP_UT_STRUCT_INIT(&scrPacket);
    //fci header
    scrPacket.fci.identifier        = 'MSTR';
    scrPacket.fci.msgTypeVersion    = (0x01<<4)|(0x01);
    scrPacket.fci.seqNum            = 468;

    //++++++++++++++++++first active request+++++++++++++++++++++++
    RequestInfo *firstRequest = &scrPacket.request;
    //request header
    firstRequest->requestHeader.vid             = 19;
    firstRequest->requestHeader.sourceId        = 165;//simulcastSource_RequestIdPolicyMap
    firstRequest->requestHeader.bitrate         = 1004;
    firstRequest->requestHeader.policyType      = 204;//map <id, policy>
    uint16_t policyInfoLen                        = 4;
    firstRequest->requestHeader.policyInfoLen   = policyInfoLen;//common policy is 0, otherwise must be 4

    //specificInfo
    firstRequest->specificInfo = new SpecificInfo;
    RTP_UT_STRUCT_INIT(firstRequest->specificInfo);
    PolicySpecificInfoActive *activeInfo = (PolicySpecificInfoActive *)firstRequest->specificInfo;
    activeInfo->priority   = 10;
    activeInfo->groupAdj   = 5;
    activeInfo->dupFlag    = true;

    //payload section
    firstRequest->payloadSection = new PayloadSection;
    RTP_UT_STRUCT_INIT(firstRequest->payloadSection);
    firstRequest->payloadSection->header.payloadType             = 101;
    firstRequest->payloadSection->header.reserved                = 0;
    firstRequest->payloadSection->specificInfo.maxMBPS           = 100000;
    firstRequest->payloadSection->specificInfo.maxFS             = 30;
    firstRequest->payloadSection->specificInfo.maxFPS            = 45;
    firstRequest->payloadSection->specificInfo.temporalLayerNum  = 3;
    for (int ii = 0; ii < firstRequest->payloadSection->specificInfo.temporalLayerNum; ++ii) {
        firstRequest->payloadSection->specificInfo.temporalLayer[ii]  = ii+3;
    }

    //fill length
    uint16_t payloadSpecificLen = 1 + 2 * firstRequest->payloadSection->specificInfo.temporalLayerNum;
    payloadSpecificLen = LENGTH_OF_H264CCHEAD + payloadSpecificLen + 4 - payloadSpecificLen % 4;
    firstRequest->payloadSection->header.payloadInfoLen          = payloadSpecificLen;
    firstRequest->requestHeader.requestLen                       = LENGTH_OF_REQUEST_HEADER + policyInfoLen + LENGTH_OF_PAYLOAD_HEADER + payloadSpecificLen;
    packetLen += firstRequest->requestHeader.requestLen;
    firstRequest->payloadSection->next                           = nullptr;
    firstRequest->next                                           = nullptr;

    //Encode
    char buffer[1024] = {0};
    CCmMessageBlock mb(1024, (LPCSTR)buffer);
    CCmByteStreamNetwork os(mb);

    receiverIdPolicyMap.insert(make_pair(firstRequest->requestHeader.policyType, POLICY_TYPE_ACTIVE_SPEAKER));
    simulcastReceiverIdPolicyMap.insert(make_pair(firstRequest->requestHeader.sourceId, receiverIdPolicyMap));

    EncodeRTCPFCI(&scrPacket.fci, os);
    EncodeRequest(&scrPacket.request, os, receiverIdPolicyMap);

    //initail decoder
    CSubsessionChannelRequestMsg decoder(&simulcastReceiverIdPolicyMap);
    uint32_t result = decoder.DecodeFrom((const uint8_t *)buffer, packetLen);
    EXPECT_EQ(0, result);

    //check content
    CompareDecodeRTCPFCI(&scrPacket.fci, &decoder);
    CompareDecodeSCR(&scrPacket.request, &decoder, receiverIdPolicyMap);

    ReleaseMemory(&scrPacket.request);
}
#if 0
TEST_F(CSubsessionChannelRequestMsgTest, Decode_SCR_one_AS_request_two_payload_section)
{
    CScopedTracer test_info;
    uint16_t packetLen = 8;
    RequestIdPolicyMap receiverIdPolicyMap;
    SimulcastSource_RequestIdPolicyMap simulcastReceiverIdPolicyMap;
    SCRPacket scrPacket;
    RTP_UT_STRUCT_INIT(&SCRPacket);
    //fci header
    scrPacket.fci.identifier        = 'MSTR';
    scrPacket.fci.msgTypeVersion    = (0x01<<4)|(0x01);
    scrPacket.fci.seqNum            = 468;

    //++++++++++++++++++first active request+++++++++++++++++++++++
    RequestInfo *firstRequest = &scrPacket.request;
    //request header
    firstRequest->requestHeader.vid             = 19;
    firstRequest->requestHeader.sourceId        = 165;//simulcastSource_RequestIdPolicyMap
    firstRequest->requestHeader.bitrate         = 1004;
    firstRequest->requestHeader.policyType      = 204;//map <id, policy>
    uint16_t policyInfoLen                        = 4;
    firstRequest->requestHeader.policyInfoLen   = policyInfoLen;//common policy is 0, otherwise must be 4

    //specificInfo
    firstRequest->specificInfo = new SpecificInfo;
    RTP_UT_STRUCT_INIT(firstRequest->specificInfo);
    PolicySpecificInfoActive *activeInfo = (PolicySpecificInfoActive *)firstRequest->specificInfo;
    activeInfo->priority   = 10;
    activeInfo->groupAdj   = 5;
    activeInfo->reserved   = 0;//test !0

    //payload section
    firstRequest->payloadSection = new PayloadSection;
    RTP_UT_STRUCT_INIT();
    firstRequest->payloadSection->header.payloadType             = 101;
    firstRequest->payloadSection->header.reserved                = 0;
    firstRequest->payloadSection->specificInfo.maxMBPS           = 100000;
    firstRequest->payloadSection->specificInfo.maxFS             = 30;
    firstRequest->payloadSection->specificInfo.maxFPS            = 45;
    firstRequest->payloadSection->specificInfo.temporalLayerNum  = 3;
    for (int ii = 0; ii < firstRequest->payloadSection->specificInfo.temporalLayerNum; ++ii) {
        firstRequest->payloadSection->specificInfo.temporalLayer[ii]  = ii+3;
    }

    //second payload section
    firstRequest->payloadSection->next = new PayloadSection;
    RTP_UT_STRUCT_INIT();
    PayloadSection *payloadSection = firstRequest->payloadSection->next;
    payloadSection->header.payloadType = 98;
    payloadSection->header.reserved    = 0;
    payloadSection->specificInfo.maxMBPS = 2987;
    payloadSection->specificInfo.maxFS   = 22;
    payloadSection->specificInfo.maxFPS  = 8;
    payloadSection->specificInfo.temporalLayerNum  = 0;

    //fill length
    uint16_t payloadSpecificLen = 1 + 2 * firstRequest->payloadSection->specificInfo.temporalLayerNum;
    payloadSpecificLen = LENGTH_OF_H264CCHEAD + payloadSpecificLen + 4 - payloadSpecificLen % 4;
    firstRequest->payloadSection->header.payloadInfoLen          = payloadSpecificLen;
    payloadSection->header.payloadInfoLen                        = LENGTH_OF_H264CCHEAD + 4;
    firstRequest->requestHeader.requestLen                       = LENGTH_OF_REQUEST_HEADER + policyInfoLen + LENGTH_OF_PAYLOAD_HEADER + payloadSpecificLen + payloadSection->header.payloadInfoLen;
    packetLen += firstRequest->requestHeader.requestLen;
    firstRequest->payloadSection->next                           = nullptr;
    firstRequest->next                                           = nullptr;

    //Encode
    char buffer[1024] = {0};
    CCmMessageBlock mb(1024, (LPCSTR)buffer);
    CCmByteStreamNetwork os(mb);

    receiverIdPolicyMap.insert(make_pair(firstRequest->requestHeader.policyType, POLICY_TYPE_ACTIVE_SPEAKER));
    simulcastReceiverIdPolicyMap.insert(make_pair(firstRequest->requestHeader.sourceId, receiverIdPolicyMap));

    EncodeRTCPFCI(&scrPacket.fci, os);
    EncodeRequest(&scrPacket.request, os, receiverIdPolicyMap);

    //initail decoder
    CSubsessionChannelRequestMsg decoder(&simulcastReceiverIdPolicyMap);
    uint32_t result = decoder.DecodeFrom((const uint8_t *)buffer, packetLen);
    EXPECT_EQ(0, result);

    //check content
    CompareDecodeRTCPFCI(&scrPacket.fci, &decoder);
    CompareDecodeSCR(&scrPacket.request, &decoder, receiverIdPolicyMap);

    ReleaseMemory(&scrPacket.request);
}
#endif

TEST_F(CSubsessionChannelRequestMsgTest, Decode_SCR_AS_wrong_len)
{
    CScopedTracer test_info;
    uint16_t packetLen = 8;
    RequestIdPolicyMap receiverIdPolicyMap;
    SimulcastSource_RequestIdPolicyMap simulcastReceiverIdPolicyMap;
    SCRPacket scrPacket;
    RTP_UT_STRUCT_INIT(&scrPacket);
    //fci header
    scrPacket.fci.identifier        = 'MSTR';
    scrPacket.fci.msgTypeVersion    = (0x01<<4)|(0x01);
    scrPacket.fci.seqNum            = 468;

    //++++++++++++++++++first active request+++++++++++++++++++++++
    RequestInfo *firstRequest = &scrPacket.request;
    //request header
    firstRequest->requestHeader.vid             = 19;
    firstRequest->requestHeader.sourceId        = 165;//simulcastSource_RequestIdPolicyMap
    firstRequest->requestHeader.bitrate         = 1004;
    firstRequest->requestHeader.policyType      = 204;//map <id, policy>
    uint16_t policyInfoLen                        = 4;
    firstRequest->requestHeader.policyInfoLen   = policyInfoLen;//common policy is 0, otherwise must be 4

    //specificInfo
    firstRequest->specificInfo = new SpecificInfo;
    RTP_UT_STRUCT_INIT(firstRequest->specificInfo);
    PolicySpecificInfoActive *activeInfo = (PolicySpecificInfoActive *)firstRequest->specificInfo;
    activeInfo->priority   = 10;
    activeInfo->groupAdj   = 5;
    activeInfo->dupFlag    = false;

    //payload section
    firstRequest->payloadSection = new PayloadSection;
    RTP_UT_STRUCT_INIT(firstRequest->payloadSection);
    firstRequest->payloadSection->header.payloadType             = 101;
    firstRequest->payloadSection->header.reserved                = 0;
    firstRequest->payloadSection->specificInfo.maxMBPS           = 100000;
    firstRequest->payloadSection->specificInfo.maxFS             = 30;
    firstRequest->payloadSection->specificInfo.maxFPS            = 45;
    firstRequest->payloadSection->specificInfo.temporalLayerNum  = 3;
    for (int ii = 0; ii < firstRequest->payloadSection->specificInfo.temporalLayerNum; ++ii) {
        firstRequest->payloadSection->specificInfo.temporalLayer[ii]  = ii+3;
    }

    //fill length
    uint16_t payloadSpecificLen = 1 + 2 * firstRequest->payloadSection->specificInfo.temporalLayerNum;
    payloadSpecificLen = LENGTH_OF_H264CCHEAD + payloadSpecificLen + 4 - payloadSpecificLen % 4;
    firstRequest->payloadSection->header.payloadInfoLen          = 20;
    firstRequest->requestHeader.requestLen                       = LENGTH_OF_REQUEST_HEADER + policyInfoLen + LENGTH_OF_PAYLOAD_HEADER + payloadSpecificLen;
    packetLen += firstRequest->requestHeader.requestLen;
    firstRequest->payloadSection->next                           = nullptr;
    firstRequest->next                                           = nullptr;

    //Encode
    char buffer[1024] = {0};
    CCmMessageBlock mb(1024, (LPCSTR)buffer);
    CCmByteStreamNetwork os(mb);

    receiverIdPolicyMap.insert(make_pair(firstRequest->requestHeader.policyType, POLICY_TYPE_ACTIVE_SPEAKER));
    simulcastReceiverIdPolicyMap.insert(make_pair(firstRequest->requestHeader.sourceId, receiverIdPolicyMap));

    EncodeRTCPFCI(&scrPacket.fci, os);
    EncodeRequest(&scrPacket.request, os, receiverIdPolicyMap);

    //initail decoder
    CSubsessionChannelRequestMsg decoder(&simulcastReceiverIdPolicyMap);
    uint32_t result = decoder.DecodeFrom((const uint8_t *)buffer, packetLen);
    EXPECT_NE(0, result);

    ReleaseMemory(&scrPacket.request);
}

TEST_F(CSubsessionChannelRequestMsgTest, Decode_SCR_AS_SR_request)
{
    CScopedTracer test_info;
    uint16_t packetLen = 8;
    RequestIdPolicyMap receiverIdPolicyMap;
    SimulcastSource_RequestIdPolicyMap simulcastReceiverIdPolicyMap;
    SCRPacket scrPacket;
    RTP_UT_STRUCT_INIT(&scrPacket);
    //fci header
    scrPacket.fci.identifier        = 'MSTR';
    scrPacket.fci.msgTypeVersion    = (0x01<<4)|(0x01);
    scrPacket.fci.seqNum            = 468;

    //++++++++++++++++++first active request+++++++++++++++++++++++
    RequestInfo *request = &scrPacket.request;
    //request header
    request->requestHeader.vid             = 19;
    request->requestHeader.sourceId        = 165;//simulcastSource_RequestIdPolicyMap
    request->requestHeader.bitrate         = 1004;
    request->requestHeader.policyType      = 204;//map <id, policy>
    uint16_t policyInfoLen                        = 4;
    request->requestHeader.policyInfoLen   = policyInfoLen;//common policy is 0, otherwise must be 4

    //specificInfo
    request->specificInfo = new SpecificInfo;
    RTP_UT_STRUCT_INIT(request->specificInfo);
    PolicySpecificInfoActive *activeInfo = (PolicySpecificInfoActive *)request->specificInfo;
    activeInfo->priority   = 10;
    activeInfo->groupAdj   = 5;
    activeInfo->dupFlag    = true;

    //payload section
    request->payloadSection = new PayloadSection;
    RTP_UT_STRUCT_INIT(request->payloadSection);
    request->payloadSection->header.payloadType             = 101;
    request->payloadSection->header.reserved                = 0;
    request->payloadSection->specificInfo.maxMBPS           = 100000;
    request->payloadSection->specificInfo.maxFS             = 30;
    request->payloadSection->specificInfo.maxFPS            = 45;
    request->payloadSection->specificInfo.temporalLayerNum  = 3;
    for (int ii = 0; ii < request->payloadSection->specificInfo.temporalLayerNum; ++ii) {
        request->payloadSection->specificInfo.temporalLayer[ii]  = ii+3;
    }

    //fill length
    uint16_t payloadSpecificLen = 1 + 2 * request->payloadSection->specificInfo.temporalLayerNum;
    payloadSpecificLen = LENGTH_OF_H264CCHEAD + payloadSpecificLen + 4 - payloadSpecificLen % 4;
    request->payloadSection->header.payloadInfoLen          = payloadSpecificLen;
    request->requestHeader.requestLen                       = LENGTH_OF_REQUEST_HEADER + policyInfoLen + LENGTH_OF_PAYLOAD_HEADER + payloadSpecificLen;
    packetLen += request->requestHeader.requestLen;
    request->payloadSection->next                           = nullptr;
    request->next                                           = nullptr;
    receiverIdPolicyMap.insert(make_pair(request->requestHeader.policyType, POLICY_TYPE_ACTIVE_SPEAKER));
    simulcastReceiverIdPolicyMap.insert(make_pair(request->requestHeader.sourceId, receiverIdPolicyMap));


    //++++++++++++++++++sencond select request+++++++++++++++++++++++
    request->next = new RequestInfo;
    RTP_UT_STRUCT_INIT(request->next);
    request = request->next;

    //request header
    request->requestHeader.vid             = 20;
    request->requestHeader.sourceId        = 76;//simulcastSource_RequestIdPolicyMap
    request->requestHeader.bitrate         = 135;
    request->requestHeader.policyType      = 98;//map <id, policy>
    policyInfoLen                          = 4;
    request->requestHeader.policyInfoLen   = policyInfoLen;//common policy is 0, otherwise must be 4

    //specificInfo
    request->specificInfo = new SpecificInfo;
    RTP_UT_STRUCT_INIT(request->specificInfo);
    PolicySpecificInfoSelect *selectInfo = (PolicySpecificInfoSelect *)request->specificInfo;
    selectInfo->csi        = 22;

    //payload section
    request->payloadSection = new PayloadSection;
    RTP_UT_STRUCT_INIT(request->payloadSection);
    request->payloadSection->header.payloadType             = 105;
    request->payloadSection->header.reserved                = 0;
    request->payloadSection->specificInfo.maxMBPS           = 2345;
    request->payloadSection->specificInfo.maxFS             = 24;
    request->payloadSection->specificInfo.maxFPS            = 98;
    request->payloadSection->specificInfo.temporalLayerNum  = 4;
    for (int ii = 0; ii < request->payloadSection->specificInfo.temporalLayerNum; ++ii) {
        request->payloadSection->specificInfo.temporalLayer[ii]  = ii+3;
    }

    //fill length
    payloadSpecificLen = 1 + 2 * request->payloadSection->specificInfo.temporalLayerNum;
    payloadSpecificLen = LENGTH_OF_H264CCHEAD + payloadSpecificLen + 4 - payloadSpecificLen % 4;
    request->payloadSection->header.payloadInfoLen          = payloadSpecificLen;
    request->requestHeader.requestLen                       = LENGTH_OF_REQUEST_HEADER + policyInfoLen + LENGTH_OF_PAYLOAD_HEADER + payloadSpecificLen;
    packetLen += request->requestHeader.requestLen;
    request->payloadSection->next                           = nullptr;
    request->next                                           = nullptr;
    receiverIdPolicyMap.insert(make_pair(request->requestHeader.policyType, POLICY_RECEIVER_SELECTED_SOURCE));
    simulcastReceiverIdPolicyMap.insert(make_pair(request->requestHeader.sourceId, receiverIdPolicyMap));

    //+++++++++++++++++++++++++++++Encode+++++++++++++++++++++++
    char buffer[1024] = {0};
    CCmMessageBlock mb(1024, (LPCSTR)buffer);
    CCmByteStreamNetwork os(mb);

    EncodeRTCPFCI(&scrPacket.fci, os);
    EncodeRequest(&scrPacket.request, os, receiverIdPolicyMap);

    //+++++++++++++++++++++++++++++Decode+++++++++++++++++++++++
    CSubsessionChannelRequestMsg decoder(&simulcastReceiverIdPolicyMap);
    uint32_t result = decoder.DecodeFrom((const uint8_t *)buffer, packetLen);
    EXPECT_EQ(0, result);

    //check content
    CompareDecodeRTCPFCI(&scrPacket.fci, &decoder);
    CompareDecodeSCR(&scrPacket.request, &decoder, receiverIdPolicyMap);

    ReleaseMemory(&scrPacket.request);
}

TEST_F(CSubsessionChannelRequestMsgTest, Decode_SCR_Common_policy_without_payloadSection_return_error)
{
    CScopedTracer test_info;
    uint16_t packetLen = 8;
    RequestIdPolicyMap receiverIdPolicyMap;
    SimulcastSource_RequestIdPolicyMap simulcastReceiverIdPolicyMap;
    SCRPacket scrPacket;
    RTP_UT_STRUCT_INIT(&scrPacket);
    //fci header
    scrPacket.fci.identifier        = 'MSTR';
    scrPacket.fci.msgTypeVersion    = (0x01<<4)|(0x01);
    scrPacket.fci.seqNum            = 468;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    RequestInfo *request = &scrPacket.request;
    //request header
    request->requestHeader.vid             = 19;
    request->requestHeader.sourceId        = 165;//simulcastSource_RequestIdPolicyMap
    request->requestHeader.bitrate         = 1004;
    request->requestHeader.policyType      = 0;//map <id, policy>
    uint16_t policyInfoLen                   = 0;
    request->requestHeader.policyInfoLen   = policyInfoLen;//common policy is 0, otherwise must be 4
    request->requestHeader.requestLen                       = LENGTH_OF_REQUEST_HEADER + policyInfoLen;
    packetLen += request->requestHeader.requestLen;

    request->specificInfo   = nullptr;
    request->payloadSection = nullptr;
    request->next           = nullptr;
    receiverIdPolicyMap.insert(make_pair(request->requestHeader.policyType, POLICY_TYPE_NONE_POLICY));
    simulcastReceiverIdPolicyMap.insert(make_pair(request->requestHeader.sourceId, receiverIdPolicyMap));

    //+++++++++++++++++++++++++++++Encode+++++++++++++++++++++++
    char buffer[1024] = {0};
    CCmMessageBlock mb(1024, (LPCSTR)buffer);
    CCmByteStreamNetwork os(mb);

    EncodeRTCPFCI(&scrPacket.fci, os);
    EncodeRequest(&scrPacket.request, os, receiverIdPolicyMap);

    //+++++++++++++++++++++++++++++Decode+++++++++++++++++++++++
    CSubsessionChannelRequestMsg decoder(&simulcastReceiverIdPolicyMap);
    uint32_t result = decoder.DecodeFrom((const uint8_t *)buffer, packetLen);
    EXPECT_NE(0, result);

    ReleaseMemory(&scrPacket.request);

}

TEST_F(CSubsessionChannelRequestMsgTest, Decode_SCR_Common_policy)
{
    CScopedTracer test_info;
    uint16_t packetLen = 8;
    RequestIdPolicyMap receiverIdPolicyMap;
    SimulcastSource_RequestIdPolicyMap simulcastReceiverIdPolicyMap;
    SCRPacket scrPacket;
    RTP_UT_STRUCT_INIT(&scrPacket);
    //fci header
    scrPacket.fci.identifier        = 'MSTR';
    scrPacket.fci.msgTypeVersion    = (0x01<<4)|(0x01);
    scrPacket.fci.seqNum            = 468;

    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    RequestInfo *request = &scrPacket.request;
    //request header
    request->requestHeader.vid             = 19;
    request->requestHeader.sourceId        = 165;//simulcastSource_RequestIdPolicyMap
    request->requestHeader.bitrate         = 1004;
    request->requestHeader.policyType      = 0;//map <id, policy>
    uint16_t policyInfoLen                   = 0;
    request->requestHeader.policyInfoLen   = policyInfoLen;//common policy is 0, otherwise must be 4
    request->specificInfo   = nullptr;

    request->payloadSection = new PayloadSection;
    RTP_UT_STRUCT_INIT(request->payloadSection);
    request->payloadSection->header.payloadType = 98;
    request->payloadSection->header.reserved = 0;
    request->payloadSection->header.payloadInfoLen = 0;
    request->payloadSection->next = nullptr;

    request->requestHeader.requestLen                       = LENGTH_OF_REQUEST_HEADER + policyInfoLen + LENGTH_OF_PAYLOAD_HEADER;
    packetLen += request->requestHeader.requestLen;

    request->next           = nullptr;
    receiverIdPolicyMap.insert(make_pair(request->requestHeader.policyType, POLICY_TYPE_NONE_POLICY));
    simulcastReceiverIdPolicyMap.insert(make_pair(request->requestHeader.sourceId, receiverIdPolicyMap));

    //+++++++++++++++++++++++++++++Encode+++++++++++++++++++++++
    char buffer[1024] = {0};
    CCmMessageBlock mb(1024, (LPCSTR)buffer);
    CCmByteStreamNetwork os(mb);

    EncodeRTCPFCI(&scrPacket.fci, os);
    EncodeRequest(&scrPacket.request, os, receiverIdPolicyMap);

    //+++++++++++++++++++++++++++++Decode+++++++++++++++++++++++
    CSubsessionChannelRequestMsg decoder(&simulcastReceiverIdPolicyMap);
    uint32_t result = decoder.DecodeFrom((const uint8_t *)buffer, packetLen);
    EXPECT_EQ(0, result);

    //check content
    CompareDecodeRTCPFCI(&scrPacket.fci, &decoder);
    CompareDecodeSCR(&scrPacket.request, &decoder, receiverIdPolicyMap);

    ReleaseMemory(&scrPacket.request);
}

TEST_F(CSubsessionChannelRequestMsgTest, Encode_Decode_SCR)
{
    CScopedTracer test_info;
    uint8_t                subsessionChannelId = 21;
    uint8_t                sourceId = 81;
    uint32_t               bitrate = 1001;
    SimulcastSource_RequestPolicyIdMap simulcastSource_RequestPolicyIdMap;
    SimulcastSource_RequestIdPolicyMap simulcastSource_RequestIdPolicyMap;

    RequestPolicyIdMap requestPolicyIdMap;
    RequestIdPolicyMap requestIdPolicyMap;

    requestPolicyIdMap.insert(make_pair(POLICY_TYPE_ACTIVE_SPEAKER,101));
    requestPolicyIdMap.insert(make_pair(POLICY_RECEIVER_SELECTED_SOURCE, 102));
    requestPolicyIdMap.insert(make_pair(POLICY_TYPE_NONE_POLICY, 103));
    simulcastSource_RequestPolicyIdMap.insert(make_pair(sourceId,requestPolicyIdMap));
    simulcastSource_RequestPolicyIdMap.insert(make_pair(sourceId+1, requestPolicyIdMap));
    simulcastSource_RequestPolicyIdMap.insert(make_pair(sourceId+2, requestPolicyIdMap));

    requestIdPolicyMap.insert(make_pair(101,POLICY_TYPE_ACTIVE_SPEAKER));
    requestIdPolicyMap.insert(make_pair(102, POLICY_RECEIVER_SELECTED_SOURCE));
    requestIdPolicyMap.insert(make_pair(102, POLICY_TYPE_NONE_POLICY));
    simulcastSource_RequestIdPolicyMap.insert(make_pair(sourceId,requestIdPolicyMap));
    simulcastSource_RequestIdPolicyMap.insert(make_pair(sourceId+1, requestIdPolicyMap));
    simulcastSource_RequestIdPolicyMap.insert(make_pair(sourceId+2, requestIdPolicyMap));

    CSubsessionChannelRequestMsg scrmsg_encoder(&simulcastSource_RequestPolicyIdMap);
    CSubsessionChannelRequestMsg scrmsg_decoder(&simulcastSource_RequestIdPolicyMap);


    //SelectedSourceRequestTrait
    uint32_t              captureSourceId = 2002;

    //ActiveSpeakerRequestTrait
    uint8_t               priority = 41;
    uint8_t               groupingAdjacencyId = 61;

    CodecCapability codecCapH264;
    codecCapH264.type = REQUEST_CODEC_TYPE_H264;
    codecCapH264.h264CodecCap.payloadType = 1;
    codecCapH264.h264CodecCap.maxMBPS = 2;
    codecCapH264.h264CodecCap.maxFS = 3;
    codecCapH264.h264CodecCap.maxFPS = 4;
    codecCapH264.h264CodecCap.temporalLayerCount = 3;
    codecCapH264.h264CodecCap.temporalLayers[0] = 15;
    codecCapH264.h264CodecCap.temporalLayers[1] = 16;
    codecCapH264.h264CodecCap.temporalLayers[2] = 17;

    CodecCapability codecCapCommon;
    codecCapCommon.type = REQUEST_CODEC_TYPE_COMMON;
    codecCapCommon.commonCodecCap.payloadType = 98;

    int32_t ret = scrmsg_encoder.AddChannelRequest(ActiveSpeakerRequestTrait(subsessionChannelId,sourceId,bitrate,captureSourceId,1, false),codecCapH264);
    EXPECT_EQ(0, ret);
    ret = scrmsg_encoder.AddChannelRequest(ActiveSpeakerRequestTrait(subsessionChannelId + 1,sourceId + 1,bitrate + 1,captureSourceId + 1,1, true),codecCapCommon);
    EXPECT_EQ(0, ret);
    ret = scrmsg_encoder.AddChannelRequest(SelectedSourceRequestTrait(subsessionChannelId + 2,sourceId + 2,bitrate + 2,captureSourceId +2),codecCapH264);
    EXPECT_EQ(0, ret);
    scrmsg_encoder.SetSequenceNumber(111);

    uint8_t buff[1024];
    uint16_t nLen = 1024;

    ret = scrmsg_encoder.EncodeTo(buff,nLen);
    EXPECT_EQ(0, ret);

    uint32_t res = scrmsg_decoder.DecodeFrom(buff,nLen);
    EXPECT_EQ(0, res);
    SimulcastRequestInfoMap scrinfMap = scrmsg_decoder.GetRequestInfoMap();
    EXPECT_EQ(3, scrinfMap.size());

    CompareDecodeRTCPFCI(&scrmsg_encoder, &scrmsg_decoder);
    CompareDecodeSCR(&scrmsg_encoder, &scrmsg_decoder);

}

//Test SCA Decode/Encode
class CSubsessionChannelAnnounceMsgTest: public ::testing::Test
{
protected:
    virtual void SetUp()
    {

    }
    virtual void TearDown()
    {

    }

    //SCA format
    struct RTCPFCI {
        uint32_t identifier;
        uint16_t msgTypeVersion;
        uint16_t seqNum;
    };

    struct AnnounceHeader {
        uint16_t currentRequest;
        uint8_t  subSessionAvalible;
        uint8_t  maxAdjReserveAck;
    };

    struct WarningInfo {
        uint8_t  invalidChannelId;
        uint16_t reservedAndCFlag;
        uint8_t  errorCode;
        uint32_t captureSourceId;
        WarningInfo *next;
    };

    struct AnnounceInfo {
        AnnounceHeader header;
        WarningInfo    *waining;
    };
    struct SCAPacket {
        RTCPFCI        fci;
        AnnounceInfo   announce;
    };

    void EncodeRTCPFCI(RTCPFCI *fci, CCmByteStreamNetwork &os)
    {
        os << fci->identifier;
        os << fci->msgTypeVersion;
        os << fci->seqNum;
        ASSERT_TRUE(os.IsGood() == TRUE);
    }

    void EncodeAnnounce(AnnounceInfo *announce, CCmByteStreamNetwork &os)
    {
        ASSERT_TRUE(nullptr != announce);
        os << announce->header.currentRequest;
        os << announce->header.subSessionAvalible;
        os << announce->header.maxAdjReserveAck;

        WarningInfo *warnings = announce->waining;
        while (nullptr != warnings) {
            bool cFlag = (warnings->reservedAndCFlag) & 0x01;
            os << warnings->invalidChannelId;
            os << warnings->reservedAndCFlag;
            os << warnings->errorCode;
            if (cFlag) {
                os << warnings->captureSourceId;
            }
            warnings = warnings->next;
        }
        ASSERT_TRUE(os.IsGood() == TRUE);
    }

    void CompareDecodeRTCPFCI(RTCPFCI *fci, CSubsessionChannelAnnounceMsg *msg)
    {
        EXPECT_EQ(fci->msgTypeVersion>>4, msg->GetFeedbackType());
        EXPECT_EQ(fci->seqNum, msg->GetSequenceNumber());
    }
    void CompareDecodeRTCPFCI(CSubsessionChannelAnnounceMsg *encodeMsg, CSubsessionChannelAnnounceMsg *decodeMsg)
    {
        EXPECT_EQ(encodeMsg->GetFeedbackType(), decodeMsg->GetFeedbackType());
    }

    void CompareDecodeSCA(CSubsessionChannelAnnounceMsg *encodeMsg, CSubsessionChannelAnnounceMsg *decodeMsg)
    {
        EXPECT_EQ(encodeMsg->GetRequestSeq(), decodeMsg->GetRequestSeq());
        EXPECT_EQ(encodeMsg->GetAvailableSubsessions(), decodeMsg->GetAvailableSubsessions());
        EXPECT_EQ(encodeMsg->GetMaxAdjSources(), decodeMsg->GetMaxAdjSources());
        EXPECT_EQ(encodeMsg->GetAck(), decodeMsg->GetAck());

        const InvalidRequestVector encodeInvalidRequest = encodeMsg->GetInvalidRequests();
        const InvalidRequestVector decodeInvalidRequest = decodeMsg->GetInvalidRequests();
        //compare two struct

        for (InvalidRequestVector::const_iterator itEncode = encodeInvalidRequest.begin(); itEncode != encodeInvalidRequest.end(); ++itEncode) {
            InvalidRequestVector::const_iterator itDecode = decodeInvalidRequest.begin();
            for (; itDecode != decodeInvalidRequest.end(); ++itDecode) {
                if (itEncode->subsessionChannelId == itDecode->subsessionChannelId) {
                    EXPECT_EQ(itEncode->errorCode, itDecode->errorCode);
                    break;
                }
            }
            EXPECT_TRUE(itDecode != decodeInvalidRequest.end());
        }
    }

    void CompareDecodeSCA(AnnounceInfo *announce, CSubsessionChannelAnnounceMsg *msg)
    {
        EXPECT_EQ(announce->header.currentRequest, msg->GetRequestSeq());
        EXPECT_EQ(announce->header.subSessionAvalible, msg->GetAvailableSubsessions());
        EXPECT_EQ(announce->header.maxAdjReserveAck>>4, msg->GetMaxAdjSources());
        EXPECT_EQ(announce->header.maxAdjReserveAck & 0x01, msg->GetAck());

        WarningInfo *warinings = announce->waining;
        while (nullptr != warinings) {
            const InvalidRequestVector invalidRequest = msg->GetInvalidRequests();
            InvalidRequestVector::const_iterator it = invalidRequest.begin();
            for (; it != invalidRequest.end(); ++it) {
                if (warinings->invalidChannelId == it->subsessionChannelId) {
                    EXPECT_EQ(warinings->errorCode, it->errorCode);
                    if (warinings->reservedAndCFlag) {
                        EXPECT_EQ(warinings->captureSourceId, it->csid);
                    }
                    break;
                }
            }
            EXPECT_TRUE(it != invalidRequest.end());//if it == invalidRequest.end() means cannot find this warning in vector
            warinings = warinings->next;
        }
    }

    void ReleaseMemory(AnnounceInfo *announce)
    {
        WarningInfo *warnings = announce->waining;
        while (nullptr != warnings) {
            announce->waining = warnings->next;
            delete warnings;
            warnings = announce->waining;
        }
    }
};


TEST_F(CSubsessionChannelAnnounceMsgTest, Decode_SCA_Without_CSI)
{
    CScopedTracer test_info;
    SCAPacket scaPacket;
    RTP_UT_STRUCT_INIT(&scaPacket);
    scaPacket.fci.identifier = 'MSTR';
    scaPacket.fci.msgTypeVersion = (0x02<<4)|(0x01);
    scaPacket.fci.seqNum    = 87;

    scaPacket.announce.header.currentRequest = 99;
    scaPacket.announce.header.subSessionAvalible = 22;
    scaPacket.announce.header.maxAdjReserveAck = (12<<4)|(0x01);//maxAdj=12,ackFlag=1
    WarningInfo *warnings = new WarningInfo;
    RTP_UT_STRUCT_INIT(warnings);
    scaPacket.announce.waining = warnings;
    warnings->invalidChannelId = 56;
    warnings->reservedAndCFlag = 0;
    warnings->errorCode        = 0x10;
    warnings->next             = new WarningInfo;
    RTP_UT_STRUCT_INIT(warnings->next);
    warnings                   = warnings->next;
    warnings->invalidChannelId = 42;
    warnings->reservedAndCFlag = 0;
    warnings->errorCode        = 0x32;
    warnings->next             = nullptr;

    uint8_t buff[1024] = {0};
    CCmMessageBlock mb(1024, (LPSTR)buff);
    CCmByteStreamNetwork os(mb);

    EncodeRTCPFCI(&scaPacket.fci, os);
    EncodeAnnounce(&scaPacket.announce, os);

    uint16_t nLen = 20;
    CSubsessionChannelAnnounceMsg decoder;
    uint32_t res = decoder.DecodeFrom(buff, nLen);
    EXPECT_EQ(0, res);

    CompareDecodeRTCPFCI(&scaPacket.fci, &decoder);
    CompareDecodeSCA(&scaPacket.announce, &decoder);

    ReleaseMemory(&scaPacket.announce);
}

TEST_F(CSubsessionChannelAnnounceMsgTest, Decode_SCA_With_CSI)
{
    CScopedTracer test_info;
    SCAPacket scaPacket;
    RTP_UT_STRUCT_INIT(&scaPacket);
    scaPacket.fci.identifier = 'MSTR';
    scaPacket.fci.msgTypeVersion = (0x02<<4)|(0x01);
    scaPacket.fci.seqNum    = 87;

    scaPacket.announce.header.currentRequest = 99;
    scaPacket.announce.header.subSessionAvalible = 22;
    scaPacket.announce.header.maxAdjReserveAck = (12<<4)|(0x01);//maxAdj=12,ackFlag=1
    WarningInfo *warnings = new WarningInfo;
    RTP_UT_STRUCT_INIT(warnings);
    scaPacket.announce.waining = warnings;
    warnings->invalidChannelId = 56;
    warnings->reservedAndCFlag = 0;
    warnings->errorCode        = 0x10;
    warnings->captureSourceId  = 2;
    warnings->next             = new WarningInfo;
    RTP_UT_STRUCT_INIT(warnings->next);
    warnings                   = warnings->next;
    warnings->invalidChannelId = 42;
    warnings->reservedAndCFlag = 1;
    warnings->errorCode        = 0x32;
    warnings->captureSourceId  = 4;
    warnings->next             = nullptr;

    uint8_t buff[1024] = {0};
    CCmMessageBlock mb(1024, (LPSTR)buff);
    CCmByteStreamNetwork os(mb);

    EncodeRTCPFCI(&scaPacket.fci, os);
    EncodeAnnounce(&scaPacket.announce, os);

    uint16_t nLen = 24;
    CSubsessionChannelAnnounceMsg decoder;
    uint32_t res = decoder.DecodeFrom(buff, nLen);
    EXPECT_EQ(0, res);

    CompareDecodeRTCPFCI(&scaPacket.fci, &decoder);
    CompareDecodeSCA(&scaPacket.announce, &decoder);

    ReleaseMemory(&scaPacket.announce);
}

TEST_F(CSubsessionChannelAnnounceMsgTest, Encode_Decode_SCA_Without_SCI)
{
    CScopedTracer test_info;
    CSubsessionChannelAnnounceMsg encoder;
    CSubsessionChannelAnnounceMsg decoder;
    encoder.SetRequestSeq(55);
    encoder.SetAvailableSubsessions(32);
    encoder.SetAck(1);

    InvalidRequestVector invalidRequestVector;
    InvalidRequest       invalidRequest1(13, 0x11, false, 0);
    InvalidRequest       invalidRequest2(21, 0x32, false, 0);
    invalidRequestVector.push_back(invalidRequest1);
    invalidRequestVector.push_back(invalidRequest2);

    encoder.SetInvalidRequests(invalidRequestVector);
    encoder.SetSequenceNumber(111);

    uint8_t buff[1024] = {0};
    uint16_t nLen = 1024;
    int32_t res = encoder.EncodeTo(buff, nLen);
    ASSERT_EQ(0, res);
    EXPECT_GE(1024, nLen);

    res = decoder.DecodeFrom(buff, nLen);
    ASSERT_EQ(0, res);

    CompareDecodeRTCPFCI(&encoder, &decoder);
    CompareDecodeSCA(&encoder, &decoder);
}

TEST_F(CSubsessionChannelAnnounceMsgTest, Encode_Decode_SCA_With_SCI)
{
    CScopedTracer test_info;
    CSubsessionChannelAnnounceMsg encoder;
    CSubsessionChannelAnnounceMsg decoder;
    encoder.SetRequestSeq(55);
    encoder.SetAvailableSubsessions(32);
    encoder.SetAck(1);

    InvalidRequestVector invalidRequestVector;
    InvalidRequest       invalidRequest1(13, 0x11, true, 30);
    InvalidRequest       invalidRequest2(21, 0x32, false, 0);
    InvalidRequest       invalidRequest3(21, 0x32, true, 77);
    invalidRequestVector.push_back(invalidRequest1);
    invalidRequestVector.push_back(invalidRequest2);
    invalidRequestVector.push_back(invalidRequest3);

    encoder.SetInvalidRequests(invalidRequestVector);
    encoder.SetSequenceNumber(111);

    uint8_t buff[1024] = {0};
    uint16_t nLen = 1024;
    int32_t res = encoder.EncodeTo(buff, nLen);
    ASSERT_EQ(0, res);
    EXPECT_GE(1024, nLen);

    res = decoder.DecodeFrom(buff, nLen);
    ASSERT_EQ(0, res);

    CompareDecodeRTCPFCI(&encoder, &decoder);
    CompareDecodeSCA(&encoder, &decoder);
}

//Test SCA-ACK Decode/Encode
class CSubsessionChannelAnnounceAckMsgTest: public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }

    //SCA-ACK format
    struct RTCPFCI {
        uint32_t identifier;
        uint16_t msgTypeVersion;
        uint16_t seqNum;
    };

    typedef RTCPFCI SCAACKPacket;

    void EncodeRTCPFCI(RTCPFCI *fci, CCmByteStreamNetwork &os)
    {
        os << fci->identifier;
        os << fci->msgTypeVersion;
        os << fci->seqNum;
        ASSERT_TRUE(os.IsGood() == TRUE);
    }

    void CompareDecodeRTCPFCI(RTCPFCI *fci, CSubsessionChannelAnnounceAckMsg *msg)
    {
        EXPECT_EQ(fci->msgTypeVersion>>4, msg->GetFeedbackType());
        EXPECT_EQ(fci->seqNum, msg->GetSequenceNumber());
    }
    void CompareDecodeRTCPFCI(CSubsessionChannelAnnounceAckMsg *encodeMsg, CSubsessionChannelAnnounceAckMsg *decodeMsg)
    {
        EXPECT_EQ(encodeMsg->GetFeedbackType(), decodeMsg->GetFeedbackType());
        EXPECT_EQ(encodeMsg->GetSequenceNumber(), decodeMsg->GetSequenceNumber());
    }



};

TEST_F(CSubsessionChannelAnnounceAckMsgTest, Decode_SCAACK)
{
    CScopedTracer test_info;
    SCAACKPacket scaAckPacket;
    RTP_UT_STRUCT_INIT(&scaAckPacket);
    scaAckPacket.identifier = 'MSTR';
    scaAckPacket.msgTypeVersion = (0x03<<4)|(0x01);
    scaAckPacket.seqNum = 55;

    uint8_t buff[1024] = {0};
    CCmMessageBlock mb(1024, (LPSTR)buff);
    CCmByteStreamNetwork os(mb);
    EncodeRTCPFCI(&scaAckPacket, os);

    CSubsessionChannelAnnounceAckMsg decoder;
    uint16_t nLen = LENGTH_OF_PSFB_HEADER;
    int32_t res = decoder.DecodeFrom(buff, nLen);
    ASSERT_EQ(0, res);

    CompareDecodeRTCPFCI(&scaAckPacket, &decoder);
}

TEST_F(CSubsessionChannelAnnounceAckMsgTest, Encode_Decode_SCAACK)
{
    CScopedTracer test_info;
    CSubsessionChannelAnnounceAckMsg encoder;
    CSubsessionChannelAnnounceAckMsg decoder;
    encoder.SetSequenceNumber(34);

    uint8_t buff[1024] = {0};
    uint16_t nLen = 1024;

    int32_t res = encoder.EncodeTo(buff, nLen);
    ASSERT_EQ(0, res);
    EXPECT_GT(1024, nLen);

    res = decoder.DecodeFrom(buff, nLen);
    ASSERT_EQ(0, res);

    CompareDecodeRTCPFCI(&encoder, &decoder);

}

