//
//  WmeAudio.h
//  MediaSessionTest
//
//  Created by aruo on 11/20/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#ifndef __MediaSessionTest__WmeAudio__
#define __MediaSessionTest__WmeAudio__


#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) do { if(x) x->Release(); x = NULL; } while(0);
#endif

#include<vector>
#include<map>
#include "WmeDevice.h"
#include "WmeEngine.h"

using namespace wme;
using namespace std;

enum AUDIO_DEV
{
    AUDIO_DEV_NONE = -1,
    AUDIO_DEV_MIC = 0,
    AUDIO_DEV_SPEAKER,
    AUDIO_DEV_NUM
};

class WmeAudio: public IWmeMediaDeviceObserver,
                public IWmeAudioVolumeObserver
{
private:
    WmeAudio();
    WmeAudio(const WmeAudio& tmp);
    ~WmeAudio();
public:
    static WmeAudio* Instance();
    static void Destroy();
public:
    WMERESULT OnDeviceChanged(IWmeMediaEventNotifier *pNotifier, WmeEventDataDeviceChanged &stChangeEvent);
    WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    void OnVolumeChange(IWmeMediaEventNotifier *pNotifier, WmeEventDataVolumeChanged &stChangeEvent);
public:
    
    long GetAudioDeviceEnum(WmeDeviceInOutType type,vector<string>& inputdevice,vector<string>& outputdevice);
    void SetDevice(WmeDeviceInOutType type,NSInteger idx);
    
    unsigned int GetSpeakerVol();
    int SetSpeakerVol(unsigned int vol);
    unsigned int GetMicVol();
    int SetMicVol(unsigned int vol);
    
    int GetSpeakerMuteStatus();
    int GetMicphoneMuteStatus();
    
    int SetSpeakerMute(int bMute);
    int SetMicrophoneMute(int bMute);
protected:
    std::vector<string> m_vectorCaptureDevice;
    std::vector<string> m_vectorPlayDevice;
    
    map<long,IWmeMediaDevice *> m_AudioOutputDeviceList;
    map<long,IWmeMediaDevice *> m_AudioInputDeviceList;
    IWmeMediaEngine* m_pMediaEngine;
    IWmeMediaEventNotifier    *m_pEventNotifier[AUDIO_DEV_NUM];
    IWmeAudioDeviceController *m_pDeviceCtrl;
    
    IWmeAudioVolumeController* m_pMicVolumeCtrl;
    IWmeAudioVolumeController* m_pSpeakerVolumeCtrl;
};

#endif /* defined(__MediaSessionTest__WmeAudio__) */
