//
//  WMERawFilesViewController.h
//  MediaEngineTestApp
//
//  Created by rory on 3/6/14.
//  Copyright (c) 2014 video. All rights reserved.
//

#import <UIKit/UIKit.h>
typedef enum
{
    VideoFile,
    AudioFile
    
}FileType;

@protocol SelectRawFile <NSObject>

- (void) DidSelectRawFile:(NSString*)rawFileName VideoOrAudio:(FileType)fileType;

@end

@interface WMERawFilesViewController : UITableViewController
@property FileType fileTypeInfo;
@property(weak, nonatomic) id<SelectRawFile>delegate;
@end
