#ifndef MEDIA_CONNECTION_H
#define MEDIA_CONNECTION_H


/*
#ifdef WIN32
#if defined (_LIB) || (CM_OS_BUILD_LIB)
    #define CM_OS_EXPORT
#else
    #if defined (_USRDLL) || (CM_OS_BUILD_DLL)
            #define CM_OS_EXPORT __declspec(dllexport)
    #else
            #define CM_OS_EXPORT __declspec(dllimport)
    #endif // _USRDLL || CM_OS_BUILD_DLL
#endif // _LIB || CM_OS_BUILD_LIB
#else
#define CM_OS_EXPORT
#endif //WIN32

*/

#include "CmMessageBlock.h"

typedef enum
{
    TYPE_DATA_NONE      = -1,
	TYPE_VIDEO_DATA		= 0,	
	TYPE_AUDIO_DATA		= 1,	
	TYPE_VIDEO_CONTROL    = 2,    
	TYPE_AUDIO_CONTROL    = 3,
	TYPE_CONN_CONTROL	 = 4
}NetDataType;

typedef enum
{
	CONN_UDP = 0,
	CONN_TCP = 1
}NetConnType;

typedef enum
{
    CONN_SINK_NONE  = -1,
	CONN_SINK_VIDEO = 0,
	CONN_SINK_AUDIO = 1,
	CONN_SINK_DESKSHARE = 2 //whsu
}ConnSinkType;

///
/// both for sender and receiver client
class IMediaConnectionSink
{
public:
	virtual ~IMediaConnectionSink(){}

	virtual int OnConnected(bool bSuccess) = 0; //true for sucess; false for failed
	virtual int OnDisconnected() = 0;
	virtual int OnReceiveData(CCmMessageBlock &aData, NetDataType datatype) = 0;
	virtual int OnReceiveData(unsigned char *pData, int nDataLen, NetDataType datatype) = 0;
};

class IMediaConnection
{
public:
	virtual ~IMediaConnection(){}

    /// for normal connect and server
	virtual int InitHost(bool enableCtrl, int dataPort, NetConnType dataType, int ctrlPort, NetConnType ctrlType) = 0;
	virtual int ConnectTo(const char *hostIP, int dataPort, NetConnType dataType, int ctrlPort, NetConnType ctrlType) = 0;
	virtual void SetSink(IMediaConnectionSink* pSink) = 0;

	virtual int Stop() = 0;
	virtual int DisConnect() = 0;

	virtual unsigned short GetBindPort() = 0;
	virtual bool IsConnected() = 0;

	virtual int	SendData(CCmMessageBlock &aData, NetDataType datatype) = 0;
	virtual int	SendData(unsigned char *pData, int nDataLen, NetDataType datatype) = 0;

    /// for ICE connect and server
	virtual int InitHost(const char *myName, const char *jingleIP, int jinglePort, const char *stunIP, int stunPort) = 0;
	virtual int ConnectTo(const char *myName, const char *hostName, const char *jingleIP, int jinglePort, const char *stunIP, int stunPort) = 0;
	virtual void SetSink(IMediaConnectionSink* pSink, ConnSinkType sinkType) = 0;
    
    /// for pcap dump file
    virtual int ConnectFile(const char *sourceIP, unsigned short sourcePort, const char *destinationIP, unsigned short destinationPort) { return 0; }
};


///
/// external interfaces for tp network
int  CreateMediaConnection(IMediaConnection **ppConn);
int  CreateMediaConnection(const char *fileName, IMediaConnection **ppConn);
int  DeleteMediaConnection(IMediaConnection *pConn);
IMediaConnection* GetMediaConnectionInstance();
void DeleteMediaConnectionInstance();

#endif // MEDIA_CONNECTION_H
