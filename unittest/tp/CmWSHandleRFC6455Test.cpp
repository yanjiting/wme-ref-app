
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "CmWSHandleRFC6455.h"
#include "CmByteStream.h"

const uint32_t CONST_SIZE_LE_125 = 100;
const uint32_t CONST_SIZE_GT_125_LE_FFFF = 1000;
const uint32_t CONST_SIZE_GT_FFFF = 0x20000;


const char SZ_DATA_TEST1[] = "hello world";
const char SZ_DATA_TEST2[] = "abcdefghijklmnopqrstuvwxyz";
const char SZ_DATA_TEST3[] = "0123456789";

const size_t SZ_DATA_TEST1_LEN = strlen(SZ_DATA_TEST1);
const size_t SZ_DATA_TEST2_LEN = strlen(SZ_DATA_TEST2);
const size_t SZ_DATA_TEST3_LEN = strlen(SZ_DATA_TEST3);


BOOL IsMessageEqual(CCmMessageBlock& src, CCmMessageBlock& dst)
{
    DWORD dwSrcLen = src.GetChainedLength();
    DWORD dwDstLen = dst.GetChainedLength();
    if(dwSrcLen != dwDstLen)
        return FALSE;

    char szSrcData[100] = {0};
    char szDstData[100] = {0};
    const DWORD dwLen100 = 100;

    DWORD dwRead = 0;
    while(dwRead < dwSrcLen)
    {
        DWORD dwLen = CM_MIN(dwLen100, (dwSrcLen-dwRead));
        src.Read((void*)szSrcData, dwLen);
        dst.Read((void*)szDstData, dwLen);
        dwRead += dwLen;

        if(0 != strncmp(szSrcData, szDstData, dwLen))
        {
            return FALSE;
        }
    }

    return TRUE;
}



#ifndef SAFE_DELETE
#define SAFE_DELETE(p)  \
    if(NULL != p)\
{\
    delete p;\
    p = NULL;\
}
#endif

class CmWSFrameHeadRFC6455Test : public testing::Test
{
public:
    CmWSFrameHeadRFC6455Test()
    {

    }

    virtual ~CmWSFrameHeadRFC6455Test()
    {

    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

protected:
    CmWSFrameHeadRFC6455   m_RFC6455;
};


class CmWSHandleRFC6455Test : public testing::Test
{
public:
    CmWSHandleRFC6455Test()
    {
        m_mask_key[0] = 0x37;
        m_mask_key[1] = 0xfa;
        m_mask_key[2] = 0x21;
        m_mask_key[3] = 0x3d;

        m_pMaskData1 = Mask(SZ_DATA_TEST1, SZ_DATA_TEST1_LEN);
        m_pMaskData2 = Mask(SZ_DATA_TEST2, SZ_DATA_TEST2_LEN);
        m_pMaskData3 = Mask(SZ_DATA_TEST3, SZ_DATA_TEST3_LEN);
    }

    virtual ~CmWSHandleRFC6455Test()
    {
        delete [] m_pMaskData1;
        delete [] m_pMaskData2;
        delete [] m_pMaskData3;
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

protected:
    char* Mask(const char *pSrc, size_t szLen)
    {
        if(NULL == pSrc)
            return NULL;

        char* pMasked = new char[szLen + 1];
        for(size_t i = 0; i < szLen; i++)
        {
            pMasked[i] = pSrc[i] ^ m_mask_key[i%4];
        }
        pMasked[szLen] = '\0';
        return pMasked;
    }

protected:
    CmWSHandleRFC6455      m_RFC6455;

    char                   m_mask_key[4];
    char*                  m_pMaskData1;
    char*                  m_pMaskData2;
    char*                  m_pMaskData3;
};

TEST_F(CmWSFrameHeadRFC6455Test, HandleDataMaskTest)
{
    m_RFC6455.m_frame_hdr.u2.HByte.PayloadLen = CONST_SIZE_LE_125;
    m_RFC6455.m_frame_hdr.u2.HByte.Mask = 1;
    m_RFC6455.m_frame_hdr.maskey[0] = 0x11;
    m_RFC6455.m_frame_hdr.maskey[1] = 0x22;
    m_RFC6455.m_frame_hdr.maskey[2] = 0x33;
    m_RFC6455.m_frame_hdr.maskey[3] = 0x44;

    char szBuf[CONST_SIZE_LE_125];
    memset(szBuf, 'a', CONST_SIZE_LE_125-1);
    szBuf[CONST_SIZE_LE_125-1] = '\0';
    CCmMessageBlock aDataIn(CONST_SIZE_LE_125, szBuf, CCmMessageBlock::DONT_DELETE, CONST_SIZE_LE_125);

    CmResult nRet = m_RFC6455.HandleDataMask(aDataIn);
    EXPECT_EQ(CM_OK, nRet);
    for(DWORD i = 0; i <4 ; i++)
    {
        UINT8 uByte = 0;
        aDataIn.Read(&uByte, 1);
        UINT8 uMask = uByte ^ m_RFC6455.m_frame_hdr.maskey[i];
        EXPECT_EQ('a', uMask);
    }
}

TEST_F(CmWSHandleRFC6455Test, Encode125_0Test)
{
    m_RFC6455.SetDataType(E_WS_DTYPE_TEXT);

    char szBuf[CONST_SIZE_LE_125];
    memset(szBuf, 'a', CONST_SIZE_LE_125-1);
    szBuf[CONST_SIZE_LE_125-1] = '\0';
    CCmMessageBlock aDataIn(CONST_SIZE_LE_125, szBuf);

    CCmMessageBlock *pDataOut = NULL;
    EXPECT_TRUE(CM_ERROR_INVALID_ARG == m_RFC6455.Encode(aDataIn, pDataOut));
    EXPECT_TRUE(NULL == pDataOut);
}

TEST_F(CmWSHandleRFC6455Test, Encode125Test)
{
    UINT8  uFirstByte = 0x81;

    m_RFC6455.SetDataType(E_WS_DTYPE_TEXT);

    char szBuf[CONST_SIZE_LE_125];
    memset(szBuf, 'a', CONST_SIZE_LE_125-1);
    szBuf[CONST_SIZE_LE_125-1] = '\0';
    CCmMessageBlock aDataIn(CONST_SIZE_LE_125, szBuf, CCmMessageBlock::DONT_DELETE, CONST_SIZE_LE_125);

    CCmMessageBlock *pDataOut = NULL;
    m_RFC6455.Encode(aDataIn, pDataOut);

    EXPECT_TRUE(NULL != pDataOut);


    UINT8  uActualFirstByte = 0;
    UINT8  uActualSecondByte = 0;
    pDataOut->Read(&uActualFirstByte, sizeof(UINT8));
    pDataOut->Read(&uActualSecondByte, sizeof(UINT8));

    EXPECT_EQ(uFirstByte, uActualFirstByte);
    EXPECT_EQ(CONST_SIZE_LE_125, uActualSecondByte);
}

TEST_F(CmWSHandleRFC6455Test, Encode126Test)
{
    UINT8  uFirstByte = 0x81;
    UINT8  uSecondByte = 126;

    m_RFC6455.SetDataType(E_WS_DTYPE_TEXT);

    char szBuf[CONST_SIZE_GT_125_LE_FFFF];
    memset(szBuf, 'b', CONST_SIZE_GT_125_LE_FFFF-1);
    szBuf[CONST_SIZE_GT_125_LE_FFFF-1] = '\0';
    CCmMessageBlock aDataIn(CONST_SIZE_GT_125_LE_FFFF, szBuf, CCmMessageBlock::DONT_DELETE, CONST_SIZE_GT_125_LE_FFFF);

    CCmMessageBlock *pDataOut = NULL;
    m_RFC6455.Encode(aDataIn, pDataOut);

    EXPECT_TRUE(NULL != pDataOut);


    UINT8  uActualFirstByte = 0;
    UINT8  uActualSecondByte = 0;
    pDataOut->Read(&uActualFirstByte, sizeof(UINT8));
    pDataOut->Read(&uActualSecondByte, sizeof(UINT8));

    EXPECT_EQ(uFirstByte, uActualFirstByte);
    EXPECT_EQ(uSecondByte, uActualSecondByte);
}

TEST_F(CmWSHandleRFC6455Test, Encode127Test)
{
    UINT8  uFirstByte = 0x81;
    UINT8  uSecondByte = 127;

    m_RFC6455.SetDataType(E_WS_DTYPE_TEXT);

    char szBuf[CONST_SIZE_GT_FFFF];
    memset(szBuf, 'c', CONST_SIZE_GT_FFFF-1);
    szBuf[CONST_SIZE_GT_FFFF-1] = '\0';
    CCmMessageBlock aDataIn(CONST_SIZE_GT_FFFF, szBuf, CCmMessageBlock::DONT_DELETE, CONST_SIZE_GT_FFFF);

    CCmMessageBlock *pDataOut = NULL;
    m_RFC6455.Encode(aDataIn, pDataOut);

    EXPECT_TRUE(NULL != pDataOut);


    UINT8  uActualFirstByte = 0;
    UINT8  uActualSecondByte = 0;
    pDataOut->Read(&uActualFirstByte, sizeof(UINT8));
    pDataOut->Read(&uActualSecondByte, sizeof(UINT8));

    EXPECT_EQ(uFirstByte, uActualFirstByte);
    EXPECT_EQ(uSecondByte, uActualSecondByte);
}


TEST_F(CmWSHandleRFC6455Test, DataTypeTest2)
{
    UINT8  uFirstByte = 0x82;
    m_RFC6455.SetDataType(E_WS_DTYPE_BINARY);

    char szBuf[CONST_SIZE_LE_125];
    memset(szBuf, 'a', CONST_SIZE_LE_125-1);
    szBuf[CONST_SIZE_LE_125-1] = '\0';
    CCmMessageBlock aDataIn(CONST_SIZE_LE_125, szBuf, CCmMessageBlock::DONT_DELETE, CONST_SIZE_LE_125);

    CCmMessageBlock *pDataOut = NULL;
    m_RFC6455.Encode(aDataIn, pDataOut);

    EXPECT_TRUE(NULL != pDataOut);


    UINT8  uActualFirstByte = 0;
    UINT8  uActualSecondByte = 0;
    pDataOut->Read(&uActualFirstByte, sizeof(UINT8));
    pDataOut->Read(&uActualSecondByte, sizeof(UINT8));

    EXPECT_EQ(uFirstByte, uActualFirstByte);
    EXPECT_EQ(CONST_SIZE_LE_125, uActualSecondByte);
}

TEST_F(CmWSHandleRFC6455Test, DataTypeTest3)
{
    UINT8  uFirstByte = 0x81;
    m_RFC6455.SetDataType(E_WS_DTYPE_BASE64);

    char szBuf[CONST_SIZE_LE_125];
    memset(szBuf, 'a', CONST_SIZE_LE_125-1);
    szBuf[CONST_SIZE_LE_125-1] = '\0';
    CCmMessageBlock aDataIn(CONST_SIZE_LE_125, szBuf, CCmMessageBlock::DONT_DELETE, CONST_SIZE_LE_125);

    CCmMessageBlock *pDataOut = NULL;
    m_RFC6455.Encode(aDataIn, pDataOut);

    EXPECT_TRUE(NULL != pDataOut);


    UINT8  uActualFirstByte = 0;
    UINT8  uActualSecondByte = 0;
    pDataOut->Read(&uActualFirstByte, sizeof(UINT8));
    pDataOut->Read(&uActualSecondByte, sizeof(UINT8));

    EXPECT_EQ(uFirstByte, uActualFirstByte);
    EXPECT_EQ(CONST_SIZE_LE_125, uActualSecondByte);
}

//A single-frame unmasked text message
//0x81 0x05 0x48 0x65 0x6c 0x6c 0x6f (contains "Hello")

TEST_F(CmWSHandleRFC6455Test, Decode126Test)
{
    char szBuf[500] = {0};
    memset(szBuf, 'e', 500);
    szBuf[499] = '\0';

    UINT8  uFirstByte = 0x81;
    UINT8  uSecondByte = 126;

    CCmMessageBlock aHead(4);
    CCmMessageBlock aData(strlen(szBuf));
    aData.Write(szBuf, strlen(szBuf));

    UINT16 uDataLen = (UINT16)aData.GetChainedLength();

    CCmByteStreamNetwork networkStream(aHead);

    networkStream << uFirstByte;
    networkStream << uSecondByte;
    networkStream << uDataLen;

    aHead.Append(aData.DuplicateChained());

    CCmMessageBlock* aDataOut = NULL;

    m_RFC6455.SetDataType(E_WS_DTYPE_TEXT);

    EXPECT_EQ(CM_OK, m_RFC6455.Decode(aHead, aDataOut));
    EXPECT_TRUE(IsMessageEqual(aData, *aDataOut));

    aDataOut->DestroyChained();
    aDataOut = NULL;
}

TEST_F(CmWSHandleRFC6455Test, Decode127Test)
{
    char szBuf[0x10000] = {0};
    memset(szBuf, 'f', 0x10000);
    szBuf[0x10000 - 1] = '\0';

    UINT8  uFirstByte = 0x81;
    UINT8  uSecondByte = 127;

    CCmMessageBlock aHead(10);
    CCmMessageBlock aData(strlen(szBuf));
    aData.Write(szBuf, strlen(szBuf));

    unsigned long long uDataLen = aData.GetChainedLength();

    CCmByteStreamNetwork networkStream(aHead);

    networkStream << uFirstByte;
    networkStream << uSecondByte;
    networkStream << uDataLen;

    aHead.Append(aData.DuplicateChained());

    CCmMessageBlock* aDataOut = NULL;

    m_RFC6455.SetDataType(E_WS_DTYPE_TEXT);

    EXPECT_EQ(CM_OK, m_RFC6455.Decode(aHead, aDataOut));

    EXPECT_TRUE(IsMessageEqual(aData, *aDataOut));

    ASSERT_TRUE(NULL != aDataOut);
    aDataOut->DestroyChained();
}

TEST_F(CmWSHandleRFC6455Test, Decode127TestMask)
{
    char* pBuf = new char[565633];
    memset(pBuf, 'f', 565633);
    pBuf[565632] = '\0';

    UINT8  uFirstByte = 0x81;
    UINT8  uSecondByte = 0xff;

    DWORD dwSize = 565632;
    char* pMask = Mask(pBuf, dwSize);

    CCmMessageBlock aHead(14);
    CCmMessageBlock aData(dwSize);
    aData.Write(pBuf, dwSize);

    CCmMessageBlock aDataMask(dwSize);
    aDataMask.Write(pMask, dwSize);

    unsigned long long uDataLen = aDataMask.GetChainedLength();

    CCmByteStreamNetwork networkStream(aHead);

    networkStream << uFirstByte;
    networkStream << uSecondByte;
    networkStream << uDataLen;

    networkStream << m_mask_key[0];
    networkStream << m_mask_key[1];
    networkStream << m_mask_key[2];
    networkStream << m_mask_key[3];

    aHead.Append(&aDataMask);

    CCmMessageBlock* aDataOut = NULL;

    m_RFC6455.SetDataType(E_WS_DTYPE_TEXT);

    EXPECT_EQ(CM_OK, m_RFC6455.Decode(aHead, aDataOut));
    EXPECT_TRUE(IsMessageEqual(aData, *aDataOut));

    delete [] pMask;
    pMask = NULL;
    delete [] pBuf;
    pBuf = NULL;

    ASSERT_TRUE(NULL != aDataOut);
    aDataOut->DestroyChained();
}

TEST_F(CmWSHandleRFC6455Test, Decode126ErrorTest)
{
    char szBuf[500] = {0};
    memset(szBuf, 'e', 500);
    szBuf[499] = '\0';

    UINT8  uFirstByte = 0x81;
    UINT8  uSecondByte = 126;

    CCmMessageBlock aHead(2);
    CCmMessageBlock aData(strlen(szBuf));
    aData.Write(szBuf, strlen(szBuf));

    CCmByteStreamNetwork networkStream(aHead);
    networkStream << uFirstByte;
    networkStream << uSecondByte;

    aHead.Append(&aData);

    CCmMessageBlock* aDataIn = aHead.DuplicateChained();
    CCmMessageBlock* aDataOut = NULL;

    m_RFC6455.SetDataType(E_WS_DTYPE_TEXT);

    EXPECT_EQ(CM_OK, m_RFC6455.Decode(*aDataIn, aDataOut));

    SAFE_DELETE(aDataIn);
    SAFE_DELETE(aDataOut);
}

TEST_F(CmWSHandleRFC6455Test, Decode1Msg1PktTest)
{
    UINT8  uFirstByte = 0x81;
    UINT8  uSecondByte = SZ_DATA_TEST1_LEN;

    CCmMessageBlock aHead(2);
    CCmMessageBlock aData(SZ_DATA_TEST1_LEN);

    aHead.Write(&uFirstByte, sizeof(UINT8));
    aHead.Write(&uSecondByte, sizeof(UINT8));

    aData.Write(SZ_DATA_TEST1, SZ_DATA_TEST1_LEN);
    aHead.Append(&aData);

    CCmMessageBlock* aDataIn = aHead.DuplicateChained();
    CCmMessageBlock* aDataOut = NULL;

    m_RFC6455.SetDataType(E_WS_DTYPE_TEXT);

    EXPECT_EQ(CM_OK, m_RFC6455.Decode(*aDataIn, aDataOut));
    EXPECT_TRUE(IsMessageEqual(aData, *aDataOut));

    SAFE_DELETE(aDataIn);
    SAFE_DELETE(aDataOut);
}

TEST_F(CmWSHandleRFC6455Test, Decode2Msg1PktTest)
{
    UINT8  uFirstByte = 0x81;
    DWORD  dwDataLen = SZ_DATA_TEST1_LEN + SZ_DATA_TEST2_LEN;

    CCmMessageBlock aHead(2);
    CCmMessageBlock aData1(SZ_DATA_TEST1_LEN);
    CCmMessageBlock aData2(SZ_DATA_TEST2_LEN);

    aHead.Write(&uFirstByte, sizeof(UINT8));
    aHead.Write(&dwDataLen, sizeof(UINT8));

    aData1.Write(SZ_DATA_TEST1, SZ_DATA_TEST1_LEN);
    aData2.Write(SZ_DATA_TEST2, SZ_DATA_TEST2_LEN);

    CCmMessageBlock* aExpect = aData1.DuplicateChained();
    aExpect->Append(aData2.DuplicateChained());

    aHead.Append(&aData1);

    CCmMessageBlock* aDataOut = NULL;
    m_RFC6455.SetDataType(E_WS_DTYPE_TEXT);

    EXPECT_EQ(CM_OK, m_RFC6455.Decode(aHead, aDataOut));
    EXPECT_TRUE(NULL == aDataOut);

    EXPECT_EQ(CM_OK, m_RFC6455.Decode(aData2, aDataOut));

    EXPECT_TRUE(IsMessageEqual(*aExpect, *aDataOut));

    SAFE_DELETE(aExpect);
    SAFE_DELETE(aDataOut);
}

TEST_F(CmWSHandleRFC6455Test, Decode1Msg2PktTest)
{
    UINT8  uFirstByte = 0x81;

    CCmMessageBlock aHead1(2);
    CCmMessageBlock aData1(SZ_DATA_TEST1_LEN);

    aHead1.Write(&uFirstByte, sizeof(UINT8));
    aHead1.Write(&SZ_DATA_TEST1_LEN, sizeof(UINT8));
    aData1.Write(SZ_DATA_TEST1, SZ_DATA_TEST1_LEN);

    CCmMessageBlock aHead2(2);
    CCmMessageBlock aData2(SZ_DATA_TEST2_LEN);

    aHead2.Write(&uFirstByte, sizeof(UINT8));
    aHead2.Write(&SZ_DATA_TEST2_LEN, sizeof(UINT8));
    aData2.Write(SZ_DATA_TEST2, SZ_DATA_TEST2_LEN);

    CCmMessageBlock* aDataIn = aHead1.DuplicateChained();
    CCmMessageBlock* aDataOut = NULL;

    CCmMessageBlock* aExpect = aData1.DuplicateChained();
    aExpect->Append(aData2.DuplicateChained());

    aDataIn->Append(&aData1);
    aDataIn->Append(&aHead2);
    aDataIn->Append(&aData2);

    m_RFC6455.SetDataType(E_WS_DTYPE_TEXT);

    EXPECT_EQ(CM_OK, m_RFC6455.Decode(*aDataIn, aDataOut));
    EXPECT_TRUE(IsMessageEqual(*aExpect, *aDataOut));

    int nMessageCount = 0;
    CCmMessageBlock* pNext = aDataOut;
    while(NULL != pNext)
    {
        nMessageCount++;
        pNext = pNext->GetNext();
    }
    EXPECT_EQ(2, nMessageCount);


    SAFE_DELETE(aExpect);
    SAFE_DELETE(aDataIn);
    SAFE_DELETE(aDataOut);
}

TEST_F(CmWSHandleRFC6455Test, Decode1Msg1PktMaskTest)
{
    UINT8  uFirstByte = 0x81;
    UINT8  uDataLen = SZ_DATA_TEST1_LEN;
    UINT8  uSecondByte = 0x80 | uDataLen;

    CCmMessageBlock aHead(6);
    CCmMessageBlock aData(SZ_DATA_TEST1_LEN);

    aHead.Write(&uFirstByte, sizeof(UINT8));
    aHead.Write(&uSecondByte, sizeof(UINT8));
    aHead.Write(m_mask_key, 4);

    aData.Write(m_pMaskData1, SZ_DATA_TEST1_LEN);
    aHead.Append(&aData);

    CCmMessageBlock* aDataIn = aHead.DuplicateChained();
    CCmMessageBlock* aDataOut = NULL;

    m_RFC6455.SetDataType(E_WS_DTYPE_TEXT);

    EXPECT_EQ(CM_OK, m_RFC6455.Decode(*aDataIn, aDataOut));

    CCmMessageBlock aExpect(SZ_DATA_TEST1_LEN, SZ_DATA_TEST1, CCmMessageBlock::DONT_DELETE, SZ_DATA_TEST1_LEN);
    EXPECT_TRUE(IsMessageEqual(aExpect, *aDataOut));

    SAFE_DELETE(aDataIn);
    SAFE_DELETE(aDataOut);
}

TEST_F(CmWSHandleRFC6455Test, Decode2Msg1PktMaskTest)
{
    UINT8  uFirstByte = 0x81;
    UINT8  uDataLen = SZ_DATA_TEST2_LEN;
    UINT8  uSecondByte = 0x80 | uDataLen;

    CCmMessageBlock aHead(6);
    CCmMessageBlock aData1(SZ_DATA_TEST2_LEN/2);
    CCmMessageBlock aData2(SZ_DATA_TEST2_LEN- SZ_DATA_TEST2_LEN/2);

    aHead.Write(&uFirstByte, sizeof(UINT8));
    aHead.Write(&uSecondByte, sizeof(UINT8));
    aHead.Write(m_mask_key, 4);


    aData1.Write(m_pMaskData2, SZ_DATA_TEST2_LEN/2);
    aData2.Write((m_pMaskData2+SZ_DATA_TEST2_LEN/2), SZ_DATA_TEST2_LEN- SZ_DATA_TEST2_LEN/2);

    aHead.Append(&aData1);

    CCmMessageBlock aExpect(SZ_DATA_TEST2_LEN, SZ_DATA_TEST2, CCmMessageBlock::DONT_DELETE, SZ_DATA_TEST2_LEN);


    CCmMessageBlock* aDataOut = NULL;
    m_RFC6455.SetDataType(E_WS_DTYPE_TEXT);

    EXPECT_EQ(CM_OK, m_RFC6455.Decode(aHead, aDataOut));
    EXPECT_TRUE(NULL == aDataOut);

    EXPECT_EQ(CM_OK, m_RFC6455.Decode(aData2, aDataOut));

    EXPECT_TRUE(IsMessageEqual(aExpect, *aDataOut));

    SAFE_DELETE(aDataOut);
}

TEST_F(CmWSHandleRFC6455Test, Decode1Msg2PktMaskTest)
{
    UINT8 uFirstByte = 0x81;
    UINT8 uSecondByte = 0x80 | SZ_DATA_TEST1_LEN;

    CCmMessageBlock aHead1(6);
    CCmMessageBlock aData1(SZ_DATA_TEST1_LEN);

    aHead1.Write(&uFirstByte, sizeof(UINT8));
    aHead1.Write(&uSecondByte, sizeof(UINT8));
    aHead1.Write(m_mask_key, 4);
    aData1.Write(m_pMaskData1, SZ_DATA_TEST1_LEN);

    CCmMessageBlock aHead2(6);
    CCmMessageBlock aData2(SZ_DATA_TEST2_LEN);
    uSecondByte = 0x80 | SZ_DATA_TEST2_LEN;

    aHead2.Write(&uFirstByte, sizeof(UINT8));
    aHead2.Write(&uSecondByte, sizeof(UINT8));
    aHead2.Write(m_mask_key, 4);
    aData2.Write(m_pMaskData2, SZ_DATA_TEST2_LEN);

    CCmMessageBlock* aDataIn = aHead1.DuplicateChained();
    CCmMessageBlock* aDataOut = NULL;

    CCmMessageBlock aExpect(SZ_DATA_TEST1_LEN, SZ_DATA_TEST1, CCmMessageBlock::DONT_DELETE, SZ_DATA_TEST1_LEN);
    CCmMessageBlock aExpect2(SZ_DATA_TEST2_LEN, SZ_DATA_TEST2, CCmMessageBlock::DONT_DELETE, SZ_DATA_TEST2_LEN);
    aExpect.Append(&aExpect2);


    aDataIn->Append(&aData1);
    aDataIn->Append(&aHead2);
    aDataIn->Append(&aData2);

    m_RFC6455.SetDataType(E_WS_DTYPE_TEXT);

    EXPECT_EQ(CM_OK, m_RFC6455.Decode(*aDataIn, aDataOut));
    EXPECT_TRUE(IsMessageEqual(aExpect, *aDataOut));

    SAFE_DELETE(aDataIn);
    SAFE_DELETE(aDataOut);
}