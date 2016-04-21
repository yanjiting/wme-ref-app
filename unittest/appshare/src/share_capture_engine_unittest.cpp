/*
============================================================================================================
        CaptureEngine_API_Test---> CShareCaptureEngineTest --> MockIShareCaptureEngineSink
                |                           |
        CaptureEngineAPI   -->  FackCShareCaptureEngine[CShareCaptureEngine]
                                 |              |                   |
                          FakeSource        FakeCapturer,       FakePointing,
============================================================================================================
*/
#include "share_capture_engine_impl.h"
#include "gtest/gtest.h"
#include "share_capture_engine_mock.h"
#include "share_util.h"

#include "appshare_fake_common.h"

using ::testing::_;
using ::testing::AnyNumber;
using ::testing::Return;
using ::testing::SaveArg;



//=====================================================
//CaptureEngine_API_Test testing
TEST(share_capture_engine,CreateShareCaptureEngine_DestroyShareCaptureEngine)
{
    IShareCaptureEngine *pIShareCaptureEngine = WBXNull;

    //
    pIShareCaptureEngine = CreateShareCaptureEngine(SHARE_CAPTURE_ENGINE_TYPE_DESKTOP);
    ASSERT_TRUE(pIShareCaptureEngine!=WBXNull);
    ASSERT_TRUE(DestroyShareCaptureEngine(pIShareCaptureEngine)==WBX_SUCCESS);
    pIShareCaptureEngine=NULL;

    //
    pIShareCaptureEngine = CreateShareCaptureEngine(SHARE_CAPTURE_ENGINE_TYPE_APPSHARE);
    ASSERT_TRUE(pIShareCaptureEngine!=WBXNull);
    ASSERT_TRUE(DestroyShareCaptureEngine(pIShareCaptureEngine)==WBX_SUCCESS);
    pIShareCaptureEngine=NULL;

    //
    pIShareCaptureEngine = CreateShareCaptureEngine(SHARE_CAPTURE_ENGINE_TYPE_USER_CUSTOMIZATION);
    ASSERT_TRUE(pIShareCaptureEngine!=WBXNull);
    ASSERT_TRUE(DestroyShareCaptureEngine(pIShareCaptureEngine)==WBX_SUCCESS);
    pIShareCaptureEngine=NULL;

    //
    pIShareCaptureEngine = CreateShareCaptureEngine(SHARE_CAPTURE_ENGINE_TYPE_NULL);
    ASSERT_TRUE(pIShareCaptureEngine==WBXNull);
    ASSERT_TRUE(DestroyShareCaptureEngine(pIShareCaptureEngine)==WBX_ERR_PARAM);
    pIShareCaptureEngine=NULL;
}
//======================================================================
//Fack

#define UT_CAPTURE_WIDTH 1024
#define UT_CAPTURE_HEIGH 768
#define UT_CAPTURE_BUF_LEN UT_CAPTURE_WIDTH*UT_CAPTURE_HEIGH*4

class FackShareCapturer : public IShareCapturer
{
public:
    FackShareCapturer() {}
    virtual ~FackShareCapturer() {}
    //Capture Source>> Support multi source at same time, e.g. Application sharing.
    virtual WBXResult AddCaptureShareSource(IShareSource *pIShareSource) {if (pIShareSource==WBXNull) { return WBX_FAIL; } return m_IShareSourceMgr.AddItem(pIShareSource);}
    virtual WBXResult RemoveCaptureShareSource(IShareSource *pIShareSource) {return m_IShareSourceMgr.RemoveItem(pIShareSource);}
    virtual WBXInt32 GetCaptureShareSourceCount() {return m_IShareSourceMgr.GetItemCount();}
    virtual IShareSource *GetCaptureShareSource(WBXInt32 nIndex) {return m_IShareSourceMgr.GetItem(nIndex);}
    //
    virtual WBXResult AddSink(IShareCapturerSink *pIShareCapturerSink) {return m_IShareCapturerSinkMgr.AddItem(pIShareCapturerSink);}
    virtual WBXResult RemoveSink(IShareCapturerSink *pIShareCapturerSink) {return m_IShareCapturerSinkMgr.RemoveItem(pIShareCapturerSink);}
    virtual WBXResult CleanSink() {return m_IShareCapturerSinkMgr.CleanItems();}
    //Real Capture API
    virtual WBXResult Capture(IShareFrame *pIShareFrame)
    {
        if (pIShareFrame) {
            memset(g_Buf,0x08,sizeof(g_Buf));
            pIShareFrame->WriteFrameData(g_Buf,WbxMin(pIShareFrame->GetFrameDataLen(),UT_CAPTURE_BUF_LEN));
            return WBX_SUCCESS;
        }
        return WBX_FAIL;
    }

    virtual DisplayID GetCurrentCaptureDisplayId() { return 0; }
    virtual WBXResult SetCaptureMaxSize(WBXSize szMaxSize) { return WBX_SUCCESS;}
    virtual WBXResult GetCurrentCaptureSourceSize(WBXSize &szCapture) { szCapture.SetSize(UT_CAPTURE_WIDTH,UT_CAPTURE_HEIGH); return WBX_SUCCESS;}
    WBXResult SetFilterSelf(bool bFilterSelf) { return WBX_ERR_NO_IMPLEMENT; }
    WBXResult AddSharedWindow(void *handle) { return WBX_ERR_NO_IMPLEMENT; }
    WBXResult RemoveSharedWindow(void *handle) { return WBX_ERR_NO_IMPLEMENT; }
protected:
    WBXByte g_Buf[UT_CAPTURE_BUF_LEN];
    WbxUniquePointArrayT<IShareSource> m_IShareSourceMgr;
    WbxUniquePointArrayT<IShareCapturerSink> m_IShareCapturerSinkMgr;
};

class FackCShareCaptureEngine : public CShareCaptureEngine
{
public:
    FackCShareCaptureEngine()
    {
        m_pIShareCapturer = new FackShareCapturer;
        m_pISharePointing = new FakeSharePointing;
        m_eSHARE_CAPTURE_ENGINE_TYPE = SHARE_CAPTURE_ENGINE_TYPE_DESKTOP;
    }
    ~FackCShareCaptureEngine() {WBX_SAFE_DELETE(m_pIShareCapturer); WBX_SAFE_DELETE(m_pISharePointing);}

};

//=====================================================
//CShareCaptureEngineTest testing
class CShareCaptureEngineTest : public testing::Test
{
public:
    virtual void SetUp()
    {
    }
    virtual void TearDown()
    {
    }
protected:
    FackCShareCaptureEngine m_CShareCaptureEngine;
    FakeShareSource m_FakeShareSource;
    FakeSharePointing m_FakeSharePointing_External;
    FackShareCapturer m_FackShareCapturer_External;

    //
    MockIShareCaptureEngineSink m_MockIShareCaptureEngineSink;
};

TEST_F(CShareCaptureEngineTest, Start)
{

    SHARE_CAPTURE_ENGINE_STATUS eSHARE_CAPTURE_ENGINE_STATUS = SHARE_CAPTURE_ENGINE_STATUS_STOPPED;
    WBXSize size(0,0);
    IShareFrame *pIShareFrame = WBXNull;
    WBXRect  rect(0,0,0,0);
    EXPECT_CALL(m_MockIShareCaptureEngineSink,OnCaptureStatusChanged(_))
    .Times(2)//stop->start->stop
    .WillRepeatedly(SaveArg<0>(&eSHARE_CAPTURE_ENGINE_STATUS));
    EXPECT_CALL(m_MockIShareCaptureEngineSink,OnCaptureDisplaySizeChanged(_))
    .WillOnce(SaveArg<0>(&size));
    EXPECT_CALL(m_MockIShareCaptureEngineSink,OnCaptureData(_,_))
    .WillRepeatedly(DoAll(SaveArg<0>(&pIShareFrame),SaveArg<1>(&rect)));
    EXPECT_CALL(m_MockIShareCaptureEngineSink,OnGetExternalIShareFrame(_,_))
    .Times(AnyNumber());

    ASSERT_TRUE(m_CShareCaptureEngine.SetSink(&m_MockIShareCaptureEngineSink)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.Start()!=WBX_SUCCESS);
    m_CShareCaptureEngine.AddCaptureShareSource(&m_FakeShareSource);

    ASSERT_TRUE(m_CShareCaptureEngine.Start()==WBX_SUCCESS);
    ASSERT_TRUE(eSHARE_CAPTURE_ENGINE_STATUS==SHARE_CAPTURE_ENGINE_STATUS_STARTED);
    ASSERT_TRUE(m_CShareCaptureEngine.Start()==WBX_SUCCESS);

    m_CShareCaptureEngine.Stop();
}

TEST_F(CShareCaptureEngineTest, Stop)
{
    ASSERT_TRUE(m_CShareCaptureEngine.Stop()==WBX_SUCCESS);
    m_CShareCaptureEngine.SetSink(&m_MockIShareCaptureEngineSink);
    m_CShareCaptureEngine.Start();
    ASSERT_TRUE(m_CShareCaptureEngine.Stop()==WBX_SUCCESS);
}

TEST_F(CShareCaptureEngineTest, Pause)
{
    ASSERT_TRUE(m_CShareCaptureEngine.Pause()==WBX_FAIL);
    m_CShareCaptureEngine.AddCaptureShareSource(&m_FakeShareSource);
    m_CShareCaptureEngine.Start();
    ASSERT_TRUE(m_CShareCaptureEngine.Pause()==WBX_SUCCESS);
    m_CShareCaptureEngine.Stop();
}

TEST_F(CShareCaptureEngineTest, Resume)
{
    ASSERT_TRUE(m_CShareCaptureEngine.Resume()==WBX_FAIL);
    m_CShareCaptureEngine.AddCaptureShareSource(&m_FakeShareSource);
    m_CShareCaptureEngine.Start();
    ASSERT_TRUE(m_CShareCaptureEngine.Resume()==WBX_SUCCESS);
    m_CShareCaptureEngine.Pause();
    ASSERT_TRUE(m_CShareCaptureEngine.Resume()==WBX_SUCCESS);
}

TEST_F(CShareCaptureEngineTest, SetSink)
{
    ASSERT_TRUE(m_CShareCaptureEngine.SetSink(WBXNull)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.SetSink(&m_MockIShareCaptureEngineSink)==WBX_SUCCESS);
}

TEST_F(CShareCaptureEngineTest, SetExternalIShareCapturer)
{
    ASSERT_TRUE(m_CShareCaptureEngine.SetExternalIShareCapturer(WBXNull)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.SetExternalIShareCapturer(&m_FackShareCapturer_External)==WBX_SUCCESS);
}

TEST_F(CShareCaptureEngineTest, SetExternalISharePointing)
{
    ASSERT_TRUE(m_CShareCaptureEngine.SetExternalISharePointing(WBXNull)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.SetExternalISharePointing(&m_FakeSharePointing_External)==WBX_SUCCESS);
}

TEST_F(CShareCaptureEngineTest, AddCaptureShareSource)
{
    FakeShareSource fakeShareSource1;
    FakeShareSource fakeShareSource2;

    ASSERT_TRUE(m_CShareCaptureEngine.AddCaptureShareSource(WBXNull)==WBX_FAIL);
    ASSERT_TRUE(m_CShareCaptureEngine.AddCaptureShareSource(&fakeShareSource1)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.AddCaptureShareSource(&fakeShareSource1)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.AddCaptureShareSource(&fakeShareSource2)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.GetCaptureShareSourceCount()==2);
}

TEST_F(CShareCaptureEngineTest, RemoveCaptureShareSource)
{
    FakeShareSource fakeShareSource1;
    FakeShareSource fakeShareSource2;

    ASSERT_TRUE(m_CShareCaptureEngine.RemoveCaptureShareSource(&fakeShareSource1)==WBX_FAIL);
    ASSERT_TRUE(m_CShareCaptureEngine.AddCaptureShareSource(&fakeShareSource1)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.AddCaptureShareSource(&fakeShareSource2)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.RemoveCaptureShareSource(&fakeShareSource1)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.RemoveCaptureShareSource(&fakeShareSource1)==WBX_FAIL);
}

TEST_F(CShareCaptureEngineTest, GetCaptureShareSourceCount)
{
    FakeShareSource fakeShareSource1;
    FakeShareSource fakeShareSource2;

    ASSERT_TRUE(m_CShareCaptureEngine.GetCaptureShareSourceCount()==0);
    ASSERT_TRUE(m_CShareCaptureEngine.AddCaptureShareSource(&fakeShareSource1)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.AddCaptureShareSource(&fakeShareSource2)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.GetCaptureShareSourceCount()==2);
    ASSERT_TRUE(m_CShareCaptureEngine.RemoveCaptureShareSource(&fakeShareSource1)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.GetCaptureShareSourceCount()==1);
    ASSERT_TRUE(m_CShareCaptureEngine.RemoveCaptureShareSource(&fakeShareSource2)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.GetCaptureShareSourceCount()==0);
}


TEST_F(CShareCaptureEngineTest, GetCaptureShareSource)
{
    FakeShareSource fakeShareSource1;
    FakeShareSource fakeShareSource2;

    ASSERT_TRUE(m_CShareCaptureEngine.GetCaptureShareSource(0)==WBXNull);
    ASSERT_TRUE(m_CShareCaptureEngine.AddCaptureShareSource(&fakeShareSource1)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.AddCaptureShareSource(&fakeShareSource2)==WBX_SUCCESS);
    ASSERT_TRUE(m_CShareCaptureEngine.GetCaptureShareSource(0)==&fakeShareSource1);
    ASSERT_TRUE(m_CShareCaptureEngine.GetCaptureShareSource(1)==&fakeShareSource2);
}