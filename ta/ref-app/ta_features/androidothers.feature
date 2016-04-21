Feature: Test other Android features in development

@uMaxPacketSize @dev
Scenario: Test Android uMaxPacketSize
	Given I have 2 android devices	
      And I create a venue
	When I start call without creating offer on device 1 
	When I start call without creating offer on device 2
	Then I keep the call for 5 seconds  
    Then I set the following Video parameters for device 1 as: uMaxPacketSize = 1200
    Then I keep the call for 5 seconds 
    Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    Then I should receive the following value from device 1, Video parameter: uMaxPacketSize == 1200
    
@ePacketizationMode @dev
Scenario: Test Android ePacketizationMode
	Given I have 2 android devices	
      And I create a venue
	When I start call without creating offer on device 1 
	When I start call without creating offer on device 2
	Then I keep the call for 5 seconds  
    Then I set the following Video parameters for device 1 as: ePacketizationMode = 1
    Then I keep the call for 5 seconds 
	Then I should get SDP from device 1 and create confluence
      And I should get SDP from device 2 and create confluence
    Then I should receive the following value from device 1, Video parameter: ePacketizationMode == 1
  	

@mute-unmute @dev
Scenario: Test Android Mute/Unmute
	Given I have 2 android devices
    And I create a venue
  	When I create offer on device 1
    And I create offer on device 2
  	Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  	Then I keep the call for 5 seconds  
  	Then I should have received and decoded some media in devices
  	
  	Then I mute local audio on device 1
  	Then I keep the call for 5 seconds  
  	Then device 2 should not be receiving audio data
  	Then I unmute local audio on device 1
  	Then I keep the call for 5 seconds  
  	Then I should have received and decoded some media in devices
  	
  	Then I pause to send video on device 1
  	Then I keep the call for 5 seconds  
  	Then device 2 should not be receiving video data
  	Then I unpause to send video on device 1
  	Then I keep the call for 5 seconds  
  	Then I should have received and decoded some media in devices
  	
  	Then I stop the call on device 1
  	Then I stop the call on device 2  