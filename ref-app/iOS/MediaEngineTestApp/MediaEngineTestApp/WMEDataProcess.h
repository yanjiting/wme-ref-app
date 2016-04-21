//
//  WMEDataProcess.h
//  MediaEngineTestApp
//
//  Created by admin on 10/15/13.
//  Copyright (c) 2013 video. All rights reserved.
//

#import <Foundation/Foundation.h>
#include "WmeEngine.h"
#import "DemoParameters.h"
#import "DemoClient.h"
#import "NotificationTransfer.h"


//added by Alan for device platform detection
#define tDeviceUnknown							0
#define tDeviceNotSupport                       1
#define tDeviceSimulator						2

#define tDeviceiPhone3GS						10
#define tDeviceiPhone4G							11
#define tDeviceiPhone4GCDMA                     12
#define tDeviceiPhone4S                         13
#define tDeviceiPhone5                          14
#define tDeviceiPhoneFuture						19

#define tDeviceiPad1G							20
#define tDeviceiPad2GWiFi                       21
#define tDeviceiPad2GGSM                        22
#define tDeviceiPad2GCDMA                       23
#define tDeviceiPad3                            24
#define tDeviceiPad3ATT                         25
#define tDeviceiPad3CDMA                        26
#define tDeviceiPadFuture                       29

#define tDeviceiPod3G							30
#define tDeviceiPod4G							31
#define tDeviceiPodFuture						39

enum {
    WME_SENDING,
    WME_RECVING,
};


@interface WMEDataProcess : NSObject <NotificationTranferDelegate>
{
    DemoClient *m_pWMEDemo;
    
    DemoClient::DevicePropertyList _audioInList;
    DemoClient::DevicePropertyList _audioOutList;
    DemoClient::DevicePropertyList _videoInList;
    
    DemoClient::AudioMediaCapabilityList _audioCapList;
    DemoClient::VideoMediaCapabilityList _videoCapList;
    DemoClient::DeviceCapabilityList _cameraCapList;
}

@property (retain) NSMutableArray *arrayAudioCapabilities;
@property (retain) NSMutableArray *arrayVideoCapabilities;
@property (retain) NSMutableArray *arrayCameraCapabilities;

@property (retain)  NSMutableArray *arrayMicDevices;
@property (retain)  NSMutableArray *arraySpeakerDevices;
@property (retain)  NSMutableArray *arrayCameraDevices;

//network property
@property (readwrite) BOOL   isHost;
@property (readwrite) BOOL   useICE;
@property (retain) NSString *jingleServerIP;
@property (retain) NSString *jingleServerPort;
@property (retain) NSString *stunServerIP;
@property (retain) NSString *stunServerPort;
@property (retain) NSString *hostIPAddress;
@property (retain) NSString *myName;
@property (retain) NSString *hostName;

//video property
//@property (readwrite) bool videoPreview;
@property (readwrite) BOOL bVideoSending;
@property (readwrite) NSInteger cameraIndex;
@property (readwrite) NSInteger cameraPosition;
@property (readwrite) NSInteger cameraCapIndex;
@property (readwrite) NSInteger videoCapIndex;
@property (readwrite) BOOL bKeepAspectRatio;
@property (readwrite) NSInteger recvWidth;
@property (readwrite) NSInteger recvHight;
@property (readwrite) BOOL bEnableExternalVideoInput;
@property (readwrite) BOOL bEnableExternalVideoOutput;
@property (copy, nonatomic) NSString *sVInputFilename;
@property (copy, nonatomic) NSString *sVOutputFilename;
@property (readwrite) NSInteger iWidth;
@property (readwrite) NSInteger iHeight;
@property (readwrite) NSInteger iColorFormat;
@property (readwrite) NSInteger iFPS;

//audio property
@property (readwrite) BOOL bAudioSending;
@property (readwrite) NSInteger speakerIndex;
@property (readwrite) NSInteger speakerPosition;
@property (readwrite) NSInteger micIndex;
@property (readwrite) NSInteger audioCapIndex;
@property (readwrite) BOOL bEnableExternalAudioInput;
@property (readwrite) BOOL bEnableExternalAudioOutput;
@property (copy, nonatomic) NSString *sAInputFilename;
@property (copy, nonatomic) NSString *sAOutputFilename;
@property (readwrite) NSInteger iChannels;
@property (readwrite) NSInteger iSampleRate;
@property (readwrite) NSInteger iBitsPerSample;

@property (readwrite) BOOL bAudioSpeakerMute;
@property (readwrite) BOOL bAudioMicMute;

// content share
@property (readwrite) BOOL bContentSharing;

//render
@property (nonatomic) void* localRender;
@property (nonatomic) void* remoteRender;
@property (nonatomic) void* contentRender;

//debug
@property (readwrite) BOOL bVideoModuleEnable;
@property (readwrite) BOOL bAudioModuleEnable;
@property (readwrite) BOOL bContentShareModuleEnable;


@property (readwrite) BOOL bDumpFlag;
@property (readwrite) BOOL bDumpCaptureDataEnable;
@property (readwrite) BOOL bDumpEncodeToRTPDataEnable;
@property (readwrite) BOOL bDumpNALToListenChannelDataEnable;
@property (readwrite) BOOL bDumpNALToDecodeDataEnable;
@property (readwrite) BOOL bDumpAfterDecodeDataEnable;

@property (readwrite) BOOL bAudioPauseStatus;

//pcap file property
@property (readwrite) BOOL   isFile;
@property (readwrite) BOOL   isFileAudio;
@property (readwrite) NSString   *audioFile;
@property (readwrite) NSString   *audioSourceIP;
@property (readwrite) unsigned short  audioSourcePort;
@property (readwrite) NSString   *audioDestinationIP;
@property (readwrite) unsigned short  audioDestinationPort;
@property (readwrite) BOOL   isFileVideo;
@property (readwrite) NSString   *videoFile;
@property (readwrite) NSString   *videoSourceIP;
@property (readwrite) unsigned short  videoSourcePort;
@property (readwrite) NSString   *videoDestinationIP;
@property (readwrite) unsigned short  videoDestinationPort;
@property (readwrite) BOOL   isFileContentShare;    // coming soon

//Method
+ (id)instance;

- (long)initWME;
- (void)uninitWME;
- (void)setTraceMaxLevel: (NSInteger)level;
- (void)setDumpDataEnabled:(unsigned int)uDataDumpFlag;
- (void)setDumpDataPath:(const char *)path;

- (void)setUISink: (backUISink *)sink;

- (void)queryAudioInDevices;
- (void)queryAudioOutDevices;
- (void)queryVideoInDevices;

- (void)queryAudioCapabilities;
- (void)queryVideoCapabilities;
- (void)queryVideoCameraCapabilities;

- (long)createAudioClient;
- (long)createVideoClient;
- (void)deleteAudioClient;
- (void)deleteVideoClient;

- (long)startAudioClient: (NSInteger)iType;
- (long)startVideoClient: (NSInteger)iType;
- (long)stopAudioClient: (NSInteger)iType;
- (long)stopVideoClient: (NSInteger)iType;
- (long)startContentShareClient;
- (long)stopContentShareClient;


- (long)clickedConnect;
- (void)clickedDisconnect;


- (long)setVideoCameraDevice:(NSInteger)index;
- (void)switchCameraDevice;

- (long)setVideoEncodingParam:(NSInteger)index;
- (long)setVideoCameraParam:(NSInteger)index;
- (long)setVideoQualityType;
- (long)getVideoMediaCapability:(WmeVideoMediaCapability&)vMC;

- (void)setAudioCaptureDevice:(NSInteger)index;
- (long)setAudioEncodingParam:(NSInteger)index;
- (long)setAudioSpeaker:(NSInteger)index;
- (void)switchAudioSpeaker;

- (void)setRemoteRender:(void *)render;
- (void)setLocalRender:(void *)render;
- (void)setAppShareRender:(UIView *)render;
- (long)setRenderAdaptiveAspectRatio:(BOOL)enable;
- (void)getVoiceLevel:(unsigned int &)level;

- (long)getVideoStatistics:(wme::WmeSessionStatistics &)statistics;
- (long)getVideoStatistics:(wme::WmeVideoStatistics &)statistics trackType:(DEMO_TRACK_TYPE)type;
- (long)getAudioStatistics:(wme::WmeSessionStatistics &)statistics;
- (long)getAudioStatistics:(wme::WmeAudioStatistics &)statistics trackType:(DEMO_TRACK_TYPE)type;

- (void)OnDeviceChanged:(DeviceProperty *) pDP event:(WmeEventDataDeviceChanged &) changeEvent;

- (long)enableMyVideo:(BOOL)enable;
- (long)enableMyAudio:(BOOL)enable;

- (long)checkAudioPauseStatus;

- (long)checkVideoEnableStatus;
- (long)checkAudioEnableStatus;

- (void)disableSendingFilterFeedback;

//- (int) WbxAeGetDocumentPath:(unsigned char*)pPath :(unsigned long&)uPathLen;

- (void)setSpeakerMute:(BOOL)enable;
- (void)setMicMute:(BOOL)enable;
@end

