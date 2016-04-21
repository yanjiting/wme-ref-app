#!/bin/bash
# rimarsh

ROOT=`pwd`/../..

#sh build.sh

target=distribute
topdir=$ROOT/$target
incdir=$topdir/sdk/include
libsdir=$topdir/sdk/libs

rm -rf $topdir
mkdir -p $topdir $incdir $libsdir
mkdir -p $topdir/external/{include,libs}

# For $topdir/external/include
utildir=$ROOT/mediaengine/util
ln $utildir/include/*.h $topdir/external/include/
tpdir=$ROOT/mediaengine/tp
ln $tpdir/include/*.h $topdir/external/include/
#securitydir=$ROOT/vendor/security
#ls $securitydir/include/*.h $topdir/external/include/

ln -s $utildir/include/ios $topdir/external/include/ios
ln -s $utildir/include/client $topdir/external/include/client

# For $topdir/external/libs
distdir=$ROOT/distribution/ios
ssllibdir=$ROOT/vendor/openssl/ciscossl/libs/ios
profiles="Debug-iphoneos Debug-iphonesimulator Release-iphoneos Release-iphonesimulator"
for prof in $profiles; do
    mkdir -p $topdir/external/libs/$prof
    ln -s $distdir/$prof/{libtp_ios.a,libwp2p_ios.a} $topdir/external/libs/$prof/
    ln -s $ssllibdir/$prof/{libcrypto.a,libssl.a} $topdir/external/libs/$prof/
done

# For $incdir
apidir=$ROOT/api
ln -s $apidir/*.h $incdir/

# For $incdir/wqos
mkdir -p $incdir/wqos
wqosdir=$ROOT/mediaengine/wqos
ln -s $wqosdir/include $incdir/wqos

# For $incdir/wrtp
mkdir -p $incdir/wrtp
transdir=$ROOT/mediaengine/transmission
ln -s $transdir/include $incdir/wrtp

# For $libsdir
distdir=$ROOT/distribution/ios
for prof in $profiles; do
  mkdir -p $libsdir/$prof/
  ln -s $distdir/$prof/include $libsdir/$prof/include
  ln $distdir/$prof/*.a $libsdir/$prof/
  rm -f $libsdir/$prof/{libcrypto.a,libssl.a,libtp_ios.a,libwp2p_ios.a}
done

exit 0
