How to develop or run automation test
======================================

   cucumber/calabash scripts/features are located at: `ref-app/ta_features` which can cover windows/mac/ios/android/winphone. 

   Notes: to feature developers, please write your case which can run in one device as it is possible since that will be easier to test and debug. 

Install calabash and cucumber
------------------------------
   
   1. Prerequisition:
       - MAC: 
       
            XCode commandline develop tools: check with `xcode-select -p`
            bundle: `sudo gem install bundler` 
            
       - Windows
       
            ruby, dev-tool and bundle
            
   2. Install with bundle, in this folder. (you need to have bundle installed at first, it is now the preferred way of installing calabash)
   
        * bundle install
        
        
Build guides:
-----------------------------
   > Firstly build WME:

   > Automatically build ref-app in one script:
      
      * ios: ta/ref-app/ios/script/build.sh
      * android: ta/ref-app/android/build.py
      * mac: ta/ref-app/MacOSX/MediaSessionTest/build/build_refapp.sh (64bit only)
      * windows/win phone: not provided
      
   > or Manually build  ref-app with development tools:
      
      1. ios:  ta/ref-app/iOS/MediaSessionTest.xcodeproj (MediaSessionIntegrationTest schema)
      2. mac: ta/ref-app/MacOSX/MediaSessionTest/MediaSessionTest.xcodeproj (MediaSessionTATest schema) 
      3. android: import with Eclipse or Android Studio from: ta/ref-app/android/ClickCall, as Eclipse project.
      4. Windows: ta/ref-app/windows/MediaSessionTest.sln (ClickCall project as start project)
      5. Windows Phone: ta/ref-app/WP8/Refapp-wp/Refapp-wp.sln


Run integration test with cucumber:
--------------------------------------

   Please ensure that you have mobile devices are attached to the host and drivers are fine installed. For android, USB debug has been enabled and authorized in the phone. For iOS, make sure the device has been added to trust list of your host.
   Please also make sure the device and host machine (desktop) in the same network, no VPN.

   1. Run all features without set Linus server: `cucumber ta_features`
   2. Run all features with Linus server: `cucumber ta_features LINUS_SERVER="http://<your linus IP>:5000/"`
   3. Run specific scenario with tags: `cucumber ta_features -t @sanity`
   4. Run scenarios skip android devices: `cucumber ta_features SKIPANDROID=1`
   5. Run scenarios skip ios devices: `cucumber ta_features SKIPIOS=1`
   6. We provided a simple wrap: `ref-app/run_ta`, please use `ruby run_ta --help` to find its usage

   Note: trace file will be captured and saved in `ref-app/trace` folder.


How to develop a TA case
------------------------

   1. Give: we provided two given statement for your choice:
    
        Given I have 2 <type> devices
        Given I have 1 <type> and 1 <type>
        
        where, <type> = [android|ios|mac|windows|pc|any], pc means it is windows or mac. 
        
   2. When: we provided a lot of mode for your option, basically:
    
        * loopback mode:
        * 2 device call mode:
        * file capture/render mode:
        * p2p mode:
        
   3. Then: we provided a lot of checkpoints, it depends on your requirement. The most general one is:
    
        Then I should have received and decoded some media in devices
        
        
   Example:
    
        @startcall @sanity @weichen2
        Scenario Outline: Make a call with default settings through android to ios
          Given I have 1 <dev1> and 1 <dev2>
            And I create a venue
          When I create offer on device 1
            And I create offer on device 2
          Then I should get SDP from device 1 and create confluence
            And I should get SDP from device 2 and create confluence
          Then I keep the call for 10 seconds
          Then I should have received and decoded some media in devices

          Examples:
            | dev1       | dev2     |
            | ios        | android  |
            | ios        | ios      |

    
Supported Backdoors
------------------------

  - **backdoorStartCall**: Place a call, create offer, it can take a JSON as parameter
  - **backdoorGetLocalSdp**: Get the local(offer) SDP created in backdoorStartCall, this SDP can be used to interact with Linus to get SDP answer
  - **backdoorSetRemoteSdp**: Set the remote(answer) SDP received from Linus, it will start to connect to linus and sending/receiving media
  - **backdoorGetStatistics**: Get the call statistics
  - **backdoorStopCall**: Stop the call when a test case is done
  - **backdoorIsFileCaptureEnded**:
  - **backdoorGetNetworkMetrics**:
  - **backdoorMuteUnMute**:
  - **backdoorStartStopTrack**:
  - **backdoorUpdateSdp**:
  - **backdoorSetRemoteOffer**:
  - **backdoorSetParam**:
  - **backdoorGetParam**:
  - **backdoorGetVideoParameters**:
  - **backdoorGetAudioParameters**:
  - **backdoorSetAudioParam**:
  - **backdoorSetVideoParam**:
  - **backdoorSetShareParam**:
  - **backdoorGetCSIChangeHistory**:
  - **backdoorGetCSICount**:
  - **backdoorGetMediaStatus**:
  - **backdoorGetVideoCSI**:
  - **backdoorCheckHWSupport**:
  - **backdoorStopSharing**:
  - **backdoorGetSharingStatus**:
  - **backdoorGetSharingResult**:
  - **backdoorSetQRCodeContext**:
  - **backdoorCollectReceivedQRCodeContents**:
  - **backdoorSubscribe**:
  - **backdoorRequestFloor**:
  - **backdoorOutputTrace**:
  - **backdoorScreenChangeCaptureFps**:
  - **backdoorSetScreenFileCaptureFilePath**:
  - **backdoorGetVoiceLevel**:
  - **backdoorOverridePerfJson**:
  - **backdoorCheckSyncStatus**:
  - **backdoorSetFecParam**:


How to debug a TA Case
-------------------

1.  Debug WIN/MAC

    *  Compile and run the project MediaSeesionTATest(ClickCall) in debug mode at first with command line: `--ta PORT`
    *  `ruby run_ta -t @tag --debug PORT` (If your desktop device is not the first device in the case, use :PORT, or ::PORT)

2.  Debug IOS

    *  Compile and run the project MediaSeesionIntegrationTest in debug mode at first,
    *  `ruby run_ta -t @tag --debug-ios IP`, where IP is the ip address of the ios device.


Notes for case developer on how to tag a case
---------------------------------------------

   Note: Please add tags for every case, the first tag MUST be unique in global, we predefined following tags, please also tag your CEC for your case:
   
   * @sanity
      * This tag includes the cases will be run in hourly Jenkins job, the case MUST be stable and pass every time.
   * @newtest
      * This tag includes the new test cases for Jenkins, to make sure they are stable enough for the local test devices. These tests include virtual rotation with onDecodeSize call, runtime callback events, set/get video and audio parameters. 
      * For this tag, let's send notifications to our internal scrum team only and let these test cases run for a few days. If the results are favorable and stable enough, I can change these to @sanity tag.
   * @intensive
      * There are two test cases under @intensive tag that would significantly take more time to finish than others. This includes long calls while checking for parameters every two minutes, and checking confluence while restarting the call rapidly
      * We could run these test cases over the weekend when no commitments are in place.
   * @dev
      * These test cases are currently failing all the time and they are required to be fixed. Some of these parameters are in low priority now, so we do not have to add these in the official build pipeline yet. 
      * We could use this tag during development phase and allow Jenkins try your test case in our local test devices.
