Feature: Test SRTP in 2 devices

@dtls_srtp_mac @andy  @sanity
Scenario: Test dtls-srtp mac loopback call.
        Given I have 1 mac device
        When I start loopback dtls_srtp call
        Then I keep the call for 10 seconds
        Then I should have received and decoded some media in devices

@dtls-srtp—linus-degrade-01 @fortest
Scenario: Make a call with srtp linus
  Given I have 2 any devices
    And I create a venue
    And I create dtls_srtp offer on device 1
    And I create dtls_srtp offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 15 seconds
  Then I should have received and decoded some media in devices

