import msvccompiler
import os, sys, shutil
import errno
import glob

current_dir = os.path.dirname(__file__)
root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../"))
build_type = "Build"
configuration = "release"
platform = "ARM"

cc = False
for arg in sys.argv[1:]:
    arg = arg.lower()
    if(arg == "clean"):
        build_type = "Clean"
    elif(arg == "rebuild"):
        build_type = "Rebuild"
    elif(arg == "debug" or arg == "release"):
        configuration = sys.argv[1]
    elif(arg == "cc"):
        cc = True
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

def copy_dlls(src, dst, filter = "*.dll"):
    filter = os.path.join(src, filter)
    mkdir_p(dst)
    print("copy from %s to %s..." % (filter, dst))
    for file in glob.glob(filter):
        print("  " + file)
        shutil.copy2(file, dst)

def is_debug():
    global configuration
    return configuration == "debug"

#msbuild_dir = msvccompiler.find_msbuild(12.0)
#print(msbuild_dir)
devenv_dir = msvccompiler.find_devenv()
print 'devenv path = %s \n' %(devenv_dir)
if '12.0' not in devenv_dir:
    print 'Error: devenv.exe version should be 12.0, please check the devenv.exe path in registry.'
    print 'The correct key/values should be:'
    print '    Key: HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\devenv.exe'
    print '    Value: C:\Program Files (x86)\Microsoft Visual Studio 12.0\Common7\IDE\devenv.exe'
    exit(-1)

os.environ["PATH"] += os.pathsep + devenv_dir
sln_dir = os.path.join(root_dir, "Refapp-wp", "Refapp-wp.sln")
cert =""
#cert = "/p:SignAssembly=true /p:AssemblyOriginatorKeyFile=../../../../wme.snk"
coverage = ""
#if(cc):
    #profile_path = os.path.join(root_dir, "build", "windows", "profile.props")
    #coverage = "/p:ForceImportAfterCppTargets=\"%s\"" % (profile_path)
#cmdline = "devenv \"%s\" /t:%s /p:Configuration=%s /p:Platform=%s %s %s" % (sln_dir, build_type, configuration, platform, cert, coverage)
cmdline = "devenv  %s /%s \"%s|%s\"" %(sln_dir, build_type, configuration, platform)
print("-------------")
print(cmdline)
print("")
exitcode = os.system(cmdline)
print("devenv exited with %d" % exitcode)
if(exitcode != 0):
    exit(exitcode)

print("-------------")
print("start to copy binaries")
print("")

src_dir = os.path.join(root_dir, "Refapp-wp/Refapp-wp/Bin/ARM/%s/" % configuration)
dst_dir = os.path.join(root_dir, "bin/%s" % configuration)
copy_dlls(src_dir, dst_dir, "*.xap")
#src_dir = os.path.join(root_dir, "vendor/openssl/WP8/bin")
#copy_dlls(src_dir, dst_dir)
