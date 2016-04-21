#! /bin/bash

BASEDIR=$(dirname $0)
adb push $BASEDIR/tcpdump /sdcard/tcpdump

adb shell "su -c 'mount -o remount,rw /system'"
# not workin on some device
#adb shell "su -c 'mv /sdcard/tcpdump /system/bin/tcpdump'"
adb shell "su -c 'cat /sdcard/tcpdump > /system/bin/tcpdump'"
adb shell "su -c 'chown root:shell /system/bin/tcpdump'"
adb shell "su -c 'chmod 754 /system/bin/tcpdump'"
adb shell "su -c 'mount -o remount,ro /system'"
adb shell "rm /sdcard/tcpdump"
