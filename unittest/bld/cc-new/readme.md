##How to use CC script to generate code coverage info##

**1. Make sure the gcov version is 4.2.1, if xcode 5.1 is installed, please rollback gcov like this.**  
   `sudo cp /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/gcov /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/gcov_bak`
   `sudo ln -fs  /Applications/Xcode.app/Contents/Developer/usr/bin/gcov-4.2 /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/gcov` 
   
**2. Build the source code of all module of wme.**  
    **- ios:**   
   `./wme/trunk/build/ios/build.sh cc`   
    **- anroid:**  
   `./wme/trunk/build/android/build.sh release cc` 

**3. Build and run all unit test module with "cc" parameter.**  
    **- ios:**   
        `./wme/trunk/build/ios/build_ut.sh sim release cc`   
    **- anroid:**  
        `./wme/trunk/build/android/build_ut.sh release cc`  
        `./wme/trunk/build/android/run_ut.sh release`

**4. Install Lcov on build enviroment'**  
   `cd ./wme/trunk/vendor/lcov-1.10`  
   `sudo make install`

**5. Generate the gcov file via '/wme/trunk/unittest/bld/ci/run'**
    `./wme/trunk/unittest/bld/ci/run [ios|Android] [sim|dev] [release]`  
     Remind: code coverage do not support debug model now.

**6. Publish the xml file on jenkins via Cobertura Coverage plugin.**  
    - `Post build action -> Add post-build action -> Publicsh Cobertura Report`  
 
