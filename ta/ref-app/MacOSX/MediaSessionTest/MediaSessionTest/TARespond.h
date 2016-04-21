#import <Foundation/Foundation.h>

@protocol TARespond <NSObject>

@optional
- (void) setParameters:(NSDictionary *) parameters;

//- (void) setConnection:(LPHTTPConnection *) connection;

- (BOOL) supportsMethod:(NSString *) method atPath:(NSString *) path;

- (NSDictionary *) JSONResponseForMethod:(NSString *) method URI:(NSString *) path data:(NSDictionary *) data;

@end
