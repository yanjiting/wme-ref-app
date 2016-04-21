Feature: Test Android Audio Features

@speaker-earpiece @sanity
Scenario: Test Android Speaker/Earpiece mode
	Given I have 2 android devices
    And I create a venue
  	When I create offer on device 1
    And I create offer on device 2
  	Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  	Then I keep the call for 8 seconds  
  	Then I should have received and decoded some media in devices
  	
  	Then I set the audio playback on device 1 to Earpiece
  	Then I keep the call for 1 seconds  
  	Then I set the audio playback on device 1 to Speaker
  	Then I keep the call for 1 seconds  
  	
  	Then I set the audio playback on device 2 to Earpiece
  	Then I keep the call for 1 seconds  
  	Then I set the audio playback on device 2 to Speaker
  	Then I keep the call for 1 seconds  
  	
  	Then I stop the call on device 1
  	Then I stop the call on device 2
    Then I check file size of each trace in a call lasting 0 seconds

  	
