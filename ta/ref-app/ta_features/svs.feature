Feature: Ensure SVS feature for train can work

@svs-loop @mac @sanity 
Scenario: SVS and switch to camera video
    Given I have 1 pc device
    When I start loopback svs call
    Then I keep the call for 5 seconds  
    Then I should have decoded some media in devices with:
        |audio | isSvs | true|
    Then I change it back to camera video
    Then I keep the call for 5 seconds  
    Then I should have decoded some media in devices with:
        |audio | isSvs | true|
    
