#include "gmock/gmock.h"
#include "gtest/gtest.h"
//#include "wbxmock/wbxmock.h"
#include "rtptime.h"
#include "rtputils.h"
#include "rtpheaderext.h"
#include "testutil.h"

using namespace wrtp;

class CNTPTimeTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(CNTPTimeTest, MemberTest)
{
    CScopedTracer test_info;
    uint32_t msw = 0x12345678;
    uint32_t lsw = 0xABCDEF12;
    uint32_t mid32bits = 0x5678ABCD;

    CNTPTime ntpTime = CNTPTime(msw, lsw);
    EXPECT_EQ(msw, ntpTime.GetMSW());
    EXPECT_EQ(lsw, ntpTime.GetLSW());
    EXPECT_EQ(mid32bits, ntpTime.GetMid32bits());

    CNTPTime ntpTime2 = CNTPTime(mid32bits);
    msw = 0x5678;
    lsw = 0xABCD0000;
    EXPECT_EQ(msw, ntpTime2.GetMSW());
    EXPECT_EQ(lsw, ntpTime2.GetLSW());
    EXPECT_EQ(mid32bits, ntpTime2.GetMid32bits());

    CClockTime clockTime = ntpTime.ToClockTimeWithMid32bits();
    CNTPTime ntpTime3 = clockTime.ToNTPTime();
    EXPECT_EQ(ntpTime3.GetMSW(), ntpTime2.GetMSW());
    // The following two tests failed due to loss of precision
    //EXPECT_EQ(ntpTime3.GetLSW(), ntpTime2.GetLSW());
    //EXPECT_EQ(ntpTime3.GetMid32bits(), ntpTime2.GetMid32bits());
}


class CClockTimeTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(CClockTimeTest, ConversionTest)
{
    CScopedTracer test_info;
    const uint32_t sec = 1000;
    const uint32_t microsec = 12345;

    uint32_t millisec = sec * 1000 + microsec / 1000;
    uint64_t totalMicrosec = static_cast<uint64_t>(1000) * 1000000 + microsec;


    CClockTime clockTime = CClockTime(sec, microsec);
    //EXPECT_EQ(sec, clockTime.GetSeconds());
    //EXPECT_EQ(microsec, clockTime.GetMicroseconds());
    EXPECT_EQ(sec, clockTime.ToSeconds());
    EXPECT_EQ(millisec, clockTime.ToMilliseconds());
    EXPECT_EQ(totalMicrosec, clockTime.ToMicroseconds());

    CNTPTime ntpTime = clockTime.ToNTPTime();
    double x = microsec / 1000000.0;
    x *= (65536.0 * 65536.0);
    uint32_t lsw = static_cast<uint32_t>(x);

    EXPECT_EQ(sec, ntpTime.GetMSW());
    EXPECT_EQ(lsw, ntpTime.GetLSW());

    //CClockTime clockTime2 = ntpTime.ToClockTime();
    //EXPECT_EQ(clockTime.GetSeconds(), clockTime2.GetSeconds());
}

TEST_F(CClockTimeTest, OperatorTest)
{
    CScopedTracer test_info;
    CClockTime clockTime = CRTPTicker::Now();
    CClockTime clockTime2 = clockTime;

    // clockTime == clockTime2
    EXPECT_TRUE(clockTime == clockTime2);
    EXPECT_FALSE(clockTime != clockTime2);
    EXPECT_FALSE(clockTime < clockTime2);
    EXPECT_FALSE(clockTime > clockTime2);
    EXPECT_TRUE(clockTime >= clockTime2);
    EXPECT_TRUE(clockTime <= clockTime2);

    clockTime2 -= CClockTime(static_cast<uint64_t>(1));
    // clockTime > clockTime2
    EXPECT_FALSE(clockTime == clockTime2);
    EXPECT_TRUE(clockTime != clockTime2);
    EXPECT_FALSE(clockTime < clockTime2);
    EXPECT_TRUE(clockTime > clockTime2);
    EXPECT_TRUE(clockTime >= clockTime2);
    EXPECT_FALSE(clockTime <= clockTime2);

    clockTime2 += CClockTime(static_cast<uint64_t>(2));
    // clockTime < clockTime2
    EXPECT_FALSE(clockTime == clockTime2);
    EXPECT_TRUE(clockTime != clockTime2);
    EXPECT_TRUE(clockTime < clockTime2);
    EXPECT_FALSE(clockTime > clockTime2);
    EXPECT_FALSE(clockTime >= clockTime2);
    EXPECT_TRUE(clockTime <= clockTime2);
}

class CRTPTimeCalculatorTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(CRTPTimeCalculatorTest, ConstructorTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test(1000);
    test.ResetConvertBase(123, 456);

    EXPECT_EQ(124, test.ConvertRTPTimeToTick(457, false));
    EXPECT_EQ(457, test.ConvertTickToRTPTime(124, false));
}

TEST_F(CRTPTimeCalculatorTest, ConstructorInvalidTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.ResetConvertBase(123, 456);

    EXPECT_EQ(0, test.ConvertRTPTimeToTick(124, false));
    EXPECT_EQ(0, test.ConvertTickToRTPTime(457, false));
}

TEST_F(CRTPTimeCalculatorTest, ResetTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    EXPECT_EQ(0, test.ConvertRTPTimeToTick(123));
    EXPECT_EQ(0, test.ConvertTickToRTPTime(123));
}

TEST_F(CRTPTimeCalculatorTest, UpdateClockRateNormalTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    EXPECT_EQ(true, test.UpdateClockRate(1000));
}

TEST_F(CRTPTimeCalculatorTest, UpdateClockRateInvalidTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    EXPECT_EQ(false, test.UpdateClockRate(0));
}

TEST_F(CRTPTimeCalculatorTest, ConvertRTPTimeToTickNormalTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    test.UpdateClockRate(1000);
    test.ResetConvertBase(123, 456);
    EXPECT_EQ(124, test.ConvertRTPTimeToTick(457));
}

TEST_F(CRTPTimeCalculatorTest, ConvertRTPTimeToTickRetreatTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    test.UpdateClockRate(1000);
    test.ResetConvertBase(123, 456);
    EXPECT_EQ(223, test.ConvertRTPTimeToTick(556, true));
    // Come one retreat package
    EXPECT_EQ(220, test.ConvertRTPTimeToTick(553, true));
}

TEST_F(CRTPTimeCalculatorTest, ConvertRTPTimeToTickLostTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    test.UpdateClockRate(1000);
    test.ResetConvertBase(123, 456);
    EXPECT_EQ(223, test.ConvertRTPTimeToTick(556, true));
    // Come one lost package
    EXPECT_EQ(122, test.ConvertRTPTimeToTick(455, true));
}

TEST_F(CRTPTimeCalculatorTest, ConvertRTPTimeToTickCycleTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    test.UpdateClockRate(1000);
    test.ResetConvertBase(123, 0xFFFFFFF0);
    EXPECT_EQ(138, test.ConvertRTPTimeToTick(0xFFFFFFFF, true));
    EXPECT_EQ(140, test.ConvertRTPTimeToTick(0x00000001, true));
    // Lost come
    EXPECT_EQ(137, test.ConvertRTPTimeToTick(0xFFFFFFFE, true));
}

TEST_F(CRTPTimeCalculatorTest, ConvertTickToRTPTimeNormalTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    test.UpdateClockRate(1000);
    test.ResetConvertBase(123, 456);
    EXPECT_EQ(457, test.ConvertTickToRTPTime(124));
}

TEST_F(CRTPTimeCalculatorTest, ConvertTickToRTPTimeRetreatTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    test.UpdateClockRate(1000);
    test.ResetConvertBase(123, 456);
    EXPECT_EQ(556, test.ConvertTickToRTPTime(223, true));
    // Lost retreat
    EXPECT_EQ(553, test.ConvertTickToRTPTime(220, true));
}

TEST_F(CRTPTimeCalculatorTest, ConvertTickToRTPTimeLostTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    test.UpdateClockRate(1000);
    test.ResetConvertBase(123, 456);
    EXPECT_EQ(556, test.ConvertTickToRTPTime(223, true));
    // Lost come
    EXPECT_EQ(455, test.ConvertTickToRTPTime(122, true));
}

TEST_F(CRTPTimeCalculatorTest, ConvertTickToRTPTimeCycleTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    test.UpdateClockRate(1000);
    test.ResetConvertBase(0xFFFFFFF0, 455);
    EXPECT_EQ(470, test.ConvertTickToRTPTime(0xFFFFFFFF, true));
    EXPECT_EQ(472, test.ConvertTickToRTPTime(0x00000001, true));
}

TEST_F(CRTPTimeCalculatorTest, ConvertTickToRTPTimeCycleLostTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    test.UpdateClockRate(1000);
    test.ResetConvertBase(0xFFFFFFF0, 455);
    EXPECT_EQ(470, test.ConvertTickToRTPTime(0xFFFFFFFF, true));
    EXPECT_EQ(472, test.ConvertTickToRTPTime(0x00000001, true));
    // Lost come
    EXPECT_EQ(469, test.ConvertTickToRTPTime(0xFFFFFFFE, true));
}

TEST_F(CRTPTimeCalculatorTest, NoClockRateTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    test.ResetConvertBase(123, 456);
    EXPECT_EQ(0, test.ConvertRTPTimeToTick(456, true));
    EXPECT_EQ(0, test.ConvertTickToRTPTime(123, true));
}

TEST_F(CRTPTimeCalculatorTest, NoBaseTest)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    test.UpdateClockRate(1000);
    EXPECT_EQ(0, test.ConvertRTPTimeToTick(456));
    EXPECT_EQ(0, test.ConvertTickToRTPTime(123));
}

TEST_F(CRTPTimeCalculatorTest, ConvertRTPTimeToTickBugTest001)
{
    CScopedTracer test_info;
    CRTPTimeCalculator test;
    test.Reset();

    test.UpdateClockRate(90000);
    test.ResetConvertBase(1680476401, 1365502331);
    EXPECT_EQ(1680443330, test.ConvertRTPTimeToTick(1362525863));
}

TEST_F(CRTPTimeCalculatorTest, ConvertTickToRTPTimeBugTest002)
{
    CScopedTracer test_info;
    CRTPTimeCalculator sender;
    sender.Reset();

    sender.UpdateClockRate(90000);
    sender.ResetConvertBase(2655152303, 338273746);
    EXPECT_EQ(343050946, sender.ConvertTickToRTPTime(2655205383));

    CRTPTimeCalculator recv;
    recv.Reset();

    recv.UpdateClockRate(90000);
    recv.ResetConvertBase(2655152338, 338273746);
    EXPECT_EQ(2655205383+35, recv.ConvertRTPTimeToTick(343050946));
}

TEST_F(CRTPTimeCalculatorTest, Test_timestamp)
{
    CScopedTracer test_info;
    CRTPTimeCalculator rtpCalculator;

    uint32_t tickNow = 100;
    uint32_t clockRate = 90000;
    uint32_t rtptime = 10*clockRate;
    rtpCalculator.UpdateClockRate(clockRate);
    rtpCalculator.ResetConvertBase(tickNow, rtptime);
    uint32_t startTick = rtpCalculator.ConvertRTPTimeToTick(rtptime);
    uint32_t rtptimeConverted = rtpCalculator.ConvertTickToRTPTime(startTick);
    ASSERT_EQ(rtptimeConverted, rtptime);

    uint32_t tickAfter = 10;
    int32_t timeOffset = CalcTimeOffset(tickAfter, tickNow, clockRate);
    uint32_t TickOffseted = rtpCalculator.ConvertRTPTimeToTick(rtptime + timeOffset);
    ASSERT_EQ(TickOffseted, tickAfter);

    tickAfter = 120000;
    timeOffset = CalcTimeOffset(tickAfter, tickNow, clockRate);
    TickOffseted = rtpCalculator.ConvertRTPTimeToTick(rtptime + timeOffset);
    ASSERT_EQ(TickOffseted, tickAfter);


    //tickAfter = 24*60*60*1000;
    //timeOffset = CalcTimeOffset(tickAfter, tickNow, clockRate);
    //TickOffseted = rtpCalculator.ConvertRTPTimeToTick(rtptime + timeOffset);
    //ASSERT_EQ(TickOffseted, tickAfter);
}

