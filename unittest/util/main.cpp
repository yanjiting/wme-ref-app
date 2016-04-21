#include <iostream>
#include <stdlib.h>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "CmDebug.h"
#include "CmUtilMisc.h"

extern "C" void __gcov_flush();
int GetDocumentPath(char *pPath, unsigned long &uPathLen);
int util_unittest_main(const char* path, const char* filter = NULL)
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
    const char *argv[] = {"util_unittest_main"};
    if (NULL == path || strlen(path) <= 0)
        return 1;

    char xmlPath[1024] = "";
    sprintf(xmlPath, "xml:%s", path);
    
    printf("absolute path for XML:%s\n", xmlPath);
	::testing::GTEST_FLAG(output) = xmlPath;
    //::testing::FLAGS_gtest_filter="CCmListAllocatorPoolTest.*:CCmListAllocatorTest.*";
    if(filter != NULL) {
        ::testing::FLAGS_gtest_filter = filter;
    }
    
    //::testing::FLAGS_gtest_repeat=10;
	::testing::InitGoogleTest(&argc, (char **)argv);
    RUN_ALL_TESTS();

#ifdef MEMORY_LEAK_PROFILE
#if defined(MACOS) ||  defined(APP_IOS)
    usleep(1000*1000*15);//wait for the instrument doing refresh
#endif
#endif

#ifdef ENABLED_GCOV_FLAG
#if defined(MACOS) ||  defined(APP_IOS)
    __gcov_flush();
#endif
#ifdef ANDROID
    exit(0);
#endif
#endif
 
    return 0;
}

#if defined(MACOS) || defined(WIN32) || defined(CM_LINUX)
static void trace_printf(unsigned long trace_level, char *trace_info, unsigned int len)
{
    printf("%s\n",trace_info);
}

int main(int argc, const char * argv[])
{
    //set_util_external_trace_sink(trace_printf);
    //can use --gtest_filter to debug specific test cases .e.g --gtest_filter=CmLockfree*
#if defined(WP8) || defined(UWP)
    bool bAppInitGoogleTest = false;
    bool bAppGoogleTestDelayExit=false;
#else
    bool bAppInitGoogleTest=NULL!=getenv("WME_UNITTEST_APP_InitGoogleTest");
    bool bAppGoogleTestDelayExit=NULL!=getenv("WME_UNITTEST_APP_GoogleTest_DelayExit");
#endif
    if (bAppInitGoogleTest){
        ::testing::InitGoogleTest(&argc,(char**)argv);
        int ret = RUN_ALL_TESTS();
        if (bAppGoogleTestDelayExit)
            SleepMs(1000 * 10); //delay exit 10s
        return ret;
    }
    
#if defined(WIN32)
    const char *path = "./utilTest_windows.xml";
#elif defined(MACOS)
    const char *path = "./utilTest_mac.xml";
#elif defined(CM_LINUX)
    const char *path = "./utilTest_linux.xml";
#endif

    const char *filter = NULL;
    bool bWait = false;
    if(argc > 1)
    {
        bool bSetPath = false;
        for (int i = 1; i < argc; i++){
            const char* filter_prefix = "--gtest-filter=";
            if (strstr(argv[i], "--wait") != NULL) {
                bWait = true;
            }
            else if (strstr(argv[i], filter_prefix) != NULL) {
                filter = argv[i] + strlen(filter_prefix);
            }
            else if (!bSetPath){
                path = argv[i];
                bSetPath = true;
            }
        }
    }
    
    int retCode = util_unittest_main(path, filter);
    if(bWait) {
        getchar();
    }
    
    return retCode;
}
#endif
