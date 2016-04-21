#include "CameraPreview.h"

CCameraPreview::CCameraPreview()
{
    m_pTrack = NULL;
    m_pConn = NULL;
    m_pVideoRender = NULL;
}

CCameraPreview::~CCameraPreview()
{
    if (m_pVideoRender != NULL)
        m_pVideoRender->Release();
    m_pVideoRender = NULL;
}

void CCameraPreview::Init()
{
    WMERESULT res = CreateMediaConnection4T(8888, &m_pConn);
    if (m_pConn != NULL) {
        m_pConn->SetSink(this);
        m_pConn->AddMedia(WmeSessionType_Video, WmeDirection_Send, 0, NULL);
    }
}

void CCameraPreview::OnMediaReady(unsigned long mid, WmeDirection direction,
    WmeSessionType mediaType, IWmeMediaTrackBase *pTrack)
{
    m_pTrack = pTrack;
    m_pTrack->AddRef();
}

void CCameraPreview::Preview(IWmeMediaDevice *pDevice, HWND hWnd)
{
    if (m_pTrack == NULL)
        return;

    if (m_pVideoRender != NULL)
        m_pVideoRender->Release();
    m_pVideoRender = NULL;
    CreateVideoRender(hWnd, &m_pVideoRender);
    m_pTrack->SetCaptureDevice(pDevice);
    m_pTrack->AddExternalRender(m_pVideoRender);
    m_pTrack->StartPreview();
}

void CCameraPreview::ChangeDevice(IWmeMediaDevice *pDevice)
{
    if (m_pTrack)
        m_pTrack->SetCaptureDevice(pDevice);
}

void CCameraPreview::UnInit()
{
    if (m_pConn) {
        m_pConn->Stop();
        m_pConn->Release();
        m_pConn = NULL;
    }
    if (m_pTrack) {
        m_pTrack->Release();
        m_pTrack = NULL;
    }
    if (m_pVideoRender != NULL) {
        m_pVideoRender->Release();
        m_pVideoRender = NULL;
    }
}

void CCameraPreview::StopPreview(HWND hWnd)
{
    if (m_pTrack != NULL) {
        m_pTrack->Stop();
        m_pTrack->RemoveExternalRender(m_pVideoRender);
    }
    if (m_pVideoRender != NULL)
        m_pVideoRender->Release();
    m_pVideoRender = NULL;
}
