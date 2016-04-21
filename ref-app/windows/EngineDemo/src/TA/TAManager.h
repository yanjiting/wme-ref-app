#ifndef TA_MANAGER_H
#define TA_MANAGER_H

#include "WMETADef.h"
#include "WMETAUtil.h"
#include "WMETANetwork.h"
#include "CmConnectionInterface.h"
#include "DemoClient.h"
#include <atlstr.h>

//#import "WMEDataProcess.h"




class TAManager: public ITANetworkSink,
				public ITATraceSink,
                public CCmTimerWrapperIDSink,
				public backUISink

{
public:

	TAManager();
	~TAManager();
	void SetDemoClient(DemoClient*);
	void CreateTANetwork();
	virtual int OnTACommandHandle(unsigned long nEvent, unsigned long wParam, unsigned long lParam);
	virtual int OnTABigDataSent(unsigned long nEvent, unsigned long wParam, unsigned long lParam);
	virtual int TATrace(unsigned long trace_level, const char* szInfo, int len);
    void OnTimer(CCmTimerWrapperID *aId);
	void SchedulePerformTimer();
	void CancelPerformTimer();
    void ScheduleNetTimer();
	void CancelNetTimer();
//    CGImageRef RetriveFrameImage();
//    UIImage* GetCaptureScreenShot();
//   UIImage* Snapshot(UIView* eaglview);

public:
	virtual void networkDisconnectFromClient(DEMO_MEDIA_TYPE eType)	;
	virtual void connectRemoteFailureFromClient(DEMO_MEDIA_TYPE eType);
	virtual void connectRemoteOKFromClient(DEMO_MEDIA_TYPE mtype);
	virtual void OnDeviceChanged(DeviceProperty *pDP, WmeEventDataDeviceChanged &changeEvent);
	virtual void OnVolumeChange(WmeEventDataVolumeChanged &changeEvent);
	virtual void OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight){};

private:
	void ClearCameraList();
	void ClearMicList();
	void ClearSpeakerList();
	long CreateMediaClient(DEMO_MEDIA_TYPE mType);
	long DeleteMediaClient(DEMO_MEDIA_TYPE mType);
	void CreateRenderWindow();
	void CloseRenderWindow();  
	HWND createWindow(char *pName, DEMO_WINDOW_TYPE eWindowType);
private:
	void* hComponent;
	TAData m_theTAData;
    CCmTimerWrapperID m_PerformTimerId;
    CCmTimerWrapperID m_NetTimerId;
    unsigned long m_time;
    
    //Point to WME data process module
//    WMEDataProcess *pWMEDataProcess;
	DemoClient* m_pDemoClient;
	DemoClient::DevicePropertyList m_CameraList;
	DemoClient::DevicePropertyList m_MicList;
	DemoClient::DevicePropertyList m_SpeakerList;

	DemoClient::VideoMediaCapabilityList m_VideoCapabilityList;
	DemoClient::AudioMediaCapabilityList m_AudioCapabilityList;
	std::map<int, std::string> m_mapVideoCodecName;
	std::map<int, std::string> m_mapAudioCodecName;

	BOOL m_bVideoClientCreated;
	BOOL m_bAudioClientCreated;

	BOOL m_bHost;
	char m_strHostIP[32];

	HWND m_senderWindowHandle;
	HWND m_receiverWindowHandle;
	HWND m_previewerWindowHandle;
};

#endif
