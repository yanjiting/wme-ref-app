#!/usr/bin/env python

import sys, os, shutil
import glob
import subprocess
if __name__ == '__main__':
    sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), '../../../build/android')))
    from buildtool import * 
    current_dir = os.path.abspath(os.path.dirname(__file__))
    root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../"))

distdir = os.path.abspath(os.path.join(current_dir, '../../../distribution/android/armv7', configuration))
libsdir = os.path.join(current_dir, "libs")

def generateProjectProperties(path):
    projprops = """
key.store=keystore/wmeAndroid.store
key.alias=wmeAndroid
key.store.password=wme123
key.alias.password=wme123
    """
    projprops = "target=" + getHighestPlatformAPI() + projprops
    
    prop_file = open(path, "w")
    prop_file.write(projprops)
    prop_file.close()

if(not ndk_clean):
    buildModule(os.path.join(current_dir, 'jni'))
    os.chdir(current_dir)
    sodir = os.path.join(current_dir, "libs", "armeabi-v7a")
    for file in glob.glob(distdir + "/*.so"):
        print (file)
        shutil.copy(file, sodir)
        
    for file in glob.glob(distdir + "/*.jar"):
        print (file)
        shutil.copy(file, libsdir)

    properties_path = os.path.join(current_dir, "project.properties")
    generateProjectProperties(properties_path)

    cmds = ["ant", "-f", "build.xml", configuration]
    os.system(" ".join(cmds))
    apkpath = os.path.join(current_dir, "bin", "SettingActivity-%s.apk" % configuration);
    targetname = os.path.join(distdir, "EngineAndroid-%s-%s.apk" % (configuration, version))
    targetpath = os.path.join(current_dir, "target", targetname)
    shutil.copy(apkpath, targetpath)
else:
    try:
        buildModule(os.path.join(current_dir, 'jni'))
    except:
        pass
    cleanLibs(libsdir + "/armeabi-v7a", "*.so")
    cleanLibs(libsdir, "wseclient*.jar")
    os.chdir(current_dir)
    cmds = ["ant", "clean"]
    runCmd(cmds)

    try:
        shutil.rmtree("bin")
        shutil.rmtree("gen")
        shutil.rmtree("obj")
    except:
        pass
