//
//  WMEDeviceStatus.h
//  MediaEngineTestApp
//
//  Created by zhaozheng on 14-4-21.
//  Copyright (c) 2014年 video. All rights reserved.
//
#import <Foundation/Foundation.h>


typedef enum
{
    URLRequestUnknown,
    URLRequestStartSession,
    URLRequestStopSession,
    URLRequestStartCall,
    URLRequestStopCall,
    URLRequestDeviceInfo
}URLRequestType;


@interface MessageBody : NSObject
@property URLRequestType urlRequestType;
@property id             param;

- (MessageBody*)initwithRequestType:(URLRequestType)urlReqType WithParam:(id)param;

@end

@interface PerformanceReport : NSObject
- (NSString*)generatePostMessageType:(URLRequestType)urlRequestType withParam:(id)param;
@end

@interface WMEDeviceStatusClient : NSObject
@property(strong) NSString* serverURL;
@property         int64_t updateInterval;
@property         BOOL enableCPU;
@property         BOOL enableBattery;
@property         BOOL isJson;

+ (WMEDeviceStatusClient*)instance;
- (long)postUrlMessage:(MessageBody*)msgBody;
- (long)initWithSetting;
@end
