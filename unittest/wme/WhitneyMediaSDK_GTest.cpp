#ifdef WIN32
#include <stdio.h>
#endif

#include <stdlib.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
extern "C" void __gcov_flush();
int GetDocumentPath(char *pPath, unsigned long &uPathLen);
#include "WmeInterface.h"
class CWmeTestEnvironment : public testing::Environment
{
public:
    virtual void SetUp()
    {
        wme::WmeInit(0);
    }
    virtual void TearDown()
    {
        wme::WmeUninit();
    }
};

int wme_unittest_main(const char* path)
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
    char *argv[] = {"wme_unittest_main"};
    if (NULL == path || strlen(path) <= 0)
        return 1;
    
    char xmlPath[1024] = "";
    sprintf(xmlPath, "xml:%s", path);

    printf("absolute path for XML:%s\n", xmlPath);
	::testing::GTEST_FLAG(output) = xmlPath;
    ::testing::AddGlobalTestEnvironment(new CWmeTestEnvironment());
	::testing::InitGoogleTest(&argc, argv);
    //::testing::FLAGS_gtest_filter = "CWmeLocalVideoExternalTrackTest.GetExternalInputter";
    int ret = RUN_ALL_TESTS();
	//getchar();
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

#if defined(MACOS) || defined(WIN32)
int main(int argc, const char * argv[])
{
	if (getenv("WME_UNITTEST_APP_InitGoogleTest")){
		::testing::AddGlobalTestEnvironment(new CWmeTestEnvironment());
		::testing::InitGoogleTest(&argc,(char**)argv); 
		return RUN_ALL_TESTS();
    }

    if(argc > 1)
    {
        return wme_unittest_main(argv[1]);
    }
    else
    {
#if defined(WIN32)
		return wme_unittest_main("./wmeclientTest_windows.xml");
#endif

#if defined(MACOS)
		return wme_unittest_main("./wmeclientTest_mac.xml");
#endif
	}
}
#endif

