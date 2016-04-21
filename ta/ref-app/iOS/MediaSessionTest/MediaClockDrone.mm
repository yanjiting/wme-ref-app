#import "MediaClockDrone.h"

#import <mach/mach.h>
#import "sys/sysctl.h"

@interface MediaClockDrone ()
@end

const int FakeLabelCount = 30;

@implementation MediaClockDrone {
    UILabel *_fakeLabel[FakeLabelCount];
    NSTimer *_updateFakeLabel;
}

- (BOOL) platformType:(NSString *)platform
{
    static bool bSkip=getenv("WME_PERFORMANCE_TEST_ios_clock_no_prompt")?true:false;
    if(bSkip)
    {
        NSLog(@"skip clock drone");
        return NO;
    }
    NSLog(@"do clock drone");
    if ([platform isEqualToString:@"iPhone1,1"])    return NO;
    if ([platform isEqualToString:@"iPhone1,2"])    return NO;
    if ([platform isEqualToString:@"iPhone2,1"])    return NO;
    if ([platform isEqualToString:@"iPhone3,1"])    return NO;
    if ([platform isEqualToString:@"iPhone3,3"])    return NO;
    if ([platform isEqualToString:@"iPhone4,1"])    return NO;
    if ([platform isEqualToString:@"iPhone5,1"])    return NO;
    if ([platform isEqualToString:@"iPhone5,2"])    return NO;
    if ([platform isEqualToString:@"iPhone5,3"])    return NO;
    if ([platform isEqualToString:@"iPhone5,4"])    return NO;
    return YES;
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    size_t size;
    sysctlbyname("hw.machine", NULL, &size, NULL, 0);
    char *machine = new char[size];
    sysctlbyname("hw.machine", machine, &size, NULL, 0);
    NSString *platform = [NSString stringWithCString:machine encoding:NSUTF8StringEncoding];
    if(![self platformType:platform])
        return self;
    
    delete [] (machine);
    if (self) {
        self.alpha = 0.01;

        for(int i = 0; i < FakeLabelCount; i++) {
            _fakeLabel[i] = [[UILabel alloc] initWithFrame:CGRectMake(0,0, 100 + i*10, 10)];
            [self addSubview: _fakeLabel[i]];
            [_fakeLabel[i] setAlpha: 0.01];
        }
        _updateFakeLabel = [NSTimer scheduledTimerWithTimeInterval:1 target:self selector:@selector(updateFakeLabelText) userInfo:nil repeats:YES];
    }
    return self;
}


- (void)updateFakeLabelText {
    static int nIdx = 0;
    nIdx++;
    NSString *txt = [NSString stringWithFormat:@"test set: %d", nIdx];
    for(int i = 0; i < FakeLabelCount; i++) {
        [_fakeLabel[i] setText: txt];
    }
}

- (void)dealloc {
}

@end