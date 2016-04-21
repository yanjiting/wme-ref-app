//
//  ViewController.m
//  appshareUnitTest
//
//  Created by Aulion Huang on 5/9/14.
//  Copyright (c) 2014 webex. All rights reserved.
//

#import "ViewController.h"
#import "share_capturer.h"
#import "ios/ContentContainerViewController.h"

@interface ViewController()

@end

@implementation ViewController


-(void)CheckPasteBoard:(id)sender
{
    static time_t t = time(NULL);
    if (time(NULL) - t > 5) {
        captureContent->OpenPasteBoard();
    }
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    CShareCapturerContentiOS *pCaptureContent = (CShareCapturerContentiOS *)CreateShareCapture(SHARE_CAPTURER_TYPE_CONTENT);

    ContentContainerViewControllerEx *pController = (ContentContainerViewControllerEx *)pCaptureContent->GetContentContainerViewControllerEx();
    if (![pController isKindOfClass:[ContentContainerViewControllerEx class]]) {
        DestroyShareCapture(pCaptureContent);
        pController = NULL;
    } else {
        content = pController;
        captureContent = pCaptureContent;
        [self.view addSubview:pController.view];
        pController.view.frame = self.view.bounds;
        pController.view.autoresizingMask = UIViewAutoresizingFlexibleHeight|UIViewAutoresizingFlexibleWidth;


        NSString *file = [[NSBundle mainBundle] pathForResource:@"pobabies" ofType:@"jpg"];
        captureContent->OpenDocument([file UTF8String]);
    }

    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(CheckPasteBoard:) name:UIApplicationDidBecomeActiveNotification object:nil];
    //    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(CheckPasteBoard:) name:UIPasteboardChangedNotification object:nil];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

@end
