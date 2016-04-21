//
//  ProxyAuthenticationDialog.m
//  MediaSessionTest
//
//  Created by Wilson Chen on 9/7/15.
//  Copyright (c) 2015 juntang. All rights reserved.
//

#import "ProxyAuthenticationDialog.h"
#include <string>

@interface ProxyAuthenticationDialog ()

@end

@implementation ProxyAuthenticationDialog
{
    std::string m_sProxyServer;
    std::string m_sUserName;
    std::string m_sPassword;
    std::string m_sRealm;
    int m_nPort;
}

@synthesize labelProxyInfo = _labelProxyInfo;

- (void)windowDidLoad {
    [super windowDidLoad];
    NSString *proxyInfo = [NSString stringWithFormat:@"%@:%d realm=%@", [NSString stringWithUTF8String: m_sProxyServer.c_str()],
                           m_nPort, [NSString stringWithUTF8String: m_sRealm.c_str()]];
    [_labelProxyInfo setStringValue:proxyInfo];
}

- (IBAction)didTapCancelButton:(id)sender {
    [self.window.sheetParent endSheet:self.window returnCode:NSModalResponseCancel];
}

- (IBAction)didTapDoneButton:(id)sender {
    m_sUserName = [[self.inputUserName stringValue] UTF8String];
    m_sPassword = [[self.inputPassword stringValue] UTF8String];
    [self.window.sheetParent endSheet:self.window returnCode:NSModalResponseOK];
}

- (void)setDescription: (const char*)szProxy port:(int)nPort realm:(const char*)szRealm{
    m_sProxyServer = szProxy;
    m_nPort = nPort;
    m_sRealm = szRealm;
}

- (const char*)getProxyAddr {
    return m_sProxyServer.c_str();
}

- (int)getProxyPort {
    return m_nPort;
}

- (const char*)getUsername {
    return m_sUserName.c_str();
}

- (const char*)getPassword {
    return m_sPassword.c_str();
}

@end
