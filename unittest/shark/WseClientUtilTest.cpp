#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WseClientUtil.h"
#include "WseDebug.h"

using namespace shark;
class CWseClientUtilTest : public testing::Test
{
public:
	CWseClientUtilTest()
	{

	}

	virtual ~CWseClientUtilTest()
	{
	
	}

	virtual void SetUp()
	{

	}

	virtual void TearDown()
	{

	}

};

static void CheckAdjustTimeStamp(int nFps, int *pSeq, int *pRes, int nSeqLen)
{
	if(!pSeq || !pRes )
		return;

	for(int j=0; j<nSeqLen; j++)
	{
		unsigned long nTimeStamp = pSeq[j];
		unsigned long nAdjustedTimeStamp = 0;
		long ret = AdjustTimestamp(nTimeStamp, nFps*1.0, nAdjustedTimeStamp);

		if(nFps <= 0)
		{
			EXPECT_NE(WSE_S_OK, ret);
		}
		else
		{
			EXPECT_EQ(WSE_S_OK, ret);
		}

		if(WSE_S_OK == ret)
		{
			EXPECT_EQ(nAdjustedTimeStamp, pRes[j]);
		}
	}
}

TEST_F(CWseClientUtilTest, AdjustTimestamp)
{
	int TestSeq_FPS30[] = { 16,  17,  37,  49,  50,  69,  83,  84, 106, 116, 117, 139, 149, 150, 167, 183, 184};
	int TestRes_FPS30[] = {  0,  33,  33,  33,  66,  66,  66, 100, 100, 100, 133, 133, 133, 166, 166, 166, 200};

	int TestSeq_FPS25[] = { 19,  20,  45,  59,  60,  89,  99, 100, 121, 139, 140, 160, 179, 180, 209, 219, 220};
	int TestRes_FPS25[] = {  0,  40,  40,  40,  80,  80,  80, 120, 120, 120, 160, 160, 160, 200, 200, 200, 240};

	int TestSeq_FPS24[] = { 20,  21,  49,  62,  63,  87, 104, 105, 131, 145, 146, 167, 187, 188, 208, 229, 230};
	int TestRes_FPS24[] = {  0,  41,  41,  41,  83,  83,  83, 125, 125, 125, 166, 166, 166, 208, 208, 208, 250};

	int TestSeq_FPS20[] = { 24,  25,  54,  74,  75, 100, 124, 125, 153, 174, 175, 210, 224, 225, 261, 274, 275};
	int TestRes_FPS20[] = {  0,  50,  50,  50, 100, 100, 100, 150, 150, 150, 200, 200, 200, 250, 250, 250, 300};

	int TestSeq_FPS15[] = { 33,  34,  69,  99, 100, 144, 166, 167, 209, 233, 234, 270, 299, 300, 341, 366, 367};
	int TestRes_FPS15[] = {  0,  66,  66,  66, 133, 133, 133, 200, 200, 200, 266, 266, 266, 333, 333, 333, 400};

	int TestSeq_FPS12[] = { 41,  42,  84, 124, 125, 170, 208, 209, 269, 291, 292, 342, 374, 375, 432, 458, 459};
	int TestRes_FPS12[] = {  0,  83,  83,  83, 166, 166, 166, 250, 250, 250, 333, 333, 333, 416, 416, 416, 500};

	int TestSeq_FPS10[] = { 49,  50, 122, 149, 150, 223, 249, 250, 306, 349, 350, 409, 449, 450, 505, 549, 550};
	int TestRes_FPS10[] = {  0, 100, 100, 100, 200, 200, 200, 300, 300, 300, 400, 400, 400, 500, 500, 500, 600};

	int TestSeq_FPS09[] = { 55,  56, 123, 166, 167, 223, 277, 278, 350, 388, 389, 462, 499, 500, 572, 611, 612};
	int TestRes_FPS09[] = {  0, 111, 111, 111, 222, 222, 222, 333, 333, 333, 444, 444, 444, 555, 555, 555, 666};

	int TestSeq_FPS08[] = { 62,  63, 143, 187, 188, 274, 312, 313, 400, 437, 438, 503, 562, 563, 650, 687, 688};
	int TestRes_FPS08[] = {  0, 125, 125, 125, 250, 250, 250, 375, 375, 375, 500, 500, 500, 625, 625, 625, 750};

	int TestSeq_FPS05[] = { 99, 100, 246, 299, 300, 408, 499, 500, 614, 699, 700, 800, 899, 900, 986, 1099, 1100};
	int TestRes_FPS05[] = {  0, 200, 200, 200, 400, 400, 400, 600, 600, 600, 800, 800, 800, 1000, 1000, 1000, 1200};

#define  TEST_SEQ(fps)  TestSeq_FPS##fps
#define  TEST_RES(fps)  TestRes_FPS##fps

	//	int nCount = sizeof(TEST_SEQ(FPS_NO(1)))/sizeof(int);
	CheckAdjustTimeStamp(-1, TEST_SEQ(30), TEST_RES(30), 16);
	CheckAdjustTimeStamp(0, TEST_SEQ(30), TEST_RES(30), 16);

	CheckAdjustTimeStamp(30, TEST_SEQ(30), TEST_RES(30), 16);
	CheckAdjustTimeStamp(25, TEST_SEQ(25), TEST_RES(25), 16);
	CheckAdjustTimeStamp(24, TEST_SEQ(24), TEST_RES(24), 16);
	CheckAdjustTimeStamp(20, TEST_SEQ(20), TEST_RES(20), 16);
	CheckAdjustTimeStamp(15, TEST_SEQ(15), TEST_RES(15), 16);
	CheckAdjustTimeStamp(12, TEST_SEQ(12), TEST_RES(12), 16);
	CheckAdjustTimeStamp(10, TEST_SEQ(10), TEST_RES(10), 16);
	CheckAdjustTimeStamp(9, TEST_SEQ(09), TEST_RES(09), 16);
	CheckAdjustTimeStamp(8, TEST_SEQ(08), TEST_RES(08), 16);
	CheckAdjustTimeStamp(5, TEST_SEQ(05), TEST_RES(05), 16);
	return;
}