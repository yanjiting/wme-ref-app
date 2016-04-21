//
//  rtpdump.h
//  MediaSessionTest
//
//  Created by Wilson Chen on 11/30/15.
//  Copyright © 2015 cisco. All rights reserved.
//

#ifndef rtpdump_h
#define rtpdump_h

#include "MediaConnection4T.h"

namespace ClickCall{
    
class CRtpDumper : public wme::IWmeExternalRenderer
{
public:
    CRtpDumper() {}
    virtual ~CRtpDumper() {}
    
public:
    virtual WMERESULT IsPushModeSupport(bool& isSupport) {
        isSupport = true;
        return WME_S_OK;
    }

    virtual WMERESULT RenderMediaData(IWmeMediaPackage *pPackage) {
        return WME_S_OK;
    }
    
    virtual WMERESULT RenderMediaData(uint32_t uTimestamp, WmeMediaFormatType eType,
                                      void* pFormat, unsigned char *pData, uint32_t uLength)
    {
        return WME_S_OK;
    }
    
    virtual WMERESULT IsPullModeSupport(bool& isSupport) {
        isSupport = false;
        return WME_S_OK;
    }
    virtual WMERESULT RegisterRequestAnswerer(IWmeExternalRenderAnswerer* pAnswerer) {
        return WME_E_NOTIMPL;
    }
    virtual WMERESULT UnregisterRequestAnswerer() {
        return WME_E_NOTIMPL;
    }
    
    virtual WmeMediaExternalRenderTypeConfig GetTypeExt() { return WmeExternalRender_Type_RTP; }
    virtual void SetTypeExt(WmeMediaExternalRenderTypeConfig eType) { return; }
};
    
}

#endif /* rtpdump_h */
