Feature: Test SRTP in 2 devices

@srtp—linus-degrade-01 @intensive
Scenario Outline: Make a call with srtp linus
  Given I have 1 <os1> and 1 <os2>
    And I create a venue
    And I create srtp offer on device 1
    And I create srtp offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 15 seconds
  Then I should have received and decoded some media in devices

  Examples:
    | os1     | os2     |
    | android | ios     |
  
@srtp-p2p-degrade-01 @intensive
Scenario Outline: Make a call with srtp p2p
  Given I have 1 <os1> and 1 <os2>
  When I create offer on device 1 with the default setting and:
    | SRTP     | <srtpEnable1>   |
    And I set offer to device 2 with the default setting and:
    | SRTP     | <srtpEnable2>   |
  Then I should receive answer from device 2 and set to device 1    
  Then I keep the call for 15 seconds  
  Then I should have received and decoded some media in devices

  Examples:
    | os1     | os2     | srtpEnable1 | srtpEnable2 |
    | android | ios     | true        | true        |
    
