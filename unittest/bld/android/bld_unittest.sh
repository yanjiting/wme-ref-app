ROOT=`pwd`/../../..
BUILD_PATH=$ROOT/unittest/bld/android
DIST_PATH=$ROOT/distribution/android/armv7

source $ROOT/build/android/common.sh

usage() {
     echox 31 "usage: $0 profile [option]"
     echo "     profile: debug|release|clean"
     echo "     option:  cc - enable codecoverage for debug/release"
     echo "     e.g: '$0 debug' is to build debug"
     echo "     e.g: '$0 debug cc' is to build debug with codecoverage"
}

check_env
check_opt $*

[ $# -ne 1 -a $# -ne 2 ] && usage && exit 1

if [ $profile != "clean" ]; then
    export HAVE_GCOV_FLAG=0
    [ $# -eq 2 ] && [ $2 != "cc" ] && usage && exit 1
    [ $# -eq 2 ] && export HAVE_GCOV_FLAG=1
fi

mod_list="vendor/gtest/bld/android"
mod_list="$mod_list unittest/tp unittest/util unittest/wqos unittest/wrtp"
mod_list="$mod_list unittest/dolphin unittest/shark unittest/wme unittest/appshare/bld/android"
mod_list=($mod_list)


# for unittest modules
if [ $profile = "clean" ]; then
    for ((k=${#mod_list[@]}-1; k>=0; k--))
    do
        cd $ROOT/${mod_list[$k]}/jni
        $NDK NDK_DEBUG=0 clean
        $NDK NDK_DEBUG=1 clean
        rm -rf ../obj
    done
else
    for mod in ${mod_list[@]}
    do
        cd $ROOT/$mod/jni
        $NDK $ndk_profile
        check_err $mod
    done
fi


# for unittest jni
cd $BUILD_PATH/jni
if [ $profile = "clean" ]; then
    $NDK NDK_DEBUG=0 clean
    $NDK NDK_DEBUG=1 clean
    rm -rf ../obj
else
    $NDK $ndk_profile
    [ $? -ne 0 ] && echox 31 "Failed to build unittest jni!" && exit 1
fi

# for unittest apk
cd $BUILD_PATH
ant -f build.xml clean
if [ $profile != "clean" ]; then
    cp -f $DIST_PATH/$configuration/*.jar $BUILD_PATH/libs/
    cp -f $DIST_PATH/$configuration/lib*.so $BUILD_PATH/libs/armeabi-v7a/
    ant -f build.xml $profile

    bin_name=$BUILD_PATH/bin/MainActivity-$profile.apk
    [ ! -f $bin_name ] && echox 31 "Fail to gen target APK." && exit 1

    apk_name=$BUILD_PATH/bin/AllUnitTest-$profile.apk
    cp -f $bin_name $apk_name
    echox 33 "[OK] Target APK in $apk_name"
fi

exit 0
