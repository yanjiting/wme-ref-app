#!/bin/bash

usage="usage: $0 [-a 32|64] [-t util/tp/wqos/wrtp/appshare/dolphin/shark/wme] debug|release|clean cc"

# parse command options 
#echo $@
args=`getopt a:t: $*`
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

# args options default value
arch=32
project="BuildAll"
# get options args value
for i
do
	   case "$i"
	   in
			   -a)
					   #echo arch is "'"$2"'"; 
					   arch="$2"; shift;
					   shift;;
               -t)
                       project="$2"; shift;
                       shift;;
			   --)
					   shift; break;;
	   esac
done
# check args value
if [ $arch = "32" ]; then
	WME_ARCH=i386
	LIB_FOLDER_NAME=mac
elif [ $arch = "64" ]; then
	WME_ARCH=x86_64
	LIB_FOLDER_NAME=mac64
else
	echo $usage
	exit 2
fi

# get project value
project=`echo ${project} | tr '[A-Z]' '[a-z]'`
WME_UNITTEST_TARGET_PROJECT="${project}"
echo Unit Test will run project:${WME_UNITTEST_TARGET_PROJECT}

# get other args value
echo $@
#[ $# -ne 1 ] && echo $usage && exit 3
# check other args value
configuration=`tr '[A-Z]' '[a-z]' <<<"$1"`
if [ $configuration = "debug" ]; then
	WME_CONFIGURATION=Debug
elif [ $configuration = "release" ]; then
	WME_CONFIGURATION=Release
elif [ $configuration = "clean" ]; then
	WME_CONFIGURATION=Clean
else
	echo $usage && exit 4
fi

if [ $# -gt 1 ]; then
    cc_opt=`tr '[A-Z]' '[a-z]' <<<"$2"`
    if [ $cc_opt = "cc" ]; then
        CODE_COVERAGE_CONFIG="-xcconfig  ./EnableCoverage.xcconfig";
        echo "project will run with code coverage enable"
    else
        echo $usage && exit 4
    fi
fi



WME_SDK=macosx

# save UT status info and result
ut_status_info=""
ut_status_result=0

function checkUTStatus {
 	echo "********************************************"
 	echo ""
 	echo -e $ut_status_info
 	echo "********************************************"
 	if [ $ut_status_result -ne 0 ]; then
 		echo "Error: found some failed status"
 		exit 2
 	fi
    return $ut_status_result
}

function recordUTStatus {
    "$@"
    status=$?
    if [ $status -ne 0 ]; then
        echo "Error with command: $1"
		ut_status_info=${ut_status_info}"[Fail]: $@\n"
		ut_status_result=1
	else
		ut_status_info=${ut_status_info}"[Success]: $@\n"
    fi
    return $status
}

function copywbtlogs()
{
   wbtfiles=`ls -1rt /tmp/*.wbt`
   wbtfiles=`echo $wbtfiles | awk '{print $(NF)" "$(NF-1)}'`
   mkdir -p "$1"/wbxlogs/$2
   for x in $wbtfiles; do
     cp $x "$1"/wbxlogs/$2
   done
}

WME_UNITTEST_PATH=../../unittest
WME_UNITTEST_BIN_PATH=../../unittest/bin/mac/$WME_CONFIGURATION
WME_UNITTEST_REPORT_PATH=./report/$WME_CONFIGURATION
WME_DYLD_FRAMEWORK_PATH=../../distribution/mac/$WME_CONFIGURATION
OPENH264_LIB_PATH=../../vendor/openh264/libs/$LIB_FOLDER_NAME
WBXTRACE_LIB_PATH=../../vendor/wbxtrace/libs/$LIB_FOLDER_NAME

echo "Start $WME_CONFIGURATION Unit Test for $WME_SDK with $WME_ARCH"

mkdir -p $WME_DYLD_FRAMEWORK_PATH

# Set environment variables
#export DYLD_FRAMEWORK_PATH=$WME_DYLD_FRAMEWORK_PATH

# Copy verdor bundles
#cp -rf $OPENH264_LIB_PATH/*.bundle $WME_DYLD_FRAMEWORK_PATH
cp -rf $OPENH264_LIB_PATH/*.dylib $WME_DYLD_FRAMEWORK_PATH
cp -rf $WBXTRACE_LIB_PATH/*.bundle $WME_DYLD_FRAMEWORK_PATH
 
# for clean
if [ "$WME_CONFIGURATION" = "Clean" ]; then
    xcodebuild clean -project $WME_UNITTEST_PATH/util/utilTest_mac.xcodeproj -target utilTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
    xcodebuild clean -project $WME_UNITTEST_PATH/tp/tpTest_mac.xcodeproj -target tpTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
    xcodebuild clean -project $WME_UNITTEST_PATH/wqos/wqosTest_mac.xcodeproj -target wqosTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
    xcodebuild clean -project $WME_UNITTEST_PATH/wrtp/wrtpTest_mac.xcodeproj -target wrtpTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
    xcodebuild clean -project $WME_UNITTEST_PATH/appshare/bld/mac/appshareTest_mac.xcodeproj -target appshareTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
    xcodebuild clean -project $WME_UNITTEST_PATH/dolphin/wbxaudioengineTest_mac.xcodeproj -target wbxaudioengineTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
    xcodebuild clean -project $WME_UNITTEST_PATH/shark/wseclientTest_mac.xcodeproj -target wseclientTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
    xcodebuild clean -project $WME_UNITTEST_PATH/wme/wmeclientTest_mac.xcodeproj -target wmeclientTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
    exit 0
fi


# Clean, Build and run WME SDK unit test projects
if [ "${WME_UNITTEST_TARGET_PROJECT}" = "util" -o  "${WME_UNITTEST_TARGET_PROJECT}" = "buildall" ] ; then
recordUTStatus xcodebuild ${CODE_COVERAGE_CONFIG} -project $WME_UNITTEST_PATH/util/utilTest_mac.xcodeproj -target utilTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
recordUTStatus $WME_UNITTEST_BIN_PATH/utilTest_mac $WME_UNITTEST_REPORT_PATH/utilTest_mac.xml
copywbtlogs $WME_UNITTEST_REPORT_PATH util
echo "Finished running unit test on the utilTest_mac target"
fi

if [ "${WME_UNITTEST_TARGET_PROJECT}" = "tp" -o  "${WME_UNITTEST_TARGET_PROJECT}" = "buildall" ] ; then
recordUTStatus xcodebuild ${CODE_COVERAGE_CONFIG} -project $WME_UNITTEST_PATH/tp/tpTest_mac.xcodeproj -target tpTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
recordUTStatus $WME_UNITTEST_BIN_PATH/tpTest_mac $WME_UNITTEST_REPORT_PATH/tpTest_mac.xml
copywbtlogs $WME_UNITTEST_REPORT_PATH tp
echo "Finished running unit test on the tpTest_mac target"
fi

if [ "${WME_UNITTEST_TARGET_PROJECT}" = "wqos" -o  "${WME_UNITTEST_TARGET_PROJECT}" = "buildall" ] ; then
recordUTStatus xcodebuild ${CODE_COVERAGE_CONFIG} -project $WME_UNITTEST_PATH/wqos/wqosTest_mac.xcodeproj -target wqosTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
recordUTStatus $WME_UNITTEST_BIN_PATH/wqosTest_mac $WME_UNITTEST_REPORT_PATH/wqosTest_mac.xml
copywbtlogs $WME_UNITTEST_REPORT_PATH wqos
echo "Finished running unit test on the wqosTest_mac target"
fi

if [ "${WME_UNITTEST_TARGET_PROJECT}" = "wrtp" -o  "${WME_UNITTEST_TARGET_PROJECT}" = "buildall" ] ; then
recordUTStatus xcodebuild ${CODE_COVERAGE_CONFIG} -project $WME_UNITTEST_PATH/wrtp/wrtpTest_mac.xcodeproj -target wrtpTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
recordUTStatus $WME_UNITTEST_BIN_PATH/wrtpTest_mac $WME_UNITTEST_REPORT_PATH/wrtpTest_mac.xml
copywbtlogs $WME_UNITTEST_REPORT_PATH wrtp
echo "Finished running unit test on the wrtpTest_mac target"
fi

if [ "${WME_UNITTEST_TARGET_PROJECT}" = "appshare" -o  "${WME_UNITTEST_TARGET_PROJECT}" = "buildall" ] ; then
recordUTStatus xcodebuild ${CODE_COVERAGE_CONFIG} -project $WME_UNITTEST_PATH/appshare/bld/mac/appshareTest_mac.xcodeproj -target appshareTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
recordUTStatus $WME_UNITTEST_BIN_PATH/appshareTest_mac --gtest_output="xml:$WME_UNITTEST_REPORT_PATH/appshareTest_mac.xml"
copywbtlogs $WME_UNITTEST_REPORT_PATH appshare
echo "Finished running unit test on the appshareTest_mac target"
fi

if [ "${WME_UNITTEST_TARGET_PROJECT}" = "dolphin" -o  "${WME_UNITTEST_TARGET_PROJECT}" = "buildall" ] ; then
recordUTStatus xcodebuild ${CODE_COVERAGE_CONFIG} -project $WME_UNITTEST_PATH/dolphin/wbxaudioengineTest_mac.xcodeproj -target wbxaudioengineTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
recordUTStatus $WME_UNITTEST_BIN_PATH/wbxaudioengineTest_mac $WME_UNITTEST_REPORT_PATH/wbxaudioengineTest_mac.xml
copywbtlogs $WME_UNITTEST_REPORT_PATH dolphin
echo "Finished running unit test on the wbxaudioengineTest_mac target"
fi

if [ "${WME_UNITTEST_TARGET_PROJECT}" = "shark" -o  "${WME_UNITTEST_TARGET_PROJECT}" = "buildall" ] ; then
recordUTStatus xcodebuild ${CODE_COVERAGE_CONFIG} -project $WME_UNITTEST_PATH/shark/wseclientTest_mac.xcodeproj -target wseclientTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
recordUTStatus $WME_UNITTEST_BIN_PATH/wseclientTest_mac $WME_UNITTEST_REPORT_PATH/wseclientTest_mac.xml
copywbtlogs $WME_UNITTEST_REPORT_PATH wse
echo "Finished running unit test on the wseclientTest_mac target"
fi

if [ "${WME_UNITTEST_TARGET_PROJECT}" = "wme" -o  "${WME_UNITTEST_TARGET_PROJECT}" = "buildall" ] ; then
recordUTStatus xcodebuild ${CODE_COVERAGE_CONFIG} -project $WME_UNITTEST_PATH/wme/wmeclientTest_mac.xcodeproj -target wmeclientTest_mac -configuration $WME_CONFIGURATION -sdk $WME_SDK -arch $WME_ARCH
recordUTStatus $WME_UNITTEST_BIN_PATH/wmeclientTest_mac $WME_UNITTEST_REPORT_PATH/wmeclientTest_mac.xml
copywbtlogs $WME_UNITTEST_REPORT_PATH wme
echo "Finished running unit test on the wmeclientTest_mac target"
fi


echo "End $WME_CONFIGURATION Unit Test for $WME_SDK with $WME_ARCH"

echo "Check $WME_CONFIGURATION Unit Test status..."
checkUTStatus


