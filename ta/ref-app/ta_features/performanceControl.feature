Feature: Test Dynamic Performance Controller on PC

@performcontrol-mac-multi @pc @mac @dev
Scenario: Establish a call on Mac, manipulate CPU usage, and monitor performance on runtime	
	Given I have 3 any devices
    And I create a venue
    When I create offer on 1st device with options and params:
	| global | {"videoStreams": 2, "multilayer": true, "enableQos": false, "ice": true}	|
	| feature| {"audio": true, "video": true, "share": "" }					|
	| video  | {"selectedCodec" : ["H264-SVC"], "enableAVCSimulcast": false}|
    When I create offer on 2nd device with options and params:
	| global | {"videoStreams": 2, "multilayer": true, "enableQos": false, "ice": true}	|
	| feature| {"audio": true, "video": true, "share": "" }					|
	| video  | {"selectedCodec" : ["H264-SVC"], "enableAVCSimulcast": false}|
    When I create offer on 3rd device with options and params:
	| global | {"videoStreams": 2, "multilayer": true, "enableQos": false, "ice": true}	|
	| feature| {"audio": true, "video": true, "share": "" }					|
	| video  | {"selectedCodec" : ["H264-SVC"], "enableAVCSimulcast": false}|
    Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    And I should get SDP from device 3 and create confluence	
    Then I check dynamic performance when the CPU usage is changed in mac. Test will take about ~2 minutes
  	Then I stop the call on device 1
  	Then I stop the call on device 2
  	Then I stop the call on device 3		

@performcontrol-win-multi @pc @win @dev
Scenario: Establish a call on Win, manipulate CPU usage, and monitor performance on runtime
	Given I have 3 any devices
	And I create a venue
	When I create offer on 1st device with options and params:
	| global | {"videoStreams": 2, "multilayer": true, "enableQos": false, "ice": true}	|
	| feature| {"audio": true, "video": true, "share": "" }					|
	| video  | {"selectedCodec" : ["H264-SVC"], "enableAVCSimulcast": false}|
    When I create offer on 2nd device with options and params:
	| global | {"videoStreams": 2, "multilayer": true, "enableQos": false, "ice": true}	|
	| feature| {"audio": true, "video": true, "share": "" }					|
	| video  | {"selectedCodec" : ["H264-SVC"], "enableAVCSimulcast": false}|
    When I create offer on 3rd device with options and params:
	| global | {"videoStreams": 2, "multilayer": true, "enableQos": false, "ice": true}	|
	| feature| {"audio": true, "video": true, "share": "" }					|
	| video  | {"selectedCodec" : ["H264-SVC"], "enableAVCSimulcast": false}|
  	Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
     And I should get SDP from device 3 and create confluence
	Then I check dynamic performance when the CPU usage is changed in win. Test will take about ~2 minutes
  	Then I stop the call on device 1
  	Then I stop the call on device 2
  	Then I stop the call on device 3	
	

@performcontrol-mac-p2p @mac @dev @sanity
Scenario: Establish a call on Mac, manipulate CPU usage, and monitor performance on runtime	
	Given I have 1 mac and 1 ios
	And I create a venue
	When I create offer on 1st device with options and params:
		|	video	|	{"enableAVCSimulcast" : false}		|
		| 	global 	| 	{"enableQos": false, "ice": true}	|
	When I create offer on 2nd device with options and params:
		|	video	|	{"enableAVCSimulcast" : false}		|
		| 	global 	| 	{"enableQos": false, "ice": true}	|
	Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    Then I check dynamic performance when the CPU usage is changed in mac. Test will take about ~2 minutes
  	Then I stop the call on device 1
  	Then I stop the call on device 2
    Then I check file size of each trace in a call lasting 0 seconds
