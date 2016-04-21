//
//  TAVersionRespond.m
//  MediaSessionTest
//
//  Created by juntang on 6/20/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#import "TAVersionRespond.h"
#import <sys/utsname.h>

@implementation TAVersionRespond

- (BOOL) supportsMethod:(NSString *) method atPath:(NSString *) path {
    return [method isEqualToString:@"GET"];
}

- (NSDictionary *) JSONResponseForMethod:(NSString *) method URI:(NSString *) path data:(NSDictionary *) data
{
    
    NSString *calabashVersion = [kLPCALABASHVERSION componentsSeparatedByString:@" "].lastObject;
    
    NSString *versionString = [[NSBundle mainBundle]
                               objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
    if (!versionString) {
        versionString = @"Unknown";
    }
    
    NSString *idString = [[NSBundle mainBundle]
                          objectForInfoDictionaryKey:@"CFBundleIdentifier"];
    
    if (!idString) { idString = @"Unknown";  }
    
    SInt32 major, minor, bugfix;
    Gestalt(gestaltSystemVersionMajor, &major);
    Gestalt(gestaltSystemVersionMinor, &minor);
    Gestalt(gestaltSystemVersionBugFix, &bugfix);
    
    NSString *systemVersion = [NSString stringWithFormat:@"%d.%d.%d",
                               (int)major, (int)minor, (int)bugfix];
    
    NSString *nameString = [[NSBundle mainBundle]
                            objectForInfoDictionaryKey:@"CFBundleDisplayName"];
    
    if (!nameString) { nameString = @"Unknown";  }
    
    struct utsname systemInfo;
    uname(&systemInfo);
    
    NSString *machine = @(systemInfo.machine);
    
    NSDictionary *res = @{@"version": calabashVersion,
                          @"app_id": idString,
                          @"OS_version": systemVersion,
                          @"app_name": nameString,
                          @"system": machine,
                          @"app_version": versionString,
                          @"outcome": @"SUCCESS"};
    
    return res;

}

@end
