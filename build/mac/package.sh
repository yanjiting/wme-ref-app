#!/bin/bash
# yongzxu

ROOT=`pwd`

#sh build.sh

target=MediaSDK_Demo_Mac
topdir=$ROOT/$target
incdir=$topdir/sdk/include
libsdir=$topdir/sdk/libs

rm -rf $topdir
mkdir -p $topdir $incdir $libsdir

mkdir -p $topdir/external/{include,libs}
# For $topdir/external/include
utildir=$ROOT/../../mediaengine/util
cp -f $utildir/include/*.h $topdir/external/include
tpdir=$ROOT/../../mediaengine/tp
cp -f $tpdir/include/*.h $topdir/external/include/
#securitydir=$ROOT/../../vendor/security
#cp -f $securitydir/include/*.h $topdir/external/include/
mkdir -p $topdir/external/include/{macos,client}
cp -f $utildir/include/macos/*.h $topdir/external/include/macos/
cp -f $utildir/include/client/*.h $topdir/external/include/client/

# For $topdir/external/libs
distdir=$ROOT/../../distribution/mac
profiles="Debug Release"
for prof in $profiles; do
    mkdir -p $topdir/external/libs/$prof
    cp -rf $distdir/$prof/tp.framework* $topdir/external/libs/$prof
done

# For $incdir
apidir=$ROOT/../../api
cp -f $apidir/*.h $incdir

# For $incdir/wqos
mkdir -p $incdir/wqos
wqosdir=$ROOT/../../mediaengine/wqos
cp -f $wqosdir/include/*.h $incdir/wqos/

# For $incdir/wrtp
mkdir -p $incdir/wrtp
transdir=$ROOT/../../mediaengine/transmission
cp -f $transdir/include/*.h $incdir/wrtp/

# For $libsdir
distdir=$ROOT/../../distribution/mac
cp -rf $distdir/* $libsdir/
for prof in $profiles; do
    rm -rf $libsdir/$prof/tp.framework*
    rm -rf $libsdir/$prof/appsharing.bundle*
done

# For $topdir/ref-app
#mkdir -p $topdir/ref-app
#svn export $ROOT/../../ref-app/mac/MediaEngineTestApp $topdir/ref-app
#svn export $ROOT/../../ref-app/common $topdir/ref-app/MediaEngineTestApp/common
cp -r $ROOT/../../ref-app/mac/MediaEngineTestApp $topdir/ref-app
cp -r $ROOT/../../mediaengine/bld/mac/Config $topdir/ref-app/

rm -rf $topdir/ref-app/bin
rm -rf $topdir/ref-app/objs
rm -rf $topdir/ref-app/MediaEngineTestApp.xcodeproj/project.xcworkspace
rm -rf $topdir/ref-app/MediaEngineTestApp.xcodeproj/xcuserdata
cp -r $ROOT/../../ref-app/common $topdir/ref-app/MediaEngineTestApp/
#cp -rf $libsdir/ $topdir/ref-app/bin/
#cp -rf $topdir/external/libs/ $topdir/ref-app/bin/
sed -i "" 's#path = ../../../common#path = common#' $topdir/ref-app/MediaEngineTestApp.xcodeproj/project.pbxproj
sed -i "" 's#path = ../../../mediaengine/bld/mac/Config#path = Config#' $topdir/ref-app/MediaEngineTestApp.xcodeproj/project.pbxproj
#rm -rf $topdir/ref-app/MediaEngineTestAppTA.xcodeproj
#rm -rf $topdir/ref-app/MediaEngineTestApp/TA
cp $ROOT/build_refapp_for_package.sh $topdir/ref-app

mkdir $topdir/doc
cp $ROOT/../../ref-app/mac/release_note.txt $topdir/doc/

find $target -name .svn -exec rm -rf {} \; 2>/dev/null
rm -rf $target.tar.gz
tar -czf $target.tar.gz $target
rm -rf $target

exit 0
