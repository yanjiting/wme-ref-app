#ifndef  __MOCK_IIceEngineSink_H__
#define  __MOCK_IIceEngineSink_H__

#include <gmock/gmock.h>
#include "csiceengine.h"

class MockIIceEngineSink : public IIceEngineSink
{
public:
	MOCK_METHOD1(OnIceConcluded, void(const IIceEngine* pIcer));
	MOCK_METHOD4(OnValidTransport, void(const IIceEngine* pIcer, const IIceEngineTransport** pTransport, int iTransportCount, UINT64 transportPriority));
	MOCK_METHOD2(OnIceError, void(const IIceEngine* pIcer, int iError));
	MOCK_METHOD4(OnLocalCandidateInfo, void(const IIceEngine* pIcer,const IceEngineCandidate** pLocalCandidates,int iCandidatesSize, bool bCompleted));
};

#endif