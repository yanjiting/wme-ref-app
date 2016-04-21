//
//  WMEAppDelegate.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-5-17.
//  Copyright (c) 2013年 video. All rights reserved.
//
//#import "WMEDemo.h"
#import "WMEAppDelegate.h"
#import "CSILog.h"
#include "MediaConnection.h"
#include "ClickCall.h"
#include "Loopback.h"
#include "PeerCall.h"
#ifdef TEST_MODE
#import "WMEAppDelegate+Calabash.h"
#endif
#include "UdpTrace.h"

#ifdef ENABLED_GCOV_FLAG
extern "C" void __gcov_flush();
#endif
int GetDocumentPath(char *pPath, unsigned long &uPathLen)
{
    if(NULL == pPath || 0 == uPathLen)
    {
        return 1;
    }
    
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    if([paths count] == 0)
    {
        return 2;
    }
    
    NSString* document = [paths objectAtIndex:0];
    
    NSString* escapedPath = [document stringByReplacingOccurrencesOfString:@" " withString:@"\\ "];
    
    unsigned long uDocumentPathLen = [escapedPath length];
    uPathLen = (uDocumentPathLen <= uPathLen) ? uDocumentPathLen : uPathLen;
    memcpy(pPath, [escapedPath UTF8String], uPathLen);
    
    return 0;
}


class WMETraceSink : public IWmeTraceSink
{
public:
    virtual ~WMETraceSink() {}
    virtual WMERESULT OnTrace(uint32_t trace_level, const char* szInfo)
    {
        if(!TestConfig::Instance().m_bTrace2File)
            return WME_S_OK;//Hardcode for performance testing >> don't write trace file by hard code
        @autoreleasepool {
            CSILogMessage((CSILogLevel)(trace_level+1), @"%@", [NSString stringWithUTF8String:szInfo]);
        }
        return WME_S_OK;
    };
};

@implementation WMEAppDelegate {
    WMETraceSink* _traceSink;
    char * _buffer;

}

- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions
{
#ifdef ENABLED_GCOV_FLAG
    [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(gcov_flush_timer) userInfo:nil repeats:YES];
#endif
    
#ifdef RUN_ON_DEVICE
    //Set the directory of gcda files to "Documents"
    char gcdaFolder[1024] = "";
    unsigned long uPathLen = 1024;
    GetDocumentPath(gcdaFolder, uPathLen);
    setenv("GCOV_PREFIX", gcdaFolder, 1);
    setenv("GCOV_PREFIX_STRIP", "1", 1);
#endif
    
    // Override point for customization after application launch.
    NSArray* appSupportPaths = NSSearchPathForDirectoriesInDomains(NSApplicationSupportDirectory, NSUserDomainMask, YES);
    if ([appSupportPaths count] == 0) {
        return NO;
    }
    NSString *appSupportDir = [appSupportPaths lastObject];
    NSLog(@"%@", appSupportDir);
    
    //ignore the SIGPIPE signal
    signal(SIGPIPE, SIG_IGN);

    //set the user database
    NSDictionary *userDefaultsValueDictionary = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithInt:2], @"max_trace_level_preference", [NSNumber numberWithBool:NO], @"dump_data_enabled_preference", nil];

    _userDefaults = [NSUserDefaults standardUserDefaults];
    [self.userDefaults registerDefaults:userDefaultsValueDictionary];
    
#ifdef TEST_MODE
    NSString *ip = [self getIPAddress];
    // This is a handle means to allow UI Automation scripts to obtain our device's I
    [_userDefaults setObject:ip forKey:@"local_ip_for_test_mode"];
    [_userDefaults synchronize];
    

    [self initTAContext];
#endif

    //set the preference updated notification
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(updatePreferenceItem) name:NSUserDefaultsDidChangeNotification object:_userDefaults];
    
    [[CSILog sharedLog] startLogging];
    
    _traceSink = new WMETraceSink();
    WmeSetTraceMaxLevel(TestConfig::Instance().m_eTraceLevel);
    WmeSetTraceSink(_traceSink);
    
    wme::InitMediaEngine(!TestConfig::Instance().m_bUsingVPIO,TestConfig::Instance().m_bUsingTCAEC,false,false);
    wme::SetStunTraceSink(CTaIWmeStunTraceSink::GetInstance());
    
    wme::SetTraceServerSink(CTaIWmeTraceServerSink::GetInstance());
    
    char *traceNodeList =
    (char*)"192.168.31.1:10000\n192.168.31.1:10001\n192.168.31.1:10002\n192.168.31.1:10003";
    
    int sz = (int)strlen(traceNodeList);
    wme::StartTraceServer(traceNodeList, sz);
        
    /*NSString * resourcePath = [[NSBundle mainBundle] resourcePath];
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentPath = [paths objectAtIndex:0];
    //TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"audio_1_8000_16.pcm"] UTF8String], "", WmeSessionType_Audio);
    TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"audio_1_8000_16.pcm"] UTF8String], [[documentPath stringByAppendingPathComponent:@"audio_1_8000_16.pcm"] UTF8String], WmeSessionType_Audio);
    TestConfig::i().setFilePath([[resourcePath stringByAppendingPathComponent:@"video_640_360_24_i420.yuv"] UTF8String], [[documentPath stringByAppendingPathComponent:@"video_640_360_24_i420.yuv"] UTF8String], WmeSessionType_Video);*/
    TestConfig::i().m_strDeviceModelName = [[self getIOSDeviceModel] UTF8String];
    TestConfig::i().m_strSystemOSVersion = [[[UIDevice currentDevice] systemVersion] UTF8String];
    udptrace::start();
    
    _buffer = NULL;
    if(TestConfig::Instance().m_nProfileConsumerMemorySize > 0){
        _buffer = new char[TestConfig::Instance().m_nProfileConsumerMemorySize];
        memset(_buffer,2,TestConfig::Instance().m_nProfileConsumerMemorySize);
    }
    return YES;
}
#ifdef ENABLED_GCOV_FLAG
-(void)gcov_flush_timer
{
    NSLog(@"timer flush gcov!");
    __gcov_flush();
}
#endif

-(void)initTAContext{
    
    
    if(backdoorAgent==NULL){

        
        NSString * resourcePath = [[NSBundle mainBundle] resourcePath];
        NSString * refPath = [resourcePath stringByAppendingString:@"/../../../../../../../"];
        NSLog(@"refPath = %@", refPath);
        backdoorAgent = new CBackdoorAgent([refPath UTF8String],
                                           NULL,
                                           NULL,
                                           NULL );
        

    }
}


//Notify from userdefault
- (void)updatePreferenceItem
{
    //NSInteger maxTraceLevel = [_userDefaults integerForKey:@"max_trace_level_preference"];
    //BOOL dumpDataEnabled = [_userDefaults boolForKey:@"dump_data_enabled_preference"];

}

- (void)applicationWillResignActive:(UIApplication *)application
{
    // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
    // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application
{
    // Use this method to release shared resources, save user data, invalidate timers, and store enough application state information to restore your application to its current state in case it is terminated later. 
    // If your application supports background execution, this method is called instead of applicationWillTerminate: when the user quits.
#ifdef ENABLED_GCOV_FLAG
    NSLog(@"timer flush gcov!");
    __gcov_flush();
#endif
}

- (void)applicationWillEnterForeground:(UIApplication *)application
{
    // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application
{
    // Restart any tasks that were paused (or not yet started) while the application was inactive. If the application was previously in the background, optionally refresh the user interface.
}

- (void)applicationWillTerminate:(UIApplication *)application
{
    // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
    if(!PeerCall::Instance()->isActive())
    {
        LoopbackCall::Instance()->stopLoopback();
    }
    else
    {
        PeerCall::Instance()->stopPeer();
    }
    wme::UninitMediaEngine();
    WmeSetTraceSink(NULL);
    if (_traceSink) {
        delete _traceSink;
        _traceSink = NULL;
    }
    [[CSILog sharedLog] stopLogging];
    
    if(_buffer!=NULL)
        delete [] _buffer;
}
#import <sys/sysctl.h>
- (NSString*) getIOSDeviceModel
{
    size_t size;
    sysctlbyname("hw.machine", NULL, &size, NULL, 0);
    char *machine = (char *)malloc(size);
    sysctlbyname("hw.machine", machine, &size, NULL, 0);
    NSString *platform = [NSString stringWithUTF8String:machine];
    free(machine);
    
     if ([platform isEqualToString:@"iPhone1,1"])    return @"iPhone 1G";
     if ([platform isEqualToString:@"iPhone1,2"])    return @"iPhone 3G";
     if ([platform isEqualToString:@"iPhone2,1"])    return @"iPhone 3GS";
     if ([platform isEqualToString:@"iPhone3,1"])    return @"iPhone 4 (GSM)";
     if ([platform isEqualToString:@"iPhone3,3"])    return @"iPhone 4 (CDMA)";
     if ([platform isEqualToString:@"iPhone4,1"])    return @"iPhone 4S";
     if ([platform isEqualToString:@"iPhone5,1"])    return @"iPhone 5 (GSM)";
     if ([platform isEqualToString:@"iPhone5,2"])    return @"iPhone 5 (GSM+CDMA)";
     if ([platform isEqualToString:@"iPhone5,3"])    return @"iPhone 5c (GSM)";
     if ([platform isEqualToString:@"iPhone5,4"])    return @"iPhone 5c (GSM+CDMA)";
     if ([platform isEqualToString:@"iPhone6,1"])    return @"iPhone 5s (GSM)";
     if ([platform isEqualToString:@"iPhone6,2"])    return @"iPhone 5s (GSM+CDMA)";
     if ([platform isEqualToString:@"iPhone7,1"])    return @"iPhone 6 Plus";
     if ([platform isEqualToString:@"iPhone7,2"])    return @"iPhone 6";
     if ([platform isEqualToString:@"iPod1,1"])      return @"iPod Touch 1G";
     if ([platform isEqualToString:@"iPod2,1"])      return @"iPod Touch 2G";
     if ([platform isEqualToString:@"iPod3,1"])      return @"iPod Touch 3G";
     if ([platform isEqualToString:@"iPod4,1"])      return @"iPod Touch 4G";
     if ([platform isEqualToString:@"iPod5,1"])      return @"iPod Touch 5G";
     if ([platform isEqualToString:@"iPad1,1"])      return @"iPad";
     if ([platform isEqualToString:@"iPad2,1"])      return @"iPad 2 (WiFi)";
     if ([platform isEqualToString:@"iPad2,2"])      return @"iPad 2 (GSM)";
     if ([platform isEqualToString:@"iPad2,3"])      return @"iPad 2 (CDMA)";
     if ([platform isEqualToString:@"iPad2,4"])      return @"iPad 2 (WiFi)";
     if ([platform isEqualToString:@"iPad2,5"])      return @"iPad Mini (WiFi)";
     if ([platform isEqualToString:@"iPad2,6"])      return @"iPad Mini (GSM)";
     if ([platform isEqualToString:@"iPad2,7"])      return @"iPad Mini (GSM+CDMA)";
     if ([platform isEqualToString:@"iPad3,1"])      return @"iPad 3 (WiFi)";
     if ([platform isEqualToString:@"iPad3,2"])      return @"iPad 3 (GSM+CDMA)";
     if ([platform isEqualToString:@"iPad3,3"])      return @"iPad 3 (GSM)";
     if ([platform isEqualToString:@"iPad3,4"])      return @"iPad 4 (WiFi)";
     if ([platform isEqualToString:@"iPad3,5"])      return @"iPad 4 (GSM)";
     if ([platform isEqualToString:@"iPad3,6"])      return @"iPad 4 (GSM+CDMA)";
     if ([platform isEqualToString:@"iPad4,1"])      return @"iPad Air (WiFi)";
     if ([platform isEqualToString:@"iPad4,2"])      return @"iPad Air (GSM)";
     if ([platform isEqualToString:@"iPad4,4"])      return @"iPad Mini Retina (WiFi)";
     if ([platform isEqualToString:@"iPad4,5"])      return @"iPad Mini Retina (GSM)";
     if ([platform isEqualToString:@"i386"])         return @"Simulator";
     if ([platform isEqualToString:@"x86_64"])       return @"Simulator";
    return @"";
}
@end
