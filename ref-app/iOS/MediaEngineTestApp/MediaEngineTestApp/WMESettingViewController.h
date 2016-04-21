//
//  WMESettingViewController.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-7-14.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "WMEDataProcess.h"

//for table view section index
typedef enum
{
    SECTION_CAPABILITY = 0,
    SECTION_STATISTICS,
    SECTION_PORTRAIT
}SECTION_TYPE;
//for capability secton row index
typedef enum
{
    CAPABILITY_VIDEO_ENCODER = 0,
    CAPABILITY_AUDIO_ENCODER,
    CAPABILITY_VIDEO_CAPTURE
}CAPABILITY_TYPE;

//for statistics secton row index
typedef enum
{
    STATISTICS_VIDEO_NETWORK = 0,
    STATISTICS_AUDIO_NETWORK,
    STATISTICS_VIDEO_TRACK,
    STATISTICS_AUDIO_TRACK,
}STATISTICS_TYPE;

//for portrait secton row index
typedef enum
{
    PORTRAIT_VIEW_ROTATION = 0,
}PORTRAIT_TYPE;

@interface WMESettingViewController : UITableViewController
{
    NSInteger selectedRow;
    NSTimer *updateStatisticsTimer;
    //BOOL bPortraitViewRotationEnable;
}

@property (retain, nonatomic) WMEDataProcess *pWMEDataProcess;
@property (nonatomic)  BOOL bSendVideoState;
@property (nonatomic)  NSInteger selectedRow;
@property (nonatomic)  NSInteger selectedSection;
@property (retain)  NSTimer *updateStatisticsTimer;
@property (retain)  NSMutableArray *arrayStatistics;
@property (retain)  NSMutableArray *arrayStatisticsTitle;

@property (nonatomic) NSUInteger selectedVideoCapabilitySetting;
@property (nonatomic) NSUInteger selectedAudioCapabilitySetting;
@property (nonatomic) NSUInteger selectedCameraCapabilitySetting;
@property (nonatomic) BOOL bKeepAspectRatio;


- (IBAction)RotationEnableSwitch:(id)sender;
@property (weak, nonatomic) IBOutlet UISwitch *portraitRotationEnableSWT;
//unwind segue
- (IBAction)GetSetting:(UIStoryboardSegue *)segue;
@end
