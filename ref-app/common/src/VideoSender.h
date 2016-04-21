#ifndef _VIDEO_SENDER_H_
#define _VIDEO_SENDER_H_

#include "IMediaClient.h"
#include "IMediaSettings.h"
#include "FileCaptureEngineImp.h"
#include <string>

class VideoSender : 
    public MediaSender, 
    public VideoSettings,
    public IWmeMediaMessageTransmitterObserver,
	public IWmeMediaCaptureObserver,
	public IWmeLocalVideoTrackObserver,
    public IWmeMediaFileCapturerObserver
{
public:
    VideoSender();
    virtual ~VideoSender();

    /// for MediaClient
    virtual long SetCapability(WmeMediaBaseCapability *pMC);
    virtual long GetCapability(WmeMediaBaseCapability *pMC);
    
    virtual long CreateTrack();
    virtual long DeleteTrack();
    virtual long SetTransport(IWmeMediaTransport *sink);
    virtual long StartTrack();
    virtual long StopTrack();

	virtual long SetTrackCodec(IWmeMediaCodec *pCodec);

    long SetDefaultSettings();

    /// for VideoSettings
    virtual long SetDevice(IWmeMediaDevice* pCamera);
    virtual long GetDevice(IWmeMediaDevice** ppCamera);
	virtual long SetCaptureCapability(const WmeVideoRawFormat & format);
	virtual long SetVideoFile(const char* pFileName, WmeVideoRawFormat *pFormat);
	virtual void SetVideoSource(int nSource); //nSource: 0 camera; 1 file;

    // for IWmeMediaMessageTransmitterObserver
    virtual WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnPictureLossIndication(IWmeMediaEventNotifier *pNotifier, uint32_t uSourceID, uint32_t uStreamId);

	//for IWmeMediaCaptureObserver
	virtual WMERESULT OnStartedCapture(IWmeMediaEventNotifier *pNotifier, WMERESULT eStartedResult); //empty
	virtual WMERESULT OnUpdateCaptureResolution(IWmeMediaEventNotifier *pNotifier, uint32_t uWidth, uint32_t uHeight);
	virtual WMERESULT OnUpdateCaptureFPS(IWmeMediaEventNotifier *pNotifier, uint32_t uFPS);
    virtual WMERESULT OnCameraRuntimeDie(IWmeMediaEventNotifier *pNotifier) {return WME_S_OK;}

	//for IWmeLocalVideoTrackObserver
	virtual WMERESULT OnRequiredFrameRateUpdate(float fRequiredFrameRate);	//empty
	virtual WMERESULT OnEncodedResolutionUpdate(uint32_t uWidth, uint32_t uHeight);
	virtual WMERESULT OnEncodedFrameRateUpdate(uint32_t uFPS);
	virtual WMERESULT OnStreamBandwidthUpdate(WmeAllStreamsBandwidth* pAllStreams) {return WME_S_OK;}
    virtual WMERESULT OnSelfviewSizeUpdate(WmeVideoSizeInfo *pSizeInfo) {return WME_S_OK;}

    //for IWmeMediaFileCapturerObserver
    virtual WMERESULT OnEndOfStream(IWmeMediaFileCapturer* pCapturer);

private:
    IWmeMediaDevice *m_pCamera;
	std::string		m_strVideoFileName;
    WmeVideoRawFormat m_rawFormat;
	IWmeMediaFileCapturer* m_pMediaFileCap;
    //CFileCaptureEngineImp *m_pFileCapEngine;
	BOOL				m_bUseVideoFile;
};

#endif // _VIDEO_SENDER_H_
