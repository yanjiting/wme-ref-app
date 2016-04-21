

# ARG 1 IP address of device.  Once we have a working build script this can be learned
#DEVICE_IP="171.68.21.175"
DEVICE_IP=$1
if [ "$DEVICE_IP" == "" ]; then
    echo " "
    echo "ERROR. Can't continue."
    echo "Couldn't determine the device's IP. Please specify it on command line: $0 <device_ip> <device_udid>  linus_ip  linus_venueId"
    exit -1
fi
IP_ADDR_DOT_COUNT="${DEVICE_IP//[^.]}" 
if [ ${#IP_ADDR_DOT_COUNT} -ne 3  ]; then
    echo " "
    echo "ERROR. Can't continue."
    echo "Couldn't determine the device's IP address. Please specify it on command line: $0 <device_ip> <device_udid>  linus_ip  linus_venueId"
    exit -1
fi
# If we had a build script this would be able to auto-obtain the IP addrs of the device
#OUR_APP=./build_output/Applications/MediaSessionIntegrationTest.app
#echo DEVICE_IP=`instruments -w ${DEVICE_ID} -t /Applications/Xcode.app/Contents/Applications/Instruments.app/Contents/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate ${OUR_APP}  -e UIASCRIPT ./build/getip.js  -e UIARESULTSPATH /tmp/uiautomation_out -D /tmp/uiautomation_out  | head -1 | awk -F ": " '{print $2}'`
#DEVICE_IP=`instruments -w ${DEVICE_ID} -t /Applications/Xcode.app/Contents/Applications/Instruments.app/Contents/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate ${OUR_APP}  -e UIASCRIPT ./build/getip.js  -e UIARESULTSPATH /tmp/uiautomation_out -D /tmp/uiautomation_out  | head -1 | awk -F ": " '{print $2}'`



# ARG 2 device ID of iphone
DEVICE_ID=$2
if [ "$DEVICE_ID" == "" ]; then
    echo " "
    echo "ERROR. Can't continue."
    echo "Couldn't determine the device's ID Please specify it on command line: $0 <device_ip> <device_udid>  linus_ip  linus_venueId"
    echo "Run this - "
    echo "      system_profiler SPUSBDataType | sed -n -e '/iPad/,/Serial/p' -e '/iPhone/,/Serial/p' | grep 'Serial Number:'  "
    exit -1
fi

# ARG 3 the linus node's IP
#
#LINUS_SERVER="192.168.34.12"
#export LINUS_SERVER=http://192.168.34.12:5000/
#
LINUS_SERVER=$3
export LINUS_SERVER="http://${LINUS_SERVER}:5000/"

LINUS_IP_ADDR_DOT_COUNT="${LINUS_SERVER//[^.]}" 
if [ ${#LINUS_IP_ADDR_DOT_COUNT} -ne 3  ]; then
    echo " "
    echo "ERROR. Can't continue."
    echo "Couldn't determine the Linus server IP address. Please specify it on command line: $0 <device_ip> <device_udid>  linus_ip  linus_venueId"
    echo "    If running against a vagrant linus the default IP address is 192.168.34.12"
    exit -1
fi
echo "Will use LINUS_SERVER of $LINUS_SERVER"

# ARG 4 the venue ID
LINUS_VENUE_ID=$4
if [ "$LINUS_VENUE_ID" == "" ]; then
    echo " "
    echo "ERROR. Can't continue."
    echo "Couldn't determine the Linus venueId. Please specify it on command line: $0 <device_ip> <device_udid>  linus_ip  linus_venueId"
    exit -1
fi
export LINUS_VENUE_ID="$LINUS_VENUE_ID"


export IOS_IP1="$DEVICE_IP"
# NO SECOND DEVICE YET
export IOS_IP2="$DEVICE_IP"
echo "iPhone IP $IOS_IP1"




cucumber   ../../features/linuscall.feature:7  BUNDLE_ID=com.cisco.MediaSessionTest DEVICE=iphone DEVICE_TARGET=${DEVICE_ID}   DEVICE_ENDPOINT=http://${DEVICE_IP}:37265  NO_LAUNCH=1

