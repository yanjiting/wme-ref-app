#!/bin/bash

ROOT=`pwd`/../..
cd $ROOT/unittest/bld/android/
sh bld_unittest.sh $*
[ $? -ne 0 ] && exit 1

exit 0
