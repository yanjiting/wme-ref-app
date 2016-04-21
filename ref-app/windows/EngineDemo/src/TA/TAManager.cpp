#include "stdafx.h"
#include "TAManager.h"
#include <wchar.h>
//#import "WMEDataProcess.h"

//#import "WMERenderView.h"

//extern WMERenderView *s_attendeeView;
//extern WMERenderView *s_selfView;

#define STRACE_MAX_TRACE_LEN  255
#define DEFAULT_BUFFER_LEN 1024

static TCHAR gClassNameInterWindow[] = _T("INTERWINDOW");

#define _T(str) str


TAManager::TAManager()
{
	hComponent = NULL;
	m_theTAData.m_pTASink = NULL;
	m_theTAData.m_pTraceSink = NULL;

	m_pDemoClient = NULL;

	m_mapVideoCodecName[WmeCodecType_AVC] = _T("AVC");
	m_mapVideoCodecName[WmeCodecType_SVC] = _T("SVC");
	m_mapVideoCodecName[WmeCodecType_HEVC] = _T("HEVC");
	m_mapVideoCodecName[WmeCodecType_VP8] = _T("VP8");
	m_mapVideoCodecName[WmeCodecType_Unknown] = _T("Unknown");

	m_mapAudioCodecName[WmeCodecType_Unknown] = _T("Unknown");
	m_mapAudioCodecName[WmeCodecType_G711_ULAW] = _T("G711_ULAW");
	m_mapAudioCodecName[WmeCodecType_G711_ALAW] = _T("G711_ALAW");
	m_mapAudioCodecName[WmeCodecType_ILBC] = _T("ILBC");
	m_mapAudioCodecName[WmeCodecType_OPUS] = _T("OPUS");
	m_mapAudioCodecName[WmeCodecType_G722] = _T("G722");
	m_mapAudioCodecName[WmeCodecType_CNG] = _T("CNG");

	m_bVideoClientCreated = FALSE;
	m_bAudioClientCreated = FALSE;

	m_bHost = TRUE;

	memset(m_strHostIP, 0, sizeof(m_strHostIP));

	m_senderWindowHandle = NULL;
	m_receiverWindowHandle = NULL;
	m_previewerWindowHandle = NULL;

}

TAManager::~TAManager()
{
	ClearCameraList();
	ClearMicList();
	ClearSpeakerList();

	CloseRenderWindow();

	if(hComponent)
	{
		WBXI_TANetworkDeleteInstance(hComponent);
		hComponent = NULL;
	}
}

void TAManager::SetDemoClient(DemoClient *pDemoClient)
{
	m_pDemoClient = pDemoClient;
}

void TAManager::ClearCameraList()
{
	if(!m_CameraList.empty())
	{
		for(int i=0; i<m_CameraList.size(); i++)
		{
			DeviceProperty dev = m_CameraList.at(i);
			if(dev.dev)
			{
				dev.dev->Release();
				dev.dev = NULL;
			}
		}
	}

	m_CameraList.clear();
}
void TAManager::ClearMicList()
{
	if(!m_MicList.empty())
	{
		for(int i=0; i<m_MicList.size(); i++)
		{
			DeviceProperty dev = m_MicList.at(i);
			if(dev.dev)
			{
				dev.dev->Release();
				dev.dev = NULL;
			}
		}
	}

	m_MicList.clear();
}
void TAManager::ClearSpeakerList()
{
	if(!m_SpeakerList.empty())
	{
		for(int i=0; i<m_SpeakerList.size(); i++)
		{
			DeviceProperty dev = m_SpeakerList.at(i);
			if(dev.dev)
			{
				dev.dev->Release();
				dev.dev = NULL;
			}
		}
	}

	m_SpeakerList.clear();
}

long TAManager::CreateMediaClient(DEMO_MEDIA_TYPE mType)
{
	if(mType == DEMO_MEDIA_VIDEO && m_bVideoClientCreated)
		return WME_S_OK;

	if(mType == DEMO_MEDIA_AUDIO && m_bAudioClientCreated)
		 return WME_S_OK;

	long ret = m_pDemoClient->CreateMediaClient(mType);

	if(ret == WME_S_OK)
	{
		if(mType == DEMO_MEDIA_VIDEO)
			m_bVideoClientCreated = TRUE;
		else if(mType == DEMO_MEDIA_AUDIO)
			m_bAudioClientCreated = TRUE;
	}

	return ret;
}

long TAManager::DeleteMediaClient(DEMO_MEDIA_TYPE mType)
{
	if(mType == DEMO_MEDIA_VIDEO && !m_bVideoClientCreated)
		return WME_S_OK;

	if(mType == DEMO_MEDIA_AUDIO && !m_bAudioClientCreated)
		return WME_S_OK;

	m_pDemoClient->DeleteMediaClient(mType);

	if(mType == DEMO_MEDIA_VIDEO)
		m_bVideoClientCreated = FALSE;
	else if(mType == DEMO_MEDIA_AUDIO)
		m_bAudioClientCreated = FALSE;

	return WME_S_OK;
}

void TAManager::CreateTANetwork()
{
	m_theTAData.m_pTASink = this;
	m_theTAData.m_pTraceSink = this;

	hComponent = WBXI_TANetworkNewInstance();
	if(!hComponent)
		return;
	WBXI_TANetworkSetTAData(hComponent, m_theTAData);
}

HWND TAManager::createWindow(char *pName, DEMO_WINDOW_TYPE eWindowType)
{
	HWND windowHandle = NULL;

	CString sName(pName);  
	LPCTSTR lpszName = sName; 
	int nScreenX = GetSystemMetrics(SM_CXSCREEN);
	int nScreenY = GetSystemMetrics(SM_CYSCREEN);
	int nStartY = 0;
	unsigned int x = 0;
	unsigned int y = 0;

	int nWidth = 640, nHeight = 360, nGap = 0;


	if(nScreenY > (nHeight + 20)*2)
	{
		nGap = (nScreenY - (nHeight + 20)*2)/3;
	}

	nStartY = nGap;


	if (DEMO_WINDOW_SENDER == eWindowType)
	{
		x = 200;
		y = nStartY;
	}
	else if (DEMO_WINDOW_RECEIVER == eWindowType)
	{
		x = 200;
		y = nStartY + nHeight + 20 + nGap ;
	}
	else if(DEMO_WINDOW_PREVIEW == eWindowType)
	{
		x = 600;
		y = nStartY;	
	}

	// TODO: Add your control notification handler code here
	windowHandle = ::CreateWindow(gClassNameInterWindow,
		lpszName,
		WS_VISIBLE|WS_SYSMENU|WS_THICKFRAME ,
		x,
		y,
		nWidth,
		nHeight+20,
		NULL,
		NULL,
		AfxGetInstanceHandle(),
		0);

	return windowHandle;
}




void TAManager::CreateRenderWindow()
{
	if(!m_senderWindowHandle)
	{
		m_senderWindowHandle =  createWindow("my video", DEMO_WINDOW_SENDER);
	}


	if(!m_receiverWindowHandle)
	{
		m_receiverWindowHandle =  createWindow("remote video", DEMO_WINDOW_RECEIVER);
	}

	if(!m_previewerWindowHandle)
	{
		m_previewerWindowHandle =  createWindow("my preview", DEMO_WINDOW_PREVIEW);
	}

	return;
}

void TAManager::CloseRenderWindow()
{


	if (m_senderWindowHandle )
	{
		::DestroyWindow(m_senderWindowHandle);
		m_senderWindowHandle = NULL;
	}
	
	if (m_receiverWindowHandle)
	{
		::DestroyWindow(m_receiverWindowHandle);
		m_receiverWindowHandle = NULL;
	}
	
	if(m_previewerWindowHandle)
	{
		::DestroyWindow(m_previewerWindowHandle);
		m_previewerWindowHandle = NULL;
	}

	return;
}


static void DeleteTaResult(int nNum, TAResult* &pResult)
{
	if(pResult)
	{
		for(int i=0; i<nNum; i++)
		{
			if(pResult[i].szTAStr)
			{
				delete []pResult[i].szTAStr;
				pResult[i].szTAStr = NULL;
			}
		}

		delete []pResult;
		pResult = NULL;
	}
}

int TAManager::OnTACommandHandle(unsigned long nEvent, unsigned long wParam, unsigned long lParam)//APP->TA
{
	char* clParam = NULL;
	clParam = (char*)lParam;
    TAResult taResult = {0};
	
	CreateMediaClient(DEMO_MEDIA_VIDEO);
	CreateRenderWindow();

	CreateMediaClient(DEMO_MEDIA_AUDIO);

	switch (nEvent)
	{
	case WBXI_EVENTTYPE_TA_VIDEO_GETCAPTUREDEVICENUM:
		{
			ClearCameraList();
			int nNum = 0;
			TAResult *pResult = NULL;
			if(m_pDemoClient->GetDeviceList(DEMO_MEDIA_VIDEO, DEV_TYPE_CAMERA, m_CameraList) == WME_S_OK)
			{
				nNum = m_CameraList.size();

				pResult = new TAResult;
				pResult->nStrLen = DEFAULT_BUFFER_LEN;
				pResult->szTAStr = new char[pResult->nStrLen];
				memset(pResult->szTAStr, 0, pResult->nStrLen);

				for(int i=0; i<nNum; i++)
				{

					DeviceProperty Dev = m_CameraList.at(i);

					if(i > 0)
					{
						sprintf_s(pResult->szTAStr, pResult->nStrLen, "%s#%s", pResult->szTAStr, Dev.dev_name);
					}
					else
					{
						sprintf_s(pResult->szTAStr, pResult->nStrLen, "%s", Dev.dev_name);
					}

				}

				pResult->nStrLen = strlen(pResult->szTAStr);

			}
			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_GETCAPTUREDEVICENUM, nNum, pResult);

			DeleteTaResult(1, pResult);
		}
		break;
    
	case WBXI_EVENTTYPE_TA_VIDEO_SELECTCAPTUREDEVICE:
		{
			bool bResult = 0;
			long ret = WME_E_FAIL;
			char *p = (char *)lParam;
			if(m_pDemoClient )
			{
				if(wParam < m_CameraList.size())
				{
					DeviceProperty Dev = m_CameraList.at(wParam);

					if(*p == '0')
					{
						ret = m_pDemoClient->SetCamera(DEMO_PREVIEW_TRACK, Dev.dev);
					}
					else if(*p == '1')
					{
						ret = m_pDemoClient->SetCamera(DEMO_LOCAL_TRACK, Dev.dev);
					}

//					if(ret == WME_S_OK)
					{
						bResult = 1;
					}
				}
			}
			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_SELECTCAPTUREDEVICE, bResult, NULL);
		}
		break;

	case WBXI_EVENTTYPE_TA_VIDEO_GETCURRENTCAPTUREDEVICE:
		{
			int nDevIndex = -1;

			if(m_pDemoClient )
			{
				IWmeMediaDevice *pDev = NULL;
				long ret = WME_E_FAIL;
				char *p = (char *)lParam;

				if(*p == '0')
				{
					ret = m_pDemoClient->GetCaptureDevice(DEMO_PREVIEW_TRACK, DEV_TYPE_CAMERA, &pDev);
				}
				else if(*p == '1')
				{
					ret = m_pDemoClient->GetCaptureDevice(DEMO_LOCAL_TRACK, DEV_TYPE_CAMERA, &pDev);
				}

				if(ret == WME_S_OK && pDev)
				{
					for(int i=0; i<m_CameraList.size(); i++)
					{
						DeviceProperty Dev = m_CameraList.at(i);

						if(WME_S_OK == pDev->IsSameDevice(Dev.dev))
						{
							nDevIndex = i;
							break;
						}
					}

					pDev->Release();
					pDev = NULL;
				}

			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_GETCURRENTCAPTUREDEVICE, nDevIndex, NULL);
		}
		break;


	case WBXI_EVENTTYPE_TA_VIDEO_GETENCODINGPARAMLIST:
		{
			int nNum = 0;
			TAResult *pResult = NULL;

			m_VideoCapabilityList.clear();

			if(m_pDemoClient->GetMediaCapabilities(DEMO_MEDIA_VIDEO, &m_VideoCapabilityList) == WME_S_OK)
			{
				nNum = m_VideoCapabilityList.size();

				pResult = new TAResult;
				pResult->nStrLen = DEFAULT_BUFFER_LEN;
				pResult->szTAStr = new char[pResult->nStrLen];
				memset(pResult->szTAStr, 0, pResult->nStrLen);

				for(int i=0; i<nNum; i++)
				{

					WmeVideoMediaCapability vc = m_VideoCapabilityList.at(i);

					if(i > 0)
					{
						sprintf_s(pResult->szTAStr, pResult->nStrLen, "%s#%s, %dx%d", pResult->szTAStr, m_mapVideoCodecName[vc.eCodecType].c_str(), vc.width, vc.height);
					}
					else
					{
						sprintf_s(pResult->szTAStr, pResult->nStrLen, "%s, %dx%d", m_mapVideoCodecName[vc.eCodecType].c_str(), vc.width, vc.height);
					}
					
				}

				pResult->nStrLen = strlen(pResult->szTAStr);
			}		

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_GETENCODINGPARAMLIST, nNum, pResult);

			DeleteTaResult(1, pResult);
		}
		break;

	case WBXI_EVENTTYPE_TA_VIDEO_SELECTENCODINGPARAM:
		{
			bool bResult = 0;
			long ret = WME_E_FAIL;
			if(m_pDemoClient )
			{
				if(wParam < m_VideoCapabilityList.size())
				{
					WmeVideoMediaCapability vc = m_VideoCapabilityList.at(wParam);
	
					ret = m_pDemoClient->SetMediaCodec(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK, vc.eCodecType);
					ret = m_pDemoClient->SetMediaCapability(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK, &vc);

					if(ret == WME_S_OK)
					{
						bResult = 1;
					}
				}
			}
			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_SELECTENCODINGPARAM, bResult, NULL);
		}
		break;

	case WBXI_EVENTTYPE_TA_VIDEO_CHECK_ENCODINGPARAM:
		{
			int nIndex = -1;
			long ret = WME_E_FAIL;

			if(m_pDemoClient )
			{
				WmeVideoMediaCapability wmc;
				ret = m_pDemoClient->GetCapability(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK, &wmc);

				WmeCodecType codecType = WmeCodecType_Unknown;
				ret = m_pDemoClient->GetMediaCodec(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK, codecType);

				for(int i=0; i<m_VideoCapabilityList.size(); i++)
				{
					WmeVideoMediaCapability vc = m_VideoCapabilityList.at(i);

					if(vc.eCodecType == codecType && vc.width == wmc.width 
						&& vc.height == wmc.height) //&& vc.frame_layer[vc.frame_layer_number-1] == wmc.frame_layer[wmc.frame_layer_number-1]
					{
						nIndex = i;
						break;
					}
				}
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_CHECK_ENCODINGPARAM, nIndex, NULL);

		}
		break;


	case WBXI_EVENTTYPE_TA_VIDEO_GETENCODELEVEL:
		{

		}
		break;

	case WBXI_EVENTTYPE_TA_VIDEO_STARTPREVIEW:
		{
			long ret = WME_E_FAIL;
			bool bResult = 0;

			if(m_pDemoClient)

			{
				ret = m_pDemoClient->SetRenderView(DEMO_PREVIEW_TRACK, m_previewerWindowHandle, false);
				ret = m_pDemoClient->StartMediaTrack(DEMO_MEDIA_VIDEO, DEMO_PREVIEW_TRACK);
			}

			if(ret == WME_S_OK)
				bResult = 1;

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_STARTPREVIEW, bResult, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_VIDEO_STOPPREVIEW:
		{
			long ret = WME_E_FAIL;
			bool bResult = 0;

			if(m_pDemoClient)
			{
				ret = m_pDemoClient->StopRenderView(DEMO_PREVIEW_TRACK);
				ret = m_pDemoClient->StopMediaTrack(DEMO_MEDIA_VIDEO, DEMO_PREVIEW_TRACK);
			}

			if(ret == WME_S_OK)
				bResult = 1;

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_VIDEO_STOPPREVIEW, bResult, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_GETCAPTUREDEVICENUM:
		{
			ClearMicList();

			int nNum = 0;
			TAResult *pResult = NULL;
			if(m_pDemoClient->GetDeviceList(DEMO_MEDIA_AUDIO, DEV_TYPE_MIC, m_MicList) == WME_S_OK)
			{
				nNum = m_MicList.size();

				pResult = new TAResult;
				pResult->nStrLen = DEFAULT_BUFFER_LEN;
				pResult->szTAStr = new char[pResult->nStrLen];
				memset(pResult->szTAStr, 0, pResult->nStrLen);

				for(int i=0; i<nNum; i++)
				{

					DeviceProperty Dev = m_MicList.at(i);

					if(i > 0)
					{
						sprintf_s(pResult->szTAStr, pResult->nStrLen, "%s#%s", pResult->szTAStr, Dev.dev_name);
					}
					else
					{
						sprintf_s(pResult->szTAStr, pResult->nStrLen, "%s", Dev.dev_name);
					}

				}

				pResult->nStrLen = strlen(pResult->szTAStr);

			}
			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_GETCAPTUREDEVICENUM, nNum, pResult);

			DeleteTaResult(1, pResult);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_SELECTCAPTUREDEVICE:
		{
			bool bResult = 0;
			long ret = WME_E_FAIL;

			if(m_pDemoClient )
			{
				if(wParam < m_MicList.size())
				{
					DeviceProperty Dev = m_MicList.at(wParam);
	
					ret = m_pDemoClient->SetMic(Dev.dev);

					if(ret == WME_S_OK)
					{
						bResult = 1;
					}
				}
			}
			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_SELECTCAPTUREDEVICE, bResult, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_GETCURRENTCAPTUREDEVICE: 
		{
			int nDevIndex = -1;

			if(m_pDemoClient )
			{
				IWmeMediaDevice *pDev = NULL;
				long ret = WME_E_FAIL;

				ret = m_pDemoClient->GetCaptureDevice(DEMO_LOCAL_TRACK, DEV_TYPE_MIC, &pDev);

				if(ret == WME_S_OK && pDev)
				{
					for(int i=0; i<m_MicList.size(); i++)
					{
						DeviceProperty Dev = m_MicList.at(i);

						if(WME_S_OK == pDev->IsSameDevice(Dev.dev))
						{
							nDevIndex = i;
							break;
						}
					}

					pDev->Release();
					pDev = NULL;
				}

			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_GETCURRENTCAPTUREDEVICE, nDevIndex, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_GETPLAYDEVICENUM:
		{
			ClearSpeakerList();

			int nNum = 0;
			TAResult *pResult = NULL;

			if(m_pDemoClient->GetDeviceList(DEMO_MEDIA_AUDIO, DEV_TYPE_SPEAKER, m_SpeakerList) == WME_S_OK)
			{
				nNum = m_SpeakerList.size();

				pResult = new TAResult;
				pResult->nStrLen = DEFAULT_BUFFER_LEN;
				pResult->szTAStr = new char[pResult->nStrLen];
				memset(pResult->szTAStr, 0, pResult->nStrLen);

				for(int i=0; i<nNum; i++)
				{

					DeviceProperty Dev = m_SpeakerList.at(i);
		
					if(i > 0)
					{
						sprintf_s(pResult->szTAStr, pResult->nStrLen, "%s#%s", pResult->szTAStr, Dev.dev_name);
					}
					else
					{
						sprintf_s(pResult->szTAStr, pResult->nStrLen, "%s", Dev.dev_name);
					}

				}

				pResult->nStrLen = strlen(pResult->szTAStr);
	
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_GETPLAYDEVICENUM, nNum, pResult);

			DeleteTaResult(1, pResult);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_SELECTPLAYDEVICE:
		{
			bool bResult = 0;
			long ret = WME_E_FAIL;

			if(m_pDemoClient )
			{
				if(wParam < m_SpeakerList.size())
				{
					DeviceProperty Dev = m_SpeakerList.at(wParam);

					ret = m_pDemoClient->SetSpeaker(Dev.dev);

					if(ret == WME_S_OK)
					{
						bResult = 1;
					}
				}
			}
			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_SELECTPLAYDEVICE, bResult, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_GETCURRENTPLAYDEVICE:
		{
			int nDevIndex = -1;

			if(m_pDemoClient )
			{
				IWmeMediaDevice *pDev = NULL;
				long ret = WME_E_FAIL;

				ret = m_pDemoClient->GetCaptureDevice(DEMO_PREVIEW_TRACK, DEV_TYPE_SPEAKER, &pDev);

				if(ret == WME_S_OK && pDev)
				{
					for(int i=0; i<m_SpeakerList.size(); i++)
					{
						DeviceProperty Dev = m_SpeakerList.at(i);

						if(WME_S_OK == pDev->IsSameDevice(Dev.dev))
						{
							nDevIndex = i;
							break;
						}
					}

					pDev->Release();
					pDev = NULL;
				}

			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_GETCURRENTPLAYDEVICE, nDevIndex, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_GETENCODINGPARAMLIST:
		{
			int nNum = 0;
			TAResult *pResult = NULL;

			m_AudioCapabilityList.clear();

			if(m_pDemoClient->GetMediaCapabilities(DEMO_MEDIA_AUDIO, &m_AudioCapabilityList) == WME_S_OK)
			{

				nNum = m_AudioCapabilityList.size();

				pResult = new TAResult;
				pResult->nStrLen = DEFAULT_BUFFER_LEN;
				pResult->szTAStr = new char[pResult->nStrLen];
				memset(pResult->szTAStr, 0, pResult->nStrLen);

				for(int i=0; i<nNum; i++)
				{

					WmeAudioMediaCapability ac = m_AudioCapabilityList.at(i);

					if(i > 0)
					{
						sprintf_s(pResult->szTAStr, pResult->nStrLen, "%s#%s", pResult->szTAStr, m_mapAudioCodecName[ac.eCodecType].c_str());
					}
					else
					{
						sprintf_s(pResult->szTAStr, pResult->nStrLen, "%s", m_mapAudioCodecName[ac.eCodecType].c_str());
					}

				}

				pResult->nStrLen = strlen(pResult->szTAStr);
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_GETENCODINGPARAMLIST, nNum, pResult);

			DeleteTaResult(1, pResult);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_SELECTENCODINGPARAM:
		{
			bool bResult = 0;
			long ret = WME_E_FAIL;
			if(m_pDemoClient )
			{
				if(wParam < m_AudioCapabilityList.size())
				{
					WmeAudioMediaCapability mc = m_AudioCapabilityList.at(wParam);

					ret = m_pDemoClient->SetMediaCodec(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK, mc.eCodecType);

					if(ret == WME_S_OK)
					{
						bResult = 1;
					}
				}
			}
			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_SELECTENCODINGPARAM, bResult, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_CHECK_ENCODINGPARAM:
		{
			int nIndex = -1;
			long ret = WME_E_FAIL;

			if(m_pDemoClient )
			{
				WmeCodecType codecType = WmeCodecType_Unknown;
				ret = m_pDemoClient->GetMediaCodec(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK, codecType);

				for(int i=0; i<m_AudioCapabilityList.size(); i++)
				{
					WmeAudioMediaCapability mc = m_AudioCapabilityList.at(i);

					if(mc.eCodecType == codecType)
					{
						nIndex = i;
						break;
					}
				}
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_CHECK_ENCODINGPARAM, nIndex, NULL);


		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_CAPTUREMUTE:
		{
			bool bRet = 0;

			if(m_pDemoClient)
			{
				m_pDemoClient->MuteAudio(DEV_TYPE_MIC, true);
				bRet = 1;
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_CAPTUREMUTE, bRet, NULL);
		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_CAPTUREUNMUTE:
		{
			bool bRet = 0;

			if(m_pDemoClient)
			{
				m_pDemoClient->MuteAudio(DEV_TYPE_MIC, false);
				bRet = 1;
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_CAPTUREUNMUTE, bRet, NULL);
		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_CHECK_CAPTUREMUTE:
		{
			bool bResult = 0;

			if(m_pDemoClient)
			{
				bResult = m_pDemoClient->IsAudioMute(DEV_TYPE_MIC);
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_CHECK_CAPTUREMUTE, bResult, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_SETCAPTUREVOLUME:
		{
			int nVol = wParam;

			bool bResult = 0;

			if(nVol >= 0 && nVol <= 65535)
			{
				if(m_pDemoClient)
				{
					m_pDemoClient->SetAudioVolume(DEV_TYPE_MIC, nVol);
					bResult = 1;
				}
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_SETCAPTUREVOLUME, bResult, NULL);
			
		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_GETCAPTUREVOLUME:
		{
			int nVol = -1;

			long ret = WME_E_FAIL;

			if(m_pDemoClient)
			{
				ret = m_pDemoClient->GetAudioVolume(DEV_TYPE_MIC, nVol);
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_GETCAPTUREVOLUME, nVol, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_PLAYMUTE:
		{
			bool bRet = 0;

			if(m_pDemoClient)
			{
				m_pDemoClient->MuteAudio(DEV_TYPE_SPEAKER, true);
				bRet = 1;
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_PLAYMUTE, bRet, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_PLAYUNMUTE:
		{
			bool bRet = 0;

			if(m_pDemoClient)
			{
				m_pDemoClient->MuteAudio(DEV_TYPE_SPEAKER, false);
				bRet = 1;
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_PLAYUNMUTE, bRet, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_CHECK_PLAYMUTE:
		{
			bool bResult = 0;

			if(m_pDemoClient)
			{
				bResult = m_pDemoClient->IsAudioMute(DEV_TYPE_SPEAKER);
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_CHECK_PLAYMUTE, bResult, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_SETPLAYVOLUME:
		{
			int nVol = wParam;

			bool bResult = 0;

			if(nVol >= 0 && nVol <= 65535)
			{
				if(m_pDemoClient)
				{
					m_pDemoClient->SetAudioVolume(DEV_TYPE_SPEAKER, nVol);
					bResult = 1;
				}
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_SETPLAYVOLUME, bResult, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_GETPLAYVOLUME:
		{
			int nVol = -1;

			long ret = WME_E_FAIL;

			if(m_pDemoClient)
			{
				ret = m_pDemoClient->GetAudioVolume(DEV_TYPE_SPEAKER, nVol);
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_AUDIO_GETPLAYVOLUME, nVol, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_AUDIO_GETVOICELEVEL:
		{


		}
		break;

	case WBXI_EVENTTYPE_TA_ROLE_ASHOST:
		{
			m_bHost = TRUE;
			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_ROLE_ASHOST, 1, NULL);
		}
		break;

	case WBXI_EVENTTYPE_TA_ROLE_ASCLIENT:
		{
			m_bHost = FALSE;
			char *pIP = (char *)lParam;

			if(pIP)
			{
				strcpy_s(m_strHostIP, sizeof(m_strHostIP), pIP);
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_ROLE_ASCLIENT, 1, NULL);
		}
		break;

	case WBXI_EVENTTYPE_TA_ROLE_SETHOSTIP:
		{
			char *pIP = (char *)lParam;

			strcpy_s(m_strHostIP, sizeof(m_strHostIP), pIP);

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_ROLE_SETHOSTIP, 1, NULL);
	
		}
		break;

	case WBXI_EVENTTYPE_TA_ACTION_CONNECT:
		{
			long ret = WME_E_FAIL;

			if (m_pDemoClient)
			{
				m_pDemoClient->SetRenderView(DEMO_LOCAL_TRACK, m_senderWindowHandle, false);
				m_pDemoClient->SetRenderView(DEMO_REMOTE_TRACK, m_receiverWindowHandle, false);
				ret = m_pDemoClient->StartMediaTrack(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK);
				ret = m_pDemoClient->StartMediaTrack(DEMO_MEDIA_VIDEO, DEMO_REMOTE_TRACK);
				ret = m_pDemoClient->StartMediaTrack(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK);
				ret = m_pDemoClient->StartMediaTrack(DEMO_MEDIA_AUDIO, DEMO_REMOTE_TRACK);

				if (m_bHost)
				{
					ret = m_pDemoClient->InitHost(DEMO_MEDIA_VIDEO);
					ret = m_pDemoClient->InitHost(DEMO_MEDIA_AUDIO);	


				}
				else
				{
					ret = m_pDemoClient->ConnectRemote(DEMO_MEDIA_VIDEO, m_strHostIP);
					ret = m_pDemoClient->ConnectRemote(DEMO_MEDIA_AUDIO, m_strHostIP);

				}
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_ACTION_CONNECT, 1, NULL);

		}
		break;

	case WBXI_EVENTTYPE_TA_ACTION_DISCONNECT:
		{
			long ret = WME_E_FAIL;
			if (m_pDemoClient)
			{
				m_pDemoClient->StopRenderView(DEMO_LOCAL_TRACK);
				m_pDemoClient->StopRenderView(DEMO_REMOTE_TRACK);	
				ret = m_pDemoClient->StopMediaTrack(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK);

				ret = m_pDemoClient->StopMediaTrack(DEMO_MEDIA_VIDEO, DEMO_REMOTE_TRACK);

				ret = m_pDemoClient->StopMediaTrack(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK);

				ret = m_pDemoClient->StopMediaTrack(DEMO_MEDIA_AUDIO, DEMO_REMOTE_TRACK);

				m_pDemoClient->DisConnect(DEMO_MEDIA_VIDEO);
				m_pDemoClient->DisConnect(DEMO_MEDIA_AUDIO);

				DeleteMediaClient(DEMO_MEDIA_VIDEO);
				DeleteMediaClient(DEMO_MEDIA_AUDIO);
			}

			WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_ACTION_DISCONNECT, 1, NULL);
		}
		break;

	}

	return 0;
}


int TAManager::OnTABigDataSent(unsigned long nEvent, unsigned long wParam, unsigned long lParam)
{

	return 0;
}


int TAManager::TATrace(unsigned long trace_level, const char* szInfo, int len)
{

	if( !szInfo || szInfo[0] == 0 )
		return -1;
	

	
	return 0;

}



void TAManager::OnTimer(CCmTimerWrapperID *aId)
{
    TAPerformResult taPerformResult;
    TANetUsageResult taNetUsageResult;
    TACPUInfo cpuInfo;
    TAMemInfo memInfo;
    TANetUsageInfo netUsageInfo;
    TAResult taResult = {0};
    
    if(aId == NULL)
    {
        return;
    }
    
    if(aId == &m_PerformTimerId)
    {
        if(0 == WBXI_TAGetHostInfo(&cpuInfo, &memInfo))
        {
            taPerformResult.nUserCPU = cpuInfo.nUser;
            taPerformResult.nSysCPU = cpuInfo.nSys;
            taPerformResult.nIdleCPU = cpuInfo.nIdle;
            taPerformResult.nUsedMem = memInfo.nUsed;
            taPerformResult.nFreeMem = memInfo.nFree;
            taPerformResult.nWiredMem = memInfo.nWired;
            taResult.szTAStr = (char*)&taPerformResult;
            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_OTHER_PERFORM, 0,&taResult);
            
        }
        
    }
    
    
    if(aId == &m_NetTimerId)
    {
        if(0 == WBXI_TAGetNetUsageInfo(&netUsageInfo))
        {
            taNetUsageResult.nSent = netUsageInfo.nSent;
            taNetUsageResult.nReceived = netUsageInfo.nReceived;
            taResult.szTAStr = (char*)&taNetUsageResult;

            WBXI_TANetworkHandleCommand(hComponent, WBXI_TA_RESULT_OTHER_NETUSAGE, 0,&taResult);
            
        }
        
    }
    
	return;
}

void TAManager::CancelPerformTimer()
{
    
	m_PerformTimerId.Cancel();
}

void TAManager::SchedulePerformTimer()
{
    
	CancelPerformTimer();

    m_PerformTimerId.Schedule(this, CCmTimeValue(m_time, 0));

}

void TAManager::CancelNetTimer()
{
    
	m_NetTimerId.Cancel();
}

void TAManager::ScheduleNetTimer()
{
    
	CancelNetTimer();
    
    m_NetTimerId.Schedule(this, CCmTimeValue(m_time, 0));
    
}

void TAManager::networkDisconnectFromClient(DEMO_MEDIA_TYPE eType)
{
	OnTACommandHandle(WBXI_EVENTTYPE_TA_ACTION_DISCONNECT, 0, 0);
	//	EnableControls();
}


void TAManager::connectRemoteFailureFromClient(DEMO_MEDIA_TYPE eType)
{
	OnTACommandHandle(WBXI_EVENTTYPE_TA_ACTION_DISCONNECT, 0, 0);
}

void TAManager::connectRemoteOKFromClient(DEMO_MEDIA_TYPE mtype)
{
	if(m_pDemoClient)
	{
		m_pDemoClient->EnableMyMedia(DEMO_MEDIA_AUDIO, true);
		m_pDemoClient->StartMediaSending(DEMO_MEDIA_AUDIO);
		m_pDemoClient->EnableMyMedia(DEMO_MEDIA_VIDEO, true);
		m_pDemoClient->StartMediaSending(DEMO_MEDIA_VIDEO);
	}

}

void TAManager::OnDeviceChanged(DeviceProperty *pDP, WmeEventDataDeviceChanged &changeEvent)
{
	return;
}

void TAManager::OnVolumeChange(WmeEventDataVolumeChanged &changeEvent)
{
	return;
}