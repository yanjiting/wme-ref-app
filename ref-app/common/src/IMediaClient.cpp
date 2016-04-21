#include "IMediaClient.h"

MediaClient::MediaClient()
{
    m_pSink = NULL;
    m_pEngine = NULL;
    m_pSession = NULL;
    m_pTrack = NULL;
    m_bStarted = false;
}

MediaClient::~MediaClient()
{
    SAFE_RELEASE(m_pEngine);
    SAFE_RELEASE(m_pSession);
    SAFE_RELEASE(m_pTrack);
}

void MediaClient::SetSink(IMediaClientSink *pSink)
{
    m_pSink = pSink;
}

void MediaClient::SetMediaEngine(IWmeMediaEngine *pEngine)
{
    SAFE_RELEASE(m_pEngine);
    if (pEngine) {
        m_pEngine = pEngine;
        m_pEngine->AddRef();
    }
}

void MediaClient::SetMediaSession(IWmeMediaSession *pSession)
{
    SAFE_RELEASE(m_pSession);
    if (pSession) {
        m_pSession = pSession;
        m_pSession->AddRef();
    }
}

long MediaClient::SetTrackCodec(IWmeMediaCodec *pCodec)
{
    returnv_if_fail(pCodec != NULL, WME_E_FAIL);
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

    return m_pTrack->SetCodec(pCodec);
}

long MediaClient::GetTrackCodec(IWmeMediaCodec **ppCodec)
{
	returnv_if_fail(ppCodec != NULL, WME_E_FAIL);
	returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

	return m_pTrack->GetCodec(ppCodec);
}

long MediaClient::SetRtpSession(uint8_t payload, uint32_t clock)
{
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);
    returnv_if_fail(m_pSession != NULL, WME_E_FAIL);

    long ret = WME_E_FAIL;
    WmeRTPInfo rtpInfo;
    rtpInfo.uPayloadType = payload;
    rtpInfo.uClockRate = clock;
    ret = m_pSession->AddTrack(m_pTrack, rtpInfo);
    return ret;
}

long MediaClient::SetCapability(WmeMediaBaseCapability *pMC)
{
    return WME_S_OK;
}

long MediaClient::GetCapability(WmeMediaBaseCapability *pMC)
{
    return WME_S_FALSE;
}

long MediaClient::CreateTrack()
{
    return WME_S_OK;
}

long MediaClient::DeleteTrack()
{
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

    if(m_pSession) {
        m_pSession->RemoveTrack(m_pTrack);
    }

    SAFE_RELEASE(m_pTrack);
    return WME_S_OK;
}

long MediaClient::StartTrack()
{
    CM_INFO_TRACE_THIS("MediaClient::StartTrack, m_pTrack="<<m_pTrack);
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);
    m_bStarted = true;
    return m_pTrack->Start();
}

long MediaClient::StopTrack()
{
    CM_INFO_TRACE_THIS("MediaClient::StopTrack, m_pTrack="<<m_pTrack);
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);
    m_bStarted = false;
    return m_pTrack->Stop();
}

IWmeMediaTrack * MediaClient::GetTrack()
{
    return m_pTrack;
}


///=======================================================================
/// 
/// class MediaSender


long MediaSender::SetTransport(IWmeMediaTransport *sink)
{
    long ret = WME_S_FALSE;
    if (m_pSession) {
        ret = m_pSession->SetTransport(sink);
    }
    return ret;
}

long MediaSender::SetOptionForSendingData(bool bsend)
{
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);
    m_pTrack->SetOption(WmeTrackOption_SendingData, &bsend, sizeof(bsend));
    return WME_S_OK;
}

///=======================================================================
/// 
/// class MediaReceiver

MediaReceiver::MediaReceiver()
{
    m_pSyncBox = NULL;
}

MediaReceiver::~MediaReceiver()
{
    SAFE_RELEASE(m_pSyncBox);
}

void MediaReceiver::SetMediaSyncBox(IWmeMediaSyncBox *pSyncBox)
{
    SAFE_RELEASE(m_pSyncBox);
    if (pSyncBox) {
        m_pSyncBox = pSyncBox;
        m_pSyncBox->AddRef();
    }
}

