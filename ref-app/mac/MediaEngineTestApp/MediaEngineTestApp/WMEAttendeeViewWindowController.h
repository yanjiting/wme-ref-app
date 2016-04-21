//
//  WMEAttendeeViewWindowController.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-8-15.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface WMEAttendeeViewWindowController : NSWindowController {
    NSView *attendeeViewRenderWindow;
}


@property (assign) IBOutlet NSView *attendeeViewRenderWindow;
@end
