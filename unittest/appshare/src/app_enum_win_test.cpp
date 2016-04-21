
#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "./windows/app_enum_win.h"



using namespace testing;

const int g_nFakeSourceCount = 5;

class FakeAppEnumWin : public CAppEnumWin
{
public:
    FakeAppEnumWin()
    {
        for (int i = 0; i < g_nFakeSourceCount; i++) {
            AddApp(i);
        }
    }

    virtual ~FakeAppEnumWin()
    {
        Cleanup();
    }

    virtual void Cleanup() { return CAppEnumWin::Cleanup(); }
    WBXResult EnumRunningApplications() { return CAppEnumWin::EnumRunningApplications(); }
    WBXBool IsApplicationEnumed(WBXDWord dwProcessId) { return CAppEnumWin::IsApplicationEnumed(dwProcessId); }
};

class CAppEnumWinTest : public Test
{
public:

    virtual void SetUp()
    {

    }
    virtual void TearDown()
    {

    }

protected:

    FakeAppEnumWin m_AppEnum;
};


TEST_F(CAppEnumWinTest, Cleanup)
{
    WBXInt32 nNumber = 0;
    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == g_nFakeSourceCount);

    m_AppEnum.Cleanup();


    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 0);
}


TEST_F(CAppEnumWinTest, Refresh)
{
    WBXInt32 nNumber = 0;
    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == g_nFakeSourceCount);

    m_AppEnum.Cleanup();


    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 0);

    ASSERT_FALSE(m_AppEnum.IsApplicationEnumed(GetCurrentProcessId()));


    ASSERT_TRUE(m_AppEnum.EnumRunningApplications() != WBX_FAIL);


    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber > 0);

    //  WBXBool bTmp = m_AppEnum.IsApplicationEnumed(GetCurrentProcessId()) ;

    //  ASSERT_TRUE(m_AppEnum.IsApplicationEnumed(GetCurrentProcessId()));

}


TEST_F(CAppEnumWinTest, EnumRunningApplications)
{
    WBXInt32 nNumber = 0;
    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == g_nFakeSourceCount);

    m_AppEnum.Cleanup();


    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 0);

    ASSERT_FALSE(m_AppEnum.IsApplicationEnumed(GetCurrentProcessId()));


    ASSERT_TRUE(m_AppEnum.EnumRunningApplications() != WBX_FAIL);


    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber > 0);


    //  WBXBool bTmp = m_AppEnum.IsApplicationEnumed(GetCurrentProcessId()) ;

    //  ASSERT_TRUE(bTmp != 0);

}




