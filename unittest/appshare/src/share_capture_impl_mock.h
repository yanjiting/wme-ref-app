#ifndef _SHARE_CAPTURE_MOCK_H_
#define _SHARE_CAPTURE_MOCK_H_
#include "gmock/gmock.h"
#include "share_capturer.h"

class MockIShareCaptureSink : public IShareCapturerSink
{
public:
    MOCK_METHOD1(OnShareSourceAdded, WBXResult(IShareSource *lpSource));
    MOCK_METHOD1(OnShareSourceRemoved, WBXResult(IShareSource *lpSource));
    MOCK_METHOD0(OnCapturedDataChanged, WBXResult());
    MOCK_METHOD1(OnCapturedDisplaySizedChanged,WBXResult(WBXSize));
    MOCK_METHOD1(OnCapturedDisplayChanged,WBXResult(WBXBool));
    MOCK_METHOD0(OnCapturedNoSharedApp, WBXResult());
};
#endif //_SHARE_CAPTURE_MOCK_H_