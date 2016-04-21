Feature: Test MARI Fec related features

@fec-enable-calliope-audio-video-pc-mobile @lab-dev @nlc
Scenario:  Make a call through calliope with specific fec policy: 1 pc and 1 mobile devices, enable fec for both audio and video
  Given I have 1 pc and 1 mobile
    And I create a venue
    And I setup downlink network simulator with loss rate=5% for the pc device
  Then I register video fec payloadType 111 and clockRate 8000 on device 1
  Then I register video fec payloadType 110 and clockRate 8000 on device 2
  Then I register audio fec payloadType 110 and clockRate 8000 on device 1
  Then I register audio fec payloadType 111 and clockRate 8000 on device 2
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 2 seconds
  Then I keep the call for 15 seconds
  Then I clear network simulator rules  
  Then video recovered packet count should be > 0 for the pc device
  Then audio recovered packet count should be > 0 for the pc device


@fec-enable-video-train-fec-level @lab-dev @nlc
Scenario:  Make a call through calliope with specific fec policy:1 pc and 1 mobile devices, enable fec for video
  Given I have 1 pc and 1 mobile
    And I create a venue
    And I setup downlink network simulator with loss rate=5% for the pc device
  Then I set LOSS_RATIO_WITH_PRIORITY fec dynamic fec level scheme on device 1
  Then I set LOSS_RATIO_WITH_PRIORITY fec dynamic fec level scheme on device 2
  Then I register video fec payloadType 111 and clockRate 8000 on device 1
  Then I register video fec payloadType 110 and clockRate 8000 on device 2
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 2 seconds
  Then I keep the call for 15 seconds
  Then I clear network simulator rules  
  Then video recovered packet count should be > 0 for the pc device
 
@fec-enable-p2p-pc-mobile @lab-dev @nlc
Scenario Outline:  Make a p2p call with specific fec policy: 1 pc and 1 mobile devices, enable fec for audio or video 
  Given I have 1 pc and 1 mobile
    And I setup downlink network simulator with loss rate=5% for the pc device
  Then I register <sessionType> fec payloadType 111 and clockRate 8000 on device 1
  Then I register <sessionType> fec payloadType 110 and clockRate 8000 on device 2
  Then I set <sessionType> session <fecOrder1> fec order on device 1
  Then I set <sessionType> session <fecOrder2> fec order on device 2
  When I create offer on device 1 with the default setting and:
    | SRTP     | <srtpEnable1>   |
    And I set offer to device 2 with the default setting and:
    | SRTP     | <srtpEnable2>   |
  Then I should receive answer from device 2 and set to device 1 
  Then I keep the call for 15 seconds
  Then I clear network simulator rules  
  Then I should have received and decoded some media in devices
  Then <sessionType> recovered packet count should be > 0 for the pc device

  Examples:
    | sessionType | srtpEnable1 | srtpEnable2 |    fecOrder1   |    fecOrder2   |
#   |   video     |   true      | true        | ORDER_FEC_SRTP | ORDER_FEC_SRTP |
    |   video     |   true      | true        | ORDER_SRTP_FEC | ORDER_SRTP_FEC |
    |   video     |   false     | false       | ORDER_FEC_SRTP | ORDER_FEC_SRTP |
#   |   audio     |   true     | true       | ORDER_FEC_SRTP | ORDER_FEC_SRTP |
#   |   share     |   true     | true       | ORDER_FEC_SRTP | ORDER_FEC_SRTP |

@fec-enable-p2p-audio-video-pc-mobile @lab-dev @nlc
Scenario Outline:  Make a p2p call with specific fec policy: 1 pc and 1 mobile devices, both enable fec for video and audio
  Given I have 1 pc and 1 mobile
    And I setup downlink network simulator with loss rate=5% for the pc device
  Then I register audio fec payloadType 111 and clockRate 8000 on device 1
  Then I register audio fec payloadType 110 and clockRate 8000 on device 2
  Then I register video fec payloadType 110 and clockRate 8000 on device 1
  Then I register video fec payloadType 111 and clockRate 8000 on device 2
  When I create offer on device 1 with the default setting and:
    | SRTP     | <srtpEnable1>   |
    And I set offer to device 2 with the default setting and:
    | SRTP     | <srtpEnable2>   |
  Then I should receive answer from device 2 and set to device 1 
  Then I keep the call for 15 seconds
  Then I clear network simulator rules  
  Then audio recovered packet count should be > 0 for the pc device
  Then video recovered packet count should be > 0 for the pc device

  Examples:
    | srtpEnable1 | srtpEnable2 |
    |   false     | false       |

  @fec-enable-calliope-downlink-pc-mobile @lab-dev @nlc @sanity
Scenario Outline: Make a call through calliope with one pc and one mobile, then simulate uplink network and check the result(device1:mobile, device2:mac)
  Given I have 1 pc and 1 mobile
    And I create a venue
    And I setup downlink network simulator with loss rate=5% for the pc device
  Then I register <sessionType> fec payloadType 111 and clockRate 8000 on device 1
  Then I register <sessionType> fec payloadType 110 and clockRate 8000 on device 2
  When I create loopFile offer on device 1
    And I create loopFile offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 15 seconds
   Then I clear network simulator rules  
  Then <sessionType> recovered packet count should be > 0 for the pc device
  Then I check file size of each trace in a call lasting 0 seconds

  Examples:
  | sessionType |
  |   video     |
  |   audio     |
 # |   share     |
