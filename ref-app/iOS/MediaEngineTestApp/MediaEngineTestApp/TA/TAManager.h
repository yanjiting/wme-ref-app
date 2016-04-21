#ifndef TA_MANAGER_H
#define TA_MANAGER_H

#include "WMETADef.h"
#include "WMETAUtil.h"
#include "WMETANetwork.h"
#include "CmConnectionInterface.h"

#import <Foundation/Foundation.h>
#import <QuartzCore/QuartzCore.h>
#import <UIKit/UIKit.h>
#import "WMEDisplayViewController.h"
#import "WMEDataProcess.h"




class TAManager: public ITANetworkSink,
				public ITATraceSink,
                public CCmTimerWrapperIDSink	

{
public:

	TAManager();
	~TAManager();
	void CreateTANetwork();
	virtual int OnTACommandHandle(unsigned long nEvent, unsigned long wParam, unsigned long lParam);
	virtual int OnTABigDataSent(unsigned long nEvent, unsigned long wParam, unsigned long lParam);
	virtual int TATrace(unsigned long trace_level, const char* szInfo, int len);
    void OnTimer(CCmTimerWrapperID *aId);
	void SchedulePerformTimer();
	void CancelPerformTimer();
    void ScheduleNetTimer();
	void CancelNetTimer();
    CGImageRef RetriveFrameImage();
    UIImage* GetCaptureScreenShot();
    UIImage* Snapshot(UIView* eaglview);
    
private:
	void* hComponent;
	TAData m_theTAData;
    CCmTimerWrapperID m_PerformTimerId;
    CCmTimerWrapperID m_NetTimerId;
    unsigned long m_time;
    
    //Point to WME data process module
    WMEDataProcess *pWMEDataProcess;

};

#endif
