Feature:try to start a call between iOS and iOS
  Scenario:Create call between iOS and iOS
    Given I have 2 ios devices
     And I create a venue
    When I create offer on device 1
     And I create offer on device 2
    Then I keep the call for 5 seconds
    Then I should get SDP from device 1 and create confluence
     And  I should get SDP from device 2 and create confluence
    #Then I delete signal file
    Then I call remote windows to run malden test
    Then I wait for signal file to exist
    Then I keep the call for 20 seconds

#Scenario:Create call between iOS and iOS
#    Given I have 2 ios devices
#    When I create offer on device 1 with the default setting
#    Then I keep the call for 5 seconds

  
