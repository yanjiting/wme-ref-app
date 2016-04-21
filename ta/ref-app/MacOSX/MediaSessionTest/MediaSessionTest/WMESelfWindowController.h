//
//  WMESelfWindowController.h
//  MediaSessionTest
//
//  Created by juntang on 6/5/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface WMESelfWindowController : NSWindowController
{
    NSView *selfView;
    NSView *peerView;
    NSMutableArray *remoteViews;
}

@property (assign) IBOutlet NSView *selfView;
@property (assign) IBOutlet NSView *peerView;
@property (assign) BOOL bPlayback;

- (NSView *)createThumbnail:(CGFloat)nextY;

- (void)ViewMuted:(BOOL)muted mutedLayer:(CALayer *) layer;
- (void)ViewShow:(BOOL)show showLayer:(CALayer *) layer;

@end
