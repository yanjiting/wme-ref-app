#!/usr/bin/env python
import os, sys
from buildtool import *
import time
import datetime
from multiprocessing import Pool

#path is from the root dir
def build_module(module):
    vglobals = {
        "__file__": __file__,
        "modules": [module], 
        "ndk_home": ndk_home,
        "ndk_clean": ndk_clean,
        "ndk_debug": ndk_debug,
        "ndk_args": ndk_args,
        "configuration": configuration,
        "version": version,
        "enabled_cc": enabled_cc,
        "dst_modules": dst_modules,
        "buildModule": buildModule,
        "cleanLibs": cleanLibs,
        "runCmd": runCmd,
    }
    execfile("buildscript.py", vglobals)


module = {"title": "gtest/gmock", "path": "vendor/gtest/bld/android/jni/", "status": None}
build_module(module)

modules = [
    {"title": "unittest - wrtp", "path": "unittest/wrtp/jni", "status": None},
    {"title": "unittest - tp", "path": "unittest/tp/jni", "status": None},
    {"title": "unittest - util", "path": "unittest/util/jni", "status": None},
    {"title": "unittest - wqos", "path": "unittest/wqos/jni", "status": None},
    {"title": "unittest - dolphin", "path": "unittest/dolphin/jni", "status": None},
    {"title": "unittest - shark", "path": "unittest/shark/jni", "status": None},
    {"title": "unittest - wme", "path": "unittest/wme/jni", "status": None},
    {"title": "unittest - appshare", "path": "unittest/appshare/bld/android/jni", "status": None}
]
print "=============== pool_num %d ================= " % pool_num
pool = Pool(pool_num)
results = pool.map(build_module, modules)
pool.close()
pool.join()
print results

module = {"title": "unittest app", "path": "unittest/bld/android", "status": None}
build_module(module)

