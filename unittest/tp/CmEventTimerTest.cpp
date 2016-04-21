
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "CmError.h"
#include "CmThread.h"
#include "CmThreadManager.h"
#include "CmEventQueueBase.h"

#include "CmBoshDataBuild.h"
#include "CmEventTimer.h"


class MockICmEventTimerSink : public ICmEventTimerSink 
{
public:
    MOCK_METHOD1(OnEvent,
        void(UINT32 u32EventID));
};


class CCmEventTimerTest : 
    public testing::Test
    , public ICmEventTimerSink
{
public:
    CCmEventTimerTest()
    {
        m_pEventTimer = NULL;
    }

    virtual ~CCmEventTimerTest()
    {
        if(NULL != m_pEventTimer)
        {
            delete m_pEventTimer;
            m_pEventTimer = NULL;
        }
    }

    virtual void SetUp()
    {
        CCmThreadManager* pThreadManage = CCmThreadManager::Instance();
        if(NULL != pThreadManage)
        {
            delete pThreadManage;
            pThreadManage = NULL;
        }

        m_bExistEventQueue = FALSE;
        if(NULL != m_pEventTimer)
        {
            delete m_pEventTimer;
            m_pEventTimer = NULL;
        }
    }

    virtual void TearDown()
    {
        CCmThreadManager* pThreadManage = CCmThreadManager::Instance();
        if(NULL != pThreadManage)
        {
            delete pThreadManage;
            pThreadManage = NULL;
        }
    }

public:
    virtual void OnEvent(
        UINT32 u32EventID)
    {
        if(13 == u32EventID)
        {
            ACmThread* pThread = CCmThreadManager::Instance()->GetThread(TT_MAIN);
            if(NULL != pThread)
            {
                pThread->Stop();
                m_bExistEventQueue = TRUE;
            }

        }
    }

protected:
    BOOL                    m_bExistEventQueue;
    CCmEventTimer*          m_pEventTimer;
    CCmTimeValue            m_TimeValue;
    MockICmEventTimerSink   m_EventTimerSink;
};


TEST_F(CCmEventTimerTest, ScheduleTestInvalidInput)
{
    ASSERT_TRUE(NULL == m_pEventTimer);

    m_pEventTimer = new CCmEventTimer(NULL);
    ASSERT_TRUE(NULL != m_pEventTimer);

    m_TimeValue.SetByTotalMsec(10);
    EXPECT_TRUE(CM_OK != m_pEventTimer->Schedule(10, m_TimeValue));
    EXPECT_TRUE(CM_OK != m_pEventTimer->Schedule(10, m_TimeValue, 10));

    EXPECT_TRUE(CM_OK != m_pEventTimer->Cancel(10));
}

TEST_F(CCmEventTimerTest, ScheduleTestNormalInput)
{

	//make current thread a TP thread
	CCmThreadManager::Instance();

    ASSERT_TRUE(NULL == m_pEventTimer);

    m_pEventTimer = new CCmEventTimer(&m_EventTimerSink);
    ASSERT_TRUE(NULL != m_pEventTimer);

    m_TimeValue.SetByTotalMsec(10);
    EXPECT_TRUE(CM_OK == m_pEventTimer->Schedule(10, m_TimeValue));
    EXPECT_TRUE(CM_OK != m_pEventTimer->Schedule(10, m_TimeValue, 10));

    EXPECT_TRUE(CM_OK == m_pEventTimer->Cancel(10));
    EXPECT_TRUE(CM_OK != m_pEventTimer->Cancel(10));
}

TEST_F(CCmEventTimerTest, CancelAllTestWithItems)
{
    ASSERT_TRUE(NULL == m_pEventTimer);

    ACmThread* pThread = CCmThreadManager::Instance()->GetThread(TT_MAIN);
    ASSERT_TRUE(NULL != pThread);

    ICmEventTimerSink* pTimerSink = dynamic_cast<ICmEventTimerSink*>(this);
    m_pEventTimer = new CCmEventTimer(pTimerSink);
    ASSERT_TRUE(NULL != m_pEventTimer);

    m_TimeValue.SetByTotalMsec(10);
    EXPECT_TRUE(CM_OK == m_pEventTimer->Schedule(10, m_TimeValue));
    EXPECT_TRUE(CM_OK == m_pEventTimer->Schedule(11, m_TimeValue, 10));
    EXPECT_TRUE(CM_OK == m_pEventTimer->Schedule(12, m_TimeValue, 10));
    EXPECT_TRUE(CM_OK == m_pEventTimer->Schedule(13, m_TimeValue, 10));


#ifndef WIN32
    while(!m_bExistEventQueue)
#endif
    {
        pThread->OnThreadRun();
    }
    EXPECT_TRUE(CM_OK == m_pEventTimer->CancelAll());
}

TEST_F(CCmEventTimerTest, CancelAllTestWithoutItems)
{
    ASSERT_TRUE(NULL == m_pEventTimer);

    m_pEventTimer = new CCmEventTimer(&m_EventTimerSink);
    ASSERT_TRUE(NULL != m_pEventTimer);

    EXPECT_TRUE(CM_OK == m_pEventTimer->CancelAll());
}
