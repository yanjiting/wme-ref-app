
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "share_source_enum.h"
#include "share_source_impl.h"


using namespace testing;



class FakeShareSourceEnumerator : public CShareSourceEnumerator
{
public:
    FakeShareSourceEnumerator()
    {
    }

    virtual ~FakeShareSourceEnumerator()
    {
        Cleanup();
    }


    void Cleanup() { return CShareSourceEnumerator::Cleanup(); }

};

// api test
TEST(share_source_enum, CreateShareSourceEnumerator_DestroyShareSourceEnumerator)
{
    IShareSourceEnumerator *pEnumerator = WBXNull;

    //
    pEnumerator = CreateShareSourceEnumerator(SST_APPLICATION);
    ASSERT_TRUE(pEnumerator != WBXNull);
    ASSERT_TRUE(DestroyShareSourceEnumerator(pEnumerator)==WBX_SUCCESS);
    pEnumerator=NULL;

    //
    pEnumerator = CreateShareSourceEnumerator(SST_MONITOR);
    ASSERT_TRUE(pEnumerator != WBXNull);
    ASSERT_TRUE(DestroyShareSourceEnumerator(pEnumerator)==WBX_SUCCESS);
    pEnumerator=NULL;


    //
    pEnumerator = CreateShareSourceEnumerator(SST_CONTENT);
    ASSERT_TRUE(pEnumerator == WBXNull);
    //  ASSERT_TRUE(DestroyShareSourceEnumerator(pEnumerator)==WBX_SUCCESS);
    //  pEnumerator=NULL;
}


class CShareSourceEnumeratorTest : public Test
{
public:

    virtual void SetUp()
    {

    }
    virtual void TearDown()
    {

    }

protected:

    FakeShareSourceEnumerator m_Enumerator;
};



TEST_F(CShareSourceEnumeratorTest, GetSource)
{
    IShareSource *pSource = WBXNull;

    ASSERT_TRUE(m_Enumerator.GetSource(-1, &pSource) == WBX_FAIL);
    ASSERT_TRUE(m_Enumerator.GetSource(5, &pSource) == WBX_FAIL);

}

TEST_F(CShareSourceEnumeratorTest, SetFilter)
{

}




