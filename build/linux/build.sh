#!/bin/bash



CLEAN=""

print_usage()
{
	echo "usage: $0 [clean]"
}

build_build()
{
	echo "begin build"
	(echo "build util"; cd ../../mediaengine/util/src; make ) || exit 1
	(echo "build tp"; cd ../../mediaengine/tp/src; make ) || exit 1
	(echo "build transmission"; cd ../../mediaengine/transmission/src; make ) || exit 1
	(echo "build libsrtp"; cd ../../vendor/libsrtp; chmod +x ./configure && ./configure && make && cp -f libsrtp2.a ../../distribution/linux/libsrtp.a && rm -f ./crypto/include/config.h) || exit 1
	(echo "build wme"; cd ../../mediaengine/wme/src; make ) || exit 1
	(echo "build wmeqos"; cd ../../mediaengine/wqos/src; make ) || exit 1
	(echo "build mediasession"; cd ../../mediasession/src; make ) || exit 1
	(echo "build libsdp"; cd ../../vendor/libsdp; mkdir -p  build && cd build && cmake ../code; make && cp -f ./src/libsdp.a ../../../distribution/linux/) || exit 1
	(echo "build SuperEasyJSON"; cd ../../vendor/SuperEasyJSON; make) || exit 1
	(echo "build mari"; cd ../../vendor/mari; mkdir -p build && cd build && cmake -DCMAKE_BUILD_TYPE=release -DUSE_FUNCTION_OBJECT=ON -DBUILD_TESTS=OFF ../code; make && cp -f ./src/*.a ../../../distribution/linux/) || exit 1
	(echo "build nattools"; cd ../../vendor/nattools-0.2; aclocal && autoreconf && chmod +x ./configure; ./configure && make; cp -f ./icelib/src/.libs/libice.a ../../distribution/linux/ && cp -f  ./sockaddrutil/src/.libs/libsockaddrutil.a ../../distribution/linux/ && cp -f ./stunlib/src/.libs/libstun.a ../../distribution/linux/ && git clean -dfx; git checkout ../../vendor/nattools-0.2 ) || exit 1
	echo "finish build"
}

build_clean()
{
	echo "begin clean"
	(echo "clean util"; cd ../../mediaengine/util/src; make clean) || exit 1
	(echo "clean tp"; cd ../../mediaengine/tp/src; make clean) || exit 1
	(echo "clean transmission"; cd ../../mediaengine/transmission/src; make clean) || exit 1
	(echo "clean ciscortp"; cd ../../mediaengine/transmission/ciscosrtp; make clean && rm -f ../../../distribution/linux/libsrtp.a ) || exit 1
	(echo "clean wme"; cd ../../mediaengine/wme/src; make clean) || exit 1
	(echo "clean wmeqos"; cd ../../mediaengine/wqos/src; make clean) || exit 1
	(echo "clean mediasession"; cd ../../mediasession/src; make clean) || exit 1
	(echo "clean libsdp"; cd ../../vendor/libsdp/build; make clean; cd ..; rm -rf build ../../distribution/linux/libsdp.a) || exit 1
	(echo "clean SuperEasyJSON"; cd ../../vendor/SuperEasyJSON; make clean) || exit 1
	(echo "clean mari"; cd ../../vendor/mari/build; make clean; cd ..; rm -rf build ../../distribution/linux/librsfec.a ../../distribution/linux/libwqos.a) || exit 1
	(echo "clean nattools"; cd ../../vendor/nattools-0.2; make clean; rm -f ../../distribution/linux/libstun.a ../../distribution/linux/libsockaddrutil.a ../../distribution/linux/libice.a) || exit 1
	echo "finish clean"
}

for arg in "$@"
do
	case "$arg" in
		clean)
			build_clean
			exit 0
			break;;
		"help")
			print_usage
			exit 0
			break;;
	esac
done

build_build
#mkdir -p ../../distribution/linux
#cp -f ../../mediaengine/libs/*.a ../../distribution/linux

exit 0

