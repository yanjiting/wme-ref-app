//
//  ios_ut_main.cpp
//  tp_ut_ios
//
//  Created by cisco mmp on 13-6-5.
//  Copyright (c) 2013年 cisco mmp. All rights reserved.
//

#include "ios_ut_main.h"

GTEST_API_ int ios_gtest_main()
{
    int argc = 1;
    const char *program = "ios_gtest_main";
    char **argv = (char **)&program;
    
    //printf();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}