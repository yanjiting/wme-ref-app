#if defined(MACOS)
    #include <stdio.h>
#elif defined(WIN32)
    #include "appshare_unittest.h"
#endif

extern "C" void __gcov_flush();

#include <stdlib.h>
#include "gtest/gtest.h"
int appshare_unittest(int argc, char *argv[])
{
#ifdef RUN_ON_DEVICE
    setenv("GCOV_PREFIX", "/sdcard/wme_gcov/", 1);
    setenv("GCOV_PREFIX_STRIP", "6", 1);
#endif

    ::testing::InitGoogleTest(&argc,argv);
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

#if defined(MACOS) ||  defined(WIN32)
int main(int argc, char *argv[])
{
#if defined(WIN32)
    CoInitializeEx(0, COINIT_MULTITHREADED);
#endif
    if (argc < 2) {
        ::testing::GTEST_FLAG(output) = "xml:./appshare_unittest.xml";
    }
    int ret = appshare_unittest(argc,argv);
    return ret;
}
#endif

int appshare_unittest_main(const char *path)
{
    if (path) {
        char xmlPath[1024] = "";
        sprintf(xmlPath, "xml:%s", path);
        printf("absolute path for XML:%s\n", xmlPath);
        ::testing::GTEST_FLAG(output) = xmlPath;
    }
    std::vector<char *> args;
    /*
    std::istringstream iss(path);
    std::string token;
    while(iss >> token) {
    int nBufLen = token.size() + 1;
    char *arg = new char[nBufLen];
    memset(arg,0,nBufLen);
    std::copy(token.begin(), token.end(), arg);
    arg[token.size()] = '\0';
    args.push_back(arg);
    }
    */
    int argc = args.size();
    appshare_unittest(argc,&args[0]);

    for (size_t i = 0; i < args.size(); i++) {
        delete[] args[i];
    }
    return 0;//return appshare_unittest(argc,argv);
}
