import sys, os, shutil

wme_path = os.path.abspath(os.path.join(os.path.dirname(__file__), '../../../build/android'))
old_curr = os.getcwd()
os.chdir(wme_path)

cmd = "python build.py " + " ".join(sys.argv[1:])
print(cmd)
print("========================")
os.system(cmd)

os.chdir(old_curr)
