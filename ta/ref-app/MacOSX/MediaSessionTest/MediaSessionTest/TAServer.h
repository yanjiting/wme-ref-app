//
//  TAServer.h
//  MediaSessionTest
//
//  Created by juntang on 6/19/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#include "CmHttpInterface.h"
#include "TAResponder.h"


#ifndef MediaSessionTest_TAServer_h
#define MediaSessionTest_TAServer_h

class TAServerChannel : public ICmTransportSink
{
public:
    TAServerChannel(ICmChannelHttpServer *aChannel);
    virtual ~TAServerChannel();
    
    virtual void OnDisconnect(CmResult aReason, ICmTransport *aTrptId);
    void OnReceive(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara = NULL);
    void OnSend(ICmTransport *aTrptId, CCmTransportParameter *aPara = NULL);
    
private:
    CCmComAutoPtr<ICmChannelHttpServer> m_pChannel;
    TAResponder *m_responder;
};

class TAServer : public ICmChannelServerSink
{
public:
    TAServer();
    virtual ~TAServer();
    
    CmResult Init(const CCmInetAddr &aAddr);
    
    void OnServerCreation(ICmChannelHttpServer *aServer);

private:
    CCmComAutoPtr<ICmChannelServerAcceptor> m_pServerAcceptor;
};

#endif
