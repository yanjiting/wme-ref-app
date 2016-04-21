#include "share_common_frame_impl.h"
#include "gtest/gtest.h"
#include "share_define.h"

class CShareFrameTest : public testing::Test
{
public:
    CShareFrameTest()
    {
    }
    ~CShareFrameTest()
    {
    }
    virtual void SetUp()
    {
        m_pCShareFrame = new CShareFrame;
        m_pCShareFrame->AddRef();
    }
    virtual void TearDown()
    {
        SAFE_RELEASE(m_pCShareFrame);
    }
protected:
    CShareFrame *m_pCShareFrame;
    WBXByte pBuf[1024*768*4];
    WBXByte pBuf1[1024*768*5];
};

TEST_F(CShareFrameTest, SetExternalBuf)
{
    if (m_pCShareFrame==NULL) { return ; }

    memset(pBuf,0,sizeof(pBuf));

    ASSERT_TRUE(WBX_FAIL==m_pCShareFrame->SetExternalBuf(WBXNull,0));
    ASSERT_TRUE(WBX_FAIL==m_pCShareFrame->SetExternalBuf(WBXNull,1000));

    ASSERT_TRUE(WBX_SUCCESS==m_pCShareFrame->SetExternalBuf(pBuf,sizeof(pBuf)));
    WBXByte *pFrameData =  m_pCShareFrame->GetFrameData();
    ASSERT_TRUE(pFrameData==pBuf);

    char *pTest = "this is test";
    memcpy(pBuf,pTest,strlen(pTest));
    ASSERT_TRUE(0==memcmp(pFrameData,pBuf,strlen(pTest)));

    ASSERT_TRUE(WBX_SUCCESS==m_pCShareFrame->SetExternalBuf(pFrameData,sizeof(pBuf)));
    pFrameData =  m_pCShareFrame->GetFrameData();
    ASSERT_TRUE(0==memcmp(pFrameData,pTest,strlen(pTest)));
}

TEST_F(CShareFrameTest, GetFrameData)
{
    if (m_pCShareFrame==NULL) { return ; }
    WBXByte *pFrameData = m_pCShareFrame->GetFrameData();

    //if size is null, the data is null too.
    ASSERT_TRUE(pFrameData==WBXNull);

    // set size is
    WBXSize size(1024,768);
    m_pCShareFrame->SetFrameSize(size);
    pFrameData = m_pCShareFrame->GetFrameData();
    ASSERT_TRUE(pFrameData==WBXNull);

    //can get frame data
    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_RGBA);
    pFrameData = m_pCShareFrame->GetFrameData();
    ASSERT_TRUE(pFrameData!=WBXNull);

}
TEST_F(CShareFrameTest, GetFrameLineData)
{
    if (m_pCShareFrame==NULL) { return ; }

    WBXByte *pFrameData = m_pCShareFrame->GetFrameLineData(0);

    //if size is null, the data is null too.
    ASSERT_TRUE(pFrameData==WBXNull);

    // set size is
    WBXSize size(1024,768);
    m_pCShareFrame->SetFrameSize(size);
    pFrameData = m_pCShareFrame->GetFrameLineData(0);
    ASSERT_TRUE(pFrameData==WBXNull);

    //can get frame data
    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_RGBA);
    pFrameData = m_pCShareFrame->GetFrameLineData(0);
    ASSERT_TRUE(pFrameData!=WBXNull);

    //
    char *pTest = "this is test";
    m_pCShareFrame->WriteFrameData((WBXByte *)pTest,strlen(pTest));
    pFrameData = m_pCShareFrame->GetFrameLineData(0);
    ASSERT_TRUE(0==memcmp(pFrameData,pTest,strlen(pTest)));

}
TEST_F(CShareFrameTest, GetFrameDataLen)
{
    if (m_pCShareFrame==NULL) { return ; }

    WBXSize size(1024,768);
    m_pCShareFrame->SetFrameSize(size);
    WBXInt nFrameDataLen = 0;

    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_RGB);
    nFrameDataLen = m_pCShareFrame->GetFrameDataLen();
    EXPECT_TRUE(nFrameDataLen==1024*768*3);

    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_ARGB);
    nFrameDataLen = m_pCShareFrame->GetFrameDataLen();
    EXPECT_TRUE(nFrameDataLen==1024*768*4);

    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_RGBA);
    nFrameDataLen = m_pCShareFrame->GetFrameDataLen();
    EXPECT_TRUE(nFrameDataLen==1024*768*4);

}
TEST_F(CShareFrameTest, GetFrameLineBytes)
{
    if (m_pCShareFrame==NULL) { return ; }

    WBXSize size(1024,768);
    m_pCShareFrame->SetFrameSize(size);
    WBXInt nFrameLineBytes = 0;

    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_RGB);
    nFrameLineBytes = m_pCShareFrame->GetFrameLineBytes();
    EXPECT_TRUE(nFrameLineBytes==1024*3);

    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_ARGB);
    nFrameLineBytes = m_pCShareFrame->GetFrameLineBytes();
    EXPECT_TRUE(nFrameLineBytes==1024*4);

    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_RGBA);
    nFrameLineBytes = m_pCShareFrame->GetFrameLineBytes();
    EXPECT_TRUE(nFrameLineBytes==1024*4);
}
TEST_F(CShareFrameTest, SetFrameLineBytes)
{
    if (m_pCShareFrame==NULL) { return ; }

    WBXSize size(1024,768);
    m_pCShareFrame->SetFrameSize(size);
    WBXInt nFrameLineBytes = 0;

    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_RGB);
    EXPECT_TRUE(m_pCShareFrame->SetFrameLineBytes(1024*8)==WBX_SUCCESS);
    nFrameLineBytes = m_pCShareFrame->GetFrameLineBytes();
    EXPECT_TRUE(nFrameLineBytes==1024*8);

    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_ARGB);
    EXPECT_TRUE(m_pCShareFrame->SetFrameLineBytes(1024*8)==WBX_FAIL);
    nFrameLineBytes = m_pCShareFrame->GetFrameLineBytes();
    EXPECT_TRUE(nFrameLineBytes==1024*4);

    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_RGBA);
    EXPECT_TRUE(m_pCShareFrame->SetFrameLineBytes(1024*8)==WBX_FAIL);
    nFrameLineBytes = m_pCShareFrame->GetFrameLineBytes();
    EXPECT_TRUE(nFrameLineBytes==1024*4);

    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_RGB);
    nFrameLineBytes = m_pCShareFrame->GetFrameLineBytes();
    EXPECT_TRUE(nFrameLineBytes==1024*3);
}
TEST_F(CShareFrameTest, SetFrameDataFormat_GetFrameDataFormat)
{
    if (m_pCShareFrame==NULL) { return ; }

    EXPECT_TRUE(m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_RGB)==SHARE_FRAME_FORMAT_RGB);
    EXPECT_TRUE(m_pCShareFrame->GetFrameDataFormat()==SHARE_FRAME_FORMAT_RGB);
    EXPECT_TRUE(m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_ARGB)==SHARE_FRAME_FORMAT_ARGB);
    EXPECT_TRUE(m_pCShareFrame->GetFrameDataFormat()==SHARE_FRAME_FORMAT_ARGB);
    EXPECT_TRUE(m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_RGBA)==SHARE_FRAME_FORMAT_RGBA);
    EXPECT_TRUE(m_pCShareFrame->GetFrameDataFormat()==SHARE_FRAME_FORMAT_RGBA);
    EXPECT_TRUE(m_pCShareFrame->SetFrameDataFormat((SHARE_FRAME_FORMAT)(SHARE_FRAME_FORMAT_NULL-1))==SHARE_FRAME_FORMAT_RGBA);
    EXPECT_TRUE(m_pCShareFrame->GetFrameDataFormat()==SHARE_FRAME_FORMAT_RGBA);
    EXPECT_TRUE(m_pCShareFrame->SetFrameDataFormat((SHARE_FRAME_FORMAT)SHARE_FRAME_FORMAT_END_NULL)==SHARE_FRAME_FORMAT_RGBA);
    EXPECT_TRUE(m_pCShareFrame->GetFrameDataFormat()==SHARE_FRAME_FORMAT_RGBA);
    EXPECT_TRUE(m_pCShareFrame->SetFrameDataFormat((SHARE_FRAME_FORMAT)(SHARE_FRAME_FORMAT_END_NULL+1))==SHARE_FRAME_FORMAT_RGBA);
    EXPECT_TRUE(m_pCShareFrame->GetFrameDataFormat()==SHARE_FRAME_FORMAT_RGBA);
}
TEST_F(CShareFrameTest, SetFrameSize_GetFrameSize)
{
    if (m_pCShareFrame==NULL) { return ; }
    {
        WBXSize size(1024,768);
        WBXSize sizeGet(0,0);
        EXPECT_TRUE(m_pCShareFrame->SetFrameSize(size.cx,size.cy)==WBX_SUCCESS);
        EXPECT_TRUE(m_pCShareFrame->GetFrameSize(sizeGet)==WBX_SUCCESS);
        EXPECT_TRUE(sizeGet.cx==size.cx);
        EXPECT_TRUE(sizeGet.cy==size.cy);
        EXPECT_TRUE(m_pCShareFrame->GetFrameSize(sizeGet.cx,sizeGet.cy)==WBX_SUCCESS);
        EXPECT_TRUE(sizeGet.cx==size.cx);
        EXPECT_TRUE(sizeGet.cy==size.cy);
    }

    {
        WBXSize size(1024,768);
        WBXSize sizeGet(0,0);
        EXPECT_TRUE(m_pCShareFrame->SetFrameSize(size)==WBX_SUCCESS);
        EXPECT_TRUE(m_pCShareFrame->GetFrameSize(sizeGet)==WBX_SUCCESS);
        EXPECT_TRUE(sizeGet.cx==size.cx);
        EXPECT_TRUE(sizeGet.cy==size.cy);
        EXPECT_TRUE(m_pCShareFrame->GetFrameSize(sizeGet.cx,sizeGet.cy)==WBX_SUCCESS);
        EXPECT_TRUE(sizeGet.cx==size.cx);
        EXPECT_TRUE(sizeGet.cy==size.cy);
    }
}
TEST_F(CShareFrameTest, WriteFrameData_ReadFrameData)
{
    if (m_pCShareFrame==NULL) { return ; }
    memset(pBuf,0,sizeof(pBuf));
    memset(pBuf1,1,sizeof(pBuf1));

    //empty frame.
    EXPECT_TRUE(m_pCShareFrame->WriteFrameData(WBXNull,0)<=0);
    EXPECT_TRUE(m_pCShareFrame->WriteFrameData(pBuf,sizeof(pBuf))<=0);
    EXPECT_TRUE(m_pCShareFrame->ReadFrameData(WBXNull,0)<=0);
    EXPECT_TRUE(m_pCShareFrame->ReadFrameData(pBuf,sizeof(pBuf))<=0);


    //set frame info
    WBXSize size(1024,768);
    m_pCShareFrame->SetFrameSize(size);
    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_ARGB);

    //Invalidate Param
    EXPECT_TRUE(m_pCShareFrame->WriteFrameData(pBuf1,sizeof(pBuf1))<=0);
    EXPECT_TRUE(m_pCShareFrame->WriteFrameData(WBXNull,0)<=0);
    EXPECT_TRUE(m_pCShareFrame->WriteFrameData(pBuf,0)<=0);
    EXPECT_TRUE(m_pCShareFrame->ReadFrameData(WBXNull,0)<=0);
    EXPECT_TRUE(m_pCShareFrame->ReadFrameData(pBuf,0)<=0);

    //
    EXPECT_TRUE(m_pCShareFrame->WriteFrameData(pBuf1,sizeof(pBuf))==sizeof(pBuf));
    EXPECT_TRUE(m_pCShareFrame->ReadFrameData(pBuf,sizeof(pBuf))==sizeof(pBuf));
    EXPECT_TRUE(memcmp(pBuf,pBuf1,sizeof(pBuf))==0);
}
TEST_F(CShareFrameTest, WriteFrameLineData_ReadFrameLineData)
{
    if (m_pCShareFrame==NULL) { return ; }

    //set frame info
    WBXSize size(8,8);
    m_pCShareFrame->SetFrameSize(size);
    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_RGBA);

    WBXByte pBuf[8][8][4] = {
        {{0x01,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
        {{0x02,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
        {{0x03,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
        {{0x04,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
        {{0x05,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
        {{0x06,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
        {{0x07,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
        {{0x08,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
    };
    WBXByte pBufLine[8*4]= {0};
    EXPECT_TRUE(m_pCShareFrame->WriteFrameData((const WBXByte *)pBuf,sizeof(pBuf))==sizeof(pBuf));

    EXPECT_TRUE(m_pCShareFrame->ReadFrameLineData(1,pBufLine,sizeof(pBufLine))==sizeof(pBufLine));
    EXPECT_TRUE(memcmp(pBuf[1],pBufLine,sizeof(pBufLine))==0);

    EXPECT_TRUE(m_pCShareFrame->WriteFrameLineData(8,(const WBXByte *)pBuf[2],8*4)<=0);
    EXPECT_TRUE(m_pCShareFrame->WriteFrameLineData(1,(const WBXByte *)pBuf[2],8*2)<=0);
    EXPECT_TRUE(m_pCShareFrame->WriteFrameLineData(1,(const WBXByte *)pBuf[2],8*8)== 8*4);
    EXPECT_TRUE(m_pCShareFrame->WriteFrameLineData(1,(const WBXByte *)pBuf[2],8*4)== 8*4);

    EXPECT_TRUE(m_pCShareFrame->ReadFrameLineData(1,pBufLine,sizeof(pBufLine))==sizeof(pBufLine));
    EXPECT_TRUE(memcmp(pBuf[2],pBufLine,sizeof(pBufLine))==0);
}

TEST_F(CShareFrameTest, WriteFrameAreaData_ReadFrameAreaData)
{
    if (m_pCShareFrame==NULL) { return ; }


    //set frame info
    WBXSize size(8,8);
    m_pCShareFrame->SetFrameSize(size);
    m_pCShareFrame->SetFrameDataFormat(SHARE_FRAME_FORMAT_RGBA);

    WBXByte pBuf[8][8][4] = {
        {{0x01,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
        {{0x02,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
        {{0x03,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
        {{0x04,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
        {{0x05,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
        {{0x06,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
        {{0x07,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
        {{0x08,0xFF,0xFF,0xFF},{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF},{0x06,0xFF,0xFF,0xFF},{0x07,0xFF,0xFF,0xFF},{0x08,0xFF,0xFF,0xFF}},
    };


    WBXByte pBufArea_1_1_5_5[4][4][4] = {
        {{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF}},
        {{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF}},
        {{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF}},
        {{0x02,0xFF,0xFF,0xFF},{0x03,0xFF,0xFF,0xFF},{0x04,0xFF,0xFF,0xFF},{0x05,0xFF,0xFF,0xFF}}
    };

    WBXByte pBufAreaSrc[4][4][4] = {
        {{0x22,0xFF,0xFF,0xFF},{0x22,0xFF,0xFF,0xFF},{0x22,0xFF,0xFF,0xFF},{0x22,0xFF,0xFF,0xFF}},
        {{0x22,0xFF,0xFF,0xFF},{0x22,0xFF,0xFF,0xFF},{0x22,0xFF,0xFF,0xFF},{0x22,0xFF,0xFF,0xFF}},
        {{0x22,0xFF,0xFF,0xFF},{0x22,0xFF,0xFF,0xFF},{0x22,0xFF,0xFF,0xFF},{0x22,0xFF,0xFF,0xFF}},
        {{0x22,0xFF,0xFF,0xFF},{0x22,0xFF,0xFF,0xFF},{0x22,0xFF,0xFF,0xFF},{0x22,0xFF,0xFF,0xFF}}
    };
    WBXByte pBufAreaRead[4][4][4] = {0};

    EXPECT_TRUE(m_pCShareFrame->WriteFrameData((const WBXByte *)pBuf,sizeof(pBuf))==sizeof(pBuf));

    WBXRect rcArea(1,1,5,5);
    EXPECT_TRUE(m_pCShareFrame->ReadFrameAreaData(rcArea,(WBXByte *)pBufAreaRead,sizeof(pBufAreaRead))==sizeof(pBufAreaRead));
    EXPECT_TRUE(memcmp(pBufAreaRead,pBufArea_1_1_5_5,sizeof(pBufAreaRead))==0);

    EXPECT_TRUE(m_pCShareFrame->WriteFrameAreaData(rcArea,(WBXByte *)pBufAreaSrc,sizeof(pBufAreaSrc))==sizeof(pBufAreaSrc));
    EXPECT_TRUE(m_pCShareFrame->ReadFrameAreaData(rcArea,(WBXByte *)pBufAreaRead,sizeof(pBufAreaRead))==sizeof(pBufAreaRead));
    EXPECT_TRUE(memcmp(pBufAreaRead,pBufAreaSrc,sizeof(pBufAreaRead))==0);

}