//
//  TAResponder.m
//  MediaSessionTest
//
//  Created by juntang on 6/19/14.
//  Copyright (c) 2014 juntang. All rights reserved.
//

#import "TAResponder.h"

#import "TAVersionRespond.h"
#import "TABackdoorRespond.h"


@implementation TAResponder

static NSMutableDictionary *responds = nil;

+ (void) initialize
{
    responds = [[NSMutableDictionary alloc] initWithCapacity:16];
    
    TAVersionRespond *verr = [TAVersionRespond new];
    [self addRespond:verr forPath:@"version"];
    [verr release];
    
    TABackdoorRespond *bd = [TABackdoorRespond new];
    [self addRespond:bd forPath:@"backdoor"];
    [bd release];
}

+ (void) addRespond:(id <TARespond>) respond forPath:(NSString *) path
{
    [responds setObject:respond forKey:path];
}


-(NSDictionary *)httpResponseForMethod:(NSString *) method URI:(NSString *) path withData: (NSString *)data
{
    NSArray *components = [path componentsSeparatedByString:@"?"];
    NSArray *pathComponents = [[components objectAtIndex:0]
                               componentsSeparatedByString:@"/"];
    NSString *lastSegment = [pathComponents lastObject];
    
    
    NSLog(@"lastSegment: %@", lastSegment);
    
    id <TARespond> respond = [responds objectForKey:lastSegment];
    
    if ([respond supportsMethod:method atPath:path]) {
        NSDictionary *params = nil;
        if ([method isEqualToString:@"GET"]) {
            params = nil;
        }
        if ([method isEqualToString:@"POST"]) {
            
            NSError *err;
            if (!data)
                return nil;
            
            params = [NSJSONSerialization JSONObjectWithData:[data dataUsingEncoding:NSUTF8StringEncoding]
                                                     options:NSJSONReadingMutableContainers
                                                    error:&err];
            if (!params) {
                NSLog(@"%@", err);
                return nil;
            }
        }

        if ([respond respondsToSelector:@selector(setParameters:)]) {
            [respond setParameters:params];
        }
         
        NSDictionary *json = [respond JSONResponseForMethod:method URI:path
                                                     data:params];
        return json;
    }
     
    return nil;
}

@end
