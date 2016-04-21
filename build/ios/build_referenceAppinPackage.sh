#!/bin/bash
# Build media sdk reference application of iOS platform


echo "###################################################################"
echo "##Start to build the media SDK reference application"

if [ $# -gt 1 ]; then
echo "*please use command $0 [cc]"
echo "*cc      - option: enable code coverage function"
exit 2
fi

if [ -n "$1" ]; then
if [ "cc" == "$1" ]; then
CODE_COVERAGE_CONFIG="./EnableCoverage.xcconfig"
echo "Unit Test will run with code coverage enable"
else
echo "$1" is an illegal parameter!!!
exit 2
fi
fi

# Path macro
CURRENT_PATH=`pwd`

# Set the work space path
REF_APP_PROJECT_PATH=${CURRENT_PATH}
WME_REF_APP_IOS_STD_OUT_ERR=/dev/null



function buildProjectAllVersions()
{
if [ ${CODE_COVERAGE_CONFIG}x = "x" ]; then
xcodebuild ARCHS="armv7 armv7s" VALID_ARCHS="armv7 armv7s" ONLY_ACTIVE_ARCH=NO -project $1.xcodeproj -target $2 -configuration Debug -sdk iphoneos clean build #> ${WME_REF_APP_IOS_STD_OUT_ERR} 2>&1
else
xcodebuild -xcconfig "${CODE_COVERAGE_CONFIG}" ARCHS="armv7 armv7s" VALID_ARCHS="armv7 armv7s" ONLY_ACTIVE_ARCH=NO -project $1.xcodeproj -target $2 -configuration Debug -sdk iphoneos clean build #> ${WME_REF_APP_IOS_STD_OUT_ERR} 2>&1
fi

if [ $? == 0 ]; then
echo "build $1 Debug iphoneos successfully"
else
echo "build $1 Debug iphoneos fail"
return 1
fi

if [ ${CODE_COVERAGE_CONFIG}x = "x" ]; then
xcodebuild ARCHS="armv7 armv7s" VALID_ARCHS="armv7 armv7s" ONLY_ACTIVE_ARCH=NO -project $1.xcodeproj -target $2 -configuration Release -sdk iphoneos clean build #> ${WME_REF_APP_IOS_STD_OUT_ERR} 2>&1
else
xcodebuild -xcconfig "${CODE_COVERAGE_CONFIG}" ARCHS="armv7 armv7s" VALID_ARCHS="armv7 armv7s" ONLY_ACTIVE_ARCH=NO -project $1.xcodeproj -target $2 -configuration Release -sdk iphoneos clean build #> ${WME_REF_APP_IOS_STD_OUT_ERR} 2>&1
fi

if [ $? == 0 ]; then
echo "build $1 Release iphoneos successfully"
else
echo "build $1 Release iphoneos fail"
return 1
fi

if [ ${CODE_COVERAGE_CONFIG}x = "x" ]; then
xcodebuild ARCHS="i386" VALID_ARCHS="i386" ONLY_ACTIVE_ARCH=NO -project $1.xcodeproj -target $2 -configuration Debug -sdk iphonesimulator clean build #> ${WME_REF_APP_IOS_STD_OUT_ERR} 2>&1
else
xcodebuild -xcconfig "${CODE_COVERAGE_CONFIG}" ARCHS="i386" VALID_ARCHS="i386" ONLY_ACTIVE_ARCH=NO -project $1.xcodeproj -target $2 -configuration Debug -sdk iphonesimulator clean build #> ${WME_REF_APP_IOS_STD_OUT_ERR} 2>&1
fi

if [ $? == 0 ]; then
echo "build $1 Debug iphonesimulator successfully"
else
echo "build $1 Debug iphonesimulator fail"
return 1
fi

if [ ${CODE_COVERAGE_CONFIG}x = "x" ]; then
xcodebuild ARCHS="i386" VALID_ARCHS="i386" ONLY_ACTIVE_ARCH=NO -project $1.xcodeproj -target $2 -configuration Release -sdk iphonesimulator clean build #> ${WME_REF_APP_IOS_STD_OUT_ERR} 2>&1
else
xcodebuild -xcconfig "${CODE_COVERAGE_CONFIG}" ARCHS="i386" VALID_ARCHS="i386" ONLY_ACTIVE_ARCH=NO -project $1.xcodeproj -target $2 -configuration Release -sdk iphonesimulator clean build #> ${WME_REF_APP_IOS_STD_OUT_ERR} 2>&1
fi

if [ $? == 0 ]; then
echo "build $1 Release iphonesimulator successfully"
else
echo "build $1 Release iphonesimulator fail"
return 1
fi

return 0
}

# Build media SDK reference application
PROJECT_FILE_NAME="MediaEngineTestApp"
TARGET_NAME="MediaEngineTestApp"

buildProjectAllVersions ${REF_APP_PROJECT_PATH}/${PROJECT_FILE_NAME} ${TARGET_NAME}
if [ $? != 0 ]; then
echo "Build ${PROJECT_FILE_NAME} failed, exit now"
exit 1
fi

cd ${CURRENT_PATH}

exit 0
