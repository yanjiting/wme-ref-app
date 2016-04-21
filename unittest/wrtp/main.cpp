#include <stdlib.h>
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <iostream>
#include "CmAtomicOperationT.h"
#include "wrtpcommonapi.h"

#include "CmThreadInterface.h"
#include "CmThread.h"
#include "timer.h"

#include "testutil.h"

#include "CmThreadManager.h"

extern int GetDocumentPath(char *pPath, unsigned long &uPathLen);
extern "C" void __gcov_flush();

class CLowTickEvent : public ICmEvent
{
public:
    // functions from ICmEvent
    virtual CmResult OnEventFire()
    {
        //low_tick_generator is an instance obj
        //and call its now has no side affect across multithread
        low_ticker dump;
        return CM_OK;
    }
};

class LowerTickUpdater
{
public:
    LowerTickUpdater()
    {
        m_pThread = nullptr;
    }
    virtual ~LowerTickUpdater()
    {
        Stop();
    }

    void Start()
    {
        CmResult ret = CM_OK;
        ret = ::CreateUserTaskThread("LowerTickUpdater", m_pThread);
        CM_ASSERTE(ret==CM_OK);
        CM_ASSERTE(m_pThread);

        //use send to enforce low_ticker scheduled in user task thread
        m_pThread->GetEventQueue()->SendEvent(new CLowTickEvent());
    }

    void Stop()
    {
        if (m_pThread) {
            m_pThread->Stop();
            m_pThread->Join();
            m_pThread->Destory(CM_OK);
            m_pThread = nullptr;
        }
    }

private:
    ACmThread *m_pThread;
};

//#define PRINT_SRTP_LOG

#ifdef PRINT_SRTP_LOG
#include "srtp.h"
static int OutputSRTPLog(err_reporting_level_t log_level, char *log_msg)
{
    printf("%s\n", log_msg);

    return 0;
}
#endif // ~PRINT_SRTP_LOG

#if defined(MACOS)
    //#define REDIRECT_TRACE
#endif
#if defined(REDIRECT_TRACE)
class WRTPUTTracer : public  wrtp::IWrtpTraceSink
{
public:
    WRTPUTTracer(char *directory, char *platform) : m_file(nullptr), m_mutex()
    {
        char filename[256] = {};
        snprintf(filename, sizeof(filename), "%s/wrtp_ut_trace_%s-%s.log", directory, platform, GetTimestamp());
        m_file = fopen(filename, "a");
        if (!m_file) {
            abort();
        }
    }

    ~WRTPUTTracer()
    {
        fclose(m_file);
        m_file = nullptr;
    }

    virtual long SinkTrace(unsigned long trace_level, const char *trace_info, int len)
    {
        RecursiveMutexGuard g(m_mutex);
        fprintf(m_file, "%s: %2lu %s\n", GetTimestamp(), trace_level, trace_info);
        fflush(m_file);
        return 0;
    }

private:
    const char *GetTimestamp()
    {
        time_t rawtime;
        struct tm *tmval;

        time(&rawtime);
        tmval = localtime(&rawtime);
        mktime(tmval);

        static char timestamp[256] = {};
        snprintf(timestamp, sizeof(timestamp), "%04u-%02u%02u-%02u%02u%02u", 1900+tmval->tm_year, 1+tmval->tm_mon, tmval->tm_mday, tmval->tm_hour, tmval->tm_min, tmval->tm_sec);

        return timestamp;
    }

private:
    FILE *m_file;
    RecursiveMutex m_mutex;

};
#endif

#ifdef ENABLE_PERFORMENCE_TEST
#include "RtpPerfTest.cpp"
#endif
int wrtp_unittest_main(const char *path, int argc, const char *argv[])
{
#if defined(REDIRECT_TRACE)
#if defined(ANDROID)
    WRTPUTTracer tracer("/sdcard", "android");
    wrtp::SetWRTPExternalTraceSink(&tracer);
    wrtp::SetWRTPExternalTraceMask(50);
#elif defined(CM_MACOS)
    WRTPUTTracer tracer("/tmp", "mac");
    wrtp::SetWRTPExternalTraceSink(&tracer);
    wrtp::SetWRTPExternalTraceMask(50);
#elif defined(CM_IOS)
#ifdef IOS_SIMULATOR
    WRTPUTTracer tracer("/tmp", "ios_sim");
#else
    char xmlWritePath[1024] = "";
    unsigned long uPathLen = 1024;
    char patht[1024] = "";
    GetDocumentPath(xmlWritePath, uPathLen);
    WRTPUTTracer tracer(xmlWritePath, "ios_dev");
#endif
    wrtp::SetWRTPExternalTraceSink(&tracer);
    wrtp::SetWRTPExternalTraceMask(50);
#endif
#endif//~ifdef (REDIRECT_TRACE)

#ifdef RUN_ON_DEVICE
#ifdef APP_IOS
    char gcdaFolder[1024] = "";
    unsigned long uPathLen = 1024;
    GetDocumentPath(gcdaFolder, uPathLen);
    setenv("GCOV_PREFIX", gcdaFolder, 1);
    setenv("GCOV_PREFIX_STRIP", "1", 1);
#endif
#ifdef ANDROID
    setenv("GCOV_PREFIX", "/sdcard/wme_gcov/", 1);
    setenv("GCOV_PREFIX_STRIP", "6", 1);
#endif
#endif
    //int argc = 1;
    //const char *argv[] = {"wrtp_unittest_main"};
    //if (nullptr == path || strlen(path) <= 0)
    //    return 1;

    char xmlPath[1024] = "";
    if (nullptr == path || strlen(path) <= 0) {
        sprintf(xmlPath, "xml:");
    } else {
        sprintf(xmlPath, "xml:%s", path);
    }

    startLowTick();
    ::setCreateUserTaskThread(CreateFakeThread);

    ACmThread *thread;
    ::CreateUserTaskThread("main", thread, TF_JOINABLE, true, TT_MAIN);
    if (thread) {
        thread->OnThreadInit();
    }

    int32_t ret = wrtp::WRTPInit();
    EXPECT_TRUE(ret == wrtp::WRTP_ERR_NOERR);

#ifdef PRINT_SRTP_LOG
    set_srtp_log_func(OutputSRTPLog);
#endif // ~PRINT_SRTP_LOG

    printf("absolute path for XML:%s\n", xmlPath);
    ::testing::GTEST_FLAG(output) = xmlPath;
    ::testing::InitGoogleTest(&argc, (char **)(argv));

    //LowerTickUpdater lowerTickUpdater;
    //lowerTickUpdater.Start();

    int retTest = 0;
#ifdef ENABLE_PERFORMENCE_TEST
    RunRtpPerfTest();
#else
    //::testing::FLAGS_gtest_filter = "*CSendControlAPITest*";
    retTest = RUN_ALL_TESTS();
#endif

#ifdef ENABLED_GCOV_FLAG
#if defined(MACOS) ||  defined(APP_IOS)
    __gcov_flush();
#endif
#ifdef ANDROID
    exit(0);
#endif
#endif

    wrtp::SetWRTPExternalTraceSink(nullptr);
    ret = wrtp::WRTPUninit();
    EXPECT_TRUE(ret == wrtp::WRTP_ERR_NOERR);
    return retTest;
}

int wrtp_unittest_main(const char *path)
{
    int argc = 0;
    const char *argv[] = {nullptr};
    return wrtp_unittest_main(path, argc, argv);
}

#if !defined(APP_IOS)
#if defined(MACOS) || defined(WIN32)
int main(int argc, const char *argv[])
{

    if (argc > 1) {
        return wrtp_unittest_main(argv[1], argc, argv);
    } else {
#if defined(WIN32)
        return wrtp_unittest_main("./wrtpTest_windows.xml", argc, argv);
#endif

#if defined(MACOS)
        return wrtp_unittest_main("./wrtpTest_mac.xml", argc, argv);
#endif
    }

}
#endif
#endif
