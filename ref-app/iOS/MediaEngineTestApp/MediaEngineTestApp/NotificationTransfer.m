//
//  NotificationTransfer.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-9-12.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import "NotificationTransfer.h"
#import "WMEDataProcess.h"


@implementation NotificationTransfer
@synthesize nativeNotificationSink = _nativeNotificationSink;

+ (NotificationTransfer *)instance
{
    static dispatch_once_t pred = 0;
    __strong static id _sharedInstance = nil;
    dispatch_once(&pred, ^{
        _sharedInstance = [[self alloc]init];
    });
    return _sharedInstance;
}

-  (void)initNotificationTransfer
{
    self.arrayObserver = [[NSMutableArray alloc] init];
    self.nativeNotificationSink = new NativeNotificationSink((__bridge void *)self);
    
    //Set the sink to WME module
    [[WMEDataProcess instance]setUISink:self.nativeNotificationSink];
}
- (void)addNotificationObserver:(id<NotificationTranferDelegate> )observer
{
    [self.arrayObserver addObject:observer];
}

- (void)removeNotificationObserver:(id<NotificationTranferDelegate> )observer
{
    [self.arrayObserver removeObject:observer];
}

- (void)networkDisconnect:(DEMO_MEDIA_TYPE) eType
{
    id<NotificationTranferDelegate> obsever;
    
    for (NSUInteger index=0; index < [self.arrayObserver count]; index++) {
        obsever = [self.arrayObserver objectAtIndex:index];
        if ([obsever respondsToSelector:@selector(networkDisconnect:)]) {
            [obsever networkDisconnect:eType];
        }
    }
    
}
- (void)networkConnect:(DEMO_MEDIA_TYPE) eType
{
    id<NotificationTranferDelegate> obsever;
    
    for (NSUInteger index=0; index < [self.arrayObserver count]; index++) {
        obsever = [self.arrayObserver objectAtIndex:index];
        if ([obsever respondsToSelector:@selector(networkConnect:)]) {
            [obsever networkConnect:eType];
        }
    }
}

- (void)OnDeviceChanged:(DeviceProperty *)pDP event:(wme::WmeEventDataDeviceChanged &)event
{
    id<NotificationTranferDelegate> obsever;
    
    for (NSUInteger index=0; index < [self.arrayObserver count]; index++) {
        obsever = [self.arrayObserver objectAtIndex:index];
        if ([obsever respondsToSelector:@selector(OnDeviceChanged:event:)]) {
            [obsever OnDeviceChanged:pDP event:event];
        }
    }
}
- (void)OnDecodeSizeChanged:(uint32_t)uLabel width:(uint32_t)uWidth height:(uint32_t)uHeight
{
    id<NotificationTranferDelegate> obsever;
    
    for (NSUInteger index=0; index < [self.arrayObserver count]; index++) {
        obsever = [self.arrayObserver objectAtIndex:index];
        if ([obsever respondsToSelector:@selector(OnDecodeSizeChanged:width:height:)]) {
            [obsever OnDecodeSizeChanged:uLabel width:uWidth height:uHeight];
        }
    }
}
- (void)OnEndOfStream:(DEMO_MEDIA_TYPE) mtype
{
    id<NotificationTranferDelegate> obsever;
    
    for (NSUInteger index=0; index < [self.arrayObserver count]; index++) {
        obsever = [self.arrayObserver objectAtIndex:index];
        if ([obsever respondsToSelector:@selector(OnEndOfStream:)]) {
            [obsever OnEndOfStream:mtype];
        }
    }
}
@end

