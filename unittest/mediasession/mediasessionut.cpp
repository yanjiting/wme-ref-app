#include "CmBase.h"
#include "mediasessionut.h"
#ifdef WIN32
#include <stdio.h>
#endif

#include <stdlib.h>
#include "gtest/gtest.h"
extern "C" void __gcov_flush();

int mediasession_unittest_main(const char* path, const char* filter = NULL)
{
#ifdef RUN_ON_DEVICE
	setenv("GCOV_PREFIX", "/sdcard/mediasession_gcov/", 1);
	setenv("GCOV_PREFIX_STRIP", "6", 1);
#endif

	int argc = 1;
	char *argv[] = { "mediasession_unittest_main" };
	if (NULL == path || strlen(path) <= 0)
		return 1;

	char xmlPath[1024] = "";
	sprintf(xmlPath, "xml:%s", path);

	printf("absolute path for XML:%s\n", xmlPath);
	::testing::GTEST_FLAG(output) = xmlPath;
	::testing::InitGoogleTest(&argc, argv);
	if (filter != NULL)
		::testing::FLAGS_gtest_filter = filter;
	int ret = RUN_ALL_TESTS();

#ifdef ENABLED_GCOV_FLAG
#ifdef APP_IOS
	__gcov_flush();
#endif
#ifdef ANDROID
	exit(0);
#endif
#endif

	return ret;
}

#if defined(CM_MAC) || defined(WIN32)
int main(int argc, const char * argv[])
{
	if (getenv("WME_UNITTEST_APP_InitGoogleTest")){
		::testing::InitGoogleTest(&argc, (char**)argv);
		return RUN_ALL_TESTS();
	}
#ifdef WIN32
	wme::InitMinidump("Mediasession_ut");
#endif
	if (argc > 1)
	{
		int i = 0;
		bool bWait = false;
		const char* filter = NULL;
		const char* path = "./mediasessionTest_windows.xml";
		for (i = 1; i < argc; i++){
			const char* filter_prefix = "--gtest-filter=";
			if (strstr(argv[i], "--wait") != NULL)
				bWait = true;
            else if (strstr(argv[i], "--trace2file") != NULL) {
                cm_set_trace_option(CmTrace_UserDefined | CmTrace_DefaultFile);
            }
			else if (strstr(argv[i], filter_prefix) != NULL)
				filter = argv[i] + strlen(filter_prefix);
			else if (i == 1){
				path = argv[i];
			}
		}
		int ret = mediasession_unittest_main(path, filter);

		if (bWait)
			getchar();

		return ret;
	}
	else
	{
#if defined(WIN32)
		return mediasession_unittest_main("./mediasessionTest_windows.xml");
#endif

#if defined(CM_MAC)
		return mediasession_unittest_main("./mediasessionTest_mac.xml");
#endif
	}
}
#endif

