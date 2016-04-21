//
//  WMEConnectionViewController.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-7-17.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import "WMEConnectionViewController.h"
#import "WMEMainViewController.h"
#import "WMEICEConnectionViewController.h"
#import "WMEDataProcess.h"
#import "WMEDeviceStatus.h"

//NotificationTransfer *m_pNotificationTransfer;

@interface WMEConnectionViewController ()

@end

@implementation WMEConnectionViewController

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
	// Do any additional setup after loading the view.
    self.pNotificationTransfer = [NotificationTransfer instance];
    [self.pNotificationTransfer initNotificationTransfer];
    [self.pNotificationTransfer addNotificationObserver:self];

    self.pWMEDataProcess = [WMEDataProcess instance];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)StopPlay:(UIStoryboardSegue *)segue
{
    //Generate the disconnect event
    [self networkDisconnect:DEMO_MEDIA_AUDIO];
    [self networkDisconnect:DEMO_MEDIA_VIDEO];
    [self networkDisconnect:DEMO_MEDIA_DESKSHARE];
    
    [self.pWMEDataProcess clickedDisconnect];

    [self.pNotificationTransfer removeNotificationObserver:self];
    [self.pNotificationTransfer initNotificationTransfer];
    [self.pNotificationTransfer addNotificationObserver:self];
    MessageBody* msgBody = [[MessageBody alloc] initwithRequestType:URLRequestStopCall WithParam:nil];
    [[WMEDeviceStatusClient instance] postUrlMessage:msgBody];
}


- (void)networkConnect:(DEMO_MEDIA_TYPE) eType
{
    if ((_bAudioConnectionState == FALSE) && (_bVideoConnectionState == FALSE))
    {
        //Send delegate
        if (self.pWMEDataProcess.useICE == FALSE)
            [(WMEMainViewController *)self.selectedViewController networkConnect];
        else
            [(WMEICEConnectionViewController *)self.selectedViewController networkConnect];
    }
    
    if (eType == DEMO_MEDIA_AUDIO) {
        _bAudioConnectionState = TRUE;
    }
    else if (eType == DEMO_MEDIA_VIDEO)
    {
        _bVideoConnectionState = TRUE;
    }
    else if( eType == DEMO_MEDIA_DESKSHARE )
        _bShareConnectionState = YES;
    
    //if (((_bAudioConnectionState == TRUE) || (self.pWMEDataProcess.bAudioModuleEnable == NO))&&
    //    ((_bVideoConnectionState == TRUE) || (self.pWMEDataProcess.bVideoModuleEnable == NO)))

    
    
}
- (void)networkDisconnect:(DEMO_MEDIA_TYPE) eType
{    

    if (((_bAudioConnectionState == TRUE) || (self.pWMEDataProcess.bAudioModuleEnable == NO))&&
        ((_bVideoConnectionState == TRUE) || (self.pWMEDataProcess.bVideoModuleEnable == NO)))
    {
        //Send delegate
        if (self.pWMEDataProcess.useICE == FALSE)
            [(WMEMainViewController *)self.selectedViewController networkDisconnect];
        else
            [(WMEICEConnectionViewController *)self.selectedViewController networkDisconnect];
    }
    
    if (eType == DEMO_MEDIA_AUDIO) {
        _bAudioConnectionState = FALSE;
        [self.pWMEDataProcess stopAudioClient:WME_RECVING];
        [self.pWMEDataProcess stopAudioClient:WME_SENDING];
    }
    else if (eType == DEMO_MEDIA_VIDEO)
    {
        _bVideoConnectionState = FALSE;
        [self.pWMEDataProcess stopVideoClient:WME_RECVING];
        [self.pWMEDataProcess stopVideoClient:WME_SENDING];
    }
    else if( eType == DEMO_MEDIA_DESKSHARE )
    {
        _bShareConnectionState = NO;
        [self.pWMEDataProcess stopContentShareClient];
    }
    
}


@end
