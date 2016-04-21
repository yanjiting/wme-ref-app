#!/bin/bash -x 
ROOT=`pwd`
UT_DIR=$ROOT
MODULES="dolphin shark util wqos tp wme wrtp "
MODULE_NUM=8
suffix="_UT_ios_sim."

function parse_files {
modules_str=$MODULES
filenames_str=`ls`
filenames=($filenames_str)
[ ${#filenames[@]} -eq 0 ] && echo "no test results or logs generated" && exit 1
rand=`date +%s`
for filename in ${filenames[@]}; do
OIFS="$IFS"
IFS=.
name_parts=($filename)
IFS="$OIFS"
new_filename="${name_parts[0]}_$rand.${name_parts[1]}"
mv $filename $new_filename
module=${filename%$suffix$1}
modules_str=${modules_str/$module/}
done
modules_arr=($modules_str)
[ ${#modules_arr[@]} -ne 0 ] && echo "unittest $1 lost ${modules_arr[@]} modules report" #&& exit 1
}

cd $UT_DIR
mkdir report
cp /tmp/*_UT_ios_sim.xml report
cd report
parse_files "xml"
cd -
