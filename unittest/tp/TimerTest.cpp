//
//  TimerTest.cpp
//  tpTest_mac
//
//  Created by Wilson Chen on 12/16/15.
//  Copyright © 2015 cisco. All rights reserved.
//

#include <stdio.h>
#include "gtest/gtest.h"
#include "timer.h"
#include "CmUtilMisc.h"
#include "CmThreadManager.h"
#include "CmTimerWrapperID.h"
#include "CmThreadTask.h"

uint64_t checkTimerDiff() {
    ticker theAcurateTicker;
    low_ticker theTicker;
    
    uint64_t diff = 0;
    for(int i = 0; i < 10; i++) {
        uint64_t a = theAcurateTicker.now();
        uint64_t b = theTicker.now();
        diff +=  b > a ? (b - a) : (a - b);
        SleepMs(5);
    }
    return diff;
}

TEST(TimerTest, TestStartStopLowTicket) {
    startLowTick();
    EXPECT_GT(checkTimerDiff(), 10);
    stopLowTick();
    EXPECT_LT(checkTimerDiff(), 10);
    CCmThreadManager::Instance();
    startLowTick();
    startLowTick();
    EXPECT_GT(checkTimerDiff(), 10);
    stopLowTick();
    stopLowTick();
    CCmThreadManager::CleanupOnlyOne();
    EXPECT_LT(checkTimerDiff(), 10);
}

class CTimerWrapperTest : public testing::Test, public CCmTimerWrapperIDSink
{
public:
    
    class CCancelThread : public ACmThread {
    public:
        CCancelThread(CTimerWrapperTest *pParent, int to) : m_pParent(pParent), m_timeout(to) {}
    protected:
        virtual void OnThreadRun()  {
            CCmTimeValue aWait(0, m_timeout*1000);
            m_pParent->m_waitEvent.Wait(&aWait);
            if(m_pParent->m_timer) {
                CM_INFO_TRACE_THIS("CTimerWrapperTest::Cancel, begin");
                m_pParent->m_timer->Cancel();
                CM_INFO_TRACE_THIS("CTimerWrapperTest::Cancel, end");
            }
        }
        
        CTimerWrapperTest *m_pParent;
        int m_timeout;
    };
    
    virtual void SetUp() {
    }
    
    virtual void TearDown() {
    }
    
    virtual void OnTimer(CCmTimerWrapperID *aId) {
        CM_INFO_TRACE("CTimerWrapperTest::OnTimer, begin");
        SleepMs(200);
        CM_INFO_TRACE("CTimerWrapperTest::OnTimer, end");
        m_waitEvent.Signal();
    }
    
protected:
    CCmTimerWrapperID *m_timer = NULL;
    CCancelThread *m_pThread[2];
    CCmEventThread m_waitEvent;
};

TEST_F(CTimerWrapperTest, CancelInDifferentThread) {
    CCmThreadManager::Instance();
    m_timer = new CCmTimerWrapperID();
    m_timer->ScheduleInThread(TT_MAIN, this, CCmTimeValue(0, 1));
    
    m_pThread[0] = new CCancelThread(this, 20);
    m_pThread[1] = new CCancelThread(this, 190);
    m_pThread[0]->Create("cancel-0", TT_USER_DEFINE_BASE + 17, TF_JOINABLE, FALSE);
    m_pThread[1]->Create("cancel-1", TT_USER_DEFINE_BASE + 19, TF_JOINABLE, FALSE);
    SleepMsWithLoop(10);
}

/*
TEST(SelectError, TestSelectParamterError) {
    timeval tvSelect;
    tvSelect.tv_sec = 0;
    tvSelect.tv_usec = -1;
    int fd = 0;
    fd_set fsRead, fsWrite, fsException;
    int nMaxFd = fd + 1;
    int nSelect = ::select(nMaxFd+1, &fsRead, &fsWrite, &fsException, &tvSelect);
    int nErr = errno;
    EXPECT_EQ(nSelect, -1 && nErr == ETIMEDOUT);
}
 */

class CCmTimerWrapperIDTest : public testing::Test, public CCmTimerWrapperIDSink
{
public:
    CCmTimerWrapperIDTest() : m_pThreadWithoutTimer(this)
    {
    }
    
    virtual ~CCmTimerWrapperIDTest()
    {
    }
    
    virtual void OnTimer(CCmTimerWrapperID *aId) {
        for(int i = 0; i < m_timerCount; i++) {
            if(aId == &m_timer[i]) {
                m_bTimer[i]++;
                break;
            }
        }
    }
    
    virtual void SetUp()
    {
        CCmThreadManager::Instance();
        ::CreateUserTaskThread("t-withtimer", m_pThreadWithTimer, TF_JOINABLE, TRUE, TT_USER_DEFINE_BASE + 1);
    }
    
    virtual void TearDown()
    {
        for(int i = 0; i < m_timerCount; i++) {
            m_timer[i].Cancel();
        }
        if(m_pThreadWithTimer) {
            m_pThreadWithTimer->Stop();
            m_pThreadWithTimer->Join();
            m_pThreadWithTimer->Destory(0);
        }
        
        m_pThreadWithoutTimer.Join();
    }
    
    void ScheduleTest() {
        m_timer[2].ScheduleInThread(m_pThreadWithTimer, this, CCmTimeValue(0, 10000));
        SleepMs(90);
        m_timer[0].Cancel();
    }
    
protected:
    ACmThread *m_pThreadWithTimer = NULL;
    class TestTreadForTimerWrap : public ACmThread {
    public:
        TestTreadForTimerWrap(CCmTimerWrapperIDTest *pThis) {
            m_pThis = pThis;
        }
        virtual void OnThreadRun() {
            m_pThis->ScheduleTest();
        }
        
        CCmTimerWrapperIDTest *m_pThis = NULL;
    };
    TestTreadForTimerWrap m_pThreadWithoutTimer;
    static const int m_timerCount = 3;
    CCmTimerWrapperID m_timer[m_timerCount];
    int m_bTimer[m_timerCount] = {0};
};

TEST_F(CCmTimerWrapperIDTest, TestScheduleCancel) {
    m_pThreadWithoutTimer.Create("t-notimer", TT_USER_DEFINE_BASE + 2, TF_JOINABLE, FALSE);
    m_timer[0].Schedule(this, CCmTimeValue(0, 10000));
    m_timer[1].ScheduleInThread(m_pThreadWithTimer ,this, CCmTimeValue(0, 10000));
    SleepMsWithLoop(100);
    EXPECT_TRUE(m_bTimer[0] > 0);
    EXPECT_TRUE(m_bTimer[1] > 0);
    EXPECT_TRUE(m_bTimer[2] > 0);
}

TEST_F(CCmTimerWrapperIDTest, TestExceptionalCase) {
    CmResult rv = m_timer[0].Cancel();
    EXPECT_TRUE(CM_ERROR_NOT_FOUND == rv);
    m_timer[1].ScheduleInThread(m_pThreadWithTimer ,this, CCmTimeValue(0, 10000));
    m_timer[1].Cancel();
    SleepMsWithLoop(50);
    EXPECT_TRUE(m_bTimer[1] < 2);
}
