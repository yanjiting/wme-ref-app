//
//  WMEConnectionViewController.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-7-17.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "DemoParameters.h"
#import "NotificationTransfer.h"
#import "WMEDataProcess.h"

@interface WMEConnectionViewController : UITabBarController <NotificationTranferDelegate>

@property (retain, nonatomic) NotificationTransfer *pNotificationTransfer;
@property (retain, nonatomic) WMEDataProcess *pWMEDataProcess;
@property (atomic)BOOL bVideoConnectionState;
@property (atomic)BOOL bAudioConnectionState;
@property (atomic)BOOL bShareConnectionState;

//unwind segue
- (IBAction)StopPlay:(UIStoryboardSegue *)segue;
@end
