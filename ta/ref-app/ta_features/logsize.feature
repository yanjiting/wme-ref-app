Feature: Test Log Size

@logsize @pc @intensive @sanity
Scenario: Check log size
	Given I have 3 any devices
    And I create a venue
    When I create offer on 1st device with options and params:
	| global | {"videoStreams": 2, "multilayer": true, "ice": true}	|			
    When I create offer on 2nd device with options and params:
	| global | {"videoStreams": 2, "multilayer": true, "ice": true}	|
	When I create offer on 3rd device with options and params:
	| global | {"videoStreams": 2, "multilayer": true, "ice": true}	|
    Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    And I should get SDP from device 3 and create confluence
    Then I check file size of each trace in a call lasting 240 seconds, should be under 15 MBs
  	Then I stop the call on device 1
  	Then I stop the call on device 2
  	Then I stop the call on device 3		
  	
