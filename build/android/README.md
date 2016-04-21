Build instructions for Android
===============================
WME python/shell building/unittest scripts, which support windows/linux/mac machine.
 

Pre-requisition
-------------------------------
1.  Install python >=2.7.x and add it to PATH (verify with: `python --version`)
2.  Install JDK >=1.7.x and add it to PATH (verify with: `javac -version`)
3.  Install ANT and add it to PATH (verify with: `ant -version`)
4.  Download and unzip Android NDK(>=r10e), add ENV variable `ANDROID_NDK_HOME` and `ANDROID_NDK`, also add it to PATH
5.  Download and unzip Android SDK, add ENV variable `ANDROID_HOME`, also add it to PATH
    execute ${ANDROID_HOME}/tools/android to update to the API 19. (you also can use SDK manager GUI tool to manage API)
6.  Install cmake and add it to PATH (verify with: `cmake --version`)


Building scripts (python) for Android platform (for mac/linux/windows)
----------------------------------------------------------------
1. 'python build.py [configuration] [version] [cc]' will build all libraries, cc means enable code coverage, for example: python build.py release 2.8.0 cc 

2. 'python build_ut.py [configuration] [cc]' will build all unit test related modules

3. 'python run_ut.py [apk_path]' will run the unit test in attached device, apk_path is optional, default is the result of step 2.


How to run unittest apk with specified module manually?
--------------------------------------------------------
1. 'adb install -r AllUnitTest-???.apk' to install it in devices

2. Select all modules for testing:
    <module>: all,
    <path>: the absolute directory of output,
        'adb shell am start --es module "all" --es path "/sdcard" -n com.cisco.wme.unittest/.MainActivity'

    After running if normal (no crush):
        'adb pull /sdcard/all-tp-gtest.xml'
        'adb pull /sdcard/all-util-gtest.xml'
        'adb pull /sdcard/all-wqos-gtest.xml'
        'adb pull /sdcard/all-wrtp-gtest.xml'
        'adb pull /sdcard/all-dolphin-gtest.xml'
        'adb pull /sdcard/all-shark-gtest.xml'
        'adb pull /sdcard/all-wme-gtest.xml'

3. Select one module for testing:
    <module>: tp, util, wqos, wrtp, dolphin, shark and wme,
    <path>:  the absolute path of output file,
        'adb shell am start --es module "tp" --es path "/sdcard/tp-gtest.xml" -n com.cisco.wme.unittest/.MainActivity'

    After running if normal (no crush):
        'adb pull /sdcard/tp-gtest.xml'

