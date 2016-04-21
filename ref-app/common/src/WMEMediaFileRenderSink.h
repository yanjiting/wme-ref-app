#ifndef WMEMediaFileRenderSink_H
#define WMEMediaFileRenderSink_H
#include "IMediaClient.h"
#include "IMediaSettings.h"

//#include <iostream>

class CWMEMediaFileRenderSink : public IWmeMediaFileRenderSink
{
public:
    virtual WMERESULT OnOutputMediaData(uint32_t uTimestamp, WmeMediaFormatType eType, void* pFormat, unsigned char *pData, uint32_t uLength);
};
#endif /* defined(__MediaEngineTestApp__WMEMediaFileRenderSink__) */
