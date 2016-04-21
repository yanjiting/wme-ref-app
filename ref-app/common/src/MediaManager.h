#ifndef _MEDIA_MANAGER_H_
#define _MEDIA_MANAGER_H_

#include "DemoParameters.h"

///
/// NOTE:
/// Both AudioManager and VideoManager are seperate modules, focus on codec, devices and etc.
///

class IMediaManagerSink
{
public:
    virtual ~IMediaManagerSink() {}

    virtual long OnAudioInChanged(WmeEventDataDeviceChanged &changeEvent) { return WME_S_OK; }
    virtual long OnAudioOutChanged(WmeEventDataDeviceChanged &changeEvent) { return WME_S_OK; }
    virtual long OnVolumeChanged(WmeEventDataVolumeChanged &changeEvent) { return WME_S_OK; }
    virtual long OnVideoInChanged(WmeEventDataDeviceChanged &changeEvent) {return WME_S_OK; }
    
};

/// for Device Observer, Codec Enumerator, Device Enumerator, and etc.
class MediaManager
{
public:
    explicit MediaManager(WmeMediaType wmt);
    virtual ~MediaManager();
    
    virtual void SetSink(IMediaManagerSink *pSink);
    virtual void SetMediaEngine(IWmeMediaEngine *pEngine);

    virtual long QueryMediaCodec(WmeCodecType ctype, IWmeMediaCodec *& pCodec);
    virtual long GetMediaCodecs(std::vector<IWmeMediaCodec *> &codecs);
    virtual void ClearMediaCodecs(std::vector<IWmeMediaCodec *> &codecs);

    virtual long GetMediaDevices(DEMO_DEV_TYPE dtype, std::vector<DeviceProperty> &dpList);
    virtual long ClearMediaDevices(std::vector<DeviceProperty> &dpList);

    virtual long GetMediaCapabilities(void * pMcList);

protected:
    IWmeMediaEngine     *m_pEngine;
    IMediaManagerSink   *m_pSink;

    WmeMediaType        m_wmt;
    IWmeMediaCodecEnumerator        *m_pCodecEnum;
    std::vector<IWmeMediaCodec *>   m_CodecList;
};

/// for extra audio features
enum AUDIO_DEV{
    AUDIO_DEV_NONE = -1,
    AUDIO_DEV_MIC = 0,
    AUDIO_DEV_SPEAKER,
    AUDIO_DEV_NUM
};

class AudioManager : 
    public MediaManager,
    public IWmeMediaDeviceObserver,
    public IWmeAudioVolumeObserver
{
public:
    AudioManager();
    virtual ~AudioManager();

    long Init();
    long UnInit();

    /// for device
    long SetDevice(IWmeMediaDevice* pDevice, AUDIO_DEV dev); 
	long GetDevice(IWmeMediaDevice** ppDevice, AUDIO_DEV dev); 

    /// only valid for Android
    long SetAudioOutType(int nType);

    /// for volume
    long GetVolume(int &vol, AUDIO_DEV dev);
    long SetVolume(int vol, AUDIO_DEV dev);
    long GetMute(bool &bmute, AUDIO_DEV dev);
    long SetMute(bool bmute, AUDIO_DEV dev);

    // for IWmeMediaDeviceObserver
    virtual WMERESULT OnDeviceChanged(IWmeMediaEventNotifier *pNotifier, WmeEventDataDeviceChanged &changeEvent);
    virtual WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);

    // for IWmeAudioVolumeObserver
    virtual void OnVolumeChange(IWmeMediaEventNotifier *pNotifier, WmeEventDataVolumeChanged &changeEvent);

protected:
    IWmeAudioDeviceController *m_pDeviceCtrl;       // share between mic and speaker
    IWmeMediaEventNotifier    *m_pEventNotifier[AUDIO_DEV_NUM];
    IWmeAudioVolumeController *m_pVolumeCtrl[AUDIO_DEV_NUM];
};


/// for video extra features
class VideoManager : 
    public MediaManager,
    public IWmeMediaDeviceObserver
{
public:
    VideoManager();
    virtual ~VideoManager();

    long Init();
    long UnInit();

    // for IWmeMediaDeviceObserver
    virtual WMERESULT OnDeviceChanged(IWmeMediaEventNotifier *pNotifier, WmeEventDataDeviceChanged &changeEvent);
    virtual WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);

private:
    IWmeMediaEventNotifier    *m_pCameraNotifier;
};

class ShareSourceManager : public MediaManager
                        , public IWmeScreenSourceEnumeratorEventObserver
{
public:
    ShareSourceManager();
    virtual ~ShareSourceManager();
    
    long Init();
    long UnInit();
    
    //
    IWmeScreenSource * GetOneScreenSource();
    IWmeScreenSource * GetScreenSourceByIndex(int idx);
    IWmeScreenSource * GetAppSourceSourceByIndex(int idx);
    int GetScreenSourceNames(std::vector<std::string> &screens);
    int GetAppSourceSourceNames(std::vector<std::string> &apps);
    
    // for IWmeScreenSourceEnumeratorEventObserver
    virtual WMERESULT OnScreenSourceChanged(IWmeMediaEventNotifier *pNotifier, WmeScreenSourceEvent event,
                                            IWmeScreenSource *pScreenSource);
    virtual WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    
protected:
    IWmeScreenSourceEnumerator * m_pIWmeScreenSourceEnumerator;
    IWmeScreenSource * m_pIWmeScreenSourceCurDesktop;
    IWmeScreenSourceEnumerator * m_pIWmeAppSourceEnumerator;
};
#endif // _MEDIA_MANAGER_H_
