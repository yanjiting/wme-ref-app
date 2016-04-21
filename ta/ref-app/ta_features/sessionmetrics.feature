Feature: Test the media session metrics

@metrics_1 @newtest 
Scenario: Make a call and test the cpu metrics
  Given I have 1 pc and 1 mobile
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 8 seconds  
  Then I should have received and decoded some media in devices
  Then I check the CPU metrics