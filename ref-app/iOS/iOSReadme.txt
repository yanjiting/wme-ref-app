

--->Media SDK iOS Demo Readme<---


System environment for build/debug

1. Supported iOS target: iOS 4.0 and later;
2. Xcode version: Xcode 4.5.2 and later;
3. iOS SDK: Latest iOS SDK with Xcode (iOS 6.0 and later);
4. Mac OS X: Mac OS X 10.7 and later.




Compile and run

1. Prepare a Mac laptop or iMac PC(Intel CPU based),

2. Connect a iPad into the Mac, make sure it is iPad 2 and above version, no any iPhones are supported so far, 

3. download the reference package, which includes reference program and all the required binaries from this SVN site, https://10.224.166.140/svn/mediasdk, username: mediasdk password: cisco123

4. unpack the package and enter its folder, e.g. EngineReleasePackage-ios,

5. Compile reference program,

   use project file MediaEngineTestApp.xcodeproj in ref-app\MediaEngineTestApp.xcodeproj,

6. please use your own provisioning profile, and configure it into project file for build/debug,

7. Run Demo,
   >check your iPad if it is connected(the app will be installed into device by Xcode automatically),
   >or locate the APP in ref-app\bin\Debug-iphoneos or ref-app\bin\Release-iphoneos, and sync it into your iPad(with organizer or iphone configuration utility), so you can test it anytime.

8. You will be able to track the performance/bitrate/memory/system resource usage of this APP with Xcode/Instruments.



Any additional questions or comments, please contact Wei-Lien Hsu(weihsu@cisco.com) or Alan Zhu(shangzhu@cisco.com).


