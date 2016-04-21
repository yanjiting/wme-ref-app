Feature: Test iOS Audio Features

@speaker-earpiece @dev
Scenario: Test iOS Speaker/Earpiece mode

	Given I have 2 ios devices
	And I create a venue
	When I create offer on device 1
	And I create offer on device 2
	Then I should get SDP from device 1 and create confluence
	And I should get SDP from device 2 and create confluence
	Then I keep the call for 5 seconds
	Then I should have received and decoded some media in devices

	Then I set the audio playback on device 1 to Speaker
	Then I keep the call for 10 seconds

	Then I set the audio playback on device 2 to Speaker
	Then I keep the call for 10 seconds

	Then I stop the call on device 1
	Then I stop the call on device 2


