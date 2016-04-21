
#include "gtest/gtest.h"
#include "gmock/gmock.h"


#include "share_source_impl.h"
#include "share_source.h"


using namespace testing;


const SourceID g_FakeSourceID = 100;

class CShareSourceTest : public Test
{
public:

    virtual void SetUp()
    {
        m_pShareSource = new CShareSource;
        m_pShareSource->AddRef();
    }
    virtual void TearDown()
    {
        SAFE_RELEASE(m_pShareSource);
    }

protected:

    CShareSource *m_pShareSource;
};

TEST_F(CShareSourceTest, GetSourceUniqueId)
{

    SourceID id = m_pShareSource->GetSourceUniqueId();


    //    ASSERT_TRUE( != );


}

TEST_F(CShareSourceTest, GetSourceType)
{


}

TEST_F(CShareSourceTest, IsShared)
{


}


TEST_F(CShareSourceTest, SetInfo)
{
    ShareSourceInfo info;

    info.sourceId = g_FakeSourceID;
    info.type = SST_APPLICATION;


    m_pShareSource->SetInfo(&info);

    ASSERT_TRUE(m_pShareSource->GetSourceUniqueId() == g_FakeSourceID);
    ASSERT_TRUE(m_pShareSource->GetSourceType() == SST_APPLICATION);



    ASSERT_TRUE(m_pShareSource->GetSourceType() == SST_APPLICATION);

}


TEST_F(CShareSourceTest, GetSourceRect)
{


}






