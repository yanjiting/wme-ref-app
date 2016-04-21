ROOT=`pwd`
BUILD_PATH=$ROOT

[ $# -ne 1 ] && echo "usage: $0 debug|release|clean" && exit 1

if [ "$1" = "clean" ]; then
    rm -rf bin/ obj/ gen/
    exit 0
fi

EXT=$1
if [ "$1" = "debug" ]; then
ndk_debug="NDK_DEBUG=1"
configuration_name="debug"
echo "build debug version"
else
ndk_debug="NDK_DEBUG=0"
configuration_name="release"
echo "build release version"
fi

echox() { color=$1; shift; echo -e "\033[${color}m$*\033[00m"; }

[ "#$ANDROID_HOME" = "#" ] && echox 31 "[WARN] Please set ANDROID_HOME" && exit 1
[ "#$ANDROID_NDK_HOME" = "#" ] && echox 31 "[WARN] Please set ANDROID_NDK_HOME" && exit 1
NDK="$ANDROID_NDK_HOME/ndk-build"

# step1: building refapp 
echox 34 "STEP-1: Building depended native libs..\n"
cd $BUILD_PATH/jni
export NDK_TOOLCHAIN_VERSION=4.8
$NDK ${ndk_debug} NDK_TOOLCHAIN_VERSION=4.8
if [ $? -ne 0 ]; then
  exit 1
fi

cd $BUILD_PATH
cp ../libs/${configuration_name}/*.so libs/armeabi-v7a/
cp -f ../libs/*.jar libs/


# step2: generating WME APK
echox 34 "STEP-2: Generating APK for wme ref-app...\n"
cd $BUILD_PATH
ant -Dtarget=android-19 -f build.xml ${configuration_name}

bin_name=bin/SettingActivity-${configuration_name}.apk
[ ! -f $bin_name ] && echox 31 "fail to gen refapp apk" && exit 1

cp -f $bin_name WMERefApp-$EXT.apk
echox 33 "[OK] Generating APK in `pwd` WMERefApp-$EXT.apk"

exit 0
