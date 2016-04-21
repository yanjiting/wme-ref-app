//
//  WMEDeviceStatus.m
//  MediaEngineTestApp
//
//  Created by zhaozheng on 14-4-21.
//  Copyright (c) 2014年 video. All rights reserved.
//

#import "WMEDeviceStatus.h"
#import "UIDevice-Hardware.h"
#import "WMEDataProcess.h"



@implementation MessageBody

- (MessageBody*)initwithRequestType:(URLRequestType)urlReqType WithParam:(id)param
{
    if ([super init] == nil) {
        return nil;
    }
    _urlRequestType = urlReqType;
    _param = param;
    return self;
}
@end



@implementation PerformanceReport

UIDevice* mDevice;
WmeVideoStatistics wmeVideoLocalTrackStatistics;
WmeVideoStatistics wmeVideoRemoteTrackStatistics;

 - (PerformanceReport*)init
{
    self = [super init];
    mDevice =[[UIDevice alloc] init];
    return self;
}

- (NSString*)getDate
{
    NSDate *date = [NSDate date];
    NSDateFormatter *dateFormatter =[[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"MM-dd-yyyy-hh-mm-ss"];
    NSString *dateString = [dateFormatter stringFromDate:date];
    return dateString;
}

- (void) getVideoStatics
{
    [[WMEDataProcess instance] getVideoStatistics:wmeVideoLocalTrackStatistics trackType:DEMO_LOCAL_TRACK];
    [[WMEDataProcess instance] getVideoStatistics:wmeVideoRemoteTrackStatistics trackType:DEMO_REMOTE_TRACK];
    return;
}

- (NSString*)generatePostMessageType:(URLRequestType)urlRequestType withParam:(id)param
{
    if (urlRequestType == URLRequestUnknown) {
        return nil;
    }
    [self getVideoStatics];
    NSMutableString* post = nil;
    NSString* dateString = [self getDate];
    switch (urlRequestType) {
        case URLRequestStartSession:
        {
            post = [[NSMutableString alloc] initWithFormat:@"SESSION=PERFORMANCE&COMMAND=START-SESSION&TIME=%@&DEVICE=%@&OS=%@&CPU=%@&MEMORY=%lu",dateString,[mDevice platformString],[mDevice osVersion], [mDevice cpuType],(unsigned long)[mDevice totalMemoryBytes]];
            break;
        }
        case URLRequestStopSession:
        {
            post = [[NSMutableString alloc] initWithFormat:@"SESSION=PERFORMANCE&COMMAND=STOP-SESSION&TIME=%@",dateString];
            break;
        }
        case URLRequestStartCall:
        {
            NSString *sendVideo =[[WMEDataProcess instance] bVideoModuleEnable]?@"ON":@"OFF";
            NSString *recvVideo =[[WMEDataProcess instance] bVideoModuleEnable]?@"ON":@"OFF";
            NSString *audioOn = [[WMEDataProcess instance] bAudioModuleEnable]?@"ON":@"OFF";
            post =[[NSMutableString alloc] initWithFormat:@"SESSION=PERFORMANCE&COMMAND=START-CALL&TIME=%@&SEND-VIDEO=%@&RECV-VIDEO=%@&AUDIO=%@",dateString, sendVideo, recvVideo, audioOn];
            break;
        }
        case URLRequestStopCall:
        {
            post = [[NSMutableString alloc] initWithFormat:@"SESSION=PERFORMANCE&COMMAND=STOP-CALL&TIME=%@",dateString];
            break;
        }
        case URLRequestDeviceInfo:
        {
            post = [[NSMutableString alloc] init];
            [post setString:[[NSString alloc] initWithFormat:@"SESSION=PERFORMANCE&COMMAND=INFO&TIME=%@&BATTERY=%@&CHARGING=%@&CPU=%@",dateString,[mDevice batteryLevelString],[mDevice isCharging], [mDevice cpuUsage]]];
            if ([[WMEDataProcess instance] bVideoModuleEnable]) {
                NSString* resStr = [[NSString alloc] initWithFormat:@"&SEND-RES=%d-%d", wmeVideoLocalTrackStatistics.uWidth,wmeVideoLocalTrackStatistics.uHeight];
                NSString* fpsStr = [[NSString alloc] initWithFormat:@"&SEND-FPS=%d", int(wmeVideoLocalTrackStatistics.fFrameRate)];
                [post appendString:resStr];
                [post appendString:fpsStr];
            }
            if ([[WMEDataProcess instance] bVideoModuleEnable]) {
                NSString* resStr = [[NSString alloc] initWithFormat:@"&RECV-RES=%d-%d", wmeVideoRemoteTrackStatistics.uWidth,wmeVideoRemoteTrackStatistics.uHeight];
                NSString* fpsStr = [[NSString alloc] initWithFormat:@"&RECV-FPS=%d", int(wmeVideoRemoteTrackStatistics.fFrameRate)];
                [post appendString:resStr];
                [post appendString:fpsStr];
            }
            break;
        }
        default:
            break;
    }
    return post;
}


@end

@implementation WMEDeviceStatusClient

UIDevice* deviceInfo;
NSURLConnection * urlConnection;
BOOL sessionOk = NO;
dispatch_queue_t gdcqueue;

PerformanceReport* performanceReport;

+ (WMEDeviceStatusClient *)instance
{
    static dispatch_once_t pred = 0;
    __strong static id _sharedInstance = nil;
    dispatch_once(&pred, ^{
        _sharedInstance = [[self alloc]init];
    });
    return _sharedInstance;
}



- (long)initWithSetting
{
    NSUserDefaults *_userDefaults = [NSUserDefaults standardUserDefaults];
    NSDictionary *userDefaultsValueDictionary;
    self.serverURL = [_userDefaults stringForKey:@"StatusServer"];
    if ([self.serverURL lengthOfBytesUsingEncoding:NSUTF8StringEncoding]  == 0) {
        userDefaultsValueDictionary = [NSDictionary dictionaryWithObjectsAndKeys:[NSString stringWithUTF8String:"" ], @"StatusServer", nil];
        [_userDefaults registerDefaults:userDefaultsValueDictionary];
        self.serverURL = [_userDefaults stringForKey:@"StatusServer"];
    }
    self.enableCPU = [_userDefaults boolForKey:@"SendCpuUsageStatus"];
    self.enableBattery = [_userDefaults boolForKey:@"SendBatteryStatus"];
    self.updateInterval = [[_userDefaults stringForKey:@"StatusUpdateInterval"] intValue];
    self.updateInterval = 10; //now the interval is 10s.
     [UIDevice currentDevice].batteryMonitoringEnabled = YES;
    urlConnection = [[NSURLConnection alloc]init];
    gdcqueue = dispatch_queue_create("com.cisco.WME", NULL);
    performanceReport = [[PerformanceReport alloc] init];
    _isJson = NO;
    if(urlConnection == nil || gdcqueue == nil ||performanceReport == nil)
        return 1;
    MessageBody* msg = [[MessageBody alloc] initwithRequestType:URLRequestStartSession WithParam:nil];
    [self postUrlMessage:msg];
    [[WMEDataProcess instance] disableSendingFilterFeedback];
    return 0;
}


- (long)postMessage:(id)messageBody
{
    MessageBody* msgBody = messageBody;
    if (msgBody.urlRequestType == URLRequestUnknown || (msgBody.urlRequestType != URLRequestStartSession && sessionOk == NO)) {
        return 0;
    }
    
    NSString* post = nil;
    long res = 0;
    if (!self.isJson) {
        post = [performanceReport generatePostMessageType:msgBody.urlRequestType withParam:msgBody.param];
        res = [self sendURLRequestType:msgBody.urlRequestType WithContent:post];
    }
    if (msgBody.urlRequestType == URLRequestStartSession) {
        sessionOk = YES;
    }
    if (msgBody.urlRequestType == URLRequestStartCall) {
        MessageBody* msg = [[MessageBody alloc] initwithRequestType:URLRequestDeviceInfo WithParam:nil];
        dispatch_async(gdcqueue, ^{
            WMEDeviceStatusClient* __weak weakStatusClient = self;
            [weakStatusClient postMessage:msg];});
    }
    if (msgBody.urlRequestType == URLRequestDeviceInfo) {
        MessageBody* msg = [[MessageBody alloc] initwithRequestType:URLRequestDeviceInfo WithParam:nil];
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, self.updateInterval * NSEC_PER_SEC);
        dispatch_after(popTime, gdcqueue, ^{
            WMEDeviceStatusClient* __weak weakStatusClient = self;
            [weakStatusClient postMessage:msg];});
    }
    if (msgBody.urlRequestType == URLRequestStopSession) {
        sessionOk = NO;
         [UIDevice currentDevice].batteryMonitoringEnabled = NO;
    }
    
    return res;
}

- (long)postUrlMessage:(MessageBody*)msgBody
{
    WMEDeviceStatusClient* __weak weakStatusClient = self;
    dispatch_async(gdcqueue,  ^{[weakStatusClient postMessage:msgBody];});
    return 0;
}


-(long) sendURLRequestType:(URLRequestType)urlRequestType WithContent:(NSString*)post
{
    NSData *postData = [post dataUsingEncoding:NSASCIIStringEncoding allowLossyConversion:YES];
    NSString* postLength = [NSString stringWithFormat:@"%d", [postData length]];
    NSMutableURLRequest *request = [NSMutableURLRequest requestWithURL:[NSURL URLWithString:self.serverURL]];
    [request setHTTPMethod:@"POST"];
    [request setValue:postLength forHTTPHeaderField:@"Content-Length"];
    [request setValue:@"application/x-www-form-urlencoded" forHTTPHeaderField:@"Content-Type"];
    [request setHTTPBody:postData];
    NSURLResponse * response = nil;
    NSError * error = nil;
    [NSURLConnection sendSynchronousRequest:request returningResponse:&response error:&error];
    if (error) {
        NSLog(@"Send URL request(type = %d Error and Error Description is %@",urlRequestType, error.localizedDescription);
        return 1;
    }
    return 0;
}
@end
