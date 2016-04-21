
#ifndef __MOCK_CMHTTPINTERFACE_H__
#define __MOCK_CMHTTPINTERFACE_H__

#include "CmHttpInterface.h"


class MockCCmChannelManager : public CCmChannelManager {
};

class MockICmChannel : public ICmChannel {
public:
    MOCK_METHOD1(AsyncOpen,
        CmResult(ICmChannelSink *aSink));
    MOCK_METHOD1(GetUrl,
        CmResult(CCmHttpUrl *&aUrl));
  MOCK_METHOD1(OpenWithSink,CmResult(ICmTransportSink *aSink));
  MOCK_METHOD0(GetSink,ICmTransportSink*());
  MOCK_METHOD2(SendData,CmResult(CCmMessageBlock &aData, CCmTransportParameter *aPara));
  MOCK_METHOD2(SetOption,CmResult(DWORD aCommand, LPVOID aArg));
  MOCK_METHOD2(GetOption,CmResult(DWORD aCommand, LPVOID aArg));
  MOCK_METHOD1(Disconnect,CmResult(CmResult aReason));
  MOCK_METHOD0(AddReference,DWORD());
  MOCK_METHOD0(ReleaseReference,DWORD());
};

class MockICmChannelSink : public ICmChannelSink {
public:
    MOCK_METHOD2(OnConnect,
        void(CmResult aReason, ICmChannel *aChannelId));
};

class MockICmChannelHttpClient : public ICmChannelHttpClient {
public:
    MOCK_METHOD2(SetOrAddRequestHeader,
        CmResult(const CCmString &aHeader, const CCmString &aValue));
    MOCK_METHOD1(SetRequestMethod,
        CmResult(const CCmString &aMethod));
    MOCK_METHOD1(SetPath,
        CmResult(const CCmString& aPath));
    MOCK_METHOD1(GetRequestMethod,
        CmResult(CCmString &aMethod));
    MOCK_METHOD1(GetResponseStatus,
        CmResult(LONG &aStatus));
    MOCK_METHOD2(GetResponseHeader,
        CmResult(const CCmString &aHeader, CCmString &aValue));
};

class MockICmChannelHttpServer : public ICmChannelHttpServer {
public:

    MockICmChannelHttpServer(){ m_pInvokeSink = NULL;  }

    //{{{ICmReferenceControl
    MOCK_METHOD0(AddReference, DWORD());
    MOCK_METHOD0(ReleaseReference, DWORD());
    //}}}ICmReferenceControl
    
    //{{{ICmTransport
    MOCK_METHOD1(OpenWithSink, CmResult(ICmTransportSink*));
    MOCK_METHOD0(GetSink, ICmTransportSink*());
    MOCK_METHOD2(SendData, CmResult(CCmMessageBlock&, CCmTransportParameter*));
    MOCK_METHOD2(SetOption, CmResult(DWORD, void*));
    MOCK_METHOD2(GetOption, CmResult(DWORD, void*));
    MOCK_METHOD1(Disconnect, CmResult(CmResult));
    //}}}ICmTransport
    
    //{{{ICmChannelHttpServer
    MOCK_METHOD2(SetOrAddResponseHeader, CmResult(const CCmString &aHeader, const CCmString &aValue));
    MOCK_METHOD2(SetResponseStatus, CmResult(DWORD aStatus, const CCmString &aText));
    MOCK_METHOD1(GetRequestMethod, CmResult(CCmString &aMethod));
    MOCK_METHOD2(GetRequestHeader, CmResult(const CCmString &aHeader, CCmString &aValue));
    MOCK_METHOD1(GetRequestPath, CmResult(CCmString &aPath));
    MOCK_METHOD0(GetTcpTransport, ICmTransport*());
    //}}}ICmChannelHttpServer

public:
    CmResult InvokeOpenWithSink(ICmTransportSink* pSink){ m_pInvokeSink = pSink; return CM_OK;};
    ICmTransportSink* InvokeGetSink(){ return  m_pInvokeSink;}

protected:
    ICmTransportSink*   m_pInvokeSink;
};

//class HelperChannelHttpServer
//{
//public:
//    HelperChannelHttpServer()
//    {
//        m_pSink = NULL;
//    }
//
//public:
//    CmResult OpenWithSink(ICmTransportSink* pSink){ m_pSink = pSink; return CM_OK;};
//    ICmTransportSink* GetSink(){ return  m_pSink;}
//
//protected:
//    ICmTransportSink*   m_pSink;
//};

//class MockICmChannelServerAcceptor : public ICmChannelServerAcceptor {
//public:
//    MOCK_METHOD3(StartListen,
//        CmResult(ICmChannelServerSink *aSink,         const CCmInetAddr &aAddrListen, int nTraceInterval));
//    MOCK_METHOD1(StopListen,
//        CmResult(CmResult aReason));
//};

class MockICmChannelServerSink : public ICmChannelServerSink {
public:
    MOCK_METHOD1(OnServerCreation,
        void(ICmChannelHttpServer *aServer));
};

#endif