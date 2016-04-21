
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "./windows/share_capturer_app_win.h"
#include "share_util.h"
#include "share_common_frame_impl.h"
#include "share_source_impl.h"

using namespace testing;


class CShareCapturerAppWinTest : public Test
{
public:

    virtual void SetUp()
    {

    }
    virtual void TearDown()
    {

    }

protected:

    CShareCapturerAppWin m_Capturer;
};


TEST_F(CShareCapturerAppWinTest, Capture)
{
    ASSERT_TRUE(m_Capturer.Capture(WBXNull) == WBX_FAIL);


    CShareSource *pShareSource = new CShareSource;
    pShareSource->AddRef();

    ShareSourceInfo ssi;
    ssi.sourceId = GetCurrentProcessId();
    ssi.type = SST_APPLICATION;

    pShareSource->SetInfo(&ssi);



    m_Capturer.AddCaptureShareSource(pShareSource);



    //////////////////////////////////////////////////////////////////////////
    //  CShareSource *pShareSource2 = new CShareSource;
    //  pShareSource2->AddRef();
    //
    //  ShareSourceInfo ssi2;
    //  ssi2.sourceId = 7000;
    //  ssi2.type = SST_APPLICATION;
    //
    //  pShareSource2->SetInfo(&ssi2);
    //
    //
    //
    //  m_Capturer.AddCaptureShareSource(pShareSource2);
    //
    //////////////////////////////////////////////////////////////////////////


    CShareFrame *pShareFrame = new CShareFrame;
    pShareFrame->AddRef();

    ASSERT_TRUE(m_Capturer.Capture(pShareFrame) == WBX_SUCCESS);
    SaveFrameToImg(pShareFrame, "c:\\app.bmp");

    ASSERT_TRUE(pShareFrame->GetFrameData() != WBXNull);

    pShareFrame->Release();


    m_Capturer.RemoveCaptureShareSource(pShareSource);

    //  m_Capturer.RemoveCaptureShareSource(pShareSource2);

}


TEST_F(CShareCapturerAppWinTest, AddCaptureShareSource)
{


}

TEST_F(CShareCapturerAppWinTest, RemoveCaptureShareSource)
{


}
