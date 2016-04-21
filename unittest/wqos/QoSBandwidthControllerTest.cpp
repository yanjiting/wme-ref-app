#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "qosapi.h"
#include "mari/wqos/qosdefines.h"
#include "QoSBandwidthController.h"
#include "QoSManager.h"
#include "QoSServiceListenerMock.h"
#include "BandwidthBoundMock.h"
#include "timer.h"
#include <string>
using namespace wqos;
using ::testing::_;
using ::testing::AtLeast;
using ::testing::NiceMock;

class CQoSBandwidthControllerTestEx : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        uint8_t strKey[6] = "12345";
        QoSGroupKey groupKey((uint8_t *)strKey,5);
        pQoSCtl = new CQoSBandwidthController(groupKey);
        pQoSCtl->AddReference();
    }
    virtual void TearDown()
    {
        if (NULL != pQoSCtl) {
            pQoSCtl->Destroy();
            pQoSCtl = NULL;
        }
    }
    CQoSBandwidthController *pQoSCtl;
    CAppBandwidthContactMock     bandwidthBound;
};
TEST_F(CQoSBandwidthControllerTestEx, WQoSCreateBandwidthController_Regular)
{
    QoSGroupKey groupKey;

    uint8_t strKey[6] = "11111";
    memcpy(groupKey.m_buff,strKey,5);

    IQoSBandwidthController *pCtl = WQoSCreateBandwidthController(groupKey);

    EXPECT_NE(pCtl,(IQoSBandwidthController *)NULL);

    IQoSBandwidthController *pCtl2 = WQoSCreateBandwidthController(groupKey);

    EXPECT_EQ(pCtl, pCtl2);

    pCtl->Destroy();
    pCtl2->Destroy();
}

TEST_F(CQoSBandwidthControllerTestEx, WQoSCreateBandwidthController_DifferentController)
{
    QoSGroupKey groupKey;

    uint8_t strKey[6] = "11111";
    memcpy(groupKey.m_buff,strKey,5);

    IQoSBandwidthController *pCtl = WQoSCreateBandwidthController(groupKey);

    EXPECT_NE(pCtl,(IQoSBandwidthController *)NULL);

    QoSGroupKey groupKey2;
    IQoSBandwidthController *pCtl2 = WQoSCreateBandwidthController(groupKey2);

    EXPECT_NE(pCtl, pCtl2);

    pCtl->Destroy();
    pCtl2->Destroy();
}

TEST_F(CQoSBandwidthControllerTestEx, Initialize_First)
{
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);

    int32_t ret = pQoSCtl->Initialize(true, 100);
    EXPECT_EQ(ret, 0);
}
TEST_F(CQoSBandwidthControllerTestEx, Initialize_Twice)
{
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);

    int32_t ret = pQoSCtl->Initialize(true, 100);
    ret = pQoSCtl->Initialize(true, 200);
    EXPECT_EQ(ret, 0);
}

TEST_F(CQoSBandwidthControllerTestEx, RegisterService_UDP_Normal)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);

    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl->Initialize(true, 100);
    ret = pQoSCtl->RegisterService(&bandwidthBound, &serviceLister,serviceId,1,true);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(serviceId, 1);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),1);
}
TEST_F(CQoSBandwidthControllerTestEx, RegisterService_UDP_BadParametersBandwidthBound)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);
    int32_t ret = pQoSCtl->Initialize(true, 100);
    uint32_t serviceId = 0;

    ret = pQoSCtl->RegisterService(NULL, &serviceLister,serviceId,1,true);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),0);
}
TEST_F(CQoSBandwidthControllerTestEx, RegisterService_UDP_BadParametersQoSServiceListener)
{
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);
    int32_t ret = pQoSCtl->Initialize(true, 100);
    uint32_t serviceId = 0;

    ret = pQoSCtl->RegisterService(&bandwidthBound, NULL, serviceId,1,true);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),0);
}
TEST_F(CQoSBandwidthControllerTestEx, RegisterService_Non_UDP_Normal)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);

    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl->Initialize(false, 100);
    ret = pQoSCtl->RegisterService(&bandwidthBound, &serviceLister,serviceId,1,true);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(serviceId, 1);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),1);
}
TEST_F(CQoSBandwidthControllerTestEx, RegisterService_Non_UDP_BadParametersBandwidthBound)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);
    int32_t ret = pQoSCtl->Initialize(false, 100);
    uint32_t serviceId = 0;

    ret = pQoSCtl->RegisterService(NULL, &serviceLister,serviceId,1,true);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),0);
}
TEST_F(CQoSBandwidthControllerTestEx, RegisterService_Non_UDP_BadParametersQoSServiceListener)
{
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);
    int32_t ret = pQoSCtl->Initialize(false, 100);
    uint32_t serviceId = 0;

    ret = pQoSCtl->RegisterService(&bandwidthBound, NULL, serviceId,1,true);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),0);
}
TEST_F(CQoSBandwidthControllerTestEx, RegisterService_BadStatus_Uninitialize)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);

    uint32_t serviceId = 0;

    uint32_t ret = pQoSCtl->RegisterService(&bandwidthBound, &serviceLister,serviceId,1,true);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),0);
}

TEST_F(CQoSBandwidthControllerTestEx, UnregisterService_Normal)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl->Initialize(true, 100);
    ret = pQoSCtl->RegisterService(&bandwidthBound, &serviceLister,serviceId,1,true);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),1);
    ret = pQoSCtl->UnregisterService(serviceId);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),0);
}
TEST_F(CQoSBandwidthControllerTestEx, UnregisterService_Not_Found)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl->Initialize(true, 100);
    ret = pQoSCtl->UnregisterService(serviceId);
    EXPECT_EQ(ret, -1);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),0);
}

TEST_F(CQoSBandwidthControllerTestEx, FeedbackNetworkMetrics_Normal)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl->Initialize(true, 100);
    ret = pQoSCtl->RegisterService(&bandwidthBound, &serviceLister,serviceId,1,true);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),1);

    NetworkMetricEx netMetric;
    uint32_t tick = (uint32_t)low_ticker::now()/1000;
    ret = pQoSCtl->FeedbackNetworkMetrics(tick, serviceId, netMetric);
    EXPECT_EQ(ret, 0);
}
TEST_F(CQoSBandwidthControllerTestEx, FeedbackNetworkMetrics_Not_Found)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl->Initialize(true, 100);

    NetworkMetricEx netMetric;
    uint32_t tick = (uint32_t)low_ticker::now()/1000;
    ret = pQoSCtl->FeedbackNetworkMetrics(tick, serviceId, netMetric);
    EXPECT_EQ(ret, -1);
}

TEST_F(CQoSBandwidthControllerTestEx, FeedbackDataSent_Normal)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);
    uint32_t serviceId = 0;
    uint32_t sourceId = 0;
    int32_t ret = pQoSCtl->Initialize(true, 100);
    ret = pQoSCtl->RegisterService(&bandwidthBound, &serviceLister,serviceId,1,true);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),1);

    uint32_t tick = (uint32_t)low_ticker::now()/1000;
    ret = pQoSCtl->FeedbackDataSent(tick, serviceId, sourceId, 100, false);
    EXPECT_EQ(ret, 0);
}
TEST_F(CQoSBandwidthControllerTestEx, FeedbackDataSent_Sent_Fail)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);
    uint32_t serviceId = 0;
    uint32_t sourceId = 0;
    int32_t ret = pQoSCtl->Initialize(true, 100);
    ret = pQoSCtl->RegisterService(&bandwidthBound, &serviceLister,serviceId,1,true);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),1);

    uint32_t tick = (uint32_t)low_ticker::now()/1000;
    ret = pQoSCtl->FeedbackDataSent(tick, serviceId, sourceId, 100, true);
    EXPECT_EQ(ret, 0);
}
TEST_F(CQoSBandwidthControllerTestEx, FeedbackDataSent_Not_Found)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);
    uint32_t serviceId = 0;
    uint32_t sourceId = 0;
    int32_t ret = pQoSCtl->Initialize(true, 100);
    //uint32_t ret = pQoSCtl->RegisterService(&bandwidthBound, &serviceLister,serviceId);
    //EXPECT_EQ(pQoSCtl->GetServiceNumber(),1);

    uint32_t tick = (uint32_t)low_ticker::now()/1000;
    ret = pQoSCtl->FeedbackDataSent(tick, serviceId, sourceId, 100, false);
    EXPECT_EQ(ret, -1);
}
TEST_F(CQoSBandwidthControllerTestEx, GetAllowedPriority_Normal)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl->Initialize(true, 100);
    ret = pQoSCtl->RegisterService(&bandwidthBound, &serviceLister,serviceId,1,true);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),1);

    //ret = pQoSCtl->GetAllowedPriority(serviceId);
    //EXPECT_NE(ret, 0);
}
TEST_F(CQoSBandwidthControllerTestEx, OnAdjustment_BANDWIDTH_BLOCK)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl->Initialize(true, 100);
    ret = pQoSCtl->RegisterService(&bandwidthBound, &serviceLister,serviceId,1,true);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),1);

    //ret = pQoSCtl->GetAllowedPriority(serviceId);
    AggregateMetric netMetric;
    EXPECT_CALL(serviceLister,OnNetworkControlledInformation(_,_,_,_)).Times(AtLeast(1));
    pQoSCtl->OnAdjustment(BANDWIDTH_BLOCK,150,netMetric,NULL);

}
TEST_F(CQoSBandwidthControllerTestEx, OnAdjustment_Other)
{
    static NiceMock<CQoSServiceListenerMock> serviceLister;
    EXPECT_NE(pQoSCtl,(IQoSBandwidthController *)NULL);
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl->Initialize(true, 100);
    ret = pQoSCtl->RegisterService(&bandwidthBound, &serviceLister,serviceId,1,true);
    EXPECT_EQ(pQoSCtl->GetServiceNumber(),1);

    //ret = pQoSCtl->GetAllowedPriority(serviceId);
    AggregateMetric netMetric;
    EXPECT_CALL(serviceLister,OnNetworkControlledInformation(_,_,_,_)).Times(AtLeast(1));
    pQoSCtl->OnAdjustment(BANDWIDTH_HOLD,100,netMetric,NULL);
}
TEST(CQoSManagerTest, QoSSetOption_QOS_OPTION_ADJUSTMENT_POLICY_Basic_Hybrid)
{
    std::string policy =  AdjustmentPolicyToStringMethod(LOSS_DELAY_HYBRID);

    int32_t ret = QoSSetOption(QOS_OPTION_ADJUSTMENT_POLICY, (uint8_t *)policy.c_str(), (uint32_t)policy.length());
    EXPECT_EQ(0, ret);
    EXPECT_EQ(LOSS_DELAY_HYBRID, CQoSManager::GetInstance()->GetAdjustmentPolicy());

    policy = "Basic-Hybrid";
    ret = QoSSetOption(QOS_OPTION_ADJUSTMENT_POLICY, (uint8_t *)policy.c_str(), (uint32_t)policy.length());
    EXPECT_EQ(0, ret);
    EXPECT_EQ(LOSS_DELAY_HYBRID, CQoSManager::GetInstance()->GetAdjustmentPolicy());
}
TEST(CQoSManagerTest, QoSSetOption_QOS_OPTION_ADJUSTMENT_POLICY_Loss_Based)
{
    std::string policy =  AdjustmentPolicyToStringMethod(LOSS_BASED_ONLY);

    int32_t ret = QoSSetOption(QOS_OPTION_ADJUSTMENT_POLICY, (uint8_t *)policy.c_str(), (uint32_t)policy.length());
    EXPECT_EQ(0, ret);
    EXPECT_EQ(LOSS_BASED_ONLY, CQoSManager::GetInstance()->GetAdjustmentPolicy());

    policy = "Loss-Based";
    ret = QoSSetOption(QOS_OPTION_ADJUSTMENT_POLICY, (uint8_t *)policy.c_str(), (uint32_t)policy.length());
    EXPECT_EQ(0, ret);
    EXPECT_EQ(LOSS_BASED_ONLY, CQoSManager::GetInstance()->GetAdjustmentPolicy());
}
TEST(CQoSManagerTest, QoSSetOption_QOS_OPTION_ADJUSTMENT_POLICY_Unknown_Policy)
{
    std::string policy = "Unknown_Policy";
    int32_t ret = QoSSetOption(QOS_OPTION_ADJUSTMENT_POLICY, (uint8_t *)policy.c_str(), (uint32_t)policy.length());
    EXPECT_EQ(-1, ret);
}
