Feature: Test rotate mobile devices, iOS rotate devices, only support left, right and down, Test Android rotation with updated rendering
  	
@rotate-camera @sanity @mac @weichen2
Scenario: Test camera rotation with updated rendering
	Given I have 2 android devices
    And I create a venue
  	When I create offer on device 1
    And I create offer on device 2
  	Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  	Then I keep the call for 5 seconds  
  	Then I should have received and decoded some media in devices
  	
  	Then I set device 1 to default portrait orientation
  	Then I set device 2 to default portrait orientation
  	Then I keep the call for 3 seconds
  	Then I set the correct render mode to device 1 while being in portrait mode
  	Then I set the correct render mode to device 2 while being in portrait mode
  	Then I keep the call for 3 seconds
  	Then I check if remote rendering surface in device 2 is using CropFill
  	Then I check if remote rendering surface in device 1 is using CropFill
  	Then I keep the call for 5 seconds
  	Then I should have received and decoded some media in devices
  	
  	Then I rotate camera on device 1 
  	Then I keep the call for 3 seconds
  	Then I set the correct render mode to device 1 while being in landscape mode
  	Then I set the correct render mode to device 2 while being in portrait mode 	
  	Then I keep the call for 3 seconds 	
  	Then I check if remote rendering surface in device 1 is using LetterBox
  	Then I check if remote rendering surface in device 2 is using LetterBox
  	Then I keep the call for 5 seconds 
  	Then I change the display port on device 1 to portrait
  	Then I keep the call for 5 seconds 
  	Then I should have received and decoded some media in devices

  	Then I rotate camera on device 1 
  	Then I keep the call for 3 seconds
  	Then I set the correct render mode to device 1 while being in portrait mode
  	Then I set the correct render mode to device 2 while being in portrait mode
  	Then I keep the call for 3 seconds
  	Then I check if remote rendering surface in device 1 is using CropFill
  	Then I check if remote rendering surface in device 2 is using CropFill
  	Then I keep the call for 5 seconds 
  	Then I should have received and decoded some media in devices
  	
	Then I rotate camera on device 1 
  	Then I keep the call for 3 seconds
  	Then I set the correct render mode to device 1 while being in landscape mode
  	Then I set the correct render mode to device 2 while being in portrait mode	
  	Then I keep the call for 3 seconds
  	Then I check if remote rendering surface in device 1 is using LetterBox
  	Then I check if remote rendering surface in device 2 is using LetterBox
  	Then I keep the call for 5 seconds 
  	Then I change the display port on device 1 to portrait
  	Then I keep the call for 5 seconds 
  	Then I should have received and decoded some media in devices

	Then I set device 1 to default portrait orientation
  	Then I set device 2 to default portrait orientation
  	Then I stop the call on device 1
  	Then I stop the call on device 2
  
  Then I check file size of each trace in a call lasting 0 seconds

@rotate1device @sanity @mac
Scenario Outline: rotate 1 ios device
  Given I have 1 ios devices
  When I start loopback p2p call
  Then I keep the call for 10 seconds 
  Then I record current local mode on device 1

  Then I rotate <rotation>
  Then I keep the call for 10 seconds 
  Then I check loopback-call <rotation> result on device 1

  Then I should have received and decoded some media in devices
  Then I stop the call on device 1
  Then I check file size of each trace in a call lasting 0 seconds

  Examples:
  | rotation |
  | left     |
  | down     |
  | right    |
