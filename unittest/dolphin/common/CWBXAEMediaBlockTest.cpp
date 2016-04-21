/*****************************************************************************************
*Generate by AUT Tool on 2010-12-22
*class:CWBXAEMediaBlockTest
********************************************************************************************/
#include "WbxAeDefine.h"
//#include "CmStdCpp.h"

// in CmStdCpp.h, remove CM_DISABLE_MSVCP_DLL define
// #ifdef CM_DISABLE_MSVCP_DLL
// #undef CM_DISABLE_MSVCP_DLL
// #endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
//#include "wbxmock.h"

#define private public
#define protected public
#include "WbxAeMediaBlock.h"

using testing::Return;
using testing::_;
using testing::Invoke;
using testing::ReturnRef;
using testing::Matcher;


class CWBXAEMediaBlockTest : public testing::Test
{
public:
    CWBXAEMediaBlockTest()
    {
    }

    virtual ~CWBXAEMediaBlockTest()
    {
    }

    virtual void SetUp()
    {

		m_pMediaBlock = new CWbxAeMediaBlock(100);
    }

    virtual void TearDown()
    {
		delete m_pMediaBlock;
    }

protected:
	CWbxAeMediaBlock * m_pMediaBlock;
};

TEST_F(CWBXAEMediaBlockTest, GetBeginePtr)
{
	EXPECT_EQ(m_pMediaBlock->m_pBeginPtr,m_pMediaBlock->GetBeginePtr());
}

TEST_F(CWBXAEMediaBlockTest, GetReadPtr)
{
	EXPECT_EQ(m_pMediaBlock->m_pReadPtr,m_pMediaBlock->GetReadPtr());
}

TEST_F(CWBXAEMediaBlockTest, AdvanceReadPtr)
{
	EXPECT_EQ(WBXAE_ERROR_MEMORY_OVERWRITE,m_pMediaBlock->AdvanceReadPtr(200));

	//first need remove write ptr, otherwise, therer is no data in buffer.
	m_pMediaBlock->AdvanceWritePtr(100);
	EXPECT_EQ(WBXAE_SUCCESS,m_pMediaBlock->AdvanceReadPtr(80));
}


TEST_F(CWBXAEMediaBlockTest, GetWritePtr)
{
	EXPECT_EQ(m_pMediaBlock->m_pWritePtr,m_pMediaBlock->GetWritePtr());
}

TEST_F(CWBXAEMediaBlockTest, AdvanceWritePtr)
{
	EXPECT_EQ(WBXAE_ERROR_MEMORY_OVERWRITE,m_pMediaBlock->AdvanceWritePtr(WBX_MEMORY_BLOCK_SIZE +1));
	EXPECT_EQ(WBXAE_SUCCESS,m_pMediaBlock->AdvanceWritePtr(80));	
}

TEST_F(CWBXAEMediaBlockTest, GetLeftLength)
{
	EXPECT_EQ((m_pMediaBlock->m_pWritePtr - m_pMediaBlock->m_pReadPtr),m_pMediaBlock->GetLeftLength());
}

TEST_F(CWBXAEMediaBlockTest, GetWriteSpace)
{
	EXPECT_EQ((m_pMediaBlock->m_pEndPtr - m_pMediaBlock->m_pWritePtr),m_pMediaBlock->GetWriteSpace());
}

TEST_F(CWBXAEMediaBlockTest, GetWaveFormat)
{
	WBXWAVEFORMAT *waveFormat=m_pMediaBlock->GetWaveFormat();

	unsigned long p1=0,p2=0;
	p1=(unsigned long)waveFormat;
	p2=(unsigned long)&(m_pMediaBlock->m_waveFormat);
	EXPECT_EQ(p1,p2);
}

TEST_F(CWBXAEMediaBlockTest, GetProperty)
{
	WBXAEAudioProperty *Property = m_pMediaBlock->GetProperty();

	unsigned long p1=0,p2=0;
	p1=(unsigned long)Property;
	p2=(unsigned long)&(m_pMediaBlock->m_property);
	EXPECT_EQ(p1,p2);
}

