//
//  mediaconnectioninfo_ut.cpp
//  mediasessionUT_ios
//
//  Created by LiSiyuan on 14-11-4.
//  Copyright (c) 2014年 cisco. All rights reserved.
//

#include "gtest/gtest.h"
#include "stringhelper.h"
#include "MediaConnectionInfo.h"

using namespace wme;

class MediaConnectionInfoTest : public ::testing::Test
{
public:
    MediaConnectionInfoTest() : m_info(1234) {}
    
protected:
    virtual void SetUp() {
    }
    
    virtual void TearDown() {
    }
    
    CMediaConnectionInfo m_info;
};

TEST_F(MediaConnectionInfoTest, TestOnSubscribe)
{
    // audio
    m_info.Init(NULL, WmeSessionType_Audio, 0);
    WmeSimulcastRequest stRequest = {WmeSCRRequestPolicyType_NonePolicy, {0, 0, 64000}, WmeSCRRequestCodecType_Common, {0x80}};
    WmeVideoEncodeCapability encodeCap = {0};
    uint32_t layer = 0;
    uint8_t ret = m_info._onSubscribe(stRequest, encodeCap, layer);
    ASSERT_EQ(ret, 0);
    
    // video/share
    m_info.Init(NULL, WmeSessionType_Video, 0);
    vector<WmeVideoMediaCapability> &caps = m_info.getCaps(CT_H264, H264_PACK_MODE_1);
    WmeVideoMediaCapability cap;
    cap.eMediaType = WmeMediaTypeVideo;
    cap.eCodecType = WmeCodecType_AVC;
    
    cap.width = 160;
    cap.height = 90;
    cap.profile_level_id = 0x42e00b;
    cap.max_mbps = 3000;
    cap.max_fs = 396;
    cap.frame_layer_number = 1;
    cap.frame_layer[0] = 3000;
    cap.min_bitrate_bps = 100000;
    caps.push_back(cap);
    
    cap.width = 320;
    cap.height = 180;
    cap.profile_level_id = 0x42e00c;
    cap.max_mbps = 7200;
    cap.max_fs = 396;
    cap.frame_layer_number = 1;
    cap.frame_layer[0] = 3000;
    cap.min_bitrate_bps = 200000;
    caps.push_back(cap);
    
    cap.width = 640;
    cap.height = 360;
    cap.profile_level_id = 0x42e014;
    cap.max_mbps = 27600;
    cap.max_fs = 920;
    cap.frame_layer_number = 1;
    cap.frame_layer[0] = 3000;
    cap.min_bitrate_bps = 400000;
    caps.push_back(cap);
    
    cap.width = 1280;
    cap.height = 720;
    cap.profile_level_id = 0x42e015;
    cap.max_mbps = 108000;
    cap.max_fs = 3600;
    cap.frame_layer_number = 1;
    cap.frame_layer[0] = 3000;
    cap.min_bitrate_bps = 800000;
    caps.push_back(cap);
    
    stRequest.eRequestCodecType = WmeSCRRequestCodecType_H264;
    stRequest.uCodecCapability.stH264CodecCap = {0x80, false, false, 2000, 400, 3000, 1, {3000}};
    ret = m_info._onSubscribe(stRequest, encodeCap, layer);
    ASSERT_EQ(ret, 0xFF);
    ASSERT_EQ(layer, 0);
    ASSERT_EQ(encodeCap.uMaxBitRate, 64000);
    
    stRequest.uRequestInfo.stNonePolicyInfo = {0, 0, 100000};
    stRequest.uCodecCapability.stH264CodecCap = {0x80, false, false, 4000, 400, 3000, 1, {3000}};
    ret = m_info._onSubscribe(stRequest, encodeCap, layer);
    ASSERT_EQ(ret, 0xFF);
    ASSERT_EQ(layer, 0);
    ASSERT_EQ(encodeCap.uMaxBitRate, 100000);
    
    stRequest.uRequestInfo.stNonePolicyInfo = {0, 0, 300000};
    stRequest.uCodecCapability.stH264CodecCap = {0x80, false, false, 10000, 400, 3000, 1, {3000}};
    ret = m_info._onSubscribe(stRequest, encodeCap, layer);
    ASSERT_EQ(ret, 0xFF);
    ASSERT_EQ(layer, 1);
    ASSERT_EQ(encodeCap.uMaxBitRate, 300000);
    
    stRequest.uRequestInfo.stNonePolicyInfo = {0, 0, 600000};
    stRequest.uCodecCapability.stH264CodecCap = {0x80, false, false, 28000, 920, 3000, 1, {3000}};
    ret = m_info._onSubscribe(stRequest, encodeCap, layer);
    ASSERT_EQ(ret, 0xFF);
    ASSERT_EQ(layer, 2);
    ASSERT_EQ(encodeCap.uMaxBitRate, 600000);
    
    stRequest.uRequestInfo.stNonePolicyInfo = {0, 0, 100000};
    stRequest.uCodecCapability.stH264CodecCap = {0x80, false, false, 28000, 920, 3000, 1, {3000}};
    ret = m_info._onSubscribe(stRequest, encodeCap, layer);
    ASSERT_EQ(ret, 0xFF);
    ASSERT_EQ(layer, 0);
    ASSERT_EQ(encodeCap.uMaxBitRate, 100000);
    
    stRequest.uRequestInfo.stNonePolicyInfo = {0, 0, 2000000};
    stRequest.uCodecCapability.stH264CodecCap = {0x80, false, false, 110000, 4000, 3000, 1, {3000}};
    ret = m_info._onSubscribe(stRequest, encodeCap, layer);
    ASSERT_EQ(ret, 0xFF);
    ASSERT_EQ(layer, 3);
    ASSERT_EQ(encodeCap.uMaxBitRate, 2000000);
}

TEST_F(MediaConnectionInfoTest, TranslateToVideoMetrics)
{
  m_info.Init(NULL, WmeSessionType_Video, 0);
    unsigned int uOutOfSyncRatio = 0;
    unsigned int uEcAreaRatio = 0;
    unsigned int uCurrentResolultionFs = 0;
    bool bIsOutOfSyncSecond = false;
  
    WmeVideoStatistics remoteVideoStat;
    remoteVideoStat.uWidth = 1280;
    remoteVideoStat.uHeight = 720;
    remoteVideoStat.uInSyncMs = 1000;
    remoteVideoStat.uOutOfSyncMs=0;
    remoteVideoStat.uAvgEcRatio=0;
    remoteVideoStat.bIsCurrentOutOfSync = false;

    int ret = m_info.TranslateToVideoMetrics(remoteVideoStat,
                          uOutOfSyncRatio, uEcAreaRatio,
                          uCurrentResolultionFs,
                            bIsOutOfSyncSecond);
    ASSERT_EQ(ret, 0);
    ASSERT_EQ(uOutOfSyncRatio, 0);
    ASSERT_EQ(uEcAreaRatio, 0);
    ASSERT_EQ(uCurrentResolultionFs, 3600);
    ASSERT_EQ(bIsOutOfSyncSecond, false);
  
}

