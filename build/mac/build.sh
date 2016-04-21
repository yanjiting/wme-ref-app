#!/bin/bash

usage="usage: $0 [-a 32|64] [-c] [-C] [-d] [-r]"
# -d means debug only
# -r means release only
# -d/-r cannot exist together, and without -d/-r will compile both
# parse command options 
#echo $@
args=`getopt cCa:dr $*`
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
WITH_DEBUG="true"
WITH_RELEASE="true"
# get args value
for i
do
	   case "$i"
	   in
			   -a)
					   #echo arch is "'"$2"'"; 
					   arch="$2"; shift;
					   shift;;
               -c)
                       echo "project build with clean"
                       PROJECT_CLEAN="clean";
                       shift;;
               -d)
                       echo "debug only"
                       WITH_DEBUG="true"
                       WITH_RELEASE="false"
                       shift;;
               -r)
                       echo "release only"
                       WITH_DEBUG="false"
                       WITH_RELEASE="true"
                       shift;;
               -C)
                       CODE_COVERAGE_CONFIG="-xcconfig  ./EnableCoverage.xcconfig";
                       echo "project will run with code coverage enable"
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

WME_SDK=macosx

WME_SRC_PATH=../../mediaengine
MEDIASESSION_PROJ_PATH=../../mediasession/bld/mac/MediaSession.xcodeproj
OPENH264_LIB_PATH=../../vendor/openh264/libs/$LIB_FOLDER_NAME
WBXTRACE_LIB_PATH=../../vendor/wbxtrace/libs/$LIB_FOLDER_NAME
DISTRIBUTION_PATH=../../distribution/mac

function exitOnFailure {
    "$@"
    status=$?
    if [ $status -ne 0 ]; then
        echo "Error with command: $1"
        exit $status
    fi
    return $status
}

dateBegin=$(date +"%s")

if [ $WITH_RELEASE == "true" ]; then
echo "Start Release Build for $WME_SDK with $WME_ARCH"
dateBeforeRelease=$(date +"%s")
mkdir -p $DISTRIBUTION_PATH/Release
# Copy verdor bundles
cp -rf $OPENH264_LIB_PATH/*.dylib $DISTRIBUTION_PATH/Release
cp -rf $WBXTRACE_LIB_PATH/*.bundle $DISTRIBUTION_PATH/Release

# Build WME SDK projects with release configuration
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/util/src/util.xcodeproj -target util -configuration Release -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the util target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/tp/src/tp.xcodeproj -target tp -configuration Release -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the tp target"
#exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/wp2p/src/wp2p_mac.xcodeproj -target wp2p_mac -configuration Release -sdk $WME_SDK -arch $WME_ARCH
#echo "Finished running xcodebuild on the wp2p_mac target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/appshare/bld/client/mac/appshare.xcodeproj -target appshare -configuration Release -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the appshare target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/wqos/src/wqos.xcodeproj -target wqos -configuration Release -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wqos target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/transmission/src/wrtp.xcodeproj -target wrtp -configuration Release -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wrtp target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/wmeutil/bld/client/mac/wmeutil.xcodeproj -target wmeutil -configuration Release -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wmeutil target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/shark/bld/client/mac/wsertp.xcodeproj -target wsertp -configuration Release -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wsertp target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/shark/src/wsevp/build/mac/wsevp.xcodeproj -target wsevp -configuration Release -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wsevp target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/shark/bld/client/mac/wseclient.xcodeproj -target wseclient -configuration Release -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wseclient target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/dolphin/bld/client/mac/wbxaecodec.xcodeproj -target wbxaecodec -configuration Release -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wbxaecodec target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/dolphin/bld/client/mac/wbxaudioengine.xcodeproj -target wbxaudioengine -configuration Release -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wbxaudioengine target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/wme/bld/client/mac/wmeclient.xcodeproj -target wmeclient -configuration Release -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wmeclient target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $MEDIASESSION_PROJ_PATH -target MediaSession -configuration Release -sdk $WME_SDK -arch $WME_ARCH

dateAfterRelease=$(date +"%s")

echo "Finished running xcodebuild on the mediasession target"
echo "End Release Build for $WME_SDK with $WME_ARCH"
diff=$(($dateAfterRelease-$dateBeforeRelease))
echo "$(($diff / 60)) minutes and $(($diff % 60)) seconds elapsed."
fi


if [ "${WITH_DEBUG}" == "true" ]; then
dateBeforeDebug=$(date +"%s")
echo "Start Debug Build for $WME_SDK with $WME_ARCH"
mkdir -p $DISTRIBUTION_PATH/Debug
# Copy verdor bundles
cp -rf $OPENH264_LIB_PATH/*.dylib $DISTRIBUTION_PATH/Debug
cp -rf $WBXTRACE_LIB_PATH/*.bundle $DISTRIBUTION_PATH/Debug
# Build WME SDK projects with debug configuration
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/util/src/util.xcodeproj -target util -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the util target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/tp/src/tp.xcodeproj -target tp -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the tp target"
#exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/wp2p/src/wp2p_mac.xcodeproj -target wp2p_mac -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
#echo "Finished running xcodebuild on the wp2p_mac target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/appshare/bld/client/mac/appshare.xcodeproj -target appshare -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the appshare target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/wqos/src/wqos.xcodeproj -target wqos -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wqos target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/transmission/src/wrtp.xcodeproj -target wrtp -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wrtp target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/wmeutil/bld/client/mac/wmeutil.xcodeproj -target wmeutil -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wmeutil target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/shark/bld/client/mac/wsertp.xcodeproj -target wsertp -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wsertp target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/shark/src/wsevp/build/mac/wsevp.xcodeproj -target wsevp -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wsevp target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/shark/bld/client/mac/wseclient.xcodeproj -target wseclient -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wseclient target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/dolphin/bld/client/mac/wbxaecodec.xcodeproj -target wbxaecodec -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wbxaecodec target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/dolphin/bld/client/mac/wbxaudioengine.xcodeproj -target wbxaudioengine -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wbxaudioengine target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $WME_SRC_PATH/wme/bld/client/mac/wmeclient.xcodeproj -target wmeclient -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the wmeclient target"
exitOnFailure xcodebuild ${CODE_COVERAGE_CONFIG} ${PROJECT_CLEAN} build -project $MEDIASESSION_PROJ_PATH -target MediaSession -configuration Debug -sdk $WME_SDK -arch $WME_ARCH
echo "Finished running xcodebuild on the mediasession target"
echo "End Debug Build for $WME_SDK with $WME_ARCH"
dateAfterDebug=$(date +"%s")
diff=$(($dateAfterDebug-$dateBeforeDebug))
echo "$(($diff / 60)) minutes and $(($diff % 60)) seconds elapsed."
fi

dateEnd=$(date +"%s")
diff=$(($dateEnd-$dateBegin))
echo "Total: $(($diff / 60)) minutes and $(($diff % 60)) seconds elapsed."
