#ifndef _SHARE_CAPTURE_ENGINE_MOCK_H_
#define _SHARE_CAPTURE_ENGINE_MOCK_H_
#include "gmock/gmock.h"
#include "share_capture_engine.h"

class MockIShareCaptureEngineSink : public IShareCaptureEngineSink
{
public:
    MOCK_METHOD1(OnCaptureDisplaySizeChanged, WBXVoid(WBXSize &size));
    MOCK_METHOD1(OnCaptureStatusChanged, WBXVoid(SHARE_CAPTURE_ENGINE_STATUS eSHARE_CAPTURE_ENGINE_STATUS));
    MOCK_METHOD2(OnCaptureData, WBXVoid(IShareFrame *pIShareFrame,WBXRect &rcUpdate));
    MOCK_METHOD2(OnGetExternalIShareFrame, IShareFrame*(WBXSize &sizeFrame,SHARE_FRAME_FORMAT eSHARE_FRAME_FORMATPrefer));
    MOCK_METHOD1(OnError, WBXVoid(WBXInt32 nErrorCode));
};
#endif //_SHARE_CAPTURE_ENGINE_MOCK_H_