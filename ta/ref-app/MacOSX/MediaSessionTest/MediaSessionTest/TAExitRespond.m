//
//  TAExitRespond.m
//  MediaSessionTest
//
//  Created by juntang on 6/25/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#import "TAExitRespond.h"

@implementation TAExitRespond

- (BOOL) supportsMethod:(NSString *) method atPath:(NSString *) path {
    return [method isEqualToString:@"GET"] || [method isEqualToString:@"POST"];
}


- (NSDictionary *) JSONResponseForMethod:(NSString *) method URI:(NSString *) path data:(NSDictionary *) data {
    // Exiting the app causes the HTTP connection to shutdown immediately.
    // Clients will get an empty response and need to handle the error
    // condition accordingly.
    exit(0);
}

@end
