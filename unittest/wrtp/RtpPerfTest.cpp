//
//  RtpPerfTest.cpp
//  Turbo
//
//  Created by folkib on 11/18/15.
//  Copyright © 2015 Ziyue Zhang. All rights reserved.
//

#include <stdio.h>

#include <chrono>

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
#include "fragmentop.h"

using namespace wrtp;
using namespace wme;

using clocker = std::chrono::steady_clock;

#define CODEC_TYPE_AUDIO_TEST   101
#define PAYLOAD_TYPE_AUDIO_TEST 101
#define CODEC_TYPE_VIDEO_TEST   98
#define PAYLOAD_TYPE_VIDEO_TEST 98

#define DEFAULT_AUDIO_CAPTURE_CLOCK_RATE (48000)
#define DEFAULT_VIDIO_CAPTURE_CLOCK_RATE (90000)

#define TEST_MEDIA_DATA_SIZE    10240

class CRTPSendingSinkMock : public IRTPMediaTransport
{
public:
    CRTPSendingSinkMock() : peerSession(nullptr), peerChannel(nullptr)
    {}
    
    ~CRTPSendingSinkMock()
    {}
    
public:
    virtual WMERESULT GetTransportType(WmeTransportType &eTransportType) {
        eTransportType = WmeTransportType_UDP;
        return WME_S_OK;
    }
    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage) {
        if(peerChannel) {
            peerChannel->ReceiveRTPPacket(pRTPPackage);
        }
        return WME_S_OK;
    }
    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage) {
        if(peerSession) {
            peerSession->ReceiveRTCPPacket(pRTCPPackage);
        }
        return WME_S_OK;
    }
    
    void SetPeerSession(IRTPSessionClient *s) {
        peerSession = s;
    }
    
    void SetPeerChannel(IRTPChannel *c) {
        peerChannel = c;
    }
    
public:
    IRTPSessionClient *peerSession;
    IRTPChannel *peerChannel;
};

class CMediaDataRecvSinkMock : public IMediaDataRecvSink
{
public:
    CMediaDataRecvSinkMock() : nalCount(0) {}

    virtual int32_t OnRecvMediaData(WRTPMediaData *pData, WRTPMediaDataIntegrityInfo *pIntegrityInfo) {
        ++nalCount;
        return 0;
    }
    
    uint32_t nalCount;
};

void trace_func(unsigned long trace_level, char *trace_info, unsigned int len) {
    printf("%s\n", trace_info);
}

bool enable_rtcp = true;
bool enable_qos = false;
bool enable_fec = false;
bool enable_srtp = false;
bool send_only = false;
bool enable_scr = true;

void RunRtpPerfTest()
{
    //set_util_external_trace_sink(trace_func);
    WRTPSessionParams rtpSessionParams;
    rtpSessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    rtpSessionParams.enableRTCP = enable_rtcp;
    
    IRTPSessionClient *sendSession =  WRTPCreateRTPSessionClient(rtpSessionParams);
    sendSession->IncreaseReference();
    
    sendSession->SetPacketizationMode(PACKETIZATION_MODE1);
    sendSession->EnableSendingAggregationPacket(false);
    sendSession->SetMaxPacketSize(1400);
    
    uint8_t vidID = 1;
    uint8_t frameMarkingID = 3;
    uint8_t mariTimestampID = 4;
    uint8_t timeOffsetID = 5;
    uint8_t rapicSyncID = 6;
    
    sendSession->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_VID), vidID, wrtp::STREAM_INOUT);
    sendSession->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_FrameMarking), frameMarkingID, wrtp::STREAM_INOUT);
    sendSession->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_MARITimestamp), mariTimestampID, wrtp::STREAM_INOUT);
    sendSession->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_TOFFSET), timeOffsetID, wrtp::STREAM_INOUT);
    sendSession->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_RapicSync64), rapicSyncID, wrtp::STREAM_INOUT);
    CRTPSendingSinkMock rtpSendingSink1;
    sendSession->SetMediaTransport(&rtpSendingSink1);
    
    int32_t ret = sendSession->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
    
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
    inOutConf.ekt                      = NULL;
    
    if(enable_srtp) {
        ret = sendSession->SetInboundSecurityConfiguration(inOutConf);
        if (ret) {
            WRTP_WARNTRACE("WRTPCreateRTPSessionClient(): Failed to set inbound security, " << WRTP_ERROR_CODE(ret));
        }
        ret = sendSession->SetOutboundSecurityConfiguration(inOutConf);
        if (ret) {
            WRTP_WARNTRACE("WRTPCreateRTPSessionClient(): Failed to set outbound security, " << WRTP_ERROR_CODE(ret));
        }
    }
    
    CFragmentOperatorMock fragmentOperator;
    uint32_t channelId = 777;
    WRTPChannelParams rtpChannelParams;
    rtpChannelParams.pFragmentOp = &fragmentOperator;
    IRTPChannel *rtpSendChannel =  sendSession->CreateRTPChannel(channelId, rtpChannelParams);
    rtpSendChannel->IncreaseReference();
    CRTPPacketizationMock packetizerMock;
    rtpSendChannel->SetPacketizationOperator(&packetizerMock);
    
    IRTPSessionClient *recvSession =  WRTPCreateRTPSessionClient(rtpSessionParams);
    recvSession->IncreaseReference();
    CRTPSendingSinkMock rtpSendingSink2;
    recvSession->SetMediaTransport(&rtpSendingSink2);
    ret = recvSession->RegisterPayloadType(CODEC_TYPE_VIDEO_TEST, PAYLOAD_TYPE_VIDEO_TEST, DEFAULT_VIDIO_CAPTURE_CLOCK_RATE);
    
    recvSession->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_VID), vidID, wrtp::STREAM_INOUT);
    recvSession->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_FrameMarking), frameMarkingID, wrtp::STREAM_INOUT);
    recvSession->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_MARITimestamp), mariTimestampID, wrtp::STREAM_INOUT);
    recvSession->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_TOFFSET), timeOffsetID, wrtp::STREAM_INOUT);
    recvSession->UpdateRTPExtension(GetVideoRTPHeaderExtName(RTPEXT_RapicSync64), rapicSyncID, wrtp::STREAM_INOUT);
    
    if(enable_srtp) {
        ret = recvSession->SetInboundSecurityConfiguration(inOutConf);
        if (ret) {
            WRTP_WARNTRACE("WRTPCreateRTPSessionClient(): Failed to set inbound security, " << WRTP_ERROR_CODE(ret));
        }
        ret = recvSession->SetOutboundSecurityConfiguration(inOutConf);
        if (ret) {
            WRTP_WARNTRACE("WRTPCreateRTPSessionClient(): Failed to set outbound security, " << WRTP_ERROR_CODE(ret));
        }
    }
    
    CMediaDataRecvSinkMock mediaDataRecvSink;
    IRTPChannel *rtpRecvChannel =  recvSession->CreateRTPChannel(1, rtpChannelParams);
    rtpRecvChannel->IncreaseReference();
    rtpRecvChannel->SetMediaDataRecvSink(&mediaDataRecvSink);
    rtpRecvChannel->SetPacketizationOperator(&packetizerMock);
    
    if(!send_only) {
        rtpSendingSink1.SetPeerChannel(rtpRecvChannel);
        rtpSendingSink1.SetPeerSession(recvSession);
        rtpSendingSink2.SetPeerChannel(rtpSendChannel);
        rtpSendingSink2.SetPeerSession(sendSession);
    }
    
    sendSession->EnableCiscoSCR(enable_scr);
    recvSession->EnableCiscoSCR(enable_scr);

    uint8_t buf[TEST_MEDIA_DATA_SIZE] = {0};
    
    for (int i = 0; i < TEST_MEDIA_DATA_SIZE; ++i) {
        buf[i] = i;
    }
    clocker::time_point t1 = clocker::now();
    int loopCount = 30000;
    uint32_t tick = 1;
    printf("start test\n");
    int i = 0;
    uint8_t vids[WRTP_MAX_VID_SIZE];
    vids[0] = 0;
    vids[1] = 1;
    while (i < loopCount) {
        
        WRTPMediaDataVideo mdv1;
        mdv1.type = MEDIA_DATA_VIDEO;
        mdv1.data = buf;
        mdv1.size = 996;
        mdv1.timestamp = tick;
        mdv1.sampleTimestamp = mdv1.timestamp*90;
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
        mdv1.vidArray = vids;
        mdv1.vidCount = 2;
        
        WRTPMediaDataVideo mdv2;
        mdv2.type = MEDIA_DATA_VIDEO;
        mdv2.data = buf;
        mdv2.size = sizeof(buf);
        mdv2.timestamp = tick;
        mdv2.sampleTimestamp = mdv2.timestamp*90;
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
        mdv2.vidArray = vids;
        mdv2.vidCount = 2;
        
        rtpSendChannel->SendMediaData(&mdv1);
        rtpSendChannel->SendMediaData(&mdv2);
        
        tick += 33;
        if(++i%10000 == 0) {
            printf("test count: %d\n", i);
        }
    }
    clocker::time_point t2 = clocker::now();
    
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
    printf("complete: cost: %lld, nals=%u\n", ms.count(), mediaDataRecvSink.nalCount);
    
    rtpSendChannel->Close();
    rtpSendChannel->DecreaseReference();
    rtpRecvChannel->Close();
    rtpRecvChannel->DecreaseReference();
    sendSession->DecreaseReference();
    recvSession->DecreaseReference();
}
