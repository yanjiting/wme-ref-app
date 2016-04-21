Feature: group call with calliope

@groupcall—activespeaker @intensive
Scenario Outline: 2 iOS in calliope call.
        Given I have 1 ios and 1 pc
          And I create a venue
        When I create multilayer offer on device 2
          And I create multilayer offer on device 1
        Then I should get SDP from device 2 and create confluence
        Then I should get SDP from device 1 and create confluence
        Then I keep the call for 5 seconds
        Then I should have received and decoded some media in devices
        And I subscribe specific layers video with <Parameters>
        Then I keep the call for 15 seconds
        Then I should received specific layers video less then <Result>
        Examples:
            | Parameters                    | Result |
            |mbps 450 fs 60 fps 1500 dpb 0        |180p    |
            |mbps 3600 fs 240 fps 1500 dpb 0      |360p    |
            |mbps 3600 fs 240 fps 3000 dpb 0      |360p    |
            |mbps 27600 fs 920 fps 1500 dpb 0     |720p    |
            |mbps 27600 fs 920 fps 2400 dpb 0     |720p    |
            |mbps 27600 fs 920 fps 3000 dpb 0     |720p    |
            |mbps 3000 fs 396 fps 3000 dpb 0      |180p    |
            |mbps 7200 fs 396 fps 3000 dpb 0      |360p    |
            |mbps 27600 fs 920 fps 3000 dpb 0     |720p    |
            |mbps 27600 fs 1200 fps 3000 dpb 0    |720p    |

@gc-simulcast-disableQos @dev @sanity 
Scenario: SVC simulcast in group call.
    Given I have 3 any devices
    And I create a venue
    When I create disableQos offer on device 3
    And I create disableQos offer on device 2
    And I create disableQos offer on device 1
    Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    And I should get SDP from device 3 and create confluence
    Then I subscribe sld video on device 1
    Then I subscribe sd video on device 2
    Then I keep the call for 1 seconds
    Then I mute local audio on device 1
    Then I mute local audio on device 2
    Then I keep the call for 15 seconds  
    Then Device 3 should have encoded with encodedLayers == 2
    Then Device 1 should have decoded some media with videoPixels >= 14400 
    Then Device 2 should have decoded some media with videoPixels >= 230400
    Then I check file size of each trace in a call lasting 0 seconds

@simul-hardware-disableQos @dev @sanity
Scenario: Test hardware on ios in group call
    Given I have 3 any devices
    And  I create a venue
    And  I create disableQos offer on device 2 with:
         |video|{"bHWAcceleration": true}|
    And  I create disableQos offer on device 1
    And  I create disableQos offer on device 3
    Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    And I should get SDP from device 3 and create confluence
    Then I subscribe hd video on device 3
    Then I subscribe sd video on device 1
    Then I keep the call for 1 seconds
    Then I mute local audio on device 3
    Then I pause to send video on device 3
    Then I mute local audio on device 1
    Then I pause to send video on device 1
    Then I keep the call for 15 seconds  
    Then Device 2 should have encoded with encodedLayers == 2
    Then Device 3 should have decoded some media with videoPixels >= 921600 
    Then Device 1 should have decoded some media with videoPixels >= 230400
    Then I check file size of each trace in a call lasting 0 seconds

@mac-simulcast-gc-multiscr @dev
Scenario Outline: test multiple SCRs in group call.
        Given I have 3 mac devices
        And  I create a venue
        And I create multilayer offer on device 1 
        And I create multilayer offer on device 2 
        And I create multilayer offer on device 3
        And I should get SDP from device 1 and create confluence
        And I should get SDP from device 2 and create confluence
        And I should get SDP from device 3 and create confluence
        Then I keep the call for 3 seconds
        And I subscribe multi layers video with on device 1:
            |scr|<scrArray1>|
        And I subscribe multi layers video with on device 2:
            |scr|<scrArray2>|
        Then I keep the call for 10 seconds
        Then I should received specific layers video less than <Result1> on device 1
        Then I should received specific layers video less than <Result2> on device 2
        Then Device 3 should have encoded with encodedLayers == 2
         Examples:
        |scrArray1                               |scrArray2                                            | Result1 |  |Result2|
        |[{"br": 330000, "mbps":7200, "fs": 398, "fps": 3000, "dpb": 0}]   | [{"br": 180000,"mbps":1800, "fs": 398, "fps": 3000, "dpb": 0}]   |360p| |180p|
        |[{"br": 600000, "mbps":27600, "fs": 920, "fps": 3000, "dpb": 0}]  | [{"br": 330000, "mbps":7200, "fs": 398, "fps": 3000, "dpb": 0}]   |720p| |360p|
 #       |[{"br":1500000, "mbps":108000, "fs": 3600, "fps": 3000, "dpb": 0}]| [{"br": 1500000, "mbps":27600, "fs": 920, "fps": 3000, "dpb": 0}]   |1080p| |720p|
        |[{"br":1500000, "mbps":108000, "fs": 3600, "fps": 3000, "dpb": 0}]| [{"br": 180000,"mbps":1800, "fs": 398, "fps": 3000, "dpb": 0}]   |1080p| |180p|
        |[{"br": 600000, "mbps":27600, "fs": 920, "fps": 3000, "dpb": 0}]  | [{"br": 180000,"mbps":1800, "fs": 398, "fps": 3000, "dpb": 0}]   |720p| |180p|

