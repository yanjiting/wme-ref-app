//
//  main.m
//  appshareUnitTest
//
//  Created by Aulion Huang on 5/9/14.
//  Copyright (c) 2014 webex. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "AppDelegate.h"

extern int unittest_main(const char* path);
void __gcov_flush();

void gtest_entry()
{
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString* document = [paths objectAtIndex:0];
#ifdef IOS_SIMULATOR
    const char* path="/tmp/appshare_UT_ios_sim.xml";
#else
    NSString* xmlPath = [document stringByAppendingPathComponent:@"/appshare_UT_ios_dev.xml"];
    
    char path[1024] = "";
    strcpy(path, [xmlPath UTF8String]);
#endif

#ifdef RUN_ON_DEVICE
#ifdef APP_IOS
    char gcdaFolder[1024] = "";
    strcpy(gcdaFolder, [document UTF8String]);
    setenv("GCOV_PREFIX", gcdaFolder, 1);
    setenv("GCOV_PREFIX_STRIP", "1", 1);
#endif
#endif
    unittest_main(path);
	
#ifdef ENABLED_GCOV_FLAG
    __gcov_flush();
#endif

#ifdef GTEST // if GTEST == 1, do test only, exit app after all test case done
#if GTEST
    exit(0);
#endif
#endif
}

int main(int argc, char * argv[])
{
//#ifdef GTEST
//    gtest_entry();
//#endif
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}


