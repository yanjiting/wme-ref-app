//
//  WMEPreviewWindowController.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-8-15.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import "WMEPreviewWindowController.h"

@interface WMEPreviewWindowController ()

@end

@implementation WMEPreviewWindowController
@synthesize previewRenderWindow;

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

@end
