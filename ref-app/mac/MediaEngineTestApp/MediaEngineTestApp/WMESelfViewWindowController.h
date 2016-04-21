//
//  WMESelfViewWindowController.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-8-8.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface WMESelfViewWindowController : NSWindowController {
    NSView *selfViewRenderWindow;
}


@property (assign) IBOutlet NSView *selfViewRenderWindow;
@end
