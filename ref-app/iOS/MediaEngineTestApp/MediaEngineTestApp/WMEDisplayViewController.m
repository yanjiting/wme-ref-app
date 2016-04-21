//
//  WMEDisplayViewController.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-5-17.
//  Copyright (c) 2013年 video. All rights reserved.
//
#import "WMEDisplayViewController.h"
#import "WMESettingViewController.h"
#import "WMEDataProcess.h"
#import "WMEDeviceStatus.h"

@interface WMEDisplayViewController ()
@property (nonatomic) BOOL bShowMoreViewFlag;
@end

@implementation WMEDisplayViewController

@synthesize btSendVideo = _btSendVideo;
@synthesize btSendAudio = _btSendAudio;
@synthesize attendeeView = _attendeeView;
@synthesize selfView     = _selfView;
@synthesize previewModeLabel = _previewModeLabel;
@synthesize btMueSpeaker = _btMueSpeaker;
@synthesize btMueMic = _btMueMic;
@synthesize maskView = _maskView;


- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    
#if __IPHONE_OS_VERSION_MAX_ALLOWED >= 70000
    //exclude the top/bottom bar
    if ([self respondsToSelector:@selector(setEdgesForExtendedLayout:)]) {
        self.edgesForExtendedLayout = UIRectEdgeNone;
    }
#endif
    //Initialize the WME data processing
    self.pWMEDataProcess = [WMEDataProcess instance];
    
#ifdef TA_ENABLE
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(TAStartShowVoiceLevel:) name:@"NOTIFICATION_AVSYNCSTART" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(TAStopShowVoiceLevel:) name:@"NOTIFICATION_AVSYNCSTOP" object:nil];
#endif
    
    //Initial the last orientation flag
    lastDeviceOrientationIsPortrait = FALSE;
    bCameraSwitchFlag = YES;
    bSpeakerSwitchFlag = YES;
    //Check the device type
    bIsPad  = TRUE;
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone) {
        bIsPad  = FALSE;
    }

    //Hide the top bar
    bBarHiddenFlag = TRUE;
    [super.navigationController setNavigationBarHidden:bBarHiddenFlag animated:TRUE];
    [super.navigationController setToolbarHidden:bBarHiddenFlag animated:TRUE];
    
    disconnectIndication = [[UIAlertView alloc] initWithTitle: @"Disconnect" message: @"Network disconnect!" delegate: self cancelButtonTitle: @"OK" otherButtonTitles: nil];

    //query the default data
    [self.pWMEDataProcess queryAudioInDevices];
    [self.pWMEDataProcess queryAudioOutDevices];
    [self.pWMEDataProcess queryAudioCapabilities];
    
    [self.pWMEDataProcess queryVideoInDevices];
    [self.pWMEDataProcess queryVideoCapabilities];
    
    
	// set the initial state
    self.pWMEDataProcess.bVideoSending = YES;
    self.pWMEDataProcess.bAudioSending = YES;
    self.pWMEDataProcess.bKeepAspectRatio = YES;
    self.pWMEDataProcess.bDumpCaptureDataEnable = NO;
    self.pWMEDataProcess.bDumpEncodeToRTPDataEnable = NO;
    self.pWMEDataProcess.bDumpNALToListenChannelDataEnable = NO;
    self.pWMEDataProcess.bDumpNALToDecodeDataEnable = NO;
    self.pWMEDataProcess.bDumpAfterDecodeDataEnable = NO;
    
    // initial the video parameters
    [self.pWMEDataProcess setVideoQualityType];
    //[self.pWMEDataProcess setRenderAdaptiveAspectRatio:self.pWMEDataProcess.bKeepAspectRatio];
    [self.pWMEDataProcess setVideoEncodingParam:self.pWMEDataProcess.videoCapIndex];
    [self.pWMEDataProcess setVideoCameraDevice:self.pWMEDataProcess.cameraIndex];
    [self.pWMEDataProcess setVideoCameraParam:self.pWMEDataProcess.cameraCapIndex];

    // initial the audio parameters
    [self.pWMEDataProcess setAudioSpeaker:self.pWMEDataProcess.speakerIndex];
    [self.pWMEDataProcess setAudioEncodingParam:self.pWMEDataProcess.audioCapIndex];
    
    /// default start video preview and recving
    [self.pWMEDataProcess startVideoClient:WME_SENDING];
    [self.pWMEDataProcess startVideoClient:WME_RECVING];
    
    /// default start audio sending & recvinv
    [self.pWMEDataProcess startAudioClient:WME_SENDING];
    [self.pWMEDataProcess startAudioClient:WME_RECVING];
    
    self.contentView.hidden = YES;
    //Add the observer to notification centre
    [[NotificationTransfer instance] addNotificationObserver:self];
    
    [self.attendeeView setTranslatesAutoresizingMaskIntoConstraints:YES];
    [self.selfView setTranslatesAutoresizingMaskIntoConstraints:YES];
    
    UITapGestureRecognizer *singleOneForCameraSwitch = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(switchCamera:)];
    singleOneForCameraSwitch.numberOfTapsRequired = 1;
    singleOneForCameraSwitch.numberOfTouchesRequired = 1;
    [self.ivCameraSwitch addGestureRecognizer:singleOneForCameraSwitch];
    [self.ivCameraSwitch setUserInteractionEnabled:YES];

    UITapGestureRecognizer *singleOneForBar = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(changeBarViewState:)];
    singleOneForBar.numberOfTapsRequired = 1;
    singleOneForBar.numberOfTouchesRequired = 1;
    [self.view addGestureRecognizer:singleOneForBar];

    bSpeakerMuteFlag = FALSE;
    bMicMuteFlag = FALSE;
    
    //Draw tool bar
    [self.vMoreButtonView setTranslatesAutoresizingMaskIntoConstraints:YES];
    self.bShowMoreViewFlag = NO;
    [self closeMoreButtonView];

    self.maskView = [[UIView alloc] init];
    self.maskView.backgroundColor = [UIColor redColor];
    self.maskView.alpha = 0;
    [self.view addSubview:self.maskView];
}

- (void)viewDidAppear:(BOOL)animated          
{
    //Set the notification for device orientation change
    [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];

    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    
    //Add the notifier for application switch state between active and in-active
    [center addObserver:self selector:@selector(applicationWillResignActive) name:UIApplicationWillResignActiveNotification object:nil];
    [center addObserver:self selector:@selector(applicationDidBecomeActive) name:UIApplicationDidBecomeActiveNotification object:nil];
    
    //Add the notifier for orientation changed
    [center addObserver:self selector:@selector(orientationChanged:) name:UIDeviceOrientationDidChangeNotification object:nil];
    
    //Arrange the render window for relative device orientation
    UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
    if (UIInterfaceOrientationIsPortrait(orientation)) {
        [self updateRenderUIForPortrait];
        lastDeviceOrientationIsPortrait = TRUE;
    }
    else if (UIInterfaceOrientationIsLandscape(orientation))
    {
        [self updateRenderUIForLandscape];
        lastDeviceOrientationIsPortrait = FALSE;
    }
    else
    {
        NSLog(@"failed to get initial orientation");
    }

    if (self.pWMEDataProcess.bVideoSending == YES) {
        [_btSendVideo setTitle:@"Pause Video" forState:UIControlStateNormal];
        [_previewModeLabel setHidden:TRUE];
    }
    else
    {
        [_btSendVideo setTitle:@"Start Video" forState:UIControlStateNormal];
        //[_btSendVideo.titleLabel setTextColor:[UIColor redColor]];
        [_previewModeLabel setHidden:FALSE];
    }
    
    /// remove previous render
    [self.pWMEDataProcess setRemoteRender: NULL];
    [self.pWMEDataProcess setLocalRender:NULL];
    
    // add the render window
    [self.pWMEDataProcess setRemoteRender:(void *)_attendeeView];
    [self.pWMEDataProcess setLocalRender:(void *)_selfView];
    [self.pWMEDataProcess setAppShareRender:_contentView];
    // disable the screen timeout
    [UIApplication sharedApplication].idleTimerDisabled = YES;
}

- (void)viewDidDisappear:(BOOL)animated
{
    //End the notification for device orientation changed
    [[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];
    
    //Remove the observer
    NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
    [center removeObserver:self name:UIApplicationWillResignActiveNotification object:nil];
    [center removeObserver:self name:UIApplicationDidBecomeActiveNotification object:nil];
    [center removeObserver:self name:UIDeviceOrientationDidChangeNotification object:nil];
    
    //Remove the render window
    [self.pWMEDataProcess setRemoteRender:NULL];
    [self.pWMEDataProcess setLocalRender:NULL];
    [self.pWMEDataProcess setAppShareRender:nil];
    
    //restore the screen timeout setting
    [UIApplication sharedApplication].idleTimerDisabled = NO;
}


- (void)applicationWillResignActive
{
    //Remove the render window
    [self.pWMEDataProcess setLocalRender:NULL];
    [self.pWMEDataProcess setRemoteRender:NULL];
    [self.pWMEDataProcess setAppShareRender:nil];
}

- (void)applicationDidBecomeActive
{
    //Arrange the render window for relative device orientation
    UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
    if (UIInterfaceOrientationIsPortrait(orientation)) {
        [self updateRenderUIForPortrait];
        lastDeviceOrientationIsPortrait = TRUE;
    }
    else if (UIInterfaceOrientationIsLandscape(orientation))
    {
        [self updateRenderUIForLandscape];
        lastDeviceOrientationIsPortrait = FALSE;
    }
    else
    {
        NSLog(@"failed to get initial orientation");
    }

    //Add the render window
    if (self.pWMEDataProcess.bVideoSending == YES) {
        [_btSendVideo setTitle:@"Pause Video" forState:UIControlStateNormal];
        [_previewModeLabel setHidden:TRUE];
    }
    else
    {
        [_btSendVideo setTitle:@"Start Video" forState:UIControlStateNormal];
        //[_btSendVideo.titleLabel setTextColor:[UIColor redColor]];
        [_previewModeLabel setHidden:FALSE];
    }
    
    [self.pWMEDataProcess setRemoteRender:(__bridge void *)_attendeeView];
    [self.pWMEDataProcess setLocalRender:(__bridge void *)_selfView];
    [self.pWMEDataProcess setAppShareRender:_contentView];
}

- (void)orientationChanged:(NSNotification *)notification
{
    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    
    if ((UIDeviceOrientationIsPortrait(orientation)) && (!lastDeviceOrientationIsPortrait))
    {
        [self updateRenderUIForPortrait];
        lastDeviceOrientationIsPortrait = TRUE;
    }
    else if((UIDeviceOrientationIsLandscape(orientation))&& (lastDeviceOrientationIsPortrait))
    {
        [self updateRenderUIForLandscape];
        lastDeviceOrientationIsPortrait = FALSE;
    }
    else
    {
        return;
    }
    
    /// remove previous render
    [self.pWMEDataProcess setRemoteRender: NULL];
    [self.pWMEDataProcess setLocalRender:NULL];

    /// set the latest render
    [self.pWMEDataProcess setRemoteRender:(__bridge void *)_attendeeView];
    [self.pWMEDataProcess setLocalRender:(__bridge void *)_selfView];
}

//update the UI for orientation changed
- (void)updateRenderUIForLandscape
{
    float selfWidth,selfHeight, selfPositionX, selfPositionY,
    attendeeWidth,attendeeHeight, attendeePositionX, attendeePositionY;
    
    float viewWidth = self.view.frame.size.width;
    float viewHeight = self.view.frame.size.height;
    float gapH;
    float gapV;

    if( bRemoteContentShare )
    {
        viewHeight /= 2.;
        viewWidth /= 2.;
    }
    
    if (bIsPad == TRUE) {
        //for iPad landscape
        gapH = 20;
        gapV = 20;
    }
    else
    {
        //for iphone landscape
        gapH = 10;
        gapV = 10;
    }

    if ((viewWidth/16) > (viewHeight/9)) {
        attendeeHeight = viewHeight-(gapV*2);
        attendeeWidth = (attendeeHeight/9.0)*16.0;
        //self-window
        selfHeight = (viewHeight/4) - gapV;
        selfWidth = (selfHeight/9.0)*16.0;
    }
    else
    {
        attendeeWidth = viewWidth-(gapH*2);
        attendeeHeight = (attendeeWidth/16.0)*9.0;
        //self-window
        selfWidth = (viewWidth/4) - gapH;
        selfHeight = (selfWidth/16.0)*9.0;
    }
    
    if (bAttendeeRatioIsPortrait == YES) {
        attendeeHeight = viewHeight-(gapV*2);
        attendeeWidth = (attendeeHeight/16.0)*9.0;
    }
    attendeePositionX = (viewWidth - attendeeWidth)/2;
    attendeePositionY = (viewHeight -attendeeHeight)/2;
    selfPositionX = viewWidth -selfWidth -gapH;
    selfPositionY = viewHeight - selfHeight - gapV;

    [_selfView setFrame:CGRectMake(selfPositionX, selfPositionY, selfWidth, selfHeight)];
    [_attendeeView setFrame:CGRectMake(attendeePositionX, attendeePositionY, attendeeWidth,attendeeHeight)];
    [_maskView setFrame:CGRectMake(attendeePositionX, attendeePositionY, attendeeWidth,attendeeHeight)];
    
}

- (void)updateRenderUIForPortrait
{
    float selfWidth,selfHeight, selfPositionX, selfPositionY,
    attendeeWidth,attendeeHeight, attendeePositionX, attendeePositionY;
    float viewWidth = self.view.frame.size.width;
    float viewHeight = self.view.frame.size.height;
    float gapH;
    float gapV;
    
    
    if (bIsPad == TRUE) {
        //for iPad portrait
        gapH = 20;
        gapV = 20;
    }
    else
    {
        //for iphone portrait
        gapH = 10;
        gapV = 10;
    }

    if( bRemoteContentShare )
        viewHeight /= 2.;
    
    if ((viewWidth/9) > (viewHeight/16)) {
        //attendee-window
        attendeeHeight = viewHeight-(gapV*2);
        attendeeWidth = (attendeeHeight/16.0)*9.0;

        //self-window
        selfHeight  = (viewHeight/4) - gapV;
        selfWidth = (selfHeight/16.0)*9.0;
    }
    else
    {
        //attendee-window
        attendeeWidth = viewWidth-(gapH*2);
        attendeeHeight = (attendeeWidth/9.0)*16.0;

        //self-window
        selfWidth  = (viewWidth/4) - gapH;
        selfHeight = (selfWidth/9.0)*16.0;
    }
    
    if (bAttendeeRatioIsPortrait == NO)
    {
        attendeeWidth  = viewWidth - (gapH*2);
        attendeeHeight = (attendeeWidth/16.0)*9.0;
    }
        
    if (self.pWMEDataProcess.bKeepAspectRatio == NO)
    {
        selfWidth  = (viewWidth/2) - gapH;
        selfHeight = (selfWidth/16.0)*9.0;
    }
    attendeePositionX = (viewWidth - attendeeWidth)/2;
    attendeePositionY = (viewHeight -attendeeHeight)/2;
    selfPositionX = viewWidth -selfWidth -gapH;
    selfPositionY = viewHeight - selfHeight - gapV;
    
    [_selfView setFrame:CGRectMake(selfPositionX, selfPositionY, selfWidth, selfHeight)];
    [_attendeeView setFrame:CGRectMake(attendeePositionX, attendeePositionY, attendeeWidth,attendeeHeight)];
    [_maskView setFrame:CGRectMake(attendeePositionX, attendeePositionY, attendeeWidth,attendeeHeight)];
}


- (void)networkDisconnect:(DEMO_MEDIA_TYPE) eType
{
    [disconnectIndication show];
//    [[NotificationTransfer instance] removeNotificationObserver:self];
    
    if (self.pWMEDataProcess.bVideoSending == YES) {
        [self ButtonSendVideo:_btSendVideo];
    }
}
- (void)OnDecodeSizeChanged:(uint32_t)uLabel width:(uint32_t)uWidth height:(uint32_t)uHeight
{
    if( kRemoteScreenShareTrack_Label == uLabel || kLocalScreenShareTrack_Label == uLabel )
    {
        if( kRemoteScreenShareTrack_Label == uLabel ) {
            bRemoteContentShare = YES;
            self.contentView.hidden = NO;
        }
    }
    else
        bVideo = YES;
    
    NSString *strResolution = [NSString stringWithFormat:@"%dX%d", uWidth, uHeight];
    
    self.attendeeResolution.text = strResolution;
    
    if (uWidth>uHeight) {
        bAttendeeRatioIsPortrait = NO;
    }
    else
    {
        bAttendeeRatioIsPortrait = YES;
    }

    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    
    if( kRemoteScreenShareTrack_Label == uLabel || kLocalScreenShareTrack_Label == uLabel )
    {
        _szContent = CGSizeMake(uWidth, uHeight);
    }
    else {
        self.pWMEDataProcess.recvWidth = uWidth;
        self.pWMEDataProcess.recvHight = uHeight;
    }
    
    if (UIDeviceOrientationIsPortrait(orientation))
    {
        [self updateRenderUIForPortrait];

    }
    else if(UIDeviceOrientationIsLandscape(orientation))
    {
        [self updateRenderUIForLandscape];

    }
    /// remove previous render
    [self.pWMEDataProcess setRemoteRender: NULL];
    [self.pWMEDataProcess setLocalRender:NULL];
    
    /// set the latest render
    [self.pWMEDataProcess setRemoteRender:(__bridge void *)_attendeeView];
    [self.pWMEDataProcess setLocalRender:(__bridge void *)_selfView];
    
}

- (void)OnEndOfStream:(DEMO_MEDIA_TYPE)mtype
{
    [self performSelectorOnMainThread:@selector(backToParent) withObject:nil waitUntilDone:NO];
}


- (void)backToParent
{
    //sleep(1);
    //dismiss the alert window
    UIAlertView *topAlertController = [NSClassFromString(@"_UIAlertManager") performSelector:@selector(topMostAlert)];
    if (topAlertController.visible == YES)
    {
        [topAlertController dismissWithClickedButtonIndex:0 animated:YES];
    }
    
    //Back to the connection window
    [self performSegueWithIdentifier:@"unwindSegueStopPlay" sender:self];
}

- (void)OnDeviceChanged:(DeviceProperty *)pDP event:(wme::WmeEventDataDeviceChanged &)event
{
    //TBD...
    //UI processing
    if (event.iType == WmeDefaultDeviceChanged) {
        if (strcmp(event.szUniqueName, "BuiltInSpeaker") == 0) {
            [self.ivSpeakerSwitch setHidden:YES];
        }
        else{
            [self.ivSpeakerSwitch setHidden:NO];
        }
    }
    else if (event.iType == WmeDeviceInterruptionBegin)
    {
        [self.pWMEDataProcess stopAudioClient:WME_SENDING];
        [self.pWMEDataProcess stopAudioClient:WME_RECVING];
    }
    else if (event.iType == WmeDeviceInterruptionEnd)
    {
        [self.pWMEDataProcess startAudioClient:WME_SENDING];
        [self.pWMEDataProcess startAudioClient:WME_RECVING];
    }
}
- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)ButtonSendVideo:(id)sender
{
    //stop current self-view render window
    [self.pWMEDataProcess setLocalRender:NULL];
    [self.pWMEDataProcess stopVideoClient:WME_SENDING];
    
    if (self.pWMEDataProcess.bVideoSending == NO) {
        self.pWMEDataProcess.bVideoSending = YES;
        //update UI
        [_btSendVideo setTitle:@"Pause Video" forState:UIControlStateNormal];
        [_previewModeLabel setHidden:TRUE];
    }
    else{
        self.pWMEDataProcess.bVideoSending = NO;
        //Update UI
        [_btSendVideo setTitle:@"Start Video" forState:UIControlStateNormal];
        [_previewModeLabel setHidden:FALSE];

    }
    //start video client using new sending state
    [self.pWMEDataProcess setLocalRender:(void *)_selfView];
    [self.pWMEDataProcess startVideoClient:WME_SENDING];

}
- (IBAction)ButtonSendAudio:(id)sender
{
    if (self.pWMEDataProcess.bAudioSending == NO) {
        self.pWMEDataProcess.bAudioSending = YES;
        //enable audio
        [self.pWMEDataProcess startAudioClient:WME_SENDING];
        [_btSendAudio setTitle:@"Pause Audio" forState:UIControlStateNormal];
    }
    else{
        self.pWMEDataProcess.bAudioSending = NO;
        //disable send audio
        [self.pWMEDataProcess stopAudioClient:WME_SENDING];
        [_btSendAudio setTitle:@"Start Audio" forState:UIControlStateNormal];
    }
}

- (IBAction)ButtonContentShare:(id)sender
{
    if( _pWMEDataProcess.isHost )
    {
        self.pWMEDataProcess.bContentSharing = !self.pWMEDataProcess.bContentSharing;
        if( self.pWMEDataProcess.bContentSharing )
        {
            [self.pWMEDataProcess startContentShareClient];
            [_btContentShare setTitle:@"Stop Share" forState:UIControlStateNormal];
        }
        else {
            [self.pWMEDataProcess stopContentShareClient];
            [_btContentShare setTitle:@"Start Share" forState:UIControlStateNormal];
        }
    }
}

- (IBAction)TapBlankPlace:(id)sender {
#ifdef CUCUMBER_ENABLE
    [self changeBarViewState:(UITapGestureRecognizer *)sender];
#endif
}

//
- (IBAction)SetMicMute:(id)sender
{
    if (bMicMuteFlag)//unmute
    {
        [_btMueMic setTitle:@"Mute Audio Mic" forState:UIControlStateNormal];
        bMicMuteFlag = FALSE;
        [self.pWMEDataProcess setMicMute:NO];
    }
    else
    {
        [_btMueMic setTitle:@"Unmute Audio Mic" forState:UIControlStateNormal];
        bMicMuteFlag = TRUE;
        [self.pWMEDataProcess setMicMute:YES];
    }
}
- (IBAction)SetSpeakerMute:(id)sender
{
    if (bSpeakerMuteFlag)//unmute
    {
        [_btMueSpeaker setTitle:@"Mute Audio Speaker" forState:UIControlStateNormal];
        bSpeakerMuteFlag = FALSE;
        [self.pWMEDataProcess setSpeakerMute:NO];
    }
    else
    {
        [_btMueSpeaker setTitle:@"Unmute Audio Speaker" forState:UIControlStateNormal];
        bSpeakerMuteFlag = TRUE;
        [self.pWMEDataProcess setSpeakerMute:YES];
    }
}

- (void)switchCamera:(UITapGestureRecognizer *)sender
{
    [self.pWMEDataProcess switchCameraDevice];
}

- (IBAction)ButtonSwitchSpeaker:(id)sender {
    [self.pWMEDataProcess switchAudioSpeaker];
}


- (IBAction)ButtonMore:(id)sender {
    [self navPlusBtnPressed:sender];
}

-(void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{

}

//unwind segue
- (IBAction)SetSetting:(UIStoryboardSegue *)segue
{
    
    if ([[segue identifier] isEqualToString:@"settingUnwindSegue"]) {
        WMESettingViewController *settingViewController = [segue sourceViewController];
        self.pWMEDataProcess.bKeepAspectRatio = settingViewController.bKeepAspectRatio;
        MessageBody* msgBody = [[MessageBody alloc] initwithRequestType:URLRequestStartCall WithParam:nil];
        [[WMEDeviceStatusClient instance] postUrlMessage:msgBody];
    }
}
- (void)changeBarViewState:(UITapGestureRecognizer *)sender
{
  	bBarHiddenFlag=!bBarHiddenFlag;
	[super.navigationController setNavigationBarHidden:bBarHiddenFlag animated:TRUE];
    [super.navigationController setToolbarHidden:bBarHiddenFlag animated:TRUE];
    if (self.bShowMoreViewFlag == YES) {
        self.bShowMoreViewFlag = NO;
        [self closeMoreButtonView];
    }
    //Update the view size
    UIDeviceOrientation orientation = [[UIDevice currentDevice] orientation];
    
    if (UIDeviceOrientationIsPortrait(orientation))
    {
        [self updateRenderUIForPortrait];
        
    }
    else if(UIDeviceOrientationIsLandscape(orientation))
    {
        [self updateRenderUIForLandscape];
        
    }
    /// remove previous render
    [self.pWMEDataProcess setRemoteRender: NULL];
    [self.pWMEDataProcess setLocalRender:NULL];
    
    /// set the latest render
    [self.pWMEDataProcess setRemoteRender:(__bridge void *)_attendeeView];
    [self.pWMEDataProcess setLocalRender:(__bridge void *)_selfView];
}


#pragma mark -
#pragma mark TA CASE
#ifdef TA_ENABLE
//for TA testing
- (void)TAStartShowVoiceLevel:(NSNotification*)notification
{
    NSInteger interval = [[notification.userInfo objectForKey:@"interval"] integerValue];
    
    self.voiceLevelLabel = [[UILabel alloc] initWithFrame:CGRectMake(10, 0, 180, 50)];
    [_attendeeView addSubview:self.voiceLevelLabel];
    self.voiceLevelLabel.textColor = [UIColor greenColor];
    self.voiceLevelLabel.backgroundColor = [UIColor clearColor];
    [self.voiceLevelLabel setFont:[UIFont systemFontOfSize:25.0]];
    
    unsigned int level;
    [self.pWMEDataProcess getVoiceLevel:level];
    NSString *stringText = [[NSString alloc] initWithFormat:@"VoiceLevel:%d", level];
    self.voiceLevelLabel.text = stringText;
    
    self.updateVoiceLevelTimer = [NSTimer scheduledTimerWithTimeInterval:interval target:self selector:@selector(updateVoiceLevel) userInfo:nil repeats:YES];
}

- (void)TAStopShowVoiceLevel:(NSNotification*)notification
{
    [self.updateVoiceLevelTimer invalidate];
    [self.voiceLevelLabel removeFromSuperview];
}

- (void)updateVoiceLevel
{
    unsigned int level;
    [self.pWMEDataProcess getVoiceLevel:level];
    
    NSString *stringText = [[NSString alloc] initWithFormat:@"VoiceLevel:%d", level];
    [self performSelectorOnMainThread:@selector(redrawUIforVoiceLevelLabel:) withObject:stringText waitUntilDone:NO];
    
}
- (void)redrawUIforVoiceLevelLabel:(NSString *)stringText
{    
    self.voiceLevelLabel.text = stringText;
    [self.voiceLevelLabel setNeedsDisplay];
}
#endif


- (void)navPlusBtnPressed:(UIButton *)sender
{
    if (self.bShowMoreViewFlag == NO)
    {
        self.bShowMoreViewFlag = YES;
        [self showMoreButtonView];
    }
    else
    {
        self.bShowMoreViewFlag = NO;
        [self closeMoreButtonView];
    }
    
}

- (void)showMoreButtonView
{
    NSUInteger superHeight =  self.view.frame.size.height;
    NSUInteger superWidth =  self.view.frame.size.width;
    
    NSUInteger moreViewHeight = self.vMoreButtonView.frame.size.height;
    NSUInteger moreViewWidth = self.vMoreButtonView.frame.size.width;
    
    [self.vMoreButtonView setCenter:CGPointMake( superWidth - (moreViewWidth / 2), superHeight- (moreViewHeight / 2) )];
}

- (void)closeMoreButtonView
{
    NSUInteger superHeight =  self.view.frame.size.height;
    NSUInteger moreViewHeight = self.vMoreButtonView.frame.size.height;
    NSUInteger moreViewWidth = self.vMoreButtonView.frame.size.width;
    
    [self.vMoreButtonView setCenter:CGPointMake( -(moreViewWidth / 2), superHeight- (moreViewHeight / 2) )];
}

@end
