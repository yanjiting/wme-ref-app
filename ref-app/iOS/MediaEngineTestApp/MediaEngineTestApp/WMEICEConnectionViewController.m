//
//  WMEICEConnectionViewController.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-7-17.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import "WMEICEConnectionViewController.h"
#import "WMEDataProcess.h"


@interface WMEICEConnectionViewController ()

@end

@implementation WMEICEConnectionViewController
//Synthesize

@synthesize lbToID = _lbToID;
@synthesize tfToID = _tfToID;
@synthesize tfFromID = _tfFromID;
@synthesize tfSTUNServerIP = _tfSTUNServerIP;
@synthesize tfJINGLEServerIP = _tfJINGLEServerIP;
@synthesize tfSTUNServerPort = _tfSTUNServerPort;
@synthesize tfJINGLEServerPort = _tfJINGLEServerPort;
@synthesize scServerOrClient = _scServerOrClient;
@synthesize statusIndication = _statusIndication;


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
    //Initialize the WME data processing
    self.pWMEDataProcess = [WMEDataProcess instance];
    
	// Do any additional setup after loading the view.
     [_scServerOrClient addTarget:self action:@selector(didChangeSegmentControl:) forControlEvents:UIControlEventValueChanged];
    _scServerOrClient.selectedSegmentIndex = 0;
    [_lbToID setHidden:YES];
    [_tfToID setHidden:YES];
    
    //Init the status indication window
    _statusIndication = [[UIAlertView alloc] initWithTitle: @"Connecting" message: @"Waiting the connection" delegate: self cancelButtonTitle: @"Cancel" otherButtonTitles: nil];

}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

//text field delegate
-(BOOL)textFieldShouldReturn:(UITextField *)textField
{
    if ((textField == _tfSTUNServerIP)||
        (textField == _tfJINGLEServerIP)||
        (textField == _tfFromID)||
        (textField == _tfSTUNServerPort)||
        (textField == _tfJINGLEServerPort)||
        (textField == _tfToID))
    {
        [textField resignFirstResponder];
    }
    
    return YES;
}

//Button
- (IBAction)ButtonConnect:(id)sender
{

    [_statusIndication show];
    
    //ICE connection
    self.pWMEDataProcess.useICE = YES;
    self.pWMEDataProcess.isHost = (_scServerOrClient.selectedSegmentIndex == 0) ? YES:NO;
    self.pWMEDataProcess.myName = [_tfFromID text];
    self.pWMEDataProcess.hostName = [_tfToID text];
    self.pWMEDataProcess.jingleServerIP = [_tfJINGLEServerIP text];
    self.pWMEDataProcess.jingleServerPort = [_tfJINGLEServerPort text];
    self.pWMEDataProcess.stunServerIP = [_tfSTUNServerIP text];
    self.pWMEDataProcess.stunServerPort = [_tfSTUNServerPort text];
    
    [self.pWMEDataProcess clickedConnect];
}

- (void)showAlertWnd
{
    [_statusIndication dismissWithClickedButtonIndex:0 animated:(BOOL)YES];
    [self showAlertWindowTitle:@"Successful" message: @"Connection is successful!"];
    [self performSegueWithIdentifier:@"ICE_DISPLAY_SEGUE" sender:self];
}

-(void)showAlertWindowTitle:(NSString*)title message:(NSString*)message
{
    UIAlertView *someError = [[UIAlertView alloc] initWithTitle: title message: message delegate: self cancelButtonTitle: @"Ok" otherButtonTitles: nil];
    [someError show];
}

//Delegate for alertView
- (void)alertView:(UIAlertView *)alertView clickedButtonAtIndex:(NSInteger)buttonIndex
{
    if (_statusIndication == alertView) {
        //TBD...
        [self.pWMEDataProcess clickedDisconnect];
    }
    
}


//For segmented Control
-(void)didChangeSegmentControl:(UISegmentedControl *)control
{
    //Server
    if (control.selectedSegmentIndex == 0) {
        [_lbToID setHidden:YES];
        [_tfToID setHidden:YES];
    }
    //Client
    else if (control.selectedSegmentIndex == 1)
    {
        [_lbToID setHidden:NO];
        [_tfToID setHidden:NO];
    }
    
}

- (void)networkDisconnect
{
    
}
- (void)networkConnect
{
    [self performSelectorOnMainThread:@selector(showAlertWnd) withObject:nil waitUntilDone:NO];
}
@end
