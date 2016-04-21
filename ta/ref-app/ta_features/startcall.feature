Feature: startcall Test WME call in 2 devices

@ios-ios @dev
Scenario: Make a call with default settings through ios to ios
  Given I have 2 ios devices
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 8 seconds  
  Then I should have received and decoded some media in devices
  Then I check assertion is zero
  
    @win-winphone @dev
Scenario: Make a call with default settings through windows to winphone
  Given I have 1 windows and 1 winphone
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 8 seconds  
  Then I should have received and decoded some media in devices

@android-android @sanity
Scenario: Make a call with default settings through android to android
  Given I have 2 android devices
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 8 seconds  
  Then I should have received and decoded some media in devices
  Then I check file size of each trace in a call lasting 0 seconds

 
@android-ios @sanity 
Scenario: Make a call with default settings through android to ios
  Given I have 1 android and 1 ios
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 8 seconds  
  Then I should have received and decoded some media in devices
  Then I check file size of each trace in a call lasting 0 seconds

@2devices @intensive @pc
Scenario Outline: Make a call with default settings through any 2 devices, one mobile and one PC
  Given I have 2 any devices
    And I create a venue
  When I create offer on device 1 with:
		|video |{"selectedCodec" : ["<CODEC>"] }|
		|video |{"decodeCodec" : [{"codec":"<CODEC>","uProfileLevelID":"<PROFILE>","max-mbps":<MAXMBPS>,"max-fs":<MAXFS>,"max-fps":<MAXFPS>,"max-br":<MAXBR>}]}|
		|video |{"encodeCodec" : [{"codec":"<CODEC>","uProfileLevelID":"<PROFILE>","max-mbps":<MAXMBPS>,"max-fs":<MAXFS>,"max-fps":<MAXFPS>,"max-br":<MAXBR>}]}|
    And I create offer on device 2 with:
		|video |{"selectedCodec" : ["<CODEC>"]}|
		|video |{"decodeCodec" : [{"codec":"<CODEC>","uProfileLevelID":"<PROFILE>","max-mbps":<MAXMBPS>,"max-fs":<MAXFS>,"max-fps":<MAXFPS>,"max-br":<MAXBR>}]}|
		|video |{"encodeCodec" : [{"codec":"<CODEC>","uProfileLevelID":"<PROFILE>","max-mbps":<MAXMBPS>,"max-fs":<MAXFS>,"max-fps":<MAXFPS>,"max-br":<MAXBR>}]}|
  Then I should get SDP from device 1 and create confluence with mangling sdp
    And I should get SDP from device 2 and create confluence with mangling sdp
  Then I keep the call for 6 seconds  
  Then I should have decoded some media in devices with:
		|video |pixels | 230400 |
  Then I check assertion is zero
  
     Examples:
    | CODEC    | PROFILE  |  MAXMBPS | MAXFS | MAXFPS | MAXBR |
    | H264     | 42000c   | 27600    | 920   | 3000   | 1000  |
    #| H264-SVC | 42000c   | 27600    | 920   | 3000   | 1000  |
    #| H264     | 42000c   | 0        | 0     | 0      | 0     |
    #| H264-SVC | 42000c   | 0        | 0     | 0      | 0     |
	
@pc-pc @intensive @pc
Scenario: Make a call with default settings through pc to pc
  Given I have 2 pc devices
    And I create a venue
  When I create hd offer on device 1
    And I create hd offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 10 seconds  
  Then I should have received and decoded some media in devices

@overrideip @intensive @deliver
Scenario Outline: Make a override ip call with default settings through pc to mobile
  Given I have 2 any devices
    And I create a venue
  When I create overrideip offer on device 1 with:
		|video |{"selectedCodec" : ["<CODEC>"]}|
    And I create overrideip offer on device 2 with:
		|video |{"selectedCodec" : ["<CODEC>"]}|
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 8 seconds  
  Then I should have received and decoded some media in devices with overrideip
  
    Examples:
    | CODEC    | 
    | H264     |
#    | H264-SVC |
	
@overridelinus @newtest @deliver
Scenario Outline: Make a call with production linus, but override it with local linus ip/port.
  Given I have 2 any devices
    And I create a venue in production
	And I create an override venue with <CODEC>
  When I create offer on device 1 with:
		|video |{"selectedCodec" : ["<CODEC>"]}|
    And I create offer on device 2 with:
		|video |{"selectedCodec" : ["<CODEC>"]}|
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 8 seconds  
  Then I should have received and decoded some media in devices with overrideip

    Examples:
    | CODEC    | 
    | H264     |
#    | H264-SVC |
