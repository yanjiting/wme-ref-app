#!/bin/bash
#
#Array Tang
#cd ../../wme/build/mac
#sh build.sh
#cd ../../../ref-app/MacOSX

WMETEST_PROJECT=../MediaSessionTest.xcodeproj
WMETEST_SCREENSHARE_DUMMY_PROJECT=../../../ta_as_dummy_app/macosx/DummyApp.xcodeproj

function exitOnFailure {
    "$@"
    status=$?
    if [ $status -ne 0 ]; then
        echo "Error with command: $1"
        exit $status
    fi
    return $status
}

#echo "Start In-House Build for mac"
#xcodebuild clean -project WebExSquared.xcodeproj -target WebExSquared -configuration InHouse -sdk iphoneos
#exitOnFailure xcodebuild -project WebExSquared.xcodeproj -target WebExSquared -configuration InHouse -sdk iphoneos
#echo "Finished running xcodebuild on the WebExSquared target"

#exitOnFailure /usr/bin/xcrun -sdk iphoneos PackageApplication -v $PWD/build/InHouse-iphoneos/WebExSquared.app -o $PWD/WebExSquaredInHouse.ipa --sign "iPhone Distribution: Cisco Systems, Inc." --embed "$PWD/build/cert/WebExSquared-InHouse.mobileprovision"
#echo "Finished creating WebExSquaredInHouse.ipa"
#echo "End In-House Build"

echo "Start Integration Tests Build for MacOS X -- Debug version"
xcodebuild clean -project $WMETEST_PROJECT -target MediaSessionTest -configuration Debug -sdk macosx
exitOnFailure xcodebuild -project $WMETEST_PROJECT -target MediaSessionTest -configuration Debug -sdk macosx
echo "Screen Sharing Dummy application --Debug version"
xcodebuild clean -project $WMETEST_SCREENSHARE_DUMMY_PROJECT -target DummyApp -configuration Debug -sdk macosx
exitOnFailure xcodebuild -project $WMETEST_SCREENSHARE_DUMMY_PROJECT -target DummyApp -configuration Debug -sdk macosx

echo "Start Integration Tests Build for MacOS X -- Release version"
xcodebuild clean -project $WMETEST_PROJECT -target MediaSessionTest -configuration Release -sdk macosx
exitOnFailure xcodebuild -project $WMETEST_PROJECT -target MediaSessionTest -configuration Release -sdk macosx
echo "Screen Sharing Dummy application --Debug version"
xcodebuild clean -project $WMETEST_SCREENSHARE_DUMMY_PROJECT -target DummyApp -configuration Release -sdk macosx
exitOnFailure xcodebuild -project $WMETEST_SCREENSHARE_DUMMY_PROJECT -target DummyApp -configuration Release -sdk macosx


# We need to sign the IntegrationTest app with a developer cert so that it can be installed and run by instruments on a device. By not
# specifying the signing entity here or in Release config, Xcode will use the wildcard 'iOS Developer' which will try to find a developer
# cert/key on your system. We override this with the Distribution cert for the InHouse configuration.
#exitOnFailure /usr/bin/xcrun -sdk iphoneos PackageApplication -v $PWD/build/Release-iphoneos/WebExSquaredIntegrationTests.app -o $PWD/WebExSquaredIntegrationTests.ipa
#echo "Finished creating WebExSquaredIntegrationTests.ipa"

#xcodebuild clean -project $WMETEST_PROJECT -target MediaSessionTest -configuration Release -sdk iphonesimulator
#exitOnFailure xcodebuild -project $WMETEST_PROJECT -target MediaSessionIntegrationTest -configuration Release -sdk iphonesimulator
#echo "Finished running xcodebuild on the MediaSessionIntegrationTest target"
