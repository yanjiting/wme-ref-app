#ifndef _AUDIO_SENDER_H_
#define _AUDIO_SENDER_H_

#include "IMediaClient.h"
#include "IMediaSettings.h"
#include "DemoParameters.h"

class AudioSender : 
    public MediaSender
	,public AudioSettings,
    public IWmeMediaFileCapturerObserver
{
public:
    AudioSender();
    virtual ~AudioSender();

    /// for MediaClient
    virtual long CreateTrack();
    virtual long DeleteTrack();
    virtual long StartTrack();
    virtual long StopTrack();

    void SetAudioSource(int src) { m_src = src; }   //1 is file, 0 is mic
    long SetAudioInputFile(const char* pFileName, WmeAudioRawFormat* pFormat);


    //for IWmeMediaFileCapturerObserver
    virtual WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnEndOfStream(IWmeMediaFileCapturer* pCapturer);

 protected:
 	int m_src;
 	IWmeMediaFileCapturer* m_pMediaFileCap;
};

#endif // _AUDIO_SENDER_H_
