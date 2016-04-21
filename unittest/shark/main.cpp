#include <stdlib.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WseEngine.h"

#if defined( ANDROID) || defined(MAC_IOS) || defined(WIN32)
#include "WseTraceSink.h"
#include <string>
#include <sstream>
#include <iostream>

#if defined(WP8)
#include "WseCodecTrace.h"
using namespace Platform;
using namespace Windows;
using namespace Windows::Storage;
#endif
    
#endif

#include "WseTaskBase.h"

extern "C" void __gcov_flush();

int GetDocumentPath(char *pPath, unsigned long &uPathLen);
int shark_unittest_main(const char* path)
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
    char* argv[] = {"shark_unittest_main"};

    if (NULL == path || strlen(path) <= 0)
        return 1;
    
    char xmlPath[1024] = "";
    sprintf(xmlPath, "xml:%s", path);

    printf("absolute path for XML:%s\n", xmlPath);
	::testing::GTEST_FLAG(output) = xmlPath;
    ::testing::InitGoogleTest(&argc, argv);

    RUN_ALL_TESTS();
    
#if defined(WP8)
	shark::WseSetWseCodecTraceSink(NULL);
#endif
    
#ifdef ENABLED_GCOV_FLAG
#if defined(MACOS) ||  defined(APP_IOS)
    __gcov_flush();
#endif
#ifdef ANDROID
    exit(0);
#endif
#endif

#ifdef WP8
    exit(0);
#endif

    return 0;
}

#if defined(MACOS)
int main(int argc, const char * argv[])
{
    //can use --gtest_filter to debug specific test cases .e.g --gtest_filter=CWseEncodeParamDSNewTest
    if(getenv("WME_UNITTEST_APP_InitGoogleTest")){
        ::testing::InitGoogleTest(&argc,(char**)argv);
        return RUN_ALL_TESTS();
    }
    
    if(argc > 1)
    {
        return shark_unittest_main(argv[1]);
    }
    else
    {
        return shark_unittest_main("./wseclientTest_mac.xml");
    }
}
#endif
