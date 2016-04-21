//
//  WMEPreMeetingSettingViewController.h
//  MediaEngineTestApp
//
//  Created by rory on 1/6/14.
//  Copyright (c) 2014 video. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "WMERawFilesViewController.h"

@protocol PreMeetingSettingViewControllerDelegate <NSObject>
- (void)DidFinishPreMeetingSetting;
@end

@interface WMEPreMeetingSettingViewController : UITableViewController<SelectRawFile>
@property (weak, nonatomic) IBOutlet UILabel *videoFileName;
@property (weak, nonatomic) IBOutlet UILabel *audioFileName;
@property (weak, nonatomic) IBOutlet UISwitch *videoSwitch;
@property (weak, nonatomic) IBOutlet UISwitch *audioSwitch;
@property (weak, nonatomic) IBOutlet UISwitch *contentShareSwitch;
@property (weak, nonatomic) IBOutlet UITextField *outputVideoFileName;
@property (weak, nonatomic) IBOutlet UITextField *outputAudioFileName;
@property (weak, nonatomic) IBOutlet UIBarButtonItem *doneBarBtn;
@property (weak, nonatomic) id<PreMeetingSettingViewControllerDelegate>delegate;
@property (strong, nonatomic) IBOutlet UISwitch *dumpCaptureDataSWT;
@property (strong, nonatomic) IBOutlet UISwitch *dumpEncodeToRTPDataSWT;
@property (strong, nonatomic) IBOutlet UISwitch *dumpNALToListenChannelDataSWT;
@property (strong, nonatomic) IBOutlet UISwitch *dumpNALToDecodeDataSWT;
@property (strong, nonatomic) IBOutlet UISwitch *dumpAfterDecodeDataSWT;

@end
