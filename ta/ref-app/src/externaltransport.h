//
//  externaltransport.h
//  MediaSessionTest
//
//  Created by LiSiyuan on 15/1/6.
//  Copyright (c) 2015 cisco. All rights reserved.
//

#ifndef MediaSessionTest_externaltransport_h
#define MediaSessionTest_externaltransport_h

#include "CmConnectionInterface.h"
#include "WmeSession.h"

using namespace wme;

class Endpoint;
class ExternalTransport : public ICmTransportSink, public ICmAcceptorConnectorSink, public IWmeMediaTransport
{
public:
    ExternalTransport(Endpoint* endpoint, int mid)
        : m_endpoint(endpoint)
        , m_mid(mid)
    {}
    
    virtual ~ExternalTransport()
    {
        Disconnect();
    }
    
    void StartListen(int mid, const char* localIp, unsigned int uLocalPort);
    void Connect(const char* remoteIp, unsigned int uRemotePort);
    void Disconnect();
    
    virtual void OnConnectIndication(
                                     CmResult aReason,
                                     ICmTransport *aTrpt,
                                     ICmAcceptorConnectorId *aRequestId);

    virtual void OnReceive(
                           CCmMessageBlock &aData,
                           ICmTransport *aTrptId,
                           CCmTransportParameter *aPara = NULL);
    
    virtual void OnSend(
                        ICmTransport *aTrptId,
                        CCmTransportParameter *aPara = NULL)
    {
        
    }
    
    virtual void OnDisconnect(
                              CmResult aReason,
                              ICmTransport *aTrptId);
    
    virtual WMERESULT GetTransportType(WmeTransportType &eTransportType);
    

    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTPPackage);

    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCPPackage);
    
private:
    Endpoint* m_endpoint;
    CCmComAutoPtr<ICmTransport> m_transport;
    CCmComAutoPtr<ICmAcceptor>  m_acceptor;
    int m_mid;
};

#endif
