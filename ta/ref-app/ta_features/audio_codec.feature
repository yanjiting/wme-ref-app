Feature: switch audio codec

@audiocodecswitch @dev
Scenario Outline: switch audio codec on 1 device
  Given I have 1 <OS> devices
  When I start loopback p2p call
  Then I keep the call for 5 seconds 

  Then I set audio codec <type> on device 1
  Then I keep the call for 5 seconds 
  Then I check audio codec <value> set result via device 1
  Then I stop the call on device 1

  Examples:
  |OS      |type    |value |
  |android |PCMU    |1     |
  |android |PCMA    |2     |
  |android |opus    |4     |
  |android |G722    |5     |
  |ios     |PCMU    |1     | 
  |ios     |PCMA    |2     |
  |ios     |opus    |4     |
  |ios     |G722    |5     |
  |mac     |PCMU    |1     |
  |mac     |PCMA    |2     |
  |mac     |opus    |4     |
  |mac     |G722    |5     |
