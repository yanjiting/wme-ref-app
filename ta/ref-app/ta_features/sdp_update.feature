Feature: SDP update during a call

@sdp_update_p2p @dev
Scenario: SDP update during a p2p call through any 2 devices
  Given I have 1 mobile and 1 pc
  When I create offer on device 1 with the default setting and:
    | ICE     | true  |
    And I set offer to device 2 with the default setting and:
    | ICE     | true  |
  Then I should receive answer from device 2 and set to device 1  
  Then I keep the call for 5 seconds  
    And I should have received and decoded some media in devices  
  Then I hold the call for device 1
    And I get the offer from device 1 to device 2
  Then I keep the call for 5 seconds
    And device 1 should not be receiving audio data
    And device 1 should not be receiving video data
  Then I unhold the call for device 1
    And I get the offer from device 1 to device 2
  Then I keep the call for 5 seconds
    And I should have received and decoded some media in devices

@sdp_update_linus @intensive @deliver
Scenario: SDP update during a linus call through any 2 devices
  Given I have 2 any devices
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 8 seconds
    And I should have received and decoded some media in devices  
  Then I hold the call for device 1
    And I should get SDP from device 1 and update confluence
  Then I keep the call for 5 seconds
    And device 1 should not be receiving audio data
    And device 1 should not be receiving video data
  Then I unhold the call for device 1
    And I should get SDP from device 1 and update confluence
  Then I keep the call for 5 seconds
    And I should have received and decoded some media in devices
