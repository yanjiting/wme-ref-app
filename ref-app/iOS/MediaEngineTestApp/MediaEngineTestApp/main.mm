//
//  main.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-5-17.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "WMEAppDelegate.h"
#include "mttimer.h"

int main(int argc, char *argv[])
{
    @autoreleasepool {
        
        CCmThreadManager theThreadManager;
        theThreadManager.InitMainThread(0, NULL);
        CTimerID myTimer(0);
        myTimer.SetTimer(20, NULL);
        
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([WMEAppDelegate class]));
    }
}
