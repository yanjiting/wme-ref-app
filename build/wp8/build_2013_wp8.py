import msvccompiler
import os, sys, shutil
import errno
import glob

current_dir = os.path.dirname(__file__)
root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../"))
build_type = "Build"
configuration = "release"
platform = "ARM"

cc = False
test = False
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
    elif(arg == "test"):
        test = True
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

def copy_headers(src, dst, filter = "*.h"):
    filter = os.path.join(src, filter)
    mkdir_p(dst)
    print("copy from %s to %s..." % (filter, dst))
    for file in glob.glob(filter):
        print("  " + file)
        shutil.copy2(file, dst)
		
def copy_pdbs(src, dst):
	return copy_dlls(src, dst, "*.pdb")
	
def copy_dlls(src, dst, filter = "*.dll"):
    filter = os.path.join(src, filter)
    mkdir_p(dst)
    print("copy from %s to %s..." % (filter, dst))
    for file in glob.glob(filter):
        print("  " + file)
        shutil.copy2(file, dst)

def copy_libs(src, dst, filter = "*.lib"):
    filter = os.path.join(src, filter)
    mkdir_p(dst)
    print("copy from %s to %s..." % (filter, dst))
    for file in glob.glob(filter):
        print("  " + file)
        shutil.copy2(file, dst)

def is_debug():
    global configuration
    return configuration == "debug"

msbuild_dir = msvccompiler.find_msbuild(12.0)
os.environ["PATH"] += os.pathsep + msbuild_dir
sln_dir = os.path.join(root_dir, "mediaengine", "bld", "WP8", "MediaSDKClient.sln")
cert = "/p:SignAssembly=true /p:AssemblyOriginatorKeyFile=../../wme.snk"
coverage = ""
#if(cc):
    #profile_path = os.path.join(root_dir, "build", "windows", "profile.props")
    #coverage = "/p:ForceImportAfterCppTargets=\"%s\"" % (profile_path)
cmdline = "msbuild \"%s\" /t:%s /p:Configuration=%s /p:Platform=%s %s %s" % (sln_dir, build_type, configuration, platform, cert, coverage)

print("-------------")
print(cmdline)
print("")

if test == True:
    exitcode = 0
else:
    exitcode = os.system(cmdline)

print("msbuild exited with %d" % exitcode)
if(exitcode != 0):
    exit(exitcode)

if build_type == "Clean":
    exit(0)

print("-------------")
print("start to copy distribution files\n")

#copy dll from vendor to bin
dst_dir = os.path.join(root_dir, "mediaengine/bin/WP8/%s" % configuration)
src_dir = os.path.join(root_dir, "vendor/openssl/WP8/bin")
copy_dlls(src_dir, dst_dir)
copy_pdbs(src_dir, dst_dir)

src_dir = os.path.join(root_dir, "vendor/openh264/libs/wp8/Release")
copy_dlls(src_dir, dst_dir)
copy_pdbs(src_dir, dst_dir)

#copy lib from vendor to libs
dst_dir = os.path.join(root_dir, "mediaengine/libs/WP8/%s" % configuration)
src_dir = os.path.join(root_dir, "vendor/libsdp/build/wp8/%s" % configuration)
copy_libs(src_dir, dst_dir)

src_dir = os.path.join(root_dir, "vendor/nattools-0.2/target/wp8/%s" % configuration)
copy_libs(src_dir, dst_dir)

src_dir = os.path.join(root_dir, "vendor/security/libs/WP8/%s" % configuration)
copy_libs(src_dir, dst_dir)

#copy dll form bin to distribution
src_dir = os.path.join(root_dir, "mediaengine/bin/WP8/%s" % configuration)
dst_dir = os.path.join(root_dir, "distribution/WP8/dlls/%s/" % configuration)
copy_dlls(src_dir, dst_dir)
copy_pdbs(src_dir, dst_dir)
copy_dlls(src_dir, dst_dir, "*.cso")

#copy lib to distribution
src_dir = os.path.join(root_dir, "mediaengine/bin/WP8/%s" % configuration)
dst_dir = os.path.join(root_dir, "distribution/WP8/libs/%s" % configuration)
copy_libs(src_dir, dst_dir)

src_dir = os.path.join(root_dir, "mediaengine/libs/WP8/%s" % configuration)
copy_libs(src_dir, dst_dir)

#copy header files to distribution
src_dir = os.path.join(root_dir, "api")
dst_dir = os.path.join(root_dir, "distribution/WP8/include")
copy_headers(src_dir, dst_dir)
