#!/usr/bin/env python
import os, sys, shutil, distutils.dir_util
import errno

target = "../../distribution/WP8/wme-wp8-"
configration = "release"
current_dir = os.path.dirname(__file__)
root_dir = os.path.abspath(os.path.join(current_dir, "../../"))

modules = [
    "distribution/WP8/include",
    "distribution/WP8/libs/",
    "distribution/WP8/dlls/"
]

def package():
    if configration == 'clean':
        os.remove("%s.zip" % target)
        sys.exit(0)
    for module in modules:
        src_path = os.path.abspath(os.path.join(root_dir, module))
        print src_path
        cmd = "7z a -y -tzip %s.zip %s" % (target, src_path)
        print(cmd)
        os.system(cmd)

def usage():
    print "Usage: python package.py [all | debug | release | clean]"
    sys.exit(0)

if __name__ == "__main__":
    args = sys.argv[1:]
    if len(args) == 0:
        configration = 'release'
    else :
        arg = args[0].lower()
        if  arg == "debug":
            configration = 'debug'
        elif arg == "release":
            configration = 'release'
        elif arg == 'all':
            configration = ''
        elif arg == 'clean':
            configration = 'clean'
        else:
            usage()
    target += configration
    package()