//
//  WMEContentSelectionViewController.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-7-14.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface WMEContentSelectionViewController : UITableViewController
{
    NSInteger selectedRow;
    NSMutableArray *arrayCapabilityList;
}
@property (weak, nonatomic)  NSMutableArray *arrayCapabilityList;
@property (nonatomic)  NSInteger selectedRow;
@end
