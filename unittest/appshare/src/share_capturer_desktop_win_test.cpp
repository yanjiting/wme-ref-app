
#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "./windows/share_capturer_desktop_win.h"
#include "share_util.h"
#include "share_common_frame_impl.h"


using namespace testing;


class CShareCapturerDesktopWinTest : public Test
{
public:

    virtual void SetUp()
    {

    }
    virtual void TearDown()
    {

    }

protected:

    CShareCapturerDesktopWin m_Capturer;
};





TEST_F(CShareCapturerDesktopWinTest, Capture)
{
    ASSERT_TRUE(m_Capturer.Capture(WBXNull) == WBX_FAIL);


    CShareFrame *pShareFrame = new CShareFrame;
    pShareFrame->AddRef();

    ASSERT_TRUE(m_Capturer.Capture(pShareFrame) == WBX_SUCCESS);
    SaveFrameToImg(pShareFrame, "c:\\desktop.bmp");

    ASSERT_TRUE(pShareFrame->GetFrameData() != WBXNull);

    pShareFrame->Release();
}


