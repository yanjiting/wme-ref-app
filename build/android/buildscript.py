import os
import sys
import time
import datetime
current_dir = os.path.dirname(__file__)
current_absdir = os.path.abspath(current_dir)
begin_time = int(round(time.time() * 1000))
root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../"))
exitcode = 0

def build(folder):
    global current_dir
    global __file__
    global __name__
    back_dir = current_dir
    back_file = __file__
    back_name = __name__
    starttime = int(round(time.time() * 1000))
    
    current_dir = folder
    build_script = os.path.join(folder, "build.py")
    __file__ = build_script
    __name__ = "build"
    
    print ("")
    print ("".ljust(15, "-"))
    print (folder)
    print ("")
    if(os.path.isfile(build_script)):
        exec(open(build_script).read())
    else:
        buildModule(folder)
    current_dir = back_dir
    __file__ = back_file
    __name__ = back_name
    return int(round(time.time() * 1000)) - starttime

def report(title, result, time_sp):
    output = ("".ljust(5) + title).ljust(60)
    d = datetime.timedelta(milliseconds=time_sp) 
    time_str = str(d).split('.')
    time_str = "".join(time_str[0:-1])

    print  (output + result + " [%s] " % time_str)
    sys.stdout.flush()

modules_seq = modules
if(ndk_clean):
    modules_seq = reversed(modules)
for module in modules_seq:
    try:
        if(len(dst_modules) > 0):
            if(not module["tag"] in dst_modules):
                continue;
            
        module["time"] = 0
        time_spent = build(os.path.join(root_dir, module["path"]))
        module["status"] = "Success"
        module["time"] = time_spent
    except Exception as e:
        print (e)
        exitcode = 3
        module["status"] = "Failed"
        break

print ("")
print ("".ljust(80, "="))
for module in modules:
    result = "Skipped"
    if(module["status"]):
        result = module["status"]
    if(not "time" in module):
        module["time"] = 0
    report(module["title"], result, module["time"])

print ("")
print ("".ljust(80, "="))
total_time = int(round(time.time() * 1000)) - begin_time
d = datetime.timedelta(milliseconds=total_time) 
time_str = str(d).split('.')
time_str = "".join(time_str[0:-1])
print ("".ljust(50) + "Total Time: %s" % time_str)

os.chdir(current_absdir) 
