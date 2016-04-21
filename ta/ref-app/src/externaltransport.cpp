//
//  externaltransport.cpp
//  MediaSessionTest
//
//  Created by LiSiyuan on 15/1/6.
//  Copyright (c) 2015 cisco. All rights reserved.
//

#include "externaltransport.h"
#include "ClickCall.h"
#include "CmInetAddr.h"

void ExternalTransport::StartListen(int mid, const char* localIp, unsigned int uLocalPort)
{
    CM_INFO_TRACE("ExternalTransport::StartListen, mid = " << mid << ", localIp = " << localIp << ", uLocalPort = " << uLocalPort);
    m_mid = mid;
    CCmInetAddr localAddr(localIp, uLocalPort);
    ICmAcceptor *aAcceptor = NULL;
    CmResult rv = CCmConnectionManager::Instance()->CreateConnectionServer(CCmConnectionManager::CTYPE_UDP, aAcceptor);
    CM_ASSERTE_RETURN_VOID(CM_SUCCEEDED(rv) && aAcceptor);
    m_acceptor = aAcceptor;
    rv = m_acceptor->StartListen(this, localAddr);
    CM_ASSERTE_RETURN_VOID(CM_SUCCEEDED(rv));
}

void ExternalTransport::Connect(const char* remoteIp, unsigned int uRemotePort)
{
    if (m_acceptor == NULL)
        return;
    CM_INFO_TRACE("ExternalTransport::Connect, remoteIp = " << remoteIp << ", uRemotePort = " << uRemotePort);
    CCmInetAddr remoteAddr(remoteIp, uRemotePort);
    CmResult rv = m_acceptor->Connect(remoteAddr);
    CM_ASSERTE_RETURN_VOID(CM_SUCCEEDED(rv));
}

void ExternalTransport::Disconnect()
{
    if (m_transport){
        m_transport->Disconnect(CM_OK);
        m_transport = NULL;
    }
    if(m_acceptor) {
        m_acceptor->StopListen(CM_OK);
        m_acceptor = NULL;
    }
    
    if (m_endpoint && m_endpoint->m_pMediaConn) {
        IWmeMediaConnection4T *pConn = static_cast<IWmeMediaConnection4T*>(m_endpoint->m_pMediaConn);
        CM_ASSERTE_RETURN_VOID(pConn);
        pConn->SetTransport(m_mid, NULL);
    }
}

void ExternalTransport::OnConnectIndication(
                                 CmResult aReason,
                                 ICmTransport *aTrpt,
                                 ICmAcceptorConnectorId *aRequestId)
{
    CM_INFO_TRACE("ExternalTransport::OnConnectIndication, aReason = " << aReason);
    if(CM_SUCCEEDED(aReason) && aTrpt)
    {
        if ( m_transport ) {
            m_transport->Disconnect(CM_OK);
        }
        m_transport = aTrpt;
        m_transport->OpenWithSink(this);
        
        CM_ASSERTE_RETURN_VOID(m_endpoint && m_endpoint->m_pMediaConn);
        IWmeMediaConnection4T *pConn = static_cast<IWmeMediaConnection4T*>(m_endpoint->m_pMediaConn);
        CM_ASSERTE_RETURN_VOID(pConn);
        pConn->SetTransport(m_mid, this);
    }
}

void ExternalTransport::OnReceive(
                       CCmMessageBlock &aData,
                       ICmTransport *aTrptId,
                                  CCmTransportParameter *aPara)
{
    CM_ASSERTE_RETURN_VOID(m_endpoint && m_endpoint->m_pMediaConn);
    IWmeMediaConnection4T *pConn = static_cast<IWmeMediaConnection4T*>(m_endpoint->m_pMediaConn);
    CM_ASSERTE_RETURN_VOID(pConn);
    
    DWORD nRead = aData.GetChainedLength();
    CCmString strBuffer = aData.FlattenChained();
    unsigned char * buffer = (unsigned char*)(strBuffer.c_str());
    CM_ASSERTE_RETURN_VOID(nRead > 0);
    
    if(*buffer > 127 && *buffer < 192)//RTP/RTCP
    {
        CM_ASSERTE_RETURN_VOID(nRead >= 12);
        
        unsigned char rtpPayload = 0x7F & buffer[1];
        //DEBUG_TRACE_THIS_MS_WITH_TYPE_WmeSessionType("CMediaConnectionInfo::OnReceive, rtpPayload=" << (int)rtpPayload);
        //http://www.packetizer.com/in/q9.html
        if((rtpPayload >= 96 && rtpPayload <= 127) ||
           (rtpPayload == 0 || rtpPayload == 8 || rtpPayload == 9 || rtpPayload == 4 || rtpPayload == 15 || rtpPayload == 18 || rtpPayload == 31 || rtpPayload == 34))
        {
            pConn->ReceiveRTP(m_mid, buffer, nRead);
        }
        else//RTCP
        {
            pConn->ReceiveRTCP(m_mid, buffer, nRead);
        }
    }
    else if(*buffer < 2) //STUN
    {
    }
    else if(*buffer > 19 && *buffer < 64) //DTLS
    {
    }
}


void ExternalTransport::OnDisconnect(
                          CmResult aReason,
                          ICmTransport *aTrptId)
{
    CM_INFO_TRACE("ExternalTransport::OnDisconnect, aReason = " << aReason);
    Disconnect();
}

WMERESULT ExternalTransport::GetTransportType(WmeTransportType &eTransportType)
{
    eTransportType = WmeTransportType_Unknown;
    if (m_transport != NULL){
        CCmConnectionManager::CType cTransType = CCmConnectionManager::CTYPE_NONE;
        m_transport->GetOption(CM_OPT_TRANSPORT_TRAN_TYPE, &cTransType);
        if(cTransType & CCmConnectionManager::CTYPE_TCP) {
            eTransportType = WmeTransportType_TCP;
        }
        else if(cTransType & CCmConnectionManager::CTYPE_UDP) {
            eTransportType = WmeTransportType_UDP;
        }
    }
    return WME_S_OK;
}

WMERESULT ExternalTransport::SendRTPPacket(IWmeMediaPackage *pRTPPackage)
{
    uint32_t nSize = 0;
    unsigned char *pData = NULL;
    pRTPPackage->GetDataPointer(&pData);
    pRTPPackage->GetDataSize(nSize);
    
    CM_ASSERTE_RETURN(pData && nSize > 0, CM_ERROR_INVALID_ARG);
    
    if (m_transport) {
        CCmMessageBlock mbSend(nSize, (LPCSTR)pData, CCmMessageBlock::DONT_DELETE, nSize);
        return m_transport->SendData(mbSend);
    }
    
    return WME_E_FAIL;
}

WMERESULT ExternalTransport::SendRTCPPacket(IWmeMediaPackage *pRTCPPackage)
{
    uint32_t nSize = 0;
    unsigned char *pData = NULL;
    pRTCPPackage->GetDataPointer(&pData);
    pRTCPPackage->GetDataSize(nSize);
    
    CM_ASSERTE_RETURN(pData && nSize > 0, CM_ERROR_INVALID_ARG);
    
    if (m_transport) {
        CCmMessageBlock mbSend(nSize, (LPCSTR)pData, CCmMessageBlock::DONT_DELETE, nSize);
        return m_transport->SendData(mbSend);
    }
    
    return WME_E_FAIL;
}
