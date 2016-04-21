import os
import sys
import numpy as np
import matplotlib.pyplot as plt

filePath = sys.path[0]
curPath = os.getcwd()

command = "cd " + filePath
print command
os.system(command)

filename = filePath + "/../../vqtest_result.csv"
fresult = open(filename,"wb")
fresult.close()

command = filePath + "/../../run_ta -t @macosx-videoquality-bitrate --use-nlc --network-emulate"
print command
os.system(command)

command = "cp " + filePath + "/../../vqtest_result.csv " + filePath
print command
os.system(command)

#plot
bitrate = []
psnr_s = []
psnr_r = []
filename = filePath + "/vqtest_result.csv"
fresult = open(filename,"r")
for line in fresult:
	info = line.split(',')
	bitrate.append(int(info[0]))
	psnr_s.append(float(info[1]))
	psnr_r.append(float(info[2]))
	#print (bitrate,psnr_s,psnr_r)
fresult.close()

plt.title("vqtest_result")
plt.xlabel("Bitrate(bps)")
plt.ylabel("PSNR")
plt.xlim(0,2000)
plt.ylim(25,40)
plt.plot(bitrate,psnr_s,label="PSNR_Send")
plt.plot(bitrate,psnr_r,label="PSNR_Recv")
plt.legend()
filename = filePath + "/vqtest_result.jpg"
plt.savefig(filename)

command = "cd " + curPath
print command
os.system(command)