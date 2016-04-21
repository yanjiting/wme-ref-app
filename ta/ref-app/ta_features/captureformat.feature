Feature: set capture format to bgra

@capture-format @dev @pc
Scenario Outline: Make a call with capture format rgba on ios, one ios and one PC
  Given I have 1 ios and 1 mac
    And I create a venue
  When I create offer on device 1 with BGRA=<BgraEnable>
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 10 seconds  
  Then I should have received and decoded some media in devices
  
  Examples:
    | BgraEnable |
    | true       | 
