
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "share_capturer_impl.h"
#include "share_capturer.h"

#include "appshare_fake_common.h"
#include "share_capture_impl_mock.h"
#include "share_common_frame_impl.h"


using namespace testing;


// api test
TEST(share_capture,CreateShareCapture_DestroyShareCapture)
{
    IShareCapturer *pIShareCapture = WBXNull;

    //
    pIShareCapture = CreateShareCapture(SHARE_CAPTURER_TYPE_DESKTOP);
    ASSERT_TRUE(pIShareCapture!=WBXNull);
    ASSERT_TRUE(DestroyShareCapture(pIShareCapture)==WBX_SUCCESS);
    pIShareCapture=NULL;

    //
    pIShareCapture = CreateShareCapture(SHARE_CAPTURER_TYPE_APP);
    ASSERT_TRUE(pIShareCapture!=WBXNull);
    ASSERT_TRUE(DestroyShareCapture(pIShareCapture)==WBX_SUCCESS);
    pIShareCapture=NULL;


    //
    pIShareCapture = CreateShareCapture(SHARE_CAPTURER_TYPE_NULL);
    ASSERT_TRUE(pIShareCapture==WBXNull);
    ASSERT_TRUE(DestroyShareCapture(pIShareCapture)==WBX_ERR_PARAM);
    pIShareCapture=NULL;
}


class FakeShareCapturer : public CShareCapturer
{
public:
    FakeShareCapturer() {}
    virtual ~FakeShareCapturer() {}

    virtual WBXResult Capture(IShareFrame *pIShareFrame) { return WBX_FAIL; }
    void Cleanup() { return CShareCapturer::Cleanup(); }

};

class CShareCapturerTest : public Test
{
public:

    virtual void SetUp()
    {
        m_FakeShareSource1 = new FakeShareSource();
        m_FakeShareSource1->AddRef();
        m_FakeShareSource2 = new FakeShareSource();
        m_FakeShareSource2->AddRef();
        m_FakeShareSource3 = new FakeShareSource();
        m_FakeShareSource3->AddRef();


    }
    virtual void TearDown()
    {
        m_FakeShareSource1->Release();
        m_FakeShareSource2->Release();
        m_FakeShareSource3->Release();
    }

protected:

    FakeShareCapturer m_Capturer;
    FakeShareSource *m_FakeShareSource1;
    FakeShareSource *m_FakeShareSource2;
    FakeShareSource *m_FakeShareSource3;
    MockIShareCaptureSink m_MockIShareCaptureSink;
};



TEST_F(CShareCapturerTest, AddCaptureShareSource)
{

    ASSERT_TRUE(m_Capturer.AddCaptureShareSource(WBXNull) == WBX_FAIL);

    EXPECT_CALL(m_MockIShareCaptureSink,OnShareSourceAdded(_)).Times(2);

    ASSERT_TRUE(m_Capturer.AddSink(&m_MockIShareCaptureSink) == WBX_SUCCESS);


    ASSERT_TRUE(m_Capturer.AddCaptureShareSource(m_FakeShareSource1) == WBX_SUCCESS);

    // added twice
    ASSERT_TRUE(m_Capturer.AddCaptureShareSource(m_FakeShareSource1) == WBX_FAIL);

    ASSERT_TRUE(m_Capturer.AddCaptureShareSource(m_FakeShareSource2) == WBX_SUCCESS);


    ASSERT_TRUE(m_Capturer.GetCaptureShareSourceCount() == 2);



}

TEST_F(CShareCapturerTest, RemoveCaptureShareSource)
{

    ASSERT_TRUE(m_Capturer.RemoveCaptureShareSource(WBXNull) == WBX_FAIL);


    ASSERT_TRUE(m_Capturer.AddCaptureShareSource(m_FakeShareSource1) == WBX_SUCCESS);
    ASSERT_TRUE(m_Capturer.AddCaptureShareSource(m_FakeShareSource2) == WBX_SUCCESS);
    ASSERT_TRUE(m_Capturer.RemoveCaptureShareSource(m_FakeShareSource3) == WBX_FAIL);

    ASSERT_TRUE(m_Capturer.GetCaptureShareSourceCount() == 2);

    ASSERT_TRUE(m_Capturer.RemoveCaptureShareSource(m_FakeShareSource1) == WBX_SUCCESS);
    ASSERT_TRUE(m_Capturer.GetCaptureShareSourceCount() == 1);



    ASSERT_TRUE(m_Capturer.RemoveCaptureShareSource(m_FakeShareSource2) == WBX_SUCCESS);
    ASSERT_TRUE(m_Capturer.GetCaptureShareSourceCount() == 0);

    ASSERT_TRUE(m_Capturer.RemoveCaptureShareSource(m_FakeShareSource1) == WBX_FAIL);
}

TEST_F(CShareCapturerTest, GetCaptureShareSourceCount)
{
    ASSERT_TRUE(m_Capturer.GetCaptureShareSourceCount() == 0);


    ASSERT_TRUE(m_Capturer.AddCaptureShareSource(m_FakeShareSource1) == WBX_SUCCESS);
    ASSERT_TRUE(m_Capturer.GetCaptureShareSourceCount() == 1);


    ASSERT_TRUE(m_Capturer.AddCaptureShareSource(m_FakeShareSource2) == WBX_SUCCESS);
    ASSERT_TRUE(m_Capturer.GetCaptureShareSourceCount() == 2);

    ASSERT_TRUE(m_Capturer.AddCaptureShareSource(m_FakeShareSource3) == WBX_SUCCESS);
    ASSERT_TRUE(m_Capturer.GetCaptureShareSourceCount() == 3);

}


TEST_F(CShareCapturerTest, GetCaptureShareSource)
{
    ASSERT_TRUE(m_Capturer.GetCaptureShareSource(0) == WBXNull);

    ASSERT_TRUE(m_Capturer.GetCaptureShareSource(-1) == WBXNull);

    ASSERT_TRUE(m_Capturer.AddCaptureShareSource(m_FakeShareSource1) == WBX_SUCCESS);

    ASSERT_TRUE(m_Capturer.AddCaptureShareSource(m_FakeShareSource2) == WBX_SUCCESS);

    ASSERT_TRUE(m_Capturer.GetCaptureShareSource(0) == m_FakeShareSource1);
    ASSERT_TRUE(m_Capturer.GetCaptureShareSource(1) == m_FakeShareSource2);

    ASSERT_TRUE(m_Capturer.RemoveCaptureShareSource(m_FakeShareSource1) == WBX_SUCCESS);
    ASSERT_TRUE(m_Capturer.GetCaptureShareSource(0) == m_FakeShareSource2);

}


TEST_F(CShareCapturerTest, AddSink)
{
    ASSERT_TRUE(m_Capturer.AddSink(WBXNull) == WBX_FAIL);



    ASSERT_TRUE(m_Capturer.AddSink(&m_MockIShareCaptureSink) == WBX_SUCCESS);

    // twice
    ASSERT_TRUE(m_Capturer.AddSink(&m_MockIShareCaptureSink) == WBX_FAIL);
}

TEST_F(CShareCapturerTest, RemoveSink)
{
    ASSERT_TRUE(m_Capturer.RemoveSink(WBXNull) == WBX_FAIL);

    ASSERT_TRUE(m_Capturer.RemoveSink(&m_MockIShareCaptureSink) == WBX_FAIL);

    ASSERT_TRUE(m_Capturer.AddSink(&m_MockIShareCaptureSink) == WBX_SUCCESS);

    ASSERT_TRUE(m_Capturer.RemoveSink(&m_MockIShareCaptureSink) == WBX_SUCCESS);

    ASSERT_TRUE(m_Capturer.RemoveSink(&m_MockIShareCaptureSink) == WBX_FAIL);

}


TEST_F(CShareCapturerTest, CleanSink)
{
    ASSERT_TRUE(m_Capturer.CleanSink() == WBX_SUCCESS);
}

TEST_F(CShareCapturerTest, Cleanup)
{
    ASSERT_TRUE(m_Capturer.AddCaptureShareSource(m_FakeShareSource1) == WBX_SUCCESS);

    ASSERT_TRUE(m_Capturer.AddCaptureShareSource(m_FakeShareSource2) == WBX_SUCCESS);
    ASSERT_TRUE(m_Capturer.GetCaptureShareSourceCount() == 2);

    m_Capturer.Cleanup();

    ASSERT_TRUE(m_Capturer.GetCaptureShareSourceCount() == 0);
}





