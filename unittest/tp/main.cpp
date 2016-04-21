#include <stdlib.h>
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <iostream>
extern "C" void __gcov_flush();

std::string g_sGTestFilter;

int GetDocumentPath(char *pPath, unsigned long &uPathLen);
int tp_unittest_main(const char* path)
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
    const char *argv[] = {"tp_unittest_main"};
    if (NULL == path || strlen(path) <= 0)
        return 1;

    char xmlPath[1024] = "";
    sprintf(xmlPath, "xml:%s", path);

    printf("absolute path for XML:%s\n", xmlPath);
	::testing::GTEST_FLAG(output) = xmlPath;
	::testing::InitGoogleTest(&argc, (char **)argv);
    if(!g_sGTestFilter.empty()) {
        ::testing::FLAGS_gtest_filter = g_sGTestFilter.c_str();
    }
    int ret = RUN_ALL_TESTS();
    
#ifdef ENABLED_GCOV_FLAG
#if defined(MACOS) ||  defined(APP_IOS)
    __gcov_flush();
#endif
#ifdef ANDROID
    exit(0);
#endif
#endif
    return ret;
}
#if !defined(APP_IOS)
#if defined(MACOS) || defined(WIN32) || defined(CM_LINUX)
#include "CmDebug.h"
static void mytrace(unsigned long trace_level, char *trace_info, unsigned int len)
{
	printf("%s\n",trace_info);
}

int main(int argc, const char * argv[])
{
#if defined(WIN32)
    const char *path = "./tpTest_windows.xml";
#elif defined(MACOS)
    const char *path = "./tpTest_mac.xml";
#elif defined(CM_LINUX)
    const char *path = "./tpTest_linux.xml";
#else
    const char *path = "./tpTest_unknown.xml";
#endif
    bool bPause = false;
    
	//set_util_external_trace_sink(mytrace);
    if(argc > 1)
    {
        bool bSetPath = false;
        for (int i = 1; i < argc; i++){
            const char* filter_prefix = "--gtest-filter=";
            if (strstr(argv[i], "--wait") != NULL) {
                bPause = true;
            } else if (strstr(argv[i], "--trace2file") != NULL) {
                cm_set_trace_option(CmTrace_UserDefined | CmTrace_DefaultFile);
            }
            else if (strstr(argv[i], filter_prefix) != NULL) {
                g_sGTestFilter = argv[i] + strlen(filter_prefix);
            }
            else if (!bSetPath){
                path = argv[i];
                bSetPath = true;
            }
        }
    }
	int ret = tp_unittest_main(path);
    if(bPause)
        getchar();
    
    return ret;
}
#endif
#endif
