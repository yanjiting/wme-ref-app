#ifndef _I_MEDIA_SETTINGS_H_
#define _I_MEDIA_SETTINGS_H_

#include "DemoParameters.h"

/// for video features, should be implemented in VideoSender/VideoReciever/VideoPreview
class VideoSettings : public IWmeVideoRenderObserver
{
public:
    VideoSettings(IWmeMediaTrack * &pTrack);
    virtual ~VideoSettings();

    virtual long SetDevice(IWmeMediaDevice* pCamera);
	virtual long GetDevice(IWmeMediaDevice** ppCamera);
	virtual long SetCaptureCapability(const WmeVideoRawFormat & format);
    virtual long SetScreenSource(IWmeScreenSource* pIWmeScreenSource);
    
    virtual long SetVideoQuality(WmeVideoQualityType quality);
	virtual long SetStaticPerformance(WmePerformanceProfileType perf);

    virtual long SetRenderer(void *render, bool windowless=false);
    virtual long RemoveRenderer();
    virtual long SetRenderAspectRatioSameWithSource(bool keepsame);
	virtual long SetRenderMode(WmeTrackRenderScalingModeType eMode);

    virtual long OnRenderWindowPositionChanged(void *render);
    virtual long OnRenderingDisplayChanged();

	virtual long GetStatistics(WmeVideoStatistics &stat);

    /// for IWmeVideoRenderObserver
    virtual WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnRequestedDC(IWmeMediaEventNotifier *pNotifier, uint32_t uRequestedID);

protected:
    IWmeMediaTrack * &m_pTrackRef;
    bool m_bWindowLess;
    void * m_pRender;
	WmeTrackRenderScalingModeType m_eRenderMode;

#ifdef WIN32
    HDC m_pDC;
#endif
};


class AudioSettings
{
public:
	AudioSettings(IWmeMediaTrack * &pTrack);
	virtual ~AudioSettings();

	long EnableEC(bool bEnable);
	long EnableVAD(bool bEnable);
	long EnableNS(bool bEnable);
	long EnableAGC(bool bEnable);
	long EnableAutoDropData(bool bEnable);


	virtual long GetStatistics(WmeAudioStatistics &stat);

protected:
	IWmeMediaTrack * &m_pTrackRef;
};
#endif // _I_MEDIA_SETTINGS_H_
