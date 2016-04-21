//
//  WMEStatisticsWindowController.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-9-5.
//  Copyright (c) 2013年 video. All rights reserved.
//
#import "WMEDataProcess.h"
#import "WMEStatisticsWindowController.h"
extern WMEDataProcess *g_wmeDataProcess;
@interface WMEStatisticsWindowController ()

@end

@implementation WMEStatisticsWindowController
@synthesize statisticsWindow;
@synthesize videoInwardLossRatio;
@synthesize videoInwardRTT;
@synthesize videoInwardJitter;
@synthesize videoInwardPackets;
@synthesize videoInwardBytes;
@synthesize videoOutwardLossRatio;
@synthesize videoOutwardRTT;
@synthesize videoOutwardJitter;
@synthesize videoOutwardPackets;
@synthesize videoOutwardBytes;
@synthesize AudioInwardLossRatio;
@synthesize AudioInwardRTT;
@synthesize AudioInwardJitter;
@synthesize AudioInwardPackets;
@synthesize AudioInwardBytes;
@synthesize AudioOutwardLossRatio;
@synthesize AudioOutwardRTT;
@synthesize AudioOutwardJitter;
@synthesize AudioOutwardPackets;
@synthesize AudioOutwardBytes;
@synthesize networkIndexUplink;
@synthesize networkIndexDownlink;
@synthesize networkIndexBothlink;


+ (NSColor*)colorWithHexColorString:(NSString*)inColorString
{
    NSColor* result = nil;
    unsigned colorCode = 0;
    unsigned char redByte, greenByte, blueByte;
    
    if (nil != inColorString)
    {
        NSScanner* scanner = [NSScanner scannerWithString:inColorString];
        (void) [scanner scanHexInt:&colorCode]; // ignore error
    }
    redByte = (unsigned char)(colorCode >> 16);
    greenByte = (unsigned char)(colorCode >> 8);
    blueByte = (unsigned char)(colorCode); // masks off high bits
    
    result = [NSColor
              colorWithCalibratedRed:(CGFloat)redByte / 0xff
              green:(CGFloat)greenByte / 0xff
              blue:(CGFloat)blueByte / 0xff
              alpha:1.0];
    return result;
}

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
        indexColor = [NSArray arrayWithObjects:
                      [WMEStatisticsWindowController colorWithHexColorString:@"0xffffff"],
                      [WMEStatisticsWindowController colorWithHexColorString:@"0xC30C20"],
                      [WMEStatisticsWindowController colorWithHexColorString:@"0xE9331D"],
                      [WMEStatisticsWindowController colorWithHexColorString:@"0xFF8E10"],
                      [WMEStatisticsWindowController colorWithHexColorString:@"0xFDF10B"],
                      [WMEStatisticsWindowController colorWithHexColorString:@"0xA0BA94"],
                      [WMEStatisticsWindowController colorWithHexColorString:@"0x89AC2D"],
                      [WMEStatisticsWindowController colorWithHexColorString:@"0x36A238"],
                      nil];
        [indexColor retain];
    }
    
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
    
    //[videoInwardLossRatio.
}

- (void)awakeFromNib
{
    [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(updateStatistics) userInfo:nil repeats:YES];
}

- (void)updateStatistics
{
    WmeSessionStatistics wmeStatistics;

    //static float test =10;
    //[videoInwardLossRatio setStringValue:[NSString stringWithFormat:@"%f", test++]];

    [g_wmeDataProcess getVideoStatistics:wmeStatistics];
    [videoInwardLossRatio setStringValue:[NSString stringWithFormat:@"%f", wmeStatistics.stInNetworkStat.fLossRatio]];
    [videoInwardJitter setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stInNetworkStat.uJitter]];
    [videoInwardRTT setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stInNetworkStat.uRoundTripTime]];
    [videoInwardPackets setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stInNetworkStat.uPackets]];
    [videoInwardBytes setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stInNetworkStat.uBytes]];
    
    [videoOutwardLossRatio setStringValue:[NSString stringWithFormat:@"%f", wmeStatistics.stOutNetworkStat.fLossRatio]];
    [videoOutwardJitter setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stOutNetworkStat.uJitter]];
    [videoOutwardRTT setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stOutNetworkStat.uRoundTripTime]];
    [videoOutwardPackets setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stOutNetworkStat.uPackets]];
    [videoOutwardBytes setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stOutNetworkStat.uBytes]];
 
    [g_wmeDataProcess getAudioStatistics:wmeStatistics];
    [AudioInwardLossRatio setStringValue:[NSString stringWithFormat:@"%f", wmeStatistics.stInNetworkStat.fLossRatio]];
    [AudioInwardJitter setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stInNetworkStat.uJitter]];
    [AudioInwardRTT setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stInNetworkStat.uRoundTripTime]];
    [AudioInwardPackets setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stInNetworkStat.uPackets]];
    [AudioInwardBytes setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stInNetworkStat.uBytes]];
    
    [AudioOutwardLossRatio setStringValue:[NSString stringWithFormat:@"%f", wmeStatistics.stOutNetworkStat.fLossRatio]];
    [AudioOutwardJitter setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stOutNetworkStat.uJitter]];
    [AudioOutwardRTT setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stOutNetworkStat.uRoundTripTime]];
    [AudioOutwardPackets setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stOutNetworkStat.uPackets]];
    [AudioOutwardBytes setStringValue:[NSString stringWithFormat:@"%d", wmeStatistics.stOutNetworkStat.uBytes]];
    
    WmeNetworkIndex idx;
    [g_wmeDataProcess getNetworkIndex:idx Direction:DIRECTION_UPLINK];
    [networkIndexUplink setStringValue:[NSString stringWithFormat:@"%d", idx]];
    NSColor * color = [indexColor objectAtIndex:idx];
    [networkIndexUplink setBackgroundColor:color];

    [g_wmeDataProcess getNetworkIndex:idx Direction:DIRECTION_DOWNLINK];
    [networkIndexDownlink setStringValue:[NSString stringWithFormat:@"%d", idx]];
    color = [indexColor objectAtIndex:idx];
    [networkIndexDownlink setBackgroundColor:color];

    [g_wmeDataProcess getNetworkIndex:idx Direction:DIRECTION_BOTHLINK];
    [networkIndexBothlink setStringValue:[NSString stringWithFormat:@"%d", idx]];
    color = [indexColor objectAtIndex:idx];
    [networkIndexBothlink setBackgroundColor:color];


#ifdef ENABLE_COMMAND_LINE
    if (g_wmeDataProcess.clInfo->IsSyslogEnabled() == false) {
        return;
    }
    //Log out the video/audio statistics
    WmeVideoStatistics wmeVideoStatistics;

    [g_wmeDataProcess getVideoStatistics:wmeVideoStatistics Track:DEMO_LOCAL_TRACK];
    g_wmeDataProcess.logger->log("video-out:%u,%u,%u,%u,%u,%u,%u,%.02f",
                                 static_cast<unsigned int>(wmeVideoStatistics.stNetworkStat.fLossRatio * 100),
                                 wmeVideoStatistics.stNetworkStat.uRoundTripTime,
                                 wmeVideoStatistics.stNetworkStat.uJitter,
                                 wmeVideoStatistics.stNetworkStat.uBytes,
                                 wmeVideoStatistics.stNetworkStat.uPackets,
                                 wmeVideoStatistics.uWidth,
                                 wmeVideoStatistics.uHeight,
                                 wmeVideoStatistics.fFrameRate);
    
    [g_wmeDataProcess getVideoStatistics:wmeVideoStatistics Track:DEMO_REMOTE_TRACK];
    g_wmeDataProcess.logger->log("video-in:%u,%u,%u,%u,%u,%u,%u,%.02f",
                                 static_cast<unsigned int>(wmeVideoStatistics.stNetworkStat.fLossRatio * 100),
                                 wmeVideoStatistics.stNetworkStat.uRoundTripTime,
                                 wmeVideoStatistics.stNetworkStat.uJitter,
                                 wmeVideoStatistics.stNetworkStat.uBytes,
                                 wmeVideoStatistics.stNetworkStat.uPackets,
                                 wmeVideoStatistics.uWidth,
                                 wmeVideoStatistics.uHeight,
                                 wmeVideoStatistics.fFrameRate);
    
    
    WmeAudioStatistics wmeAudioStatistics;
    
    [g_wmeDataProcess getAudioStatistics:wmeAudioStatistics Track:DEMO_LOCAL_TRACK];
    g_wmeDataProcess.logger->log("audio-out:%u,%u,%u,%u,%u",
                                         static_cast<unsigned int>(wmeAudioStatistics.stNetworkStat.fLossRatio * 100),
                                         wmeAudioStatistics.stNetworkStat.uRoundTripTime,
                                         wmeAudioStatistics.stNetworkStat.uJitter,
                                         wmeAudioStatistics.stNetworkStat.uBytes,
                                         wmeAudioStatistics.stNetworkStat.uPackets
                                         );
    
    [g_wmeDataProcess getAudioStatistics:wmeAudioStatistics Track:DEMO_REMOTE_TRACK];
    g_wmeDataProcess.logger->log("audio-in:%u,%u,%u,%u,%u",
                                 static_cast<unsigned int>(wmeAudioStatistics.stNetworkStat.fLossRatio * 100),
                                 wmeAudioStatistics.stNetworkStat.uRoundTripTime,
                                 wmeAudioStatistics.stNetworkStat.uJitter,
                                 wmeAudioStatistics.stNetworkStat.uBytes,
                                 wmeAudioStatistics.stNetworkStat.uPackets
                                 );
#endif
    
}

@end
