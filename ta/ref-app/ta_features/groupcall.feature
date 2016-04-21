Feature: group call with calliope

@gc-activespeaker @sanity 
Scenario Outline: multi-stream in group call.
    Given I have 3 any devices
      And I create a venue
    When I create multilayer offer on device 1 with:
               |video |{"selectedCodec" : ["<CODEC>"] }|
        And I create multilayer offer on device 2 with:
               |video |{"selectedCodec" : ["<CODEC>"] }|
        And I create multilayer offer on device 3 with:
               |video |{"selectedCodec" : ["<CODEC>"] }|
    Then I should get SDP from device 1 and create confluence
        And I should get SDP from device 2 and create confluence
        And I should get SDP from device 3 and create confluence
    Then I keep the call for 10 seconds  
    Then I should have received and decoded some media in devices
    Then I should get participants count is 3 in devices
    Then I check file size of each trace in a call lasting 0 seconds

Examples:
| CODEC    |
| H264     |
| H264-SVC |

@gc-audiomultistream @sanity 
Scenario Outline: audiomultistream in group call.
    Given I have 3 any devices
      And I create a venue
    When I create multilayer offer on device 1 with audiocount = <count>
        And I create multilayer offer on device 2 with audiocount = <count>
        And I create multilayer offer on device 3 with audiocount = <count>
    Then I should get SDP from device 1 and create confluence
        And I should get SDP from device 2 and create confluence
        And I should get SDP from device 3 and create confluence
    Then I keep the call for 10 seconds  
    Then I should have received and decoded some media in devices
    Then I should get participants count is 3 in devices
  
Examples:
    | count    |
    | 3        |

@gc-nonmultistream @intensive 
Scenario: non-multistream group call.
    Given I have 3 any devices
        And I create a venue
    When I create offer on device 1
        And I create offer on device 2
        And I create offer on device 3
    Then I should get SDP from device 1 and create confluence
        And I should get SDP from device 2 and create confluence
        And I should get SDP from device 3 and create confluence
    Then I keep the call for 10 seconds
    Then I should have received and decoded some media in devices
    Then I should get participants count is 3 in devices

@gc-multi-nonmulti @sanity 
Scenario: multi-stream endpoint call non-multi
    Given I have 2 any devices
      And I create a venue
    When I create multilayer offer on device 1
        And I create offer on device 2
    Then I should get SDP from device 1 and create confluence
        And I should get SDP from device 2 and create confluence
    Then I keep the call for 10 seconds  
    Then I should have decoded some media in devices with videoPixels <= 230400
    Then I check file size of each trace in a call lasting 0 seconds

@gc-share @sanity 
Scenario: multi-stream call subscribe low video after start share
    Given I have 1 pc device
    When I start screenshare as _sharer_ on device 1 with the default setting and:
        | LOOPBACK  | true   |
        | CALLIOPE  | true   |
        | AUDIO     | true   |
        | VIDEO     | true   |
        | SHARE     | sharer |
        |MULTISTREAM| true   |
    Then I keep the call for 8 seconds 
    Then I should have decoded some media in devices with videoPixels >= 230400
    Then I subscribe sld video on device 1
    Then I keep the call for 5 seconds 
    Then I should have decoded some media in devices with videoPixels >= 14400
    Then I check file size of each trace in a call lasting 0 seconds


@gc-multi_video @sanity 
Scenario: subscribe multi video streams in group call.
    Given I have 3 any devices
      And I create a venue
    When I create multilayer offer on device 1 with the default setting and:
		|VIDEO_STREAMS |2|
        And I create multilayer offer on device 2 with the default setting and:
		|VIDEO_STREAMS |2|
        And I create multilayer offer on device 3 with the default setting and:
		|VIDEO_STREAMS |2|
    Then I should get SDP from device 1 and create confluence
        And I should get SDP from device 2 and create confluence
        And I should get SDP from device 3 and create confluence
    Then I keep the call for 10 seconds  
    Then I should have received and decoded some media in devices

@gc-subscribe @intensive @deliver
Scenario: subscribe another stream in group call.
    Given I have 1 pc and 2 mobile
      And I create a venue
    When I create multilayer offer on device 1
        And I create multilayer offer on device 2
        And I create multilayer offer on device 3
    Then I should get SDP from device 1 and create confluence
        And I should get SDP from device 2 and create confluence
        And I should get SDP from device 3 and create confluence
    Then I keep the call for 8 seconds  
    Then I should have received and decoded some media in devices
    Then I subscribe_another sld video on device 1
    Then I keep the call for 3 seconds  
    Then I should have received and decoded some media in devices
    Then I unsubscribe sld video on device 1
    Then I keep the call for 3 seconds

@gc-audio-only @intensive @deliver
Scenario: subscribe multi video streams in audio only group call.
    Given I have 3 any devices
      And I create a venue
    When I create multilayer offer on device 1 with the default setting and:
		|VIDEO_STREAMS |2|
        And I create multilayer offer on device 2 with the default setting and:
		|VIDEO_STREAMS |2|
        And I create multilayer offer on device 3 with the default setting and:
		|VIDEO_STREAMS |2|
    Then I pause to send video on device 1
         And I pause to send video on device 2
         And I pause to send video on device 3
    Then I should get SDP from device 1 and create confluence
        And I should get SDP from device 2 and create confluence
        And I should get SDP from device 3 and create confluence
    Then I keep the call for 10 seconds  
    Then I should have active speaker of device 1 come from other devices
        And I should have active speaker of device 2 come from other devices
        And I should have active speaker of device 3 come from other devices

@gc-simulcast @dev 
Scenario: SVC simulcast in group call.
    Given I have 3 any devices
      And I create a venue
    When I create multilayer offer on device 1
        And I create multilayer offer on device 2
        And I create multilayer offer on device 3
    Then I should get SDP from device 1 and create confluence
        And I should get SDP from device 2 and create confluence
        And I should get SDP from device 3 and create confluence
    Then I subscribe sld video on device 1
    Then I subscribe sd video on device 2
    Then I keep the call for 10 seconds  
    Then I should have received and decoded some media in devices       

@gc-hd-test @dev
Scenario: 2 PCs and 1 mobile devices to test, so first it should be HD and then downgrade
    Given I have 2 pc device
      And I create a venue production
    When I create multilayer offer on device 1
      And I create multilayer offer on device 2
    Then I should get SDP from device 1 and create confluence
      And I should get SDP from device 2 and create confluence
    Then I keep the call for 8 seconds  
    Then I should have decoded some media in devices with videoPixels >= 921600
    When I joint further 1 mobile device
      And I create offer on device 3
      And I should get SDP from device 3 and create confluence
    Then I keep the call for 8 seconds 
    Then I should have decoded some media in devices with videoPixels >= 230400

@gc-set-fps	@dev
Scenario Outline: if we set fps to 24 fps in multi-stream, non-multi-stream with 180p/360p make sure we will get that.
  Given I have 2 any devices
    And I create a venue
  When I create <multi> offer on device 1 with:
		|video |{"selectedCodec" : ["<CODEC>"] }|
		|video |{"encodeCodec" : [{"codec":"<CODEC>","uProfileLevelID":"<PROFILE>","max-mbps":<MAXMBPS>,"max-fs":<MAXFS>,"max-fps":<MAXFPS>,"max-br":<MAXBR>}]}|
    And I create offer on device 2 with:
		|video |{"selectedCodec" : ["<CODEC>"]}|
		|video |{"encodeCodec" : [{"codec":"<CODEC>","uProfileLevelID":"<PROFILE>","max-mbps":<MAXMBPS>,"max-fs":<MAXFS>,"max-fps":<MAXFPS>,"max-br":<MAXBR>}]}|
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 6 seconds  
  Then I should have decoded some media in devices with:
		|video | fps | <FPS> |

     Examples:
    |multi     | CODEC    | PROFILE  |  MAXMBPS | MAXFS | MAXFPS | MAXBR | FPS |
    #|          | H264     | 42000c   | 27600    | 920   | 2400   | 1000  | 17   |
    |          | H264     | 42000b   | 6900    | 230   | 2400   | 1000  | 17   |
    #|          | H264-SVC | 42000c   | 27600    | 920   | 2400   | 1000  | 17   |
    #|multilayer| H264     | 42000c   | 27600    | 920   | 2400   | 1000  | 17   |
    #|multilayer| H264-SVC | 42000c   | 27600    | 920   | 2400   | 1000  | 17   |
