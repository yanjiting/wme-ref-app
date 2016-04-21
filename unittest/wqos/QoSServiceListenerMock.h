#ifndef _QOS_SERVICE_LISTENER_H_
#define _QOS_SERVICE_LISTENER_H_

#include "gmock/gmock.h"

#include "qosapi.h"

class CQoSServiceListenerMock : public wqos::IQoSServiceListener
{
public:
    MOCK_METHOD4(OnNetworkControlledInformation, void(wqos::AdjustmentState, const wqos::AggregateMetric &, UINT32, UINT32));
};

#endif
