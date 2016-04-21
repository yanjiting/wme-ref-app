
#ifndef  __MOCK_CMONEPORT_INTERFACE_H__
#define  __MOCK_CMONEPORT_INTERFACE_H__

#include <gmock/gmock.h>
#include "CmOnePortInterface.h"


class MockICmOnePortTransport : public ICmOnePortTransport {
public:
    //mock ICmOnePortTransport
    MOCK_METHOD1(GetConnType, CmResult(UINT32  &type) );
    MOCK_METHOD2(GetRequestHeader, CmResult(const CCmString &aHeader, CCmString &aValue) );
    MOCK_METHOD2(SetOrAddResponseHeader, CmResult(const CCmString &aHeader, const CCmString &aValue) );
    MOCK_METHOD1(GetRequestPath, CmResult(CCmString& aPath));

    //mock ICmTransport
    MOCK_METHOD1(OpenWithSink, CmResult(ICmTransportSink *aSink) );
    MOCK_METHOD0(GetSink, ICmTransportSink*() );
    MOCK_METHOD2(SendData, CmResult(CCmMessageBlock &aData, CCmTransportParameter *aPara ) );
    MOCK_METHOD2(SetOption,CmResult(DWORD aCommand, LPVOID aArg));
    MOCK_METHOD2(GetOption,CmResult(DWORD aCommand, LPVOID aArg));
    MOCK_METHOD1(Disconnect, CmResult(CmResult aReason) );


    //mock ICmReferenceControl
    MOCK_METHOD0(AddReference, DWORD() );
    MOCK_METHOD0(ReleaseReference, DWORD() );

};

class MockICmOnePortSink : public ICmOnePortSink {
public:
    MockICmOnePortSink()
    {
        m_pInvokeTransport = NULL;
    }

    MOCK_METHOD1(OnServerCreation, void(ICmOnePortTransport* aTransport));
    
    void InvokeOnServerCreation(ICmOnePortTransport* aTransport)
    {
        m_pInvokeTransport = aTransport;
    }

    ICmOnePortTransport* GetOnePortTransport()
    {
        return m_pInvokeTransport;
    }

protected:
    ICmOnePortTransport*        m_pInvokeTransport;
};

class MockICmOnePort : public ICmOnePort {
 public:
  MOCK_METHOD2(StartListen,
      CmResult(ICmOnePortSink *aSink, const char *addr));
  MOCK_METHOD1(StopListen,
      CmResult(CmResult aReason));

    //mock ICmReferenceControl
    MOCK_METHOD0(AddReference, DWORD() );
    MOCK_METHOD0(ReleaseReference, DWORD() );
  
};

#endif
