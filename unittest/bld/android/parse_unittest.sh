ROOT=`pwd`/../../..

source $ROOT/build/android/common.sh

[ $# -lt 1 ] && echox 31 "usage: $0 report_dir" && exit 1

REPORT=$1
[ ! -e $REPORT ] && echox 31 "$REPORT is not exist!" && exit 1
[ $# -ge 2 ] && shift && modules="$*"
modlist=($modules)
total=${#modlist[@]}
echo "parse modules [$total]: $modules"

parse_unittest() {
    res=$1
    [ ! -f $res ] && return 0

    check=`cat $res | grep "<testsuites"`
    [ $? -ne 0 ] && return 0

    echox 34 "[INFO] Start to parse unittest results for `basename $res`"
    tests=`cat $res | grep "<testsuites" | awk -F " " '{print $2;}' | awk -F "\"" '{print $2;}'`
    fails=`cat $res | grep "<testsuites" | awk -F " " '{print $3;}' | awk -F "\"" '{print $2;}'`
    times=`cat $res | grep "<testsuites" | awk -F " " '{print $6;}' | awk -F "\"" '{print $2;}'`
    waste=`cat $res | grep "<testsuites" | awk -F " " '{print $7;}' | awk -F "\"" '{print $2;}'`
    msg="[INFO] Total cases: $tests, failed: $fails, time: $waste seconds, xml: `basename $res`" 
    [ $fails -ne 0 ] && echox 31 $msg || echox 33 $msg
    return $fails
}


xmlfiles=(`ls $REPORT/*.xml 2>/dev/null`)
logfiles=(`ls $REPORT/*.log 2>/dev/null`)
echox 33 "[INFO] There are [${#xmlfiles[*]}] (total $total) unittests generated."

hasfail=0
for file in ${xmlfiles[*]}; do
    parse_unittest $file
    ret=$?
    hasfail=$((hasfail+ret))
done
[ $hasfail -ne 0 ] && echox 31 "[ERRO] There are [$hasfail] cases failed!(check xml shown above)"

lossfiles=()
for mod in ${modlist[@]};
do
    test -e $REPORT/$mod-*.xml || lossfiles=(${lossfiles[*]} $mod)
done

if [ ${#lossfiles[*]} -ne 0 ]; then
    echox 31 "[ERRO] There are [${#lossfiles[*]}] unittest results not generated(may crash), check logs below: "
    for file in ${lossfiles[*]}; do
    	echox 31 "  [*] no gtest xml results for module:        $file"
    done
    exit 1
fi

[ $hasfail -ne 0 -o ${#lossfiles[*]} -ne 0 -o ${#xmlfiles[*]} -lt $total ] && exit 1

exit 0
