//
//  WMEAppDelegate.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-5-17.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "WMEDataProcess.h"
#ifdef TA_ENABLE
#import "TAManager.h"
#endif
@interface WMEAppDelegate : UIResponder <UIApplicationDelegate>
#ifdef TA_ENABLE
@property (nonatomic) TAManager* pTAManager;
#endif
@property (retain, nonatomic) WMEDataProcess *pWMEDataProcess;
@property (retain) NSUserDefaults* userDefaults;
@property (strong, nonatomic) UIWindow *window;
//@property (retain, nonatomic) UIWindow *window;
#ifdef CUCUMBER_ENABLE
@property (retain, nonatomic) NSString *extendParameter;
#endif
@end
