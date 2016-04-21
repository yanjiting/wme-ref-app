import msvccompiler
import os, sys, shutil
import errno
import glob
import subprocess

build_type = ""
configuration = "Release"
platform = "Win32"
is_clean = False
def parseCommandLines(argv):
    global build_type, configuration, platform, is_clean
    print(argv)
    for arg in argv[1:]:
        arg = arg.lower()
        if(arg == "debug"):
            configuration = "Debug"
        elif(arg == "release"):
            configuration = "Release"
        elif(arg == "win32"):
            platform = "Win32"
        elif(arg == "x64"):
            platform = "x64"
        elif(arg == "clean"):
            build_type = "/clean"
            is_clean = True
        elif(arg == "rebuild"):
            build_type = "/rebuild"
        else:
            pass

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: 
            raise
        
def is_debug():
    global configuration
    return configuration == "Debug"
 
parseCommandLines(sys.argv)
current_dir = os.path.dirname(__file__)
root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../"))
vc_env = msvccompiler.query_vcvarsall(9.0)
sln_dir = os.path.join(root_dir, "mediasession", "wme4net", "wme4net.vcproj")
cmdline = "vcbuild  %s \"%s\" /platform=%s %s" % (build_type, sln_dir, platform, configuration)

print("-------------")
print(cmdline)
print("")
os.system(cmdline)
