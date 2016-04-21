#include <gmock/gmock.h>
#include <gtest/gtest.h>

#define private public
#include "CmThread.h"
#include "qosapi.h"
#include "QoSBandwidthController.h"
#include "QoSManager.h"
#include "timer.h"
#include "CmUtilMisc.h"

#include <string>
#include <sstream>

using namespace wqos;

class CQoSServiceListenerMockMT : public wqos::IQoSServiceListener
{
public:
    virtual void OnNetworkControlledInformation(AdjustmentState state, const AggregateMetric &netMetric,
            uint32_t uBandwidth,uint32_t uTotalBandwidth) {}
};

class  CAppBandwidthContactMockMT: public wqos::IAppBandwidthContact
{
public:
    virtual uint32_t GetMaxBandwidth() { return 100; }
    virtual uint32_t GetMinBandwidth() { return 50; }
    virtual int8_t   GetPrimaryPriority() { return 1; }
    virtual bool GetDropFlag(bool bReset) { return false; }
    virtual bool GetShareFlag() {return false;}
};

static CCmAtomicOperationT<CCmMutexThread> s_counter = 0;
class CQoSThread : public ACmThread
{
public:
    CQoSThread(IQoSBandwidthController *ctrl) : m_action(0), m_serviceId(0), m_sourceId(0), m_qosController(ctrl) {}
    virtual void OnThreadRun()
    {

        int32_t ret = m_qosController->RegisterService(&m_bandwidthBound, &m_serviceLister, m_serviceId, 1, true);
        EXPECT_TRUE(0 == ret);
        uint32_t tick = (uint32_t)low_ticker::now()/1000;
        if (1 == m_action) {
            NetworkMetricEx nm;
            m_qosController->FeedbackNetworkMetrics(tick, m_serviceId, nm);
        } else if (2 == m_action) {
            NetworkMetricEx nm;
            m_qosController->FeedbackNetworkMetrics(tick, m_serviceId, nm);
            tick = (uint32_t)low_ticker::now()/1000;
            m_qosController->FeedbackDataSent(tick, m_serviceId, m_sourceId, 1024, false);
            tick = (uint32_t)low_ticker::now()/1000;
            m_qosController->FeedbackDataSent(tick, m_serviceId, m_sourceId, 1024, true);
        }
        ++s_counter;
    }

    CmResult Stop(CCmTimeValue *aTimeout = NULL)
    {
        return CM_OK;
    }

    void Destroy()
    {
        m_qosController->UnregisterService(m_serviceId);

        this->Stop();
        this->Join();
        this->Destory(CM_OK);
    }

public:
    uint8_t m_action;
    uint32_t m_serviceId;
    uint32_t m_sourceId;
    IQoSBandwidthController    *m_qosController;
    CQoSServiceListenerMockMT   m_serviceLister;
    CAppBandwidthContactMockMT  m_bandwidthBound;
};

class CQoSMultiThreadTest : public testing::Test
{
public:
    CQoSMultiThreadTest() : m_qosController(NULL)
    {
    }

    virtual void SetUp()
    {
        QoSGroupKey groupKey((const uint8_t *)"12345", 5);
        m_qosController = WQoSCreateBandwidthController(groupKey);
        m_qosController->Initialize(true, 100);
    }

    virtual void TearDown()
    {
        if (m_qosController) {
            m_qosController->Destroy();
            m_qosController = NULL;
        }
    }

public:
    IQoSBandwidthController    *m_qosController;
};

#define MAX_THREAD_COUNT   50
TEST_F(CQoSMultiThreadTest, WQoSMultiThreadTest_RegisterService_500)
{
    ASSERT_NE(m_qosController, (IQoSBandwidthController *)NULL);
    CQoSThread **threadList = new CQoSThread*[MAX_THREAD_COUNT];
    int i = 0;
    s_counter = 0;
    for (i = 0; i < MAX_THREAD_COUNT; ++i) {
        threadList[i] = new CQoSThread(m_qosController);
        CmResult result = threadList[i]->Create("QoS",TT_CURRENT, TF_JOINABLE, true);
        EXPECT_TRUE(CM_SUCCEEDED(result));
    }

    while (s_counter < MAX_THREAD_COUNT) {
        ::SleepMs(10);
    }
    CQoSBandwidthController *qosController = dynamic_cast<CQoSBandwidthController *>(m_qosController);
    EXPECT_EQ(qosController->GetServiceNumber(), MAX_THREAD_COUNT);
    EXPECT_EQ(qosController->m_uServiceIdCounter, MAX_THREAD_COUNT);

    for (i = 0; i < MAX_THREAD_COUNT; ++i) {
        threadList[i]->Destroy();
    }
    delete[] threadList;
    EXPECT_EQ(qosController->GetServiceNumber(), 0);
}

TEST_F(CQoSMultiThreadTest, WQoSMultiThreadTest_FeedbackNetworkMetrics_500)
{
    ASSERT_NE(m_qosController, (IQoSBandwidthController *)NULL);
    CQoSThread **threadList = new CQoSThread*[MAX_THREAD_COUNT];
    int i = 0;
    s_counter = 0;
    for (i = 0; i < MAX_THREAD_COUNT; ++i) {
        threadList[i] = new CQoSThread(m_qosController);
        threadList[i]->m_action = 1;
        CmResult result = threadList[i]->Create("QoS", TT_CURRENT, TF_JOINABLE, false);
        EXPECT_TRUE(CM_SUCCEEDED(result));
    }

    while (s_counter < MAX_THREAD_COUNT) {
        ::SleepMs(10);
    }
    CQoSBandwidthController *qosController = dynamic_cast<CQoSBandwidthController *>(m_qosController);
    EXPECT_EQ(qosController->GetServiceNumber(), MAX_THREAD_COUNT);

    for (i = 0; i < MAX_THREAD_COUNT; ++i) {
        threadList[i]->Destroy();
    }
    delete[] threadList;
    EXPECT_EQ(qosController->GetServiceNumber(), 0);
}

#define TEST_THREAD_COUNT   100
TEST_F(CQoSMultiThreadTest, WQoSMultiThreadTest_FeedbackDataSent_100)
{
    QoSGroupKey groupKey((const uint8_t *)"159357", 6);
    IQoSBandwidthController *iqbc = WQoSCreateBandwidthController(groupKey);
    ASSERT_NE(iqbc, (IQoSBandwidthController *)NULL);
    iqbc->Initialize(true, 100);

    CQoSThread **threadList = new CQoSThread*[TEST_THREAD_COUNT];
    int i = 0;
    s_counter = 0;
    for (i = 0; i < TEST_THREAD_COUNT; ++i) {
        threadList[i] = new CQoSThread(iqbc);
        threadList[i]->m_action = 2;
        CmResult result = threadList[i]->Create("QoS", TT_CURRENT, TF_JOINABLE, false);
        EXPECT_TRUE(CM_SUCCEEDED(result));
    }

    while (s_counter < TEST_THREAD_COUNT) {
        ::SleepMs(10);
    }
    CQoSBandwidthController *cqbc = dynamic_cast<CQoSBandwidthController *>(iqbc);
    ASSERT_NE(cqbc, (CQoSBandwidthController *)NULL);
    EXPECT_EQ(cqbc->GetServiceNumber(), TEST_THREAD_COUNT);

    for (i = 0; i < TEST_THREAD_COUNT; ++i) {
        threadList[i]->Destroy();
    }
    delete[] threadList;
    EXPECT_EQ(cqbc->GetServiceNumber(), 0);
    iqbc->Destroy();
}

TEST_F(CQoSMultiThreadTest, WQoSMultiThreadTest_Create_Multi_BMController_100)
{
    class CQoSThreadBW : public ACmThread
    {
    public:
        CQoSThreadBW(uint32_t gkey) : m_groupKey(gkey) {}
        virtual void OnThreadRun()
        {
            ACmThread::GetThreadId();
            std::stringstream ss;
            ss << "GKEY_" << m_groupKey;
            std::string strGKey;
            ss >> strGKey;

            uint32_t serviceId = 0;
            uint32_t sourceId = 0;
            IQoSBandwidthController    *qosController;
            CQoSServiceListenerMockMT   serviceLister;
            CAppBandwidthContactMockMT  bandwidthBound;

            QoSGroupKey groupKey((const uint8_t *)strGKey.c_str(), (uint32_t)strGKey.length());
            qosController = WQoSCreateBandwidthController(groupKey);
            if (NULL != qosController) {
                EXPECT_EQ(qosController->Initialize(true, 100), 0);
                int32_t ret = qosController->RegisterService(&bandwidthBound, &serviceLister, serviceId, 1, true);
                EXPECT_EQ(ret, 0);

                NetworkMetricEx nm;
                uint32_t tick = (uint32_t)low_ticker::now()/1000;
                qosController->FeedbackNetworkMetrics(tick, serviceId, nm);
                tick = (uint32_t)low_ticker::now()/1000;
                qosController->FeedbackDataSent(tick, serviceId, sourceId, 1024, false);
                tick = (uint32_t)low_ticker::now()/1000;
                qosController->FeedbackDataSent(tick, serviceId, sourceId, 1024, true);

                qosController->UnregisterService(serviceId);
                qosController->Destroy();
            }

            ++s_counter;
        }

        CmResult Stop(CCmTimeValue *aTimeout = NULL)
        {
            return CM_OK;
        }

        void Destroy()
        {
            this->Stop();
            this->Join();
            this->Destory(CM_OK);
        }

    public:
        uint32_t  m_groupKey;
    };

    uint32_t oldMapSize = CQoSManager::GetInstance()->m_mapBandwidthCtl.size();
    CQoSThreadBW **threadList = new CQoSThreadBW*[TEST_THREAD_COUNT];
    int i = 0;
    s_counter = 0;
    for (i = 0; i < TEST_THREAD_COUNT; ++i) {
        threadList[i] = new CQoSThreadBW(i%10);
        CmResult result = threadList[i]->Create("QoS", TT_CURRENT, TF_JOINABLE, false);
        EXPECT_TRUE(CM_SUCCEEDED(result));
    }

    while (s_counter < TEST_THREAD_COUNT) {
        ::SleepMs(10);
    }
    EXPECT_EQ(CQoSManager::GetInstance()->m_mapBandwidthCtl.size(), oldMapSize);

    for (i = 0; i < TEST_THREAD_COUNT; ++i) {
        threadList[i]->Destroy();
    }
    delete[] threadList;
}

static volatile bool s_start = false;
class CQoSThread1 : public ACmThread
{
public:
    CQoSThread1(IQoSBandwidthController *ctrl) : m_serviceId(0), m_sourceId(0), m_qosController(ctrl) {}
    virtual void OnThreadRun()
    {
        int32_t ret = m_qosController->RegisterService(&m_bandwidthBound, &m_serviceLister, m_serviceId, 1, true);
        EXPECT_TRUE(0 == ret);
        ++s_counter;
        while (!s_start) {
            ::SleepMs(1);
        }
        NetworkMetricEx nm;
        uint32_t tick = (uint32_t)low_ticker::now()/1000;
        m_qosController->FeedbackNetworkMetrics(tick, m_serviceId, nm);
        tick = (uint32_t)low_ticker::now()/1000;
        m_qosController->FeedbackDataSent(tick, m_serviceId, m_sourceId, 1024, false);
        tick = (uint32_t)low_ticker::now()/1000;
        m_qosController->FeedbackDataSent(tick, m_serviceId, m_sourceId, 1024, true);

        ++s_counter;
    }
    CmResult Stop(CCmTimeValue *aTimeout = NULL)
    {
        return CM_OK;
    }
    void UnregisterService()
    {
        m_qosController->UnregisterService(m_serviceId);
    }
    void Destroy()
    {
        m_qosController->UnregisterService(m_serviceId);
        this->Stop();
        this->Join();
        this->Destory(CM_OK);
    }

public:
    uint32_t m_serviceId;
    uint32_t m_sourceId;
    IQoSBandwidthController    *m_qosController;
    CQoSServiceListenerMockMT   m_serviceLister;
    CAppBandwidthContactMockMT  m_bandwidthBound;
};

class CQoSThread2 : public ACmThread
{
public:
    CQoSThread2(IQoSBandwidthController *ctrl) : m_qosController(ctrl) {}
    virtual void OnThreadRun()
    {
        ++s_counter;
        while (!s_start) {
            ::SleepMs(1);
        }
        m_qosController->UnregisterService(m_serviceId);
        ++s_counter;
    }
    CmResult Stop(CCmTimeValue *aTimeout = NULL)
    {
        return CM_OK;
    }
    void Destroy()
    {
        this->Stop();
        this->Join();
        this->Destory(CM_OK);
    }

public:
    uint32_t m_serviceId;
    IQoSBandwidthController    *m_qosController;
};
#ifdef NDEBUG

TEST_F(CQoSMultiThreadTest, WQoSMultiThreadTest_UnregisterService_500)
{
    ASSERT_NE(m_qosController, (IQoSBandwidthController *)NULL);
    CQoSThread1 **threadList = new CQoSThread1*[MAX_THREAD_COUNT];
    int i = 0;
    s_counter = 0;
    for (i = 0; i < MAX_THREAD_COUNT; ++i) {
        threadList[i] = new CQoSThread1(m_qosController);
        CmResult result = threadList[i]->Create("QoS", TT_CURRENT, TF_JOINABLE, false);
        EXPECT_TRUE(CM_SUCCEEDED(result));
    }

    while (s_counter < MAX_THREAD_COUNT) {
        ::SleepMs(10);
    }
    CQoSBandwidthController *qosController = dynamic_cast<CQoSBandwidthController *>(m_qosController);
    EXPECT_EQ(qosController->GetServiceNumber(), MAX_THREAD_COUNT);
    s_counter = 0;
    CQoSThread2 **threadList2 = new CQoSThread2*[MAX_THREAD_COUNT];
    for (i = 0; i < MAX_THREAD_COUNT; ++i) {
        threadList2[i] = new CQoSThread2(m_qosController);
        threadList2[i]->m_serviceId = threadList[i]->m_serviceId;
        CmResult result = threadList2[i]->Create("QoS", TT_CURRENT, TF_JOINABLE, false);
        EXPECT_TRUE(CM_SUCCEEDED(result));
    }
    while (s_counter < MAX_THREAD_COUNT) {
        ::SleepMs(10);
    }
    ::SleepMs(10);
    s_counter = 0;
    s_start = true;
    while (s_counter < 2*MAX_THREAD_COUNT) {
        ::SleepMs(10);
    }
    EXPECT_EQ(qosController->GetServiceNumber(), 0);
    for (i = 0; i < MAX_THREAD_COUNT; ++i) {
        threadList[i]->Destroy();
        threadList2[i]->Destroy();
    }
    delete[] threadList;
    delete[] threadList2;
}
#endif // NDEBUG

static CCmAtomicOperationT<CCmMutexThread> s_sequence = 12578;
class CQoSThreadFBRecv : public ACmThread
{
public:
    CQoSThreadFBRecv(IQoSFeedbackGenerator *fb) : m_fbGenerator(fb) {}
    virtual void OnThreadRun()
    {
        for (int i=0; i < 100; ++i) {
            uint16_t sequence = (++s_sequence)&0xFFFF;

            uint32_t tick = (uint32_t)low_ticker::now()/1000;
            m_fbGenerator->OnDataReceived(tick, 0, 11223344, sequence, 1024, 300);
            ::SleepMs(1);
        }
        ++s_counter;
    }
    CmResult Stop(CCmTimeValue *aTimeout = NULL)
    {
        return CM_OK;
    }
    void Destroy()
    {
        this->Stop();
        this->Join();
        this->Destory(CM_OK);
    }

public:
    IQoSFeedbackGenerator *m_fbGenerator;
};

class CQoSThreadFBGet : public ACmThread
{
public:
    CQoSThreadFBGet(IQoSFeedbackGenerator *fb) : m_fbGenerator(fb) {}
    virtual void OnThreadRun()
    {
        for (int i=0; i < 100; ++i) {
            NetworkMetric networkmetric;
            uint32_t tick = (uint32_t)low_ticker::now()/1000;
            m_fbGenerator->GetNetworkMetricFeedback(tick,networkmetric);
            ::SleepMs(1);
        }
        ++s_counter;
    }
    CmResult Stop(CCmTimeValue *aTimeout = NULL)
    {
        return CM_OK;
    }
    void Destroy()
    {
        this->Stop();
        this->Join();
        this->Destory(CM_OK);
    }

public:
    IQoSFeedbackGenerator *m_fbGenerator;
};

TEST(QoSMultiThreadTest, WQoSMultiThreadTest_FeedbackGenerator_10)
{
    const int thread_num = 10;
    QoSGroupKey groupKey;
    IQoSFeedbackGenerator *fbGenerator = wqos::CreateFeedbackGeneratorEx(groupKey);
    ASSERT_NE(fbGenerator, (IQoSFeedbackGenerator *)NULL);
    CQoSThreadFBRecv **threadList = new CQoSThreadFBRecv*[thread_num];
    int i = 0;
    s_counter = 0;
    for (i = 0; i < thread_num; ++i) {
        threadList[i] = new CQoSThreadFBRecv(fbGenerator);
        CmResult result = threadList[i]->Create("QoSFBR", TT_CURRENT, TF_JOINABLE, false);
        EXPECT_TRUE(CM_SUCCEEDED(result));
    }

    CQoSThreadFBGet **threadList2 = new CQoSThreadFBGet*[thread_num];
    for (i = 0; i < thread_num; ++i) {
        threadList2[i] = new CQoSThreadFBGet(fbGenerator);
        CmResult result = threadList2[i]->Create("QoSFBG", TT_CURRENT, TF_JOINABLE, false);
        EXPECT_TRUE(CM_SUCCEEDED(result));
    }

    while (s_counter < 2*thread_num) {
        ::SleepMs(10);
    }

    for (i = 0; i < thread_num; ++i) {
        threadList[i]->Destroy();
        threadList2[i]->Destroy();
    }
    delete[] threadList;
    delete[] threadList2;

    wqos::DestroyFeedbackGeneratorEx(fbGenerator);
}
