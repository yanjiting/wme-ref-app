//
//  WMEStatisticsWindowController.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-9-5.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface WMEStatisticsWindowController : NSWindowController {
    NSTextField *videoInwardLossRatio;
    NSTextField *videoInwardRTT;
    NSTextField *videoInwardJitter;
    NSTextField *videoInwardPackets;
    NSTextField *videoInwardBytes;
    NSTextField *videoOutwardLossRatio;
    NSTextField *videoOutwardRTT;
    NSTextField *videoOutwardJitter;
    NSTextField *videoOutwardPackets;
    NSTextField *videoOutwardBytes;
    NSTextField *AudioInwardLossRatio;
    NSTextField *AudioInwardRTT;
    NSTextField *AudioInwardJitter;
    NSTextField *AudioInwardPackets;
    NSTextField *AudioInwardBytes;
    NSTextField *AudioOutwardLossRatio;
    NSTextField *AudioOutwardRTT;
    NSTextField *AudioOutwardJitter;
    NSTextField *AudioOutwardPackets;
    NSTextField *AudioOutwardBytes;
    NSTextField *networkIndexUplink;
    NSTextField *networkIndexDownlink;
    NSTextField *networkIndexBothlink;
    NSWindow *statisticsWindow;
    NSArray *indexColor;
}

@property (assign) IBOutlet NSWindow *statisticsWindow;

@property (assign) IBOutlet NSTextField *videoInwardLossRatio;
@property (assign) IBOutlet NSTextField *videoInwardRTT;
@property (assign) IBOutlet NSTextField *videoInwardJitter;
@property (assign) IBOutlet NSTextField *videoInwardPackets;
@property (assign) IBOutlet NSTextField *videoInwardBytes;
@property (assign) IBOutlet NSTextField *videoOutwardLossRatio;
@property (assign) IBOutlet NSTextField *videoOutwardRTT;
@property (assign) IBOutlet NSTextField *videoOutwardJitter;
@property (assign) IBOutlet NSTextField *videoOutwardPackets;
@property (assign) IBOutlet NSTextField *videoOutwardBytes;
@property (assign) IBOutlet NSTextField *AudioInwardLossRatio;
@property (assign) IBOutlet NSTextField *AudioInwardRTT;
@property (assign) IBOutlet NSTextField *AudioInwardJitter;
@property (assign) IBOutlet NSTextField *AudioInwardPackets;
@property (assign) IBOutlet NSTextField *AudioInwardBytes;
@property (assign) IBOutlet NSTextField *AudioOutwardLossRatio;
@property (assign) IBOutlet NSTextField *AudioOutwardRTT;
@property (assign) IBOutlet NSTextField *AudioOutwardJitter;
@property (assign) IBOutlet NSTextField *AudioOutwardPackets;
@property (assign) IBOutlet NSTextField *AudioOutwardBytes;
@property (assign) IBOutlet NSTextField *networkIndexUplink;
@property (assign) IBOutlet NSTextField *networkIndexDownlink;
@property (assign) IBOutlet NSTextField *networkIndexBothlink;
@end
