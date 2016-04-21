//
//  WMERenderView.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-5-28.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import "WMERenderView.h"
#import <QuartzCore/CAEAGLLayer.h>
@implementation WMERenderView

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code
    }
    return self;
}

/*
// Only override drawRect: if you perform custom drawing.
// An empty implementation adversely affects performance during animation.
- (void)drawRect:(CGRect)rect
{
    // Drawing code
}
*/
+(Class)layerClass
{
    return [CAEAGLLayer class];
}

@end
