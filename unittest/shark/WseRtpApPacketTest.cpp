#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "WseRtpApPacket.h"


class WseRtpApPacketTest : public testing::Test
{
public:
	enum {
		MAX_STAP_SIZE = 1024,
		FRAGMENT_NUM = 5,
	};

	WseRtpApPacketTest()
	{
		m_pH264Packer = new shark::CWseH264AggregationAOperator();
		m_pH265Packer = new shark::CWseH265AggregationNoDonOperator();

		m_pFragments = new wrtp::FragmentBuffer[FRAGMENT_NUM];
		for (int i=0; i<FRAGMENT_NUM; i++)
		{
			m_pFragments[i].pBuffer = new unsigned char[MAX_STAP_SIZE];
			m_pFragments[i].fragmentLength = 0;
			m_pFragments[i].bufferLength = MAX_STAP_SIZE;
		}
	}

	virtual ~WseRtpApPacketTest()
	{
		delete m_pH264Packer;
		delete m_pH265Packer;
		delete m_pFragments;
	}

	virtual void SetUp()
	{

	}

	virtual void TearDown()
	{

	}

public:
	wrtp::ISingleTimeAggregationAOperator *m_pH264Packer;
	wrtp::ISingleTimeAggregationAOperator *m_pH265Packer;
	wrtp::FragmentBuffer	*m_pFragments;
};


/////////////////////////////////////////////

struct H264_NAL_Info_t {
	int len;
	int fzb;
	int nri;
	int type;
};

static H264_NAL_Info_t s_H264NALs[] =
{	
	{12, 0, 1, 6},
	{34, 0, 1, 7},
	{11, 0, 1, 8},
	{800, 0, 1, 5},
	{200, 0, 0, 1},
};

TEST_F(WseRtpApPacketTest, TestH264Aggregation)
{	
	UINT32 count = 0;
	UINT8 *outputBuffer = new UINT8[1024];
	UINT32 length = 1024;
	wrtp::FragmentBuffer *nals = m_pFragments;

	EXPECT_EQ(-1, m_pH264Packer->Aggregate(nals, count, outputBuffer, length));	
	delete outputBuffer;
	outputBuffer = NULL;
	EXPECT_EQ(-1, m_pH264Packer->Aggregate(nals, count, outputBuffer, length));
	length = 0;
	EXPECT_EQ(-1, m_pH264Packer->Aggregate(nals, count, outputBuffer, length));

	// create several NALs
	shark::CWseH264AggregationAOperator::H264NUHeader *pNuHeader = NULL;

	count = 0;
	int expectedStapLen = 0;
	for (int k=0; k < FRAGMENT_NUM; k++) {
		pNuHeader = (shark::CWseH264AggregationAOperator::H264NUHeader *)(m_pFragments[k].pBuffer);
		m_pFragments[k].fragmentLength = s_H264NALs[k].len;
		pNuHeader->m_fzb = s_H264NALs[k].fzb;
		pNuHeader->m_nri = s_H264NALs[k].nri;
		pNuHeader->m_type = s_H264NALs[k].type;
		expectedStapLen += (2 + s_H264NALs[k].len);
		count++;
	}

	nals = m_pFragments;	
	outputBuffer = new UINT8[1200];	
	
	length = 1000;
	EXPECT_EQ(-1, m_pH264Packer->Aggregate(nals, count, outputBuffer, length));
	length = 1200;
	EXPECT_EQ(0, m_pH264Packer->Aggregate(nals, count, outputBuffer, length));
	EXPECT_EQ(expectedStapLen+1, length);


	UINT8* rawData = outputBuffer;
	UINT32 rawLength = expectedStapLen+1;	
	UINT32 rawCount = count;
	wrtp::FragmentBuffer *rawNals = NULL;
	EXPECT_EQ(rawCount, m_pH264Packer->GetContainedNALCount(rawData, rawLength));
	EXPECT_EQ(0, m_pH264Packer->GetContainedNALCount(rawData, rawLength-1));
	EXPECT_EQ(0, m_pH264Packer->GetContainedNALCount(rawData, rawLength+1));	//FIXME
	EXPECT_EQ(0, m_pH264Packer->GetContainedNALCount(rawData, 0));
	EXPECT_EQ(0, m_pH264Packer->GetContainedNALCount(NULL, rawLength));

	rawNals = new wrtp::FragmentBuffer[rawCount];
	for (int k=0; k<rawCount; k++)
	{
		rawNals[k].pBuffer = new unsigned char[MAX_STAP_SIZE];
		rawNals[k].fragmentLength = 0;
		rawNals[k].bufferLength = MAX_STAP_SIZE;
	}
	EXPECT_EQ(0, m_pH264Packer->Deaggregate(rawData, rawLength, rawNals, rawCount));

	for (int k=0; k<rawCount; k++)
	{
		EXPECT_EQ(rawNals[k].fragmentLength, m_pFragments[k].fragmentLength);
		if (rawNals[k].fragmentLength == m_pFragments[k].fragmentLength) {
			int ret = memcmp(rawNals[k].pBuffer, m_pFragments[k].pBuffer, m_pFragments[k].fragmentLength);
			EXPECT_EQ(0, ret);
		}
	}
}

//////////////////////////////////////////////////////

struct H265_NAL_Info_t {
	int len;
	int fzb;
	int layerid; // 0-63
	int tid;	// 0-7
	int type;	// 0-47,
};

static H265_NAL_Info_t s_H265NALs[] =
{	
	{12,	0, 1,	6,	5},
	{34,	0, 25,	0,	43},
	{11,	0, 12,	3,	1},
	{800,	0, 13,	4,	8},
	{200,	0, 5,	1,	7},
};

TEST_F(WseRtpApPacketTest, TestH265Aggregation)
{	
	UINT32 count = 0;
	UINT8 *outputBuffer = new UINT8[1024];
	UINT32 length = 1024;
	wrtp::FragmentBuffer *nals = m_pFragments;

	EXPECT_EQ(-1, m_pH265Packer->Aggregate(nals, count, outputBuffer, length));	
	delete outputBuffer;
	outputBuffer = NULL;
	EXPECT_EQ(-1, m_pH265Packer->Aggregate(nals, count, outputBuffer, length));
	length = 0;
	EXPECT_EQ(-1, m_pH265Packer->Aggregate(nals, count, outputBuffer, length));

	// create several NALs
	shark::CWseH265AggregationNoDonOperator::H265NUHeader *pNuHeader = NULL;

	count = 0;
	int expectedStapLen = 0;
	for (int k=0; k < FRAGMENT_NUM; k++) {
		pNuHeader = (shark::CWseH265AggregationNoDonOperator::H265NUHeader *)(m_pFragments[k].pBuffer);
		m_pFragments[k].fragmentLength = s_H265NALs[k].len;
		pNuHeader->m_fzb = s_H265NALs[k].fzb;
		pNuHeader->m_layerIdH = (s_H265NALs[k].layerid>>5);
		pNuHeader->m_layerIdL = s_H265NALs[k].layerid & 0x1f;
		pNuHeader->m_tid = s_H265NALs[k].tid;
		pNuHeader->m_type = s_H265NALs[k].type;
		expectedStapLen += (2 + s_H265NALs[k].len);
		count++;
	}

	nals = m_pFragments;	
	outputBuffer = new UINT8[1200];	
	
	length = 1000;
	EXPECT_EQ(-1, m_pH265Packer->Aggregate(nals, count, outputBuffer, length));
	length = 1200;
	EXPECT_EQ(0, m_pH265Packer->Aggregate(nals, count, outputBuffer, length));
	EXPECT_EQ(expectedStapLen+2, length);


	UINT8* rawData = outputBuffer;
	UINT32 rawLength = expectedStapLen+2;	
	UINT32 rawCount = count;
	wrtp::FragmentBuffer *rawNals = NULL;
	EXPECT_EQ(rawCount, m_pH265Packer->GetContainedNALCount(rawData, rawLength));
	EXPECT_EQ(0, m_pH265Packer->GetContainedNALCount(rawData, rawLength-1));
	EXPECT_EQ(0, m_pH265Packer->GetContainedNALCount(rawData, rawLength+1));	//FIXME
	EXPECT_EQ(0, m_pH265Packer->GetContainedNALCount(rawData, 0));
	EXPECT_EQ(0, m_pH265Packer->GetContainedNALCount(NULL, rawLength));

	rawNals = new wrtp::FragmentBuffer[rawCount];
	for (int k=0; k<rawCount; k++)
	{
		rawNals[k].pBuffer = new unsigned char[MAX_STAP_SIZE];
		rawNals[k].fragmentLength = 0;
		rawNals[k].bufferLength = MAX_STAP_SIZE;
	}
	EXPECT_EQ(0, m_pH265Packer->Deaggregate(rawData, rawLength, rawNals, rawCount));

	for (int k=0; k<rawCount; k++)
	{
		EXPECT_EQ(rawNals[k].fragmentLength, m_pFragments[k].fragmentLength);
		if (rawNals[k].fragmentLength == m_pFragments[k].fragmentLength) {
			int ret = memcmp(rawNals[k].pBuffer, m_pFragments[k].pBuffer, m_pFragments[k].fragmentLength);
			EXPECT_EQ(0, ret);
		}
	}
}