//
//  ProxyAuthenticationDialog.h
//  MediaSessionTest
//
//  Created by Wilson Chen on 9/7/15.
//  Copyright (c) 2015 juntang. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface ProxyAuthenticationDialog : NSWindowController {
    NSTextField *_labelProxyInfo;
}

@property (assign) IBOutlet NSTextField *inputUserName;
@property (assign) IBOutlet NSSecureTextField *inputPassword;
@property (weak) IBOutlet NSTextField *labelProxyInfo;

- (const char*)getUsername;
- (const char*)getPassword;
- (void)setDescription: (const char*)szProxy port:(int)nPort realm:(const char*)szRealm;
- (const char*)getProxyAddr;
- (int)getProxyPort;

@end
