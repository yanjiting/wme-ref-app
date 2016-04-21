//
//  WMEPeerWindowController.h
//  MediaSessionTest
//
//  Created by juntang on 6/5/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface WMEPeerWindowController : NSWindowController
{
    NSView *peerView;
}

@property (assign) IBOutlet NSView *peerView;

@end
