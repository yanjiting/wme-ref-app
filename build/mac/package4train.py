#!/usr/bin/env python
import os, sys, shutil, distutils.dir_util
import errno


current_dir = os.path.abspath(os.path.dirname(__file__))
root_dir = os.path.abspath(os.path.join(current_dir, "../../"))
target = os.path.join(current_dir, "wme4train_macos.tar")

modules = [
           "api",
           "distribution/mac",
           "mediaengine/util/include",
           "mediaengine/tp/include",
           "mediaengine/dolphin/include",
           "mediaengine/shark/src/wsevp/interface",  #This is only for OpenH264, CMR
           "mediaengine/shark/include",              #This is only for OpenH264, CMR
           "vendor/openh264/api",
           "vendor/nattools-0.2/icelib/include",
           "vendor/nattools-0.2/sockaddrutil/include",
           "vendor/nattools-0.2/stunlib/include"
          ]
try:
    os.remove(target)
except:
    pass

try:
    os.remove(target + ".gz")
except:
    pass

os.system("tar cfT %s /dev/null" % target)
os.chdir(root_dir)
for module in modules:
    cmd = "tar uf %s %s" % (target, module)
    print(cmd)
    os.system(cmd)
os.system("gzip %s" % target)
os.chdir(current_dir)
