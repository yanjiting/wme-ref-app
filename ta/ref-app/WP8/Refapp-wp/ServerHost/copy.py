import sys
import os
import shutil
import tempfile
import errno
import glob

#scripts_dir=os.path.dirname(os.path.abspath(__file__))
#src_path = os.path.join(scripts_dir, "../../../../../mediaengine/bin/WP8/")
#dst_path = os.path.join(scripts_dir, "../Refapp-wp/")

src_path = '../../../../../mediaengine/bin/WP8/'
dst_path = '../Refapp-wp/'

def copy_dlls(src, dst, filter = "*.dll"):
    filter = os.path.join(src, filter)
    print("copy from %s to %s..." % (filter, dst))
    for file in glob.glob(filter):
        print("  " + file)
        shutil.copy2(file, dst)

if __name__ == "__main__":
    args = sys.argv[1:]
    configuration = 'Release'
    #print args
    if len(args) == 0:
        configuration = 'Release/'
    else:
        if (args[0].lower()) == 'debug':
            configuration = "Debug/"
        else:
            configuration = 'Release/'
    src_path += configuration

    copy_dlls(src_path, dst_path)
    src_path = '../../../../../vendor/openssl/WP8/bin'
    copy_dlls(src_path, dst_path)

    src_path = '../../../../../vendor/openh264/libs/wp8/' + configuration
    copy_dlls(src_path, dst_path)

    src_path = '../ARM/' + configuration + 'VoipBackEndProxyStub/'
    copy_dlls(src_path, dst_path)

    src_path = '../ARM/' + configuration + 'ServerHost/'
    copy_dlls(src_path, dst_path, "*.exe")