//
//  TAServer.cpp
//  MediaSessionTest
//
//  Created by juntang on 6/19/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#include "TAServer.h"
#include "CmInetAddr.h"

#include "json/json.h"

#import "TAResponder.h"

TAServerChannel::TAServerChannel(ICmChannelHttpServer *aChannel) : m_pChannel(aChannel)
{
    CM_ASSERTE(m_pChannel);
    
    CmResult rv = m_pChannel->OpenWithSink(this);
    CM_ASSERTE(CM_SUCCEEDED(rv));
    
    m_responder = [[TAResponder alloc] init];
}

TAServerChannel::~TAServerChannel()
{
    if (m_pChannel) {
        m_pChannel->Disconnect(CM_OK);
        m_pChannel = NULL;
    }
    
    [m_responder release];
}

void TAServerChannel::OnDisconnect(CmResult aReason, ICmTransport *aTrptId)
{
    CM_ASSERTE(aTrptId == m_pChannel.Get());
    
    delete this;
}

void TAServerChannel::OnReceive(CCmMessageBlock &aData, ICmTransport *aTrptId, CCmTransportParameter *aPara)
{
    
    
    DWORD len, alen;
    len = aData.GetChainedLength();
    NSLog(@"len of data: %d", len);
    char *data = new char[len+1];
    aData.Read(data, len, &alen, FALSE);
    NSLog(@"actual read len: %d", alen);
    
    data[len] = 0;
    
    NSString *body = [NSString stringWithUTF8String:data];
    NSLog(@"body of request: %@", body);
    
    CCmString method, path;
    m_pChannel->GetRequestMethod(method);
    m_pChannel->GetRequestPath(path);
    
    NSString *nmethod = [NSString stringWithUTF8String:method.c_str()];
    NSString *npath = [NSString stringWithUTF8String:path.c_str()];
    
    NSDictionary *respData = [m_responder httpResponseForMethod:nmethod URI:npath withData:body];
    
    std::string sSend;
    Json::Value root(Json::objectValue);
    
    for (NSString *key in respData) {
        NSString *value = [respData objectForKey:key];
        root[[key UTF8String]] = Json::Value([value UTF8String]);
    }
    
    Json::FastWriter w;
    sSend = w.write(root);
    
    CCmMessageBlock mb(sSend.length() + 1);
    mb.Write(sSend.c_str(), sSend.length());
    
    m_pChannel->SendData(mb);

}

void TAServerChannel::OnSend(ICmTransport *aTrptId, CCmTransportParameter *aPara)
{
    
}


/////////////

TAServer::TAServer()
{
    CCmInetAddr addr("127.0.0.1", 43334);
    Init(addr);
}

TAServer::~TAServer()
{
    if (m_pServerAcceptor) {
        m_pServerAcceptor->StopListen(CM_OK);
        m_pServerAcceptor = NULL;
    }
}

CmResult TAServer::Init(const CCmInetAddr &aAddr)
{
    //prepare all responds
    [TAResponder initialize];
    
    int rv = CCmChannelManager::Instance()->CreateHttpAcceptor(m_pServerAcceptor.ParaOut(), 0);
    CM_ASSERTE_RETURN(CM_SUCCEEDED(rv), rv);
    
    rv = m_pServerAcceptor->StartListen(this, aAddr);
    
    return rv;
}

void TAServer::OnServerCreation(ICmChannelHttpServer *aServer)
{
    CM_ASSERTE(aServer);
    
    new TAServerChannel(aServer);
    CM_ASSERTE(pChannel);
}
