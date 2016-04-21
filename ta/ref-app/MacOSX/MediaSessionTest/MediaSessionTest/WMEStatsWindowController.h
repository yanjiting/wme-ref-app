//
//  WMEStatsWindowController.h
//  MediaSessionTest
//
//  Created by juntang on 7/28/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "StatsInfoWrapper.h"

@interface WMEStatsWindowController : NSWindowController
{
    StatsInfoWrapper m_statsInfo;
    NSTextField *_txStatsInfo;
}
@property (assign) IBOutlet NSTextField *txStatsInfo;
@property (assign) BOOL bPlayback;

-(void)OnGetInfo: (NSString *)StatsInfo;


@end
