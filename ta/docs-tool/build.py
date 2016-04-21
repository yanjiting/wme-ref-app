import os, sys, shutil
import re
import argparse
import platform

parser = argparse.ArgumentParser()
parser.add_argument("-t", "--test", action="store_true", help="do local test with sphinx.")
parser.add_argument("-s", "--skip", default = False, action="store_true", help="skip doxygen which is very slow")
parser.add_argument("-c", "--clean", default = False, action="store_true", help="clean all temporary files")
parser.add_argument("-w", "--wiki", default = False, action="store_true", help="test with confluence wiki format")
args = parser.parse_args()

def copytree(src, dst):
    names = os.listdir(src)
    for name in names:
        srcname = os.path.join(src, name)
        dstname = os.path.join(dst, name)
        shutil.copy(srcname, dstname)
                
def rmdir(path):
    if os.path.exists(path):
        shutil.rmtree(path)

def purge(dir, pattern = ".*\.rest"):
    for f in os.listdir(dir):
        if re.search(pattern, f):
            os.remove(os.path.join(dir, f))
                    
current_path = os.path.abspath(os.path.dirname(__file__))
print(current_path)

if(args.clean):
    print("Cleaning doxygen files...")
    rmdir(os.path.join(current_path, "doxygen", "_doxygen"))
    print("Cleaning output files...")
    rmdir(os.path.join(current_path, "output", "images"))
    rmdir(os.path.join(current_path, "output", "_build"))
    purge(os.path.join(current_path, "output"), ".*\.rest")
    exit(0)
    
if(not args.skip):
    os.chdir(os.path.join(current_path, "doxygen"))
    os.system("doxygen")

os.chdir(current_path)
if(args.test):
    os.system("python doxygen2rst.py -u -i doxygen -o output")
    copytree(os.path.join(current_path, "doxygen/images"), os.path.join(current_path, "output/images"));
    os.chdir(os.path.join(current_path, "output"))
    os.system("make html")
    os.chdir(os.path.join(current_path, "output", "_build", "html"))
    osver = platform.system()
    if(osver == "Darwin"):
        os.system("open MediaSessionAPI.html")
    elif(osver == "Windows"):    
        os.system("start MediaSessionAPI.html")
    else:
        pass
elif(args.wiki): 
    os.system("python doxygen2rst.py -e wiki -u -i doxygen -o output")
    os.chdir(os.path.join(current_path, "output"))
    os.system("make wiki")
else:
    os.system("python doxygen2rst.py -g -u -i doxygen -o ../../../wme.wiki")
