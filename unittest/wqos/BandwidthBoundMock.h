#ifndef _BANDWIDTH_BOUND_MOCK_H_
#define _BANDWIDTH_BOUND_MOCK_H_
#include "gmock/gmock.h"

#include "qosapi.h"
class  CAppBandwidthContactMock: public wqos::IAppBandwidthContact
{
public:
    MOCK_METHOD0(GetMaxBandwidth, UINT32());
    MOCK_METHOD0(GetMinBandwidth, UINT32());
    MOCK_METHOD0(GetPrimaryPriority, INT8());
    MOCK_METHOD1(GetDropFlag, bool(bool));
    MOCK_METHOD0(GetShareFlag, bool());
};

#endif