#include "MediaManager.h"

/// =================
/// for media manager

MediaManager::MediaManager(WmeMediaType wmt) : m_wmt(wmt)
{
    m_pSink = NULL;
    m_pEngine = NULL;
    m_pCodecEnum = NULL;
}

MediaManager::~MediaManager()
{
    SAFE_RELEASE(m_pEngine);
    SAFE_RELEASE(m_pCodecEnum);
    ClearMediaCodecs(m_CodecList);
}

void MediaManager::SetSink(IMediaManagerSink *pSink)
{
    m_pSink = pSink;
}

void MediaManager::SetMediaEngine(IWmeMediaEngine *pEngine)
{
    SAFE_RELEASE(m_pEngine);
    if (pEngine) {
        m_pEngine = pEngine;
        m_pEngine->AddRef();
    }
}

long MediaManager::QueryMediaCodec(WmeCodecType ctype, IWmeMediaCodec *& pCodec)
{
    pCodec = NULL;
    if (m_CodecList.empty()) {
        GetMediaCodecs(m_CodecList);
    }

    for(int k=0; k < m_CodecList.size(); k++) {
        IWmeMediaCodec *codec = m_CodecList[k];
        if (!codec)
            continue;

        WmeCodecType codecType;
        codec->GetCodecType(codecType);
        if (codecType == ctype) {
            pCodec = codec;
            return WME_S_OK;
        }
    }

    return WME_E_FAIL;
}

long MediaManager::GetMediaCodecs(std::vector<IWmeMediaCodec *> &codecs)
{
    returnv_if_fail(m_pEngine != NULL, WME_E_FAIL);

    if (!m_pCodecEnum) {
        long ret = m_pEngine->CreateMediaCodecEnumerator(m_wmt, &m_pCodecEnum);
        if(WME_S_OK != ret) {
            CM_ERROR_TRACE_THIS("MediaManager::QueryMediaCodecs, fail to CreateMediaCodecEnumerator m_wmt="<<m_wmt);
            return WME_E_FAIL;
        }
    }

    /// get codec number
    int nNumber = 0;
    if (m_pCodecEnum)
        m_pCodecEnum->GetNumber(nNumber);    
    
    if(nNumber == 0) {
        CM_ERROR_TRACE_THIS("MediaManager::GetMediaCodecs, GetNumber="<<nNumber);
        return WME_E_FAIL;
    }

    WMERESULT ret = WME_E_FAIL;
    IWmeMediaCodec *pCodec = NULL;
    for(int i=0; i < nNumber; i++)
    {
        if (m_pCodecEnum)
            ret = m_pCodecEnum->GetCodec(i, &pCodec);
        if(ret == WME_S_OK && pCodec) {
            codecs.push_back(pCodec);
        }
    }

    return WME_S_OK;
}

void MediaManager::ClearMediaCodecs(std::vector<IWmeMediaCodec *> &codecs)
{
    if(!codecs.empty()) {
        for(int i=0; i < codecs.size(); i++)
        {
            IWmeMediaCodec *pCodec = codecs.at(i);
            SAFE_RELEASE(pCodec);
        }
        codecs.clear();
    }
}

long MediaManager::GetMediaDevices(DEMO_DEV_TYPE dtype, std::vector<DeviceProperty> &dpList)
{
    returnv_if_fail(m_pEngine != NULL, WME_E_FAIL);

    WmeDeviceInOutType devInOut;
    if (dtype == DEV_TYPE_CAMERA || dtype == DEV_TYPE_MIC) {
        devInOut = WmeDeviceIn;
    }else {
        devInOut = WmeDeviceOut;
    }

    IWmeMediaDeviceEnumerator* pEnum = NULL;
    m_pEngine->CreateMediaDeviceEnumerator(m_wmt, devInOut, &pEnum);
    returnv_if_fail(pEnum != NULL, WME_E_FAIL);

    IWmeMediaDevice* pDefaultDev = NULL;
    m_pEngine->GetSystemDefaultDevice(m_wmt, devInOut, &pDefaultDev);
    CM_INFO_TRACE_THIS("MediaManager::GetMediaDevices, default device="<<pDefaultDev);

    int device_idx = 0;
    int32_t num = 0;
    DeviceProperty dev_property;

    long ret = pEnum->GetDeviceNumber(num);
    CM_INFO_TRACE_THIS("MediaManager::GetMediaDevices, GetDeviceNumber="<<num);
    while (num > 0 && device_idx < num)
    {
        memset(&dev_property, 0, sizeof(dev_property));
        IWmeMediaDevice *pDevice = NULL;
        ret = pEnum->GetDevice(device_idx, &pDevice);
        if (pDevice)
        {
            dev_property.dev_index = device_idx;
            dev_property.dev_type = dtype;

            int32_t len = sizeof(dev_property.unique_name);
            ret = pDevice->GetUniqueName(dev_property.unique_name, len);
            dev_property.unique_name_len = len;
            len = sizeof(dev_property.dev_name);
            ret = pDevice->GetFriendlyName(dev_property.dev_name, len);
            dev_property.dev_name_len = len;

            dev_property.dev = pDevice;
            dev_property.is_default_dev = 0;
            if(WME_S_OK == pDevice->IsSameDevice(pDefaultDev))
            {
                dev_property.is_default_dev = 1;
            }
            
            dev_property.position = DEV_POSITION_UNKNOWN;
            if (dtype == DEV_TYPE_CAMERA) {
                IWmeVideoCapDevicePosition *pWmeVideoCapDevicePosition = NULL;
                WmeVideoCapDevicePosition position = WME_VIDEO_CAP_DEVICE_POSITION_UNKNOWN;
                pDevice->QueryInterface(WMEIID_IWmeVideoCapDevicePosition, (void**)&pWmeVideoCapDevicePosition);
            
                if (pWmeVideoCapDevicePosition)
                {
                    pWmeVideoCapDevicePosition->GetPosition(&position);
                    if (position == WME_VIDEO_CAP_DEVICE_POSITION_FRONT) {
                        dev_property.position = DEV_CAMERA_FRONT;
                        dev_property.is_default_dev = 1;
                    }else if (position == WME_VIDEO_CAP_DEVICE_POSITION_BACK) {
                        dev_property.position = DEV_CAMERA_BACK;
                        dev_property.is_default_dev = 0;
                    }
                }
            }else if (dtype == DEV_TYPE_SPEAKER) {
                /// TODO: for audio speaker's built-in device or non built-in device
            }
            dpList.push_back(dev_property);
        }
        device_idx++;
    }
    SAFE_RELEASE(pEnum);
    SAFE_RELEASE(pDefaultDev);

    return WME_S_OK;
}

long MediaManager::ClearMediaDevices(std::vector<DeviceProperty> &dpList)
{
    if(!dpList.empty()) {
        for(int i=0; i < dpList.size(); i++)
        {
            IWmeMediaDevice *pDevice = dpList[i].dev;
            SAFE_RELEASE(pDevice);
        }
        dpList.clear();
    }
    return WME_S_OK;
}

long MediaManager::GetMediaCapabilities(void * pMcList)
{
    returnv_if_fail(pMcList != NULL, WME_E_INVALIDARG);
    returnv_if_fail(m_pEngine != NULL, WME_E_FAIL);

    CM_INFO_TRACE_THIS("MediaManager::GetMediaCapabilities, m_wmt="<<m_wmt);

    IWmeMediaInfoGetter* pGetter = NULL;
    m_pEngine->CreateMediaInfoGetter(m_wmt, &pGetter);
    returnv_if_fail(pGetter != NULL, WME_E_FAIL);

    if(m_wmt == WmeMediaTypeVideo)
    {
        WmeMediaInfo_VideoMax vMax;
        vMax.maxWidth = MAX_VIDEO_WIDTH;
        vMax.maxHeight = MAX_VIDEO_HEIGHT;
        vMax.maxFrameRate = MAX_VIDEO_FPS;
        vMax.maxLayerNum = MAX_VIDEO_SPACIAL_LAYER;
        if(WME_S_OK != pGetter->SetConfig(WmeWIC_VideoMax, &vMax, sizeof(vMax))) {
            SAFE_RELEASE(pGetter);
            CM_ERROR_TRACE_THIS("MediaManager::GetMediaCapabilities, fail to SetConfig for WmeWIC_VideoMax");
            return WME_E_FAIL;
        }
    }

    int index = 0;
    int nTotalNumber = 0;
    pGetter->GetInfoNumber(nTotalNumber);
    CM_INFO_TRACE_THIS("MediaManager::GetMediaCapabilities, nTotalNumber="<<nTotalNumber);
    for (; index < nTotalNumber; index++)
    {
        IWmeMediaInfo* pInfo = NULL;
        if(WME_S_OK != pGetter->GetMediaInfo(index, &pInfo)) {
            CM_ERROR_TRACE_THIS("MediaManager::GetMediaCapabilities, fail to GetMediaInfo");
            continue;
        }

        WmeMediaType wmt = m_wmt;
        if(WME_S_OK != pInfo->GetMediaType(wmt)) {
            SAFE_RELEASE(pInfo);
            CM_ERROR_TRACE_THIS("MediaManager::GetMediaCapabilities, fail to GetMediaType");
            break;
        }

        int32_t cap_num = 0;
        if(WME_S_OK != pInfo->GetCapabilityNumber(cap_num) || cap_num == 0) {
            SAFE_RELEASE(pInfo);
            CM_ERROR_TRACE_THIS("MediaManager::GetMediaCapabilities, fail to GetCapabilityNumber");
            break;
        }

        int i = 0;
        if(wmt == WmeMediaTypeAudio)
        {
            WmeAudioMediaCapability cap;
            while (WME_S_OK == pInfo->GetMediaCapabilities(i, &cap, sizeof(cap)))
            {
                ((std::vector<WmeAudioMediaCapability> *)pMcList)->push_back(cap);
                i++;
            }
        }
        else
        {
            WmeVideoMediaCapability cap; ///???: cannot memset of this sturcture
            while (WME_S_OK == pInfo->GetMediaCapabilities(i, &cap, sizeof(cap)))
            {
                ((std::vector<WmeVideoMediaCapability> *)pMcList)->push_back(cap);
                i++;
            }
        }
        SAFE_RELEASE(pInfo);
    }
    SAFE_RELEASE(pGetter);

    return (index == 0) ? WME_E_FAIL : WME_S_OK;
}


/// ==================
/// for audio manager

AudioManager::AudioManager() : MediaManager(WmeMediaTypeAudio)
{
    m_pDeviceCtrl = NULL;
    m_pEventNotifier[AUDIO_DEV_MIC] = NULL;
    m_pEventNotifier[AUDIO_DEV_SPEAKER] = NULL;
    m_pVolumeCtrl[AUDIO_DEV_MIC] = NULL;
    m_pVolumeCtrl[AUDIO_DEV_SPEAKER] = NULL;
}

AudioManager::~AudioManager()
{
    UnInit();
}

long AudioManager::Init()
{
    returnv_if_fail(m_pEngine != NULL, WME_E_FAIL);

    long ret = WME_E_FAIL;
    ret = m_pEngine->CreateAudioDeviceController(&m_pDeviceCtrl);
    if (ret != WME_S_OK) {
        CM_ERROR_TRACE_THIS("AudioManager::Init, fail to CreateAudioDeviceController ret="<<ret);
        return WME_E_FAIL;
    }

    /// for MIC devices notifier
    ret = m_pEngine->CreateMediaDevicesNotifier(WmeMediaTypeAudio, WmeDeviceIn, &m_pEventNotifier[AUDIO_DEV_MIC]);
    if (ret == WME_S_OK) {
        m_pEventNotifier[AUDIO_DEV_MIC]->AddObserver(WMEIID_IWmeMediaDeviceObserver, this);
    }

    /// for SPEAKER devices notifier
    ret = m_pEngine->CreateMediaDevicesNotifier(WmeMediaTypeAudio, WmeDeviceOut, &m_pEventNotifier[AUDIO_DEV_SPEAKER]);
    if (ret == WME_S_OK) {
        m_pEventNotifier[AUDIO_DEV_SPEAKER]->AddObserver(WMEIID_IWmeMediaDeviceObserver, this);
    }

    /// for MIC volume control
    ret = m_pEngine->CreateAudioVolumeController(&m_pVolumeCtrl[AUDIO_DEV_MIC], WmeDeviceIn);
    if(WME_S_OK != ret) {
        CM_ERROR_TRACE_THIS("AudioManager::Init, fail to CreateAudioVolumeController for mic!");
        return WME_E_FAIL;
    }

    {   /// Add observer into mic volume control
        IWmeMediaEventNotifier *pVolumeNotifier = NULL;
        m_pVolumeCtrl[AUDIO_DEV_MIC]->QueryInterface(WMEIID_IWmeAudioVolumeNotifier, (void **) &pVolumeNotifier);
        if (pVolumeNotifier) {
            pVolumeNotifier->AddObserver(WMEIID_IWmeAudioVolumeObserver, this);
        }
    }

    /// for SPEAKER volume control
    ret = m_pEngine->CreateAudioVolumeController(&m_pVolumeCtrl[AUDIO_DEV_SPEAKER], WmeDeviceOut);
    if(WME_S_OK != ret) {
        CM_ERROR_TRACE_THIS("AudioManager::Init, fail to CreateAudioVolumeController for mic!");
        return WME_E_FAIL;
    }

    {   /// add observer into SPEAKER volume control
        IWmeMediaEventNotifier *pVolumeNotifier = NULL;
        m_pVolumeCtrl[AUDIO_DEV_SPEAKER]->QueryInterface(WMEIID_IWmeAudioVolumeNotifier, (void **) &pVolumeNotifier);
        if (pVolumeNotifier) {
            pVolumeNotifier->AddObserver(WMEIID_IWmeAudioVolumeObserver, this);
        }
    }

    return WME_S_OK;
}

long AudioManager::UnInit()
{
    SAFE_RELEASE(m_pDeviceCtrl);
    SAFE_RELEASE(m_pEventNotifier[AUDIO_DEV_MIC]);
    SAFE_RELEASE(m_pEventNotifier[AUDIO_DEV_SPEAKER]);
    SAFE_RELEASE(m_pVolumeCtrl[AUDIO_DEV_MIC])
        SAFE_RELEASE(m_pVolumeCtrl[AUDIO_DEV_SPEAKER]);
    return WME_S_OK;
}

long AudioManager::SetDevice(IWmeMediaDevice* pDevice, AUDIO_DEV dev)
{
    returnv_if_fail(pDevice != NULL, WME_E_INVALIDARG);
    returnv_if_fail(dev > AUDIO_DEV_NONE && dev < AUDIO_DEV_NUM, WME_E_INVALIDARG);
    returnv_if_fail(m_pDeviceCtrl != NULL, WME_E_FAIL);

    long ret = WME_E_FAIL;
    switch(dev) {
        case AUDIO_DEV_MIC:
            ret = m_pDeviceCtrl->SetCaptureDevice(pDevice);
            break;
        case AUDIO_DEV_SPEAKER:
            ret = m_pDeviceCtrl->SetPlayDevice(pDevice);
            break;
        default:
            break;
    }
    return ret;
}

long AudioManager::GetDevice(IWmeMediaDevice** ppDevice, AUDIO_DEV dev)
{
	returnv_if_fail(ppDevice != NULL, WME_E_INVALIDARG);
	returnv_if_fail(m_pDeviceCtrl != NULL, WME_E_FAIL);

	long ret = WME_E_FAIL;
	switch(dev) {
		case AUDIO_DEV_MIC:
			ret = m_pDeviceCtrl->GetCaptureDevice(ppDevice);
			break;
		case AUDIO_DEV_SPEAKER:
			ret = m_pDeviceCtrl->GetPlayDevice(ppDevice);
			break;
		default:
			break;
	}
	return ret;
}

long AudioManager::SetAudioOutType(int nType)
{
    returnv_if_fail(m_pDeviceCtrl != NULL, WME_E_FAIL);
	long ret = WME_E_FAIL;
//#ifdef ANDROID
	IWmeMediaDeviceEnumerator* pEnum = NULL;
    m_pEngine->CreateMediaDeviceEnumerator(m_wmt, WmeDeviceOut, &pEnum);
    returnv_if_fail(pEnum != NULL, WME_E_FAIL);
	int nSpeakerNum = 0;
	pEnum->GetDeviceNumber(nSpeakerNum);
	for(int i=0;i<nSpeakerNum;i++)
	{
		if(i== nType)
		{
			IWmeMediaDevice *pdevice = NULL;
			pEnum->GetDevice(i,&pdevice);
			if(NULL != pdevice)
			{
				ret=SetDevice(pdevice, AUDIO_DEV_SPEAKER);
			}
			break;
		}
	}

    //m_pDeviceCtrl->SetPlayOutStreamType(nType);
//#endif
    return ret;
}

long AudioManager::GetVolume(int &vol, AUDIO_DEV dev)
{
    returnv_if_fail(dev > AUDIO_DEV_NONE && dev < AUDIO_DEV_NUM, WME_E_INVALIDARG);
    returnv_if_fail(m_pVolumeCtrl[dev] != NULL, WME_E_FAIL);

    vol = m_pVolumeCtrl[dev]->GetVolume();
    return WME_S_OK;
}

long AudioManager::SetVolume(int vol, AUDIO_DEV dev)
{
    returnv_if_fail(dev > AUDIO_DEV_NONE && dev < AUDIO_DEV_NUM, WME_E_INVALIDARG);
    returnv_if_fail(m_pVolumeCtrl[dev] != NULL, WME_E_FAIL);

    if (vol < 0 || vol > 65535) {
        return WME_E_FAIL;
    }

    return m_pVolumeCtrl[dev]->SetVolume(vol);
}

long AudioManager::GetMute(bool &bmute, AUDIO_DEV dev)
{
    returnv_if_fail(dev > AUDIO_DEV_NONE && dev < AUDIO_DEV_NUM, WME_E_INVALIDARG);
    returnv_if_fail(m_pVolumeCtrl[dev] != NULL, WME_E_FAIL);

    bmute = m_pVolumeCtrl[dev]->IsMute();
    return WME_S_OK;
}

long AudioManager::SetMute(bool bmute, AUDIO_DEV dev)
{
    returnv_if_fail(dev > AUDIO_DEV_NONE && dev < AUDIO_DEV_NUM, WME_E_INVALIDARG);
    returnv_if_fail(m_pVolumeCtrl[dev] != NULL, WME_E_FAIL);

    if (bmute)
        m_pVolumeCtrl[dev]->Mute();
    else
        m_pVolumeCtrl[dev]->UnMute();
    return WME_S_OK;
}

WMERESULT AudioManager::OnDeviceChanged(IWmeMediaEventNotifier *pNotifier, WmeEventDataDeviceChanged &changeEvent)
{
    returnv_if_fail(pNotifier != NULL, WME_E_INVALIDARG);
    returnv_if_fail(m_pSink != NULL, WME_E_FAIL);

    if(pNotifier == m_pEventNotifier[AUDIO_DEV_MIC])
    {
        m_pSink->OnAudioInChanged(changeEvent);
    }
    else if(pNotifier == m_pEventNotifier[AUDIO_DEV_SPEAKER])
    {
        m_pSink->OnAudioOutChanged(changeEvent);
    }

    return WME_S_OK;
}

WMERESULT AudioManager::OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

WMERESULT AudioManager::OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

void AudioManager::OnVolumeChange(IWmeMediaEventNotifier *pNotifier, WmeEventDataVolumeChanged &changeEvent)
{
    return_if_fail(m_pSink != NULL);
    m_pSink->OnVolumeChanged(changeEvent);
}

/// =================
/// for video manager

VideoManager::VideoManager() : MediaManager(WmeMediaTypeVideo)
{
    m_pCameraNotifier = NULL;
}

VideoManager::~VideoManager()
{
    UnInit();
}

long VideoManager::Init()
{
    returnv_if_fail(m_pEngine != NULL, WME_E_FAIL);

    long ret = WME_E_FAIL;

    ret = m_pEngine->CreateMediaDevicesNotifier(WmeMediaTypeVideo, WmeDeviceIn, &m_pCameraNotifier);
    if(WME_S_OK == ret && m_pCameraNotifier)
    {
        m_pCameraNotifier->AddObserver(WMEIID_IWmeMediaDeviceObserver, this);
    }

    return ret;
}

long VideoManager::UnInit()
{
    SAFE_RELEASE(m_pCameraNotifier);
    return WME_S_OK;
}

WMERESULT VideoManager::OnDeviceChanged(IWmeMediaEventNotifier *pNotifier, WmeEventDataDeviceChanged &changeEvent)
{
    returnv_if_fail(m_pSink != NULL, WME_E_FAIL);

    if(pNotifier == m_pCameraNotifier)
    {
        m_pSink->OnVideoInChanged(changeEvent);
    }

    return WME_S_OK;
}

WMERESULT VideoManager::OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

WMERESULT VideoManager::OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

ShareSourceManager::ShareSourceManager(): MediaManager(WmeMediaTypeVideo), m_pIWmeAppSourceEnumerator(NULL)
{
    m_pIWmeScreenSourceEnumerator = NULL;
    m_pIWmeScreenSourceCurDesktop = NULL;
}
ShareSourceManager::~ShareSourceManager()
{
    SAFE_RELEASE(m_pIWmeScreenSourceEnumerator);
    SAFE_RELEASE(m_pIWmeScreenSourceCurDesktop);
    SAFE_RELEASE(m_pIWmeAppSourceEnumerator)
}

long ShareSourceManager::Init()
{
    returnv_if_fail(m_pEngine != NULL, WME_E_FAIL);
    
    long ret = WME_E_FAIL;
    m_pEngine->CreateScreenSourceEnumerator(&m_pIWmeAppSourceEnumerator,wme::WmeScreenSourceType::WmeScreenSourceTypeApplication);
    if( m_pIWmeAppSourceEnumerator )
        m_pIWmeAppSourceEnumerator->AddObserver(WMEIID_IWmeScreenSourceEnumeratorEventObserver, this);
    ret = m_pEngine->CreateScreenSourceEnumerator(&m_pIWmeScreenSourceEnumerator,wme::WmeScreenSourceType::WmeScreenSourceTypeDesktop);
    if( m_pIWmeScreenSourceEnumerator )
        m_pIWmeScreenSourceEnumerator->AddObserver(WMEIID_IWmeScreenSourceEnumeratorEventObserver, this);
    
    return ret;

}
long ShareSourceManager::UnInit()
{
    SAFE_RELEASE(m_pIWmeAppSourceEnumerator);
    SAFE_RELEASE(m_pIWmeScreenSourceEnumerator);
    return WME_S_OK;
    
}

IWmeScreenSource * ShareSourceManager::GetOneScreenSource(){
    
    SAFE_RELEASE(m_pIWmeScreenSourceCurDesktop);
    if(m_pIWmeScreenSourceEnumerator){
        int32_t iNumber =0;
        m_pIWmeScreenSourceEnumerator->GetNumber(iNumber);
        if(iNumber>0){
            m_pIWmeScreenSourceEnumerator->GetSource(0, &m_pIWmeScreenSourceCurDesktop);
            //m_pIWmeScreenSourceCurDesktop->AddRef();
        }
    }
    return m_pIWmeScreenSourceCurDesktop;
}

IWmeScreenSource * ShareSourceManager::GetScreenSourceByIndex(int idx)
{
    IWmeScreenSource *ret = NULL;
    if(idx>=0 && m_pIWmeScreenSourceEnumerator){
        int32_t iNumber =0;
        m_pIWmeScreenSourceEnumerator->GetNumber(iNumber);
        if(iNumber>idx){
            m_pIWmeScreenSourceEnumerator->GetSource(idx, &ret);
        }
    }
    return ret;
}

IWmeScreenSource * ShareSourceManager::GetAppSourceSourceByIndex(int idx)
{
    IWmeScreenSource *ret = NULL;
    if(idx>=0 && m_pIWmeAppSourceEnumerator){
        int32_t iNumber =0;
        m_pIWmeAppSourceEnumerator->GetNumber(iNumber);
        if(iNumber>idx){
            m_pIWmeAppSourceEnumerator->GetSource(idx, &ret);
        }
    }
    return ret;
}

int ShareSourceManager::GetScreenSourceNames(std::vector<std::string> &screens)
{
    if( m_pIWmeScreenSourceEnumerator )
    {
        int count(0);
        m_pIWmeScreenSourceEnumerator->GetNumber(count);
        IWmeScreenSource *source;
        char name[256];
        int len;
        for (int i=0; i<count; i++) {
            if( 0 == m_pIWmeScreenSourceEnumerator->GetSource(i, &source) )
            {
                source->GetFriendlyName(name, len);
                if( len > 0 )
                    screens.push_back(name);
                else
                    screens.push_back("No Name");
            }
            else {
                screens.push_back("");
            }
        }
        return count;
    }
    return 0;
}

int ShareSourceManager::GetAppSourceSourceNames(std::vector<std::string> &apps)
{
    if( m_pIWmeAppSourceEnumerator )
    {
        int count(0);
        m_pIWmeAppSourceEnumerator->GetNumber(count);
        IWmeScreenSource *source;
        char name[256];
        int len;
        for (int i=0; i<count; i++) {
            if( 0 == m_pIWmeAppSourceEnumerator->GetSource(i, &source) )
            {
                len = sizeof(name);
                name[0] = '\0';
                source->GetUniqueName(name, len);
                if( strlen(name) > 0 )
                    apps.push_back(name);
                else
                    apps.push_back("No Name");
            }
            else {
                apps.push_back("");
            }
        }
        return count;
    }
    return 0;
}

// for IWmeScreenSourceEnumeratorEventObserver
WMERESULT ShareSourceManager::OnScreenSourceChanged(IWmeMediaEventNotifier *pNotifier,
                                                            WmeScreenSourceEvent event,
                                                            IWmeScreenSource *pScreenSource)
{
    
    return WME_S_OK;
    
}

WMERESULT ShareSourceManager::OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer,
                                                          REFWMEIID ulObserverID)
{
    
    return WME_S_OK;
    
}
WMERESULT ShareSourceManager::OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer,
                                                            REFWMEIID ulObserverID)
{
    
    return WME_S_OK;
    
}
