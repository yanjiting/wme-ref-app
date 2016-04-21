#!/bin/bash

if [ $# -lt 2 ]; then
echo "USAGE: $0 libfilepath dumpfilepath|dumpfile [dstpath]"
echo " e.g.: $0 ./maps ./crash.dmp ./disfolder"
exit 1
fi
libfilepath=$1
dumpfile=$2
if [ $# -eq 2 ]; then
  dstpath="."
else
  dstpath=$3
fi

if [ ! -d "$libfilepath" ]; then
  echo "can not find the path "$libfilepath
  exit 1
fi

dumpfilecount=0
if [ -f "$dumpfile" ]; then
  dumpfilelist[dumpfilecount]=$dumpfile
  dumpfilecount=$dumpfilecount+1
fi
if [ -d "$dumpfile" ]; then
  dumpfilecount=0
  for file in `ls "$dumpfile"/*.dmp`
  do
    dumpfilelist[dumpfilecount]=$file
    dumpfilecount=$dumpfilecount+1
  done
fi

if [ $dumpfilecount = 0 ]; then
  echo "can not find any dumpfile with the name or in the folder "$dumpfile
  exit 1
fi

serverUser="wme"
serverIP="10.224.76.84"
libfolder="libfiles"
dumpfolder="dumpfiles"
now=`date "+%Y%m%d%H%M%S"`
workfolder="dumpanalyze_"$now"_"$RANDOM

echo "create the work folder on the server "$serverUser"@"$serverIP
ssh $serverUser@$serverIP > /dev/null 2>&1 << eeooff
mkdir $workfolder
cd $workfolder
mkdir $libfolder
mkdir $dumpfolder
exit
eeooff

echo "upload map files from "$libfilepath" to the server"
scp $libfilepath/*.so $serverUser@$serverIP:~/$workfolder/$libfolder 

echo "upload dump files from "$dumpfile" to the server"
for file in ${dumpfilelist[@]}
do
  scp $file $serverUser@$serverIP:~/$workfolder/$dumpfolder
done

echo "create the work folder on the server "$serverUser"@"$serverIP
ssh $serverUser@$serverIP > /dev/null 2>&1 << eeooff
cd $workfolder
../create_symbol_file.sh
../analyze_dump_file.sh 
exit
eeooff

echo "get analyzed dump file from the server"
mkdir -p $dstpath
scp $serverUser@$serverIP:~/$workfolder/$dumpfolder/*.dmp.txt $dstpath

echo "delete workfolder on the server"
ssh $serverUser@$serverIP "rm -rf $workfolder"

echo "Please check the analyzed dump file under the folder "$dstpath


