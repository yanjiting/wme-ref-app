//
//  TAServer.cpp
//  MediaSessionTest
//
//  Created by juntang on 6/19/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#include "TAHTTPServer.h"
#include "CmInetAddr.h"
#include "json/json.h"

TAServerChannel::TAServerChannel(ICmChannelHttpServer *aChannel, IBackdoorSink* pSink) 
: m_pChannel(aChannel), m_pSink(pSink)
{
    CM_ASSERTE(m_pChannel);
    
    CmResult rv = m_pChannel->OpenWithSink(this);
    CM_ASSERTE(CM_SUCCEEDED(rv));
}

TAServerChannel::~TAServerChannel()
{
    if (m_pChannel) {
        m_pChannel->Disconnect(CM_OK);
        m_pChannel = NULL;
    }
}

void TAServerChannel::OnDisconnect(CmResult aReason, ICmTransport *aTrptId)
{
    CM_ASSERTE(aTrptId == m_pChannel.Get());
    
    delete this;
}

void TAServerChannel::OnReceive(CCmMessageBlock &aData, 
								ICmTransport *aTrptId, 
								CCmTransportParameter *aPara)
{
    DWORD len, alen;
    len = aData.GetChainedLength();
    char *data = new char[len+1];
    aData.Read(data, len, &alen, FALSE);
	data[len] = 0;
	CM_INFO_TRACE_THIS("TAServerChannel::OnReceive, len=" << len << ", body=" << data);
    
    CCmString method, path;
    m_pChannel->GetRequestMethod(method);
    m_pChannel->GetRequestPath(path);

	std::string resp;
	Json::Reader r;
	Json::Value request_root;
	r.parse(data, request_root, true);
	std::string selector = request_root["selector"].asString();
	std::string arg = request_root["arg"].asString();
	bool bOutcome = m_pSink->OnRequest(selector, arg, resp);
    
    std::string sSend;
    Json::Value resp_root(Json::objectValue);
	resp_root["outcome"] = Json::Value(bOutcome ? std::string("SUCCESS") : std::string("FAILURE"));
	resp_root["result"] = Json::Value(resp);

    Json::FastWriter w;
	sSend = w.write(resp_root);
    
    CCmMessageBlock mb(sSend.length() + 1);
    mb.Write(sSend.c_str(), sSend.length());
    
    m_pChannel->SendData(mb);
}

void TAServerChannel::OnSend(ICmTransport *aTrptId, CCmTransportParameter *aPara)
{
    
}


/////////////
TAServer::TAServer(IBackdoorSink* pSink)
:m_pSink(pSink)
{
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
    int rv = CCmChannelManager::Instance()->CreateHttpAcceptor(m_pServerAcceptor.ParaOut(), 0);
    CM_ASSERTE_RETURN(CM_SUCCEEDED(rv), rv);
    
    rv = m_pServerAcceptor->StartListen(this, aAddr);
    
    return rv;
}

void TAServer::OnServerCreation(ICmChannelHttpServer *aServer)
{
    CM_ASSERTE(aServer);
    
	TAServerChannel* pChannel = new TAServerChannel(aServer, m_pSink);
    CM_ASSERTE(pChannel);
}
