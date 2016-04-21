#ifndef _I_MEDIA_CLIENT_H_
#define _I_MEDIA_CLIENT_H_

#include "DemoParameters.h"

/// sink for MediaClient
class IMediaClientSink
{
public:
    /// for video decode in remote track
    virtual void OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight) {}

    /// for media file I/O
    virtual void OnEndOfStream(DEMO_MEDIA_TYPE eType) = 0;

protected:
    virtual ~IMediaClientSink() {}
};


/// for base video/audio interfaces
class MediaClient
{
public:
    MediaClient();
    virtual ~MediaClient();
    virtual void SetSink(IMediaClientSink *pSink);

    virtual void SetMediaEngine(IWmeMediaEngine *pEngine);

    /// NOTE:
    /// (a) audio sender(local) and receiver(remote) share the same session
    /// (b) video sender(local) and receiver(remote) share another session
    /// (c) different sessions between audio and video
    virtual void SetMediaSession(IWmeMediaSession *pSession);

    virtual long SetTrackCodec(IWmeMediaCodec *pCodec);

	virtual long GetTrackCodec(IWmeMediaCodec **ppCodec);

    virtual long SetCapability(WmeMediaBaseCapability *pMC);
    
    virtual long GetCapability(WmeMediaBaseCapability *pMC);

    /// Valid when both session and track must be OK.
    virtual long SetRtpSession(uint8_t payload, uint32_t clock);

    virtual long CreateTrack();
    virtual long DeleteTrack();
    virtual long StartTrack();
    virtual long StopTrack();
    virtual IWmeMediaTrack * GetTrack();

protected:
    IMediaClientSink            *m_pSink;
    IWmeMediaEngine             *m_pEngine;
    IWmeMediaSession            *m_pSession;
    IWmeMediaTrack              *m_pTrack;
    bool                        m_bStarted;
};

/// for audio/video local track
class MediaSender : public MediaClient
{
public:
    virtual long SetTransport(IWmeMediaTransport *sink);
    virtual long SetOptionForSendingData(bool bsend);
};

/// for audio/video remote track
class MediaReceiver : public MediaClient
{
public:
    MediaReceiver();
    virtual ~MediaReceiver();

    virtual void SetMediaSyncBox(IWmeMediaSyncBox *pSyncBox);

protected:
    IWmeMediaSyncBox        *m_pSyncBox;
};

#endif // _I_MEDIA_CLIENT_H_
