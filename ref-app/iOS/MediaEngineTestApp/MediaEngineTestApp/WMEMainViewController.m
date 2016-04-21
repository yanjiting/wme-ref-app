//
//  WMEViewController.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-5-17.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import "WMEMainViewController.h"
#import "WMEDisplayViewController.h"
#import "WMEDataProcess.h"

#include <ifaddrs.h>
#include <arpa/inet.h>

@interface WMEMainViewController ()

@end

@implementation WMEMainViewController

//synthesize 
@synthesize scServerOrClient = _scServerOrClient;
@synthesize tfServerIP = _tfServerIP;
@synthesize lbServerIP = _lbServerIP;
@synthesize btConnect = _btConnect;
@synthesize btStartServer = _btStartServer;
@synthesize btLocalMode = _btLocalMode;

@synthesize statusIndication = _statusIndication;

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    //Initialize the WME data processing
    self.pWMEDataProcess = [WMEDataProcess instance];
	
    // Do any additional setup after loading the view, typically from a nib.
    [_scServerOrClient addTarget:self action:@selector(didChangeSegmentControl:) forControlEvents:UIControlEventValueChanged];
    _scServerOrClient.selectedSegmentIndex = 0;
    
    //Hidden the server IP item
    //[_tfServerIP setHidden:YES];
    //[_lbServerIP setHidden:YES];
    [_lbServerIP setText:@"This IP:"];
    [_tfServerIP setText:[self getIPAddress]];
    
    [_btConnect setHidden:YES];
    [_btLocalMode setHidden:YES];
    
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
    if (textField == self.tfServerIP) {
        [textField resignFirstResponder];
    }
    return YES;
}

-(NSString *)getIPAddress
{
    NSString *address = @"error";
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;
    // retrieve the current interfaces - returns 0 on success
    success = getifaddrs(&interfaces);
    if (success == 0)
    {
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while(temp_addr != NULL)
        {
            if(temp_addr->ifa_addr->sa_family == AF_INET)
            {
                // Check if interface is en0 which is the wifi connection on the iPhone
                if([[NSString stringWithUTF8String:temp_addr->ifa_name] isEqualToString:@"en0"])
                {
                    // Get NSString from C String
                    address = [NSString stringWithUTF8String:inet_ntoa(((struct sockaddr_in *)temp_addr->ifa_addr)->sin_addr)];
                    
                }
                
            }
            
            temp_addr = temp_addr->ifa_next;
        }
    }
    // Free memory
    freeifaddrs(interfaces);
    return address;
}

//For segmented Control
-(void)didChangeSegmentControl:(UISegmentedControl *)control
{
    //Server
    if (control.selectedSegmentIndex == 0) {
        //Hidden the server IP item
        //[_tfServerIP setHidden:YES];
        //[_lbServerIP setHidden:YES];
        [_lbServerIP setText:@"This IP:"];
        [_tfServerIP setText:[self getIPAddress]];
        
        [_btConnect setHidden:YES];
        [_btStartServer setHidden:NO];
        
    }
    //Client
    else if (control.selectedSegmentIndex == 1)
    {
        //show the server IP item
        //[_tfServerIP setHidden:NO];
        //[_lbServerIP setHidden:NO];
        [_lbServerIP setText:@"Host IP:"];
        [_tfServerIP setText:@""];
        
        [_btConnect setHidden:NO];
        [_btStartServer setHidden:YES];        
    }

}

//For button action
//For start server button
- (IBAction)ButtonStartServer:(id)sender
{
    [_statusIndication show];
    self.pWMEDataProcess.UseICE = NO;
    self.pWMEDataProcess.IsHost = YES;
    [self.pWMEDataProcess clickedConnect];
}

- (IBAction)TapBlankPlace:(id)sender {
    if (sender != self.tfServerIP) {
        [self.tfServerIP resignFirstResponder];
    }
}

//For connect button
- (IBAction)ButtonConnect:(id)sender
{
    [_statusIndication show];

    self.pWMEDataProcess.UseICE = NO;
    self.pWMEDataProcess.IsHost = NO;
    self.pWMEDataProcess.HostIPAddress = [_tfServerIP text];    
    [self.pWMEDataProcess clickedConnect];
    //for local audio file track
    [self.pWMEDataProcess startAudioClient:WME_SENDING];
    [self.pWMEDataProcess startAudioClient:WME_RECVING];
}
- (void)showAlertWnd
{
    [_statusIndication dismissWithClickedButtonIndex:0 animated:(BOOL)YES];
    [self showAlertWindowTitle:@"Successful" message: @"Connection is successful!"];
    [self performSegueWithIdentifier:@"DISPLAY_SEGUE" sender:self];
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
        //Force to exit from the connection
        [self.pWMEDataProcess clickedDisconnect];
    }
    
}


- (void)networkDisconnect
{

}
- (void)networkConnect
{
     [self performSelectorOnMainThread:@selector(showAlertWnd) withObject:nil waitUntilDone:NO];
}

- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
    if ([segue.identifier isEqualToString:@"PreMeetingSetting"]) {
        UINavigationController* navigationCon = [segue destinationViewController];
        WMEPreMeetingSettingViewController* preMeetingSettingViewController = [navigationCon viewControllers][0];
        preMeetingSettingViewController.delegate = self;
    }
}

- (void)DidFinishPreMeetingSetting
{
    [self dismissViewControllerAnimated:YES completion:nil];
}

@end
