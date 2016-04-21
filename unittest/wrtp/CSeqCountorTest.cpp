#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "testutil.h"

using ::testing::_;
using ::testing::Return;
using ::testing::AtLeast;
using ::testing::NiceMock;
using ::testing::StrictMock;
using namespace wrtp;
using namespace std;

#include <string>
#include "rtcphandler.h"

class CSeqCountorTest : public testing::Test
{
public:
    CSeqCountorTest() {}

protected:
    //will be called before running every case
    virtual void SetUp()
    {
    }
    //will be called after running every case
    virtual void TearDown()
    {
    }

    //will be called before running the first case
    static void SetUpTestCase()
    {
    }

    //will be called after running the last case
    static void TearDownTestCase()
    {
    }
private:

};

TEST_F(CSeqCountorTest, Test_CSeqCountor_Normal_01)
{
    CScopedTracer test_info;
    CSeqCountor seqCountor;
    for (int i=0; i< 1024; ++i) {
        seqCountor.AddSequence(i);
    }

    for (int i=0; i< 1024; ++i) {
        EXPECT_TRUE(seqCountor.IsSequenceExist(i));
    }

    EXPECT_FALSE(seqCountor.IsSequenceExist(1024));
    EXPECT_FALSE(seqCountor.IsSequenceExist(2000));
    EXPECT_FALSE(seqCountor.IsSequenceExist(65500));
    EXPECT_FALSE(seqCountor.IsSequenceExist(65535));

    EXPECT_EQ(0, seqCountor.GetLostSeqCount());
}

TEST_F(CSeqCountorTest, Test_CSeqCountor_Lost_02)
{
    CScopedTracer test_info;
    CSeqCountor seqCountor;
    for (int i=0; i< 1024; ++i) {
        if (0 == i%2) {
            continue;
        }
        seqCountor.AddSequence(i);
    }

    for (int i=0; i< 1024; ++i) {
        if (0 == i%2) {
            EXPECT_FALSE(seqCountor.IsSequenceExist(i));
        } else {
            EXPECT_TRUE(seqCountor.IsSequenceExist(i));
        }
    }

    EXPECT_FALSE(seqCountor.IsSequenceExist(1024));
    EXPECT_FALSE(seqCountor.IsSequenceExist(2000));
    EXPECT_FALSE(seqCountor.IsSequenceExist(65500));
    EXPECT_FALSE(seqCountor.IsSequenceExist(65535));

    EXPECT_EQ(511, seqCountor.GetLostSeqCount());
}

TEST_F(CSeqCountorTest, Test_CSeqCountor_Disorder03)
{
    CScopedTracer test_info;
    CSeqCountor seqCountor;
    seqCountor.AddSequence(0);
    seqCountor.AddSequence(1023);
    seqCountor.AddSequence(1);
    seqCountor.AddSequence(5);
    seqCountor.AddSequence(88);
    seqCountor.AddSequence(50);
    seqCountor.AddSequence(66);
    seqCountor.AddSequence(73);
    seqCountor.AddSequence(512);
    seqCountor.AddSequence(999);
    seqCountor.AddSequence(435);
    seqCountor.AddSequence(107);
    seqCountor.AddSequence(888);
    seqCountor.AddSequence(327);
    seqCountor.AddSequence(191);
    seqCountor.AddSequence(234);
    seqCountor.AddSequence(398);
    seqCountor.AddSequence(824);
    seqCountor.AddSequence(763);
    seqCountor.AddSequence(777);

    EXPECT_TRUE(seqCountor.IsSequenceExist(435));
    EXPECT_TRUE(seqCountor.IsSequenceExist(107));
    EXPECT_TRUE(seqCountor.IsSequenceExist(888));
    EXPECT_TRUE(seqCountor.IsSequenceExist(327));
    EXPECT_TRUE(seqCountor.IsSequenceExist(191));
    EXPECT_TRUE(seqCountor.IsSequenceExist(234));
    EXPECT_TRUE(seqCountor.IsSequenceExist(398));
    EXPECT_TRUE(seqCountor.IsSequenceExist(824));
    EXPECT_TRUE(seqCountor.IsSequenceExist(763));
    EXPECT_TRUE(seqCountor.IsSequenceExist(777));
    EXPECT_TRUE(seqCountor.IsSequenceExist(0));
    EXPECT_TRUE(seqCountor.IsSequenceExist(1023));
    EXPECT_TRUE(seqCountor.IsSequenceExist(1));
    EXPECT_TRUE(seqCountor.IsSequenceExist(5));
    EXPECT_TRUE(seqCountor.IsSequenceExist(88));
    EXPECT_TRUE(seqCountor.IsSequenceExist(50));
    EXPECT_TRUE(seqCountor.IsSequenceExist(66));
    EXPECT_TRUE(seqCountor.IsSequenceExist(73));
    EXPECT_TRUE(seqCountor.IsSequenceExist(512));
    EXPECT_TRUE(seqCountor.IsSequenceExist(999));

    EXPECT_FALSE(seqCountor.IsSequenceExist(2));
    EXPECT_FALSE(seqCountor.IsSequenceExist(3));
    EXPECT_FALSE(seqCountor.IsSequenceExist(998));
    EXPECT_FALSE(seqCountor.IsSequenceExist(1022));
    EXPECT_FALSE(seqCountor.IsSequenceExist(4));
    EXPECT_FALSE(seqCountor.IsSequenceExist(399));
    EXPECT_FALSE(seqCountor.IsSequenceExist(233));
    EXPECT_FALSE(seqCountor.IsSequenceExist(87));
    EXPECT_FALSE(seqCountor.IsSequenceExist(800));
    EXPECT_FALSE(seqCountor.IsSequenceExist(500));
    EXPECT_FALSE(seqCountor.IsSequenceExist(123));
    EXPECT_FALSE(seqCountor.IsSequenceExist(1024));
    EXPECT_FALSE(seqCountor.IsSequenceExist(2000));
    EXPECT_FALSE(seqCountor.IsSequenceExist(65500));
    EXPECT_FALSE(seqCountor.IsSequenceExist(65535));

    EXPECT_EQ(1004, seqCountor.GetLostSeqCount());
}

TEST_F(CSeqCountorTest, Test_CSeqCountor_Border_05)
{
    CScopedTracer test_info;
    CSeqCountor seqCountor;
    seqCountor.AddSequence(1000);
    seqCountor.AddSequence(1006);
    seqCountor.AddSequence(1001);
    seqCountor.AddSequence(1002);
    seqCountor.AddSequence(1004);
    seqCountor.AddSequence(1003);
    seqCountor.AddSequence(1005);
    seqCountor.AddSequence(1007);
    seqCountor.AddSequence(1000 + RECEIVED_SEQ_MAP_SIZE-5);
    seqCountor.AddSequence(1000 + RECEIVED_SEQ_MAP_SIZE-3);
    seqCountor.AddSequence(1000 + RECEIVED_SEQ_MAP_SIZE-1);
    seqCountor.AddSequence(1000 + RECEIVED_SEQ_MAP_SIZE);
    seqCountor.AddSequence(1000 + RECEIVED_SEQ_MAP_SIZE+1);
    seqCountor.AddSequence(1000 + RECEIVED_SEQ_MAP_SIZE+2);

    EXPECT_FALSE(seqCountor.IsSequenceExist(1000));
    EXPECT_FALSE(seqCountor.IsSequenceExist(1001));
    EXPECT_FALSE(seqCountor.IsSequenceExist(1002));
    EXPECT_FALSE(seqCountor.IsSequenceExist(1000+RECEIVED_SEQ_MAP_SIZE-4));
    EXPECT_FALSE(seqCountor.IsSequenceExist(1000+RECEIVED_SEQ_MAP_SIZE+3));

    EXPECT_TRUE(seqCountor.IsSequenceExist(1003));
    EXPECT_TRUE(seqCountor.IsSequenceExist(1004));
    EXPECT_TRUE(seqCountor.IsSequenceExist(1005));
    EXPECT_TRUE(seqCountor.IsSequenceExist(1006));
    EXPECT_TRUE(seqCountor.IsSequenceExist(1007));

    EXPECT_TRUE(seqCountor.IsSequenceExist(1000+RECEIVED_SEQ_MAP_SIZE-5));
    EXPECT_TRUE(seqCountor.IsSequenceExist(1000+RECEIVED_SEQ_MAP_SIZE-3));
    EXPECT_TRUE(seqCountor.IsSequenceExist(1000+RECEIVED_SEQ_MAP_SIZE-1));
    EXPECT_TRUE(seqCountor.IsSequenceExist(1000+RECEIVED_SEQ_MAP_SIZE));
    EXPECT_TRUE(seqCountor.IsSequenceExist(1000+RECEIVED_SEQ_MAP_SIZE+1));
    EXPECT_TRUE(seqCountor.IsSequenceExist(1000+RECEIVED_SEQ_MAP_SIZE+2));

    EXPECT_EQ(1013, seqCountor.GetLostSeqCount());
}

TEST_F(CSeqCountorTest, Test_CSeqCountor_RollingOver_05)
{
    CScopedTracer test_info;
    CSeqCountor seqCountor;
    seqCountor.AddSequence(65530);
    seqCountor.AddSequence(65531);
    seqCountor.AddSequence(65532);
    seqCountor.AddSequence(0);
    seqCountor.AddSequence(1);
    seqCountor.AddSequence(2);
    seqCountor.AddSequence(7);
    seqCountor.AddSequence(8);
    seqCountor.AddSequence(9);

    EXPECT_TRUE(seqCountor.IsSequenceExist(65530));
    EXPECT_TRUE(seqCountor.IsSequenceExist(65531));
    EXPECT_TRUE(seqCountor.IsSequenceExist(65532));
    EXPECT_TRUE(seqCountor.IsSequenceExist(0));
    EXPECT_TRUE(seqCountor.IsSequenceExist(1));
    EXPECT_TRUE(seqCountor.IsSequenceExist(2));
    EXPECT_TRUE(seqCountor.IsSequenceExist(7));
    EXPECT_TRUE(seqCountor.IsSequenceExist(8));
    EXPECT_TRUE(seqCountor.IsSequenceExist(9));

    EXPECT_FALSE(seqCountor.IsSequenceExist(65529));
    EXPECT_FALSE(seqCountor.IsSequenceExist(65533));
    EXPECT_FALSE(seqCountor.IsSequenceExist(65534));
    EXPECT_FALSE(seqCountor.IsSequenceExist(65535));
    EXPECT_FALSE(seqCountor.IsSequenceExist(3));
    EXPECT_FALSE(seqCountor.IsSequenceExist(4));
    EXPECT_FALSE(seqCountor.IsSequenceExist(5));
    EXPECT_FALSE(seqCountor.IsSequenceExist(6));
    EXPECT_FALSE(seqCountor.IsSequenceExist(10));



    EXPECT_EQ(7, seqCountor.GetLostSeqCount());
}