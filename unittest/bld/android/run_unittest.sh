ROOT=`pwd`/../../..

source $ROOT/build/android/common.sh
check_env
check_opt $*

[ $profile = "clean" ] && exit 0

apk_name=$ROOT/unittest/bld/android/bin/AllUnitTest-$profile.apk
[ ! -e $apk_name ] && echox 31 "Fail to find unittest APK: $apk_name." && exit 1

ccheck=0
xreport=0
devidx=-1
modules="wtp util wqos wrtp dolphin shark wme"
if [ $# -ge 2 ]; then
    if [ "$2" = "ccheck" ]; then
        ccheck=1    
    elif test ${2:0:8} = "xreport="; then
        xreport=${2#xreport=} && shift
    else
        shift
        test ${1:0:7} = "devidx=" && devidx=${1#devidx=} && shift
        [ $# -ge 1 ] && modules="$*"
    fi
fi
echo "[*] The devidx of android: $devidx"
echo "[*] The modules of unitest: $modules"

modlist=($modules)
MODNUM=${#modlist[@]}
REPORT=$ROOT/build/android/report/$profile

get_unittest() {
    dev=$1
    mod=$2
    xml=$3
    echox 34 "Run and get unittest results for device of $dev: $mod: $xml"

    rand=`date +%s`
    res="$REPORT/$mod-$dev-$rand.xml"
    log="$REPORT/$mod-$dev-$rand.log"

    # start logcat and apk
    ret=`$ADB -s $dev logcat -c`
    ret=`$ADB -s $dev logcat -v time >$log &`
    $ADB -s $dev shell am start --es module "$mod" --es path "$xml" -n com.cisco.wme.unittest/.MainActivity

    # TODO: parse and check gtest xml result
    for (( ; ; )); do
        ret=`$ADB -s $dev shell ps | grep com.cisco.wme.unittest`
        if [ $? -ne 0 ]; then
            sleep 2
            ret=`$ADB -s $dev shell ps | grep com.cisco.wme.unittest`
            [ $? -ne 0 ] && break
        fi
        sleep 2
    done

    # kill logcat
    pid=`$ADB -s $dev shell ps | grep logcat | awk '{print $2;}'`
    [ "$pid" != "" ] && $ADB -s $dev shell kill $pid >/dev/null

    # adb pull xml
    $ADB -s $dev pull $xml $res 
}

run_unittest() {
    # get devices
    devices=(`$ADB devices | awk -F" " '/\tdevice/{print $1}'`)
    devnum=${#devices[*]}
    [ $devnum -eq 0 ] && echox 31 "[Warn] no android devices" && exit 1

    dev=""
    rand=`date +%s`
    if [ $ccheck -eq 1 ]; then
        for id in ${devices[*]}; do
            tmpdir=/tmp/wme_gcov_$rand
            mkdir -p $tmpdir 
            cd $tmpdir
            $ADB -s $id pull /sdcard/wme_gcov 2>/dev/null && retstr="ok" || retstr="fail"
            cd -
            [ "$retstr" = "ok" ] && dev=$id && break
        done
        [ "$dev" = "" ] && echo "[Warn] No previous code coverage in devices" && exit 1
    else
        idx=0
        [ $devidx -eq -1 ] && idx=$((rand % devnum)) || idx=$((devidx % devnum))
        dev=${devices[idx]}
        [ "$dev" = "" ] && echo "[Warn] No android devices: $idx" && exit 1
    fi

    echo "Running on device of $dev"
    $ADB -s $dev uninstall com.cisco.wme.unittest
    $ADB -s $dev install -r $apk_name
    for mod in ${modlist[@]}; do
        echox 32 "running unittest of $mod for devices of $dev"
        xml="/sdcard/$mod-gtest.xml"
        $ADB -s $dev shell rm $xml
        get_unittest $dev $mod $xml
        ret=`$ADB -s $dev get-state`
        [ "$ret" != "device" ] && echox 31 "[Erro] device disconnected: $dev" && exit 1
    done
}

if [ $xreport -ge 1 ]; then
    bash parse_unittest.sh $REPORT $modules
    [ $? -ne 0 ] && exit 1
elif [ $xreport -le -1 ]; then
    [ -e $REPORT ] && rm -rf $REPORT
elif [ $devidx -ne -1 ]; then
    mkdir -p $REPORT
    run_unittest
else
    [ -e $REPORT ] && rm -rf $REPORT
    mkdir -p $REPORT
    run_unittest
    bash parse_unittest.sh $REPORT $modules
    [ $? -ne 0 ] && exit 1
fi

exit 0
