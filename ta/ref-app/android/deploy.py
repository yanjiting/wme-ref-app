import os, sys
import subprocess

configuration = "release"
if(len(sys.argv) > 1):
    if(sys.argv[1] == "debug"):
        configuration = "debug"
    elif(sys.argv[1] == "release"):
        configuration = "release"
    
current_dir = os.path.abspath(os.path.dirname(__file__))
adb = os.path.join(os.environ['ANDROID_HOME'], "platform-tools", "adb")
def listDevices():
    cmds = [adb, "devices"]
    retstr = subprocess.check_output(" ".join(cmds), shell=True)
    devices = []
    for item in retstr.split("\n"):
        line = item.split()
        if(len(line) == 2 and line[1].strip() == "device"):
            devices.append(line[0])
    return devices

def deployToDevice(device, apk_path):
    print "Start to work on device: %s" % device
    #sys.stdout.write("Uninstall Clickcall:")
    #os.system("%s -s %s uninstall com.wx2.clickcall" % (adb, device))
    sys.stdout.write("Re-install Clickcall:")
    os.system("%s -s %s install -r %s" % (adb, device, apk_path))

devices = listDevices()
deploy_idx = -1
if(len(devices) == 0):
    print "[Error] No device detected, please attach device to complete the native unit test."
    exit(1)
elif(len(devices) > 1):
    print "You have attached multiple devices:"
    i = 0
    for device in devices:
        print "  %d - %s" % (i, device)
        i += 1
    sys.stdout.write("please select device to deploy[all]:")
    try:
        i = input()
        deploy_idx = i
        print "You selected: %s" % devices[deploy_idx]
    except:
        print "use default, install to all"

apk_path = os.path.join(current_dir, "ClickCall", "bin", "ClickCall-%s.apk" % configuration)        
if(deploy_idx == -1):
    for device in devices:
        deployToDevice(device, apk_path)
else:
    deployToDevice(devices[deploy_idx], apk_path)