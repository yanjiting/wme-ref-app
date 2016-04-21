Feature: Test Video Features

@uMaxPacketSize-ePacketizationMode @dev
Scenario Outline: Test uMaxPacketSize and ePacketizationMode
	Given I have 1 <dev1> and 1 <dev2>
      And I create a venue
    When I create offer on device 1 
	And I create offer on device 2
	Then I keep the call for 5 seconds
	Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    Then I set the following Video parameters for device 1 as: uMaxPacketSize = 1000
    Then I set the following Video parameters for device 1 as: ePacketizationMode = 0
    Then I keep the call for 5 seconds 
    Then I should receive the following value from device 1, Video parameter: uMaxPacketSize == 1000
    Then I should receive the following value from device 1, Video parameter: ePacketizationMode == 0
    
Examples:
    | dev1       | dev2       |
    | android    | mac        |
    | mac        | ios        |
    | ios        | android    |


@eVideoQualityLevel @dev
Scenario Outline: Test eVideoQualityLevel
    Given I have 1 <dev1> and 1 <dev2>
    And I create a venue
    When I create offer on device 1
    And I create offer on device 2
    Then I keep the call for 5 seconds
    Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    Then I keep the call for 5 seconds
    Then I set the following Video parameters for device 1 as: eVideoQualityLevel = sld
    Then I keep the call for 10 seconds
    Then I should have sent and received sld video in devices
    Then I set the following Video parameters for device 1 as: eVideoQualityLevel = sd
    Then I keep the call for 10 seconds
    Then I should have sent and received sd video in devices
    Then I set the following Video parameters for device 1 as: eVideoQualityLevel = ld
    Then I keep the call for 10 seconds
    Then I should have sent and received ld video in devices

Examples:
    | dev1       | dev2       |
    | android    | mac        |
    | mac        | ios        |
    | ios        | android    |


@targetBR @dev
    Scenario Outline: Test Setting target bit-rate
    Given I have 2 any devices
    And   I create a venue
    When  I create <VideoSize> offer on device 1
    And   I create <VideoSize> offer on device 2
    Then  I keep the call for 2 seconds  
    Then  I set MaxWidth as <Width>, MaxHeight as <Height>, MaxFrameRate as <Fps>, MaxSpatialLayer as 3 in device 1
    Then  I set MaxWidth as <Width>, MaxHeight as <Height>, MaxFrameRate as <Fps>, MaxSpatialLayer as 3 in device 2
    Then  I should get SDP from device 1 and create confluence
    And   I should get SDP from device 2 and create confluence
    Then  I keep the call for 20 seconds
    Then  Video resolution should == <Resolution> 
    And   The bit-rate of video bitstream output from device 1 should be in +/- <Percent>% of <TargetBR> Kbps
    Then  I stop the call on device 1
    And   I stop the call on device 2
    Examples:
    | VideoSize | Width  | Height | Fps  | TargetBR  | Resolution   | Percent  |
    | sld       | 160    | 90     | 6    | 64        | 90p          |  10      |
    | ld        | 320    | 180    | 12   | 180       | 180p         |  10      |
    |           | 640    | 360    | 24   | 640       | 360p         |  10      |


@VideoEncoderConfigure @dev
Scenario Outline: Test VideoEncoderConfigure
    Given I have 1 <dev1> and 1 <dev2>
    And I create a venue
    When I create offer on device 1
    And I create offer on device 2
    Then I keep the call for 5 seconds
    Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    Then I keep the call for 5 seconds
    Then I set MaxWidth as 320, MaxHeight as 180, MaxFrameRate as 1500, in device 1
    Then I keep the call for 5 seconds
    Then I should receive the following value from device 1, Video parameter: iMaxWidth == 320
    Then I should receive the following value from device 1, Video parameter: iMaxHeight == 180
    Then I should receive the following value from device 1, Video parameter: iMaxFrameRate == 15
    Then I should sent video with MaxWidth == 320, MaxHeight == 180, MaxFrameRate == 15
    Then I set MaxWidth as 640, MaxHeight as 360, MaxFrameRate as 3000, in device 1
    Then I keep the call for 5 seconds
    Then I should receive the following value from device 1, Video parameter: iMaxWidth == 640
    Then I should receive the following value from device 1, Video parameter: iMaxHeight == 360
    Then I should receive the following value from device 1, Video parameter: iMaxFrameRate == 30
    Then I should sent video with MaxWidth == 640, MaxHeight == 360, MaxFrameRate == 30

Examples:
    | dev1       | dev2       |
    | android    | mac        |
    | mac        | ios        |
    | ios        | android    |

@VideoMaxCapability @dev
Scenario Outline: Test Video Max Capability
    Given I have 1 <dev1> and 1 <dev2>
    And I create a venue
    When I create offer on device 1
    And I create offer on device 2
    Then I keep the call for 5 seconds
    Then I should get SDP from device 1 and create confluence
    Then I should get SDP from device 2 and create confluence
    Then I keep the call for 5 seconds

    Then I set max video capability with params: <uPLID> <uMMBPS> <uMFS> <uMFPS> <uMBR> <uMNUS> on device 1
    Then I keep the call for 5 seconds
    Then I check max video capability setting result <uMFS> <uMBR> on device 1

    Then I stop the call on device 1
    Then I stop the call on device 2
 Examples:
    |dev1    |dev2    |uPLID   |uMMBPS |uMFS |uMFPS |uMBR     |uMNUS |
    |ios     |mac     |4382740 |11880  |396  |0     | 2000000 |1000  |
    |ios     |mac     |4325406 |40500  |1620 |0     |10000000 |1000  |
    |mac     |android |4382740 |11880  |396  |0     | 2000000 |1000  |
    |mac     |android |4325406 |40500  |1620 |0     |10000000 |1000  |
    |android |ios     |4382740 |11880  |396  |0     | 2000000 |1000  |
    |android |ios     |4325406 |40500  |1620 |0     |10000000 |1000  |

    