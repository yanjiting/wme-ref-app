#!/usr/bin/env python

import sys, os, errno
import inspect
import subprocess
import time
from datetime import datetime

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: raise
        
apk_dir = os.path.join(os.path.dirname(__file__), "bin")
if(len(sys.argv) < 2):
    apk_name = os.path.join(apk_dir, "AllUnitTest-release.apk")
    if(not os.path.isfile(apk_name)):
        apk_name = os.path.join(apk_dir, "AllUnitTest-debug.apk")
    if(not os.path.isfile(apk_name)):
        print apk_name
        raise Exception("default AllUnitTest-(release|debug).apk file not found.")
else:
    apk_name = os.path.join(apk_dir, sys.argv[1])

adb = os.path.join(os.environ['ANDROID_HOME'], "platform-tools", "adb")
cmds = [adb, "devices"]
retstr = subprocess.check_output(cmds, shell=True)
devices = []
for item in retstr.split("\n"):
    line = item.split()
    if(len(line) == 2 and line[1].strip() == "device"):
        devices.append(line[0])

if(len(devices) == 0):
    print "[Error] No device detected, please attach device to complete the native unit test."
    exit(1)

def get_unittest(dev, mod, xml):
    print "Start to parse unittest results for device of %s: %s: %s" % (dev, mod, xml)

    # TODO: parse and check gtest xml result
    while 1:
        cmds = [adb, "shell", "ps", "|", "grep", "com.cisco.wme.unittest"]
        try:
            subprocess.check_output(cmds, shell=True)
            time.sleep(2)
        except subprocess.CalledProcessError as e:
            print "process exited"
            break
    rand = datetime.now().strftime('%Y_%m_%d_%H_%M_%S')        
    res = os.path.join(os.path.dirname(__file__), "report", mod + "-" + dev + "-" + rand + ".xml")
    cmds = [adb, "-s", dev, "pull", xml, res]
    try:
        print subprocess.check_output(cmds, shell=True)
    except subprocess.CalledProcessError as e:
        print "get result failed."
        print(e.output)

mkdir_p(os.path.join(os.path.dirname(__file__), "report"))     
for dev in devices:
    cmds = [adb, "-s", dev, "uninstall", "com.cisco.wme.unittest"]
    print subprocess.check_output(cmds, shell=True)
    cmds = [adb, "-s", dev, "install", "-r", apk_name]
    print subprocess.check_output(cmds, shell=True)
    modules="tp util wqos wrtp dolphin shark wme"
    for mod in modules.split():
        print "running unittest of %s for devices of %s" % (mod, dev)
        sys.stdout.flush()
        xml="/sdcard/%s-gtest.xml" % mod
        cmds = [adb, "-s", dev, "shell", "rm", xml]
        print subprocess.check_output(cmds, shell=True)
        cmds = [adb, "-s", dev, "shell", "am", "start", "--es", "module", mod, "--es", "path", xml, "-n", "com.cisco.wme.unittest/.MainActivity"]
        print subprocess.check_output(cmds, shell=True)
        get_unittest(dev, mod, xml)
        sys.stdout.flush()
    break
    