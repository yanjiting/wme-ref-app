Feature: Ensure quality with comparing file capture and render

@ios-filecapture @sanity
Scenario: iOS quality with file capture and render
        Given I have 1 ios device
        When I start loopback sld file call
        Then I wait for file capture ended
          And I got audio and sld video render files
        Then I check file size of each trace in a call lasting 0 seconds

@android-filecapture @sanity
Scenario: android quality with file capture and render
        Given I have 1 android device
        When I start loopback sld file call
        Then I wait for file capture ended
          And I got audio and sld video render files
        Then I check file size of each trace in a call lasting 0 seconds

@macosx-filecapture @sanity @pc
Scenario: macosx quality with file capture and render
	Given I have 1 mac device
        When I start loopback sld file call
        Then I wait for file capture ended
          And I got audio and sld video render files
        Then I check file size of each trace in a call lasting 0 seconds

@win-filecapture @sanity @pc
Scenario: Windows quality with file capture and render
	Given I have 1 windows device
        When I start loopback sld file call
        Then I wait for file capture ended
          And I got audio and sld video render files
        Then I check file size of each trace in a call lasting 0 seconds

@file-2device @dev
Scenario: a call with 2 devices, one is from file capture and render
  Given I have 1 pc and 1 mobile
  When I create loopFile offer on device 1
    And I set offer to device 2 with the default setting and:
    | ICE     | true   |
    | RTCPMUX | true   |
  Then I should receive answer from device 2 and set to device 1  
  Then I set the following Audio parameters for device 2 as: enableKeyDumpFile = 5000
  Then I keep the call for 8 seconds  
    And I got files on 2 device:
		|AECOutput-float32-Chn1-16000.pcm       | 370560 |

@squareclienttestecho-2device @dev
Scenario: test echo
	Given I have 1 ios and 1 mac
	When sq create offer on device 1
	And I set offer to device 2 with the default setting and:
	| ICE | true |
	| RTCPMUX | true |
	Then I should receive answer from device 2 and set to device 1
	Then I set the following Audio parameters for device 2 as: enableKeyDumpFile = 5000
	Then I keep the call for 5 seconds
	And I got compare echo files on 1 device:
	|AECOutput-short16-Chn1-16000.wav | 0 |

@squareclientgetmosvalue-2device @dev
Scenario: calculate mos value
    Given I have 1 ios and 1 mac
    When sq create offer to calculate mos value on device 1
    And I set offer to device 2 with the default setting and:
    | ICE | true |
    | RTCPMUX | true |
    Then I should receive answer from device 2 and set to device 1
    Then I set the following Audio parameters for device 2 as: enableKeyDumpFile = 8000
    Then I keep the call for 8 seconds
    And I got calculate mos value on 2 device:
    |Playback-short16-Chn1-16000.wav | 0 |


