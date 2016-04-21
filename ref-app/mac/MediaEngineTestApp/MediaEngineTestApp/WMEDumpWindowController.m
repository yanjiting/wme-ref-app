//
//  WMEDumpWindowController.m
//  MediaEngineTestApp
//
//  Created by hfe-vscm-mac on 3/16/14.
//  Copyright (c) 2014 video. All rights reserved.
//
#import "WMEDataProcess.h"
#import "WMEDumpWindowController.h"
extern WMEDataProcess *g_wmeDataProcess;

@interface WMEDumpWindowController ()

@end

@implementation WMEDumpWindowController
@synthesize dumpCaptureDataCBX;
@synthesize dumpEncodeToRTPDataCBX;
@synthesize dumpNALToListenChannelDataCBX;
@synthesize dumpNALToDecodeDataCBX;
@synthesize dumpAfterDecodeDataCBX;
@synthesize startDumpBTN;

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

- (IBAction)buttonStartDump:(id)sender {
    
    NSString *titleName = [self.startDumpBTN title];
    
    if ([titleName isEqualToString:@"Stop Dump Data"]) {
        [self.startDumpBTN setTitle:@"Start Dump Data"];
        [g_wmeDataProcess SetDumpDataEnabled:0];
    }
    else
    {
        [self.startDumpBTN setTitle:@"Stop Dump Data"];
        [g_wmeDataProcess SetDumpDataEnabled:
         ([dumpCaptureDataCBX state] == TRUE ? WME_DATA_DUMP_VIDEO_RAW_CAPTURE : 0) |
         ([dumpEncodeToRTPDataCBX state]  == TRUE ? WME_DATA_DUMP_VIDEO_ENCODE_RTP_LAYER : 0) |
         ([dumpNALToListenChannelDataCBX state]  == TRUE ? WME_DATA_DUMP_VIDEO_NAL_TO_LISTEN_CHANNEL : 0) |
         ([dumpNALToDecodeDataCBX state]  == TRUE ? WME_DATA_DUMP_VIDEO_NAL_TO_DECODER : 0) |
         ([dumpAfterDecodeDataCBX state]  == TRUE ? WME_DATA_DUMP_VIDEO_RAW_AFTER_DECODE_TO_RENDER : 0)];
    }
    


}
@end
