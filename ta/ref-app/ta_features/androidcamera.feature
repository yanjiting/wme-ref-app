Feature: Test Android Camera Occupied Features

@android-camera @dev @sonhe @deliver
Scenario: android camera is occupied
        Given I have 1 android device
        When I open camera
        When I start loopback call
        Then I get media session return error code 0x46024101
        When I close camera
        When I start loopback call
        Then I get media session return error code 0x0
  	
