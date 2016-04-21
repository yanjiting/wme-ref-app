//
//  WMEPreMeetingSettingViewController.m
//  MediaEngineTestApp
//
//  Created by rory on 1/6/14.
//  Copyright (c) 2014 video. All rights reserved.
//

#import "WMEPreMeetingSettingViewController.h"
#import "WMEDataProcess.h"

@interface WMEPreMeetingSettingViewController ()

@end

@implementation WMEPreMeetingSettingViewController
{
    NSString* inputVideoFile;
    NSString* inputAudioFile;
    NSString* outputVideoFile;
    NSString* outputAudioFile;
    BOOL      enableVideo;
    BOOL      enableAudio;
    BOOL      enableContentShare;
    BOOL bDumpCaptureDataEnable;
    BOOL bDumpEncodeToRTPDataEnable;
    BOOL bDumpNALToListenChannelDataEnable;
    BOOL bDumpNALToDecodeDataEnable;
    BOOL bDumpAfterDecodeDataEnable;
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
    enableAudio = YES;
    enableVideo = YES;
    enableContentShare = YES;
}

- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
     WMEDataProcess* dataProcess = [WMEDataProcess instance];
    bDumpCaptureDataEnable = dataProcess.bDumpCaptureDataEnable;
    bDumpEncodeToRTPDataEnable = dataProcess.bDumpEncodeToRTPDataEnable;
    bDumpNALToListenChannelDataEnable = dataProcess.bDumpNALToListenChannelDataEnable;
    bDumpNALToDecodeDataEnable = dataProcess.bDumpNALToDecodeDataEnable;
    bDumpAfterDecodeDataEnable = dataProcess.bDumpAfterDecodeDataEnable;
    [self.dumpCaptureDataSWT setOn:bDumpCaptureDataEnable animated:NO];
    [self.dumpEncodeToRTPDataSWT setOn:bDumpEncodeToRTPDataEnable animated:NO];
    [self.dumpNALToListenChannelDataSWT setOn:bDumpNALToListenChannelDataEnable animated:NO];
    [self.dumpNALToDecodeDataSWT setOn:bDumpNALToDecodeDataEnable animated:NO];
    [self.dumpAfterDecodeDataSWT setOn:bDumpAfterDecodeDataEnable animated:NO];

    
}
- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (IBAction)SwitchInPreMeetingSettings:(id)sender
{
    if ([sender tag] == 0) {
        enableVideo = [self.videoSwitch isOn];
    }else if([sender tag] == 1){
        enableAudio = [self.audioSwitch isOn];
    }
    else if( sender == self.contentShareSwitch )
    {
        enableContentShare = [self.contentShareSwitch isOn];
    }
}

- (IBAction)DataDumpEnableSwitch:(id)sender {
    if ([sender tag] == 2) {
        bDumpCaptureDataEnable = [self.dumpCaptureDataSWT isOn];
    }
    else if ([sender tag] == 3)
    {
        bDumpEncodeToRTPDataEnable = [self.dumpEncodeToRTPDataSWT isOn];
    }
    else if ([sender tag] == 4)
    {
        bDumpNALToListenChannelDataEnable = [self.dumpNALToListenChannelDataSWT isOn];
    }
    else if ([sender tag] == 5)
    {
        bDumpNALToDecodeDataEnable = [self.dumpNALToDecodeDataSWT isOn];
    }
    else if ([sender tag] == 6)
    {
        bDumpAfterDecodeDataEnable = [self.dumpAfterDecodeDataSWT isOn];
    }
    
}

- (BOOL)ValidateFileNames
{
    return YES;
}

-(void)setPreMeetingShareSettings
{
    WMEDataProcess* dataProcess = [WMEDataProcess instance];
    dataProcess.bContentSharing = self.contentShareSwitch.on;
    
}

- (void)setPreMeetingSettings
{
    [self setPreMeetingVideoFileSettings];
    [self setPreMeetingAudioFileSettings];
    [self setDumpFileSettings];
    [self setPreMeetingShareSettings];
}

- (void)setDumpFileSettings
{
    WMEDataProcess* dataProcess = [WMEDataProcess instance];
    dataProcess.bDumpCaptureDataEnable = bDumpCaptureDataEnable;
    dataProcess.bDumpEncodeToRTPDataEnable = bDumpEncodeToRTPDataEnable;
    dataProcess.bDumpNALToListenChannelDataEnable = bDumpNALToListenChannelDataEnable;
    dataProcess.bDumpNALToDecodeDataEnable = bDumpNALToDecodeDataEnable;
    dataProcess.bDumpAfterDecodeDataEnable = bDumpAfterDecodeDataEnable;
    dataProcess.bDumpFlag = YES; //For triggling the KVO

}

-(void) setPreMeetingVideoFileSettings
{
    WMEDataProcess* dataProcess = [WMEDataProcess instance];
    dataProcess.bVideoModuleEnable = self.videoSwitch.on;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *sharingFolderPath = [paths objectAtIndex:0];
    if ([outputVideoFile isEqualToString:@""] == NO) {
        dataProcess.bEnableExternalVideoOutput = YES;
        dataProcess.sVOutputFilename = [sharingFolderPath stringByAppendingPathComponent:outputVideoFile];
    }else{
        dataProcess.bEnableExternalVideoOutput = NO;
    }
    if (inputVideoFile != nil) {
        NSArray* foo = [[inputVideoFile lastPathComponent] componentsSeparatedByString: @"_"];
        if([foo count] != 4)
        {
            dataProcess.bEnableExternalVideoInput = NO;
            return;
        }
        NSInteger Width = [[foo objectAtIndex: 0] integerValue];
        NSInteger Height = [[foo objectAtIndex:1] integerValue];
        NSInteger FPS = [[foo objectAtIndex:2] integerValue];
        if (Width ==0 || Height ==0 || FPS == 0) {
            dataProcess.bEnableExternalVideoInput = NO;
            return;
        }
        dataProcess.bEnableExternalVideoInput = YES;
        dataProcess.iWidth = Width;
        dataProcess.iHeight = Height;
        dataProcess.iFPS = FPS;
        if ([[inputVideoFile lastPathComponent] hasSuffix:@"yuv"]) {
            dataProcess.iColorFormat = WmeI420;
        }else if([[inputVideoFile lastPathComponent] hasSuffix:@"rgb"]){
            dataProcess.iColorFormat = WmeRGBA32;
        }
        dataProcess.sVInputFilename = inputVideoFile;
    }else{
        dataProcess.bEnableExternalVideoInput = NO;
    }

}

-(void) setPreMeetingAudioFileSettings
{
    WMEDataProcess* dataProcess = [WMEDataProcess instance];
    dataProcess.bAudioModuleEnable = self.audioSwitch.on;
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *sharingFolderPath = [paths objectAtIndex:0];
    if ([outputAudioFile isEqualToString:@""] == NO) {
        dataProcess.bEnableExternalAudioOutput = YES;
        dataProcess.sAOutputFilename = [sharingFolderPath stringByAppendingPathComponent:outputAudioFile];
    }else{
        dataProcess.bEnableExternalAudioOutput = NO;
    }
    if (inputAudioFile != nil) {
        NSArray* foo = [[inputAudioFile lastPathComponent] componentsSeparatedByString:@"_"];
        if ([foo count] != 4)
        {
            dataProcess.bEnableExternalAudioInput = NO;
            return;
        }
        NSInteger Channels = [[foo objectAtIndex:0] integerValue];
        NSInteger SampleRate = [[foo objectAtIndex:1] integerValue];
        NSInteger BitsPerSample = [[foo objectAtIndex:2] integerValue];
        if (Channels == 0 || SampleRate == 0 || BitsPerSample == 0) {
            dataProcess.bEnableExternalAudioInput = NO;
            return;
        }
        dataProcess.bEnableExternalAudioInput = YES;
        dataProcess.iChannels = Channels;
        dataProcess.iSampleRate = SampleRate;
        dataProcess.iBitsPerSample = BitsPerSample;
        dataProcess.sAInputFilename = inputAudioFile;

    }else{
        dataProcess.bEnableExternalAudioInput = NO;
    }
}

- (IBAction)FinishPreMeetingSetting:(id)sender {
    outputVideoFile = self.outputVideoFileName.text;
    outputAudioFile = self.outputAudioFileName.text;
    [self setPreMeetingSettings];
    [self.delegate DidFinishPreMeetingSetting];
    
}
- (IBAction)CanclePreMeetingSetting:(id)sender {
    [self.delegate DidFinishPreMeetingSetting];
}

- (void) DidSelectRawFile:(NSString *)rawFileName VideoOrAudio:(FileType)fileType
{
    if (fileType == VideoFile && rawFileName != nil) {
        inputVideoFile = [NSString stringWithString:rawFileName];
        self.videoFileName.text = [inputVideoFile lastPathComponent];
    }else if(fileType == AudioFile && rawFileName != nil)
    {
        inputAudioFile = [NSString stringWithString:rawFileName];
        self.audioFileName.text = [inputAudioFile lastPathComponent];
    }
    [self.navigationController popViewControllerAnimated:YES];
}

#pragma mark - Table view data source
/*
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
#warning Potentially incomplete method implementation.
    // Return the number of sections.
    return 0;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
#warning Incomplete method implementation.
    // Return the number of rows in the section.
    return 0;
}
*/
/*
- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:<#@"reuseIdentifier"#> forIndexPath:indexPath];
    
    // Configure the cell...
    
    return cell;
}
*/

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


#pragma mark - Navigation

// In a storyboard-based application, you will often want to do a little preparation before navigation
- (void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    WMERawFilesViewController* rawFilesViewController = [segue destinationViewController];
    // Get the new view controller using [segue destinationViewController].
    // Pass the selected object to the new view controller.
    if ([segue.identifier isEqualToString:@"VideoFileExplorer"]) {
        rawFilesViewController.fileTypeInfo = VideoFile;
    }else if([segue.identifier isEqualToString:@"AudioFileExplorer"])
    {
        rawFilesViewController.fileTypeInfo = AudioFile;
    }
    rawFilesViewController.delegate = self;
    
}


@end
