#!/bin/sh
RunApp=../build/Release-iphoneos/MediaSessionTest.app
AppId=com.cisco.MediaSessionTest
FromLogFile=Library/Caches/Logs/wme_ta_ios.log
ENVS=" -e  WME_PERFORMANCE_TEST_udp 173.39.168.71 -e  WME_PERFORMANCE_TEST_720p 1  -e  WME_PERFORMANCE_TEST_VIDEO_SIZE  720 "

JS_FILENAME="./trace/perf.js"
JS_DELAY=10
if [ "a$1" != "a" ]
then
    JS_DELAY=$1
fi

LOOP_SLEEP=10
if [ "a$2" != "a" ]
then
    LOOP_SLEEP=$2
fi

RUN_TIME=0
if [ "a$3" != "a" ]
then
    RUN_TIME=$3
fi



function gen_js()
{
    echo "var target = UIATarget.localTarget();" > $JS_FILENAME
    echo "var val = null;" >> $JS_FILENAME  
    echo "while (true) {" >> $JS_FILENAME
    echo "  target.delay("$JS_DELAY");" >> $JS_FILENAME
    echo "  break;">>$JS_FILENAME
    echo "}" >>$JS_FILENAME
}

function run_test()
{
    deviceIds=`system_profiler SPUSBDataType | sed -n -e '/iPad/,/Serial/p' -e '/iPhone/,/Serial/p' | grep "Serial Number:" | awk -F ": " '{print $2}'`
    for deviceId in $deviceIds
    do
        while true
        do
            echo "work dir:"`pwd`
            echo "Before run for "$ToLogFile", do sleeping $LOOP_SLEEP"
            killall instruments
            sleep $LOOP_SLEEP
            echo "deviceId: $deviceId"
            echo "Installing application"
            ./ios-deploy  --id $deviceId  --bundle $RunApp || continue
            echo "Finish install"

            echo "run $RunApp"
            instruments -w $deviceId -D ./trace/test.trace -t /Applications/Xcode.app/Contents/Applications/Instruments.app/Contents/PlugIns/AutomationInstrument.xrplugin/Contents/Resources/Automation.tracetemplate $RunApp -e UIASCRIPT $JS_FILENAME $ENVS $PARAMS || continue
            echo "After run for "$ToLogFile

            rm $ToLogFile
            ./iFileTransfer -o download -id $deviceId -app $AppId -from $FromLogFile -to $ToLogFile || continue
            killall instruments
            break
        done
        break
    done
}

gen_js

ToLogFile=./trace/wme_ta_ios_720p_hardware_loopback.log
PARAMS=" --loopback --video-hardware --disable-qos --auto-start --run-time $RUN_TIME "
run_test

ToLogFile=./trace/wme_ta_ios_720p_hardware_loopback_calliope.log
PARAMS=" --loopback --calliope --video-hardware --disable-qos --auto-start  --run-time $RUN_TIME "
run_test

ToLogFile=./trace/wme_ta_ios_720p_software_loopback.log
PARAMS=" --loopback --disable-qos --auto-start  --run-time $RUN_TIME "
run_test

ToLogFile=./trace/wme_ta_ios_720p_software_loopback_calliope.log
PARAMS=" --loopback --calliope --disable-qos --auto-start  --run-time $RUN_TIME "
run_test


