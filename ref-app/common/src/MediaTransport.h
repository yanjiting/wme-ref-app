#ifndef _MEDIA_TRANSPORT_H_
#define _MEDIA_TRANSPORT_H_

#include "MediaConnection.h"
#include "DemoParameters.h"

class IMediaTransportSink 
{
public:
    virtual ~IMediaTransportSink() {}
    virtual void OnConnected(bool bSuccess, ConnSinkType stype) = 0;
    virtual void OnDisConnected(ConnSinkType stype) = 0;
};

class MediaReceiver;

class MediaTransport :
    public IMediaConnectionSink    // for network receive
    , public IWmeMediaTransport      // for network send
{
public:
    MediaTransport();
    virtual ~MediaTransport();

    void SetSink(IMediaTransportSink *sink, ConnSinkType sinkType);

    /// for receiver(remote track)
    void SetSession(IWmeMediaSession *session);
    void SetReceiver(MediaReceiver *receiver);

    long InitHost(bool enableCtrl, int dataPort, NetConnType dataConnType, int ctrlPort, NetConnType ctrlConnType);
    long ConnectRemote(const char *ipaddr, int dataPort, NetConnType dataConnType, int ctrlPort, NetConnType ctrlConnType);

    /// for ICE
    long InitHost(const char *myName, const char *jingleIP, int jinglePort, const char *stunIP, int stunPort);
    long ConnectRemote(const char *myName, const char *hostName, const char *jingleIP, int jinglePort, const char *stunIP, int stunPort);
    
    /// for pcap dump file
    long ConnectFile(const char *fileName, const char *sourceIP, unsigned short sourcePort, const char *destinationIP, unsigned short destinationPort);

    long CreateTransport();
    long DeleteTransport();
    long StopTransport();

    /// for IWmeMediaTransport, data sending
    virtual WMERESULT SendRTPPacket(IWmeMediaPackage *pRTP);
    virtual WMERESULT SendRTCPPacket(IWmeMediaPackage *pRTCP);
    virtual WMERESULT GetTransportType(WmeTransportType &eTransportType);

    /// for IMediaConnectionSink, data receiving
    virtual int OnConnected(bool bSuccess);
    virtual int OnDisconnected();
    virtual int OnReceiveData(CCmMessageBlock &aData, NetDataType dataType);
    virtual int OnReceiveData(unsigned char *pData, int nDataLen, NetDataType dataType);

private:
    IMediaTransportSink        *m_pSink;

    IWmeMediaSession        *m_pSession; 
    MediaReceiver           *m_pReceiver;

    IMediaConnection        *m_pConnection;
    bool                    m_bIcer;
    bool                    m_bConnected;
    bool                    m_bHost;

    ConnSinkType            m_sinkType;
    NetDataType             m_mediaDataType;
    NetDataType             m_ctrlDataType;
};


#endif // _MEDIA_TRANSPORT_H_
