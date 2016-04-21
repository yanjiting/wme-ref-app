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
#import "PeerCall.h"
#import "Loopback.h"

@interface WMESettingViewController ()
+ (NSString*)timeFormat: (unsigned long)time;
@end

@implementation WMESettingViewController

@synthesize selectedRow = _selectedRow;
@synthesize selectedSection = _selectedSection;
@synthesize updateStatisticsTimer = _updateStatisticsTimer;
@synthesize arrayStatistics = _arrayStatistics;
@synthesize arrayStatisticsTitle = _arrayStatisticsTitle;
@synthesize bLoopback;

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
    /*self.pWMEDataProcess = [WMEDataProcess instance];
    
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
    }*/
    
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
    //_selectedRow = indexPath.row;
    _selectedSection = indexPath.section;

}




//unwind segue
- (IBAction)GetSetting:(UIStoryboardSegue *)segue
{
    //Get the setting
    if (_selectedSection == SECTION_CAPABILITY) {
        WMEContentSelectionViewController *contentSelectionViewController = [segue sourceViewController];
        WMESettingViewController *settingViewController = [segue destinationViewController];
        /*
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
        }*/
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
    /*if ([[segue identifier] isEqualToString:@"videoEncodingParametersSegue"]) {
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
    }*/

    //For show the statistics
    if ([[segue identifier] isEqualToString:@"videoNetworkStatisticsSegue"]) {
        WMEStatisticsViewController *statisticsViewController = [segue destinationViewController];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Input Statistics" ]];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Output Statistics"]];
        _selectedRow = STATISTICS_VIDEO_NETWORK;
        [self updateStatistics];
        statisticsViewController.arrayStatistics = _arrayStatistics;
        statisticsViewController.arrayStatisticsTitle = _arrayStatisticsTitle;
        updateStatisticsTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(updateStatistics) userInfo:nil repeats:YES];
    }
    if ([[segue identifier] isEqualToString:@"audioNetworkStatisticsSegue"]) {
        WMEStatisticsViewController *statisticsViewController = [segue destinationViewController];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Input Statistics" ]];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Output Statistics"]];
        _selectedRow = STATISTICS_AUDIO_NETWORK;
        [self updateStatistics];
        statisticsViewController.arrayStatistics = _arrayStatistics;
        statisticsViewController.arrayStatisticsTitle = _arrayStatisticsTitle;
        updateStatisticsTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(updateStatistics) userInfo:nil repeats:YES];
    }
    if ([[segue identifier] isEqualToString:@"videoTrackParametersSegue"]) {
        WMEStatisticsViewController *statisticsViewController = [segue destinationViewController];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Local Statistics"]];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Remote Statistics"]];
        _selectedRow = STATISTICS_VIDEO_TRACK;
        [self updateStatistics];
        statisticsViewController.arrayStatistics = _arrayStatistics;
        statisticsViewController.arrayStatisticsTitle = _arrayStatisticsTitle;
        updateStatisticsTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(updateStatistics) userInfo:nil repeats:YES];
    }
    if ([[segue identifier] isEqualToString:@"audioTrackParametersSegue"]) {
        WMEStatisticsViewController *statisticsViewController = [segue destinationViewController];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Local Statistics"]];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Remote Statistics"]];
        _selectedRow = STATISTICS_AUDIO_TRACK;
        [self updateStatistics];
        statisticsViewController.arrayStatistics = _arrayStatistics;
        statisticsViewController.arrayStatisticsTitle = _arrayStatisticsTitle;        
        updateStatisticsTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(updateStatistics) userInfo:nil repeats:YES];
    }
    if ([[segue identifier] isEqualToString:@"videoConnectionParametersSegue"]) {
        WMEStatisticsViewController *statisticsViewController = [segue destinationViewController];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Local Statistics"]];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Remote Statistics"]];
        _selectedRow = STATISTICS_VIDEO_CONNECTION;
        [self updateStatistics];
        statisticsViewController.arrayStatistics = _arrayStatistics;
        statisticsViewController.arrayStatisticsTitle = _arrayStatisticsTitle;
        updateStatisticsTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(updateStatistics) userInfo:nil repeats:YES];
    }
    if ([[segue identifier] isEqualToString:@"audioConnectionParametersSegue"]) {
        WMEStatisticsViewController *statisticsViewController = [segue destinationViewController];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Local Statistics"]];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Remote Statistics"]];
        _selectedRow = STATISTICS_AUDIO_CONNECTION;
        [self updateStatistics];
        statisticsViewController.arrayStatistics = _arrayStatistics;
        statisticsViewController.arrayStatisticsTitle = _arrayStatisticsTitle;
        updateStatisticsTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(updateStatistics) userInfo:nil repeats:YES];
    }
    
    if ([[segue identifier] isEqualToString:@"networkIndexSegue"]) {
        WMEStatisticsViewController *statisticsViewController = [segue destinationViewController];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Network Index"]];
        _selectedRow = STATISTICS_NETWORK_INDEX;
        [self updateStatistics];
        statisticsViewController.arrayStatistics = _arrayStatistics;
        statisticsViewController.arrayStatisticsTitle = _arrayStatisticsTitle;
        updateStatisticsTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(updateStatistics) userInfo:nil repeats:YES];
    }
    
    if ([[segue identifier] isEqualToString:@"systemInfoSegue"]) {
        WMEStatisticsViewController *statisticsViewController = [segue destinationViewController];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"CPU Usage"]];
        [self.arrayStatisticsTitle addObject:[[NSString alloc] initWithFormat:@"Memory Usage"]];
        _selectedRow = STATISTICS_SYSTEM_INFO;
        [self updateStatistics];
        statisticsViewController.arrayStatistics = _arrayStatistics;
        statisticsViewController.arrayStatisticsTitle = _arrayStatisticsTitle;
        updateStatisticsTimer = [NSTimer scheduledTimerWithTimeInterval:0.5 target:self selector:@selector(updateStatistics) userInfo:nil repeats:YES];
    }
}

+ (NSString*)timeFormat: (unsigned long)time
{
    NSTimeInterval t = time/1000.0;
    NSDate* date = [NSDate dateWithTimeIntervalSince1970:t];
    NSDateFormatter *dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setTimeStyle:NSDateFormatterShortStyle];
    [dateFormatter setDateFormat:@"HH:mm:ss.SSS"];
    return [dateFormatter stringFromDate:date];
}

- (void)updateStatistics
{
    wme::WmeAudioConnectionStatistics audio = {{0}};
    wme::WmeVideoConnectionStatistics video = {{0}};
    wme::WmeNetworkIndex idxUplink = NETWORK_EXCELLENT;
    wme::WmeNetworkIndex idxDownlink = NETWORK_EXCELLENT;
    wme::WmeNetworkIndex idxBothlink = NETWORK_EXCELLENT;
    WmeMemoryUsage memoryUsage = {0};
    WmeCpuUsage cpuUsage = {0};
    
    if(bLoopback){
        LoopbackCall::Instance()->getStatistics(audio, video);
        LoopbackCall::Instance()->getNetworkIndex(DIRECTION_UPLINK, idxUplink);
        LoopbackCall::Instance()->getNetworkIndex(DIRECTION_DOWNLINK, idxDownlink);
        LoopbackCall::Instance()->getNetworkIndex(DIRECTION_BOTHLINK, idxBothlink);
        LoopbackCall::Instance()->m_endCaller.GetCpuUsage(cpuUsage);
        LoopbackCall::Instance()->m_endCaller.GetMemoryUsage(memoryUsage);
    }else{
        PeerCall::Instance()->m_endCaller->getStatistics(audio, video);
        PeerCall::Instance()->m_endCaller->getNetworkIndex(DIRECTION_UPLINK, idxUplink);
        PeerCall::Instance()->m_endCaller->getNetworkIndex(DIRECTION_DOWNLINK, idxDownlink);
        PeerCall::Instance()->m_endCaller->getNetworkIndex(DIRECTION_BOTHLINK, idxBothlink);
        PeerCall::Instance()->m_endCaller->GetCpuUsage(cpuUsage);
        PeerCall::Instance()->m_endCaller->GetMemoryUsage(memoryUsage);
    }
    
    NSString *strItem = nil;
    if(_selectedRow == STATISTICS_AUDIO_NETWORK)
    {
        //For input info of network statisticS
        strItem = [NSString stringWithFormat:@"Loss Ratio:      %f", audio.sessStat.stInNetworkStat.fLossRatio];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 0];
        strItem = [NSString stringWithFormat:@"Jitter:          %d", audio.sessStat.stInNetworkStat.uJitter];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 1];
        strItem = [NSString stringWithFormat:@"Round Trip Time: %d", audio.sessStat.stInNetworkStat.uRoundTripTime];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 2];
        strItem = [NSString stringWithFormat:@"Input Bytes:     %d", audio.sessStat.stInNetworkStat.uBytes];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 3];
        strItem = [NSString stringWithFormat:@"Input Packets:   %d", audio.sessStat.stInNetworkStat.uPackets];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 4];
        //For output info of network statistics
        strItem = [NSString stringWithFormat:@"Loss Ratio:      %f", audio.sessStat.stOutNetworkStat.fLossRatio];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 0];
        strItem = [NSString stringWithFormat:@"Jitter:          %d", audio.sessStat.stOutNetworkStat.uJitter];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 1];
        strItem = [NSString stringWithFormat:@"Round Trip Time: %d", audio.sessStat.stOutNetworkStat.uRoundTripTime];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 2];
        strItem = [NSString stringWithFormat:@"Output Bytes:    %d", audio.sessStat.stOutNetworkStat.uBytes];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 3];
        strItem = [NSString stringWithFormat:@"Output Packets:  %d", audio.sessStat.stOutNetworkStat.uPackets];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 4];
    }
    
    if(_selectedRow == STATISTICS_VIDEO_NETWORK)
    {
        //For input info of network statisticS
        strItem = [NSString stringWithFormat:@"Loss Ratio:      %f", video.sessStat.stInNetworkStat.fLossRatio];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 0];
        strItem = [NSString stringWithFormat:@"Jitter:          %d", video.sessStat.stInNetworkStat.uJitter];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 1];
        strItem = [NSString stringWithFormat:@"Round Trip Time: %d", video.sessStat.stInNetworkStat.uRoundTripTime];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 2];
        strItem = [NSString stringWithFormat:@"Input Bytes:     %d", video.sessStat.stInNetworkStat.uBytes];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 3];
        strItem = [NSString stringWithFormat:@"Input Packets:   %d", video.sessStat.stInNetworkStat.uPackets];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 4];
        //For output info of network statistics
        strItem = [NSString stringWithFormat:@"Loss Ratio:      %f", video.sessStat.stOutNetworkStat.fLossRatio];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 0];
        strItem = [NSString stringWithFormat:@"Jitter:          %d", video.sessStat.stOutNetworkStat.uJitter];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 1];
        strItem = [NSString stringWithFormat:@"Round Trip Time: %d", video.sessStat.stOutNetworkStat.uRoundTripTime];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 2];
        strItem = [NSString stringWithFormat:@"Output Bytes:    %d", video.sessStat.stOutNetworkStat.uBytes];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 3];
        strItem = [NSString stringWithFormat:@"Output Packets:  %d", video.sessStat.stOutNetworkStat.uPackets];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 4];
    }
    
    if (_selectedRow == STATISTICS_VIDEO_TRACK)
    {
        strItem = [NSString stringWithFormat:@"Width:            %d", video.localVideoStat.uWidth];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 0];
        strItem = [NSString stringWithFormat:@"Height:           %d", video.localVideoStat.uHeight];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 1];
        strItem = [NSString stringWithFormat:@"Frame Rate:       %f", video.localVideoStat.fFrameRate];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 2];
        strItem = [NSString stringWithFormat:@"Bit Rate:         %f", video.localVideoStat.fBitRate];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 3];
        strItem = [NSString stringWithFormat:@"Average Bit Rate: %f", video.localVideoStat.fAvgBitRate];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 4];
        strItem = [NSString stringWithFormat:@"PLI number:       %d", video.localVideoStat.uIDRReqNum];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 5];

        strItem = [NSString stringWithFormat:@"Width:            %d", video.remoteVideoStat.uWidth];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 0];
        strItem = [NSString stringWithFormat:@"Height:           %d", video.remoteVideoStat.uHeight];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 1];
        strItem = [NSString stringWithFormat:@"Frame Rate:       %f", video.remoteVideoStat.fFrameRate];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 2];
        strItem = [NSString stringWithFormat:@"Bit Rate:         %f", video.remoteVideoStat.fBitRate];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 3];
        strItem = [NSString stringWithFormat:@"Average Bit Rate: %f", video.remoteVideoStat.fAvgBitRate];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 4];
        strItem = [NSString stringWithFormat:@"PLI number:       %d", video.remoteVideoStat.uIDRReqNum];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 5];
        strItem = [NSString stringWithFormat:@"Frame Count:      %d", video.remoteVideoStat.uRenderFrameCount];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 6];
    }
    
    if (_selectedRow == STATISTICS_AUDIO_TRACK)
    {
        strItem = [NSString stringWithFormat:@"Bit Rate:         %d", audio.localAudioStat.uBitRate];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 0];
        
        strItem = [NSString stringWithFormat:@"Bit Rate:         %d", audio.remoteAudioStat.uBitRate];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 0];
    }
    
    if (_selectedRow == STATISTICS_VIDEO_CONNECTION)
    {
        strItem = [NSString stringWithFormat:@"IP:            %@:%d", [NSString stringWithUTF8String:video.connStat.localIp], video.connStat.uLocalPort];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 0];
        strItem = [NSString stringWithFormat:@"RTP sent:      %d", video.connStat.uRTPSent];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 1];
        strItem = [NSString stringWithFormat:@"RTCP sent:     %d", video.connStat.uRTCPSent];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 2];
        
        strItem = [NSString stringWithFormat:@"Bind Time:     %@", [WMESettingViewController timeFormat:video.connStat.timeBound]];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 3];
        strItem = [NSString stringWithFormat:@"Connect Time:  %@", [WMESettingViewController timeFormat:video.connStat.timeConnecting]];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 4];
        strItem = [NSString stringWithFormat:@"Connected Time:%@", [WMESettingViewController timeFormat:video.connStat.timeConnected]];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 5];
        strItem = [NSString stringWithFormat:@"Send Time:     %@", [WMESettingViewController timeFormat:video.connStat.timeSent]];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 6];
        strItem = [NSString stringWithFormat:@"Received Time: %@", [WMESettingViewController timeFormat:video.connStat.timeReceived]];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 7];
        
        strItem = [NSString stringWithFormat:@"IP:            %@:%d", [NSString stringWithUTF8String:video.connStat.remoteIp], video.connStat.uRemotePort];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 0];
        strItem = [NSString stringWithFormat:@"RTP received:  %d", video.connStat.uRTPReceived];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 1];
        strItem = [NSString stringWithFormat:@"RTCP received: %d", video.connStat.uRTCPReceived];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 2];
    }
    
    if (_selectedRow == STATISTICS_AUDIO_CONNECTION)
    {
        strItem = [NSString stringWithFormat:@"IP:            %@:%d", [NSString stringWithUTF8String:audio.connStat.localIp], audio.connStat.uLocalPort];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 0];
        strItem = [NSString stringWithFormat:@"RTP sent:      %d", audio.connStat.uRTPSent];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 1];
        strItem = [NSString stringWithFormat:@"RTCP sent:     %d", audio.connStat.uRTCPSent];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 2];
        
        strItem = [NSString stringWithFormat:@"Bind Time:     %@", [WMESettingViewController timeFormat:audio.connStat.timeBound]];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 3];
        strItem = [NSString stringWithFormat:@"Connect Time:  %@", [WMESettingViewController timeFormat:audio.connStat.timeConnecting]];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 4];
        strItem = [NSString stringWithFormat:@"Connected Time:%@", [WMESettingViewController timeFormat:audio.connStat.timeConnected]];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 5];
        strItem = [NSString stringWithFormat:@"Send Time:     %@", [WMESettingViewController timeFormat:audio.connStat.timeSent]];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 6];
        strItem = [NSString stringWithFormat:@"Received Time: %@", [WMESettingViewController timeFormat:audio.connStat.timeReceived]];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 7];
        
        strItem = [NSString stringWithFormat:@"IP:            %@:%d", [NSString stringWithUTF8String:audio.connStat.remoteIp], audio.connStat.uRemotePort];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 0];
        strItem = [NSString stringWithFormat:@"RTP received:  %d", audio.connStat.uRTPReceived];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 1];
        strItem = [NSString stringWithFormat:@"RTCP received: %d", audio.connStat.uRTCPReceived];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 2];
    }
    
    if (_selectedRow == STATISTICS_NETWORK_INDEX)
    {
        strItem = [NSString stringWithFormat:@"Uplink Index:    %d", idxUplink];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 0];
        
        strItem = [NSString stringWithFormat:@"Downlink Index:  %d", idxUplink];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 1];
        
        strItem = [NSString stringWithFormat:@"Bothlink Index:  %d", idxBothlink];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 2];
    }
    
    if (_selectedRow == STATISTICS_SYSTEM_INFO)
    {
        strItem = [NSString stringWithFormat:@"Total:    %f", cpuUsage.fTotalUsage];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 0];
        for(int i = 0; i < cpuUsage.nCores; i++){
            strItem = [NSString stringWithFormat:@"Core %d:    %f", i+1, cpuUsage.pfCores[i]];
            [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: i+1];
        }
        
        strItem = [NSString stringWithFormat:@"Total:    %f", cpuUsage.fProcessUsage];
        [[_arrayStatistics objectAtIndex:0] setObject: strItem atIndexedSubscript: 0];
        
        strItem = [NSString stringWithFormat:@"Memory Usage:  %f", memoryUsage.fMemroyUsage];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 0];
        strItem = [NSString stringWithFormat:@"Memory Used:   %d", memoryUsage.uMemoryUsed];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 1];
        strItem = [NSString stringWithFormat:@"Memory Total:   %d", memoryUsage.uMemoryTotal];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 2];
        strItem = [NSString stringWithFormat:@"Process Memory: %d", memoryUsage.uProcessMemroyUsed];
        [[_arrayStatistics objectAtIndex:1] setObject: strItem atIndexedSubscript: 3];
    }
}

- (IBAction)RotationEnableSwitch:(id)sender {
    self.bKeepAspectRatio = [self.portraitRotationEnableSWT isOn];
}



@end
