//
//  main.m
//  sharkUnitTestApp
//
//  Created by hfe-vscm-mac on 4/28/14.
//  Copyright (c) 2014 WME. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "UTAppDelegate.h"
#import "CWseResourceForUT.h"
#include <sys/sysctl.h>
#import <sys/types.h>
extern int shark_unittest_main(const char* path);

void GetResourceData()
{
    NSBundle *applicationBundle = [NSBundle mainBundle];
    NSString *path = [applicationBundle pathForResource:@"720p"
                                                 ofType:@"264"];	//Returns nil if not found
    NSData* myData = [NSData dataWithContentsOfFile:path];
    sResourceInfo.uiBitSreamForDecoderControlLen =  [myData length];
    sResourceInfo.pBitStreamForDecoderControl = [myData bytes];
    
}

int GetDeviceInfo()
{
    size_t size;
    sysctlbyname("hw.machine", NULL, &size, NULL, 0);
    sDeviceInfo.iInfoLen = size-1;
    char *machine = (char*)malloc(size);
    sysctlbyname("hw.machine", machine, &size, NULL, 0);
    memcpy(sDeviceInfo.pDeviceInfo, machine, size);
    free(machine);
    return 0;
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
    const char* path="/tmp/shark_UT_ios_sim.xml";
#else
    char xmlWritePath[1024] = "";
    unsigned long uPathLen = 1024;
    char path[1024] = "";
    GetDocumentPath(xmlWritePath, uPathLen);
    GetResourceData();
    GetDeviceInfo();
    sprintf(path, "%s%s", xmlWritePath, "/shark_UT_ios_dev.xml");
#endif
    shark_unittest_main(path);
	
    abort();
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([UTAppDelegate class]));
    }
}
