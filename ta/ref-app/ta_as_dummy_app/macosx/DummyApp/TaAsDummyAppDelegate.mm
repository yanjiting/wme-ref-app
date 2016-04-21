//
//  TaAsDummyAppDelegate.m
//  DummyApp
//
//  Created by vagouzhou on 8/11/14.
//  Copyright (c) 2014 wx2. All rights reserved.
//

#import "TaAsDummyAppDelegate.h"
#include "DummyBackdoorAgent.h"
#include "DummyDbg.h"
#include "QR_encode_mac.h"

extern CDummyBackdoorAgent *g_DummyBackdoorAgent ;

void ShowQRCodeView(int x, int y, int w, int h, int row, int column, std::vector<std::string>& v_qr_contents);

@implementation TaAsDummyAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    //Init TA Backdoor
    if (g_DummyBackdoorAgent &&
        DummyConfig::Instance().m_strShowQRCode.size() > 0)
        g_DummyBackdoorAgent->backdoorShowQRCodeView(DummyConfig::Instance().m_strShowQRCode);
    
//    //Create windows without frame.
//    if(fullscreenWindow==nil){
//        fullscreenWindow = [[NSWindow alloc]
//                            initWithContentRect:[[self window] contentRectForFrameRect:[[self window] frame]]
//                            styleMask:NSBorderlessWindowMask
//                            backing:NSBackingStoreBuffered
//                            defer:YES
//                            screen:[[self window] screen]];
//        
//        NSView *contentView = [[self window] contentView] ;
//        [[self window] setContentView:[[NSView alloc] init] ];
//        [fullscreenWindow setContentView:contentView];
//        /*
//        [fullscreenWindow setHidesOnDeactivate:NO];
//        [fullscreenWindow setLevel:NSFloatingWindowLevel];
//        [fullscreenWindow setTitle:[[self window] title]];
//        [fullscreenWindow makeKeyAndOrderFront:nil];
//         */
//    }
    
    //
//    [self.window toggleFullScreen:self];
//    CGRect rcWindow = {{0., 0.}, {0., 0.}};
//    [[self window] setFrame:rcWindow display:NO];
    _fullScreen = false;
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(willEnterFull:)
                                                 name:NSWindowWillEnterFullScreenNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(didExitFull:)
                                                 name:NSWindowDidExitFullScreenNotification
                                               object:nil];
}

// -------------------------------------------------------------------------------
//  didEnterFull:notif
// -------------------------------------------------------------------------------
- (void)didExitFull:(NSNotification *)notif
{
    _fullScreen = false;
}

// -------------------------------------------------------------------------------
//  willEnterFull:notif
// -------------------------------------------------------------------------------
- (void)willEnterFull:(NSNotification *)notif
{
    _fullScreen = true;
}

- (void)UpdateWindowLocation:(NSValue *)newRect
{
    NSRect rc = [newRect rectValue];
//    //hide menubar/dock
//    [NSMenu setMenuBarVisible:NO];
//    
//    //
//    [fullscreenWindow setHidesOnDeactivate:NO];
//    [fullscreenWindow setLevel:NSFloatingWindowLevel];
//    [fullscreenWindow setContentView:[self imgView]];
//    [fullscreenWindow setFrame:rc display:YES];
//    [fullscreenWindow makeKeyAndOrderFront:nil];
//    [fullscreenWindow becomeKeyWindow];
    NSRect rcScreen = self.window.screen.frame;
    if( CGRectEqualToRect(rcScreen, rc) )
    {
        if( !_fullScreen )
            [self.window toggleFullScreen:self];
    }
    else {
        if( _fullScreen )
            [self.window toggleFullScreen:self];
        [self.window setFrame:rc display:YES];
    }
}
@end



void ShowQRCodeView(int x, int y, int w, int h, int row, int column, std::vector<std::string>& v_qr_contents)
{
    NSScreen *mainScreen = [NSScreen mainScreen];
	if (w <= 0)
        w = (int)mainScreen.frame.size.width;
	if (h <= 0)
        h = (int)mainScreen.frame.size.height;
    
    //Create QRCode NSImage
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    int bytesPerRow = w*4;
    CGColorSpaceRef deviceRGB = CGColorSpaceCreateDeviceRGB();
    CGContextRef fullImage = CGBitmapContextCreate(NULL, w, h, 8, bytesPerRow, deviceRGB, kCGImageAlphaNoneSkipLast);
    CGFloat whiteColor[] = {1.0, 1.0, 1.0, 1.0};
    CGContextSetFillColor(fullImage, whiteColor);
    CGContextSetStrokeColor(fullImage, whiteColor);
    CGRect rcTotal = {{0., 0.}, {static_cast<CGFloat>(w), static_cast<CGFloat>(h)}};
    CGRect rc = rcTotal;
    CGAffineTransform transform = { 1., 0., 0, -1, 0, static_cast<CGFloat>(h)};
    CGContextConcatCTM(fullImage, transform);
    CGContextFillRect(fullImage, rc);
    
    int wPerImg = w / column;
    int hPerImg = h / row;
    CQR_Encode_mac encoder;
    
    rc.size.width = rc.size.height = std::min(wPerImg,hPerImg);
    
    CGFloat delta = rc.size.width * 0.1;
    rc.size.width -= delta;
    rc.size.height -= delta;
    
    CGContextSetInterpolationQuality(fullImage, kCGInterpolationNone);
    for( int i=0; i<row; i++ )
    {
        rc.origin.y = hPerImg * i + (hPerImg-rc.size.height)/2;
        for( int j=0; j<column; j++ )
        {
            if (encoder.EncodeData(QRCODE_DEFAULT_LEVEL, QRCODE_DEFAULT_VERSION,
                                   true, QRCODE_DEFAULT_MASKINGNO,
                                   v_qr_contents[i*column+j].c_str()))
            {
                rc.origin.x = wPerImg * j + (wPerImg-rc.size.width)/2;
                CGContextDrawImage(fullImage, rc, encoder.img);
            }
        }
    }
    CGContextFlush(fullImage);
    
    CGImageRef cgImg = CGBitmapContextCreateImage(fullImage);
    NSImage *theNewImage = [[NSImage alloc] initWithCGImage:cgImg size:NSZeroSize];
    CGImageRelease(cgImg);
    CGContextRelease(fullImage);
    CGColorSpaceRelease(deviceRGB);
    
    
    //Show QRCode view window
    //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    NSApplication *theApp = [NSApplication sharedApplication];
    TaAsDummyAppDelegate *appDele = theApp.delegate;
    appDele.imgView.image = theNewImage;
    rcTotal.origin.y = mainScreen.frame.size.height - rcTotal.size.height;
    [appDele performSelector:@selector(UpdateWindowLocation:) withObject:[NSValue valueWithRect:rcTotal] afterDelay:0.1];
}

void HideQRCodeView()
{
    NSApplication *theApp = [NSApplication sharedApplication];
    TaAsDummyAppDelegate *appDele = theApp.delegate;
    [appDele.window setFrame: appDele.window.frame display:NO];
    [NSMenu setMenuBarVisible:NO];
    
}

