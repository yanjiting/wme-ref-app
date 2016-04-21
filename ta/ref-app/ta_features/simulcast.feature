Feature: multi-sources simulcast negotiation

@mac-simulcast-p2p @intensive
Scenario Outline: Subscribe multi-layers video in p2p call.
        Given I have 1 mac device
        When I start loopback p2p multilayer call
        Then I keep the call for 3 seconds
        And I subscribe specific layers video with <Parameters>
        Then I keep the call for 10 seconds
        Then I should received specific layers video less then <Result>
        
        Examples:
            | Parameters                         | Result |
            |mbps 450 fs 60 fps 1500 dpb 0       |180p    |
            |mbps 3600 fs 240 fps 1500 dpb 0     |360p    |
            |mbps 3600 fs 240 fps 3000 dpb 0     |360p    |
            |mbps 27600 fs 920 fps 1500 dpb 0    |720p    |
            |mbps 27600 fs 920 fps 2400 dpb 0    |720p    |
            |mbps 27600 fs 920 fps 3000 dpb 0    |720p    |
            |mbps 3000 fs 396 fps 3000 dpb 0     |180p    |
            |mbps 7200 fs 396 fps 3000 dpb 0     |360p    |
            |mbps 27600 fs 920 fps 3000 dpb 0    |720p    |
            |mbps 27600 fs 1200 fps 3000 dpb 0   |720p    |
            |mbps 108000 fs 3600 fps 3000 dpb 0  |1080p   |

@ios-simulcast-p2p @intensive
Scenario Outline: Subscribe multi-layers video in p2p call.
        Given I have 1 ios device
        When I start loopback p2p multilayer call
        Then I keep the call for 3 seconds
        And I subscribe specific layers video with <Parameters>
        Then I keep the call for 10 seconds
        Then I should received specific layers video less then <Result>
        
        Examples:
            | Parameters                         | Result |
            |mbps 450 fs 60 fps 1500 dpb 0       |180p    |
            |mbps 3600 fs 240 fps 1500 dpb 0     |360p    |
            |mbps 3600 fs 240 fps 3000 dpb 0     |360p    |
            |mbps 27600 fs 920 fps 1500 dpb 0    |720p    |
            |mbps 27600 fs 920 fps 2400 dpb 0    |720p    |
            |mbps 27600 fs 920 fps 3000 dpb 0    |720p    |
            |mbps 3000 fs 396 fps 3000 dpb 0     |180p    |
            |mbps 7200 fs 396 fps 3000 dpb 0     |360p    |
            |mbps 27600 fs 920 fps 3000 dpb 0    |720p    |
            |mbps 27600 fs 1200 fps 3000 dpb 0   |720p    |

 