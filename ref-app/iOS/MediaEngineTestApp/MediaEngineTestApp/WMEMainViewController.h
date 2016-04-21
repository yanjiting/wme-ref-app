//
//  WMEViewController.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-5-17.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "mttimer.h"
#import "DemoParameters.h"
#import "WMEDataProcess.h"
#import "WMEPreMeetingSettingViewController.h"
@interface WMEMainViewController : UIViewController <UITextFieldDelegate, UIAlertViewDelegate, PreMeetingSettingViewControllerDelegate>

@property (retain, nonatomic) WMEDataProcess *pWMEDataProcess;
@property (retain, nonatomic)UIAlertView *statusIndication;
@property (weak, nonatomic) IBOutlet UITextField *tfServerIP;
@property (weak, nonatomic) IBOutlet UILabel *lbServerIP;
@property (weak, nonatomic) IBOutlet UIButton *btConnect;
@property (weak, nonatomic) IBOutlet UIButton *btStartServer;
@property (weak, nonatomic) IBOutlet UIButton *btLocalMode;

@property (nonatomic, retain, readonly) IBOutlet UISegmentedControl *scServerOrClient;
@property CTimerID m_Timer;

- (IBAction)ButtonConnect:(id)sender;
- (IBAction)ButtonStartServer:(id)sender;
- (IBAction)TapBlankPlace:(id)sender;




- (void)showAlertWindowTitle:(NSString*)title message:(NSString*)message;
- (NSString *)getIPAddress;
- (void)networkDisconnect;
- (void)networkConnect;
@end
