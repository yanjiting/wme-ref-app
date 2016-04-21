Feature: Test MARI QoS related features

@qos-disable-loopback-p2p-init_bw @lab-dev
Scenario: Make a loopback p2p call with different initial bandwidth
  Given I have 1 mobile device
  When I start loopback and p2p and loopFile and disableQos call
  Then I keep the call for 2 seconds
  Then I set Video initial bandwidth as 1024kbps for device 1
  Then I keep the call for 5 seconds
  Then Video evaluated bandwidth should == 1024kbps
  Then Video resolution should >= 180p
  Then I set Video initial bandwidth as 80kbps for device 1
  Then I keep the call for 5 seconds
  Then Video evaluated bandwidth should == 80kbps
  Then Video resolution should <= 90p

@qos-disable-p2p-init_bw @lab-dev
Scenario: Make a p2p call with same initial bandwidth
  Given I have 2 mobile devices
  When I create disableQos offer on device 1 with the default setting and:
    | ICE | true |
    | RTCPMUX | true |
  And I set offer to device 2 with the default setting and:
    | ICE | true |
    | RTCPMUX | true |
  Then I should receive answer from device 2 and set to device 1
  Then I keep the call for 10 seconds
  Then I set Video initial bandwidth as 1024kbps for device 1
  Then I set Video initial bandwidth as 1024kbps for device 2
  Then I keep the call for 10 seconds
  Then Video evaluated bandwidth should == 1024kbps
  Then Video resolution should >= 180p
  Then I set Video initial bandwidth as 80kbps for device 1
  Then I set Video initial bandwidth as 80kbps for device 2
  Then I keep the call for 10 seconds
  Then Video evaluated bandwidth should == 80kbps
  Then Video resolution should <= 90p

@qos-disable-calliope-uplink-pc-mobile @lab-dev @nlc
Scenario: Make a call through calliope, then simulate uplink bandwidth and check test result
  Given I have 1 pc and 1 mobile
    And I create a venue
    And I setup uplink network simulator with bw=200Kbps, qdelay=200ms and delay=100ms for the pc device
  When I create  disableQos and loopFile offer on device 1
    And I create disableQos and loopFile offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 15 seconds  
  Then I clear network simulator rules  
  Then Video resolution should <= 180p for the pc device

  
@qos-disable-calliope-downlink-pc-mobile @lab-dev @nlc
Scenario: Make a call through calliope, then simulate downlink bandwidth and check test result
  Given I have 1 pc and 1 mobile
    And I create a venue
    And I setup downlink network simulator with bw=200Kbps, qdelay=200ms and delay=100ms for the pc device
  When I create disableQos and loopFile offer on device 1
    And I create disableQos and loopFile offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 15 seconds  
  Then I clear network simulator rules  
  Then Video resolution should <= 180p for the mobile device

@qos-enable-calliope-check-metrics @lab-dev @nlc
Scenario Outline: Make a call through calliope, then simulate uplink network and check the network metrics
  Given I have 1 pc and 1 mobile
    And I create a venue
    And I setup uplink network simulator with bw=<MaxBandwidth>Kbps, qdelay=<qdelay>ms and delay=<delay>ms for the pc device
  When I create loopFile offer on device 1
    And I create loopFile offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 30 seconds
  Then Video resolution should >= 180p for the pc device
  Then I setup uplink network simulator with bw=<MinBandwidth>Kbps, qdelay=<qdelay>ms and delay=<delay>ms for the pc device
  Then I keep the call for 30 seconds
  Then Video resolution should <= 90p for the pc device
  # Then Video smoothSend drop frame count should > 0
  #Then Video frame max buffer time should > 0ms
  Then I setup uplink network simulator with bw=<MaxBandwidth>Kbps, qdelay=<qdelay>ms and delay=<delay>ms for the pc device
  Then I keep the call for 120 seconds
  Then Video resolution should >= 180p for the pc device
  Then I clear network simulator rules

  @policing
  Examples:
    | MaxBandwidth | MinBandwidth | qdelay | delay |
    |     1000     |     120      |   10   |   10  |
    |     1000     |     120      |   10   |  100  |
    |     1000     |     120      |   10   |  300  |

  @shaping
  Examples:
    | MaxBandwidth | MinBandwidth | qdelay | delay |
    |     1000     |     120      |  1000  |   10  |
    |     1000     |     120      |  1000  |  100  |
    |     1000     |     120      |  1000  |  300  |
    |     1000     |     120      |   150  |   10  |
    |     1000     |     120      |   150  |  100  |
    |     1000     |     120      |   150  |  300  |

#  @test
#  Examples:
#    | MaxBandwidth | MinBandwidth | qdelay | delay |
#    |     1000     |     120      |  1000  |  300  |


@qos-enable-p2p-fec-enable-1-pc-mobile @lab-dev @probing @nlc
Scenario: Make a p2p call and enable QoS and video FEC
  Given I have 1 pc and 1 mobile
   And I setup uplink network simulator with bw=800Kbps, qdelay=150ms and delay=100ms for the pc device
   And I register video fec payloadType 111 and clockRate 8000 on device 1
   And I register video fec payloadType 110 and clockRate 8000 on device 2
  When I create loopFile offer on device 1 with the default setting and:
    | SRTP | false |
    | ICE | true |
    | RTCPMUX | true |
  And I set loopFile offer to device 2 with the default setting and:
    | SRTP | false |
    | ICE | true |
    | RTCPMUX | true |
  Then I should receive answer from device 2 and set to device 1
  Then I keep the call for 10 seconds
  Then Video resolution should >= 360p for the pc device
  Then I setup uplink network simulator with bw=120Kbps, qdelay=150ms and delay=100ms for the pc device
  Then I keep the call for 10 seconds
  Then Video resolution should <= 90p for the pc device
  Then I setup uplink network simulator with bw=800Kbps, qdelay=150ms and delay=100ms for the pc device
  Then I keep the call for 120 seconds
  Then Video resolution should >= 360p for the pc device
  Then I clear network simulator rules


@qos-enable-p2p-fec-disable-1-pc-mobile @probing @nlc
Scenario: Make a p2p call and enable QoS and disable FEC
  Given I have 1 pc and 1 mobile
   And I setup uplink network simulator with bw=800Kbps, qdelay=150ms and delay=100ms for the pc device
  When I create loopFile offer on device 1 with the default setting and:
    | SRTP | false |
    | ICE | true |
    | RTCPMUX | true |
  And I set loopFile offer to device 2 with the default setting and:
    | SRTP | false |
    | ICE | true |
    | RTCPMUX | true |
  Then I should receive answer from device 2 and set to device 1
  Then I keep the call for 10 seconds
  Then Video resolution should >= 360p for the pc device
  Then I setup uplink network simulator with bw=120Kbps, qdelay=150ms and delay=100ms for the pc device
  Then I keep the call for 30 seconds
  Then Video resolution should <= 90p for the pc device
  Then I setup uplink network simulator with bw=800Kbps, qdelay=150ms and delay=100ms for the pc device
  Then I keep the call for 120 seconds
  Then Video resolution should <= 180p for the pc device
  Then I clear network simulator rules


@qos-enable-calliope-uplink-pc-mobile @lab-dev @nlc @sanity
Scenario: Make a call through calliope with one pc and one mobile, then simulate uplink network and check the result
  Given I have 1 pc and 1 mobile
    And I create a venue
    And I setup uplink network simulator with delay=100ms for the pc device
  When I create loopFile offer on device 1
    And I create loopFile offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 5 seconds
  Then Video resolution should >= 360p for the pc device
  Then I setup uplink network simulator with bw=120Kbps and qdelay=150ms for the pc device
  Then I keep the call for 30 seconds
  Then Video evaluated bandwidth should <= 120kbps for the pc device
  Then Video resolution should <= 90p for the pc device
 #Then Video smoothSend drop frame count should > 0
  Then I clear network simulator rules
  #Then I keep the call for 60 seconds
  #Then Video evaluated bandwidth should >= 500kbps
  Then I check file size of each trace in a call lasting 0 seconds

@qos-enable-calliope-downlink-pc-mobile @lab-dev @nlc @sanity
Scenario: Make a call through calliope, then simulate downlink network and check the result
  Given I have 1 pc and 1 mobile
    And I create a venue
    And I setup downlink network simulator with delay=100ms for the pc device
  When I create loopFile offer on device 1
    And I create loopFile offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 5 seconds
  Then Video resolution should >= 360p for the mobile device
  Then I setup downlink network simulator with bw=200Kbps and qdelay=150ms for the pc device
  Then I keep the call for 30 seconds
  Then Video resolution should <= 180p for the mobile device
  Then I clear network simulator rules
#  Then I keep the call for 15 seconds
#  Then Video evaluated bandwidth should >= 300kbps for the mobile device
  Then I check file size of each trace in a call lasting 0 seconds

  @video-disable-uplink-pc-mobile @lab-dev @video-disable @nlc
Scenario: Make a call through calliope, then simulate bad uplink network and check if video disabled
  Given I have 1 pc and 1 mobile
    And I create a venue
    And I setup uplink network simulator with bw=800Kbps, qdelay=150ms and delay=100ms for the pc device
  When I create loopFile offer on device 1
    And I create loopFile offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 5 seconds
  Then Video resolution should >= 360p for the pc device
  Then I setup uplink network simulator with bw=100Kbps, qdelay=150ms and delay=100ms for the pc device
  Then I keep the call for 30 seconds
  Then Video should stop for the pc device
  Then I clear network simulator rules

@video-disable-downlink-pc-mobile @lab-dev @video-disable @nlc
Scenario: Make a call through calliope, then simulate bad downlink network and check if video disabled
  Given I have 1 pc and 1 mobile
    And I create a venue
    And I setup downlink network simulator with bw=800Kbps, qdelay=150ms and delay=100ms for the pc device
  When I create loopFile offer on device 1
    And I create loopFile offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 5 seconds
  Then Video resolution should >= 360p for the mobile device
  Then I setup downlink network simulator with bw=100Kbps, qdelay=150ms and delay=100ms for the pc device
  Then I keep the call for 30 seconds
  Then Video should stop for the mobile device
  Then I clear network simulator rules

@qos-enable-calliope-uplink-loss-based-pc-mobile @lab-dev @nlc @sanity
Scenario: Make a call through calliope with one pc and one mobile, set feature toggles as true ,and mari rate adaptation as "loss-based", then simulate uplink network and check the result
  Given I have 1 pc and 1 mobile
    And I create a venue
    And I setup uplink network simulator with delay=100ms for the pc device
    And I set mari rate adaptation policy as loss-based
  When I create loopFile offer on device 1
    And I create loopFile offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 5 seconds
  Then Video resolution should >= 360p for the pc device
  Then I setup uplink network simulator with bw=120Kbps and qdelay=150ms for the pc device
  Then I keep the call for 30 seconds
  Then I check the mari rate adaptation policy is loss-based
  Then Video evaluated bandwidth should <= 120kbps for the pc device
  Then Video resolution should <= 90p for the pc device
  Then I clear network simulator rules
  Then I check file size of each trace in a call lasting 0 seconds

@enable-packet-loss-raw-data-recording-calliope-pc-mobile
Scenario: Make a call through calliope with one pc and one mobile, verifying the packet loss raw data recording feature is working when feature toggle is enabled
  Given I have 1 pc and 1 mobile
    And I create a venue 
    And I setup downlink network simulator with loss rate=10% for device 1
    And I setup downlink network simulator with loss rate=10% for device 2
    And I set packet loss raw data recording policy as allowing maximum 10 bytes to be recorded
  When I create loopFile offer on device 1
    And I create loopFile offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 30 seconds
  Then I check the packet loss raw data recording policy is allowing maximun 10 bytes to be recorded
  Then I clear network simulator rules

@disable-packet-loss-raw-data-recording-calliope-pc-mobile
Scenario: Make a call through calliope with one pc and one mobile, verifying the packet loss raw data recording feature is not working when feature toggle is disabled
  Given I have 1 pc and 1 mobile
    And I create a venue 
    And I set packet loss raw data recording policy as allowing maximum 0 bytes to be recorded
  When I create loopFile offer on device 1
    And I create loopFile offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 10 seconds
  Then I check the packet loss raw data recording policy is allowing maximun 0 bytes to be recorded

@dont-set-packet-loss-raw-data-recording-calliope-pc-mobile
Scenario: Make a call through calliope with one pc and one mobile, verifying the packet loss raw data recording feature is not working when feature toggle is disabled
  Given I have 1 pc and 1 mobile
    And I create a venue 
  When I create loopFile offer on device 1
    And I create loopFile offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 10 seconds
  Then I check the packet loss raw data recording policy is allowing maximun 0 bytes to be recorded


