#1. Install cmake and add it to PATH in ~/.bash_profile 

(verify with: `cmake --version`)  

#2 Install xcpretty 
    gem install xcpretty

#3. Build all of the library of Media SDK for iOS platform 
    ./build.sh [cc] [clean] [debug/release] [sim/dev]
    
### cc: ( option parameter)  
  for enable the  code coverage function.  
   
### clean: ( option parameter)  
  force to clean it before building the project  
   
### debug/release:  ( option parameter)  
  only build debug or release, default build all  
    
### sim/dev: ( option parameter)  
  only build simulator or device, default build all  

#4. Run all unit test on simulator 
 a) generate all libs of media SDK using build.sh [cc]  
 b) install ios-sim tool for start app on simulator on command line. As in page [https://github.com/phonegap/ios-sim](https://github.com/phonegap/ios-sim)   
 c) use ./build_ut.sh sim release [cc] or ./build_ut.sh sim debug [cc]  
 d) run bash file to get XML files gtest XML file and trace file are in /tmp/ folder of Mac machine.  

#5. Run all unit test on device 
 a) generate all libs of media SDK using build.sh [cc]  
 b) use ./build_ut.sh dev release [cc] or ./build_ut.sh dev debug [cc]  
 c) gtest XML files are in document folder of sandbox of the iOS application in device.  

#6. Run one unit test project on simulator   
 a) generate all libs of media SDK using build.sh [cc]  
 b) install ios-sim tool for start app on simulator on command line. As in page [https://github.com/phonegap/ios-sim](https://github.com/phonegap/ios-sim)   
 c) use ./build_ut.sh sim release [cc] PROJECT_NAME or ./build_ut.sh sim debug [cc] PROJECT_NAME  
 d) run bash file to get XML files gtest XML file and trace file are in /tmp/ folder of Mac machine.  

#7. Run one unit test project on device 
 a) generate all libs of media SDK using build.sh [cc]  
 b) use ./build_ut.sh dev release [cc] PROJECT_NAME or ./build_ut.sh dev debug [cc] PROJECT_NAME  
 c) gtest XML files are in the current folder (the same as this file). 

#Note: 
if you want to get the code coverage information, you need to do the following steps:   
        
    ./build.sh cc
    ./build_ut.sh … cc ...
      
