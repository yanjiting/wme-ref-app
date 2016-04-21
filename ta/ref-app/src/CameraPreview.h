#pragma once

#include "MediaConnection4T.h"

using namespace wme;

class CCameraPreview : public IWmeMediaConnectionSink
{
public:
    CCameraPreview();
    ~CCameraPreview();

    void Preview(IWmeMediaDevice *pDevice, HWND hWnd);
    void ChangeDevice(IWmeMediaDevice *pDevice);
    void StopPreview(HWND hWnd);
    void Init();
    void UnInit();

protected:
    virtual void OnMediaReady(unsigned long mid, WmeDirection direction,
        WmeSessionType mediaType, IWmeMediaTrackBase *pTrack);
    virtual void OnSDPReady(WmeSdpType sdpType, const char *sdp) {}
    virtual void OnMediaBlocked(unsigned long mid, unsigned long vid, bool blocked) {}
    virtual void OnDecodeSizeChanged(unsigned long mid, unsigned long vid, uint32_t uWidth, uint32_t uHeight) {}
    virtual void OnRenderSizeChanged(unsigned long mid, unsigned long vid, WmeDirection direction, WmeVideoSizeInfo *pSizeInfo){}
    virtual void OnSessionStatus(unsigned long mid, WmeSessionType mediaType, WmeSessionStatus status) {}
    virtual void OnMediaStatus(unsigned long mid, unsigned long vid, WmeMediaStatus status, bool hasCSI, uint32_t csi) {}
    virtual void OnMediaError(unsigned long mid, unsigned long vid, WMERESULT errorCode) {}
    virtual void OnAvailableMediaChanged(unsigned long mid, int count) {}
    virtual void OnNetworkStatus(WmeNetworkStatus status, WmeNetworkDirection direc) {}
    virtual void OnError(WMERESULT errorCode) {}
    virtual void OnCSIsChanged(unsigned long mid, unsigned int vid,
        const unsigned int* oldCSIArray, unsigned int oldCSICount,
        const unsigned int* newCSIArray, unsigned int newCSICount) {}

private:
    IWmeMediaConnection4T *m_pConn;
    IWmeMediaTrackBase *m_pTrack;
    IWmeVideoRender *m_pVideoRender;
};
