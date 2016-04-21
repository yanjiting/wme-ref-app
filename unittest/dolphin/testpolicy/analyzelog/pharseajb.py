#!/usr/bin/env python

'Analyze jitter buffer status and draw picture'
m_import_err = ""
import re
import os
import sys
import getopt

try:
    import matplotlib.pyplot as plt
except:
    m_import_err += "Can't import matplotlib.pyplot"

print(m_import_err)

"""
'AE Info: [AudioEngine] Info: [AJB buffer]:CAudioJitterBuffer::OnTimer(),
BuffIno, ssrc :15520, max BS:6, min BS:1, Average BS:3, Policy, m_ulAverageDelay:62,
total received 146, packets, and drop 0, E-MOS: PESQ = 4.145506, PESQ-LQ = 4.145506,
PESQ-LQO = 4.145506, packetlostrate = 0.000000, ulAverageJitter = 162.000000, ulTotoalCount = 146,
ulJitterBufferLost = 0, lNetworkLost = 0,this=0x15d9f4b0'
"""
ajb_re = re.compile(r'^.*PESQ = (\d+\.\d+),'
                    r'.*PESQ-LQ = (\d+\.\d+),'
                    r'.*PESQ-LQO = (\d+\.\d+),'
                    r'.*packetlostrate = (\d+\.\d+),'
                    r'.*ulAverageJitter = (\d+\.\d+),.*')



def Test(value):
	Test.result = value
	return value




class PharseJitterInfo(object):
    'this class is designed to phase audio jitter information'
    startTime = []
    pesq = []
    pesqlq = []
    pesqlqo = []
    packetlostrate = []
    ulAverageJitter = []
    count = 0
    def __init__(self):
        """
        construnction
        """
    def CleanAllData(self):

        self.startTime = []
        self.pesq = []
        self.pesqlq = []
        self.pesqlqo = []
        self.packetlostrate = []
        self.ulAverageJitter = []
        count = 0

    def ProcessLine(self, line):
        if None != Test( ajb_re.search(line)):
            self.startTime.append(int(self.count))
            self.pesq.append(float(Test.result.groups()[0]))
            self.pesqlq.append(float(Test.result.groups()[1]))
            self.pesqlqo.append(float(Test.result.groups()[2]))
            self.packetlostrate.append(float(Test.result.groups()[3]))
            self.ulAverageJitter.append(float(Test.result.groups()[4]))
            self.count += 1

    def PrintGraph(self,filename):
        figobj = plt.figure(figsize=(28,14), dpi=1000)
        axisLeft = None
        axisLeft = figobj.add_subplot(3,1,1)
        axisLeft.set_title("PESQ Compare")
        axisLeft.set_xlabel("Received Time")
        axisLeft.set_ylabel("MOS Value")
        axisLeft.plot(self.startTime,self.pesq,'r')
        axisLeft.plot(self.startTime,self.pesqlq,'g')
        axisLeft.plot(self.startTime,self.pesqlqo,'b')

        axisLeft = figobj.add_subplot(3,1,2)
        axisLeft.set_title("Average Delay")
        axisLeft.set_xlabel("Received Time")
        axisLeft.set_ylabel("Average Delay(ms)")
        axisLeft.plot(self.startTime,self.ulAverageJitter)

        axisLeft = figobj.add_subplot(3,1,3)
        axisLeft.set_title("Packet Lost")
        axisLeft.set_xlabel("Received Time")
        axisLeft.set_ylabel("Packet loss %")
        axisLeft.plot(self.startTime,self.packetlostrate)

        plt.savefig(filename, bbox_inches = 0)


    #os.mkdir('testdir')


    def ProcessFile(self,filename, imagefile):
        result = re.search("txt", filename)
        if None != result:
            try:
                print(filename)
                self.CleanAllData()
                originfile = open(filename, 'rb')
                for eachLine in originfile:
                    self.ProcessLine(eachLine)
                originfile.close()
                self.PrintGraph(imagefile)
                print ("End of process " + filename)
            except IOError, e:
                print ("file open error:" + e)






if __name__=="__main__":
    argc = len(sys.argv)
    print sys.argv
    try:
	opts, args = getopt.getopt(sys.argv[1:], "hi:o:", ["help","input=","output="])
    except getopt.GetoptError as err:
	print str(err)
	print "Usage: pharseabj.py -i <cvs_dir> -o <out_dir>"
	sys.exit(2)
    input_dir = None
    out_dir = None
	
    for opt, arg in opts:
  	if opt == "-h":
   	    print "Usage: pharseabj.py -i <cvs_dir> -o <out_dir>"
	    sys.exit()
	elif opt in ("-i", "--input"):
	   input_dir = arg
	elif opt in ("-o", "--output"):
	   out_dir = arg
	if input_dir == None:
		print "Usage: pharseabj.py -i <cvs_dir> -o <out_dir>"
		sys.exit()

    print input_dir
    print out_dir
    currentpy = sys.argv[0]
    
    fileinfomations = os.listdir(input_dir)
    if os.path.exists(out_dir):
        print "Alread exist result folder"
    else:     
        os.mkdir(out_dir)
    os.chdir(out_dir)
    cwd = os.getcwd()
    print cwd
    
    print "*** current cwd's files  ***"
    print  fileinfomations

    'test re'
    testprocess = PharseJitterInfo()
    Testline = 'AE Info: [AudioEngine] Info: [AJB buffer]:CAudioJitterBuffer::OnTimer(), BuffIno, ssrc :15520, ' \
               'max BS:6, min BS:1, Average BS:3, Policy, m_ulAverageDelay:62, total received 146, packets, ' \
               'and drop 0, E-MOS: PESQ = 4.145506, PESQ-LQ = 4.145506, PESQ-LQO = 4.145506, ' \
               'packetlostrate = 0.000000, ulAverageJitter = 162.000000, ulTotoalCount = 146, ulJitterBufferLost = 0, ' \
               'lNetworkLost = 0,this=0x15d9f4b0'
    testprocess.ProcessLine(Testline)

    for i, filename in enumerate(fileinfomations):
        fileProcess = PharseJitterInfo()
        fullpath = os.path.join(os.pardir, filename)
        imagefilename = filename+".jpeg"
        imagefullpath = os.path.join(cwd, imagefilename)
        fileProcess.ProcessFile(fullpath, imagefullpath)
        del fileProcess







