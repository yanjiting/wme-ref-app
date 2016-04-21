#ifndef ICERCLIENTINTERFACE_H
#define ICERCLIENTINTERFACE_H

#include "csttransport.h" 
class IIcerClient
{
public:
    
	virtual ~IIcerClient(){}
    
	virtual void SetName(const char* szMyName) = 0;
	virtual void SetPeerName(const char* szPeerName) = 0;
	virtual void SetStunServer(const char* szSver, UINT nPort) = 0;
    virtual void ConnectToServer(const char* szServer, int nPort) = 0;
	virtual void SetSink(IIcerSink *pIcerSink) = 0;
};

extern "C"
{
 int  CreateIcerClient(IIcerClient **ppClient, bool bIsCaller,int nComponent);
 int  DeleteIcerClient(IIcerClient *pClient);
};
#endif