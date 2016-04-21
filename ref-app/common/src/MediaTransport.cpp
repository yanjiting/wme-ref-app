#include "MediaTransport.h"
#include "IMediaClient.h"

MediaTransport::MediaTransport()
{
    m_pSession = NULL;
    m_pConnection = NULL;   

    m_pReceiver = NULL;

    m_bIcer = false;
    m_bConnected = false;
    m_bHost = false;

    m_sinkType = CONN_SINK_NONE;
    m_mediaDataType = TYPE_DATA_NONE;
    m_ctrlDataType = TYPE_DATA_NONE;
}

MediaTransport::~MediaTransport()
{
    DeleteTransport();
    SAFE_RELEASE(m_pSession);
}

void MediaTransport::SetSink(IMediaTransportSink *sink, ConnSinkType sinkType)
{
    m_pSink = sink;
    m_sinkType = sinkType;

    switch(sinkType) {
    case CONN_SINK_AUDIO:
        m_mediaDataType = TYPE_AUDIO_DATA;
        m_ctrlDataType  = TYPE_AUDIO_CONTROL;
        break;
    case CONN_SINK_VIDEO:
        m_mediaDataType = TYPE_VIDEO_DATA;
        m_ctrlDataType  = TYPE_VIDEO_CONTROL;
        break;
    case CONN_SINK_DESKSHARE: // whsu
        m_mediaDataType = TYPE_VIDEO_DATA;
        m_ctrlDataType  = TYPE_VIDEO_CONTROL;
        break;
    }
}

void MediaTransport::SetSession(IWmeMediaSession *session)
{
    SAFE_RELEASE(m_pSession);
    if (session) {
        m_pSession = session;
        m_pSession->AddRef();
    }
}

void MediaTransport::SetReceiver(MediaReceiver *receiver)
{
    m_pReceiver = receiver;
}

long MediaTransport::InitHost(bool enableCtrl, int dataPort, NetConnType dataConnType, int ctrlPort, NetConnType ctrlConnType)
{
    if (m_bConnected) {
        CM_INFO_TRACE_THIS("MediaTransport::InitHost, connected!");
        return WME_E_FAIL;
    }
    m_bIcer = false;

    long ret = WME_E_FAIL;
    CreateTransport();
    if (m_pConnection) {
        m_pConnection->SetSink(this);
        ret = m_pConnection->InitHost(enableCtrl, dataPort, dataConnType, ctrlPort, ctrlConnType);
        m_bHost = true;
    }
    
    return ret;
}

long MediaTransport::ConnectRemote(const char *ipaddr, int dataPort, NetConnType dataConnType, int ctrlPort, NetConnType ctrlConnType)
{
    if (m_bConnected){
        CM_INFO_TRACE_THIS("MediaTransport::ConnectRemote, connected!");
        return WME_E_FAIL;
    }
    m_bIcer = false;

    long ret = WME_E_FAIL;
    CreateTransport();
    if (m_pConnection) {
        m_pConnection->SetSink(this);
        ret = m_pConnection->ConnectTo(ipaddr, dataPort, dataConnType, ctrlPort, ctrlConnType);
        m_bHost = false;
    }

    return ret;
}

long MediaTransport::InitHost(const char *myName, const char *jingleIP, int jinglePort, const char *stunIP, int stunPort)
{
    if (m_bConnected) {
        CM_INFO_TRACE_THIS("MediaTransport::InitHost, connected!");
        return WME_E_FAIL;
    }
    m_bIcer = true;

    long ret = WME_E_FAIL;
    CreateTransport();
    if (m_pConnection) {
        m_pConnection->SetSink(this, m_sinkType);
        ret = m_pConnection->InitHost(myName, jingleIP, jinglePort, stunIP, stunPort);
        m_bHost = true;
    }
    
    return ret;
}

long MediaTransport::ConnectRemote(const char *myName, const char *hostName, const char *jingleIP, int jinglePort, const char *stunIP, int stunPort)
{
    if (m_bConnected){
        CM_INFO_TRACE_THIS("MediaTransport::ConnectRemote, connected!");
        return WME_E_FAIL;
    }
    m_bIcer = true;

    long ret = WME_E_FAIL;
    CreateTransport();
    if (m_pConnection) {
        m_pConnection->SetSink(this, m_sinkType);
        ret = m_pConnection->ConnectTo(myName, hostName, jingleIP, jinglePort, stunIP, stunPort);
        m_bHost = false;
    }

    return ret;
}

long MediaTransport::ConnectFile(const char *fileName, const char *sourceIP, unsigned short sourcePort, const char *destinationIP, unsigned short destinationPort)
{
    if (m_bConnected){
        CM_INFO_TRACE_THIS("MediaTransport::ConnectRemote(file), connected!");
        return WME_E_FAIL;
    }
    m_bIcer = false;
    
    long ret = WME_E_FAIL;
    if (NULL == m_pConnection) {
        ret = CreateMediaConnection(fileName, &m_pConnection);
    }
    if (m_pConnection) {
        m_pConnection->SetSink(this, m_sinkType);
        ret = m_pConnection->ConnectFile(sourceIP, sourcePort, destinationIP, destinationPort);
        m_bHost = false;
    }
    
    return ret;
}

/// for IMediaConnectionSink
int MediaTransport::OnConnected(bool bSuccess)
{
//    printf("OnConnected %s\n", bSuccess?"success":"failed");
    CM_INFO_TRACE_THIS("MediaTransport::OnConnected, bSuccess="<<bSuccess);

    m_bConnected = bSuccess;
    if (m_pSink) {
        m_pSink->OnConnected(bSuccess, m_sinkType);
    }
    return WME_S_OK;
}

/// for IMediaConnectionSink
int MediaTransport::OnDisconnected()
{
//    printf("OnDisconnected");
    CM_INFO_TRACE_THIS("MediaTransport::OnDisconnected");

    m_bConnected = false;
    if(m_pSink) {
        m_pSink->OnDisConnected(m_sinkType);
    }
    return WME_S_OK;
}

/// for IMediaConnectionSink
int MediaTransport::OnReceiveData(CCmMessageBlock &aData, NetDataType dataType)
{
    unsigned char pData[2048];
    DWORD nDataLen = 0;
    aData.Read(pData, 2048, &nDataLen);
    return OnReceiveData(pData, nDataLen, dataType);
}

/// for IMediaConnectionSink
int MediaTransport::OnReceiveData(unsigned char *pData, int nDataLen, NetDataType dataType)
{
    returnv_if_fail(pData != NULL, WME_E_INVALIDARG);
    returnv_if_fail(nDataLen > 0, WME_E_INVALIDARG);
    returnv_if_fail(m_pSession != NULL, WME_E_FAIL);

    if(dataType == TYPE_VIDEO_DATA || dataType == TYPE_AUDIO_DATA) {
        returnv_if_fail(m_pReceiver != NULL, WME_E_FAIL);
        //CM_INFO_TRACE_THIS("MediaTransport::OnReceiveData, receive RTP dataType="<<dataType);
        m_pSession->ReceiveRTPPacket(m_pReceiver->GetTrack(), pData, nDataLen);
    }else {
        m_pSession->ReceiveRTCPPacket(pData, nDataLen);
    }

    return WME_S_OK;
}

/// for IWmeMediaTransport
WMERESULT MediaTransport::SendRTPPacket(IWmeMediaPackage *pRTP)
{
    //CM_INFO_TRACE_THIS("MediaTransport::SendRTPPacket, m_mediaDataType="<<m_mediaDataType);

    returnv_if_fail(pRTP != NULL, WME_E_INVALIDARG);
    returnv_if_fail(m_pConnection != NULL, WME_E_FAIL);

    if (!m_bConnected) {
        CM_WARNING_TRACE_THIS("MediaTransport::SendRTPPacket, not connected");
        return WME_S_FALSE;
    }

    long ret = WME_E_FAIL;
    uint32_t nSize = 0;
    unsigned char *pData = NULL;
    pRTP->GetDataPointer(&pData);
    pRTP->GetDataSize(nSize);
    if (pData && nSize > 0) {
        m_pConnection->SendData(pData, nSize, m_mediaDataType);
        ret = WME_S_OK;
    }

    return ret;
}

/// for IWmeMediaTransport
WMERESULT MediaTransport::SendRTCPPacket(IWmeMediaPackage *pRTCP)
{
    //CM_INFO_TRACE_THIS("MediaTransport::SendRTCPPacket, m_ctrlDataType="<<m_ctrlDataType);

    returnv_if_fail(pRTCP != NULL, WME_E_FAIL);
    returnv_if_fail(m_pConnection != NULL, WME_E_FAIL);

    if (!m_bConnected) {
        CM_WARNING_TRACE_THIS("MediaTransport::SendRTCPPacket, not connected");
        return WME_S_FALSE;
    }

    long ret = WME_E_FAIL;
    uint32_t nSize = 0;
    unsigned char *pData = NULL;
    pRTCP->GetDataPointer(&pData);
    pRTCP->GetDataSize(nSize);
    if (pData && nSize > 0) {
        m_pConnection->SendData(pData, nSize, m_ctrlDataType);
        ret = WME_S_OK;
    }

    return ret;
}

/// for IWmeMediaTransport
WMERESULT MediaTransport::GetTransportType(WmeTransportType &eTransportType)
{
    return WmeTransportType_UDP;
}

long MediaTransport::CreateTransport()
{
    if (m_pConnection) {
        CM_INFO_TRACE_THIS("MediaTransport::CreateTransport, m_pConnection is not NULL!");
        return WME_S_OK;
    }

    if(m_bIcer) {
        m_pConnection = GetMediaConnectionInstance();
        if (m_pConnection == NULL) {
            CM_ERROR_TRACE_THIS("MediaTransport::CreateTransport, fail to GetMediaConnectionInstance");
            return WME_E_FAIL;
        }
    }else {
        long ret = CreateMediaConnection(&m_pConnection);
        if (ret != WME_S_OK || m_pConnection == NULL) {
            CM_ERROR_TRACE_THIS("MediaTransport::CreateTransport, fail to CreateMediaConnection");
            return WME_E_FAIL;
        }
    }

    return WME_S_OK;
}

long MediaTransport::DeleteTransport()
{
    CM_INFO_TRACE_THIS("MediaTransport::DeleteTransport");
    m_bConnected = false;
    if (!m_pConnection) {
        CM_INFO_TRACE_THIS("MediaTransport::DeleteTransport, m_pConnection is NULL");
        return WME_S_OK;
    }

    if (m_bIcer) {
        DeleteMediaConnectionInstance();
    }else {
        CM_INFO_TRACE_THIS("MediaTransport::DeleteTransport, DeleteMediaConnection");
        DeleteMediaConnection(m_pConnection);
    }
    m_pConnection = NULL;
    m_bIcer = false;
    m_bHost = false;

    return WME_S_OK;
}

long MediaTransport::StopTransport()
{
    if (m_pConnection) {
        m_pConnection->Stop();
    }
    return WME_S_OK;
}

