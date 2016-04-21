#include "gtest/gtest.h"

#include "./mac/share_capturer_desktop_mac.h"
#include "./mac/share_capturer_app_mac.h"
#include "./mac/app_enum_mac.h"

#include "share_util.h"
#include "share_common_frame_impl.h"


using namespace testing;


class CShareCapturerDesktopMacTest : public Test
{
public:

    virtual void SetUp()
    {
        _LaunchOneApp();
    }
    virtual void TearDown()
    {
        _ExitOneApp();
    }

    IShareSource *getOneAppSource()
    {
        IShareSource *pIShareSource = NULL;
        appEnumMac.Refresh();
        WBXInt32  iNumber =0 ;
        appEnumMac.GetNumber(iNumber);
        if (iNumber<=0) { return  NULL; }
        appEnumMac.GetSource(0, &pIShareSource);
        return pIShareSource;
    }
    

    void _LaunchOneApp() {
        if(![[NSWorkspace sharedWorkspace] launchApplication:@"/Applications/Safari.app"])
            NSLog(@"Path Finder failed to launch");
    }
    
    void _ExitOneApp() {
#ifdef MACOS
        
#endif
    }
    
protected:
    CAppEnumMac appEnumMac;
    CShareCapturerDesktopMac m_Capturer;
    CShareCapturerAppMac m_CapturerApp;
};



TEST_F(CShareCapturerDesktopMacTest, CaptureDesktop)
{


    ASSERT_TRUE(m_Capturer.Capture(WBXNull) == WBX_FAIL);



    CShareFrame *pShareFrame = new CShareFrame;
    pShareFrame->AddRef();

    ASSERT_TRUE(m_Capturer.Capture(pShareFrame) == WBX_SUCCESS);
    //  SaveFrameToPNG(pShareFrame, "/tmp/unit-testing/desktop.png");
    ASSERT_TRUE(pShareFrame->GetFrameData() != WBXNull);

    pShareFrame->Release();
}

TEST_F(CShareCapturerDesktopMacTest, CaptureApp)
{
    //    appEnumMac.
    IShareSource *pIShareSource = getOneAppSource();

    m_CapturerApp.AddCaptureShareSource(pIShareSource);
    //capture
    CShareFrame *pShareFrame = new CShareFrame;
    pShareFrame->AddRef();

    ASSERT_TRUE(m_CapturerApp.Capture(pShareFrame) == WBX_SUCCESS);
    //   SaveFrameToPNG(pShareFrame, "/tmp/unit-testing/app-dump.png");
    ASSERT_TRUE(pShareFrame->GetFrameData() != WBXNull);

    pShareFrame->Release();
}

TEST_F(CShareCapturerDesktopMacTest, AddRemoveSharedWindow)
{
    CShareFrame *pShareFrame = new CShareFrame;
    pShareFrame->AddRef();
    ASSERT_TRUE(m_Capturer.Capture(pShareFrame) == WBX_SUCCESS);
    //  SaveFrameToPNG(pShareFrame, "/tmp/unit-testing/desktop.png");
    ASSERT_TRUE(pShareFrame->GetFrameData() != WBXNull);
    pShareFrame->Release();

    ASSERT_TRUE(m_Capturer.AddSharedWindow((void *)1) == WBX_SUCCESS);
    ASSERT_TRUE(m_Capturer.RemoveSharedWindow((void *)2)!= WBX_SUCCESS);
    ASSERT_TRUE(m_Capturer.RemoveSharedWindow((void *)1) == WBX_SUCCESS);
    ASSERT_TRUE(m_Capturer.RemoveSharedWindow((void *)1)!= WBX_SUCCESS);
}



