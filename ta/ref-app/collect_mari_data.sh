#!/bin/bash

if [ $# -ne 1 ] ; then
    echo "./`basename "$0"` [network_interface]"
    echo "network interface like: en0, en1, for vpn connection, should be utun0"
    exit 1
fi

realpath() {
    [[ $1 = /* ]] && echo "$1" || echo "$PWD/${1#./}"
}

SCRIPT_PATH=$(realpath "$0")
ORIGIN_DIR=$(dirname $SCRIPT_PATH)

cd $ORIGIN_DIR
echo $ORIGIN_DIR

cucumber ta_features -t @parallel -t @calliope -t @local NOINSTALL=1 LINUS_SERVER=http://10.224.166.110:5000/ NETWORK_INF=$1 SKIPANDROID=1

if [ $? -ne 0 ]; then
    exit -1
fi

log_dir="trace"

cd $log_dir
last_log=`ls -rt | tail -n1`
last_dir_path="${last_log}"
tar_basename="${last_log}@$HOSTNAME"
tar_name="${tar_basename}.tar.gz"
printf "tar dir:\n\t - $last_dir_path\n"
printf "into tar file:\n\t - $tar_name\n"
tar -zcvf $tar_name $last_dir_path
mv $tar_name ../
cd ..

HOST=${FTP_HOST:-'171.68.16.44'}
USER='jeromy'
PASSWD='cisco123'
REMOTEPATH='Downloads/mari-data-sets'
SRCPATH=$(realpath $tar_name)

ftp -n $HOST <<END_SCRIPT
quote USER $USER
quote PASS $PASSWD
cd $REMOTEPATH
bin
put $SRCPATH $tar_name
quit
END_SCRIPT

rm $SRCPATH

exit 0
