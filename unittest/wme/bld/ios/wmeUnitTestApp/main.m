//
//  main.m
//  wmeUnitTestApp
//
//  Created by hfe-vscm-mac on 4/28/14.
//  Copyright (c) 2014 WME. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "UTAppDelegate.h"
extern int wme_unittest_main(const char* path);

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
    const char* path="/tmp/wme_UT_ios_sim.xml";
#else
    char xmlWritePath[1024] = "";
    unsigned long uPathLen = 1024;
    char path[1024] = "";
    GetDocumentPath(xmlWritePath, uPathLen);
    sprintf(path, "%s%s", xmlWritePath, "/wme_UT_ios_dev.xml");
#endif
    wme_unittest_main(path);
	
    abort();
    @autoreleasepool {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([UTAppDelegate class]));
    }
}
