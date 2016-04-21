#ifndef MOCK_CMCHANNELHTTPSERVERACCEPTOR_H
#define MOCK_CMCHANNELHTTPSERVERACCEPTOR_H

#include <gmock/gmock.h>
#include "CmChannelHttpServerAcceptor.h"
#include "CmInetAddr.h"

class MockCmChannelHttpServerAcceptor : public CCmChannelHttpServerAcceptor {
/*
public:
  MOCK_METHOD3(StartListen,
      CmResult(ICmChannelServerSink *aSink, const CCmInetAddr &aAddrListen, int nTraceInterval));
  MOCK_METHOD1(StopListen,
      CmResult(CmResult aReason));
  MOCK_METHOD0(AddReference,
      DWORD());
  MOCK_METHOD0(ReleaseReference,
      DWORD());
  MOCK_METHOD3(OnConnectIndication,
      void(CmResult aReason, ICmTransport *aTrpt, ICmAcceptorConnectorId *aRequestId));
*/
};

#endif
