#!/usr/bin/env python

import sys, os, shutil, commands

# build this
PROJECT_PATH = ".\\..\\..\\mediaengine\\dolphin\\src\\cochelea\\aaec\\"

SCRIPT_PATH = ".\\..\\..\\mediaengine\\dolphin\\src\\cochelea\\aaec\\jni\\Android.so.mk"

# copy from
SRC_PATH 	= "..\\..\\mediaengine\\dolphin\\src\\cochelea\\aaec\\libs\\armeabi-v7a\\libAAEC.so"

# copy to
DST_PATH 	= "..\\..\\distribution\\android\\armv7\\release\\"

def buildFiles(proj_path, script_path):
	cmds = "ndk-build NDK_PROJECT_PATH=" + proj_path + " " + "APP_BUILD_SCRIPT=" + script_path
	os.system(cmds)


def copyFiles(src, dst):
	shutil.copy(src, dst)

if __name__ == '__main__':
	buildFiles(PROJECT_PATH, SCRIPT_PATH)
	copyFiles(SRC_PATH, DST_PATH)
