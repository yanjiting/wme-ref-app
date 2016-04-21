#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "testutil.h"
#include "speedmeasure.h"

using ::testing::_;
using ::testing::Return;
using ::testing::AtLeast;
using namespace wrtp;

class SpdMeasureTest : public testing::Test
{
protected:

    virtual void SetUp()
    {
        lastTick = 0;
    }

    virtual void TearDown()
    {
    }

    void TestWithBandwidthPerSec(int32_t rate, int32_t pktlen)
    {
        int32_t ptkcntPerSec = (rate+pktlen-1)/pktlen;
        int32_t tsGap = 1000/ptkcntPerSec;//ms

        int32_t sendTs = lastTick + tsGap;
        for (int32_t i =0; i<ptkcntPerSec; ++i) {
            sendTs += tsGap;
            if (i == ptkcntPerSec-1) {
                m.Add(rate - i*pktlen, sendTs);
            } else {
                m.Add(pktlen, sendTs);
            }
        }
        lastTick = sendTs;
        ASSERT_EQ(m.GetSpeed(sendTs), rate*8);
    }

    CSpeedMeasure m;
    int32_t lastTick;
};

TEST_F(SpdMeasureTest, TestWithDifferentRateAndPktlen)
{
    CScopedTracer test_info;
    int32_t pktlen = 1000;
    //change rate per sec
    TestWithBandwidthPerSec(3*pktlen/10, pktlen);
    TestWithBandwidthPerSec(15*pktlen/10, pktlen);
    TestWithBandwidthPerSec(5*pktlen, pktlen);
    TestWithBandwidthPerSec(10*pktlen, pktlen);
    TestWithBandwidthPerSec(20*pktlen, pktlen);

    pktlen = 500;
    //change pktlen
    TestWithBandwidthPerSec(3*pktlen/10, pktlen);
    TestWithBandwidthPerSec(15*pktlen/10, pktlen);
    TestWithBandwidthPerSec(5*pktlen, pktlen);
    TestWithBandwidthPerSec(10*pktlen, pktlen);
    TestWithBandwidthPerSec(20*pktlen, pktlen);
}

TEST_F(SpdMeasureTest, TestWithBytes)
{
    int32_t sendTs = 0;
    m.Add(0, sendTs);
    sendTs = lastTick + 1000;
    m.Add(86780, sendTs);

    ASSERT_EQ(m.GetSpeed(sendTs), 86780*8);
}
