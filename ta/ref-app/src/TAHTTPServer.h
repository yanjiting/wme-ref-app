//
//  TAServer.h
//  MediaSessionTest
//
//  Created by juntang on 6/19/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#ifndef MediaSessionTest_TAHTTPServer_h
#define MediaSessionTest_TAHTTPServer_h

#include "CmHttpInterface.h"
#include "BackdoorAgentSink.h"

class TAServerChannel : public ICmTransportSink
{
public:
	TAServerChannel(ICmChannelHttpServer *aChannel, IBackdoorSink* pSink);
    virtual ~TAServerChannel();
    
    virtual void OnDisconnect(CmResult aReason, ICmTransport *aTrptId);
    void OnReceive(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara = NULL);
    void OnSend(ICmTransport *aTrptId, CCmTransportParameter *aPara = NULL);

private:
    CCmComAutoPtr<ICmChannelHttpServer> m_pChannel;
	IBackdoorSink* m_pSink;
};

class TAServer : public ICmChannelServerSink
{
public:
	TAServer(IBackdoorSink* pSink);
    virtual ~TAServer();
    
    CmResult Init(const CCmInetAddr &aAddr);
    
    void OnServerCreation(ICmChannelHttpServer *aServer);

private:
    CCmComAutoPtr<ICmChannelServerAcceptor> m_pServerAcceptor;
	IBackdoorSink* m_pSink;
};

#endif
