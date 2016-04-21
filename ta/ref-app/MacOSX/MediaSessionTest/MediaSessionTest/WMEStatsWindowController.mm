//
//  WMEStatsWindowController.m
//  MediaSessionTest
//
//  Created by juntang on 7/28/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#import "WMEStatsWindowController.h"

@interface WMEStatsWindowController ()

@end

@implementation WMEStatsWindowController
@synthesize txStatsInfo = _txStatsInfo;
@synthesize bPlayback = _bPlayback;

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
        _bPlayback = false;
    }
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];

    if (_bPlayback) {
        [[self window] setOpaque: NO];
        [[self window] setAlphaValue: 0.4];
        [self window].styleMask = NSResizableWindowMask | NSFullSizeContentViewWindowMask;

        [[self window] setLevel:NSScreenSaverWindowLevel + 1];
        [[self window] orderFront:nil];

        [[self window] center];
    }

    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
    m_statsInfo.setupCallback(self, @selector(OnGetInfo:));
}

-(void)OnGetInfo:(NSString *)statsInfo
{
    NSLog(@"%@\n", statsInfo);
    if(statsInfo)
        [_txStatsInfo setStringValue:statsInfo];
}

@end
