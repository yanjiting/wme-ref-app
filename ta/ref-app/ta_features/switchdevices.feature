Feature: Switch devices

@switchcamera @dev @deliver
Scenario Outline: Make a loopback call then switch device to check whether it works well
  Given I have 1 <OS> device
  When I start loopback p2p call
  Then I keep the call for 5 seconds  
  Then I switch to back camera
  Then I keep the call for 5 seconds
  Then I switch to front camera
  Then I keep the call for 5 seconds
  Then I should have received and decoded some media in devices
  
  Examples:
    | OS       |
    | android  |
    | ios      |

@switchcamera @dev @sijia
Scenario Outline: Make a loopback call then switch device back-forward whether it works well
  Given I have 1 <OS> device
  When I start loopback p2p call
  Then I keep the call for 5 seconds  
  Then I switch to back camera
  Then I keep the call for 5 seconds
  Then I switch to front camera
  Then I keep the call for 5 seconds
  Then I switch to back camera
  Then I keep the call for 5 seconds
  Then I should have received and decoded some media in devices
  
  Examples:
    | OS       |
    | android  |
    | ios      |

@switchcamera @dev @sijia
Scenario Outline: Make a loopback call then switch device quick whether it works well
  Given I have 1 <OS> device
  When I start loopback p2p call
  Then I keep the call for 5 seconds  
  Then I switch to back camera
  Then I switch to front camera
  Then I keep the call for 5 seconds
  Then I should have received and decoded some media in devices
  
  Examples:
    | OS       |
    | android  |
    | ios      |
