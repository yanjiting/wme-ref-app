#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "CmThreadManager.h"
#include "networkindicator.h"
#include "rtputils.h"

using namespace testing;
using namespace wrtp;

TEST(CLossPatternTest, TestDuration)
{
    CLossPattern p;
    uint32_t now = 5;
    p.SetStart(now);
    now += 5;
    p.Update(now, 10, 0.1f);

    ASSERT_EQ(p.GetDuration(), 5);
    ASSERT_EQ(p.GetDuration(15), 10);

    now += 5;
    p.Update(now, 10, 0.05f);
    now += 5;
    ASSERT_EQ(p.GetDurationSinceLastLoss(now), 5);
}


TEST(CLossPatternDectectorTest, TestSpikeLoss)
{
    CLossPatternDectector detector;
    uint32_t now = 0;
    detector.Update(now, 0, 0.01f);

    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_SPIKE);
    ASSERT_NEAR(detector.GetCurrentLossPattern().GetAvgLossratio(), 0.01, 0.00001);

    now += detector.GetMininumLossInterval();
    detector.Update(now, 0, 0.005f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_SPIKE);

    now += detector.GetMininumLossInterval() - 5;
    detector.Update(now, 0, 0.1f);
    ASSERT_NE(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_SPIKE);
}


TEST(CLossPatternDectectorTest, TestInitState)
{
    CLossPatternDectector detector;
    uint32_t now = 0;
    detector.Update(now, 0, 0.01f);

    now += detector.GetMininumLossInterval();
    detector.Update(now, 0, 0.0f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_INIT);

    now += 1;
    detector.Update(now, 0, 0.1f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_SPIKE);
}


TEST(CLossPatternDectectorTest, TestContinuesState)
{
    CLossPatternDectector detector;
    uint32_t now = 0;
    detector.Update(now, 0, 0.01f);

    now += detector.GetMininumLossInterval()/2;
    detector.Update(now, 0, 0.1f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_CONTINUOUS);

    now += detector.GetMininumLossInterval();
    detector.Update(now, 0, 0.1f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_SPIKE);
}


TEST(CLossPatternDectectorTest, TestCongestionState)
{
    CLossPatternDectector detector;
    uint32_t now = 0;
    detector.Update(now, 0, 0.01f);

    now += 500;
    detector.Update(now, 300, 0.1f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_CONTINUOUS);

    now += 500;
    detector.Update(now, 500, 0.1f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_CONGESTION);

    now += 500;
    detector.Update(now, 0, 0.1f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_CONGESTION);

    now += 500;
    detector.Update(now, 0, 0.0f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_INIT);
}


TEST(CLossPatternDectectorTest, TestFECState)
{
    CLossPatternDectector detector;
    uint32_t now = 0;
    detector.Update(now, 0, 0.01f);

    now += 500;
    detector.Update(now, 50, 0.05f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_CONTINUOUS);

    now += 1500;
    detector.Update(now, 100, 0.1f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_CONTINUOUS);

    now += 2000;
    detector.Update(now, 10, 0.1f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_CONTINUOUS);

    now += 1500;
    detector.Update(now, 50, 0.0f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_FEC);

    now += 500;
    detector.Update(now, 50, 0.0f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_FEC);

    now += 500;
    detector.Update(now, 500, 0.1f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_FEC);

    now += 500;
    detector.Update(now, 500, 0.1f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_CONGESTION);

    now += 500;
    detector.Update(now, 10, 0.1f);

    now += 500;
    detector.Update(now, 10, 0.1f);

    now += 500;
    detector.Update(now, 10, 0.1f);

    now += 500;
    detector.Update(now, 10, 0.1f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_SPIKE);

    now += 5000;
    detector.Update(now, 10, 0.0f);
    ASSERT_EQ(detector.GetCurrentLossPattern().GetState(), CLossPattern::ST_INIT);

}


class CRTPSessionClientMock: public IRTPSessionClient
{
public:
    //IRTPSessionClient
    MOCK_METHOD1(SetMediaPackageAllocator, int32_t(IRTPMediaPackageAllocator *));
    MOCK_METHOD1(SetMediaTransport, int32_t(IRTPMediaTransport *));
    MOCK_METHOD1(ReceiveRTCPPacket, int32_t(IRTPMediaPackage *));
    MOCK_METHOD1(SetSendingFilterSink, void(ISendingFilterSink *));
    MOCK_METHOD3(RegisterRTPExtensionProcessFunc, int32_t(const char *, ExtensionBuildFunc, ExtensionParseFunc));
    MOCK_METHOD4(RegisterPayloadType, int32_t(CodecType, uint8_t, uint32_t, eStreamDirection));
    MOCK_METHOD2(RegisterFECInfo, int32_t(MariFECInfo &fecInfo,eStreamDirection esd));
    MOCK_METHOD1(SetMaxFecOverhead, int32_t(uint32_t));
    MOCK_METHOD1(SetFecProbing, int32_t(bool));
    MOCK_METHOD1(SetDynamicFecScheme, int32_t(DynamicFECLevelScheme));
    MOCK_METHOD3(SetSrtpFecOrder, int32_t(const SRTPFecOrder, eStreamDirection, bool));
    MOCK_METHOD1(SetMaxPacketSize, int32_t(uint16_t));
    MOCK_METHOD1(GetMaxPacketSize, int32_t(uint16_t &));
    MOCK_METHOD1(GetInitialBandwidth, int32_t(uint32_t &));
    MOCK_METHOD1(SetInitialBandwidth, int32_t(uint32_t));
    MOCK_METHOD1(SetMaxBandwidth, int32_t(uint32_t));
    MOCK_METHOD1(GetMaxBandwidth, int32_t(uint32_t &));
    MOCK_METHOD1(SetPacketizationMode, int32_t(RTPPacketizationMode));
    MOCK_METHOD1(EnableSendingAggregationPacket, int32_t(bool));
    MOCK_METHOD2(CreateRTPChannel, IRTPChannel*(uint32_t, WRTPChannelParams &));
    MOCK_METHOD0(Close, void());

    MOCK_METHOD1(RegisterSessionConfigObserver, int32_t(IRTPSessionConfigObserver *));
    MOCK_METHOD1(UnregisterSessionConfigObserver, int32_t(IRTPSessionConfigObserver *));
    MOCK_METHOD1(EnableCiscoSCR, int32_t(bool));
    MOCK_METHOD1(GetRTPPacketQuerier, IRTPPacketQuerier*(eStreamDirection));
    MOCK_METHOD2(QueryRemoteCName, int32_t(uint32_t, CNameBlock &));
    MOCK_METHOD1(SetLocalCName, void(const char *));
    MOCK_METHOD1(GetLocalCName, int32_t(CNameBlock &));

    //IRTPReferenceControl
    MOCK_METHOD0(IncreaseReference, uint32_t());
    MOCK_METHOD0(DecreaseReference, uint32_t());

    //IRTPExtensionObserver
    MOCK_METHOD3(UpdateRTPExtension, int32_t(const char *, uint8_t, eStreamDirection));
    MOCK_METHOD3(QueryRTPExtensionID, int32_t(const char *, eStreamDirection, uint8_t &));

    //IRTPPayloadSpecificIndication
    MOCK_METHOD1(SetPayloadSpecificIndicationSink, void(IRTPPayloadSpecificIndicationSink *));
    MOCK_METHOD2(IndicatePictureLoss, int32_t(uint32_t, uint32_t));

    //IQoSControl
    MOCK_METHOD1(EnableQoS, int32_t(const QoSParameter &));
    MOCK_METHOD0(DisableQoS, int32_t());
    MOCK_METHOD0(PauseQoS, int32_t());
    MOCK_METHOD0(ResumeQoS, int32_t());

    //IIOStatistic
    MOCK_METHOD4(GetIOStatistic, void(uint32_t &, uint32_t &, uint32_t &, uint32_t &));

    //IRTPSessionOption
    MOCK_METHOD4(SetOption, int32_t(uint32_t, WRTPOption, const void *, uint32_t));
    MOCK_METHOD4(GetOption, int32_t(uint32_t, WRTPOption, void *, uint32_t *));

    //IRTPSecurityConfigure
    MOCK_METHOD1(SetOutboundSecurityConfiguration, int32_t(const SecurityConfiguration &));
    MOCK_METHOD1(SetInboundSecurityConfiguration, int32_t(const SecurityConfiguration &));

    MOCK_METHOD1(SetMediaContentCipher, int32_t(IRTPMediaContentCipher *cipher));
    MOCK_METHOD2(UpdateStreamBandwidth, int32_t(const StreamIdInfo &streamId, uint32_t bandwidthInBytePerSec));

    //ISimulcastRequest
    MOCK_METHOD4(RegisterPolicyId, int32_t(uint8_t, SCRRequestPolicyType, uint16_t, eStreamDirection));
    MOCK_METHOD2(SetMaxSubsessionChannels, int32_t(uint8_t, eStreamDirection));
    MOCK_METHOD2(Subscribe, int32_t(const SelectedSourceRequestTrait &, const CodecCapability &));
    MOCK_METHOD2(Subscribe, int32_t(const ActiveSpeakerRequestTrait &, const CodecCapability &));
    MOCK_METHOD2(Subscribe, int32_t(const NonePolicyRequestTrait &, const CodecCapability &));
    MOCK_METHOD2(Subscribe, int32_t(const SubsessionChannelRequest *, uint8_t));
    MOCK_METHOD1(Unsubscribe, int32_t(uint8_t));
    MOCK_METHOD0(UnsubscribeAll, int32_t());
    MOCK_METHOD0(SendRequest, int32_t());
    MOCK_METHOD2(Announce, int32_t(const SubsessionChannelAnnounce &, bool));

    MOCK_METHOD1(SetSimulcastRequestCallback, void(IRTPSimulcastRequestCallback *));
    MOCK_METHOD1(SetSimulcastResponseCallback, void(IRTPSimulcastResponseCallback *));

    MOCK_METHOD1(GetSecurityStats,int32_t(SrtpStats &));
    MOCK_METHOD1(GetSessionStats, int32_t(SessStats &));
    MOCK_METHOD2(GetLocalStats, int32_t(StreamOutStats *, uint32_t &));
    MOCK_METHOD2(GetRemoteStats, int32_t(StreamInStats *, uint32_t &));
    MOCK_METHOD0(Failover,int32_t());
    MOCK_METHOD1(GetLossBurstStats, int32_t(LossBurstLenStat &));
    MOCK_METHOD1(GetOOOGapStats, int32_t(OutOfOrderStat &));
    MOCK_METHOD1(GetBwDownEventStats, int32_t(BwDownEventStat &));
    MOCK_METHOD1(GetSevereCongestionStats, int32_t(uint32_t &));
    MOCK_METHOD1(SetMaxDelay,int32_t(uint32_t maxDelay));

    MOCK_METHOD1(SetFeatureToggles, void(const char *));
    MOCK_METHOD1(GetFeatureToggles, void(json::Object *root));

    MOCK_METHOD2(UpdateCSI, int32_t(uint32_t, uint32_t));
    MOCK_METHOD2(GetCSI, int32_t(uint32_t, uint32_t &));

    MOCK_METHOD1(EnableRecordLossData, void(bool));
    MOCK_METHOD0(IsRecordLossDataEnabled, bool());
    MOCK_METHOD0(GetLossRawDataRecordedBytes, uint32_t());
};


TEST(CNetworkIndicatorImpTest, TestRegisterUnRegister)
{
    EXPECT_TRUE(CNetworkIndicatorImp::getInstance() != nullptr);

    CRTPSessionClientMock rtpSessionClient;

    bool ret = CNetworkIndicatorImp::getInstance()->RegisterSessionClient(&rtpSessionClient);
    ASSERT_EQ(ret, true);
    ret = CNetworkIndicatorImp::getInstance()->RegisterSessionClient(&rtpSessionClient);
    ASSERT_EQ(ret, false);

    ASSERT_TRUE(CNetworkIndicatorImp::getInstance()->IsStarted());

    ret = CNetworkIndicatorImp::getInstance()->UnRegisterSessionClient(&rtpSessionClient);
    ASSERT_EQ(ret, true);
    ret = CNetworkIndicatorImp::getInstance()->UnRegisterSessionClient(&rtpSessionClient);
    ASSERT_EQ(ret, false);

    ASSERT_FALSE(CNetworkIndicatorImp::getInstance()->IsStarted());
}

TEST(CNetworkIndicatorImpTest, TestNotStart)
{
    ASSERT_FALSE(CNetworkIndicatorImp::getInstance()->IsStarted());
}

TEST(CNetworkIndicatorImpTest, TestStartStop)
{
    CRTPSessionClientMock rtpSessionClient;

    bool ret = CNetworkIndicatorImp::getInstance()->RegisterSessionClient(&rtpSessionClient);
    ASSERT_EQ(ret, true);

    CmResult result = CNetworkIndicatorImp::getInstance()->Start();
    ASSERT_EQ(result, CM_OK);

    ASSERT_TRUE(CNetworkIndicatorImp::getInstance()->IsStarted());

    result = CNetworkIndicatorImp::getInstance()->Stop();
    ASSERT_EQ(result, CM_OK);

    ret = CNetworkIndicatorImp::getInstance()->UnRegisterSessionClient(&rtpSessionClient);
    ASSERT_EQ(ret, true);
}


TEST(CNetworkIndicatorImpTest, TestUpdateNetworkIndexSmallSpikeState)
{
    CNetworkIndicatorImp::IndexPerDirection direction;
    uint32_t now = 0;
    int32_t rtt = 0;
    float loss = 0.0f;
    direction.lr_pattern_detector.Update(now, rtt, loss);
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(direction, rtt, loss);

    ASSERT_EQ(direction.network_idx, CNetworkIndicator::NETWORK_EXCELLENT);

    now += 500;
    rtt = 50;
    loss = 0.01f;
    direction.lr_pattern_detector.Update(now, rtt, loss);
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(direction, rtt, loss);
    ASSERT_EQ(direction.network_idx, CNetworkIndicator::NETWORK_VERY_GOOD);
}


TEST(CNetworkIndicatorImpTest, TestUpdateNetworkIndexLargeSpikeState)
{
    CNetworkIndicatorImp::IndexPerDirection direction;
    uint32_t now = 0;
    int32_t rtt = 0;
    float loss = 0.0f;
    direction.lr_pattern_detector.Update(now, rtt, loss);
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(direction, rtt, loss);

    ASSERT_EQ(direction.network_idx, CNetworkIndicator::NETWORK_EXCELLENT);

    now += 500;
    rtt = 50;
    loss = 0.8f;
    direction.lr_pattern_detector.Update(now, rtt, loss);
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(direction, rtt, loss);
    ASSERT_LE(direction.network_idx, CNetworkIndicator::NETWORK_NOT_ACCEPTABLE);
    ASSERT_GT(direction.network_idx, CNetworkIndicator::NETWORK_SEVERE);
}


TEST(CNetworkIndicatorImpTest, TestUpdateNetworkIndexContinuesState)
{
    CNetworkIndicatorImp::IndexPerDirection direction;
    uint32_t now = 0;
    int32_t rtt = 0;
    float loss = 0.0f;
    direction.lr_pattern_detector.Update(now, rtt, loss);
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(direction, rtt, loss);

    ASSERT_EQ(direction.network_idx, CNetworkIndicator::NETWORK_EXCELLENT);

    now += 500;
    rtt = 50;
    loss = 0.01f;
    direction.lr_pattern_detector.Update(now, rtt, loss);
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(direction, rtt, loss);
    ASSERT_EQ(direction.network_idx, CNetworkIndicator::NETWORK_VERY_GOOD);

    now += 500;
    rtt = 50;
    loss = 0.16f;
    direction.lr_pattern_detector.Update(now, rtt, loss);
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(direction, rtt, loss);
    ASSERT_GE(direction.network_idx, CNetworkIndicator::NETWORK_NOT_ACCEPTABLE);
    ASSERT_LE(direction.network_idx, CNetworkIndicator::NETWORK_GOOD);

    now += 500;
    rtt = 50;
    loss = 0.30f;
    direction.lr_pattern_detector.Update(now, rtt, loss);
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(direction, rtt, loss);
    ASSERT_GE(direction.network_idx, CNetworkIndicator::NETWORK_SEVERE);
    ASSERT_LE(direction.network_idx, CNetworkIndicator::NETWORK_NOT_ACCEPTABLE);

    now += 500;
    rtt = 50;
    loss = 0.5f;
    direction.lr_pattern_detector.Update(now, rtt, loss);
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(direction, rtt, loss);
    ASSERT_GE(direction.network_idx, CNetworkIndicator::NETWORK_SEVERE);
    ASSERT_LE(direction.network_idx, CNetworkIndicator::NETWORK_NOT_ACCEPTABLE);

    now += 3500;
    rtt = 50;
    loss = 0.5f;
    direction.lr_pattern_detector.Update(now, rtt, loss);
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(direction, rtt, loss);
    ASSERT_LE(direction.network_idx, CNetworkIndicator::NETWORK_SEVERE);

}


TEST(CNetworkIndicatorImpTest, TestUpdateNetworkIndexCongestionState)
{
    CNetworkIndicatorImp::IndexPerDirection direction;
    uint32_t now = 0;
    int32_t rtt = 0;
    float loss = 0.0f;
    direction.lr_pattern_detector.Update(now, rtt, loss);
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(direction, rtt, loss);

    now += 500;
    rtt = 500;
    loss = 0.0f;
    direction.lr_pattern_detector.Update(now, rtt, loss);
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(direction, rtt, loss);

    now += 500;
    rtt = 600;
    loss = 0.1f;
    direction.lr_pattern_detector.Update(now, rtt, loss);
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(direction, rtt, loss);
    ASSERT_GE(direction.network_idx, CNetworkIndicator::NETWORK_NOT_ACCEPTABLE);
    ASSERT_LT(direction.network_idx, CNetworkIndicator::NETWORK_POOR);

    now += 500;
    rtt = 800;
    loss = 0.1f;
    direction.lr_pattern_detector.Update(now, rtt, loss);
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(direction, rtt, loss);
    ASSERT_LE(direction.network_idx, CNetworkIndicator::NETWORK_NOT_ACCEPTABLE);

}


TEST(CNetworkIndicatorImpTest, TestGetSubstractScoreBound)
{
    int32_t substractScore = CNetworkIndicatorImp::getInstance()->GetSubstractScore(0, 0.09f, 100, 0.1f);
    ASSERT_EQ(substractScore, 0);

    substractScore = CNetworkIndicatorImp::getInstance()->GetSubstractScore(0, 0.1f, 100, 0.1f);
    ASSERT_EQ(substractScore, 0);

    substractScore = CNetworkIndicatorImp::getInstance()->GetSubstractScore(100, 0.0f, 100, 0.1f);
    ASSERT_EQ(substractScore, 0);
}


TEST(CNetworkIndicatorImpTest, TestGetSubstractScoreNonLinear)
{
    int32_t substractScore_lr_8 = CNetworkIndicatorImp::getInstance()->GetSubstractScore(0, 0.8f, 100, 0.1f);
    int32_t substractScore_lr_5 = CNetworkIndicatorImp::getInstance()->GetSubstractScore(0, 0.5f, 100, 0.1f);
    int32_t substractScore_lr_2 = CNetworkIndicatorImp::getInstance()->GetSubstractScore(0, 0.2f, 100, 0.1f);
    ASSERT_LT(substractScore_lr_8 - substractScore_lr_5, substractScore_lr_5 - substractScore_lr_2);


    int32_t substractScore_rtt_400 = CNetworkIndicatorImp::getInstance()->GetSubstractScore(400, 0.0f, 100, 0.1);

    int32_t substractScore_rtt_300 = CNetworkIndicatorImp::getInstance()->GetSubstractScore(300, 0.0f, 100, 0.1);

    int32_t substractScore_rtt_200 = CNetworkIndicatorImp::getInstance()->GetSubstractScore(200, 0.0f, 100, 0.1);
    ASSERT_LT(substractScore_rtt_400 - substractScore_rtt_300, substractScore_rtt_300 - substractScore_rtt_200);
}


TEST(CNetworkIndicatorImpTest, TestGetNormailizedIndex)
{
    CNetworkIndicator::NetworkIndex idx;
    idx = CNetworkIndicatorImp::GetNormailizedIndex(CNetworkIndicator::NETWORK_EXCELLENT);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_EXCELLENT);

    idx = CNetworkIndicatorImp::GetNormailizedIndex((CNetworkIndicator::NETWORK_EXCELLENT+CNetworkIndicator::NETWORK_VERY_GOOD)/2);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_VERY_GOOD);

    idx = CNetworkIndicatorImp::GetNormailizedIndex(CNetworkIndicator::NETWORK_VERY_GOOD);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_VERY_GOOD);

    idx = CNetworkIndicatorImp::GetNormailizedIndex((CNetworkIndicator::NETWORK_VERY_GOOD+CNetworkIndicator::NETWORK_GOOD)/2);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_GOOD);

    idx = CNetworkIndicatorImp::GetNormailizedIndex(CNetworkIndicator::NETWORK_GOOD);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_GOOD);

    idx = CNetworkIndicatorImp::GetNormailizedIndex((CNetworkIndicator::NETWORK_GOOD+CNetworkIndicator::NETWORK_POOR)/2);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_POOR);

    idx = CNetworkIndicatorImp::GetNormailizedIndex(CNetworkIndicator::NETWORK_POOR);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_POOR);

    idx = CNetworkIndicatorImp::GetNormailizedIndex((CNetworkIndicator::NETWORK_POOR+CNetworkIndicator::NETWORK_NOT_ACCEPTABLE)/2);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_NOT_ACCEPTABLE);

    idx = CNetworkIndicatorImp::GetNormailizedIndex(CNetworkIndicator::NETWORK_NOT_ACCEPTABLE);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_NOT_ACCEPTABLE);

    idx = CNetworkIndicatorImp::GetNormailizedIndex((CNetworkIndicator::NETWORK_NOT_ACCEPTABLE+CNetworkIndicator::NETWORK_SEVERE)/2);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_SEVERE);

    idx = CNetworkIndicatorImp::GetNormailizedIndex(CNetworkIndicator::NETWORK_SEVERE);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_SEVERE);

    idx = CNetworkIndicatorImp::GetNormailizedIndex((CNetworkIndicator::NETWORK_SEVERE+CNetworkIndicator::NETWORK_USELESS)/2);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_USELESS);

    idx = CNetworkIndicatorImp::GetNormailizedIndex(CNetworkIndicator::NETWORK_USELESS);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_USELESS);

    idx = CNetworkIndicatorImp::GetNormailizedIndex(CNetworkIndicator::NETWORK_UNKNOWN);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_UNKNOWN);

    idx = CNetworkIndicatorImp::GetNormailizedIndex(CNetworkIndicator::NETWORK_EXCELLENT + 1);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_UNKNOWN);
}

TEST(CNetworkIndicatorImpTest, TestResetWhenNoSession)
{
    CRTPSessionClientMock rtpSessionClient1;
    CRTPSessionClientMock rtpSessionClient2;

    CNetworkIndicatorImp::getInstance()->Stop();

    bool ret = CNetworkIndicatorImp::getInstance()->RegisterSessionClient(&rtpSessionClient1);
    ASSERT_TRUE(ret);
    ret = CNetworkIndicatorImp::getInstance()->RegisterSessionClient(&rtpSessionClient2);
    ASSERT_TRUE(ret);

    SessStats stats1;
    stats1.outStats.netStats.rtt = 500;
    stats1.inStats.netStats.lossRatio = stats1.outStats.netStats.lossRatio = 0.8f;
    SessStats stats2;
    stats2.outStats.netStats.rtt = 400;
    stats2.inStats.netStats.lossRatio = stats2.outStats.netStats.lossRatio = 0.8f;
    EXPECT_CALL(rtpSessionClient1, GetSessionStats(_)).WillRepeatedly(DoAll(SetArgReferee<0>(stats1), Return(0)));
    EXPECT_CALL(rtpSessionClient2, GetSessionStats(_)).WillRepeatedly(DoAll(SetArgReferee<0>(stats2), Return(0)));

    uint32_t now = 0;

    WRTP_INFOTRACE("first update");
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(now);

    CNetworkIndicator::NetworkIndex idx = CNetworkIndicatorImp::getInstance()->GetNetworkIndex(CNetworkIndicator::DIRECTION_UPLINK);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_SEVERE);

    idx = CNetworkIndicatorImp::getInstance()->GetNetworkIndex(CNetworkIndicator::DIRECTION_DOWNLINK);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_SEVERE);


    ret = CNetworkIndicatorImp::getInstance()->UnRegisterSessionClient(&rtpSessionClient1);
    ASSERT_TRUE(ret);
    ret = CNetworkIndicatorImp::getInstance()->UnRegisterSessionClient(&rtpSessionClient2);
    ASSERT_TRUE(ret);

    idx = CNetworkIndicatorImp::getInstance()->GetNetworkIndex(CNetworkIndicator::DIRECTION_UPLINK);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_UNKNOWN);

    idx = CNetworkIndicatorImp::getInstance()->GetNetworkIndex(CNetworkIndicator::DIRECTION_DOWNLINK);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_UNKNOWN);
}


TEST(CNetworkIndicatorImpTest, TestUpdateNetworkIndex)
{
    CRTPSessionClientMock rtpSessionClient1;
    CRTPSessionClientMock rtpSessionClient2;

    CNetworkIndicatorImp::getInstance()->Stop();

    bool ret = CNetworkIndicatorImp::getInstance()->RegisterSessionClient(&rtpSessionClient1);
    ret = CNetworkIndicatorImp::getInstance()->RegisterSessionClient(&rtpSessionClient2);

    SessStats stats1;
    stats1.outStats.netStats.rtt = 500;
    stats1.outStats.netStats.lossRatio = 0.8f;
    SessStats stats2;
    stats2.outStats.netStats.rtt = 400;
    stats2.outStats.netStats.lossRatio = 0.8f;
    EXPECT_CALL(rtpSessionClient1, GetSessionStats(_)).WillRepeatedly(DoAll(SetArgReferee<0>(stats1), Return(0)));
    EXPECT_CALL(rtpSessionClient2, GetSessionStats(_)).WillRepeatedly(DoAll(SetArgReferee<0>(stats2), Return(0)));

    uint32_t now = 0;

    WRTP_INFOTRACE("first update");
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(now);

    CNetworkIndicator::NetworkIndex idx = CNetworkIndicatorImp::getInstance()->GetNetworkIndex(CNetworkIndicator::DIRECTION_UPLINK);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_SEVERE);

    idx = CNetworkIndicatorImp::getInstance()->GetNetworkIndex(CNetworkIndicator::DIRECTION_DOWNLINK);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_EXCELLENT);

    WRTP_INFOTRACE("second update");
    now += 500;
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(now);

    idx = CNetworkIndicatorImp::getInstance()->GetNetworkIndex(CNetworkIndicator::DIRECTION_UPLINK);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_SEVERE);


    WRTP_INFOTRACE("third update");
    now += 500;
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(now);

    idx = CNetworkIndicatorImp::getInstance()->GetNetworkIndex(CNetworkIndicator::DIRECTION_UPLINK);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_SEVERE);

    stats1.outStats.netStats.rtt = 0;
    stats1.outStats.netStats.lossRatio = 0.0f;
    stats2.outStats.netStats.rtt = 0;
    stats2.outStats.netStats.lossRatio = 0.0f;
    EXPECT_CALL(rtpSessionClient1, GetSessionStats(_)).WillRepeatedly(DoAll(SetArgReferee<0>(stats1), Return(0)));
    EXPECT_CALL(rtpSessionClient2, GetSessionStats(_)).WillRepeatedly(DoAll(SetArgReferee<0>(stats2), Return(0)));

    WRTP_INFOTRACE("recover first update");
    now += 5000;
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(now);

    idx = CNetworkIndicatorImp::getInstance()->GetNetworkIndex(CNetworkIndicator::DIRECTION_UPLINK);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_SEVERE);


    WRTP_INFOTRACE("recover second update");
    now += 500;
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(now);

    idx = CNetworkIndicatorImp::getInstance()->GetNetworkIndex(CNetworkIndicator::DIRECTION_UPLINK);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_NOT_ACCEPTABLE);


    WRTP_INFOTRACE("recover third update");
    now += 500;
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(now);

    idx = CNetworkIndicatorImp::getInstance()->GetNetworkIndex(CNetworkIndicator::DIRECTION_UPLINK);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_POOR);

    WRTP_INFOTRACE("recover fourth update");
    now += 500;
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(now);

    idx = CNetworkIndicatorImp::getInstance()->GetNetworkIndex(CNetworkIndicator::DIRECTION_UPLINK);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_POOR);

    WRTP_INFOTRACE("recover fifth update");
    now += 500;
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(now);

    idx = CNetworkIndicatorImp::getInstance()->GetNetworkIndex(CNetworkIndicator::DIRECTION_UPLINK);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_GOOD);

    WRTP_INFOTRACE("recover sixth update");
    now += 500;
    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(now);

    idx = CNetworkIndicatorImp::getInstance()->GetNetworkIndex(CNetworkIndicator::DIRECTION_UPLINK);
    ASSERT_EQ(idx, CNetworkIndicator::NETWORK_GOOD);

    ret = CNetworkIndicatorImp::getInstance()->UnRegisterSessionClient(&rtpSessionClient1);
    ret = CNetworkIndicatorImp::getInstance()->UnRegisterSessionClient(&rtpSessionClient2);

    CNetworkIndicatorImp::getInstance()->Start();
}

//Disable it because with CFakeThread enabled now, it will not trigger onTimer
/*TEST(CNetworkIndicatorImpTest, DISABLED_TestOnTimerGetCalled)
{
    CRTPSessionClientMock rtpSessionClient;

    bool ret = CNetworkIndicatorImp::getInstance()->RegisterSessionClient(&rtpSessionClient);

    RTCPStats stats;
    EXPECT_CALL(rtpSessionClient, GetRTCPStats()).Times(AtLeast(1)).WillRepeatedly(Return(stats));

    CmResult result = CNetworkIndicatorImp::getInstance()->Start("networkindicator");
    ASSERT_EQ(result, CM_OK);

    ::SleepMs(100 + CIndicatorConfigLoader::getInstance()->GetUpdateIntervalInMilliSec());

    ret = CNetworkIndicatorImp::getInstance()->UnRegisterSessionClient(&rtpSessionClient);
}*/

TEST(CNetworkIndicatorTest, TestCNetworkIndicatorGetNetworkIndex)
{
    CRTPSessionClientMock rtpSessionClient1;
    CRTPSessionClientMock rtpSessionClient2;

    CNetworkIndicatorImp::getInstance()->Stop();

    bool ret = CNetworkIndicatorImp::getInstance()->RegisterSessionClient(&rtpSessionClient1);
    ret = CNetworkIndicatorImp::getInstance()->RegisterSessionClient(&rtpSessionClient2);

    SessStats stats1;
    stats1.outStats.netStats.rtt = 500;
    stats1.outStats.netStats.lossRatio = 0.8f;
    SessStats stats2;
    stats2.outStats.netStats.rtt = 400;
    stats2.outStats.netStats.lossRatio = 0.8f;
    EXPECT_CALL(rtpSessionClient1, GetSessionStats(_)).WillRepeatedly(DoAll(SetArgReferee<0>(stats1), Return(0)));
    EXPECT_CALL(rtpSessionClient2, GetSessionStats(_)).WillRepeatedly(DoAll(SetArgReferee<0>(stats2), Return(0)));

    CNetworkIndicatorImp::getInstance()->UpdateNetworkIndex(0);

    ASSERT_EQ(CNetworkIndicator::NETWORK_SEVERE, CNetworkIndicator::GetNetworkIndex(CNetworkIndicator::DIRECTION_UPLINK));

    ASSERT_EQ(CNetworkIndicator::NETWORK_EXCELLENT, CNetworkIndicator::GetNetworkIndex(CNetworkIndicator::DIRECTION_DOWNLINK));

    ASSERT_EQ(CNetworkIndicator::NETWORK_POOR, CNetworkIndicator::GetNetworkIndex(CNetworkIndicator::DIRECTION_BOTHLINK));

    ret = CNetworkIndicatorImp::getInstance()->UnRegisterSessionClient(&rtpSessionClient1);
    ret = CNetworkIndicatorImp::getInstance()->UnRegisterSessionClient(&rtpSessionClient2);

    CNetworkIndicatorImp::getInstance()->Start();
}

TEST(CNetworkIndicatorTest, TestCNetworkIndicatorUpdateAvgNetworkIndex)
{
    float avgIdx = CNetworkIndicator::NETWORK_UNKNOWN;
    CNetworkIndicator::NetworkIndex idx = CNetworkIndicator::NETWORK_VERY_GOOD;
    CNetworkIndicatorImp::UpdateAvgNetworkIndex(avgIdx, idx, 0.8f);

    ASSERT_EQ(static_cast<int32_t>(round(avgIdx)), idx);

    idx = CNetworkIndicator::NETWORK_EXCELLENT;

    for (int i=0; i<100; ++i) {
        CNetworkIndicatorImp::UpdateAvgNetworkIndex(avgIdx, idx, 0.8f);
        if (static_cast<int32_t>(round(avgIdx)) == idx) {
            break;
        }
    }

    ASSERT_EQ(static_cast<int32_t>(round(avgIdx)), idx);
}
