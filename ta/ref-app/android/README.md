Android Reference App
======================

Develop with Android Studio
-----------------------------

Open Android Studio and import ta/ref-app/android/ClickCallGradle
This is only supported on MAC right now because of file links are used.

How to profile
---------------------

Refer to: http://google.github.io/fplutil/index.html
install android_ndk_perf as in: 

    git clone --recursive https://github.com/google/fplutil.git  
    
and add bin into PATH.

    cd ta/ref-app/android
    python build_wme.py debug profile; python build.py debug profile; python deploy.py debug;
    adb shell am force-stop com.wx2.clickcall;adb shell am start -n com.wx2.clickcall/com.wx2.clickcall.MainActivity -e loopback 1 -e WME_PERFORMANCE_TEST_VIDEO_SIZE 360 -e auto-start 1 -e disable-qos 1 -e video-hardware 1
    android_ndk_perf --no-launch-on-start --apk ClickCall/target/com-wx2-clickcallapp-1.0.0-SNAPSHOT.apk record -o output/perf.data
    android_ndk_perf visualize -i output/perf.data -o report.html

How to run with particular parameters
----------------------
Here is an example, it will run as 720p, loopback, enable hardware codec

    adb shell am force-stop com.wx2.clickcall;adb shell am start -n com.wx2.clickcall/com.wx2.clickcall.MainActivity -e loopback 1 -e WME_PERFORMANCE_TEST_VIDEO_SIZE 720 -e auto-start 1 -e disable-qos 1 -e video-hardware 1
