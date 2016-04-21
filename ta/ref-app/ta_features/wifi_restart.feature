Feature: network Test WME call in different network scenario

@disable-wifi @sanity  
Scenario: android loopback mode when wifi disable and enable during a call
  Given I have 1 android device
  When I start loopback call
  Then I keep the call for 8 seconds
    And I disable wifi
  Then I keep the call for 1 seconds 
    And I enable wifi 
  Then I wait for wifi enable
  Then I keep the call for 10 seconds 
  Then I should continue to receive some media in device
  Then I check file size of each trace in a call lasting 0 seconds
