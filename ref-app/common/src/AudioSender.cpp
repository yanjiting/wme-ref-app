#include "AudioSender.h"

AudioSender::AudioSender():AudioSettings(m_pTrack)
{
    m_src = 0; //0 is mic; 1 is file;
    m_pMediaFileCap = NULL;
}

AudioSender::~AudioSender()
{
    CM_INFO_TRACE_THIS("AudioSender destructor");
    if (m_pMediaFileCap)
    {
        m_pMediaFileCap->RemoveObserver(WMEIID_IWmeMediaFileCapturerObserver, this);
        m_pMediaFileCap->Release();
        m_pMediaFileCap = NULL;
    }
}

long AudioSender::CreateTrack()
{
    returnv_if_fail(m_pEngine != NULL, WME_E_FAIL);
    
    if (m_pTrack) {
        CM_INFO_TRACE_THIS("AudioSender::CreateTrack, m_pTrack is not NULL!");
        return WME_S_OK;
    }

    long ret = WME_S_FALSE;

    if (m_src == 0)
    {
        ret = m_pEngine->CreateLocalAudioTrack((IWmeLocalAudioTrack **)(&m_pTrack));
        if (ret != WME_S_OK || !m_pTrack) {
            CM_ERROR_TRACE_THIS("AudioSender::CreateTrack, fail to CreateLocalAudioTrack!");
            return WME_E_FAIL;
        }
        CM_INFO_TRACE_THIS("AudioSender::CreateTrack, CreateLocalAudioTrack!");
    }
    else
    {
        ret = m_pEngine->CreateLocalAudioExternalTrack((IWmeLocalAudioExternalTrack**)(&m_pTrack));
        if (ret != WME_S_OK || !m_pTrack) {
            CM_ERROR_TRACE_THIS("AudioSender::CreateTrack, fail to CreateLocalAudioExternalTrack!");
            return WME_E_FAIL;
        }
		bool bEnable = false;
		m_pTrack->SetOption(WmeTrackOption_EnableDropSeconds,(void*)&bEnable,sizeof(bool));
        CM_INFO_TRACE_THIS("AudioSender::CreateTrack, CreateLocalAudioExternalTrack!");

        if (m_pMediaFileCap)
        {
            IWmeExternalInputter *pInputter = NULL;
            ((IWmeLocalAudioExternalTrack *)m_pTrack)->GetExternalInputter(&pInputter);

            m_pMediaFileCap->SetExternalInputter(WmeMediaTypeAudio, pInputter);
            
            pInputter->Release();
            //m_pMediaFileCap->Start();
        }
    }
    uint32_t label = kAudioLocalTrack_Label;
    m_pTrack->SetTrackLabel(label);

    return WME_S_OK;
}

long AudioSender::DeleteTrack()
{
    CM_INFO_TRACE_THIS("AudioSender::DeleteTrack, m_pMediaFileCap="<<m_pMediaFileCap);
    StopTrack();
    SetTransport(NULL);

    return MediaClient::DeleteTrack();
}

long AudioSender::StartTrack()
{
    CM_INFO_TRACE_THIS("AudioSender::StartTrack, m_pMediaFileCap="<<m_pMediaFileCap);
    long res = WME_S_OK;


    res = MediaClient::StartTrack();
    if (res)
    {
        CM_ERROR_TRACE_THIS("AudioSender::StartTrack, start track fail, res="<<res);
        return res;
    }

    if (m_pMediaFileCap)
    {
        res = m_pMediaFileCap->Start();
        if (res)
        {
            CM_ERROR_TRACE_THIS("AudioSender::StartTrack, start file capture, res="<<res);
            return res;
        }
    }

    return res;
}

long AudioSender::StopTrack()
{
    CM_INFO_TRACE_THIS("AudioSender::StopTrack, m_pMediaFileCap="<<m_pMediaFileCap);
    long res = WME_S_OK;

    if (m_pMediaFileCap)
    {
        m_pMediaFileCap->Stop();
    }

    return MediaClient::StopTrack();
}

long AudioSender::SetAudioInputFile(const char* pFileName, WmeAudioRawFormat* pFormat)
{
    CM_INFO_TRACE_THIS("AudioSender::SetAudioInputFile, file:"<<pFileName);
    WMERESULT res = WME_S_OK;

    if (pFileName == NULL || pFormat == NULL)
    {
        return WME_E_FAIL;
    }

    if (m_pMediaFileCap)
    {
        m_pMediaFileCap->Release();
        m_pMediaFileCap = NULL;
    }

    res = m_pEngine->CreateMediaFileCapturer(&m_pMediaFileCap);
    CM_INFO_TRACE_THIS("AudioSender::SetAudioInputFile, CreateMediaFileCapturer");
    if (res != WME_S_OK || m_pMediaFileCap == NULL)
    {
        return WME_E_FAIL;
    }

    m_pMediaFileCap->AddObserver(WMEIID_IWmeMediaFileCapturerObserver, this);

    //hardcode to disable loop
    bool loop = false;
    m_pMediaFileCap->SetConfig(WmeMediaFileConfig_GoLoop, &loop, sizeof(bool));

    m_pMediaFileCap->SetCaptureFile((char*)pFileName, strlen(pFileName));

    m_pMediaFileCap->SetConfig(WmeMediaFileConfig_AudioRawFormat, pFormat, sizeof(WmeAudioRawFormat));

    return WME_S_OK;
}

WMERESULT AudioSender::OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

WMERESULT AudioSender::OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

WMERESULT AudioSender::OnEndOfStream(IWmeMediaFileCapturer* pCapturer)
{
    CM_INFO_TRACE_THIS("AudioSender::OnEndOfStream, pCapturer="<<pCapturer);

    if (m_pMediaFileCap != pCapturer)
    {
        CM_INFO_TRACE_THIS("AudioSender::OnEndOfStream, ignore unknown capturer");
        return WME_S_OK;
    }

    StopTrack();

    if (m_pSink)
        m_pSink->OnEndOfStream(DEMO_MEDIA_AUDIO);
    return WME_S_OK;
}