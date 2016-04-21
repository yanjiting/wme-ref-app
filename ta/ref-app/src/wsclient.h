#ifndef _WEBEX_SQUARE_WME_TEST_WS_CLIENT_H__
#define _WEBEX_SQUARE_WME_TEST_WS_CLIENT_H__

#pragma once

#include "CmBase.h"
#include <string>
#include "CmHttpInterface.h"
#include "CmHttpUrl.h"
#include "CmHttpUtilClasses.h"
#include "CmInetAddr.h"
#include "CmErrorNetwork.h"

using namespace std;

class ISignalHook
{
public:
    virtual ~ISignalHook() {};

    virtual void OnConnected() = 0;
    virtual void OnStartCall(int nCount) = 0;
    virtual void OnSDPReceived(const string &type, const string &text) = 0;
};

class CWsClient : public ICmChannelSink, public CCmTimerWrapperIDSink
{
public:
    CWsClient(ISignalHook *pSink);
    virtual ~CWsClient() { Stop(); }

public:
    void Open(const char *url);
    void Stop();
    virtual void OnConnect(CmResult aReason, ICmChannel *aChannelId, long id);
	virtual void OnReceive(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara);
	virtual void OnSend(ICmTransport *aTrptId, CCmTransportParameter *aPara = NULL);
	virtual void OnDisconnect(CmResult aReason, ICmTransport *aTrptId);
    void SendTo(string type, string text, int index = -1); //-1 is broad cast
	virtual void OnTimer(CCmTimerWrapperID* aId);
    int GetCount() { return m_nMyId; }

private:
    CCmComAutoPtr<ICmChannel> m_wsChannel;
    CCmHttpUrl m_sUrl;
    ISignalHook *m_pSink;
    CCmTimerWrapperID m_timerKeepAlive;
    int m_nMyId;
    bool m_bStartedCall;
};

#endif //!define _WEBEX_SQUARE_WME_TEST_WS_CLIENT_H__
