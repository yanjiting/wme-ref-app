#!/bin/bash
#
#Soya Li

function showUsage {
    echo "*please use command $0 [cc] [clean] [debug/release] [sim/dev]"
    echo "*cc              - option: enable code coverage function"
    echo "*clean           - option: force to clean it before building the project"
    echo "*debug/release   - option: only build debug or release, default build all"
    echo "*sim/dev         - option: only build simulator or device, default build all"
}

if [ $# -gt 4 ]; then
    showUsage
    exit 2
fi


for PARAM in $*; do
    if [ "clean" = "${PARAM}" ]; then
        PROJECT_CLEAN="clean"
    elif [ "cc" = "${PARAM}" ]; then
        CODE_COVERAGE_CONFIG="./EnableCoverage.xcconfig"
        echo "Unit Test will run with code coverage enable"
    elif [ "debug" == "${PARAM}" ]; then
        DEBUGONLY=1
    elif [ "release" == "${PARAM}" ]; then
        RELEASEONLY=1
    elif [ "releasetest" == "${PARAM}" ]; then
        RELEASETESTONLY=1    
    elif [ "sim" == "${PARAM}" ]; then
        SIMULATORONLY=1
    elif [ "dev" == "${PARAM}" ]; then
        DEVICEONLY=1
    else
        echo parameters are illegal!!!, please have a check.
        showUsage
    exit 2
    fi
done

# prefer the xctool runner
#if hash xctool 2>/dev/null; then
#runner=xctool
runner=xcodebuild
#else
#runner=xcodebuild
#fi
TARGET_ARCHS="armv7 arm64"

if [ "${DEBUGONLY}x" == "x" ] && [ "${RELEASEONLY}x" == "x" ] && [ "${RELEASETESTONLY}x" == "x" ]; then
    echo "It will build all by default"
    ALLTARGET=1
fi

if [ "${SIMULATORONLY}x" == "x" ] && [ "${DEVICEONLY}x" == "x" ]; then
   echo "we build sim."
   echo "we build dev."
fi

if [ "${SIMULATORONLY}x" != "x" ]; then
    echo "we build sim."
fi

if [ "${DEVICEONLY}x" != "x" ]; then
    echo "we build dev."
fi

if [ "${DEBUGONLY}x" != "x" ] || [ "${ALLTARGET}x" != "x" ]; then
    echo "we are debug."
fi

if [ "${RELEASEONLY}x" != "x" ] || [ "${ALLTARGET}x" != "x" ]; then
    echo "we are release."
fi

WME_PROJECT=../../mediasession/bld/ios/MediaSession.xcodeproj
WME_TARGET=MediaSession

DISTRIBUTION_PATH=../../distribution/ios
CODEC_PROJECT_PATH=../../vendor/openh264/libs/ios
OPENSSL_PROJECT_PATH=../../vendor/openssl/ciscossl/libs/ios
SDP_PROJECT_PATH=../../vendor/libsdp/build/ios
NATTOOL_PROJECT_PATH=../../vendor/nattools-0.2/target/ios
MARI_PROJECT_PATH=../../vendor/mari/build/libs/ios

function exitOnFailure {
    "$@" | xcpretty -c
    status=${PIPESTATUS[0]}
    if [ $status -ne 0 ]; then
    echo "Error with command: $1"
    exit $status
    fi
    return $status
}

if [ ${CODE_COVERAGE_CONFIG}x = "x" ]; then
    if [ "${SIMULATORONLY}x" = "x" ]; then
        if [ "${DEBUGONLY}x" != "x" ] || [ "${ALLTARGET}x" != "x" ]; then
            echo "Start Debug Build for iOS"
            exitOnFailure $runner ARCHS="${TARGET_ARCHS}" VALID_ARCHS="armv7 armv7s arm64" ONLY_ACTIVE_ARCH=NO ${PROJECT_CLEAN} build -project $WME_PROJECT -target $WME_TARGET -configuration Debug -sdk iphoneos ENABLE_NS_ASSERTIONS=YES
            echo "Finished running xcodebuild on the iOS debug target"
        fi

        if [ "${RELEASEONLY}x" != "x" ] || [ "${ALLTARGET}x" != "x" ]; then
            echo "Start Release Build for iOS"
            exitOnFailure $runner ARCHS="${TARGET_ARCHS}" VALID_ARCHS="armv7 armv7s arm64" ONLY_ACTIVE_ARCH=NO ${PROJECT_CLEAN} build -project $WME_PROJECT -target $WME_TARGET -configuration Release -sdk iphoneos RUN_CLANG_STATIC_ANALYZER=NO ENABLE_NS_ASSERTIONS=NO
            echo "Finished running xcodebuild on the iOS release target"
        fi
    fi
    if [ "${DEVICEONLY}x" = "x" ]; then
        if [ "${DEBUGONLY}x" != "x" ] || [ "${ALLTARGET}x" != "x" ]; then
            echo "Start Debug Build for simulator"
            exitOnFailure $runner ARCHS="i386 x86_64" VALID_ARCHS="i386 x86_64" ONLY_ACTIVE_ARCH=NO ${PROJECT_CLEAN} build -project $WME_PROJECT -target $WME_TARGET -configuration Debug -sdk iphonesimulator ENABLE_NS_ASSERTIONS=YES
            echo "Finished running xcodebuild on the simulator debug target"
        fi
        if [ "${RELEASEONLY}x" != "x" ] || [ "${ALLTARGET}x" != "x" ]; then
            echo "Start Release Build for simulator"
            exitOnFailure $runner ARCHS="i386 x86_64" VALID_ARCHS="i386 x86_64" ONLY_ACTIVE_ARCH=NO ${PROJECT_CLEAN} build -project $WME_PROJECT -target $WME_TARGET -configuration Release -sdk iphonesimulator RUN_CLANG_STATIC_ANALYZER=NO ENABLE_NS_ASSERTIONS=NO
            echo "Finished running xcodebuild on the simulator release target"
        fi
    fi
else
    if [ "${SIMULATORONLY}x" = "x" ]; then
    if [ "${DEBUGONLY}x" != "x" ] || [ "${ALLTARGET}x" != "x" ]; then
        echo "Start Debug Build for iOS"
        exitOnFailure $runner ARCHS="${TARGET_ARCHS}" VALID_ARCHS="armv7 armv7s arm64" ONLY_ACTIVE_ARCH=NO ${PROJECT_CLEAN} build -xcconfig "${CODE_COVERAGE_CONFIG}" -project $WME_PROJECT -target $WME_TARGET -configuration Debug -sdk iphoneos
        echo "Finished running xcodebuild on the iOS debug target"
    fi

    if [ "${RELEASEONLY}x" != "x" ] || [ "${ALLTARGET}x" != "x" ]; then
        echo "Start Release Build for iOS"
        exitOnFailure $runner ARCHS="${TARGET_ARCHS}" VALID_ARCHS="armv7 armv7s arm64" ONLY_ACTIVE_ARCH=NO ${PROJECT_CLEAN} build -xcconfig "${CODE_COVERAGE_CONFIG}" -project $WME_PROJECT -target $WME_TARGET -configuration Release -sdk iphoneos RUN_CLANG_STATIC_ANALYZER=NO
        echo "Finished running xcodebuild on the iOS release target"
    fi
    fi

    if [ "${DEVICEONLY}x" = "x" ]; then
    if [ "${DEBUGONLY}x" != "x" ] || [ "${ALLTARGET}x" != "x" ]; then
        echo "Start Debug Build for simulator"
        exitOnFailure $runner ARCHS="i386" VALID_ARCHS="i386 x86_64" ONLY_ACTIVE_ARCH=NO ${PROJECT_CLEAN} build -xcconfig "${CODE_COVERAGE_CONFIG}" -project $WME_PROJECT -target $WME_TARGET -configuration Debug -sdk iphonesimulator
        echo "Finished running xcodebuild on the simulator debug target"
    fi

    if [ "${RELEASEONLY}x" != "x" ] || [ "${ALLTARGET}x" != "x" ]; then
        echo "Start Release Build for simulator"
        exitOnFailure $runner ARCHS="i386" VALID_ARCHS="i386 x86_64" ONLY_ACTIVE_ARCH=NO ${PROJECT_CLEAN} build -xcconfig "${CODE_COVERAGE_CONFIG}" -project $WME_PROJECT -target $WME_TARGET -configuration Release -sdk iphonesimulator RUN_CLANG_STATIC_ANALYZER=NO
        echo "Finished running xcodebuild on the simulator release target"
    fi
    fi
fi

if [ "${DEBUGONLY}x" != "x" ] || [ "${ALLTARGET}x" != "x" ]; then
    echo "Copy debug library"
    cp -f ${CODEC_PROJECT_PATH}/Debug-iphoneos/*.a ${DISTRIBUTION_PATH}/Debug-iphoneos/
    cp -f ${OPENSSL_PROJECT_PATH}/Debug-iphoneos/*.a ${DISTRIBUTION_PATH}/Debug-iphoneos/
    cp -f ${SDP_PROJECT_PATH}/Debug-iphoneos/*.a ${DISTRIBUTION_PATH}/Debug-iphoneos/
    cp -f ${NATTOOL_PROJECT_PATH}/Debug-iphoneos/*.a ${DISTRIBUTION_PATH}/Debug-iphoneos/
    echo "Finished copy debug"
fi

if [ "${RELEASEONLY}x" != "x" ] || [ "${ALLTARGET}x" != "x" ]; then
    echo "Copy release library"
    cp -f ${OPENSSL_PROJECT_PATH}/Release-iphoneos/*.a ${DISTRIBUTION_PATH}/Release-iphoneos/
    cp -f ${CODEC_PROJECT_PATH}/Release-iphoneos/*.a ${DISTRIBUTION_PATH}/Release-iphoneos/
    cp -f ${SDP_PROJECT_PATH}/Release-iphoneos/*.a ${DISTRIBUTION_PATH}/Release-iphoneos/
    cp -f ${NATTOOL_PROJECT_PATH}/Release-iphoneos/*.a ${DISTRIBUTION_PATH}/Release-iphoneos/
    echo "Finished copy release"
fi

if [ "${DEBUGONLY}x" == "x" ] && [ "${RELEASEONLY}x" == "x" ]; then
    echo "Copy simulator library"
    cp -f ${CODEC_PROJECT_PATH}/Debug-iphonesimulator/*.a ${DISTRIBUTION_PATH}/Debug-iphonesimulator/
    cp -f ${CODEC_PROJECT_PATH}/Release-iphonesimulator/*.a ${DISTRIBUTION_PATH}/Release-iphonesimulator/
    cp -f ${OPENSSL_PROJECT_PATH}/Debug-iphonesimulator/*.a ${DISTRIBUTION_PATH}/Debug-iphonesimulator/
    cp -f ${OPENSSL_PROJECT_PATH}/Release-iphonesimulator/*.a ${DISTRIBUTION_PATH}/Release-iphonesimulator/
    cp -f ${SDP_PROJECT_PATH}/Debug-iphonesimulator/*.a ${DISTRIBUTION_PATH}/Debug-iphonesimulator/
    cp -f ${SDP_PROJECT_PATH}/Release-iphonesimulator/*.a ${DISTRIBUTION_PATH}/Release-iphonesimulator/
    cp -f ${NATTOOL_PROJECT_PATH}/Debug-iphonesimulator/*.a ${DISTRIBUTION_PATH}/Debug-iphonesimulator/
    cp -f ${NATTOOL_PROJECT_PATH}/Release-iphonesimulator/*.a ${DISTRIBUTION_PATH}/Release-iphonesimulator/
    echo "Finished copy simulator libraries"
fi

#For generating the ReleaseTest target
if [ "${ALLTARGET}x" != "x" ] || [ "${RELEASETESTONLY}x" != "x" ]; then

    echo "Start ReleaseTest Build for iOS"
    exitOnFailure $runner ARCHS="${TARGET_ARCHS}" VALID_ARCHS="armv7 armv7s arm64" ONLY_ACTIVE_ARCH=NO ${PROJECT_CLEAN} build -project $WME_PROJECT -target $WME_TARGET -configuration Release -sdk iphoneos RUN_CLANG_STATIC_ANALYZER=NO CONFIGURATION=ReleaseTest -xcconfig "./ReleaseTest.xcconfig"
    echo "Finished running xcodebuild on the iOS ReleaseTest target"

    echo "Copy ReleaseTest library"
    mkdir -p ${OPENSSL_PROJECT_PATH}/ReleaseTest-iphoneos
    cp -f ${OPENSSL_PROJECT_PATH}/Release-iphoneos/*.a ${OPENSSL_PROJECT_PATH}/ReleaseTest-iphoneos/
    cp -f ${CODEC_PROJECT_PATH}/Release-iphoneos/*.a ${DISTRIBUTION_PATH}/ReleaseTest-iphoneos/
    cp -f ${SDP_PROJECT_PATH}/Release-iphoneos/*.a ${DISTRIBUTION_PATH}/ReleaseTest-iphoneos/
    cp -f ${NATTOOL_PROJECT_PATH}/Release-iphoneos/*.a ${DISTRIBUTION_PATH}/ReleaseTest-iphoneos/
    cp -f ${DISTRIBUTION_PATH}/Release-iphoneos/{libmariqos.a,librsfec.a} ${DISTRIBUTION_PATH}/ReleaseTest-iphoneos/

    if [ "${DEVICEONLY}x" = "x" ]; then
        echo "Start ReleaseTest Build for simulator"
        exitOnFailure $runner ARCHS="i386 x86_64" VALID_ARCHS="i386 x86_64" ONLY_ACTIVE_ARCH=NO ${PROJECT_CLEAN} build -project $WME_PROJECT -target $WME_TARGET -configuration Release -sdk iphonesimulator RUN_CLANG_STATIC_ANALYZER=NO CONFIGURATION=ReleaseTest -xcconfig "./ReleaseTest.xcconfig"
        echo "Finished running xcodebuild on the simulator ReleaseTest target"
        
        mkdir -p ${OPENSSL_PROJECT_PATH}/ReleaseTest-iphonesimulator
        cp -f ${OPENSSL_PROJECT_PATH}/Release-iphonesimulator/*.a ${OPENSSL_PROJECT_PATH}/ReleaseTest-iphonesimulator/
        cp -f ${CODEC_PROJECT_PATH}/Release-iphonesimulator/*.a ${DISTRIBUTION_PATH}/ReleaseTest-iphonesimulator/
        cp -f ${SDP_PROJECT_PATH}/Release-iphonesimulator/*.a ${DISTRIBUTION_PATH}/ReleaseTest-iphonesimulator/
        cp -f ${NATTOOL_PROJECT_PATH}/Release-iphonesimulator/*.a ${DISTRIBUTION_PATH}/ReleaseTest-iphonesimulator/
        cp -f ${DISTRIBUTION_PATH}/Release-iphonesimulator/{libmariqos.a,librsfec.a} ${DISTRIBUTION_PATH}/ReleaseTest-iphonesimulator/
        echo "Finished copy ReleaseTest libraries"
    fi
fi
