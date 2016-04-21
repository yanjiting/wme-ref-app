//
//  WMEDisplayViewController.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-5-17.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "WMERenderView.h"
#import "MediaClockDrone.h"
#include "ClickCall.h"

class CUIRenderUpdateSink;

@interface WMEDisplayViewController : UIViewController <UIScrollViewDelegate, UITableViewDataSource>
{
    BOOL bBarHiddenFlag;
    BOOL bIsPad;
    BOOL lastDeviceOrientationIsPortrait;
    BOOL bAttendeeRatioIsPortrait;
    BOOL bCameraSwitchFlag;
    BOOL bSpeakerSwitchFlag;
    UIAlertView *disconnectIndication;
    BOOL bRemoteContentShare;
    BOOL bEnableAppshare;
    BOOL bEnableSRTP;
    BOOL bEnableFec;
    NSTimer *updateTableViewSourceTimer;
    NSMutableArray *arrayStats;
    int iVideoWidth;
    int iVideoHeight;
    
    CUIRenderUpdateSink* m_pUISink;
}


@property (strong, nonatomic) IBOutlet UIBarButtonItem *btSendAudio;
@property (strong, nonatomic) IBOutlet UIBarButtonItem *btSetting;
@property (strong, nonatomic) IBOutlet UIBarButtonItem *btSendVideo;
@property (strong, nonatomic) IBOutlet WMERenderView *attendeeView;
@property (strong, nonatomic) IBOutlet WMERenderView *selfView;
@property (strong, nonatomic) IBOutlet WMERenderView *contentView;
@property (strong, nonatomic) IBOutlet UIScrollView *contentScrollView;
@property (strong, nonatomic) IBOutlet UILabel *previewModeLabel;
@property (strong, nonatomic) IBOutlet UILabel *attendeeResolution;
@property (strong, nonatomic) IBOutlet UIImageView *ivCameraSwitch;
@property (strong, nonatomic) IBOutlet UIImageView *ivSpeakerSwitch;
@property (weak, nonatomic) IBOutlet UIButton *btStartServer;
@property (strong, nonatomic) IBOutlet MediaClockDrone *fakeView;
@property (strong, nonatomic) UITableView *statsView;
@property (atomic) BOOL bLoopback;
@property (atomic) BOOL bVideoHW;
@property (atomic) BOOL bCalliope;
@property (retain) NSString *strSdp;
@property (retain) NSString *strLinus;
@property (retain) NSString *strServer;
@property (atomic) BOOL bInit;
@property (atomic) BOOL bTA;
@property (assign) BOOL bEnableAppshare;
@property (assign) BOOL bEnableSRTP;
@property (assign) BOOL bEnableFec;
@property (assign) BOOL bUsingVPIO;
@property (assign) BOOL bUsingTCAEC;
@property (atomic) BOOL bEnableCVO;

- (IBAction)ButtonSendVideo:(id)sender;
- (IBAction)ButtonSendAudio:(id)sender;
- (IBAction)ButtonSwitchSpeaker:(id)sender;
- (IBAction)ButtonStartServer:(id)sender;

//unwind segue
- (IBAction)SetSetting:(UIStoryboardSegue *)segue;

//- (void)incomingCall:(NSNotification *)notification;
//- (void)enableCall:(NSNotification *)notification;

- (void) UpdateUI:(uint32_t)uiWidth height:(uint32_t)uiHeight data:(void *)callee;

@end


// sink between c++ and objc
class CUIRenderUpdateSink : public IUIRenderUpdateSink{
public:
    CUIRenderUpdateSink(WMEDisplayViewController* ctrl) {m_pCtrl = ctrl;}
    virtual ~CUIRenderUpdateSink() {}
    
    virtual void UpdateUI(uint32_t uiWidth, uint32_t uiHeight, void *callee) {
        if (m_pCtrl) {
            [m_pCtrl UpdateUI:uiWidth height:uiHeight data:callee];
        }
    }
    
private:
    WMEDisplayViewController* m_pCtrl;
};


