Feature: Test Android long calls, and test perpetually stopping and starting the call
  	
@start-stop-frequently @intensive
Scenario: Test Start/Stop frequently
	Given I have 2 mobile devices
      And I create a venue
  	When I create offer on device 1
    And I create offer on device 2
  	Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  	Then I keep the call for 5 seconds  
  	Then I should have received and decoded some media in devices
  	
  	Then I stop the call on device 1
  	Then I stop the call on device 2
	Then I recreate a venue
  	When I create offer on device 1
    And I create offer on device 2
    Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    Then I keep the call for 3 seconds 
  	Then I should have received and decoded some media in devices
    
  	Then I stop the call on device 1
  	Then I stop the call on device 2
	Then I recreate a venue
  	When I create offer on device 1
    And I create offer on device 2
    Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    Then I keep the call for 3 seconds 
  	Then I should have received and decoded some media in devices

  	Then I stop the call on device 1
  	Then I stop the call on device 2
	Then I recreate a venue
  	When I create offer on device 1
    And I create offer on device 2
    Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    Then I keep the call for 3 seconds 
  	Then I should have received and decoded some media in devices

  	Then I stop the call on device 1
  	Then I stop the call on device 2
	Then I recreate a venue
  	When I create offer on device 1
    And I create offer on device 2
    Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    Then I keep the call for 3 seconds 
  	Then I should have received and decoded some media in devices
	
@long-calls @intensive
Scenario: Test long call
	Given I have 2 android devices
      And I create a venue
  	When I create offer on device 1
    And I create offer on device 2
  	Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  	Then I keep the call for 5 seconds  	
  	Then I should have received and decoded some media in devices
  	Then I keep the call for 120 seconds
  	Then I should have received and decoded some media in devices
  	Then I keep the call for 120 seconds
  	Then I should have received and decoded some media in devices
  	Then I keep the call for 120 seconds
  	Then I should have received and decoded some media in devices
  	Then I keep the call for 120 seconds
  	Then I should have received and decoded some media in devices
  	Then I keep the call for 120 seconds
  	Then I should have received and decoded some media in devices
  	Then I keep the call for 120 seconds
  	Then I should have received and decoded some media in devices	
  	Then I keep the call for 120 seconds
  	Then I should have received and decoded some media in devices	
  	Then I keep the call for 120 seconds
  	Then I should have received and decoded some media in devices	
  	Then I keep the call for 120 seconds
  	Then I should have received and decoded some media in devices	
  	Then I keep the call for 120 seconds
  	Then I should have received and decoded some media in devices		
  	Then I stop the call on device 1
  	Then I stop the call on device 2