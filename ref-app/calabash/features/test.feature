Feature: testing 

@audio_iOS
Scenario: testing the device set up audio file io iOS
    Given I have 2 ios devices
    And one ios device as the host, the other ios device as the client
    And set the input audio file of the host to 1_16000_16_female1.pcm
    And set the output audio file of the client to 1_16000_16_female1_target0.pcm
    Then set up the connection
    Then check the output audio file of the client with 1_16000_16_female1.pcm

@Resolution_Change_iOS
Scenario: testing when input resolution change in iOS
    Given I have 2 ios devices
    And one ios device as the host, the other ios device as the client
    Then set up the connection
    Then I change resolution in encoder param

@Resolution_Change_Android_90P
Scenario: testing when input resolution change in Android
    Given I have 2 android devices
    And one android device as the host, the other android device as the client
    Then I change resolution in encoder param to 90P
    Then set up the connection

@Resolution_Change_Android_180P
Scenario: testing when input resolution change in Android
    Given I have 2 android devices
    And one android device as the host, the other android device as the client
    Then I change resolution in encoder param to 180P
    Then set up the connection

@Resolution_Change_Android_360P
Scenario: testing when input resolution change in Android
    Given I have 2 android devices
    And one android device as the host, the other android device as the client
    Then I change resolution in encoder param to 360P
    Then set up the connection
    
@audio_android_0
Scenario: testing the device set up audio file io android
    Given I have 2 android devices
    And one android device as the host, the other android device as the client
    And set the input audio file of the host to fem1_1_16000_16_abc.pcm
    And set the output audio file of the client to output.pcm
    Then set up the connection
    #Then check the output audio file of the client
    Then I keep the call for 10 seconds
    #Then I cleanup resources

@audio_android_1
Scenario: testing the device set up audio file io android
    Given I have 2 android devices
    And one android device as the host, the other android device as the client
    #And set the input audio file of the host to fem1_1_16000_16_abc.pcm
    #And set the output audio file of the client to output.pcm
    Then set up the connection
    Then check the output audio file of the client with fem1_1_16000_16_abc.pcm
#    Then I keep the call for 10 seconds
