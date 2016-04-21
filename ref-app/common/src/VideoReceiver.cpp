#include "VideoReceiver.h"

VideoReceiver::VideoReceiver() : VideoSettings(m_pTrack)
{
    m_pFileRender = NULL;
    m_file_target = false;
}

VideoReceiver::~VideoReceiver()
{
    if (m_pFileRender != NULL) {
        m_pFileRender->Release();
        m_pFileRender = NULL;
    }
}

long VideoReceiver::CreateTrack()
{
    returnv_if_fail(m_pEngine != NULL, WME_E_FAIL);
    returnv_if_fail(m_pSyncBox != NULL, WME_E_FAIL);

    if (m_pTrack) {
        CM_ERROR_TRACE_THIS("VideoReceiver::CreateTrack, m_pTrack is not NULL!");
        return WME_S_OK;
    }

    long ret = WME_S_FALSE;
    ret = m_pEngine->CreateRemoteVideoTrack((IWmeRemoteVideoTrack **)&m_pTrack);
    if (ret != WME_S_OK || !m_pTrack) {
        CM_ERROR_TRACE_THIS("VideoReceiver::CreateTrack, fail to CreateRemoteVideoTrack!");
        return WME_E_FAIL;
    }
    
    if (m_file_target == true) //file
    {
        CM_INFO_TRACE_THIS("VideoReceiver::CreateTrack, add external render, "<<(void*)m_pFileRender);
        //TODO:
        //1. add external renderer to remote video track
        if (m_pFileRender)
        {
            ((IWmeRemoteVideoTrack *)m_pTrack)->AddExternalRenderer(m_pFileRender, NULL);   //the second param "rawvideoformat" is ignored;
        }
        
    }
    
    uint32_t label = kVideoRemoteTrack_Label;
    m_pTrack->SetTrackLabel(label);

    /// Add remote video track observer
    ret = m_pTrack->AddObserver(WMEIID_IWmeRemoteVideoTrackObserver, (IWmeRemoteVideoTrackObserver*)this);

    /// Add remote track into sync box
    ret = m_pSyncBox->AddTrack(m_pTrack);

    return WME_S_OK;
}

long VideoReceiver::DeleteTrack()
{
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

    m_pTrack->RemoveObserver(WMEIID_IWmeRemoteVideoTrackObserver, (IWmeRemoteVideoTrackObserver *)this);

    if (m_pSyncBox) {
        m_pSyncBox->RemoveTrack(m_pTrack);
    }
    
    if (m_file_target == true) //file
    {
        CM_INFO_TRACE_THIS("VideoReceiver::DeleteTrack, remove external render, "<<(void*)m_pFileRender);
        //TODO:
        //1. remove external renderer
        if (m_pFileRender)
        {
            ((IWmeRemoteVideoTrack *)m_pTrack)->RemoveExternalRenderer(m_pFileRender);
            m_pFileRender->Release();
            m_pFileRender = NULL;
        }
        
    }

    return MediaClient::DeleteTrack();
}

long VideoReceiver::SetVideoFile(const char* pFileName)
{
    CM_INFO_TRACE_THIS("VideoReceiver::SetVideoFile, file:"<<pFileName);
    //TODO:
    //1. create file renderer
    if (m_pFileRender != NULL) {
        m_pFileRender->Release();
        m_pFileRender = NULL;
    }
    WMERESULT res = m_pEngine->CreateMediaFileRenderer(&m_pFileRender);
    if (res != WME_S_OK || m_pFileRender == NULL) {
         CM_ERROR_TRACE_THIS("VideoReceiver::SetVideoFile, create media file render fail,res="<<res);
        return res;
    }
    CM_INFO_TRACE_THIS("VideoReceiver::SetVideoFile, create media file render OK");
    
    
    //2. set file path
    res = m_pFileRender->SetTargetFile((char*)pFileName, strlen(pFileName));
    if (res != WME_S_OK) {
        CM_ERROR_TRACE_THIS("VideoReceiver::SetVideoFile, set target file fail,res="<<res);
        return res;
    }
    
    //3. set config (optional)
    
    
    CM_INFO_TRACE_THIS("VideoReceiver::SetVideoFile OK, file render="<<m_pFileRender);
    return WME_S_OK;
}

/// for IWmeRemoteVideoTrackObserver
WMERESULT VideoReceiver::OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

/// for IWmeRemoteVideoTrackObserver
WMERESULT VideoReceiver::OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

/// for IWmeRemoteVideoTrackObserver
WMERESULT VideoReceiver::OnKeyFrameLost(uint32_t uLabel, uint32_t uDID)
{
    returnv_if_fail(m_pSession != NULL, WME_E_FAIL);
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

    uint32_t uTrackLabel = 0;
    m_pTrack->GetTrackLabel(uTrackLabel);
    m_pSession->IndicatePictureLoss(uTrackLabel, uLabel);
    return WME_S_OK;
}

/// for IWmeRemoteVideoTrackObserver
WMERESULT VideoReceiver::OnRenderBlocked(uint32_t uLabel, bool bBlocked)
{
	CM_INFO_TRACE_THIS("VideoReceiver::OnRenderBlocked, bBlocked="<<bBlocked<<", uLabel="<<uLabel);
    return WME_S_OK;
}

WMERESULT VideoReceiver::OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight)
{
    if (m_pSink) {
        m_pSink->OnDecodeSizeChanged(uLabel, uWidth, uHeight);
    }
    return WME_S_OK;
}

long VideoReceiver::EnableDecoderMosaic(bool bEnable)
{
	returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);
	return m_pTrack->SetOption(WmeTrackOption_EnableDecoderMosaic, (void *)&bEnable, sizeof(bEnable));
}
#ifdef CUCUMBER_ENABLE
bool VideoReceiver::AddFileRenderSink(IWmeMediaFileRenderSink* pSink)
{
    CM_INFO_TRACE_THIS("VideoReceiver::AddFileRenderSink, m_pFileRender="<<m_pFileRender);
    if(!m_pFileRender)
        return false;
    
    m_pFileRender->SetRenderSink(pSink);
    return true;

}
#endif