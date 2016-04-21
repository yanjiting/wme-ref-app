/*****************************************************************************************
*Generate by AUT Tool on 2010-12-22
*class:CWbxMemPool
Test
********************************************************************************************/
#include "WbxAeDefine.h"
//#include "CmStdCpp.h"
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

class CWbxMemPoolTest : public testing::Test
{
public:
    CWbxMemPoolTest()
    {
    }

    virtual ~CWbxMemPoolTest()
    {
    }

    virtual void SetUp()
    {
		m_pMemPool = new CWbxMemPool;
    }

    virtual void TearDown()
    {
		delete m_pMemPool;
    }

protected:
	CWbxMemPool* m_pMemPool;
};


TEST_F(CWbxMemPoolTest, CWbxMemPool)
{
	EXPECT_EQ(WBX_MAX_MEMPOOL_SIZE,m_pMemPool->m_MemPool.size());
}


TEST_F(CWbxMemPoolTest, Alloc)
{
	//alloc a normal size
	int size = m_pMemPool->m_MemPool.size();
	BYTE* p = m_pMemPool->Alloc(100);
	EXPECT_TRUE(p);
	EXPECT_EQ(size-1,m_pMemPool->m_MemPool.size());
	m_pMemPool->Free(p,100);

	//alloc a max+1 size
	size = m_pMemPool->m_MemPool.size();
	p= m_pMemPool->Alloc(WBX_MEMORY_BLOCK_SIZE+1);
	EXPECT_TRUE(p);
	EXPECT_EQ(size,m_pMemPool->m_MemPool.size());
	m_pMemPool->Free(p,WBX_MEMORY_BLOCK_SIZE+1);
	EXPECT_EQ(size,m_pMemPool->m_MemPool.size());

	//After pop up maxsize, the left should WBX_ADDNUMBER_EACH_TIME
	size = m_pMemPool->m_MemPool.size();
	for(int i =0;i<size+1;i++)
	{
		p = m_pMemPool->Alloc(1);
		EXPECT_TRUE(p);
		if(p)
			delete []p;
	}
	EXPECT_EQ(WBX_ADDNUMBER_EACH_TIME-1,m_pMemPool->m_MemPool.size());
}

TEST_F(CWbxMemPoolTest, Free)
{
	//free a normal size
	int size = m_pMemPool->m_MemPool.size();
	BYTE *p = m_pMemPool->Alloc(10);
	m_pMemPool->Free(p,10);
	EXPECT_EQ(size,m_pMemPool->m_MemPool.size());

	//free a max+1 size
	size = m_pMemPool->m_MemPool.size();
	p= m_pMemPool->Alloc(WBX_MEMORY_BLOCK_SIZE+1);
	EXPECT_TRUE(p);
	EXPECT_EQ(size,m_pMemPool->m_MemPool.size());
	m_pMemPool->Free(p,WBX_MEMORY_BLOCK_SIZE+1);
	EXPECT_EQ(size,m_pMemPool->m_MemPool.size());
}

TEST_F(CWbxMemPoolTest, AddMemPoolItem)
{
	int size = m_pMemPool->m_MemPool.size();
	EXPECT_EQ(WBXAE_ERROR_INVALID_VALUE,m_pMemPool->AddMemPoolItem(0));

	//normal added
	size = m_pMemPool->m_MemPool.size();
	EXPECT_EQ(WBXAE_SUCCESS,m_pMemPool->AddMemPoolItem(20));
	EXPECT_EQ(size+20,m_pMemPool->m_MemPool.size());
}

TEST_F(CWbxMemPoolTest, CreateMemPool)
{
	int size = m_pMemPool->m_dwMaxBlockSize;
	m_pMemPool->DestroyMemPool();
	EXPECT_EQ(WBXAE_SUCCESS,m_pMemPool->CreateMemPool());
	EXPECT_EQ(size,m_pMemPool->m_MemPool.size());

}

TEST_F(CWbxMemPoolTest, DestroyMemPool)
{
	int size = m_pMemPool->m_dwMaxBlockSize;
	EXPECT_EQ(WBXAE_SUCCESS,m_pMemPool->DestroyMemPool());
	EXPECT_EQ(0,m_pMemPool->m_MemPool.size());
}

