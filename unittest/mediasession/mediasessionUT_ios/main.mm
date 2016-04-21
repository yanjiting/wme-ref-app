//
//  main.m
//  mediasessionUT_ios
//
//  Created by Soya Li on 13-6-5.
//  Copyright (c) 2013年 cisco. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "AppDelegate.h"
#include <stdlib.h>
#include "gtest/gtest.h"
#include "gmock/gmock.h"

extern "C" void __gcov_flush();
int GetDocumentPath(char *pPath, unsigned long &uPathLen);

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
	::testing::InitGoogleTest(&argc, argv);
    //::testing::FLAGS_gtest_filter = "MediaConnectionTest.TestGetVideoCapability";
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

int GetDocumentPath(char *pPath, unsigned long &uPathLen)
{
    if(NULL == pPath || 0 == uPathLen)
    {
        return 1;
    }
    
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    if([paths count] == 0)
    {
        return 2;
    }
    
    NSString* document = [paths objectAtIndex:0];
    
    NSString* escapedPath = [document stringByReplacingOccurrencesOfString:@" " withString:@"\\ "];
    
    unsigned long uDocumentPathLen = [escapedPath length];
    uPathLen = (uDocumentPathLen <= uPathLen) ? uDocumentPathLen : uPathLen;
    memcpy(pPath, [escapedPath UTF8String], uPathLen);
    
    return 0;
}
int main(int argc, char * argv[])
{
    //Call the UT
#ifdef IOS_SIMULATOR
    const char* path="/tmp/mediasession_UT_ios_sim.xml";
#else
    char xmlWritePath[1024] = "";
    unsigned long uPathLen = 1024;
    char path[1024] = "";
    GetDocumentPath(xmlWritePath, uPathLen);
    sprintf(path, "%s%s", xmlWritePath, "/mediasession_UT_ios_dev.xml");
#endif
    wme_unittest_main(path);
	
    abort();
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}
