//
//  WMEDumpWindowController.h
//  MediaEngineTestApp
//
//  Created by hfe-vscm-mac on 3/16/14.
//  Copyright (c) 2014 video. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface WMEDumpWindowController : NSWindowController {
    NSButton *dumpCaptureDataCBX;
    NSButton *dumpEncodeToRTPDataCBX;
    NSButton *dumpNALToListenChannelDataCBX;
    NSButton *dumpNALToDecodeDataCBX;
    NSButton *dumpAfterDecodeDataCBX;
    NSButton *startDumpBTN;
}

@property (assign) IBOutlet NSButton *dumpCaptureDataCBX;
@property (assign) IBOutlet NSButton *dumpEncodeToRTPDataCBX;
@property (assign) IBOutlet NSButton *dumpNALToListenChannelDataCBX;
@property (assign) IBOutlet NSButton *dumpNALToDecodeDataCBX;
@property (assign) IBOutlet NSButton *dumpAfterDecodeDataCBX;

@property (assign) IBOutlet NSButton *startDumpBTN;
- (IBAction)buttonStartDump:(id)sender;
@end
