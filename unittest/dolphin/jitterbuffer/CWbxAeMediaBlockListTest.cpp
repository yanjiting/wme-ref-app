#ifndef WINVER                  // Specifies that the minimum required platform is Windows XP.
#define WINVER 0x0500           // Change this to the appropriate value to target other versions of Windows.
#endif

#include "gtest/gtest.h"
#include "gmock/gmock.h"
//#include "wbxmock/wbxmock.h"

#define private public
#define protected public

#include "WbxAeMediaBlockList.h"


using testing::Return;
using testing::_;
using testing::Invoke;
using testing::ReturnRef;
using testing::Matcher;

class CWbxAeMediaBlockListTestEx : public testing::Test
{
public:
	CWbxAeMediaBlockListTestEx()
	{
	}

	virtual ~CWbxAeMediaBlockListTestEx()
	{
	}

	virtual void SetUp()
	{
		m_pList = new CWbxAeMediaBlockList();
	}

	virtual void TearDown()
	{
		delete m_pList;
	}

protected:
	CWbxAeMediaBlockList * m_pList;
};

TEST_F(CWbxAeMediaBlockListTestEx, CleanList)
{
	CWbxAeMediaBlock * p1 = new CWbxAeMediaBlock(WBX_MEMORY_BLOCK_SIZE);
	p1->AdvanceWritePtr(100);
	EXPECT_EQ(WBXAE_SUCCESS,m_pList->PushBack(p1));
	EXPECT_EQ(1,m_pList->m_CaptureDataList.size());

	m_pList->CleanList();
	EXPECT_EQ(0,m_pList->m_CaptureDataList.size());	

}

TEST_F(CWbxAeMediaBlockListTestEx, GetDataPtr)
{
	CWbxAeMediaBlock * p1 = new CWbxAeMediaBlock(WBX_MEMORY_BLOCK_SIZE);
	p1->AdvanceWritePtr(100);
	EXPECT_EQ(WBXAE_SUCCESS,m_pList->PushBack(p1));
	EXPECT_EQ(1,m_pList->m_CaptureDataList.size());

	//test one packet include all frame case;
	BYTE * pSrc = NULL;
	CWbxAeMediaBlock * pMediaBlock = NULL;
	EXPECT_EQ(WBXAE_SUCCESS,m_pList->GetDataPtr(pSrc, 50,pMediaBlock));
	EXPECT_TRUE(pSrc == p1->GetBeginePtr());
	EXPECT_EQ(1,m_pList->m_CaptureDataList.size());
	if(pMediaBlock)
		pMediaBlock->ReleaseReference();

	CWbxAeMediaBlock * p2 = new CWbxAeMediaBlock(WBX_MEMORY_BLOCK_SIZE);
	p2->AdvanceWritePtr(100);
	EXPECT_EQ(WBXAE_SUCCESS,m_pList->PushBack(p2));
	EXPECT_EQ(2,m_pList->m_CaptureDataList.size());

	EXPECT_EQ(WBXAE_SUCCESS,m_pList->GetDataPtr(pSrc, 100,pMediaBlock));
	//EXPECT_TRUE(pMediaBlock-> == pSrc);
	EXPECT_EQ(1,m_pList->m_CaptureDataList.size());
	if(pMediaBlock)
		pMediaBlock->ReleaseReference();

	m_pList->CleanList();
	EXPECT_EQ(0,m_pList->m_CaptureDataList.size());

	//test mutiltp packet
	//add 3 packet, each 100 bytes, get 280 bytes, should left 1 packet
	p1 = new CWbxAeMediaBlock(WBX_MEMORY_BLOCK_SIZE);
	p1->AdvanceWritePtr(100);
	EXPECT_EQ(WBXAE_SUCCESS,m_pList->PushBack(p1));
	EXPECT_EQ(1,m_pList->m_CaptureDataList.size());

	p2 = new CWbxAeMediaBlock(WBX_MEMORY_BLOCK_SIZE);
	p2->AdvanceWritePtr(100);
	EXPECT_EQ(WBXAE_SUCCESS,m_pList->PushBack(p2));
	EXPECT_EQ(2,m_pList->m_CaptureDataList.size());

	CWbxAeMediaBlock *p3 = new CWbxAeMediaBlock(WBX_MEMORY_BLOCK_SIZE);
	p3->AdvanceWritePtr(100);
	EXPECT_EQ(WBXAE_SUCCESS,m_pList->PushBack(p3));
	EXPECT_EQ(3,m_pList->m_CaptureDataList.size());

	EXPECT_EQ(WBXAE_SUCCESS,m_pList->GetDataPtr(pSrc, 280,pMediaBlock));
	EXPECT_TRUE(p1->GetBeginePtr() == pSrc);
	EXPECT_EQ(1,m_pList->m_CaptureDataList.size());
	EXPECT_EQ(20,p3->GetLeftLength());
	if(pMediaBlock)
		pMediaBlock->ReleaseReference();

	m_pList->CleanList();
	EXPECT_EQ(0,m_pList->m_CaptureDataList.size());

}

TEST_F(CWbxAeMediaBlockListTestEx, GetDataLen)
{
	DWORD temp = m_pList->GetDataLen();
	EXPECT_EQ(0,temp);

}
TEST_F(CWbxAeMediaBlockListTestEx, PushBack)
{
	CWbxAeMediaBlock * p1 = new CWbxAeMediaBlock(WBX_MEMORY_BLOCK_SIZE);
	p1->AdvanceWritePtr(100);
	EXPECT_EQ(WBXAE_SUCCESS,m_pList->PushBack(p1));
	EXPECT_EQ(1,m_pList->m_CaptureDataList.size());
	m_pList->CleanList();

}
TEST_F(CWbxAeMediaBlockListTestEx, AlignFirst)
{
	CWbxAeMediaBlock * p1 = new CWbxAeMediaBlock(WBX_MEMORY_BLOCK_SIZE);
	p1->AdvanceWritePtr(100);
	p1->AdvanceReadPtr(50);
	EXPECT_EQ(WBXAE_SUCCESS,m_pList->PushBack(p1));
	EXPECT_EQ(1,m_pList->m_CaptureDataList.size());

	EXPECT_EQ(WBXAE_SUCCESS,m_pList->AlignFirst());
	EXPECT_EQ(0,m_pList->m_CaptureDataList.size());

	p1 = new CWbxAeMediaBlock(WBX_MEMORY_BLOCK_SIZE);
	p1->AdvanceWritePtr(100);
	EXPECT_EQ(WBXAE_SUCCESS,m_pList->PushBack(p1));
	EXPECT_EQ(1,m_pList->m_CaptureDataList.size());

	EXPECT_EQ(WBXAE_SUCCESS,m_pList->AlignFirst());
	EXPECT_EQ(1,m_pList->m_CaptureDataList.size());

	m_pList->CleanList();
}
