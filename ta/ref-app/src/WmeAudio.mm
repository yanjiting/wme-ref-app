//
//  WmeAudio.cpp
//  MediaSessionTest
//
//  Created by aruo on 11/20/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//


#include "WmeAudio.h"
#include "WmeInterface.h"
#include "CmDebug.h"
#include <CoreFoundation/CoreFoundation.h>
#include "Notification.h"

static WmeAudio *s_wme_audio_global = NULL;

WmeAudio* WmeAudio::Instance()
{
    if(s_wme_audio_global == NULL)
        s_wme_audio_global = new WmeAudio;
    return s_wme_audio_global;
}

void WmeAudio::Destroy(){
    if(s_wme_audio_global != NULL) {
        delete s_wme_audio_global;
        s_wme_audio_global = NULL;
    }
}

WmeAudio::WmeAudio():
m_pMediaEngine(NULL),
m_pDeviceCtrl(NULL),
m_pSpeakerVolumeCtrl(NULL),
m_pMicVolumeCtrl(NULL)
{
    WmeCreateMediaEngine(&m_pMediaEngine);
    m_pEventNotifier[AUDIO_DEV_MIC] = NULL;
    m_pEventNotifier[AUDIO_DEV_SPEAKER] = NULL;
    
    long ret = WME_E_FAIL;
    ret = m_pMediaEngine->CreateAudioDeviceController(&m_pDeviceCtrl);
    if (ret != WME_S_OK)
    {
        CM_ERROR_TRACE_THIS("WmeAudio::Init, fail to CreateAudioDeviceController ret="<<ret);
        return;
    }
    
    /// for MIC devices notifier
    ret = m_pMediaEngine->CreateMediaDevicesNotifier(WmeMediaTypeAudio, WmeDeviceIn, &m_pEventNotifier[AUDIO_DEV_MIC]);
    if (ret == WME_S_OK)
    {
        m_pEventNotifier[AUDIO_DEV_MIC]->AddObserver(WMEIID_IWmeMediaDeviceObserver, this);
    }
    
    /// for SPEAKER devices notifier
    ret = m_pMediaEngine->CreateMediaDevicesNotifier(WmeMediaTypeAudio, WmeDeviceOut, &m_pEventNotifier[AUDIO_DEV_SPEAKER]);
    if (ret == WME_S_OK)
    {
        m_pEventNotifier[AUDIO_DEV_SPEAKER]->AddObserver(WMEIID_IWmeMediaDeviceObserver, this);
    }
}

WmeAudio::~WmeAudio()
{
    m_pEventNotifier[AUDIO_DEV_MIC]->RemoveObserver(WMEIID_IWmeMediaDeviceObserver, this);
    m_pEventNotifier[AUDIO_DEV_SPEAKER]->RemoveObserver(WMEIID_IWmeMediaDeviceObserver, this);
    SAFE_RELEASE(m_pDeviceCtrl);
    SAFE_RELEASE(m_pEventNotifier[AUDIO_DEV_MIC]);
    SAFE_RELEASE(m_pEventNotifier[AUDIO_DEV_SPEAKER]);
    SAFE_RELEASE(m_pMediaEngine);
}

WMERESULT WmeAudio::OnDeviceChanged(IWmeMediaEventNotifier *pNotifier, WmeEventDataDeviceChanged &stChangeEvent)
{
    if(pNotifier == m_pEventNotifier[AUDIO_DEV_MIC])
    {
        //CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(), kCFStrMicChange, NULL, NULL, true);
        [[NSNotificationCenter defaultCenter] postNotificationName:KPSpeakerChange object:nil userInfo:nil];
    }
    else if(pNotifier == m_pEventNotifier[AUDIO_DEV_SPEAKER])
    {
        //CFNotificationCenterPostNotification(CFNotificationCenterGetLocalCenter(),kCFStrSpeakerChange, NULL, NULL, true);
        [[NSNotificationCenter defaultCenter] postNotificationName:KPMicChange object:nil userInfo:nil];
    }
    
    return WME_S_OK;
}

void WmeAudio::OnVolumeChange(IWmeMediaEventNotifier *pNotifier, WmeEventDataVolumeChanged &stChangeEvent)
{


}

WMERESULT WmeAudio::OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

WMERESULT WmeAudio::OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID)
{
    return WME_S_OK;
}

long WmeAudio::GetAudioDeviceEnum(WmeDeviceInOutType type,vector<string>& inputdevice,vector<string>& outputdevice)
{
    //    WmeDeviceInOutType devInOut;
    
    const int nMaxDeviceNum = 10;
	const int nMaxNameLen = 512;
	int nDeviceNum = 0;
	char szDeviceName[nMaxDeviceNum * nMaxNameLen];
	memset(szDeviceName, 0, sizeof(szDeviceName));
    
	IWmeMediaDevice * pDeviceItem[nMaxDeviceNum];
    
    if(m_pMediaEngine==NULL)
        return -1;
    
    IWmeMediaDeviceEnumerator * pWmeMediaDeviceEnumerator = NULL;
	m_pMediaEngine->CreateMediaDeviceEnumerator(WmeMediaTypeAudio, type, &pWmeMediaDeviceEnumerator);
    
    if (pWmeMediaDeviceEnumerator)
    {
        WMERESULT res = WME_S_OK;
		int device_idx = 0;
		int32_t num = 0;
        //DeviceProperty dev_property;
		res = pWmeMediaDeviceEnumerator->GetDeviceNumber(num);
        
        if (WME_SUCCEEDED(res))
        {
			nDeviceNum = 0;
			while (num > 0 && device_idx < num && device_idx < nMaxDeviceNum)
            {
				IWmeMediaDevice * m_device = NULL;
				res = pWmeMediaDeviceEnumerator->GetDevice(device_idx, &m_device);
                
				if (WME_SUCCEEDED(res) && m_device)
                {
					int szLen = nMaxNameLen;
					res = m_device->GetFriendlyName(szDeviceName + nMaxNameLen * nDeviceNum, szLen);
					if (WME_SUCCEEDED(res))
                    {
						pDeviceItem[nDeviceNum] = m_device;
						nDeviceNum++;
					}
				}
                
				device_idx++;
			}
		}
        
        SAFE_RELEASE(pWmeMediaDeviceEnumerator);
    }
    
    for (long i = 0; i < nDeviceNum; i++)
    {
		char *p = szDeviceName + i * nMaxNameLen;
		printf("type is %d,Device name is %s,index is %ld\n",type,p,i);
		std::string name = p;
        
        if (type == WmeDeviceIn)
        {
            //m_vectorCaptureDevice.push_back(name);
            inputdevice.push_back(name);
            m_AudioInputDeviceList.insert(std::make_pair(i,pDeviceItem[i]));
        }
        else if (type == WmeDeviceOut)
        {
            //m_vectorPlayDevice.push_back(name);
            outputdevice.push_back(name);
            m_AudioOutputDeviceList.insert(std::make_pair(i,pDeviceItem[i]));
        }
        
		CM_INFO_TRACE("[NATIVE] getAudiodeviceList, pDeviceItem" << i << " = " << pDeviceItem[i]);
	}
    
    return 0;
}

void WmeAudio::SetDevice(WmeDeviceInOutType type,NSInteger idx)
{
    if (idx >= 0)
    {
        if (type == WmeDeviceIn)
        {
            if (m_AudioInputDeviceList.find(idx)!=m_AudioInputDeviceList.end())
            {
                if (m_pDeviceCtrl)
                {
                    //printf("\n Set WmeDeviceIn index = %ld\n",idx);
                    m_pDeviceCtrl->SetCaptureDevice(m_AudioInputDeviceList[idx]);
                    
                    if (m_pMicVolumeCtrl)
                        m_pMicVolumeCtrl->Release();
                    
                    if (m_pMediaEngine)
                        m_pMediaEngine->CreateAudioVolumeController(&m_pMicVolumeCtrl, WmeDeviceInOutType::WmeDeviceIn);
                }
            }
        }
        else if (type == WmeDeviceOut)
        {
            if (m_AudioOutputDeviceList.find(idx)!=m_AudioOutputDeviceList.end())
            {
                if (m_pDeviceCtrl)
                {
                    //printf("\n Set WmeDeviceOut index = %ld\n",idx);
                    m_pDeviceCtrl->SetPlayDevice(m_AudioOutputDeviceList[idx]);
                    if (m_pSpeakerVolumeCtrl)
                        m_pSpeakerVolumeCtrl->Release();
                    
                    if (m_pMediaEngine)
                        m_pMediaEngine->CreateAudioVolumeController(&m_pSpeakerVolumeCtrl, WmeDeviceInOutType::WmeDeviceOut);
                }
                
            }
        }
    }
}

unsigned int WmeAudio::GetSpeakerVol()
{
    unsigned int volume = 0;
    if(NULL != m_pSpeakerVolumeCtrl)
    {
       volume =  m_pSpeakerVolumeCtrl->GetVolume();
    }
    return volume;
}

int WmeAudio::SetSpeakerVol(unsigned int vol)
{
    if (m_pSpeakerVolumeCtrl)
        return m_pSpeakerVolumeCtrl->SetVolume(vol);
    else
        return 0;
}

unsigned int WmeAudio::GetMicVol()
{
    unsigned int volume = 0;
    if(NULL != m_pMicVolumeCtrl)
    {
        volume =  m_pMicVolumeCtrl->GetVolume();
    }
    return volume;

}

int WmeAudio::SetMicVol(unsigned int vol)
{
    if (m_pMicVolumeCtrl)
        return m_pMicVolumeCtrl->SetVolume(vol);
    else
        return 0;
}

int WmeAudio::GetSpeakerMuteStatus()
{
    int rc =0;
    
    if (m_pSpeakerVolumeCtrl)
        rc = m_pSpeakerVolumeCtrl->IsMute();
    
    return rc;
}

int WmeAudio::GetMicphoneMuteStatus()
{
    int rc =0;
    
    if (m_pMicVolumeCtrl)
        rc = m_pMicVolumeCtrl->IsMute();
    
    return rc;
}

int WmeAudio::SetSpeakerMute(int bMute)
{
    if (m_pSpeakerVolumeCtrl)
    {
        if (bMute)
            return m_pSpeakerVolumeCtrl->Mute();
        else
            return m_pSpeakerVolumeCtrl->UnMute();
    }    
    
    return 0;
}

int WmeAudio::SetMicrophoneMute(int bMute)
{
    if (m_pMicVolumeCtrl)
    {
        if (bMute)
            return m_pMicVolumeCtrl->Mute();
        else
            return m_pMicVolumeCtrl->UnMute();
    }
    
    return 0;
}

