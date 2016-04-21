#!/usr/bin/env python
import os, sys
from buildtool import *
import time
import datetime
import shutil
import platform
import imp

print ("  --  " + " ".join(sys.argv))
#path is from the root dir
modules = [
    {"title": "Cochelea", "tag":"cochelea", "path": "mediaengine/dolphin/src/cochelea/cwcochlea_bld/Android/jni/", "status": None},
    {"title": "Dolphin engine", "tag":"dolphin", "path": "mediaengine/dolphin/bld/client/Android/jni", "status": None},
]

exec(open("buildscript.py").read())

#libcpath = os.path.join(ndk_home,"sources/cxx-stl/llvm-libc++/libs/armeabi-v7a")
libdistpath = "../../distribution/android/armv7"
libmapspath = os.path.join(libdistpath,"maps")
if not os.path.exists(libmapspath):
    os.makedirs(libmapspath)
#shutil.copy2(os.path.join(libcpath,"libc++_shared.so"), libmapspath)
#if(ndk_debug):
#    shutil.copy2(libcpath+"/libc++_shared.so", libdistpath+"/debug")
#else:
#    shutil.copy2(libcpath+"/libc++_shared.so", libdistpath+"/release")

if(ndk_clean):
    print ("goes here")
    exit(0)

exit(exitcode)
