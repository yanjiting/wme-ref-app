#include "gtest/gtest.h"
#include "CmTimeValue.h"
#include <time.h>
#include "SystemCapacity.h"

TEST(CCmTimeValue,CCmTimeValue)
{
	CCmTimeValue tm1;
	EXPECT_TRUE((0 == tm1.GetSec()) && (0 == tm1.GetUsec()));
}

TEST(CCmTimeValue,CCmTimeValueOnlyWithSecond)
{
	time_t nowTime;
	time(&nowTime);
	CCmTimeValue tm1((long)nowTime);
	EXPECT_TRUE((nowTime == tm1.GetSec()) && (0 == tm1.GetUsec()));
}

TEST(CCmTimeValue,CCmTimeValueOnlyWithDoubleSecond)
{
	double tt = 1375684858.999999;
	CCmTimeValue tm1(tt);
	EXPECT_TRUE((1375684858 == tm1.GetSec()) && (999999 == tm1.GetUsec()));
}
TEST(CCmTimeValue,CCmTimeValueWithSecondAndMicroSecond)
{
	time_t nowTime;
	time(&nowTime);
	CCmTimeValue tm1((long)nowTime,100);
	EXPECT_TRUE((nowTime == tm1.GetSec()) && (100 == tm1.GetUsec()));
}
TEST(CCmTimeValue,CCmTimeValueWithTimeVal)
{
	timeval tv;
	tv.tv_sec = 1375684858;
	tv.tv_usec = 100;
	CCmTimeValue tm1(tv);
	EXPECT_TRUE((1375684858 == tm1.GetSec()) && (100 == tm1.GetUsec()));
}
TEST(CCmTimeValue,CCmTimeValueSetAndGetTotalMsecond)
{
	CCmTimeValue tm1;
	long lMsecond = 1375684858;
	tm1.SetByTotalMsec(lMsecond);
	EXPECT_TRUE((1375684 == tm1.GetSec()) && (858000 == tm1.GetUsec()));
	long lRet = tm1.GetTotalInMsec();
	EXPECT_TRUE(1375684858 == lRet);
}
TEST(CCmTimeValue,CCmTimeValueOperationAddAndUsecLessThan1000000L)
{
	/*srand((unsigned)time(NULL));
	CCmTimeValue tm1((long)rand(),(long)rand());1000000L
	CCmTimeValue tm2((long)rand(),(long)rand());*/
	CCmTimeValue tm1(375684858,999);
	CCmTimeValue tm2(375684858,999000);
	CCmTimeValue tm3 = tm2;
	tm2 += tm1;
	EXPECT_TRUE((tm2.GetSec() == (tm3.GetSec() + tm1.GetSec())) && (tm2.GetUsec() == (tm3.GetUsec() + tm1.GetUsec())));
	tm2 = tm1 + tm3;
	EXPECT_TRUE((tm2.GetSec() == (tm3.GetSec() + tm1.GetSec())) && (tm2.GetUsec() == (tm3.GetUsec() + tm1.GetUsec())));
}
TEST(CCmTimeValue,CCmTimeValueOperationAddAndUsecMoreThan1000000L)
{
	CCmTimeValue tm1(375684858,999);
	CCmTimeValue tm2(375684858,999001);
	CCmTimeValue tm3 = tm2;
	tm2 += tm1;
	EXPECT_TRUE((tm2.GetSec() == (tm3.GetSec() + tm1.GetSec() + 1)) && (tm2.GetUsec() == (tm3.GetUsec() + tm1.GetUsec() - CM_ONE_SECOND_IN_USECS)));
	tm2 = tm1 + tm3;
	EXPECT_TRUE((tm2.GetSec() == (tm3.GetSec() + tm1.GetSec() + 1)) && (tm2.GetUsec() == (tm3.GetUsec() + tm1.GetUsec() - CM_ONE_SECOND_IN_USECS)));
}
TEST(CCmTimeValue,CCmTimeValueOperationAddminusUsecMoreThan0L)
{
	CCmTimeValue tm1(1375684858,999);
	CCmTimeValue tm2(1375684858,999001);
	CCmTimeValue tm3 = tm2;
	tm3 = tm2;
	tm2 -= tm1;
	EXPECT_TRUE((tm2.GetSec() == (tm3.GetSec() - tm1.GetSec())) && (tm2.GetUsec() == (tm3.GetUsec() - tm1.GetUsec())));
	tm2 = tm3 - tm1;
	EXPECT_TRUE((tm2.GetSec() == (tm3.GetSec() - tm1.GetSec())) && (tm2.GetUsec() == (tm3.GetUsec() - tm1.GetUsec())));
}
TEST(CCmTimeValue,CCmTimeValueOperationMinusAndUsecLessThan0L)
{
	CCmTimeValue tm1(1375684858,999);
	CCmTimeValue tm2(1375684856,999001);
	CCmTimeValue tm3 = tm1;
	tm1 -= tm2;
	EXPECT_TRUE((tm1.GetSec() == (tm3.GetSec() - tm2.GetSec() - 1)) && (tm1.GetUsec() == (tm3.GetUsec() - tm2.GetUsec() + CM_ONE_SECOND_IN_USECS)));
	tm1 = tm3 - tm2;
	EXPECT_TRUE((tm1.GetSec() == (tm3.GetSec() - tm2.GetSec() - 1)) && (tm1.GetUsec() == (tm3.GetUsec() - tm2.GetUsec() + CM_ONE_SECOND_IN_USECS)));
}
TEST(CCmTimeValue,CCmTimeValueCompareOfBiggerThanAndSmallerThanWhenSecondIsEqual)
{
	CCmTimeValue tm1(1375684858,288);
	CCmTimeValue tm2(1375684858,200);
	EXPECT_TRUE(tm1 > tm2);
	EXPECT_TRUE(tm1 >= tm2);
	EXPECT_TRUE(tm2 < tm1);
	EXPECT_TRUE(tm2 <= tm1);
}
TEST(CCmTimeValue,CCmTimeValueCompareOfBiggerThanAndSmallerThanFailWhenSecondIsEqual)
{
	CCmTimeValue tm1(1375684858,188);
	CCmTimeValue tm2(1375684858,200);
	EXPECT_FALSE(tm1 > tm2);
	EXPECT_FALSE(tm1 >= tm2);
	EXPECT_FALSE(tm2 < tm1);
	EXPECT_FALSE(tm2 <= tm1);
}
TEST(CCmTimeValue,CCmTimeValueCompareOfBiggerThanAndSmallerThanWhenSecondIsNotEqual)
{
	CCmTimeValue tm1(1375684859,188);
	CCmTimeValue tm2(1375684858,200);
	EXPECT_TRUE(tm1 > tm2);
	EXPECT_TRUE(tm1 >= tm2);
	EXPECT_TRUE(tm2 < tm1);
	EXPECT_TRUE(tm2 <= tm1);
}
TEST(CCmTimeValue,CCmTimeValueCompareOfBiggerThanAndSmallerThanFailWhenSecondIsNotEqual)
{
	CCmTimeValue tm1(1375684859,388);
	CCmTimeValue tm2(1375684860,200);
	EXPECT_FALSE(tm1 > tm2);
	EXPECT_FALSE(tm1 >= tm2);
	EXPECT_FALSE(tm2 < tm1);
	EXPECT_FALSE(tm2 <= tm1);
}
TEST(CCmTimeValue,CCmTimeValueCompareOfBiggerThanAndSmallerThanWhenValueIsEqual)
{
	CCmTimeValue tm1(1375684859,200);
	CCmTimeValue tm2(1375684859,200);
	EXPECT_TRUE(tm1 >= tm2);
	EXPECT_TRUE(tm2 <= tm1);
}
TEST(CCmTimeValue,CCmTimeValueCompareWhenValueIsEqual)
{
	CCmTimeValue tm1(1375684859,200);
	CCmTimeValue tm2(1375684859,200);
	EXPECT_TRUE(tm1 == tm2);
	EXPECT_FALSE(tm2 != tm1);
}
TEST(CCmTimeValue,CCmTimeValueCompareWhenValueIsNotEqual)
{
	CCmTimeValue tm1(1375684858,200);
	CCmTimeValue tm2(1375684859,200);
	EXPECT_FALSE(tm1 == tm2);
	EXPECT_TRUE(tm2 != tm1);
}
TEST(CCmTimeValue,CCmTimeValueGetTimeOfDay)
{
	CCmTimeValue tm1 = CCmTimeValue::GetTimeOfDay();
	EXPECT_TRUE((0 != tm1.GetSec()) && (0 != tm1.GetUsec()));
}
TEST(CCmTimeValue,CCmTimeValueGetTvzero)
{
	CCmTimeValue tm1(111,11);
	tm1 = CCmTimeValue::get_tvZero();
	EXPECT_TRUE((0 == tm1.GetSec()) && (0 == tm1.GetUsec()));
}

TEST(SystemCapacity, TestGetMemoryUsage)
{
    float memoryUsage = 0;
    unsigned int memoryUsed = 0, memorySize = 0;
    get_memory_usage(memoryUsage, memoryUsed, memorySize);
    EXPECT_TRUE(memoryUsed < memorySize);
    EXPECT_TRUE(memoryUsage < 100);
}

#include "CmHttpUrl.h"

TEST(CCmHttpUrl, CmUri) {
    CCmUri aUri;
    CCmString sHost;
    uint16_t nPort;
    CCmString sForTrim("   ");
    TrimString(sForTrim, CCmIsSpace());
    EXPECT_TRUE(sForTrim.empty());
    TrimString(sForTrim = "  L R  ", CCmIsSpace());
    EXPECT_TRUE(sForTrim == "L R");
    TrimString(sForTrim = "  L", CCmIsSpace());
    EXPECT_TRUE(sForTrim == "L");
    TrimString(sForTrim = "R   ", CCmIsSpace());
    EXPECT_TRUE(sForTrim == "R");
    
    EXPECT_TRUE(aUri.ParseHostnamePort("10.224.166.110:8000", sHost, nPort));
    EXPECT_TRUE(sHost == "10.224.166.110" && nPort == 8000);
    EXPECT_TRUE(aUri.ParseHostnamePort("10.224.166.110:", sHost, nPort));
    EXPECT_TRUE(sHost == "10.224.166.110" && nPort == 0);
    EXPECT_TRUE(aUri.ParseHostnamePort(" 10.224.166.110 ", sHost, nPort));
    EXPECT_TRUE(sHost == "10.224.166.110" && nPort == 0);
    EXPECT_TRUE(aUri.ParseHostnamePort(" ::1 ", sHost, nPort));
    EXPECT_TRUE(sHost == "::1" && nPort == 0);
    EXPECT_TRUE(aUri.ParseHostnamePort(" [ ::1 ] : 90", sHost, nPort));
    EXPECT_TRUE(sHost == "::1" && nPort == 90);
    EXPECT_TRUE(aUri.ParseHostnamePort(" [ ::1 ] : ", sHost, nPort));
    EXPECT_TRUE(sHost == "::1" && nPort == 0);
    EXPECT_FALSE(aUri.ParseHostnamePort(" [ ::1 ] : xxx", sHost, nPort));
    EXPECT_TRUE(sHost == "::1" && nPort == 0);
    EXPECT_FALSE(aUri.ParseHostnamePort(" [ ::1  : 9000", sHost, nPort));
    EXPECT_TRUE(sHost == "::1" && nPort == 0);
    //This is wrong format but we don't want differentiate that.
    EXPECT_TRUE(aUri.ParseHostnamePort("  ::1  : 9000", sHost, nPort));
    EXPECT_TRUE(sHost == "::1  : 9000" && nPort == 0);
    
    EXPECT_FALSE(aUri.Parse("中文"));
    EXPECT_FALSE(aUri.Parse("mailto:2@b.c?"));
    EXPECT_TRUE(aUri.Parse("ftp://test:pass@10.224.166.110:8000/test.uri?name=mine#"));
    EXPECT_TRUE(aUri.Parse("Http://test:pass@10.224.166.110:8000/test.uri?name=mine#iD-1"));
    EXPECT_TRUE(aUri.m_sScheme == "http");
    EXPECT_TRUE(aUri.m_sFragment == "id-1");
    EXPECT_TRUE(aUri.m_sHostName == "10.224.166.110");
    EXPECT_TRUE(aUri.m_nPort == 8000);
    EXPECT_TRUE(aUri.m_sUsername == "test");
    EXPECT_TRUE(aUri.m_sPassword == "pass");
    EXPECT_TRUE(aUri.m_sQuery == "name=mine");
    EXPECT_TRUE(aUri.Parse("stun:a.b.c"));
    EXPECT_TRUE(aUri.m_sHostName == "a.b.c");
    EXPECT_TRUE(aUri.m_nPort == 3478);
    EXPECT_TRUE(aUri.Parse(" stun:10.224.166.110:3000"));
    EXPECT_TRUE(aUri.m_sHostName == "10.224.166.110");
    EXPECT_TRUE(aUri.m_nPort == 3000);
    EXPECT_TRUE(aUri.Parse("Https://www.webex.com:/"));
    EXPECT_TRUE(aUri.m_sHostName == "www.webex.com");
    EXPECT_TRUE(aUri.Parse("Https://www.webex.com"));
    EXPECT_TRUE(aUri.m_sHostName == "www.webex.com");
    EXPECT_TRUE(aUri.Parse("file:///c:/"));
    EXPECT_TRUE(aUri.m_sPath == "/c:/");
    EXPECT_TRUE(aUri.m_sHostName.empty());
    EXPECT_TRUE(aUri.Parse("mailto:  "));
    EXPECT_TRUE(aUri.Parse("mailto:  a@b.c"));
    EXPECT_TRUE(aUri.m_sPath == "a@b.c");
    EXPECT_TRUE(aUri.Parse("tel: +86000000 "));
    EXPECT_TRUE(aUri.m_sPath == "+86000000");
}
