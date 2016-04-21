#include "gmock/gmock.h"
#include "gtest/gtest.h"
#define private public
#include "testutil.h"

#include "seqdb.h"

class CSequenceDBTEST : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }
};

TEST_F(CSequenceDBTEST, CSequenceDB_normal_case)
{
    CScopedTracer test_info;
    CSequenceDB seqDB;
    seqDB.AddSequence(835);
    seqDB.AddSequence(836);
    seqDB.AddSequence(837);
    seqDB.AddSequence(838);

    EXPECT_TRUE(seqDB.IsSequenceExist(835));
    EXPECT_TRUE(seqDB.IsSequenceExist(836));
    EXPECT_TRUE(seqDB.IsSequenceExist(837));
    EXPECT_TRUE(seqDB.IsSequenceExist(838));
}

TEST_F(CSequenceDBTEST, CSequenceDB_interleave_and_disorder)
{
    CScopedTracer test_info;
    CSequenceDB seqDB;
    seqDB.AddSequence(835);
    seqDB.AddSequence(836);
    seqDB.AddSequence(839);
    seqDB.AddSequence(838);
    seqDB.AddSequence(873);
    seqDB.AddSequence(857);

    EXPECT_TRUE(seqDB.IsSequenceExist(835));
    EXPECT_TRUE(seqDB.IsSequenceExist(836));
    EXPECT_FALSE(seqDB.IsSequenceExist(837));
    EXPECT_TRUE(seqDB.IsSequenceExist(838));
    EXPECT_TRUE(seqDB.IsSequenceExist(839));
    EXPECT_TRUE(seqDB.IsSequenceExist(857));
    EXPECT_TRUE(seqDB.IsSequenceExist(873));
    EXPECT_FALSE(seqDB.IsSequenceExist(840));
    EXPECT_FALSE(seqDB.IsSequenceExist(856));
    EXPECT_FALSE(seqDB.IsSequenceExist(869));
    EXPECT_FALSE(seqDB.IsSequenceExist(899));
    EXPECT_FALSE(seqDB.IsSequenceExist(1));
    EXPECT_FALSE(seqDB.IsSequenceExist(32768));
}

TEST_F(CSequenceDBTEST, CSequenceDB_border)
{
    CScopedTracer test_info;
    CSequenceDB seqDB;
    seqDB.AddSequence(835);
    seqDB.AddSequence(836);
    seqDB.AddSequence(837);
    seqDB.AddSequence(838);
    seqDB.AddSequence(840);
    seqDB.AddSequence(841);
    seqDB.AddSequence(835+SEQDB_BUFFER_SIZE-8);
    seqDB.AddSequence(835+SEQDB_BUFFER_SIZE-7);
    seqDB.AddSequence(835+SEQDB_BUFFER_SIZE-5);
    seqDB.AddSequence(835+SEQDB_BUFFER_SIZE-3);
    seqDB.AddSequence(835+SEQDB_BUFFER_SIZE-2);
    seqDB.AddSequence(835+SEQDB_BUFFER_SIZE-4);
    seqDB.AddSequence(835+SEQDB_BUFFER_SIZE+1);
    seqDB.AddSequence(835+SEQDB_BUFFER_SIZE+2);
    seqDB.AddSequence(835+SEQDB_BUFFER_SIZE);
    seqDB.AddSequence(835+SEQDB_BUFFER_SIZE-1);

    EXPECT_FALSE(seqDB.IsSequenceExist(835));
    EXPECT_FALSE(seqDB.IsSequenceExist(836));
    EXPECT_FALSE(seqDB.IsSequenceExist(837));
    EXPECT_TRUE(seqDB.IsSequenceExist(838));
    EXPECT_FALSE(seqDB.IsSequenceExist(839));
    EXPECT_TRUE(seqDB.IsSequenceExist(840));
    EXPECT_TRUE(seqDB.IsSequenceExist(841));
    EXPECT_FALSE(seqDB.IsSequenceExist(842));

    EXPECT_TRUE(seqDB.IsSequenceExist(835+SEQDB_BUFFER_SIZE-8));
    EXPECT_TRUE(seqDB.IsSequenceExist(835+SEQDB_BUFFER_SIZE-7));
    EXPECT_FALSE(seqDB.IsSequenceExist(835+SEQDB_BUFFER_SIZE-6));
    EXPECT_TRUE(seqDB.IsSequenceExist(835+SEQDB_BUFFER_SIZE-5));
    EXPECT_TRUE(seqDB.IsSequenceExist(835+SEQDB_BUFFER_SIZE-4));
    EXPECT_TRUE(seqDB.IsSequenceExist(835+SEQDB_BUFFER_SIZE-3));
    EXPECT_TRUE(seqDB.IsSequenceExist(835+SEQDB_BUFFER_SIZE-2));
    EXPECT_TRUE(seqDB.IsSequenceExist(835+SEQDB_BUFFER_SIZE-1));
    EXPECT_TRUE(seqDB.IsSequenceExist(835+SEQDB_BUFFER_SIZE));
    EXPECT_TRUE(seqDB.IsSequenceExist(835+SEQDB_BUFFER_SIZE+1));
    EXPECT_TRUE(seqDB.IsSequenceExist(835+SEQDB_BUFFER_SIZE+2));
    EXPECT_FALSE(seqDB.IsSequenceExist(835+SEQDB_BUFFER_SIZE+3));
    EXPECT_FALSE(seqDB.IsSequenceExist(835+SEQDB_BUFFER_SIZE+4));
    EXPECT_FALSE(seqDB.IsSequenceExist(835+SEQDB_BUFFER_SIZE+5));
}

TEST_F(CSequenceDBTEST, CSequenceDB_sequence_rollover)
{
    CScopedTracer test_info;
    CSequenceDB seqDB;
    seqDB.AddSequence(65531);
    seqDB.AddSequence(65532);
    seqDB.AddSequence(65535);
    seqDB.AddSequence(65533);
    seqDB.AddSequence(4);
    seqDB.AddSequence(2);
    seqDB.AddSequence(5);
    seqDB.AddSequence(0);
    seqDB.AddSequence(6);
    seqDB.AddSequence(7);

    EXPECT_FALSE(seqDB.IsSequenceExist(6666));
    EXPECT_FALSE(seqDB.IsSequenceExist(65430));
    EXPECT_FALSE(seqDB.IsSequenceExist(65528));
    EXPECT_FALSE(seqDB.IsSequenceExist(65530));
    EXPECT_TRUE(seqDB.IsSequenceExist(65531));
    EXPECT_TRUE(seqDB.IsSequenceExist(65532));
    EXPECT_TRUE(seqDB.IsSequenceExist(65533));
    EXPECT_FALSE(seqDB.IsSequenceExist(65534));
    EXPECT_TRUE(seqDB.IsSequenceExist(65535));
    EXPECT_TRUE(seqDB.IsSequenceExist(0));
    EXPECT_FALSE(seqDB.IsSequenceExist(1));
    EXPECT_TRUE(seqDB.IsSequenceExist(2));
    EXPECT_FALSE(seqDB.IsSequenceExist(3));
    EXPECT_TRUE(seqDB.IsSequenceExist(4));
    EXPECT_TRUE(seqDB.IsSequenceExist(5));
    EXPECT_TRUE(seqDB.IsSequenceExist(6));
    EXPECT_TRUE(seqDB.IsSequenceExist(7));
    EXPECT_FALSE(seqDB.IsSequenceExist(8));
    EXPECT_FALSE(seqDB.IsSequenceExist(9));
    EXPECT_FALSE(seqDB.IsSequenceExist(44));
    EXPECT_FALSE(seqDB.IsSequenceExist(1006));
    EXPECT_FALSE(seqDB.IsSequenceExist(61535));
}
