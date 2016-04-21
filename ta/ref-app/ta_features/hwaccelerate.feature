Feature: HW acceleration enable and android whitelist

  @hw-acc-android @android-whitelist @dev
  Scenario Outline: Make a loopback call with hw acceleration and whitelist on android
   Given I have 1 android device
   Then I check if current device 1, supports HW Acceleration with the following device from WDM whitelist, <@devices>
   Then I start loopback call with: 
      |video|{"bHWAcceleration": true}|
   Then I keep the call for 5 seconds
   Then I check the HW acceleration result from video statistics on device 1
   Then I stop the call on device 1 
   Examples:
   | @devices |
   | LG-D855  |
   | SM-G900FD|
   | SM-G900  |
   | GT-I9500 |
   | Nexus 5  |
   | Nexus 7  |

 @hw-acc-ios-mac @dev
 Scenario Outline: Make a loopback p2p call with hw acceleration on ios and mac
    Given I have 1 <os> device
    Then I check current <os> device 1 supports HW acceleration in advance
    Then I start loopback p2p call with: 
      |video|{"bHWAcceleration": true}|
    Then I keep the call for 20 seconds
    Then I should have received and decoded some media in devices
    Then I check the HW acceleration result from video statistics on device 1
    Then I stop the call on device 1 
Examples:
    |os |
    |ios|
    #|mac|    
   
 @hw-ios-rotate @dev
 Scenario: Make a loopback call with hw acceleration on ios, check status when rotation
    Given I have 1 ios device
    Then I check current ios device 1 supports HW acceleration in advance
    Then I start loopback call with:
      |video|{"bHWAcceleration": true}|
    Then I rotate device 1 10 times every 5 seconds
	Then I stop the call on device 1

@hw-android-rotate @dev
 Scenario: Make a loopback call with hw acceleration on android, check status when rotation
    Given I have 1 android device
    Then I start loopback call with:
      |video|{"bHWAcceleration": true}|
    Then I rotate camera on device 1 10 times every 5 seconds
	Then I stop the call on device 1

@p2p-hardware-disableQos-ios @dev @sanity
Scenario: Test hardware on ios in p2p call
    Given I have 1 ios and 1 pc
    Then I check current ios device 1 supports HW acceleration in advance
    And I create a venue
    When I create disableQos offer on device 1 with:
    |video|{"bHWAcceleration": true}|
    And I create disableQos offer on device 2
    Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    Then I subscribe hd video on device 2
    Then I keep the call for 1 seconds
    Then I keep the call for 15 seconds  
    Then Device 2 should have decoded some media with videoPixels >= 921600 
    Then Device 1 should have decoded some media with videoPixels >= 921600 
    Then I check file size of each trace in a call lasting 0 seconds

@p2p-hardware-disableQos-android @dev
Scenario Outline: Test hardware on android in p2p call
    Given I have 1 android and 1 pc
    Then I check if current device 1, supports HW Acceleration with the following device from WDM whitelist, <@device>
    And I create a venue
    When I create disableQos offer on device 1 with:
    |video|{"bHWAcceleration": true}|
    And I create disableQos offer on device 2
    Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    Then I subscribe hd video on device 2
    Then I keep the call for 1 seconds
    Then I keep the call for 15 seconds  
    Then Device 2 should have decoded some media with videoPixels >= 921600 
    Then Device 1 should have decoded some media with videoPixels >= 921600 
    Examples:
   | @device  |
#   | LG-D855  |
   | SM-G900FD|
#   | SM-G900  |
#   | GT-I9500 |
#   | Nexus 5  |
#   | Nexus 7  |

