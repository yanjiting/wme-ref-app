#@rev dleejo

checkerr() {
    err=$?
    mod=None
    [ $# -ge 1 ] && mod=$1
    [ $err -ne 0 ] && echo "[$mod] build failed" && exit 1
}

###################
### Debug/Release
if [ "$1"x = "debug"x ]; then
ndk_debug="NDK_DEBUG=1"
configuration_name="debug"
echo "Build debug version"
else
ndk_debug="NDK_DEBUG=0"
configuration_name="release"
echo "Build release version"
fi

###################
### Variables
HOME_DIR=`pwd`
MVN_DIR="$M2"
LIB_DIR=$HOME_DIR/../../../distribution/android/armv7/${configuration_name}
VERSION=2-7-0

#1) Checking for Maven environment
#if [ ! -n "$M2" ]; then
#echo -e "[WARN] Please set M2 with the path of Maven binary directory";
#exit 1;
#fi

#2) Checking Android SDK
if [ ! -n "$ANDROID_HOME" ]; then
echo -e "[WARN] Please set ANDROID_HOME with the path of Android SDK";
exit 1;
fi

#3) Checking Android NDK
if [ ! -n "$ANDROID_NDK_HOME" ]; then
echo -e "[WARN] Please set ANDROID_NDK_HOME with the path of Android SDK";
exit 1;
fi


if [ "$1"x = "clean"x ]; then
(
###################
### Clean Option
echo "*************************************"
echo "START CLEANING"
echo "*************************************"
    cd $HOME_DIR/jni
    $ANDROID_NDK_HOME/ndk-build ${ndk_debug} clean

    find $HOME_DIR/libs/armeabi-v7a/ -iname "*.so" -delete
    find $HOME_DIR/libs/ -iname "wseclient.jar" -delete

    cd $HOME_DIR
    ant -f build.xml clean
    find $HOME_DIR/../../../distribution/android/armv7/debug -iname "*.apk" -delete
    find $HOME_DIR/../../../distribution/android/armv7/release -iname "*.apk" -delete

    rm -rf bin/ gen/ obj/
)
exit 0

else

###################
### Packaging Option
echo "*************************************"
echo "START PACKAGING"
echo "*************************************"

cd $HOME_DIR/jni
$ANDROID_NDK_HOME/ndk-build ${ndk_debug}
checkerr wmenative-jni

cp -f $LIB_DIR/*.so $HOME_DIR/libs/armeabi-v7a
cp -f $LIB_DIR/*.jar $HOME_DIR/libs
cp -f $HOME_DIR/../../../vendor/openh264/libs/android/*.so $HOME_DIR/libs/armeabi-v7a

cd $HOME_DIR
ant -f build.xml clean
ant -f build.xml ${configuration_name}

bin_name=bin/SettingActivity-${configuration_name}.apk
    if [ -e $bin_name ]; then
        apkname=EngineAndroid-${configuration_name}-$VERSION.apk
        cp $bin_name $HOME_DIR/../../../distribution/android/armv7/${configuration_name}/$apkname
        echo "[OK] Generating APK called under /cwme/trunk/distribution/android/armv7/${configuration_name} directory"
    fi

if [ ! -f "$LIB_DIR/EngineAndroid-${configuration_name}-$VERSION.apk" ]; then
        echo "Failed to generate APK! Please check ant logs above"
        exit 1
    fi

fi
exit 0




