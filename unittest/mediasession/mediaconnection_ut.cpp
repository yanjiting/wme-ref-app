//
//  mediaconnection_ut.cpp
//  mediasessionUT_ios
//
//  Created by LiSiyuan on 14-7-29.
//  Copyright (c) 2014年 cisco. All rights reserved.
//

#include "gtest/gtest.h"
#include "stringhelper.h"
#include "MediaConnection.hpp"
#include "mediaconfig.h"
#include "MultistreamAudioTrack.h"
#include <string>
#include "json.h"
#include "atdef.h"
#include "MediaPerformance.h"
#include <openssl/crypto.h>
#include "MediaCSI.h"
#include "perf-settings.h"
#include "TypeConvert.h"

using namespace wme;

extern bool g_useThreadForOpenSSL;

int testcount = 0;

std::string android_mediacodec_true()
{
    return std::string(
                       "{"
                       " \"video\":{"
                       "  \"mediaCodec\":true, \"yv12Capture\":true}"
                       "}");
}
std::string android_mediacodec_false()
{
    return std::string(
                       "{"
                       " \"video\":{"
                       "  \"mediaCodec\":false, \"yv12Capture\":false}"
                       "}");
}

std::string ex90_sdp_answer()
{
    return std::string(
                       "v=0\n\r"
                       "o=tandberg 5 1 IN IP4 128.107.141.3\n\r"
                       "s=-\n\r"
                       "c=IN IP4 128.107.141.3\n\r"
                       "b=AS:768\n\r"
                       "t=0 0\n\r"
                       "m=audio 2350 RTP/AVP 100 101 102 103 104 105 9 8 0 106\n\r"
                       "b=TIAS:128000\n\r"
                       "a=rtpmap:100 MP4A-LATM/90000\n\r"
                       "a=fmtp:100 profile-level-id=25;object=23;bitrate=128000\n\r"
                       "a=rtpmap:101 MP4A-LATM/90000\n\r"
                       "a=fmtp:101 profile-level-id=24;object=23;bitrate=64000\n\r"
                       "a=rtpmap:102 MP4A-LATM/90000\n\r"
                       "a=fmtp:102 profile-level-id=24;object=23;bitrate=56000\n\r"
                       "a=rtpmap:103 MP4A-LATM/90000\n\r"
                       "a=fmtp:103 profile-level-id=24;object=23;bitrate=48000\n\r"
                       "a=rtpmap:104 G7221/16000\n\r"
                       "a=fmtp:104 bitrate=32000\n\r"
                       "a=rtpmap:105 G7221/16000\n\r"
                       "a=fmtp:105 bitrate=24000\n\r"
                       "a=rtpmap:9 G722/8000\n\r"
                       "a=rtpmap:8 PCMA/8000\n\r"
                       "a=rtpmap:0 PCMU/8000\n\r"
                       "a=rtpmap:106 telephone-event/8000\n\r"
                       "a=fmtp:106 0-15\n\r"
                       "a=crypto:0 AES_CM_128_HMAC_SHA1_80 inline:dv7DyEfva71d7MpWWaGQYbcBkDK17iaasE4ymFE0|2^48 UNENCRYPTED_SRTCP\n\r"
                       "a=sendrecv\n\r"
                       "m=video 2352 RTP/AVP 97 98 99 34 31\n\r"
                       "b=TIAS:768000\n\r"
                       "a=rtpmap:97 H264/90000\n\r"
                       "a=fmtp:97 profile-level-id=428014;max-br=3333;max-mbps=108000;max-fs=3600;max-smbps=108000\n\r"
                       "a=rtpmap:98 H264/90000\n\r"
                       "a=fmtp:98 profile-level-id=428014;max-br=3333;max-mbps=108000;max-fs=3600;max-smbps=108000;packetization-mode=1\n\r"
                       "a=rtpmap:99 H263-1998/90000\n\r"
                       "a=fmtp:99 custom=1280,720,3;custom=1024,768,1;custom=1024,576,2;custom=800,600,1;cif4=1;custom=720,480,1;custom=640,480,1;custom=512,288,1;cif=1;custom=352,240,1;qcif=1;maxbr=7680\n\r"
                       "a=rtpmap:34 H263/90000\n\r"
                       "a=fmtp:34 cif4=1;cif=1;qcif=1;maxbr=7680\n\r"
                       "a=rtpmap:31 H261/90000\n\r"
                       "a=fmtp:31 cif=1;qcif=1;maxbr=7680\n\r"
                       "a=rtcp-fb:* nack pli\n\r"
                       "a=crypto:0 AES_CM_128_HMAC_SHA1_80 inline:ev4awXBPQ4pNJiFNLfOILF+g59VfX45yF7Uceb3h|2^48 UNENCRYPTED_SRTCP\n\r"
                       "a=sendrecv\n\r"
                       "a=content:main\n\r"
                       "a=label:11\n\r"
                       "a=answer:full\n\r"
                       "m=application 5070 UDP/BFCP *\n\r"
                       "a=floorctrl:c-s\n\r"
                       "a=confid:1\n\r"
                       "a=floorid:2 mstrm:12\n\r"
                       "a=userid:5\n\r"
                       "a=setup:passive\n\r"
                       "a=connection:new\n\r"
                       "m=video 2354 RTP/AVP 97 98 99 34 31\n\r"
                       "b=TIAS:768000\n\r"
                       "a=rtpmap:97 H264/90000\n\r"
                       "a=fmtp:97 profile-level-id=428014;max-br=3333;max-mbps=108000;max-fs=3840;max-smbps=108000\n\r"
                       "a=rtpmap:98 H264/90000\n\r"
                       "a=fmtp:98 profile-level-id=428014;max-br=3333;max-mbps=108000;max-fs=3840;max-smbps=108000;packetization-mode=1\n\r"
                       "a=rtpmap:99 H263-1998/90000\n\r"
                       "a=fmtp:99 custom=1280,720,3;custom=1024,768,1;custom=1024,576,2;custom=800,600,1;cif4=1;custom=720,480,1;custom=640,480,1;custom=512,288,1;cif=1;custom=352,240,1;qcif=1;maxbr=7680\n\r"
                       "a=rtpmap:34 H263/90000\n\r"
                       "a=fmtp:34 cif4=1;cif=1;qcif=1;maxbr=7680\n\r"
                       "a=rtpmap:31 H261/90000\n\r"
                       "a=fmtp:31 cif=1;qcif=1;maxbr=7680\n\r"
                       "a=rtcp-fb:* nack pli\n\r"
                       "a=crypto:0 AES_CM_128_HMAC_SHA1_80 inline:z8ZkCzJIcdbhQu9SK6rK1JJjEjKUY0OXMYWFnw25|2^48 UNENCRYPTED_SRTCP\n\r"
                       "a=sendrecv\n\r"
                       "a=content:slides\n\r"
                       "a=label:12\n\r"
                       "m=application 2356 RTP/AVP 107\n\r"
                       "a=rtpmap:107 H224/4800\n\r"
                       "a=crypto:0 AES_CM_128_HMAC_SHA1_80 inline:4IBVx3ctvBVAfI3VQDYDfVNunc1tf9MMAkzx1Hk3|2^48 UNENCRYPTED_SRTCP\n\r"
                       "a=sendrecv\n\r");
}

class MediaConnectionTest : public ::testing::Test, public wme::IWmeMediaConnectionSink
{
public:
    MediaConnectionTest() : m_pMediaConn(NULL), m_status(WmeSession_Disconnected) {}
    
protected:
    const int audio_mid = 1;
    const int video_mid = 0;
    const int sharing_mid = 2;
    virtual void SetUp() {
        m_sdp.clear();
        testcount ++;
        if (testcount % 2 == 0) {
            InitMediaEngine(true);
        }
        else
        {
            InitMediaEngine(false);
        }
        ASSERT_TRUE(CreateMediaConnection(&m_pMediaConn) == WME_S_OK);
        ASSERT_TRUE(m_pMediaConn != NULL);
        m_pMediaConn->SetSink(this);
        m_pMediaConn->AddMedia(WmeSessionType_Video, WmeDirection_SendRecv, video_mid, "");
        m_pMediaConn->AddMedia(WmeSessionType_Audio, WmeDirection_SendRecv, audio_mid, "");
        m_pMediaConn->AddMedia(WmeSessionType_ScreenShare, WmeDirection_SendRecv, sharing_mid, "");
    }
    
    virtual void TearDown() {
        SleepMsWithLoop(1);
        if(m_pMediaConn)
        {
            m_pMediaConn->SetSink(NULL);
			m_pMediaConn->Stop();
            m_pMediaConn->Release();
            m_pMediaConn = NULL;
        }
        for (auto it : m_tracks) {
            it.second->Release();
        }
        m_tracks.clear();
#ifdef CM_MAC
        SleepMsWithLoop(1);
#endif
        UninitMediaEngine();
    }
    
    json::Value SetAndGetParam(const string &media, const string &key, const json::Value& value) {
        json::Object video;
        video[key] = value;
        json::Object root;
        root[media] = video;
        string input = json::Serialize(root);
        int mid = (media == "audio")?1:0;
        WMERESULT ret = m_pMediaConn->SetParams(mid, input.c_str(), input.length());
        if(WME_FAILED(ret))
        {
            return value;
        }
        string output = m_pMediaConn->GetParams(mid);
        root = json::Deserialize(output);
        return root[media][key];
    }
    
    void TestBaseConfig(string type, IBaseConfig* baseConfig){
        WMERESULT cret;
        int i_result, i_expectResult;
        WmeCodecType ct_result, ct_expectResult;
        
        i_expectResult = 1;
        cret = baseConfig->SetMaxBandwidth(i_expectResult);
        ASSERT_TRUE(WME_SUCCEEDED(cret));
        i_result = baseConfig->GetMaxBandwidth();
        ASSERT_TRUE(i_result == i_expectResult);
        
        if(type == "audio"){
            ct_expectResult = WmeCodecType_OPUS;
        }else{
            ct_expectResult = WmeCodecType_AVC;
        }
        cret = baseConfig->SetPreferedCodec(ct_expectResult);
        ASSERT_TRUE(WME_SUCCEEDED(cret));
        ct_result = baseConfig->GetPreferedCodec();
        ASSERT_TRUE(ct_result == ct_expectResult);
        
        if(type == "audio"){
            ct_expectResult = WmeCodecType_OPUS;
        }else{
            ct_expectResult = WmeCodecType_AVC;
        }
        cret = baseConfig->SetSelectedCodec(ct_expectResult);
        ASSERT_TRUE(WME_SUCCEEDED(cret));
        ct_result = baseConfig->GetSelectedCodec();
        ASSERT_TRUE(ct_result == ct_expectResult);
        
    
        cret = baseConfig->OverrideMediaIPAddress("127.0.0.1", 1234);
        ASSERT_TRUE(WME_SUCCEEDED(cret));
        
        cret = baseConfig->EnableFileRender("./");
        ASSERT_TRUE(WME_SUCCEEDED(cret));
        
        cret = baseConfig->EnableFileCapture("./", true);
        ASSERT_TRUE(WME_SUCCEEDED(cret));
        
        WmeConnectionPolicy cp_result, cp_expectResult;
        cp_expectResult = TCPOnly;
        cret = baseConfig->SetIcePolicy(cp_expectResult);
        ASSERT_TRUE(WME_SUCCEEDED(cret));
        cp_result = baseConfig->GetIcePolicy();
        ASSERT_TRUE(cp_result == cp_expectResult);

		cret = baseConfig->SetAutoSwitchDefaultDeviceFlag(true);
		ASSERT_EQ(cret, WME_E_DEPRECATED); 
		cret = baseConfig->SetAutoSwitchDefaultDeviceFlag(false);
		ASSERT_EQ(cret, WME_E_DEPRECATED);
    }
    
    void TestBaseVideoConfig(IBaseVideoConfig* baseVideoConfig){
        WMERESULT cret;
        WmeCodecType codec;
        uint32_t uProfileLevelID, max_mbps, max_fs, max_fps, max_br;
        uint32_t r_uProfileLevelID, r_max_mbps, r_max_fs, r_max_fps, r_max_br;
        codec = WmeCodecType_AVC;
        uProfileLevelID =0x42001E;
        max_mbps = 40500;
        max_fs = 1620;
        max_fps = 3000;
        max_br = 10000;
        cret = baseVideoConfig->SetDecodeParams(codec, uProfileLevelID, max_mbps, max_fs, max_fps, max_br);
        ASSERT_TRUE(WME_SUCCEEDED(cret));
        cret = baseVideoConfig->GetDecodeParams(codec, &r_uProfileLevelID, &r_max_mbps, &r_max_fs, &r_max_fps, &r_max_br);
        ASSERT_TRUE(WME_SUCCEEDED(cret));
        ASSERT_TRUE(uProfileLevelID==r_uProfileLevelID);
        ASSERT_TRUE(max_mbps==r_max_mbps);
        ASSERT_TRUE(max_fs==r_max_fs);
        ASSERT_TRUE(max_fps==r_max_fps);
        ASSERT_TRUE(max_br==r_max_br);
        
        
        cret = baseVideoConfig->SetEncodeParams(codec, uProfileLevelID, max_mbps, max_fs, max_fps, max_br);
        ASSERT_TRUE(WME_SUCCEEDED(cret));
        cret = baseVideoConfig->GetEncodeParams(codec, &r_uProfileLevelID, &r_max_mbps, &r_max_fs, &r_max_fps, &r_max_br);
        ASSERT_TRUE(WME_SUCCEEDED(cret));
        ASSERT_TRUE(uProfileLevelID==r_uProfileLevelID);
        ASSERT_TRUE(max_mbps==r_max_mbps);
        ASSERT_TRUE(max_fs==r_max_fs);
        ASSERT_TRUE(max_fps==r_max_fps);
        ASSERT_TRUE(max_br==r_max_br);
        
        int i_result, i_expectResult;
        i_expectResult = 1;
        cret = baseVideoConfig->SetMaxPacketSize(i_expectResult);
        ASSERT_TRUE(WME_SUCCEEDED(cret));
        i_result = baseVideoConfig->GetMaxPacketSize();
        ASSERT_TRUE(i_result == i_expectResult);
        
        WmePacketizationMode ePacketizationMode, r_ePacketizationMode;
        ePacketizationMode = WmePacketizationMode_0;
        cret = baseVideoConfig->SetPacketizationMode(ePacketizationMode);
        ASSERT_TRUE(WME_SUCCEEDED(cret));
        r_ePacketizationMode = baseVideoConfig->GetPacketizationMode();
        ASSERT_TRUE(r_ePacketizationMode == ePacketizationMode);
        
        TestBaseConfig("video", baseVideoConfig);
    }
    
    IWmeMediaTrackBase* GetTrack(unsigned long mid, WmeDirection direction, WmeSessionType mediaType, int vid = 0) {
        return m_tracks[mid * 1000 + direction * 100 + vid * 30 + mediaType];
    }

	virtual void OnMediaReady(unsigned long mid, WmeDirection direction,
		WmeSessionType mediaType, IWmeMediaTrackBase *pTrack)
    {
        CM_INFO_TRACE_THIS("MediaConnectionTest, mid=" << mid << ", direction=" << direction << ", mediaType=" << mediaType);
        pTrack->AddRef();
        unsigned int vid = 0;
        pTrack->GetVid(vid);
        m_tracks[mid * 1000 + direction * 100 + vid * 30 + mediaType] = pTrack;
    }
	virtual void OnSDPReady(WmeSdpType sdpType, const char *sdp) {
        if(sdp != NULL)
            m_sdp = sdp;
	}
	virtual void OnMediaBlocked(unsigned long mid,unsigned long csi, bool blocked) {}
	virtual void OnDecodeSizeChanged(unsigned long mid,unsigned long csi, uint32_t uWidth, uint32_t uHeight) {}
    virtual void OnEncodeSizeChanged(unsigned long mid, uint32_t uWidth, uint32_t uHeight) {}
	virtual void OnSessionStatus(unsigned long mid, WmeSessionType mediaType, WmeSessionStatus status)
    {
        m_status = status;
    }
	virtual void OnNetworkStatus(WmeNetworkStatus status, WmeNetworkDirection direc) {}
    virtual void OnMediaStatus(unsigned long mid, unsigned long vid, WmeMediaStatus status, bool hasCSI, uint32_t csi) {}
    virtual void OnMediaError(unsigned long mid,unsigned long csi,int errorCode) {}
	virtual void OnError(int errorCode) {}
	virtual void OnCSIsChanged(unsigned long mid, unsigned int vid,
		const unsigned int* oldCSIArray, unsigned int oldCSICount,
		const unsigned int* newCSIArray, unsigned int newCSICount) {}

    IWmeMediaConnection* m_pMediaConn;
    WmeSessionStatus m_status;
    std::map<unsigned long, IWmeMediaTrackBase*> m_tracks;
    std::string m_sdp;
};

TEST_F(MediaConnectionTest, TestMediaPerformance_HW_Check)
{
    /////mediaCodec available
    json::Value root = json::Deserialize(android_mediacodec_true().c_str());
    ASSERT_EQ(root.GetType(), json::ObjectVal);
    
    // Get Media Performance
    CMediaPerformanceStaticControl* performance = CMediaPerformanceStaticControl::Instance();
    if (root.HasKey("video")) {
        performance->SetVideoSetting(root["video"].ToObject());
    }
    
    bool bHWEnable = false;
    // Mock mac
    performance->overrideSpec(0, 0, 0, 0, 0, "Mac", "mac", 10, 9);
    bHWEnable = performance->CheckHWAcceleration();
    ASSERT_EQ(bHWEnable, false);
    
    performance->overrideSpec(0, 0, 0, 0, 0, "Mac", "mac", 10, 10);
    bHWEnable = performance->CheckHWAcceleration();
    ASSERT_EQ(bHWEnable, true);
    
    performance->overrideSpec(0, 0, 0, 0, 0, "Mac", "mac", 11, 0);
    bHWEnable = performance->CheckHWAcceleration();
    ASSERT_EQ(bHWEnable, true);
    
    // Mock iPhone 5
    performance->overrideSpec(1000, 1300, 2, 2, 8.0, "iphone5", "ios", 0, 0);
    bHWEnable = performance->CheckHWAcceleration();
    ASSERT_EQ(bHWEnable, true);
    
    performance->overrideSpec(1000, 1300, 2, 2, 7.0, "iphone5", "ios", 0, 0);
    bHWEnable = performance->CheckHWAcceleration();
    ASSERT_EQ(bHWEnable, false);
    
    // Mock Nexus 1
    performance->overrideSpec(0, 0, 0, 0, 19, "Nexus 1", "android", 0, 0);
    bHWEnable = performance->CheckHWAcceleration();
    ASSERT_EQ(bHWEnable, true);
    
    performance->overrideSpec(0, 0, 0, 0, 18, "Nexus 1", "android", 0, 0);
    bHWEnable = performance->CheckHWAcceleration();
    ASSERT_EQ(bHWEnable, false);
    
    ///// mediaCodec unvailable
    root = json::Deserialize(android_mediacodec_false().c_str());
    ASSERT_EQ(root.GetType(), json::ObjectVal);
    if (root.HasKey("video")) {
        performance->SetVideoSetting(root["video"].ToObject());
    }
    
    // Mock Nexus 1
    performance->overrideSpec(0, 0, 0, 0, 19, "Nexus 1", "android", 0, 0);
    bHWEnable = performance->CheckHWAcceleration();
    ASSERT_EQ(bHWEnable, false);
}

WmeSimulcastRequest buildRequest(WmeVideoQualityType type, uint8_t vid) {
    WmeSimulcastRequest res;
    
     // Base request info
    res.eRequestPolicyType = WmeSCRRequestPolicyType_ActiveSpeaker;
    res.uRequestInfo.stActiveSpeakerInfo.uPriority = 255;
    res.uRequestInfo.stActiveSpeakerInfo.uSourceId = 0;
    res.uRequestInfo.stActiveSpeakerInfo.uGroupingAdjacencyId = 0;
    res.uRequestInfo.stActiveSpeakerInfo.bDuplicationFlag = false;
    res.eRequestCodecType = WmeSCRRequestCodecType_H264;
    res.uCodecCapability.stH264CodecCap.uPayloadType = 0x80;
    res.uCodecCapability.stH264CodecCap.uTemporalLayerCount = 0;
    
    WmeVideoEncodeCapability cap = {0};
    ConvCapabilityFromQuality(type, cap);
    
    res.uRequestInfo.stActiveSpeakerInfo.uSubsessionChannelId = vid;
    res.uRequestInfo.stActiveSpeakerInfo.uBitrate = cap.uMaxBitRate;
    res.uCodecCapability.stH264CodecCap.uMaxMBPS = cap.uMaxMBPS;
    res.uCodecCapability.stH264CodecCap.uMaxFS = cap.uMaxFS;
    res.uCodecCapability.stH264CodecCap.uMaxFPS = cap.uMaxFPS;
    
    return res;
}

TEST_F(MediaConnectionTest, TestDPCMultiStream)
{
    unique_ptr<CMediaPerformanceDynControl> DPC(new CMediaPerformanceDynControl());
    vector<WmeSimulcastRequest> scrSetByClient;
    WmeVideoPerformance outputLocalPerf;
    vector<WmeSimulcastRequest> outputRemotePerf;
    DynMediaPriority priority;
    
    // Initialize static performance levels
    CMediaPerformanceStaticControl::Instance()->overrideSpec(0, 0, 2, 0, 7.1, "iPhone5,3", "ios", 0, 0);
    WmeVideoPerformance sendPerform = CMediaPerformanceStaticControl::Instance()->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    WmeVideoPerformance receivePerform = CMediaPerformanceStaticControl::Instance()->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    /*****
    ** Test: Remote tracks: 360p, 180p, 180p, 180p. Local track: 360p
    *****/
    // Set requests
    uint8_t vid = 0;
    scrSetByClient.push_back(buildRequest(WmeVideoQuality_SD, vid)); // 360p
    vid++;
    for (int i = 0; i < 3; ++i) { // 180p
        scrSetByClient.push_back(buildRequest(WmeVideoQuality_LD, vid));
        vid++;
    }
    
    // Query, test results
    bool needUpdate = false;
    DPC->QueryDynPerformance(scrSetByClient, true, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoEnc);
    ASSERT_EQ(outputLocalPerf, WmeVideoPerformanceMedium);
    DPC->SetCurrSendVideoQuality(outputLocalPerf);
    
    // RX: 360p, 180p, 180p, 180p. TX: 180p
    DPC->QueryDynPerformance(scrSetByClient, true, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoDec);
    ASSERT_EQ(outputRemotePerf[0].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[1].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[2].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[3].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    DPC->SetCurrVideoSCR(outputRemotePerf);
    
    // RX: 180p, 180p, 180p, 180p. TX: 180p
    DPC->QueryDynPerformance(scrSetByClient, true, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoEnc);
    ASSERT_EQ(outputLocalPerf, WmeVideoPerformanceLow);
    DPC->SetCurrSendVideoQuality(outputLocalPerf);
    
    // RX: 180p, 180p, 180p, 180p. TX: 90p
    DPC->QueryDynPerformance(scrSetByClient, true, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoDec);
    ASSERT_EQ(outputRemotePerf[0].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[1].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[2].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[3].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    DPC->SetCurrVideoSCR(outputRemotePerf);
    
    // RX: 180p, 180p, 180p, 90p. TX: 90p
    DPC->QueryDynPerformance(scrSetByClient, true, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoDec);
    ASSERT_EQ(outputRemotePerf[0].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[1].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[2].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    ASSERT_EQ(outputRemotePerf[3].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    DPC->SetCurrVideoSCR(outputRemotePerf);
    
    // RX: 180p, 180p, 90p, 90p. TX: 90p
    DPC->QueryDynPerformance(scrSetByClient, true, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoDec);
    ASSERT_EQ(outputRemotePerf[0].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[1].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    ASSERT_EQ(outputRemotePerf[2].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    ASSERT_EQ(outputRemotePerf[3].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    DPC->SetCurrVideoSCR(outputRemotePerf);
    
    // RX: 180p, 90p, 90p, 90p. TX: 90p
    DPC->QueryDynPerformance(scrSetByClient, true, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoDec);
    ASSERT_EQ(outputRemotePerf[0].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    ASSERT_EQ(outputRemotePerf[1].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    ASSERT_EQ(outputRemotePerf[2].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    ASSERT_EQ(outputRemotePerf[3].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    DPC->SetCurrVideoSCR(outputRemotePerf);
    
    // RX: 90p, 90p, 90p, 90p. TX: 90p
    DPC->QueryDynPerformance(scrSetByClient, true, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, false);
    
    // RX: 90p, 90p, 90p, 90p. TX: 90p
    DPC->QueryDynPerformance(scrSetByClient, true, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, false);
    
    // RX: 90p, 90p, 90p, 90p. TX: 90p
    DPC->QueryDynPerformance(scrSetByClient, false, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoDec);
    ASSERT_EQ(outputRemotePerf[0].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[1].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    ASSERT_EQ(outputRemotePerf[2].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    ASSERT_EQ(outputRemotePerf[3].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    DPC->SetCurrVideoSCR(outputRemotePerf);
    
    // RX: 180p, 90p, 90p, 90p. TX: 90p
    DPC->QueryDynPerformance(scrSetByClient, false, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoDec);
    ASSERT_EQ(outputRemotePerf[0].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[1].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[2].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    ASSERT_EQ(outputRemotePerf[3].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    DPC->SetCurrVideoSCR(outputRemotePerf);
    
    // RX: 180p, 180p, 90p, 90p. TX: 90p
    DPC->QueryDynPerformance(scrSetByClient, false, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoDec);
    ASSERT_EQ(outputRemotePerf[0].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[1].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[2].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[3].uCodecCapability.stH264CodecCap.uMaxMBPS, 1800);
    DPC->SetCurrVideoSCR(outputRemotePerf);
    
    // RX: 180p, 180p, 180p, 90p. TX: 90p
    DPC->QueryDynPerformance(scrSetByClient, false, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoDec);
    ASSERT_EQ(outputRemotePerf[0].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[1].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[2].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[3].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    DPC->SetCurrVideoSCR(outputRemotePerf);
    
    // RX: 180p, 180p, 180p, 180p. TX: 90p
    DPC->QueryDynPerformance(scrSetByClient, false, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoEnc);
    ASSERT_EQ(outputLocalPerf, WmeVideoPerformanceMedium);
    DPC->SetCurrSendVideoQuality(outputLocalPerf);
    
    
    // RX: 180p, 180p, 180p, 180p. TX: 180p
    DPC->QueryDynPerformance(scrSetByClient, false, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoDec);
    ASSERT_EQ(outputRemotePerf[0].uCodecCapability.stH264CodecCap.uMaxMBPS, 27600);
    ASSERT_EQ(outputRemotePerf[1].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[2].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputRemotePerf[3].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    DPC->SetCurrVideoSCR(outputRemotePerf);
    
    // RX: 360p, 180p, 180p, 180p. TX: 180p
    DPC->QueryDynPerformance(scrSetByClient, false, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoEnc);
    ASSERT_EQ(outputLocalPerf, WmeVideoPerformanceNormal);
    DPC->SetCurrSendVideoQuality(outputLocalPerf);

    // RX: 360p, 180p, 180p, 180p. TX: 360p
    DPC->QueryDynPerformance(scrSetByClient, false, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, false);
    
    // RX: 360p, 180p, 180p, 180p. TX: 360p
    DPC->QueryDynPerformance(scrSetByClient, false, needUpdate, priority, outputLocalPerf, outputRemotePerf);
    ASSERT_EQ(needUpdate, false);
}

TEST_F(MediaConnectionTest, TestDPCMultiStream_ClientChangesSCR)
{
    // Initialize static performance levels,
    // both send and recv: 360P.
    CMediaPerformanceStaticControl::Instance()->overrideSpec(0, 0, 2, 0, 7.1, "iPhone5,3", "ios", 0, 0);
    WmeVideoPerformance sendPerform = CMediaPerformanceStaticControl::Instance()->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    WmeVideoPerformance receivePerform = CMediaPerformanceStaticControl::Instance()->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    unique_ptr<CMediaPerformanceDynControl> DPC(new CMediaPerformanceDynControl());

    /*****
     ** at the begining, the client subscribes only 1 request.
     ** Test: Remote tracks: 360p. Local track: 360p
     *****/
    uint8_t vid = 0;
    vector<WmeSimulcastRequest> scrSetByClient;
    scrSetByClient.push_back(buildRequest(WmeVideoQuality_SD, vid)); // 360p
    
    bool needUpdate = false;
    DynMediaPriority priority;
    WmeVideoPerformance outputLocalPerf;
    vector<WmeSimulcastRequest> outputSCR;
    
    // RX: 360p. TX: 360p
    ASSERT_EQ(WME_S_OK, DPC->QueryDynPerformance(scrSetByClient, false, needUpdate, priority, outputLocalPerf, outputSCR));
    ASSERT_EQ(needUpdate, false); // because both RX and TX have reached to their upper bound.
    
    /*****
     ** in the second step, the client subscribes 3 requests.
     ** Test: Remote tracks: 720p, 180p, 180p. Local track: 360p
     *****/
    vid = 0;
    scrSetByClient.clear();
    scrSetByClient.push_back(buildRequest(WmeVideoQuality_HD_720P, vid)); // 720p
    vid++;
    for (int i = 0; i < 2; ++i) { // 180p
        scrSetByClient.push_back(buildRequest(WmeVideoQuality_LD, vid));
        vid++;
    }
    
    // RX: 720p, 180p, 180p. TX: 360p
    ASSERT_EQ(WME_S_OK, DPC->QueryDynPerformance(scrSetByClient, true, needUpdate, priority, outputLocalPerf, outputSCR));
    ASSERT_EQ(needUpdate, true);
    ASSERT_EQ(priority, VideoDec);
    ASSERT_EQ(outputSCR[0].uCodecCapability.stH264CodecCap.uMaxMBPS, 27600);
    ASSERT_EQ(outputSCR[1].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    ASSERT_EQ(outputSCR[2].uCodecCapability.stH264CodecCap.uMaxMBPS, 7200);
    DPC->SetCurrVideoSCR(outputSCR);
}

TEST_F(MediaConnectionTest, TestMediaPerformance)
{
    // Get Media Performance
    CMediaPerformanceStaticControl* performance = CMediaPerformanceStaticControl::Instance();
    // performance->SetCapabilities(root); Using default capabilities from MediaPerformance
    
    WmeVideoPerformance sendPerform;
    WmeVideoPerformance receivePerform;
    
    performance->overrideSpec(0, 0, 2, 0, 7.1, "iPhone5,3", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPhone7,1", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPhone5,3", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 1, 0, 7.1, "iPhone5,3", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 1, 0, 0, " ", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(4000, 0, 2, 0, 4.5, "motorola", "android", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(1600, 0, 4, 0, 0, "nexus6", "android", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 3, 0, 0, "lg", "android", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 1, 0, 0, "lg", "android", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceLow);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(3000, 0, 4, 0, 0, "", "android", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(3000, 0, 4, 0, 0, "", "android", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);

	performance->overrideSpec(2000, 0, 4, 0, 0, " ", "win", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(1001, 1600, 2, 2, 0, " ", "win", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(1001, 0, 2, 0, 0, " ", "win", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(1001, 0, 2, 0, 0, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);

    performance->overrideSpec(0, 0, 1, 0, 0, " ", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    
    performance->overrideSpec(1000, 2400, 1, 0, 0, " ", "win", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    
    performance->overrideSpec(1000, 2400, 1, 0, 0, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    
    performance->overrideSpec(2000, 2800, 2, 1, 0, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(1000, 2400, 1, 0, 0, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(500, 500, 0, 0, 1, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceLow);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(4000, 1800, 2, 2, 0, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(8000, 2000, 2, 2, 0, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPhone4,1", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPhone5,1", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPhone6,1", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPad4,1", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPad3,6", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPad5,4", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
}

TEST_F(MediaConnectionTest, TestMediaPerformance_RoundCpuFrequency)
{
    ASSERT_EQ(2400, CMediaPerformanceStaticControl::RoundCpuFrequency(2394));
    ASSERT_EQ(1600, CMediaPerformanceStaticControl::RoundCpuFrequency(1600));
    ASSERT_EQ(100, CMediaPerformanceStaticControl::RoundCpuFrequency(100));
    ASSERT_EQ(100, CMediaPerformanceStaticControl::RoundCpuFrequency(33));
    ASSERT_EQ(0, CMediaPerformanceStaticControl::RoundCpuFrequency(0));
}

// test the configurations passed from cloud which will override the local default one.
TEST_F(MediaConnectionTest, TestMediaPerformance_Cloud)
{
    // Get Media Performance
    CMediaPerformanceStaticControl* performance = CMediaPerformanceStaticControl::Instance();
    
    // Override
    json::Object curObject = json::Deserialize(g_cloud_perf_settings).ToObject();
    
    // test ios
    ASSERT_EQ(true, curObject.HasKey("ios"));
    ASSERT_EQ(true, curObject["ios"].HasKey("defaults"));
    performance->SetCapabilities(curObject["ios"]["defaults"]);
    
    WmeVideoPerformance sendPerform;
    WmeVideoPerformance receivePerform;
    
    performance->overrideSpec(0, 0, 2, 0, 7.1, "iPhone5,3", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPhone7,1", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPhone5,3", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 1, 0, 7.1, "iPhone5,3", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);

    performance->overrideSpec(0, 0, 1, 0, 7.1, "iPhone5,3", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(0, 0, 1, 0, 0, " ", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);

    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPhone4,1", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPhone5,1", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPhone6,1", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPad4,1", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPad3,6", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 2, 0, 8.0, "iPad5,4", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(0, 0, 1, 0, 0, " ", "ios", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    
    // test android
    ASSERT_EQ(true, curObject.HasKey("android"));
    ASSERT_EQ(true, curObject["android"].HasKey("defaults"));
    performance->SetCapabilities(curObject["android"]["defaults"]);
    
    performance->overrideSpec(4000, 0, 2, 0, 4.5, "motorola", "android", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(1600, 0, 4, 0, 0, "nexus6", "android", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 3, 0, 0, "lg", "android", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(0, 0, 1, 0, 0, "lg", "android", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceLow);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(3000, 0, 4, 0, 0, "", "android", 0, 0);
    sendPerform = performance->QuerySendPerformance(true);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(true);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(3000, 0, 4, 0, 0, "", "android", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    
    // test win
    ASSERT_EQ(true, curObject.HasKey("win"));
    ASSERT_EQ(true, curObject["win"].HasKey("defaults"));
    performance->SetCapabilities(curObject["win"]["defaults"]);
    
    performance->overrideSpec(2000, 0, 4, 0, 0, " ", "win", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(1001, 1600, 2, 2, 0, " ", "win", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(1001, 0, 2, 0, 0, " ", "win", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(1000, 2400, 1, 0, 0, " ", "win", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    
    
    // test mac
    ASSERT_EQ(true, curObject.HasKey("mac"));
    ASSERT_EQ(true, curObject["mac"].HasKey("defaults"));
    performance->SetCapabilities(curObject["mac"]["defaults"]);
    
    performance->overrideSpec(1001, 0, 2, 0, 0, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(1000, 2400, 1, 0, 0, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    
    performance->overrideSpec(2000, 2800, 2, 1, 0, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(1001, 0, 4, 0, 0, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);

    performance->overrideSpec(1001, 2400, 2, 2, 0, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(1000, 2400, 1, 0, 0, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceMedium);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(500, 500, 0, 0, 1, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceLow);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceNormal);
    
    performance->overrideSpec(4000, 1800, 2, 2, 0, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
    
    performance->overrideSpec(8000, 2000, 2, 2, 0, " ", "mac", 0, 0);
    sendPerform = performance->QuerySendPerformance(false);
    ASSERT_EQ(sendPerform, WmeVideoPerformanceNormal);
    receivePerform = performance->QueryRecvPerformance(false);
    ASSERT_EQ(receivePerform, WmeVideoPerformanceHigh);
}

TEST_F(MediaConnectionTest, TestCreateOffer)
{
    // create offer
    WMERESULT ret = m_pMediaConn->CreateOffer();
    ASSERT_TRUE(WME_SUCCEEDED(ret));
	m_pMediaConn->UpdateMedia(WmeDirection_Inactive, 0);
	m_pMediaConn->UpdateMedia(WmeDirection_Inactive, 1);
	ret = m_pMediaConn->CreateOffer();
    ASSERT_TRUE(WME_SUCCEEDED(ret));
   
    // set anwser
    ret = m_pMediaConn->SetReceivedSDP(WmeSdp_Offer, ex90_sdp_answer().c_str());
    ASSERT_FALSE(WME_SUCCEEDED(ret));
    ret = m_pMediaConn->SetReceivedSDP(WmeSdp_Anwser, ex90_sdp_answer().c_str());
    ASSERT_TRUE(WME_SUCCEEDED(ret));
    
    // negotiated and create update offer
    ret = m_pMediaConn->CreateOffer();
    ASSERT_TRUE(WME_SUCCEEDED(ret));
    
    // set upate anwser
    ret = m_pMediaConn->SetReceivedSDP(WmeSdp_Anwser, ex90_sdp_answer().c_str());
    ASSERT_TRUE(WME_SUCCEEDED(ret));
    
    // set update offer
    ret = m_pMediaConn->SetReceivedSDP(WmeSdp_Offer, ex90_sdp_answer().c_str());
    ASSERT_TRUE(WME_SUCCEEDED(ret));
    
    // create update answer
    ret = m_pMediaConn->CreateAnswer();
    ASSERT_TRUE(WME_SUCCEEDED(ret));
}

TEST_F(MediaConnectionTest, TestCreateAnwser)
{
    // create anwser
    WMERESULT ret = m_pMediaConn->CreateAnswer();
    ASSERT_FALSE(WME_SUCCEEDED(ret));
    
    // set offer
    ret = m_pMediaConn->SetReceivedSDP(WmeSdp_Anwser, ex90_sdp_answer().c_str());
    ASSERT_FALSE(WME_SUCCEEDED(ret));
    ret = m_pMediaConn->SetReceivedSDP(WmeSdp_Offer, ex90_sdp_answer().c_str());
    ASSERT_TRUE(WME_SUCCEEDED(ret));
    
    // create anwser
    ret = m_pMediaConn->CreateOffer();
    ASSERT_FALSE(WME_SUCCEEDED(ret));
    ret = m_pMediaConn->CreateAnswer();
    ASSERT_TRUE(WME_SUCCEEDED(ret));
    
    // negotiated and create update offer
    ret = m_pMediaConn->CreateOffer();
    ASSERT_TRUE(WME_SUCCEEDED(ret));
    
    // set upate anwser
    ret = m_pMediaConn->SetReceivedSDP(WmeSdp_Anwser, ex90_sdp_answer().c_str());
    ASSERT_TRUE(WME_SUCCEEDED(ret));
    
    // set update offer
    ret = m_pMediaConn->SetReceivedSDP(WmeSdp_Offer, ex90_sdp_answer().c_str());
    ASSERT_TRUE(WME_SUCCEEDED(ret));
    
    // create update answer
    ret = m_pMediaConn->CreateAnswer();
    ASSERT_TRUE(WME_SUCCEEDED(ret));
}

TEST_F(MediaConnectionTest, TestParam)
{
    ASSERT_TRUE(SetAndGetParam("video", "enableQos", json::Value(true)).ToBool());
    ASSERT_TRUE(SetAndGetParam("video", "uMaxBandwidth", json::Value(160000)).ToInt() == 160000);
    ASSERT_TRUE(SetAndGetParam("video", "uMaxPacketSize", json::Value(1200)).ToInt() == 1200);
    ASSERT_TRUE(SetAndGetParam("video", "ePacketizationMode", json::Value(1)).ToInt() == 1);
    ASSERT_TRUE(SetAndGetParam("video", "uInitBandWidth", json::Value(100000)).ToInt() == 100000);
    ASSERT_TRUE(SetAndGetParam("video", "supportCmulti", json::Value(true)).ToBool());
    ASSERT_TRUE(SetAndGetParam("audio", "enableAGC", json::Value(true)).ToBool());
    ASSERT_TRUE(SetAndGetParam("audio", "enableEC", json::Value(true)).ToBool());
    ASSERT_TRUE(SetAndGetParam("audio", "enableVAD", json::Value(true)).ToBool());
    ASSERT_TRUE(SetAndGetParam("audio", "enableNS", json::Value(true)).ToBool());
    json::Array array;
    array.push_back("H264");
    ASSERT_TRUE(SetAndGetParam("audio", "selectedCodec", json::Value(array)).ToArray().HasValue("H264"));
    ASSERT_TRUE(SetAndGetParam("video", "selectedCodec", json::Value(array)).ToArray().HasValue("H264"));
    ASSERT_TRUE(SetAndGetParam("video", "preferedCodec", json::Value("H264")).ToString() == "H264");
    
    json::Object in;
    in["codec"] = "H264";
    in["uProfileLevelID"] = "42001E";
    in["max-mbps"] = 40500;
    in["max-fs"] = 1620;
    in["max-fps"] = 3000;
    in["max-br"] = 10000;
    json::Array encode;
    encode.push_back(in);
    json::Value out = SetAndGetParam("video", "encodeCodec", json::Value(encode)).ToArray()[0];
    ASSERT_TRUE(out["codec"].ToString() == "H264");
    ASSERT_TRUE(out["uProfileLevelID"].ToString() == "42001E");
    ASSERT_TRUE(out["max-mbps"].ToInt() == 40500);
    ASSERT_TRUE(out["max-fs"].ToInt() == 1620);
    ASSERT_TRUE(out["max-fps"].ToInt() == 3000);
    ASSERT_TRUE(out["max-br"].ToInt() == 10000);
    
    json::Array decode;
    decode.push_back(in);
    out = SetAndGetParam("video", "decodeCodec", json::Value(decode)).ToArray()[0];
    ASSERT_TRUE(out["codec"].ToString() == "H264");
    ASSERT_TRUE(out["uProfileLevelID"].ToString() == "42001E");
    ASSERT_TRUE(out["max-mbps"].ToInt() == 40500);
    ASSERT_TRUE(out["max-fs"].ToInt() == 1620);
    ASSERT_TRUE(out["max-fps"].ToInt() == 3000);
    ASSERT_TRUE(out["max-br"].ToInt() == 10000);
}

TEST_F(MediaConnectionTest, TestSetNetworkNotificationParam)
{
    WMERESULT ret;
    
    ASSERT_TRUE(m_pMediaConn->GetGlobalConfig() != NULL);
    
    ret = m_pMediaConn->GetGlobalConfig()->SetNetworkNotificationParam(WmeNetwork_bad, DIRECTION_UPLINK, 0);
    ASSERT_EQ(ret, WME_S_OK);
    
    ret = m_pMediaConn->GetGlobalConfig()->SetNetworkNotificationParam(WmeNetwork_bad, DIRECTION_DOWNLINK, 1);
    ASSERT_EQ(ret, WME_S_OK);
    
    ret = m_pMediaConn->GetGlobalConfig()->SetNetworkNotificationParam(WmeNetwork_bad, DIRECTION_BOTHLINK, 1);
    ASSERT_EQ(ret, WME_S_OK);
    
    ret = m_pMediaConn->GetGlobalConfig()->SetNetworkNotificationParam(WmeNetwork_video_off, DIRECTION_UPLINK, 0);
    ASSERT_EQ(ret, WME_S_OK);
    
    ret = m_pMediaConn->GetGlobalConfig()->SetNetworkNotificationParam(WmeNetwork_video_off, DIRECTION_DOWNLINK, 1);
    ASSERT_EQ(ret, WME_S_OK);
    
    ret = m_pMediaConn->GetGlobalConfig()->SetNetworkNotificationParam(WmeNetwork_video_off, DIRECTION_BOTHLINK, 1);
    ASSERT_EQ(ret, WME_S_OK);
    
    ret = m_pMediaConn->GetGlobalConfig()->SetNetworkNotificationParam(WmeNetwork_recovered, DIRECTION_UPLINK, 0);
    ASSERT_EQ(ret, WME_S_OK);
    
    ret = m_pMediaConn->GetGlobalConfig()->SetNetworkNotificationParam(WmeNetwork_recovered, DIRECTION_DOWNLINK, 1);
    ASSERT_EQ(ret, WME_S_OK);
    
    ret = m_pMediaConn->GetGlobalConfig()->SetNetworkNotificationParam(WmeNetwork_recovered, DIRECTION_BOTHLINK, 1);
    ASSERT_EQ(ret, WME_S_OK);
}

TEST_F(MediaConnectionTest, TestGetNetworkIndex)
{
    ASSERT_EQ(m_pMediaConn->GetNetworkIndex(DIRECTION_UPLINK), NETWORK_UNKNOWN);
    ASSERT_EQ(m_pMediaConn->GetNetworkIndex(DIRECTION_DOWNLINK), NETWORK_UNKNOWN);
    ASSERT_EQ(m_pMediaConn->GetNetworkIndex(DIRECTION_BOTHLINK), NETWORK_UNKNOWN);
}

TEST_F(MediaConnectionTest, TestGlobalConfig)
{
    WMERESULT cret;
    bool b_result, b_expectResult;
    float f_result, f_expectResult;
    int i_result, i_expectResult;
    IGlobalConfig* globalConfig = m_pMediaConn->GetGlobalConfig();
    
    b_expectResult = true;
    cret = globalConfig->EnableQos(b_expectResult);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    b_result = globalConfig->IsQosEnabled();
    ASSERT_TRUE(b_result == b_expectResult);
    
    b_expectResult = true;
    cret = globalConfig->EnableMultiStream(b_expectResult);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    b_result = globalConfig->IsMultiStreamEnabled();
    ASSERT_TRUE(b_result == b_expectResult);
    
    f_expectResult = 1.0;
    cret = globalConfig->SetQoSMaxLossRatio(f_expectResult);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    f_result = globalConfig->GetQoSMaxLossRatio();
    ASSERT_TRUE(f_result == f_expectResult);
    
    i_expectResult = 8;//bits
    cret = globalConfig->SetQoSMinBandwidth(i_expectResult);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    i_result = globalConfig->GetQoSMinBandwidth();
    ASSERT_TRUE(i_result == i_expectResult);
    
    b_expectResult = true;
    cret = globalConfig->EnableICE(b_expectResult);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    b_result = globalConfig->IsICEEnabled();
    ASSERT_TRUE(b_result == b_expectResult);
    
    b_expectResult = true;
    cret = globalConfig->EnableSRTP(b_expectResult);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    b_result = globalConfig->IsSRTPEnabled();
    ASSERT_TRUE(b_result == b_expectResult);
    
    b_expectResult = true;
    cret = globalConfig->EnableRTCPMux(b_expectResult);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    b_result = globalConfig->IsRTCPMuxEnabled();
    ASSERT_TRUE(b_result == b_expectResult);
    
    string str_param;
    str_param = "{\"audio\":{},\"video\":{}}";
    cret = globalConfig->SetDeviceMediaSettings(str_param.c_str());
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    
    str_param = "";
    cret = globalConfig->SetDeviceMediaSettings(str_param.c_str());
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    
    str_param = "{}";
    cret = globalConfig->SetDeviceMediaSettings(str_param.c_str());
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    
    str_param = "{audio:{}}";
    cret = globalConfig->SetDeviceMediaSettings(str_param.c_str());
    ASSERT_TRUE(WME_SUCCEEDED(cret));

    b_expectResult = true;
    cret = globalConfig->EnableBGRA(b_expectResult);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    b_result = globalConfig->IsBGRAEnabled();
    ASSERT_TRUE(b_result == b_expectResult);
}

TEST_F(MediaConnectionTest, TestAudioConfig)
{
    WMERESULT cret;
    bool b_result, b_expectResult;
    IAudioConfig* audioConfig = m_pMediaConn->GetAudioConfig(1);

    //EnableAGC can not work on IOS and Android. We should add code to separate this case.
    b_expectResult = true;
    cret = audioConfig->EnableAGC(b_expectResult);
    if(WME_SUCCEEDED(cret)){
        b_result = audioConfig->IsAGCEnabled();
        ASSERT_TRUE(b_result == b_expectResult);
    }

    
    b_expectResult = true;
    cret = audioConfig->EnableEC(b_expectResult);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    b_result = audioConfig->IsECEnabled();
    ASSERT_TRUE(b_result == b_expectResult);
    
    b_expectResult = true;
    cret = audioConfig->EnableVAD(b_expectResult);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    b_result = audioConfig->IsVADEnabled();
    ASSERT_TRUE(b_result == b_expectResult);
    
    b_expectResult = true;
    cret = audioConfig->EnableNS(b_expectResult);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    b_result = audioConfig->IsNSEnabled();
    ASSERT_TRUE(b_result == b_expectResult);
    
    cret = audioConfig->EnableKeyDumpFiles(0);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    
	b_expectResult = true;
	cret = audioConfig->EnableDAGC(b_expectResult);
	ASSERT_TRUE(WME_SUCCEEDED(cret));
	b_result = audioConfig->IsDAGCEnabled();
	ASSERT_TRUE(b_result == b_expectResult);

	uint32_t u_expectResult = 40000, u_Result = 0;
	cret = audioConfig->SetCodecBandwidth(u_expectResult);
	ASSERT_TRUE(WME_SUCCEEDED(cret));
	cret = audioConfig->GetCodecBandwidth(u_Result);
	ASSERT_TRUE(WME_SUCCEEDED(cret));
	ASSERT_TRUE(u_expectResult == u_Result);

	cret = audioConfig->SetAutoSwitchDefaultMicrophoneDeviceFlag(true);
	ASSERT_EQ(cret, WME_S_OK);
	cret = audioConfig->SetAutoSwitchDefaultSpeakerDeviceFlag(true);
	ASSERT_EQ(cret, WME_S_OK);
    

    // some platorm doesn't support TC, Alpha, so only check the one can be set successfully.
    WmeAecType type = WmeAecTypeNone;
    WmeAecType gettype = WmeAecTypeWmeDefault;
    cret = audioConfig->SetECType(type);
    if(cret == WME_S_OK)
    {
        audioConfig->GetECType(gettype);
        ASSERT_EQ(type, gettype);
    }
    
    type = WmeAecTypeWmeDefault;
    gettype = WmeAecTypeNone;
    cret = audioConfig->SetECType(type);
    if(cret == WME_S_OK)
    {
        audioConfig->GetECType(gettype);
        ASSERT_EQ(type, gettype);
    }
    
    type = WmeAecTypeAlpha;
    gettype = WmeAecTypeNone;
    cret = audioConfig->SetECType(type);
    if(cret == WME_S_OK)
    {
        audioConfig->GetECType(gettype);
        ASSERT_EQ(type, gettype);
    }
    
    type = WmeAecTypeBuildin;
    gettype = WmeAecTypeNone;
    cret = audioConfig->SetECType(type);
    if(cret == WME_S_OK)
    {
        audioConfig->GetECType(gettype);
        ASSERT_EQ(type, gettype);
    }
    
    type = WmeAecTypeTc;
    gettype = WmeAecTypeNone;
    cret = audioConfig->SetECType(type);
    if(cret == WME_S_OK)
    {
        audioConfig->GetECType(gettype);
        ASSERT_EQ(type, gettype);
    }
   
    
    TestBaseConfig("audio",(IBaseConfig*)audioConfig);
}

TEST_F(MediaConnectionTest, TestVideoConfig)
{
    WMERESULT cret;
    bool b_result, b_expectResult;
    IVideoConfig* videoConfig = m_pMediaConn->GetVideoConfig(0);
    
    TestBaseVideoConfig((IBaseVideoConfig*)videoConfig);
}

TEST_F(MediaConnectionTest, TestShareConfig)
{
    IShareConfig* shareConfig = m_pMediaConn->GetShareConfig(2);
    TestBaseVideoConfig((IBaseVideoConfig*)shareConfig);
}

TEST_F(MediaConnectionTest, TestMultistreamAudioTrack)
{
    CMediaTrack* pTrack = NULL;
    CMultistreamAudioTrack* audioTracks = new CMultistreamAudioTrack(NULL, 100, TT_NETWORK);
    for(int i=0;i<3; ++i){
        audioTracks->Push(i,pTrack);
    }
    
    WMERESULT cret;
    cret = audioTracks->Mute();
    ASSERT_TRUE(WME_FAILED(cret));
    
    cret = audioTracks->Unmute();
    ASSERT_TRUE(WME_FAILED(cret));
    
    bool bMute = true;
    cret = audioTracks->Start(bMute);
    ASSERT_TRUE(WME_FAILED(cret));
    
    cret = audioTracks->Stop();
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    
    cret = audioTracks->SetCaptureDevice(NULL);
    ASSERT_TRUE(WME_FAILED(cret));
    
    cret = audioTracks->SetPlayoutDevice(NULL);
    ASSERT_TRUE(WME_FAILED(cret));
    
    cret = audioTracks->AddScreenSource(NULL);
    ASSERT_TRUE(WME_FAILED(cret));
    
    cret = audioTracks->SetExternalScreenCapturer(NULL);
    ASSERT_TRUE(WME_FAILED(cret));
    
    cret = audioTracks->RemoveScreenSource(NULL);
    ASSERT_TRUE(WME_FAILED(cret));
    
    cret = audioTracks->SetRenderMode(WmeRenderModeFill);
    ASSERT_TRUE(WME_FAILED(cret));
    
    cret = audioTracks->AddRenderWindow(NULL);
    ASSERT_TRUE(WME_FAILED(cret));
    
    cret = audioTracks->RemoveRenderWindow(NULL);
    ASSERT_TRUE(WME_FAILED(cret));
    
    cret = audioTracks->UpdateRenderWindow(NULL);
    ASSERT_TRUE(WME_FAILED(cret));

    cret = audioTracks->AddExternalRender(NULL);
    ASSERT_TRUE(WME_FAILED(cret));
    
    cret = audioTracks->RemoveExternalRender(NULL);
    ASSERT_TRUE(WME_FAILED(cret));
    
    unsigned int pCSIArray[100];
    unsigned int csiCount = 100;
    cret = audioTracks->GetCSI(pCSIArray, csiCount);
    ASSERT_TRUE(WME_FAILED(cret));
    
    unsigned int vid;
    cret = audioTracks->GetVid(vid);
    ASSERT_TRUE(WME_FAILED(cret));

    WmeVideoQualityType type = WmeVideoQuality_SLD;
    cret = audioTracks->RequestVideo(type);
    ASSERT_TRUE(WME_FAILED(cret));
    
    WmeMediaStatus status;
    cret = audioTracks->GetMediaStatus(status);
    ASSERT_TRUE(WME_FAILED(cret) || WME_S_FALSE == cret);

    unsigned int pTotalCSIArray[100];
    unsigned int totalCSICount = 100;
    
    vid = 0;
    csiCount = 2;
    pCSIArray[0]=1;
    pCSIArray[1]=2;
    audioTracks->GetTotalCSI(vid, pCSIArray, csiCount, pTotalCSIArray, totalCSICount);
    ASSERT_TRUE(totalCSICount == 0);
    audioTracks->Clear();
}

//These test cases are not valid for MAC because MAC we link ssl as static library in each framework.
#if !defined (CM_MAC)
//test multi-thread support for OpenSSL
TEST(MediaConnection, TestMultiThreadForOpenSSL_internal)
{
    ASSERT_TRUE(!CRYPTO_get_locking_callback());
    
    wme::InitMediaEngine(true);
    EXPECT_TRUE(CRYPTO_get_locking_callback());
    //EXPECT_TRUE(g_useThreadForOpenSSL == true);
    
    wme::UninitMediaEngine();
    EXPECT_TRUE(!CRYPTO_get_locking_callback());
}

void pthreads_locking_callback_test(int mode, int type, const char *file, int line)
{
    //nothing to do
}

TEST(MediaConnection, TestMultiThreadForOpenSSL_external)
{
    ASSERT_TRUE(!CRYPTO_get_locking_callback());
    
    CRYPTO_set_locking_callback((void (*)(int, int, const char *, int))pthreads_locking_callback_test);
    
    wme::InitMediaEngine(true);
    EXPECT_TRUE(CRYPTO_get_locking_callback());
    //EXPECT_TRUE(g_useThreadForOpenSSL == false);
    
    wme::UninitMediaEngine();
    EXPECT_TRUE(CRYPTO_get_locking_callback());
    
    CRYPTO_set_locking_callback(NULL);
    EXPECT_TRUE(!CRYPTO_get_locking_callback());
}

#endif

TEST_F(MediaConnectionTest, TestMediaConnection4T)
{
    IWmeMediaConnection4T* pMediaConn4T = NULL;
    ASSERT_TRUE(CreateMediaConnection4T(1, &pMediaConn4T) == WME_S_OK);
    pMediaConn4T->SetSink(this);
    IWmeMediaConnection4T* pMediaConn4T_test = NULL;
    ASSERT_TRUE(CreateMediaConnection4T(1, &pMediaConn4T_test) == WME_S_OK);
    
    ASSERT_TRUE(pMediaConn4T != NULL);
    pMediaConn4T->AddMedia(WmeSessionType_Video, WmeDirection_SendRecv, 0, "");
    pMediaConn4T->AddMedia(WmeSessionType_Audio, WmeDirection_SendRecv, 1, "");
    pMediaConn4T->AddMedia(WmeSessionType_ScreenShare, WmeDirection_SendRecv, 2, "");
    
    class CWmeMediaTransport: public IWmeMediaTransport
    {
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
    
    CWmeMediaTransport transport;
    ASSERT_TRUE(pMediaConn4T->SetTransport(0, &transport) == WME_S_OK);
    ASSERT_EQ(m_status, WmeSession_Connected);
    ASSERT_TRUE(pMediaConn4T->SetTransport(0, NULL) == WME_S_OK);
    ASSERT_EQ(m_status, WmeSession_Disconnected);
    ASSERT_TRUE(pMediaConn4T->SetTransport(0, &transport) == WME_S_OK);
    ASSERT_EQ(m_status, WmeSession_Reconnected);
    CWmeMediaTransport transport_new;
    ASSERT_TRUE(pMediaConn4T->SetTransport(0, &transport_new, true) == WME_S_OK);
    ASSERT_EQ(m_status, WmeSession_TransportSwitched);
    
    pMediaConn4T->SetSink(NULL);
    pMediaConn4T->Stop();
    pMediaConn4T->Release();
    pMediaConn4T_test->SetSink(NULL);
    pMediaConn4T_test->Stop();
    pMediaConn4T_test->Release();
}

#ifdef AT_WIN_DESKTOP
TEST_F(MediaConnectionTest, TestSharedCamera_1)
{
    IWmeMediaConnection4T* pMediaConn4T_1 = NULL, *pMediaConn4T_2 = NULL;
    ASSERT_TRUE(CreateMediaConnection4T(1, &pMediaConn4T_1) == WME_S_OK);
    ASSERT_TRUE(CreateMediaConnection4T(1, &pMediaConn4T_2) == WME_S_OK);
    pMediaConn4T_1->SetSink(this);
    pMediaConn4T_2->SetSink(this);

    pMediaConn4T_1->AddMedia(WmeSessionType_Video, WmeDirection_Send, 1, NULL);
    pMediaConn4T_2->AddMedia(WmeSessionType_Video, WmeDirection_Send, 2, NULL);
    
    SleepMsWithLoop(200);

    IWmeMediaTrackBase *pTrack1 = GetTrack(1, WmeDirection_Send, WmeSessionType_Video);
    IWmeMediaTrackBase *pTrack2 = GetTrack(2, WmeDirection_Send, WmeSessionType_Video);

    pTrack1->StartPreview();
    pTrack2->Start(false);

    //Wait until started callback

    pTrack1->Stop();
    pTrack2->Stop();

    pMediaConn4T_1->SetSink(NULL);
    pMediaConn4T_2->SetSink(NULL);
    pMediaConn4T_1->Stop();
    pMediaConn4T_2->Stop();
}
#endif

TEST_F(MediaConnectionTest, TestCreateOfferForTrain)
{
    IWmeMediaConnection4T* pMediaConn4T = NULL;
    ASSERT_TRUE(CreateMediaConnection4T(1, &pMediaConn4T) == WME_S_OK);
    pMediaConn4T->SetSink(this);

    pMediaConn4T->AddMedia(WmeSessionType_Audio, WmeDirection_SendRecv, 1, NULL);
    pMediaConn4T->CreateOffer();
    pMediaConn4T->SetReceivedSDP(WmeSdp_Anwser, "v=0\no=wme-drone-3.3.0 0 1 IN IP4 127.0.0.1\ns=-\nt=0 0\nm=audio 9000 RTP/AVP 0 8\nc=IN IP4 127.0.0.1\nb=TIAS:64000\na=content:main\na=sendrecv\na=rtpmap:8 PCMA/8000\na=rtcp-mux");
#ifdef AT_WIN_DESKTOP
    SleepMsWithLoop(200);

    ASSERT_TRUE(GetTrack(1, WmeDirection_Recv, WmeSessionType_Audio) != NULL);
    ASSERT_TRUE(GetTrack(1, WmeDirection_Send, WmeSessionType_Audio) != NULL);
#endif
    pMediaConn4T->SetSink(NULL);
    pMediaConn4T->Stop();
}

TEST_F(MediaConnectionTest, TestCreateOfferForTrainVideo)
{
    IWmeMediaConnection4T* pMediaConn4T = NULL;
    ASSERT_TRUE(CreateMediaConnection4T(1, &pMediaConn4T) == WME_S_OK);
    pMediaConn4T->SetSink(this);

    pMediaConn4T->AddMedia(WmeSessionType_Video, WmeDirection_SendRecv, 1, NULL);
    pMediaConn4T->GetGlobalConfig()->EnableMultiStream(true);
    pMediaConn4T->CreateOffer();
    pMediaConn4T->SetReceivedSDP(WmeSdp_Anwser, "v=0\r\no=wcs 3519517365 0 IN IP4 10.224.218.46\r\ns=wcssession\r\nc=IN IP4 10.224.218.46\r\nt=1421807990 1421836790\r\nm=video 9000 RTP/AVP 98\r\na=rtpmap:98 H264-SVC/90000\r\na=fmtp:98 profile-level-id=42e020;max-mbps=1112500;max-fs=3600;packetization-mode=1;uc-mode=1\r\na=extmap:1/sendrecv http://protocols.cisco.com/virtualid\r\na=extmap:2/sendrecv http://protocols.cisco.com/framemarking\r\na=extmap:3/sendrecv urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:4/sendrecv http://protocols.cisco.com/timestamp#100us\r\na=content:main\r\na=sendrecv\r\na=sprop-source:1 count=11;policies=as:10,rs:11\r\na=sprop-simul:1 98 *\r\na=rtcp-fb:* ccm cisco-scr\r\na=ice-ufrag:2418016412#4286585487\r\na=ice-pwd:DjQNP1IhOf8cjVp8fBpuq1/C");
#ifdef AT_WIN_DESKTOP
    SleepMsWithLoop(200);
    
    ASSERT_TRUE(GetTrack(1, WmeDirection_Recv, WmeSessionType_Video) != NULL);
    ASSERT_TRUE(GetTrack(1, WmeDirection_Send, WmeSessionType_Video) != NULL);
#endif
    int vid = -1;
    ASSERT_TRUE(WME_SUCCEEDED(pMediaConn4T->Subscribe(1, WmePolicy_ReceiverSelected, WmeVideoQuality_SD, vid, 254, 0, false, 1234)));
    ASSERT_TRUE(WME_SUCCEEDED(pMediaConn4T->Unsubscribe(1, vid)));
    
    pMediaConn4T->SetSink(NULL);
    pMediaConn4T->Stop();
}
/*
TEST(InitMediaEngine, TestMultipleInitUninit)
{
    class CNullEvent : public ICmEvent {
    public:
        virtual CmResult OnEventFire() {
            CM_INFO_TRACE_THIS("TestMultipleInitUninit");
            ACmThread* pThread = GetThread(TT_MAIN);
            UninitMediaEngine();
            InitMediaEngine(false);
            pThread = GetThread(TT_MAIN);
            return 0;
        }
    };
    
    InitMediaEngine(false);
    ACmThread *pMain = CCmThreadManager::Instance()->GetThread(TT_MAIN);
    CNullEvent *pNullEvent = new CNullEvent;
    pMain->GetEventQueue()->PostEvent(pNullEvent);
    
    SleepMsWithLoop(10);
    UninitMediaEngine();
}
*/

TEST_F(MediaConnectionTest, CSIClash)
{

    uint32_t csiOthers[5] = {0};
    uint32_t csiUpdated[5] = {0};
    std::string sUniqueName("None");
    WMERESULT hr = m_pMediaConn->AddObservedCSIs(csiOthers, 5, sUniqueName.c_str(), sUniqueName.length());
    EXPECT_TRUE(WME_SUCCEEDED(hr));
    for(int i = 0; i < sizeof(csiOthers) / sizeof(uint32_t); i++) {
        csiOthers[i] = 1;
    }
    hr = m_pMediaConn->AddObservedCSIs(csiOthers, 5, sUniqueName.c_str(), sUniqueName.length());
    EXPECT_TRUE(WME_SUCCEEDED(hr));
    
    m_pMediaConn->GetGlobalConfig()->EnableMultiStream(true);
    m_pMediaConn->CreateOffer();
    SleepMsWithLoop(100);
    
    const std::pair<int, WmeSessionType> mediaMaps[3] = {{audio_mid, WmeSessionType_Audio},
                                                         {video_mid, WmeSessionType_Video},
                                                         {sharing_mid, WmeSessionType_ScreenShare}};
    uint32_t csi = 0;
    unsigned int csiCount = 1;
    for(int i = 0; i < 3; i++) {
        IWmeMediaTrackBase *pTrack = GetTrack(mediaMaps[i].first, WmeDirection_Send, mediaMaps[i].second);
        pTrack->GetCSI(&csi, csiCount);
        csiOthers[i] = csi;
    }
    hr = m_pMediaConn->AddObservedCSIs(csiOthers, 3, sUniqueName.c_str(), sUniqueName.length());
    EXPECT_TRUE(hr == WME_S_FALSE);
    if(hr == WME_S_FALSE) {
        m_pMediaConn->CreateOffer();
        SleepMsWithLoop(10);
    }
    
    for(int i = 0; i < 3; i++) {
        IWmeMediaTrackBase *pTrack = GetTrack(mediaMaps[i].first, WmeDirection_Send, mediaMaps[i].second);
        pTrack->GetCSI(&csi, csiCount);
        csiUpdated[i] = csi;
        EXPECT_NE(csi, csiOthers[i]);
    }
    EXPECT_TRUE(CCsiDB::Instance().IsSameScene(csiUpdated[0], csiUpdated[1]));
    
    int vid = 0;
    m_pMediaConn->SetReceivedSDP(WmeSdp_Anwser, "v=0\r\no=wcs 3519517365 0 IN IP4 10.224.218.46\r\ns=wcssession\r\nc=IN IP4 10.224.218.46\r\nt=1421807990 1421836790\r\nm=video 9000 RTP/AVP 98\r\na=rtpmap:98 H264-SVC/90000\r\na=fmtp:98 profile-level-id=42e020;max-mbps=1112500;max-fs=3600;packetization-mode=1;uc-mode=1\r\na=extmap:1/sendrecv http://protocols.cisco.com/virtualid\r\na=extmap:2/sendrecv http://protocols.cisco.com/framemarking\r\na=extmap:3/sendrecv urn:ietf:params:rtp-hdrext:toffset\r\na=extmap:4/sendrecv http://protocols.cisco.com/timestamp#100us\r\na=content:main\r\na=sendrecv\r\na=sprop-source:1 count=11;policies=as:10,rs:11\r\na=sprop-simul:1 98 *\r\na=rtcp-fb:* ccm cisco-scr\r\na=ice-ufrag:2418016412#4286585487\r\na=ice-pwd:DjQNP1IhOf8cjVp8fBpuq1/C");
    
    m_pMediaConn->Subscribe(video_mid, WmePolicy_ReceiverSelected, WmeVideoQuality_SD, vid, 254, 1, true, 54321);
    SleepMsWithLoop(10);
    
    IWmeMediaTrackBase *pTrack = GetTrack(video_mid, WmeDirection_Recv, WmeSessionType_Video, vid);
    pTrack->Start(false);
    std::string sSubscribed = "subscribed";
    hr = m_pMediaConn->AddObservedCSI(54321, sSubscribed.c_str(), sSubscribed.length(), WmeSessionType_Video);
    EXPECT_TRUE(WME_SUCCEEDED(hr));
    SleepMsWithLoop(100);
    hr = m_pMediaConn->AddObservedCSI(12345, sSubscribed.c_str(), sSubscribed.length(), WmeSessionType_Video);
    EXPECT_TRUE(WME_SUCCEEDED(hr));
    SleepMsWithLoop(100);
    
    CMediaTrack *pTrackReal = static_cast<CMediaTrack*>(pTrack);
    if(pTrackReal) {
        WmeSimulcastRequest* pRequest = pTrackReal->GetRequest(true);
        EXPECT_TRUE(pRequest->uRequestInfo.stSelectedSourceInfo.uCaptureSourceId == 12345);
    }

}

TEST_F(MediaConnectionTest, TestMediaCSI)
{
    CMediaCSI mediaCSI(1);
    uint32_t csi[2] = {1234, 2234};
    mediaCSI.SetCSI(csi, 2);
    uint32_t csiV[2] = {0};
    unsigned count = 2;
    mediaCSI.GetCSI(true, csiV, count);
    ASSERT_EQ(csi[0], csiV[0]);
    ASSERT_EQ(csi[1], csiV[1]);
    uint32_t csiOld[2] = {0};
    unsigned countOld = 2;
    uint32_t csiNew[2] = {0};
    unsigned countNew = 2;
    bool change = mediaCSI.CheckCSIChanged(csiOld, countOld, csiNew, countNew);
    ASSERT_TRUE(change);
    ASSERT_EQ(csi[0], csiNew[0]);
    ASSERT_EQ(csi[1], csiNew[1]);
    mediaCSI.SetCSI(csi, 2);
    change = mediaCSI.CheckCSIChanged(csiOld, countOld, csiNew, countNew);
    ASSERT_FALSE(change);
}

TEST_F(MediaConnectionTest, TestGetVideoCapability)
{
    CMediaConnection* pConn = (CMediaConnection*)m_pMediaConn;
    vector<wme::WmeVideoMediaCapability> caps;
    json::Value encode;
    WMERESULT cret = pConn->GetVideoCapability(encode, WmeCodecType_AVC, WmeVideoQuality_HD_720P, caps, true, 4);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    ASSERT_EQ(caps.size(), 4);
    
    cret = pConn->GetVideoCapability(encode, WmeCodecType_AVC, WmeVideoQuality_SD, caps, true, 4);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    ASSERT_EQ(caps.size(), 3);
    
    cret = pConn->GetVideoCapability(encode, WmeCodecType_AVC, WmeVideoQuality_SD, caps, false, 4);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    ASSERT_EQ(caps.size(), 1);
    
    cret = pConn->GetVideoCapability(encode, WmeCodecType_AVC, WmeVideoQuality_HD_720P, caps, true, 2);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    ASSERT_EQ(caps.size(), 2);
    ASSERT_EQ(caps[0].max_mbps, 7200);
    ASSERT_EQ(caps[1].max_mbps, 108000);
    
    cret = pConn->GetVideoCapability(encode, WmeCodecType_AVC, WmeVideoQuality_SD, caps, true, 2);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    ASSERT_EQ(caps.size(), 2);
    ASSERT_EQ(caps[0].max_mbps, 7200);
    ASSERT_EQ(caps[1].max_mbps, 27600);
    
    cret = pConn->GetVideoCapability(encode, WmeCodecType_AVC, WmeVideoQuality_HD_720P, caps, true, 3);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    ASSERT_EQ(caps.size(), 3);
    ASSERT_EQ(caps[0].max_mbps, 7200);
    ASSERT_EQ(caps[1].max_mbps, 27600);
    ASSERT_EQ(caps[2].max_mbps, 108000);
    
    string strEncode = "[{\"codec\": \"H264\", \"uProfileLevelID\": \"42000D\", \"max-mbps\": 40500, \"max-fs\": 1620, \"max-fps\": 2400, \"max-br\": 10000}]";
    encode = json::Deserialize(strEncode);
    cret = pConn->GetVideoCapability(encode, WmeCodecType_AVC, WmeVideoQuality_HD_720P, caps, true, 4);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    ASSERT_EQ(caps.size(), 3);
    ASSERT_EQ(caps[0].frame_layer[0], 2400);
    ASSERT_EQ(caps[1].frame_layer[0], 2400);
    ASSERT_EQ(caps[2].frame_layer[0], 2400);
    ASSERT_EQ(caps[2].max_mbps, 40500);
    ASSERT_EQ(caps[2].max_fs, 1620);
    unsigned level = caps[2].profile_level_id & 0x000000FF;
    ASSERT_EQ(level, 0x0D);
    
    cret = pConn->GetVideoCapability(encode, WmeCodecType_AVC, WmeVideoQuality_SD, caps, false, 4);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    ASSERT_EQ(caps.size(), 1);
    ASSERT_EQ(caps[0].frame_layer[0], 2400);
    ASSERT_EQ(caps[0].max_mbps, 27600);
    ASSERT_EQ(caps[0].max_fs, 1620);
    level = caps[0].profile_level_id & 0x000000FF;
    ASSERT_EQ(level, 0x0D);
    
    strEncode = "[{\"codec\": \"H264\", \"uProfileLevelID\": \"42000D\", \"max-mbps\": 108001, \"max-fs\": 50, \"max-fps\": 4000, \"max-br\": 10000}]";
    encode = json::Deserialize(strEncode);
    cret = pConn->GetVideoCapability(encode, WmeCodecType_AVC, WmeVideoQuality_HD_720P, caps, true, 4);
    ASSERT_TRUE(WME_SUCCEEDED(cret));
    ASSERT_EQ(caps.size(), 1);
    ASSERT_EQ(caps[0].frame_layer[0], 4000);
    ASSERT_EQ(caps[0].max_mbps, 108000);
    ASSERT_EQ(caps[0].max_fs, 50);
    level = caps[0].profile_level_id & 0x000000FF;
    ASSERT_EQ(level, 0x0D);
}
