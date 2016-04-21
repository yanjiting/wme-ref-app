#!/bin/bash

usage="usage: $0 [-a 32|64] debug|release"

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

# get other args value
[ $# -ne 1 ] && echo $usage && exit 3
# check other args value
configuration=`tr '[A-Z]' '[a-z]' <<<"$1"`
if [ $configuration = "debug" ]; then
	REFAPP_CONFIGURATION=Debug
elif [ $configuration = "release" ]; then
	REFAPP_CONFIGURATION=Release
else
	echo $usage && exit 4
fi

WME_SDK=macosx

WME_REFAPP_PATH=../../ref-app/mac
DISTRIBUTION_PATH=../../distribution/mac/$REFAPP_CONFIGURATION

function exitOnFailure {
    "$@"
    status=$?
    if [ $status -ne 0 ]; then
        echo "Error with command: $1"
        exit $status
    fi
    return $status
}

echo "Start Ref-App $REFAPP_CONFIGURATION Build for macosx"
# Clean WME ref-app projects
xcodebuild clean -project $WME_REFAPP_PATH/MediaEngineTestApp/MediaEngineTestApp.xcodeproj -target MediaEngineTestApp -configuration $REFAPP_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
# Build WME ref-app projects
exitOnFailure xcodebuild -project $WME_REFAPP_PATH/MediaEngineTestApp/MediaEngineTestApp.xcodeproj -target MediaEngineTestApp -configuration $REFAPP_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the MediaEngineTestApp target"
echo "End Ref-App $REFAPP_CONFIGURATION Build"

echo "Start Run $REFAPP_CONFIGURATION Ref-App"
# Set environment variables
#export DYLD_FRAMEWORK_PATH=$DISTRIBUTION_PATH
exitOnFailure $WME_REFAPP_PATH/MediaEngineTestApp/bin/$REFAPP_CONFIGURATION/MediaEngineTestApp.app/Contents/MacOS/MediaEngineTestApp
echo "End Run $REFAPP_CONFIGURATION Ref-App"
