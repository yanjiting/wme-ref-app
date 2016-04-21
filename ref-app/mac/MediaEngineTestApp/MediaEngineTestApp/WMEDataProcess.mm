//
//  WMEDataProcess.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-8-6.
//  Copyright (c) 2013年 video. All rights reserved.
//
//#define STATIC_DEBUG

#import "WMEDataProcess.h"

@implementation WMEDataProcess
@synthesize delegate                   = _delegate;
@synthesize arrayVideoCaptureDeviceCBX = _arrayVideoCaptureDeviceCBX;
@synthesize arrayVideoEncodingParamCBX = _arrayVideoEncodingParamCBX;
@synthesize arrayAudioCaptureDeviceCBX = _arrayAudioCaptureDeviceCBX;
@synthesize arrayAudioEncodingParamCBX = _arrayAudioEncodingParamCBX;
@synthesize arrayAudioPlayDeviceCBX    = _arrayAudioPlayDeviceCBX;
@synthesize arrayShareSources = _arrayShareSources;
@synthesize audioCaptureVolumeSLD      = _audioCaptureVolumeSLD;
@synthesize audioPlayVolumeSLD         = _audioPlayVolumeSLD;
@synthesize JINGLEServerIP             = _JINGLEServerIP;
@synthesize JINGLEServerPort           = _JINGLEServerPort;
@synthesize STUNServerIP               = _STUNServerIP;
@synthesize STUNServerPort             = _STUNServerPort;
@synthesize HostIPAddress              = _HostIPAddress;
@synthesize MyName                     = _MyName;
@synthesize HostName                   = _HostName;
@synthesize audioOutMute               = _audioOutMute;
@synthesize audioInMute                = _audioInMute;
@synthesize enableVideo                = _enableVideo;
@synthesize enableAudio                = _enableAudio;
@synthesize enableDSVideo              = _enableDSVideo;
#ifdef ENABLE_COMMAND_LINE
@synthesize logger                     = _logger;
@synthesize clInfo                     = _clInfo;
#endif
- (long)initWME
{
#ifdef ENABLE_COMMAND_LINE
    _logger = NULL;
    _clInfo = NULL;
#endif
    _demoClientSink = new DemoClientSink(self);
    if (!_demoClientSink) {
        return WME_E_FAIL;
    }
    
    _demoClient = new DemoClient(_demoClientSink);
    if (!_demoClient || _demoClient->Init() != WME_S_OK) {
        return WME_E_FAIL;
    }
    
    [self queryAudioInDevices];
    [self queryAudioOutDevices];
    [self queryVideoDevices];
    [self queryCapabilities];
    
    return WME_S_OK;
}

- (void)uninitWME
{
#ifdef ENABLE_COMMAND_LINE
    if (_logger != NULL)
    {
        delete _logger;
        _logger = NULL;
    }
    if (_clInfo != NULL)
    {
        delete _clInfo;
        _clInfo = NULL;
    }
#endif
    if (_demoClient) {
        _demoClient->UnInit();
        delete _demoClient;
        _demoClient = NULL;
    }
    
    if (_demoClientSink) {
        delete _demoClientSink;
        _demoClientSink = NULL;
    }
}

- (void)SetDumpDataEnabled:(unsigned int)uDataDumpFlag
{
    if (_demoClient) {
        _demoClient->SetDumpDataEnabled(uDataDumpFlag);
    }
}

- (void)queryAudioInDevices
{
    if (_demoClient)
    {
        _demoClient->ClearDeviceList(DEMO_MEDIA_AUDIO, _audioInList);
        _demoClient->GetDeviceList(DEMO_MEDIA_AUDIO, DEV_TYPE_MIC, _audioInList);
    }
}

- (void)queryAudioOutDevices
{
    if (_demoClient)
    {
        _demoClient->ClearDeviceList(DEMO_MEDIA_AUDIO, _audioOutList);
        _demoClient->GetDeviceList(DEMO_MEDIA_AUDIO, DEV_TYPE_SPEAKER, _audioOutList);
    }
}

- (void)queryVideoDevices
{
    if (_demoClient)
    {
        _demoClient->ClearDeviceList(DEMO_MEDIA_VIDEO, _videoInList);
        _demoClient->GetDeviceList(DEMO_MEDIA_VIDEO, DEV_TYPE_CAMERA, _videoInList);
    }
}

- (void)queryCapabilities
{
    if (_demoClient)
    {
        _audioCapList.clear();
        _videoCapList.clear();
        _demoClient->GetMediaCapabilities(DEMO_MEDIA_AUDIO, &_audioCapList);
        _demoClient->GetMediaCapabilities(DEMO_MEDIA_VIDEO, &_videoCapList);
    }
}

- (void) selectAudioInDevice:(long)index
{
    if (_demoClient && index >= 0 && index < _audioInList.size())
    {
        DeviceProperty *dp = &(_audioInList.at(index));
        _demoClient->SetMic(dp->dev);
    }    
}

- (void) selectAudioOutDevice:(long)index
{
    if (_demoClient && index >= 0 && index < _audioOutList.size())
    {
        DeviceProperty *dp = &(_audioOutList.at(index));
        _demoClient->SetSpeaker(dp->dev);
    }
}

///
/// only valid for DEMO_LOCAL_TRACK and DEMO_PREVIEW_TRACK
- (void) selectVideoDevice:(long)index track:(DEMO_TRACK_TYPE)ttype
{
    if (_demoClient && index >= 0 && index < _videoInList.size())
    {
        DeviceProperty *dp = &(_videoInList.at(index));
        _demoClient->SetCamera(ttype, dp->dev);
    }
}

- (void)selectAudioCapability:(long)index track:(DEMO_TRACK_TYPE)ttype
{
    if (_demoClient && index >= 0 && index < _audioCapList.size())
    {
        WmeAudioMediaCapability *cap = &(_audioCapList.at(index));
        _demoClient->SetMediaCodec(DEMO_MEDIA_AUDIO, ttype, cap->eCodecType);
        _demoClient->SetMediaSession(DEMO_MEDIA_AUDIO, ttype, cap->eCodecType);
        
        if (ttype == DEMO_LOCAL_TRACK )
        {
            _demoClient->SetMediaCapability(DEMO_MEDIA_AUDIO, ttype, cap);
        }
    }
}

- (void)selectVideoCapability:(long)index track:(DEMO_TRACK_TYPE)ttype
{
    if (_demoClient && index >= 0 && index < _videoCapList.size())
    {
        WmeVideoMediaCapability *cap = &(_videoCapList.at(index));
        _demoClient->SetMediaCodec(DEMO_MEDIA_VIDEO, ttype, cap->eCodecType);
        _demoClient->SetMediaSession(DEMO_MEDIA_VIDEO, ttype, cap->eCodecType);
        
        if (ttype == DEMO_LOCAL_TRACK)
        {
            _demoClient->SetMediaCapability(DEMO_MEDIA_VIDEO, ttype, cap);
        }
    }
}

- (void)setAudioInVolume:(long)value
{
    if (_demoClient)
        _demoClient->SetAudioVolume(DEV_TYPE_MIC, value);
        
}
- (void)setAudioOutVolume:(long)value
{
    if (_demoClient)
        _demoClient->SetAudioVolume(DEV_TYPE_SPEAKER, value);
}
- (void)setAudioInMuteOrUnmute:(bool)bmute
{
    if (_demoClient)
    {
        _demoClient->MuteAudio(DEV_TYPE_MIC, bmute);
    }
}
- (void)setAudioOutMuteOrUnmute:(bool)bmute
{
    if (_demoClient)
    {
        _demoClient->MuteAudio(DEV_TYPE_SPEAKER, bmute);
    }
}


- (void) networkDisconnectFromClient:(DEMO_MEDIA_TYPE) eType
{
    /// notify UI to process
    [_delegate networkDisconnectFromClient:eType];
}

- (void) connectRemoteFailureFromClient:(DEMO_MEDIA_TYPE) eType
{
    /// notify UI to process
    [_delegate connectRemoteFailureFromClient:eType];
}

- (void) OnDeviceChanged:(DeviceProperty *) pDP event:(WmeEventDataDeviceChanged &) changeEvent
{
    if (!pDP)
        return;
    
    int selectedIndex = -1;
    DemoClient::DevicePropertyList *dplist = NULL;
    
    switch (pDP->dev_type) {
        case DEV_TYPE_CAMERA:
            dplist = &_videoInList;
            selectedIndex = _videoCaptureIndex;
            break;
        case DEV_TYPE_MIC:
            dplist = &_audioInList;
            selectedIndex = _audioCaptureIndex;
            break;
        case DEV_TYPE_SPEAKER:
            dplist = &_audioOutList;
            selectedIndex = _audioPlayIndex;
            break;
        default:
            return;
    }
    
#if 1
    switch(changeEvent.iType)
    {
        case wme::WmeDeviceAdded:
        {
            DeviceProperty dp;
            memcpy(&dp, pDP, sizeof(DeviceProperty));
            dp.dev = pDP->dev;
            dp.dev->AddRef();
            dplist->push_back(dp);
            
            switch (pDP->dev_type) {
                case DEV_TYPE_CAMERA:
                    [self getVideoCaptureDeviceCBX];
                    //_videoCaptureIndex = selectedIndex;
                    break;
                case DEV_TYPE_MIC:
                    [self getAudioCaptureDeviceCBX];
                    //_audioCaptureIndex = selectedIndex;
                    break;
                case DEV_TYPE_SPEAKER:
                    [self getAudioPlayDeviceCBX];
                    //_audioPlayIndex = selectedIndex;
                    break;
                default:
                    return;
            }
            break;
        }
        case wme::WmeDeviceRemoved:
        {
            DemoClient::DevicePropertyList::iterator iter = dplist->begin();
            for (int index = 0; iter != dplist->end(); iter++, index++)
            {
                if (iter->dev->IsSameDevice(pDP->unique_name, pDP->unique_name_len) == WME_S_OK)
                {
                    iter->dev->Release();
                    dplist->erase(iter);
                    // in using
                    if (selectedIndex < index)
                    {}
                    else
                    {
                        if (!dplist->empty())
                            selectedIndex = (selectedIndex) % dplist->size();
                        else
                            selectedIndex = -1;
                    }
                    break;
                }
            }
            switch (pDP->dev_type) {
                case DEV_TYPE_CAMERA:
                    [self getVideoCaptureDeviceCBX];
                    _videoCaptureIndex = selectedIndex;
                    [self selectVideoDevice:selectedIndex track:m_vtrack];
                    break;
                case DEV_TYPE_MIC:
                    [self getAudioCaptureDeviceCBX];
                    _audioCaptureIndex = selectedIndex;
                    [self selectAudioInDevice:selectedIndex];
                    break;
                case DEV_TYPE_SPEAKER:
                    [self getAudioPlayDeviceCBX];
                    _audioPlayIndex = selectedIndex;
                    [self selectAudioOutDevice:selectedIndex];
                    break;
                default:
                    break;
            }
            break;
        }
        case wme::WmeDefaultDeviceChanged:
        {
            DemoClient::DevicePropertyList::iterator iter = dplist->begin();
            for (int index = 0; iter != dplist->end(); iter++, index++)
            {
                if (pDP->dev->IsSameDevice(iter->dev) == WME_S_OK && pDP->is_default_dev)
                {
                    if (index == selectedIndex)
                    {
                        // in using
                    }
                    break;
                }
            }
            break;
        }
        default:
            break;
    }
#endif
    
    [_delegate OnDeviceChanged:pDP event:changeEvent index:selectedIndex];
}
- (void)OnVolumeChange:(WmeEventDataVolumeChanged &) changeEvent
{
    [_delegate OnVolumeChange:changeEvent];
    
    if (changeEvent.iDeviceType == WmeDeviceIn) {
        
        if(changeEvent.iChangedType == WmeAudioVolumeMute)
        {
            _audioInMute = changeEvent.bMute;
        }
    }
    else if (changeEvent.iDeviceType == WmeDeviceOut)
    {
        if(changeEvent.iChangedType == WmeAudioVolumeMute)
        {
            _audioOutMute = changeEvent.bMute;
        }
    }
}
- (void)OnDecodeSizeChanged:(uint32_t)uLabel width:(uint32_t)uWidth height:(uint32_t)uHeight
{
    [_delegate OnDecodeSizeChanged:uLabel width:uWidth height:uHeight];
}

- (void)initDataSource
{
    _arrayVideoCaptureDeviceCBX = [[NSMutableArray alloc] init];
    _arrayVideoEncodingParamCBX = [[NSMutableArray alloc] init];
    _arrayAudioCaptureDeviceCBX = [[NSMutableArray alloc] init];
    _arrayAudioEncodingParamCBX = [[NSMutableArray alloc] init];
    _arrayAudioPlayDeviceCBX = [[NSMutableArray alloc] init];
    
    _audioCaptureVolumeSLD = 50;
    _audioPlayVolumeSLD = 50;
    
    _demoClient = NULL;
    _demoClientSink = NULL;
    
    //
    // default value for options
    _videoCaptureIndex = 0;
    _videoEncodingIndex = 0;
    _audioCaptureIndex = 0;
    _audioEncodingIndex = 0;
    _audioPlayIndex = 0;
    
    m_vtrack = DEMO_LOCAL_TRACK;
    
    _audioInMute = FALSE;
    _audioOutMute = FALSE;
    
    _isHost = TRUE;
    _enableNAT = FALSE;
    _sendAudio = TRUE;
    _sendVideo = TRUE;
    
    _eRenderModeScreenSharing = WmeRenderModeOriginal;
    
    [self initWME];
}

- (void)getDataSource
{
    //Get the capture
    [self getVideoCaptureDeviceCBX];
    [self getVideoEncodingParamCBX];
    [self getAudioCaptureDeviceCBX];
    [self getAudioEncodingParamCBX];
    [self getAudioPlayDeviceCBX];
    [self getShareSources];
    [self getAudioCaptureVolumeSLD];
    [self getAudioPlayVolumeSLD];
}

- (void)getVideoCaptureDeviceCBX
{
#ifdef STATIC_DEBUG
    [_arrayVideoCaptureDeviceCBX addObject:@"VideoCaptureDevice0"];
    [_arrayVideoCaptureDeviceCBX addObject:@"VideoCaptureDevice1"];
    [_arrayVideoCaptureDeviceCBX addObject:@"VideoCaptureDevice2"];
#else
    [_arrayVideoCaptureDeviceCBX removeAllObjects];
     
    // video device list
    int defaultIndex = 0;
    for (int k=0; k < _videoInList.size(); k++) {
        DeviceProperty *dev = &(_videoInList.at(k));
        if (dev->is_default_dev) {
            defaultIndex = k;
        }
        [_arrayVideoCaptureDeviceCBX  addObject:[[NSString alloc] initWithCString:dev->dev_name encoding:NSUTF8StringEncoding]];
    }
    _videoCaptureIndex = defaultIndex;
#endif
}

- (void)getVideoEncodingParamCBX
{
#ifdef STATIC_DEBUG
    [_arrayVideoEncodingParamCBX addObject:@"VideoEncodingParam0"];
    [_arrayVideoEncodingParamCBX addObject:@"VideoEncodingParam1"];
    [_arrayVideoEncodingParamCBX addObject:@"VideoEncodingParam2"];
#else
    [_arrayVideoEncodingParamCBX removeAllObjects];
    
    // video cap list
    for (int k=0; k < _videoCapList.size(); k++) {
        WmeVideoMediaCapability *cap = &(_videoCapList.at(k));
        if (cap->eCodecType == WmeCodecType_Unknown) {
            continue;
        }
        
        float fps = cap->frame_layer[cap->frame_layer_number-1] * 1.0 / 100;
        char strFmt[128];
        sprintf(strFmt, "%s, %dx%d@%1.ffps", kWmeVideoCodecTag[cap->eCodecType-WmeCodecType_AVC], (int)cap->width, (int)cap->height, fps);
        [_arrayVideoEncodingParamCBX addObject:[[NSString alloc] initWithCString:strFmt encoding:NSUTF8StringEncoding]];
    }
#endif    
}

- (void)getAudioCaptureDeviceCBX
{
#ifdef STATIC_DEBUG
    [_arrayAudioCaptureDeviceCBX addObject:@"AudioCaptureDevice0"];
    [_arrayAudioCaptureDeviceCBX addObject:@"AudioCaptureDevice1"];
    [_arrayAudioCaptureDeviceCBX addObject:@"AudioCaptureDevice2"];
#else
    [_arrayAudioCaptureDeviceCBX removeAllObjects];
    
    // audio device list         
    int defaultIndex = 0;
    for (int k=0; k < _audioInList.size(); k++) {
        DeviceProperty *dev = &(_audioInList.at(k));
        if (dev->is_default_dev) {
            defaultIndex = k;
        }
        [_arrayAudioCaptureDeviceCBX  addObject:[[NSString alloc] initWithCString:dev->dev_name encoding:NSUTF8StringEncoding]];
    }
    _audioCaptureIndex = defaultIndex;
#endif
    
}

- (void)getAudioEncodingParamCBX
{
#ifdef STATIC_DEBUG
    [_arrayAudioEncodingParamCBX addObject:@"AudioEncodingParam0"];
    [_arrayAudioEncodingParamCBX addObject:@"AudioEncodingParam1"];
    [_arrayAudioEncodingParamCBX addObject:@"AudioEncodingParam2"];
#else
    [_arrayAudioEncodingParamCBX removeAllObjects];
    
    // audio cap list    
    for (int k=0; k < _audioCapList.size(); k++) {
        WmeAudioMediaCapability *cap = &(_audioCapList.at(k));
        char strFmt[128];
        sprintf(strFmt, "%s, sample freq: %dKHz, bitrate: %dKbps",
                cap->stdname, cap->clockrate/1000, cap->rate/1000);
        [_arrayAudioEncodingParamCBX addObject:[[NSString alloc] initWithCString:strFmt encoding:NSUTF8StringEncoding]];
    }
#endif
}

- (void)getAudioPlayDeviceCBX
{
#ifdef STATIC_DEBUG
    [_arrayAudioPlayDeviceCBX addObject:@"AudioPlayDevice0"];
    [_arrayAudioPlayDeviceCBX addObject:@"AudioPlayDevice1"];
    [_arrayAudioPlayDeviceCBX addObject:@"AudioPlayDevice2"];
#else
    [ _arrayAudioPlayDeviceCBX removeAllObjects];
    
    // audio device list        
    int defaultIndex = 0;
    for (int k=0; k < _audioOutList.size(); k++) {
        DeviceProperty *dev = &(_audioOutList.at(k));
        if (dev->is_default_dev) {
            defaultIndex = k;
        }
        [_arrayAudioPlayDeviceCBX  addObject:[[NSString alloc] initWithCString:dev->dev_name encoding:NSUTF8StringEncoding]];
    }
    _audioPlayIndex = defaultIndex;
#endif
}

-(void)getShareSources
{
    if( _demoClient )
    {
//        _demoClient->GetScreenSources();
        _appSource.clear();
        _demoClient->GetShareSourceManager()->GetAppSourceSourceNames(_appSource);
        NSMutableArray *arySources = [NSMutableArray array];
        [arySources addObject:@"Main Screen"];
        for( std::vector<std::string>::iterator i=_appSource.begin(); i!=_appSource.end(); i++ )
        {
            [arySources addObject:[NSString stringWithUTF8String: (*i).c_str()]];
        }
        [_arrayShareSources release];
        _arrayShareSources = [[NSArray alloc] initWithArray:arySources];
    }
}

- (void)getAudioCaptureVolumeSLD
{
#ifdef STATIC_DEBUG
    _audioCaptureVolumeSLD = 65535/2;
#else
    if (_demoClient)
    {
        _audioInMute = _demoClient->IsAudioMute(DEV_TYPE_MIC);
        int volume = 0; // fix for 64bit
        _demoClient->GetAudioVolume(DEV_TYPE_MIC, volume);
        _audioCaptureVolumeSLD = volume;
    }
#endif
}
- (void)getAudioPlayVolumeSLD
{
#ifdef STATIC_DEBUG
    _audioPlayVolumeSLD = 65535/2;
#else
    if (_demoClient)
    {
        _audioOutMute = _demoClient->IsAudioMute(DEV_TYPE_SPEAKER);
        int volume = 0; // fix for 64bit
        _demoClient->GetAudioVolume(DEV_TYPE_SPEAKER, volume);
        _audioPlayVolumeSLD = volume;
    }
#endif
}

- (void)setVideoCaptureDevice:(NSInteger)index
{
    if (index >= 0 && _videoCaptureIndex != index)
    {
        _videoCaptureIndex = index;
        [self selectVideoDevice:index track:m_vtrack];
    }
}
- (void)setVideoEncodingParam:(NSInteger)index
{
    if (index >= 0 && _videoEncodingIndex != index)
    {
        _videoEncodingIndex = index;
        [self selectVideoCapability:index track:DEMO_LOCAL_TRACK];
    }
}
- (void)setAudioCaptureDevice:(NSInteger)index
{
    if (index >= 0 && _audioCaptureIndex != index)
    {
        _audioCaptureIndex = index;
        [self selectAudioInDevice:index];
    }
    
}
- (void)setAudioEncodingParam:(NSInteger)index
{
    if (index >= 0 && _audioEncodingIndex != index)
    {
        _audioEncodingIndex = index;
        [self selectAudioCapability:index track:DEMO_LOCAL_TRACK];
    }
}
- (void)setAudioPlayDevice:(NSInteger)index
{
    if (index >= 0 && _audioPlayIndex != index)
    {
        _audioPlayIndex = index;
        [self selectAudioOutDevice:index];
    }
}

- (void)setAudioCaptureVolume:(NSInteger)number
{
    if (_audioCaptureVolumeSLD != number)
    {
        _audioCaptureVolumeSLD = number;
        [self setAudioInVolume:number];
    }
}
- (void)setAudioPlayVolume:(NSInteger)number
{
    if(_audioPlayVolumeSLD != number)
    {
        _audioPlayVolumeSLD = number;
        [self setAudioOutVolume:number];
    }
}
- (void)setAudioPlayVolumeMute:(BOOL)state
{
    if (_audioOutMute != state)
    {
        _audioOutMute = state;
        [self setAudioOutMuteOrUnmute:state];
    }
}
- (void)setAudioCaptureVolumeMute:(BOOL)state
{
    if (_audioInMute != state)
    {
        _audioInMute = state;
        [self setAudioInMuteOrUnmute:state];
    }
}

- (void)setAudioInputFile:(NSString*)path
{
    if (NO == [path isEqualToString:@""]) {
        NSArray* components = [[path lastPathComponent] componentsSeparatedByString:@"_"];
        if ([components count] < 4)
        {
        	NSLog(@"setAudioInputFile, file name format isn't supported ([Channels]_[SampleRate]_[BitsPerSample]_XXX.pcm), %@", path);
            return;
        }
        
        WmeAudioRawFormat fmt;
        fmt.eRawType = WmePCM;
        fmt.iChannels = [[components objectAtIndex:0] integerValue];
        fmt.iSampleRate = [[components objectAtIndex:1] integerValue];
        fmt.iBitsPerSample = [[components objectAtIndex:2] integerValue];
        if (fmt.iChannels == 0 || fmt.iSampleRate == 0 || fmt.iBitsPerSample == 0) {
        	NSLog(@"setAudioInputFile, audio format is invalid, iChannels = %d, iBitsPerSample = %d, frame_rate = %d", fmt.iChannels, fmt.iSampleRate, fmt.iBitsPerSample);
            return;
        }
        
        _demoClient->SetAudioSource(SOURCE_FILE);
        _demoClient->SetAudioInputFile([path UTF8String], &fmt);
        
    }else{
        _demoClient->SetAudioSource(SOURCE_MIC);
    }
}

- (void)setAudioOutputFile:(NSString*)path
{
    if (NO == [path isEqualToString:@""])
    {
        _demoClient->SetAudioTarget(TARGET_FILE);
        _demoClient->SetAudioOutputFile([path UTF8String]);
    }
    else
    {
        _demoClient->SetAudioTarget(TARGET_SPEAKER);
    }
}

- (void)setVideoInputFile:(NSString*)path
{
    if (NO == [path isEqualToString:@""]) {
        NSArray* components = [[path lastPathComponent] componentsSeparatedByString: @"_"];
        if([components count] < 4)
        {
            NSLog(@"setVideoInputFile, file name format isn't supported ([Width]_[Height]_[FrameRate]_XXX.rgb), %@", path);
            return;
        }
        WmeVideoRawFormat fmt;
        memset(&fmt, 0, sizeof(WmeVideoRawFormat));
        fmt.iWidth = [[components objectAtIndex: 0] integerValue];
        fmt.iHeight = [[components objectAtIndex:1] integerValue];
        fmt.fFrameRate = [[components objectAtIndex:2] integerValue];
        if (fmt.iWidth ==0 || fmt.iHeight ==0 || fmt.fFrameRate == 0) {
            NSLog(@"setVideoInputFile, video format is invalid, width = %d, height = %d, frame_rate = %f", fmt.iWidth, fmt.iHeight, fmt.fFrameRate);
            return;
        }
        if ([[path lastPathComponent] hasSuffix:@"yuv"]) {
            fmt.eRawType = WmeI420;
        }else if([[path lastPathComponent] hasSuffix:@"rgb"]){
            fmt.eRawType = WmeRGB24;
        }else{
            fmt.eRawType = WmeVideoUnknown;
            NSLog(@"setVideoInputFile, color space isn't supported");
            return;
        }
        
        _demoClient->SetVideoSource(SOURCE_FILE);
        _demoClient->SetVideoInputFile([path UTF8String], &fmt);
        
    }else{
        _demoClient->SetVideoSource(SOURCE_CAMERA);
    }
}

- (void)setVideoOutputFile:(NSString*)path
{
    if (NO == [path isEqualToString:@""])
    {
        _demoClient->SetVideoTarget(TARGET_FILE);
        _demoClient->SetVideoOutputFile([path UTF8String]);
    }
    else
    {
        _demoClient->SetVideoTarget(TARGET_SCREEN);
    }
}


//TRUE: host; FALSE: client
- (void)setRoleHostOrClient:(BOOL)isHost
{
    _isHost = isHost;
}
- (void)setNetworkEnableNAT:(BOOL)state
{
    _enableNAT = state;
}
- (void)setActionSendVideo:(BOOL)state
{
    if (_sendVideo != state)
    {
        _sendVideo = state;
        _demoClient->EnableMyMedia(DEMO_MEDIA_VIDEO, _sendVideo);
        if (_sendVideo)
            _demoClient->StartMediaSending(DEMO_MEDIA_VIDEO);
        else
            _demoClient->StopMediaSending(DEMO_MEDIA_VIDEO);
    }
}
- (void)setActionSendAudio:(BOOL)state
{
    if (_sendAudio != state)
    {
        _sendAudio = state;
        _demoClient->EnableMyMedia(DEMO_MEDIA_AUDIO, _sendAudio);
        if (_sendAudio)
            _demoClient->StartMediaSending(DEMO_MEDIA_AUDIO);
        else
            _demoClient->StopMediaSending(DEMO_MEDIA_AUDIO);
    }
}

- (void)setActionMaxTraceLevel:(NSInteger)level
{
    //TBD...
    WmeTraceLevel traceLevel = WME_TRACE_LEVEL_NOTRACE;
    
	switch (level)
	{
        case 0:
		{
			traceLevel = WME_TRACE_LEVEL_ERROR;
            break;
		}
        case 1:
		{
			traceLevel = WME_TRACE_LEVEL_WARNING;
            break;
		}
        case 2:
		{
			traceLevel = WME_TRACE_LEVEL_INFO;
            break;
		}
        case 3:
		{
			traceLevel = WME_TRACE_LEVEL_DEBUG;
            break;
		}
        case 4:
		{
			traceLevel = WME_TRACE_LEVEL_ALL;
            break;
		}
        default:
            return;
            
	}
    WmeSetTraceMaxLevel(traceLevel);
}

-(void)setShareSourceIndex:(NSInteger)idx
{
    _shareSourceIndex = idx;
    if (_isHost){
        if( _demoClient->IsMediaSending(DEMO_MEDIA_DESKSHARE) )
        {
        //set device
        if( 0 == _shareSourceIndex )
            _demoClient->SetScreenSource(_demoClient->GetOneScreenSource());
        else
        {
            ShareSourceManager *sourceMgr = _demoClient->GetShareSourceManager();
            if( sourceMgr )
            {
                IWmeScreenSource *source = sourceMgr->GetAppSourceSourceByIndex(_shareSourceIndex-1);
                if( source )
                    _demoClient->SetScreenSource(source);
            }
        }
        }
    }
}

- (void)setRenderModeScreenSharing:(WmeTrackRenderScalingModeType)mode
{
    _eRenderModeScreenSharing = mode;
}

- (WmeTrackRenderScalingModeType)getRenderModeScreenSharing
{
    return _eRenderModeScreenSharing;
}

- (CONNECTION_RETURN)clickedConnect
{
    if (!_demoClient)
    {
        return CONNECTION_INPUTERROR;
    }
    
    m_vtrack = DEMO_LOCAL_TRACK;

    if (self.enableAudio == YES) {
        // set audio parameter
        _demoClient->EnableMyMedia(DEMO_MEDIA_AUDIO, _sendAudio);
        _demoClient->CreateMediaClient(DEMO_MEDIA_AUDIO);
        _demoClient->StartMediaSending(DEMO_MEDIA_AUDIO);
        [self selectAudioInDevice:_audioCaptureIndex];
        [self selectAudioOutDevice:_audioPlayIndex];
        [self selectAudioCapability:_audioEncodingIndex track:DEMO_LOCAL_TRACK];
        [self selectAudioCapability:_audioEncodingIndex track:DEMO_REMOTE_TRACK];
        [self setAudioInVolume:_audioCaptureVolumeSLD];
        [self setAudioInMuteOrUnmute:_audioInMute];
        [self setAudioOutVolume:_audioPlayVolumeSLD];
        [self setAudioOutMuteOrUnmute:_audioOutMute];
    }

if (self.enableVideo == YES) {
    // set video parameter
    _demoClient->EnableMyMedia(DEMO_MEDIA_VIDEO, _sendVideo);
    _demoClient->CreateMediaClient(DEMO_MEDIA_VIDEO);
    _demoClient->StartMediaSending(DEMO_MEDIA_VIDEO);
    [self selectVideoDevice:_videoCaptureIndex track:DEMO_LOCAL_TRACK];
    [self selectVideoCapability:_videoEncodingIndex track:DEMO_LOCAL_TRACK];
    [self selectVideoCapability:_videoEncodingIndex track:DEMO_REMOTE_TRACK];
}
    
    
    if (self.enableDSVideo == YES) {
        
        _demoClient->CreateMediaClient(DEMO_MEDIA_DESKSHARE);
        _demoClient->EnableMyMedia(DEMO_MEDIA_DESKSHARE, _isHost);
        _demoClient->SetRenderMode(DEMO_REMOTE_DESKTOP_SHARE_TRACK, _eRenderModeScreenSharing);

        if (_isHost){
            //_demoClient->StartMediaSending(DEMO_MEDIA_DESKSHARE);
            //set device
            if( 0 == _shareSourceIndex )
                _demoClient->SetScreenSource(_demoClient->GetOneScreenSource());
            else
            {
                ShareSourceManager *sourceMgr = _demoClient->GetShareSourceManager();
                if( sourceMgr )
                {
                    IWmeScreenSource *source = sourceMgr->GetAppSourceSourceByIndex(_shareSourceIndex-1);
                    if( source )
                        _demoClient->SetScreenSource(source);
                }
            }

            //start local track
            _demoClient->StartMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_LOCAL_DESKTOP_SHARE_TRACK);
            
        }
        else{
            //for attendee, it will attach windows then start track.
            
        }
    }

    //
    // set network
    if (_isHost)
    {
        if (!_enableNAT) {
            if (self.enableAudio == YES) {
                _demoClient->InitHost(DEMO_MEDIA_AUDIO);
            }
            if (self.enableVideo == YES) {
                _demoClient->InitHost(DEMO_MEDIA_VIDEO);
            }
            if (self.enableDSVideo == YES) {
                _demoClient->InitHost(DEMO_MEDIA_DESKSHARE);
            }
        }else {
            _demoClient->InitHost(DEMO_MEDIA_AUDIO, [_MyName UTF8String],
                                  [_JINGLEServerIP UTF8String], [_JINGLEServerPort intValue],
                                  [_STUNServerIP UTF8String], [_STUNServerPort intValue]);
            _demoClient->InitHost(DEMO_MEDIA_VIDEO, [_MyName UTF8String],
                                  [_JINGLEServerIP UTF8String], [_JINGLEServerPort intValue],
                                  [_STUNServerIP UTF8String], [_STUNServerPort intValue]);
            _demoClient->InitHost(DEMO_MEDIA_DESKSHARE, [_MyName UTF8String],
                                  [_JINGLEServerIP UTF8String], [_JINGLEServerPort intValue],
                                  [_STUNServerIP UTF8String], [_STUNServerPort intValue]);

        }
    }
    else
    {
        if (!_enableNAT) {
            if (self.enableAudio == YES) {
                _demoClient->ConnectRemote(DEMO_MEDIA_AUDIO, (char *)[_HostIPAddress UTF8String]);
            }
            if (self.enableVideo == YES) {
                _demoClient->ConnectRemote(DEMO_MEDIA_VIDEO, (char *)[_HostIPAddress UTF8String]);
            }
            if (self.enableDSVideo == YES) {
                _demoClient->ConnectRemote(DEMO_MEDIA_DESKSHARE, (char *)[_HostIPAddress UTF8String]);
            }

        }else {
            _demoClient->ConnectRemote(DEMO_MEDIA_AUDIO, [_MyName UTF8String], [_HostName UTF8String],
                                       [_JINGLEServerIP UTF8String], [_JINGLEServerPort intValue],
                                       [_STUNServerIP UTF8String], [_STUNServerPort intValue]);
            _demoClient->ConnectRemote(DEMO_MEDIA_VIDEO, [_MyName UTF8String], [_HostName UTF8String],
                                       [_JINGLEServerIP UTF8String], [_JINGLEServerPort intValue],
                                       [_STUNServerIP UTF8String], [_STUNServerPort intValue]);
            _demoClient->ConnectRemote(DEMO_MEDIA_DESKSHARE, [_MyName UTF8String], [_HostName UTF8String],
                                       [_JINGLEServerIP UTF8String], [_JINGLEServerPort intValue],
                                       [_STUNServerIP UTF8String], [_STUNServerPort intValue]);
        }
    }
    
    //
    if (self.enableAudio == YES) {
    //
    // start audio loca/remote track
        _demoClient->StartMediaTrack(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK);
        _demoClient->StartMediaTrack(DEMO_MEDIA_AUDIO, DEMO_REMOTE_TRACK);
    }
    return CONNECTION_SUCCESS;
}

- (void)clickedDisconnect
{
    if (_demoClient) {
        _demoClient->DeleteMediaClient(DEMO_MEDIA_AUDIO);
        _demoClient->DeleteMediaClient(DEMO_MEDIA_VIDEO);
        _demoClient->DeleteMediaClient(DEMO_MEDIA_DESKSHARE);
        _demoClient->DisConnect(DEMO_MEDIA_VIDEO);
        _demoClient->DisConnect(DEMO_MEDIA_AUDIO);
        _demoClient->DisConnect(DEMO_MEDIA_DESKSHARE);
    }
}

- (void)clickedPreview:(BOOL)state
{
    if (!_demoClient)
        return;
    
    m_vtrack = DEMO_PREVIEW_TRACK;
    
    if (state)
    {
        _demoClient->CreateMediaClient(DEMO_MEDIA_VIDEO);
        [self selectVideoDevice:_videoCaptureIndex track:DEMO_PREVIEW_TRACK];
        _demoClient->SetRenderView(DEMO_PREVIEW_TRACK, _previewView);
        _demoClient->StartMediaTrack(DEMO_MEDIA_VIDEO, DEMO_PREVIEW_TRACK);
    }
    else
    {
        _demoClient->StopMediaTrack(DEMO_MEDIA_VIDEO, DEMO_PREVIEW_TRACK);
        _demoClient->DeleteMediaClient(DEMO_MEDIA_VIDEO);
    }
}

- (void)setWindowForSelfView:(id)windowhandle
{
    // start local video track
    _selfView = windowhandle;
    if (_demoClient &&
        (self.enableVideo==YES)) {
        _demoClient->SetRenderView(DEMO_LOCAL_TRACK, _selfView);
        _demoClient->StartMediaTrack(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK);
    }
}

- (void)setWindowForAttendeeView:(id)windowhandle
{
    // start remote video track
    _remoteView = windowhandle;
    if (_demoClient &&
        (self.enableVideo==YES)) {
        _demoClient->SetRenderView(DEMO_REMOTE_TRACK, _remoteView);
        _demoClient->StartMediaTrack(DEMO_MEDIA_VIDEO, DEMO_REMOTE_TRACK);
    }
}

- (void)setWindowForScreenAttendeeView:(id)windowhandle
{
    
    // start remote screen track
    _remoteDSView = windowhandle;
    if (_demoClient &&
        (self.enableDSVideo==YES)) {
        _demoClient->SetRenderView(DEMO_REMOTE_DESKTOP_SHARE_TRACK, _remoteDSView);
        _demoClient->StartMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_REMOTE_DESKTOP_SHARE_TRACK);
    }

}
- (void)setWindowForPreview:(id)windowhandle
{
    _previewView = windowhandle;
}

- (void)getVideoStatistics:(WmeSessionStatistics &)statistics
{
    //TBD...
    _demoClient->GetVideoStatistics(statistics);
}
- (void)getVideoStatistics:(WmeVideoStatistics &)statistics Track:(DEMO_TRACK_TYPE)type
{
    //TBD...
    _demoClient->GetVideoStatistics(type, statistics);
}
- (void)getAudioStatistics:(WmeSessionStatistics &)statistics
{
    //TBD...
    _demoClient->GetAudioStatistics(statistics);
}
- (void)getAudioStatistics:(WmeAudioStatistics &)statistics Track:(DEMO_TRACK_TYPE)type
{
    //TBD...
    _demoClient->GetAudioStatistics(type, statistics);
}

- (void)getNetworkIndex:(WmeNetworkIndex &)idx Direction:(WmeNetworkDirection)d
{
    _demoClient->GetNetworkIndex(idx, d);
}


@end
