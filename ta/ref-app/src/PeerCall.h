//
//  PeerCall.h
//  MediaSessionTest
//
//  Created by Soya Li on 5/14/14.
//  Copyright (c) 2014 Soya Li. All rights reserved.
//

#ifndef __MediaSessionTest__PeerCall__
#define __MediaSessionTest__PeerCall__

#include "ClickCall.h"
#include "wsclient.h"

class PeerCall : public EventSink,
				 public IOrpheusClientSink,
                 public ISignalHook
{
public:
    static PeerCall* Instance()
    {
        static PeerCall call;
        return &call;
    }
    
    PeerCall()
        : m_endCaller(NULL), 
		  m_ws(NULL),
		  m_bActive(false),
		  m_linusCall(this)
    {
        m_endCaller = new Endpoint(this, true);
    }
    
    ~PeerCall()
    {
        stopPeer();
        if(m_endCaller)
        {
            delete m_endCaller;
            m_endCaller = NULL;
        }
        if(m_ws)
        {
            delete m_ws;
            m_ws = NULL;
        }
    }
    
	bool isActive() const { return m_bActive; }
    void connect(const char *server);
    void disconnect();
	void mute(WmeSessionType mediaType);
	void unmute(WmeSessionType mediaType);
	void mute(WmeSessionType mediaType, bool bMute, bool bLocal);
    unsigned int getVoiceLevel(bool bSpk);
	long startStopTrack(WmeSessionType mediaType, bool bRemote, bool bStart);

    virtual void OnConnected();
    virtual void OnStartCall(int nCount);
    virtual void OnSDPReceived(const string &type, const string &text);
    void pushRemoteWindow(void *winRemote);
    void startPeer(void *winLocal, void *winScreenViewer, IAppSink *pSink = NULL);
	std::string stopPeer();
	void createVenue();
	void createConfluence(const std::string& sdp);
    void requestFloor();
    void releaseFloor();
    void setCamera(const char *cameraName);
    void startSVS();
    
    void SetAutoSwitchDefaultMic(bool bFlag);
    void SetAutoSwitchDefaultSpeaker(bool bFlag);

    virtual void onEvent(Endpoint *origin, const string &type, const string &text = "", const string &extra = "");
    
    Endpoint *m_endCaller;
    
    // Calabash testing
    Calabash * getCalabash();
    COrpheusClient m_linusCall;
protected:
	virtual void OnVenue(const string &venueUrl, COrpheusClient* pOrigin);
	virtual void OnFlowParameters(const string &parameters, const string &url, COrpheusClient* pOrigin);

private:
	std::string m_sVenueUrl;
	CWsClient *m_ws;
	bool m_bActive;
};

#endif /* defined(__MediaSessionTest__PeerCall__) */
