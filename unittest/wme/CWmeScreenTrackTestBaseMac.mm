
#import "Cocoa/Cocoa.h"
//==============
//
void* CreateWindowX()
{
    //Need create one UI thread for it
    /* 
     [NSApplication sharedApplication];
    // Style flags
    NSUInteger windowStyle =(NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask);
    
    // Window bounds (x, y, width, height)
    NSRect windowRect = NSMakeRect(100, 100, 400, 400);
    NSWindow * window = [[NSWindow alloc] initWithContentRect:windowRect
                                                    styleMask:windowStyle
                                                      backing:NSBackingStoreBuffered
                                                        defer:NO];
    
    // This will add a simple text view to the window,
    // so we can write a test string on it.
    NSView * view = [[NSView alloc] initWithFrame:windowRect];
    [window setContentView:view];
    
    return [view layer];
     */
    return NULL;
}
void DestroyWindowX(void* window)
{
    
}


void ShowWindowX(void* window)
{
}

void LaunchOneApp() {
#ifdef MACOS
    if(![[NSWorkspace sharedWorkspace] launchApplication:@"/Applications/Safari.app"])
        NSLog(@"Path Finder failed to launch");
#endif
}