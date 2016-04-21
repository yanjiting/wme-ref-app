#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "./mac/share_source_mac.h"
#include "share_source.h"


using namespace testing;



class CShareSourceMacTest : public Test
{
public:

    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }
};

TEST_F(CShareSourceMacTest, GetName)
{

    CShareSourceMac source;

    SHARE_SOURCE_INFO ssi;
    ssi.sourceId = -1;
    ssi.type = SST_APPLICATION;


    source.SetInfo(&ssi);

    char szName[MAX_SHARE_SOURCE_NAME] = {0};


    ASSERT_TRUE(source.GetName((WBXLPTSTR)szName, MAX_SHARE_SOURCE_NAME)!=WBX_SUCCESS);

    //    ssi.sourceId = ;


    //    source.SetInfo(&ssi);

    //    ASSERT_TRUE(source.GetName((WBXLPTSTR)szName, MAX_SHARE_SOURCE_NAME));

}







