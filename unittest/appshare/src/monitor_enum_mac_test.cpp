#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "./mac/monitor_enum_mac.h"




using namespace testing;

const int g_nFakeSourceCount = 5;

class FakeMonitorEnumMac : public CMonitorEnumMac
{
public:
    FakeMonitorEnumMac()
    {


        for (int i = 0; i < g_nFakeSourceCount; i++) {
            AddOneMonitor(i, cszPrimaryMonitorName);
        }
    }

    virtual ~FakeMonitorEnumMac()
    {
        Cleanup();
    }

    virtual void Cleanup() { return CMonitorEnumMac::Cleanup(); }
    WBXResult EnumMonitors() { return CMonitorEnumMac::EnumMonitors();}

    CShareSource *AddOneMonitor(CGDirectDisplayID nDisplayId, WBXLPCSTR lpszDeviceName) { return CMonitorEnumMac::AddOneMonitor(nDisplayId, lpszDeviceName); }


};


class CMonitorEnumMacTest : public Test
{
public:

    virtual void SetUp()
    {

    }
    virtual void TearDown()
    {

    }

protected:

    FakeMonitorEnumMac m_MonitorEnum;
};


TEST_F(CMonitorEnumMacTest, Refresh)
{
    WBXInt32 nNumber = 0;
    m_MonitorEnum.Cleanup();


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 0);


    m_MonitorEnum.Refresh();


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber > 0);

}


TEST_F(CMonitorEnumMacTest, AddOneMonitor)
{
    WBXInt32 nNumber = 0;
    m_MonitorEnum.Cleanup();


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 0);

    ASSERT_TRUE(m_MonitorEnum.AddOneMonitor(0,cszPrimaryMonitorName) != WBXNull);


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 1);


    ASSERT_TRUE(m_MonitorEnum.AddOneMonitor(1,cszPrimaryMonitorName) != WBXNull);


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 2);

}


TEST_F(CMonitorEnumMacTest, GetMonitorListByHardCode)
{
    WBXInt32 nNumber = 0;
    m_MonitorEnum.Cleanup();


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 0);


    ASSERT_TRUE(m_MonitorEnum.EnumMonitors() != WBX_FAIL);


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber  >0);

}

TEST_F(CMonitorEnumMacTest, EnumMonitors)
{
    WBXInt32 nNumber = 0;
    m_MonitorEnum.Cleanup();


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 0);


    ASSERT_TRUE(m_MonitorEnum.EnumMonitors() != WBX_FAIL);


    ASSERT_TRUE(m_MonitorEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber > 0);

}







