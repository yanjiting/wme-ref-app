try:
    import _winreg as wr
except ImportError:
    import winreg as wr

import os, sys
import subprocess

RegOpenKeyEx = wr.OpenKeyEx
RegEnumKey = wr.EnumKey
RegEnumValue = wr.EnumValue
RegError = wr.error

HKEYS = (wr.HKEY_USERS,
         wr.HKEY_CURRENT_USER,
         wr.HKEY_LOCAL_MACHINE,
         wr.HKEY_CLASSES_ROOT)

def Is64Windows():
    return 'PROGRAMFILES(X86)' in os.environ

#NATIVE_WIN64 = (sys.platform == 'win32' and sys.maxsize > 2**32)
NATIVE_WIN64 = Is64Windows()

if NATIVE_WIN64:
    # Visual C++ is a 32-bit application, so we need to look in
    # the corresponding registry branch, if we're running a
    # 64-bit Python on Win64
    VS_BASE = r"Software\Wow6432Node\Microsoft\VisualStudio\%0.1f"
    VSEXPRESS_BASE = r"Software\Wow6432Node\Microsoft\VCExpress\%0.1f"
    WINSDK_BASE = r"Software\Wow6432Node\Microsoft\Microsoft SDKs\Windows"
    NET_BASE = r"Software\Wow6432Node\Microsoft\.NETFramework"
    MSBUILD_BASE = r"Software\Wow6432Node\Microsoft\MSBuild\ToolsVersions\%0.1f"
    DEVENV_BASE = r"Software\Microsoft\Windows\CurrentVersion\App Paths\devenv.exe"
else:
    VS_BASE = r"Software\Microsoft\VisualStudio\%0.1f"
    VSEXPRESS_BASE = r"Software\Microsoft\VCExpress\%0.1f"
    WINSDK_BASE = r"Software\Microsoft\Microsoft SDKs\Windows"
    NET_BASE = r"Software\Microsoft\.NETFramework"
    MSBUILD_BASE = r"Software\Microsoft\MSBuild\ToolsVersions\%0.1f"
    DEVENV_BASE = r"Software\Microsoft\Windows\CurrentVersion\App Paths\devenv.exe"


# A map keyed by get_platform() return values to values accepted by
# 'vcvarsall.bat'.  Note a cross-compile may combine these (eg, 'x86_amd64' is
# the param to cross-compile on x86 targetting amd64.)
PLAT_TO_VCVARS = {
    'win32' : 'x86',
    'win-amd64' : 'amd64',
    'win-ia64' : 'ia64',
}

class Reg:
    """Helper class to read values from the registry
    """

    def get_value(cls, path, key):
        for base in HKEYS:
            d = cls.read_values(base, path)
            if d and key in d:
                return d[key]
        raise KeyError(key)
    get_value = classmethod(get_value)

    def read_keys(cls, base, key):
        """Return list of registry keys."""
        try:
            handle = RegOpenKeyEx(base, key)
        except RegError:
            return None
        L = []
        i = 0
        while True:
            try:
                k = RegEnumKey(handle, i)
            except RegError:
                break
            L.append(k)
            i += 1
        return L
    read_keys = classmethod(read_keys)

    def read_values(cls, base, key):
        """Return dict of registry keys and values.

        All names are converted to lowercase.
        """
        try:
            handle = RegOpenKeyEx(base, key)
        except RegError:
            return None
        d = {}
        i = 0
        while True:
            try:
                name, value, type = RegEnumValue(handle, i)
            except RegError:
                break
            name = name.lower()
            d[cls.convert_mbcs(name)] = cls.convert_mbcs(value)
            i += 1
        return d
    read_values = classmethod(read_values)

    def convert_mbcs(s):
        dec = getattr(s, "decode", None)
        if dec is not None:
            try:
                s = dec("mbcs")
            except UnicodeError:
                pass
        return s
    convert_mbcs = staticmethod(convert_mbcs)

class MacroExpander:

    def __init__(self, version):
        self.macros = {}
        self.vsbase = VS_BASE % version
        self.load_macros(version)

    def set_macro(self, macro, path, key):
        self.macros["$(%s)" % macro] = Reg.get_value(path, key)

    def load_macros(self, version):
        self.set_macro("VCInstallDir", self.vsbase + r"\Setup\VC", "productdir")
        self.set_macro("VSInstallDir", self.vsbase + r"\Setup\VS", "productdir")
        self.set_macro("FrameworkDir", NET_BASE, "installroot")
        try:
            if version >= 8.0:
                self.set_macro("FrameworkSDKDir", NET_BASE,
                               "sdkinstallrootv2.0")
            else:
                raise KeyError("sdkinstallrootv2.0")
        except KeyError:
            raise DistutilsPlatformError(
            """Python was built with Visual Studio 2008;
extensions must be built with a compiler than can generate compatible binaries.
Visual Studio 2008 was not found on this system. If you have Cygwin installed,
you can try compiling with MingW32, by passing "-c mingw32" to setup.py.""")

        if version >= 9.0:
            self.set_macro("FrameworkVersion", self.vsbase, "clr version")
            self.set_macro("WindowsSdkDir", WINSDK_BASE, "currentinstallfolder")
        else:
            p = r"Software\Microsoft\NET Framework Setup\Product"
            for base in HKEYS:
                try:
                    h = RegOpenKeyEx(base, p)
                except RegError:
                    continue
                key = RegEnumKey(h, 0)
                d = Reg.get_value(base, r"%s\%s" % (p, key))
                self.macros["$(FrameworkVersion)"] = d["version"]

    def sub(self, s):
        for k, v in self.macros.items():
            s = s.replace(k, v)
        return s

def find_vcroot(version):
    vsbase = VS_BASE % version
    try:
        productdir = Reg.get_value(r"%s\Setup\VC" % vsbase,
                                   "productdir")
    except KeyError:
        productdir = None

    # trying Express edition
    if productdir is None:
        vsbase = VSEXPRESS_BASE % version
        try:
            productdir = Reg.get_value(r"%s\Setup\VC" % vsbase,
                                       "productdir")
        except KeyError:
            productdir = None
            print ("Unable to find productdir in registry")

    return productdir

def is_express(productdir):
    return not os.path.exists(os.path.join(productdir, "atlmfc", "include"))

def find_msbuild(version):
    try:
        msbuilddir = Reg.get_value(MSBUILD_BASE % version, "MSBuildToolsPath".lower())
    except KeyError:
        msbuilddir = None
        print("Unable to find msbuild in registry")
    return msbuilddir

def find_devenv():
    try:
        devenv_dir = Reg.get_value(DEVENV_BASE, "")
        devenv_dir = devenv_dir.rsplit("\\", 1 )[0];
    except KeyError:
        devenv_dir = None
        print("Unable to find devenv in registry")
    return devenv_dir

def removeDuplicates(variable):
    """Remove duplicate values of an environment variable.
    """
    oldList = variable.split(os.pathsep)
    newList = []
    for i in oldList:
        if i not in newList:
            newList.append(i)
    newVariable = os.pathsep.join(newList)
    return newVariable

def query_vcvarsall(version, arch="x86"):
    """Launch vcvarsall.bat and read the settings from its environment
    """
    vcroot = find_vcroot(version)
    vcvarsall = os.path.join(vcroot, "bin", "vcvars32.bat")
    interesting = set(("include", "lib", "libpath", "path"))
    result = {}

    if vcvarsall is None:
        raise PackagingPlatformError("Unable to find vcvarsall.bat")
    print("calling 'vcvarsall.bat %s' (version=%s)", arch, version)
    popen = subprocess.Popen('"%s" %s & set' % (vcvarsall, arch),
                             stdout=subprocess.PIPE,
                             stderr=subprocess.PIPE)

    stdout, stderr = popen.communicate()
    if popen.wait() != 0:
        raise PackagingPlatformError(stderr.decode("mbcs"))

    stdout = stdout.decode("mbcs")
    for line in stdout.split("\n"):
        line = Reg.convert_mbcs(line)
        if '=' not in line:
            continue
        line = line.strip()
        key, value = line.split('=', 1)
        key = key.lower()
        if key in interesting:
            if value.endswith(os.pathsep):
                value = value[:-1]
            result[key] = removeDuplicates(value)

    if len(result) != len(interesting):
        raise ValueError(str(list(result)))

    os.environ['lib'] = result['lib']
    os.environ['include'] = result['include']
    os.environ['PATH'] += os.pathsep + os.path.join(vcroot, "vcpackages")
    return result