//
//  WMEDisplayViewController.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-5-17.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import "WMEDisplayViewController.h"
#import "WMESettingViewController.h"
#import "PeerCall.h"
#import "Loopback.h"
#include "testconfig.h"


class WMEProxyCredentialSink : public IProxyCredentialSink
{
public:
    WMEProxyCredentialSink(UIViewController *view) {
        m_mainViewController = view;
    }
    
    virtual ~WMEProxyCredentialSink() {}
    virtual void OnProxyCredentialRequired(const char* szProxy, uint16_t port, const char* szRealm)
    {
        TestConfig &config = TestConfig::i();

        if (!config.m_sProxyUser.empty() && !config.m_sProxyPasswd.empty()) {
            SetProxyUsernamePassword(config.m_sProxyUser.c_str(), config.m_sProxyPasswd.c_str());
            return;
        }

        NSString *sMessage = [NSString stringWithFormat:@"Proxy %s:%d says %s", szProxy, port, szRealm];
        UIAlertController * alert=   [UIAlertController
                                      alertControllerWithTitle:@"Authentication Required:"
                                      message:sMessage
                                      preferredStyle:UIAlertControllerStyleAlert];
        
        UIAlertAction* ok = [UIAlertAction actionWithTitle:@"OK" style:UIAlertActionStyleDefault
                                                   handler:^(UIAlertAction * action) {
                                                       //Do Some action here
                                                       UITextField *userName = [alert textFields][0];
                                                       UITextField *passWord = [alert textFields][1];
                                                       
                                                       NSString *sUserName = [userName text];
                                                       NSString *sPassword = [passWord text];
                                                       
                                                       SetProxyUsernamePassword([sUserName UTF8String], [sPassword UTF8String]);
                                                   }];
        UIAlertAction* cancel = [UIAlertAction actionWithTitle:@"Cancel" style:UIAlertActionStyleDefault
                                                       handler:^(UIAlertAction * action) {
                                                           SetProxyUsernamePassword(NULL, NULL);
                                                           [alert dismissViewControllerAnimated:YES completion:nil];
                                                       }];
        
        [alert addAction:ok];
        [alert addAction:cancel];
        
        [alert addTextFieldWithConfigurationHandler:^(UITextField *textField) {
            textField.placeholder = @"Username";
        }];
        [alert addTextFieldWithConfigurationHandler:^(UITextField *textField) {
            textField.placeholder = @"Password";
            textField.secureTextEntry = YES;
        }];
        
        [m_mainViewController presentViewController:alert animated:YES completion:nil];
    }
private:
    UIViewController *m_mainViewController;
};


@interface WMEDisplayViewController ()

@property (assign, nonatomic) CGFloat scale;

@end

@implementation WMEDisplayViewController {
    WMEProxyCredentialSink *_proxySink;
}


@synthesize btSendVideo = _btSendVideo;
@synthesize btSendAudio = _btSendAudio;
@synthesize attendeeView = _attendeeView;
@synthesize selfView     = _selfView;
@synthesize contentView = _contentView;
@synthesize contentScrollView = _contentScrollView;
@synthesize previewModeLabel = _previewModeLabel;
@synthesize btStartServer = _btStartServer;
@synthesize strSdp;
@synthesize strLinus;
@synthesize strServer;
@synthesize bLoopback;
@synthesize bVideoHW;
@synthesize bCalliope;
@synthesize bInit;
@synthesize bTA, bEnableAppshare, bEnableSRTP;
@synthesize bEnableCVO;

@synthesize scale;



- (id)initWithNibName:(NSString *)nibNameOrNil bundle:(NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];
    if (self) {
        // Custom initialization
    }
    return self;
}


-(void)pinch:(UIPinchGestureRecognizer *) gesture
{
    if((gesture.state == UIGestureRecognizerStateChanged) ||
       (gesture.state == UIGestureRecognizerStateEnded)){
        CGRect oldframe = self.contentView.frame;
        if (((oldframe.size.width <= 30 || oldframe.size.height <= 30) && gesture.scale < 1)
            || ((oldframe.size.width >= 3000 || oldframe.size.height >= 3000) && gesture.scale > 1)) {
            gesture.scale = 1;
            return;
        }
        self.scale *= gesture.scale;
        [self contentScaleTo];
        gesture.scale = 1;
    }
}

-(void)contentScaleTo
{
    self.contentView.transform = CGAffineTransformScale(CGAffineTransformIdentity, self.scale, self.scale);
    self.contentScrollView.contentSize = self.contentView.frame.size;
    NSLog(@"self.contentView.frame:%@",NSStringFromCGRect(self.contentView.frame));
    [self.view setNeedsLayout];
}


- (void)viewDidLoad
{
    [super viewDidLoad];
    
    _proxySink = new WMEProxyCredentialSink(self);
    wme::SetProxyCredentialSink(_proxySink);

    self.scale = 1;
    
//    UIPinchGestureRecognizer* pinchRecer = [[UIPinchGestureRecognizer alloc] initWithTarget:self action:@selector(pinch:)];
//    [self.view addGestureRecognizer:pinchRecer];

    //Initialize the WME data processing
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(incomingCall:) name:@"NOTIFICATION_INCOMINGCALL" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(enableCall:) name:@"NOTIFICATION_ONSTARTED" object:nil];
#ifdef TEST_MODE
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(testStopCall:) name:@"NOTIFICATION_TEST_STOPCALL" object:nil];
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
    
    iVideoWidth = 0;
    iVideoHeight = 0;

    //Hide the top bar
    bBarHiddenFlag = TRUE;
    [super.navigationController setNavigationBarHidden:FALSE animated:TRUE];
    [super.navigationController setToolbarHidden:bBarHiddenFlag animated:TRUE];
    
    disconnectIndication = [[UIAlertView alloc] initWithTitle: @"Disconnect" message: @"Network disconnect!" delegate: self cancelButtonTitle: @"OK" otherButtonTitles: nil];
    
    //query the default data

    
    //Add the observer to notification centre
    //[[NotificationTransfer instance] addNotificationObserver:self];
    
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
    
    [_btStartServer setHidden:YES];
    
//    [self.contentScrollView addSubView:self.contentView];
    self.contentScrollView.delegate = self;
    
    updateTableViewSourceTimer = [NSTimer scheduledTimerWithTimeInterval:1 target:self selector:@selector(runTimer) userInfo:nil repeats:YES];

    self.fakeView = [[MediaClockDrone alloc] initWithFrame:CGRectMake(0, 150, 100, 1)];
    [self.view addSubview: self.fakeView];
    
    [self showStats: YES];
}

- (void)runTimer
{
    wme::WmeAudioConnectionStatistics audio = {{0}};
    wme::WmeVideoConnectionStatistics video = {{0}};
    wme::WmeCpuUsage cpu={0};
    wme::WmeMemoryUsage memoryUsage = {0};
    if(bLoopback){
        LoopbackCall::Instance()->getStatistics(audio, video);
        LoopbackCall::Instance()->getCpuStatistics(cpu);
        LoopbackCall::Instance()->m_endCaller.GetMemoryUsage(memoryUsage);
    }else{
        PeerCall::Instance()->m_endCaller->getStatistics(audio, video);
        PeerCall::Instance()->m_endCaller->getCpuStatistics(cpu);
        PeerCall::Instance()->m_endCaller->GetMemoryUsage(memoryUsage);
    }
    if(arrayStats == nil) {
        arrayStats = [[NSMutableArray alloc] init];
    }
    [arrayStats removeAllObjects];
    NSMutableArray *arrayCpu = [[NSMutableArray alloc] init];
    [arrayCpu addObject:[NSString stringWithFormat:@"CPU usage: %f", cpu.fTotalUsage]];
    [arrayCpu addObject:[NSString stringWithFormat:@"Memory usage: %f", memoryUsage.fMemroyUsage]];
    
    NSMutableArray *arrayAudio = [[NSMutableArray alloc] init];
    [arrayAudio addObject:[NSString stringWithFormat:@"RTT: %d", audio.sessStat.stOutNetworkStat.uRoundTripTime]];
    [arrayAudio addObject:[NSString stringWithFormat:@"TX Bytes(Packets):%d(%d)", audio.sessStat.stOutNetworkStat.uBytes, audio.sessStat.stOutNetworkStat.uPackets]];
        [arrayAudio addObject:[NSString stringWithFormat:@"Loss(Jitter):%f(%d)", audio.sessStat.stOutNetworkStat.fLossRatio, audio.sessStat.stOutNetworkStat.uJitter]];
    [arrayAudio addObject:[NSString stringWithFormat:@"Bitrate: %d", audio.localAudioStat.uBitRate]];
    [arrayAudio addObject:[NSString stringWithFormat:@"RX Bytes(Packets):%d(%d)", audio.sessStat.stInNetworkStat.uBytes, audio.sessStat.stInNetworkStat.uPackets]];
        [arrayAudio addObject:[NSString stringWithFormat:@"Loss(Jitter):%f(%d)", audio.sessStat.stInNetworkStat.fLossRatio, audio.sessStat.stInNetworkStat.uJitter]];
    [arrayAudio addObject:[NSString stringWithFormat:@"Bitrate: %d", audio.remoteAudioStat.uBitRate]];
    
    NSMutableArray *arrayVideo = [[NSMutableArray alloc] init];
    [arrayVideo addObject:[NSString stringWithFormat:@"RTT: %d", video.sessStat.stOutNetworkStat.uRoundTripTime]];
    [arrayVideo addObject:[NSString stringWithFormat:@"TX Bytes(Packets):%d(%d)", video.sessStat.stOutNetworkStat.uBytes, video.sessStat.stOutNetworkStat.uPackets]];
    [arrayVideo addObject:[NSString stringWithFormat:@"Loss(Jitter):%f(%d)", video.sessStat.stOutNetworkStat.fLossRatio, video.sessStat.stOutNetworkStat.uJitter]];
    [arrayVideo addObject:[NSString stringWithFormat:@"br:%f, fps:%f(%d*%d)", video.localVideoStat.fBitRate, video.localVideoStat.fFrameRate, video.localVideoStat.uWidth, video.localVideoStat.uHeight]];
    [arrayVideo addObject:[NSString stringWithFormat:@"RX Bytes(Packets):%d(%d)", video.sessStat.stInNetworkStat.uBytes, video.sessStat.stInNetworkStat.uPackets]];
        [arrayVideo addObject:[NSString stringWithFormat:@"Loss(Jitter):%f(%d)", video.sessStat.stInNetworkStat.fLossRatio, video.sessStat.stInNetworkStat.uJitter]];
        [arrayVideo addObject:[NSString stringWithFormat:@"br:%f, fps:%f(%d*%d)", video.remoteVideoStat.fBitRate, video.remoteVideoStat.fFrameRate, video.remoteVideoStat.uWidth, video.remoteVideoStat.uHeight]];
    
    [arrayStats addObject:arrayCpu];
    [arrayStats addObject:arrayAudio];
    [arrayStats addObject:arrayVideo];
    [self.statsView reloadData];
    
    if(0<TestConfig::i().m_tRunSeconds)
    {
        time_t now=time(NULL);
        if(now>TestConfig::i().m_tRunSeconds+TestConfig::i().m_tStartSeconds)
        {
            std::string ret;
            if (PeerCall::Instance()->isActive())
                ret = PeerCall::Instance()->stopPeer();
            else
                ret = LoopbackCall::Instance()->stopLoopback();
            TestConfig::i().m_tRunSeconds=0;
        }
    }
}

-(void) checkVideoWindow {
    unsigned int count = TestConfig::Instance().m_uMaxVideoStreams;
    CGFloat nextX = 0;
    while ( count > 1) {
        WMERenderView *thumbnail = [[WMERenderView alloc] initWithFrame:CGRectMake(10, 15 + nextX, 90, 160)];
        [self.view addSubview:thumbnail];
        PeerCall::Instance()->pushRemoteWindow((__bridge void*)thumbnail);
        nextX += 95;
        count--;
    }
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
    UIEdgeInsets zeroInsets = { 0. };
    self.contentScrollView.contentInset = zeroInsets;
    self.contentScrollView.contentSize = self.contentView.frame.size;
    [self.contentScrollView setZoomScale:self.contentScrollView.minimumZoomScale animated:YES];
/*
    if (self.pWMEDataProcess.bVideoSending == YES) {
        [_btSendVideo setTitle:@"Stop Video"];
        [_previewModeLabel setHidden:TRUE];
    }
    else
    {
        [_btSendVideo setTitle:@"Start Video"];
        //[_btSendVideo.titleLabel setTextColor:[UIColor redColor]];
        [_previewModeLabel setHidden:FALSE];
    }
    
    [self.pWMEDataProcess setRemoteRender:(void *)_attendeeView];
    [self.pWMEDataProcess setLocalRender:(void *)_selfView];*/
    if (self.bTA)
    {
        bEnableAppshare =  TestConfig::Instance().m_bAppshare;
    }
    
    
    self.contentScrollView.hidden = !bEnableAppshare;
    self.contentView.hidden = !bEnableAppshare;
    
    if (!self.bTA)
    {
        TestConfig::Instance().m_bAppshare = bEnableAppshare;
        TestConfig::Instance().m_audioDebugOption["enableSRTP"] = bEnableSRTP?true:false;
        TestConfig::Instance().m_videoDebugOption["enableSRTP"] = bEnableSRTP?true:false;
    }

    if( bEnableAppshare )
    {
        CGRect rcView = self.view.bounds;
        rcView.origin.y = rcView.size.height / 2;
        rcView.size.height /= 2;
        rcView.origin.x = 10.;
        rcView.size.width -= 20.;
        self.contentScrollView.frame = rcView;
    }
    if(!self.bInit)
    {
        TestConfig::Instance().m_bLoopback = self.bLoopback;
        TestConfig::Instance().m_bVideoHW  = self.bVideoHW;
        TestConfig::Instance().m_bEnableCVO = self.bEnableCVO;
        TestConfig::Instance().m_bCalliope = self.bCalliope;
        TestConfig::Instance().m_sLinusUrl = [self.strLinus UTF8String];
        
        if(self.bLoopback)
        {
            //vagouz>>TBD>>leave NULL for screen iOS first
            LoopbackCall::Instance()->startLoopback((__bridge void*)_attendeeView, (__bridge void*)_selfView, (__bridge void *)_contentView/*, self.bCalliope, [self.strLinus UTF8String], NULL*/);
        }
        else
        {
            PeerCall::Instance()->pushRemoteWindow((__bridge void*)_attendeeView);
            [self checkVideoWindow];
            PeerCall::Instance()->startPeer((__bridge void*)_selfView, (__bridge void *)_contentView);
            if(self.bTA)
            {
                if(self.strSdp.length == 0)
                    PeerCall::Instance()->m_endCaller->startCall();
                else
                    PeerCall::Instance()->m_endCaller->acceptCall([self.strSdp UTF8String]);
            }
            else
            {
                PeerCall::Instance()->connect([self.strServer UTF8String]);
            }
        }
        
        m_pUISink = new CUIRenderUpdateSink(self);
        if (PeerCall::Instance() && PeerCall::Instance()->m_endCaller)
            PeerCall::Instance()->m_endCaller->SetUISink(m_pUISink);
        if (LoopbackCall::Instance())
            LoopbackCall::Instance()->m_endCaller.SetUISink(m_pUISink);
        
        self.bInit = YES;
    }
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
    //[self.pWMEDataProcess setRemoteRender:NULL];
    //[self.pWMEDataProcess setLocalRender:NULL];
}

- (void)applicationWillResignActive
{
    if(self.bLoopback)
        LoopbackCall::Instance()->resumeLoopback(false);
    else
        PeerCall::Instance()->m_endCaller->resume(false);
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

    if(self.bLoopback)
        LoopbackCall::Instance()->resumeLoopback(true);
    else
        PeerCall::Instance()->m_endCaller->resume(true);
}

- (UIView *)viewForZoomingInScrollView:(UIScrollView *)scrollView
{
    return self.contentView;
}

- (void)scrollViewDidEndZooming:(UIScrollView *)scrollView withView:(UIView *)view atScale:(CGFloat)scale
{
    
}

- (void)orientationChanged:(NSNotification *)notification
{
    self.statsView.frame = self.view.frame;
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
    /*[self.pWMEDataProcess setRemoteRender: NULL];
    [self.pWMEDataProcess setLocalRender:NULL];

    /// set the latest render
    [self.pWMEDataProcess setRemoteRender:(__bridge void *)_attendeeView];
    [self.pWMEDataProcess setLocalRender:(__bridge void *)_selfView];*/
    
    if(self.bLoopback)
        LoopbackCall::Instance()->resizeLoopback();
    else
        PeerCall::Instance()->m_endCaller->resizeRender();
    
    [self.contentScrollView setZoomScale:self.contentScrollView.minimumZoomScale animated:YES];
    
    if (iVideoHeight > 0 && iVideoWidth > 0 && _attendeeView)
        [self UpdateUI:iVideoWidth height:iVideoHeight data:(void *)_attendeeView];
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

    if( bEnableAppshare )
    {
        viewWidth /= 2;
        viewHeight /= 2;
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

    
    //[_selfView setFrame:CGRectMake(50, 120, 240, 320)];
    //[_attendeeView setFrame:CGRectMake(320, 120, 240, 320)];
    [_selfView setFrame:CGRectMake(selfPositionX, selfPositionY, selfWidth, selfHeight)];
    [_attendeeView setFrame:CGRectMake(attendeePositionX, attendeePositionY, attendeeWidth,attendeeHeight)];
    
}

- (void)updateRenderUIForPortrait
{
    float selfWidth,selfHeight, selfPositionX, selfPositionY,
    attendeeWidth,attendeeHeight, attendeePositionX, attendeePositionY;
    float viewWidth = self.view.frame.size.width;
    float viewHeight = self.view.frame.size.height;
    float gapH;
    float gapV;
    
    
    if( bEnableAppshare )
    {
        viewWidth /= 2;
        viewHeight /= 2;
    }
    
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
    /*
    if (bAttendeeRatioIsPortrait == NO)
    {
        attendeeWidth  = viewWidth - (gapH*2);
        attendeeHeight = (attendeeWidth/16.0)*9.0;
    }
      */  
    /*if (self.pWMEDataProcess.bKeepAspectRatio == NO)
    {
        selfWidth  = (viewWidth/2) - gapH;
        selfHeight = (selfWidth/16.0)*9.0;
    }*/
    attendeePositionX = (viewWidth - attendeeWidth)/2;
    attendeePositionY = (viewHeight -attendeeHeight)/2;
    selfPositionX = viewWidth -selfWidth -gapH;
    selfPositionY = viewHeight - selfHeight - gapV - 80;
    
    //[_selfView setFrame:CGRectMake(10, 120, 120, 160)];
    //[_attendeeView setFrame:CGRectMake(160, 120, 120, 160)];
    [_selfView setFrame:CGRectMake(selfPositionX, selfPositionY, selfWidth, selfHeight)];
    [_attendeeView setFrame:CGRectMake(attendeePositionX, attendeePositionY, attendeeWidth,attendeeHeight)];
    
}

/*
- (void)networkDisconnect:(DEMO_MEDIA_TYPE) eType
{
    [disconnectIndication show];
    [[NotificationTransfer instance] removeNotificationObserver:self];
    
    if (self.pWMEDataProcess.bVideoSending == YES) {
        [self ButtonSendVideo:_btSendVideo];
    }
}*/
- (void)OnDecodeSizeChanged:(uint32_t)uLabel width:(uint32_t)uWidth height:(uint32_t)uHeight
{
}

- (void)OnRenderSizeChanged:(uint32_t)uLabel width:(uint32_t)uWidth height:(uint32_t)uHeight
{
    if( 3 == uLabel )
    {
        self.contentView.hidden = NO;
        bRemoteContentShare = YES;
    }
    else 
    {
        NSString *strResolution = [NSString stringWithFormat:@"%dX%d", uWidth, uHeight];
    
        self.attendeeResolution.text = strResolution;
    
        if (uWidth>uHeight) {
            bAttendeeRatioIsPortrait = NO;
        }
        else
        {
            bAttendeeRatioIsPortrait = YES;
        }
    }

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
    /*[self.pWMEDataProcess setRemoteRender: NULL];
    [self.pWMEDataProcess setLocalRender:NULL];
    
    /// set the latest render
    [self.pWMEDataProcess setRemoteRender:(__bridge void *)_attendeeView];
    [self.pWMEDataProcess setLocalRender:(__bridge void *)_selfView];*/
}

/*
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
    
}*/
- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)ButtonSendVideo:(id)sender
{
    if ([_btSendVideo.title isEqualToString:@"Start Video"]) {
        if(self.bLoopback)
            LoopbackCall::Instance()->m_endCaller.mute(false, true);
        else
            PeerCall::Instance()->m_endCaller->mute(false, true);
        
        //update UI
        [_btSendVideo setTitle:@"Stop Video"];
    }
    else{
        if(self.bLoopback)
            LoopbackCall::Instance()->m_endCaller.mute(true, true);
        else
            PeerCall::Instance()->m_endCaller->mute(true, true);
        //Update UI
        [_btSendVideo setTitle:@"Start Video"];

    }
}
- (IBAction)ButtonSendAudio:(id)sender
{
    if ([_btSendAudio.title isEqualToString:@"Start Audio"]) {
        if(self.bLoopback)
            LoopbackCall::Instance()->m_endCaller.mute(false, false);
        else
            PeerCall::Instance()->m_endCaller->mute(false, false);
        //update UI
        [_btSendAudio setTitle:@"Stop Audio"];
    }
    else{
        if(self.bLoopback)
            LoopbackCall::Instance()->m_endCaller.mute(true, false);
        else
            PeerCall::Instance()->m_endCaller->mute(true, false);
        //Update UI
        [_btSendAudio setTitle:@"Start Audio"];
        
    }
}

- (void)switchCamera:(UITapGestureRecognizer *)sender
{
    NSString *name = [NSString string];
    if(bCameraSwitchFlag == YES)
    {
        name = @"back";
        bCameraSwitchFlag = NO;
        self.ivCameraSwitch.image = [UIImage imageNamed:@"ic_action_switch_camera.png"];
    }
    else
    {
        name = @"front";
        bCameraSwitchFlag = YES;
        self.ivCameraSwitch.image = [UIImage imageNamed:@"cameraSwitch.png"];
    }

    if(self.bLoopback)
        LoopbackCall::Instance()->m_endCaller.setCamera([name UTF8String]);
    else
        PeerCall::Instance()->m_endCaller->setCamera([name UTF8String]);
    //[self.pWMEDataProcess switchCameraDevice];
}

- (IBAction)ButtonSwitchSpeaker:(id)sender {
    //[self.pWMEDataProcess switchAudioSpeaker];
}

-(void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    
    //For set the parameters setting
    if ([[segue identifier] isEqualToString:@"settingSegue"]) {
        WMESettingViewController *settingViewController = [segue destinationViewController];
        settingViewController.bLoopback = self.bLoopback;
    }
    
}

//For start server button
- (IBAction)ButtonStartServer:(id)sender
{/*
    if(self.bCalliope)
    {
        if([self.strSdp length] == 0)
            PeerCall::Instance()->m_endCaller->connectVenue(NULL);
        else
            PeerCall::Instance()->m_endCaller->connectVenue([self.strSdp UTF8String]);
    }
    else
    {
        if([self.strSdp length] == 0)
            PeerCall::Instance()->m_endCaller->startCall();
        else
            PeerCall::Instance()->m_endCaller->acceptCall([self.strSdp UTF8String]);
    }

    [_btStartServer setHidden:YES];
    
    self.pWMEDataProcess.UseICE = NO;
    self.pWMEDataProcess.IsHost = YES;
    [self.pWMEDataProcess clickedConnect];*/
}

//unwind segue
- (IBAction)SetSetting:(UIStoryboardSegue *)segue
{
    
    if ([[segue identifier] isEqualToString:@"settingUnwindSegue"]) {
        WMESettingViewController *settingViewController = [segue sourceViewController];
        //self.pWMEDataProcess.bKeepAspectRatio = settingViewController.bKeepAspectRatio;
    }
}

- (void)changeBarViewState:(UITapGestureRecognizer *)sender
{
  	bBarHiddenFlag=!bBarHiddenFlag;
	[super.navigationController setNavigationBarHidden:bBarHiddenFlag animated:TRUE];
    if(!self.bLoopback)
        [super.navigationController setToolbarHidden:bBarHiddenFlag animated:TRUE];
}

#ifdef TEST_MODE
- (void)testStopCall:(NSNotification *)notification
{
    [self performSegueWithIdentifier:@"backSegue" sender:self];
}
#endif
/*
- (void)incomingCall:(NSNotification *)notification
{
    [_btStartServer setTitle:@"Accept Call" forState: UIControlStateNormal];
    self.strSdp = [notification object];
}

- (void)enableCall:(NSNotification *)notification
{
    [_btStartServer setHidden:NO];
}
*/

- (void)showStats: (BOOL)show
{
    if(show) {
        if(self.statsView == nil) {
            self.statsView = [[UITableView alloc] initWithFrame:self.view.frame style:UITableViewStyleGrouped];
            self.statsView.backgroundColor = [UIColor clearColor];
            self.statsView.backgroundView = nil;
            self.statsView.opaque = NO;
            self.statsView.dataSource = self;
            self.statsView.allowsSelection = NO;
            [self.statsView registerClass:[UITableViewCell class] forCellReuseIdentifier:@"StatisticsContentCell"];
        }
        [self.view addSubview:self.statsView];
    }
    else {
        [self.statsView removeFromSuperview];
    }
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    return [arrayStats[section] count];
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"StatisticsContentCell" forIndexPath:indexPath];
    
    // Configure the cell...
    cell.textLabel.text = arrayStats[indexPath.section][indexPath.row];
    cell.backgroundColor = [UIColor clearColor];
    cell.backgroundView = nil;
    return cell;
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return [arrayStats count];
}

- (NSString *)tableView:(UITableView *)tableView titleForHeaderInSection:(NSInteger)section
{
    switch (section) {
        case 0:
        return @"CPU";
        break;
        case 1:
        return @"Audio";
        break;
        case 2:
        return @"Video";
        break;
        default:
        break;
    }
    return @"nil";
}

- (void) UpdateUI:(uint32_t)uWidth height:(uint32_t)uHeight data:(void *)callee
{
    iVideoWidth = uWidth;
    iVideoHeight = uHeight;
    
    bool isPortrait = uWidth < uHeight ? true : false;
    
    float attendeeWidth,attendeeHeight, attendeePositionX, attendeePositionY;
    float viewWidth = self.view.frame.size.width;
    float viewHeight = self.view.frame.size.height;
    float gapH = 0;
    float gapV = 0;
    
    bool bEnableAppshare = TestConfig::Instance().m_bAppshare;
    if( bEnableAppshare ) {
        viewWidth /= 2;
        viewHeight /= 2;
    }
    
    bool bIsPad  = TRUE;
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone) {
        bIsPad  = FALSE;
    }
    if (bIsPad == TRUE) {
        //for iPad portrait
        gapH = 20;
        gapV = 20;
    } else {
        //for iphone portrait
        gapH = 10;
        gapV = 10;
    }
    
    if (isPortrait) {  // portrait;
        if ((viewWidth/9) > (viewHeight/16)) {
            //attendee-window
            attendeeHeight = viewHeight-(gapV*2);
            attendeeWidth = (attendeeHeight/16.0)*9.0;
        } else {
            //attendee-window
            attendeeWidth = viewWidth-(gapH*2);
            attendeeHeight = (attendeeWidth/9.0)*16.0;
        }
    } else {  // landscape;
        if ((viewWidth/16) > (viewHeight/9)) {
            attendeeHeight = viewHeight-(gapV*2);
            attendeeWidth = (attendeeHeight/9.0)*16.0;
        } else {
            attendeeWidth = viewWidth-(gapH*2);
            attendeeHeight = (attendeeWidth/16.0)*9.0;
        }
    }
    
    attendeePositionX = (viewWidth - attendeeWidth)/2;
    attendeePositionY = (viewHeight -attendeeHeight)/2;
    [(__bridge WMERenderView *)callee setFrame:CGRectMake(attendeePositionX, attendeePositionY, attendeeWidth,attendeeHeight)];
}

#pragma mark -
#pragma mark TA CASE

@end


