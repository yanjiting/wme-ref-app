#!/bin/bash
###############################################################################
#
#
TEST_NAME="Media SDK Unit Test"
###############################################################################
START_TIME=$(date +%s)

echo "###################################################################"
echo "##Checking parameters"

function showUsage()
{
echo "*please use command $0 [sim/dev] [release/debug] [cc] [PROJECT_NAME]"
echo "*[sim/dev]       - unit test on simulator or on device, default is simulator"
echo "*[release/debug] - unit test with debug or release version, default is release "
echo "*[cc]            - option: enable code coverage function, default is disable"
echo "*[PROJECT_NAME]  - option: dolphin/tp/wqos/util/appshare/mediasession/wme/shark/wrtp/. default is build all"
echo "*for example: ./build_ut.sh sim release cc dolphinUnitTestApp"
}

if [ $# -gt 4 ]; then
showUsage
exit 2
fi

#set the default configuration
WME_UNITTEST_IOS_ARCH="i386"
WME_UNITTEST_IOS_PLATFORM="iphonesimulator"
WME_UNITTEST_IOS_DEBUG_RELEASE="Release"
WME_UNITTEST_IOS_TARGET_PROJECT="BuildAll"
WME_UNITTEST_IOS_REPORT_SUBFOLDER="release"

for PARAM in $*; do
  if [ "sim" = "${PARAM}" ]; then
    WME_UNITTEST_IOS_ARCH="i386"
    WME_UNITTEST_IOS_PLATFORM="iphonesimulator"
  elif [ "dev" = "${PARAM}" ]; then
    WME_UNITTEST_IOS_ARCH="armv7 arm64"
    WME_UNITTEST_IOS_PLATFORM="iphoneos"
  elif [ "release" = "${PARAM}" ]; then
    WME_UNITTEST_IOS_DEBUG_RELEASE="Release"
    WME_UNITTEST_IOS_REPORT_SUBFOLDER="release"
  elif [ "debug" = "${PARAM}" ]; then
    WME_UNITTEST_IOS_DEBUG_RELEASE="Debug"
    WME_UNITTEST_IOS_REPORT_SUBFOLDER="debug"
  elif [ "cc" = "${PARAM}" ]; then
    CODE_COVERAGE_CONFIG="./EnableCoverage.xcconfig"
    echo "Unit Test will run with code coverage enable"
  elif [ "clean" = "${PARAM}" ]; then
    PROJECT_CLEAN="clean"
  else
    if [ "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "BuildAll" ]; then
      WME_UNITTEST_IOS_TARGET_PROJECT=${PARAM}
      echo Unit Test will run project:${WME_UNITTEST_IOS_TARGET_PROJECT}
    else
      echo parameters are illegal!!!, please have a check.
      showUsage
      exit 2
    fi
  fi
done

#command on real device is still in developing, so disable it now
#if [ ${WME_UNITTEST_IOS_PLATFORM} == "iphoneos"  ]; then
#    echo "Command on real device is still in developing, so disable it now"
#    exit 2
#fi

echo "Unit Test will run on ${WME_UNITTEST_IOS_PLATFORM} with ${WME_UNITTEST_IOS_DEBUG_RELEASE}"


###############################################################################
echo "###################################################################"
echo "##Building gtest, gmock"

CURRENT_PATH=`pwd`
WME_UNITTEST_IOS_GTEST_PATH=${CURRENT_PATH}/../../vendor/gtest/bld/ios
WME_UNITTEST_IOS_GMOCK_PATH=${CURRENT_PATH}/../../vendor/gmock/bld/ios
WME_UNITTEST_IOS_STD_OUT_ERR=/dev/null
WME_UNITTEST_IOS_REPORT_PATH="./report/${WME_UNITTEST_IOS_REPORT_SUBFOLDER}"
mkdir -p ${WME_UNITTEST_IOS_REPORT_PATH}
WME_UNITTEST_SUM_INFO="\n##################The conclusion of the ios UT##################\n"
WME_UNITTEST_SUM_INFO+="\nUT run on ${WME_UNITTEST_IOS_PLATFORM} with ${WME_UNITTEST_IOS_DEBUG_RELEASE}\n"
WME_UNITTEST_SUM_FAILED_PROJECT=0

function buildProject()
{
if [ ${CODE_COVERAGE_CONFIG}x = "x" ]; then
xcodebuild ARCHS="${WME_UNITTEST_IOS_ARCH}" VALID_ARCHS="${WME_UNITTEST_IOS_ARCH}" ONLY_ACTIVE_ARCH=NO -project $1.xcodeproj -target $2 -configuration $3 -sdk $4 ${PROJECT_CLEAN} build | xcpretty -c
else
xcodebuild -xcconfig "${CODE_COVERAGE_CONFIG}" ARCHS="${WME_UNITTEST_IOS_ARCH}" VALID_ARCHS="${WME_UNITTEST_IOS_ARCH}" ONLY_ACTIVE_ARCH=NO -project $1.xcodeproj -target $2 -configuration $3 -sdk $4 ${PROJECT_CLEAN} build | xcpretty -c
fi
status=${PIPESTATUS[0]}
if [ $status == 0 ]; then
echo "build $1 $3 $4 successfully"
return 0;
else
echo "build $1 $3 $4 fail"
return 1;
fi
}

###############################################################################

###############################################################################
#cd ${WME_UNITTEST_IOS_GTEST_PATH}

PROJECT_FILE_NAME="gtest"
TARGET_NAME=${PROJECT_FILE_NAME}

buildProject ${WME_UNITTEST_IOS_GTEST_PATH}/${PROJECT_FILE_NAME} ${TARGET_NAME} ${WME_UNITTEST_IOS_DEBUG_RELEASE} ${WME_UNITTEST_IOS_PLATFORM}
if [ $? != 0 ]; then
echo "Build ${PROJECT_FILE_NAME} failed, exit now"
exit 1
fi
###############################################################################
#xcodebuild -project gtest.xcodeproj -target gtest -configuration Debug -sdk iphoneos clean build
#xcodebuild -project gtest.xcodeproj -target gtest -configuration Release -sdk iphoneos clean build
#xcodebuild -project gtest.xcodeproj -target gtest -configuration Debug -sdk iphonesimulator clean build
#xcodebuild -project gtest.xcodeproj -target gtest -configuration Release -sdk iphonesimulator clean build
###############################################################################

###############################################################################
#cd ${WME_UNITTEST_IOS_GMOCK_PATH}

PROJECT_FILE_NAME="gmock"
TARGET_NAME=${PROJECT_FILE_NAME}

buildProject ${WME_UNITTEST_IOS_GTEST_PATH}/${PROJECT_FILE_NAME} ${TARGET_NAME} ${WME_UNITTEST_IOS_DEBUG_RELEASE} ${WME_UNITTEST_IOS_PLATFORM}
if [ $? != 0 ]; then
echo "Build ${PROJECT_FILE_NAME} failed, exit now"
exit 1
fi
###############################################################################
#cd ${WME_UNITTEST_IOS_GMOCK_PATH}
#xcodebuild -project gmock.xcodeproj -target gmock -configuration Debug -sdk iphoneos clean build
#xcodebuild -project gmock.xcodeproj -target gmock -configuration Release -sdk iphoneos clean build
#xcodebuild -project gmock.xcodeproj -target gmock -configuration Debug -sdk iphonesimulator clean build
#xcodebuild -project gmock.xcodeproj -target gmock -configuration Release -sdk iphonesimulator clean build
###############################################################################


WME_UNITTEST_IOS_TOOL_LAUNCH_ON_SIMULATOR="ios-sim"

echo "Checking tool"
if [ ${WME_UNITTEST_IOS_PLATFORM} == iphonesimulator ]; then

if ! which ${WME_UNITTEST_IOS_TOOL_LAUNCH_ON_SIMULATOR} ; then
echo "${WME_UNITTEST_IOS_TOOL_LAUNCH_ON_SIMULATOR} is not found, please install it"
exit 1
else
echo "Find ${WME_UNITTEST_IOS_TOOL_LAUNCH_ON_SIMULATOR} tool"
fi

fi


function copylogsfromdev()
{
   mkdir -p $4/wbxlogs/$3
   str=`./iFileTransfer -o listFiles -id $1  -app $2 -path /tmp/logs/ 2>&1`
   str=`echo ${str#*(}`
   str=`echo ${str%)*}`
   str=`echo ${str//\"/}`
   arr=$(echo $str|tr "," "\n")
   for x in $arr; do
     echo "./iFileTransfer -o download -id $1 -app $2 -from /tmp//logs/$x -to $4/wbxlogs/$3"
     ./iFileTransfer -o download -id $1 -app $2 -from /tmp/logs/$x -to "$4"/wbxlogs/$3
   done
}

function copylogsfromsim()
{
   mkdir -p $2
   LOG_PATH=`find ~/Library/Application\ Support/iPhone\ Simulator/ -name $1\.app -print`
   LOG_PATH=`dirname "${LOG_PATH}"`
   LOG_PATH="${LOG_PATH}/Library/Application Support/Logs"
   mkdir -p "$2/wbxlogs/$1"
   cp "${LOG_PATH}"/* "$2/wbxlogs/$1/"
   cp "$3" "$2"
   cp "$4" "$2"
}


function buildRunUnitTestProject()
{
    WME_UNITTEST_IOS_PROJECT_PATH=$1
    WME_UNITTEST_IOS_PROJECT_NAME=$2
    WME_UNITTEST_IOS_APP_NAME=$3
    WME_UNITTEST_IOS_XML_DEV=$4
    WME_UNITTEST_IOS_XML_SIM=$5
    WME_UNITTEST_IOS_PROJECT_FILE=$1/$2
    TARGET_NAME=${WME_UNITTEST_IOS_PROJECT_NAME}
    WME_UNITTEST_IOS_APP_PATH=${WME_UNITTEST_IOS_PROJECT_PATH}/build

    WME_UNITTEST_IOS_ERR_FILE="/tmp/gtest_${WME_UNITTEST_IOS_PROJECT_NAME}.err"
    WME_UNITTEST_IOS_LOG_FILE="/tmp/gtest_${WME_UNITTEST_IOS_PROJECT_NAME}.log"
    WME_UNITTEST_IOS_APP_FOR_SIMULATOR=${WME_UNITTEST_IOS_APP_PATH}/${WME_UNITTEST_IOS_DEBUG_RELEASE}-iphonesimulator/${WME_UNITTEST_IOS_PROJECT_NAME}.app
    WME_UNITTEST_IOS_APP_FOR_DEVICE=${WME_UNITTEST_IOS_APP_PATH}/${WME_UNITTEST_IOS_DEBUG_RELEASE}-iphoneos/${WME_UNITTEST_IOS_PROJECT_NAME}.app
    WME_UNITTEST_IOS_APP_FOR_DEVICE_DSYM=${WME_UNITTEST_IOS_APP_PATH}/${WME_UNITTEST_IOS_DEBUG_RELEASE}-iphoneos/${WME_UNITTEST_IOS_PROJECT_NAME}.app.dSYM

    if [ -f ${WME_UNITTEST_IOS_PROJECT_FILE} ]; then
        echo "File ${WME_UNITTEST_IOS_PROJECT_FILE} is not found in $1"
        WME_UNITTEST_SUM_INFO+="\nFile ${WME_UNITTEST_IOS_PROJECT_FILE} is not found in $1!"
        return 1
    fi

    buildProject ${WME_UNITTEST_IOS_PROJECT_FILE} ${TARGET_NAME} ${WME_UNITTEST_IOS_DEBUG_RELEASE} ${WME_UNITTEST_IOS_PLATFORM}
    if [ $? != 0 ]; then
        echo "Build ${WME_UNITTEST_IOS_PROJECT_NAME} failed, exit now"
        WME_UNITTEST_SUM_INFO+="\nBuild ${WME_UNITTEST_IOS_PROJECT_NAME} failed"
        return 1
    fi


    if [ ${WME_UNITTEST_IOS_PLATFORM} == iphonesimulator ]; then
        if [ ! -d ${WME_UNITTEST_IOS_APP_FOR_SIMULATOR} ] ; then
            echo "${WME_UNITTEST_IOS_APP_FOR_SIMULATOR} is not found"
            WME_UNITTEST_SUM_INFO+="\n${WME_UNITTEST_IOS_APP_FOR_SIMULATOR} is not found"
            return 1
        else
            echo "Find App ${WME_UNITTEST_IOS_APP_FOR_SIMULATOR}"
        fi

        echo "Cleaning old test app"
        pkill -9 'iPhone Simulator'
        rm -rf ${WME_UNITTEST_IOS_ERR_FILE}
        rm -rf ${WME_UNITTEST_IOS_LOG_FILE}

        echo "Begin to launching ${WME_UNITTEST_IOS_PROJECT_NAME}"

        ${WME_UNITTEST_IOS_TOOL_LAUNCH_ON_SIMULATOR} launch ${WME_UNITTEST_IOS_APP_FOR_SIMULATOR} --stderr ${WME_UNITTEST_IOS_ERR_FILE} --stdout ${WME_UNITTEST_IOS_LOG_FILE} #> ${WME_UNITTEST_IOS_STD_OUT_ERR} 2>&1


        #copy to report folder
        copylogsfromsim ${WME_UNITTEST_IOS_PROJECT_NAME} ${WME_UNITTEST_IOS_REPORT_PATH} ${WME_UNITTEST_IOS_LOG_FILE} ${WME_UNITTEST_IOS_ERR_FILE}
        cp /tmp/${WME_UNITTEST_IOS_XML_SIM} ${WME_UNITTEST_IOS_REPORT_PATH}/${WME_UNITTEST_IOS_XML_SIM}
        if [ $? -ne 0 ]; then
        echo "copy file: /tmp/${WME_UNITTEST_IOS_XML_SIM} is failed!"
        WME_UNITTEST_SUM_INFO+="\ncopy file: /tmp/${WME_UNITTEST_IOS_XML_SIM} is failed!"
        return 1
        else
        #show the folder content
        ls ${WME_UNITTEST_IOS_REPORT_PATH}
        fi

        #remove the useless files
        rm /tmp/${WME_UNITTEST_IOS_XML_SIM}

        #		#echo "wme_gtest.log and wme_gtest.xml are in /tmp/ folder"
    elif [ ${WME_UNITTEST_IOS_PLATFORM} == iphoneos ]; then
        # for real device
        if [ ! -d ${WME_UNITTEST_IOS_APP_FOR_DEVICE} ] ; then
            echo "${WME_UNITTEST_IOS_APP_FOR_DEVICE} is not found"
            WME_UNITTEST_SUM_INFO+="\n${WME_UNITTEST_IOS_APP_FOR_DEVICE} is not found"
            return 1
        else
            echo "Find app ${WME_UNITTEST_IOS_APP_FOR_DEVICE}"
        fi
        #
        #		echo "Begin to launching $TEST_NAME"
        #ensure instruments not runing
        echo "Try to kill the runing instruments"
        pids_str=`ps x -o pid,command | grep -v grep | grep "Instruments" | awk '{printf "%s,", $1}'`
        instruments_pids="${pids_str//,/ }"
        for pid in ${instruments_pids}; do
            echo "Found instruments ${pid}. Killing..."
            kill -9 ${pid} && wait ${pid} &> /dev/null
        done

        GREP_RESULT=`system_profiler SPUSBDataType | sed -n -e '/iPad/,/Serial/p' -e '/iPhone/,/Serial/p' | grep "Serial Number:" | awk -F ": " '{print $2}'`
        GREP_RESULT=`../../ta/ref-app/iOS/script/ios-deploy -c -t 2 | grep "Found" | sed 's/.*(\([0-9a-z]\{40\}\)).*/\1/'`

        for DEVICE_ID in ${GREP_RESULT}
        do
            echo "Try to run on device:${DEVICE_ID}"

            INSTALL_APP=`./iFileTransfer -o list -id ${DEVICE_ID} 2>&1 | grep ${WME_UNITTEST_IOS_APP_NAME}`
            if [ "${INSTALL_APP}x" != "x" ]; then
                echo "uninstall the application: ${WME_UNITTEST_IOS_APP_NAME}"
                #uninstall the application from device to remove the last result
                ./fruitstrap uninstall --bundle ${WME_UNITTEST_IOS_APP_NAME} --id ${DEVICE_ID} #> ${WME_UNITTEST_IOS_STD_OUT_ERR} 2>&1
                if [ $? != 0 ]; then
                    echo "uninstall application: ${WME_UNITTEST_IOS_APP_NAME} from device: ${DEVICE_ID} is failed!"
                    WME_UNITTEST_SUM_INFO+="\nuninstall application: ${WME_UNITTEST_IOS_APP_NAME} from device: ${DEVICE_ID} is failed!"
                    return 1
                fi
            fi
            #install the application
            ./fruitstrap -v install --bundle ${WME_UNITTEST_IOS_APP_FOR_DEVICE} --id ${DEVICE_ID} #> ${WME_UNITTEST_IOS_STD_OUT_ERR} 2>&1
            if [ $? != 0 ]; then
                echo "install application: ${WME_UNITTEST_IOS_APP_FOR_DEVICE} to device: ${DEVICE_ID} is failed!"
                WME_UNITTEST_SUM_INFO+="\ninstall application: ${WME_UNITTEST_IOS_APP_FOR_DEVICE} to device: ${DEVICE_ID} is failed!"
                return 1
            fi

            if [ ${CODE_COVERAGE_CONFIG}x != "x" ]; then

                #upload the gcda files to the cur app
                if [ -d ./gcdaFile/cc_iOS ]; then
                    ./iFileTransfer -o copy -id ${DEVICE_ID} -app ${WME_UNITTEST_IOS_APP_NAME} -from ./gcdaFile/cc_iOS -to /Documents
                    errno=$?
                    if [ ${errno} != 0 ]; then
                        echo "upload file: ./gcdaFile/cc_iOS to ${WME_UNITTEST_IOS_APP_NAME} is failed!"
                        WME_UNITTEST_SUM_INFO+="\nupload file: ./gcdaFile/cc_iOS to ${WME_UNITTEST_IOS_APP_NAME} is failed!"
                        return 1
                    fi
                fi

            fi

            #get the crash report list before running the IOS UT
            ./iFileTransfer -o listcrashreport -id ${DEVICE_ID} 2> /tmp/crashlist_before.txt
            ./iFileTransfer -o delete -id ${DEVICE_ID} -app ${WME_UNITTEST_IOS_APP_NAME} -target /Documents/${WME_UNITTEST_IOS_XML_DEV}
            rm -f ${WME_UNITTEST_IOS_REPORT_PATH}/${WME_UNITTEST_IOS_XML_DEV}

            #instruments -v -w ${DEVICE_ID}  -t /Applications/Xcode.app/Contents/Applications/Instruments.app/Contents/PlugIns/AutomationInstrument.bundle/Contents/Resources/Automation.tracetemplate ${WME_UNITTEST_IOS_APP_FOR_DEVICE} -e UIASCRIPT ./uiascript.js  -e UIARRESULTPATH /tmp/ #> ${WME_UNITTEST_IOS_STD_OUT_ERR} 2>&1
            instruments -v -w ${DEVICE_ID}  -t "/Applications/Xcode.app/Contents/Applications/Instruments.app/Contents/Resources/templates/Time Profiler.tracetemplate" ${WME_UNITTEST_IOS_APP_FOR_DEVICE}

            #copy to report folder
            copylogsfromdev ${DEVICE_ID} ${WME_UNITTEST_IOS_APP_NAME} ${WME_UNITTEST_IOS_PROJECT_NAME} ${WME_UNITTEST_IOS_REPORT_PATH}

            LOOP_NUM=0
            while true; do
                LOOP_NUM=$((LOOP_NUM+1))
                ./iFileTransfer -o download -id ${DEVICE_ID} -app ${WME_UNITTEST_IOS_APP_NAME} -from /Documents/${WME_UNITTEST_IOS_XML_DEV} -to ${WME_UNITTEST_IOS_REPORT_PATH}
                errno=$?
                if [ ${errno} -eq 0 ]; then
                    break
                fi

                if [ ${LOOP_NUM} -gt 2 ]; then
                    #copy the related crash report to report folder
                    ./iFileTransfer -o listcrashreport -id ${DEVICE_ID} 2> /tmp/crashlist_after.txt
                    crashReportFile=`diff /tmp/crashlist_before.txt /tmp/crashlist_after.txt | grep ".ips" | awk -F "\"" '{print $2;}'`

                    if [ "${crashReportFile}x" != "x" ]; then
                        mkdir -p ${WME_UNITTEST_IOS_REPORT_PATH}/Crashes
                        ./iFileTransfer -o getcrashlog -id ${DEVICE_ID} -from ${crashReportFile} -to /tmp
                        sh analyzeiOSCrashLogs.sh /tmp/${crashReportFile} ${WME_UNITTEST_IOS_APP_FOR_DEVICE_DSYM} ${WME_UNITTEST_IOS_REPORT_PATH}/Crashes/${crashReportFile}
                        echo "A crash is happened, Please check the crash report: ${WME_UNITTEST_IOS_REPORT_PATH}/Crashes/${crashReportFile}!"
                        WME_UNITTEST_SUM_INFO+="\nA crash is happened, Please check the crash report: ${WME_UNITTEST_IOS_REPORT_PATH}/Crashes/${crashReportFile}!"
                    else
                        echo "download file: ${WME_UNITTEST_IOS_XML_DEV} from ${WME_UNITTEST_IOS_APP_NAME} is failed!"
                        WME_UNITTEST_SUM_INFO+="\ndownload file: ${WME_UNITTEST_IOS_XML_DEV} from ${WME_UNITTEST_IOS_APP_NAME} is failed!"
                    fi
                    return 1
                fi
                sleep 8
            done

            if [ ${CODE_COVERAGE_CONFIG}x != "x" ]; then
                rm -fr ./gcdaFile
                mkdir -p ./gcdaFile
                ./iFileTransfer -o download -id ${DEVICE_ID} -app ${WME_UNITTEST_IOS_APP_NAME} -from /Documents/cc_iOS -to ./gcdaFile
                if [ $? != 0 ]; then
                    echo "download file: /Documents/cc_iOS from ${WME_UNITTEST_IOS_APP_NAME} is failed!"
                    WME_UNITTEST_SUM_INFO+="\ndownload file: /Documents/cc_iOS from ${WME_UNITTEST_IOS_APP_NAME} is failed!"
                    return 1
                fi

            fi
            ls ${WME_UNITTEST_IOS_REPORT_PATH}
            #only run on one device
            return 0

        done

        echo "Can not find any connected device! please check device is connected to MAC!"
        WME_UNITTEST_SUM_INFO+="\nCan not find any connected device! please check device is connected to MAC!"
        return 1

    fi

}

#PROJECT_FILE_NAME="WMEUnitTestApp"
#TARGET_NAME=${PROJECT_FILE_NAME}
UNIT_TEST_MAIN_PATH=${CURRENT_PATH}/../../unittest

if [ "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "dolphin" -o  "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "BuildAll" ]  ; then
WME_UNITTEST_IOS_PROJECT_PATH=${UNIT_TEST_MAIN_PATH}/dolphin/bld/ios
WME_UNITTEST_IOS_PROJECT_NAME="dolphinUnitTestApp"
WME_UNITTEST_IOS_APP_NAME="cisco.dolphinUnitTestApp"
WME_UNITTEST_IOS_XML_DEV="dolphin_UT_ios_dev.xml"
WME_UNITTEST_IOS_XML_SIM="dolphin_UT_ios_sim.xml"
buildRunUnitTestProject ${WME_UNITTEST_IOS_PROJECT_PATH} ${WME_UNITTEST_IOS_PROJECT_NAME} ${WME_UNITTEST_IOS_APP_NAME} ${WME_UNITTEST_IOS_XML_DEV} ${WME_UNITTEST_IOS_XML_SIM}

if [ $? != 0 ]; then
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is failed!"
WME_UNITTEST_SUM_INFO+="\nFailed!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
WME_UNITTEST_SUM_FAILED_PROJECT=$((WME_UNITTEST_SUM_FAILED_PROJECT+1))
else
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is successful!"
WME_UNITTEST_SUM_INFO+="\nSuccessfully!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
fi

fi
if [ "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "shark" -o  "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "BuildAll" ]; then
WME_UNITTEST_IOS_PROJECT_PATH=${UNIT_TEST_MAIN_PATH}/shark/bld/ios
WME_UNITTEST_IOS_PROJECT_NAME="sharkUnitTestApp"
WME_UNITTEST_IOS_APP_NAME="cisco.sharkUnitTestApp"
WME_UNITTEST_IOS_XML_DEV="shark_UT_ios_dev.xml"
WME_UNITTEST_IOS_XML_SIM="shark_UT_ios_sim.xml"
buildRunUnitTestProject ${WME_UNITTEST_IOS_PROJECT_PATH} ${WME_UNITTEST_IOS_PROJECT_NAME} ${WME_UNITTEST_IOS_APP_NAME} ${WME_UNITTEST_IOS_XML_DEV} ${WME_UNITTEST_IOS_XML_SIM}

if [ $? != 0 ]; then
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is failed!"
WME_UNITTEST_SUM_INFO+="\nFailed!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
WME_UNITTEST_SUM_FAILED_PROJECT=$((WME_UNITTEST_SUM_FAILED_PROJECT+1))
else
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is successful!"
WME_UNITTEST_SUM_INFO+="\nSuccessfully!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
fi

fi

if [ "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "tp" -o  "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "BuildAll" ]; then
WME_UNITTEST_IOS_PROJECT_PATH=${UNIT_TEST_MAIN_PATH}/tp/bld/ios
WME_UNITTEST_IOS_PROJECT_NAME="tpUnitTestApp"
WME_UNITTEST_IOS_APP_NAME="cisco.tpUnitTestApp"
WME_UNITTEST_IOS_XML_DEV="tp_UT_ios_dev.xml"
WME_UNITTEST_IOS_XML_SIM="tp_UT_ios_sim.xml"
buildRunUnitTestProject ${WME_UNITTEST_IOS_PROJECT_PATH} ${WME_UNITTEST_IOS_PROJECT_NAME} ${WME_UNITTEST_IOS_APP_NAME} ${WME_UNITTEST_IOS_XML_DEV} ${WME_UNITTEST_IOS_XML_SIM}

if [ $? != 0 ]; then
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is failed!"
WME_UNITTEST_SUM_INFO+="\nFailed!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
WME_UNITTEST_SUM_FAILED_PROJECT=$((WME_UNITTEST_SUM_FAILED_PROJECT+1))
else
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is successful!"
WME_UNITTEST_SUM_INFO+="\nSuccessfully!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
fi

fi

if [ "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "util" -o  "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "BuildAll" ]; then
WME_UNITTEST_IOS_PROJECT_PATH=${UNIT_TEST_MAIN_PATH}/util/bld/ios
WME_UNITTEST_IOS_PROJECT_NAME="utilUnitTestApp"
WME_UNITTEST_IOS_APP_NAME="cisco.utilUnitTestApp"
WME_UNITTEST_IOS_XML_DEV="util_UT_ios_dev.xml"
WME_UNITTEST_IOS_XML_SIM="util_UT_ios_sim.xml"
buildRunUnitTestProject ${WME_UNITTEST_IOS_PROJECT_PATH} ${WME_UNITTEST_IOS_PROJECT_NAME} ${WME_UNITTEST_IOS_APP_NAME} ${WME_UNITTEST_IOS_XML_DEV} ${WME_UNITTEST_IOS_XML_SIM}

if [ $? != 0 ]; then
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is failed!"
WME_UNITTEST_SUM_INFO+="\nFailed!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
WME_UNITTEST_SUM_FAILED_PROJECT=$((WME_UNITTEST_SUM_FAILED_PROJECT+1))
else
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is successful!"
WME_UNITTEST_SUM_INFO+="\nSuccessfully!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
fi

fi

if [ "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "wme" -o  "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "BuildAll" ]; then
WME_UNITTEST_IOS_PROJECT_PATH=${UNIT_TEST_MAIN_PATH}/wme/bld/ios
WME_UNITTEST_IOS_PROJECT_NAME="wmeUnitTestApp"
WME_UNITTEST_IOS_APP_NAME="cisco.wmeUnitTestApp"
WME_UNITTEST_IOS_XML_DEV="wme_UT_ios_dev.xml"
WME_UNITTEST_IOS_XML_SIM="wme_UT_ios_sim.xml"
buildRunUnitTestProject ${WME_UNITTEST_IOS_PROJECT_PATH} ${WME_UNITTEST_IOS_PROJECT_NAME} ${WME_UNITTEST_IOS_APP_NAME} ${WME_UNITTEST_IOS_XML_DEV} ${WME_UNITTEST_IOS_XML_SIM}

if [ $? != 0 ]; then
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is failed!"
WME_UNITTEST_SUM_INFO+="\nFailed!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
WME_UNITTEST_SUM_FAILED_PROJECT=$((WME_UNITTEST_SUM_FAILED_PROJECT+1))
else
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is successful!"
WME_UNITTEST_SUM_INFO+="\nSuccessfully!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
fi

fi
if [ "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "wqos" -o  "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "BuildAll" ]; then
WME_UNITTEST_IOS_PROJECT_PATH=${UNIT_TEST_MAIN_PATH}/wqos/bld/ios
WME_UNITTEST_IOS_PROJECT_NAME="wqosUnitTestApp"
WME_UNITTEST_IOS_APP_NAME="cisco.wqosUnitTestApp"
WME_UNITTEST_IOS_XML_DEV="wqos_UT_ios_dev.xml"
WME_UNITTEST_IOS_XML_SIM="wqos_UT_ios_sim.xml"
buildRunUnitTestProject ${WME_UNITTEST_IOS_PROJECT_PATH} ${WME_UNITTEST_IOS_PROJECT_NAME} ${WME_UNITTEST_IOS_APP_NAME} ${WME_UNITTEST_IOS_XML_DEV} ${WME_UNITTEST_IOS_XML_SIM}

if [ $? != 0 ]; then
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is failed!"
WME_UNITTEST_SUM_INFO+="\nFailed!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
WME_UNITTEST_SUM_FAILED_PROJECT=$((WME_UNITTEST_SUM_FAILED_PROJECT+1))
else
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is successful!"
WME_UNITTEST_SUM_INFO+="\nSuccessfully!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
fi

fi

if [ "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "wrtp" -o  "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "BuildAll" ]; then
WME_UNITTEST_IOS_PROJECT_PATH=${UNIT_TEST_MAIN_PATH}/wrtp/bld/ios
WME_UNITTEST_IOS_PROJECT_NAME="wrtpUnitTestApp"
WME_UNITTEST_IOS_APP_NAME="cisco.wrtpUnitTestApp"
WME_UNITTEST_IOS_XML_DEV="wrtp_UT_ios_dev.xml"
WME_UNITTEST_IOS_XML_SIM="wrtp_UT_ios_sim.xml"
buildRunUnitTestProject ${WME_UNITTEST_IOS_PROJECT_PATH} ${WME_UNITTEST_IOS_PROJECT_NAME} ${WME_UNITTEST_IOS_APP_NAME} ${WME_UNITTEST_IOS_XML_DEV} ${WME_UNITTEST_IOS_XML_SIM}

if [ $? != 0 ]; then
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is failed!"
WME_UNITTEST_SUM_INFO+="\nFailed!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
WME_UNITTEST_SUM_FAILED_PROJECT=$((WME_UNITTEST_SUM_FAILED_PROJECT+1))
else
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is successful!"
WME_UNITTEST_SUM_INFO+="\nSuccessfully!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
fi

fi

if [ "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "appshare" -o  "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "BuildAll" ]; then
WME_UNITTEST_IOS_PROJECT_PATH=${UNIT_TEST_MAIN_PATH}/appshare/bld/ios/appshareUnitTest
WME_UNITTEST_IOS_PROJECT_NAME="appshareUnitTest"
WME_UNITTEST_IOS_APP_NAME="cisco.appshareUnitTest"
WME_UNITTEST_IOS_XML_DEV="appshare_UT_ios_dev.xml"
WME_UNITTEST_IOS_XML_SIM="appshare_UT_ios_sim.xml"
buildRunUnitTestProject ${WME_UNITTEST_IOS_PROJECT_PATH} ${WME_UNITTEST_IOS_PROJECT_NAME} ${WME_UNITTEST_IOS_APP_NAME} ${WME_UNITTEST_IOS_XML_DEV} ${WME_UNITTEST_IOS_XML_SIM}

if [ $? != 0 ]; then
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is failed!"
WME_UNITTEST_SUM_INFO+="\nFailed!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
WME_UNITTEST_SUM_FAILED_PROJECT=$((WME_UNITTEST_SUM_FAILED_PROJECT+1))
else
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is successful!"
WME_UNITTEST_SUM_INFO+="\nSuccessfully!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
fi

fi
if [ "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "mediasession" -o  "${WME_UNITTEST_IOS_TARGET_PROJECT}" = "BuildAll" ]; then
WME_UNITTEST_IOS_PROJECT_PATH=${UNIT_TEST_MAIN_PATH}/mediasession
WME_UNITTEST_IOS_PROJECT_NAME="mediasessionUT_ios"
WME_UNITTEST_IOS_APP_NAME="cisco.mediasessionUT-ios"
WME_UNITTEST_IOS_XML_DEV="mediasession_UT_ios_dev.xml"
WME_UNITTEST_IOS_XML_SIM="mediasession_UT_ios_sim.xml"
buildRunUnitTestProject ${WME_UNITTEST_IOS_PROJECT_PATH} ${WME_UNITTEST_IOS_PROJECT_NAME} ${WME_UNITTEST_IOS_APP_NAME} ${WME_UNITTEST_IOS_XML_DEV} ${WME_UNITTEST_IOS_XML_SIM}

if [ $? != 0 ]; then
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is failed!"
WME_UNITTEST_SUM_INFO+="\nFailed!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
WME_UNITTEST_SUM_FAILED_PROJECT=$((WME_UNITTEST_SUM_FAILED_PROJECT+1))
else
echo "Run UT project: ${WME_UNITTEST_IOS_PROJECT_NAME} is successful!"
WME_UNITTEST_SUM_INFO+="\nSuccessfully!!! - UT project:${WME_UNITTEST_IOS_PROJECT_NAME}\n"
fi
fi

if [ ${WME_UNITTEST_IOS_ERR_FILE}x = "x" ]; then
echo the project name:"${WME_UNITTEST_IOS_TARGET_PROJECT}" is invalid!
exit 1
else
WME_UNITTEST_SUM_INFO+="\nTotal failed project number: ${WME_UNITTEST_SUM_FAILED_PROJECT}\n"
WME_UNITTEST_SUM_INFO+="\n################################################################"
echo -e ${WME_UNITTEST_SUM_INFO}
echo The output XML file is placed at: ${WME_UNITTEST_IOS_REPORT_PATH}, please check it!
fi

if [ ${WME_UNITTEST_SUM_FAILED_PROJECT} -ne 0 ]; then
exit 1
else
END_TIME=$(date +%s)
((BUILD_TIME=${END_TIME}-${START_TIME}))
echo "UT cases runing time is: ${BUILD_TIME} seconds"
exit 0
fi
