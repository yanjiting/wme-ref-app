//
//  WMERawFilesViewController.m
//  MediaEngineTestApp
//
//  Created by rory on 3/6/14.
//  Copyright (c) 2014 video. All rights reserved.
//

#import "WMERawFilesViewController.h"

@interface WMERawFilesViewController ()

@end

@implementation WMERawFilesViewController
{
    NSMutableArray *resVideoFileArray;
    NSMutableArray *resAudioFileArray;
}

- (id)initWithStyle:(UITableViewStyle)style
{
    self = [super initWithStyle:style];
    if (self) {
        // Custom initialization
    }
    return self;
}

- (void)viewDidLoad
{
    [super viewDidLoad];
    
    // Uncomment the following line to preserve selection between presentations.
    // self.clearsSelectionOnViewWillAppear = NO;
    
    // Uncomment the following line to display an Edit button in the navigation bar for this view controller.
    // self.navigationItem.rightBarButtonItem = self.editButtonItem;
    resVideoFileArray = [[NSMutableArray alloc] init];
    resAudioFileArray = [[NSMutableArray alloc] init];
    [self updateVideoResourceArray];
    [self updateAudeoResourceArray];
    
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (void) updateVideoResourceArray
{
    if ([resVideoFileArray count] > 0) {
        [resVideoFileArray removeAllObjects];
    }
    //get the sharing folder path
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *sharingFolderPath = [paths objectAtIndex:0];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    NSError *error;
    NSArray * directoryContents = [fileManager contentsOfDirectoryAtPath:sharingFolderPath error:&error];
    for (NSUInteger index=0; index < [directoryContents count]; index++) {
        NSString *fileName = [directoryContents objectAtIndex:index];
        if (([fileName hasSuffix:@"yuv"] == YES) || ([fileName hasSuffix:@"rgb"] == YES))
        {
            [resVideoFileArray addObject:[sharingFolderPath stringByAppendingPathComponent:fileName]];
        }
    }
}

- (void) updateAudeoResourceArray
{
    if ([resAudioFileArray count] > 0) {
        [resAudioFileArray removeAllObjects];
    }
    //get the sharing folder path
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *sharingFolderPath = [paths objectAtIndex:0];
    NSFileManager *fileManager = [NSFileManager defaultManager];
    
    NSError *error;
    NSArray * directoryContents = [fileManager contentsOfDirectoryAtPath:sharingFolderPath error:&error];
    for (NSUInteger index=0; index < [directoryContents count]; index++) {
        NSString *fileName = [directoryContents objectAtIndex:index];
        if (([fileName hasSuffix:@"pcm"] == YES))
        {
            [resAudioFileArray addObject:[sharingFolderPath stringByAppendingPathComponent:fileName]];
        }
    }
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
    if (self.fileTypeInfo == VideoFile) {
        return [resVideoFileArray count];
    }else if(self.fileTypeInfo == AudioFile)
    {
        return [resAudioFileArray count];
    }else
    {
        return 0;
    }
}


- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:@"RawFileCell" forIndexPath:indexPath];
    
    // Configure the cell...
    if (self.fileTypeInfo == VideoFile) {
        cell.textLabel.text = [[resVideoFileArray objectAtIndex:indexPath.row] lastPathComponent];
    }else if(self.fileTypeInfo == AudioFile)
    {
        cell.textLabel.text = [[resAudioFileArray objectAtIndex:indexPath.row] lastPathComponent];
    }
    return cell;
}


- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    NSString* fileName = nil;
    // Navigation logic may go here. Create and push another view controller.
    if (self.fileTypeInfo == VideoFile) {
        fileName = [resVideoFileArray objectAtIndex:indexPath.row];
    }else if(self.fileTypeInfo == AudioFile)
    {
        fileName = [resAudioFileArray objectAtIndex:indexPath.row];
    }
    [self.delegate DidSelectRawFile:fileName VideoOrAudio:self.fileTypeInfo];
}


/*
// Override to support conditional editing of the table view.
- (BOOL)tableView:(UITableView *)tableView canEditRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the specified item to be editable.
    return YES;
}
*/

/*
// Override to support editing the table view.
- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath
{
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete the row from the data source
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationFade];
    } else if (editingStyle == UITableViewCellEditingStyleInsert) {
        // Create a new instance of the appropriate class, insert it into the array, and add a new row to the table view
    }   
}
*/

/*
// Override to support rearranging the table view.
- (void)tableView:(UITableView *)tableView moveRowAtIndexPath:(NSIndexPath *)fromIndexPath toIndexPath:(NSIndexPath *)toIndexPath
{
}
*/

/*
// Override to support conditional rearranging of the table view.
- (BOOL)tableView:(UITableView *)tableView canMoveRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Return NO if you do not want the item to be re-orderable.
    return YES;
}
*/

/*
#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
}
*/

@end
