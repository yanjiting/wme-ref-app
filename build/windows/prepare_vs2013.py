#!/usr/bin/env python
import os, sys, shutil
import msvccompiler

current_dir = os.path.dirname(__file__)
root_dir = os.path.abspath(os.path.join(os.path.dirname(__file__), "../../"))

product_dir = msvccompiler.find_vcroot(12)
print(product_dir)
if(not product_dir):
    print("Cannot find VS2013 in your machine, please download express version from: http://www.visualstudio.com/zh-cn/downloads/download-visual-studio-vs#DownloadFamilies_2")
    exit(2)

if(msvccompiler.is_express(product_dir)):    
    print("Your VS2013 is express version, so need to hack it with a resource header file from MFC.")
    shutil.copy2(os.path.join(current_dir, "tools", "afxres.h"), os.path.join(product_dir, "include"))
shutil.copy2(os.path.join(current_dir, "tools", "nasm.exe"), os.path.join(product_dir, "bin"))