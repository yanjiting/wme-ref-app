#!/bin/bash



CLEAN=""

print_usage()
{
	echo "usage: $0 [clean]"
}

build_build()
{
	echo "begin build"
	(echo "build gtest/gmock"; cd ../../vendor/gtest/bld/linux && sh build.sh ) || exit 1
	(echo "build util"; cd ../../unittest/util/ && make ) || exit 1
	(echo "build tp"; cd ../../unittest/tp/ && make ) || exit 1
	echo "finish build"
}

build_clean()
{
	echo "begin clean"
	(echo "build gtest/gmock"; cd ../../vendor/gtest/bld/linux &&  sh build.sh clean) || exit 1
	(echo "build util"; cd ../../unittest/util/ &&  make clean) || exit 1
	(echo "build tp"; cd ../../unittest/tp/ && make clean) || exit 1
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
exit 0

