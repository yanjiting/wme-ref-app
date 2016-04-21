//
//  WMEICEConnectionViewController.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-7-17.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "WMEDataProcess.h"



@interface WMEICEConnectionViewController : UIViewController <UITextFieldDelegate>

@property (retain, nonatomic) WMEDataProcess *pWMEDataProcess;
@property (retain, nonatomic)UIAlertView *statusIndication;
@property (weak, nonatomic) IBOutlet UILabel *lbToID;
@property (weak, nonatomic) IBOutlet UITextField *tfFromID;
@property (weak, nonatomic) IBOutlet UITextField *tfToID;
@property (weak, nonatomic) IBOutlet UITextField *tfSTUNServerIP;
@property (weak, nonatomic) IBOutlet UITextField *tfJINGLEServerIP;
@property (weak, nonatomic) IBOutlet UITextField *tfSTUNServerPort;
@property (weak, nonatomic) IBOutlet UITextField *tfJINGLEServerPort;
@property (nonatomic, retain, readonly) IBOutlet UISegmentedControl *scServerOrClient;

- (IBAction)ButtonConnect:(id)sender;

- (void)networkDisconnect;
- (void)networkConnect;

@end
