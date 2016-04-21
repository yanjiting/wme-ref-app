Feature: ice with any 2 devices to test ICE connectivity check

@ice-enabled @intensive @ice @pc @deliver
Scenario Outline: Make a call with ice enabled through any 2 devices, one mobile and one PC
  Given I have 1 pc and 1 mobile
    And I create a venue
  When I create offer on device 1 with the default setting and:
    | ICE     | <IceEnable1>   |
    | RTCPMUX | <RtcpMux1>     |
    And I create offer on device 2 with the default setting and:
    | ICE     | <IceEnable2>   |
    | RTCPMUX | <RtcpMux2>     |
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 6 seconds  
  Then I should have received and decoded some media in devices
  	And I stop the call on device 1
  	And I stop the call on device 2

  Examples:
    | IceEnable1 | RtcpMux1 | IceEnable2 | RtcpMux2 |
    | false      | false    | false      | false    | 
    | true       | true     | true       | true     | 
    | false      | true     | false      | true     | 
    | false      | true     | true       | true     | 
    
@ice-p2p @intensive @ice @pc
Scenario Outline: Make a call with different ice and rtcp-mux setting in p2p call.
  Given I have 1 pc and 1 mobile
# Then I add one _any_ name _desktop_source_ screensource on device 1
  When I create offer on device 1 with the default setting and:
    | ICE     | <IceEnable1>   |
    | RTCPMUX | <RtcpMux1>     |
	| VIDEO	  | true           |
	| AUDIO	  | true		   |
#	| SHARE   | sharer		   |
    And I set offer to device 2 with the default setting and:
    | ICE     | <IceEnable2>   |
    | RTCPMUX | <RtcpMux2>     |
	| VIDEO	  | true           |
	| AUDIO	  | true		   |
#	| SHARE   | viewer		   |
  Then I should receive answer from device 2 and set to device 1  
  Then I keep the call for 8 seconds  
  Then I should have received and decoded some media in devices
  	And I stop the call on device 1
  	And I stop the call on device 2

  Examples:
    | IceEnable1 | RtcpMux1 | IceEnable2 | RtcpMux2 |
    | true       | true     | true       | true     | 
    | true       | true     | true       | false    | 
    | true       | false    | true       | true     | 
#    | true       | true     | false      | true     | 
#    | false      | true     | true       | true     | 

@p2pmobile @ice
Scenario: Make a call with ice enabled through any 2 mobile devices
  Given I have 2 mobile devices
  When I create offer on device 1 with the default setting and:
    | ICE     | true   |
    | RTCPMUX | true   |
    And I set offer to device 2 with the default setting and:
    | ICE     | true   |
    | RTCPMUX | true   |
  Then I should receive answer from device 2 and set to device 1  
  Then I keep the call for 6 seconds  
  Then I should have received and decoded some media in devices
  	And I stop the call on device 1
  	And I stop the call on device 2
	
	
@android-p2p @weichen2 @fortest
Scenario: Test android p2p loopback call.
    Given I have 1 android device
    When I start loopback p2p multilayer call
    Then I keep the call for 6 seconds
	Then I should have received and decoded some media in devices

@stuntrace @dev @rozheng
Scenario: StunTrace sanity check
	Given I have 1 mac and 1 ios
	When I start call on device 1
	Then I wait out the stuntrace on device 1
	When I start call on device 2
	Then I wait out the stuntrace on device 2
	
@stuntrace_and @dev @rozheng
Scenario: StunTrace sanity check
	Given I have 1 mac and 1 android
	When I start call on device 1
	Then I wait out the stuntrace on device 1
	When I start call on device 2
	Then I wait out the stuntrace on device 2

@stuntrace_win @dev @rozheng
Scenario: StunTrace sanity check
	Given I have 1 windows device
	When I start call on device 1
	Then I wait out the stuntrace on device 1

@traceserver @dev @rozheng
Scenario: TracesServer sanity check
	Given I have 1 mac and 1 ios
	When I start call on device 1
	Then I wait out the traceserver on device 1
	When I start call on device 2
	Then I wait out the traceserver on device 2

@traceserver_and @dev @rozheng
Scenario: TracesServer sanity check
	Given I have 1 mac and 1 android
	When I start call on device 1
	Then I wait out the traceserver on device 1
	When I start call on device 2
	Then I wait out the traceserver on device 2
	
@traceserver_win @dev @rozheng
Scenario: TracesServer sanity check
	Given I have 1 windows device
	When I start call on device 1
	Then I wait out the traceserver on device 1

@proxy-NTLM_win @jikkyy @fortest
Scenario: Test windows loopback call with proxy.
	Given I have 1 windows device
	When I do proxy setting from device 1 for NTLM
	When I start loopback call
	Then I keep the call for 10 seconds
	Then I should have received and decoded some media in devices
	Then I clean proxy setting from device 1
	
@proxy-SOCKS_win @jikkyy @fortest
Scenario: Test windows loopback call with proxy.
	Given I have 1 windows device
	When I do proxy setting from device 1 for SOCKS
	When I start loopback call
	Then I keep the call for 10 seconds
	Then I should have received and decoded some media in devices
	Then I clean proxy setting from device 1	

@proxy-NTLM_mac @jikkyy @fortest
Scenario: Test mac loopback call with proxy.
	Given I have 1 mac device
	When I do proxy setting from mac device 1 for HTTP
	When I start loopback call
	Then I keep the call for 10 seconds
	Then I should have received and decoded some media in devices
	Then I clean proxy setting from mac device 1

@proxy-SOCKS_mac @jikkyy @fortest
Scenario: Test mac loopback call with proxy.
	Given I have 1 mac device
	When I do proxy setting from mac device 1 for SOCKS
	When I start loopback call
	Then I keep the call for 10 seconds
	Then I should have received and decoded some media in devices
	Then I clean proxy setting from mac device 1
	When I start call on device 1
