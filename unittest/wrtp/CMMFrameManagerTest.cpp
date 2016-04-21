#include "gmock/gmock.h"
#include "gtest/gtest.h"
#define private public
#include "mmframemgr.h"
#include "testutil.h"

using namespace wrtp;

class CMMFrameManagerTest : public ::testing::Test
{
public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

public:
};

TEST_F(CMMFrameManagerTest, Test_GetFragments_and_FreeFragments)
{
    CScopedTracer test_info;
    CMMFrameManager mgr;
    FragmentContainer vec;
    mgr.GetFragments(15, vec, 1400, nullptr);
    EXPECT_EQ(vec.size(), 15);
    for (uint32_t i = 0; i < vec.size(); ++i) {
        EXPECT_TRUE(vec[i]->GetBufferSize() >= 1400);
    }
    vec.clear();
    EXPECT_EQ(vec.size(), 0);
}

TEST_F(CMMFrameManagerTest, Test_GetFrame_FreeFrame)
{
    CScopedTracer test_info;
    CMMFrameManager mgr;
    CSendFramePtr frame = mgr.GetFrame(nullptr);
    EXPECT_TRUE(!!frame);
}
