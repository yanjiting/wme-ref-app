#ifndef _VIDEO_PREVIEW_H_
#define _VIDEO_PREVIEW_H_

#include "IMediaClient.h"
#include "IMediaSettings.h"

class VideoPreview : 
    public MediaClient,
    public VideoSettings,
	public IWmeMediaCaptureObserver
{
public:
    VideoPreview();
    virtual ~VideoPreview();

    /// for MediaClient
    virtual long CreateTrack();
    virtual long DeleteTrack();

    /// for VideoSettings
    virtual long SetDevice(IWmeMediaDevice* pCamera);
	virtual long GetDevice(IWmeMediaDevice** ppCamera);
    virtual long SetCaptureCapability(const WmeVideoRawFormat & format);

	//for IWmeMediaCaptureObserver
	virtual WMERESULT OnStartedCapture(IWmeMediaEventNotifier *pNotifier, WMERESULT eStartedResult); //empty
	virtual WMERESULT OnUpdateCaptureResolution(IWmeMediaEventNotifier *pNotifier, uint32_t uWidth, uint32_t uHeight);
	virtual WMERESULT OnUpdateCaptureFPS(IWmeMediaEventNotifier *pNotifier, uint32_t uFPS);
    virtual WMERESULT OnCameraRuntimeDie(IWmeMediaEventNotifier *pNotifier) {return WME_S_OK;}

private:
    IWmeMediaDevice *m_pCamera;
    WmeVideoRawFormat m_rawFormat;
};

#endif // _VIDEO_PREVIEW_H_
