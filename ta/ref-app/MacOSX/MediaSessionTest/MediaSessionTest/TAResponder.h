//
//  TAResponder.h
//  MediaSessionTest
//
//  Created by juntang on 6/19/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "TARespond.h"

@interface TAResponder : NSObject

+ (void)addRespond:(id <TARespond>) route forPath:(NSString *) path;

- (NSDictionary *)httpResponseForMethod:(NSString *) method URI:(NSString *) path withData:(NSString *)data;

@end
