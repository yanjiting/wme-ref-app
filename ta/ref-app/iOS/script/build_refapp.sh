#!/bin/bash
#
#Soya Li
#cd ../../wme/build/ios
#sh build.sh
#cd ../../../ref-app/iOS

if [ $# -gt 2 ]; then
echo "*please use command $0 [cc] [clean]"
echo "*cc      - option: enable code coverage function"
echo "*clean   - option: force to clean it before building the project"
exit 2
fi


for PARAM in $*; do
if [ "clean" = "${PARAM}" ]; then
PROJECT_CLEAN="clean"
elif [ "cc" = "${PARAM}" ]; then
CODE_COVERAGE_CONFIG="./EnableCoverage.xcconfig"
echo "Target will run with code coverage enable"
else
echo parameters are illegal!!!, please have a check.
exit 2
fi
done

WMETEST_PROJECT=../MediaSessionTest.xcodeproj

function exitOnFailure {
    "$@"
    status=$?
    if [ $status -ne 0 ]; then
        echo "Error with command: $1"
        exit $status
    fi
    return $status
}

#echo "Start In-House Build for iOS"
#xcodebuild clean -project WebExSquared.xcodeproj -target WebExSquared -configuration InHouse -sdk iphoneos
#exitOnFailure xcodebuild -project WebExSquared.xcodeproj -target WebExSquared -configuration InHouse -sdk iphoneos
#echo "Finished running xcodebuild on the WebExSquared target"

#exitOnFailure /usr/bin/xcrun -sdk iphoneos PackageApplication -v $PWD/build/InHouse-iphoneos/WebExSquared.app -o $PWD/WebExSquaredInHouse.ipa --sign "iPhone Distribution: Cisco Systems, Inc." --embed "$PWD/build/cert/WebExSquared-InHouse.mobileprovision"
#echo "Finished creating WebExSquaredInHouse.ipa"
#echo "End In-House Build"

echo "Start Ref Tests Build for iOS"
if [ ${CODE_COVERAGE_CONFIG}x = "x" ]; then
exitOnFailure xcodebuild ARCHS="armv7 arm64" VALID_ARCHS="armv7 armv7s arm64" ONLY_ACTIVE_ARCH=NO ${PROJECT_CLEAN} build -project $WMETEST_PROJECT -target MediaSessionTest -configuration Release -sdk iphoneos
else
exitOnFailure xcodebuild ARCHS="armv7 arm64" VALID_ARCHS="armv7 armv7s arm64" ONLY_ACTIVE_ARCH=NO ${PROJECT_CLEAN} build -xcconfig "${CODE_COVERAGE_CONFIG}" -project $WMETEST_PROJECT -target MediaSessionTest -configuration Release -sdk iphoneos
fi
# We need to sign the RefApp Test app with a developer cert so that it can be installed and run by instruments on a device. By not
# specifying the signing entity here or in Release config, Xcode will use the wildcard 'iOS Developer' which will try to find a developer
# cert/key on your system. We override this with the Distribution cert for the InHouse configuration.
#exitOnFailure /usr/bin/xcrun -sdk iphoneos PackageApplication -v $PWD/build/Release-iphoneos/WebExSquaredTests.app -o $PWD/WebExSquaredTests.ipa
#echo "Finished creating WebExSquaredTests.ipa"

#if [ ${CODE_COVERAGE_CONFIG}x = "x" ]; then
#exitOnFailure xcodebuild ARCHS="armv7 armv7s arm64" VALID_ARCHS="armv7 armv7s arm64" ONLY_ACTIVE_ARCH=NO ${PROJECT_CLEAN} build -project $WMETEST_PROJECT -target MediaSessionTest -configuration Release -sdk iphonesimulator
#else
#exitOnFailure xcodebuild ARCHS="armv7 armv7s arm64" VALID_ARCHS="armv7 armv7s arm64" ONLY_ACTIVE_ARCH=NO ${PROJECT_CLEAN} build -xcconfig "${CODE_COVERAGE_CONFIG}" -project $WMETEST_PROJECT -target MediaSessionTest -configuration Release -sdk iphonesimulator
#fi

echo "Finished running xcodebuild on the MediaSessionTest target"
