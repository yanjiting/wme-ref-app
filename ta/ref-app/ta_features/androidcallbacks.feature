Feature: Test Test MediaSession callbacks

@callbacks-sdp-type @newtest
Scenario: Test SDP type after creating confluence
	Given I have 2 android devices
      And I create a venue
  	When I create offer on device 1
    And I create offer on device 2
  	Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  	Then I keep the call for 5 seconds  
  	Then I should have received and decoded some media in devices 	
  	Then device 1 SDP type should be Offer
  	Then device 2 SDP type should be Offer
  	Then I keep the call for 5 seconds
  	Then I stop the call on device 1
  	Then I stop the call on device 2
  	
@callbacks-connectivity @newtest
Scenario: Test connectivity status
	Given I have 2 android devices
      And I create a venue
  	When I create offer on device 1
    And I create offer on device 2
  	Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  	Then I keep the call for 5 seconds  
  	Then I should have received and decoded some media in devices
  	Then device 1 must be connected
  	Then device 2 must be connected
  	Then I stop the call on device 1
  	Then I stop the call on device 2
  	
@callbacks-blocked @newtest
Scenario: Test if media is blocked after confluence is established
 	Given I have 2 android devices
    And I create a venue
  	When I create offer on device 1
    And I create offer on device 2
  	Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  	Then I keep the call for 5 seconds  
  	Then I should have received and decoded some media in devices
  	
  	Then device 1 must NOT be blocked
  	Then device 2 must NOT be blocked
  	
    And I disable wifi
    Then I wait for wifi disable
  	Then I keep the call for 10 seconds 
  	Then device 1 must be blocked
  	Then device 2 must be blocked
  	
    And I enable wifi 
  	Then I wait for wifi enable
  	Then I keep the call for 10 seconds 
  	Then device 1 must NOT be blocked
  	Then device 2 must NOT be blocked
  	
  	Then I stop the call on device 1
    Then I stop the call on device 2  	
  	
@callbacks-media-ready @newtest
Scenario Outline: Test callback MediaSession onMediaReady
	Given I have 1 <dev1> and 1 <dev2>
    And I create a venue
  	When I create offer on device 1
    And I create offer on device 2
  	Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  	Then I keep the call for 5 seconds  
  	Then I should have received and decoded some media in devices
  	
  	Then I keep the call for 5 seconds 
  	Then I check I have all media tracks running on device 1
  	Then I keep the call for 5 seconds 
  	Then I check I have all media tracks running on device 2
  	
  	Then I stop the call on device 1
    Then I stop the call on device 2
    
    Examples:
    | dev1       | dev2       |
    | ios    	 | android    |

  	