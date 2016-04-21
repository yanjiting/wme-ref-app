//
//  WMEPeerWindowController.m
//  MediaSessionTest
//
//  Created by juntang on 6/5/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#import "WMEPeerWindowController.h"

@interface WMEPeerWindowController ()

@end

@implementation WMEPeerWindowController
@synthesize peerView;

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
    
    [[self window] setTitle:@"Peer View"];
    
    NSPoint p = NSMakePoint(200, 1000);
    [[self window] setFrameTopLeftPoint:p];
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
}

@end
