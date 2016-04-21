Feature: Test Mute and Unmute Features

@mute-unmute1 @sanity
Scenario Outline: Test Any devices Mute/Unmute
        Given I have 1 <dev1> and 1 <dev2>
    And I create a venue
        When I create offer on device 1
    And I create offer on device 2
        Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
        Then I keep the call for 8 seconds  
        Then I should have received and decoded some media in devices
        
        Then I mute local audio on device 1
        Then I keep the call for 8 seconds  
        Then device 2 should not be receiving audio data
        Then device 2 should have encoded and decoded some video
        Then device 2 should have encoded some audio
        Then device 1 should have encoded and decoded some video 
        Then device 1 should have decoded some audio 
        Then I unmute local audio on device 1
        Then I keep the call for 8 seconds  
        Then I should have received and decoded some media in devices
       
        Then I pause to send video on device 1
        Then I keep the call for 8 seconds  
        Then device 2 should not be receiving video data
        Then device 2 should have encoded and decoded some audio
        Then device 2 should have encoded some video
        Then device 1 should have encoded and decoded some audio 
        Then device 1 should have decoded some video 
        Then I unpause to send video on device 1
        Then I keep the call for 8 seconds  
        Then I should have received and decoded some media in devices
        
        Then I mute remote audio on device 1
        Then I keep the call for 8 seconds  
        Then device 1 should not be receiving audio data
        Then device 1 should have encoded and decoded some video
        Then device 1 should have encoded some audio
        Then device 2 should have encoded and decoded some media 
        Then I unmute remote audio on device 1
        Then I keep the call for 8 seconds  
        Then I should have received and decoded some media in devices  	
       
        Then I pause to receive video on device 1
        Then I keep the call for 8 seconds  
        Then device 1 should not be receiving video data
        Then device 1 should have encoded and decoded some audio
        Then device 1 should have encoded some video
        #Then device 2 should have encoded and decoded some media 
        Then I unpause to receive video on device 1
        Then I keep the call for 8 seconds  
        Then I should have received and decoded some media in devices
        
        Then I stop the call on device 1
        Then I stop the call on device 2
  
        Then I check file size of each trace in a call lasting 0 seconds

  Examples:
    | dev1       | dev2       |
    | ios        | mac        |
    | mac        | ios        |
    | android    | mac        |

@start-stop-track @mytest @weichen2
Scenario Outline: Test start stop track and mute/unmute
  Given I have 1 <dev1> and 1 <dev2>
    And I create a venue
  When I create offer on device 1
    And I create offer on device 2
  Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
  Then I stop <direction> <media> track on device 1
  Then I mute <direction> <media> track on device 1
  Then I unmute <direction> <media> track on device 1
  Then I start <direction> <media> track on device 1
  Then I keep the call for 5 seconds  
  Then I stop <direction2> <media> track on device 2
  Then I mute <direction2> <media> track on device 2
  Then I unmute <direction2> <media> track on device 2
  Then I start <direction2> <media> track on device 2
  Then I keep the call for 5 seconds  
    And I should have received and decoded some media in devices	

  Examples:
    | dev1       | dev2       | direction | media | direction2 |
    | android    | pc         | local     | audio | local     |
    | android    | pc         | local     | video | local     |
    | ios        | pc         | local     | audio | local     |
    | ios        | pc         | local     | video | local     |
    | android    | pc         | remote    | audio | remote    |
    | android    | pc         | remote    | video | remote    |
    | ios        | pc         | remote    | audio | remote    |
    | ios        | pc         | remote    | video | remote    |


@mute-sca @dev
Scenario: Test mute with sending SCA
    Given I have 2 any devices
      And I create a venue
    When I create multilayer offer on device 1
      And I create multilayer offer on device 2
    Then I should get SDP from device 1 and create confluence
      And I should get SDP from device 2 and create confluence
    Then I keep the call for 8 seconds 
      And I should get video media status available on device 2
    Then I pause to send video on device 1
    Then I keep the call for 3 seconds
      And I should get video media status unavailable on device 2
    Then I unpause to send video on device 1
    Then I keep the call for 3 seconds  
      And I should get video media status available on device 2