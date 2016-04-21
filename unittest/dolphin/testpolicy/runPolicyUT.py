#!/usr/bin/env python

'Analyze Jitter buffer dump cvs, and plot result'
m_import_err = ""
import re
import os
import sys
import getopt
import fnmatch
import subprocess


try:
    import matplotlib.pyplot as plt
except:
    m_import_err += "Can't import matplotlib.pyplot"

print m_import_err

cvs_dir = None
out_dir = None
cwd = None

filename_re = re.compile(r'^(.*).csv')
def Test(value):
	Test.result = value
	return value

def RunCMD(cmdwithpara):
    os.system(cmdwithpara)

def ProcessFile(filename, processnumber, resultfile):
    try:
        print "Start processing file: " , filename
        for i in range(processnumber):
            exefilename = r'./policyUT'
            cmdwithpara =  exefilename + ' ' + filename + ' ' + resultfile[i] + ' ' + "%d" %i
            #print cmdwithpara
            RunCMD(cmdwithpara)
        print "Stop processsing file: ", filename
    except IOError, e:
        print 'file open error:', e




def Test(value):
	Test.result = value
	return value



if __name__=="__main__":
    argc = len(sys.argv)
    print sys.argv
    try:
	    opts, args = getopt.getopt(sys.argv[1:], "hi:o:", ["help","input=","output="])
    except getopt.GetoptError as err:
	    print str(err)
	    print "Usage: runPolicyUT.py -i <cvs_dir> -o <out_dir>"
	    sys.exit(2)


    for opt, arg in opts:
        if opt == "-h":
            print "Usage runPoliyUt.py -i <csv_dir> -o <out_dir>"
            sys.exit(0)
        elif opt in ("-i", "--input"):
            cvs_dir = arg
        elif opt in ("-o", "--output"):
            out_dir = arg

	if cvs_dir == None:
		print "Usage: runPolicyUT.py -i <cvs_dir> -o <out_dir>"
		sys.exit()

    print "Input directory is ", cvs_dir
    print "Output directory is ", out_dir
    currentpy = sys.argv[0]
    
    fileinfomations = os.listdir(cvs_dir)
    if os.path.exists(out_dir):
        print "Alread exist result folder"
        os.system("rm -rf out_dir")
    else:     
        os.mkdir(out_dir)
    cwd = os.getcwd()
    print cwd
    
    print "*** current cwd's files  ***"
    print  fileinfomations
    for i, filename in enumerate(fileinfomations):
        Test( fnmatch.fnmatch(filename, '*.csv'))
        if False != Test.result:
            filenameraw = []
            Test(filename_re.search(filename))
            filenameraw = Test.result.groups()[0]
            OrginFileName = os.path.join(cvs_dir, filename)
            TestfileresultName = []
            tmpfilenamewme = filenameraw + "_wme.csv"
            TestfileresultName.append(os.path.join(out_dir, tmpfilenamewme))  # for mode 0, this is according to the UT tools

            tmpfilenametrain = filenameraw + "_train.csv"
            TestfileresultName.append(os.path.join(out_dir, tmpfilenametrain))  # for mode 0, this is according to the UT tools

            tmpfilenamedolphin10 = filenameraw + "_dolphin10.csv"
            TestfileresultName.append(os.path.join(out_dir, tmpfilenamedolphin10))  # for mode 0, this is according to the UT tools

            tmpfilenamewmemodified = filenameraw + "_wme_Origin.csv"
            TestfileresultName.append(os.path.join(out_dir, tmpfilenamewmemodified))  # for mode 0, this is according to the UT tools

            ProcessFile(OrginFileName, 4, TestfileresultName)

            TestfileresultName.append(os.path.join(out_dir, filenameraw + ".png"))

            os.system("python comparejitter.py %s %s %s %s %s"%(TestfileresultName[0], TestfileresultName[1],TestfileresultName[2], TestfileresultName[3],TestfileresultName[4]))







