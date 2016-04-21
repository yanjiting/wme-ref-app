#ifndef _DS_RECEIVER_H_
#define _DS_RECEIVER_H_

#include "IMediaClient.h"
#include "IMediaSettings.h"

class DeskShareReceiver : 
    public MediaReceiver, 
    public VideoSettings,
    public IWmeRemoteVideoTrackObserver
{
public:
    DeskShareReceiver();
    virtual ~DeskShareReceiver();

    /// for MediaClient
    virtual long CreateTrack();
    virtual long DeleteTrack();

    /// for IWmeRemoteVideoTrackObserver
    virtual WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnKeyFrameLost(uint32_t uLabel, uint32_t uDID);
    virtual WMERESULT OnRenderBlocked(uint32_t uLabel, bool bBlocked);
	virtual WMERESULT OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight);
};

#endif // _VIDEO_RECEIVER_H_
