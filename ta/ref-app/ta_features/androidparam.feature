Feature: Test Android Parameters
   
@androidparam @dev
Scenario: Test Android enableQos
	Given I have 2 android devices	
      And I create a venue
	When I start call without creating offer on device 1 
	When I start call without creating offer on device 2
	Then I keep the call for 5 seconds  
	Then I create my SDP offer for my device 1
    Then I create my SDP offer for my device 2  
	Then I set the following Video parameters for device 1 as: enableQos = true
    Then I keep the call for 5 seconds 
    Then I should receive the following value from device 1, Video parameter: enableQos == true
    Then I stop the call on device 1
  	Then I stop the call on device 2  

@androidparam @dev
Scenario: Test Android uMaxBandwidth
	Given I have 2 android devices	
      And I create a venue
	When I start call without creating offer on device 1 
	When I start call without creating offer on device 2
	Then I keep the call for 5 seconds  
    Then I set the following Video parameters for device 1 as: uMaxBandwidth = 140000
    Then I keep the call for 5 seconds 
    Then I should receive the following value from device 1, Video parameter: uMaxBandwidth == 140000
	Then I create my SDP offer for my device 1
    Then I create my SDP offer for my device 2  
    Then I stop the call on device 1
  	Then I stop the call on device 2  
  
@androidparam @dev
Scenario: Test Android iMaxWidth
	Given I have 2 android devices	
      And I create a venue
	When I start call without creating offer on device 1 
	When I start call without creating offer on device 2
	Then I keep the call for 5 seconds  
   	Then I set MaxWidth as 640, MaxHeight as 360, MaxFrameRate as 26, in device 1
    Then I keep the call for 5 seconds 
    Then I should receive the following value from device 1, Video parameter: iMaxWidth == 640
    Then I should receive the following value from device 1, Video parameter: iMaxHeight == 360
    Then I should receive the following value from device 1, Video parameter: iMaxFrameRate == 26
    Then I create my SDP offer for my device 1
    Then I create my SDP offer for my device 2  
    Then I stop the call on device 1
  	Then I stop the call on device 2
  	
@androidparam @dev
Scenario: Test Android uProfileLevelID
	Given I have 2 android devices	
      And I create a venue
	When I start call without creating offer on device 1 
	When I start call without creating offer on device 2
	Then I keep the call for 5 seconds  
    Then I set the following Video parameters for device 1 as: uProfileLevelID = 42002A
    Then I keep the call for 5 seconds 
    Then I should receive the following value from device 1, Video parameter: uProfileLevelID == 42002A
    Then I create my SDP offer for my device 1
    Then I create my SDP offer for my device 2    
     
    
