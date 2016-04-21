#ifndef _DS_SENDER_H_
#define _DS_SENDER_H_

#include "IMediaClient.h"
#include "IMediaSettings.h"
#include "WmeScreenSource.h"

class DeskShareSender : 
    public MediaSender, 
    public VideoSettings,
    public IWmeMediaMessageTransmitterObserver
{
public:
    DeskShareSender();
    virtual ~DeskShareSender();

    /// for MediaClient
    virtual long SetCapability(WmeMediaBaseCapability *pMC);
    virtual long CreateTrack();
    virtual long DeleteTrack();
    virtual long SetTransport(IWmeMediaTransport *sink);

    long SetDefaultSettings();

    /// for VideoSettings
    virtual long SetCaptureCapability(const WmeVideoRawFormat & format);
    virtual long SetScreenSource(IWmeScreenSource* pIWmeScreenSource);
    // for IWmeMediaMessageTransmitterObserver
    virtual WMERESULT OnWillAddByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnDidRemoveByNotifier(IWmeMediaEventNotifier* pNotifer, REFWMEIID ulObserverID);
    virtual WMERESULT OnPictureLossIndication(IWmeMediaEventNotifier *pNotifier, uint32_t uSourceID, uint32_t uStreamId);

private:
    IWmeScreenSource * m_pScreenSource;
    //IWmeMediaDevice *m_pCamera;
    WmeVideoRawFormat m_rawFormat;
};

#endif // _DS_SENDER_H_
