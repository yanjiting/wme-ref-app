//
//  WMEAppDelegate.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-8-5.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "WMEDataProcess.h"
#import "WMESelfViewWindowController.h"
#import "WMEAttendeeViewWindowController.h"
#import "WMEPreviewWindowController.h"

@interface WMEAppDelegate : NSObject <NSApplicationDelegate, NSTextFieldDelegate, NSComboBoxDataSource, NSComboBoxDelegate, NSMatrixDelegate, UIChangeNotifierDelegate>
{
    NSWindow *window;

    BOOL audioCaptureVolumeMuteState;
    BOOL audioPlayVolumeMuteState;
    BOOL actionPreviewState;
    
    NSMutableDictionary *_dictionaryState;
    WMEDataProcess *_wmeDataProcess;
    WMESelfViewWindowController *_wmeSelfViewWindowController;
    WMEAttendeeViewWindowController *_wmeAttendeeViewWindowController;
    WMEAttendeeViewWindowController *_wmeScreenAttendeeViewWindowController;
    WMEPreviewWindowController *_wmePreviewWindowController;
    
    NSComboBox *_videoCaptureDeviceCBX;
    NSComboBox *_videoEncodingParamCBX;
    
    NSSlider *_audioCaptureVolumeSLD;
    NSSlider *_audioPlayVolumeSLD;
    NSComboBox *_audioCaptureDeviceCBX;
    NSComboBox *_audioEncodingParamCBX;
    NSComboBox *_audioPlayDeviceCBX;
    
    NSButton *_networkEnableNATCBX;
    NSTextField *_networkJINGLEServerIPTF;
    NSTextField *_networkJINGLEServerPortTF;
    NSTextField *_networkSTUNServerIPTF;
    NSTextField *_networkSTUNServerPortTF;
    
    NSMatrix *_roleHostAndClientMTX;
    NSTextField *_roleHostIPTF;
    NSTextField *_roleMyNameTF;
    NSTextField *_roleHostNameTF;
    
    NSButton *_actionSendVideoCBX;
    NSButton *_actionSendAudioCBX;
    NSButton *_actionConnectBTN;
    NSButton *_actionDisconnectBTN;
    NSButton *_audioCaptureVolumeMuteBTN;
    NSButton *_audioPlayVolumeMuteBTN;
    NSButton *_actionPreviewBTN;
    NSComboBox *_actionMaxTraceLevelCBX;
    NSButton *_videoEnableVideoCBX;
    NSButton *_audioEnableAudioCBX;
    
    NSMatrix *_screenRenderModeMTX;
    NSButton *_screenEnableCBX;
    IBOutlet NSComboBox *_shareList;
    
    NSButton *_videoOptionButton;
    NSDrawer *_videoOptionDrawer;
    NSTextField *_videoInputFileLabel;
    NSTextField *_videoOutputFileLabel;
    
    NSButton *_audioOptionButton;
    NSDrawer *_audioOptionDrawer;
    NSTextField *_audioInputFileLabel;
    NSTextField *_audioOutputFileLabel;
}



//Default
@property (assign) IBOutlet NSWindow *window;

//Screen Setting
@property (weak) IBOutlet NSMatrix *screenRenderModeMTX;
@property (weak) IBOutlet NSButton *screenEnableCBX;
- (IBAction)checkboxEnableScreen:(id)sender;
- (IBAction)matrixRenderModeScreenSharing:(id)sender;

//Video Setting
@property (weak) IBOutlet NSButton *videoEnableVideoCBX;
@property (retain) IBOutlet NSComboBox *videoCaptureDeviceCBX;
@property (weak) IBOutlet NSComboBox *videoEncodingParamCBX;
@property (weak) IBOutlet NSButton *videoOptionButton;
@property (weak) IBOutlet NSDrawer *videoOptionDrawer;
@property (weak) IBOutlet NSTextField *videoInputFileLabel;
@property (weak) IBOutlet NSTextField *videoOutputFileLabel;
- (IBAction)checkboxEnableVideo:(id)sender;
- (IBAction)selectInputVideoFile:(id)sender;
- (IBAction)clearInputVideoFile:(id)sender;
- (IBAction)setOutputVideoFile:(id)sender;
- (IBAction)clearOutputVideoFile:(id)sender;

//Audio Setting
@property (weak) IBOutlet NSButton *audioEnableAudioCBX;
@property (weak) IBOutlet NSComboBox *audioCaptureDeviceCBX;
@property (weak) IBOutlet NSSlider *audioCaptureVolumeSLD;
@property (weak) IBOutlet NSComboBox *audioEncodingParamCBX;
@property (weak) IBOutlet NSComboBox *audioPlayDeviceCBX;
@property (weak) IBOutlet NSSlider *audioPlayVolumeSLD;
@property (assign) IBOutlet NSButton *audioCaptureVolumeMuteBTN;
@property (assign) IBOutlet NSButton *audioPlayVolumeMuteBTN;
@property (weak) IBOutlet NSButton *audioOptionButton;
@property (weak) IBOutlet NSDrawer *audioOptionDrawer;
@property (weak) IBOutlet NSTextField *audioInputFileLabel;
@property (weak) IBOutlet NSTextField *audioOutputFileLabel;
- (IBAction)sliderAudioCaptureVolume:(id)sender;
- (IBAction)sliderAudioPlayVolume:(id)sender;
- (IBAction)buttonAudioCaptureVolumeMute:(id)sender;
- (IBAction)buttonAudioPlayVolumeMute:(id)sender;
- (IBAction)checkboxEnableAudio:(id)sender;
- (IBAction)selectInputAudioFile:(id)sender;
- (IBAction)clearInputAudioFile:(id)sender;
- (IBAction)setOutputAudioFile:(id)sender;
- (IBAction)clearOutputAudioFile:(id)sender;

//Network Setting
@property (weak) IBOutlet NSButton *networkEnableNATCBX;
@property (weak) IBOutlet NSTextField *networkJINGLEServerIPTF;
@property (weak) IBOutlet NSTextField *networkJINGLEServerPortTF;
@property (weak) IBOutlet NSTextField *networkSTUNServerIPTF;
@property (weak) IBOutlet NSTextField *networkSTUNServerPortTF;
- (IBAction)checkboxEnableNAT:(id)sender;

//Role Setting
@property (weak) IBOutlet NSMatrix *roleHostAndClientMTX;
@property (weak) IBOutlet NSTextField *roleHostIPTF;
@property (weak) IBOutlet NSTextField *roleMyNameTF;
@property (weak) IBOutlet NSTextField *roleHostNameTF;
- (IBAction)matrixHostOrClient:(id)sender;

//Action setting
@property (weak) IBOutlet NSButton *actionSendAudioCBX;
@property (weak) IBOutlet NSButton *actionSendVideoCBX;
@property (assign) IBOutlet NSButton *actionConnectBTN;
@property (assign) IBOutlet NSButton *actionDisconnectBTN;
@property (assign) IBOutlet NSButton *actionPreviewBTN;
- (IBAction)buttonConnect:(id)sender;
- (IBAction)buttonDisconnect:(id)sender;
- (IBAction)checkboxSendVideo:(id)sender;
- (IBAction)checkboxSendAudio:(id)sender;
- (IBAction)buttonPreview:(id)sender;
@property (assign) IBOutlet NSComboBox *actionMaxTraceLevelCBX;



@property (retain) WMEDataProcess *wmeDataProcess;
@property (retain) NSMutableDictionary *dictionaryState;
@property (retain) WMESelfViewWindowController *wmeSelfViewWindowController;
@property (retain) WMEAttendeeViewWindowController *wmeAttendeeViewWindowController;
@property (retain) WMEAttendeeViewWindowController *wmeScreenAttendeeViewWindowController;
@property (retain) WMEPreviewWindowController *wmePreviewWindowController;
@end
