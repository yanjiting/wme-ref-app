//
//  WMEViewController.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-5-17.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <UIKit/UIKit.h>




@interface WMEMainViewController : UIViewController <UITextFieldDelegate, UIAlertViewDelegate>

@property (weak, nonatomic) IBOutlet UIScrollView *scrollView;
@property (weak, nonatomic) IBOutlet UITextField *tfServerIP;
@property (weak, nonatomic) IBOutlet UILabel *lbServerIP;
@property (weak, nonatomic) IBOutlet UIButton *btConnect;
@property (weak, nonatomic) IBOutlet UITextField *tfLinusIP;
@property (weak, nonatomic) IBOutlet UILabel *lbLinusIP;
@property (weak, nonatomic) IBOutlet UITextField *tfStreamsCount;

@property (weak, nonatomic) IBOutlet UIButton *btLocalMode;

@property (nonatomic, retain, readonly) IBOutlet UISegmentedControl *scServerOrClient;
@property (nonatomic, retain, readonly) IBOutlet UISwitch *swCalliope;
@property (nonatomic, retain, readonly) IBOutlet UISwitch *swLoopback;
@property (nonatomic, retain, readonly) IBOutlet UISwitch *swAppshare;
@property (weak, nonatomic) IBOutlet UISwitch *swSRTP;
@property (weak, nonatomic) IBOutlet UISwitch *swMultiStream;
@property (weak, nonatomic) IBOutlet UISwitch *swFec;
@property (weak, nonatomic) IBOutlet UISwitch *swVideoHW;
@property (weak, nonatomic) IBOutlet UISwitch *swVPIOMode;
@property (weak, nonatomic) IBOutlet UISwitch *swRemoteAECMode;
@property (weak, nonatomic) IBOutlet UISwitch *swEnableCVO;
@property (atomic) BOOL bTA;
@property (retain) id json;


- (IBAction)ButtonConnect:(id)sender;


//unwind segue
- (IBAction)StopPlay:(UIStoryboardSegue *)segue;

- (IBAction)ButtonSwitchVPIO;

- (IBAction)ButtonSwitchTCAEC;

@end
