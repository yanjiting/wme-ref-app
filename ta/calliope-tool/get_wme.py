import os
import time
import sys
import math
import json
import argparse
import ssl
import tarfile
import errno
import shutil
import glob
import zipfile
import base64

try:
    # For Python 3.0 and later
    from urllib.request import urlopen
    from urllib.error import HTTPError
    from urllib.error import URLError
    from urllib.request import Request
    from urllib.parse import urlencode
except ImportError:
    # Fall back to Python 2's urllib2
    from urllib2 import urlopen
    from urllib2 import HTTPError
    from urllib2 import URLError
    from urllib2 import Request
    from urllib import urlencode
    #print("Use python 2.7.x")
    
job_name = ""
pkg_path = ""
dst_path = "."
parser = argparse.ArgumentParser()
parser.add_argument("-j", "--job", help="The job type of the build", required=True, choices=["win32", "mac32", "mac64", "android", "ios"])
parser.add_argument("-d", "--dest", help="The destination dir of the package", required=False, default=".")
parser.add_argument("-b", "--branch", help="The branch of the release", required=False, default="master")
parser.add_argument("-n", "--number", help="Manual build number", type=int, required=False, default=0)
parser.add_argument("-t", "--test", help="get the latest build number without download", dest='test', action='store_true', default=False)
parser.add_argument("-u", "--update", help="update to the latest WME and output a meta file.", dest='update', action='store_true', default=False)
parser.add_argument("-l", "--load", help="load wme from meta file.", dest='load', action='store_true', default=False)
parser.add_argument("-f", "--force", help="force to load from file server.", dest='force', action='store_true', default=False)
parser.add_argument("-p", "--pipeline", help="download from pipeline.", dest='pipeline', action='store_true', default=False)
parser.add_argument("--token", help="The API token to access jenkins.", dest='token', required=True, default="")

context = None
try:
    context = ssl._create_unverified_context()
except:
    pass

args = parser.parse_args()
dst_path = args.dest
if(args.job == "win32"):
    job_name = "WME-GIT-BUILDSDK-WIN"
    pkg_path = "windows/wme4train_win32.zip"
elif(args.job == "mac32"):
    job_name = "WME-GIT-BUILDSDK-MAC32"
    pkg_path = "mac/wme4train_macos.tar.gz"
elif(args.job == "mac64"):
    job_name = "WME-GIT-BUILDSDK-MAC64"
    pkg_path = "mac/wme4train_macos.tar.gz"
elif(args.job == "android"):
    job_name = "WME-GIT-BUILDSDK-ANDROID"
    pkg_path = "android/wme4train_android.tar.gz"
elif(args.job == "ios"):
    job_name = "WME-GIT-BUILDSDK-IOS"
    pkg_path = "ios/MediaSDK_Demo_IOS.tar.gz"
WME_JENKINS_BASE = "https://wme-jenkins.cisco.com:8443/"
WME_JENKINS = WME_JENKINS_BASE + "job/"
DAILY_JOB = "WmeDailyBuild-template"
DAILY_PIPELINE_JOB = "Pipeline-Daily"
BUILD_JOBS = ["WME-GIT-BUILDSDK-WIN", "WME-GIT-BUILDSDK-MAC32", "WME-GIT-BUILDSDK-MAC64", "WME-GIT-BUILDSDK-ANDROID"]

def open_url_basic(url, token):
    req = Request(url)
    auth_encoded = base64.encodestring(token.encode('utf-8'))[:-1]
    req.add_header('Authorization', 'Basic %s' % auth_encoded.decode('utf-8'))
    global context
    if(context): 
        response = urlopen(req, context = context)
    else:
        response = urlopen(req)
    return response

def open_url(url, token):
    json_result = open_url_basic(url, token).read()
    result = json.loads(json_result.decode('utf-8'))
    return result

def check_code_base(url, branch_name):
    """
    Check if the code base is branch or a particular branch name matched branch_name
    """
    result = open_url(WME_JENKINS_BASE + url + "/api/json", args.token)
    actions = result["actions"]
    params = None
    for action in actions:
        if("parameters" in action):
            params = action["parameters"]
            break
    branch_matched = False
    for param in params:
        if(param["name"] == "wme_git_branch" and param["value"] == branch_name):
            branch_matched = True
            break
    return branch_matched
    
def get_latest_avail_pipeline(test = False, branch_name = "master"):
    """
    get_latest_avail_pipeline enumerate the daily pipeline job and try to find the latest successful daily build matched the branch.
    """
    result = open_url(WME_JENKINS + DAILY_PIPELINE_JOB + "/api/json", args.token)
    builds = result["builds"]
    build_num = 0
    for build in builds:
        found = False
        for sub in build["subBuilds"]:
            if sub["phaseName"] == "Build" and sub["result"] == "SUCCESS":
                if(check_code_base(sub["url"], branch_name)):
                    found = True
                    build_num = build["number"]
                    break
        if(found):
            break
    return build_num
    
def get_latest_avail(test = False):
    """
    get_latest_avail enumerate the daily job list and try to find the latest successful daily build matched the branch.
    """
    build_num = 0
    result = open_url(WME_JENKINS + DAILY_JOB + "/api/json", args.token)
    builds = result["builds"]
    for build in builds:
        findBuilds = 0
        buildNum = build["number"]
        for sub in build["subBuilds"]:
            if sub["jobName"] in BUILD_JOBS and sub["result"] == "SUCCESS":
                findBuilds += 1
        if findBuilds == len(BUILD_JOBS):
            build_num = get_downlouad_url(buildNum)
            if(build_num == 0):
                continue
            else:
                if(test):
                    return buildNum
                print("daily build number is: %d " % buildNum)
                break;
    return build_num
            
def get_downlouad_url(parent_num):
    """
    Get the download link from the particular daily build. If the branch doesn't match, it will return 0.
    """
    build_num = 0
    if parent_num <= 0: return 0

    result = open_url("%s%s/%d/api/json" % (WME_JENKINS, DAILY_JOB, parent_num), args.token)
    actions = result["actions"]
    params = None
    for action in actions:
        if("parameters" in action):
            params = action["parameters"]
            break
    #print(params)
    branch_matched = False
    for param in params:
        if(param["name"] == "wme_git_branch" and param["value"] == args.branch):
            branch_matched = True
            break
    if(not branch_matched):
        return 0
    
    subBuilds = result["subBuilds"]
    for sub_build in subBuilds:
        if(sub_build["jobName"] == job_name):
            #print(sub_build)
            build_num = sub_build["buildNumber"]
            break
    return int(build_num)
    
def downloadChunks(url, dest_dir = "."):
    """Helper to download large files
       this file will go to dest_dir directory, the default is current folder
       the file will also be downloaded
       in chunks and print out how much remains, speed(KBps) and time consumed.
    """

    baseFile = os.path.basename(url)
    #move the file to a more uniq path
    #os.umask(0002)
    try:
        file = os.path.join(dest_dir, baseFile)

        req = open_url_basic(url, args.token)
        total_size = int(req.info().get('Content-Length').strip())
        downloaded = 0
        CHUNK = 16 * 10240
        time_spent = time.time()
        time_last = time_spent
        with open(file, 'wb') as fp:
            speed = 0
            while True:
                chunk = req.read(CHUNK)
                downloaded += len(chunk)
                if(time.time() > time_last):
                    speed = float(CHUNK) / ((time.time() - time_last) * 1000.0)
                sys.stdout.write("Downloading: %d/%d (%.2f KBps)-- %.2f seconds            \r" 
                                % (downloaded, total_size, speed, time.time() - time_spent))
                time_last = time.time()
                if not chunk:
                    speed = 0
                    if(time.time() > time_spent):
                        speed = float(downloaded) / ((time.time() - time_spent) * 1000.0)
                    sys.stdout.write("\r\n---------\r\nCompleted in: %.2f seconds with %.2f KBps\r" 
                                % (time.time() - time_spent, speed))
                    break
                fp.write(chunk)
    except HTTPError as e:
        print("HTTP Error:",e.code , url)
        return False
    except URLError as e:
        print("URL Error:",e.reason , url)
        return False

    return file


def make_sure_path_exists(path):
    try:
        os.makedirs(path)
    except OSError as exception:
        if exception.errno != errno.EEXIST:
            raise

def copy_lib_files(srcDir, dstDir):
    profiles=["Debug-iphoneos", "Debug-iphonesimulator", "Release-iphoneos", "Release-iphonesimulator"]
    for profile in profiles:
        make_sure_path_exists(os.path.join(dstDir, profile))
        for filename in glob.glob(os.path.join(os.path.join(srcDir, profile), '*.a')):
            shutil.move(filename, os.path.join(dstDir, profile))

def copy_include_files(srcDir, dstDir):
    for filepath in glob.glob(os.path.join(srcDir, '*')):
        filename = os.path.basename(filepath)
        shutil.copytree(filepath, os.path.join(dstDir, filename))
        
def extract_ios(srcFile, destPath):
    tfile = tarfile.open(srcFile, 'r:gz')
    tfile.extractall("/tmp")
    
    libPath = os.path.join(destPath, "distribution/ios")
    make_sure_path_exists(libPath)
    copy_lib_files("/tmp/MediaSDK_Demo_IOS/sdk/libs", libPath)
    copy_lib_files("/tmp/MediaSDK_Demo_IOS/external/libs", libPath)
    
    try:
        copy_include_files("/tmp/MediaSDK_Demo_IOS/wme4train", destPath)
    except Exception as e:
        print(e.message)
        
    shutil.rmtree("/tmp/MediaSDK_Demo_IOS")

def extract_zip(srcFile, destPath):
    with zipfile.ZipFile(srcFile, "r") as z:
        z.extractall(destPath)
        
def extract_tar(srcFile, destPath):
    make_sure_path_exists(destPath)
    tfile = tarfile.open(srcFile, 'r:gz')
    tfile.extractall(destPath)

def write_wme_meta(build_num, dst_path = "./"):
    """
    generate the wme.meta for later access.
    """
    result = open_url("%s%s/%d/api/json" % (WME_JENKINS, job_name, build_num), args.token)
    actions = result["actions"]
    meta_data = {}
    meta_data["build"] = build_num
    meta_data["job"] = job_name
    for action in actions:
        if("parameters" in action):
            params = action["parameters"]
            break
    for param in params:
        if(param["name"] == "git_commit_revision"):
            meta_data["revision"] = param["value"]
        if(param["name"] == "fast_mode"):
            meta_data["fast_mode"] = param["value"]
        if(param["name"] == "parent_project"):
            meta_data["parent"] = param["value"]
        if(param["name"] == "parent_build_number"):
            meta_data["parent_build"] = param["value"]

    with open(os.path.join(dst_path, 'wme.meta'), 'w') as f:
        json.dump(meta_data, f, indent=2)

def get_most_matched_release(build_num):
    """
    If the build number in the wme.meta was not saved in the pipeline anymore, 
    client team need to get the most close WME release build which is always available
    but that still could cause the build failure, then client team need to update wme manulaly to pass the build.
    """
    result = open_url("%sWME-Release/api/json" % (WME_JENKINS), args.token)
    builds = result["builds"]
    matched = 0
    for build in builds:
        subBuilds = build["subBuilds"]
        for sub in subBuilds:
            if(sub["jobName"] == job_name):
                itBuild = sub["buildNumber"]
                if(abs(matched - build_num) > abs(itBuild - build_num)):
                    matched = itBuild
    return matched

def load_build_wme_meta(filePath = "./wme.meta"):
    try:
        with open(filePath) as data_file:    
            data = json.load(data_file)
    except Exception as e:
        print("%s not found." % filePath)
        return 0
    
    if("build" in data):
        return data["build"]
    return 0

def cache_matched(dst_path, build_num):
    cached = load_build_wme_meta(os.path.join(dst_path, "wme.meta"))
    return cached == build_num

def format_download_url(build_num, job_type):
    global pkg_path
    global WME_JENKINS
    global job_name
    
    download_url = ""
    if(job_type == "daily"):
        download_url = WME_JENKINS + job_name + "/" + str(build_num) + "/artifact/build-all/wme-daily/build/" + pkg_path
    elif(job_type == "release"):
        download_url = WME_JENKINS + job_name + "/" + str(build_num) + "/artifact/build-all/wme-release/build/" + pkg_path
    elif(job_type == "daily-pipeline"):
         download_url = WME_JENKINS + DAILY_PIPELINE_JOB + "/" + str(build_num) + "/artifact/artifacts/" + os.path.basename(pkg_path)
    else:
         pass
    return download_url
         
if __name__ == '__main__':
    def main(args):
        if(args.load):
            build_num = load_build_wme_meta()
            if(build_num == 0):
                print("There is no wme.meta file to load.")
                exit(9)
                
            if(not args.force and cache_matched(args.dest, build_num)):
                print("Load: Your cache is up to date.")
                exit(0)
            
            print("load wme.meta: %d" % build_num)
        else:
            if(args.test):
                if(args.pipeline):
                    build_num = get_latest_avail_pipeline(True, args.branch)
                else:
                    build_num = get_latest_avail(True)
                print(build_num)
                exit(0)
            if(args.number == 0):
                if(args.pipeline):
                    build_num = get_latest_avail_pipeline(False, args.branch)
                else:
                    build_num = get_latest_avail()
            else:
                if(args.pipeline):
                    build_num = args.number
                else:
                    build_num = get_downlouad_url(args.number)
                
        print("job build number is: %d" % build_num)
        if(build_num == 0):
            print("Cannot get the download url")
            exit(2)

        if(not args.force and args.update and cache_matched(args.dest, build_num)):
            print("Update: Your cache is update to date.")
            exit(0)
        
        job_type = "daily"
        if(args.pipeline):
            job_type = "daily-pipeline"
        download_url = format_download_url(build_num, job_type)
        result = downloadChunks(download_url, dst_path)
        if(not result):
            print("download %s failed." % download_url)
            if(args.load):
                matched_num = get_most_matched_release(build_num)
                print("download matched: %d" % matched_num)
                if(matched_num == 0):
                    exit(4)
                build_num = matched_num    
                download_url = format_download_url(matched_num, pkg_path)
                result = downloadChunks(download_url, dst_path)
                if(not result):
                    print("download matched release failed too.")
                    exit(5)
            else:
                exit(3)

        print("\r\nFile has been downloaded to: %s" % result)

        #Try to delete the target dir if it exists.
        target_dir = os.path.join(dst_path, "wme")
        try:
            shutil.rmtree(target_dir)
        except:
            pass
        
        if(args.job == "ios"):
            print("\r\nStart to extract wme for iOS client")
            extract_ios(result, target_dir)
            print("\r\nDone!")

        if(args.update or args.load):
            if(args.job == "win32"):
                extract_zip(result, target_dir)
            elif(args.job != "ios"):
                extract_tar(result, target_dir)
                
            write_wme_meta(build_num)
            shutil.copy("./wme.meta", target_dir)
    
    main(args)
    exit(0)
