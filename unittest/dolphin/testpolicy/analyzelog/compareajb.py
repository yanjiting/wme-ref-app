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



ajb_re = re.compile(r'^.*PESQ = (\d+\.\d+),'
                    r'.*PESQ-LQ = (\d+\.\d+),'
                    r'.*PESQ-LQO = (\d+\.\d+),'
                    r'.*packetlostrate = (\d+\.\d+),'
                    r'.*ulAverageJitter = (\d+\.\d+),.*')

file_re = re.compile(r'(\w*).txt')


def Test(value):
	Test.result = value
	return value



class JitterInfoStruct:
    'This class to save some jitter buffer information'


    def __init__(self):
        """
        JitterInfoStruct construction
        """
        self.filename = []
        self.startTime = []
        self.pesq = []
        self.pesqlq = []
        self.pesqlqo = []
        self.packetlostrate = []
        self.ulAverageJitter = []
        self.pesqModified = []
        self.pesqlqModified = []
        self.pesqlqoModified = []
        self.packetlostrateModified = []
        self.ulAverageJitterModified = []
        self.count = 0

        #print "JitterInfoStruct construction __init__"
        #print self.averageDelay
        #print self


class PharseJitterInfo(object):
    'this class is designed to phase audio jitter information'

    def __init__(self, numberoffile):
        """
        PharseJitterInfo construnction
        """
        self.infolist = []
        self.filecount = 0
        i = 0
        for i in range(numberoffile):
            tmpJitterInfoStruct = JitterInfoStruct()
            #print  tmpJitterInfoStruct
            self.infolist.append(tmpJitterInfoStruct)
        self.filecount = numberoffile
        self.starttime = []

    def ProcessLine(self, i, line):
        if None != Test( ajb_re.search(line)):
            self.infolist[i].startTime.append(int(self.infolist[i].count))
            self.infolist[i].pesq.append(float(Test.result.groups()[0]))
            self.infolist[i].pesqlq.append(float(Test.result.groups()[1]))
            self.infolist[i].pesqlqo.append(float(Test.result.groups()[2]))
            self.infolist[i].packetlostrate.append(float(Test.result.groups()[3]))
            self.infolist[i].ulAverageJitter.append(float(Test.result.groups()[4]))
            self.infolist[i].count += 1
        else:
            print "Not match for the ", i, "files, line:", line

    def PrintGraph(self,filename):
        if self.filecount <= 0:
            return
        figobj = plt.figure(figsize=(28,28), dpi=360)

        minimalcount = 32767
        for i in range(self.filecount):
            if minimalcount > self.infolist[i].count:
                minimalcount = self.infolist[i].count

        for i in range(minimalcount):
            self.starttime.append(i)

        for i in range(self.filecount):
            for j in range(minimalcount):
                self.infolist[i].pesqModified.append(self.infolist[i].pesq[j])
                self.infolist[i].pesqlqModified.append(self.infolist[i].pesqlq[j])
                self.infolist[i].pesqlqoModified.append(self.infolist[i].pesqlqo[j])
                self.infolist[i].packetlostrateModified.append((self.infolist[i].packetlostrate[j]))
                self.infolist[i].ulAverageJitterModified.append(self.infolist[i].ulAverageJitter[j])



        """
        ================    ===============================
        character           description
        ================    ===============================
        ``'-'``             solid line style
        ``'--'``            dashed line style
        ``'-.'``            dash-dot line style
        ``':'``             dotted line style
        ``'.'``             point marker
        ``','``             pixel marker
        ``'o'``             circle marker
        ``'v'``             triangle_down marker
        ``'^'``             triangle_up marker
        ``'<'``             triangle_left marker
        ``'>'``             triangle_right marker
        ``'1'``             tri_down marker
        ``'2'``             tri_up marker
        ``'3'``             tri_left marker
        ``'4'``             tri_right marker
        ``'s'``             square marker
        ``'p'``             pentagon marker
        ``'*'``             star marker
        ``'h'``             hexagon1 marker
        ``'H'``             hexagon2 marker
        ``'+'``             plus marker
        ``'x'``             x marker
        ``'D'``             diamond marker
        ``'d'``             thin_diamond marker
        ``'|'``             vline marker
        ``'_'``             hline marker
        ================    ===============================


        The following color abbreviations are supported:

        ==========  ========
        character   color
        ==========  ========
        'b'         blue
        'g'         green
        'r'         red
        'c'         cyan
        'm'         magenta
        'y'         yellow
        'k'         black
        'w'         white
        ==========  ========
        """
        axisLeft = None
        legendmode = ['b-+', 'g-*', 'r-d','c-o','m--.','y--','k-v','w-s' ]
        ' Print PESQ'
        axisLeft = figobj.add_subplot(5,1,1)

        for i in range(self.filecount):
            if None != Test(file_re.search(self.infolist[i].filename)):
                axisLeft.plot(self.starttime,self.infolist[i].pesqModified, legendmode[i], label= Test.result.groups()[0])
            else:
                axisLeft.plot(self.starttime,self.infolist[i].pesqModified, legendmode[i], label=self.infolist[i].filename)

        axisLeft.set_title("PESQ ",color='blue',size=18,weight='heavy')
        axisLeft.set_xlabel("Received Time")
        axisLeft.set_ylabel("MOS")
        axisLeft.grid(b=True,which='major',axis='both',color="black")
        axisLeft.tick_params(axis='both',labelsize=8)



        plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
           ncol=2, borderaxespad=0.)


        ' Print PESQ-LQ'
        axisLeft = figobj.add_subplot(5,1,2)
        for i in range(self.filecount):
            if None != Test(file_re.search(self.infolist[i].filename)):
                axisLeft.plot(self.starttime,self.infolist[i].pesqlqModified, legendmode[i], label= Test.result.groups()[0])
            else:
                axisLeft.plot(self.starttime,self.infolist[i].pesqlqModified, legendmode[i], label=self.infolist[i].filename)

        axisLeft.set_title("PESQ -LQ ",color='blue',size=18,weight='heavy')
        axisLeft.set_xlabel("Received Time")
        axisLeft.set_ylabel("MOS")
        axisLeft.grid(b=True,which='major',axis='both',color="black")
        axisLeft.tick_params(axis='both',labelsize=8)


        plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
           ncol=2, borderaxespad=0.)


        ' Print PESQ-LQO'
        axisLeft = figobj.add_subplot(5,1,3)
        for i in range(self.filecount):
            if None != Test(file_re.search(self.infolist[i].filename)):
                axisLeft.plot(self.starttime,self.infolist[i].pesqlqoModified, legendmode[i], label= Test.result.groups()[0])
            else:
                axisLeft.plot(self.starttime,self.infolist[i].pesqlqoModified, legendmode[i], label=self.infolist[i].filename)

        axisLeft.set_title("PESQ -LQO ",color='blue',size=18,weight='heavy')
        axisLeft.set_xlabel("Received Time")
        axisLeft.set_ylabel("MOS")
        axisLeft.grid(b=True,which='major',axis='both',color="black")
        axisLeft.tick_params(axis='both',labelsize=8)

        plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
           ncol=2, borderaxespad=0.)

        ' Print Delay'
        axisLeft = figobj.add_subplot(5,1,4)
        for i in range(self.filecount):
            if None != Test(file_re.search(self.infolist[i].filename)):
                axisLeft.plot(self.starttime,self.infolist[i].ulAverageJitterModified, legendmode[i], label= Test.result.groups()[0])
            else:
                axisLeft.plot(self.starttime,self.infolist[i].ulAverageJitterModified, legendmode[i], label=self.infolist[i].filename)

        axisLeft.set_title("Delay ",color='blue',size=18,weight='heavy')
        axisLeft.set_xlabel("Received Time")
        axisLeft.set_ylabel("Delay(ms)")
        axisLeft.grid(b=True,which='major',axis='both',color="black")
        axisLeft.tick_params(axis='both',labelsize=8)


        plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
           ncol=2, borderaxespad=0.)


        ' Print Lost'
        axisLeft = figobj.add_subplot(5,1,5)
        for i in range(self.filecount):
            if None != Test(file_re.search(self.infolist[i].filename)):
                axisLeft.plot(self.starttime,self.infolist[i].packetlostrateModified, legendmode[i], label= Test.result.groups()[0])
            else:
                axisLeft.plot(self.starttime,self.infolist[i].packetlostrateModified, legendmode[i], label=self.infolist[i].filename)

        axisLeft.set_title("Packet Los ",color='blue',size=18,weight='heavy')
        axisLeft.set_xlabel("Received Time")
        axisLeft.set_ylabel("Packet Lost(%)")
        axisLeft.grid(b=True,which='major',axis='both',color="black")
        axisLeft.tick_params(axis='both',labelsize=8)


        plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
           ncol=2, borderaxespad=0.)


        plt.savefig(filename, bbox_inches = 0)


    #os.mkdir('testdir')


    def ProcessFile(self,i, filename):
        result = re.search(".txt", filename)
        if None != result:
            try:
                self.infolist[i].filename = filename
                originfile = open(filename, 'rb')
                for eachLine in originfile:
                    self.ProcessLine(i,eachLine)
                originfile.close()
            except IOError, e:
                print 'file open error:', e






if __name__=="__main__":
    print "Usage: compareajb.py txt ... txtn result"
    argc = len(sys.argv)
    print "Input paramters are:"
    print "Input " , argc , "parameters"
    print "Paramters are " , sys.argv
    if argc < 3:
        print "Usage: comparejitter.py cvs1 ... csvn result"
        sys.exit(2)
    if argc > 10:
        print  "It support only max compare 8 jitter info"
        sys.exit(1)

    fileProcess = PharseJitterInfo(argc - 1 - 1) # process file number
    for i in range(argc - 1 -1):
        fileProcess.ProcessFile(i,sys.argv[i+1])
        print  'End of processing file ', sys.argv[i + 1]
    fileProcess.PrintGraph(sys.argv[argc - 1])

    del fileProcess







