#include "VideoPreview.h"

#ifdef ANDROID
#include "wmenative-jni.h"
#endif //ANDROID

VideoPreview::VideoPreview() : VideoSettings(m_pTrack)
{
    m_pCamera = NULL;
    memset(&m_rawFormat, 0, sizeof(m_rawFormat));
}

VideoPreview::~VideoPreview()
{
    SAFE_RELEASE(m_pCamera);
}

long VideoPreview::CreateTrack()
{
    returnv_if_fail(m_pEngine != NULL, WME_E_FAIL);

    if (m_pTrack) {
        CM_INFO_TRACE_THIS("VideoPreview::CreateTrack, m_pTrack is not NULL!");
        return WME_S_OK;
    }

    long ret = WME_S_FALSE;
    ret = m_pEngine->CreateVideoPreviewTrack((IWmeVideoPreviewTrack **)&m_pTrack);
    if (ret != WME_S_OK || !m_pTrack) {
        return WME_E_FAIL;
    }
    uint32_t label = kVideoPreviewTrack_Label;
    m_pTrack->SetTrackLabel(label);

	ret = ((IWmeVideoPreviewTrack *)m_pTrack)->SetCaptureDevice(m_pCamera);
	ret = ((IWmeVideoPreviewTrack *)m_pTrack)->SetCaptureFormat(&m_rawFormat);

	m_pTrack->AddObserver(WMEIID_IWmeMediaCaptureObserver, this);
    return ret;
}

long VideoPreview::DeleteTrack()
{
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);
	m_pTrack->RemoveObserver(WMEIID_IWmeMediaCaptureObserver, this);
    return MediaClient::DeleteTrack();
}

long VideoPreview::SetDevice(IWmeMediaDevice* pCamera)
{
    SAFE_RELEASE(m_pCamera);
    m_pCamera = pCamera;
    if(m_pCamera) {
        m_pCamera->AddRef();
    }else {
        return WME_S_OK;
    }
    
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

    long ret = WME_E_FAIL;
    IWmeVideoPreviewTrack *pTrack = (IWmeVideoPreviewTrack *)m_pTrack;
    if (m_bStarted) {
        ret = pTrack->Stop();
        ret = pTrack->SetCaptureDevice(m_pCamera);

		if(m_rawFormat.eRawType != WmeVideoUnknown)
			ret = pTrack->SetCaptureFormat(&m_rawFormat);

        ret = pTrack->Start();
    }else {
        ret = pTrack->SetCaptureDevice(m_pCamera);

		if(m_rawFormat.eRawType != WmeVideoUnknown)
			ret = pTrack->SetCaptureFormat(&m_rawFormat);
    }
    
    return ret;
}

long VideoPreview::GetDevice(IWmeMediaDevice** ppCamera)
{
	if(!ppCamera)
		return WME_E_FAIL;

	returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

	long ret = WME_E_FAIL;
	IWmeVideoPreviewTrack *pTrack = (IWmeVideoPreviewTrack *)m_pTrack;
	ret = pTrack->GetCaptureDevice(ppCamera);

	return ret;
}

long VideoPreview::SetCaptureCapability(const WmeVideoRawFormat & format)
{
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);
    m_rawFormat = format;
    long ret = WME_S_OK;
    if (!m_bStarted) {
        ret = ((IWmeVideoPreviewTrack *)m_pTrack)->SetCaptureFormat(&m_rawFormat);
    }
    return ret;
}

WMERESULT VideoPreview::OnStartedCapture(IWmeMediaEventNotifier *pNotifier, WMERESULT eStartedResult)
{
	CM_INFO_TRACE_THIS("VideoPreview::OnStartedCapture, eStartedResult="<<eStartedResult);
	return WME_S_OK;
}

WMERESULT VideoPreview::OnUpdateCaptureResolution(IWmeMediaEventNotifier *pNotifier, uint32_t uWidth, uint32_t uHeight)
{
	CM_INFO_TRACE_THIS("VideoPreview::OnUpdateCaptureResolution, uWidth="<<uWidth<<",uHeight="<<uHeight);
#ifdef ANDROID
	javaSetVideoCaptureResolution_STAT(uWidth, uHeight);
#endif	//ANDROID
	return WME_S_OK;
}

WMERESULT VideoPreview::OnUpdateCaptureFPS(IWmeMediaEventNotifier *pNotifier, uint32_t uFPS)
{
	CM_INFO_TRACE_THIS("VideoPreview::OnUpdateCaptureFPS, uFPS="<<uFPS);
#ifdef ANDROID
	javaSetVideoCaptureFPS_STAT(uFPS);
#endif //ANDROID
	return WME_S_OK;
}