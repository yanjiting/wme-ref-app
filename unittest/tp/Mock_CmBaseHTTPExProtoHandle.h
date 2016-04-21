
#ifndef __MOCK_CMBASEHTTPEXPROTOHANDLE_H__ 
#define __MOCK_CMBASEHTTPEXPROTOHANDLE_H__

#include <gmock/gmock.h>
#include "CmBaseHTTPExProtoHandle.h"


class MockICmBaseHTTPExProtoHandle: public ICmBaseHTTPExProtoHandle 
{
public:

    MOCK_METHOD2(Initialize,
        CmResult(ICmBaseHTTPExProtoHandleSink *pHTTPExProtoSink, ICmOnePortSink *pOnePortSink));

    MOCK_METHOD2(Handshake,
        CmResult(ICmChannelHttpServer *pHttpRequest, CCmMessageBlock *pData));

    MOCK_METHOD0(AddReference,
        DWORD());

    MOCK_METHOD0(ReleaseReference,
        DWORD());
    
};


class MockICmBaseHTTPExProtoHandleSink: public ICmBaseHTTPExProtoHandleSink 
{
public:

    MOCK_METHOD2(OnConnect,
        void(CmResult aReason, ICmBaseHTTPExProtoHandle *pProtoHandle));

    MOCK_METHOD2(OnDisconnect,
        void(CmResult aReason, ICmBaseHTTPExProtoHandle *pProtoHandle));
};

#endif
