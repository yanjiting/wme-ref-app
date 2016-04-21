Feature: Test video quality with comparing file capture and render

@ios-videoquality @dev
Scenario: iOS video quality with file capture and render
        Given I have 1 ios device
        When I start loopback sd loopFile timestamp call
        Then I keep the call for 10 seconds
        Then I stop the call on device 1
        Then I got good quality of sd video 0 with received psnr > 32, average psnr > 30 and loss frame rate < 25%

@android-videoquality @dev
Scenario: Android video quality with file capture and render
        Given I have 1 android device
        When I start loopback sd loopFile timestamp call
        Then I keep the call for 10 seconds
        Then I stop the call on device 1
        Then I got good quality of sd video 0 with received psnr > 32, average psnr > 30 and loss frame rate < 25%

@macosx-videoquality @dev
Scenario: MAC video quality with file capture and render
        Given I have 1 mac device
        When I start loopback sd loopFile timestamp call
        Then I keep the call for 10 seconds
        Then I stop the call on device 1
        Then I got good quality of sd video 0 with received psnr > 32, average psnr > 30 and loss frame rate < 25%

@macosx-videoquality-bitrate @dev
Scenario Outline: MAC video quality with file capture and render
		Given I have 1 mac device
		And I setup <direction> network simulator with bw=<bandwidth>Kbps, qdelay=<qdelay>ms and delay=<delay>ms for the pc device
		When I start loopback loopfile call on device 1 with:
			| files  | {"video": {"source": "video_1280_720_30_i420.yuv", "loop": true, "timestamp": true} }		|
			| global | {"loopback": true, "timestamp": true, "filemode":true}				|
			| video  | {"uVideoDataDumpFlag" : 2} |
			| feature| {"audio": false, "video": true, "share": "" }  |
		Then I keep the call for 60 seconds
		Then I stop the call on device 1
		Then I clear network simulator rules
		Then I got good quality of <bitrate>Kbps video from device 1, file name is:
			| video_1280_720_30_i420.yuv |
Examples:
	| direction	 | bandwidth  | qdelay     | delay      | bitrate    |
    | uplink     | 512        | 0          | 0          | 512        |
    | uplink     | 768        | 0          | 0          | 768        |
    | uplink     | 1024       | 0          | 0          | 1024       |
		
@mac-ios-videoquality @dev
Scenario: MAC and iOS call video quality with file capture and render
		Given I have 1 pc and 1 mobile
		And I create a venue
		When I create offer on 1st device with options and params:
			| global | {"loopback": true, "timestamp": true, "filemode":true}				|
			| files  | {"video": {"source": "video_640_360_24_i420.yuv", "loop": true, "timestamp": true} }		|
    	When I create offer on 2nd device with options and params:
			| global | {"loopback": true, "timestamp": true, "filemode":true}				|
			| files  | {"video": {"source": "video_640_360_24_i420.yuv", "loop": true, "timestamp": true} }		|
		Then I should get SDP for each device and create confluence
		Then I keep the call for 5 seconds
		Then I stop the call on all devices

@ios-videoquality-limited-bw @dev
Scenario Outline: iOS video quality test under limited network condition
        Given I have 1 ios device
        And I setup <direction> network simulator with bw=<bandwidth>Kbps for device 1
        When I start loopback sd loopFile timestamp call
        Then I keep the call for 60 seconds
        Then I stop the call on device 1
        Then I clear network simulator rules
        Then I got good quality of sd video <idx> with received psnr > <psnr-r>, average psnr > <psnr-a> and loss frame rate < <rate>%

Examples:
    | direction	 | bandwidth  | idx        | psnr-r     | psnr-a     | rate       |
    | uplink     | 350        | 1          | 25         | 21         | 70         |
    | uplink     | 500        | 2          | 30         | 27         | 60         |
    | uplink     | 1000       | 3          | 32         | 30         | 40         |
    | downlink   | 350        | 4          | 25         | 21         | 70         |
    | downlink   | 500        | 5          | 30         | 27         | 60         |
    | downlink   | 1000       | 6          | 32         | 30         | 40         |
