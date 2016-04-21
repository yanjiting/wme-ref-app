import msvccompiler
import os, sys, shutil
import errno
import glob

current_dir = os.path.dirname(__file__)
root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../"))
build_type = "Build"
configuration = "Release"
platform = "Win32"
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
#    elif(arg == "nodotnet"):    
#        wme4nettest = False
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
        
def copy_app(src, dst, filter = "wme4netTest.exe"):
    filter = os.path.join(src, filter)
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
if (platform == "Win32"):
    sln_dir = os.path.join(root_dir, "unittest", "mediasession", "wme4netTest", "wme4netTest.csproj")
else:
    sln_dir = os.path.join(root_dir, "unittest", "mediasession", "wme4netTest64", "wme4netTest64.csproj")

coverage = ""
target = "/t:%s" % build_type
#if(not wme4net):
#    if(build_type == "Build"):
#        target = "/t:mediasession"
#    else:    
#        target = "/t:mediasession:%s" % build_type
if(cc):
    profile_path = os.path.join(root_dir, "build", "windows", "profile.props")
    coverage = "/p:ForceImportAfterCppTargets=\"%s\"" % (profile_path)
if (platform == "Win32"):
    cmdline = "msbuild \"%s\" %s /p:Configuration=%s %s" % (sln_dir, target, configuration, coverage)
else:
    cmdline = "msbuild \"%s\" %s /p:Platform=%s /p:Configuration=%s %s" % (sln_dir, target, platform, configuration, coverage)

print("-------------")
print(cmdline)
print("")
exitcode = os.system(cmdline)
print("msbuild exited with %d" % exitcode)
if(exitcode != 0):
    exit(exitcode)

print("-------------")
print("start to copy binaries")
print("")

src_dir = os.path.join(root_dir, "mediaengine",  "bin",     "%s" % platform, "%s" % configuration)
dst_dir = os.path.join(root_dir, "distribution", "windows", "%s" % platform, "%s" % configuration)
copy_app(src_dir, dst_dir)

