//
//  WMEAppDelegate.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-5-17.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <UIKit/UIKit.h>
#include "BackdoorAgent.h"




@interface WMEAppDelegate : UIResponder <UIApplicationDelegate>
{
    CBackdoorAgent * backdoorAgent;
}

@property (retain) NSUserDefaults* userDefaults;
@property (strong, nonatomic) UIWindow *window;
//@property (retain, nonatomic) UIWindow *window;

@end
