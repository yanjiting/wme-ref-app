
#ifndef __MOCK_CMCONNECTION_INTERFACE_H__ 
#define __MOCK_CMCONNECTION_INTERFACE_H__

#include <gmock/gmock.h>
#include "CmConnectionInterface.h"
#include "CmInetAddr.h"
class MockCCmConnectionManager : public CCmConnectionManager {
};

class MockCCmTransportParameter : public CCmTransportParameter {
 public:
};

class MockICmAcceptorConnectorSink : public ICmAcceptorConnectorSink {
 public:
  MOCK_METHOD3(OnConnectIndication,
      void(CmResult aReason,         ICmTransport *aTrpt,         ICmAcceptorConnectorId *aRequestId));
};

class MockICmTransportSink : public ICmTransportSink {
 public:
  MOCK_METHOD3(OnReceive,
      void(CCmMessageBlock &aData,         ICmTransport *aTrptId,         CCmTransportParameter *aPara));
  MOCK_METHOD2(OnSend,
      void(ICmTransport *aTrptId,         CCmTransportParameter *aPara));
  MOCK_METHOD2(OnDisconnect,
      void(CmResult aReason,         ICmTransport *aTrptId));

public:
    
    void InvokeIncreaseRecvCount()
    {
        m_nRecvCounts++;
    }

    void SetRecvCount(int nCount) 
    {
        m_nRecvCounts = 0;
    }

    int GetRecvCount()
    {
        return m_nRecvCounts;
    }

protected:
    int         m_nRecvCounts;
};

class MockICmTransport : public ICmTransport {
 public:
  MOCK_METHOD1(OpenWithSink,
      CmResult(ICmTransportSink *aSink));
  MOCK_METHOD0(GetSink,
      ICmTransportSink*());
  MOCK_METHOD2(SendData,
      CmResult(CCmMessageBlock &aData, CCmTransportParameter *aPara));
  MOCK_METHOD2(SetOption,
      CmResult(DWORD aCommand, LPVOID aArg));
  MOCK_METHOD2(GetOption,
      CmResult(DWORD aCommand, LPVOID aArg));
  MOCK_METHOD1(Disconnect,
      CmResult(CmResult aReason));
  
  MOCK_METHOD0(AddReference,
      DWORD());
  
  MOCK_METHOD0(ReleaseReference,
      DWORD());
  
};

class MockICmAcceptorConnectorId : public ICmAcceptorConnectorId {
 public:
  MOCK_METHOD0(IsConnector,
      BOOL());
};

/*
class MockICmChannel: public ICmChannel{
public:
  MOCK_METHOD1(AsyncOpen,CmResult(ICmChannelSink *aSink));
  MOCK_METHOD1(GetUrl,CmResult(CCmHttpUrl *&aUrl));
  MOCK_METHOD1(OpenWithSink,CmResult(ICmTransportSink *aSink));
  MOCK_METHOD0(GetSink,ICmTransportSink*());
  MOCK_METHOD2(SendData,CmResult(CCmMessageBlock &aData, CCmTransportParameter *aPara));
  MOCK_METHOD2(SetOption,CmResult(DWORD aCommand, LPVOID aArg));
  MOCK_METHOD2(GetOption,CmResult(DWORD aCommand, LPVOID aArg));
  MOCK_METHOD1(Disconnect,CmResult(CmResult aReason));
  MOCK_METHOD0(AddReference,DWORD());
  MOCK_METHOD0(ReleaseReference,DWORD());
};
*/

/*
class MockCIceEngineCheck: public CIceEngineCheck
{
public:
	MockCIceEngineCheck(uint64_t priority, CIceEngineCheckMgr* sink, IceEngineCandidate* lcand, IceEngineCandidate* rcand):
	CIceEngineCheck(priority,sink,lcand,rcand)
	{};
public:
	MOCK_METHOD3(DetectConflicts,bool(uint64_t tieBreaker, eIceRole role, StunTransactionID* id));
	MOCK_METHOD0(SetNominated,void());
	MOCK_METHOD1(SetCompleted,void(p2p_ice_check_state));
	MOCK_METHOD2(SetWaitingStatus,void(bool,eIceRole));
};
*/


// class MockICmConnector : public ICmConnector {
 // public:
  // MOCK_METHOD4(AsycConnect,
      // void(ICmAcceptorConnectorSink *aSink,         const CCmInetAddr &aAddrPeer,         CCmTimeValue *aTimeout = NULL,         CCmInetAddr *aAddrLocal));
  // MOCK_METHOD1(CancelConnect,
      // void(CmResult aReason));
// };

// class MockICmDetectionConnector : public ICmDetectionConnector {
 // public:
  // MOCK_METHOD3(AddConnection,
      // CmResult(CCmConnectionManager::CType Type,         const CCmInetAddr &aAddrPeer,         CCmTimeValue *aTimeDelay));
  // MOCK_METHOD2(StartDetectionConnect,
      // void(ICmAcceptorConnectorSink *aSink,         CCmTimeValue *aTimeout));
// };

// class MockICmAcceptor : public ICmAcceptor {
 // public:
  // MOCK_METHOD3(StartListen,
      // CmResult(ICmAcceptorConnectorSink *aSink,         const CCmInetAddr &aAddrListen, int nTraceInterval));
  // MOCK_METHOD1(StopListen,
      // CmResult(CmResult aReason));
  // MOCK_METHOD2(SetOption,
      // CmResult(DWORD aCommand, LPVOID aArg));
  // MOCK_METHOD2(GetOption,
      // CmResult(DWORD aCommand, LPVOID aArg));
// };


#endif