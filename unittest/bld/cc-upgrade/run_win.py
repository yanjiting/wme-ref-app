from os.path import normpath, dirname, join, abspath
import os, sys, shutil
local_module = abspath(join(dirname(__file__), '../../../build/windows'))
sys.path.insert(0, local_module)

import msvccompiler
import errno
import glob
from vs_cobertura import VSCobertura
import subprocess
import time
    
root_dir = abspath(join(dirname(__file__), '../../../'))
configuration = "release"

def mkdir_p(path):
    try:
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else: 
            raise
        
def copy_dlls(src, dst, filter_ext="*.dll"):
    filter = os.path.join(src, filter_ext)
    mkdir_p(dst)
    print("copy from %s to %s..." % (filter, dst))
    for file in glob.glob(filter):
        print("  " + file)
        shutil.copy2(file, dst)

def print_sep(text):
    print("---------")
    print("")
    print(text)

def convert_to_xml(coverage_path, program):
    convert_tool = abspath(os.path.join(root_dir, "unittest", "bld", "cc", "Coverage2XML", "Coverage2XML", "bin", "Release"))
    os.environ["PATH"] += os.pathsep + convert_tool
    exe_path = os.path.join(coverage_path, "%s.exe" % program)
    pdb_path = os.path.join(coverage_path, "%s.pdb" % program)
    cov_path = os.path.join(coverage_path, "%s.coverage" % program)
    os.system("Coverage2XML.exe %s %s %s" % (exe_path, pdb_path, cov_path))

def instrument_file(file):
    cmd = "vsinstr.exe \"%s\" /COVERAGE" % (file)
    print_sep("Start to instruments: %s" % file)
    os.system(cmd)

def get_coverage(coverage_path, file):
    file_name = os.path.basename(file)
    module_name = os.path.splitext(file_name)[0]
    coverage_data = os.path.join(coverage_path, "%s.coverage" % module_name)
    os.system("start /B vsperfmon /COVERAGE /OUTPUT:\"%s\"" % coverage_data)
    time.sleep(2)
    os.system(file)
    os.system("vsperfcmd.exe -shutdown")
    print("\nstart to covert to XML")
    convert_to_xml(coverage_path, module_name)

def run(targets):
    dist_path = os.path.join(root_dir, "distribution", "windows", configuration)
    map_path = os.path.join(root_dir, "unittest", "maps", "Win32", configuration)
    mapdll_path = os.path.join(root_dir, "mediaengine", "maps", "Win32", configuration)
    exe_path = os.path.join(root_dir, "mediaengine", "bin", "win32", configuration)
    coverage_path = os.path.join(root_dir, "win_coverage_tmp")
    mkdir_p(coverage_path)
    os.chdir(coverage_path)
    copy_dlls(dist_path, coverage_path)
    copy_dlls(map_path, coverage_path, "*.pdb")
    copy_dlls(mapdll_path, coverage_path, "*.pdb")
    copy_dlls(dist_path, coverage_path)
    copy_dlls(exe_path, coverage_path, "*.exe")
    vcroot = msvccompiler.find_vcroot(9.0)
    perf_tool = abspath(os.path.join(vcroot, "..", "Team Tools", "Performance Tools"))
    os.environ["PATH"] += os.pathsep + perf_tool
    
    for target in targets:
        starttime = int(round(time.time() * 1000))
        file = os.path.join(coverage_path, target[0])
        instrument_file(file)
        for dll_file in target[1:]:
            dll_path = os.path.join(coverage_path, dll_file)
            instrument_file(dll_path)
        print_sep("Start to run ut: %s" % file)
        get_coverage(coverage_path, file)
        print_sep("Used time: %d for %s" % (int(round(time.time() * 1000)) - starttime, target))
        for dll_file in target[1:]:
            copy_dlls(dist_path, coverage_path, dll_file)
    
    try:
        vs_cobertura = VSCobertura(root_dir)
        for file in glob.glob(os.path.join(coverage_path, "*.exe.xml")):
            print("add %s coverage file" % file)
            starttime = int(round(time.time() * 1000))
            vs_cobertura.addCoverage(file, ["wme\\mediasession", "wme\\mediaengine"], ["include", "unittest"])
            print("add %s coverage file end with time: %d " % (file, int(round(time.time() * 1000)) - starttime))
        cobertura_xml = vs_cobertura.convert()
        with open(os.path.join(coverage_path, "coverage.xml"), mode='wt') as output_file:
            output_file.write(cobertura_xml)
    except IOError:
        sys.stderr.write("Unable to convert visual studio coverage to Cobertura XML.")

if __name__ == '__main__':
    targets = [
        ["utilTest.exe", "util.dll"],
        ["tpTest.exe", "tp.dll"],
        ["wrtpTest.exe", "wrtp.dll"],
        ["wqosTest.exe", "wqos.dll"],
        ["DolphinTest.exe", "wbxaudioengine.dll"],
        ["SharkTest.exe", "wseclient.dll"],
        ["wmetest.exe", "wmeclient.dll"],
        ["mediasessionTest.exe", "mediasession.dll"],
        ["appshare_unittest.exe", "appshare.dll"]
    ]
    run(targets)
