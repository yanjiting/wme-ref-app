//
//  AppDelegate.m
//  MediaSessionTest
//
//  Created by juntang on 6/4/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//
#include <stdlib.h>
#include <signal.h>
#import "AppDelegate.h"
#import "MediaConnection.h"
#import "ClickCall.h"
#import "Loopback.h"
#import "PeerCall.h"
#import "TAHTTPServer.h"
#include "../../../src/asenum.h"
#include "ClickCall.h"

#include "WmeAudio.h"
#include "Notification.h"

#import <CoreGraphics/CoreGraphics.h>
#import <AppKit/AppKit.h>

#ifdef ENABLED_GCOV_FLAG
extern "C" void __gcov_flush();
#endif

class WMETraceSink : public wme::IWmeTraceSink
{
public:
    virtual ~WMETraceSink() {}
    virtual WMERESULT OnTrace(uint32_t trace_level, const char* szInfo)
    {
        if(!TestConfig::Instance().m_bTrace2File)
            return WME_S_OK;//Hardcode for performance testing >> don't write trace file by hard code
        CSILogMessage((CSILogLevel)(trace_level+1), @"%@", [NSString stringWithUTF8String:szInfo]);
        return WME_S_OK;
    };
};

void OnScreenSourceChanged()
{
    AppDelegate *appDelegate = [NSApplication sharedApplication].delegate;
    if( appDelegate )
       [appDelegate loadASScreenSource];
}

@interface OnlyIntegerValueFormatter : NSNumberFormatter

@end

@implementation OnlyIntegerValueFormatter

- (BOOL)isPartialStringValid:(NSString*)partialString newEditingString:(NSString**)newString errorDescription:(NSString**)error
{
    if([partialString length] == 0) {
        return YES;
    }

    NSScanner* scanner = [NSScanner scannerWithString:partialString];

    if(!([scanner scanInt:0] && [scanner isAtEnd])) {
        NSBeep();
        return NO;
    }

    return YES;
}

@end

@implementation AppDelegate


@synthesize window = window;
@synthesize wmePeerWindowController = _wmePeerWindowController;
@synthesize wmePeerWindowControllerScreenShare = _wmePeerWindowControllerScreenShare;
@synthesize wmePeerWindowControllerScreenPreview = _wmePeerWindowControllerScreenPreview;
@synthesize wmeSelfWindowController = _wmeSelfWindowController;
//@synthesize wmeDataProcess = _wmeDataProcess;
@synthesize isASCBX = _isASCBX;
@synthesize isDpcCBX = _isDpcCBX;
@synthesize isShowASPrviewCBX = _isShowASPrviewCBX;
@synthesize isASHostCBX = _isASHostCBX;
@synthesize isVideoCBX = _isVideoCBX;
@synthesize isAudioCBX = _isAudioCBX;
@synthesize isEnableQoSCBX = _isEnableQoSCBX;
@synthesize isVideoHWCBX = _isVideoHWCBX;
@synthesize isEnableFecCBX = _isEnableFecCBX;
@synthesize isEnableCVOCBX = _isEnableCVOCBX;
@synthesize isMultiStreamCBX = _isMultiStreamCBX;
@synthesize isEnableAudioDeviceSwitchMicCBX = _isEnableAudioDeviceSwitchMicCBX;
@synthesize isEnableAudioDeviceSwitchSpeakerCBX = _isEnableAudioDeviceSwitchSpeakerCBX;


@synthesize actionConnectBTN = _actionConnectBTN;
@synthesize actionVideoBTN = _actionVideoBTN;
@synthesize actionShareBTN = _actionShareBTN;
@synthesize actionQuitSVSBTN = _actionQuitSVSBTN;
@synthesize isLoopCBX = _isLoopCBX;
@synthesize isCalliopeCBX = _isCalliopeCBX;
@synthesize tfHostIP = _tfhostIP;
@synthesize tfLinus = _tfLinus;
@synthesize labelHostIP = _labelHostIP;
@synthesize labelLinus = _labelLinus;
@synthesize screenRenderModeMTX = _screenRenderModeMTX;
@synthesize actionAudioBTN = _actionAudioBTN;
@synthesize statsButton = _statsButton;

@synthesize audioOptionDrawer = _audioOptionDrawer;
@synthesize audioPlayDeviceCBX = _audioPlayDeviceCBX;
@synthesize audioCaptureDeviceCBX = _audioCaptureDeviceCBX;

@synthesize audioPlayDeviceSlider = _audioPlayDeviceSlider;
@synthesize audioCaptureDeviceSlider = _audioCaptureDeviceSlider;
@synthesize audioPlayDeviceMuteBox = _audioPlayDeviceMuteBox;
@synthesize audioCaptureDeviceMuteBox = _audioCaptureDeviceMuteBox;

/*
- (void)onThreadTimer:(NSTimer*)timer {
    doThreadRun();
}
 */

TAServer *taServer;
CBackdoorAgent * backdoorAgent;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    signal (SIGTERM, gcov_terminate_handler);
    // Insert code here to initialize your application
    
    /*
    _threadTimer = [NSTimer timerWithTimeInterval:0.1
                                           target:self
                                         selector:@selector(onThreadTimer:)
                                         userInfo:nil repeats:YES];
    [[NSRunLoop mainRunLoop] addTimer:_threadTimer forMode:NSRunLoopCommonModes];
     */

    [[CSILog sharedLog] startLogging];

    _traceSink = new WMETraceSink();
    WmeSetTraceMaxLevel(TestConfig::i().m_eTraceLevel);
    if(!TestConfig::i().m_bTraceToWbxTraceFile)
        WmeSetTraceSink(_traceSink);
    
    
    wme::InitMediaEngine(false);
    wme::SetStunTraceSink(CTaIWmeStunTraceSink::GetInstance());
    wme::SetTraceServerSink(CTaIWmeTraceServerSink::GetInstance());

     char *traceNodeList =
        (char*)"173.37.38.16:3478\n192.168.31.1:10000\n192.168.31.1:10001\n192.168.31.1:10002\n192.168.31.1:10003";

    int sz = (int)strlen(traceNodeList);
    wme::StartTraceServer(traceNodeList, sz);
    
    _appSink = new CAppSink;
    wme::SetProxyCredentialSink(_appSink);
    isConnected = false;
    isVideoMute = false;
    isAudioMute = false;
    isScreenMute = false;
    [_actionAudioBTN setEnabled:NO];
    [_actionVideoBTN setEnabled:NO];
    [_actionShareBTN setEnabled:NO];
    [self.isEnableFecCBX setState:YES];
    
    [self.tfLinus setHidden:YES];
    [self.labelLinus setHidden:YES];
    [self.tfHostIP setHidden:YES];
    [self.labelHostIP setHidden:YES];
    
    [self.tfLinus setStringValue:@"http://10.224.166.47:5000/"];
    [self.tfHostIP setStringValue:[NSString stringWithUTF8String:TestConfig::Instance().m_sWSUrl.c_str()]];
    [self.tfLinus setStringValue:[NSString stringWithUTF8String:TestConfig::Instance().m_sLinusUrl.c_str()]];
    [self.tfVenueUrl setStringValue:[NSString stringWithUTF8String:TestConfig::Instance().m_sVenuUrl.c_str()]];
   
    //Init UI status
    [self.isASCBX setState:(TestConfig::Instance().m_bAppshare?YES:NO)];
    [self.isDpcCBX setState:(TestConfig::Instance().m_bDPC?YES:NO)];
    [self.isAudioCBX setState:(TestConfig::Instance().m_bHasAudio?YES:NO)];
    [self.isVideoCBX setState:(TestConfig::Instance().m_bHasVideo?YES:NO)];
    [self.isLoopCBX setState:(TestConfig::Instance().m_bLoopback?YES:NO)];
    [self.isASHostCBX setState:(TestConfig::Instance().m_bSharer && TestConfig::Instance().m_bAutoRequestFloor?YES:NO)];
    [self.isCalliopeCBX setState:(TestConfig::Instance().m_bCalliope?YES:NO)];
    [self.isMultiStreamCBX setState:(TestConfig::Instance().m_bMultiStreamEnable?YES:NO)];
    [self.tfVenueUrl setHidden:(TestConfig::Instance().m_bNoSignal?YES:NO)];
    [self.isNoSignal setState:(TestConfig::Instance().m_bNoSignal?YES:NO)];
    [self.isShowASPrviewCBX setState:TestConfig::Instance().m_bASPreview?YES:NO];
    [self.isEnableQoSCBX setState:TestConfig::Instance().m_bQoSEnable?YES:NO];
    [self.isVideoHWCBX setState:TestConfig::Instance().m_bVideoHW?YES:NO];
    [self.isEnableCVOCBX setState:TestConfig::Instance().m_bEnableCVO?YES:NO];
    [self.isEnableAudioDeviceSwitchMicCBX setState:(TestConfig::Instance().m_bAutoSwitchAudioMicDevice?YES:NO)];
    [self.isEnableAudioDeviceSwitchSpeakerCBX setState:(TestConfig::Instance().m_bAutoSwitchAudioSpeakerDevice?YES:NO)];

    OnlyIntegerValueFormatter *formatter = [[[OnlyIntegerValueFormatter alloc] init] autorelease];
    [formatter setNumberStyle:NSNumberFormatterDecimalStyle];
    [self.ManualBandwidthKbps setFormatter:formatter];
    
    isInSvsMode = TestConfig::i().m_bHasSVS;
    if(isInSvsMode) {
        [_actionQuitSVSBTN setTitle:@"Quit SVS"];
    } else {
        [_actionQuitSVSBTN setTitle:@"Start SVS"];
    }

    NSInteger column = 0;
    switch(TestConfig::Instance().m_eRenderModeScreenSharing){
        case WmeRenderModeFill:
            column = 0;
            break;
        case WmeRenderModeLetterBox:
            column = 1;
            break;
        case WmeRenderModeCropFill:
            column = 2;
            break;
        case WmeRenderModeOriginal:
            column = 3;
            break;
    }
    [_screenRenderModeMTX selectCellAtRow:0 column:column];
    
    [_viewsList selectItemAtIndex:0];
    if(TestConfig::Instance().m_uMaxVideoStreams>0)
        [_viewsList selectItemAtIndex: TestConfig::Instance().m_uMaxVideoStreams - 1];
    [self checkStatus];
    
    if(TestConfig::Instance().m_nTAPort>0)
        [self initTAContext];
//    _wmeDataProcess = [[WMEDataProcess alloc] init];
//    [_wmeDataProcess getShareSources];
    
    
    [self loadASScreenSource];
    [self loadAECType];
    
    CASEnum::Instance()->SetNotifyFunction(OnScreenSourceChanged);
    TestConfig::i().m_handleSharedWindow = (void*)[window windowNumber];
    if(TestConfig::i().m_bAutoStart){
        [self buttonConnect:0];
    }
    
//mine
    
    [self registerforDeviceChangeNotif];
    // post a notification
    
    [self initDataSource];
    [self GetDeviceList];
    _audioPlayIndex = 0;
    _audioCaptureIndex = 0;
    
    WmeAudio::Instance()->SetDevice(WmeDeviceIn,_audioCaptureIndex);
    WmeAudio::Instance()->SetDevice(WmeDeviceOut,_audioPlayIndex);
    
    if ([[_audioCaptureDeviceCBX dataSource] numberOfItemsInComboBox:_audioCaptureDeviceCBX] > 0)
    {
        [_audioCaptureDeviceCBX setObjectValue:[[_audioCaptureDeviceCBX dataSource] comboBox:_audioCaptureDeviceCBX objectValueForItemAtIndex:0]];
    }
    
    if ([[_audioPlayDeviceCBX dataSource] numberOfItemsInComboBox:_audioPlayDeviceCBX] > 0)
    {
        [_audioPlayDeviceCBX setObjectValue:[[_audioPlayDeviceCBX dataSource] comboBox:_audioPlayDeviceCBX objectValueForItemAtIndex:0]];
    }
    
    //init of new UI control,slider,mute status....
    [_audioCaptureDeviceSlider setMaxValue:1.0];
    [_audioCaptureDeviceSlider setMinValue:0.0];
    
    [_audioPlayDeviceSlider setMaxValue:1.0];
    [_audioPlayDeviceSlider setMinValue:0.0];
    
    unsigned int vol = WmeAudio::Instance()->GetSpeakerVol();
    Float32 douVal = vol/65535.0;
    [_audioPlayDeviceSlider setDoubleValue:douVal];
        
    vol = WmeAudio::Instance()->GetMicVol();
    douVal = vol/65535.0;
    [_audioCaptureDeviceSlider setDoubleValue:douVal];
    
    _bSpeakerMute = FALSE;
    _bMicrophoneMute = FALSE;
    
//    [_audioPlayDeviceMuteBox setState:NSOffState];
//    [_audioCaptureDeviceMuteBox setState:NSOffState];
    
    if (WmeAudio::Instance()->GetSpeakerMuteStatus())
        [_audioPlayDeviceMuteBox setState:NSOnState];
    else
        [_audioPlayDeviceMuteBox setState:NSOffState];
    
    if (WmeAudio::Instance()->GetMicphoneMuteStatus())
        [_audioCaptureDeviceMuteBox setState:NSOnState];
    else
        [_audioCaptureDeviceMuteBox setState:NSOffState];
}
void gcov_terminate_handler(int sig)
{

#ifdef ENABLED_GCOV_FLAG
    NSLog(@"gcov flush signal!");
    __gcov_flush();
#endif
    _exit(0);
}

#pragma mark - Get Audio Device List

//static void notificationCallback(CFNotificationCenterRef center, void *observer, CFStringRef name_cf, const void *object, CFDictionaryRef userInfo)
//{
//    //NSString *name = (__bridge NSString*)name_cf;
//    //if ([name isEqualToString:kCFStrSpeakerChange])
//    if (name_cf && CFStringCompare(kCFStrSpeakerChange,name_cf,0) == kCFCompareEqualTo)
//    {
//        //NSLog(@"DEVICE kCFStrSpeakerChange");
//        AppDelegate* client = reinterpret_cast<AppDelegate*>(observer);
//        [client GetDeviceList];
//    }
//    //else if ([name isEqualToString:kCFStrMicChange])
//    else if (name_cf && CFStringCompare(kCFStrMicChange,name_cf,0) == kCFCompareEqualTo)
//    {
//        //NSLog(@"LOCK STATUS kCFStrMicChange");
//        AppDelegate* client = reinterpret_cast<AppDelegate*>(observer);
//        [client GetDeviceList];
//    }
//}

- (void)ProcessMsg:(NSNotification*)notification
{
    NSString* msg = [notification name];
    
    if ([msg isEqualToString:(NSString*)KPSpeakerChange])
    {
        [self GetDeviceList];
    }
    else if ([msg isEqualToString:(NSString*)KPMicChange])
    {
        [self GetDeviceList];
    }
    
}

- (void)registerforDeviceChangeNotif
{
//    CFNotificationCenterRef center = CFNotificationCenterGetLocalCenter();
//    // add an observer
//    
//    CFNotificationCenterAddObserver(center, NULL, notificationCallback,
//                                    (__bridge CFStringRef)kCFStrSpeakerChange, NULL,
//                                    CFNotificationSuspensionBehaviorDeliverImmediately);
//    
//    CFNotificationCenterAddObserver(center, NULL, notificationCallback,
//                                    (__bridge CFStringRef)kCFStrMicChange, NULL,
//                                    CFNotificationSuspensionBehaviorDeliverImmediately);
    
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(ProcessMsg:) name:KPSpeakerChange object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(ProcessMsg:) name:KPMicChange object:nil];
}

- (void)unregisterforDeviceChangeNotif
{
    [[NSNotificationCenter defaultCenter] removeObserver:self name:KPSpeakerChange object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:KPMicChange object:nil];
}

- (void)GetDeviceList
{
    _vectorCaptureDevice.clear();
    _vectorPalyDevice.clear();
    //PeerCall::Instance()->m_endCaller->GetAudioDeviceEnum(WmeDeviceIn,_vectorCaptureDevice,_vectorPalyDevice);
    //PeerCall::Instance()->m_endCaller->GetAudioDeviceEnum(WmeDeviceOut,_vectorCaptureDevice,_vectorPalyDevice);
    //printf("\n size1 = %d,size2 = %d\n",_vectorCaptureDevice.size(),_vectorPalyDevice.size());
    
    WmeAudio::Instance()->GetAudioDeviceEnum(WmeDeviceIn,_vectorCaptureDevice,_vectorPalyDevice);
    WmeAudio::Instance()->GetAudioDeviceEnum(WmeDeviceOut,_vectorCaptureDevice,_vectorPalyDevice);
    
    int k = 0;
    std::string str;
    
    [_arrayAudioCaptureDeviceCBX removeAllObjects];
    [_arrayAudioPlayDeviceCBX removeAllObjects];
   
#if 1
    for (k=0; k < _vectorCaptureDevice.size(); k++)
    {
        str = _vectorCaptureDevice.at(k);
        //printf("\n name = %s\n",str.c_str());
        [_arrayAudioCaptureDeviceCBX  addObject:[[NSString alloc] initWithCString:str.c_str() encoding:NSUTF8StringEncoding]];
    }
    
    for (k=0; k < _vectorPalyDevice.size(); k++)
    {
        str = _vectorPalyDevice.at(k);
        [_arrayAudioPlayDeviceCBX  addObject:[[NSString alloc] initWithCString:str.c_str() encoding:NSUTF8StringEncoding]];
    }
#else
     [_arrayAudioPlayDeviceCBX addObject:@"AudioPlayDevice0"];
     [_arrayAudioPlayDeviceCBX addObject:@"AudioPlayDevice0"];
     [_arrayAudioPlayDeviceCBX addObject:@"AudioPlayDevice0"];
    
#endif
}

#pragma mark - Set Combox Datasource
- (void)initDataSource
{
    _arrayAudioCaptureDeviceCBX = [[NSMutableArray alloc] init];
    _arrayAudioPlayDeviceCBX = [[NSMutableArray alloc] init];
}

- (void)getAudioCaptureDeviceCBX
{
    [_arrayAudioCaptureDeviceCBX removeAllObjects];
    
//    int defaultIndex = 0;
//    for (int k=0; k < _audioInList.size(); k++)
//    {
//        DeviceProperty *dev = &(_audioInList.at(k));
//        if (dev->is_default_dev)
//        {
//            defaultIndex = k;
//        }
//        [_arrayAudioCaptureDeviceCBX  addObject:[[NSString alloc] initWithCString:dev->dev_name encoding:NSUTF8StringEncoding]];
//    }
    
}

- (void)comboBoxSelectionDidChange:(NSNotification *)notification
{
    _audioCaptureIndex = [_audioCaptureDeviceCBX indexOfSelectedItem];
    _audioPlayIndex = [_audioPlayDeviceCBX indexOfSelectedItem];
    
    WmeAudio::Instance()->SetDevice(WmeDeviceIn,_audioCaptureIndex);
    WmeAudio::Instance()->SetDevice(WmeDeviceOut,_audioPlayIndex);
    
    unsigned int vol = WmeAudio::Instance()->GetSpeakerVol();
    Float32 douVal = vol/65535.0;
    [_audioPlayDeviceSlider setDoubleValue:douVal];
    
    vol = WmeAudio::Instance()->GetMicVol();
    douVal = vol/65535.0;
    [_audioCaptureDeviceSlider setDoubleValue:douVal];
    
    if (WmeAudio::Instance()->GetSpeakerMuteStatus())
        [_audioPlayDeviceMuteBox setState:NSOnState];
    else
        [_audioPlayDeviceMuteBox setState:NSOffState];
    
    if (WmeAudio::Instance()->GetMicphoneMuteStatus())
        [_audioCaptureDeviceMuteBox setState:NSOnState];
    else
        [_audioCaptureDeviceMuteBox setState:NSOffState];
}

- (NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox
{
    if (aComboBox == _audioCaptureDeviceCBX)
    {
        return [_arrayAudioCaptureDeviceCBX count];
    }
    else if (aComboBox == _audioPlayDeviceCBX)
    {
        return [_arrayAudioPlayDeviceCBX count];
    }

    return 0;
}

- (id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)index
{
    if (aComboBox == _audioCaptureDeviceCBX)
    {
        return [_arrayAudioCaptureDeviceCBX objectAtIndex:index];
    }
    else if (aComboBox == _audioPlayDeviceCBX)
    {
        return [_arrayAudioPlayDeviceCBX objectAtIndex:index];
    }

    return 0;
    
}
- (NSUInteger)comboBox:(NSComboBox *)aComboBox indexOfItemWithStringValue:(NSString *)string
{
    if (aComboBox == _audioCaptureDeviceCBX)
    {
        return [_arrayAudioCaptureDeviceCBX indexOfObject:string];
    }
    else if (aComboBox == _audioPlayDeviceCBX)
    {
        return [_arrayAudioPlayDeviceCBX indexOfObject:string];
    }
    return 0;
}


- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    if (PeerCall::Instance()->isActive()) {
        PeerCall::Instance()->stopPeer();
        PeerCall::Instance()->m_linusCall.Close();
    }
    else {
        LoopbackCall::Instance()->stopLoopback();
        LoopbackCall::Instance()->m_linusCaller.Close();
        LoopbackCall::Instance()->m_linusCallee.Close();
    }
    WmeAudio::Destroy();
    [self unregisterforDeviceChangeNotif];
    wme::UninitMediaEngine();
    WmeSetTraceSink(NULL);
    if (_traceSink) {
        delete _traceSink;
        _traceSink = NULL;
    }
    [[CSILog sharedLog] stopLogging];
#ifdef ENABLED_GCOV_FLAG
    __gcov_flush();
#endif
}

-(void)initGUI{
    if(_wmePeerWindowController == nil)
        _wmePeerWindowController = [[WMEPeerWindowController alloc] initWithWindowNibName:@"WMEPeerWindowController"];
    
    if(_wmePeerWindowControllerScreenShare == nil){
        _wmePeerWindowControllerScreenShare = [[WMEPeerWindowController alloc] initWithWindowNibName:@"WMEPeerWindowController"];
        [[_wmePeerWindowControllerScreenShare window] orderOut:self];
        [[_wmePeerWindowControllerScreenShare window] setTitle:@"View host's screen sharing"];
    }
    if(_wmeSelfWindowController == nil){
        _wmeSelfWindowController = [[WMESelfWindowController alloc] initWithWindowNibName:@"WMESelfWindowController"];
        _wmeSelfWindowController.bPlayback = TestConfig::i().m_bPlayback;
    }
    if(_wmePeerWindowControllerScreenPreview == nil){
        _wmePeerWindowControllerScreenPreview = [[WMEPeerWindowController alloc] initWithWindowNibName:@"WMEPeerWindowController"];
        [[_wmePeerWindowControllerScreenPreview window] orderOut:self];
        [[_wmePeerWindowControllerScreenPreview window] setTitle:@"screen sharing preview"];
    }
}

-(void)initTAContext{
    [self initGUI];
    if(backdoorAgent==NULL){
        
        [_wmeSelfWindowController showWindow:self];
        
        NSString * resourcePath = [[NSBundle mainBundle] resourcePath];
        NSString * refPath = [resourcePath stringByAppendingString:@"/../../../../../../../"];
        NSLog(@"refPath = %@", refPath);
        backdoorAgent = new CBackdoorAgent([refPath UTF8String],
                                           (void*)[[_wmeSelfWindowController peerView] layer],
                                           (void*)[[_wmeSelfWindowController selfView] layer],
                                           (void*)[[_wmePeerWindowControllerScreenShare peerView] layer],
                                           _appSink);
        
        taServer = new TAServer(backdoorAgent);
        NSLog(@"TestConfig::Instance().m_nTAPort is: %d\n", TestConfig::Instance().m_nTAPort);
        CCmInetAddr addr("127.0.0.1", TestConfig::Instance().m_nTAPort);
        taServer->Init(addr);
        
        [_wmePeerWindowControllerScreenShare showWindow:self];

    }
}


- (void) loadASScreenSource
{
    std::map <std::string, IWmeScreenSource *> mapScreenSource = CASEnum::Instance()->getScreenSourceList();

    std::string strPreferSourceName =TestConfig::Instance().m_strScreenSharingAutoLaunchSourceName;
    bool bAddPreferScreenShource =  strPreferSourceName.size()>0;
    
    int nSelectIndex = 0;
    for (std::map <std::string, IWmeScreenSource *>::iterator iter = mapScreenSource.begin();
         iter != mapScreenSource.end(); ++iter)
    {
        IWmeScreenSource *p = iter->second;
        
        WmeScreenSourceType type;
        p->GetSourceType(type);
        

        char szFriendName[256] = { 0 };
        int nLen = 256;
        p->GetFriendlyName(szFriendName, nLen);
        if(bAddPreferScreenShource){
            std::string strFriendeName = szFriendName;
            if(strFriendeName.find(strPreferSourceName)!=std::string::npos)
                nSelectIndex = std::distance(mapScreenSource.begin(),iter);
        }
        [_shareList addItemWithObjectValue : [NSString stringWithUTF8String : szFriendName]];

     }

    [_shareList selectItemAtIndex : nSelectIndex];
}

-(void) checkVideoWindow {
    unsigned int count = TestConfig::Instance().m_uMaxVideoStreams + 1;
    CGFloat nextY = 0;
    while ( count > 1) {
        NSView *thumbnail = [_wmeSelfWindowController createThumbnail:nextY];
        PeerCall::Instance()->pushRemoteWindow((__bridge void*)[thumbnail layer]);
        nextY += 95;
        count--;
    }
}

- (IBAction)SetSpeakerVolAction:(id)sender
{
    Float32 vol = [sender doubleValue];
    if (vol >= 0 && vol <= 1.0)
    {
        unsigned int realval = 65535*vol;
        WmeAudio::Instance()->SetSpeakerVol(realval);
    }
}

- (IBAction)SetMicVolAction:(id)sender
{
    Float32 vol = [sender doubleValue];
    if (vol >= 0 && vol <= 1.0)
    {
        unsigned int realval = 65535*vol;
        WmeAudio::Instance()->SetMicVol(realval);
    }
}
- (IBAction)MuteSpeakerAction:(id)sender
{
    if ([sender state] == NSOnState)
    {
        NSLog(@"Speaker Muted!");
        _bSpeakerMute = TRUE;
        WmeAudio::Instance()->SetSpeakerMute(1);
    }
    else
    {
        NSLog(@"Speaker not Muted!");
        _bSpeakerMute = FALSE;
        WmeAudio::Instance()->SetSpeakerMute(0);
    }
    
    
}
- (IBAction)MuteMicAction:(id)sender
{
    if ([sender state] == NSOnState)
    {
        NSLog(@"Microphone Muted!");
        _bMicrophoneMute = TRUE;
        WmeAudio::Instance()->SetMicrophoneMute(1);
    }
    else
    {
        NSLog(@"Microphone not Muted!!");
        _bMicrophoneMute = FALSE;
        WmeAudio::Instance()->SetMicrophoneMute(0);
    }
}

-(void) showVenueUrl: (NSString*)sVenueUrl {
    [self.self.tfVenueUrl setStringValue: sVenueUrl];
}

-(void) viewMuted:(BOOL)muted mutedLayer:(CALayer *) layer {
    [_wmeSelfWindowController ViewMuted:muted mutedLayer:layer];
}

-(void) viewShow:(BOOL)show showLayer:(CALayer *) layer {
    [_wmeSelfWindowController ViewShow:show showLayer:layer];
}

- (IBAction)checkboxAS:(id)sender {
    NSInteger as = [self.isASCBX state];
    if (as == YES)
        TestConfig::Instance().m_bAppshare = true;
    else
        TestConfig::Instance().m_bAppshare = false;
}

- (IBAction)DumpOnBtnAction:(id)sender
{
    std::string str = "{\"audio\": { \"enableKeyDumpFile\": 5000}}";
    if (TestConfig::Instance().m_bLoopback)
        LoopbackCall::Instance()->m_endCaller.setAudioParam(str.c_str());
    else
        PeerCall::Instance()->m_endCaller->setAudioParam(str.c_str());    
}

- (IBAction)checkboxASHost:(id)sender {
    NSInteger asHost = [self.isASHostCBX state];
    if (asHost == YES)
        TestConfig::Instance().m_bSharer = true;
    else
        TestConfig::Instance().m_bSharer = false;
    
    if(isConnected){
        
        if (TestConfig::Instance().m_bLoopback)
        {
            if(TestConfig::Instance().m_bSharer)
                LoopbackCall::Instance()->requestFloor();
            else
                LoopbackCall::Instance()->releaseFloor();
        }
        else
        {
            if(TestConfig::Instance().m_bSharer)
                PeerCall::Instance()->requestFloor();
            else
                PeerCall::Instance()->releaseFloor();
        }
    }
    
}

- (IBAction)buttonAudio:(id)sender {
    if(isAudioMute)
    {
        isAudioMute = false;
        [_actionAudioBTN setTitle:@"Mute Audio"];
        if (TestConfig::Instance().m_bLoopback)
        {
            LoopbackCall::Instance()->mute(WmeSessionType_Audio, false, true);
        }
        else
        {
            PeerCall::Instance()->mute(WmeSessionType_Audio, false, true);
        }
    }
    else
    {
        isAudioMute = true;
        [_actionAudioBTN setTitle:@"Unmute Audio"];
        if (TestConfig::Instance().m_bLoopback)
        {
            LoopbackCall::Instance()->mute(WmeSessionType_Audio, true, true);
        }
        else
        {
            PeerCall::Instance()->mute(WmeSessionType_Audio, true, true);
        }
    }
}

- (IBAction)buttonQuitSVS:(id)sender {
    if(isInSvsMode) {
        [_actionQuitSVSBTN setTitle:@"Start SVS"];
        isInSvsMode = false;
        if (TestConfig::Instance().m_bLoopback)
        {
            LoopbackCall::Instance()->setCamera(NULL);
        }
        else
        {
            PeerCall::Instance()->setCamera(NULL);
        }
    } else {
        [_actionQuitSVSBTN setTitle:@"Quit SVS"];
        isInSvsMode = true;
        if (TestConfig::Instance().m_bLoopback)
        {
            LoopbackCall::Instance()->startSVS();
        }
        else
        {
            PeerCall::Instance()->startSVS();
        }
    }
}

- (IBAction)buttonVideo:(id)sender {
    if(isVideoMute)
    {
        isVideoMute = false;
        [_actionVideoBTN setTitle:@"Mute Video"];
        if (TestConfig::Instance().m_bLoopback)
        {
            LoopbackCall::Instance()->mute(WmeSessionType_Video, false, true);
        }
        else
        {
            PeerCall::Instance()->mute(WmeSessionType_Video, false, true);
        }
    }
    else
    {
        isVideoMute = true;
        [_actionVideoBTN setTitle:@"Unmute Video"];

        if (TestConfig::Instance().m_bLoopback)
        {
            LoopbackCall::Instance()->mute(WmeSessionType_Video, true, true);
        }
        else
        {
            PeerCall::Instance()->mute(WmeSessionType_Video, true, true);
        }
    }

}

- (IBAction)buttonScreen:(id)sender {
    if(isScreenMute)
    {
        isScreenMute = false;
        [_actionShareBTN setTitle:@"Mute Screen"];
        if (TestConfig::Instance().m_bLoopback)
        {
            LoopbackCall::Instance()->mute(WmeSessionType_ScreenShare, false, true);
        }
        else
        {
            PeerCall::Instance()->mute(WmeSessionType_ScreenShare, false, true);
        }
    }
    else
    {
        isScreenMute = true;
        [_actionShareBTN setTitle:@"Unmute Screen"];
        
        if (TestConfig::Instance().m_bLoopback)
        {
            LoopbackCall::Instance()->mute(WmeSessionType_ScreenShare, true, true);
        }
        else
        {
            PeerCall::Instance()->mute(WmeSessionType_ScreenShare, true, true);
        }
    }
}

- (void)showProxyAuthendicationDialog:(const char*)szProxy port:(int)nPort realm:(const char*)szRealm
{
    if (!_proxyAuthenticationDlg) {
        _proxyAuthenticationDlg = [[ProxyAuthenticationDialog alloc] initWithWindowNibName:@"ProxyAuthenticationDialog"];
    }

    [_proxyAuthenticationDlg setDescription:szProxy port:nPort realm:szRealm];
    [self.window beginSheet:_proxyAuthenticationDlg.window  completionHandler:^(NSModalResponse returnCode) {
        switch (returnCode) {
            case NSModalResponseOK:
            {
                SetProxyUsernamePassword([_proxyAuthenticationDlg getUsername], [_proxyAuthenticationDlg getPassword]);
                break;
            }
            case NSModalResponseCancel:
            {
                SetProxyUsernamePassword(NULL, NULL);
            }
            default:
                break;
        }
        _proxyAuthenticationDlg = nil;
    }];
}

- (IBAction)buttonConnect:(id)sender
{
    if (isConnected) {
        [_actionConnectBTN setTitle:@"Connect"];
        //[_statsButton setEnabled:NO];
        [_actionAudioBTN setEnabled:NO];
        [_actionVideoBTN setEnabled:NO];
        [_actionShareBTN setEnabled:NO];
        isConnected = false;
        //[_wmeSelfWindowController release];
        //[_wmePeerWindowControllerScreenShare release];
        
        [_isEnableAudioDeviceSwitchMicCBX setEnabled:YES];
        [_isEnableAudioDeviceSwitchSpeakerCBX setEnabled:YES];
        
        [self stopCall];
        
    }
    else {
        [_actionConnectBTN setTitle:@"Disconnect"];
        //[_statsButton setEnabled:YES];
        [_actionAudioBTN setEnabled:YES];
        [_actionVideoBTN setEnabled:YES];
        [_actionShareBTN setEnabled:YES];
        isConnected = true;
        
        [self initGUI];
        [_isEnableAudioDeviceSwitchMicCBX setEnabled:NO];
        [_isEnableAudioDeviceSwitchSpeakerCBX setEnabled:NO];

        [self startCall:NO];
    }
}

- (IBAction)checkboxNoSignal:(id)sender
{
    [self checkStatus];
}

- (IBAction)checkboxLoop:(id)sender
{
    [self checkStatus];
}

- (IBAction)checkboxCalliope:(id)sender
{
    [self checkStatus];
}

- (IBAction)checkboxEnableQoS:(id)sender
{
    [self checkStatus];
}

- (IBAction)checkboxVideoHW:(id)sender
{
    [self checkStatus];
}

- (IBAction)checkboxEnableFec:(id)sender
{
    [self checkStatus];
}

- (IBAction)checkboxEnableDPC:(id)sender
{
    NSInteger dpc = [self.isDpcCBX state];
    if (dpc == YES) {
        TestConfig::Instance().m_bDPC = true;
    } else {
        TestConfig::Instance().m_bDPC = false;
    }
    [self checkStatus];
}
- (IBAction)checkboxEnableAutoSwitchMic:(id)sender
{
    NSInteger dpc = [self.isEnableAudioDeviceSwitchMicCBX state];
    if (dpc == YES) {
        TestConfig::Instance().m_bAutoSwitchAudioMicDevice = true;
    } else {
        TestConfig::Instance().m_bAutoSwitchAudioMicDevice = false;
    }
    [self checkStatus];
    
    bool rc = TestConfig::Instance().m_bAutoSwitchAudioMicDevice;
    if (TestConfig::Instance().m_bLoopback)
    {
        LoopbackCall::Instance()->SetAutoSwitchDefaultMic(rc);
    }
    else
    {
        PeerCall::Instance()->SetAutoSwitchDefaultMic(rc);
    }
}

- (IBAction)checkboxEnableAutoSwitchSpeaker:(id)sender
{
    NSInteger dpc = [self.isEnableAudioDeviceSwitchSpeakerCBX state];
    if (dpc == YES) {
        TestConfig::Instance().m_bAutoSwitchAudioSpeakerDevice = true;
    } else {
        TestConfig::Instance().m_bAutoSwitchAudioSpeakerDevice = false;
    }
    [self checkStatus];
    
    bool rc = TestConfig::Instance().m_bAutoSwitchAudioSpeakerDevice;
    if (TestConfig::Instance().m_bLoopback)
    {
        LoopbackCall::Instance()->SetAutoSwitchDefaultSpeaker(rc);
    }
    else
    {
        PeerCall::Instance()->SetAutoSwitchDefaultSpeaker(rc);
    }
}

- (IBAction)checkboxManuallyAdaptation:(id)sender
{
    NSInteger manualAdjust = [self.mariManualAdaptation state];
    if (manualAdjust == YES) {
        [self.ManualBandwidthKbps setEnabled: TRUE];
        TestConfig::Instance().m_mauallyAdaptationBps = 1000*[self.ManualBandwidthKbps intValue];
    } else {
        TestConfig::Instance().m_mauallyAdaptationBps = 0;
        [self.ManualBandwidthKbps setStringValue:@"0"];
        [self.ManualBandwidthKbps setEnabled: FALSE];
    }

    [self updateBandwidthMaually: TestConfig::Instance().m_mauallyAdaptationBps];
}

- (IBAction)checkboxASPreview:(id)sender
{
    [self checkStatus];
    [self updateASPreviewUIShowStatus];
}

- (IBAction)screenSourceSelected:(id)sender
{
    if(isConnected
       && TestConfig::Instance().m_bAppshare
       && TestConfig::Instance().m_bSharer) {
        IWmeScreenSource *pShareSource = NULL;
        pShareSource = CASEnum::Instance()->FindScreenSourceByFriendlyName([[_shareList objectValueOfSelectedItem] UTF8String]);
        if (pShareSource) {
            if(TestConfig::Instance().m_bLoopback){
                LoopbackCall::Instance()->m_endCaller.m_pScreenTrackSharer->Stop();
                LoopbackCall::Instance()->m_endCaller.addScreenSouce(pShareSource);
                LoopbackCall::Instance()->m_endCaller.m_pScreenTrackSharer->Start(FALSE);
            }
            else {
                PeerCall::Instance()->m_endCaller->m_pScreenTrackSharer->Stop();
                PeerCall::Instance()->m_endCaller->addScreenSouce(pShareSource);
                PeerCall::Instance()->m_endCaller->m_pScreenTrackSharer->Start(FALSE);
            }
        }
    }

}


- (IBAction)checkboxShareFilter:(id)sender
{
    NSButton *checkFilterSelf = (NSButton *)sender;
    if( [checkFilterSelf isKindOfClass:[NSButton class]] )
    {
    NSInteger bFilter = [sender state];
    if (bFilter == NO)
        TestConfig::Instance().m_bShareFilterSelf = false;
    else
        TestConfig::Instance().m_bShareFilterSelf = true;
    }
}

- (IBAction)checkboxDtlsSrtp:(id)sender
{
    NSButton *checkDtlsSrtp = (NSButton *)sender;
    if( [checkDtlsSrtp isKindOfClass:[NSButton class]] )
    {
        NSInteger bFilter = [sender state];
        if (bFilter == NO)
            TestConfig::Instance().m_bEnableDtlsSrtp = false;
        else
            TestConfig::Instance().m_bEnableDtlsSrtp = true;
    }
}

- (IBAction)checkboxEnableCVO:(id)sender {
    NSButton *checkEnableCVO = (NSButton *)sender;
    if( [checkEnableCVO isKindOfClass:[NSButton class]] )
    {
        NSInteger bFilter = [sender state];
        if (bFilter == NO)
            TestConfig::Instance().m_bEnableCVO = false;
        else
            TestConfig::Instance().m_bEnableCVO = true;
    }
}


- (void)updateASPreviewUIShowStatus{
    if(_wmePeerWindowControllerScreenPreview==nil){
        [self initGUI];
    }
    if(_wmePeerWindowControllerScreenPreview!=nil){
        if(TestConfig::i().m_bASPreview){
            [[_wmePeerWindowControllerScreenPreview window] orderFront:self];
            void* share_preview_win = [[_wmePeerWindowControllerScreenPreview peerView] layer];
            IWmeScreenSource *pShareSource = NULL;
            pShareSource = CASEnum::Instance()->FindScreenSourceByFriendlyName([[_shareList objectValueOfSelectedItem] UTF8String]);
            CASEnum::Instance()->StartPreview(pShareSource,share_preview_win);
        }
        else{
            [[_wmePeerWindowControllerScreenPreview window] orderOut:self];
            CASEnum::Instance()->StopPreview();
        }
    }
}
- (IBAction)checkboxMultiStream:(id)sender
{
    NSInteger isShow = [_isMultiStreamCBX state];
    if(isShow==YES){
        TestConfig::i().m_bMultiStreamEnable = true;
    }
    else{
        TestConfig::i().m_bMultiStreamEnable = false;
    }
}

- (IBAction)showStats:(id)sender {
    
    if (!_wmeStatsWindowController) {
        _wmeStatsWindowController = [[WMEStatsWindowController alloc] initWithWindowNibName:@"WMEStatsWindowController"];
        _wmeStatsWindowController.bPlayback = TestConfig::i().m_bPlayback;
    }
    
    [_wmeStatsWindowController showWindow:self];
}

- (IBAction)matrixRenderModeScreenSharing:(id)sender{
    NSInteger column = [_screenRenderModeMTX selectedColumn];
    WmeTrackRenderScalingModeType renderMode = WmeRenderModeLetterBox;
    
    if(column == 0){
        renderMode = WmeRenderModeFill;
    }
    else if (column == 1){
        renderMode = WmeRenderModeLetterBox;
    }
    else if (column == 2){
        renderMode = WmeRenderModeCropFill;
    }
    else if (column == 3){
        renderMode = WmeRenderModeOriginal;
    }
    TestConfig::Instance().m_eRenderModeScreenSharing = renderMode;
}

-(void)checkStatus
{
    NSInteger loopState = [_isLoopCBX state];
    NSInteger calliopeState = [self.isCalliopeCBX state];
    NSInteger enableQoS = [_isEnableQoSCBX state];
    NSInteger videoHW = [_isVideoHWCBX state];
    NSInteger enableFec = [_isEnableFecCBX state];
    NSInteger isNoSignal = [_isNoSignal state];
    NSInteger enableDtlsSrtp = [_isEnableDtlsSrtpCBX state];
    if(isNoSignal == YES) {
        loopState = NO;
        calliopeState = YES;
    }
    _isLoopCBX.enabled = !isNoSignal;
    _isCalliopeCBX.enabled = !isNoSignal;
    _viewsList.enabled = !loopState;
    [_tfVenueUrl setHidden: !isNoSignal];
    TestConfig::Instance().m_bNoSignal = isNoSignal;
    if(_tfVenueUrl != nil)
        TestConfig::Instance().m_sVenuUrl = [[_tfVenueUrl stringValue] UTF8String];
    [_viewsList selectItemAtIndex: TestConfig::Instance().m_uMaxVideoStreams - 1];
    if (loopState == YES) {
        if (calliopeState == YES) {
            [self.tfLinus setHidden:NO];
            [self.labelLinus setHidden:NO];
            
            [self.labelHostIP setHidden:YES];
            [self.tfHostIP setHidden:YES];
            
            TestConfig::Instance().m_bCalliope = true;
        }
        else {
            [self.tfLinus setHidden:YES];
            [self.labelLinus setHidden:YES];
            
            [self.labelHostIP setHidden:YES];
            [self.tfHostIP setHidden:YES];
            
            TestConfig::Instance().m_bCalliope = false;
        }
        
        TestConfig::Instance().m_bLoopback = true;
        
    }
    else {        
        if (calliopeState == YES) {
            [self.labelLinus setHidden:NO];
            [self.tfLinus setHidden:NO];
            
            [self.labelHostIP setHidden:isNoSignal];
            [self.tfHostIP setHidden:isNoSignal];
            TestConfig::Instance().m_bCalliope = true;
        }
        else {
            [self.labelLinus setHidden:YES];
            [self.tfLinus setHidden:YES];
            
            [self.labelHostIP setHidden:NO];
            [self.tfHostIP setHidden:NO];
            TestConfig::Instance().m_bCalliope = false;
        }
        
        TestConfig::Instance().m_bLoopback = false;
    }
    
    TestConfig::Instance().m_bQoSEnable = enableQoS;
    TestConfig::Instance().m_bEnableDtlsSrtp = enableDtlsSrtp;
    TestConfig::Instance().m_bVideoHW = videoHW;
    TestConfig::Instance().m_globalParam["enableDPC"] = TestConfig::Instance().m_bDPC;
    //TestConfig::Instance().m_globalParam["enableQos"] = TestConfig::Instance().m_bQoSEnable;
    if (enableFec) {
        TestConfig::i().m_videoParam["fecParams"]["uClockRate"] = 8000;
        TestConfig::i().m_videoParam["fecParams"]["uPayloadType"] = 111;
        TestConfig::i().m_videoParam["fecParams"]["bEnableFec"] = true;
        
        TestConfig::i().m_audioParam["fecParams"]["uClockRate"] = 8000;
        TestConfig::i().m_audioParam["fecParams"]["uPayloadType"] = 112;
        TestConfig::i().m_audioParam["fecParams"]["bEnableFec"] = true;
    } else {
        TestConfig::i().m_videoParam["fecParams"]["bEnableFec"] = false;
        TestConfig::i().m_audioParam["fecParams"]["bEnableFec"] = false;
    }
    if (videoHW) {
        TestConfig::i().m_videoParam["bHWAcceleration"] = true;
    }
    else {
        TestConfig::i().m_videoParam["bHWAcceleration"] = false;
    }
    TestConfig::i().m_bASPreview = ([_isShowASPrviewCBX state]==YES);
}

-(void)startCall:(BOOL)bTa
{
    TestConfig::Instance().m_bHasVideo = ([self.isVideoCBX state]==YES);
    TestConfig::Instance().m_bHasAudio = ([self.isAudioCBX state]==YES);
    
    TestConfig::i().m_audioParam["supportCmulti"] = TestConfig::i().m_bMultiStreamEnable;
    TestConfig::i().m_videoParam["supportCmulti"] = TestConfig::i().m_bMultiStreamEnable;
    TestConfig::i().m_shareParam["supportCmulti"] = TestConfig::i().m_bMultiStreamEnable;
    
    
  /*
    if(TestConfig::Instance().m_bCalliope
       && !TestConfig::Instance().m_bHasVideo
       && TestConfig::Instance().m_bAppshare)
        TestConfig::Instance().m_bFakeVideoByShare = true;
    else
        TestConfig::Instance().m_bFakeVideoByShare = false;*/
    
    TestConfig::Instance().m_sLinusUrl = [[_tfLinus stringValue] UTF8String];
    TestConfig::Instance().m_sWSUrl = [[_tfhostIP stringValue] UTF8String];
    if(_tfVenueUrl != nil)
        TestConfig::Instance().m_sVenuUrl = [[_tfVenueUrl stringValue] UTF8String];
    
    TestConfig::i().m_uMaxVideoStreams = [[_viewsList objectValueOfSelectedItem] intValue];
    //Now audio mix can be control sperately with --server-mix command line argument
    //TestConfig::i().m_uMaxAudioStreams =  TestConfig::i().m_uMaxVideoStreams>1 ? 3 : 1;

    // get aec type.
    TestConfig::Instance().m_aectype = [[_AECTypeList objectValueOfSelectedItem] UTF8String];

    
    void *selfwin = NULL;
    void *peerwin = NULL;
    void *peerwinscreen = NULL;
    void *share_preview_win = NULL;
    if(TestConfig::Instance().m_bHasVideo){
        [_wmeSelfWindowController showWindow:self];
        selfwin = [[_wmeSelfWindowController selfView] layer];
        peerwin = [[_wmeSelfWindowController peerView] layer];
    }

    
    
    if (TestConfig::Instance().m_bLoopback)
    {
        if(TestConfig::Instance().m_bAppshare){
            [_wmePeerWindowControllerScreenShare showWindow:self];
            peerwinscreen = [[_wmePeerWindowControllerScreenShare peerView] layer];
            IWmeScreenSource *pShareSource = NULL;
            pShareSource = CASEnum::Instance()->FindScreenSourceByFriendlyName([[_shareList objectValueOfSelectedItem] UTF8String]);
            LoopbackCall::Instance()->m_endCaller.addScreenSouce(pShareSource);
            
            //[_wmePeerWindowControllerScreenPreview showWindow:self];
            
            //share_preview_win = [[_wmePeerWindowControllerScreenPreview peerView] layer];
        }
        
        LoopbackCall::Instance()->startLoopback(peerwin, selfwin, peerwinscreen);
        
        if (TestConfig::Instance().m_bAppshare)
        {
            //LoopbackCall::Instance()->m_endCaller.ShowScreenSharePreview(share_preview_win);
            [self updateASPreviewUIShowStatus];
        }
    }
    else
    {
        if(TestConfig::Instance().m_bAppshare){
            if (TestConfig::Instance().m_bSharer){
                IWmeScreenSource *pShareSource = NULL;
                pShareSource = CASEnum::Instance()->FindScreenSourceByFriendlyName([[_shareList objectValueOfSelectedItem] UTF8String]);
                PeerCall::Instance()->m_endCaller->addScreenSouce(pShareSource);
            }else{
                [_wmePeerWindowControllerScreenShare showWindow:self];
                peerwinscreen = [[_wmePeerWindowControllerScreenShare peerView] layer];
            }
        }
        
        PeerCall::Instance()->pushRemoteWindow((__bridge void*)peerwin);
        if(TestConfig::Instance().m_bHasVideo)
        {
            [self checkVideoWindow];
        }
        PeerCall::Instance()->startPeer((__bridge void*)selfwin,
                                        (__bridge void*)peerwinscreen, _appSink);
        if(bTa) {
            PeerCall::Instance()->m_endCaller->startCall();
        } else {
            if ( TestConfig::Instance().m_bPlayback ) {
                PeerCall::Instance()->m_endCaller->acceptCall(TestConfig::Instance().m_sSdp.c_str());
            } else {
                PeerCall::Instance()->connect([[_tfhostIP stringValue] UTF8String]);
            }
        }
    }
    TestConfig::i().Dump();
}

-(void)stopCall
{
    [_wmeStatsWindowController release];
    _wmeStatsWindowController = nil;
    
    if (TestConfig::Instance().m_bLoopback) {
        LoopbackCall::Instance()->stopLoopback();
    }
    else{
        PeerCall::Instance()->stopPeer();
    }
    
    if (_arrayAudioPlayDeviceCBX)
    {
        [_arrayAudioPlayDeviceCBX release];
        _arrayAudioPlayDeviceCBX = NULL;
    }
    
    if (_arrayAudioCaptureDeviceCBX)
    {
        [_arrayAudioCaptureDeviceCBX release];
        _arrayAudioCaptureDeviceCBX = NULL;
    }
}

-(void)updateBandwidthMaually:(int)bandwidthInBps
{
    if (TestConfig::Instance().m_bLoopback) {
        LoopbackCall::Instance()->m_endCaller.setBandwidthMaually(bandwidthInBps);
    } else {
        PeerCall::Instance()->m_endCaller->setBandwidthMaually(bandwidthInBps);
    }
}

void CAppSink::OnProxyCredentialRequired(const char* szProxy, uint16_t port, const char* szRealm)
{
    //If there is command-line configuration, we will not prompt for user input.
    if(!TestConfig::i().m_sProxyUser.empty()) {
        SetProxyUsernamePassword(TestConfig::i().m_sProxyUser.c_str(),
                                 TestConfig::i().m_sProxyPasswd.c_str());
        return;
    }
    
    AppDelegate *app = (AppDelegate*)[[NSApplication sharedApplication] delegate];
    [app showProxyAuthendicationDialog: szProxy port:port realm:szRealm];
}

CGImageRef RawDataToImage(unsigned char* pData,int width, int height, int nLineBytes, WmeVideoRawType eWmeVideoRawType  )
{
    if( NULL == pData )
        return NULL;
    
    CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();
    CGContextRef bmp = NULL;
    switch ( eWmeVideoRawType ) {
        case WmeRGBA32:
            bmp = CGBitmapContextCreate( pData, width, height, 8, nLineBytes, colorSpace, kCGImageAlphaPremultipliedLast );
            break;
        case WmeARGB32:
            bmp = CGBitmapContextCreate( pData, width, height, 8, nLineBytes, colorSpace, kCGImageAlphaPremultipliedFirst );
            break;
        case WmeRGB24:
            bmp = CGBitmapContextCreate( pData, width, height, 8, nLineBytes, colorSpace, kCGImageAlphaNone );
            break;
        default:
            ;
    }
    CGImageRef img = NULL;
    if( bmp )
    {
        img = CGBitmapContextCreateImage(bmp);
        CGContextRelease(bmp);
    }
    CGColorSpaceRelease(colorSpace);
    return img;
}

WMERESULT SaveRawDataToJPEG( unsigned char* pData,int width, int height, int nLineBytes, WmeVideoRawType eWmeVideoRawType , const char *pathname,float compressionQuality){
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    CGImageRef img = RawDataToImage(pData, width,  height,  nLineBytes, eWmeVideoRawType);
    if( img )
    {
#if TARGET_OS_IPHONE
        UIImage *image = [[UIImage alloc] initWithCGImage:img];
        NSData *jpeg = UIImageJPEGRepresentation(image, compressionQuality);
#elif TARGET_OS_MAC
        NSBitmapImageRep *bitmap = [[NSBitmapImageRep alloc] initWithCGImage:img];
        NSData *jpeg = [bitmap representationUsingType:NSJPEGFileType properties:[NSDictionary dictionaryWithObject:[NSNumber numberWithFloat:compressionQuality] forKey:NSImageCompressionFactor]];
        [bitmap release];
#endif
        CGImageRelease(img);
        if( jpeg )
        {
            NSString *path = [NSString stringWithUTF8String:pathname];
            [jpeg writeToFile:path atomically:YES];
            [pool release];
            return WME_S_OK;
        }
    }
    [pool release];
    return WME_S_FALSE;
}

WMERESULT SaveRawDataToPNG( unsigned char* pData,int width, int height, int nLineBytes, WmeVideoRawType eWmeVideoRawType , const char *pathname){
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    CGImageRef img = RawDataToImage(pData, width,  height,  nLineBytes, eWmeVideoRawType);
    if( img )
    {
#if TARGET_OS_IPHONE
        UIImage *image = [UIImage imageWithCGImage:img];
        NSData *png = UIImagePNGRepresentation(image);
#elif TARGET_OS_MAC
        NSBitmapImageRep *bitmap = [[NSBitmapImageRep alloc] initWithCGImage:img];
        NSData *png = [bitmap representationUsingType:NSPNGFileType properties:Nil];
        [bitmap release];
#endif
        CGImageRelease(img);
        if( png )
        {
            NSString *path = [NSString stringWithUTF8String:pathname];
            [png writeToFile:path atomically:YES];
            [pool release];
            return WME_S_OK;
        }
    }
    [pool release];
    return WME_S_FALSE;
}

- (void) loadAECType
{
 /*
    typedef enum{
        WmeAecTypeNone = 0,         		///< no AEC
        WmeAecTypeBuildin = 1,				///< Build AEC, only VPIO mode is used on IOS.
        WmeAecTypeWmeDefault = 2,          	///<  Windows/Mac/Linux uses Modified Movi AEC, Android/IOS use WebRTC AECM
        WmeAecTypeTc = 3,           		///< TC AEC, Only IOS/Android supported.
        WmeAecTypeAlpha = 4             	///< A-AEC, only Mac/Windows supported.
    }WmeAecType;
*/
    std::string aectypelist[5] =   {"WmeAecTypeNone","WmeAecTypeBuildin", "WmeAecTypeWmeDefault", "WmeAecTypeTc", "WmeAecTypeAlpha"};
    for (int i =0; i < 5; i++ )
    {
            [_AECTypeList addItemWithObjectValue : [NSString stringWithUTF8String : (aectypelist[i]).c_str()]];
        
    }
    
    [_AECTypeList selectItemAtIndex : 4];
}

@end
