#!/bin/sh
(cd ../../../../build/ios/; sh build.sh dev release ) || exit 1
sh build_refapp.sh
