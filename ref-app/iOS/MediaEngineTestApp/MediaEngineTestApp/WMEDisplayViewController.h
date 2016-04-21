//
//  WMEDisplayViewController.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-5-17.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "WMERenderView.h"
#import "DemoParameters.h"
#import "WMEDataProcess.h"
#import "NotificationTransfer.h"


@interface WMEDisplayViewController : UIViewController<NotificationTranferDelegate>{
    BOOL bBarHiddenFlag;
    BOOL bIsPad;
    BOOL lastDeviceOrientationIsPortrait;
    BOOL bAttendeeRatioIsPortrait;
    BOOL bCameraSwitchFlag;
    BOOL bSpeakerSwitchFlag;
    UIAlertView *disconnectIndication;
    BOOL bSpeakerMuteFlag;
    BOOL bMicMuteFlag;
    BOOL bRemoteContentShare;
    CGSize _szContent;
    BOOL bContentPresenter;
    BOOL bVideo;
}
@property (strong, nonatomic) UIView *maskView;

@property (retain, nonatomic) WMEDataProcess *pWMEDataProcess;
@property (strong, nonatomic) IBOutlet UIControl *mainView;
@property (strong, nonatomic) IBOutlet UIButton *btSendAudio;
@property (strong, nonatomic) IBOutlet UIButton *btSendVideo;
@property (strong, nonatomic) IBOutlet UIButton *btContentShare;

@property (strong, nonatomic) IBOutlet WMERenderView *attendeeView;
@property (strong, nonatomic) IBOutlet WMERenderView *selfView;
@property (strong, nonatomic) IBOutlet WMERenderView *contentView;
@property (strong, nonatomic) IBOutlet UILabel *previewModeLabel;
@property (strong, nonatomic) IBOutlet UILabel *attendeeResolution;
@property (strong, nonatomic) IBOutlet UIImageView *ivCameraSwitch;
@property (strong, nonatomic) IBOutlet UIImageView *ivSpeakerSwitch;
@property (strong, nonatomic) IBOutlet UIView *vMoreButtonView;
@property (strong, nonatomic) IBOutlet UIButton *btMueSpeaker;
@property (strong, nonatomic) IBOutlet UIButton *btMueMic;
@property (weak, nonatomic) IBOutlet UIBarButtonItem *btSetting;

- (IBAction)ButtonSendVideo:(id)sender;
- (IBAction)ButtonSendAudio:(id)sender;
- (IBAction)ButtonSwitchSpeaker:(id)sender;
- (IBAction)ButtonMore:(id)sender;
- (IBAction)ButtonContentShare:(id)sender;
- (IBAction)TapBlankPlace:(id)sender;

- (IBAction)SetMicMute:(id)sender;
- (IBAction)SetSpeakerMute:(id)sender;

//unwind segue
- (IBAction)SetSetting:(UIStoryboardSegue *)segue;

#ifdef TA_ENABLE
//for TA testing
@property (retain) NSTimer *updateVoiceLevelTimer;
@property (retain) UILabel *voiceLevelLabel;
- (void)TAStartShowVoiceLevel:(NSNotification*)notification;
- (void)TAStopShowVoiceLevel:(NSNotification*)notification;
#endif
@end
