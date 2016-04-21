#!/bin/bash

usage="usage: $0 [-a 32|64]"

# parse command options 
#echo $@
args=`getopt a: $*`
# you should not use `getopt abo: "$@"` since that would parse
# the arguments differently from what the set command below does.
if [ $? != 0 ]
then
	   echo $usage
	   exit 1
fi
#echo $args
set -- $args
# You cannot use the set command with a backquoted getopt directly,
# since the exit code from getopt would be shadowed by those of set,
# which is zero by definition.

# args default value
arch=32
# get args value
for i
do
	   case "$i"
	   in
			   -a)
					   #echo arch is "'"$2"'"; 
					   arch="$2"; shift;
					   shift;;
			   --)
					   shift; break;;
	   esac
done
# check args value
if [ $arch = "32" ]; then
	WME_ARCH=i386
elif [ $arch = "64" ]; then
	WME_ARCH=x86_64
else
	echo $usage
	exit 2
fi

WME_SDK=macosx

WME_REFAPP_PATH=`pwd`

function exitOnFailure {
    "$@"
    status=$?
    if [ $status -ne 0 ]; then
        echo "Error with command: $1"
        exit $status
    fi
    return $status
}

echo "Start Ref-App Release Build for $WME_SDK with $WME_ARCH in Package"
# Clean WME ref-app projects
xcodebuild clean -project $WME_REFAPP_PATH/MediaEngineTestApp.xcodeproj -target MediaEngineTestApp -configuration Release -sdk $WME_SDK -arch $WME_ARCH
# Build WME ref-app projects
exitOnFailure xcodebuild -project $WME_REFAPP_PATH/MediaEngineTestApp.xcodeproj -target MediaEngineTestApp -configuration Release -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the MediaEngineTestApp target"
echo "End Ref-App Release Build for $WME_SDK with $WME_ARCH in Package"

echo "Start Ref-App Debug Build for $WME_SDK with $WME_ARCH in Package"
# Clean WME ref-app projects
xcodebuild clean -project $WME_REFAPP_PATH/MediaEngineTestApp.xcodeproj -target MediaEngineTestApp -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
# Build WME ref-app projects
exitOnFailure xcodebuild -project $WME_REFAPP_PATH/MediaEngineTestApp.xcodeproj -target MediaEngineTestApp -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the MediaEngineTestApp target"
echo "End Ref-App Debug Build for $WME_SDK with $WME_ARCH in Package"

