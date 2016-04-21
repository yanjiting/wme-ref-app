#!/bin/bash
# yongzxu

ROOT=`pwd`

#mvn package
#mvn package -Ddebug

target=MediaSDK_Demo_Android
topdir=$ROOT/$target
incdir=$topdir/include
libsdir=$topdir/libs

rm -rf $topdir
mkdir -p $topdir $incdir $libsdir

# For include/common
mkdir -p $incdir/common
utildir=$ROOT/../../mediaengine/util
cp -f $utildir/include/{,android,client}/*.h $incdir/common/

tpdir=$ROOT/../../mediaengine/tp
cp -f $tpdir/include/*.h $incdir/common/

# For include/security
mkdir -p $incdir/security
securitydir=$ROOT/../../vendor/security
cp -f $securitydir/include/*.h $incdir/security/

# For include/wme
mkdir -p $incdir/wme
apidir=$ROOT/../../api
cp -f $apidir/*.h $incdir/wme/

# For include/wqos
mkdir -p $incdir/wqos
wqosdir=$ROOT/../../mediaengine/wqos
cp -f $wqosdir/include/*.h $incdir/wqos

# For include/wrtp
mkdir -p $incdir/wrtp
transdir=$ROOT/../../mediaengine/transmission
cp -f $transdir/include/*.h $incdir/wrtp

# For libs/debug and libs/release
distdir=$ROOT/../../distribution/android/armv7
profiles="debug release"
for prof in $profiles; do
    if [ ! -e $distdir/$prof ]; then
        echo "[Error] no building profile for $prof"
        exit 1
    fi
    mkdir -p $libsdir/$prof
    cp -f $distdir/$prof/wseclient.jar $libsdir/
    cp -f $distdir/$prof/wmeclient.jar $libsdir/
    cp -f $distdir/$prof/wmeaudiohwlibrary.jar $libsdir/
    cp -f $distdir/$prof/appshare.jar $libsdir/
    cp -f $distdir/$prof/lib*.so $libsdir/$prof/
    cp -rf $distdir/maps-$prof $libsdir/$prof/maps
done


# For ref-app
#cp -r $ROOT/../../ref-app/Android/WME_Android $topdir/MediaEngineTestApp
#cp -r $ROOT/../../ref-app/common/include $topdir/MediaEngineTestApp/jni/include
#cp -r $ROOT/../../ref-app/common/src $topdir/MediaEngineTestApp/jni/src
#rm -rf $topdir/MediaEngineTestApp/{gen,obj,libs/*,bin}
#mv $topdir/MediaEngineTestApp/jni/Android{_for_ref_app,}.mk
#cp $topdir/MediaEngineTestApp/oldscripts/ant_build_apk2.sh $topdir/MediaEngineTestApp/bld_refapp.sh
#rm -rf $topdir/MediaEngineTestApp/{oldscripts,pom.xml}

rm -rf $target.tar.gz
tar -czf $target.tar.gz $target
rm -rf $target

exit 0
