Feature: Test MARI WiFi related features

# Basic operations
@wifi @monitor
Scenario: Test wifi monitor logging
   Then I start wifi monitor
   Then I keep the call for 3 seconds
   Then I stop wifi monitor

@tcpdump
Scenario: Test tcpdump functions
  Given I have 1 device
  Then I start tcpdump on device 1
  Then I keep the call for 10 seconds
  Then I stop tcpdump on device 1

@manually_adjust
Scenario: Make a p2p call and manually_adjust the bitrate
  Given I have 1 pc and 1 mobile

  When I create loopFile offer on device 1 with the default setting and:
    | SRTP | false |
    | ICE | true |
    | RTCPMUX | true |
  And I set loopFile offer to device 2 with the default setting and:
    | SRTP | false |
    | ICE | true |
    | RTCPMUX | true |
    | SDP_IDX | 1    |

  Then I should receive answer from device 2 and set to device 1

  Then I set the estimated bandwidth to 1000 kbps mannually for device 1
  Then I keep the call for 5 seconds
  Then Video resolution should >= 360p for device 1
  Then I keep the call for 15 seconds

  Then I set the estimated bandwidth to 400 kbps mannually for device 1
  Then I keep the call for 5 seconds
  Then Video resolution should <= 270p for device 1
  Then I keep the call for 15 seconds

  Then I set the estimated bandwidth to 1000 kbps mannually for device 1
  Then I keep the call for 5 seconds
  Then Video resolution should >= 360p for device 1
  Then I keep the call for 15 seconds

# contention cases

@p2p @wifi @contention @c4 @qos-enable
Scenario: Make multiple concurrent p2p calls with qos enable
  Given I have 1 pc and 3 mobile
  Then I start wifi monitor
  Then I start tcpdump on devices
  When I create loopFile offer on device 1 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
  And I set loopFile offer to device 2 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
    | SDP_IDX | 1    |

  When I create loopFile offer on device 3 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
  And I set loopFile offer to device 4 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
    | SDP_IDX | 3    |

  Then I should receive answer from device 2 and set to device 1
  Then I should receive answer from device 4 and set to device 3

  Then I keep the call for 30 seconds
  Then I stop tcpdump on devices
  Then I stop wifi monitor
  Then Video resolution should >= 360p

@p2p @wifi @contention @c4 @qos-disable
Scenario: Make multiple concurrent p2p calls with qos disable
  Given I have 1 pc and 3 mobile
  Then I start wifi monitor
  Then I start tcpdump on devices
  When I create disableQos loopFile offer on device 1 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
  And I set loopFile offer to device 2 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
    | SDP_IDX | 1    |

  When I create disableQos loopFile offer on device 3 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
  And I set loopFile offer to device 4 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
    | SDP_IDX | 3    |

  Then I should receive answer from device 2 and set to device 1
  Then I should receive answer from device 4 and set to device 3

  Then I keep the call for 30 seconds
  Then I stop tcpdump on devices
  Then I stop wifi monitor
  Then Video resolution should >= 360p

@p2p @wifi @contention @c6 @qos-enable
Scenario: Make multiple concurrent p2p calls
  Given I have 1 pc and 5 mobile
  Then I start wifi monitor
  Then I start tcpdump on devices
  When I create loopFile offer on device 1 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
  And I set loopFile offer to device 2 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
    | SDP_IDX | 1    |

  When I create loopFile offer on device 3 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
  And I set loopFile offer to device 4 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
    | SDP_IDX | 3    |
  When I create loopFile offer on device 5 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
  And I set loopFile offer to device 6 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
    | SDP_IDX | 5    |

  Then I should receive answer from device 2 and set to device 1
  Then I should receive answer from device 4 and set to device 3
  Then I should receive answer from device 6 and set to device 5

  Then I keep the call for 30 seconds
  Then I stop tcpdump on devices
  Then I stop wifi monitor
  Then Video resolution should >= 360p

@p2p @wifi @contention @c6 @qos-disable
Scenario: Make multiple concurrent p2p calls
  Given I have 1 pc and 5 mobile
  Then I start wifi monitor
  Then I start tcpdump on devices
  When I create disableQos loopFile offer on device 1 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
  And I set loopFile offer to device 2 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
    | SDP_IDX | 1    |

  When I create disableQos loopFile offer on device 3 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
  And I set loopFile offer to device 4 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
    | SDP_IDX | 3    |

  When I create disableQos loopFile offer on device 5 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
  And I set loopFile offer to device 6 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
    | SDP_IDX | 5    |

  Then I should receive answer from device 2 and set to device 1
  Then I should receive answer from device 4 and set to device 3
  Then I should receive answer from device 6 and set to device 5

  Then I keep the call for 30 seconds
  Then I stop tcpdump on devices
  Then I stop wifi monitor
  Then Video resolution should >= 360p

@calliope @wifi @contention
Scenario: Group call via calliope
  Given I have 1 pc and 4 mobile
  Then I start tcpdump on devices
  Then I start wifi monitor
    And I create a venue
  When I create loopFile offer on device 1
  And I create loopFile offer on device 2
  And I create loopFile offer on device 3
  And I create loopFile offer on device 4
  And I create loopFile offer on device 5
  Then I should get SDP from device 1 and create confluence
  And I should get SDP from device 2 and create confluence
  And I should get SDP from device 3 and create confluence
  And I should get SDP from device 4 and create confluence
  And I should get SDP from device 5 and create confluence
  Then I keep the call for 30 seconds
  Then I stop wifi monitor
  Then I stop tcpdump on devices
  Then Video resolution should >= 360p

# data collection

@wifi @calliope @parallel @remote
Scenario: Test parallel call with QoS enable/disable via calliope
  Given I have 1 pc device
  #Then I start tcpdump on devices
  And I create a default venue for mari data collection test call
  When I create loopFile offer on device 1
  Then I should get SDP from device 1 and create confluence
  Then I keep the call forever 
  #Then I stop tcpdump on devices
  #Then Video resolution should >= 360p for the mobile device

@wifi @calliope @parallel @local
Scenario Outline: Test parallel call with QoS enable/disable via calliope
  Given I have 1 pc device
  Then I start wifi monitor
  Then I start tcpdump on devices
  And I create a default venue for mari data collection test call
  When I create <QoSOption> loopFile offer on device 1
  Then I should get SDP from device 1 and create confluence
  Then I keep the call for 80 seconds
  Then I stop tcpdump on devices
  Then I stop wifi monitor
  #Then Video resolution should >= 360p for the mobile device

  Examples:
    | QoSOption  |
    |            |
    | disableQos |

@wifi @concur
Scenario: Make concurrent p2p calls with qos-enable and qos-disable
  Given I have 4 mobile devices
  Then I start wifi monitor
  Then I start tcpdump on devices
  When I create disableQos loopFile offer on device 1 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
  And I set loopFile offer to device 2 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
    | SDP_IDX | 1    |

  When I create disableQos loopFile offer on device 3 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
  And I set loopFile offer to device 4 with the default setting and:
    | SRTP | false |
    | ICE | false |
    | RTCPMUX | true |
    | SDP_IDX | 3    |

  Then I should receive answer from device 2 and set to device 1
  Then I should receive answer from device 4 and set to device 3

  Then I keep the call for 80 seconds
  Then I stop tcpdump on devices
  Then I stop wifi monitor
  Then Video resolution should >= 360p

