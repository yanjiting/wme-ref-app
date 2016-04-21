import msvccompiler
import os, sys, shutil
import errno
import glob

current_dir = os.path.dirname(__file__)
root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../"))
build_type = "Build"
configuration = "Release"
platform = "Win32"
parallel_builds_config = ""
cc = False
for arg in sys.argv[1:]:
    arg = arg.lower()
    if(arg == "clean"):
        build_type = "Clean"
    elif(arg == "rebuild"):
        build_type = "Rebuild"
    elif(arg == "debug"):
        configuration = "Debug"
    elif(arg == "release"):
        configuration = "Release"
    elif(arg == "win32"):
        platform = "Win32"
    elif(arg == "x64"):
        platform = "x64"
    elif(arg == "cc"):
        cc = True
    elif(arg == "parallel"):
        parallel_builds_config = "/m"
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
        
def copy_dlls(src, dst):
    filter = os.path.join(src, "*.dll")
    mkdir_p(dst)
    print("copy from %s to %s..." % (filter, dst))
    for file in glob.glob(filter):
        print("  " + file)
        shutil.copy2(file, dst)
    
def is_debug():
    global configuration
    return configuration == "Debug"
 
msbuild_dir = msvccompiler.find_msbuild(12.0)
os.environ["PATH"] += os.pathsep + msbuild_dir
sln_dir = os.path.join(root_dir, "unittest", "bld", "windows", "WMEUnittest12.sln")
profile_path = os.path.join(root_dir, "build", "windows", "profile.props")
#sln_dir = os.path.join(root_dir, "unittest", "mediasession", "MediaSessionUT.vcxproj")
coverage = ""
if(cc):
    coverage = "/p:ForceImportAfterCppTargets=\"%s\"" % (profile_path)
cmdline = "msbuild \"%s\" /t:%s %s /p:Configuration=%s /p:Platform=%s %s" % (sln_dir, build_type, parallel_builds_config, configuration, platform, coverage)

print("-------------")
print(cmdline)
print("")
exitcode = os.system(cmdline)
print("msbuild exited with %d" % exitcode)
if(exitcode != 0):
    exit(exitcode)

#print("-------------")
#print("start to copy binaries")
#print("")

#src_dir = os.path.join(root_dir, "mediaengine",  "bin",     "%s" % platform, "%s" % configuration)
#dst_dir = os.path.join(root_dir, "distribution", "windows", "%s" % platform, "%s" % configuration)
#copy_dlls(src_dir, dst_dir)
