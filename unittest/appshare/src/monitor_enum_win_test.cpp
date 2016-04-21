#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "./windows/monitor_enum_win.h"



using namespace testing;

const int g_nFakeSourceCount = 5;

class FakeMonitorEnumWin : public CMonitorEnumWin
{
public:
    FakeMonitorEnumWin()
    {
        for (int i = 0; i < g_nFakeSourceCount; i++) {

            WBXRect rc(0, 0, 1024, 768);
            AddOneMonitor(i,&rc, cszPrimaryMonitorName,"");
        }
    }

    virtual ~FakeMonitorEnumWin()
    {
        Cleanup();
    }

    virtual void Cleanup() { return CMonitorEnumWin::Cleanup(); }
    WBXResult EnumMonitor() { return CMonitorEnumWin::EnumMonitor(); }
};


class CMonitorEnumWinTest : public Test
{
public:

    virtual void SetUp()
    {

    }
    virtual void TearDown()
    {

    }

protected:

    FakeMonitorEnumWin m_MonitorEnum;
};


TEST_F(CMonitorEnumWinTest, Refresh)
{
    WBXInt32 nNumber = 0;
    m_MonitorEnum.Cleanup();


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 0);


    m_MonitorEnum.Refresh();


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber > 0);

}


TEST_F(CMonitorEnumWinTest, AddOneMonitor)
{
    WBXInt32 nNumber = 0;
    m_MonitorEnum.Cleanup();


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 0);

    WBXRect rc(0, 0, 1024, 768);

    ASSERT_TRUE(m_MonitorEnum.AddOneMonitor(0,&rc, cszPrimaryMonitorName,"") != WBXNull);


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 1);


    ASSERT_TRUE(m_MonitorEnum.AddOneMonitor(0,&rc, cszPrimaryMonitorName,"") != WBXNull);


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 2);

}

TEST_F(CMonitorEnumWinTest, EnumMonitor)
{
    WBXInt32 nNumber = 0;
    m_MonitorEnum.Cleanup();


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 0);


    ASSERT_TRUE(m_MonitorEnum.EnumMonitor() != WBX_FAIL);


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber > 0);

}




