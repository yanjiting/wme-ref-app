/*****************************************************************************************
*Generate by AUT Tool on 2010-12-22
*class:CWBXAEMediaBlockTestEx
********************************************************************************************/
#include "WbxAeDefine.h"
#include "CmStdCpp.h"

// in CmStdCpp.h, remove CM_DISABLE_MSVCP_DLL define
// #ifdef CM_DISABLE_MSVCP_DLL
// #undef CM_DISABLE_MSVCP_DLL
// #endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
//#include "wbxmock/wbxmock.h"

#define private public
#define protected public
#include "WbxAeMediaBlock.h"

using testing::Return;
using testing::_;
using testing::Invoke;
using testing::ReturnRef;
using testing::Matcher;


class CWBXAEMediaBlockTestEx : public testing::Test
{
public:
    CWBXAEMediaBlockTestEx()
    {
    }

    virtual ~CWBXAEMediaBlockTestEx()
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

TEST_F(CWBXAEMediaBlockTestEx, GetBeginePtr)
{
	EXPECT_EQ(m_pMediaBlock->m_pBeginPtr,m_pMediaBlock->GetBeginePtr());
}

TEST_F(CWBXAEMediaBlockTestEx, GetReadPtr)
{
	EXPECT_EQ(m_pMediaBlock->m_pReadPtr,m_pMediaBlock->GetReadPtr());
}

TEST_F(CWBXAEMediaBlockTestEx, AdvanceReadPtr)
{
	EXPECT_EQ(WBXAE_ERROR_MEMORY_OVERWRITE,m_pMediaBlock->AdvanceReadPtr(200));

	//first need remove write ptr, otherwise, therer is no data in buffer.
	m_pMediaBlock->AdvanceWritePtr(100);
	EXPECT_EQ(WBXAE_SUCCESS,m_pMediaBlock->AdvanceReadPtr(80));
}


TEST_F(CWBXAEMediaBlockTestEx, GetWritePtr)
{
	EXPECT_EQ(m_pMediaBlock->m_pWritePtr,m_pMediaBlock->GetWritePtr());
}

TEST_F(CWBXAEMediaBlockTestEx, AdvanceWritePtr)
{
	EXPECT_EQ(WBXAE_ERROR_MEMORY_OVERWRITE,m_pMediaBlock->AdvanceWritePtr(WBX_MEMORY_BLOCK_SIZE +1));
	EXPECT_EQ(WBXAE_SUCCESS,m_pMediaBlock->AdvanceWritePtr(80));	
}

TEST_F(CWBXAEMediaBlockTestEx, GetLeftLength)
{
	EXPECT_EQ((m_pMediaBlock->m_pWritePtr - m_pMediaBlock->m_pReadPtr),m_pMediaBlock->GetLeftLength());
}

TEST_F(CWBXAEMediaBlockTestEx, GetWriteSpace)
{
	EXPECT_EQ((m_pMediaBlock->m_pEndPtr - m_pMediaBlock->m_pWritePtr),m_pMediaBlock->GetWriteSpace());
}

TEST_F(CWBXAEMediaBlockTestEx, GetWaveFormat)
{
	WBXWAVEFORMAT *waveFormat=m_pMediaBlock->GetWaveFormat();

	unsigned long p1=0,p2=0;
	p1=(unsigned long)waveFormat;
	p2=(unsigned long)&(m_pMediaBlock->m_waveFormat);
	EXPECT_EQ(p1,p2);
}

TEST_F(CWBXAEMediaBlockTestEx, GetProperty)
{
	WBXAEAudioProperty *Property = m_pMediaBlock->GetProperty();

	unsigned long p1=0,p2=0;
	p1=(unsigned long)Property;
	p2=(unsigned long)&(m_pMediaBlock->m_property);
	EXPECT_EQ(p1,p2);
}

