#include "gmock/gmock.h"
#include "qosapi.h"

using namespace wqos;

class MockQoSBandwidthController: public IQoSBandwidthController
{
public:
    MOCK_METHOD2(Initialize, int32_t(BOOL bUDPMode,uint32_t uTargetBandwidth));
    MOCK_METHOD4(RegisterService, int32_t(IAppBandwidthContact *pBandwidthBound,
                                          IQoSServiceListener *pListener,
                                          uint32_t &serviceId, BYTE byPriority));
    MOCK_METHOD1(UnregisterService, int32_t(uint32_t serviceId));
    MOCK_METHOD3(FeedbackNetworkMetrics, int32_t(uint32_t tickNowMs, uint32_t serviceId, NetworkMetricEx &netMetric));
    MOCK_METHOD4(FeedbackDataSent, int32_t(uint32_t tickNowMs, uint32_t serviceId, uint32_t dataLen, BOOL bSentFail));
    MOCK_METHOD2(UpdateLeakyBucketSize, void(uint32_t serviceId, uint32_t dataLen));
    MOCK_METHOD1(GetAllowedPriority, uint8_t(uint32_t serviceId));
    MOCK_METHOD2(RequestBandwidth, BOOL(uint32_t serviceId, uint32_t uBandwidth));
    MOCK_METHOD0(Destroy, void());
};