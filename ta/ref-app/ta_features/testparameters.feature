Feature: Test varying of client parameter settings, particularly, it is described in git issue #47


@test-parameters @dev @pc @weichen2
Scenario Outline: Make a loopback call with overrided settings
	Given I have 1 pc device
    When I override static perf with low.json
    When I start loopback call with:
		|video  |{"selectedCodec" : ["<CODEC>"] }|
        |feature|{"activerequestsd": true}       |
		|video  |{"decodeCodec" : [{"codec":"<CODEC>","uProfileLevelID":"<PROFILE>","max-mbps":<MAXMBPS>,"max-fs":<MAXFS>,"max-fps":<MAXFPS>,"max-br":<MAXBR>}]}|
    Then I check the sdp has max-mbps=27600;max-fs=<MAXFS>;max-fps=3000;
    Then I keep the call for 8 seconds  
      And I should have decoded some media in devices with:
        |video |pixels | 230400 |


     Examples:
    | CODEC    | PROFILE  |  MAXMBPS | MAXFS | MAXFPS | MAXBR |
    | H264     | 42000c   | 27600    | 3840  | 3000   | 1000  |


@test-batchcall @dev @pc @weichen2
Scenario: Make a loopback call with a lot of times
	Given I have 1 pc device
    When I override static perf with low.json
    Then I start loopback call 2 times
