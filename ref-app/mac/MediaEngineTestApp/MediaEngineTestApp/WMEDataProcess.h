//
//  WMEDataProcess.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-8-6.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <Foundation/Foundation.h>

#include "DemoClient.h"
#ifdef ENABLE_COMMAND_LINE
#import "CustomCommandLineInfo.h"
#import "Syslog.h"
#endif

@protocol  UIChangeNotifierDelegate <NSObject>
@optional
- (void)networkDisconnectFromClient:(DEMO_MEDIA_TYPE) eType;
- (void)connectRemoteFailureFromClient:(DEMO_MEDIA_TYPE) eType;
- (void)OnDeviceChanged:(DeviceProperty *) pDP event:(WmeEventDataDeviceChanged &) changeEvent index:(NSInteger)selectedIndex;
- (void)OnVolumeChange:(WmeEventDataVolumeChanged &) changeEvent;
- (void)OnDecodeSizeChanged:(uint32_t)uLabel width:(uint32_t)uWidth height:(uint32_t)uHeight;

@end

typedef enum
{
    CONNECTION_SUCCESS = 0,
    CONNECTION_FAILED,
    CONNECTION_INPUTERROR,
    CONNECTION_TIMEOUT,
    CONNECTION_DEFAULT  = 0xffff,
}CONNECTION_RETURN;

class DemoClientSink;

@interface WMEDataProcess : NSObject
{
    //Data process to UI
    NSMutableArray *_arrayVideoCaptureDeviceCBX;
    NSMutableArray *_arrayVideoEncodingParamCBX;
    NSMutableArray *_arrayAudioCaptureDeviceCBX;
    NSMutableArray *_arrayAudioEncodingParamCBX;
    NSMutableArray *_arrayAudioPlayDeviceCBX;
    NSArray *_arrayShareSources;
    NSInteger _audioCaptureVolumeSLD;
    NSInteger _audioPlayVolumeSLD;

    //UI to Data process module
    NSString *_JINGLEServerIP;
    NSString *_JINGLEServerPort;
    NSString *_STUNServerIP;
    NSString *_STUNServerPort;
    NSString *_HostIPAddress;
    NSString *_MyName;
    NSString *_HostName;
    
    // others
    DemoClient *_demoClient;
    DemoClientSink *_demoClientSink;
    
    DemoClient::DevicePropertyList _videoInList;
    DemoClient::VideoMediaCapabilityList _videoCapList;
    
    DemoClient::DevicePropertyList _audioInList;
    DemoClient::AudioMediaCapabilityList _audioCapList;
    DemoClient::DevicePropertyList _audioOutList;
    
    std::vector<std::string> _appSource;
    
    NSInteger _videoCaptureIndex;
    NSInteger _videoEncodingIndex;
    NSInteger _audioCaptureIndex;
    NSInteger _audioEncodingIndex;
    NSInteger _audioPlayIndex;
    NSInteger _shareSourceIndex;
    
    DEMO_TRACK_TYPE m_vtrack;
    
    BOOL _audioOutMute;
    BOOL _audioInMute;
    
    BOOL _isHost;
    BOOL _enableNAT;
    BOOL _sendVideo;
    BOOL _sendAudio;
    BOOL _enableVideo;
    BOOL _enableAudio;
    
    BOOL _sendDSVideo;      //whsu
    BOOL _enableDSVideo;
    id _remoteDSView;
    WmeTrackRenderScalingModeType _eRenderModeScreenSharing;
    
    id _selfView;
    id _remoteView;
    id _previewView;
    //Local
    
    //Delegate
    id<UIChangeNotifierDelegate> _delegate;
    
    
#ifdef ENABLE_COMMAND_LINE
	Syslog *_logger;
	CCustomCommandLineInfo *_clInfo;
#endif
}
//For delegate
@property (weak, nonatomic) id<UIChangeNotifierDelegate> delegate;

//variable
@property (retain) NSMutableArray *arrayVideoCaptureDeviceCBX;
@property (retain) NSMutableArray *arrayVideoEncodingParamCBX;
@property (retain) NSMutableArray *arrayAudioCaptureDeviceCBX;
@property (retain) NSMutableArray *arrayAudioEncodingParamCBX;
@property (retain) NSMutableArray *arrayAudioPlayDeviceCBX;
@property (retain) NSArray *arrayShareSources;
@property (readwrite) NSInteger audioCaptureVolumeSLD;
@property (readwrite) NSInteger audioPlayVolumeSLD;
@property (retain) NSString *JINGLEServerIP;
@property (retain) NSString *JINGLEServerPort;
@property (retain) NSString *STUNServerIP;
@property (retain) NSString *STUNServerPort;
@property (retain) NSString *HostIPAddress;
@property (retain) NSString *MyName;
@property (retain) NSString *HostName;
@property (readwrite) BOOL audioOutMute;
@property (readwrite) BOOL audioInMute;
@property (readwrite) BOOL enableVideo;
@property (readwrite) BOOL enableAudio;
@property (readwrite) BOOL enableDSVideo;
#ifdef ENABLE_COMMAND_LINE
@property (readwrite) Syslog *logger;
@property (readwrite) CCustomCommandLineInfo *clInfo;
#endif
//Method
- (long)initWME;
- (void)uninitWME;

- (void)SetDumpDataEnabled:(unsigned int)uDataDumpFlag;

- (void)queryAudioInDevices;
- (void)queryAudioOutDevices;
- (void)queryVideoDevices;
- (void)queryCapabilities;

- (void)selectAudioInDevice:(long)index;
- (void)selectAudioOutDevice:(long)index;
- (void)selectVideoDevice:(long)index track:(DEMO_TRACK_TYPE)ttype;

- (void)selectAudioCapability:(long)index track:(DEMO_TRACK_TYPE)ttype;
- (void)selectVideoCapability:(long)index track:(DEMO_TRACK_TYPE)ttype;

- (void)setAudioInVolume:(long)value;
- (void)setAudioOutVolume:(long)value;
- (void)setAudioInMuteOrUnmute:(bool)bmute;
- (void)setAudioOutMuteOrUnmute:(bool)bmute;

- (void)networkDisconnectFromClient:(DEMO_MEDIA_TYPE) eType;
- (void)connectRemoteFailureFromClient:(DEMO_MEDIA_TYPE) eType;
- (void)OnDeviceChanged:(DeviceProperty *) pDP event:(WmeEventDataDeviceChanged &) changeEvent;
- (void)OnVolumeChange:(WmeEventDataVolumeChanged &) changeEvent;
- (void)OnDecodeSizeChanged:(uint32_t)uLabel width:(uint32_t)uWidth height:(uint32_t)uHeight;

- (void)initDataSource;
- (void)getDataSource;

- (void)getVideoCaptureDeviceCBX;
- (void)getVideoEncodingParamCBX;
- (void)getAudioCaptureDeviceCBX;
- (void)getAudioEncodingParamCBX;
- (void)getAudioPlayDeviceCBX;
- (void)getAudioCaptureVolumeSLD;
- (void)getAudioPlayVolumeSLD;

- (void)setVideoCaptureDevice:(NSInteger)index;
- (void)setVideoEncodingParam:(NSInteger)index;
- (void)setAudioCaptureDevice:(NSInteger)index;
- (void)setAudioEncodingParam:(NSInteger)index;
- (void)setAudioPlayDevice:(NSInteger)index;

- (void)setAudioCaptureVolume:(NSInteger)number;
- (void)setAudioPlayVolume:(NSInteger)number;
- (void)setAudioPlayVolumeMute:(BOOL)state;
- (void)setAudioCaptureVolumeMute:(BOOL)state;

- (void)setAudioInputFile:(NSString*)path;
- (void)setAudioOutputFile:(NSString*)path;
- (void)setVideoInputFile:(NSString*)path;
- (void)setVideoOutputFile:(NSString*)path;

//TRUE: host; FALSE: client
- (void)setRoleHostOrClient:(BOOL)isHost;

- (void)setNetworkEnableNAT:(BOOL)state;
- (void)setActionSendVideo:(BOOL)state;
- (void)setActionSendAudio:(BOOL)state;
- (void)setActionMaxTraceLevel:(NSInteger)level;

-(void)setShareSourceIndex:(NSInteger)idx;
- (void)setRenderModeScreenSharing:(WmeTrackRenderScalingModeType)mode;
- (WmeTrackRenderScalingModeType)getRenderModeScreenSharing;

- (CONNECTION_RETURN)clickedConnect;
- (void)clickedDisconnect;
- (void)clickedPreview:(BOOL)state;


- (void)setWindowForSelfView:(id)windowhandle;
- (void)setWindowForAttendeeView:(id)windowhandle;
- (void)setWindowForPreview:(id)windowhandle;
- (void)setWindowForScreenAttendeeView:(id)windowhandle;

- (void)getVideoStatistics:(WmeSessionStatistics &)statistics;
- (void)getAudioStatistics:(WmeSessionStatistics &)statistics;
- (void)getVideoStatistics:(WmeVideoStatistics &)statistics Track:(DEMO_TRACK_TYPE)type;
- (void)getAudioStatistics:(WmeAudioStatistics &)statistics Track:(DEMO_TRACK_TYPE)type;
- (void)getNetworkIndex:(WmeNetworkIndex &)idx Direction:(WmeNetworkDirection)d;
@end


class DemoClientSink : public backUISink
{
    void *_process;
public:
    explicit DemoClientSink(void *process)
    {
        _process = process;
    }
    
	virtual void networkDisconnectFromClient(DEMO_MEDIA_TYPE eType)
    {
        if (_process)
            [((WMEDataProcess *)_process) networkDisconnectFromClient:eType];
    }
    
	virtual void connectRemoteFailureFromClient(DEMO_MEDIA_TYPE eType)
    {
        if (_process)
            [((WMEDataProcess *)_process) connectRemoteFailureFromClient:eType];
    }
    
	virtual void OnDeviceChanged(DeviceProperty *pDP, WmeEventDataDeviceChanged &changeEvent)
    {
        if (_process)
            [((WMEDataProcess *)_process) OnDeviceChanged:pDP event:changeEvent];
    }
    virtual void OnVolumeChange(WmeEventDataVolumeChanged &changeEvent)
    {
        if (_process)
            [((WMEDataProcess *)_process) OnVolumeChange:changeEvent];
    }
    virtual void OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight)
    {
        if (_process)
            [((WMEDataProcess *)_process) OnDecodeSizeChanged:uLabel width:uWidth height:uHeight];
    }
    
};


