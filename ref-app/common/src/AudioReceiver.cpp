#include "AudioReceiver.h"

AudioReceiver::AudioReceiver()
{
    m_file_target = false;
    m_pFileRender = NULL;
    memset(&m_hardcode, 0, sizeof(m_hardcode));
    m_hardcode.eRawType = WmePCM;
    m_hardcode.iChannels = 1;
    m_hardcode.iSampleRate = 16000;
    m_hardcode.iBitsPerSample = 16;
}

AudioReceiver::~AudioReceiver()
{
    if (m_pFileRender != NULL) {
        m_pFileRender->Release();
        m_pFileRender = NULL;
    }
}

long AudioReceiver::CreateTrack()
{
    returnv_if_fail(m_pEngine != NULL, WME_E_FAIL);
    returnv_if_fail(m_pSyncBox != NULL, WME_E_FAIL);

    if (m_pTrack) {
        CM_INFO_TRACE_THIS("AudioReceiver::CreateTrack, m_pTrack is not NULL!");
        return WME_S_OK;
    }

    long ret = WME_S_FALSE;
    ret = m_pEngine->CreateRemoteAudioTrack((IWmeRemoteAudioTrack **)&m_pTrack);
    if (ret != WME_S_OK || !m_pTrack) {
        CM_ERROR_TRACE_THIS("AudioReceiver::CreateTrack, fail to CreateRemoteAudioTrack!");
        return WME_E_FAIL;
    }
	if (m_file_target == true)
	{
         //add external render to the track
        ret = ((IWmeRemoteAudioTrack*)m_pTrack)->AddExternalRenderer(m_pFileRender, &m_hardcode);
        if (ret != WME_S_OK) {
            CM_ERROR_TRACE_THIS("AudioReceiver::CreateTrack, fail to AddExternalRenderer");
            return WME_E_FAIL;
        }
        CM_INFO_TRACE_THIS("AudioReceiver::CreateTrack, AddExternalRenderer OK");
    }
    uint32_t label = kAudioRemoteTrack_Label;
    if (m_pTrack)
        m_pTrack->SetTrackLabel(label);

    /// Add remote track into sync box
    m_pSyncBox->AddTrack(m_pTrack);

    return WME_S_OK;
}

long AudioReceiver::DeleteTrack()
{
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

    if (m_pSyncBox) {
        m_pSyncBox->RemoveTrack(m_pTrack);
    }
    
    if (m_file_target == true)
    {
        //TODO:
        //1, remove external render from track
       ((IWmeRemoteAudioTrack*) m_pTrack)->RemoveExternalRenderer(m_pFileRender);
        CM_INFO_TRACE_THIS("AudioReceiver::DeleteTrack, RemoveExternalRenderer OK");
    }

    return MediaClient::DeleteTrack();
}

long AudioReceiver::GetVoiceLevel(unsigned int &level)
{
    returnv_if_fail(m_pTrack != NULL, WME_E_FAIL);

    long ret = WME_E_FAIL;
    IWmeRemoteAudioMixTrack *pMixTrack = NULL;
    if (WME_S_OK == ((IWmeRemoteAudioTrack*) m_pTrack)->GetPlayMixTrack(&pMixTrack)) {
        if (pMixTrack && pMixTrack->GetVoiceLevel(level) == WME_S_OK) {
            ret = WME_S_OK;
        }
        SAFE_RELEASE(pMixTrack);
    }
    return ret;
}

long AudioReceiver::SetAudioOutputFile(const char* pFileName)
{
    //TODO:
    //1. create file renderer
    if (m_pFileRender != NULL) {
        m_pFileRender->Release();
        m_pFileRender = NULL;
    }
    WMERESULT res = m_pEngine->CreateMediaFileRenderer(&m_pFileRender);
    if (res != WME_S_OK || m_pFileRender == NULL) {
        CM_ERROR_TRACE_THIS("AudioReceiver::SetAudioOutputFile, create media file render fail,res="<<res);
        return res;
    }
    CM_INFO_TRACE_THIS("AudioReceiver::SetAudioOutputFile, create media file render OK");
    
    //2. set file path
    res = m_pFileRender->SetTargetFile((char*)pFileName, strlen(pFileName));
    if (res != WME_S_OK) {
        CM_ERROR_TRACE_THIS("AudioReceiver::SetAudioOutputFile, set target file fail,res="<<res);
        return res;
    }
    
    //3. set config
    res = m_pFileRender->SetConfig(WmeMediaFileConfig_AudioRawFormat, &m_hardcode, sizeof(WmeAudioRawFormat));
    if (res != WME_S_OK) {
        CM_ERROR_TRACE_THIS("AudioReceiver::SetAudioOutputFile, set config fail,res="<<res);
        return res;
    }
    
    
    return WME_S_OK;
}

//long AudioReceiver::StartTrack()
//{
//    CM_INFO_TRACE_THIS("AudioReceiver::StartTrack");
//    long res = WME_S_OK;
//    
//    
//    res = MediaClient::StartTrack();
//    if (res)
//    {
//        CM_ERROR_TRACE_THIS("AudioReceiver::StartTrack, start track fail, res="<<res);
//        return res;
//    }
//    
//    
//    //TODO:
//    //1, do some extra work for file renderer
//    
//    
//    
//    return WME_S_OK;
//}
//
//
//long AudioReceiver::StopTrack()
//{
//    CM_INFO_TRACE_THIS("AudioReceiver::StopTrack");
//    long res = WME_S_OK;
//    
//    //TODO:
//    //1, do some extra work for file renderer
//    
//    
//    res = MediaClient::StopTrack();
//    if (res)
//    {
//        CM_ERROR_TRACE_THIS("AudioReceiver::StopTrack, stop track fail, res="<<res);
//        return res;
//    }
//
//    
//    
//    
//    return WME_S_OK;
//}



