#include <stdlib.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "cmdebug.h"

#if defined( ANDROID) || defined(MAC_IOS) || defined(WIN32)
#include "IWbxAeBase.h"
#include "WseTraceSink.h"
#include <string>
#include <sstream>
#include <iostream> 

#if defined(WP8)
using namespace Platform;
using namespace Windows;
using namespace Windows::Storage;
#endif

#endif

//
// Conan(xiasu@cisco.com)
// this macro can enable write trace to logcat.
// #define ANDROID_LOGCAT
#if defined(ANDROID) && defined(ANDROID_LOGCAT)

#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "AudioEngine", __VA_ARGS__);

class TestcasePrinter : public ::testing::EmptyTestEventListener {

    virtual void OnTestCaseStart(const ::testing::TestCase& test_case)
    {
        LOG("!!! TestCase %s starting.\n", test_case.name());
    }

    virtual void OnTestCaseEnd(const ::testing::TestCase& test_case)
    {
        LOG("!!! TestCase %s ending.\n", test_case.name());
    }

    virtual void OnTestStart(const ::testing::TestInfo& test_info)
    {
          LOG("!!! Test %s.%s starting.\n", test_info.test_case_name(), test_info.name());
    }

    virtual void OnTestPartResult(const ::testing::TestPartResult& test_part_result)
    {
        LOG("!!! %s in %s:%d\n%s\n", test_part_result.failed() ? "Failure" : "Success",
                                     test_part_result.file_name(),
                                     test_part_result.line_number(),
                                     test_part_result.summary());
    }

    virtual void OnTestEnd(const ::testing::TestInfo& test_info)
    {
        LOG("!!! Test %s.%s ending.\n", test_info.test_case_name(), test_info.name());
    }
};
#endif

extern "C" void __gcov_flush();
int GetDocumentPath(char *pPath, unsigned long &uPathLen);
int dolphin_unittest_main(const char* path)
{
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

    int argc = 1;
    char *argv[] = {"dolphin_unittest_main"};
    if (NULL == path || strlen(path) <= 0)
        return 1;
    
    char xmlPath[1024] = "";
    sprintf(xmlPath, "xml:%s", path);

    printf("absolute path for XML:%s\n", xmlPath);
	::testing::GTEST_FLAG(output) = xmlPath;
	::testing::InitGoogleTest(&argc, argv);

#if defined(ANDROID) && defined(ANDROID_LOGCAT)
    ::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
    listeners.Append(new TestcasePrinter);
#endif    

	int ret = RUN_ALL_TESTS();

#ifdef ENABLED_GCOV_FLAG
#if defined(MACOS) ||  defined(APP_IOS)
    __gcov_flush();
#endif
#ifdef ANDROID
    exit(0);
#endif
#endif 

#if defined(WP8) && !defined(DEBUG) 
    exit(0);
#endif

    return ret;
}

#if !defined(APP_IOS)
#if defined(MACOS) || defined(WIN32)
int main(int argc, const char * argv[])
{
    if(argc > 1)
    {
        return dolphin_unittest_main(argv[1]);
    }
    else
    {
#if defined(WIN32)
        return dolphin_unittest_main("./wbxaudioengineTest_windows.xml");
#endif

#if defined(MACOS)
		return dolphin_unittest_main("./wbxaudioengineTest_mac.xml");
#endif
    }
}
#endif
#endif
