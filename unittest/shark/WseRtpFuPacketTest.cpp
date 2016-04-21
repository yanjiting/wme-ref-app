
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WseRtpFuPacket.h"
#define ITERATIONNUM 5  //iterate like 10000 times
#define VARVALUE 999  // and every time the buffer length is like [ITERATIONNUM*MaxFuSize - VARVALUE, ITERATIONNUM*MaxFuSize + VARVALUE]
#define MAXFUSIZE 1000


class WseRtpFuPacketTest : public testing::Test
{
public:
	WseRtpFuPacketTest()
	{
		FragmentOperator = CreateVideoFragmentOperator(shark::videoCodecH264SVC);
		fragments = new wrtp::FragmentBuffer[ITERATIONNUM*2];
		for (int i=0; i<ITERATIONNUM*2; i++)
		{
			fragments[i].pBuffer = new unsigned char[MAXFUSIZE];
			fragments[i].fragmentLength = 0;
			fragments[i].bufferLength = MAXFUSIZE;
		}
	}

	virtual ~WseRtpFuPacketTest()
	{
		for (int i=0; i<ITERATIONNUM*2; i++)
		{
			delete fragments[i].pBuffer;
			fragments[i].fragmentLength =0;
			fragments[i].bufferLength = 0;
		}
		delete fragments;
		delete FragmentOperator;
	}

	virtual void SetUp()
	{

	}

	virtual void TearDown()
	{

	}

public:
	wrtp::IFragmentOperator *FragmentOperator;
	wrtp::FragmentBuffer	*fragments;

};

//this test we will first fragment one data array then defragment the resulting rtp payloads to check whether they are the same
//
TEST_F(WseRtpFuPacketTest, FragmentPlusDefragment)
{
	unsigned int i;
	int j;
	unsigned int count;
	unsigned char* recondata = NULL;
	unsigned char* data = NULL;
	unsigned int length, length1;
	srand(time(NULL));
	for (i=0; i<ITERATIONNUM; i++)
	{
		for (j=(i+1)*MAXFUSIZE-VARVALUE; j<(i+1)*MAXFUSIZE+VARVALUE; j++)
		{
			length = j;
			data = new unsigned char[length]; // random value?
			for (int k=0; k<length; k++)
			{
				data[k] = rand()%256;
			}
			recondata = new unsigned char[length*2];
			length1 = length*2;
			count = FragmentOperator->GetFragmentCount(length, MAXFUSIZE);
			EXPECT_EQ(0, FragmentOperator->Fragment(data, length, fragments, count));
			EXPECT_EQ(0, FragmentOperator->Defragment(fragments, count, recondata, length1));
			EXPECT_EQ(0, length-length1);
			EXPECT_EQ(0, memcmp(data, recondata, length));
			delete data;
			delete recondata;
		}
	}
}


TEST_F(WseRtpFuPacketTest, IsStartIsEndTest)
{
	unsigned int i;
	int j;
	unsigned int count;
	unsigned char* data = NULL;
	unsigned int length, length1;
	srand(time(NULL));
	for (i=0; i<ITERATIONNUM; i++)
	{
		for (j=(i+1)*MAXFUSIZE-VARVALUE; j<(i+1)*MAXFUSIZE+VARVALUE; j++)
		{
			length = j;
			data = new unsigned char[length]; // random value?
			for (int k=0; k<length; k++)
			{
				data[k] = rand()%256;
			}
			count = FragmentOperator->GetFragmentCount(length, MAXFUSIZE);
			EXPECT_EQ(0, FragmentOperator->Fragment(data, length, fragments, count));
			if (count>1)
			{
				EXPECT_EQ(true, FragmentOperator->IsStartFu(fragments[0].pBuffer, fragments[0].fragmentLength));
				EXPECT_EQ(true, FragmentOperator->IsEndFu(fragments[count-1].pBuffer, fragments[count-1].fragmentLength));
			}
			delete data;
		}
	}
}