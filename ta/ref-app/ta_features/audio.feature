Feature: Test Set Audio Parameters

@AudioSetOption @dev
Scenario Outline: Test iOS enableAGC
    Given I have 1 <type1> and 1 <type2>
      And I create a venue
    When I start call without creating offer on device 1 
    When I start call without creating offer on device 2
    Then I keep the call for 5 seconds  
    Then I set the following Audio parameters for device 1 as: <option> = <optvalues>
    Then I set the following Audio parameters for device 2 as: <option> = <optvalues>
    Then I keep the call for 5 seconds 
    Then I should receive the following value from device 1, Audio parameter: <option> == <optvalues>
    Then I should receive the following value from device 2, Audio parameter: <option> == <optvalues>
    Then I set the following Audio parameters for device 1 as: <option> = <optvalues2>
    Then I keep the call for 5 seconds
    Then I should receive the following value from device 1, Audio parameter: <option> == <optvalues2>
    Then I stop the call on device 1
    Then I stop the call on device 2  

    Examples:
    | type1   | type2   | option     | optvalues | optvalues2 |
    | mac     | ios     | enableAGC  | false     | true       |
    | ios     | mac     | enableVAD  | true      | false      |
    | ios     | mac     | enableNS   | true      | false      |
    | mac     | ios     | enableEC   | true      | false      |
    | mac     | android | enableAGC  | false     | true       |
    | android | mac     | enableVAD  | true      | false      |
    | android | mac     | enableNS   | true      | false      |
    | android | ios     | enableEC   | true      | false      |


@AudioDropSeconds @dev
Scenario Outline: Test Audio DropSeconds feature
    Given I have 1 <dev1> and 1 <dev2>
    And I create a venue
    When I create offer on device 1
    And I create offer on device 2
    Then I keep the call for 5 seconds
    Then I should get SDP from device 1 and create confluence
    And I should get SDP from device 2 and create confluence
    Then I keep the call for 5 seconds
    Then I set the following Audio parameters for device 1 as: enableDropSeconds = true
    Then I keep the call for 1600 milliseconds
    Then I should receive the following value from device 1, Audio parameter: enableDropSeconds == true
    Then I should sent out audio packets less than 5 on device 1
    Then I set the following Audio parameters for device 1 as: enableDropSeconds = false
    Then I keep the call for 2 seconds
    Then I should receive the following value from device 1, Audio parameter: enableDropSeconds == false
    Then I should sent out audio packets more than 10 on device 1

Examples:
    | dev1       | dev2       |
    | android    | mac        |
    | mac        | ios        |
    | ios        | android    |


@audio-quality-test-on-ios @dev
Scenario: Test audio quality on different situation
    Given I have 1 mac and 1 ios
        Then I start play sound FB_female_conditioning_seq_short1600 on device 2 and capture DAGC_Out
        Then I start play sound FB_conditioning_short1600 on device 1 and capture DAGC_Out
        Then I keep the call for 15 seconds
        Then I stop play sound on device 2
        Then I stop the call on device 2
        Then I stop the call on device 1
        Then I copy captured file from device 2

