//
//  WMEMariViewController.m
//  MediaSessionTest
//
//  Created by folkib on 3/10/16.
//  Copyright © 2016 Soya Li. All rights reserved.
//

#import "WMEMariViewController.h"
#import "PeerCall.h"
#import "Loopback.h"

@interface WMEMariViewController ()
@property (weak, nonatomic) IBOutlet UISwitch *swMariManualAdaptation;
@property (weak, nonatomic) IBOutlet UITextField *tfManualBandwidthKbps;

@end

@implementation WMEMariViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view.
    uint32_t Kbps = TestConfig::Instance().m_mauallyAdaptationBps/1000;
    [self.swMariManualAdaptation setOn: Kbps > 0];
    if (Kbps > 0) {
        NSString *strKbps = [NSString stringWithFormat:@"%u", Kbps];
        self.tfManualBandwidthKbps.text = strKbps;
    }
    self.tfManualBandwidthKbps.delegate = self;
    UITapGestureRecognizer *singleTap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(singleTapGestureCaptured:)];
    [self.view addGestureRecognizer:singleTap];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender {
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

- (void)checkManuallyAdaptation
{
    if ([self.swMariManualAdaptation isOn] == YES) {
        uint32_t Kbps = [[self.tfManualBandwidthKbps text] intValue];
        TestConfig::Instance().m_mauallyAdaptationBps = 1000*Kbps;
    } else {
        TestConfig::Instance().m_mauallyAdaptationBps = 0;
    }
    
    [self updateBandwidthMaually: TestConfig::Instance().m_mauallyAdaptationBps];
}

- (void)updateBandwidthMaually:(int)bandwidthInBps
{
    if (TestConfig::Instance().m_bLoopback) {
        LoopbackCall::Instance()->m_endCaller.setBandwidthMaually(bandwidthInBps);
    } else {
        PeerCall::Instance()->m_endCaller->setBandwidthMaually(bandwidthInBps);
    }
}

// MARK: text field delegate
- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    if (textField == self.tfManualBandwidthKbps) {
        [textField resignFirstResponder];
        [self checkManuallyAdaptation];
    }
    return YES;
}

- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
    return [self.swMariManualAdaptation isOn];
}

- (IBAction)ButtonSwitchManualBW:(id)sender {
    if ([[self.tfManualBandwidthKbps text] length] > 0) {
        [self checkManuallyAdaptation];
    }
}

- (void)singleTapGestureCaptured:(UITapGestureRecognizer *)gesture
{
    [self.tfManualBandwidthKbps resignFirstResponder];
    [self checkManuallyAdaptation];
}

@end
