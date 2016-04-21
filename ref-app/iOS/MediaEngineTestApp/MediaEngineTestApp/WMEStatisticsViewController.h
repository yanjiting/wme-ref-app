//
//  WMEStatisticsViewController.h
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-9-3.
//  Copyright (c) 2013年 video. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface WMEStatisticsViewController : UITableViewController
{
    NSTimer *updateTableViewSourceTimer;
}
@property (weak, nonatomic)  NSMutableArray *arrayStatistics;
@property (weak, nonatomic)  NSMutableArray *arrayStatisticsTitle;


@end
