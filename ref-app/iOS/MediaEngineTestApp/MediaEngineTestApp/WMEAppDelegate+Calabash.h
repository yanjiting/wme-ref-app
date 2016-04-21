//
//  WMEAppDelegate+Calabash.h
//  MediaEngineTestApp
//
//  Created by zhaozhengchu on 6/4/14.
//  Copyright (c) 2014 video. All rights reserved.
//
#import "WMEAppDelegate.h"

@interface UIWindow (TAEnhance)
- (UIViewController *) visibleViewController;
@end

@interface WMEAppDelegate (Calabash)

- (NSString *)getIPAddress;
@end
