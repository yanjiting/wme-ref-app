/*
 Erica Sadun, http://ericasadun.com
 iPhone Developer's Cookbook, 6.x Edition
 BSD License, Use at your own risk
 */

#include <sys/sysctl.h>
#import <mach/mach.h>
#import <sys/types.h>


#import "UIDevice-Hardware.h"
#define APP_STORE  0

#if APP_STORE != 1
CFTypeRef (*IOPSCopyPowerSourcesInfo)(void) = NULL;
CFArrayRef (*IOPSCopyPowerSourcesList)(CFTypeRef blob) = NULL;
CFDictionaryRef (*IOPSGetPowerSourceDescription)(CFTypeRef blob, CFTypeRef ps) = NULL;

static NSString *IOKitBatteryLevelString() {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        CFURLRef bundleURL = CFURLCreateWithString(kCFAllocatorDefault, CFSTR("/System/Library/Frameworks/IOKit.framework"), NULL);
        if (bundleURL != NULL) {
            CFBundleRef bundle = CFBundleCreate(kCFAllocatorDefault, bundleURL);
            if (bundle != NULL) {
                IOPSCopyPowerSourcesInfo = CFBundleGetFunctionPointerForName(bundle, CFSTR("IOPSCopyPowerSourcesInfo"));
                IOPSCopyPowerSourcesList = CFBundleGetFunctionPointerForName(bundle, CFSTR("IOPSCopyPowerSourcesList"));
                IOPSGetPowerSourceDescription = CFBundleGetFunctionPointerForName(bundle, CFSTR("IOPSGetPowerSourceDescription"));
                
                CFRelease(bundle);
                bundle = NULL;
            }
            
            CFRelease(bundleURL);
            bundleURL = NULL;
        }
    });
    
    NSString *batteryLevelString = nil;
    if ((IOPSCopyPowerSourcesInfo != NULL) && (IOPSCopyPowerSourcesList != NULL) && (IOPSGetPowerSourceDescription != NULL)) {
        CFTypeRef blob = IOPSCopyPowerSourcesInfo();
        if (blob != NULL) {
            CFArrayRef ps = IOPSCopyPowerSourcesList(blob);
            if (ps != NULL) {
                if (CFArrayGetCount(ps) == 1) {
                    NSDictionary *desc = (__bridge NSDictionary *)IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(ps, 0));
                    NSNumber *currentCapacity = desc[@"Current Capacity"];
                    NSNumber *maxCapacity = desc[@"Max Capacity"];
                    if ((currentCapacity != nil) && (maxCapacity != nil)) {
                        int batteryIntLevel =([currentCapacity floatValue] / [maxCapacity floatValue] * 100);
                        batteryLevelString = [NSString stringWithFormat:@"%d", batteryIntLevel];
                    }
                }
                
                CFRelease(ps);
                ps = NULL;
            }
            
            CFRelease(blob);
            blob = NULL;
        }
    }
    
    return batteryLevelString;
}
#endif


@implementation UIDevice (Hardware)

#define DH_MACHINE_IPHONE_1_1 @"iPhone1,1"
#define DH_MACHINE_IPHONE_1_2 @"iPhone1,2"
#define DH_MACHINE_IPHONE_2_1 @"iPhone2,1"
#define DH_MACHINE_IPHONE_3_1 @"iPhone3,1"
#define DH_MACHINE_IPHONE_3_3 @"iPhone3,3"
#define DH_MACHINE_IPHONE_4_1 @"iPhone4,1"
#define DH_MACHINE_IPHONE_5_1 @"iPhone5,1"
#define DH_MACHINE_IPHONE_5_2 @"iPhone5,2"
#define DH_MACHINE_IPHONE_5_3 @"iPhone5,3"
#define DH_MACHINE_IPHONE_5_4 @"iPhone5,4"
#define DH_MACHINE_IPHONE_6_1 @"iPhone6,1"
#define DH_MACHINE_IPHONE_6_2 @"iPhone6,2"
#define DH_MACHINE_IPOD_1_1 @"iPod1,1"
#define DH_MACHINE_IPOD_2_1 @"iPod2,1"
#define DH_MACHINE_IPOD_3_1 @"iPod3,1"
#define DH_MACHINE_IPOD_4_1 @"iPod4,1"
#define DH_MACHINE_IPOD_5_1 @"iPod5,1"
#define DH_MACHINE_IPAD_1_1 @"iPad1,1"
#define DH_MACHINE_IPAD_2_1 @"iPad2,1"
#define DH_MACHINE_IPAD_2_2 @"iPad2,2"
#define DH_MACHINE_IPAD_2_3 @"iPad2,3"
#define DH_MACHINE_IPAD_2_4 @"iPad2,4"
#define DH_MACHINE_IPAD_2_5 @"iPad2,5"
#define DH_MACHINE_IPAD_2_6 @"iPad2,6"
#define DH_MACHINE_IPAD_2_7 @"iPad2,7"
#define DH_MACHINE_IPAD_3_1 @"iPad3,1"
#define DH_MACHINE_IPAD_3_2 @"iPad3,2"
#define DH_MACHINE_IPAD_3_3 @"iPad3,3"
#define DH_MACHINE_IPAD_3_4 @"iPad3,4"
#define DH_MACHINE_IPAD_3_5 @"iPad3,5"
#define DH_MACHINE_IPAD_3_6 @"iPad3,6"
#define DH_MACHINE_IPAD_4_1 @"iPad4,1"
#define DH_MACHINE_IPAD_4_2 @"iPad4,2"
#define DH_MACHINE_IPAD_4_4 @"iPad4,4"
#define DH_MACHINE_IPAD_4_5 @"iPad4,5"
#define DH_MACHINE_I386   @"i386"
#define DH_MACHINE_X86_64 @"x86_64"

- (DeviceHardwareSpecificPlatform)specificPlatform {
	NSString *platform = [self machine];
	if ([platform isEqualToString:DH_MACHINE_IPHONE_1_1])    return DeviceHardwareSpecificPlatform_iPhone_1G;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_1_2])    return DeviceHardwareSpecificPlatform_iPhone_3G;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_2_1])    return DeviceHardwareSpecificPlatform_iPhone_3GS;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_3_1])    return DeviceHardwareSpecificPlatform_iPhone_4;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_3_3])    return DeviceHardwareSpecificPlatform_iPhone_4_Verizon;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_4_1])    return DeviceHardwareSpecificPlatform_iPhone_4S;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_1])    return DeviceHardwareSpecificPlatform_iPhone_5_GSM;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_2])    return DeviceHardwareSpecificPlatform_iPhone_5_CDMA_GSM;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_3])    return DeviceHardwareSpecificPlatform_iPhone_5C_CDMA_GSM;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_4])    return DeviceHardwareSpecificPlatform_iPhone_5C_Global;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_6_1])    return DeviceHardwareSpecificPlatform_iPhone_5S_CDMA_GSM;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_6_2])    return DeviceHardwareSpecificPlatform_iPhone_5S_Global;
	if ([platform isEqualToString:DH_MACHINE_IPOD_1_1])      return DeviceHardwareSpecificPlatform_iPod_Touch_1G;
	if ([platform isEqualToString:DH_MACHINE_IPOD_2_1])      return DeviceHardwareSpecificPlatform_iPod_Touch_2G;
	if ([platform isEqualToString:DH_MACHINE_IPOD_3_1])      return DeviceHardwareSpecificPlatform_iPod_Touch_3G;
	if ([platform isEqualToString:DH_MACHINE_IPOD_4_1])      return DeviceHardwareSpecificPlatform_iPod_Touch_4G;
	if ([platform isEqualToString:DH_MACHINE_IPOD_5_1])      return DeviceHardwareSpecificPlatform_iPod_Touch_5G;
	if ([platform isEqualToString:DH_MACHINE_IPAD_1_1])      return DeviceHardwareSpecificPlatform_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_1])      return DeviceHardwareSpecificPlatform_iPad_2_WiFi;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_2])      return DeviceHardwareSpecificPlatform_iPad_2_GSM;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_3])      return DeviceHardwareSpecificPlatform_iPad_2_CDMA;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_4])      return DeviceHardwareSpecificPlatform_iPad_2_WiFi;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_5])      return DeviceHardwareSpecificPlatform_iPad_Mini_WiFi;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_6])      return DeviceHardwareSpecificPlatform_iPad_Mini_GSM;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_7])      return DeviceHardwareSpecificPlatform_iPad_Mini_CDMA_GSM;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_1])      return DeviceHardwareSpecificPlatform_iPad_3_WiFi;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_2])      return DeviceHardwareSpecificPlatform_iPad_3_CDMA;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_3])      return DeviceHardwareSpecificPlatform_iPad_3_GSM;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_4])      return DeviceHardwareSpecificPlatform_iPad_4_WiFi;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_5])      return DeviceHardwareSpecificPlatform_iPad_4_GSM;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_6])      return DeviceHardwareSpecificPlatform_iPad_4_CDMA_GSM;
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_1])      return DeviceHardwareSpecificPlatform_iPad_Air_WiFi;
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_2])      return DeviceHardwareSpecificPlatform_iPad_Air_CDMA_GSM;
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_4])      return DeviceHardwareSpecificPlatform_iPad_Mini_Retina_WiFi;
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_5])      return DeviceHardwareSpecificPlatform_iPad_Mini_Retina_CDMA_GSM;
	if ([platform isEqualToString:DH_MACHINE_I386])          return DeviceHardwareSpecificPlatform_Simulator;
	if ([platform isEqualToString:DH_MACHINE_X86_64])        return DeviceHardwareSpecificPlatform_Simulator;
	return DeviceHardwareSpecificPlatform_Unknown;
}

- (DeviceHardwareGeneralPlatform)generalPlatform {
	NSString *platform = [self machine];
	if ([platform isEqualToString:DH_MACHINE_IPHONE_1_1])    return DeviceHardwareGeneralPlatform_iPhone_1G;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_1_2])    return DeviceHardwareGeneralPlatform_iPhone_3G;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_2_1])    return DeviceHardwareGeneralPlatform_iPhone_3GS;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_3_1])    return DeviceHardwareGeneralPlatform_iPhone_4;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_3_3])    return DeviceHardwareGeneralPlatform_iPhone_4;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_4_1])    return DeviceHardwareGeneralPlatform_iPhone_4S;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_1])    return DeviceHardwareGeneralPlatform_iPhone_5;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_2])    return DeviceHardwareGeneralPlatform_iPhone_5;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_3])    return DeviceHardwareGeneralPlatform_iPhone_5C;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_4])    return DeviceHardwareGeneralPlatform_iPhone_5C;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_6_1])    return DeviceHardwareGeneralPlatform_iPhone_5S;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_6_2])    return DeviceHardwareGeneralPlatform_iPhone_5S;
	if ([platform isEqualToString:DH_MACHINE_IPOD_1_1])      return DeviceHardwareGeneralPlatform_iPod_Touch_1G;
	if ([platform isEqualToString:DH_MACHINE_IPOD_2_1])      return DeviceHardwareGeneralPlatform_iPod_Touch_2G;
	if ([platform isEqualToString:DH_MACHINE_IPOD_3_1])      return DeviceHardwareGeneralPlatform_iPod_Touch_3G;
	if ([platform isEqualToString:DH_MACHINE_IPOD_4_1])      return DeviceHardwareGeneralPlatform_iPod_Touch_4G;
	if ([platform isEqualToString:DH_MACHINE_IPOD_5_1])      return DeviceHardwareGeneralPlatform_iPod_Touch_5G;
	if ([platform isEqualToString:DH_MACHINE_IPAD_1_1])      return DeviceHardwareGeneralPlatform_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_1])      return DeviceHardwareGeneralPlatform_iPad_2;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_2])      return DeviceHardwareGeneralPlatform_iPad_2;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_3])      return DeviceHardwareGeneralPlatform_iPad_2;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_4])      return DeviceHardwareGeneralPlatform_iPad_2;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_5])      return DeviceHardwareGeneralPlatform_iPad_Mini;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_6])      return DeviceHardwareGeneralPlatform_iPad_Mini;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_7])      return DeviceHardwareGeneralPlatform_iPad_Mini;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_1])      return DeviceHardwareGeneralPlatform_iPad_3;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_2])      return DeviceHardwareGeneralPlatform_iPad_3;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_3])      return DeviceHardwareGeneralPlatform_iPad_3;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_4])      return DeviceHardwareGeneralPlatform_iPad_4;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_5])      return DeviceHardwareGeneralPlatform_iPad_4;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_6])      return DeviceHardwareGeneralPlatform_iPad_4;
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_1])      return DeviceHardwareGeneralPlatform_iPad_Air;
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_2])      return DeviceHardwareGeneralPlatform_iPad_Air;
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_4])      return DeviceHardwareGeneralPlatform_iPad_Mini_Retina;
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_5])      return DeviceHardwareGeneralPlatform_iPad_Mini_Retina;
	if ([platform isEqualToString:DH_MACHINE_I386])          return DeviceHardwareGeneralPlatform_Simulator;
	if ([platform isEqualToString:DH_MACHINE_X86_64])        return DeviceHardwareGeneralPlatform_Simulator;
	return DeviceHardwareGeneralPlatform_Unknown;
}

- (DeviceHardwarePlatformType)platformType {
	NSString *platform = [self machine];
	if ([platform isEqualToString:DH_MACHINE_IPHONE_1_1])    return DeviceHardwarePlatformType_iPhone;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_1_2])    return DeviceHardwarePlatformType_iPhone;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_2_1])    return DeviceHardwarePlatformType_iPhone;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_3_1])    return DeviceHardwarePlatformType_iPhone;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_3_3])    return DeviceHardwarePlatformType_iPhone;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_4_1])    return DeviceHardwarePlatformType_iPhone;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_1])    return DeviceHardwarePlatformType_iPhone;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_2])    return DeviceHardwarePlatformType_iPhone;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_3])    return DeviceHardwarePlatformType_iPhone;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_4])    return DeviceHardwarePlatformType_iPhone;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_6_1])    return DeviceHardwarePlatformType_iPhone;
	if ([platform isEqualToString:DH_MACHINE_IPHONE_6_1])    return DeviceHardwarePlatformType_iPhone;
	if ([platform isEqualToString:DH_MACHINE_IPOD_1_1])      return DeviceHardwarePlatformType_iPod_Touch;
	if ([platform isEqualToString:DH_MACHINE_IPOD_2_1])      return DeviceHardwarePlatformType_iPod_Touch;
	if ([platform isEqualToString:DH_MACHINE_IPOD_3_1])      return DeviceHardwarePlatformType_iPod_Touch;
	if ([platform isEqualToString:DH_MACHINE_IPOD_4_1])      return DeviceHardwarePlatformType_iPod_Touch;
	if ([platform isEqualToString:DH_MACHINE_IPOD_5_1])      return DeviceHardwarePlatformType_iPod_Touch;
	if ([platform isEqualToString:DH_MACHINE_IPAD_1_1])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_1])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_2])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_3])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_4])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_5])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_6])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_7])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_1])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_2])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_3])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_4])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_5])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_6])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_1])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_2])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_4])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_5])      return DeviceHardwarePlatformType_iPad;
	if ([platform isEqualToString:DH_MACHINE_I386])          return DeviceHardwarePlatformType_Simulator;
	if ([platform isEqualToString:DH_MACHINE_X86_64])        return DeviceHardwarePlatformType_Simulator;
	return DeviceHardwarePlatformType_Unknown;
}

- (NSString *)platformString {
	NSString *platform = [self machine];
	if ([platform isEqualToString:DH_MACHINE_IPHONE_1_1])    return @"iPhone 1G";
	if ([platform isEqualToString:DH_MACHINE_IPHONE_1_2])    return @"iPhone 3G";
	if ([platform isEqualToString:DH_MACHINE_IPHONE_2_1])    return @"iPhone 3GS";
	if ([platform isEqualToString:DH_MACHINE_IPHONE_3_1])    return @"iPhone 4";
	if ([platform isEqualToString:DH_MACHINE_IPHONE_3_3])    return @"iPhone 4 (Verizon)";
	if ([platform isEqualToString:DH_MACHINE_IPHONE_4_1])    return @"iPhone 4S";
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_1])    return @"iPhone 5 (GSM)";
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_2])    return @"iPhone 5 (CDMA+GSM)";
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_3])    return @"iPhone 5C (CDMA+GSM)";
	if ([platform isEqualToString:DH_MACHINE_IPHONE_5_4])    return @"iPhone 5C (Global)";
	if ([platform isEqualToString:DH_MACHINE_IPHONE_6_1])    return @"iPhone 5S (CDMA+GSM)";
	if ([platform isEqualToString:DH_MACHINE_IPHONE_6_2])    return @"iPhone 5S (Global)";
	if ([platform isEqualToString:DH_MACHINE_IPOD_1_1])      return @"iPod Touch 1G";
	if ([platform isEqualToString:DH_MACHINE_IPOD_2_1])      return @"iPod Touch 2G";
	if ([platform isEqualToString:DH_MACHINE_IPOD_3_1])      return @"iPod Touch 3G";
	if ([platform isEqualToString:DH_MACHINE_IPOD_4_1])      return @"iPod Touch 4G";
	if ([platform isEqualToString:DH_MACHINE_IPOD_5_1])      return @"iPod Touch 5G";
	if ([platform isEqualToString:DH_MACHINE_IPAD_1_1])      return @"iPad";
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_1])      return @"iPad 2 (WiFi)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_2])      return @"iPad 2 (GSM)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_3])      return @"iPad 2 (CDMA)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_4])      return @"iPad 2 (WiFi)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_5])      return @"iPad Mini (WiFi)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_6])      return @"iPad Mini (GSM)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_2_7])      return @"iPad Mini (CDMA+GSM)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_1])      return @"iPad 3 (WiFi)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_2])      return @"iPad 3 (CDMA)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_3])      return @"iPad 3 (GSM)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_4])      return @"iPad 4 (WiFi)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_5])      return @"iPad 4 (GSM)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_3_6])      return @"iPad 4 (CDMA+GSM)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_1])      return @"iPad Air (WiFi)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_2])      return @"iPad Air (CDMA+GSM)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_4])      return @"iPad Mini Retina (WiFi)";
	if ([platform isEqualToString:DH_MACHINE_IPAD_4_5])      return @"iPad Mini Retina (CDMA+GSM)";
	if ([platform isEqualToString:DH_MACHINE_I386])          return @"Simulator";
	if ([platform isEqualToString:DH_MACHINE_X86_64])        return @"Simulator";
	return platform;
}

- (NSString *)machine {
	size_t size;
	sysctlbyname("hw.machine", NULL, &size, NULL, 0);
	char *machine = malloc(size);
	sysctlbyname("hw.machine", machine, &size, NULL, 0);
	NSString *platform = [NSString stringWithUTF8String:machine];
	free(machine);
	return platform;
}



- (NSString *) osVersion
{
    return [[UIDevice currentDevice].systemName stringByAppendingString:[UIDevice currentDevice].systemVersion];
}

#pragma mark sysctl utils
- (NSUInteger) getSysInfo: (uint) typeSpecifier
{
    size_t size = sizeof(int);
    int results;
    int mib[2] = {CTL_HW, typeSpecifier};
    sysctl(mib, 2, &results, &size, NULL, 0);
    return (NSUInteger) results;
}

#pragma mark battery information
- (NSString*) isCharging
{
    UIDeviceBatteryState deviceBatteryState =[UIDevice currentDevice].batteryState;
    BOOL isCharging = (deviceBatteryState == UIDeviceBatteryStateCharging || deviceBatteryState == UIDeviceBatteryStateFull);
    return isCharging?@"YES":@"NO";
}

- (NSString*) batteryLevelString
{
    UIDevice *currentDevice = [UIDevice currentDevice];
    NSString *batteryLevelString = nil;
#if APP_STORE != 1
    batteryLevelString = IOKitBatteryLevelString();
#endif
    if (batteryLevelString == nil)
    {
        int batteryLevel = currentDevice.batteryLevel*100;
        batteryLevelString = (currentDevice.batteryLevel >= 0) ? [NSString stringWithFormat:@"%d", batteryLevel] : @"NA";
    }
    return batteryLevelString;
}

#pragma mark cpu information
- (NSString *) cpuType
{
    switch ([self generalPlatform])
    {
        case DeviceHardwareGeneralPlatform_iPhone_4S:   return IPHONE_4S_CPUTYPE;
        case DeviceHardwareGeneralPlatform_iPhone_5:    return IPHONE_5_CPUTYPE;
        case DeviceHardwareGeneralPlatform_iPhone_5C:   return IPHONE_5_CPUTYPE;
        case DeviceHardwareGeneralPlatform_iPhone_5S:   return IPHONE_5S_CPUTYPE;
        case DeviceHardwareGeneralPlatform_iPad_2:      return IPAD_2_CPUTYPE;
        case DeviceHardwareGeneralPlatform_iPad_Mini:   return IPAD_2_CPUTYPE;
        case DeviceHardwareGeneralPlatform_iPad_3:      return IPAD_3_CPUTYPE;
        case DeviceHardwareGeneralPlatform_iPad_4:      return IPAD_4_CPUTYPE;
        case DeviceHardwareGeneralPlatform_iPad_Air:    return IPAD_AIR_CPUTYPE;
        case DeviceHardwareGeneralPlatform_iPad_Mini_Retina: return IPAD_AIR_CPUTYPE;
        default: return IOS_CPUTYPE_UNKNOWN;
    }
}



- (NSString *)cpuUsage
{
    kern_return_t kr;
    task_info_data_t tinfo;
    mach_msg_type_number_t task_info_count;
    
    task_info_count = TASK_INFO_MAX;
    kr = task_info(mach_task_self(), TASK_BASIC_INFO, (task_info_t)tinfo, &task_info_count);
    if (kr != KERN_SUCCESS) {
        return @"Get CPU Usage Error";
    }
    
    task_basic_info_t      basic_info;
    thread_array_t         thread_list;
    mach_msg_type_number_t thread_count;
    
    thread_info_data_t     thinfo;
    mach_msg_type_number_t thread_info_count;
    
    thread_basic_info_t basic_info_th;
    uint32_t stat_thread = 0; // Mach threads
    
    basic_info = (task_basic_info_t)tinfo;
    
    // get threads in the task
    kr = task_threads(mach_task_self(), &thread_list, &thread_count);
    if (kr != KERN_SUCCESS) {
        return @"Get CPU Usage Error";
    }
    if (thread_count > 0)
        stat_thread += thread_count;
    
    long tot_sec = 0;
    long tot_usec = 0;
    float tot_cpu = 0;
    int j;
    
    for (j = 0; j < thread_count; j++)
    {
        thread_info_count = THREAD_INFO_MAX;
        kr = thread_info(thread_list[j], THREAD_BASIC_INFO,
                         (thread_info_t)thinfo, &thread_info_count);
        if (kr != KERN_SUCCESS) {
            return @"Get CPU Usage Error";
        }
        
        basic_info_th = (thread_basic_info_t)thinfo;
        
        if (!(basic_info_th->flags & TH_FLAGS_IDLE)) {
            tot_sec = tot_sec + basic_info_th->user_time.seconds + basic_info_th->system_time.seconds;
            tot_usec = tot_usec + basic_info_th->system_time.microseconds + basic_info_th->system_time.microseconds;
            tot_cpu = tot_cpu + basic_info_th->cpu_usage / (float)TH_USAGE_SCALE * 100.0;
        }
        
    } // for each thread
    
    kr = vm_deallocate(mach_task_self(), (vm_offset_t)thread_list, thread_count * sizeof(thread_t));
    assert(kr == KERN_SUCCESS);
    int cpu_usage = tot_cpu;
    return [NSString stringWithFormat:@"%d", cpu_usage];
}

#pragma mark memory information
- (NSUInteger) totalMemoryBytes
{
    return [self getSysInfo:HW_PHYSMEM];
}

- (NSUInteger) freeMemoryBytes 
{
    mach_port_t           host_port = mach_host_self();
    mach_msg_type_number_t   host_size = sizeof(vm_statistics_data_t) / sizeof(integer_t);
    vm_size_t               pagesize;
    vm_statistics_data_t     vm_stat;
    
    host_page_size(host_port, &pagesize);
    
    if (host_statistics(host_port, HOST_VM_INFO, (host_info_t)&vm_stat, &host_size) != KERN_SUCCESS) NSLog(@"Failed to fetch vm statistics");
    
    natural_t   mem_free = vm_stat.free_count * pagesize;
    
    return mem_free;
}


@end