#ifndef _AUDIO_RECEIVER_H_
#define _AUDIO_RECEIVER_H_

#include "IMediaClient.h"

class AudioReceiver : 
    public MediaReceiver
{
public:
    AudioReceiver();
    virtual ~AudioReceiver();

    /// for MediaClient
    virtual long CreateTrack();
    virtual long DeleteTrack();
//    virtual long StartTrack();
//    virtual long StopTrack();
    
    long SetAudioOutputFile(const char* pFileName);
    void SetAudioTarget(bool file_tar) {m_file_target = file_tar;}

    // for TA
    long GetVoiceLevel(unsigned int &level);
    long SetFileRenderSink(IWmeMediaFileRenderSink* pSink)
    {
        if(!m_pFileRender)
            return 0;
        return m_pFileRender->SetRenderSink(pSink);
    }    
private:
    bool m_file_target;
    IWmeMediaFileRenderer*  m_pFileRender;
    WmeAudioRawFormat m_hardcode;
};


#endif // _AUDIO_RECEIVER_H_
