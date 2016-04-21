
#include "gtest/gtest.h"
#include "CmMessageBlock.h"

#include "CmUtil.h"

class CmMessageBlockTest : public testing::Test
{
public:
    CmMessageBlockTest()
    {
        m_pMsgBlock = NULL;
        m_pDataBlock = NULL;
    }

    virtual ~CmMessageBlockTest()
    {
        Clean();
    }

    virtual void SetUp()
    {
        CmUtilInit();
        Clean();
    }

    virtual void TearDown()
    {
        Clean();
    }

protected:
    void Clean()
    {
        if(NULL != m_pMsgBlock)
        {
            delete m_pMsgBlock;
            m_pMsgBlock = NULL;
        }

        if(NULL != m_pDataBlock)
        {
            m_pDataBlock = NULL;
        }
    }

protected:
    CCmMessageBlock         *m_pMsgBlock;
    CCmDataBlock            *m_pDataBlock;
};


TEST_F(CmMessageBlockTest, GetTopLevelLengthIsZeroTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);
    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
}

TEST_F(CmMessageBlockTest, GetTopLevelLengthIsNotZeroTest)
{
    char szBuff[512] = { 0 };
    char szData[] = "hello world";
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    DWORD dwDataLen = strlen(szData);
    m_pMsgBlock->Write(szData, dwDataLen);
    EXPECT_EQ(dwDataLen, m_pMsgBlock->GetTopLevelLength());
}

TEST_F(CmMessageBlockTest, GetTopLevelSpaceIsNotZeroTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);
    EXPECT_EQ(512, m_pMsgBlock->GetTopLevelSpace());
}

TEST_F(CmMessageBlockTest, GetTopLevelSpaceIsZeroTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData = 'h';
    for(int i=0; i<512; i++)
    {
        m_pMsgBlock->Write(&szData, 1);
    }

    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelSpace());
}

TEST_F(CmMessageBlockTest, GetNextIsNullTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);
    EXPECT_TRUE(NULL == m_pMsgBlock->GetNext());
}

TEST_F(CmMessageBlockTest, GetNextIsNotNullTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    CCmMessageBlock mbNext;
    m_pMsgBlock->Append(&mbNext);
    EXPECT_TRUE(NULL != m_pMsgBlock->GetNext());
}

TEST_F(CmMessageBlockTest, GetTopLevelReadPtrIsNullTest)
{
    m_pMsgBlock = new CCmMessageBlock();

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_TRUE(NULL == m_pMsgBlock->GetTopLevelReadPtr());
}

TEST_F(CmMessageBlockTest, GetTopLevelReadPtrIsNotNullTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);
    EXPECT_TRUE(NULL != m_pMsgBlock->GetTopLevelReadPtr());
}

TEST_F(CmMessageBlockTest, GetTopLevelWritePtrIsNullTest)
{
    m_pMsgBlock = new CCmMessageBlock();

    ASSERT_TRUE(NULL != m_pMsgBlock);
    EXPECT_TRUE(NULL == m_pMsgBlock->GetTopLevelWritePtr());
}

TEST_F(CmMessageBlockTest, GetTopLevelWritePtrIsNotNullTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);
    
    ASSERT_TRUE(NULL != m_pMsgBlock);
    EXPECT_TRUE(NULL != m_pMsgBlock->GetTopLevelWritePtr());
}

TEST_F(CmMessageBlockTest, AdvanceTopLevelWritePtrIsZeroTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceTopLevelWritePtr(0));
    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
}

TEST_F(CmMessageBlockTest, AdvanceTopLevelWritePtrIsInScopeTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceTopLevelWritePtr(100));
    EXPECT_EQ(100, m_pMsgBlock->GetTopLevelLength());
}

TEST_F(CmMessageBlockTest, AdvanceTopLevelWritePtrIsOutScopeTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
    EXPECT_TRUE(CM_OK != m_pMsgBlock->AdvanceTopLevelWritePtr(513));
    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
}

TEST_F(CmMessageBlockTest, RetreatTopLevelWritePtrIsZeroTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
    EXPECT_TRUE(CM_OK == m_pMsgBlock->RetreatTopLevelWritePtr(0));
    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
}

TEST_F(CmMessageBlockTest, RetreatTopLevelWritePtrIsInScopeTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceTopLevelWritePtr(100));
    EXPECT_EQ(100, m_pMsgBlock->GetTopLevelLength());
    EXPECT_TRUE(CM_OK == m_pMsgBlock->RetreatTopLevelWritePtr(50));
    EXPECT_EQ(50, m_pMsgBlock->GetTopLevelLength());
}

TEST_F(CmMessageBlockTest, RetreatTopLevelWritePtrIsOutScopeTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceTopLevelWritePtr(100));
    EXPECT_EQ(100, m_pMsgBlock->GetTopLevelLength());
    EXPECT_TRUE(CM_OK != m_pMsgBlock->RetreatTopLevelWritePtr(101));
    EXPECT_EQ(100, m_pMsgBlock->GetTopLevelLength());
}

TEST_F(CmMessageBlockTest, AdvanceTopLevelReadPtrIsZeroTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceTopLevelReadPtr(0));
    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
}

TEST_F(CmMessageBlockTest, AdvanceTopLevelReadPtrIsInScopeTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceTopLevelWritePtr(100));
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceTopLevelReadPtr(50));
    EXPECT_EQ(50, m_pMsgBlock->GetTopLevelLength());
}

TEST_F(CmMessageBlockTest, AdvanceTopLevelReadPtrIsOutScopeTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
    EXPECT_TRUE(CM_OK != m_pMsgBlock->AdvanceTopLevelReadPtr(10));
    EXPECT_EQ(0, m_pMsgBlock->GetTopLevelLength());
}

//Lack RetreatTopLevelReadPtr

TEST_F(CmMessageBlockTest, LockReadingTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello world!!!";
    char szRead[512] = {0};
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen));
    EXPECT_TRUE(CM_OK == m_pMsgBlock->Read(szRead, 5));
    m_pMsgBlock->LockReading();
    EXPECT_TRUE(CM_OK != m_pMsgBlock->Read(szRead, 3));

    EXPECT_TRUE(NULL == m_pMsgBlock->GetTopLevelReadPtr());
    EXPECT_TRUE(CM_OK != m_pMsgBlock->AdvanceTopLevelReadPtr(1));
}

TEST_F(CmMessageBlockTest, LockWritingTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen));
    m_pMsgBlock->LockWriting();
    EXPECT_TRUE(CM_OK != m_pMsgBlock->Write(szData, dwDataLen));

    EXPECT_EQ(NULL, m_pMsgBlock->GetTopLevelWritePtr());
    //EXPECT_TRUE(CM_OK != m_pMsgBlock->AdvanceTopLevelWritePtr(10));
    EXPECT_TRUE(CM_OK != m_pMsgBlock->RetreatTopLevelWritePtr(5));
}


//{{{-------------------------------------------------------
TEST_F(CmMessageBlockTest, CmMessageBlockConstruct1Test)
{
    m_pMsgBlock = new CCmMessageBlock();
    ASSERT_TRUE(NULL != m_pMsgBlock);
}

TEST_F(CmMessageBlockTest, CmMessageBlockConstruct2Test)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,10);
    ASSERT_TRUE(NULL != m_pMsgBlock);
}

TEST_F(CmMessageBlockTest, CmMessageBlockConstruct3Test)
{
    EXPECT_TRUE(CM_OK == CCmDataBlock::CreateInstance(m_pDataBlock, 512));
    DWORD aAdvanceWritePtrSize = 0;

    ASSERT_TRUE(NULL != m_pDataBlock);
    m_pMsgBlock = new CCmMessageBlock(m_pDataBlock, 0, aAdvanceWritePtrSize);
    ASSERT_TRUE(NULL != m_pMsgBlock);
}

TEST_F(CmMessageBlockTest, SetDataReturnOKTest)
{
    m_pMsgBlock = new CCmMessageBlock();

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->SetData(dwDataLen, szData, 0, 5));

    EXPECT_TRUE(CM_OK == CCmDataBlock::CreateInstance(m_pDataBlock, 512));
    DWORD aAdvanceWritePtrSize = 5;

    ASSERT_TRUE(NULL != m_pDataBlock);

    CCmMessageBlock *pData = new CCmMessageBlock();
    ASSERT_TRUE(NULL != pData);
    EXPECT_TRUE(CM_OK == pData->SetData(m_pDataBlock, CCmMessageBlock::MALLOC_AND_COPY, aAdvanceWritePtrSize));
    delete pData;
    pData = NULL;
}

TEST_F(CmMessageBlockTest, SetDataReturnError1Test)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK != m_pMsgBlock->SetData(dwDataLen, szData, 0, 5));
    EXPECT_TRUE(CM_OK != m_pMsgBlock->SetData(dwDataLen, szData, 0, 5));

    CCmMessageBlock *pData = new CCmMessageBlock();
    ASSERT_TRUE(NULL != pData);
    EXPECT_TRUE(CM_OK == pData->SetData(NULL, CCmMessageBlock::MALLOC_AND_COPY, 10));

    CCmMessageBlock *pData2 = new CCmMessageBlock(512, 0, 0);
    EXPECT_TRUE(CM_OK != pData2->SetData(m_pDataBlock, 0, 10));
    delete pData;
    pData = NULL;

    delete pData2;
    pData2 = NULL;
}

TEST_F(CmMessageBlockTest, WriteReturnOKTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);
    
    char cData1 = 'h';
    short sData2 = 100;
    long  lData3 = 1000;
    int   nData4 = -10;

    char szData[] = "hello world!!!";
    DWORD dwDataLen = strlen(szData);
    DWORD dwWriten = 0;

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(&cData1, sizeof(char), &dwWriten));
    EXPECT_EQ(sizeof(char), dwWriten);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(&sData2, sizeof(short), &dwWriten));
    EXPECT_EQ(sizeof(short), dwWriten);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(&lData3, sizeof(long), &dwWriten));
    EXPECT_EQ(sizeof(long), dwWriten);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(&nData4, sizeof(int), &dwWriten));
    EXPECT_EQ(sizeof(int), dwWriten);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen, &dwWriten));

    char cReadData1 ;
    short sReadData2 ;
    long  lReadData3 ;
    int   nReadData4 ;

    DWORD dwRead = 0;
    EXPECT_TRUE(CM_OK == m_pMsgBlock->Read(&cReadData1, sizeof(char), &dwRead));
    EXPECT_EQ(sizeof(char), dwRead);
    EXPECT_EQ(cData1, cReadData1);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Read(&sReadData2, sizeof(short), &dwRead));
    EXPECT_EQ(sizeof(short), dwRead);
    EXPECT_EQ(sReadData2, sReadData2);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Read(&lReadData3, sizeof(long), &dwRead));
    EXPECT_EQ(sizeof(long), dwRead);
    EXPECT_EQ(lData3, lReadData3);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Read(&nReadData4, sizeof(int), &dwRead));
    EXPECT_EQ(sizeof(int), dwRead);
    EXPECT_EQ(nData4, nReadData4);
}

TEST_F(CmMessageBlockTest, WriteReturnOK2Test)
{
    char szBuff[] = "123456789";
    m_pMsgBlock = new CCmMessageBlock(strlen(szBuff), szBuff, 0, strlen(szBuff));

    ASSERT_TRUE(NULL != m_pMsgBlock);

    CCmMessageBlock *pNext = new CCmMessageBlock(512, NULL,0,0);
    ASSERT_TRUE(NULL != pNext);

    m_pMsgBlock->Append(pNext);
    
    char cData1 = 'h';
    short sData2 = 100;
    long  lData3 = 1000;
    int   nData4 = -10;

    char szData[] = "hello world!!!";
    DWORD dwDataLen = strlen(szData);
    DWORD dwWriten = 0;

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(&cData1, sizeof(char), &dwWriten));
    EXPECT_EQ(sizeof(char), dwWriten);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(&sData2, sizeof(short), &dwWriten));
    EXPECT_EQ(sizeof(short), dwWriten);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(&lData3, sizeof(long), &dwWriten));
    EXPECT_EQ(sizeof(long), dwWriten);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(&nData4, sizeof(int), &dwWriten));
    EXPECT_EQ(sizeof(int), dwWriten);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen, &dwWriten));

    char cReadData1 ;
    short sReadData2 ;
    long  lReadData3 ;
    int   nReadData4 ;

    DWORD dwRead = 0;
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceChainedReadPtr(strlen(szBuff), &dwRead));
    EXPECT_TRUE(CM_OK == m_pMsgBlock->Read(&cReadData1, sizeof(char), &dwRead));
    EXPECT_EQ(sizeof(char), dwRead);
    EXPECT_EQ(cData1, cReadData1);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Read(&sReadData2, sizeof(short), &dwRead));
    EXPECT_EQ(sizeof(short), dwRead);
    EXPECT_EQ(sReadData2, sReadData2);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Read(&lReadData3, sizeof(long), &dwRead));
    EXPECT_EQ(sizeof(long), dwRead);
    EXPECT_EQ(lData3, lReadData3);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Read(&nReadData4, sizeof(int), &dwRead));
    EXPECT_EQ(sizeof(int), dwRead);
    EXPECT_EQ(nData4, nReadData4);
}

TEST_F(CmMessageBlockTest, WriteReturnOK3Test)
{
    char szBuff[] = "123456789";
    m_pMsgBlock = new CCmMessageBlock(strlen(szBuff), szBuff, 0, strlen(szBuff));

    ASSERT_TRUE(NULL != m_pMsgBlock);
    
    char szData[] = "hello world!!!";
    DWORD dwDataLen = strlen(szData);
    DWORD dwWriten = 0;
    DWORD dwRead = 0;

    char szRead[512] = {0};

    EXPECT_TRUE(CM_OK == m_pMsgBlock->RetreatTopLevelWritePtr(sizeof(char)));
    EXPECT_TRUE(CM_ERROR_PARTIAL_DATA == m_pMsgBlock->Write(szData, dwDataLen, &dwWriten));
    EXPECT_EQ(sizeof(char), dwWriten);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->RetreatTopLevelWritePtr(sizeof(short)));
    EXPECT_TRUE(CM_ERROR_PARTIAL_DATA == m_pMsgBlock->Write(szData, dwDataLen, &dwWriten));
    EXPECT_EQ(sizeof(short), dwWriten);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->RetreatTopLevelWritePtr(sizeof(long)));
    EXPECT_TRUE(CM_ERROR_PARTIAL_DATA == m_pMsgBlock->Write(szData, dwDataLen, &dwWriten));
    EXPECT_EQ(sizeof(long), dwWriten);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->RetreatTopLevelWritePtr(sizeof(int)));
    EXPECT_TRUE(CM_ERROR_PARTIAL_DATA == m_pMsgBlock->Write(szData, dwDataLen, &dwWriten));
    EXPECT_EQ(sizeof(int), dwWriten);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->RetreatTopLevelWritePtr(sizeof(long) + 1));
    EXPECT_TRUE(CM_ERROR_PARTIAL_DATA == m_pMsgBlock->Write(szData, dwDataLen, &dwWriten));
    EXPECT_EQ(sizeof(long) + 1, dwWriten);

    m_pMsgBlock->RewindChained();
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceTopLevelWritePtr(sizeof(long)));
    EXPECT_TRUE(CM_ERROR_PARTIAL_DATA == m_pMsgBlock->Read(szRead, 512, &dwRead));
    EXPECT_EQ(sizeof(long), dwRead);

    m_pMsgBlock->RewindChained();
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceTopLevelWritePtr(sizeof(int)));
    EXPECT_TRUE(CM_ERROR_PARTIAL_DATA == m_pMsgBlock->Read(szRead, 512, &dwRead));
    EXPECT_EQ(sizeof(int), dwRead);
    
    m_pMsgBlock->RewindChained();
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceTopLevelWritePtr(sizeof(short)));
    EXPECT_TRUE(CM_ERROR_PARTIAL_DATA == m_pMsgBlock->Read(szRead, 512, &dwRead));
    EXPECT_EQ(sizeof(short), dwRead);

    m_pMsgBlock->RewindChained();
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceTopLevelWritePtr(sizeof(char)));
    EXPECT_TRUE(CM_ERROR_PARTIAL_DATA == m_pMsgBlock->Read(szRead, 512, &dwRead));
    EXPECT_EQ(sizeof(char), dwRead);

    m_pMsgBlock->RewindChained();
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceTopLevelWritePtr(sizeof(long) + 1));
    EXPECT_TRUE(CM_ERROR_PARTIAL_DATA == m_pMsgBlock->Read(szRead, 512, &dwRead));
    EXPECT_EQ(sizeof(long) + 1, dwRead);
}

TEST_F(CmMessageBlockTest, GetChainedLengthIsZeroTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);
    EXPECT_EQ(0, m_pMsgBlock->GetChainedLength());
}

TEST_F(CmMessageBlockTest, GetChainedLengthIsNotZeroTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen));

    EXPECT_EQ(dwDataLen, m_pMsgBlock->GetChainedLength());
}

TEST_F(CmMessageBlockTest, GetChainedLengthHasNextTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    CCmMessageBlock *pNext = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);
    ASSERT_TRUE(NULL != pNext);

    char szData[] = "hello world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen));
    EXPECT_TRUE(CM_OK == pNext->Write(szData, dwDataLen));
    
    m_pMsgBlock->Append(pNext);
    EXPECT_EQ(2*dwDataLen, m_pMsgBlock->GetChainedLength());
}

TEST_F(CmMessageBlockTest, GetChainedSpaceIsNotZeroTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_EQ(512 , m_pMsgBlock->GetChainedSpace());
}

TEST_F(CmMessageBlockTest, GetChainedSpaceIsZeroTest)
{
    char szBuff[512] = { 0 };
    char szData[513] = { 0 };
    memset(szData, 'h', 512);
    szData[512] = '\0';

    DWORD dwDataLen = strlen(szData);

    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);
    EXPECT_EQ(512 , m_pMsgBlock->GetChainedSpace());
    EXPECT_EQ(CM_OK, m_pMsgBlock->Write(szData, dwDataLen));
    EXPECT_EQ(0 , m_pMsgBlock->GetChainedSpace());
}

//A->NULL
TEST_F(CmMessageBlockTest, AppendIsNullTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_TRUE(NULL == m_pMsgBlock->GetNext());
    m_pMsgBlock->Append(NULL);
    EXPECT_TRUE(NULL == m_pMsgBlock->GetNext());
}

//A->B
TEST_F(CmMessageBlockTest, AppendIsNotNullTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    CCmMessageBlock mbNext;
    EXPECT_TRUE(NULL == m_pMsgBlock->GetNext());
    m_pMsgBlock->Append(&mbNext);
    EXPECT_TRUE(&mbNext == m_pMsgBlock->GetNext());
}

//A->B->C
TEST_F(CmMessageBlockTest, AppendIsNotNull2Test)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    CCmMessageBlock *pNext1 = new CCmMessageBlock(512, szBuff,0,0);
    CCmMessageBlock *pNext2 = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);
    ASSERT_TRUE(NULL != pNext1);
    ASSERT_TRUE(NULL != pNext2);

    EXPECT_TRUE(NULL == m_pMsgBlock->GetNext());
    m_pMsgBlock->Append(pNext1);
    EXPECT_TRUE(pNext1 == m_pMsgBlock->GetNext());
    m_pMsgBlock->Append(pNext2);
    EXPECT_TRUE(pNext2 == pNext1->GetNext());
}

//A->A
TEST_F(CmMessageBlockTest, AppendIsMySelfTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_TRUE(NULL == m_pMsgBlock->GetNext());
    m_pMsgBlock->Append(m_pMsgBlock);
    EXPECT_TRUE(NULL == m_pMsgBlock->GetNext());
}

//A->B->A
TEST_F(CmMessageBlockTest, AppendHasRecursionTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);
    
    ASSERT_TRUE(NULL != m_pMsgBlock);
    
    CCmMessageBlock mbNext;
    EXPECT_TRUE(NULL == m_pMsgBlock->GetNext());
    m_pMsgBlock->Append(&mbNext);
    EXPECT_TRUE(&mbNext == m_pMsgBlock->GetNext());
    
    mbNext.Append(m_pMsgBlock);
    //EXPECT_TRUE(NULL == mbNext.GetNext());
}

TEST_F(CmMessageBlockTest, AdvanceChainedReadPtrTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    DWORD aBytesRead = 0;
    EXPECT_TRUE(CM_ERROR_PARTIAL_DATA == m_pMsgBlock->AdvanceChainedReadPtr(10));
    EXPECT_EQ(0, m_pMsgBlock->GetChainedLength());

    char szData[] = "hello, world!!!";
    DWORD dwDataLen = strlen(szData);

    m_pMsgBlock->Write(szData, dwDataLen);
    EXPECT_EQ(dwDataLen, m_pMsgBlock->GetChainedLength());
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceChainedReadPtr(5, &aBytesRead));
    EXPECT_EQ(dwDataLen-5, m_pMsgBlock->GetChainedLength());
    EXPECT_EQ(5, aBytesRead);
}

TEST_F(CmMessageBlockTest, AdvanceChainedWritePtrIsOKTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello, world!!!";
    DWORD dwDataLen = strlen(szData);

    DWORD aBytesWritten = 0;

    m_pMsgBlock->Write(szData, dwDataLen);
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceChainedWritePtr(10, &aBytesWritten));
    EXPECT_EQ(10, aBytesWritten);
}

TEST_F(CmMessageBlockTest, AdvanceChainedWritePtrHasError1Test)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello, world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen));
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceChainedReadPtr(dwDataLen - 1));
    EXPECT_TRUE(CM_OK != m_pMsgBlock->AdvanceChainedWritePtr(10));

    DWORD aBytesWritten = 0;
    EXPECT_TRUE(CM_OK != m_pMsgBlock->AdvanceChainedWritePtr(10, &aBytesWritten));
}

TEST_F(CmMessageBlockTest, AdvanceChainedWritePtrHasError2Test)
{
    char szBuff[512] = { 0 };
    DWORD aBytesWritten = 0;

    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceChainedWritePtr(10));
    EXPECT_TRUE(CM_OK != m_pMsgBlock->AdvanceChainedWritePtr(1000, &aBytesWritten));
}

//A->B->C
TEST_F(CmMessageBlockTest, DuplicateChainedReturnOKTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    CCmMessageBlock *pNext1 = new CCmMessageBlock(512, szBuff,0,0);
    CCmMessageBlock *pNext2 = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);
    ASSERT_TRUE(NULL != pNext1);
    ASSERT_TRUE(NULL != pNext2);

    EXPECT_TRUE(NULL == m_pMsgBlock->GetNext());
    m_pMsgBlock->Append(pNext1);
    EXPECT_TRUE(pNext1 == m_pMsgBlock->GetNext());
    m_pMsgBlock->Append(pNext2);
    EXPECT_TRUE(pNext2 == pNext1->GetNext());

    CCmMessageBlock *pDuplicate = m_pMsgBlock->DuplicateChained();
    ASSERT_TRUE(NULL != pDuplicate);
    pDuplicate->DestroyChained();
    pDuplicate = NULL;
}

//A->B->C, B DuplicateTopLevel failure
TEST_F(CmMessageBlockTest, DuplicateChainedReturnNullTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    CCmMessageBlock *pNext1 = new CCmMessageBlock(512, szBuff,0,0);
    CCmMessageBlock *pNext2 = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);
    ASSERT_TRUE(NULL != pNext1);
    ASSERT_TRUE(NULL != pNext2);

    EXPECT_TRUE(NULL == m_pMsgBlock->GetNext());
    m_pMsgBlock->Append(pNext1);
    EXPECT_TRUE(pNext1 == m_pMsgBlock->GetNext());
    m_pMsgBlock->Append(pNext2);
    EXPECT_TRUE(pNext2 == pNext1->GetNext());

    CCmMessageBlock *pDuplicate = m_pMsgBlock->DuplicateChained();
    ASSERT_TRUE(NULL != pDuplicate);
    pDuplicate->DestroyChained();
    pDuplicate = NULL;
}

TEST_F(CmMessageBlockTest, DuplicateTopLevelTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    CCmMessageBlock *pDuplicate = m_pMsgBlock->DuplicateTopLevel();
    ASSERT_TRUE(NULL != pDuplicate);
    pDuplicate->DestroyChained();
    pDuplicate = NULL;
}

TEST_F(CmMessageBlockTest, DisjointTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello, world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen));
    
    CCmMessageBlock* pDisJoint = m_pMsgBlock->Disjoint(5);
    ASSERT_TRUE(NULL != pDisJoint);

    EXPECT_EQ(5, m_pMsgBlock->GetChainedLength());
    EXPECT_EQ(dwDataLen-5, pDisJoint->GetChainedLength());

    EXPECT_TRUE(NULL == pDisJoint->Disjoint(pDisJoint->GetChainedLength()));

    CCmMessageBlock* pNext = new CCmMessageBlock(512, szBuff,0,512);
    ASSERT_TRUE(NULL != pNext);

    pDisJoint->Append(pNext);

    CCmMessageBlock* pDisJoint2 = pDisJoint->Disjoint(pDisJoint->GetTopLevelLength());
    ASSERT_TRUE(NULL != pDisJoint2);

    pDisJoint2->DestroyChained();
    pDisJoint->DestroyChained();
}

TEST_F(CmMessageBlockTest, DestroyChainedTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello, world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen));

    CCmMessageBlock* pData = new CCmMessageBlock(512, szBuff,0,0);
    ASSERT_TRUE(NULL != pData);
    EXPECT_TRUE(CM_OK == pData->Write(szData, dwDataLen));

    CCmMessageBlock* pDuplicate = m_pMsgBlock->DuplicateChained();
    ASSERT_TRUE(NULL != pDuplicate);

    pDuplicate->Append(pData);

    pDuplicate->DestroyChained();

    delete pData;
    pData = NULL;
}

TEST_F(CmMessageBlockTest, FillIovTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello, world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen));

    iovec aIov[1];
    DWORD aMax = 1;

    DWORD dwFilled = m_pMsgBlock->FillIov(aIov, aMax);
    EXPECT_EQ(aMax, dwFilled);

    CCmString strData((const char*)aIov[0].iov_base);
    EXPECT_STREQ(szData, strData.c_str());
}

TEST_F(CmMessageBlockTest, RewindChainedTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    LPCSTR pReadPtr1 = m_pMsgBlock->GetTopLevelReadPtr();
    LPSTR pWritePtr1 = m_pMsgBlock->GetTopLevelWritePtr();

    char szData[] = "hello, world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen));
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceChainedWritePtr(10));
    EXPECT_TRUE(CM_OK == m_pMsgBlock->AdvanceChainedReadPtr(dwDataLen - 1));

    LPCSTR pReadPtr2 = m_pMsgBlock->GetTopLevelReadPtr();
    LPSTR pWritePtr2 = m_pMsgBlock->GetTopLevelWritePtr();

    EXPECT_TRUE(pReadPtr1 != pReadPtr2);
    EXPECT_TRUE(pWritePtr1 != pWritePtr2);

    m_pMsgBlock->RewindChained();

    LPCSTR pReadPtr3 = m_pMsgBlock->GetTopLevelReadPtr();
    LPSTR pWritePtr3 = m_pMsgBlock->GetTopLevelWritePtr();

    EXPECT_TRUE(pReadPtr1 == pReadPtr3);
    EXPECT_TRUE(pWritePtr1 == pWritePtr3);
}

TEST_F(CmMessageBlockTest, ReclaimGarbageTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    CCmMessageBlock* pDuplicate = m_pMsgBlock->DuplicateTopLevel();
    ASSERT_TRUE(NULL != pDuplicate);
    
    //A[0]
    EXPECT_TRUE(NULL == pDuplicate->ReclaimGarbage());

    //A[0]->B[10]
    CCmMessageBlock* pMB1 = new CCmMessageBlock(512, szBuff,0,0);
    ASSERT_TRUE(NULL != pMB1);

    CCmMessageBlock* pMB2 = new CCmMessageBlock(512, szBuff,0,0);
    ASSERT_TRUE(NULL != pMB2);

    char szData[] = "hello, world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == pMB2->Write(szData, dwDataLen));

    pMB1->Append(pMB2);

    CCmMessageBlock* pRet = pMB1->ReclaimGarbage();

    EXPECT_TRUE(pMB2 == pRet);

    delete pMB1;
    pMB1 = NULL;
    delete pMB2;
    pMB2 = NULL;
}



TEST_F(CmMessageBlockTest, UpdateReturnOKTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    BYTE pUpdate[] = "just for test ----!!!";
    DWORD dwUpdateDataLen = strlen((const char*)pUpdate);


    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello, world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen));

    EXPECT_TRUE(CM_ERROR_FAILURE == m_pMsgBlock->Update(pUpdate, dwUpdateDataLen, 0));
}

TEST_F(CmMessageBlockTest, UpdateReturnOK2Test)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);
    
    char cData1 = 'h';
    short sData2 = 100;
    long  lData3 = 1000;
    int   nData4 = -10;

    char szData[] = "hello world!!!";
    DWORD dwDataLen = strlen(szData);
    DWORD dwWriten = 0;

    char szWrite[] = "h e l l o w o r l d ! ! ! j u s t f o r t e s t h a h a";
    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szWrite, strlen(szWrite)));

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)&cData1, sizeof(char), dwWriten));
    dwWriten += sizeof(char);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)&sData2, sizeof(short), dwWriten));
    dwWriten += sizeof(short);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)&lData3, sizeof(long), dwWriten));
    dwWriten += sizeof(long);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)&nData4, sizeof(int), dwWriten));
    dwWriten += sizeof(int);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)szData, dwDataLen, dwWriten));
    dwWriten += dwDataLen;

    char cReadData1 ;
    short sReadData2 ;
    long  lReadData3 ;
    int   nReadData4 ;
    char  szRead[512] = {0};

    DWORD dwRead = 0;
    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)&cReadData1, sizeof(char), dwRead));
    dwRead += sizeof(char);
    EXPECT_EQ(cData1, cReadData1);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)&sReadData2, sizeof(short), dwRead));
    dwRead += sizeof(short);
    EXPECT_EQ(sData2, sReadData2);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)&lReadData3, sizeof(long), dwRead));
    dwRead += sizeof(long);
    EXPECT_EQ(lData3, lReadData3);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)&nReadData4, sizeof(int), dwRead));
    dwRead += sizeof(int);
    EXPECT_EQ(nData4, nReadData4);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)szRead, dwDataLen, dwRead));
    dwRead += dwDataLen;
    EXPECT_STREQ(szData, szRead);

    EXPECT_EQ(dwWriten, dwRead);
}

TEST_F(CmMessageBlockTest, UpdateReturnOK3Test)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);
    
    char cData1 = 'h';
    short sData2 = 100;
    long  lData3 = 1000;
    int   nData4 = -10;

    char szData[] = "hello world!!!";
    DWORD dwDataLen = strlen(szData);
    DWORD dwWriten = 0;

    char szWrite[] = "h e l l o w o r l d ! ! ! j u s t f o r t e s t h a h a";
    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szWrite, strlen(szWrite)));

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)&cData1, sizeof(char), dwWriten));
    dwWriten += sizeof(char);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)&sData2, sizeof(short), dwWriten));
    dwWriten += sizeof(short);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)&lData3, sizeof(long), dwWriten));
    dwWriten += sizeof(long);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)&nData4, sizeof(int), dwWriten));
    dwWriten += sizeof(int);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)szData, dwDataLen, dwWriten));
    dwWriten += dwDataLen;

    char cReadData1 ;
    short sReadData2 ;
    long  lReadData3 ;
    int   nReadData4 ;
    char  szRead[512] = {0};

    DWORD dwRead = 0;
    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)&cReadData1, sizeof(char), dwRead));
    dwRead += sizeof(char);
    EXPECT_EQ(cData1, cReadData1);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)&sReadData2, sizeof(short), dwRead));
    dwRead += sizeof(short);
    EXPECT_EQ(sData2, sReadData2);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)&lReadData3, sizeof(long), dwRead));
    dwRead += sizeof(long);
    EXPECT_EQ(lData3, lReadData3);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)&nReadData4, sizeof(int), dwRead));
    dwRead += sizeof(int);
    EXPECT_EQ(nData4, nReadData4);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)szRead, dwDataLen, dwRead));
    dwRead += dwDataLen;
    EXPECT_STREQ(szData, szRead);

    EXPECT_EQ(dwWriten, dwRead);
}

TEST_F(CmMessageBlockTest, UpdateReturnOK4Test)
{
    char szBuff[] = "+++++++++++++++++++++++++++";
    DWORD dwBuffLen = strlen(szBuff);
    m_pMsgBlock = new CCmMessageBlock(dwBuffLen, szBuff, 0, dwBuffLen);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello world!!!";
    char szRead[] = "##############";
    DWORD dwDataLen = strlen(szData);

    char szBuff2[] = "******************************";
    CCmMessageBlock *pNext = new CCmMessageBlock(strlen(szBuff2), szBuff2, 0, strlen(szBuff2));
    ASSERT_TRUE(NULL != pNext);
    m_pMsgBlock->Append(pNext);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)szData, dwDataLen, dwBuffLen - sizeof(char)));
    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)szRead, dwDataLen, dwBuffLen - sizeof(char)));
    EXPECT_STREQ(szRead, szData);

    memset(szRead, '#', strlen(szRead));

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)szData, dwDataLen, dwBuffLen - sizeof(short)));
    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)szRead, dwDataLen, dwBuffLen - sizeof(short)));
    EXPECT_STREQ(szRead, szData);

    memset(szRead, '#', strlen(szRead));

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)szData, dwDataLen, dwBuffLen - sizeof(long)));
    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)szRead, dwDataLen, dwBuffLen - sizeof(long)));
    EXPECT_STREQ(szRead, szData);

    memset(szRead, '#', strlen(szRead));

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)szData, dwDataLen, dwBuffLen - sizeof(int)));
    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)szRead, dwDataLen, dwBuffLen - sizeof(int)));
    EXPECT_STREQ(szRead, szData);

    memset(szRead, '#', strlen(szRead));

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update((const BYTE*)szData, dwDataLen, dwBuffLen - sizeof(long) - 1));
    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek((BYTE*)szRead, dwDataLen, dwBuffLen - sizeof(long) - 1));
    EXPECT_STREQ(szRead, szData);
}

TEST_F(CmMessageBlockTest, UpdateReturnError1Test)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    BYTE pUpdate[1024] = {0};

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Update(NULL, 0, 0));
    EXPECT_TRUE(CM_ERROR_FAILURE == m_pMsgBlock->Update(NULL, 10, 0));
    EXPECT_TRUE(CM_ERROR_FAILURE == m_pMsgBlock->Update(pUpdate, 10, 0));
}

TEST_F(CmMessageBlockTest, PeekReturnOKTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    BYTE pPeekBuffer[1024] = {0};

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello, world!!!";
    DWORD dwDataLen = strlen(szData);


    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen));
    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek(pPeekBuffer, 10, 0));

    CCmMessageBlock *pData2 = new CCmMessageBlock(512, szBuff,0,0);
    char szData2[] = "test data ha ha ha ha";
    DWORD dwDataLen2 = strlen(szData2);
    EXPECT_TRUE(CM_OK == pData2->Write(szData2, dwDataLen2));

    m_pMsgBlock->Append(pData2);
    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek(pPeekBuffer, 10, dwDataLen + 2));
}

TEST_F(CmMessageBlockTest, PeekReturnError1Test)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    BYTE pPeekBuffer[1024] = {0};

    ASSERT_TRUE(NULL != m_pMsgBlock);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Peek(NULL, 0, 0));
    EXPECT_TRUE(CM_ERROR_FAILURE == m_pMsgBlock->Peek(NULL, 10, 0));
    EXPECT_TRUE(CM_ERROR_FAILURE == m_pMsgBlock->Peek(pPeekBuffer, 10, 0));
}

TEST_F(CmMessageBlockTest, CombineSingleBufferHas1MbTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello, world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen));

    CCmMessageBlock* pMB = m_pMsgBlock->CombineSingleBuffer();
    ASSERT_TRUE(NULL != pMB);

    EXPECT_TRUE(pMB->GetChainedLength() == m_pMsgBlock->GetChainedLength());
    pMB->DestroyChained();
    pMB = NULL;
}

TEST_F(CmMessageBlockTest, CombineSingleBufferHas2MbTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello, world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen));

    CCmMessageBlock* pMB = new CCmMessageBlock(512, szBuff,0,0);
    ASSERT_TRUE(NULL != pMB);

    EXPECT_TRUE(CM_OK == pMB->Write(szData, dwDataLen));

    m_pMsgBlock->Append(pMB);
    CCmMessageBlock* pDuplicate = m_pMsgBlock->CombineSingleBuffer();

    ASSERT_TRUE(NULL != pDuplicate);

    EXPECT_TRUE(pDuplicate->GetChainedLength() == m_pMsgBlock->GetChainedLength());
    pDuplicate->DestroyChained();
    pDuplicate = NULL;
}

TEST_F(CmMessageBlockTest, FlattenChainedTest)
{
    char szBuff[512] = { 0 };
    m_pMsgBlock = new CCmMessageBlock(512, szBuff,0,0);

    ASSERT_TRUE(NULL != m_pMsgBlock);

    char szData[] = "hello, world!!!";
    DWORD dwDataLen = strlen(szData);

    EXPECT_TRUE(CM_OK == m_pMsgBlock->Write(szData, dwDataLen));

    CCmString strData = m_pMsgBlock->FlattenChained();

    EXPECT_STREQ(szData, strData.c_str());
}

TEST_F(CmMessageBlockTest, ReserveTest)
{
    char szData[] = "0123456789abcdef";
    DWORD dwDataLen = static_cast<DWORD>(strlen(szData));
    CCmMessageBlock mb(dwDataLen, szData, 0, dwDataLen);
    
    LPCSTR pReadPtr     = mb.GetTopLevelReadPtr();
    LPCSTR pWritePtr    = mb.GetTopLevelWritePtr();
    DWORD  dwSpace      = mb.GetTopLevelSpace();
    
    CmResult rv = mb.ReserveCapacity(10);
    ASSERT_TRUE(CM_SUCCEEDED(rv));
    ASSERT_EQ(pReadPtr,     mb.GetTopLevelReadPtr());
    ASSERT_EQ(pWritePtr,    mb.GetTopLevelWritePtr());
    ASSERT_EQ(0,            mb.GetTopLevelSpace());
    
    rv = mb.ReserveCapacity(dwDataLen);
    ASSERT_TRUE(CM_SUCCEEDED(rv));
    ASSERT_EQ(pReadPtr,     mb.GetTopLevelReadPtr());
    ASSERT_EQ(pWritePtr,    mb.GetTopLevelWritePtr());
    ASSERT_EQ(0,            mb.GetTopLevelSpace());
    
    char szBuffer[128] = {0};
    rv = mb.Read(szBuffer, 2);
    ASSERT_TRUE(CM_SUCCEEDED(rv));
    
    rv = mb.ReserveCapacity(20);
    ASSERT_TRUE(CM_SUCCEEDED(rv));
    ASSERT_EQ(14, mb.GetTopLevelLength());
    ASSERT_EQ(4 , mb.GetTopLevelSpace());
    
    char szAppendData[] = "0123";
    rv = mb.Write(szAppendData, 4);
    ASSERT_TRUE(CM_SUCCEEDED(rv));
    
    rv = mb.Read(szBuffer + 2, 18);
    ASSERT_TRUE(CM_SUCCEEDED(rv));
    
    ASSERT_TRUE(CM_SUCCEEDED(rv));
    ASSERT_EQ(0, memcmp(szBuffer, mb.GetTopLevelReadPtr()-20, 20));
}

TEST_F(CmMessageBlockTest, ChainedReserveTest)
{
    char szData[]       = "0123456789";
    const DWORD dwLen   = static_cast<DWORD>(strlen(szData));
    CCmMessageBlock mb1(dwLen, szData, 0, dwLen);
    CCmMessageBlock mb2(dwLen, szData, 0, dwLen);
    mb1.Append(mb2.DuplicateChained());
    
    CmResult rv = mb1.ReserveCapacity(dwLen + 4);
    ASSERT_TRUE(CM_SUCCEEDED(rv));
    rv = mb1.Write("abcd", 4);
    ASSERT_TRUE(CM_SUCCEEDED(rv));
    
    CCmMessageBlock* mbNext = mb1.GetNext();
    rv = mbNext->ReserveCapacity(dwLen + 6);
    ASSERT_TRUE(CM_SUCCEEDED(rv));
    rv = mbNext->Write("abcdef", 6);
    ASSERT_TRUE(CM_SUCCEEDED(rv));
    
    char szBuffer[31] = {0};
    rv = mb1.Read(szBuffer, 30);
    CM_ASSERTE(CM_SUCCEEDED(rv));
    ASSERT_EQ(0, memcmp(szBuffer, "0123456789abcd0123456789abcdef", 30));
    
}

TEST_F(CmMessageBlockTest, ReserveSpaceTest)
{
    char szData[]       = "0123456789xxxx";
    const DWORD dwLen   = static_cast<DWORD>(strlen(szData));
    // the space should be 4
    CCmMessageBlock mb(dwLen, szData, 0, dwLen - 4);
    ASSERT_EQ(4, mb.GetTopLevelSpace());
    
    CmResult rv = mb.ReserveSpace(2);
    ASSERT_TRUE(CM_SUCCEEDED(rv));
    ASSERT_EQ(4, mb.GetTopLevelSpace());
    
    rv = mb.Write("abcd", 4);
    ASSERT_EQ(0, rv);
    ASSERT_EQ(0, mb.GetTopLevelSpace());
    
    rv = mb.ReserveSpace(4);
    ASSERT_TRUE(CM_SUCCEEDED(rv));
    ASSERT_EQ(4, mb.GetTopLevelSpace());
    rv = mb.Write("efgh", 4);
    
    char szBuffer[19] = {0};
    rv = mb.Read(szBuffer, dwLen + 4);
    printf("%s\n", szBuffer);
    ASSERT_TRUE(CM_SUCCEEDED(rv));
    ASSERT_EQ(0, memcmp(szBuffer, "0123456789abcdefgh", dwLen+4));
}
//-------------------------------------------------------}}}
