#!/usr/bin/env python

import sys, os, shutil, errno
import glob
import subprocess

ndk_home = ""
try:
    ndk_home = os.environ['ANDROID_NDK_HOME']
except:
    pass
    
if not ndk_home:
    print "\033[31m[WARN] Please set ANDROID_NDK_HOME\033[00m"
    exit(1)
    
ndk_debug = 0
ndk_clean=0
configuration = "release"
ndk_profile=0
ndk_args = [os.path.abspath(os.path.join(ndk_home, "ndk-build"))]
for arg in sys.argv[1:]:
    if(arg == "debug"):
        ndk_debug = 1
        configuration = "debug"
    elif(arg == "release"):
	ndk_debug = 0
	configuration = "release"
    elif(arg == "clean"):
        ndk_args += ["clean"]
        ndk_clean = 1
    elif(arg == "cc"):
        os.environ['HAVE_GCOV_FLAG'] = '1'
    elif(arg == "profile"):
        ndk_profile = 1
    else:
        print("unsupported arguments: " + arg)
        exit(5)
        
if(ndk_debug):
    ndk_args += ["NDK_DEBUG=1"]
else:    
    ndk_args += ["NDK_DEBUG=0"]
    
if(ndk_profile):
    ndk_args += ["APP_CFLAGS=-mapcs-frame"]

current_dir = os.path.abspath(os.path.dirname(__file__))
root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../../"))
distdir = os.path.join(root_dir, 'distribution/android/armv7', configuration)
libsdir = os.path.join(current_dir, "ClickCall/libs/")
dlibsdir = os.path.join(current_dir, "ClickCall/libs/armeabi-v7a/")
libcpath = os.path.join(os.environ['ANDROID_NDK_HOME'], "sources/cxx-stl/llvm-libc++/libs/armeabi-v7a")

def cleanLibs(path, pattern):
    filter = os.path.join(path, pattern)
    print filter
    for file in glob.glob(filter):
        print file
        os.remove(file)

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise

def getHighestPlatformAPI():
    platform_dir = os.path.join(os.environ["ANDROID_HOME"], "platforms")
    highest_api = 30
    lowest_api = 10
    for api in range(highest_api, lowest_api, -1):
        possible_api = "android-"+str(api)
        possible_api_path = os.path.join(platform_dir, possible_api)
        if os.path.isdir(possible_api_path):
            break
    print "possible_api is " + possible_api
    return possible_api

def buildXML():
    xml_path = os.path.join(current_dir, "ClickCall", "build.xml")
    xml_file = open(xml_path, "w")
    
    xml = """<?xml version="1.0" encoding="UTF-8"?>
<project name="ClickCall" default="help">
    <property file="local.properties" />
    <property file="ant.properties" />
    <property environment="env" />
    <condition property="sdk.dir" value="${env.ANDROID_HOME}">
        <isset property="env.ANDROID_HOME" />
    </condition>
    <import file="custom_rules.xml" optional="true" />
    <loadproperties srcFile="project.properties" />
    <import file="${sdk.dir}/tools/ant/build.xml" />
</project>
    """
    xml_file.write(xml)
    xml_file.close()

    from os.path import expanduser
    user_home = expanduser("~")
    if not (os.path.isdir(user_home+'/.android')):
        mkdir_p(user_home + '/.android')
    if not (os.path.exists(user_home+'/.android/debug.keystore')):
        os.system('keytool -genkeypair -alias androiddebugkey -keypass android -keystore ' + user_home + '/.android/debug.keystore -storepass android -dname "CN=Android Debug,O=Android,C=US" -keyalg RSA -keysize 2048 -validity 9999')
    
    projprops = """
key.store=../keystore/wmeAndroid.store
key.alias=wmeAndroid
key.store.password=wme123
key.alias.password=wme123
    """
    projprops = "target=" + getHighestPlatformAPI() + projprops
    prop_file = open(os.path.join(current_dir, "ClickCall", "project.properties"), "w")
    prop_file.write(projprops)
    prop_file.close()

    return xml_path

if(not ndk_clean):
    if not (os.path.exists(dlibsdir)):
        mkdir_p(dlibsdir)

    os.chdir(os.path.join(current_dir, 'ClickCall/jni'))
    cmds = [" ".join(ndk_args), "NDK_TOOLCHAIN_VERSION=4.8"]
    if os.system(" ".join(cmds)) != 0:
        sys.exit(1)

    os.chdir(os.path.join(current_dir, 'ClickCall'))
    #os.system(" ".join(ndk_args))
    mkdir_p(dlibsdir)   
    for file in glob.glob(distdir + "/*.jar"):
        print file
        shutil.copy(file, libsdir)
        
    for file in glob.glob(distdir + "/*.so"):
        print file
        shutil.copy(file, dlibsdir)        
    
    if(not os.path.isfile(os.path.join(dlibsdir, "libc++_shared.so"))):
        shutil.copy2(os.path.join(libcpath, "libc++_shared.so"), dlibsdir)
    
    for file in glob.glob(os.path.join(current_dir, "ClickCall", "jar", "*.jar")):
        print file
        shutil.copy(file, libsdir)
        
    buildXML()
    cmds = ["ant", "-f", "build.xml", configuration]
    os.system(" ".join(cmds))
    apkpath = os.path.join(current_dir, "ClickCall", "bin", "ClickCall-%s.apk" % (configuration))
    mkdir_p(os.path.join(current_dir, "ClickCall", "target"))
    targetpath = os.path.join(current_dir, "ClickCall", "target", "com-wx2-clickcallapp-1.0.0-SNAPSHOT.apk")
    shutil.copy(apkpath, targetpath)
else:
    try:
        os.chdir(os.path.join(current_dir, 'ClickCall'))
        os.system("ndk-build clean")
    except:
        pass
    os.system("ant clean")
    cleanLibs(os.path.join(libsdir, "armeabi-v7a"), "*.so")
    cleanLibs(libsdir, "wseclient*.jar")
    cleanLibs(libsdir, "wmeclient*.jar")
    cleanLibs(libsdir, "wmeaudiohwlibrary*.jar")
    cleanLibs(libsdir, "appshare*.jar")
	
    try:
        os.chdir(os.path.join(current_dir, 'ClickCall/jni'))
        cmds = [" ".join(ndk_args), "NDK_TOOLCHAIN_VERSION=4.8", "clean"]
        os.system(cmds)
    except:
        pass

    try:
        os.remove("build.xml")
        os.remove("project.properties")
        shutil.rmtree("target")
        shutil.rmtree("obj")
    except:
        pass
        
os.chdir(current_dir)
