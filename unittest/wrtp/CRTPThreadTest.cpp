#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "wrtpapi.h"
#include "rtpchannel.h"
#include "rtpsessionclient.h"
#include "wrtptrace.h"
#include "testutil.h"

#include "CmThreadMisc.h"

using namespace wrtp;

TEST(RTPThreadTest, Test_smooth_send_thread_is_alive)
{
    CScopedTracer test_info;
    WRTP_INFOTRACE("RTPThreadTest, Test_smooth_send_thread_is_alive, begin");

    // create a Video session
    WRTPSessionParams sessionParams;
    sessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    sessionParams.enableRTCP = true;

    IRTPSessionClient *session = nullptr;
    session =  WRTPCreateRTPSessionClient(sessionParams);
    ASSERT_TRUE(nullptr != session);

    session->IncreaseReference();

    ACmThread *thread = ::GetThread(TT_RTP_SMOOTH_SEND);
    ASSERT_TRUE(nullptr != thread);

    CM_THREAD_ID thread_id = thread->GetThreadId();
    int err = pthread_kill(thread_id, 0);
    ASSERT_TRUE(0 == err);

    session->DecreaseReference();
    WRTP_INFOTRACE("RTPThreadTest, Test_smooth_send_thread_is_alive, end");
}

TEST(RTPThreadTest, Test_smooth_send_thread_is_dead)
{
    CScopedTracer test_info;
    WRTP_INFOTRACE("RTPThreadTest, Test_smooth_send_thread_is_dead, begin");

    // create a Video session
    WRTPSessionParams sessionParams;
    sessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    sessionParams.enableRTCP = true;

    IRTPSessionClient *session = nullptr;
    session =  WRTPCreateRTPSessionClient(sessionParams);
    ASSERT_TRUE(nullptr != session);

    session->IncreaseReference();

    ACmThread *thread = ::GetThread(TT_RTP_SMOOTH_SEND);
    ASSERT_TRUE(nullptr != thread);

    WRTP_INFOTRACE("thread = " << thread);

    CM_THREAD_ID thread_id = thread->GetThreadId();

    WRTP_INFOTRACE("thread_id = " << thread_id);

    volatile int err = pthread_kill(thread_id, 0);
    ASSERT_TRUE(0 == err);

    session->DecreaseReference();

    thread = ::GetThread(TT_RTP_SMOOTH_SEND);
    ASSERT_TRUE(nullptr == thread);
    WRTP_INFOTRACE("thread1 = " << thread);

    err = pthread_kill(thread_id, 0);
    //ASSERT_TRUE(ESRCH == err);
    if (ESRCH != err) {
        WRTP_INFOTRACE("Test_smooth_send_thread_is_dead, thread_id = " << thread_id <<" is reused by system");
    }

    WRTP_INFOTRACE("RTPThreadTest, Test_smooth_send_thread_is_dead, end");
}

TEST(RTPThreadTest, Test_rtcp_report_thread_is_alive)
{
    CScopedTracer test_info;
    WRTP_INFOTRACE("RTPThreadTest, Test_rtcp_report_thread_is_alive, begin");

    WRTPSessionParams sessionParams;
    sessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    sessionParams.enableRTCP = true;

    IRTPSessionClient *session = nullptr;
    session =  WRTPCreateRTPSessionClient(sessionParams);
    ASSERT_TRUE(nullptr != session);

    session->IncreaseReference();

    ACmThread *thread = ::GetThread(TT_RTCP_REPORT);
    ASSERT_TRUE(nullptr != thread);

    CM_THREAD_ID thread_id = thread->GetThreadId();
    int err = pthread_kill(thread_id, 0);
    ASSERT_TRUE(0 == err);

    session->DecreaseReference();
    WRTP_INFOTRACE("RTPThreadTest, Test_rtcp_report_thread_is_alive, end");
}

TEST(RTPThreadTest, Test_rtcp_report_thread_is_dead)
{
    CScopedTracer test_info;
    WRTP_INFOTRACE("RTPThreadTest, Test_rtcp_report_thread_is_dead, begin");

    WRTPSessionParams sessionParams;
    sessionParams.sessionType = RTP_SESSION_WEBEX_VIDEO;
    sessionParams.enableRTCP = true;

    IRTPSessionClient *session = nullptr;
    session =  WRTPCreateRTPSessionClient(sessionParams);
    ASSERT_TRUE(nullptr != session);

    session->IncreaseReference();

    ACmThread *thread = ::GetThread(TT_RTCP_REPORT);
    ASSERT_TRUE(nullptr != thread);

    CM_THREAD_ID thread_id = thread->GetThreadId();
    int err = pthread_kill(thread_id, 0);
    ASSERT_TRUE(0 == err);

    session->DecreaseReference();

    thread = ::GetThread(TT_RTCP_REPORT);
    ASSERT_TRUE(nullptr == thread);

    err = pthread_kill(thread_id, 0);
    //ASSERT_TRUE(ESRCH == err);
    if (ESRCH != err) {
        WRTP_INFOTRACE("Test_rtcp_report_thread_is_dead, thread_id = " << thread_id <<" is reused by system");
    }

    WRTP_INFOTRACE("RTPThreadTest, Test_rtcp_report_thread_is_dead, end");
}
