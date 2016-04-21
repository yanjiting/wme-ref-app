//
//  WMESettingViewController.m
//  MediaEngineTestApp
//
//  Created by chu zhaozheng on 13-7-14.
//  Copyright (c) 2013年 video. All rights reserved.
//
#import "WMESettingViewController.h"
#import "WMEContentSelectionViewController.h"
#import "WMEStatisticsViewController.h"
#import "WMEDataProcess.h"
#import "WMEDeviceStatus.h"

@interface WMESettingViewController ()

@end

@implementation WMESettingViewController

@synthesize selectedRow = _selectedRow;
@synthesize selectedSection = _selectedSection;
@synthesize updateStatisticsTimer = _updateStatisticsTimer;
@synthesize arrayStatistics = _arrayStatistics;
@synthesize arrayStatisticsTitle = _arrayStatisticsTitle;

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
    
    //Alloc and init the mutable array for statistics feature
    _arrayStatisticsTitle = [[NSMutableArray alloc] init];
    _arrayStatistics = [[NSMutableArray alloc] init];
    [_arrayStatistics addObject:[[NSMutableArray alloc] init]];
    [_arrayStatistics addObject:[[NSMutableArray alloc] init]];
    

}
- (void)viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    //Initialize the WME data processing
    self.pWMEDataProcess = [WMEDataProcess instance];
    
    self.selectedVideoCapabilitySetting = self.pWMEDataProcess.videoCapIndex;
    self.selectedAudioCapabilitySetting = self.pWMEDataProcess.audioCapIndex;
    self.selectedCameraCapabilitySetting = self.pWMEDataProcess.cameraCapIndex;
    self.bSendVideoState = self.pWMEDataProcess.bVideoSending;
    self.bKeepAspectRatio = self.pWMEDataProcess.bKeepAspectRatio;
    
    UITableViewCell * selectedTableViewCell;
    int videoCapCount = [self.pWMEDataProcess.arrayVideoCapabilities count];
    int audioCapCount = [self.pWMEDataProcess.arrayAudioCapabilities count];
    
    if (self.selectedVideoCapabilitySetting >= videoCapCount) {
        self.selectedVideoCapabilitySetting = videoCapCount - 1;
    }
    if (self.selectedAudioCapabilitySetting >= audioCapCount) {
        self.selectedAudioCapabilitySetting = audioCapCount - 1;
    }

    //Update the capability section
    if (videoCapCount > 0) {
        selectedTableViewCell = [self.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:CAPABILITY_VIDEO_ENCODER inSection:SECTION_CAPABILITY]];
        selectedTableViewCell.detailTextLabel.text = [self.pWMEDataProcess.arrayVideoCapabilities objectAtIndex:self.selectedVideoCapabilitySetting];
    }
    
    if (audioCapCount > 0) {
        selectedTableViewCell = [self.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:CAPABILITY_AUDIO_ENCODER inSection:SECTION_CAPABILITY]];
        selectedTableViewCell.detailTextLabel.text = [self.pWMEDataProcess.arrayAudioCapabilities objectAtIndex:self.selectedAudioCapabilitySetting];
    }

    int cameraCapCount = [self.pWMEDataProcess.arrayCameraCapabilities count];
    if (self.selectedCameraCapabilitySetting >= cameraCapCount) {
        self.selectedCameraCapabilitySetting = cameraCapCount - 1;
    }
    
    if (cameraCapCount > 0) {
        selectedTableViewCell = [self.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:CAPABILITY_VIDEO_CAPTURE inSection:SECTION_CAPABILITY]];
        selectedTableViewCell.detailTextLabel.text = [self.pWMEDataProcess.arrayCameraCapabilities objectAtIndex:self.selectedCameraCapabilitySetting];
    }
    
    //update the UISwitch UI
    [self.portraitRotationEnableSWT setOn:self.bKeepAspectRatio animated:NO];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source
/*
- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView
{
#warning Potentially incomplete method implementation.
    // Return the number of sections.
    return 2;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section
{
#warning Incomplete method implementation.
    // Return the number of rows in the section.
    return 3;
}

- (UITableViewCell *)tableView:(UITableView *)tableView cellForRowAtIndexPath:(NSIndexPath *)indexPath
{
    static NSString *CellIdentifier = @"Cell";
    UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier forIndexPath:indexPath];
    
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
    }   
    else if (editingStyle == UITableViewCellEditingStyleInsert) {
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

#pragma mark - Table view delegate

- (void)tableView:(UITableView *)tableView didSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    // Navigation logic may go here. Create and push another view controller.
    /*
     <#DetailViewController#> *detailViewController = [[<#DetailViewController#> alloc] initWithNibName:@"<#Nib name#>" bundle:nil];
     // ...
     // Pass the selected object to the new view controller.
     [self.navigationController pushViewController:detailViewController animated:YES];
     */
    _selectedRow = indexPath.row;
    _selectedSection = indexPath.section;

}




//unwind segue
- (IBAction)GetSetting:(UIStoryboardSegue *)segue
{
    //Get the setting
    if (_selectedSection == SECTION_CAPABILITY) {
        WMEContentSelectionViewController *contentSelectionViewController = [segue sourceViewController];
        WMESettingViewController *settingViewController = [segue destinationViewController];
        
        if (_selectedRow == CAPABILITY_VIDEO_ENCODER) {
            self.selectedVideoCapabilitySetting = [contentSelectionViewController selectedRow];
            if (self.selectedVideoCapabilitySetting < [self.pWMEDataProcess.arrayVideoCapabilities count]) {
                UITableViewCell *selectedTableViewCell = [settingViewController.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:_selectedRow inSection:SECTION_CAPABILITY]];
                selectedTableViewCell.detailTextLabel.text = [self.pWMEDataProcess.arrayVideoCapabilities objectAtIndex:self.selectedVideoCapabilitySetting];
                self.pWMEDataProcess.videoCapIndex = _selectedVideoCapabilitySetting;
                
                
            }
        }
        else if (_selectedRow == CAPABILITY_AUDIO_ENCODER) {
            self.selectedAudioCapabilitySetting = [contentSelectionViewController selectedRow];
            if (self.selectedAudioCapabilitySetting < [self.pWMEDataProcess.arrayAudioCapabilities count]) {
                UITableViewCell *selectedTableViewCell = [settingViewController.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:_selectedRow inSection:SECTION_CAPABILITY]];
                selectedTableViewCell.detailTextLabel.text = [self.pWMEDataProcess.arrayAudioCapabilities objectAtIndex:self.selectedAudioCapabilitySetting];
                self.pWMEDataProcess.audioCapIndex = _selectedAudioCapabilitySetting;
            }
        }
        else if (_selectedRow == CAPABILITY_VIDEO_CAPTURE) {
            self.selectedCameraCapabilitySetting = [contentSelectionViewController selectedRow];
            if (self.selectedCameraCapabilitySetting < [self.pWMEDataProcess.arrayCameraCapabilities count]) {
                UITableViewCell *selectedTableViewCell = [settingViewController.tableView cellForRowAtIndexPath:[NSIndexPath indexPathForRow:_selectedRow inSection:SECTION_CAPABILITY]];
                selectedTableViewCell.detailTextLabel.text = [self.pWMEDataProcess.arrayCameraCapabilities objectAtIndex:self.selectedCameraCapabilitySetting];
                self.pWMEDataProcess.cameraCapIndex = _selectedCameraCapabilitySetting;
            }
        }
    }
    else if (_selectedSection == SECTION_STATISTICS)
    {
        //For statistics
        //if ((_selectedRow == STATISTICS_VIDEO)||(_selectedRow == STATISTICS_AUDIO)) {
        [updateStatisticsTimer invalidate];
        [[_arrayStatistics objectAtIndex:0] removeAllObjects];
        [[_arrayStatistics objectAtIndex:1] removeAllObjects];
        [_arrayStatisticsTitle removeAllObjects];
        //}
    }

    
}
 

-(void)prepareForSegue:(UIStoryboardSegue *)segue sender:(id)sender
{
    //For set the parameters setting
    if ([[segue identifier] isEqualToString:@"videoEncodingParametersSegue"]) {
        WMEContentSelectionViewController *contentSelectionViewController = [segue destinationViewController];
        contentSelectionViewController.arrayCapabilityList = self.pWMEDataProcess.arrayVideoCapabilities;
        contentSelectionViewController.selectedRow = self.selectedVideoCapabilitySetting;
        
    }
    
    if ([[segue identifier] isEqualToString:@"audioEncodingParametersSegue"]) {
        WMEContentSelectionViewController *contentSelectionViewController = [segue destinationViewController];
        
        contentSelectionViewController.arrayCapabilityList = self.pWMEDataProcess.arrayAudioCapabilities;
        contentSelectionViewController.selectedRow = self.selectedAudioCapabilitySetting;
    }
    
    if ([[segue identifier] isEqualToString:@"videoCaptureParametersSegue"]) {
        WMEContentSelectionViewController *contentSelectionViewController = [segue destinationViewController];
        
        contentSelectionViewController.arrayCapabilityList = self.pWMEDataProcess.arrayCameraCapabilities;
        contentSelectionViewController.selectedRow = self.selectedCameraCapabilitySetting;
    }

    //For show the statistics
    if ([[segue identifier] isEqualToString:@"videoNetworkStatisticsSegue"]) {
        WMEStatisticsViewController *statisticsViewController = [segue destinationViewController];
        
        _selectedRow = STATISTICS_VIDEO_NETWORK;
        [self getStatistics];        
        statisticsViewController.arrayStatistics = _arrayStatistics;
        statisticsViewController.arrayStatisticsTitle = _arrayStatisticsTitle;
        updateStatisticsTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(updateStatistics) userInfo:nil repeats:YES];
    }
    if ([[segue identifier] isEqualToString:@"audioNetworkStatisticsSegue"]) {
        WMEStatisticsViewController *statisticsViewController = [segue destinationViewController];
        _selectedRow = STATISTICS_AUDIO_NETWORK;
        [self getStatistics];
        statisticsViewController.arrayStatistics = _arrayStatistics;
        statisticsViewController.arrayStatisticsTitle = _arrayStatisticsTitle;
        updateStatisticsTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(updateStatistics) userInfo:nil repeats:YES];
    }
    if ([[segue identifier] isEqualToString:@"videoTrackParametersSegue"]) {
        WMEStatisticsViewController *statisticsViewController = [segue destinationViewController];
        _selectedRow = STATISTICS_VIDEO_TRACK;
        [self getStatistics];
        statisticsViewController.arrayStatistics = _arrayStatistics;
        statisticsViewController.arrayStatisticsTitle = _arrayStatisticsTitle;
        updateStatisticsTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(updateStatistics) userInfo:nil repeats:YES];
    }
    if ([[segue identifier] isEqualToString:@"audioTrackParametersSegue"]) {
        WMEStatisticsViewController *statisticsViewController = [segue destinationViewController];
        _selectedRow = STATISTICS_AUDIO_TRACK;
        [self getStatistics];
        statisticsViewController.arrayStatistics = _arrayStatistics;
        statisticsViewController.arrayStatisticsTitle = _arrayStatisticsTitle;        
        updateStatisticsTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(updateStatistics) userInfo:nil repeats:YES];
    }
}

- (void)updateStatistics
{
    WmeSessionStatistics wmeStatistics;
    WmeVideoStatistics wmeVideoLocalTrackStatistics;
    WmeVideoStatistics wmeVideoRemoteTrackStatistics;
    WmeAudioStatistics wmeAudioLocalTrackStatistics;
    WmeAudioStatistics wmeAudioRemoteTrackStatistics;
    
    char info_line[100];
    //static unsigned int testNumber = 11;
    
    //add the statistics to arrayStatistics
    switch (_selectedRow) {
        case STATISTICS_VIDEO_NETWORK:
            [self.pWMEDataProcess getVideoStatistics:wmeStatistics];
            break;
        case STATISTICS_AUDIO_NETWORK:
            [self.pWMEDataProcess getAudioStatistics:wmeStatistics];
            break;
        case STATISTICS_VIDEO_TRACK:
            [self.pWMEDataProcess getVideoStatistics:wmeVideoLocalTrackStatistics trackType:DEMO_LOCAL_TRACK];
            [self.pWMEDataProcess getVideoStatistics:wmeVideoRemoteTrackStatistics trackType:DEMO_REMOTE_TRACK];
            break;
        case STATISTICS_AUDIO_TRACK:
            [self.pWMEDataProcess getAudioStatistics:wmeAudioLocalTrackStatistics trackType:DEMO_LOCAL_TRACK];
            [self.pWMEDataProcess getAudioStatistics:wmeAudioRemoteTrackStatistics trackType:DEMO_REMOTE_TRACK];
            break;
        default:
            return;
    }
    if ((_selectedRow == STATISTICS_VIDEO_NETWORK) ||
        (_selectedRow == STATISTICS_AUDIO_NETWORK) )
    {
        //For input info of network statisticS
        sprintf(info_line, "Loss Ratio:      %f", wmeStatistics.stInNetworkStat.fLossRatio);
        [[_arrayStatistics objectAtIndex:0] replaceObjectAtIndex:0 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Jitter:          %d", wmeStatistics.stInNetworkStat.uJitter);
        [[_arrayStatistics objectAtIndex:0] replaceObjectAtIndex:1 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Round Trip Time: %d", wmeStatistics.stInNetworkStat.uRoundTripTime);
        [[_arrayStatistics objectAtIndex:0] replaceObjectAtIndex:2 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Input Bytes:     %d", wmeStatistics.stInNetworkStat.uBytes);
        [[_arrayStatistics objectAtIndex:0] replaceObjectAtIndex:3 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Input Packets:   %d", wmeStatistics.stInNetworkStat.uPackets);
        [[_arrayStatistics objectAtIndex:0] replaceObjectAtIndex:4 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        //For output info of network statistics
        sprintf(info_line, "Loss Ratio:      %f", wmeStatistics.stOutNetworkStat.fLossRatio);
        [[_arrayStatistics objectAtIndex:1] replaceObjectAtIndex:0 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Jitter:          %d", wmeStatistics.stOutNetworkStat.uJitter);
        [[_arrayStatistics objectAtIndex:1] replaceObjectAtIndex:1 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Round Trip Time: %d", wmeStatistics.stOutNetworkStat.uRoundTripTime);
        [[_arrayStatistics objectAtIndex:1] replaceObjectAtIndex:2 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Output Bytes:    %d", wmeStatistics.stOutNetworkStat.uBytes);
        [[_arrayStatistics objectAtIndex:1] replaceObjectAtIndex:3 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Output Packets:  %d", wmeStatistics.stOutNetworkStat.uPackets);
        //sprintf(info_line, "Output Packets:  %d", testNumber ++);
        [[_arrayStatistics objectAtIndex:1] replaceObjectAtIndex:4 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
    }
    
    if (_selectedRow == STATISTICS_VIDEO_TRACK)
    {
        //For input info of network statisticS
        sprintf(info_line, "Width:      %d", wmeVideoLocalTrackStatistics.uWidth);
        [[_arrayStatistics objectAtIndex:0] replaceObjectAtIndex:0 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Height:          %d", wmeVideoLocalTrackStatistics.uHeight);
        [[_arrayStatistics objectAtIndex:0] replaceObjectAtIndex:1 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Frame Rate: %f", wmeVideoLocalTrackStatistics.fFrameRate);
        [[_arrayStatistics objectAtIndex:0] replaceObjectAtIndex:2 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Bit Rate:     %f", wmeVideoLocalTrackStatistics.fBitRate);
        [[_arrayStatistics objectAtIndex:0] replaceObjectAtIndex:3 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Average Bit Rate:   %f", wmeVideoLocalTrackStatistics.fAvgBitRate);
        [[_arrayStatistics objectAtIndex:0] replaceObjectAtIndex:4 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        //For output info of network statistics
        sprintf(info_line, "Width:      %d", wmeVideoRemoteTrackStatistics.uWidth);
        [[_arrayStatistics objectAtIndex:1] replaceObjectAtIndex:0 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Height:          %d", wmeVideoRemoteTrackStatistics.uHeight);
        [[_arrayStatistics objectAtIndex:1] replaceObjectAtIndex:1 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Frame Rate: %f", wmeVideoRemoteTrackStatistics.fFrameRate);
        [[_arrayStatistics objectAtIndex:1] replaceObjectAtIndex:2 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Bit Rate:     %f", wmeVideoRemoteTrackStatistics.fBitRate);
        [[_arrayStatistics objectAtIndex:1] replaceObjectAtIndex:3 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Average Bit Rate:%f", wmeVideoRemoteTrackStatistics.fAvgBitRate);
        [[_arrayStatistics objectAtIndex:1] replaceObjectAtIndex:4 withObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
    }
    
    if (_selectedRow == STATISTICS_AUDIO_TRACK)
    {
        //TBD...
    }
}

- (void)getStatistics
{
    WmeSessionStatistics wmeStatistics;
    WmeVideoStatistics wmeVideoLocalTrackStatistics;
    WmeVideoStatistics wmeVideoRemoteTrackStatistics;
    WmeAudioStatistics wmeAudioLocalTrackStatistics;
    WmeAudioStatistics wmeAudioRemoteTrackStatistics;
    
    char info_line[100];

    //add the statistics to arrayStatistics
    switch (_selectedRow) {
        case STATISTICS_VIDEO_NETWORK:
            [self.pWMEDataProcess getVideoStatistics:wmeStatistics];
            break;
        case STATISTICS_AUDIO_NETWORK:
            [self.pWMEDataProcess getAudioStatistics:wmeStatistics];
            break;
        case STATISTICS_VIDEO_TRACK:
            [self.pWMEDataProcess getVideoStatistics:wmeVideoLocalTrackStatistics trackType:DEMO_LOCAL_TRACK];
            [self.pWMEDataProcess getVideoStatistics:wmeVideoRemoteTrackStatistics trackType:DEMO_REMOTE_TRACK];
            break;
        case STATISTICS_AUDIO_TRACK:
            [self.pWMEDataProcess getAudioStatistics:wmeAudioLocalTrackStatistics trackType:DEMO_LOCAL_TRACK];
            [self.pWMEDataProcess getAudioStatistics:wmeAudioRemoteTrackStatistics trackType:DEMO_REMOTE_TRACK];
            break;
        default:
            return;
    }
    if ((_selectedRow == STATISTICS_VIDEO_NETWORK) ||
        (_selectedRow == STATISTICS_AUDIO_NETWORK) )
    {
        //For input info of network statisticTitle
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Input Statistics" ]];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Output Statistics"]];
        
        //For input info of network statisticS
        sprintf(info_line, "Loss Ratio:      %f", wmeStatistics.stInNetworkStat.fLossRatio);
        [[_arrayStatistics objectAtIndex:0] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Jitter:          %d", wmeStatistics.stInNetworkStat.uJitter);
        [[_arrayStatistics objectAtIndex:0] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Round Trip Time: %d", wmeStatistics.stInNetworkStat.uRoundTripTime);
        [[_arrayStatistics objectAtIndex:0] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Input Bytes:     %d", wmeStatistics.stInNetworkStat.uBytes);
        [[_arrayStatistics objectAtIndex:0] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Input Packets:   %d", wmeStatistics.stInNetworkStat.uPackets);
        [[_arrayStatistics objectAtIndex:0] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        //For output info of network statistics
        sprintf(info_line, "Loss Ratio:      %f", wmeStatistics.stOutNetworkStat.fLossRatio);
        [[_arrayStatistics objectAtIndex:1] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Jitter:          %d", wmeStatistics.stOutNetworkStat.uJitter);
        [[_arrayStatistics objectAtIndex:1] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Round Trip Time: %d", wmeStatistics.stOutNetworkStat.uRoundTripTime);
        [[_arrayStatistics objectAtIndex:1] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Output Bytes:    %d", wmeStatistics.stOutNetworkStat.uBytes);
        [[_arrayStatistics objectAtIndex:1] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Output Packets:  %d", wmeStatistics.stOutNetworkStat.uPackets);
        [[_arrayStatistics objectAtIndex:1] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
    }
    
    if (_selectedRow == STATISTICS_VIDEO_TRACK)
    {
        //For input info of network statisticTitle
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Local Statistics"]];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Remote Statistics"]];
        
        //For input info of network statisticS
        sprintf(info_line, "Width:      %d", wmeVideoLocalTrackStatistics.uWidth);
        [[_arrayStatistics objectAtIndex:0] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Height:          %d", wmeVideoLocalTrackStatistics.uHeight);
        [[_arrayStatistics objectAtIndex:0] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Frame Rate: %f", wmeVideoLocalTrackStatistics.fFrameRate);
        [[_arrayStatistics objectAtIndex:0] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Bit Rate:     %f", wmeVideoLocalTrackStatistics.fBitRate);
        [[_arrayStatistics objectAtIndex:0] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Average Bit Rate:   %f", wmeVideoLocalTrackStatistics.fAvgBitRate);
        [[_arrayStatistics objectAtIndex:0] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        //For output info of network statistics
        sprintf(info_line, "Width:      %d", wmeVideoRemoteTrackStatistics.uWidth);
        [[_arrayStatistics objectAtIndex:1] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Height:          %d", wmeVideoRemoteTrackStatistics.uHeight);
        [[_arrayStatistics objectAtIndex:1] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Frame Rate: %f", wmeVideoRemoteTrackStatistics.fFrameRate);
        [[_arrayStatistics objectAtIndex:1] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Bit Rate:     %f", wmeVideoRemoteTrackStatistics.fBitRate);
        [[_arrayStatistics objectAtIndex:1] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
        sprintf(info_line, "Average Bit Rate:%f", wmeVideoRemoteTrackStatistics.fAvgBitRate);
        [[_arrayStatistics objectAtIndex:1] addObject:[[NSString alloc] initWithCString:info_line encoding:NSASCIIStringEncoding]];
    }
    
    if (_selectedRow == STATISTICS_AUDIO_TRACK)
    {
        //For input info of network statisticTitle
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Local Statistics"]];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Remote Statistics"]];
        //TBD...
    }

}
- (IBAction)RotationEnableSwitch:(id)sender {
    self.bKeepAspectRatio = [self.portraitRotationEnableSWT isOn];
}

@end
