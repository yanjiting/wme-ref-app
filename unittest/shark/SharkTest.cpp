// SharkTest.cpp : Defines the entry point for the console application.
//
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "WseEngine.h"

#if defined(WIN32) && !defined(WP8)
HINSTANCE ghInstance = NULL;
#endif

int main(int argc, char* argv[])
{
	::testing::GTEST_FLAG(output) = "xml:./SharkSDKTest.xml";

	//::testing::FLAGS_gtest_filter = "CWseVideoListenChannelTest.RecvNALData_AysncMode";
	
	::testing::InitGoogleTest(&argc, argv);

	int ret = RUN_ALL_TESTS();

#ifdef WIN32
	if (argc > 2)
		getchar();
#endif
	return ret;
//	system("pause");
}

