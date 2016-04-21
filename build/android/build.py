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
    {"title": "Vendor", "tag": "vendor", "path": "vendor/build/android", "status": None},
    {"title": "Util", "tag": "util", "path": "mediaengine/util/src/jni", "status": None},
    {"title": "TP", "tag": "tp", "path": "mediaengine/tp/src/jni", "status": None},
    {"title": "wqos", "tag": "wqos", "path": "mediaengine/wqos/src/jni", "status": None},
    {"title": "ciscosrtp", "tag": "ciscosrtp", "path": "mediaengine/transmission/ciscosrtp/bld/android/jni", "status": None},
    {"title": "transmission", "tag": "transmission", "path": "mediaengine/transmission/src/jni", "status": None},
    {"title": "WMEUtil", "tag":"wmeutil", "path": "mediaengine/wmeutil/bld/client/android/jni/", "status": None},
    {"title": "Cochelea", "tag":"cochelea", "path": "mediaengine/dolphin/src/cochelea/cwcochlea_bld/Android/jni/", "status": None},
    {"title": "Dolphin engine", "tag":"dolphin", "path": "mediaengine/dolphin/bld/client/Android/jni", "status": None},
    {"title": "Shark engine", "tag":"shark", "path": "mediaengine/shark/bld/client/android/", "status": None},
    {"title": "Appshare", "tag":"appshare", "path": "mediaengine/appshare/bld/client/android/", "status": None},
    {"title": "WME client", "tag":"wmeclient", "path": "mediaengine/wme/bld/client/android/jni/", "status": None},
    {"title": "libsdp", "tag":"libsdp", "path": "vendor/libsdp/code/bld/android/jni", "status": None},
    {"title": "Media Session", "tag":"mediasession", "path": "mediasession/bld/android/", "status": None},
    #{"title": "Reference App", "tag":"refapp", "path": "ref-app/Android/WME_Android", "status": None},
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
