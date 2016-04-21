//
//  ViewController.h
//  appshareUnitTest
//
//  Created by Aulion Huang on 5/9/14.
//  Copyright (c) 2014 webex. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "ios/share_capture_content_ios.h"

@interface ViewController : UIViewController
{
    CShareCapturerContentiOS *captureContent;
    UIViewController *content;
}

@end
