Feature: Test basic call feature for train release

@train-p2p @train @intensive @pc
Scenario: train p2p call on pc
    Given I have 1 pc device
    When I start loopback p2p train call
    Then I keep the call for 8 seconds  
    Then I should have received and decoded some media in devices
	
@t-voicelevel @train @intensive @pc	
Scenario: test voice level on pc
    Given I have 1 pc device
    When I start loopback p2p train call
    Then I keep checking voice level for 18 seconds
    Then I should have received and decoded some media in devices	