//
//  TaAsDummyAppDelegate.h
//  DummyApp
//
//  Created by vagouzhou on 8/11/14.
//  Copyright (c) 2014 wx2. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface TaAsDummyAppDelegate : NSObject <NSApplicationDelegate>
{
    NSWindow *fullscreenWindow;
    bool _fullScreen;
}
@property (assign) IBOutlet NSWindow *window;
@property (assign) IBOutlet NSImageView *imgView;

@end
