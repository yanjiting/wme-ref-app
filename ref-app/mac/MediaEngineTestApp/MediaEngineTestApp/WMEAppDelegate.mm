//
//  WMEAppDelegate.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-8-5.
//  Copyright (c) 2013年 video. All rights reserved.
//
#import "WMEAppDelegate.h"

@implementation WMEAppDelegate

//Syntheize
@synthesize wmeSelfViewWindowController = _wmeSelfViewWindowController;
@synthesize wmeAttendeeViewWindowController = _wmeAttendeeViewWindowController;
@synthesize wmeScreenAttendeeViewWindowController = _wmeScreenAttendeeViewWindowController;
@synthesize wmePreviewWindowController = _wmePreviewWindowController;
@synthesize audioCaptureVolumeMuteBTN = _audioCaptureVolumeMuteBTN;
@synthesize audioPlayVolumeMuteBTN = _audioPlayVolumeMuteBTN;
@synthesize window = window;
@synthesize dictionaryState = _dictionaryState;
@synthesize actionConnectBTN = _actionConnectBTN;
@synthesize actionDisconnectBTN = _actionDisconnectBTN;
@synthesize actionPreviewBTN = _actionPreviewBTN;
@synthesize actionMaxTraceLevelCBX = _actionMaxTraceLevelCBX;
@synthesize wmeDataProcess = _wmeDataProcess;
@synthesize videoCaptureDeviceCBX = _videoCaptureDeviceCBX;
@synthesize videoEncodingParamCBX = _videoEncodingParamCBX;
@synthesize audioCaptureVolumeSLD = _audioCaptureVolumeSLD;
@synthesize audioPlayVolumeSLD = _audioPlayVolumeSLD;
@synthesize audioCaptureDeviceCBX = _audioCaptureDeviceCBX;
@synthesize audioEncodingParamCBX = _audioEncodingParamCBX;
@synthesize audioPlayDeviceCBX = _audioPlayDeviceCBX;
@synthesize networkEnableNATCBX = _networkEnableNATCBX;
@synthesize networkJINGLEServerIPTF = _networkJINGLEServerIPTF;
@synthesize networkJINGLEServerPortTF = _networkJINGLEServerPortTF;
@synthesize networkSTUNServerIPTF = _networkSTUNServerIPTF;
@synthesize networkSTUNServerPortTF = _networkSTUNServerPortTF;
@synthesize roleHostAndClientMTX = _roleHostAndClientMTX;
@synthesize roleHostIPTF = _roleHostIPTF;
@synthesize roleMyNameTF = _roleMyNameTF;
@synthesize roleHostNameTF = _roleHostNameTF;
@synthesize actionSendVideoCBX = _actionSendVideoCBX;
@synthesize actionSendAudioCBX = _actionSendAudioCBX;
@synthesize videoEnableVideoCBX = _videoEnableVideoCBX;
@synthesize audioEnableAudioCBX = _audioEnableAudioCBX;
@synthesize screenEnableCBX = _screenEnableCBX;
@synthesize videoOptionButton = _videoOptionButton;
@synthesize videoOptionDrawer = _videoOptionDrawer;
@synthesize videoInputFileLabel = _videoInputFileLabel;
@synthesize videoOutputFileLabel = _videoOutputFileLabel;
@synthesize audioOptionButton = _audioOptionButton;
@synthesize audioOptionDrawer = _audioOptionDrawer;
@synthesize audioInputFileLabel = _audioInputFileLabel;
@synthesize audioOutputFileLabel = _audioOutputFileLabel;
@synthesize screenRenderModeMTX = _screenRenderModeMTX;

WMEDataProcess *g_wmeDataProcess;

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Insert code here to initialize your application
    _dictionaryState = [[NSMutableDictionary alloc] init];
    _wmeSelfViewWindowController = [[WMESelfViewWindowController alloc] initWithWindowNibName:@"WMESelfViewWindowController"];
    _wmeAttendeeViewWindowController = [[WMEAttendeeViewWindowController alloc] initWithWindowNibName:@"WMEAttendeeViewWindowController"];
    _wmeScreenAttendeeViewWindowController = [[WMEAttendeeViewWindowController alloc] initWithWindowNibName:@"WMEAttendeeViewWindowController"];
    _wmePreviewWindowController = [[WMEPreviewWindowController alloc] initWithWindowNibName:@"WMEPreviewWindowController"];
    audioCaptureVolumeMuteState = FALSE;
    audioPlayVolumeMuteState = FALSE;
    actionPreviewState = FALSE;
    _wmeDataProcess = [[WMEDataProcess alloc] init];
    
    // Set the trace maximal level
    [_actionMaxTraceLevelCBX selectItemAtIndex:2];
    [_actionMaxTraceLevelCBX setObjectValue:[_actionMaxTraceLevelCBX objectValueOfSelectedItem]];
    
    // Initialization
    [_wmeDataProcess initDataSource];
    [_wmeDataProcess getDataSource];
    g_wmeDataProcess = _wmeDataProcess;
    
    //set the delegate
    _wmeDataProcess.delegate = self;
    
    
    //Initialize the UI
    if ([[_videoCaptureDeviceCBX dataSource] numberOfItemsInComboBox:_videoCaptureDeviceCBX] > 0) {
        [_videoCaptureDeviceCBX setObjectValue:[[_videoCaptureDeviceCBX dataSource] comboBox:_videoCaptureDeviceCBX objectValueForItemAtIndex:0]];
    }
    if ([[_videoEncodingParamCBX dataSource] numberOfItemsInComboBox:_videoEncodingParamCBX] > 0) {
        [_videoEncodingParamCBX setObjectValue:[[_videoEncodingParamCBX dataSource] comboBox:_videoEncodingParamCBX objectValueForItemAtIndex:0]];
    }
    if ([[_audioCaptureDeviceCBX dataSource] numberOfItemsInComboBox:_audioCaptureDeviceCBX] > 0) {
        [_audioCaptureDeviceCBX setObjectValue:[[_audioCaptureDeviceCBX dataSource] comboBox:_audioCaptureDeviceCBX objectValueForItemAtIndex:0]];
    }
    if ([[_audioEncodingParamCBX dataSource] numberOfItemsInComboBox:_audioEncodingParamCBX] > 0) {
        [_audioEncodingParamCBX setObjectValue:[[_audioEncodingParamCBX dataSource] comboBox:_audioEncodingParamCBX objectValueForItemAtIndex:0]];
    }
    if ([[_audioPlayDeviceCBX dataSource] numberOfItemsInComboBox:_audioPlayDeviceCBX] > 0) {
        [_audioPlayDeviceCBX setObjectValue:[[_audioPlayDeviceCBX dataSource] comboBox:_audioPlayDeviceCBX objectValueForItemAtIndex:0]];
    }
    if ([[_shareList dataSource] numberOfItemsInComboBox:_shareList] > 0) {
        [_shareList setObjectValue:[[_shareList dataSource] comboBox:_shareList objectValueForItemAtIndex:0]];
    }
    
    [_wmeDataProcess setVideoCaptureDevice:0];
    [_wmeDataProcess setVideoEncodingParam:0];
    [_wmeDataProcess setAudioCaptureDevice:0];
    [_wmeDataProcess setAudioEncodingParam:0];
    [_wmeDataProcess setAudioPlayDevice:0];
    [_wmeDataProcess setShareSourceIndex:0];
    [_wmeDataProcess setRenderModeScreenSharing:WmeRenderModeOriginal];
    
    [_audioCaptureVolumeSLD setIntegerValue:[_wmeDataProcess audioCaptureVolumeSLD]];
    [_audioPlayVolumeSLD setIntegerValue:[_wmeDataProcess audioPlayVolumeSLD]];
    
    [_wmeDataProcess setNetworkEnableNAT:FALSE];
    [_wmeDataProcess setRoleHostOrClient:TRUE];
    [_wmeDataProcess setActionSendAudio:TRUE];
    [_wmeDataProcess setActionSendVideo:TRUE];
    
    audioPlayVolumeMuteState = [_wmeDataProcess audioOutMute];
    audioCaptureVolumeMuteState = [_wmeDataProcess audioInMute];
    if (audioPlayVolumeMuteState == TRUE) {
        [_audioPlayVolumeSLD setEnabled:FALSE];
        [_audioPlayVolumeMuteBTN setTitle:@"Unmute"];
    }
    if (audioCaptureVolumeMuteState == TRUE) {
        [_audioCaptureVolumeSLD setEnabled:FALSE];
        [_audioCaptureVolumeMuteBTN setTitle:@"Unmute"];
    }
    
#ifdef ENABLE_COMMAND_LINE
    //Get the arguments from comman line
    NSArray *arryArgs = [[NSProcessInfo processInfo] arguments];
    int argc=0;
    const char * argv[10];
    
    for (NSString *option in arryArgs)
    {
        argv[argc] = [option UTF8String];
        argc ++;
    }
    
    _wmeDataProcess.clInfo = new CCustomCommandLineInfo;
    _wmeDataProcess.clInfo->ParseParam(argc, (char * const*)argv);
    
    if (_wmeDataProcess.clInfo->IsHost()) {
        printf("your role is host! \n");
        [_wmeDataProcess setRoleHostOrClient:TRUE];
    }
    else if (_wmeDataProcess.clInfo->IsClient()) {
        printf("your role is client! \n");
        printf("Connect to host ==> %s  \n", _wmeDataProcess.clInfo->ClientIP().c_str());
        [_wmeDataProcess setRoleHostOrClient:FALSE];
        [_roleHostIPTF setStringValue:[[NSString alloc] initWithCString:_wmeDataProcess.clInfo->ClientIP().c_str() encoding:NSUTF8StringEncoding]];
    }
    
    if (_wmeDataProcess.clInfo->IsSyslogEnabled()) {
        printf("enable system log! \n");
        printf("log system IP: %s  \n", _wmeDataProcess.clInfo->SyslogIP().c_str());
        _wmeDataProcess.logger = new Syslog(_wmeDataProcess.clInfo->SyslogIP().c_str());
        _wmeDataProcess.logger->log("New Client Instantiated");
    }
    if (_wmeDataProcess.clInfo->IsStunPoc()) {
        printf("enable stun poc! \n");
    }

    if (_wmeDataProcess.clInfo->IsNoRender()) {
        printf("enable no render mode! \n");
    }
    
    [self buttonConnect:nil];
#endif
}

- (void)awakeFromNib
{
    //TBD...CRASH CODES!!!!!!!
    [_videoCaptureDeviceCBX selectItemAtIndex:0];
    [_videoEncodingParamCBX selectItemAtIndex:0];
    [_audioCaptureDeviceCBX selectItemAtIndex:0];
    [_audioEncodingParamCBX selectItemAtIndex:0];
    [_audioPlayDeviceCBX selectItemAtIndex:0];
    [_shareList selectItemAtIndex:0];
    

}

//For textfield delegate

//For combobox delegate
- (void)comboBoxSelectionDidChange:(NSNotification *)notification
{
    [_wmeDataProcess setVideoCaptureDevice:[_videoCaptureDeviceCBX indexOfSelectedItem]];
    [_wmeDataProcess setVideoEncodingParam:[_videoEncodingParamCBX indexOfSelectedItem]];
    [_wmeDataProcess setAudioCaptureDevice:[_audioCaptureDeviceCBX indexOfSelectedItem]];
    [_wmeDataProcess setAudioEncodingParam:[_audioEncodingParamCBX indexOfSelectedItem]];
    [_wmeDataProcess setAudioPlayDevice:[_audioPlayDeviceCBX indexOfSelectedItem]];
    [_wmeDataProcess setActionMaxTraceLevel:[_actionMaxTraceLevelCBX indexOfSelectedItem]];
    [_wmeDataProcess setShareSourceIndex:[_shareList indexOfSelectedItem]];

    //printf("%d\n", [_videoCaptureDeviceCBX indexOfSelectedItem]);
}

//For matrix delegate

//For combobox datasource
- (NSInteger)numberOfItemsInComboBox:(NSComboBox *)aComboBox{
    if (aComboBox == _videoCaptureDeviceCBX) {
        return ([[_wmeDataProcess arrayVideoCaptureDeviceCBX] count]);
    }
    else if (aComboBox == _videoEncodingParamCBX)
    {
        return ([[_wmeDataProcess arrayVideoEncodingParamCBX] count]);
    }
    else if (aComboBox == _audioCaptureDeviceCBX)
    {
        return ([[_wmeDataProcess arrayAudioCaptureDeviceCBX] count]);
    }
    else if (aComboBox == _audioEncodingParamCBX)
    {
        return ([[_wmeDataProcess arrayAudioEncodingParamCBX] count]);
    }
    else if (aComboBox == _audioPlayDeviceCBX)
    {
        return ([[_wmeDataProcess arrayAudioPlayDeviceCBX] count]);
    }
    else if( aComboBox == _shareList )
    {
        return ([[_wmeDataProcess arrayShareSources] count]);
    }
    return 0;
}

- (id)comboBox:(NSComboBox *)aComboBox objectValueForItemAtIndex:(NSInteger)index
{
    if (aComboBox == _videoCaptureDeviceCBX) {
        return [[_wmeDataProcess arrayVideoCaptureDeviceCBX] objectAtIndex:index];
    }
    else if (aComboBox == _videoEncodingParamCBX)
    {
        return ([[_wmeDataProcess arrayVideoEncodingParamCBX] objectAtIndex:index]);
    }
    else if (aComboBox == _audioCaptureDeviceCBX)
    {
        return ([[_wmeDataProcess arrayAudioCaptureDeviceCBX] objectAtIndex:index]);
    }
    else if (aComboBox == _audioEncodingParamCBX)
    {
        return ([[_wmeDataProcess arrayAudioEncodingParamCBX] objectAtIndex:index]);
    }
    else if (aComboBox == _audioPlayDeviceCBX)
    {
        return ([[_wmeDataProcess arrayAudioPlayDeviceCBX] objectAtIndex:index]);
    }
    else if( aComboBox == _shareList )
    {
        return ([[_wmeDataProcess arrayShareSources] objectAtIndex:index]);
//        if( index == 0 )
//            return @"Main Screen";
//        else
//            return @"";
    }
    return 0;
    
}
- (NSUInteger)comboBox:(NSComboBox *)aComboBox indexOfItemWithStringValue:(NSString *)string
{
    if (aComboBox == _videoCaptureDeviceCBX) {
        return [[_wmeDataProcess arrayVideoCaptureDeviceCBX] indexOfObject:string];
    }
    else if (aComboBox == _videoEncodingParamCBX)
    {
        return ([[_wmeDataProcess arrayVideoEncodingParamCBX] indexOfObject:string]);
    }
    else if (aComboBox == _audioCaptureDeviceCBX)
    {
        return ([[_wmeDataProcess arrayAudioCaptureDeviceCBX] indexOfObject:string]);
    }
    else if (aComboBox == _audioEncodingParamCBX)
    {
        return ([[_wmeDataProcess arrayAudioEncodingParamCBX] indexOfObject:string]);
    }
    else if (aComboBox == _audioPlayDeviceCBX)
    {
        return ([[_wmeDataProcess arrayAudioPlayDeviceCBX] indexOfObject:string]);
    }
    else if( aComboBox == _shareList )
    {
        return ([[_wmeDataProcess arrayShareSources] indexOfObject:string]);
    }

    return 0;
}
//- (NSString *)comboBox:(NSComboBox *)aComboBox completedString:(NSString *)string;


//For IBAction
- (IBAction)sliderAudioCaptureVolume:(id)sender
{
    [_wmeDataProcess setAudioCaptureVolume:[_audioCaptureVolumeSLD integerValue]];
    //printf("slider value:%d\n", [_audioCaptureVolumeSLD intValue]);
}
- (IBAction)sliderAudioPlayVolume:(id)sender {
    [_wmeDataProcess setAudioPlayVolume:[_audioPlayVolumeSLD integerValue]];
}

- (IBAction)buttonAudioCaptureVolumeMute:(id)sender {
    if (audioCaptureVolumeMuteState == FALSE) {
        audioCaptureVolumeMuteState = TRUE;
        [_audioCaptureVolumeSLD setEnabled:FALSE];
        [_audioCaptureVolumeMuteBTN setTitle:@"Unmute"];
    }
    else //TRUE
    {
        audioCaptureVolumeMuteState = FALSE;
        [_audioCaptureVolumeSLD setEnabled:TRUE];
        [_audioCaptureVolumeMuteBTN setTitle:@"Mute"];
    }
    
    //Transfer info
    [_wmeDataProcess setAudioCaptureVolumeMute:audioCaptureVolumeMuteState];
}

- (IBAction)buttonAudioPlayVolumeMute:(id)sender {
    if (audioPlayVolumeMuteState == FALSE) {
        audioPlayVolumeMuteState = TRUE;
        [_audioPlayVolumeSLD setEnabled:FALSE];
        [_audioPlayVolumeMuteBTN setTitle:@"Unmute"];
    }
    else //TRUE
    {
        audioPlayVolumeMuteState = FALSE;
        [_audioPlayVolumeSLD setEnabled:TRUE];
        [_audioPlayVolumeMuteBTN setTitle:@"Mute"];
    }
    
    //Transfer info
    [_wmeDataProcess setAudioPlayVolumeMute:audioPlayVolumeMuteState];
}

- (IBAction)buttonConnect:(id)sender {

    //Close the preview window
    if (actionPreviewState == TRUE) {
        [self buttonPreview:_actionPreviewBTN];
    }
    
    CONNECTION_RETURN rtn;
    
    //Transfer the info
    [_wmeDataProcess setJINGLEServerIP:[_networkJINGLEServerIPTF stringValue]];
    [_wmeDataProcess setJINGLEServerPort:[_networkJINGLEServerPortTF stringValue]];
    [_wmeDataProcess setSTUNServerIP:[_networkSTUNServerIPTF stringValue]];
    [_wmeDataProcess setSTUNServerPort:[_networkSTUNServerPortTF stringValue]];
    [_wmeDataProcess setHostIPAddress:[_roleHostIPTF stringValue]];
    [_wmeDataProcess setMyName:[_roleMyNameTF stringValue]];
    [_wmeDataProcess setHostName:[_roleHostNameTF stringValue]];
    
    [_wmeDataProcess setEnableVideo:[self.videoEnableVideoCBX state]];
    [_wmeDataProcess setEnableAudio:[self.audioEnableAudioCBX state]];
    [_wmeDataProcess setEnableDSVideo:[self.screenEnableCBX state]];//
    
    rtn = [_wmeDataProcess clickedConnect];
    //Debug
    rtn = CONNECTION_SUCCESS;
    //Check the return value
    
    if (rtn == CONNECTION_SUCCESS) {
        //Update the control state
        [_actionConnectBTN setEnabled:FALSE];
        [_actionDisconnectBTN setEnabled:TRUE];
        [_actionPreviewBTN setEnabled:FALSE];
        
        //Save the state
        [_dictionaryState setObject:[NSNumber numberWithBool:[_networkEnableNATCBX isEnabled]] forKey:@"networkEnableNATCBXState"];
        [_dictionaryState setObject:[NSNumber numberWithBool:[_networkJINGLEServerIPTF isEnabled]] forKey:@"networkJINGLEServerIPTFState"];
        [_dictionaryState setObject:[NSNumber numberWithBool:[_networkJINGLEServerPortTF isEnabled]] forKey:@"networkJINGLEServerPortTFState"];
        [_dictionaryState setObject:[NSNumber numberWithBool:[_networkSTUNServerIPTF isEnabled]] forKey:@"networkSTUNServerIPTFState"];
        [_dictionaryState setObject:[NSNumber numberWithBool:[_networkSTUNServerPortTF isEnabled]] forKey:@"networkSTUNServerPortTFState"];
        [_dictionaryState setObject:[NSNumber numberWithBool:[_roleHostAndClientMTX isEnabled]] forKey:@"roleHostAndClientMTXState"];
        [_dictionaryState setObject:[NSNumber numberWithBool:[_roleHostIPTF isEnabled]] forKey:@"roleHostIPTFState"];
        [_dictionaryState setObject:[NSNumber numberWithBool:[_roleHostNameTF isEnabled]] forKey:@"roleHostNameTFState"];
        [_dictionaryState setObject:[NSNumber numberWithBool:[_roleMyNameTF isEnabled]] forKey:@"roleMyNameTFState"];
        
        //Disable network setting module
        [_networkEnableNATCBX setEnabled:FALSE];
        [_networkJINGLEServerIPTF setEnabled:FALSE];
        [_networkJINGLEServerPortTF setEnabled:FALSE];
        [_networkSTUNServerIPTF setEnabled:FALSE];
        [_networkSTUNServerPortTF setEnabled:FALSE];
        //Disable role setting module
        [_roleHostAndClientMTX setEnabled:FALSE];
        [_roleHostIPTF setEnabled:FALSE];
        [_roleHostNameTF setEnabled:FALSE];
        [_roleMyNameTF setEnabled:FALSE];

        //Disable the enable audio/video/screen
        [_videoEnableVideoCBX setEnabled:NO];
        [_audioEnableAudioCBX setEnabled:NO];
        [_screenEnableCBX setEnabled:NO];
        [_screenRenderModeMTX setEnabled:FALSE];
        
#ifdef ENABLE_COMMAND_LINE
        if (_wmeDataProcess.clInfo->IsNoRender() == false)
#else
        if (NSOnState == [_videoEnableVideoCBX state])
#endif
        {
            //Display the self-view window
            [_wmeSelfViewWindowController showWindow:self];
            [_wmeDataProcess setWindowForSelfView:[[_wmeSelfViewWindowController selfViewRenderWindow] layer]];
            
            //Display the attendee view window
            [_wmeAttendeeViewWindowController showWindow:self];
            [_wmeDataProcess setWindowForAttendeeView:[[_wmeAttendeeViewWindowController attendeeViewRenderWindow] layer]];
            
        }
        
#ifdef ENABLE_COMMAND_LINE
        if (_wmeDataProcess.clInfo->IsNoRender() == false)
#else
        //Display the screen attendee view window: host capture sending, attendee view.
        if([_roleHostAndClientMTX selectedColumn] == 1 && [_wmeDataProcess enableDSVideo]==YES){
            [[_wmeScreenAttendeeViewWindowController window] setTitle:@"View host's screen sharing"];
            [_wmeScreenAttendeeViewWindowController showWindow:self];
            [_wmeDataProcess setWindowForScreenAttendeeView:[[_wmeScreenAttendeeViewWindowController attendeeViewRenderWindow] layer]];
        }
#endif

    }
    else if (rtn == CONNECTION_FAILED)
    {
        //Pop window
        NSAlert *statusAlert = [[[NSAlert alloc] init] autorelease];
        [statusAlert setAlertStyle:NSCriticalAlertStyle];
        [statusAlert addButtonWithTitle:@"OK"];
        [statusAlert setMessageText:@"Connection Failed!"];
        [statusAlert runModal];
        
    }
    else if (rtn == CONNECTION_INPUTERROR)
    {
        //Pop window
        NSAlert *statusAlert = [[[NSAlert alloc] init] autorelease];
        [statusAlert setAlertStyle:NSCriticalAlertStyle];
        [statusAlert addButtonWithTitle:@"OK"];
        [statusAlert setMessageText:@"Input data is error!"];
        [statusAlert runModal];
    }

}

- (IBAction)buttonDisconnect:(id)sender {
    
    [_wmeDataProcess clickedDisconnect];

    //Update the control state
    [_actionConnectBTN setEnabled:TRUE];
    [_actionDisconnectBTN setEnabled:FALSE];
    
    //Disable the enable audio/video/screen
    [_videoEnableVideoCBX setEnabled:YES];
    [_audioEnableAudioCBX setEnabled:YES];
    [_screenEnableCBX setEnabled:YES];
    [_screenRenderModeMTX setEnabled:TRUE];
    if(NSOnState == [_videoEnableVideoCBX state])
    {
        [_actionPreviewBTN setEnabled:TRUE];
    }
    
    //Restore the control state
    [_networkEnableNATCBX setEnabled:[[_dictionaryState objectForKey:@"networkEnableNATCBXState"] boolValue]];
    [_networkJINGLEServerIPTF setEnabled:[[_dictionaryState objectForKey:@"networkJINGLEServerIPTFState"] boolValue]];
    [_networkJINGLEServerPortTF setEnabled:[[_dictionaryState objectForKey:@"networkJINGLEServerPortTFState"] boolValue]];
    [_networkSTUNServerIPTF setEnabled:[[_dictionaryState objectForKey:@"networkSTUNServerIPTFState"] boolValue]];
    [_networkSTUNServerPortTF setEnabled:[[_dictionaryState objectForKey:@"networkSTUNServerPortTFState"] boolValue]];
    [_roleHostAndClientMTX setEnabled:[[_dictionaryState objectForKey:@"roleHostAndClientMTXState"] boolValue]];
    [_roleHostIPTF setEnabled:[[_dictionaryState objectForKey:@"roleHostIPTFState"] boolValue]];
    [_roleHostNameTF setEnabled:[[_dictionaryState objectForKey:@"roleHostNameTFState"] boolValue]];
    [_roleMyNameTF setEnabled:[[_dictionaryState objectForKey:@"roleMyNameTFState"] boolValue]];

    [_wmeSelfViewWindowController close];
    [_wmeAttendeeViewWindowController close];
    [_wmeScreenAttendeeViewWindowController close];    
}
- (IBAction)checkboxEnableNAT:(id)sender {
    
    NSInteger state = [_networkEnableNATCBX state];
    
    [_wmeDataProcess setNetworkEnableNAT:state];
    
    if (state == TRUE) {
        [_networkJINGLEServerIPTF setEnabled:TRUE];
        [_networkJINGLEServerPortTF setEnabled:TRUE];
        [_networkSTUNServerIPTF setEnabled:TRUE];
        [_networkSTUNServerPortTF setEnabled:TRUE];
        [_roleMyNameTF setEnabled:TRUE];
        if ([_roleHostAndClientMTX selectedColumn] == 1) {
            [_roleHostNameTF setEnabled:TRUE];
        }

        [_roleHostIPTF setEnabled:FALSE];
    }
    else
    {
        [_networkJINGLEServerIPTF setEnabled:FALSE];
        [_networkJINGLEServerPortTF setEnabled:FALSE];
        [_networkSTUNServerIPTF setEnabled:FALSE];
        [_networkSTUNServerPortTF setEnabled:FALSE];
        [_roleMyNameTF setEnabled:FALSE];
        [_roleHostNameTF setEnabled:FALSE];
        if ([_roleHostAndClientMTX selectedColumn] == 1) {
            [_roleHostIPTF setEnabled:TRUE];
        }
    }
    
    
}
- (IBAction)checkboxSendVideo:(id)sender {
    NSInteger state = [self.actionSendVideoCBX state];
    
    //printf("state:%d\n", state);
    [_wmeDataProcess setActionSendVideo:state];
}

- (IBAction)checkboxSendAudio:(id)sender {
    NSInteger state = [self.actionSendAudioCBX state];
    
    printf("state:%d\n", state);
    [_wmeDataProcess setActionSendAudio:state];
}

- (IBAction)checkboxEnableScreen:(id)sender{
    
}
- (IBAction)checkboxEnableVideo:(id)sender {
    NSInteger state = [self.videoEnableVideoCBX state];
    
    printf("state:%d\n", state);
    if (state == YES) {
        [self.videoCaptureDeviceCBX setEnabled:YES];
        [self.videoEncodingParamCBX setEnabled:YES];
        [self.actionSendVideoCBX setEnabled:YES];
        [self.actionPreviewBTN setEnabled:YES];
        [_videoOptionButton setEnabled:YES];
    }
    else
    {
        [self.videoCaptureDeviceCBX setEnabled:NO];
        [self.videoEncodingParamCBX setEnabled:NO];
        [self.actionSendVideoCBX setEnabled:NO];
        [self.actionPreviewBTN setEnabled:NO];
        [_videoOptionButton setEnabled:NO];
        [_videoOptionDrawer close];
    }
}

- (IBAction)checkboxEnableAudio:(id)sender {
    NSInteger state = [self.audioEnableAudioCBX state];
    
    //printf("state:%d\n", state);
    if (state == YES) {
        [self.audioCaptureDeviceCBX setEnabled:YES];
        [self.audioCaptureVolumeSLD setEnabled:YES];
        [self.audioEncodingParamCBX setEnabled:YES];
        [self.audioPlayDeviceCBX setEnabled:YES];
        [self.audioPlayVolumeSLD setEnabled:YES];
        [self.audioCaptureVolumeMuteBTN setEnabled:YES];
        [self.audioPlayVolumeMuteBTN setEnabled:YES];
        [self.actionSendAudioCBX setEnabled:YES];
        [_audioOptionButton setEnabled:YES];
    }
    else
    {
        [self.audioCaptureDeviceCBX setEnabled:NO];
        [self.audioCaptureVolumeSLD setEnabled:NO];
        [self.audioEncodingParamCBX setEnabled:NO];
        [self.audioPlayDeviceCBX setEnabled:NO];
        [self.audioPlayVolumeSLD setEnabled:NO];
        [self.audioCaptureVolumeMuteBTN setEnabled:NO];
        [self.audioPlayVolumeMuteBTN setEnabled:NO];
        [self.actionSendAudioCBX setEnabled:NO];
        [_audioOptionButton setEnabled:NO];
        [_audioOptionDrawer close];
    }
}

- (IBAction)buttonPreview:(id)sender {
    if (actionPreviewState == FALSE) {
        actionPreviewState = TRUE;
        [_wmePreviewWindowController showWindow:self];
        [_wmeDataProcess setWindowForPreview:[[_wmePreviewWindowController previewRenderWindow] layer]];
    }
    else //TRUE
    {
        [_wmePreviewWindowController close];
        actionPreviewState = FALSE;
    }
    
    [_wmeDataProcess clickedPreview:actionPreviewState];
}

- (IBAction)matrixRenderModeScreenSharing:(id)sender{
    NSInteger column = [_screenRenderModeMTX selectedColumn];
    WmeTrackRenderScalingModeType renderMode = WmeRenderModeOriginal;
    
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
    [_wmeDataProcess setRenderModeScreenSharing: renderMode];
}


- (IBAction)matrixHostOrClient:(id)sender {
    NSInteger column = [_roleHostAndClientMTX selectedColumn];

    if (column == 0) {
        [_roleHostNameTF setEnabled:FALSE];
        [_roleHostIPTF setEnabled:FALSE];
        [_wmeDataProcess setRoleHostOrClient:TRUE];
    }
    else if (column == 1) {
        if ([_networkEnableNATCBX state] == TRUE) {
            [_roleHostNameTF setEnabled:TRUE];
        }
        else
        {
            [_roleHostIPTF setEnabled:TRUE];
        }

        [_wmeDataProcess setRoleHostOrClient:FALSE];
    }
}

- (IBAction)selectInputVideoFile:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    
    [openPanel setAllowedFileTypes:[NSArray arrayWithObjects:@"yuv", @"rgb", nil]];
    [openPanel setCanSelectHiddenExtension:YES];
    
    [openPanel beginSheetModalForWindow:[self window] completionHandler:^(NSInteger result)
     {
         if (result == NSOKButton)
         {
         //    NSLog(@"Select input video file, url: %@", [[openPanel URL] path]);
             [_videoInputFileLabel setTitleWithMnemonic:[[openPanel URL] path]];
             [_wmeDataProcess setVideoInputFile:[[openPanel URL] path]];
         }
         else
         {
         //    NSLog(@"Cancel to select input video file.");
         }
     }];
}

- (IBAction)clearInputVideoFile:(id)sender
{
    [_videoInputFileLabel setTitleWithMnemonic:@""];
    [_wmeDataProcess setVideoInputFile:@""];
}

- (IBAction)setOutputVideoFile:(id)sender
{
    NSSavePanel *savePanel = [NSSavePanel savePanel];
    
    [savePanel setAllowedFileTypes:[NSArray arrayWithObject:@"rgb"]];
    [savePanel setCanSelectHiddenExtension:YES];
    
    [savePanel beginSheetModalForWindow:[self window] completionHandler:^(NSInteger result)
     {
         if (result == NSOKButton)
         {
         //    NSLog(@"Set output video file, url: %@", [[savePanel URL] path]);
             [_videoOutputFileLabel setTitleWithMnemonic:[[savePanel URL] path]];
             [_wmeDataProcess setVideoOutputFile:[[savePanel URL] path]];
         }
         else
         {
         //    NSLog(@"Cancel to set output video file.");
         }
     }];
}

- (IBAction)clearOutputVideoFile:(id)sender
{
    [_videoOutputFileLabel setTitleWithMnemonic:@""];
    [_wmeDataProcess setVideoOutputFile:@""];
}

- (IBAction)selectInputAudioFile:(id)sender
{
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    
    [openPanel setAllowedFileTypes:[NSArray arrayWithObject:@"pcm"]];
    [openPanel setCanSelectHiddenExtension:YES];
    
    [openPanel beginSheetModalForWindow:[self window] completionHandler:^(NSInteger result)
     {
         if (result == NSOKButton)
         {
         //    NSLog(@"Select input audio file, url: %@", [[openPanel URL] path]);
             [_audioInputFileLabel setTitleWithMnemonic:[[openPanel URL] path]];
             [_wmeDataProcess setAudioInputFile:[[openPanel URL] path]];
         }
         else
         {
         //    NSLog(@"Cancel to select input audio file.");
         }
     }];
}

- (IBAction)clearInputAudioFile:(id)sender
{
    [_audioInputFileLabel setTitleWithMnemonic:@""];
    [_wmeDataProcess setAudioInputFile:@""];
}

- (IBAction)setOutputAudioFile:(id)sender
{
    NSSavePanel *savePanel = [NSSavePanel savePanel];
    
    [savePanel setAllowedFileTypes:[NSArray arrayWithObject:@"pcm"]];
    [savePanel setCanSelectHiddenExtension:YES];
    
    [savePanel beginSheetModalForWindow:[self window] completionHandler:^(NSInteger result)
     {
         if (result == NSOKButton)
         {
         //    NSLog(@"Set output audio file, url: %@", [[savePanel URL] path]);
             [_audioOutputFileLabel setTitleWithMnemonic:[[savePanel URL] path]];
             [_wmeDataProcess setAudioOutputFile:[[savePanel URL] path]];
         }
         else
         {
         //    NSLog(@"Cancel to set output audio file.");
         }
     }];
}

- (IBAction)clearOutputAudioFile:(id)sender
{
    [_audioOutputFileLabel setTitleWithMnemonic:@""];
    [_wmeDataProcess setAudioOutputFile:@""];
}

//For UIChangeNotifierDelegate
- (void)OnVolumeChange:(WmeEventDataVolumeChanged &) changeEvent
{
    if (changeEvent.iDeviceType == WmeDeviceIn) {
        if (changeEvent.iChangedType == WmeAudioVolumeChange) {
            [_audioCaptureVolumeSLD setIntegerValue:changeEvent.nVolume];
        }
        else if(changeEvent.iChangedType == WmeAudioVolumeMute)
        {
            audioCaptureVolumeMuteState = changeEvent.bMute;
            if (audioCaptureVolumeMuteState == TRUE) {
                [_audioCaptureVolumeSLD setEnabled:FALSE];
                [_audioCaptureVolumeMuteBTN setTitle:@"Unmute"];
            }
            else
            {
                [_audioCaptureVolumeSLD setEnabled:TRUE];
                [_audioCaptureVolumeMuteBTN setTitle:@"Mute"];
            }
        }
    }
    else if (changeEvent.iDeviceType == WmeDeviceOut)
    {
        if (changeEvent.iChangedType == WmeAudioVolumeChange) {
            [_audioPlayVolumeSLD setIntegerValue:changeEvent.nVolume];
        }
        else if(changeEvent.iChangedType == WmeAudioVolumeMute)
        {
            audioPlayVolumeMuteState = changeEvent.bMute;
            
            if (audioPlayVolumeMuteState == TRUE) {
                [_audioPlayVolumeSLD setEnabled:FALSE];
                [_audioPlayVolumeMuteBTN setTitle:@"Unmute"];
            }
            else
            {
                [_audioPlayVolumeSLD setEnabled:TRUE];
                [_audioPlayVolumeMuteBTN setTitle:@"Mute"];
            }
        }
    }

}

- (void)networkDisconnectFromClient:(DEMO_MEDIA_TYPE) eType
{
    //TBD...
}

- (void)connectRemoteFailureFromClient:(DEMO_MEDIA_TYPE) eType
{
    //TBD...
}

- (void)OnDeviceChanged:(DeviceProperty *) pDP event:(WmeEventDataDeviceChanged &) changeEvent index:(NSInteger)selectedIndex
{

    switch (pDP->dev_type) {
        case DEV_TYPE_CAMERA:
            [_videoCaptureDeviceCBX reloadData];
            if (selectedIndex < 0) {
                [_videoCaptureDeviceCBX setObjectValue:nil];
                return;
            }
            [_videoCaptureDeviceCBX selectItemAtIndex:selectedIndex];
            [_videoCaptureDeviceCBX setObjectValue:[[_videoCaptureDeviceCBX dataSource] comboBox:_videoCaptureDeviceCBX objectValueForItemAtIndex:selectedIndex]];
            break;
        case DEV_TYPE_MIC:
            [_audioCaptureDeviceCBX reloadData];
            if (selectedIndex < 0) {
                [_audioCaptureDeviceCBX setObjectValue:nil];
                 return;
            }
            [_audioCaptureDeviceCBX selectItemAtIndex:selectedIndex];
            [_audioCaptureDeviceCBX setObjectValue:[[_audioCaptureDeviceCBX dataSource] comboBox:_audioCaptureDeviceCBX objectValueForItemAtIndex:selectedIndex]];
            break;
        case DEV_TYPE_SPEAKER:
            [_audioPlayDeviceCBX reloadData];
            if (selectedIndex < 0) {
                [_audioPlayDeviceCBX setObjectValue:nil];
                return;
            }
            [_audioPlayDeviceCBX selectItemAtIndex:selectedIndex];
            [_audioPlayDeviceCBX setObjectValue:[[_audioPlayDeviceCBX dataSource] comboBox:_audioPlayDeviceCBX objectValueForItemAtIndex:selectedIndex]];
            break;
        default:
            return;
    }
    
    //Update the selected item


}

- (void)OnDecodeSizeChanged:(uint32_t)uLabel width:(uint32_t)uWidth height:(uint32_t)uHeight
{
    if(WmeRenderModeOriginal == [_wmeDataProcess getRenderModeScreenSharing])
    {
        [[_wmeScreenAttendeeViewWindowController window] setFrame: NSMakeRect(0, 0, uWidth, uHeight) display: true];
    }
}

/*
- (IBAction)checkboxDumpData:(id)sender {
    [_wmeDataProcess SetDumpDataEnabled:[self.actionDumpDataCBX state]];
}*/
@end
