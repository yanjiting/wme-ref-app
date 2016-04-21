The xcode project lives in ref-app/iOS/ as MediaSessionTest.xcodeproj


To build the iOS test application from the command line:


    Step -1: Install xcode
    Install Xcode 5.x from the app store or from the developer.apple.com DMG

    You'll also need the developer provisioning profile (see developer.apple.com and user
    wx2-client-apple@cisco.com)


    Step 0:   Clone the Source Code

          git clone git@sqbu-github.cisco.com:WebExSquared/wme-wx2-test.git
          cd wme-wx2-test

    This will bring in the other modules needed by this project, namely WME

          git submodule update --init --recursive


    Step 1:   Building the Application
    The xcode project lives in ref-app/iOS/ as MediaSessionTest.xcodeproj

          cd ref-app/iOS
          ./build/build.sh
        
    This script places the .app into an output folder of build_output/Applications/MediaSessionTest.app


    Step 2:   Installing Calabash Commands

    The wme-wx2-test project is using calabash for automated testing.
    Calabash and cucumber depend on ruby. To setup ruby we use rvm (the ruby version manager)

    Install [RVM](https://rvm.io) and open a new terminal window to the same directory.

    This command should install the rvm command on your system

          curl -sSL https://get.rvm.io | bash -s stable
          OR
          sudo curl -sSL https://get.rvm.io | bash -s stable

    Then

          < assuming you have done cd wme-wx2-test   >
          rvm install 1.9.3      # this may take a very long time to run
          rvm use 1.9.3
          rvm gemset create wme-wx2-test

    When you open a new terminal window RVM may report this

                ==============================================================================
                = NOTICE                                                                     =
                ==============================================================================
                = RVM has encountered a new or modified .rvmrc file in the current directory =
                = This is a shell script and therefore may contain any shell commands.       =
                =                                                                            =
                = Examine the contents of this file carefully to be sure the contents are    =
                = safe before trusting it! ( Choose v[iew] below to view the contents )      =
                ==============================================================================
                Do you wish to trust this .rvmrc file? (/Users/zackchandler/dev/wx2-ios-client/.rvmrc)
                y[es], n[o], v[iew], c[ancel]>

                 Type "y"

    Quick check to see that rvm worked -

          rvm gemset list (make sure the => points to wx2-ios-client)

                gemsets for ruby-1.9.3-p327 (found in /Users/zackchandler/.rvm/gems/ruby-1.9.3-p327)
                   (default)
                => wme-wx2-test

          gem install calabash-cucumber
          gem install cucumber
          gem install nokogiri    # This one takes a long time
          bundle install


    Step 4:  Installing and Running the Application and Automatic Tests
    At this point you have built the application and you've installed the calabash command line tools.

    In order to run the automatic tests you'll need some details:
      - your device's wifi IP address
        Go to Settings / Wi-Fi / <click on the i-in-a-circle> / IP Address  to see this 
        On Blizzard this address will change every few days

     - Iff you have multiple devices attached you'll need the device Id of the 
       device you want to run the tests on.
       You can view the device IDs within xcode in the Organzier window
       Or you can use this command


          system_profiler SPUSBDataType | sed -n -e '/iPad/,/Serial/p' -e '/iPhone/,/Serial/p' | grep 'Serial Number:'

          cd ref-app/iOS
          ./build/install_and_run.sh
    
    
    HERE BLEAIR  NOT DONE YET
    To manually a run test, or run cucumber

    cd ref-app/iOS/calabash_files
    cucumber -r features features/common features/iphone NO_LAUNCH=1 DEVICE=iphone DEVICE_ENDPOINT=http://10.0.1.9:37265

