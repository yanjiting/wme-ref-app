
echox() { 
    [ $# -le 1 ] && echo "$*" && return 0
    color=$1; shift; 
    echo "$*" && return 0
    #[ `uname` = 'Linux' ] && echo="echo -e" || echo="echo -e"
    #$echo "\033[${color}m$*\033[00m"  
}

check_env() {
    [ "#$ANDROID_HOME" = "#" ] && echox 31 "[WARN] pls set ANDROID_HOME" && exit 1
    ADB="$ANDROID_HOME/platform-tools/adb"

    [ "#$ANDROID_NDK_HOME" = "#" ] && echox 31 "[WARN] pls set ANDROID_NDK_HOME" && exit 1
    export NDK_TOOLCHAIN_VERSION=4.8
    NDK="$ANDROID_NDK_HOME/ndk-build NDK_TOOLCHAIN_VERSION=4.8 -j 3"

    which ant 2>/dev/null 1>&2
    [ $? -ne 0 ] && echox 31 "[WARN] pls install ant" && exit 1
}

usage() {
    echox 31 "usage: $0 debug|release|clean"
}

check_opt() {
    [ $# -lt 1 ] && usage && exit 1
    profile=$1
    if [ "$profile" = "debug" ]; then
        ndk_profile="NDK_DEBUG=1"
        configuration=debug
    elif [ "$profile" = "release" ]; then
        ndk_profile="NDK_DEBUG=0"
        configuration=release
    elif [ "$profile" = "clean" ]; then
        :
    else
        usage && exit 1
    fi
}

check_err() {
    err=$? && mod=None
    [ $# -ge 1 ] && mod=$1
    [ $# -ge 2 ] && err=$2
    [ $err -ne 0 ] && echox 31 "[ERRO] error happens in '$mod'" && exit 1
}

