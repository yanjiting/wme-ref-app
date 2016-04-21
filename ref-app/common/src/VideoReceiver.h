#ifndef _VIDEO_RECEIVER_H_
#define _VIDEO_RECEIVER_H_

#include "IMediaClient.h"
#include "IMediaSettings.h"

class VideoReceiver : 
    public MediaReceiver, 
    public VideoSettings,
    public IWmeRemoteVideoTrackObserver
{
public:
    VideoReceiver();
    virtual ~VideoReceiver();

    /// for MediaClient
    virtual long CreateTrack();
    virtual long DeleteTrack();

    /// for IWmeRemoteVideoTrackObserver
    virtual WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnKeyFrameLost(uint32_t uLabel, uint32_t uDID);
    virtual WMERESULT OnRenderBlocked(uint32_t uLabel, bool bBlocked);
    virtual WMERESULT OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight);
    
    long SetVideoFile(const char* pFileName);
    void SetVideoTarget(bool file_target) {m_file_target = file_target;}

	// only for VideoReceiver
	long EnableDecoderMosaic(bool bEnable);
#ifdef CUCUMBER_ENABLE
	bool AddFileRenderSink(IWmeMediaFileRenderSink* pSink);
#endif
    
private:
    bool m_file_target;
    IWmeMediaFileRenderer*  m_pFileRender;
};

#endif // _VIDEO_RECEIVER_H_
