import inspect
import os
import subprocess
import sys
import errno
import shutil
import glob

my_root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../"))

ndk_home = ""
try:
    ndk_home = os.environ['ANDROID_NDK_HOME']
except:
    pass

if not ndk_home:
    print ("\033[31m[WARN] Please set ANDROID_NDK_HOME\033[00m")
    exit(1)

pool_num=1
ndk_clean=0
ndk_debug = 0
configuration = "release"
version = "1.0.0"
enabled_cc = False
dst_modules = []
is_profile = False

ndk_args = [os.path.abspath(os.path.join(ndk_home, "ndk-build"))]

def parseCommandLines():
    global ndk_clean, ndk_debug, configuration, version, enabled_cc, dst_modules, pool_num, is_profile
    for arg in sys.argv[1:]:
        if(arg == "debug"):
            ndk_debug = 1
            configuration = arg
        elif(arg == "release"):
            ndk_debug = 0
            configuration = arg
        elif(arg == "clean"):
            ndk_clean = 1
        elif(arg == "cc"):
            print ("Enabled code coverage!")
            enabled_cc = True
        elif(arg == "profile"):
            print ("Enabled linux sampling profile")
            is_profile = True
        elif(arg.find("=") != -1):
            values = arg.split("=")
            if(values[0].lower() == "version"):
                version = values[1]
            elif(values[0].lower() == "pool_num"):
                try:
                    import string
                    pool_num = string.atoi(values[1])
                    if pool_num <= 0: pool_num = 1
                except:
                    pool_num = 1
            elif(len(values) == 2):
                os.environ[values[0]] = values[1]
            else:
                pass
        else:
            dst_modules = arg.split(",")
            
parseCommandLines()
if(not ndk_clean): 
    ndk_args += ["-j4"]
if(ndk_debug):
    ndk_args += ["NDK_DEBUG=1"]
    os.environ['NDK_DEBUG'] = "1"
    configuration = "debug"
else:
    os.environ['NDK_DEBUG'] = "0"
    ndk_args += ["NDK_DEBUG=0"]

if(is_profile):
    ndk_args += ["APP_CFLAGS=-mapcs-frame"]

if(ndk_clean):    
    ndk_args += ["clean"]
    
if True == enabled_cc:
    print ("Open code coverage flag to genreate code coverage file")
    os.environ['ENABLE_GCOV'] = '1'
    os.environ['HAVE_GCOV_FLAG'] = '1'

os.environ['NDK_TOOLCHAIN_VERSION'] = "4.8"
ndk_args += ["NDK_TOOLCHAIN_VERSION=4.8"]
def getrightvalue(str):
    npos = str.index('=')
    rvalue = str[npos+1:]
    rvalue = rvalue.strip()
    return rvalue

def getObjPath(path):
    fp=open("Application.mk", "r");
    ndkappout = "../obj"
    appmodule = ""
    appabi = ""
    NDK_APP_OUT = "NDK_APP_OUT :="
    APP_MODULES = "APP_MODULES :="
    APP_ABI = "APP_ABI :="
    for curline in fp:
        if curline.find(NDK_APP_OUT) != -1:
            npos = curline.index('/')
            ndkappout = curline[npos+1:]
            ndkappout = ndkappout.strip()
            if ndk_debug:
                if ndkappout.find("release") != -1:
                    ndkappout = ndkappout.replace("release","debug")
            else:    
                if ndkappout.find("debug") != -1:
                    ndkappout = ndkappout.replace("debug","release")
        else:
            if curline.find(APP_MODULES) != -1:
                appmodule = getrightvalue(curline)
            else:
                if curline.find(APP_ABI) != -1:
                    appabi = getrightvalue(curline)
    if(appmodule == "" or appabi ==""):
        return ""
    objsubpath = ndkappout+"/local/"+appabi+"/lib"+appmodule+".so"
    objpath = objsubpath
    return objpath

def runCmd(cmds):
    #print(" ".join(cmds))
    return os.system(" ".join(cmds))

def buildModule(path):
    old_curr = os.getcwd()
    os.chdir(path)

    returncode = runCmd(ndk_args)

    if(ndk_clean):
        try:
            shutil.rmtree(os.path.join(path, '..', 'obj'))
            shutil.rmtree(os.path.join(path, '..', 'libs'))
        except:
            pass
    else:
        global my_root_dir
        objpath=getObjPath(path)
        if(objpath != "" and objpath.find(' ')==-1): 
            dstpath=my_root_dir+"/distribution/android/armv7/maps"
            if not os.path.exists(dstpath):
                os.makedirs(dstpath)
            shutil.copy(objpath,dstpath)
    os.chdir(old_curr)
    if(returncode != 0):
        raise "build exit with %d" % (returncode)

def cleanLibs(path, pattern):
    filter = os.path.join(path, pattern)
    print (filter)
    for file in glob.glob(filter):
        print (file)
        os.remove(file)

def getHighestPlatformAPI():
    platform_dir = os.path.join(os.environ["ANDROID_HOME"], "platforms")
    highest_api = 30
    lowest_api = 10
    platformApiFound = False
    for api in range(highest_api, lowest_api, -1):
        possible_api = "android-"+str(api)
        possible_api_path = os.path.join(platform_dir, possible_api)
        if os.path.isdir(possible_api_path):
            platformApiFound = True
            break
    if(platformApiFound):        
        print ("possible_api is " + possible_api)
    else:
        print "Can't find android platforms, please check ANDROID_HOME"
    return possible_api
