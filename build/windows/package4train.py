#!/usr/bin/env python
import os, sys, shutil, distutils.dir_util
import errno
import glob

platform="Win32"
PDBFlag = False
pdbtarget = "wme4train_pdb_Win32"
ConfigurationList= ["Debug", "Release"]

for arg in sys.argv[1:]:
    arg = arg.lower()
    if(arg == "win32"):
        platform = "Win32"
        pdbtarget = "wme4train_pdb_Win32"
    elif(arg == "x64"):
        platform = "x64"
        pdbtarget = "wme4train_pdb_x64"
    elif(arg == "pdb"):
        PDBFlag = True
    else:
        pass

current_dir  = os.path.abspath(os.path.dirname(__file__))
root_dir     = os.path.abspath(os.path.join(current_dir, "../../"))
target       = os.path.join(current_dir, "wme4train_%s.zip" % platform)
pdbtargetdir = os.path.join(current_dir, "wme4train_pdb_%s" % platform)

modules = [
            "api", 
            "mediaengine/util/include",
            "mediaengine/tp/include",
            "mediaengine/dolphin/include",            #This is only for audio local recorder, will remove after player support opus.
            "mediaengine/bin/%s"  % platform,
            "mediaengine/libs/%s" % platform,
            "mediaengine/maps/%s" % platform,
            "mediaengine/shark/src/wsevp/interface",  #This is only for OpenH264, CMR
            "mediaengine/shark/include",              #This is only for OpenH264, CMR
            "vendor/openh264/api",
            "vendor/nattools-0.2/icelib/include",
            "vendor/nattools-0.2/sockaddrutil/include",
            "vendor/nattools-0.2/stunlib/include"
          ]

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise

def copy_pdbs(src, dst, filter = "*.pdb"):
    filter = os.path.join(src, filter)
    mkdir_p(dst)
    print("copy from %s to %s..." % (filter, dst))
    for file in glob.glob(filter):
        print("  " + file)
        shutil.copy2(file, dst)

def CopyPDB(configuration = "Debug"):
	pdb_Src_dir_0  = os.path.join(root_dir, "mediaengine", "maps",                 "%s" % platform, "%s" % configuration)
	pdb_Src_dir_1  = os.path.join(root_dir, "mediaengine", "bin",                  "%s" % platform, "%s" % configuration)
	pdb_Src_dir_3  = os.path.join(root_dir, "mediaengine", "transmission", "maps", "%s" % platform, "%s" % configuration)
	pdb_Src_dir_2  = os.path.join(root_dir, "vendor",      "openh264", "libs",     "%s" % platform, "%s" % configuration)
	pdb_dst_dir    = os.path.join(pdbtargetdir,"%s" %configuration )

	copy_pdbs(pdb_Src_dir_0, pdb_dst_dir, "*.pdb")
	copy_pdbs(pdb_Src_dir_0, pdb_dst_dir, "*.map")
	copy_pdbs(pdb_Src_dir_1, pdb_dst_dir, "*.pdb")
	copy_pdbs(pdb_Src_dir_1, pdb_dst_dir, "*.map")
	copy_pdbs(pdb_Src_dir_2, pdb_dst_dir, "*.pdb")
	copy_pdbs(pdb_Src_dir_2, pdb_dst_dir, "*.map")
	copy_pdbs(pdb_Src_dir_3, pdb_dst_dir, "*.pdb")
	copy_pdbs(pdb_Src_dir_3, pdb_dst_dir, "*.map")

try:
    os.remove(target)
except:
    pass

os.chdir(root_dir)

for module in modules:
    cmd = "%s/7z a -y -tzip %s %s" % (current_dir, target, module)
    print(cmd)
    os.system(cmd)

#copy pdb for train:
os.chdir(current_dir)
if (PDBFlag):
    for configuration in ConfigurationList:
        CopyPDB(configuration)

    cmd = "%s/7z a -y -tzip %s.zip %s" % (current_dir, pdbtarget, pdbtargetdir)
    print(cmd)
    os.system(cmd)

os.chdir(current_dir)
