#ifndef __MediaSessionTest__ASENUM__
#define __MediaSessionTest__ASENUM__

#include <map>
#include "MediaConnection.h"
#include "WmeScreenSource.h"

using namespace wme;

typedef void(*PFunctionScreenSourceChanged)();

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) do { if(x) x->Release(); x = NULL; } while(0);
#endif

class CASEnum : public IWmeScreenSourceEnumeratorEventObserver
{
public:
	static CASEnum* Instance()
    {
		static CASEnum asenum;
		return &asenum;
    }

	

	CASEnum();
	virtual ~CASEnum();
	std::map <std::string, IWmeScreenSource *> & getScreenSourceList();
	IWmeScreenSource* GetOneDefaultScreenSource(WmeScreenSourceType eWmeScreenSourceType = WmeScreenSourceTypeDesktop);
	IWmeScreenSource* FindScreenSource(const char* lpszName);
    IWmeScreenSource* FindScreenSourceByFriendlyName(const char* lpszFriendlyName,bool bStrict=true);
    PFunctionScreenSourceChanged SetNotifyFunction(PFunctionScreenSourceChanged pNotifyFun);
    WMERESULT StartPreview(IWmeScreenSource *pIWmeScreenSource,void * previewView);
    WMERESULT StopPreview();
    WMERESULT StartVideoPreview(void * previewView);
    WMERESULT StopVideoPreview();
private:
	bool init();
	void _addOneScreenSource(IWmeScreenSource *pScreenSource);
	void _removeOneScreenSoruce(IWmeScreenSource *pScreenSource);
	void _cleanupScreenSource();
	virtual WMERESULT OnScreenSourceChanged(IWmeMediaEventNotifier *pNotifier, WmeScreenSourceEvent event,
		IWmeScreenSource *pScreenSource);

	virtual WMERESULT OnWillAddByNotifier(wme::IWmeMediaEventNotifier* pNotifer, const wme::WMEIID & ulObserverID) { return WME_S_OK; }
	virtual WMERESULT OnDidRemoveByNotifier(wme::IWmeMediaEventNotifier* pNotifer, const wme::WMEIID & ulObserverID) { return WME_S_OK; }


private:
	//IWmeMediaConnection *m_pMediaConn;
    IWmeMediaEngine *m_pIWmeMediaEngine;
	IWmeScreenSourceEnumerator * m_pIWmeScreenSourceEnumeratorDesktop;
	IWmeScreenSourceEnumerator * m_pIWmeScreenSourceEnumeratorApp;

	std::map <std::string, IWmeScreenSource *> m_mapScreenSource;
    IWmeLocalScreenShareTrack * m_pIWmeLocalScreenShareTrack;
    IWmeLocalVideoTrack * m_pIWmeLocalVideoTrack;

	PFunctionScreenSourceChanged m_pFunNotify;
};

#endif /* defined(__MediaSessionTest__ASENUM__) */
