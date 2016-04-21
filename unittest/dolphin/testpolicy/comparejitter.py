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




ajb_re = re.compile(r'^(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),(\d+),-?(\d+)\n')



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
class JitterInfoStruct:
    'This class to save some jitter buffer information'


    def __init__(self):
        """
        JitterInfoStruct construction
        """
        self.filename = []
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

    def ProcessLine(self, i, line):
        if None != Test( ajb_re.search(line)):
            self.infolist[i].ssrc.append(int(Test.result.groups()[0]))
            self.infolist[i].samples10ms.append(int(Test.result.groups()[1]))
            self.infolist[i].receivedtime.append(int(Test.result.groups()[2]))
            self.infolist[i].rtpSequce.append(int(Test.result.groups()[3]))
            self.infolist[i].rtpTimestamp.append(int(Test.result.groups()[4]))
            self.infolist[i].playTime.append(int(Test.result.groups()[5]))
            self.infolist[i].averageDelay.append(int(Test.result.groups()[6]))
            self.infolist[i].delayedPlay.append(int(Test.result.groups()[7]))
        else:
            print "Not match for the ", i, "files, line:", line

    def GetStat(self):
        'This function is to update all status of the file'
        if self.filecount <= 0:
            print "GetStat, no file to process ", self.filecount
            return
        filelinecount = len(self.infolist[0].receivedtime)
        for i in range(self.filecount - 1):
            if filelinecount != len(self.infolist[i + 1].receivedtime):
                print "File 1 has ", filelinecount, ", but the ", i + 1, "files has ", len(self.infolist[i+1].receivedtime)
                return
        lenofReceivetime = len(self.infolist[0].receivedtime)
        for i in range(self.filecount):
            receivedpkt = 0
            reftime = 0
            statscount = 0
            j = 0
            while j < lenofReceivetime:
                if j == 0:
                    statscount = 0;
                    self.infolist[i].StatTime.append( self.infolist[i].receivedtime[j]) ## save first received time
                    self.infolist[i].StatAverageDelay.append(self.infolist[i].averageDelay[j])
                    self.infolist[i].StatReceivedPacket.append(0)
                    receivedpkt += 1
                    reftime = self.infolist[i].receivedtime[j];

                else:
                    if (self.infolist[i].receivedtime[j] - self.infolist[i].StatTime[statscount]) <= 1000:
                        receivedpkt += 1
                    else:
                        statscount += 1
                        reftime += 1000
                        self.infolist[i].StatReceivedPacket.append(receivedpkt) # update received account
                        receivedpkt = 0                                 # reset to zero after every computation
                        self.infolist[i].StatAverageDelay.append( self.infolist[i].averageDelay[j])
                        self.infolist[i].StatTime.append(reftime)

                        while (self.infolist[i].receivedtime[j] > (self.infolist[i].StatTime[statscount] + 1000)):
                            statscount += 1
                            reftime += 1000
                            self.infolist[i].StatTime.append(reftime)  # if received time changes too long, it need change time and average dealy
                            self.infolist[i].StatAverageDelay.append( self.infolist[i].averageDelay[j])    # keep origin delay
                            self.infolist[i].StatReceivedPacket.append(0)       # change received

                j += 1

    def PrintGraph(self,filename):
        if self.filecount <= 0:
            return
        figobj = plt.figure(figsize=(28,14), dpi=1000)
        axisLeft = None
        axisLeft = figobj.add_subplot(2,1,1)
        axisLeft.set_title("Received Package Number",color='blue',size=18,weight='heavy')
        axisLeft.set_xlabel("Received Time")
        axisLeft.set_ylabel("Received Package")
        axisLeft.grid(b=True,which='major',axis='both',color="black")
        axisLeft.plot(self.infolist[0].StatTime,self.infolist[0].StatReceivedPacket, 'b-', label='received pkg')
        averagepacket = [];
        for i in range(len(self.infolist[0].StatTime)):
            averagepacket.append(50) # average received packet is 50
        axisLeft.plot(self.infolist[0].StatTime,averagepacket, 'r-', label='average pkg')     
        plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
           ncol=2, borderaxespad=0.)
        plt.savefig(filename, bbox_inches = 0)
        
        axisLeft = figobj.add_subplot(2,1,2)
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

        legendmode = ['b+', 'g*', 'rd','co','m-.','y--','kv','ws' ]
        for i in range(self.filecount):
            axisLeft.plot(self.infolist[i].StatTime,self.infolist[i].StatAverageDelay, legendmode[i], label=self.infolist[i].filename)
            print "Finish plot ", self.infolist[i].filename

        axisLeft.set_title("Average Delay",color='blue',size=18,weight='heavy')
        axisLeft.set_xlabel("Received Time")
        axisLeft.set_ylabel("Average Delat")
        axisLeft.grid(b=True,which='major',axis='both',color="black")
        axisLeft.tick_params(axis='both',labelsize=8)

        print "Start add legend"
        plt.legend(bbox_to_anchor=(0., 1.02, 1., .102), loc=3,
           ncol=2, borderaxespad=0.)
        plt.savefig(filename, bbox_inches = 0)


    #os.mkdir('testdir')


    def ProcessFile(self,i, filename):
        result = re.search("csv", filename)
        if None != result:
            try:
                self.infolist[i].filename.append(filename)
                originfile = open(filename, 'rb')
                for eachLine in originfile:
                    self.ProcessLine(i,eachLine)
                originfile.close()
            except IOError, e:
                print 'file open error:', e






if __name__=="__main__":
    print "Usage: comparejitter.py cvs1 ... csvn result"
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
    fileProcess.GetStat();
    fileProcess.PrintGraph(sys.argv[argc - 1])

    del fileProcess







