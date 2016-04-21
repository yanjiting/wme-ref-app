Feature: group call with calliope and DTMF detecting

@gc_dtmf @example_groupcall @dev @weichen2
Scenario: subscribe multi video streams in group call with file capture and file render, detect the audio and CSI change with DTMF detector.
    Given I have 3 any devices
      And I create a venue
    When I create offer on 1st device with options and params:
		| global | {"videoStreams": 4, "resolution": 2, "audiocount": 3 }  			   	    |
		| files  | {"audio": {"source": "96376257wmerocks_1_8000_16.pcm", "loop": false} }  |
		| files  | {"video": {"source": "video_160_90_6_i420.yuv", "loop": true} }        |
		| feature| {"audio": true, "video": true, "share": "" }            					|
		| video  | {"selectedCodec" : ["H264-SVC"] }                           				|
    When I create offer on 2nd device with options and params:
		| global | {"videoStreams": 4, "resolution": 2, "audiocount": 3  }  		   |
		| files  | {"audio": {"source": "293239webex_1_8000_16.pcm", "loop": false} }  |
		| files  | {"video": {"source": "video_160_90_6_i420.yuv", "loop": true} }   |
		| feature| {"audio": true, "video": true, "share": "" }            			   |
		| video  | {"selectedCodec" : ["H264-SVC"] }                                   |
    When I create offer on 3rd device with options and params:
		| global | {"videoStreams": 4, "resolution": 2, "audiocount": 3  }  		   |
		| files  | {"audio": {"source": "375267369skcoremw_1_8000_16.pcm", "loop": false} }  |
		| files  | {"video": {"source": "video_160_90_6_i420.yuv", "loop": true} }   |
		| feature| {"audio": true, "video": true, "share": "" }            			   |
		| video  | {"selectedCodec" : ["H264-SVC"] }                                   |
    Then I should get SDP from device 1 and create confluence
      And I should get SDP from device 2 and create confluence
      And I should get SDP from device 3 and create confluence
    Then I wait for file capture ended in all devices
	  And I check CSI change history
	  And I check dtmf on all devices
		|9323935267369,932395267369,93239267369  |
		|9637625735267369,963762575267369,9637625267369|
		|9637625793239    |

@gc_cm_dtmf @dev @pc @weichen2
Scenario: a group call in PC with multiple instances, audio only
    Given I have 8 pc devices
        And I create a venue
    When I create offer on all devices with options and params:
		| global | {"audiocount": 3 }  			   	             |
		| files  | {"audio": {"source": "auto", "loop": false, "render": "auto"} }  |
		| feature| {"audio": true, "video": false, "share": "" } |
		| video  | {"selectedCodec" : ["H264"] }                 |
        | audio  | {"enableEC": false, "enableAGC": false} |
    Then I should get SDP for each device and create confluence
        And I wait for file capture ended in all devices
        And I check dtmf file auto_7.pcm is 12345678901234

@gc_cm_p1 @dev @pc @weichen2
Scenario: a group call in PC with multiple instances, audio only
    Given I have 4 pc devices
        And I create a venue
    When I create offer on all devices with options and params:
		| global | {"audiocount": 3 }  			   	             |
		| files  | {"audio": {"source": "autoeven", "loop": false, "render": "autoeven"} }  |
		| feature| {"audio": true, "video": false, "share": "" } |
		| video  | {"selectedCodec" : ["H264"] }                 |
        | audio  | {"enableEC": false, "enableAGC": false} |
    Then I should get SDP for each device and create confluence
        And I wait for file capture ended in all devices
        And I check dtmf file auto_6.pcm is 135802

@gc_cm_overlap @dev @pc @weichen2
Scenario: a group call in PC with multiple instances, audio only
    Given I have 4 pc devices
        And I create a venue
    When I create offer on all devices with options and params:
		| global | {"audiocount": 3 }  			   	             |
		| files  | {"audio": {"source": "overlap", "loop": false, "render": "overlap"} }  |
		| feature| {"audio": true, "video": false, "share": "" } |
		| video  | {"selectedCodec" : ["H264"] }                 |
        | audio  | {"enableEC": false, "enableAGC": false} |
    Then I should get SDP for each device and create confluence
        And I wait for file capture ended in all devices
        And I check dtmf file o_3.pcm is 135802

@gc_active_switch @dev @pc @weichen2
Scenario Outline: a group call in PC with multiple instances, audio only
    Given I have 4 pc devices
        And I create a venue
    When I create offer on 1st device with options and params:
		| global | {"audiocount": <AUDIO_COUNT> }  			   	             |
		| files  | {"audio": {"source": "clientmix/za_1_8000_16.pcm", "loop": false, "render": "za.pcm"} }  |
		| feature| {"audio": true, "video": false, "share": "" } |
		| video  | {"selectedCodec" : ["H264"] }                 |
        | audio  | {"enableEC": false, "enableAGC": false} |
    When I create offer on 2nd device with options and params:
		| global | {"audiocount": <AUDIO_COUNT> }  			   	             |
		| files  | {"audio": {"source": "clientmix/zb_1_8000_16.pcm", "loop": false, "render": "zb.pcm"} }  |
		| feature| {"audio": true, "video": false, "share": "" } |
		| video  | {"selectedCodec" : ["H264"] }                 |
        | audio  | {"enableEC": false, "enableAGC": false} |
#    When I create offer on 3rd device with options and params:
#		| global | {"audiocount": <AUDIO_COUNT> }  			   	             |
#		| files  | {"audio": {"source": "clientmix/zc_1_8000_16.pcm", "loop": false, "render": "zc.pcm"} }  |
#		| feature| {"audio": true, "video": false, "share": "" } |
#		| video  | {"selectedCodec" : ["H264"] }                 |
#       | audio  | {"enableEC": false, "enableAGC": false} |
    When I create offer on 3th device with options and params:
		| global | {"audiocount": <AUDIO_COUNT> }  			   	             |
		| files  | {"audio": {"source": "clientmix/zd_1_8000_16.pcm", "loop": false, "render": "zd.pcm"} }  |
		| feature| {"audio": true, "video": false, "share": "" } |
		| video  | {"selectedCodec" : ["H264"] }                 |
        | audio  | {"enableEC": false, "enableAGC": false} |
    When I create offer on 4th device with options and params:
		| global | {"audiocount": <AUDIO_COUNT> }  			   	             |
		| files  | {"audio": {"source": "clientmix/auto7_1_8000_16.pcm", "loop": false, "render": "checkpoint.pcm"} }  |
		| feature| {"audio": true, "video": false, "share": "" } |
		| video  | {"selectedCodec" : ["H264"] }                 |
        | audio  | {"enableEC": false, "enableAGC": false} |
    Then I should get SDP for each device and create confluence
        And I wait for file capture ended in all devices
        And I check dtmf file checkpoint.pcm is 12

    Examples:
        | AUDIO_COUNT |
        |     3       |