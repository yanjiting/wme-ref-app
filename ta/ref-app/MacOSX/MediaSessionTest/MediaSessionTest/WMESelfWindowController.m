//
//  WMESelfWindowController.m
//  MediaSessionTest
//
//  Created by juntang on 6/5/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#import "WMESelfWindowController.h"

@interface WMESelfWindowController ()

@end

@implementation WMESelfWindowController
@synthesize selfView, peerView;
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
    
    [[self window] setTitle:@"Video Window"];
    
    NSPoint p = NSMakePoint(100, 500);
    [[self window] setFrameTopLeftPoint:p];
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
    
    [self.window.contentView addSubview:peerView];
    [self.window.contentView addSubview:selfView];

    if (_bPlayback) {
        NSRect screenRect = [[NSScreen mainScreen] frame];
        [self.window setFrame:screenRect display:YES];
    }

    remoteViews = [[NSMutableArray alloc] init];
    [remoteViews addObject: peerView];
}

- (NSView *)createThumbnail:(CGFloat)nextY
{
    NSView *thumbnail = [[NSView alloc] initWithFrame:NSMakeRect(13,8 + nextY,160,90)];
    thumbnail.layer = [CALayer layer];
    thumbnail.wantsLayer = YES;
    [self.window.contentView addSubview:thumbnail];
    [remoteViews addObject: thumbnail];
    return thumbnail;
}

- (void)ViewMuted:(BOOL)muted mutedLayer:(CALayer *) layer
{
    NSView *selectedView = NULL;
    for (NSView* view in remoteViews) {
        if(view.layer == layer) {
            selectedView = view;
            break;
        }
    }
    NSView * lastView = selectedView.subviews.lastObject;
    if(muted)
    {
        if(![lastView.toolTip isEqualToString: @"Unavailable"])
        {
            CGPoint center = CGPointMake(NSMidX(NSRectFromCGRect(layer.frame)), NSMidY(NSRectFromCGRect(layer.frame)));
            NSTextField *mutedView = [[NSTextField alloc] initWithFrame:NSMakeRect(center.x-40, center.y-10, 80, 20)];
            [mutedView setStringValue:@"Unavailable"];
            [mutedView setBezeled:NO];
            [mutedView setDrawsBackground:NO];
            [mutedView setEditable:NO];
            [mutedView setSelectable:NO];
            [mutedView setToolTip:@"Unavailable"];
            [mutedView setTextColor:[NSColor redColor]];
            mutedView.layer = [CALayer layer];
            mutedView.wantsLayer = YES;
            [mutedView.layer setBackgroundColor:[[NSColor clearColor] CGColor]];
            [selectedView addSubview:mutedView];
        }
    }
    else
    {
        if([lastView.toolTip isEqualToString: @"Unavailable"])
        {
            [lastView removeFromSuperview];
            [lastView release];
        }
    }
}

- (void)ViewShow:(BOOL)show showLayer:(CALayer *) layer
{
    NSView *selectedView = NULL;
    for (NSView* view in remoteViews) {
        if(view.layer == layer) {
            selectedView = view;
            break;
        }
    }
    selectedView.hidden = !show;
}

@end
