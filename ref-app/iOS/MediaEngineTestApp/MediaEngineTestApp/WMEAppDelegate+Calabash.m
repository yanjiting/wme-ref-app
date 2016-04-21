//
//  WMEAppDelegate+Calabash.m
//  MediaEngineTestApp
//
//  Created by zhaozhengchu on 6/4/14.
//  Copyright (c) 2014 video. All rights reserved.
//
#include <ifaddrs.h>
#include <arpa/inet.h>
#import "WMEAppDelegate+Calabash.h"
#include "pesq.h"
@implementation UIWindow (TAEnhance)

- (UIViewController *)visibleViewController {
    UIViewController *rootViewController = self.rootViewController;
    return [UIWindow getVisibleViewControllerFrom:rootViewController];
}

+ (UIViewController *) getVisibleViewControllerFrom:(UIViewController *) vc {
    if ([vc isKindOfClass:[UINavigationController class]]) {
        return [UIWindow getVisibleViewControllerFrom:[((UINavigationController *) vc) visibleViewController]];
    } else if ([vc isKindOfClass:[UITabBarController class]]) {
        return [UIWindow getVisibleViewControllerFrom:[((UITabBarController *) vc) selectedViewController]];
    } else {
        if (vc.presentedViewController) {
            return [UIWindow getVisibleViewControllerFrom:vc.presentedViewController];
        } else {
            return vc;
        }
    }
}

@end

@interface WMEAppDelegate ()

@end

@implementation WMEAppDelegate (Calabash)

- (NSString *)getIPAddress {
    
    NSString *address = @"error";
    struct ifaddrs *interfaces = NULL;
    struct ifaddrs *temp_addr = NULL;
    int success = 0;
    // retrieve the current interfaces - returns 0 on success
    success = getifaddrs(&interfaces);
    if (success == 0) {
        // Loop through linked list of interfaces
        temp_addr = interfaces;
        while(temp_addr != NULL) {
            if(temp_addr->ifa_addr->sa_family == AF_INET) {
                // Check if interface is en0 which is the wifi connection on the iPhone
                if([[NSString stringWithUTF8String:temp_addr->ifa_name] isEqualToString:@"en0"]) {
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

#pragma mark - backdoors

//=================== Common action ======================
- (NSString *)bdSetExtendParameter:(NSString *)extendParameterString
{
    self.extendParameter = extendParameterString;
    return @"successful";
}


- (NSString *)bdTouchUIControl:(NSString *)uiControlName
{
    UIViewController * topViewController = [[UIApplication sharedApplication].keyWindow visibleViewController];
    UIControl *control = (UIControl *)[topViewController valueForKey:uiControlName];
    
    [control sendActionsForControlEvents: UIControlEventTouchDown];
    return @"successful";
}


- (NSString *)bdTouchAlertView:(NSString *)clickButtonIndexString
{
    UIAlertView *topAlertController = [NSClassFromString(@"_UIAlertManager") performSelector:@selector(topMostAlert)];
    if (topAlertController.visible == YES)
    {
        [topAlertController dismissWithClickedButtonIndex:[clickButtonIndexString intValue] animated:YES];
    }
    return @"successful";
}

- (NSString *)bdGetAlertViewTitle:(NSString *)ignore
{
    UIAlertView *topAlertController = [NSClassFromString(@"_UIAlertManager") performSelector:@selector(topMostAlert)];
    if (topAlertController.visible == YES)
    {
        return [topAlertController title];
    }
    return @"";
}


- (NSString *)bdTouchBarButtonItem:(NSString *)barButtonNameString
{
    UIViewController * topViewController = [[UIApplication sharedApplication].keyWindow visibleViewController];
    UIBarButtonItem *button = (UIBarButtonItem *)[topViewController valueForKey:barButtonNameString];
    if (button.action != NULL) {
        [button.target performSelector:button.action withObject:button];
    }
    
    return @"successful";
}

- (NSString *)bdTouchTabBarItem:(NSString *)indexString
{
    UITabBarController *tabBarController = (UITabBarController *)([UIApplication sharedApplication].keyWindow.rootViewController);
    
    tabBarController.selectedIndex = [indexString intValue];
    return @"successful";
}
- (NSString *)bdTouchButton:(NSString *)buttonNameString
{
    UIViewController * topViewController = [[UIApplication sharedApplication].keyWindow visibleViewController];
    UIButton *button = (UIButton *)[topViewController valueForKey:buttonNameString];
    [button sendActionsForControlEvents: UIControlEventTouchUpInside];
    
    return @"successful";
}
- (NSString *)bdPerformSegue:(NSString *)segueNameString
{
    UIViewController * topViewController = [[UIApplication sharedApplication].keyWindow visibleViewController];
    [topViewController performSegueWithIdentifier:segueNameString sender:topViewController];
    return @"successful";
}

- (NSString *)bdTouchSwitch:(NSString *)switchNameString
{
    UIViewController * topViewController = [[UIApplication sharedApplication].keyWindow visibleViewController];
    UISwitch *switchButton = (UISwitch *)[topViewController valueForKey:switchNameString];
    
    if ([switchButton isOn] == YES)
        [switchButton setOn:NO];
    else
        [switchButton setOn:YES];
    
    [switchButton sendActionsForControlEvents: UIControlEventValueChanged];
    
    return @"successful";
}
//the backdoor depend on the bdSetExtendParameter (= segment value)
- (NSString *)bdTouchSegment:(NSString *)segmentNameString
{
    UIViewController * topViewController = [[UIApplication sharedApplication].keyWindow visibleViewController];
    UISegmentedControl *segment = (UISegmentedControl *)[topViewController valueForKey:segmentNameString];
    segment.selectedSegmentIndex = [self.extendParameter intValue];
    [segment sendActionsForControlEvents: UIControlEventValueChanged];
    return @"successful";
}
//the backdoor depend on the bdSetExtendParameter (= textfield content)
- (NSString *)bdSetTextField:(NSString *)textFieldNameString
{
    UIViewController * topViewController = [[UIApplication sharedApplication].keyWindow visibleViewController];
    UITextField *textField = (UITextField *)[topViewController valueForKey:textFieldNameString];
    textField.text = self.extendParameter;
    return @"successful";
}


- (NSString *)bdGetTextField:(NSString *)textFieldNameString
{
    UIViewController * topViewController = [[UIApplication sharedApplication].keyWindow visibleViewController];
    UITextField *textField = (UITextField *)[topViewController valueForKey:textFieldNameString];
    return (textField.text);
}

//the backdoor depend on the bdSetExtendParameter (= section number)
- (NSString *)bdTouchTableRow:(NSString *)rowString
{
    NSInteger row = [rowString intValue];
    NSInteger section = [self.extendParameter intValue];
    UITableViewController<UITableViewDelegate>* topViewController = (UITableViewController<UITableViewDelegate> *)([[UIApplication sharedApplication].keyWindow visibleViewController]);
    
    NSIndexPath *indexPath = [NSIndexPath indexPathForItem:row inSection:section];
    [topViewController tableView:topViewController.tableView didSelectRowAtIndexPath:indexPath];
    
    return @"successful";
}


//=================== customized action ======================
- (NSString *)bdCheckQulityOfAudioFile:(NSString *)fileName
{
    //self.extendParameter = @"1_16000_16_female1.pcm";
    //fileName = @"1_16000_16_female1_target.pcm";
    
    //Get the share fold path
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *sharingFolderPath = [paths objectAtIndex:0];
    
    NSString *inputFileName = [sharingFolderPath stringByAppendingPathComponent:self.extendParameter];
    NSString *outputFileName = [sharingFolderPath stringByAppendingPathComponent:fileName];
    char refFileName[1024]= {0};
    char degFileName[1024]= {0};
    
    memcpy(refFileName, [inputFileName UTF8String], [inputFileName length]);
    memcpy(degFileName, [outputFileName UTF8String], [outputFileName length]);

    float fRet;
    Get_pesq_mos(refFileName, degFileName, 16000, fRet);
    
    NSString *audioLevel = [NSString stringWithFormat:@"%f", fRet];
    
    return audioLevel;
}
@end

