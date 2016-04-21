Feature: screenshare TA

#***************************************************************************
# Screen Sharing TA - Basic Logic TA Cases
#***************************************************************************

# Windows Platform loopback & Win -> Win
#========================================================================

@screenshare-win-loopback-desktop @dev @intensive @windows
Scenario: screenshare loopback desktop in windows
	Given I have 1 windows device
        When I start screenshare as _sharer_ on device 1 with the default setting and:
            | LOOPBACK  | true   |
            | SHARE     | sharer |
	    | SOURCE     | screen: |
		Then I keep the screenshare for 30 seconds
        Then I wait for screenshare ended on device 1
        Then I check screenshare  _both_ result on device 1  
		
@screenshare-win-loopback-app @dev @intensive @windows
Scenario: screenshare loopback application in windows
	Given I have 1 windows device
        When I start screenshare as _sharer_ on device 1 with the default setting and:
            | LOOPBACK  | true   |
            | SHARE     | sharer |
		Then I keep the screenshare for 30 seconds
        Then I wait for screenshare ended on device 1
        Then I check screenshare  _both_ result on device 1  
		  
@screenshare-win-p2p-desktop @dev @intensive @windows
Scenario: screenshare p2p desktop in windows
	Given I have 2 windows device
                When I create offer on device 1 with the default setting and:
                  | ICE     | true  |
		  | AUDIO   | false |
		  | VIDEO   | false |
                  | SHARE   | sharer   |
		  | SOURCE     | screen: |
                  And I set offer to device 2 with the default setting and:
                  | ICE     | true  |
				  | AUDIO   | false |
				  | VIDEO   | false |
                  | SHARE   | viewer  |
  		Then I should receive answer from device 2 and set to device 1
		Then I keep the screenshare for 30 seconds
        Then I wait for screenshare ended on device 1
        Then I wait for screenshare ended on device 2
        Then I check screenshare  _sharer_ result on device 1  
        Then I check screenshare  _viewer_ result on device 2  
		
@screenshare-win-p2p-desktop-pause @dev @intensive @windows
Scenario: screenshare p2p desktop pause/resume in windows
	Given I have 2 windows device
                When I create offer on device 1 with the default setting and:
                  | ICE     | true  |
		  | AUDIO   | false |
		  | VIDEO   | false |
                  | SHARE   | sharer   |
 		  | SOURCE     | screen: |
                  And I set offer to device 2 with the default setting and:
                  | ICE     | true  |
				  | AUDIO   | false |
				  | VIDEO   | false |
                  | SHARE   | viewer  |
  		Then I should receive answer from device 2 and set to device 1
		Then I keep the screenshare for 30 seconds
		Then I check screenshare  _sharer_ result on device 1  
        Then I check screenshare  _viewer_ result on device 2
        Then I pause sharing on device 1
        Then I check screenshare _pause_ 4 seconds at _viewer_ side on device 2
        Then I resume sharing on device 1
        Then I check screenshare _resume_ 30 seconds at _viewer_ side on device 2

@screenshare-win-p2p-app @dev @intensive @windows
Scenario: screenshare p2p app in windows
	Given I have 2 windows device
		When I create offer on device 1 with the default setting and:
                  | ICE     | true  |
				  | AUDIO   | false |
				  | VIDEO   | false |
                  | SHARE   | sharer   |
                  And I set offer to device 2 with the default setting and:
                  | ICE     | true  |
				  | AUDIO   | false |
				  | VIDEO   | false |
                  | SHARE   | viewer  |
  		Then I should receive answer from device 2 and set to device 1 
		Then I keep the screenshare for 30 seconds
        Then I wait for screenshare ended on device 1
        Then I wait for screenshare ended on device 2
        Then I check screenshare  _sharer_ result on device 1  
        Then I check screenshare  _viewer_ result on device 2  
		
# Mac Platform loopback & Mac -> Mac
#========================================================================
@screenshare-mac-loopback-desktop @dev @intensive @mac
Scenario: screenshare loopback desktop in mac
	Given I have 1 mac device
        When I start screenshare as _sharer_ on device 1 with the default setting and:
            | LOOPBACK  | true   |
            | SHARE     | sharer |
	    | SOURCE     | screen: |
		Then I keep the screenshare for 30 seconds
        Then I wait for screenshare ended on device 1
        Then I check screenshare  _both_ result on device 1  
		
@screenshare-mac-loopback-app @dev @intensive @mac
Scenario: screenshare loopback application in mac
	Given I have 1 mac device
        When I start screenshare as _sharer_ on device 1 with the default setting and:
            | LOOPBACK  | true   |
            | SHARE     | sharer |
		Then I keep the screenshare for 30 seconds
        Then I wait for screenshare ended on device 1
        Then I check screenshare  _both_ result on device 1  
		  
@screenshare-mac-p2p-desktop @sanity @intensive @mac
Scenario: screenshare p2p desktop in mac
	Given I have 2 mac device
		Then I keep the screenshare for 5 seconds
		When I create p2p offer on device 1 with the default setting and:
                  | ICE     | true  |
                  | SHARE   | sharer   |
                  | SOURCE     | screen: |
                  And I set offer to device 2 with the default setting and:
                  | ICE     | true  |
                  | SHARE   | viewer  |
  		Then I should receive answer from device 2 and set to device 1
		Then I keep the screenshare for 30 seconds
        Then I wait for screenshare ended on device 1
        Then I wait for screenshare ended on device 2
        Then I check screenshare  _sharer_ result on device 1  
        Then I check screenshare  _viewer_ result on device 2
    Then I check file size of each trace in a call lasting 0 seconds
  
        
@screenshare-mac-p2p-desktop-pause @dev @intensive @mac
Scenario: screenshare p2p desktop pause/resume in mac
	Given I have 2 mac device
		Then I keep the screenshare for 5 seconds
		When I create offer on device 1 with the default setting and:
                  | ICE     | true  |
                  | SHARE   | sharer   |
                  | SOURCE     | screen: |
                  And I set offer to device 2 with the default setting and:
                  | ICE     | true  |
                  | SHARE   | viewer  |
  		Then I should receive answer from device 2 and set to device 1
  		Then I keep the screenshare for 30 seconds
        Then I check screenshare  _sharer_ result on device 1  
        Then I check screenshare  _viewer_ result on device 2
        Then I pause sharing on device 1
        Then I check screenshare _pause_ 4 seconds at _viewer_ side on device 2
        Then I resume sharing on device 1
        Then I check screenshare _resume_ 4 seconds at _viewer_ side on device 2

@screenshare-mac-p2p-app @dev @intensive @mac
Scenario: screenshare p2p app in mac
	Given I have 2 mac device
		Then I keep the screenshare for 5 seconds
		When I create p2p offer on device 1 with the default setting and:
                  | ICE     | true  |
                  | AUDIO   | false    |
                  | VIDEO   | false    |
                  | SHARE   | sharer   |
                  And I set offer to device 2 with the default setting and:
                  | ICE     | true  |
                  | AUDIO   | false    |
                  | VIDEO   | false    |
                  | SHARE   | viewer  |
  		Then I should receive answer from device 2 and set to device 1
		Then I keep the screenshare for 30 seconds
        Then I wait for screenshare ended on device 1
        Then I wait for screenshare ended on device 2
        Then I check screenshare  _sharer_ result on device 1  
        Then I check screenshare  _viewer_ result on device 2  

# iOS Platform Mac -> iOS
#========================================================================
@screenshare-ios-p2p-desktop @intensive @mac
Scenario: screenshare p2p desktop in iOS
	  Given I have 1 mac and 1 ios
		Then I keep the screenshare for 5 seconds
  		When I create offer on device 1 with the default setting and:
                  | ICE     | true  |
                  | SHARE   | sharer   |
                  | SOURCE     | screen: |
                  And I set offer to device 2 with the default setting and:
                  | ICE     | true  |
                  | SHARE   | viewer  |
  		Then I should receive answer from device 2 and set to device 1
		Then I keep the screenshare for 30 seconds
        Then I wait for screenshare ended on device 1
        Then I wait for screenshare ended on device 2
        Then I check screenshare  _sharer_ result on device 1  
        Then I check screenshare  _viewer_ result on device 2  

@screenshare-ios-linus-desktop @sanity @intensive @mac
Scenario: screenshare linus desktop in iOS
    Given I have 1 mac and 1 ios
        And I create a venue
        Then I keep the screenshare for 5 seconds
        When I create offer on device 1 with the default setting and:
                | ICE     | true  |
                | SHARE   | sharer   |
                | SOURCE     | screen: |
        And I set offer to device 2 with the default setting and:
                | ICE     | true  |
                | SHARE   | viewer  |
        Then I should receive answer from device 2 and set to device 1
        Then I keep the screenshare for 30 seconds
        Then I wait for screenshare ended on device 1
        Then I wait for screenshare ended on device 2
        Then I check screenshare  _sharer_ result on device 1
        Then I check screenshare  _viewer_ result on device 2

@screenshare-ios-p2p-desktop-file @sanity @intensive @mac
Scenario: screenshare p2p file capture in iOS
Given I have 1 mac and 1 ios
Given I screen share file capture resource screen_ppt_2880_1800_RGBA.raw
Then I keep the screenshare for 5 seconds
When I create offer on device 1 with the default setting and:
| ICE     | true  |
| SHARE   | sharer   |
| SHARE_SOURCE_FILE | screen_ppt_2880_1800_RGBA.raw |
And I set offer to device 2 with the default setting and:
| ICE     | true  |
| SHARE   | viewer  |
Then I should receive answer from device 2 and set to device 1
Then I keep the screenshare for 30 seconds
Then I wait for screenshare ended on device 1
Then I wait for screenshare ended on device 2
Then I check screenshare  _sharer_ result on device 1
Then I check screenshare  _viewer_ result on device 2



@screenshare-ios-p2p-desktop-pause @dev @intensive @mac
Scenario: screenshare p2p desktop pause/resume in iOS
	Given I have 1 mac and 1 ios
		Then I keep the screenshare for 5 seconds
		When I create offer on device 1 with the default setting and:
                  | ICE     | true  |
                  | SHARE   | sharer   |
                  | SOURCE     | screen: |
                  And I set offer to device 2 with the default setting and:
                  | ICE     | true  |
                  | SHARE   | viewer  |
  		Then I should receive answer from device 2 and set to device 1
  		Then I keep the screenshare for 30 seconds
        Then I check screenshare  _sharer_ result on device 1  
        Then I check screenshare  _viewer_ result on device 2
        Then I pause sharing on device 1
        Then I check screenshare _pause_ 4 seconds at _viewer_ side on device 2
        Then I resume sharing on device 1
        Then I check screenshare _resume_ 4 seconds at _viewer_ side on device 2

@screenshare-ios-p2p-app @sanity @intensive @mac
Scenario: screenshare p2p app in iOS
	  Given I have 1 mac and 1 ios
		Then I keep the screenshare for 5 seconds
		When I create offer on device 1 with the default setting and:
                  | ICE     | true  |
                  | SHARE   | sharer   |
                  And I set offer to device 2 with the default setting and:
                  | ICE     | true  |
                  | SHARE   | viewer  |
  		Then I should receive answer from device 2 and set to device 1
		Then I keep the screenshare for 30 seconds
        Then I wait for screenshare ended on device 1
        Then I wait for screenshare ended on device 2
        Then I check screenshare  _sharer_ result on device 1  
        Then I check screenshare  _viewer_ result on device 2  
    Then I check file size of each trace in a call lasting 0 seconds


# Android Platform Mac -> Android
#========================================================================
@screenshare-android-p2p-desktop @sanity @intensive @mac
Scenario: screenshare p2p desktop with android
	  Given I have 1 mac and 1 android
		Then I keep the screenshare for 5 seconds
		When I create p2p sharer offer on device 1 with the default setting and:
                  | ICE     | true     |
                  | SHARE   | sharer   |
 		  | SOURCE     | screen: |
                  And I set offer to device 2 with the default setting and:
                  | ICE     | true  |
                  | SHARE   | viewer  |
  		Then I should receive answer from device 2 and set to device 1
		Then I keep the screenshare for 30 seconds
        Then I wait for screenshare ended on device 1
        Then I wait for screenshare ended on device 2
        Then I check screenshare  _sharer_ result on device 1  
        Then I check screenshare  _viewer_ result on device 2  
        
@screenshare-android-p2p-desktop-pause @dev @intensive @mac
Scenario: screenshare p2p desktop pause/resume with android
	  Given I have 1 mac and 1 android
		Then I keep the screenshare for 5 seconds
		When I create p2p sharer offer on device 1 with the default setting and:
                  | ICE     | true     |
                  | SHARE   | sharer   |
 		  | SOURCE     | screen: |
                  And I set offer to device 2 with the default setting and:
                  | ICE     | true  |
                  | SHARE   | viewer  |
  		Then I should receive answer from device 2 and set to device 1
		Then I keep the screenshare for 30 seconds
        Then I check screenshare  _sharer_ result on device 1  
        Then I check screenshare  _viewer_ result on device 2  
        Then I pause sharing on device 1
        Then I check screenshare _pause_ 10 seconds at _viewer_ side on device 2
        Then I resume sharing on device 1
        Then I check screenshare _resume_ 11 seconds at _viewer_ side on device 2

@screenshare-android-p2p-app @dev @intensive @mac
Scenario: screenshare p2p app with android
	  Given I have 1 mac and 1 android
		Then I keep the screenshare for 5 seconds
		When I create p2p offer on device 1 with the default setting and:
                  | ICE     | true     |
                  | SHARE   | sharer   |
                  And I set offer to device 2 with the default setting and:
                  | ICE     | true  |
                  | SHARE   | viewer  |
  		Then I should receive answer from device 2 and set to device 1
		Then I keep the screenshare for 30 seconds
        Then I wait for screenshare ended on device 1
        Then I wait for screenshare ended on device 2
        Then I check screenshare  _sharer_ result on device 1  
        Then I check screenshare  _viewer_ result on device 2

#***************************************************************************
# Screen Sharing TA - Capture Content Checking TA Cases
#***************************************************************************

# Windows Desktop Sharing Content
#========================================================================
@screenshare-content-win-loopback-desktop-common @dev @intensive @windows
Scenario: screenshare loopback desktop share conent checking on windows
	Given I have 1 windows screenshare dummy app	
		Then I create 4 row 8 column qrcode view with x=0 y=0 w=0 h=0 on asDummyApp 1
	
	Given I have 1 windows device
        When I start screenshare as _sharer_ on device 1 with the default setting and:
            | LOOPBACK  | true   |
            | SHARE     | sharer |
	    | SOURCE     | screen: |
		Then I set qrcode scan context with row=4 column=8 x=0 y=0 w=0 h=0 on device 1
		Then I keep the screenshare for 30 seconds
		Then I check screenshare content is 4 row 8 column qrcode on device 1

# Mac Desktop Sharing Content
#========================================================================
@screenshare-content-mac-loopback-desktop-common @dev @intensive @mac
Scenario: screenshare loopback desktop share conent checking on mac
        
    Given I have 1 mac screenshare dummy app
        Then I create 4 row 8 column qrcode view with x=0 y=0 w=0 h=0 on asDummyApp 1

    Given I have 1 mac device
        When I start screenshare as _sharer_ on device 1 with the default setting and:
            | LOOPBACK  | true   |
            | SHARE     | sharer |
            | SOURCE     | screen: |

        Then I set qrcode scan context with row=4 column=8 x=0 y=0 w=0 h=0 on device 1
        Then I keep the screenshare for 30 seconds
        Then I check screenshare content is 4 row 8 column qrcode on device 1


# Windows Desktop Sharing Content without self app
#========================================================================


# Mac Desktop Sharing Content without self app
#========================================================================




#***************************************************************************
# Screen Sharing TA - Performance TA Cases
#***************************************************************************
@screenshare-mac-mobile-p2p-preformance-cpu-all @dev @intensive @mac @performance
Scenario: screenshare  desktop sharing mac to mobile p2p to check performance cpu for all device
	Given I run all performance test case on mobile devices

@screenshare-mac-mobile-loopback-preformance-cpu-all @dev @intensive @mac @performance
Scenario: mobile loopback audio and video performance testing 
	Given I run all loopback performance test case on mobile devices

@screenshare-mac-mobile-loopback-hardware-preformance-cpu-all @dev @intensive @mac @performanceX
Scenario: mobile loopback audio and video hardware performance testing 
	Given I run all loopback hardware performance test case on mobile devices
	
#***************************************************************************
# Screen Sharing TA - Network/Quality TA Cases
#***************************************************************************
@screenshare-mac-loopback-production-linus-package-loss @dev @intensive @mac
Scenario: screenshare loopback desktop sharing by production linus on mac
    Given I have 1 mac device
        When I start screenshare as _sharer_ on device 1 with the default setting and:
            | LOOPBACK      | true   |
            | SHARE         | sharer |
            | AUDIO         | true   |
 	    | SOURCE     | screen: |
 
            Then I keep the screenshare for 60 seconds
            Then I wait for screenshare ended on device 1
            Then I check screenshare  _both_ result on device 1
            Then I check screenshare  _both_ network statistic on device 1



@screenshare-mac-ios-p2p-package-loss @dev @intensive @mac
Scenario: screenshare  desktop sharing mac to iOS p2p to checking pacakge loss
    Given I have 1 mac and 1 ios
        Then I keep the screenshare for 5 seconds
    When I create offer on device 1 with the default setting and:
        | ICE     | true  |
        | SHARE   | sharer   |
       | SOURCE     | screen: |
    And I set offer to device 2 with the default setting and:
        | ICE     | true  |
        | SHARE   | viewer  |
    Then I should receive answer from device 2 and set to device 1
    Then I keep the screenshare for 60 seconds
    Then I wait for screenshare ended on device 1
    Then I wait for screenshare ended on device 2
    Then I check screenshare  _sharer_ result on device 1
    Then I check screenshare  _viewer_ result on device 2
    Then I check screenshare  _sharer_ network statistic on device 1
    Then I check screenshare  _sharer_ network statistic on device 2

@screenshare-mac-ios-p2p-production-linus-package-loss @dev @intensive @mac
Scenario: screenshare  desktop sharing mac to iOS by production linus on mac
    Given I have 1 mac and 1 ios
    #Given I have 2 mac device
        And I create a venue in production
        Then I keep the screenshare for 5 seconds
        When I create offer on device 1 with the default setting and:
            | ICE     | true  |
            | SHARE   | sharer   |
            | AUDIO   | true   |
            | VIDEO   | true   |
            | SOURCE     | screen: |
        And I create offer on device 2 with the default setting and:
                | ICE     | true  |
                | SHARE   | viewer  |
                | AUDIO   | true   |
                | VIDEO   | true   |
        Then I should get SDP from device 1 and create confluence
            And I should get SDP from device 2 and create confluence
        Then I request floor on device 1
        Then I keep the screenshare for 60 seconds
        Then I wait for screenshare ended on device 1
        Then I wait for screenshare ended on device 2
        Then I check screenshare  _sharer_ result on device 1
        Then I check screenshare  _viewer_ result on device 2
        Then I check screenshare  _sharer_ network statistic on device 1
        Then I check screenshare  _sharer_ network statistic on device 2


@screenshare-mac-loopback-qos-disable-init_bw @dev @intensive @mac
Scenario: screenshare loopback with different initial bandwidth
  Given I have 1 mac device
        When I start screenshare as _sharer_ on device 1 with the default setting and:
            | LOOPBACK  | true   |
            | SHARE     | sharer |
	    | SOURCE     | screen: |
	    | ENABLEQOS | false |
	Then I keep the screenshare for 15 seconds
        Then I set Share initial bandwidth as 1024kbps for device 1
        Then I keep the screenshare for 5 seconds
        Then Share evaluated bandwidth should <= 1024kbps
        Then I set Share initial bandwidth as 80kbps for device 1
        Then I keep the screenshare for 5 seconds
        Then Share evaluated bandwidth should <= 80kbps
        Then I wait for screenshare ended on device 1
        Then I check screenshare  _both_ result on device 1  



#***************************************************************************
# Screen Sharing TA - Other Cases
#***************************************************************************
