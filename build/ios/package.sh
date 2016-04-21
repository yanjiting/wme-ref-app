#!/bin/bash
# yongzxu

ROOT=`pwd`

#sh build.sh

target=MediaSDK_Demo_IOS
topdir=$ROOT/$target
incdir=$topdir/sdk/include
libsdir=$topdir/sdk/libs
traindir=$topdir/wme4train

rm -rf $topdir
mkdir -p $topdir $incdir $libsdir $traindir

mkdir -p $topdir/external/{include,libs}
# For $topdir/external/include
utildir=$ROOT/../../mediaengine/util
cp -f $utildir/include/*.h $topdir/external/include
tpdir=$ROOT/../../mediaengine/tp
cp -f $tpdir/include/*.h $topdir/external/include/
#securitydir=$ROOT/../../vendor/security
#cp -f $securitydir/include/*.h $topdir/external/include/
mkdir -p $topdir/external/include/{ios,client}
cp -f $utildir/include/ios/*.h $topdir/external/include/ios/
cp -f $utildir/include/client/*.h $topdir/external/include/client/

listOfTrainHeaders=("api" 
                    "mediaengine/util/include"
                    "mediaengine/tp/include"
                    "mediaengine/dolphin/include"
                    "mediaengine/shark/src/wsevp/interface"
                    "mediaengine/shark/include"
                    "vendor/openh264/api"
                    "vendor/nattools-0.2/icelib/include"
                    "vendor/nattools-0.2/sockaddrutil/include"
                    "vendor/nattools-0.2/stunlib/include")

for headerPath in "${listOfTrainHeaders[@]}"
do
    mkdir -p $traindir/$headerPath
    cp -rf $ROOT/../../$headerPath/* $traindir/$headerPath
done

# For $topdir/external/libs
distdir=$ROOT/../../distribution/ios
ssllibdir=$ROOT/../../vendor/openssl/ciscossl/libs/ios
profiles="Debug-iphoneos Debug-iphonesimulator Release-iphoneos Release-iphonesimulator ReleaseTest-iphoneos ReleaseTest-iphonesimulator"
for prof in $profiles; do
    mkdir -p $topdir/external/libs/$prof
    cp -f $distdir/$prof/libtp_ios.a $topdir/external/libs/$prof
    cp -f $ssllibdir/$prof/{libcrypto.a,libssl.a} $topdir/external/libs/$prof
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
distdir=$ROOT/../../distribution/ios
cp -rf $distdir/* $libsdir/
for prof in $profiles; do
    rm -f $libsdir/$prof/{libcrypto.a,libssl.a,libtp_ios.a,libwp2p_ios.a}
done

# For $topdir/ref-app
#cp -r $ROOT/../../ref-app/iOS/MediaEngineTestApp $topdir/ref-app
#cp -r $ROOT/../../ref-app/common $topdir/ref-app/MediaEngineTestApp/WMEImplement/common
#cp $ROOT/build_referenceAppinPackage.sh $topdir/ref-app
#sed -i "" 's#path = ../../common#path = MediaEngineTestApp/WMEImplement/common#' $topdir/ref-app/MediaEngineTestApp.xcodeproj/project.pbxproj
#rm -rf $topdir/ref-app/MediaEngineTestAppTA.xcodeproj
#rm -rf $topdir/ref-app/MediaEngineTestApp/TA

#mkdir $topdir/doc
#cp $ROOT/../../ref-app/iOS/{iOSDemoStep.doc,iOSReadme.txt,release_note.txt} $topdir/doc/

rm -rf $target.tar.gz
tar -czf $target.tar.gz $target
rm -rf $target

exit 0
