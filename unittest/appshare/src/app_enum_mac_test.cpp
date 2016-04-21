
#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "./mac/app_enum_mac.h"

void LaunchOneApp();

using namespace testing;

const int g_nFakeSourceCount = 5;

class FakeAppEnumMac : public CAppEnumMac
{
public:
    FakeAppEnumMac()
    {
        _LaunchOneApp();
        for (int i = 0; i < g_nFakeSourceCount; i++) {
            AddOneApp(i);
        }
    }

    virtual ~FakeAppEnumMac()
    {
        _ExitOneApp();
    }

    void  Cleanup() { return CAppEnumMac::Cleanup(); }

    WBXResult EnumRunningApplications() { return CAppEnumMac::EnumRunningApplications(); }
    
    void _LaunchOneApp() {
#ifdef MACOS
        LaunchOneApp();
#endif
    }
    
    void _ExitOneApp() {
    }

};

class CAppEnumMacTest : public Test
{
public:

    virtual void SetUp()
    {

    }
    virtual void TearDown()
    {

    }

protected:

    FakeAppEnumMac m_AppEnum;
};


TEST_F(CAppEnumMacTest, Cleanup)
{
    WBXInt32 nNumber = 0;
    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == g_nFakeSourceCount);

    m_AppEnum.Cleanup();


    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 0);
}


TEST_F(CAppEnumMacTest, Refresh)
{
    WBXInt32 nNumber = 0;
    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == g_nFakeSourceCount);

    m_AppEnum.Cleanup();


    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 0);



    ASSERT_TRUE(m_AppEnum.EnumRunningApplications() != WBX_FAIL);


    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber > 0);




}


TEST_F(CAppEnumMacTest, EnumRunningApplications)
{
    WBXInt32 nNumber = 0;
    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == g_nFakeSourceCount);

    m_AppEnum.Cleanup();


    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber == 0);



    ASSERT_TRUE(m_AppEnum.EnumRunningApplications() != WBX_FAIL);


    ASSERT_TRUE(m_AppEnum.GetNumber(nNumber) == WBX_SUCCESS);

    ASSERT_TRUE(nNumber > 0);

    IShareSource *pSource = WBXNull;

    m_AppEnum.GetSource(0, &pSource);

    if (pSource) {
        char szName[MAX_SHARE_SOURCE_NAME] = {0};


        ASSERT_TRUE(!pSource->GetName((WBXLPTSTR)szName, MAX_SHARE_SOURCE_NAME));
    }

}




