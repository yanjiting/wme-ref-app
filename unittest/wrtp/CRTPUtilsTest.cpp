#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "rtputils.h"
#include "wrtpapi.h"
#include "mari/wqos/qosdefines.h"
#include "rtpheaderext.h"
#include "activebuffer.h"
#include "testutil.h"

using namespace wrtp;
class CRTPUtilsTEST : public ::testing::Test
{
};
void RunRtpPerfTest();
TEST_F(CRTPUtilsTEST, WRTPSetOption_WRTP_OPTION_QOS_MAX_LOSS_RATIO)
{
#ifdef ENABLE_PERFORMENCE_TEST
    RunRtpPerfTest();
    return;
#endif
    CScopedTracer test_info;
    float maxLossRatio = 0.11;
    EXPECT_EQ(WRTP_ERR_NOERR, WRTPSetOption(WRTP_OPTION_QOS_MAX_LOSS_RATIO, &maxLossRatio, sizeof(maxLossRatio)));
    float f = 0.0;
    uint32_t len = sizeof(f);
    EXPECT_EQ(0, wqos::QoSGetOption(wqos::QOS_OPTION_MAX_LOSS_RATIO, &f, &len));
    EXPECT_EQ(maxLossRatio, f);
}

TEST_F(CRTPUtilsTEST, WRTPSetOption_WRTP_OPTION_QOS_MIN_BANDWIDTH)
{
    CScopedTracer test_info;
    uint32_t minBandwidth = 32*1024;
    EXPECT_EQ(WRTP_ERR_NOERR, WRTPSetOption(WRTP_OPTION_QOS_MIN_BANDWIDTH, &minBandwidth, sizeof(minBandwidth)));
    uint32_t bw = 0;
    uint32_t len = sizeof(bw);
    EXPECT_EQ(0, wqos::QoSGetOption(wqos::QOS_OPTION_MIN_BANDWIDTH, &bw, &len));
    EXPECT_EQ(minBandwidth, bw);
}

TEST(CRTPSequenceManagerTEST, CRTPSequenceManager_Get_Check)
{
    CScopedTracer test_info;
    CRTPSequenceManagerSharedPtr rtpSequenceManager(new CRTPSequenceManager());
    uint16_t checkSeq = rtpSequenceManager->CheckNextSequence(123);
    uint16_t getSeq = rtpSequenceManager->GetNextSequence(123);
    EXPECT_EQ(checkSeq, getSeq);
}

TEST(GroupTokenBucketManagerTEST, GroupTokenBucketManager_Get_Return)
{
    CScopedTracer test_info;
    char keyChar1[] = "key1";
    wqos::QoSGroupKey key1((uint8_t *)keyChar1, sizeof(keyChar1));
    CCmComAutoPtr<CTokenBucket> bucket1 = GroupTokenBucketInstance()->GetTokenBucket(key1);
    EXPECT_EQ(bucket1->GetReference(), 2);
    //CCmComAutoPtr has already implemented "operator void*" which does the implicit type conversion
    EXPECT_EQ(bucket1, GroupTokenBucketInstance()->GetTokenBucket(key1));
    EXPECT_EQ(bucket1->GetReference(), 2);
    CTokenBucket *pBucket = bucket1.Get();
    bucket1 = nullptr;//We have to do this first and return the bucket
    EXPECT_EQ(pBucket->GetReference(), 1);
    GroupTokenBucketInstance()->ReturnTokenBucket(key1);
}

TEST(GroupTokenBucketManagerTEST, GroupTokenBucketManager_Get_Return_Invalid)
{
    CScopedTracer test_info;
    const char *peerID = "peerID";
    QoSParameter qosParam;
    qosParam.peerID             = reinterpret_cast<const uint8_t *>(peerID);
    qosParam.lengthOfPeerID     = strlen(peerID);
    wqos::QoSGroupKey key(qosParam.peerID, qosParam.lengthOfPeerID);
    CCmComAutoPtr<CTokenBucket> bucket1 = GroupTokenBucketInstance()->GetTokenBucket(key);
    bucket1 = nullptr;
    GroupTokenBucketInstance()->ReturnTokenBucket(key);
}


TEST(VIDDuplicationTest, Duplication_Test)
{
    ASSERT_EQ(false, CheckVIDDup(nullptr, 0));

    uint8_t vids[] = {0};
    ASSERT_EQ(false, CheckVIDDup(vids, sizeof(vids)));

    uint8_t vids0[] = {0, 1, 2, 3, 4};
    ASSERT_EQ(false, CheckVIDDup(vids0, sizeof(vids0)));

    uint8_t vids1[] = {0, 1, 2, 3, 0};
    ASSERT_EQ(true, CheckVIDDup(vids1, sizeof(vids1)));

    uint8_t vids2[] = {0, 1, 2, 3, 1};
    ASSERT_EQ(true, CheckVIDDup(vids2, sizeof(vids2)));

    uint8_t vids3[] = {0, 0};
    ASSERT_EQ(true, CheckVIDDup(vids3, sizeof(vids3)));
}
