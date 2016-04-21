Feature: Test EC in 2 devices

@ec_enable_check @lab-sanity
Scenario Outline: Make a p2p call between 2 devices with packet lost to enable EC and verify stability
   Given I have 1 <dev1> and 1 <dev2>     
   And I create a venue
   And I setup uplink network simulator with loss rate=10% for device 1
   And I setup uplink network simulator with loss rate=10% for device 2 
   When I create offer on device 1
   And I create offer on device 2
   Then I should get SDP from device 1 and create confluence
   And I should get SDP from device 2 and create confluence
   Then I keep the call for 30 seconds
   Then I check EC status on device 1 when enable
   Then I clear network simulator rules
  
Examples:
    | dev1       | dev2       |
    | ios        | android    |
   

@ec_disable_check @dev
Scenario Outline: Make a p2p call between 2 devices with limit network and disable EC then verify stability
   Given I have 1 <dev1> and 1 <dev2>   
   And I create a venue  
   And I setup uplink network simulator with loss rate=10% for device 1
   And I setup uplink network simulator with loss rate=10% for device 2 
   When I create offer on device 1
   And I create offer on device 2
   Then I set the following Video parameters for device 1 as: enableDecoderMosaic = false
   Then I set the following Video parameters for device 2 as: enableDecoderMosaic = false  
   Then I should get SDP from device 1 and create confluence
   And I should get SDP from device 2 and create confluence
   Then I keep the call for 30 seconds
   Then I check EC status on device 1 when disable
   Then I clear network simulator rules

Examples:
    | dev1       | dev2       |
    | ios        | android    |
   

@ec_disable_test @dev
Scenario: Make a p2p call between 2 devices with limit network and disable EC then verify stability
   Given I have 2 ios devices    
   And I create a venue  
   And I setup uplink network simulator with loss rate=10% for device 1
   And I setup uplink network simulator with loss rate=10% for device 2 
   When I create offer on device 1
   And I create offer on device 2
   Then I set the following Video parameters for device 1 as: enableDecoderMosaic = false
   Then I set the following Video parameters for device 2 as: enableDecoderMosaic = false
   Then I should get SDP from device 1 and create confluence
   And I should get SDP from device 2 and create confluence
   Then I keep the call for 30 seconds
   Then I check EC status on device 1 when disable
   Then I clear network simulator rules
   



