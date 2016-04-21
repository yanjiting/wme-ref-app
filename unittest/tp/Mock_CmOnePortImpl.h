
#ifndef  __MOCK_CMONEPORT_IMPL_H__
#define  __MOCK_CMONEPORT_IMPL_H__

#include <gmock/gmock.h>
#include "CmOnePortImpl.h"
#include "CmInetAddr.h"

class MockCCmOnePortConn : public CCmOnePortConn
{
public:
    MockCCmOnePortConn(ICmChannelHttpServer *aServer, CCmOnePortImpl *aOnePort): CCmOnePortConn(aServer, aOnePort)
    {
        m_pProtoHandle = NULL;
    }
    
public:
    void SetProtoHandle(ICmBaseHTTPExProtoHandle* pProtoHandle)
    {
        m_pProtoHandle = pProtoHandle;
    }
    
protected:
    virtual ICmBaseHTTPExProtoHandle* GetProtoHandle(UINT32 uType)
    {
        return m_pProtoHandle;
    }

protected:
    ICmBaseHTTPExProtoHandle*   m_pProtoHandle;
};


class MockCCmOnePortImpl : public CCmOnePortImpl 
{
 public:
    MockCCmOnePortImpl()
    {       
    }

    ~MockCCmOnePortImpl()
    {       
    }
    
  MOCK_METHOD2(StartListen, CmResult( ICmOnePortSink *aSink,const CCmInetAddr &addr));
  
  MOCK_METHOD1(StopListen,  CmResult(CmResult aReason));
  MOCK_METHOD0(AddReference,  DWORD());
  MOCK_METHOD0(ReleaseReference, DWORD());
  MOCK_METHOD1(OnServerCreation,
      void(ICmChannelHttpServer *aServer));
  MOCK_METHOD2(OnConnect,
      void(CmResult aReason, ICmBaseHTTPExProtoHandle *pProtoHandle));
  MOCK_METHOD2(OnDisconnect,
      void(CmResult aReason, ICmBaseHTTPExProtoHandle *pProtoHandle));
};

#endif
