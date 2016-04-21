Feature: Performance test on different platform(mac/ios/android)

@performance-test-p2p-call @dev @performance-memory
Scenario: test performance of memory with one pc and one mobile through a p2p call
 Given I have 1 mac and 1 mobile
 When I create disableQos and loopFile and DumpPerformanceTrace offer on device 1 with the default setting and:
    | ICE | true |
    | RTCPMUX | true |
  And I set disableQos and loopFile and DumpPerformanceTrace offer to device 2 with the default setting and:
    | ICE | true |
    | RTCPMUX | true |
  Then I should receive answer from device 2 and set to device 1
  Then I keep the call for 5 seconds
  Then I get memory usage on the pc device
  Then I get memory usage on the mobile device
  Then I keep the call for 600 seconds
  Then I get memory usage again and memory increasing should be < 20 Mbps on the pc device
  Then I get memory usage again and memory increasing should be < 20 Mbps on the mobile device
  Then I stop the call on device 1
  Then I stop the call on device 2

  @performance-test-calliope @dev @performance-memory
  Scenario: test performance of memory with on pc and one mobile through calliope
   Given I have 1 mac and 1 mobile
   And I create a venue
  When I create  disableQos and loopFile and DumpPerformanceTrace offer on device 1
    And I create disableQos and loopFile and DumpPerformanceTrace offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence  
  Then I keep the call for 5 seconds
  Then I get memory usage on the pc device
  Then I get memory usage on the mobile device
  Then I keep the call for 600 seconds
  Then I get memory usage again and memory increasing should be < 20 Mbps on the pc device
  Then I get memory usage again and memory increasing should be < 20 Mbps on the mobile device
  Then I stop the call on device 1
  Then I stop the call on device 2

  @performance-test-p2p-call-share @dev @performance-memory
  Scenario: test performance of memory with one pc and one mobile through p2p and start screen share
    Given I have 1 mac and 1 mobile
    Given I screen share file capture resource screen_ppt_2880_1800_RGBA.raw
    Then I keep the screenshare for 5 seconds
    When I create offer on device 1 with the default setting and:
    | ICE     | true  |
    | SHARE   | sharer   |
    | SHARE_SOURCE_FILE | screen_ppt_2880_1800_RGBA.raw |
    And I set offer to device 2 with the default setting and:
    | ICE     | true  |
    | SHARE   | viewer  |
    Then I should receive answer from device 2 and set to device 1
    Then I keep the screenshare for 10 seconds
    Then I get memory usage on the pc device
    Then I get memory usage on the mobile device
    Then I keep the screenshare for 600 seconds
    Then I get memory usage again and memory increasing should be < 20 Mbps on the pc device
    Then I get memory usage again and memory increasing should be < 20 Mbps on the mobile device
    Then I wait for screenshare ended on device 1
    Then I wait for screenshare ended on device 2
    Then I get memory usage again and memory increasing should be < 5 Mbps on the pc device
    Then I get memory usage again and memory increasing should be < 5 Mbps on the mobile device
    Then I stop the call on device 1
    Then I stop the call on device 2

  @performance-test-calliope-share @dev @performance-memory
  Scenario: test performance of memory with one pc and one mobile through calliope and start screen share
  Scenario: screenshare linus desktop in iOS
    Given I have 1 mac and 1 mobile
    And I create a venue
    Then I keep the screenshare for 5 seconds
    When I create offer on device 1 with the default setting and:
            | ICE     | true  |
            | SHARE   | sharer   |
            | SOURCE     | screen: |
    And I set offer to device 2 with the default setting and:
            | ICE     | true  |
            | SHARE   | viewer  |
    Then I should receive answer from device 2 and set to device 1
    Then I keep the screenshare for 10 seconds
    Then I get memory usage on the pc device
    Then I get memory usage on the mobile device
    Then I keep the screenshare for 600 seconds
    Then I get memory usage again and memory increasing should be < 20 Mbps on the pc device
    Then I get memory usage again and memory increasing should be < 20 Mbps on the mobile device
    Then I wait for screenshare ended on device 1
    Then I wait for screenshare ended on device 2
    Then I get memory usage again and memory increasing should be < 5 Mbps on the pc device
    Then I get memory usage again and memory increasing should be < 5 Mbps on the mobile device
    Then I stop the call on device 1
    Then I stop the call on device 2
