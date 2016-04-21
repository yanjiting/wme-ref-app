#include "asenum.h"
#include <iostream>



CASEnum::CASEnum()
{
	m_pIWmeMediaEngine = NULL;
	m_pIWmeScreenSourceEnumeratorDesktop = NULL;
	m_pIWmeScreenSourceEnumeratorApp = NULL;
    m_pIWmeLocalScreenShareTrack = NULL;
    m_pIWmeLocalVideoTrack = NULL;
	m_pFunNotify = NULL;
	init();

}

CASEnum::~CASEnum()
{
    SAFE_RELEASE(m_pIWmeLocalScreenShareTrack);
    SAFE_RELEASE(m_pIWmeLocalVideoTrack);
	SAFE_RELEASE(m_pIWmeScreenSourceEnumeratorDesktop);
	SAFE_RELEASE(m_pIWmeScreenSourceEnumeratorApp);
	SAFE_RELEASE(m_pIWmeMediaEngine);

}

bool CASEnum::init()
{
	if (m_pIWmeMediaEngine)
		return false;

    WmeCreateMediaEngine(&m_pIWmeMediaEngine);
	if (m_pIWmeMediaEngine == NULL)
		return false;
    
	if (m_pIWmeMediaEngine){
		m_pIWmeMediaEngine->CreateScreenSourceEnumerator(&m_pIWmeScreenSourceEnumeratorDesktop,
			wme::WmeScreenSourceTypeDesktop);
		m_pIWmeMediaEngine->CreateScreenSourceEnumerator(&m_pIWmeScreenSourceEnumeratorApp,
			wme::WmeScreenSourceTypeApplication);

		return true;
	}

	return false;
}


std::map <std::string, IWmeScreenSource *> & CASEnum::getScreenSourceList()
{
	if (m_mapScreenSource.size()>0) return m_mapScreenSource;


	//need refresh it??
	if (m_pIWmeScreenSourceEnumeratorDesktop == NULL || m_pIWmeScreenSourceEnumeratorDesktop == NULL)
	{
		return m_mapScreenSource;
	}

	//list desktop
	if (m_pIWmeScreenSourceEnumeratorDesktop){
		int nCount = 0;
		m_pIWmeScreenSourceEnumeratorDesktop->GetNumber(nCount);
        m_pIWmeScreenSourceEnumeratorDesktop->AddObserver(WMEIID_IWmeScreenSourceEnumeratorEventObserver, this);
		for (int i = 0; i<nCount; i++){
			IWmeScreenSource * pWmeScreenSource = NULL;
			m_pIWmeScreenSourceEnumeratorDesktop->GetSource(i, &pWmeScreenSource);
			_addOneScreenSource(pWmeScreenSource);
		}
	}

	//list application
	if (m_pIWmeScreenSourceEnumeratorApp){
		int nCount = 0;
		m_pIWmeScreenSourceEnumeratorApp->GetNumber(nCount);
        m_pIWmeScreenSourceEnumeratorApp->AddObserver(WMEIID_IWmeScreenSourceEnumeratorEventObserver, this);
		for (int i = 0; i<nCount; i++){
			IWmeScreenSource * pWmeScreenSource = NULL;
			m_pIWmeScreenSourceEnumeratorApp->GetSource(i, &pWmeScreenSource);
			_addOneScreenSource(pWmeScreenSource);
		}
	}
	return m_mapScreenSource;
}


void CASEnum::_addOneScreenSource(IWmeScreenSource *pScreenSource)
{
	if (pScreenSource == NULL) return;

	int nBufLen = 64;
	char szUniqueName[64] = { 0 };
	pScreenSource->GetUniqueName(szUniqueName, nBufLen);
    
    
	std::string strUniqueName = szUniqueName;


	//
	if (m_mapScreenSource[strUniqueName] != NULL)
		m_mapScreenSource[strUniqueName]->Release();
	pScreenSource->AddRef();
	m_mapScreenSource[strUniqueName] = pScreenSource;


}
void CASEnum::_removeOneScreenSoruce(IWmeScreenSource *pScreenSource)
{
	if (pScreenSource == NULL) return;

	int nBufLen = 64;
	char szUniqueName[64] = { 0 };
	pScreenSource->GetUniqueName(szUniqueName, nBufLen);
	std::string strUniqueName = szUniqueName;

	//
	if (m_mapScreenSource[strUniqueName] != NULL)
        m_mapScreenSource[strUniqueName]->Release();
	m_mapScreenSource.erase(strUniqueName);

}
void CASEnum::_cleanupScreenSource()
{
	for (std::map<std::string, IWmeScreenSource *>::iterator it = m_mapScreenSource.begin(); it != m_mapScreenSource.end(); ++it){
		IWmeScreenSource *pWmeScreenSource = it->second;
		if (pWmeScreenSource)
			pWmeScreenSource->Release();
	}
	m_mapScreenSource.clear();
}

IWmeScreenSource* CASEnum::GetOneDefaultScreenSource(WmeScreenSourceType eWmeScreenSourceType)
{
	//refresh it
	getScreenSourceList();

	for (std::map<std::string, IWmeScreenSource *>::iterator it = m_mapScreenSource.begin(); it != m_mapScreenSource.end(); ++it){
		IWmeScreenSource *pWmeScreenSource = it->second;
		WmeScreenSourceType eWmeScreenSourceTypeTmp = WmeScreenSourceTypeUnknow;
		pWmeScreenSource->GetSourceType(eWmeScreenSourceTypeTmp);
		if (eWmeScreenSourceTypeTmp == eWmeScreenSourceType){
			return pWmeScreenSource;
		}
	}

	return NULL;
}


IWmeScreenSource* CASEnum::FindScreenSource(const char* lpszName)
{
	if (!lpszName)
		return NULL;

	for (std::map<std::string, IWmeScreenSource *>::iterator it = m_mapScreenSource.begin(); it != m_mapScreenSource.end(); ++it){
		IWmeScreenSource *pWmeScreenSource = it->second;
		char szTmp[256] = { 0 };
		int nLen = 256;
		pWmeScreenSource->GetUniqueName(szTmp, nLen);
		if (strcmp(lpszName, szTmp) == 0){
			return pWmeScreenSource;
		}
	}

	return NULL;
}


IWmeScreenSource* CASEnum::FindScreenSourceByFriendlyName(const char* lpszFriendlyName, bool bStrict)
{
   	if (!lpszFriendlyName)
		return NULL;
    
	for (std::map<std::string, IWmeScreenSource *>::iterator it = m_mapScreenSource.begin(); it != m_mapScreenSource.end(); ++it){
		IWmeScreenSource *pWmeScreenSource = it->second;
		char szTmp[256] = { 0 };
		int nLen = 256;
		pWmeScreenSource->GetFriendlyName(szTmp, nLen);
		std::string strOneSourceName = szTmp;
		std::string strSourceName = lpszFriendlyName;
		if (bStrict){
			if(strOneSourceName == strSourceName)
				return pWmeScreenSource;
		}
		else{
			if (strOneSourceName.find(strSourceName) != std::string::npos)
				return pWmeScreenSource;
		}
	}
    
	return NULL;
}

PFunctionScreenSourceChanged CASEnum::SetNotifyFunction(PFunctionScreenSourceChanged pNotifyFun)
{
	PFunctionScreenSourceChanged ret = m_pFunNotify;
	m_pFunNotify = pNotifyFun;
	return ret;
}

WMERESULT CASEnum::OnScreenSourceChanged(IWmeMediaEventNotifier *pNotifier, WmeScreenSourceEvent event,
	IWmeScreenSource *pScreenSource)
{
	if (pScreenSource == NULL) return WME_S_OK;
	switch (event) {
	case WmeScreenSourceAdded:
		_addOneScreenSource(pScreenSource);
		break;
	case WmeScreenSourceRemoved:
		_removeOneScreenSoruce(pScreenSource);
		break;
	default:
		break;
	}
	if (m_pFunNotify && (event == WmeScreenSourceAdded || event == WmeScreenSourceRemoved))
		m_pFunNotify();
	return WME_S_OK;
}

WMERESULT CASEnum::StartPreview(IWmeScreenSource *pIWmeScreenSource,void * previewView)
{
    if(previewView == NULL || pIWmeScreenSource == NULL) return WME_E_INVALIDARG;
    if(m_pIWmeMediaEngine==NULL) return  WME_S_FALSE;
    if(m_pIWmeLocalScreenShareTrack == NULL)
        m_pIWmeMediaEngine->CreateLocalScreenShareTrack(&m_pIWmeLocalScreenShareTrack);
    if(m_pIWmeLocalScreenShareTrack==NULL) return  WME_S_FALSE;
    
    m_pIWmeLocalScreenShareTrack->AddRenderWindow(previewView,NULL);
    m_pIWmeLocalScreenShareTrack->AddScreenSource(pIWmeScreenSource);
   return m_pIWmeLocalScreenShareTrack->Start();
}
WMERESULT CASEnum::StopPreview()
{
    if(m_pIWmeLocalScreenShareTrack){
        m_pIWmeLocalScreenShareTrack->Stop();
        SAFE_RELEASE(m_pIWmeLocalScreenShareTrack);
    }
    return WME_S_OK;
}

WMERESULT CASEnum::StartVideoPreview(void * previewView)
{
    if(previewView == NULL) return WME_E_INVALIDARG;
    if(m_pIWmeMediaEngine==NULL) return  WME_S_FALSE;
    if(m_pIWmeLocalVideoTrack == NULL)
        m_pIWmeMediaEngine->CreateLocalVideoTrack(&m_pIWmeLocalVideoTrack);
    if(m_pIWmeLocalVideoTrack==NULL) return  WME_S_FALSE;
    
    m_pIWmeLocalVideoTrack->AddRenderWindow(previewView,NULL);
    return m_pIWmeLocalVideoTrack->Start();
}
WMERESULT CASEnum::StopVideoPreview()
{
    if(m_pIWmeLocalVideoTrack){
        m_pIWmeLocalVideoTrack->Stop();
        SAFE_RELEASE(m_pIWmeLocalVideoTrack);
    }
    return WME_S_OK;


}