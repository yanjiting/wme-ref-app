Feature: Test audio key dump file feature

@audio-dump @dev @audio @weichen2 @deliver
Scenario Outline: Make a call with pc and mobile and try to dump some audio data
  Given I have 1 <os1> and 1 <os2>
	And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I set the following Audio parameters for device 1 as: enableKeyDumpFile = 5000
  Then I set the following Audio parameters for device 2 as: enableKeyDumpFile = 5000
  Then I keep the call for 8 seconds  
  Then I should have received and decoded some media in devices
    And I got files on all devices:
		#|AECFarInIutput-float32-Chn1-16000.pcm  | 369920 |
		#|AECNearInIutput-float32-Chn1-16000.pcm | 369920 |
		|AECOutput-float32-Chn1-16000.pcm       | 370560 |
		|DigtalAGCOutput-float32-Chn1-16000.pcm | 370560 |
		|Playback-float32-Chn1-16000.pcm        | 369920 |
		
  Examples:
    | os1        | os2      |
#    | windows    | android  |
    | mac        | ios      |
    | mac        | android  |
