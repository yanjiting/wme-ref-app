#!/usr/bin/env python

'Analyze Jitter buffer dump cvs, and plot result'
m_import_err = ""
import re
import os
import sys
import getopt

try:
    import matplotlib.pyplot as plt
except:
    m_import_err += "Can't import matplotlib.pyplot"

print m_import_err




ajb_re = re.compile(r'^(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+)\n')



def Test(value):
	Test.result = value
	return value


"""
     sprintf(writedata, "%u,%u,%u,%u,%u,%u,%d,%d\n",
     resultData.ssrc, resultData.samplesPer10MS,
     resultData.receivedTime, resultData.rtpSequce,
     resultData.rtpTimestamp, resultData.playTime,
     resultData.averageDelay, resultData.delayedPlay);
"""

class PharseJitterInfo(object):
    'this class is designed to phase audio jitter information'
    ssrc = []
    samples10ms = []
    receivedtime = []
    rtpSequce = []
    rtpTimestamp = []
    playTime = []
    averageDelay = []
    delayedPlay = []
    StatTime = [] #for every 1000ms
    StatReceivedPacket = []
    StatAverageDelay = []
    StatDisorder = []

    def __init__(self):
        """
        construnction
        """
    def CleanAllData(self):

        self.ssrc = []
        self.samples10ms = []
        self.receivedtime = []
        self.rtpSequce = []
        self.rtpTimestamp = []
        self.playTime = []
        self.averageDelay = []
        self.delayedPlay = []
        self.StatTime = [] #for every 1000ms
        self.StatReceivedPacket = []
        self.StatAverageDelay = []
        self.StatDisorder = []
    def ProcessLine(self, line):
        if None != Test( ajb_re.search(line)):
            self.ssrc.append(int(Test.result.groups()[0]))
            self.samples10ms.append(int(Test.result.groups()[1]))
            self.receivedtime.append(int(Test.result.groups()[2]))
            self.rtpSequce.append(int(Test.result.groups()[3]))
            self.rtpTimestamp.append(int(Test.result.groups()[4]))
            self.playTime.append(int(Test.result.groups()[5]))
            self.averageDelay.append(int(Test.result.groups()[6]))
            self.delayedPlay.append(int(Test.result.groups()[7]))

    def GetStat(self):
        lenofReceivetime = len(self.receivedtime)
        i = 0
        receivedpkt = 0
        reftime = 0
        statscount = 0
        while i < lenofReceivetime:
            if i == 0:
                statscount = 0;
                self.StatTime.append( self.receivedtime[i] + statscount) ## save first received time
                self.StatAverageDelay.append(self.averageDelay[i])
                self.StatReceivedPacket.append(0)
                receivedpkt += 1
                reftime = self.receivedtime[i];
            else:
                if (self.receivedtime[i] - self.StatTime[statscount]) <= 1000:
                    receivedpkt += 1
                else:
                    statscount += 1
                    reftime += 1000
                    self.StatReceivedPacket.append(receivedpkt) # update received account
                    receivedpkt = 0                                 # reset to zero after every computation
                    self.StatAverageDelay.append( self.averageDelay[i])
                    self.StatTime.append(reftime)
                    while (self.receivedtime[i] > (self.StatTime[statscount] + 1000)):
                        statscount += 1
                        reftime += 1000
                        self.StatTime.append(reftime)  # if received time changes too long, it need change time and average dealy
                        self.StatAverageDelay.append( self.averageDelay[i])    # keep origin delay
                        self.StatReceivedPacket.append(0)       # change received

            i += 1

    def PrintGraph(self,filename):
        figobj = plt.figure(figsize=(28,14), dpi=1000)
        axisLeft = None
        axisLeft = figobj.add_subplot(2,1,1)
        axisLeft.set_title("Received Package Number")
        axisLeft.set_xlabel("Received Time")
        axisLeft.set_ylabel("Received Package")
        axisLeft.plot(self.StatTime,self.StatReceivedPacket)

        axisLeft = figobj.add_subplot(2,1,2)
        axisLeft.set_title("Average Delay")
        axisLeft.set_xlabel("Received Time")
        axisLeft.set_ylabel("Average Delat")
        axisLeft.plot(self.StatTime,self.StatAverageDelay)

        plt.savefig(filename, bbox_inches = 0)


    #os.mkdir('testdir')


    def ProcessFile(self,filename, imagefile):
        result = re.search("csv", filename)
        if None != result:
            try:
                print filename
                self.CleanAllData()
                originfile = open(filename, 'rb')
                for eachLine in originfile:
                    self.ProcessLine(eachLine)
                originfile.close()
                self.GetStat()
                self.PrintGraph(imagefile)
                print "End of process ", filename
            except IOError, e:
                print 'file open error:', e






if __name__=="__main__":
    argc = len(sys.argv)
    print sys.argv
    try:
	opts, args = getopt.getopt(sys.argv[1:], "hi:o:", ["help","input=","output="])
    except getopt.GetoptError as err:
	print str(err)
	print "Usage: AnalyzeJitterinformation.py -i <cvs_dir> -o <out_dir>"
	sys.exit(2)
    cvs_dir = None
    out_dir = None
	
    for opt, arg in opts:
  	if opt == "-h":
   	    print "Usage: AnalyzeJitterinformation.py -i <cvs_dir> -o <out_dir>"
	    sys.exit()
	elif opt in ("-i", "--input"):
	   cvs_dir = arg
	elif opt in ("-o", "--output"):
	   out_dir = arg
	if cvs_dir == None:
		print "Usage: AnalyzeJitterinformation.py -i <cvs_dir> -o <out_dir>"
		sys.exit()

    print cvs_dir
    print out_dir
    currentpy = sys.argv[0]
    
    fileinfomations = os.listdir(cvs_dir)
    if os.path.exists(out_dir):
        print "Alread exist result folder"
    else:     
        os.mkdir(out_dir)
    os.chdir(out_dir)
    cwd = os.getcwd()
    print cwd
    
    print "*** current cwd's files  ***"
    print  fileinfomations
    for i, filename in enumerate(fileinfomations):
        fileProcess = PharseJitterInfo()
        fullpath = os.path.join(os.pardir, filename)
        imagefilename = filename+".jpeg"
        imagefullpath = os.path.join(cwd, imagefilename)
        fileProcess.ProcessFile(fullpath, imagefullpath)
        del fileProcess







