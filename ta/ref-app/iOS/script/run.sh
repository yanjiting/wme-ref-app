#!/bin/bash
#
#Soya Li

RunApp=../build/Release-iphoneos/MediaSessionIntegrationTest.app

function exitOnFailure {
"$@"
status=$?
if [ $status -ne 0 ]; then
echo "Error with command: $1"
exit $status
fi
return $status
}

num=1
deviceIds=`system_profiler SPUSBDataType | sed -n -e '/iPad/,/Serial/p' -e '/iPhone/,/Serial/p' | grep "Serial Number:" | awk -F ": " '{print $2}'`
echo "all devices = $deviceIds"

while read -r deviceId
do
envDevice="IOS_DEVICE$num"
export $envDevice=$deviceId
echo "$envDevice=$deviceId"

echo "Installing application"
./ios-deploy  --id $deviceId  --bundle $RunApp
echo "Finish install"

echo "run MediaSessionIntegrationTest on device to get IP"
killall instruments
output=`instruments -w $deviceId -D ./trace/getip.trace  -t /Applications/Xcode.app/Contents/Applications/Instruments.app/Contents/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate $RunApp -e UIASCRIPT getip.js`
echo "output=$output"

envIP="IOS_IP$num"
NEWLINE=$'\n'
temp="${output%$NEWLINE*}"
deviceIP="${temp##*Pass: }"
export $envIP=$deviceIP
echo "$envIP=$deviceIP"
num=$(($num + 1))
echo "Finish to get IP"
done <<< "$deviceIds"

cucumber ../../features/startcall.feature "$@"
