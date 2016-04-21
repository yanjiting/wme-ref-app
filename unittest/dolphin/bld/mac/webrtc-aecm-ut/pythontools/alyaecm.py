#! /usr/bin/env python

import matplotlib.pyplot as plt
import numpy as np
import shutil
import os
import sys
#from IPython.display import Audio


wavFileName = ["AECFarInIutput-float32-Chn1-16000.wav", \
               "AECNearInIutput-float32-Chn1-16000.wav", \
               "AECOutput-float32-Chn1-16000.wav"]

def Alywav(wave, path):

    print "\n---> Process in %s ...... " %path

    num = len(wave)

    ## Copy wav file to test folder.
    shutil.copyfile(wave[0], "./mic.wav")
    shutil.copyfile(wave[1], "./spk.wav")
    if num == 3:
        shutil.copyfile(wave[2], "./rtout.wav")

    ## Run ut and convert wav file to raw.
    os.system('./webrtc-aecm-ut mic.wav spk.wav utout.wav')

    os.system('sox mic.wav -e float -b 32 mic.raw')
    os.system('sox spk.wav -e float -b 32 spk.raw')
    os.system('sox utout.wav -e float -b 32 utout.raw')
    #os.system('sox utout.wav -e signed-integer -b 16 utout.raw')
    if num == 3:
        os.system('sox rtout.wav -e float -b 32 rtout.raw')

    ## Delete wav file in test folder.
    os.system('rm *.wav')

    ## Load raw file data.
    spk = np.fromfile('./spk.raw', np.float32)
    mic = np.fromfile('./mic.raw', np.float32)
    #utout = np.fromfile('./utout.raw', np.short)
    utout = np.fromfile('./utout.raw', np.float32)
    if num == 3:
        rtout = np.fromfile('./rtout.raw', np.float32)
    nlp = np.fromfile('./pnlp.raw', np.float32)
    hnl = np.fromfile('./phnl.raw', np.float32)
    gain = np.fromfile('./gain.raw', np.float32)
    fe = np.fromfile('./fe.raw', np.float32)
    vad = np.fromfile('./vad.raw', np.float32)
    fv = np.fromfile('./fv.raw', np.float32)
    #wn = np.fromfile('./wn.raw', np.float32)

    ## Setting for follow drawing.
    fs = 16000
    tot_len1 = len(mic)
    tot_len2 = len(spk)
    tot_len3 = len(utout)
    lw = min(tot_len1, tot_len2, tot_len3) - fs/100
    #lw = min(tot_len1, tot_len2, tot_len3, tot_len4) - fs/100
    t = lw/fs
    lp = 100*t
    a = 0
    w = t
    b = a + w
    tw = np.linspace(a, b, w*fs)
    tp = np.linspace(a, b, w*65*250)
    tg = np.linspace(a, b, w*250)
    #print len(tg)
    w0 = a*fs
    wt = b*fs
    p0 = a*65*250
    pt = b*65*250
    g0 = a*250
    gt = b*250
    mic = mic[w0:wt]
    spk = spk[w0:wt]
    utout = utout[w0:wt]
    #print 'max = %f ' %max(utout)
    if num == 3:
        rtout = rtout[w0:wt]
    nlp = nlp[p0:pt]
    hnl = hnl[p0:pt]
    #wn = wn[p0:pt]
    gain = gain[g0:gt]
    fe = fe[g0:gt]
    #print len(fe)
    vad = 8*vad[g0:gt]
    fv = fv[g0:gt]

    ## Draw and save picture.
    numPic = 4
    if num == 3:
        numPic = 5
    fig, axes = plt.subplots(numPic, 1, figsize=(16,10))
    axes[0].plot(tw, spk, label = 'spk')
    axes[0].legend()
    #axes[0].set_ylim(-0.999, 0.999)
    axes[1].plot(tw, mic, label = 'mic')
    axes[1].plot(tw, utout, color='red', label = 'utout')
    axes[1].legend()
    #axes[2].plot(tg, fe, label = 'fe')
    axes[2].plot(tg, fv, label = 'fv')
    axes[2].plot(tg, vad, color='red', label = 'vad')
    axes[2].plot(tg, gain, color='brown', label = 'supGain')
    axes[2].legend()
    #axes[2].set_ylim(4.7, 5.3)
    axes[3].plot(tp, nlp, color='red', label = 'nlp')
    axes[3].plot(tp, hnl, label = 'hnl[]')
    axes[3].legend()
    axes[3].set_ylim(-0.1, 1.1)
    if num == 3:
        axes[4].plot(tw, rtout, label = 'rtout')
        axes[4].legend()

    for ax in axes:
        ax.grid(True)    

    name = path + '.png'
    figname = name.replace("/", "_")
    plt.savefig(figname)

    ## Clean raw files.
    os.system('rm *.raw')

    return



def AlyByDir(fdr):

    global wavFileName 

    files = [x for x in os.listdir(fdr) if os.path.isfile(fdr + '/' + x)]
    far = [fdr+'/'+x for x in files if x == wavFileName[0]]
    near = [fdr+'/'+x for x in files if x == wavFileName[1]]
    out = [fdr+'/'+x for x in files if x == wavFileName[2]]

    ret = 1

    if (len(far) == 0) or (len(near) == 0):
        ret = 0
    elif (len(out) == 0):
        Alywav([near[0], far[0]], fdr)
    else:            
        Alywav([near[0], far[0], out[0]], fdr)

    return  ret



if __name__ == '__main__':

    print "\n==============================================="
    n = 0
    os.system('rm *.png')

    thisDir = os.getcwd()

    for root, dirs, files in os.walk(thisDir):
        n += AlyByDir(root)

    print "\n===============================================\n"
    print "Completed! Analyzed %d group data .\n" %n
    
