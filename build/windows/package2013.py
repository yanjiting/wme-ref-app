#!/usr/bin/env python
import os, sys, shutil, distutils.dir_util
import errno
import glob

ConfigurationList = [ 'Debug',  'Release']
platform = "Win32"
PDBFlag = False
target = "MediaSDK_Demo_Windows"

for arg in sys.argv[1:]:
    arg = arg.lower()
    if(arg == "debug"):
        ConfigurationList = [ 'Debug']
    elif(arg == "release"):
        ConfigurationList = [ 'Release' ]
    elif(arg == "debug-release"):
        ConfigurationList = [ 'Debug',  'Release']
    elif(arg == "win32"):
        platform = "Win32"
    elif(arg == "x64"):
        platform = "x64"
    elif(arg == "pdb"):
        PDBFlag = True
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

def copy_dir(src, dst):
    #filter = os.path.join(src, "*.dll")
    mkdir_p(dst)
    print("copy from %s to %s..." % (src, dst))
    for path, dirs, files in os.walk(src):
        for sub_dir in dirs:
            print("copy  " + sub_dir)
            sub_src_dir = os.path.join(path, sub_dir)
            sub_dst_dir = os.path.join(dst, sub_dir)
            mkdir_p(sub_dst_dir)
            distutils.dir_util.copy_tree(sub_src_dir, sub_dst_dir)
            
def copy_file(src, dst):
    #filter = os.path.join(src, "*.dll")
    mkdir_p(dst)
    print("copy from %s to %s..." % (src, dst))
    for path, dirs, files in os.walk(src):
        for file_name in files:
            print("copy  " + file_name)  
            src_file = os.path.join(path, file_name)   
            shutil.copy2(src_file, dst) 

def copy_pdbs(src, dst, filter = "*.pdb"):
    filter = os.path.join(src, filter)
    mkdir_p(dst)
    print("copy from %s to %s..." % (filter, dst))
    for file in glob.glob(filter):
        print("  " + file)
        shutil.copy2(file, dst)


def CopyAllFile(ConfigurationList):
	current_dir = os.path.dirname(__file__)
	root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../"))
	for  configuration in ConfigurationList:
		#dst dir setting
		include_dir    = os.path.join(current_dir, "%s" % target, "include")
		sdk_dir        = os.path.join(current_dir, "%s" % target, "sdk",  "%s" %platform, "%s" %configuration)
		libs_dir       = os.path.join(current_dir, "%s" % target, "libs", "%s" %platform, "%s" %configuration )
		libs_debug_dir = os.path.join(current_dir, "%s" % target, "libs", "%s" %platform, "Debug")
		pdb_dst_dir    = os.path.join(current_dir, "%s" % target, "pdbs", "%s" %platform, "%s" %configuration )

		#src dir setting
		api_dir         = os.path.join(root_dir, "api")
		dist_dir        = os.path.join(root_dir, "distribution", "windows", "%s" % platform, "%s" % configuration)
		media_dir       = os.path.join(root_dir, "mediaengine",  "libs",    "%s" % platform, "%s" % configuration)
		media_debug_dir = os.path.join(root_dir, "mediaengine",  "libs",    "%s" % platform, "Debug")

		#copy files
		copy_file(api_dir, include_dir)
		copy_file(dist_dir, sdk_dir)
		copy_file(media_dir, libs_dir)
		copy_file(media_debug_dir, libs_debug_dir)

		#copy pdb files
		#**********************************************************************
		# no pdb files projects listed as below:
		#  dlls: libeay32.dll  libeay64.dll  ssleay32.dll ssleay64.dll tp.dll
		#  libs: distorm.lib   ffpref.lib    g722.lib     iLBC.lib
		#        qoem.lib      stunlib.lib   wmeutil.lib
		#**********************************************************************
		pdb_Src_dir_0  = os.path.join(root_dir, "mediaengine", "maps",                 "%s" % platform, "%s" % configuration)
		pdb_Src_dir_1  = os.path.join(root_dir, "mediaengine", "bin",                  "%s" % platform, "%s" % configuration)
		pdb_Src_dir_3  = os.path.join(root_dir, "mediaengine", "transmission", "maps", "%s" % platform, "%s" % configuration)
		pdb_Src_dir_2  = os.path.join(root_dir, "vendor",      "openh264", "libs",     "%s" % platform, "%s" % configuration)

		if (PDBFlag):
			copy_pdbs(pdb_Src_dir_0, pdb_dst_dir, "*.pdb")
			copy_pdbs(pdb_Src_dir_0, pdb_dst_dir, "*.map")
			copy_pdbs(pdb_Src_dir_1, pdb_dst_dir, "*.pdb")
			copy_pdbs(pdb_Src_dir_1, pdb_dst_dir, "*.map")
			copy_pdbs(pdb_Src_dir_2, pdb_dst_dir, "*.pdb")
			copy_pdbs(pdb_Src_dir_2, pdb_dst_dir, "*.map")
			copy_pdbs(pdb_Src_dir_3, pdb_dst_dir, "*.pdb")
			copy_pdbs(pdb_Src_dir_3, pdb_dst_dir, "*.map")
		#**********************************************************************

current_dir = os.path.dirname(__file__)
root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../"))

CopyAllFile(ConfigurationList)
package_dir = os.path.join(current_dir, "%s" % target)

#zip
cmd = "7z a %s.zip %s" % (target, package_dir)
#cmd = "tar -czf %s.tar.gz %s" % (target, package_dir)
#cmd = "zip -rq9 %s.zip %s" % (target, package_dir)
print(cmd)
os.system(cmd)
 


 
