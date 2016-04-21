//
//  WMEDataProcess.m
//  MediaEngineTestApp
//
//  Created by admin on 10/15/13.
//  Copyright (c) 2013 video. All rights reserved.
//
#import <sys/sysctl.h>

#import "WMEDataProcess.h"
#import "DemoParameters.h"
#import "DemoClient.h"


typedef struct
{
    WmeVideoRawType type;
    const char *string;
}FORMAT_MAP_INFO;

const static FORMAT_MAP_INFO kFormatMapInfo[] = {
    {WmeVideoUnknown,    "Unknown"},
    {WmeI420,            "I420"   },
    {WmeYV12,            "YV12"   },
    {WmeNV12,            "NV12"   },
    {WmeNV21,            "NV21"   },
    {WmeYUY2,            "YUY2"   },
    {WmeRGB24,           "RGB24"   },
    {WmeBGR24,           "BGR24"   },
    {WmeRGB24Flip,       "RGB24Flip"   },
    {WmeBGR24Flip,       "BGR24Flip"   },
    {WmeRGBA32,          "RGBA32"   },
    {WmeBGRA32,          "BGRA32"   },
    {WmeARGB32,          "ARGB32"   },
    {WmeABGR32,          "ABGR32"   },
    {WmeRGBA32Flip,      "RGBA32Flip"   },
    {WmeBGRA32Flip,      "BGRA32Flip"   },
    {WmeARGB32Flip,      "ARGB32Flip"   },
    {WmeABGR32Flip,      "ABGR32Flip"   },
};


@implementation WMEDataProcess


+ (WMEDataProcess *)instance
{
    static dispatch_once_t pred = 0;
    __strong static id _sharedInstance = nil;
    dispatch_once(&pred, ^{
        _sharedInstance = [[self alloc]init];
    });
    return _sharedInstance;
}

- (long)initWME
{
    _arrayAudioCapabilities = [[NSMutableArray alloc] init];
    _arrayVideoCapabilities = [[NSMutableArray alloc] init];
    _arrayCameraCapabilities = [[NSMutableArray alloc]init];
    
    _arraySpeakerDevices = [[NSMutableArray alloc] init];
    _arrayMicDevices = [[NSMutableArray alloc] init];
    _arrayCameraDevices = [[NSMutableArray alloc] init];
    
    _isHost = FALSE;
    _useICE = FALSE;
    
    _cameraIndex = 0;
    _micIndex = 0;
    _speakerIndex = 0;
    
    _audioCapIndex = 0;
    _videoCapIndex = 0;
    _cameraCapIndex = 0;
    
    _bVideoSending = NO;
    _localRender = NULL;
    _remoteRender = NULL;
    _bKeepAspectRatio = NO;
    _bDumpFlag = NO;
    _bDumpCaptureDataEnable = NO;
    _bDumpEncodeToRTPDataEnable = NO;
    _bDumpNALToListenChannelDataEnable = NO;
    _bDumpNALToDecodeDataEnable = NO;
    _bDumpAfterDecodeDataEnable = NO;
    
    _bVideoModuleEnable = YES;
    _bAudioModuleEnable = YES;
    _bContentShareModuleEnable = YES;
    
    _bEnableExternalVideoInput = NO;
    _bEnableExternalVideoOutput = NO;
    _bEnableExternalAudioInput = NO;
    _bEnableExternalAudioOutput = NO;
    _sVInputFilename = nil;
    _sVOutputFilename = nil;
    _iWidth = 0;
    _iHeight = 0;
    _iColorFormat = WmeVideoUnknown;
    _iFPS = 0;
    _sAInputFilename = nil;
    _sAOutputFilename = nil;
    _iChannels = 0;
    _iSampleRate = 0;
    _iBitsPerSample = 0;
    _bAudioPauseStatus = YES;
    
    _isFile = NO;
    _isFileAudio = NO;
    _audioFile = nil;
    _audioSourceIP = nil;
    _audioSourcePort = 0;
    _audioDestinationIP = nil;
    _audioDestinationPort = 0;
    _isFileVideo = NO;
    _videoFile = nil;
    _videoSourceIP = nil;
    _videoSourcePort = 0;
    _videoDestinationIP = nil;
    _videoDestinationPort = 0;
    _isFileContentShare = NO;
    
    //Add the observer to notification centre
    //[[NotificationTransfer instance] addNotificationObserver:self];
    
    //init WME demo
    m_pWMEDemo = new DemoClient(NULL);
    m_pWMEDemo->Init(WME_TRACE_LEVEL_INFO);
    
    //init KVO observer
    [self initKVO];
    
    //init Pcap file info
    [self initPcapFile];
    
    return WME_S_OK;
}

- (void)initKVO
{
    [self addObserver:self forKeyPath:@"audioCapIndex" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld context:nil];
    [self addObserver:self forKeyPath:@"speakerIndex" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld context:nil];
    [self addObserver:self forKeyPath:@"videoCapIndex" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld context:nil];
    [self addObserver:self forKeyPath:@"cameraIndex" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld context:nil];
    [self addObserver:self forKeyPath:@"cameraCapIndex" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld context:nil];
    [self addObserver:self forKeyPath:@"bKeepAspectRatio" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld context:nil];
    [self addObserver:self forKeyPath:@"bDumpFlag" options:NSKeyValueObservingOptionNew|NSKeyValueObservingOptionOld context:nil];
}

- (void)initPcapFile
{
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    if([paths count] > 0)
    {
        // load pcapfile.plist
        NSString *documentPath = [[paths objectAtIndex:0] stringByAppendingString:@"/"];
        NSString *plistPath = [documentPath stringByAppendingString:@"pcapfile.plist"];
        NSDictionary *dictionary = [[NSDictionary alloc] initWithContentsOfFile:plistPath];
        // get pcapfile enable flag
        _isFile = [(NSNumber*)[dictionary objectForKey: @"Pcapfile enable"] boolValue];
        if(_isFile)
        {
            // get audio info
            NSDictionary *audioInfo = [dictionary objectForKey: @"Audio info"];
            _isFileAudio = [(NSNumber*)[audioInfo objectForKey: @"Audio enable"] boolValue];
            if(_isFileAudio)
            {
                _audioFile = [documentPath stringByAppendingString:[audioInfo objectForKey: @"Audio file"]];
                _audioSourceIP = [audioInfo objectForKey:@"Audio source IP"];
                _audioSourcePort = [(NSNumber*)[audioInfo objectForKey:@"Audio source port"] unsignedShortValue];
                _audioDestinationIP = [audioInfo objectForKey:@"Audio destination IP"];
                _audioDestinationPort = [(NSNumber*)[audioInfo objectForKey:@"Audio destination port"] unsignedShortValue];
            }
            
            // get video info
            NSDictionary *videoInfo = [dictionary objectForKey: @"Video info"];
            _isFileVideo = [(NSNumber*)[videoInfo objectForKey: @"Video enable"] boolValue];
            if(_isFileVideo)
            {
                _videoFile = [documentPath stringByAppendingString:[videoInfo objectForKey: @"Video file"]];
                _videoSourceIP = [videoInfo objectForKey:@"Video source IP"];
                _videoSourcePort = [(NSNumber*)[videoInfo objectForKey:@"Video source port"] unsignedShortValue];
                _videoDestinationIP = [videoInfo objectForKey:@"Video destination IP"];
                _videoDestinationPort = [(NSNumber*)[videoInfo objectForKey:@"Video destination port"] unsignedShortValue];
            }
            
            // get contect sharing info, coming soon
        }
    }
}

- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    id newValue = [change valueForKey:@"new"];
    
    if ([keyPath isEqualToString:@"audioCapIndex"]) {
        [self setAudioEncodingParam:[newValue integerValue]];
    }
    else if ([keyPath isEqualToString:@"speakerIndex"])
    {
        [self setAudioSpeaker:[newValue integerValue]];
    }
    else if ([keyPath isEqualToString:@"videoCapIndex"])
    {
        [self setVideoEncodingParam:[newValue integerValue]];
    }
    else if ([keyPath isEqualToString:@"cameraIndex"])
    {
        [self setVideoCameraDevice:[newValue integerValue]];
    }
    else if ([keyPath isEqualToString:@"cameraCapIndex"])
    {
        [self setVideoCameraParam:[newValue integerValue]];
    }
    else if ([keyPath isEqualToString:@"bKeepAspectRatio"])
    {
        [self setRenderAdaptiveAspectRatio:[newValue boolValue]];
    }
    else if ([keyPath isEqualToString:@"bDumpFlag"])
    {
        [self setDumpDataEnabled:
         (_bDumpCaptureDataEnable == YES ? WME_DATA_DUMP_VIDEO_RAW_CAPTURE : 0) |
         (_bDumpEncodeToRTPDataEnable == YES ? WME_DATA_DUMP_VIDEO_ENCODE_RTP_LAYER : 0) |
         (_bDumpNALToListenChannelDataEnable == YES ? WME_DATA_DUMP_VIDEO_NAL_TO_LISTEN_CHANNEL : 0) |
         (_bDumpNALToDecodeDataEnable == YES ? WME_DATA_DUMP_VIDEO_NAL_TO_DECODER : 0) |
         (_bDumpAfterDecodeDataEnable == YES ? WME_DATA_DUMP_VIDEO_RAW_AFTER_DECODE_TO_RENDER : 0)];
    }
    
    else
    {
        [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
    }
}

- (void)uninitWME
{
    return_if_fail(m_pWMEDemo != NULL);
    
    m_pWMEDemo->UnInit();
    
    [_arrayAudioCapabilities removeAllObjects];
    [_arrayVideoCapabilities removeAllObjects];
    [_arrayCameraCapabilities removeAllObjects];
    
    [_arraySpeakerDevices removeAllObjects];
    [_arrayMicDevices removeAllObjects];
    [_arraySpeakerDevices removeAllObjects];
    
    m_pWMEDemo->ClearDeviceList(DEMO_MEDIA_AUDIO, _audioInList);
    m_pWMEDemo->ClearDeviceList(DEMO_MEDIA_AUDIO, _audioOutList);
    
    m_pWMEDemo->ClearDeviceList(DEMO_MEDIA_VIDEO, _videoInList);
    m_pWMEDemo->ClearDeviceCapabilities(DEV_TYPE_CAMERA, _cameraCapList);
    
    _audioCapList.clear();
    _videoCapList.clear();
    
    SAFE_DELETE(m_pWMEDemo);
}




- (void)setTraceMaxLevel: (int)level
{
    return_if_fail(m_pWMEDemo != NULL);
    
    m_pWMEDemo->SetTraceMaxLevel((WmeTraceLevel)level);
}
- (void)setDumpDataEnabled:(unsigned int)uDataDumpFlag
{
    return_if_fail(m_pWMEDemo != NULL);
    m_pWMEDemo->SetDumpDataEnabled(uDataDumpFlag);
    
}
- (void)setDumpDataPath:(const char *)path
{
    return_if_fail(m_pWMEDemo != NULL);
    m_pWMEDemo->SetDumpDataPath(path);
    
}
- (void)setUISink:(backUISink *)sink
{
    return_if_fail(m_pWMEDemo != NULL);
    
    m_pWMEDemo->SetUISink(sink);
}

- (void)queryAudioInDevices
{
    return_if_fail(m_pWMEDemo != NULL);
    return_if_fail(_bAudioModuleEnable != NO);
    
    m_pWMEDemo->ClearDeviceList(DEMO_MEDIA_AUDIO, _audioInList);
    m_pWMEDemo->GetDeviceList(DEMO_MEDIA_AUDIO, DEV_TYPE_MIC, _audioInList);
    
    /// audio mic device list
    [_arrayMicDevices removeAllObjects];
    int defaultIndex = 0;
    for (int k=0; k < _audioInList.size(); k++) {
        DeviceProperty *dev = &(_audioInList.at(k));
        if (dev->is_default_dev) {
            defaultIndex = k;
        }
        [_arrayMicDevices  addObject:[[NSString alloc] initWithCString:dev->dev_name encoding:NSUTF8StringEncoding]];
    }
    _micIndex = defaultIndex;
}

- (void)queryAudioOutDevices
{
    return_if_fail(m_pWMEDemo != NULL);
    return_if_fail(_bAudioModuleEnable != NO);
    
    m_pWMEDemo->ClearDeviceList(DEMO_MEDIA_AUDIO, _audioOutList);
    m_pWMEDemo->GetDeviceList(DEMO_MEDIA_AUDIO, DEV_TYPE_SPEAKER, _audioOutList);
    
    /// audio speaker device list
    [_arraySpeakerDevices removeAllObjects];
    int defaultIndex = 0;
    int defaultPosition = DEV_POSITION_UNKNOWN;
    for (int k=0; k < _audioOutList.size(); k++) {
        DeviceProperty *dev = &(_audioOutList.at(k));
        if (dev->is_default_dev) {
            defaultIndex = k;
            defaultPosition = dev->position;
        }
        [_arraySpeakerDevices  addObject:[[NSString alloc] initWithCString:dev->dev_name encoding:NSUTF8StringEncoding]];
    }
    _speakerIndex = defaultIndex;
    _speakerPosition = defaultPosition;
}

- (void)queryVideoInDevices
{
    return_if_fail(m_pWMEDemo != NULL);
    return_if_fail(_bVideoModuleEnable != NO);
    
    m_pWMEDemo->ClearDeviceList(DEMO_MEDIA_VIDEO, _videoInList);
    m_pWMEDemo->GetDeviceList(DEMO_MEDIA_VIDEO, DEV_TYPE_CAMERA, _videoInList);
    
    /// video device list
    [_arrayCameraDevices removeAllObjects];
    int defaultIndex = 0;
    int defaultPosition = DEV_POSITION_UNKNOWN;
    for (int k=0; k < _videoInList.size(); k++) {
        DeviceProperty *dev = &(_videoInList.at(k));
        if (dev->is_default_dev) {
            defaultIndex = k;
            defaultPosition = dev->position;
        }
        [_arrayCameraDevices  addObject:[[NSString alloc] initWithCString:dev->dev_name encoding:NSUTF8StringEncoding]];
    }
    _cameraIndex = defaultIndex;
    _cameraPosition = defaultPosition;
}

- (void)queryAudioCapabilities
{
    return_if_fail(m_pWMEDemo != NULL);
    return_if_fail(_bAudioModuleEnable != NO);
    
    _audioCapList.clear();
    m_pWMEDemo->GetMediaCapabilities(DEMO_MEDIA_AUDIO, &_audioCapList);
    
    /// audio cap list
    [_arrayAudioCapabilities removeAllObjects];
    int defaultIndex = 0;
    for (int k=0; k < _audioCapList.size(); k++) {
        WmeAudioMediaCapability *cap = &(_audioCapList.at(k));
        if (cap->eCodecType == WmeCodecType_OPUS) {
            defaultIndex = k;
        }
        char strFmt[128];
        sprintf(strFmt, "%s, sample freq: %dKHz, bitrate: %dKbps",
                cap->stdname, cap->clockrate/1000, cap->rate/1000);
        [_arrayAudioCapabilities addObject:[[NSString alloc] initWithCString:strFmt encoding:NSUTF8StringEncoding]];
    }
    _audioCapIndex = defaultIndex;
}

- (void)queryVideoCapabilities
{
    return_if_fail(m_pWMEDemo != NULL);
    return_if_fail(_bVideoModuleEnable != NO);
    
    _videoCapList.clear();
    m_pWMEDemo->GetMediaCapabilities(DEMO_MEDIA_VIDEO, &_videoCapList);
    
    /// video cap list
    [_arrayVideoCapabilities removeAllObjects];
    int defaultIndex = 0;
    for (int k=0; k < _videoCapList.size(); k++) {
        WmeVideoMediaCapability *cap = &(_videoCapList.at(k));
        if (cap->eCodecType == WmeCodecType_Unknown) {
            continue;
        }
        
        if (cap->eCodecType == WmeCodecType_SVC && cap->height == 360) {
            defaultIndex = k;
        }
        
        float fps = cap->frame_layer[cap->frame_layer_number-1] * 1.0 / 100;
        char strFmt[128];
        snprintf(strFmt, 128, "%s, %dx%d@%1.ffps", kWmeVideoCodecTag[cap->eCodecType-WmeCodecType_AVC], (int)cap->width, (int)cap->height, fps);
        [_arrayVideoCapabilities addObject:[[NSString alloc] initWithCString:strFmt encoding:NSUTF8StringEncoding]];
    }
    _videoCapIndex = defaultIndex;
}

- (void)queryVideoCameraCapabilities
{
    return_if_fail(m_pWMEDemo != NULL);
    return_if_fail(_bVideoModuleEnable != NO);
    
    if (_cameraIndex >= _videoInList.size()) {
        return;
    }
    
    DeviceProperty *dev = &(_videoInList.at(_cameraIndex));
    if (dev->dev == NULL) {
        return;
    }
    
    /// camera capability list
    m_pWMEDemo->ClearDeviceCapabilities(DEV_TYPE_CAMERA, _cameraCapList);
    m_pWMEDemo->GetDeviceCapabilities(DEV_TYPE_CAMERA, dev->dev, _cameraCapList);
    
    /// 
    [_arrayCameraCapabilities removeAllObjects];
    int defaultIndex = 0;
    bool bfind = false;
    for (int k=0; k < _cameraCapList.size(); k++) {
        WmeDeviceCapability *pDC = &(_cameraCapList.at(k));
        WmeCameraCapability *pCC = (WmeCameraCapability *)pDC->pCapalibity;
        
        const char *string = NULL;
        for (int i=0; i<sizeof(kFormatMapInfo)/sizeof(FORMAT_MAP_INFO); i++) {
            if (kFormatMapInfo[i].type == pCC->type) {
                string = kFormatMapInfo[i].string;
                break;
            }
        }
        if (string == NULL) {
            continue;
        }
        
        if (pCC->width >= 640 && pCC->height >= 360 && !bfind) {
            defaultIndex = k;
            bfind = true;
        }
        
        char info[128];
        snprintf(info, 128, "%s, %ldx%ld", string, pCC->width, pCC->height);
        [_arrayCameraCapabilities addObject:[[NSString alloc]initWithCString:info encoding:NSUTF8StringEncoding]];
    }
    _cameraCapIndex = defaultIndex;
}

- (long)createAudioClient
{
    WmeAudioRawFormat fmt;
    
    fmt.eRawType=WmePCM;
    fmt.iChannels=self.iChannels;
    fmt.iSampleRate=self.iSampleRate;
    fmt.iBitsPerSample=self.iBitsPerSample;
    

    if (_bEnableExternalAudioInput)
    {
        m_pWMEDemo->SetAudioInputFile([_sAInputFilename UTF8String], &fmt);
        m_pWMEDemo->SetAudioSource(SOURCE_FILE);
    }
    else
        m_pWMEDemo->SetAudioSource(SOURCE_MIC);
    if (_bEnableExternalAudioOutput)
    {
        m_pWMEDemo->SetAudioOutputFile([_sAOutputFilename UTF8String]);
        m_pWMEDemo->SetAudioTarget(TARGET_FILE);
    }
    else
        m_pWMEDemo->SetAudioTarget(TARGET_SPEAKER);
    
    
    m_pWMEDemo->CreateMediaClient(DEMO_MEDIA_AUDIO);
    return WME_S_OK;
}

- (long)createVideoClient
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bVideoModuleEnable != NO, WME_E_FAIL);

    if (_bEnableExternalVideoInput && _sVInputFilename.length > 0 && _iWidth > 0 && _iHeight > 0 && _iColorFormat > 0) {
        WmeVideoRawFormat fmt;
        memset(&fmt, 0, sizeof(WmeVideoRawFormat));
        fmt.eRawType = (WmeVideoRawType)_iColorFormat;
        fmt.iWidth = _iWidth;
        fmt.iHeight = _iHeight;
        fmt.fFrameRate = _iFPS;
        m_pWMEDemo->SetVideoInputFile([_sVInputFilename UTF8String], &fmt);
        m_pWMEDemo->SetVideoSource(SOURCE_FILE);
    }
    else
        m_pWMEDemo->SetVideoSource(SOURCE_CAMERA);
    
    if (_bEnableExternalVideoOutput && _sVOutputFilename.length > 0) {
        m_pWMEDemo->SetVideoOutputFile([_sVOutputFilename UTF8String]);
        m_pWMEDemo->SetVideoTarget(TARGET_FILE);
    }
    else
        m_pWMEDemo->SetVideoTarget(TARGET_SCREEN);

    m_pWMEDemo->CreateMediaClient(DEMO_MEDIA_VIDEO);
    return WME_S_OK;
}

- (long)createAppShareClient
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bContentShareModuleEnable != NO, WME_E_FAIL);
    
//    if (_bEnableExternalVideoInput && _sVInputFilename.length > 0 && _iWidth > 0 && _iHeight > 0 && _iColorFormat > 0) {
//        WmeVideoRawFormat fmt;
//        memset(&fmt, 0, sizeof(WmeVideoRawFormat));
//        fmt.eRawType = (WmeVideoRawType)_iColorFormat;
//        fmt.iWidth = _iWidth;
//        fmt.iHeight = _iHeight;
//        fmt.fFrameRate = _iFPS;
//        m_pWMEDemo->SetVideoInputFile([_sVInputFilename UTF8String], &fmt);
//        m_pWMEDemo->SetVideoSource(SOURCE_FILE);
//    }
//    else
//        m_pWMEDemo->SetVideoSource(SOURCE_CAMERA);
//    
//    if (_bEnableExternalVideoOutput && _sVOutputFilename.length > 0) {
//        m_pWMEDemo->SetVideoOutputFile([_sVOutputFilename UTF8String]);
//        m_pWMEDemo->SetVideoTarget(TARGET_FILE);
//    }
//    else
//        m_pWMEDemo->SetVideoTarget(TARGET_SCREEN);
//    
    m_pWMEDemo->CreateMediaClient(DEMO_MEDIA_DESKSHARE);
    return WME_S_OK;
}


- (void)deleteAudioClient
{
    return_if_fail(m_pWMEDemo != NULL);
    return_if_fail(_bAudioModuleEnable != NO);
    
    m_pWMEDemo->DeleteMediaClient(DEMO_MEDIA_AUDIO);
}

- (void)deleteVideoClient
{
    return_if_fail(m_pWMEDemo != NULL);
    return_if_fail(_bVideoModuleEnable != NO);
    
    m_pWMEDemo->DeleteMediaClient(DEMO_MEDIA_VIDEO);
}

-(void)deleteContentShareClient
{
    m_pWMEDemo->DeleteMediaClient(DEMO_MEDIA_DESKSHARE);
}

- (long)startAudioClient: (int)iType
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bAudioModuleEnable != NO, WME_E_FAIL);
    
    long ret = WME_E_FAIL;
    if (iType == WME_SENDING) {
        if (_bAudioSending == YES) {
            m_pWMEDemo->StartMediaSending(DEMO_MEDIA_AUDIO);
        }
        ret = m_pWMEDemo->StartMediaTrack(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK);
        
        _bAudioPauseStatus = NO;
    }else {
        ret = m_pWMEDemo->StartMediaTrack(DEMO_MEDIA_AUDIO, DEMO_REMOTE_TRACK);
    }

    return ret;
}

- (long)startVideoClient: (int)iType
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bVideoModuleEnable != NO, WME_E_FAIL);
    
    long ret = WME_E_FAIL;
    
    if (iType == WME_SENDING) {        
        if (_bVideoSending) {
            m_pWMEDemo->StartMediaSending(DEMO_MEDIA_VIDEO);
            ret = m_pWMEDemo->StartMediaTrack(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK);
        }else {
            ret = m_pWMEDemo->StartMediaTrack(DEMO_MEDIA_VIDEO, DEMO_PREVIEW_TRACK);
        }
    }else {
        ret = m_pWMEDemo->StartMediaTrack(DEMO_MEDIA_VIDEO, DEMO_REMOTE_TRACK);
    }
    return ret;
}

-(long)startContentShareClient
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bContentShareModuleEnable != NO, WME_E_FAIL);
    
    long ret = WME_E_FAIL;
    if (_isHost) {
//            m_pWMEDemo->StartMediaSending(DEMO_MEDIA_DESKSHARE);
            ret = m_pWMEDemo->StartMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_LOCAL_TRACK);
    }else {
        ret = m_pWMEDemo->StartMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_REMOTE_TRACK);
    }
    return ret;
}

- (long)stopAudioClient: (int)iType
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bAudioModuleEnable != NO, WME_E_FAIL);
    
    long ret = WME_E_FAIL;
    if (iType == WME_SENDING) {
        m_pWMEDemo->StopMediaSending(DEMO_MEDIA_AUDIO);
        ret = m_pWMEDemo->StopMediaTrack(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK);
        
        _bAudioPauseStatus = YES;
    }else {
        ret = m_pWMEDemo->StopMediaTrack(DEMO_MEDIA_AUDIO, DEMO_REMOTE_TRACK);
    }
    return ret;
}

- (long)stopVideoClient: (int)iType
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bVideoModuleEnable != NO, WME_E_FAIL);
    
    long ret = WME_E_FAIL;
    if (iType == WME_SENDING) {
        if (_bVideoSending){
            m_pWMEDemo->StopMediaSending(DEMO_MEDIA_VIDEO);
            ret = m_pWMEDemo->StopMediaTrack(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK);
        }else {
            ret = m_pWMEDemo->StopMediaTrack(DEMO_MEDIA_VIDEO, DEMO_PREVIEW_TRACK);
        }
    }else {
        ret = m_pWMEDemo->StopMediaTrack(DEMO_MEDIA_VIDEO, DEMO_REMOTE_TRACK);
    }
    
    return ret;
}

-(long)stopContentShareClient
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bContentShareModuleEnable != NO, WME_E_FAIL);
    
    long ret = WME_E_FAIL;
    if (_isHost) {
//            m_pWMEDemo->StopMediaSending(DEMO_MEDIA_DESKSHARE);
        ret = m_pWMEDemo->StopMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_LOCAL_TRACK);
    }else {
        ret = m_pWMEDemo->StopMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_REMOTE_TRACK);
    }
    return ret;
}

- (long)clickedConnect
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    if (_bAudioModuleEnable == YES) {
        [self createAudioClient];
    }
    if (_bVideoModuleEnable == YES) {
        [self createVideoClient];
    }
    
    if( _bContentShareModuleEnable )
    {
        m_pWMEDemo->EnableMyMedia(DEMO_MEDIA_DESKSHARE, _isHost);
        [self createAppShareClient];
    }
    
    if (_isFile)
    {
        if (YES == _bAudioModuleEnable && YES == _isFileAudio) {
            m_pWMEDemo->ConnectFile(DEMO_MEDIA_AUDIO, [_audioFile UTF8String], [_audioSourceIP UTF8String], _audioSourcePort, [_audioDestinationIP UTF8String], _audioDestinationPort);
        }
        if (YES == _bVideoModuleEnable && YES == _isFileVideo) {
            m_pWMEDemo->ConnectFile(DEMO_MEDIA_VIDEO, [_videoFile UTF8String], [_videoSourceIP UTF8String], _videoSourcePort, [_videoDestinationIP UTF8String], _videoDestinationPort);
        }
        if (YES == _bContentShareModuleEnable && YES == _isFileContentShare) {
            // coming soon
        }
    }
    else if (_isHost)
    {
        if (!_useICE) {
            if (_bAudioModuleEnable == YES) {
                m_pWMEDemo->InitHost(DEMO_MEDIA_AUDIO);
            }
            if (_bVideoModuleEnable == YES) {
                m_pWMEDemo->InitHost(DEMO_MEDIA_VIDEO);
            }
            if( _bContentShareModuleEnable )
                m_pWMEDemo->InitHost(DEMO_MEDIA_DESKSHARE);
        }else {
            m_pWMEDemo->InitHost(DEMO_MEDIA_AUDIO, [_myName UTF8String],
                                 [_jingleServerIP UTF8String], [_jingleServerPort intValue],
                                 [_stunServerIP UTF8String], [_stunServerPort intValue]);
            m_pWMEDemo->InitHost(DEMO_MEDIA_VIDEO, [_myName UTF8String],
                                 [_jingleServerIP UTF8String], [_jingleServerPort intValue],
                                 [_stunServerIP UTF8String], [_stunServerPort intValue]);
            m_pWMEDemo->InitHost(DEMO_MEDIA_DESKSHARE, [_myName UTF8String],
                                 [_jingleServerIP UTF8String], [_jingleServerPort intValue],
                                 [_stunServerIP UTF8String], [_stunServerPort intValue]);
        }
    }
    else
    {
        if (!_useICE) {
            if (_bAudioModuleEnable == YES) {
                m_pWMEDemo->ConnectRemote(DEMO_MEDIA_AUDIO, (char *)[_hostIPAddress UTF8String]);
            }
            if (_bVideoModuleEnable == YES) {
                m_pWMEDemo->ConnectRemote(DEMO_MEDIA_VIDEO, (char *)[_hostIPAddress UTF8String]);
            }
            if( _bContentShareModuleEnable == YES )
                m_pWMEDemo->ConnectRemote(DEMO_MEDIA_DESKSHARE, (char *)[_hostIPAddress UTF8String]);
        }else {
            m_pWMEDemo->ConnectRemote(DEMO_MEDIA_AUDIO, [_myName UTF8String], [_hostName UTF8String],
                                       [_jingleServerIP UTF8String], [_jingleServerPort intValue],
                                       [_stunServerIP UTF8String], [_stunServerPort intValue]);
            m_pWMEDemo->ConnectRemote(DEMO_MEDIA_VIDEO, [_myName UTF8String], [_hostName UTF8String],
                                       [_jingleServerIP UTF8String], [_jingleServerPort intValue],
                                       [_stunServerIP UTF8String], [_stunServerPort intValue]);
            m_pWMEDemo->ConnectRemote(DEMO_MEDIA_DESKSHARE, [_myName UTF8String], [_hostName UTF8String],
                                      [_jingleServerIP UTF8String], [_jingleServerPort intValue],
                                      [_stunServerIP UTF8String], [_stunServerPort intValue]);
        }
    }
    
    return WME_S_OK;
}

- (void)clickedDisconnect
{
    return_if_fail(m_pWMEDemo != NULL);
    
    if (_bAudioModuleEnable == YES) {
        m_pWMEDemo->DisConnect(DEMO_MEDIA_AUDIO);
        [self deleteAudioClient];
    }
    if (_bVideoModuleEnable == YES) {
        m_pWMEDemo->DisConnect(DEMO_MEDIA_VIDEO);
        [self deleteVideoClient];
    }
    
    if( _bContentShareModuleEnable == YES )
    {
        m_pWMEDemo->DisConnect(DEMO_MEDIA_DESKSHARE);
        [self deleteContentShareClient];
    }
}

- (long)setVideoCameraDevice:(NSInteger)index
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bVideoModuleEnable != NO, WME_E_FAIL);
    
    long ret = WME_E_FAIL;
    
    [self queryVideoCameraCapabilities];
    if (index < _videoInList.size()) {
        //_cameraIndex = index;
        _cameraPosition = _videoInList[index].position;
        DeviceProperty *dp = &(_videoInList.at(index));
        ret = m_pWMEDemo->SetCamera(_bVideoSending ? DEMO_LOCAL_TRACK : DEMO_PREVIEW_TRACK, dp->dev);
    }
    return ret;
}

- (void)switchCameraDevice
{
    return_if_fail(_bVideoModuleEnable != NO);
    
    self.cameraIndex = (_cameraIndex + 1) % _videoInList.size();
}

- (long)setVideoEncodingParam:(NSInteger)index
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bVideoModuleEnable != NO, WME_E_FAIL);
    
    long ret = WME_E_FAIL;
    if(index < _videoCapList.size()) {
        WmeVideoMediaCapability *cap = &(_videoCapList.at(index));
        m_pWMEDemo->SetMediaCodec(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK, cap->eCodecType);
        m_pWMEDemo->SetMediaSession(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK, cap->eCodecType);
        m_pWMEDemo->SetMediaCapability(DEMO_MEDIA_VIDEO, _bVideoSending ? DEMO_LOCAL_TRACK:DEMO_PREVIEW_TRACK, cap);
        //m_pWMEDemo->SetMediaCodec(DEMO_MEDIA_VIDEO, DEMO_REMOTE_TRACK, cap->eCodecType);
        m_pWMEDemo->SetMediaSession(DEMO_MEDIA_VIDEO, DEMO_REMOTE_TRACK, cap->eCodecType);
        ret = WME_S_OK;
    }

    return ret;
}

- (long)setVideoCameraParam:(NSInteger)index
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bVideoModuleEnable != NO, WME_E_FAIL);
    
    long ret = WME_E_FAIL;
    if (index < _cameraCapList.size()) {
        //_cameraCapIndex = index;
        WmeDeviceCapability *pDC = &(_cameraCapList.at(index));
        DEMO_TRACK_TYPE ttype = DEMO_PREVIEW_TRACK;
        if (_bVideoSending) {
            ttype = DEMO_LOCAL_TRACK;
        }
        m_pWMEDemo->StopMediaTrack(DEMO_MEDIA_VIDEO, ttype);
        ret = m_pWMEDemo->SetCameraCapability(ttype, pDC);
        m_pWMEDemo->StartMediaTrack(DEMO_MEDIA_VIDEO, ttype);
        
    }
    return ret;
}

- (long)setVideoQualityType
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bVideoModuleEnable != NO, WME_E_FAIL);
    
    long devcie_platform_type = [self detectPlatform];
    WmeVideoQualityType videoQualitytype;
    
    videoQualitytype = WmeVideoQuality_SD;
    if(devcie_platform_type <= tDeviceiPhone4GCDMA)
        videoQualitytype = WmeVideoQuality_SLD;
    
    return m_pWMEDemo->SetVideoQuality(DEMO_LOCAL_TRACK, videoQualitytype);
    
}

- (long)getVideoMediaCapability:(WmeVideoMediaCapability&)vMC
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bVideoModuleEnable != NO, WME_E_FAIL);
    
    return m_pWMEDemo->GetCapability(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK, &vMC);
}

- (void)setAudioCaptureDevice:(NSInteger)index
{
    return_if_fail(m_pWMEDemo != NULL);
    return_if_fail(_bAudioModuleEnable != NO);
    
    if (index < _audioInList.size()) {
        //_micIndex = index;
        DeviceProperty *dp = &(_audioInList.at(index));
        m_pWMEDemo->SetMic(dp->dev);
    }
    
}

- (long)setAudioEncodingParam:(NSInteger)index
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bAudioModuleEnable != NO, WME_E_FAIL);
    
    long ret = WME_E_FAIL;
    if (index < _audioCapList.size()) {
        WmeAudioMediaCapability *cap = &(_audioCapList.at(index));
        m_pWMEDemo->SetMediaCodec(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK, cap->eCodecType);
        m_pWMEDemo->SetMediaSession(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK, cap->eCodecType);
        m_pWMEDemo->SetMediaCapability(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK, cap);
        m_pWMEDemo->SetMediaCodec(DEMO_MEDIA_AUDIO, DEMO_REMOTE_TRACK, cap->eCodecType);
        m_pWMEDemo->SetMediaSession(DEMO_MEDIA_AUDIO, DEMO_REMOTE_TRACK, cap->eCodecType);
        ret = WME_S_OK;
    }
    return ret;
}

- (long)setAudioSpeaker:(NSInteger)index
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bAudioModuleEnable != NO, WME_E_FAIL);
    
    long ret = WME_E_FAIL;
    
    if (index < _audioOutList.size()) {
        //_speakerIndex = index;
        _speakerPosition = _audioOutList[index].position;
        DeviceProperty *dp = &(_audioOutList.at(index));
        ret = m_pWMEDemo->SetSpeaker(dp->dev);
    }
    
    return ret;
}

- (void)switchAudioSpeaker
{
    return_if_fail(_bAudioModuleEnable != NO);
    self.speakerIndex = (_speakerIndex + 1) % _audioOutList.size();
}

- (void)setRemoteRender:(void *)render
{
    return_if_fail(m_pWMEDemo != NULL);
    return_if_fail(_bVideoModuleEnable != NO);
    
    _remoteRender = render;
    
    if (render)
        m_pWMEDemo->SetRenderView(DEMO_REMOTE_TRACK, render);
    else
        m_pWMEDemo->StopRenderView(DEMO_REMOTE_TRACK);
}

- (void)setLocalRender:(void *)render
{
    return_if_fail(m_pWMEDemo != NULL);
    return_if_fail(_bVideoModuleEnable != NO);
    
    _localRender = render;
    
    DEMO_TRACK_TYPE ttype = _bVideoSending ? DEMO_LOCAL_TRACK:DEMO_PREVIEW_TRACK;
    if (render)
        m_pWMEDemo->SetRenderView(ttype, render);
    else
        m_pWMEDemo->StopRenderView(ttype);
}

- (void)setAppShareRender:(UIView *)render
{
    if( _bContentShareModuleEnable )
    {
        m_pWMEDemo->SetRenderView(DEMO_REMOTE_DESKTOP_SHARE_TRACK, (__bridge void *)render);
        m_pWMEDemo->StartMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_REMOTE_DESKTOP_SHARE_TRACK);
    }
}

- (long)setRenderAdaptiveAspectRatio:(BOOL)enable
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bVideoModuleEnable != NO, WME_E_FAIL);
    
    return m_pWMEDemo->SetRenderAspectRatioSameWithSource(_bVideoSending ? DEMO_LOCAL_TRACK : DEMO_PREVIEW_TRACK, enable);
}

- (void)getVoiceLevel:(unsigned int &)level
{
    return_if_fail(m_pWMEDemo != NULL);
    return_if_fail(_bAudioModuleEnable != NO);
    
    m_pWMEDemo->GetVoiceLevel(level);
}

- (long)getVideoStatistics:(WmeSessionStatistics &)statistics
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bVideoModuleEnable != NO, WME_E_FAIL);
    
    return m_pWMEDemo->GetVideoStatistics(statistics);
}

- (long)getVideoStatistics:(WmeVideoStatistics &)statistics trackType:(DEMO_TRACK_TYPE)type
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bVideoModuleEnable != NO, WME_E_FAIL);
    
    return m_pWMEDemo->GetVideoStatistics(type, statistics);
}


- (long)getAudioStatistics:(WmeSessionStatistics &)statistics
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bAudioModuleEnable != NO, WME_E_FAIL);
    
    return m_pWMEDemo->GetAudioStatistics(statistics);
}

- (long)getAudioStatistics:(WmeAudioStatistics &)statistics trackType:(DEMO_TRACK_TYPE)type
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    returnv_if_fail(_bAudioModuleEnable != NO, WME_E_FAIL);
    
    return m_pWMEDemo->GetAudioStatistics(type, statistics);
}

- (long)detectPlatform
{
	char tDevice[32] = "";
	size_t size = sizeof(tDevice);
	sysctlbyname("hw.machine", tDevice, &size, 0, 0);
    
	
	if(strcmp(tDevice, "i386") == 0 || strcmp(tDevice, "x86_64") == 0)
		return tDeviceSimulator;
    
    //iphone
    if(strncmp(tDevice, "iPhone", 6) == 0)
    {
        switch(tDevice[6] - '0')
        {
            case 2: return tDeviceiPhone3GS;
            case 3:
            {
                if(strcmp(tDevice, "iPhone3,3") == 0)
                    return tDeviceiPhone4GCDMA;
                return tDeviceiPhone4G;
            }
            case 4: return tDeviceiPhone4S;
            case 5: return tDeviceiPhone5;
            default: return tDeviceiPhoneFuture;
        }
    }
    
    //ipad
    if(strncmp(tDevice, "iPad", 4) == 0)
    {
        switch(tDevice[4] - '0')
        {
            case 1: return tDeviceiPad1G;
            case 2:
            {
                if(strcmp(tDevice, "iPad2,1") == 0)
                    return tDeviceiPad2GWiFi;
                else if(strcmp(tDevice, "iPad2,3") == 0 || strcmp(tDevice, "iPad2,4") == 0)
                    return tDeviceiPad2GCDMA;
                return tDeviceiPad2GGSM;
            }
            case 3:
            {
                if(strcmp(tDevice, "iPad3,1") == 0)
                    return tDeviceiPad3;
                else if(strcmp(tDevice, "iPad3,3") == 0)
                    return tDeviceiPad3CDMA;
                return tDeviceiPad3ATT;
            }
            default: return tDeviceiPadFuture;
        }
    }
    
    //ipod
    if(strncmp(tDevice, "iPod", 4) == 0)
    {
        switch(tDevice[4] - '0')
        {
            case 3: return tDeviceiPod3G;
            case 4: return tDeviceiPod4G;
            default: return tDeviceiPodFuture;
        }
    }
    
	return tDeviceNotSupport;
}


- (void) OnDeviceChanged:(DeviceProperty *) pDP event:(WmeEventDataDeviceChanged &) changeEvent
{
    if (!pDP)
        return;
    
    DemoClient::DevicePropertyList *dplist = NULL;
    
    switch (pDP->dev_type) {
        case DEV_TYPE_CAMERA:
            dplist = &_videoInList;
            break;
        case DEV_TYPE_MIC:
            dplist = &_audioInList;
            break;
        case DEV_TYPE_SPEAKER:
            dplist = &_audioOutList;
            break;
        default:
            return;
    }
    

    switch(changeEvent.iType)
    {
        case wme::WmeDeviceAdded:
        {
            DeviceProperty dp;
            memcpy(&dp, pDP, sizeof(DeviceProperty));
            dplist->push_back(dp);

            break;
        }
        case wme::WmeDeviceRemoved:
        {
            DemoClient::DevicePropertyList::iterator iter = dplist->begin();
            for (int index = 0; iter != dplist->end(); iter++, index++)
            {
                if (iter->dev->IsSameDevice(pDP->unique_name, pDP->unique_name_len) == WME_S_OK)
                {
                    dplist->erase(iter);
                    break;
                }
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
                    break;
                }
            }
            break;
        }
        default:
            break;
    }

}

- (long)enableMyVideo:(BOOL)enable
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    
    m_pWMEDemo->EnableMyMedia(DEMO_MEDIA_VIDEO, enable);
    
    _bVideoModuleEnable = enable;
    
    return WME_S_OK;
}

- (long)enableMyAudio:(BOOL)enable
{
    returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    
    m_pWMEDemo->EnableMyMedia(DEMO_MEDIA_AUDIO, enable);
    
    _bAudioModuleEnable = enable;
    
    return WME_S_OK;
}

- (long)checkAudioPauseStatus
{
    return _bAudioPauseStatus;
}

- (long)checkVideoEnableStatus
{
    //returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    
    return _bVideoModuleEnable;
}

- (long)checkAudioEnableStatus
{
    //returnv_if_fail(m_pWMEDemo != NULL, WME_E_FAIL);
    
    return _bAudioModuleEnable;
}

- (void)disableSendingFilterFeedback
{
    if(m_pWMEDemo)
        m_pWMEDemo->DisableSendingFilterFeedback();
}

- (void)setSpeakerMute:(BOOL)enable
{
    if(m_pWMEDemo)
    {
        if (enable)
            m_pWMEDemo->SetSpeakerMute(true);
        else
            m_pWMEDemo->SetSpeakerMute(false);
    
    }
    
}
- (void)setMicMute:(BOOL)enable
{
    if(m_pWMEDemo)
    {
        if (enable)
            m_pWMEDemo->SetMicMute(true);
        else
            m_pWMEDemo->SetMicMute(false);
        
    }
}

@end
