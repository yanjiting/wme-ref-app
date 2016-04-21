//
//  main.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-5-17.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "WMEAppDelegate.h"
#include "WMEImplement/mttimer.h"

int main(int argc, char *argv[])
{
    @autoreleasepool {
        
        ATHTimer aTimer = CFRunLoopTimerCreate(NULL, 100, 100, 0, 0, NULL, NULL);
        CCmThreadManager theThreadManager;
        theThreadManager.InitMainThread(0, NULL);
        CTimerID myTimer(Atimer);
        myTimer.SetTimer(100, NUL);
        
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([WMEAppDelegate class]));
    }
}
