//
//  AppDelegate.h
//  MediaSessionTest
//
//  Created by juntang on 6/4/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#import <Cocoa/Cocoa.h>
//#import "WMEDataProcess.h"
#import "WMESelfWindowController.h"
#import "WMEPeerWindowController.h"
#import "WMEStatsWindowController.h"
#import "ProxyAuthenticationDialog.h"
#include "BackdoorAgent.h"
#include "MediaConnection.h"
#include "CSILog.h"
#include <string>
#include <vector>
class WMETraceSink;
class CAppSink;

@interface AppDelegate : NSObject <NSApplicationDelegate>
{
    NSWindow *window;
    
    WMESelfWindowController *_wmeSelfWindowController;
    ProxyAuthenticationDialog *_proxyAuthenticationDlg;
    NSButton *_isASCBX;
    NSButton * _isDpcCBX;
    NSButton *_isShowASPrviewCBX;
    NSButton *_isASHostCBX;
    NSButton *_isVideoCBX;
    NSButton *_isAudioCBX;
    NSButton *_isEnableQoSCBX;
    NSButton *_isVideoHWCBX;
    NSButton *_isEnableFecCBX;
    NSButton *_isMultiStreamCBX;
    NSButton *_isMultiAudioCBX;
    NSButton *_isNoSignal;
    NSButton *_isEnableDtlsSrtpCBX;
    NSButton *_isEnableCVOCBX;
    NSButton *_isEnableAudioDeviceSwitchMicCBX;
    NSButton *_isEnableAudioDeviceSwitchSpeakerCBX;
    
    IBOutlet NSComboBox *_shareList;
    IBOutlet NSComboBox *_viewsList;
    IBOutlet NSComboBox *_AECTypeList;
    WMEPeerWindowController *_wmePeerWindowController;
    WMEPeerWindowController *_wmePeerWindowControllerScreenShare;
    WMEPeerWindowController *_wmePeerWindowControllerScreenPreview;
    WMEStatsWindowController *_wmeStatsWindowController;
 //   WMEDataProcess *_wmeDataProcess;
    
    NSButton *_actionConnectBTN;
    NSButton *_actionVideoBTN;
    NSButton *_actionShareBTN;
    NSButton *_actionAudioBTN;
    NSButton *_statsButton;
    
    NSTextField *_labelHostIP;
    NSTextField *_labelLinus;
    NSTextField *_tfhostIP;
    NSTextField *_tfLinus;
    NSTextField *_tfVenueUrl;
    NSButton *_isLoopCBX;
    NSButton *_isCalliopeCBX;
    
    bool isConnected;
    bool isAudioMute;
    bool isVideoMute;
    bool isScreenMute;
    bool isInSvsMode;
    std::string m_strScreenSourceID;
    
    WMETraceSink* _traceSink;
    CAppSink* _appSink;
    
    NSMatrix *_screenRenderModeMTX;
    
    NSDrawer *_audioOptionDrawer;
    
    NSMutableArray *_arrayAudioCaptureDeviceCBX;
    NSMutableArray *_arrayAudioPlayDeviceCBX;
    
    std::vector<std::string> _vectorCaptureDevice;
    std::vector<std::string> _vectorPalyDevice;
    
    NSComboBox *_audioCaptureDeviceCBX;
    NSComboBox *_audioPlayDeviceCBX;
    NSInteger _audioCaptureIndex;
    NSInteger _audioPlayIndex;
    
    NSSlider *_audioPlayDeviceSlider;
    NSSlider *_audioCaptureDeviceSlider;
    NSButton *_audioPlayDeviceMuteBox;
    NSButton *_audioCaptureDeviceMuteBox;
    
    BOOL _bSpeakerMute;
    BOOL _bMicrophoneMute;
}

@property (assign) IBOutlet NSWindow *window;
@property (retain) WMEPeerWindowController *wmePeerWindowController;
@property (retain) WMEPeerWindowController *wmePeerWindowControllerScreenShare;
@property (retain) WMEPeerWindowController *wmePeerWindowControllerScreenPreview;
@property (retain) WMESelfWindowController *wmeSelfWindowController;
@property (assign) IBOutlet NSButton *isASCBX;
@property (assign) IBOutlet NSButton *isDpcCBX;
@property (assign) IBOutlet NSButton *isShowASPrviewCBX;
@property (assign) IBOutlet NSButton *isASHostCBX;
@property (assign) IBOutlet NSButton *isVideoCBX;
@property (assign) IBOutlet NSButton *isAudioCBX;
@property (assign) IBOutlet NSButton *isEnableQoSCBX;
@property (assign) IBOutlet NSButton *isVideoHWCBX;
@property (assign) IBOutlet NSButton *isEnableFecCBX;
@property (assign) IBOutlet NSButton *isMultiStreamCBX;
@property (assign) IBOutlet NSButton *isNoSignal;
@property (assign) IBOutlet NSButton *isEnableCVOCBX;
@property (assign) IBOutlet NSButton *isEnableAudioDeviceSwitchMicCBX;
@property (assign) IBOutlet NSButton *isEnableAudioDeviceSwitchSpeakerCBX;

@property (assign) IBOutlet NSButton *actionConnectBTN;
@property (assign) IBOutlet NSButton *actionAudioBTN;
@property (assign) IBOutlet NSButton *actionVideoBTN;
@property (assign) IBOutlet NSButton *actionShareBTN;
@property (assign) IBOutlet NSButton *actionQuitSVSBTN;
@property (assign) IBOutlet NSButton *isLoopCBX;
@property (assign) IBOutlet NSButton *isCalliopeCBX;
@property (weak) IBOutlet NSTextField *tfHostIP;
@property (weak) IBOutlet NSTextField *tfLinus;
@property (weak) IBOutlet NSTextField *labelHostIP;
@property (weak) IBOutlet NSTextField *labelLinus;
@property (weak) IBOutlet NSMatrix *screenRenderModeMTX;
@property (weak) IBOutlet NSTextField *tfVenueUrl;

@property (assign) IBOutlet NSButton*  audioInfoBtn;
@property (weak) IBOutlet NSDrawer *audioOptionDrawer;
@property (assign) IBOutlet NSComboBox *audioCaptureDeviceCBX;
@property (assign) IBOutlet NSComboBox *audioPlayDeviceCBX;
@property (assign) IBOutlet NSSlider *audioPlayDeviceSlider;
@property (assign) IBOutlet NSSlider *audioCaptureDeviceSlider;
@property (assign) IBOutlet NSButton *audioPlayDeviceMuteBox;
@property (assign) IBOutlet NSButton *audioCaptureDeviceMuteBox;

@property (assign) IBOutlet NSButton *mariManualAdaptation;
@property (assign) IBOutlet NSTextField *ManualBandwidthKbps;

- (IBAction)SetSpeakerVolAction:(id)sender;
- (IBAction)SetMicVolAction:(id)sender;
- (IBAction)MuteSpeakerAction:(id)sender;
- (IBAction)MuteMicAction:(id)sender;

- (IBAction)matrixRenderModeScreenSharing:(id)sender;
- (IBAction)checkboxAS:(id)sender;
- (IBAction)checkboxASHost:(id)sender;
- (IBAction)checkboxASPreview:(id)sender;
- (IBAction)checkboxShareFilter:(id)sender;
- (IBAction)checkboxDtlsSrtp:(id)sender;
- (IBAction)checkboxEnableCVO:(id)sender;
- (IBAction)MuteAudio:(id)sender;
- (IBAction)checkboxMultiStream:(id)sender;
- (IBAction)checkboxNoSignal:(id)sender;

- (IBAction)buttonConnect:(id)sender;
- (IBAction)checkboxLoop:(id)sender;
- (IBAction)checkboxCalliope:(id)sender;
- (IBAction)checkboxEnableQoS:(id)sender;
- (IBAction)checkboxEnableFec:(id)sender;
- (IBAction)checkboxVideoHW:(id) sender;
- (IBAction)checkboxEnableDPC:(id)sender;
- (IBAction)showStats:(id)sender;
- (IBAction)DumpOnBtnAction:(id)sender;
- (IBAction)checkboxEnableAutoSwitchMic:(id)sender;
- (IBAction)checkboxEnableAutoSwitchSpeaker:(id)sender;

- (IBAction)checkboxManuallyAdaptation:(id)sender;

@property (assign) IBOutlet NSButton *statsButton;

-(void)startCall:(BOOL)bTa;
-(void)stopCall;
-(void)checkStatus;
-(void)updateBandwidthMaually:(int)bandwidthInBps;

-(void) loadASScreenSource;
-(void) checkVideoWindow;
-(void) showVenueUrl: (NSString*)sVenueUrl;
-(void) viewMuted:(BOOL)muted mutedLayer:(CALayer *) layer;
-(void) viewShow:(BOOL)show showLayer:(CALayer *) layer;
-(void) showProxyAuthendicationDialog:(const char*)szProxy port:(int)nPort realm:(const char*)szRealm;

- (void)registerforDeviceChangeNotif;
- (void)unregisterforDeviceChangeNotif;
- (void)GetDeviceList;
- (void)ProcessMsg:(NSNotification*)notification;

-(void) loadAECType;
@end


class CAppSink: public IAppSink, public wme::IProxyCredentialSink
{
public:
    virtual ~CAppSink() {}
    virtual void ShowWindow(void *pOverlapWindow, bool bShow)
    {
        AppDelegate *app = (AppDelegate*)[[NSApplication sharedApplication] delegate];
        [app viewShow: bShow showLayer: (CALayer*)pOverlapWindow];
    }
    virtual void ShowMuteWindow(void *pOverlapWindow, bool bShow)
    {
        AppDelegate *app = (AppDelegate*)[[NSApplication sharedApplication] delegate];
        [app viewMuted: bShow mutedLayer: (CALayer*)pOverlapWindow];
    }
    virtual void onCheckMultiVideo()
    {
        AppDelegate *app = (AppDelegate*)[[NSApplication sharedApplication] delegate];
        [app checkVideoWindow];
    }
    virtual void onVenueUrl(std::string sVenueUrl) {
        AppDelegate *app = (AppDelegate*)[[NSApplication sharedApplication] delegate];
        [app showVenueUrl : [NSString stringWithUTF8String:sVenueUrl.c_str()]];
    }
    
    virtual bool onCheckHWEnable() {
        bool ret = false;
        SInt32 macSysVerMajor = 0, macSysVerMinor = 0;
        Gestalt(gestaltSystemVersionMajor, &macSysVerMajor);
        Gestalt(gestaltSystemVersionMinor, &macSysVerMinor);
        if (macSysVerMajor > 10) {
            ret = true;
        }
        else if (macSysVerMajor == 10 && macSysVerMinor >= 10) {
            ret = true;
        }
        return ret;
    }
    
    virtual void OnProxyCredentialRequired(const char* szProxy, uint16_t port, const char* szRealm);
};
