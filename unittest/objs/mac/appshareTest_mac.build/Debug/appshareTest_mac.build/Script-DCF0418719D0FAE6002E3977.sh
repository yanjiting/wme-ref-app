#!/bin/sh
CONTENTS_PATH="${TARGET_BUILD_DIR}/${CONTENTS_FOLDER_PATH}"
mkdir -p $CONTENTS_PATH/Frameworks

if [ -d $PLATFORM_DIR/Developer/Library/Frameworks/XCTest.framework ]; then
    cp -rf $PLATFORM_DIR/Developer/Library/Frameworks/XCTest.framework $CONTENTS_PATH
fi

if [ -f "${PROJECT_DIR}"/../../src/resource/image-2880-1800.png ]; then
    cp -rf "${PROJECT_DIR}"/../../src/resource/image-2880-1800.png $CONTENTS_PATH
fi

if [ -f "${PROJECT_DIR}"/../../src/resource/text-2880-1800.png ]; then
    cp -rf "${PROJECT_DIR}"/../../src/resource/text-2880-1800.png $CONTENTS_PATH
fi

