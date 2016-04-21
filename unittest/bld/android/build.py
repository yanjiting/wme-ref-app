#!/usr/bin/env python

import sys, os, shutil
import glob
import subprocess

if __name__ == '__main__':
    sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../../../../build/android')))
    from buildtool import * 
    current_dir = os.path.abspath(os.path.dirname(__file__))
    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../../.."))

buildModule(os.path.join(current_dir, 'jni'))
if(not ndk_clean):
    os.chdir(current_dir)
    targetname = "AllUnitTest-%s.apk" % configuration
    distdir = os.path.abspath(os.path.join(current_dir, '../../../distribution/android/armv7', configuration))

    libsdir = os.path.join(current_dir, "libs")
    for file in glob.glob(distdir + "/*.jar"):
        print file
        shutil.copy(file, libsdir)

    sodir = os.path.join(current_dir, "libs", "armeabi-v7a")
    for file in glob.glob(distdir + "/*.so"):
        print file
        shutil.copy(file, sodir)
    
    cmds = ["ant", "-f", "build.xml", configuration]
    #runCmd(cmds)
    os.system(" ".join(cmds))
    apkpath = os.path.join(current_dir, "bin", "MainActivity-%s.apk" % configuration);
    targetpath = os.path.join(current_dir, "bin", targetname)
    shutil.copy(apkpath, targetpath)
else:
    os.chdir(current_dir)
    libsdir = os.path.join(current_dir, "libs")
    cleanLibs(libsdir + "/armeabi-v7a", "*.so")
    cleanLibs(libsdir, "wseclient*.jar")
    cmds = ["ant", "clean"]
    runCmd(cmds)

    try:
        shutil.rmtree("bin")
        shutil.rmtree("gen")
        shutil.rmtree("obj")
    except:
        pass
        
