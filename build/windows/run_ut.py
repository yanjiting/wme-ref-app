import os, sys, shutil
import errno
import glob
import ctypes

STD_INPUT_HANDLE = -10
STD_OUTPUT_HANDLE= -11
STD_ERROR_HANDLE = -12

FOREGROUND_BLACK = 0x0
FOREGROUND_BLUE = 0x01 # text color contains blue.
FOREGROUND_GREEN= 0x02 # text color contains green.
FOREGROUND_RED = 0x04 # text color contains red.
FOREGROUND_INTENSITY = 0x08 # text color is intensified.

BACKGROUND_BLUE = 0x10 # background color contains blue.
BACKGROUND_GREEN= 0x20 # background color contains green.
BACKGROUND_RED = 0x40 # background color contains red.
BACKGROUND_INTENSITY = 0x80 # background color is intensified.

global configuration, platform, exefiles, SpecialExeFlag
configuration = "Release"
platform = "Win32"
exefiles = []
SpecialExeFlag = False
#exefiles = ['DolphinTest.exe', 'SharkTest.exe','tpTest.exe','utilTest.exe','WmeTest.exe','wqosTest.exe','wrtpTest.exe', 'MediaSessionTest.exe']
options = {'dolphin' : 'DolphinTest.exe',
                'shark' : 'SharkTest.exe',
                'tp' : 'tpTest.exe',
                'util' : 'utilTest.exe',
                'wme' : 'WmeTest.exe',
                'wqos' : 'wqosTest.exe',
                'wrtp' : 'wrtpTest.exe',
                'mediasession' : 'MediaSessionTest.exe',
                'appshare' : 'appshare_unittest.exe',
}

for arg in sys.argv[1:]:
    arg = arg.lower()
    if(arg == "debug"):
        configuration = "Debug"
    elif(arg == "release"):
        configuration = "Release"
    elif(arg == "win32"):
        platform = "Win32"
    elif(arg == "x64"):
        platform = "x64"
    else:
        file = arg
        SpecialExeFlag = True
        exefiles.append(options[file])


if ( SpecialExeFlag != True ):
    exefiles = list(options.values())



class print_color:

    handle_std_out = ctypes.windll.kernel32.GetStdHandle(STD_OUTPUT_HANDLE)
    
    def set_cmd_color(self, color, handle=handle_std_out):
        bool = ctypes.windll.kernel32.SetConsoleTextAttribute(handle, color)
        return bool
    
    def reset_color(self):
        self.set_cmd_color(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
    
    def print_red(self, string):
        self.set_cmd_color(FOREGROUND_RED | FOREGROUND_INTENSITY)
        print(string)
        self.reset_color()
        
    def print_green(self, string):
        self.set_cmd_color(FOREGROUND_GREEN | FOREGROUND_INTENSITY)
        print(string)
        self.reset_color()
    
    def print_blue(self, string): 
        self.set_cmd_color(FOREGROUND_BLUE | FOREGROUND_INTENSITY)
        print(string)
        self.reset_color()
		
current_dir = os.path.dirname(os.path.abspath(__file__))
root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../"))
#configuration = "Release"

dll_dir = os.path.join(root_dir, "mediaengine", "bin", "%s" % platform, "%s" % configuration)
ut_dir = dll_dir
os.chdir(ut_dir)
clr = print_color() 

success_files = ""
failed_files = ""
total_num = 0
failed_num = 0
success_num = 0

for exefile in exefiles :
	total_num = total_num + 1
	cmd_line = os.path.join(ut_dir, exefile)
	print("")
	print("---------------------------------------------------------")
	print("|            start to run %s                |" % exefile)
	print("---------------------------------------------------------")
	sys.stdout.flush()
	exitcode = os.system(cmd_line)
	if (exitcode == 0):
		success_num = success_num+1
		success_files = success_files + exefile + " "
		clr.print_green("%s exits with %d" % (exefile,exitcode))
		sys.stdout.flush()
	else:
		failed_num = failed_num + 1
		failed_files = failed_files + exefile + " "
		clr.print_red("%s exits with %d" % (exefile,exitcode))
		sys.stdout.flush()
	print("")	
		
print("Finish running %d unit tests" % total_num)

if(success_num > 0):
	print("")
	print("****************************************************************")
	clr.print_green("%d unit tests are passed: %s" % (success_num, success_files))
	print("****************************************************************")

if (failed_num > 0):
	print("")
	print("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
	clr.print_red("%d unit tests are failed: %s" % (failed_num, failed_files))
	print("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")	

sys.stdout.flush()
os.chdir(current_dir)
	
if(failed_num > 0):
    exit(1)