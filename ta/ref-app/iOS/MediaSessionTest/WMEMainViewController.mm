//
//  WMEViewController.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-5-17.
//  Copyright (c) 2013, video. All rights reserved.
//

#import "WMEMainViewController.h"
#import "WMEDisplayViewController.h"
#import "PeerCall.h"
#import "Loopback.h"
#import "WMEDisplayViewController.h"

@interface WMEMainViewController ()
@end

@implementation WMEMainViewController

//synthesize
@synthesize scrollView = _scrollView;
@synthesize scServerOrClient = _scServerOrClient;
@synthesize tfServerIP = _tfServerIP;
@synthesize lbServerIP = _lbServerIP;
@synthesize btConnect = _btConnect;
@synthesize tfLinusIP = _tfLinusIP;
@synthesize lbLinusIP = _lbLinusIP;
@synthesize tfStreamsCount = _tfStreamsCount;
@synthesize btLocalMode = _btLocalMode;

@synthesize swCalliope = _swCalliope;
@synthesize swLoopback = _swLoopback;
@synthesize swAppshare = _swAppshare;
@synthesize swVPIOMode = _swVPIOMode;
@synthesize swRemoteAECMode = _swRemoteAECMode;
@synthesize swEnableCVO = _swEnableCVO;
@synthesize bTA;
@synthesize json;

- (void)viewDidLoad
{
    [super viewDidLoad];
    self.scrollView.contentSize = CGSizeMake(320, 460*2);
    [self.scrollView flashScrollIndicators];
    self.scrollView.directionalLockEnabled = YES;
    
    UITapGestureRecognizer *singleTap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(singleTapGestureCaptured:)];
    [self.scrollView addGestureRecognizer:singleTap];
    //Initialize the WME data processing
    //self.pWMEDataProcess = [WMEDataProcess instance];
	
    // Do any additional setup after loading the view, typically from a nib.
    [_swLoopback addTarget:self action:@selector(didChangeLoopback:) forControlEvents:UIControlEventValueChanged];
    [_swCalliope addTarget:self action:@selector(didChangeCalliope:) forControlEvents:UIControlEventValueChanged];
    _scServerOrClient.selectedSegmentIndex = 0;

#ifdef TEST_MODE
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(testStartCall:) name:@"NOTIFICATION_TEST_STARTCALL" object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(testStartScreenShare:) name:@"NOTIFICATION_TEST_STARTSCREENSHARE" object:nil];
    
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    UILabel *ipLabel = [[UILabel alloc]initWithFrame:CGRectMake(0,0,80,20)];
    ipLabel.accessibilityValue = [defaults objectForKey:@"local_ip_for_test_mode"];
    ipLabel.accessibilityIdentifier = @"IPLabel";
    [self.view addSubview:ipLabel];
#endif
    _tfLinusIP.clearButtonMode = UITextFieldViewModeWhileEditing;
    _tfServerIP.clearButtonMode = UITextFieldViewModeWhileEditing;
    
    //init UI status by original default/input params
    [_swAppshare setOn:TestConfig::i().m_bAppshare];
    [_swLoopback setOn:TestConfig::i().m_bLoopback];
    [_swCalliope setOn:TestConfig::i().m_bCalliope];
    [_swVideoHW  setOn:TestConfig::i().m_bVideoHW ];
    [_swVPIOMode setOn:TestConfig::i().m_bUsingVPIO ];
    [_swRemoteAECMode setOn:TestConfig::i().m_bUsingTCAEC ];
    [_swEnableCVO setOn:TestConfig::i().m_bEnableCVO];
    if(TestConfig::i().m_bUsingVPIO)
    {
      _swRemoteAECMode.enabled = FALSE;
    }
    [self updateWmeEngine];
  
    _tfLinusIP.text = [NSString stringWithUTF8String:TestConfig::i().m_sLinusUrl.c_str()];
    _tfServerIP.text = [NSString stringWithUTF8String:TestConfig::i().m_sWSUrl.c_str()];
    _tfStreamsCount.text = [NSString stringWithFormat:@"%d", TestConfig::i().m_uMaxVideoStreams];
    //update UI
    [self didChangeLoopback:_swLoopback];
    [self didChangeCalliope:_swCalliope];
    
    //Hidden the server IP item
    //[_tfServerIP setHidden:YES];
    //[_lbServerIP setHidden:YES];
    //[_btConnect setHidden:YES];
    //[_btLocalMode setHidden:YES];
    //[_tfLinusIP setHidden:YES];
    //[_lbLinusIP setHidden:YES];
    [self.swMultiStream setOn:YES];
    
    BOOL bEnableSRTP = YES;
    if(TestConfig::Instance().m_videoDebugOption["enableSRTP"].isBool())
        bEnableSRTP = TestConfig::Instance().m_videoDebugOption["enableSRTP"].asBool() ? YES : NO;
    [self.swSRTP setOn:bEnableSRTP];
    
    BOOL bEnableFEC = YES;
    if(TestConfig::i().m_videoParam["fecParams"]["bEnableFec"].isBool())
        bEnableFEC = TestConfig::i().m_videoParam["fecParams"]["bEnableFec"].asBool() ? YES : NO;
    [self.swFec setOn:bEnableFEC];
    
    [[UIApplication sharedApplication] setIdleTimerDisabled:YES];
    
    if(TestConfig::i().m_bAutoStart)
        [self performSelector:@selector(ButtonConnect:) withObject:self afterDelay:1];

}

- (void)viewDidAppear:(BOOL)animated
{
    [_btConnect setHidden:NO];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void)checkStreamsCountField
{
    if([self.tfStreamsCount.text intValue] == 0) {
        self.tfStreamsCount.text = @"1";
    }
}
- (void)updateWmeEngine
{
    wme::UninitMediaEngine();
    wme::InitMediaEngine(!TestConfig::Instance().m_bUsingVPIO,TestConfig::Instance().m_bUsingTCAEC,false,false);
}

//text field delegate
-(BOOL)textFieldShouldReturn:(UITextField *)textField
{
    if (textField == self.tfServerIP || textField == self.tfLinusIP || textField == self.tfStreamsCount) {
        [textField resignFirstResponder];
        [self checkStreamsCountField];
    }
    return YES;
}

/*
//For segmented Control
-(void)didChangeSegmentControl:(UISegmentedControl *)control
{
    //Server
    if (control.selectedSegmentIndex == 0) {
        //Hidden the server IP item
        [_tfServerIP setHidden:YES];
        [_lbServerIP setHidden:YES];
        [_btConnect setHidden:YES];
        [_btStartServer setHidden:NO];
        
    }
    //Client
    else if (control.selectedSegmentIndex == 1)
    {
        //show the server IP item
        [_tfServerIP setHidden:NO];
        [_lbServerIP setHidden:NO];
        [_btConnect setHidden:NO];
        [_btStartServer setHidden:YES];        
    }

}*/

//For button action
- (void)singleTapGestureCaptured:(UITapGestureRecognizer *)gesture
{
    [self.tfServerIP resignFirstResponder];
    [self.tfLinusIP resignFirstResponder];
    [self.tfStreamsCount resignFirstResponder];
    [self checkStreamsCountField];
}

//For connect button
- (IBAction)ButtonConnect:(id)sender
{
    self.bTA = NO;
    [self performSegueWithIdentifier:@"DISPLAY_SEGUE" sender:self];
}

#ifdef TEST_MODE
- (void)testStartCall:(NSNotification *)notification
{
    self.bTA = YES;
    self.json = [notification object];
    BOOL isLoopback = [json[@"loopback"] boolValue];
    BOOL isP2p = [json[@"p2p"] boolValue];
    [_swLoopback setOn:isLoopback animated:NO];
    [_swCalliope setOn:!isP2p animated:NO];
    [_swVideoHW setOn:TestConfig::i().m_bVideoHW animated:NO];
    [_swEnableCVO setOn:TestConfig::i().m_bEnableCVO animated:NO];
    _tfLinusIP.text = json[@"linus"];
    _tfStreamsCount.text = [NSString stringWithFormat:@"%d", [json[@"videoStreams"] intValue]];
    [self performSegueWithIdentifier:@"DISPLAY_SEGUE" sender:self];
}
#endif


#ifdef TEST_MODE
- (void)testStartScreenShare:(NSNotification *)notification
{
    self.bTA = YES;

    id json = [notification object];
    BOOL isLoopback = FALSE;//[json[@"loopback"] boolValue];
    BOOL isP2p = TRUE;//[json[@"p2p"] boolValue];
    [_swLoopback setOn:isLoopback animated:NO];
    [_swCalliope setOn:!isP2p animated:NO];
    _tfLinusIP.text = json[@"linus"];
    [self performSegueWithIdentifier:@"DISPLAY_SEGUE" sender:self];
}
#endif

-(void)didChangeLoopback:(UISwitch *)control
{
    [_tfServerIP setHidden:control.on];
    [_lbServerIP setHidden:control.on];
    [_tfStreamsCount setHidden:control.on];
}

-(void)didChangeCalliope:(UISwitch *)control
{
    [_tfLinusIP setHidden:!control.on];
    [_lbLinusIP setHidden:!control.on];
}

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    // Make sure your segue name in storyboard is the same as this line
    if ([[segue identifier] isEqualToString:@"DISPLAY_SEGUE"])
    {
        // Get reference to the destination view controller
        UINavigationController *vc = [segue destinationViewController];
        WMEDisplayViewController *display = (WMEDisplayViewController*)vc.topViewController;
        
        // Pass any objects to the view controller here, like...
        display.bLoopback = _swLoopback.on;
        display.bVideoHW  = _swVideoHW.on;
        display.bEnableCVO = _swEnableCVO.on;
        display.bCalliope = _swCalliope.on;
        display.strLinus = [_tfLinusIP text];
        display.strServer = [_tfServerIP text];
        display.bTA = self.bTA;
        display.bEnableAppshare = self.swAppshare.on;
        display.bEnableSRTP = self.swSRTP.on;
        display.strSdp = json[@"sdp"];
        
        if(!self.bTA)
        {
            TestConfig::i().m_audioParam["supportCmulti"] = (bool)self.swMultiStream.on;
            TestConfig::i().m_videoParam["supportCmulti"] = (bool)self.swMultiStream.on;
            TestConfig::i().m_shareParam["supportCmulti"] = (bool)self.swMultiStream.on;
            if (self.swFec.on) {
                //enableFec button on ref-app just for video fec
                TestConfig::i().m_videoParam["fecParams"]["uClockRate"] = 8000;
                TestConfig::i().m_videoParam["fecParams"]["uPayloadType"] = 111;
                TestConfig::i().m_videoParam["fecParams"]["bEnableFec"] = true;
                
                TestConfig::i().m_audioParam["fecParams"]["uClockRate"] = 8000;
                TestConfig::i().m_audioParam["fecParams"]["uPayloadType"] = 112;
                TestConfig::i().m_audioParam["fecParams"]["bEnableFec"] = true;

            }else {
                TestConfig::i().m_videoParam["fecParams"]["bEnableFec"] = false;
                TestConfig::i().m_audioParam["fecParams"]["bEnableFec"] = false;
            }
            
            TestConfig::i().m_videoParam["bHWAcceleration"] = (bool)self.swVideoHW.on;
            TestConfig::i().m_videoParam["bEnableCVO"] = (bool)self.swEnableCVO.on;
            TestConfig::i().m_uMaxVideoStreams = [[_tfStreamsCount text] intValue];
            TestConfig::i().m_uMaxAudioStreams = TestConfig::i().m_uMaxVideoStreams>1 ? 3 : 1;
        }
    }
}

- (IBAction)StopPlay:(UIStoryboardSegue *)segue
{
    //Generate the disconnect event
    WMEDisplayViewController *display = [segue sourceViewController];
    if(display.bLoopback)
    {
        LoopbackCall::Instance()->stopLoopback();
    }
    else
    {
        PeerCall::Instance()->stopPeer();
    }
}

- (IBAction)ButtonSwitchVPIO
{
  if (_swVPIOMode.isOn == false) {
    _swRemoteAECMode.enabled = true;
    TestConfig::i().m_bUsingVPIO = false;
  }
  else{
    _swRemoteAECMode.enabled = false;
    TestConfig::i().m_bUsingVPIO = true;
  }
  TestConfig::i().m_bUsingTCAEC = false;
  [_swRemoteAECMode setOn:TestConfig::i().m_bUsingTCAEC];
    TestConfig::i().m_aectype = "WmeAecTypeWmeDefault";
  [self updateWmeEngine];

}

- (IBAction)ButtonSwitchTCAEC
{
  if(_swRemoteAECMode.isOn == true)
  {
      TestConfig::i().m_bUsingTCAEC = true;
      TestConfig::i().m_bUsingVPIO = false;
      TestConfig::i().m_aectype = "WmeAecTypeTc";
 }
  else
  {
      TestConfig::i().m_bUsingTCAEC = false;
      TestConfig::i().m_bUsingVPIO = false;
      TestConfig::i().m_aectype = "WmeAecTypeWmeDefault";
  }
  [self updateWmeEngine];
}

@end
