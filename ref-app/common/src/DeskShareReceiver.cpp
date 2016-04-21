#include "DeskShareReceiver.h"

DeskShareReceiver::DeskShareReceiver() : VideoSettings(m_pTrack)
{
}

DeskShareReceiver::~DeskShareReceiver()
{}

long DeskShareReceiver::CreateTrack()
{
    returnv_if_fail(m_pEngine != NULL, WME_E_FAIL);
    returnv_if_fail(m_pSyncBox != NULL, WME_E_FAIL);

    if (m_pTrack) {
        CM_INFO_TRACE_THIS("DeskShareReceiver::CreateTrack, m_pTrack is not NULL!");
        return WME_S_OK;
    }

    long ret = WME_S_FALSE;
//  ret = m_pEngine->CreateRemoteVideoTrack((IWmeRemoteVideoTrack **)&m_pTrack);
	ret = m_pEngine->CreateRemoteScreenShareTrack((IWmeRemoteScreenShareTrack **) &m_pTrack);

    if (ret != WME_S_OK || !m_pTrack) {
        CM_ERROR_TRACE_THIS("DeskShareReceiver::CreateTrack, fail to IWmeRemoteScreenShareTrack!");
        return WME_E_FAIL;
    }
    uint32_t label = kRemoteScreenShareTrack_Label;
    m_pTrack->SetTrackLabel(label);

    /// Add remote video track observer
    ret = m_pTrack->AddObserver(WMEIID_IWmeRemoteVideoTrackObserver, (IWmeRemoteVideoTrackObserver*)this);

    /// Add remote track into sync box
    ret = m_pSyncBox->AddTrack(m_pTrack);

    return WME_S_OK;
}

long DeskShareReceiver::DeleteTrack()
{
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

    m_pTrack->RemoveObserver(WMEIID_IWmeRemoteVideoTrackObserver, (IWmeRemoteVideoTrackObserver *)this);

    if (m_pSyncBox) {
        m_pSyncBox->RemoveTrack(m_pTrack);
    }

    return MediaClient::DeleteTrack();
}

/// for IWmeRemoteVideoTrackObserver
WMERESULT DeskShareReceiver::OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

/// for IWmeRemoteVideoTrackObserver
WMERESULT DeskShareReceiver::OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

/// for IWmeRemoteVideoTrackObserver
WMERESULT DeskShareReceiver::OnKeyFrameLost(uint32_t uLabel, uint32_t uDID)
{
    returnv_if_fail(m_pSession != NULL, WME_E_FAIL);
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

    uint32_t uTrackLabel = 0;
    m_pTrack->GetTrackLabel(uTrackLabel);
    m_pSession->IndicatePictureLoss(uTrackLabel, uLabel);
    return WME_S_OK;
}

/// for IWmeRemoteVideoTrackObserver
WMERESULT DeskShareReceiver::OnRenderBlocked(uint32_t uLabel, bool bBlocked)
{
    return WME_S_OK;
}

/// for IWmeRemoteVideoTrackObserver
WMERESULT DeskShareReceiver::OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight)
{
    if (m_pSink) {
        m_pSink->OnDecodeSizeChanged(uLabel, uWidth, uHeight);
    }
	return WME_S_OK;
}