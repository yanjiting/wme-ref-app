#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "ios/share_capture_content_ios.h"
#include "ios/ContentContainerViewController.h"

using namespace testing;


class CShareCapturerContentTest : public Test
{
public:
    CShareCapturerContentTest() : m_pCapturer(NULL)
    {

    }
    virtual ~CShareCapturerContentTest()
    {

    }
    virtual void SetUp()
    {
        if (NULL == m_pCapturer) {
            m_pCapturer = (CShareCapturerContentiOS *)CreateShareCapture(SHARE_CAPTURER_TYPE_CONTENT);
        }
    }
    virtual void TearDown()
    {
        if (m_pCapturer) {
            DestroyShareCapture(m_pCapturer);
            m_pCapturer = NULL;
        }
    }
    CShareCapturerContentiOS *m_pCapturer;
};

#include "share_common_frame_impl.h"
#include "share_define.h"
#include "share_util.h"

TEST_F(CShareCapturerContentTest, Start)
{
    NSString *file = [[NSBundle mainBundle] pathForResource:@"pobabies" ofType:@"jpg"];

    m_pCapturer->GetContentContainerViewControllerEx();
    ASSERT_TRUE(m_pCapturer->OpenDocument([file UTF8String]));
    CShareFrame *pFrame = new CShareFrame;
    pFrame->AddRef();

    ASSERT_TRUE(m_pCapturer->Capture(pFrame) == WBX_SUCCESS);


    NSString *savePath = nil;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    if ([paths count] > 0) {
        savePath = [paths objectAtIndex:0];
        //        SaveFrameToPNG(pFrame, [[savePath stringByAppendingPathComponent: @"content.png"] UTF8String]);
        SaveFrameToJPEG(pFrame, 0.5, [[savePath stringByAppendingPathComponent: @"content.jpg"] UTF8String]);
    }
    pFrame->Release();
}
