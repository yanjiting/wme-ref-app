Feature: Test IOS backdoor

@dev @ios
Scenario Outline: Test IOS backdoor
	Given I have 1 <dev1> and 1 <dev2>
    And I create a venue
  	When I create offer on device 1
    And I create offer on device 2
  	Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  	Then I keep the call for 5 seconds  
  	Then I should have received and decoded some media in devices
  	
  	Then I test IOS backdoor on device 1
  	Then I stop the call on device 1
	Then I stop the call on device 2
 
   Examples:
    | dev1       | dev2       |
    | ios    	 | android    |
