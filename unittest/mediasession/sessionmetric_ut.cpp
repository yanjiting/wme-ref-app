#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "stringhelper.h"
#include <list>
#include "json.h"
#include "SessionMetrics.h"

using namespace wme;

class SessionMetricUT: public ::testing::Test
{
public:
    struct BucketResult {
        BucketResult()
        : m_min(0)
        , m_max(0)
        , m_duration(0) {}
        
        BucketResult(int32_t minValue, int32_t maxValue, uint32_t cnt)
        : m_min(minValue)
        , m_max(maxValue)
        , m_duration(cnt) {}
        int32_t m_min;
        int32_t m_max;
        uint32_t m_duration;
    };
    
    struct statsResult {
        statsResult()
        : fecEnable(false)
        , mediaContent("")
        , mediaType("")
        , afterFecLossRatio()
        , lossRatio() {}
        
        statsResult(bool enabled, std::string mediaContentValue, std::string mediaTypeValue, std::vector<BucketResult> &lossRatioValue, std::vector<BucketResult> &afterFecLossRatioValue)
        : fecEnable(enabled)
        , mediaContent(mediaContentValue)
        , mediaType(mediaTypeValue)
        , afterFecLossRatio(afterFecLossRatioValue)
        , lossRatio(lossRatioValue) {}
        
        bool fecEnable;
        std::string mediaContent;
        std::string mediaType;
        std::vector<BucketResult> afterFecLossRatio;
        std::vector<BucketResult> lossRatio;
    };
    
    void CompareBucketMetrics(json::Object obj, const char* name, std::vector<BucketResult> &expResults) {
        ASSERT_TRUE(obj.HasKey(name));
        json::Array array = obj[name].ToArray();
        std::vector<BucketResult>::iterator results = expResults.begin();
        ASSERT_EQ(expResults.size(), array.size());
        for (json::Array::ValueVector::iterator it = array.begin(); it != array.end(); ++it) {
            ASSERT_TRUE((*it).HasKey("duration"));
            ASSERT_TRUE((*it).HasKey("min"));
            ASSERT_TRUE((*it).HasKey("max"));
            EXPECT_EQ(results->m_min, (*it)["min"].ToInt());
            EXPECT_EQ(results->m_max, (*it)["max"].ToInt());
            EXPECT_EQ(results->m_duration, (*it)["duration"].ToInt());
            ++results;
        }
    }
    
    void CompareStatsMetrics(json::Value obj, std::vector<statsResult> expResults)
    {
        ASSERT_TRUE(obj.HasKey("stats"));
        json::Array array = obj["stats"].ToArray();
        ASSERT_EQ(expResults.size(), array.size());
        std::vector<statsResult>::iterator result = expResults.begin();
        for (json::Array::ValueVector::iterator it = array.begin(); it != array.end(); ++it) {
            ASSERT_TRUE((*it).HasKey("fecEnable"));
            ASSERT_TRUE((*it).HasKey("mediaContent"));
            ASSERT_TRUE((*it).HasKey("mediaType"));
            EXPECT_TRUE(result->fecEnable == (*it)["fecEnable"].ToBool());
            EXPECT_EQ(0, (*it)["mediaContent"].ToString().compare(result->mediaContent));
            EXPECT_EQ(0, (*it)["mediaType"].ToString().compare(result->mediaType));
            if (result->lossRatio.size() != 0) {
                ASSERT_TRUE((*it).HasKey("lossRatio"));
                CompareBucketMetrics((*it).ToObject(), "lossRatio", result->lossRatio);
            }
            if (result->afterFecLossRatio.size()) {
                ASSERT_TRUE((*it).HasKey("afterFecLossRatio"));
                CompareBucketMetrics((*it).ToObject(), "afterFecLossRatio", result->afterFecLossRatio);
            }
            
            ++result;
        }
    }
    
    void CompareSrtpMetrics(json::Value obj, bool enabled, uint32_t rtcpAuthFail, uint32_t rtpAuthFail) {
        ASSERT_TRUE(obj.HasKey("srtp"));
        json::Value srtp = obj["srtp"];
        ASSERT_TRUE(srtp.HasKey("enabled"));
        ASSERT_TRUE(srtp.HasKey("rtcpAuthFail"));
        ASSERT_TRUE(srtp.HasKey("rtpAuthFail"));
        EXPECT_TRUE(enabled == srtp["enabled"].ToBool());
        EXPECT_EQ(rtcpAuthFail, srtp["rtcpAuthFail"].ToInt());
        EXPECT_EQ(rtpAuthFail, srtp["rtpAuthFail"].ToInt());
    }
};


TEST_F(SessionMetricUT, TestSessionMetrics)
{
	CSessionMetrics metrics(0);
	WmeCpuUsage cpuUsage = { 0 };
	cpuUsage.nCores = 2;

    cpuUsage.fProcessUsage = 40;
    cpuUsage.fTotalUsage = 61;

	metrics.Kick(cpuUsage);

    cpuUsage.fProcessUsage = 95;
    cpuUsage.fTotalUsage = 75;

    metrics.Kick(cpuUsage);

    WmeSrtpMetric srtpMetric = {};
    srtpMetric.bEnabled = true;
    srtpMetric.uRTPAuthFail = 2;
    srtpMetric.uRTCPAuthFail = 1;
    metrics.Kick(srtpMetric);

	const char* sResult = metrics.GetMetrics();
    
    std::string result(sResult);
    json::Object obj = json::Deserialize(result).ToObject();
    std::vector<BucketResult> cpuSqRes;
    cpuSqRes.push_back(BucketResult(-1,60,1));
    cpuSqRes.push_back(BucketResult(90,100,1));
    CompareBucketMetrics(obj, "cpuSq", cpuSqRes);
    
    std::vector<BucketResult> cpuSysRes;
    cpuSysRes.push_back(BucketResult(60,70,1));
    cpuSysRes.push_back(BucketResult(70,80,1));
    CompareBucketMetrics(obj, "cpuSystem", cpuSysRes);
    
    CompareSrtpMetrics(obj, true, 1, 2);
    
    std::vector<statsResult> statsRes;
    std::vector<BucketResult> lossRatio;
    std::vector<BucketResult> afterFecLossRatio;
    
    statsRes.push_back(statsResult(false, "main", "audio", lossRatio, afterFecLossRatio));
	statsRes.push_back(statsResult(false, "main", "video", lossRatio, afterFecLossRatio));
	statsRes.push_back(statsResult(false, "slides", "screen", lossRatio, afterFecLossRatio));

	CompareStatsMetrics(obj, statsRes);
}

TEST_F(SessionMetricUT, TestSessionAudioFecMetrics)
{
    CSessionMetrics metrics(0);
    WmeAudioNetworkMetrics fecMetrics;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 2;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 5;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 7;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 12;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 17;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 22;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 25;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 30;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 90;
    metrics.Kick(fecMetrics);

    const char* sResult = metrics.GetMetrics();
    
    std::string result(sResult);
    json::Object obj = json::Deserialize(result).ToObject();
    std::vector<BucketResult> cpuSqRes;
    std::vector<BucketResult> cpuSystemRes;
    CompareBucketMetrics(obj, "cpuSq", cpuSqRes);
    CompareBucketMetrics(obj, "cpuSystem", cpuSystemRes);
    CompareSrtpMetrics(obj, false, 0, 0);
    
    std::vector<statsResult> statsRes;
    std::vector<BucketResult> lossRatio;
    std::vector<BucketResult> afterFecLossRatio;
    lossRatio.push_back(BucketResult(-1, 0, 1));
    lossRatio.push_back(BucketResult(0, 2, 1));
    lossRatio.push_back(BucketResult(2, 5, 1));
    lossRatio.push_back(BucketResult(5, 7, 1));
    lossRatio.push_back(BucketResult(7, 12, 1));
    lossRatio.push_back(BucketResult(12, 17, 1));
    lossRatio.push_back(BucketResult(17, 22, 1));
    lossRatio.push_back(BucketResult(22, 25, 1));
    lossRatio.push_back(BucketResult(25, 30, 1));
    lossRatio.push_back(BucketResult(30, 100, 1));

    statsRes.push_back(statsResult(false, "main", "audio", lossRatio, afterFecLossRatio));
    
    lossRatio.clear();
	statsRes.push_back(statsResult(false, "main", "video", lossRatio, afterFecLossRatio));

	lossRatio.clear();
	afterFecLossRatio.clear();
	statsRes.push_back(statsResult(false, "slides", "screen", lossRatio, afterFecLossRatio));

    CompareStatsMetrics(obj, statsRes);
}

TEST_F(SessionMetricUT, TestSessionEnableAudioFecMetrics)
{
    CSessionMetrics metrics(0);
    metrics.EnableAudioFec(true);

    WmeAudioNetworkMetrics fecMetrics;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 2;
    fecMetrics.afterFecLossRatio = 1;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 5;
    fecMetrics.afterFecLossRatio = 4;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 7;
    fecMetrics.afterFecLossRatio = 6;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 12;
    fecMetrics.afterFecLossRatio = 11;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 17;
    fecMetrics.afterFecLossRatio = 16;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 22;
    fecMetrics.afterFecLossRatio = 21;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 25;
    fecMetrics.afterFecLossRatio = 24;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 30;
    fecMetrics.afterFecLossRatio = 29;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 90;
    fecMetrics.afterFecLossRatio = 89;
    metrics.Kick(fecMetrics);

    const char* sResult = metrics.GetMetrics();
    
    std::string result(sResult);
    json::Object obj = json::Deserialize(result).ToObject();
    std::vector<BucketResult> cpuSqRes;
    std::vector<BucketResult> cpuSystemRes;
    CompareBucketMetrics(obj, "cpuSq", cpuSqRes);
    CompareBucketMetrics(obj, "cpuSystem", cpuSystemRes);
    CompareSrtpMetrics(obj, false, 0, 0);
    
    std::vector<statsResult> statsRes;
    std::vector<BucketResult> lossRatio;
    std::vector<BucketResult> afterFecLossRatio;
    afterFecLossRatio.push_back(BucketResult(-1, 0, 1));
    afterFecLossRatio.push_back(BucketResult(0, 2, 1));
    afterFecLossRatio.push_back(BucketResult(2, 5, 1));
    afterFecLossRatio.push_back(BucketResult(5, 7, 1));
    afterFecLossRatio.push_back(BucketResult(7, 12, 1));
    afterFecLossRatio.push_back(BucketResult(12, 17, 1));
    afterFecLossRatio.push_back(BucketResult(17, 22, 1));
    afterFecLossRatio.push_back(BucketResult(22, 25, 1));
    afterFecLossRatio.push_back(BucketResult(25, 30, 1));
    afterFecLossRatio.push_back(BucketResult(30, 100, 1));

    lossRatio.push_back(BucketResult(-1, 0, 1));
    lossRatio.push_back(BucketResult(0, 2, 1));
    lossRatio.push_back(BucketResult(2, 5, 1));
    lossRatio.push_back(BucketResult(5, 7, 1));
    lossRatio.push_back(BucketResult(7, 12, 1));
    lossRatio.push_back(BucketResult(12, 17, 1));
    lossRatio.push_back(BucketResult(17, 22, 1));
    lossRatio.push_back(BucketResult(22, 25, 1));
    lossRatio.push_back(BucketResult(25, 30, 1));
    lossRatio.push_back(BucketResult(30, 100, 1));
    
    statsRes.push_back(statsResult(true, "main", "audio", lossRatio, afterFecLossRatio));
    
    afterFecLossRatio.clear();
    lossRatio.clear();
    statsRes.push_back(statsResult(false, "main", "video", lossRatio, afterFecLossRatio));

	lossRatio.clear();
	afterFecLossRatio.clear();
	statsRes.push_back(statsResult(false, "slides", "screen", lossRatio, afterFecLossRatio));
    
    CompareStatsMetrics(obj, statsRes);
}

TEST_F(SessionMetricUT, TestSessionVideoFecMetrics)
{
    CSessionMetrics metrics(0);
    WmeVideoNetworkMetrics fecMetrics;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 2;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 5;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 7;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 12;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 17;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 22;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 25;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 30;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 90;
    metrics.Kick(fecMetrics);

    const char* sResult = metrics.GetMetrics();
    
    std::string result(sResult);
    json::Object obj = json::Deserialize(result).ToObject();
    std::vector<BucketResult> cpuSqRes;
    std::vector<BucketResult> cpuSystemRes;
    CompareBucketMetrics(obj, "cpuSq", cpuSqRes);
    CompareBucketMetrics(obj, "cpuSystem", cpuSystemRes);
    CompareSrtpMetrics(obj, false, 0, 0);
    
    std::vector<statsResult> statsRes;
    std::vector<BucketResult> lossRatio;
    std::vector<BucketResult> afterFecLossRatio;
    
    statsRes.push_back(statsResult(false, "main", "audio", lossRatio, afterFecLossRatio));
    
    lossRatio.push_back(BucketResult(-1, 0, 1));
    lossRatio.push_back(BucketResult(0, 2, 1));
    lossRatio.push_back(BucketResult(2, 5, 1));
    lossRatio.push_back(BucketResult(5, 7, 1));
    lossRatio.push_back(BucketResult(7, 12, 1));
    lossRatio.push_back(BucketResult(12, 17, 1));
    lossRatio.push_back(BucketResult(17, 22, 1));
    lossRatio.push_back(BucketResult(22, 25, 1));
    lossRatio.push_back(BucketResult(25, 30, 1));
    lossRatio.push_back(BucketResult(30, 100, 1));
    statsRes.push_back(statsResult(false, "main", "video", lossRatio, afterFecLossRatio));

	lossRatio.clear();
	afterFecLossRatio.clear();
	statsRes.push_back(statsResult(false, "slides", "screen", lossRatio, afterFecLossRatio));

    CompareStatsMetrics(obj, statsRes);
 }

TEST_F(SessionMetricUT, TestSessionEnableVideoFecMetrics)
{
    CSessionMetrics metrics(0);
    metrics.EnableVideoFec(true);

    WmeVideoNetworkMetrics fecMetrics;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 2;
    fecMetrics.afterFecLossRatio = 1;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 5;
    fecMetrics.afterFecLossRatio = 4;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 7;
    fecMetrics.afterFecLossRatio = 6;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 12;
    fecMetrics.afterFecLossRatio = 11;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 17;
    fecMetrics.afterFecLossRatio = 16;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 22;
    fecMetrics.afterFecLossRatio = 21;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 25;
    fecMetrics.afterFecLossRatio = 24;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 30;
    fecMetrics.afterFecLossRatio = 29;
    metrics.Kick(fecMetrics);
    fecMetrics.lossRatio = 90;
    fecMetrics.afterFecLossRatio = 89;
    metrics.Kick(fecMetrics);

    const char* sResult = metrics.GetMetrics();
    
    std::string result(sResult);
    json::Object obj = json::Deserialize(result).ToObject();
    std::vector<BucketResult> cpuSqRes;
    std::vector<BucketResult> cpuSystemRes;
    CompareBucketMetrics(obj, "cpuSq", cpuSqRes);
    CompareBucketMetrics(obj, "cpuSystem", cpuSystemRes);
    CompareSrtpMetrics(obj, false, 0, 0);
    
    std::vector<statsResult> statsRes;
    std::vector<BucketResult> lossRatio;
    std::vector<BucketResult> afterFecLossRatio;

    statsRes.push_back(statsResult(false, "main", "audio", lossRatio, afterFecLossRatio));
    
    afterFecLossRatio.push_back(BucketResult(-1, 0, 1));
    afterFecLossRatio.push_back(BucketResult(0, 2, 1));
    afterFecLossRatio.push_back(BucketResult(2, 5, 1));
    afterFecLossRatio.push_back(BucketResult(5, 7, 1));
    afterFecLossRatio.push_back(BucketResult(7, 12, 1));
    afterFecLossRatio.push_back(BucketResult(12, 17, 1));
    afterFecLossRatio.push_back(BucketResult(17, 22, 1));
    afterFecLossRatio.push_back(BucketResult(22, 25, 1));
    afterFecLossRatio.push_back(BucketResult(25, 30, 1));
    afterFecLossRatio.push_back(BucketResult(30, 100, 1));
    
    lossRatio.push_back(BucketResult(-1, 0, 1));
    lossRatio.push_back(BucketResult(0, 2, 1));
    lossRatio.push_back(BucketResult(2, 5, 1));
    lossRatio.push_back(BucketResult(5, 7, 1));
    lossRatio.push_back(BucketResult(7, 12, 1));
    lossRatio.push_back(BucketResult(12, 17, 1));
    lossRatio.push_back(BucketResult(17, 22, 1));
    lossRatio.push_back(BucketResult(22, 25, 1));
    lossRatio.push_back(BucketResult(25, 30, 1));
    lossRatio.push_back(BucketResult(30, 100, 1));

    statsRes.push_back(statsResult(true, "main", "video", lossRatio, afterFecLossRatio));
    
	lossRatio.clear();
	afterFecLossRatio.clear();
	statsRes.push_back(statsResult(false, "slides", "screen", lossRatio, afterFecLossRatio));

    CompareStatsMetrics(obj, statsRes);
}


TEST_F(SessionMetricUT, TestVideoMetrics)
{
#define BANDWIDTH90 (100000)
#define BANDWIDTH180 (300000)
#define BANDWIDTH360 (600000)
#define BANDWIDTH720 (1400000)

  CSessionMetrics metrics(0);

  WmeVideoMetrics videoMetrics;
  videoMetrics.uOutOfSyncRatio = 0;
  videoMetrics.uEcAreaRatio = 0;
  videoMetrics.bIsOutOfSyncSecond = false;
  videoMetrics.uCurrentResolutionFS = 3600;
  videoMetrics.uTargetResolutionFS = 3840;
  metrics.Kick(videoMetrics);
  metrics.Kick(BANDWIDTH720, 30, 720, 1280);

  videoMetrics.uOutOfSyncRatio = 3;
  videoMetrics.uEcAreaRatio = 50;
  videoMetrics.bIsOutOfSyncSecond = true;
  videoMetrics.uCurrentResolutionFS = 3600;
  videoMetrics.uTargetResolutionFS = 3840;
  metrics.Kick(videoMetrics);
  metrics.Kick(BANDWIDTH360, 30, 640, 360);

  videoMetrics.uOutOfSyncRatio = 23;
  videoMetrics.uEcAreaRatio = 55;
  videoMetrics.bIsOutOfSyncSecond = false;
  videoMetrics.uCurrentResolutionFS = 3600;
  videoMetrics.uTargetResolutionFS = 3840;
  metrics.Kick(videoMetrics);
  metrics.Kick(BANDWIDTH180, 15, 320, 180);

  videoMetrics.uOutOfSyncRatio = 55;
  videoMetrics.uEcAreaRatio = 78;
  videoMetrics.bIsOutOfSyncSecond = true;
  videoMetrics.uCurrentResolutionFS = 225;
  videoMetrics.uTargetResolutionFS = 3840;
  metrics.Kick(videoMetrics);
  metrics.Kick(BANDWIDTH90, 15, 90, 160);

  const char* sResult = metrics.GetMetrics();

  std::string result(sResult);
  json::Object obj = json::Deserialize(result).ToObject();
  json::Array array = obj["stats"].ToArray();
  for (json::Array::ValueVector::iterator it = array.begin(); it != array.end(); ++it) {
    //video metric
    if ((*it).HasKey("videoAvgSendingBandwidth"))
    {
      //sender side
      EXPECT_TRUE((*it).HasKey("videoAvgSendingBandwidth90p"));
      EXPECT_TRUE((*it).HasKey("videoAvgSendingBandwidth180p"));
      EXPECT_TRUE((*it).HasKey("videoAvgSendingBandwidth360p"));
      EXPECT_TRUE((*it).HasKey("videoAvgSendingBandwidth720p"));
      EXPECT_EQ(60000, (*it)["videoAvgSendingBandwidth"].ToInt()/10);
      EXPECT_EQ(100000, (*it)["videoAvgSendingBandwidth90p"].ToInt());
      EXPECT_EQ(300000, (*it)["videoAvgSendingBandwidth180p"].ToInt());
      EXPECT_EQ(600000, (*it)["videoAvgSendingBandwidth360p"].ToInt());
      EXPECT_EQ(1400000, (*it)["videoAvgSendingBandwidth720p"].ToInt());

      EXPECT_EQ(15, (*it)["videoAvgSendingFPS90p"].ToInt());
      EXPECT_EQ(15, (*it)["videoAvgSendingFPS180p"].ToInt());
      EXPECT_EQ(30, (*it)["videoAvgSendingFPS360p"].ToInt());
      EXPECT_EQ(30, (*it)["videoAvgSendingFPS720p"].ToInt());
      
      //receiver side
      EXPECT_TRUE((*it).HasKey("outOfSyncRatio"));
      EXPECT_TRUE((*it).HasKey("ecRatio"));
      EXPECT_TRUE((*it).HasKey("reachTargetResolution"));
      EXPECT_TRUE((*it).HasKey("longestContinualOooSeconds"));
      EXPECT_TRUE((*it).HasKey("totalOooSecondsRatio"));
      EXPECT_TRUE((*it).HasKey("videoReceiverMetricTime"));
      EXPECT_TRUE((*it).HasKey("videoReceiverMetricTimes90p"));
      EXPECT_TRUE((*it).HasKey("videoReceiverMetricTimes180p"));
      EXPECT_TRUE((*it).HasKey("videoReceiverMetricTimes360p"));
      EXPECT_TRUE((*it).HasKey("videoReceiverMetricTimes720p"));

      EXPECT_EQ(55, (*it)["outOfSyncRatio"].ToInt());
      EXPECT_EQ(78, (*it)["ecRatio"].ToInt());
      EXPECT_EQ(75, (*it)["reachTargetResolution"].ToInt());
      EXPECT_EQ(1, (*it)["longestContinualOooSeconds"].ToInt());
      EXPECT_EQ(50, (*it)["totalOooSecondsRatio"].ToInt());
      EXPECT_EQ(4, (*it)["videoReceiverMetricTime"].ToInt());
      EXPECT_EQ(0, (*it)["videoReceiverMetricTimes90p"].ToInt());
      EXPECT_EQ(1, (*it)["videoReceiverMetricTimes180p"].ToInt());
      EXPECT_EQ(0, (*it)["videoReceiverMetricTimes360p"].ToInt());
      EXPECT_EQ(3, (*it)["videoReceiverMetricTimes720p"].ToInt());
    }
  }
}

TEST_F(SessionMetricUT, TestMariMetrics)
{
    CSessionMetrics metrics(0);

    WmeVideoNetworkMetrics videoMetrics;
    videoMetrics.mariOutRecvRate = 900000;
    videoMetrics.mariInRecvRate = 900000;
    metrics.Kick(videoMetrics);

    WmeAudioNetworkMetrics audioMetrics;
    audioMetrics.mariInQdelay = 100;
    audioMetrics.mariOutRecvRate = 0;
    audioMetrics.mariInRecvRate = 0;
    metrics.Kick(audioMetrics);

    const char* sResult = metrics.GetMetrics();

    std::string result(sResult);
    json::Object obj = json::Deserialize(result).ToObject();

    ASSERT_TRUE(obj.HasKey("stats"));
    json::Array array = obj["stats"].ToArray();
    for (json::Array::ValueVector::iterator it = array.begin(); it != array.end(); ++it) {
        json::Object MediaObj = (*it);
        ASSERT_TRUE(MediaObj.HasKey("mediaType"));
        ASSERT_TRUE(MediaObj.HasKey("mediaContent"));
        if (MediaObj["mediaType"] == "audio") {
            ASSERT_TRUE(MediaObj.HasKey("mari"));
            ASSERT_TRUE(MediaObj["mari"].HasKey("inQdelay"));
            ASSERT_TRUE(MediaObj["mari"].HasKey("outQdelay"));
            ASSERT_TRUE(MediaObj["mari"].HasKey("inLossRatio"));
            ASSERT_TRUE(MediaObj["mari"].HasKey("outLossRatio"));
            ASSERT_TRUE(MediaObj["mari"].HasKey("inRecvRate"));
            ASSERT_TRUE(MediaObj["mari"].HasKey("outRecvRate"));
            std::vector<BucketResult> mariRecvRateResAudio;
            mariRecvRateResAudio.push_back(BucketResult(-1,100000,1));
            CompareBucketMetrics(MediaObj["mari"], "inRecvRate", mariRecvRateResAudio);
            CompareBucketMetrics(MediaObj["mari"], "outRecvRate", mariRecvRateResAudio);
        } else if (MediaObj["mediaType"] == "video" && MediaObj["mediaContent"] == "main") {
            ASSERT_TRUE(MediaObj.HasKey("mari"));
            ASSERT_TRUE(MediaObj["mari"].HasKey("inQdelay"));
            ASSERT_TRUE(MediaObj["mari"].HasKey("outQdelay"));
            ASSERT_TRUE(MediaObj["mari"].HasKey("inLossRatio"));
            ASSERT_TRUE(MediaObj["mari"].HasKey("outLossRatio"));
            ASSERT_TRUE(MediaObj["mari"].HasKey("inRecvRate"));
            ASSERT_TRUE(MediaObj["mari"].HasKey("outRecvRate"));
            std::vector<BucketResult> mariRecvRateResVideo;
            mariRecvRateResVideo.push_back(BucketResult(500000,1000000,1));
            CompareBucketMetrics(MediaObj["mari"], "inRecvRate", mariRecvRateResVideo);
            CompareBucketMetrics(MediaObj["mari"], "outRecvRate", mariRecvRateResVideo);
        }
    }
}
