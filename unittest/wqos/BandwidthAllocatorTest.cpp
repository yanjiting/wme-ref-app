#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define private public
#include "QoSBandwidthController.h"
#include "BandwidthAllocator.h"
#include "ServiceWrapper.h"
#include "QosManager.h"
#include <string>

using namespace wqos;
uint8_t strKey[6] = "12345";
QoSGroupKey groupKey((uint8_t *)strKey,5);

class CLocalCtl: public CQoSBandwidthController
{
public:
    CLocalCtl()
        :CQoSBandwidthController(groupKey)
    {

    }
public:
    int32_t GetServiceLossRatio(INetworkMonitor *pNetworkMonitor, float &fLossRatio)
    {
        fLossRatio = (float)0.05;
        return 0;
    }
};

class CAppBandwidthContactLocalMock : public IAppBandwidthContact
{
public:
    CAppBandwidthContactLocalMock() {}
    void Init(uint32_t uMaxB, uint32_t uMinB, int8_t iPri, bool bDrop, bool bShared)
    {
        m_uMaxB = uMaxB;
        m_uMinB = uMinB;
        m_iPri = iPri;
        m_bDrop = bDrop;
        m_bShared = bShared;
    }
public:
    uint32_t GetMaxBandwidth()
    {
        return m_uMaxB;
    }
    uint32_t GetMinBandwidth()
    {
        return m_uMinB;
    }
    int8_t   GetPrimaryPriority()
    {
        return m_iPri;
    }
    bool GetDropFlag(bool bReset)
    {
        return m_bDrop;
    }
    bool GetShareFlag()
    {
        return m_bShared;
    }
private:
    uint32_t  m_uMaxB;
    uint32_t  m_uMinB;
    int8_t    m_iPri;
    bool    m_bDrop;
    bool    m_bShared;
};

class CQoSServiceListenerLocalMock : public wqos::IQoSServiceListener
{
public:
    CQoSServiceListenerLocalMock()
    : m_bandwidth(0)
    , m_called(0)
    {
    }

public:
    void OnNetworkControlledInformation(AdjustmentState state, const AggregateMetric &netMetric, uint32_t bandwidth,uint32_t totalBandwidth)
    {
        m_bandwidth = bandwidth;
        m_called++;
    }
public:
    uint32_t m_bandwidth;
    uint32_t m_called;
};

class CBandwidthEvaluatorProxyMock: public CBandwidthEvaluatorProxy
{
public:
    CBandwidthEvaluatorProxyMock()
    : CBandwidthEvaluatorProxy(true)
    , m_called(0)
    {
    }

    void SetConfig(PARAM_IN const BandwidthEvaluatorConfig &config)
    {
        if (config.initBandwidth != 0) {
            m_config.initBandwidth = config.initBandwidth;
        }
        if (config.minBandwidth != 0) {
            m_config.minBandwidth = config.minBandwidth;
        }
        if (config.maxBandwidth != 0) {
            m_config.maxBandwidth = config.maxBandwidth;
        }
        m_called++;
    }

private:
    BandwidthEvaluatorConfig m_config;
    uint32_t m_called;
};

class CBandwidthAllocatorTest : public ::testing::Test
{
protected:
    virtual void SetUp()
    {
        serviceAudio.Init(40,40,1,false,false);
        serviceSVC.Init(2000,80,3,true,false);
        serviceSVS.Init(1000,60,2,true,false);
    }
    virtual void TearDown()
    {

    }
    CAppBandwidthContactLocalMock serviceAudio;
    CAppBandwidthContactLocalMock serviceSVC;
    CAppBandwidthContactLocalMock serviceSVS;
    CQoSServiceListenerLocalMock  serviceListenerMockAudio;
    CQoSServiceListenerLocalMock  serviceListenerMockSVC;
    CQoSServiceListenerLocalMock  serviceListenerMockSVS;
    CBandwidthEvaluatorProxyMock  evaluator;
    CLocalCtl                     pQoSCtl;
};

TEST_F(CBandwidthAllocatorTest, RegisterService_Invalid_Service_Pointer_Fail)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(NULL,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_NE(ret,0);
    ret = pQoSCtl.RegisterService(&serviceAudio,NULL,serviceId,1,true);
    EXPECT_NE(ret,0);

}

TEST_F(CBandwidthAllocatorTest, RegisterService_Failed_When_QoSCtlIsNotInitialized)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_NE(ret,0);
}

TEST_F(CBandwidthAllocatorTest, RegisterService_Invalid_Priority_Fail)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,100,true);
    EXPECT_NE(ret,0);

}

TEST_F(CBandwidthAllocatorTest, RegisterService_Success)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);
}

TEST_F(CBandwidthAllocatorTest, RegisterService_With_Same_ID_Again_Fail)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);
    serviceId--;
    pQoSCtl.m_uServiceIdCounter = serviceId;
    ret = pQoSCtl.RegisterService(&serviceSVC,&serviceListenerMockSVC,serviceId,3,true);
    EXPECT_NE(ret,0);
}

TEST_F(CBandwidthAllocatorTest, GetService_Success)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);
    CServicePtr pServiceWrap = pQoSCtl.m_bandwidthAllocator.GetService(serviceId);
    EXPECT_NE(pServiceWrap.get(), (CServiceWrapper *)NULL);
}

TEST_F(CBandwidthAllocatorTest, GetService_With_Invalid_ID_Fail)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);
    CServicePtr pServiceWrap = pQoSCtl.m_bandwidthAllocator.GetService(111);
    EXPECT_EQ(pServiceWrap.get(), (CServiceWrapper *)NULL);
}

TEST_F(CBandwidthAllocatorTest, UnregisterService_Twice_With_Same_ID_Fail)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);
    ret = pQoSCtl.UnregisterService(serviceId);
    EXPECT_EQ(ret,0);
    ret = pQoSCtl.UnregisterService(serviceId);
    EXPECT_NE(ret,0);

}

TEST_F(CBandwidthAllocatorTest, UnregisterService_With_Invalid_ID_Fail)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);
    ret = pQoSCtl.UnregisterService(111);
    EXPECT_NE(ret,0);
}

TEST_F(CBandwidthAllocatorTest, UnregisterService_Success)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);
    ret = pQoSCtl.UnregisterService(serviceId);
    EXPECT_EQ(ret,0);
}

TEST_F(CBandwidthAllocatorTest, FeedbackNetworkMetrics_Success_But_NoAction)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);

    FeedbackResult feedbackResult = FEEDBACK_OK;
    NetworkMetricEx netMetric;
    netMetric.receiveRate = 256;
    NetworkFeedback networkFeedback;

    uint32_t tick = (uint32_t)low_ticker::now()/1000;
    ret = pQoSCtl.m_bandwidthAllocator.FeedbackNetworkMetrics(tick, serviceId, netMetric, feedbackResult, networkFeedback);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(FEEDBACK_ACTION, feedbackResult);

    tick = (uint32_t)low_ticker::now()/1000;
    ret = pQoSCtl.m_bandwidthAllocator.FeedbackNetworkMetrics(tick, serviceId, netMetric, feedbackResult, networkFeedback);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(FEEDBACK_ACTION, feedbackResult);
}

TEST_F(CBandwidthAllocatorTest, FeedbackNetworkMetrics_Success)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);
    NetworkMetricEx netMetric;
    uint32_t tick = (uint32_t)low_ticker::now()/1000;
    ret = pQoSCtl.FeedbackNetworkMetrics(tick, serviceId, netMetric);
    EXPECT_EQ(ret, 0);
}

TEST_F(CBandwidthAllocatorTest, FeedbackNetworkMetrics_With_Invalid_ID_Fail)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);
    NetworkMetricEx netMetric;
    uint32_t tick = (uint32_t)low_ticker::now()/1000;
    ret = pQoSCtl.FeedbackNetworkMetrics(tick, 111, netMetric);
    EXPECT_NE(ret, 0);
}

TEST_F(CBandwidthAllocatorTest, FeedbackDataSent_Success)
{
    uint32_t serviceId = 0;
    uint32_t sourceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);
    NetworkMetricEx netMetric;
    uint32_t tick = (uint32_t)low_ticker::now()/1000;
    ret = pQoSCtl.FeedbackDataSent(tick, serviceId, sourceId, 100, false);
    EXPECT_EQ(ret, 0);
}

TEST_F(CBandwidthAllocatorTest, FeedbackDataSent_With_Invalid_ID_Fail)
{
    uint32_t serviceId = 0;
    uint32_t sourceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);
    NetworkMetricEx netMetric;
    uint32_t tick = (uint32_t)low_ticker::now()/1000;
    ret = pQoSCtl.FeedbackDataSent(tick, 111, sourceId, 100, false);
    EXPECT_NE(ret, 0);
}

TEST_F(CBandwidthAllocatorTest, ReallocBW_one_service)
{
    uint32_t serviceId = 0;
    uint32_t sourceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);

    uint32_t tick = (uint32_t)low_ticker::now()/1000;
    ret = pQoSCtl.FeedbackDataSent(tick, serviceId, sourceId, 100, false);

    AggregateMetric netMetric;
    pQoSCtl.OnAdjustment(BANDWIDTH_HOLD,100,netMetric,NULL);

    EXPECT_GT(serviceListenerMockAudio.m_called, 1);
    EXPECT_EQ(100, serviceListenerMockAudio.m_bandwidth);
}

TEST_F(CBandwidthAllocatorTest, ReallocBW_three_services_adjust_Only_Audio_Max_Satisfied)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);

    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);

    ret = pQoSCtl.RegisterService(&serviceSVS,&serviceListenerMockSVS,serviceId,2,true);
    EXPECT_EQ(ret,0);

    ret = pQoSCtl.RegisterService(&serviceSVC,&serviceListenerMockSVC,serviceId,3,true);
    EXPECT_EQ(ret,0);

    AggregateMetric netMetric;
    pQoSCtl.OnAdjustment(BANDWIDTH_HOLD,200,netMetric,NULL);

    EXPECT_GT(serviceListenerMockAudio.m_called, 1);
    EXPECT_GT(serviceListenerMockSVS.m_called, 1);
    EXPECT_GT(serviceListenerMockSVC.m_called, 1);

    EXPECT_EQ(40, serviceListenerMockAudio.m_bandwidth);
    EXPECT_EQ(80, serviceListenerMockSVS.m_bandwidth);
    EXPECT_EQ(80, serviceListenerMockSVC.m_bandwidth);
}

TEST_F(CBandwidthAllocatorTest, ReallocBW_three_services_adjust_droped_Satisfied)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);

    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);

    ret = pQoSCtl.RegisterService(&serviceSVS,&serviceListenerMockSVS,serviceId,2,true);
    EXPECT_EQ(ret,0);

    ret = pQoSCtl.RegisterService(&serviceSVC,&serviceListenerMockSVC,serviceId,3,true);
    EXPECT_EQ(ret,0);


    AggregateMetric netMetric;
    pQoSCtl.OnAdjustment(BANDWIDTH_HOLD,3500,netMetric,NULL);

    EXPECT_GT(serviceListenerMockAudio.m_called, 1);
    EXPECT_GT(serviceListenerMockSVS.m_called, 1);
    EXPECT_GT(serviceListenerMockSVC.m_called, 1);

    EXPECT_EQ(40, serviceListenerMockAudio.m_bandwidth);
    EXPECT_EQ(1460, serviceListenerMockSVS.m_bandwidth);
    EXPECT_EQ(2000, serviceListenerMockSVC.m_bandwidth);
}

TEST_F(CBandwidthAllocatorTest, ReallocBW_three_services_adjust_reminder_Satisfied)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);

    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);

    serviceSVS.Init(1000,60,2,false,false);
    ret = pQoSCtl.RegisterService(&serviceSVS,&serviceListenerMockSVS,serviceId,2,true);
    EXPECT_EQ(ret,0);

    serviceSVC.Init(2000,80,3,false,false);
    ret = pQoSCtl.RegisterService(&serviceSVC,&serviceListenerMockSVC,serviceId,3,true);
    EXPECT_EQ(ret,0);


    AggregateMetric netMetric;
    pQoSCtl.OnAdjustment(BANDWIDTH_HOLD,3500,netMetric,NULL);

    EXPECT_GT(serviceListenerMockAudio.m_called, 1);
    EXPECT_GT(serviceListenerMockSVS.m_called, 1);
    EXPECT_GT(serviceListenerMockSVC.m_called, 1);

    EXPECT_EQ(40, serviceListenerMockAudio.m_bandwidth);
    EXPECT_EQ(1000, serviceListenerMockSVS.m_bandwidth);
    EXPECT_EQ(2460, serviceListenerMockSVC.m_bandwidth);
}

TEST_F(CBandwidthAllocatorTest, NotConfigEvaluator_When_MinAndMaxBWIsEnough)
{
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    pQoSCtl.m_bandwidthAllocator.SetEvaluator(&evaluator);

    serviceAudio.Init(40*1024/8, 40*1024/8, 1, false, false);
    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);

    ASSERT_GE(evaluator.m_called, 1);

    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().initBandwidth, evaluator.m_config.initBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().minBandwidth, evaluator.m_config.minBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().maxBandwidth, evaluator.m_config.maxBandwidth);

    ret = pQoSCtl.UnregisterService(serviceId);
    EXPECT_EQ(ret,0);
}

TEST_F(CBandwidthAllocatorTest, NotConfigEvaluator_When_MaxBWIsNotEnoughButPriorityIsZero)
{
    uint32_t serviceIdAudio = 0, serviceIdSVC = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    pQoSCtl.m_bandwidthAllocator.SetEvaluator(&evaluator);

    serviceSVC.Init(1420*1024/8, 60*1024/8, 3,false,false);
    ret = pQoSCtl.RegisterService(&serviceSVC, &serviceListenerMockSVC, serviceIdSVC, 3, true);
    EXPECT_EQ(ret,0);

    serviceAudio.Init(40*1024/8, 40*1024/8, 1, false, false);
    ret = pQoSCtl.RegisterService(&serviceAudio, &serviceListenerMockAudio, serviceIdAudio, 0, true);
    EXPECT_EQ(ret,0);

    ASSERT_GE(evaluator.m_called, 1);

    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().initBandwidth, evaluator.m_config.initBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().minBandwidth, evaluator.m_config.minBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().maxBandwidth, evaluator.m_config.maxBandwidth);

    ret = pQoSCtl.UnregisterService(serviceIdSVC);
    EXPECT_EQ(ret,0);

    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().initBandwidth, evaluator.m_config.initBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().minBandwidth, evaluator.m_config.minBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().maxBandwidth, evaluator.m_config.maxBandwidth);

    ret = pQoSCtl.UnregisterService(serviceIdAudio);
    EXPECT_EQ(ret,0);
}

TEST_F(CBandwidthAllocatorTest, ConfigEvaluator_When_MinBWIsNotEnough)
{
    uint32_t serviceIdAudio = 0, serviceIdSVC = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    pQoSCtl.m_bandwidthAllocator.SetEvaluator(&evaluator);

    serviceAudio.Init(40*1024/8, 40*1024/8, 1, false, false);
    ret = pQoSCtl.RegisterService(&serviceAudio, &serviceListenerMockAudio, serviceIdAudio, 0, true);
    EXPECT_EQ(ret,0);

    serviceSVC.Init(1000*1024/8, 100*1024/8, 3,false,false);
    ret = pQoSCtl.RegisterService(&serviceSVC, &serviceListenerMockSVC, serviceIdSVC, 3, true);
    EXPECT_EQ(ret,0);

    ASSERT_GE(evaluator.m_called, 1);

    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().initBandwidth, evaluator.m_config.initBandwidth);
    EXPECT_EQ(140*1024/8, evaluator.m_config.minBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().maxBandwidth, evaluator.m_config.maxBandwidth);

    ret = pQoSCtl.UnregisterService(serviceIdSVC);
    EXPECT_EQ(ret,0);

    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().initBandwidth, evaluator.m_config.initBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().minBandwidth, evaluator.m_config.minBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().maxBandwidth, evaluator.m_config.maxBandwidth);

    ret = pQoSCtl.UnregisterService(serviceIdAudio);
    EXPECT_EQ(ret,0);
}

TEST_F(CBandwidthAllocatorTest, ConfigEvaluator_When_MaxBWIsNotEnough)
{
    uint32_t serviceIdAudio = 0, serviceIdSVC = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    pQoSCtl.m_bandwidthAllocator.SetEvaluator(&evaluator);

    serviceAudio.Init(40*1024/8, 40*1024/8, 1, false, false);
    ret = pQoSCtl.RegisterService(&serviceAudio, &serviceListenerMockAudio, serviceIdAudio, 1, true);
    EXPECT_EQ(ret,0);

    serviceSVC.Init(2000*1024/8, 60*1024/8, 3,false,false);
    ret = pQoSCtl.RegisterService(&serviceSVC, &serviceListenerMockSVC, serviceIdSVC, 3, true);
    EXPECT_EQ(ret,0);

    ASSERT_GE(evaluator.m_called, 1);

    EXPECT_EQ(2040*1024/8, evaluator.m_config.initBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().minBandwidth, evaluator.m_config.minBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().maxBandwidth, evaluator.m_config.maxBandwidth);

    ret = pQoSCtl.UnregisterService(serviceIdSVC);
    EXPECT_EQ(ret,0);

    EXPECT_EQ(2040*1024/8, evaluator.m_config.initBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().minBandwidth, evaluator.m_config.minBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().maxBandwidth, evaluator.m_config.maxBandwidth);

    ret = pQoSCtl.UnregisterService(serviceIdAudio);
    EXPECT_EQ(ret,0);
}

TEST_F(CBandwidthAllocatorTest, ConfigEvaluator_When_MinAndMaxBWIsNotEnough)
{
    uint32_t serviceIdAudio = 0, serviceIdSVC = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);
    pQoSCtl.m_bandwidthAllocator.SetEvaluator(&evaluator);

    serviceAudio.Init(40*1024/8, 40*1024/8, 1, false, false);
    ret = pQoSCtl.RegisterService(&serviceAudio, &serviceListenerMockAudio, serviceIdAudio, 1, true);
    EXPECT_EQ(ret,0);

    serviceSVC.Init(20000*1024/8, 100*1024/8, 3,false,false);
    ret = pQoSCtl.RegisterService(&serviceSVC, &serviceListenerMockSVC, serviceIdSVC, 3, true);
    EXPECT_EQ(ret,0);

    ASSERT_GE(evaluator.m_called, 1);

    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().maxBandwidth, evaluator.m_config.initBandwidth);
    EXPECT_EQ(140*1024/8, evaluator.m_config.minBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().maxBandwidth, evaluator.m_config.maxBandwidth);

    ret = pQoSCtl.UnregisterService(serviceIdSVC);
    EXPECT_EQ(ret,0);

    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().maxBandwidth, evaluator.m_config.initBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().minBandwidth, evaluator.m_config.minBandwidth);
    EXPECT_EQ(CQoSManager::GetInstance()->GetBandwidthConfig().maxBandwidth, evaluator.m_config.maxBandwidth);

    ret = pQoSCtl.UnregisterService(serviceIdAudio);
    EXPECT_EQ(ret,0);
}

TEST_F(CBandwidthAllocatorTest, NotifyNetworkFeedback)
{
    uint32_t serviceId = 1;

    CServicePtr pService(new CServiceWrapper(serviceId, true, &serviceAudio, &serviceListenerMockAudio, 2));
    CBandwidthAllocate bwAllocate;
    bwAllocate.RegisterService(pService, serviceId);


    NetworkFeedback networkFeedback1;
    networkFeedback1.state = BANDWIDTH_DOWN | LOSS_TRIGGER;

    NetworkFeedback networkFeedback2;
    networkFeedback2.state = BANDWIDTH_DOWN | DELAY_TRIGGER;

    bwAllocate.NotifyNetworkFeedback(1, networkFeedback1);
    bwAllocate.NotifyNetworkFeedback(1, networkFeedback1);
    bwAllocate.NotifyNetworkFeedback(1, networkFeedback1);

    bwAllocate.NotifyNetworkFeedback(1, networkFeedback2);
    bwAllocate.NotifyNetworkFeedback(1, networkFeedback2);

    BandwidthDownStats stat;

    bwAllocate.GetBwDownEventStats(serviceId, stat);

    EXPECT_EQ(3, stat.lossEventCount);
    EXPECT_EQ(2, stat.delayEventCount);
}

TEST_F(CBandwidthAllocatorTest, GetEnoughRatioSuccessfulWhenActiveServiceCountIsNotZero_1)
{
    NetworkMetricEx netMetric;
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);

    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);

    ret = pQoSCtl.RegisterService(&serviceSVS,&serviceListenerMockSVS,serviceId,2,true);
    EXPECT_EQ(ret,0);

    ret = pQoSCtl.FeedbackNetworkMetrics(1000, serviceId, netMetric);
    EXPECT_EQ(ret,0);

    AggregateMetric aggregateNetMetric;
    pQoSCtl.OnAdjustment(BANDWIDTH_HOLD, 5000, aggregateNetMetric, NULL);

    NetworkMetricStatsEx networkMetricEx;
    EXPECT_TRUE(pQoSCtl.GetNetworkMetricsEx(2000, networkMetricEx));
    EXPECT_FLOAT_EQ(1.0, networkMetricEx.enoughRatio);
}

TEST_F(CBandwidthAllocatorTest, GetEnoughRatioSuccessfulWhenActiveServiceCountIsNotZero_2)
{
    NetworkMetricEx netMetric;
    uint32_t serviceId = 0;
    int32_t ret = pQoSCtl.Initialize(true,1000);

    ret = pQoSCtl.RegisterService(&serviceAudio,&serviceListenerMockAudio,serviceId,1,true);
    EXPECT_EQ(ret,0);

    ret = pQoSCtl.RegisterService(&serviceSVS,&serviceListenerMockSVS,serviceId,2,true);
    EXPECT_EQ(ret,0);

    ret = pQoSCtl.FeedbackNetworkMetrics(1000, serviceId, netMetric);
    EXPECT_EQ(ret,0);
    AggregateMetric aggregateNetMetric;
    pQoSCtl.OnAdjustment(BANDWIDTH_HOLD, 200, aggregateNetMetric, NULL);

    ret = pQoSCtl.FeedbackNetworkMetrics(2000, serviceId, netMetric);
    EXPECT_EQ(ret,0);
    pQoSCtl.OnAdjustment(BANDWIDTH_HOLD, 5000, aggregateNetMetric, NULL);

    NetworkMetricStatsEx networkMetricEx;
    EXPECT_TRUE(pQoSCtl.GetNetworkMetricsEx(3000, networkMetricEx));
    EXPECT_FLOAT_EQ(0.5, networkMetricEx.enoughRatio);
}

TEST(CBandwidthEnoughMetricsTest, GetEnoughRatioFailedWhenTickNowNotUpdated)
{
    CBandwidthEnoughMetrics bandwidthEnoughMetrics;

    float ratio = 0.0f;
    bandwidthEnoughMetrics.Update(2, true);
    EXPECT_FALSE(bandwidthEnoughMetrics.GetEnoughRatio(1000, ratio));
}

TEST(CBandwidthEnoughMetricsTest, GetEnoughRatioFailedWhenActiveServiceCountIsZero)
{
    CBandwidthEnoughMetrics bandwidthEnoughMetrics;

    float ratio = 0.0f;
    bandwidthEnoughMetrics.UpdateTickNowMs(1000);
    bandwidthEnoughMetrics.Update(0, true);
    EXPECT_FALSE(bandwidthEnoughMetrics.GetEnoughRatio(1000, ratio));
}

TEST(CBandwidthEnoughMetricsTest, GetEnoughRatioSuccessfulWhenActiveServiceCountIsNotZero_1)
{
    CBandwidthEnoughMetrics bandwidthEnoughMetrics;

    float ratio = 0.0f;
    bandwidthEnoughMetrics.UpdateTickNowMs(1000);
    bandwidthEnoughMetrics.Update(2, true);
    EXPECT_TRUE(bandwidthEnoughMetrics.GetEnoughRatio(5000, ratio));
    EXPECT_FLOAT_EQ(1.0, ratio);
}

TEST(CBandwidthEnoughMetricsTest, GetEnoughRatioSuccessfulWhenActiveServiceCountIsNotZero_2)
{
    CBandwidthEnoughMetrics bandwidthEnoughMetrics;

    float ratio = 0.0f;
    bandwidthEnoughMetrics.UpdateTickNowMs(1000);
    bandwidthEnoughMetrics.Update(2, false);
    bandwidthEnoughMetrics.UpdateTickNowMs(2000);
    bandwidthEnoughMetrics.Update(2, true);
    bandwidthEnoughMetrics.UpdateTickNowMs(3000);
    bandwidthEnoughMetrics.Update(2, false);
    bandwidthEnoughMetrics.UpdateTickNowMs(4000);
    bandwidthEnoughMetrics.Update(2, true);
    EXPECT_TRUE(bandwidthEnoughMetrics.GetEnoughRatio(5000, ratio));
    EXPECT_FLOAT_EQ(0.5, ratio);
}

TEST(CBandwidthEnoughMetricsTest, GetEnoughRatioSuccessfulWhenActiveServiceCountIsNotZero_3)
{
    CBandwidthEnoughMetrics bandwidthEnoughMetrics;

    float ratio = 0.0f;
    bandwidthEnoughMetrics.UpdateTickNowMs(1000);
    bandwidthEnoughMetrics.Update(2, false);
    bandwidthEnoughMetrics.UpdateTickNowMs(2000);
    bandwidthEnoughMetrics.Update(2, true);
    bandwidthEnoughMetrics.UpdateTickNowMs(3000);
    bandwidthEnoughMetrics.Update(0, true);
    EXPECT_TRUE(bandwidthEnoughMetrics.GetEnoughRatio(4000, ratio));
    EXPECT_FLOAT_EQ(0.5, ratio);
    bandwidthEnoughMetrics.Update(2, true);
    EXPECT_TRUE(bandwidthEnoughMetrics.GetEnoughRatio(6000, ratio));
    EXPECT_FLOAT_EQ(0.75, ratio);
}
