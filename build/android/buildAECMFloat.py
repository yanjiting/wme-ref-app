#!/usr/bin/env python

import sys, os, shutil, commands

# build this
PROJECT_PATH = ".\\..\\..\\mediaengine\\dolphin\\src\\cochelea\\cwcochlea_bld\\Android\\"

APP_PATH     = ".\\..\\..\\mediaengine\\dolphin\\src\\cochelea\\cwcochlea_bld\\Android\\jni\\Application.so.mk"

SCRIPT_PATH  = ".\\..\\..\\mediaengine\\dolphin\\src\\cochelea\\cwcochlea_bld\\Android\\jni\\Android.aec.float.mk"


# copy from
SRC_PATH 	= "..\\..\\mediaengine\\dolphin\\src\\cochelea\\cwcochlea_bld\\Android\\libs\\armeabi-v7a\\libAECMFloat.so"

# copy to
DST_PATH 	= "..\\..\\distribution\\android\\armv7\\release\\"

def buildFiles(proj_path, app_path, script_path):
    cmds = ["ndk-build",
            "NDK_PROJECT_PATH=" + proj_path,
            "NDK_APPLICATION_MK=" + app_path,
            "APP_BUILD_SCRIPT=" + script_path]
    os.system(" ".join(cmds))


def copyFiles(src, dst):
    shutil.copy(src, dst)

if __name__ == '__main__':
    buildFiles(PROJECT_PATH, APP_PATH, SCRIPT_PATH)
    copyFiles(SRC_PATH, DST_PATH)
