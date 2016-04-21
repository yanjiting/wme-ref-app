
#include "gtest/gtest.h"
#include "CmDataBlock.h"

#include "CmUtil.h"

TEST (CCmDataBlock,GetBasePtr) {
    
    
    char szBuff[] = "hello,world";
	CCmDataBlock *pDataBlock = NULL;
    CCmDataBlock::CreateInstance(pDataBlock, 1024,szBuff);
	EXPECT_STREQ (szBuff, pDataBlock->GetBasePtr());
    
}


TEST (CCmDataBlock,GetLength) {
	const char *szBuff = "hello,world";
	CCmDataBlock *pDataBlock = NULL;
	CCmDataBlock::CreateInstance(pDataBlock, 1024,szBuff);
	ASSERT_EQ (1024, pDataBlock->GetLength());

}


TEST (CCmDataBlock,GetCapacity) {
	const char *szBuff = "hello,world";
	CCmDataBlock *pDataBlock = NULL;
	CCmDataBlock::CreateInstance(pDataBlock, 1024,szBuff);
	ASSERT_EQ (1024, pDataBlock->GetCapacity());

}
