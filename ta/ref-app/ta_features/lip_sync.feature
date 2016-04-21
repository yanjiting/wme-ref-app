Feature: As a user I want a lip sync call from other people

@lip-sync-normal @lab-sanity @pass
Scenario: Make a call with normal time interval of audio and video
  Given I have 2 mobile devices
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I keep the call for 30 seconds
  Then I should get the lip sync result with sync over 90 precent in devices
  
@lip-sync-normal-loop @lab-sanity @pass
Scenario: Make a call with normal time interval of audio and video by loopback
  Given I have 1 mobile device
  When I start loopback and p2p call
  Then I keep the call for 30 seconds
  Then I should get the lip sync result with sync over 90 precent in device
  
@lip-sync-drop @lab-sanity @pass
Scenario: Make a call with video delay over 100ms with audio
  Given I have 2 mobile devices
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I delay video with 1000 milliseconds in devices
  Then I keep the call for 30 seconds
  Then I clean network rules
  Then I should get the lip sync result with delay over 90 precent in devices
  
@lip-sync-buffer @lab-sanity @pass
Scenario: Make a call with audio delay between 100ms and 10s with video
  Given I have 2 mobile devices
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I delay audio with 1000 milliseconds in devices
  Then I keep the call for 30 seconds
  Then I clean network rules
  Then I should get the lip sync result with sync over 90 precent in devices
  
# NOTE: I can only set a maximum delay of 10 seconds by network simulator
@lip-sync-ahead @lab-dev @unsupport
Scenario: Make a call with audio delay over 11s with video
  Given I have 2 mobile devices
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I delay audio with 11000 milliseconds in devices
  Then I keep the call for 30 seconds
  Then I clean network rules
  Then I should get the lip sync result with ahead over 90 precent in devices
  
# NOTE: I can't find a good way to mute or drop all audio packets
@lip-sync-nosync @lab-dev @unsupport
Scenario: Make a call with only video
  Given I have 2 mobile devices
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I mute audio in devices
#  Then I delay audio with 9999 milliseconds in devices
#  Then I make a audio loss with 100 precent in devices
  Then I keep the call for 10 seconds
  Then I clean network rules
  Then I should get the lip sync result with nosync over 90 precent in devices
  
# NOTE: I can't find a good way to mute or drop all audio packets
@lip-sync-nosync-loop @lab-dev @unsupport
Scenario: Make a call with only video by loopback
  Given I have 1 mobile device
  When I start loopback p2p and mute call
  Then I keep the call for 2 seconds
  Then I unmute video in device
  Then I keep the call for 10 seconds
  Then I should get the lip sync result with nosync over 90 precent in device
  
# NOTE: I can't set a jitter by network simulator
@lip-sync-audio-jitter @lab-dev @unsupport
Scenario: Make a call with audio with 5 seconds jitter
  Given I have 2 mobile devices
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I make a audio jitter with 5 seconds in devices
  Then I keep the call for 30 seconds
  Then I clean network rules
  Then I should get the lip sync result with sync over 90 precent in devices
  
@lip-sync-audio-loss @lab-sanity @pass
Scenario: Make a call with audio with 10 precent loss
  Given I have 2 mobile devices
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I make a audio loss with 10 precent in devices
  Then I keep the call for 30 seconds
  Then I clean network rules
  Then I should get the lip sync result with sync over 90 precent in devices
  
@lip-sync-bandwidth-500K @lab-dev @fail
Scenario: Make a call with 500 Kbps bandwidth
  Given I have 2 mobile devices
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I limit 500 Kbps in devices
  Then I keep the call for 30 seconds
  Then I clean network rules
  Then I should get the lip sync result with sync over 80 precent in devices
  
@lip-sync-bandwidth-1M @lab-dev @fail
Scenario: Make a call with 1 Mbps bandwidth
  Given I have 2 mobile devices
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I limit 1000 Kbps in devices
  Then I keep the call for 30 seconds
  Then I clean network rules
  Then I should get the lip sync result with sync over 90 precent in devices
  
# NOTE: I can't set a jitter by network simulator
@lip-sync-complex @lab-dev @fail
Scenario: Make a call with audio with 10 precent loss and 5 seconds jitter, 1 Mbps bandwidth
  Given I have 2 mobile devices
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I make a audio loss with 10 precent in devices
  	And I make a audio jitter with 5 seconds in devices
  	And I limit 500 Kbps in devices
  Then I keep the call for 30 seconds
  Then I clean network rules
  Then I should get the lip sync result with sync over 80 precent in devices
  