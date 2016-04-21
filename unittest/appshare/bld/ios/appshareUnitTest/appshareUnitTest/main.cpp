#include "gtest/gtest.h"
#include "gmock/gmock.h"

extern "C" int unittest_main(const char *path);

int unittest_main(const char *path)
{
    int argc = 1;
    char *argv[] = {"appshare_unittest_main"};
    if (NULL == path || strlen(path) <= 0) {
        return 1;
    }

    char xmlPath[1024] = "";
    sprintf(xmlPath, "xml:%s", path);

    printf("absolute path for XML:%s\n", xmlPath);
    ::testing::GTEST_FLAG(output) = xmlPath;
    ::testing::InitGoogleTest(&argc, argv);
    RUN_ALL_TESTS();

    return 0;
}

#if !defined(APP_IOS)
#if defined(MACOS)
int main(int argc, const char *argv[])
{
    if (argc > 1) {
        return unittest_main(argv[1]);
    } else {
        return unittest_main("./wbxaudioengineTest_mac.xml");
    }
}
#endif
#endif