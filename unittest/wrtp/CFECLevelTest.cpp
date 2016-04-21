#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "feclevel.h"
#include "testutil.h"

using namespace wrtp;

class CFECLevelTest : public ::testing::Test
{
public:
    CFECLevelTest() {}
    ~CFECLevelTest() {}

protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
public:
    CFECLevel fecLevel;
};

TEST_F(CFECLevelTest, lossratio_or_srcCount_is_0_then_GetVideoRepairCount_return_0)
{
    CScopedTracer test_info;
    unsigned lossRatio = 0;
    unsigned  priority = 2;
    unsigned srcCount = 50;
    EXPECT_EQ(0, fecLevel.GetVideoRepairCount(lossRatio, priority, srcCount));
    lossRatio = 140;
    srcCount = 0;
    EXPECT_EQ(0, fecLevel.GetVideoRepairCount(lossRatio, priority, srcCount));
}

TEST_F(CFECLevelTest, priority_is_0_then_return_correct_value)
{
    CScopedTracer test_info;
    unsigned lossRatio = 30;
    unsigned  priority = 0;
    unsigned srcCount = 3;
    EXPECT_EQ(1, fecLevel.GetVideoRepairCount(lossRatio, priority, srcCount));
}

TEST_F(CFECLevelTest, priority_is_16_then_return_correct_value)
{
    CScopedTracer test_info;
    unsigned lossRatio = 260;
    unsigned  priority = 16;
    unsigned srcCount = 6;
    EXPECT_EQ(6, fecLevel.GetVideoRepairCount(lossRatio, priority, srcCount));
}

TEST_F(CFECLevelTest, priority_is_16_and_srcCount_is_66_then_return_correct_value)
{
    CScopedTracer test_info;
    unsigned lossRatio = 260;
    unsigned  priority = 16;
    unsigned srcCount = 66;
    EXPECT_EQ(39, fecLevel.GetVideoRepairCount(lossRatio, priority, srcCount));
}

TEST_F(CFECLevelTest, lossratio_is_0_fec_ratio_is_2)
{
    float fecRatio = CFECLevel::GetFecRatio(0);
    EXPECT_EQ(2, (int)(fecRatio*10));
}

TEST_F(CFECLevelTest, lossratio_media_bandwidth_is_0_return_0)
{
    int fecBw = CFECLevel::GetFecBandwidth(0, 0);
    EXPECT_EQ(0, fecBw);
}

/* Disable dynamic
TEST_F(CFECLevelTest, lossratio_is_0_return_1_out_10)
{
    int fecBw = CFECLevel::GetFecBandwidth(100, 0);
    EXPECT_EQ(10, fecBw);
}

TEST_F(CFECLevelTest, lossratio_is_90_return_2_out_10)
{
    int fecBw = CFECLevel::GetFecBandwidth(100, 900);
    EXPECT_EQ(20, fecBw);
}
*/
