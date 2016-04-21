//
//  FileMediaConnection.h
//  MediaEngineTestApp
//
//  Created by Sand Pei on 14-6-26.
//  Copyright (c) 2014年 video. All rights reserved.
//

#ifndef FIME_MEDIA_CONNECTION_H
#define FIME_MEDIA_CONNECTION_H

#include <string>

#include "MediaConnection.h"
#include "WmeError.h"

#include "CmThread.h"
#include "CmTimerWrapperID.h"

class CFileMediaConnection :    public IMediaConnection,
                                public CCmTimerWrapperIDSink
{
public:
    CFileMediaConnection(const char *fileName);
	virtual ~CFileMediaConnection();
        
    /// for normal connect and server
	int InitHost(bool enableCtrl, int dataPort, NetConnType dataType, int ctrlPort, NetConnType ctrlType) { return WME_E_NOTIMPL; }
	int ConnectTo(const char *hostIP, int dataPort, NetConnType dataType, int ctrlPort, NetConnType ctrlType) { return WME_E_NOTIMPL; }
	void SetSink(IMediaConnectionSink* pSink) { return; /*WME_E_NOTIMPL*/ }
    
	int Stop();
	int DisConnect();
    
	unsigned short GetBindPort() { return 0; /*WME_E_NOTIMPL*/ }
	bool IsConnected();
    
	int	SendData(CCmMessageBlock &aData, NetDataType datatype) { return WME_E_NOTIMPL; }
	int	SendData(unsigned char *pData, int nDataLen, NetDataType datatype) { return WME_E_NOTIMPL; }
    
    /// for ICE connect and server
	int InitHost(const char *myName, const char *jingleIP, int jinglePort, const char *stunIP, int stunPort) { return WME_E_NOTIMPL; }
	int ConnectTo(const char *myName, const char *hostName, const char *jingleIP, int jinglePort, const char *stunIP, int stunPort) { return WME_E_NOTIMPL; }
	void SetSink(IMediaConnectionSink* pSink, ConnSinkType sinkType);
    
    /// for pcap dump file
    int ConnectFile(const char *sourceIP, unsigned short sourcePort, const char *destinationIP, unsigned short destinationPort);
    
    /// for timer
    void OnTimer(CCmTimerWrapperID* aId);
    
protected:
    int Connect();
    
    int SchduleTimer();
    int CancelTimer();
    
    int ReadPacket();
    int CheckPacket();
    
private:
    IMediaConnectionSink* m_pSink;
    ConnSinkType m_eConnSinkType;
    
    std::string m_strFileName;
    FILE *m_pFile;
    
    bool m_bConnected;
    unsigned int m_uSourceIP;
    unsigned int m_uDestinationIP;
    unsigned short m_uSourcePort;
	unsigned short m_uDestinationPort;
    
    int m_iPacketCount;
    unsigned char *m_pBuffer;
    int m_iBufferLen;
    unsigned char *m_pPacket;
    int m_iPacketLen;
    unsigned int m_uPacketTime;
    unsigned int m_uLastPacketTime;
    unsigned int m_uFirstPacketTime;
    bool    m_bDelivered;
    unsigned int m_uLastDeliverTime;
    unsigned int m_uFirstDeliverTime;
    
    ACmThread *m_pTimerThread;
    CCmTimerWrapperID m_TimerId;
    
private:
    class CTimerEvent : public ICmEvent
    {
    public:
        CTimerEvent(CCmTimerWrapperID *pTimer, bool bSechdule, CCmTimerWrapperIDSink *pSink, double fInterval = 0.0, int iCount = 0);
        ~CTimerEvent();
        
        // ICmEvent interface
        CmResult OnEventFire();
        
    protected:
        CCmTimerWrapperID				*m_pTimer;
        CCmTimerWrapperIDSink			*m_pSink;
        bool							m_bSechdule;
        double							m_fInterval; // unit: second
        int								m_iCount;
    };
};

#endif //FIME_MEDIA_CONNECTION_H
