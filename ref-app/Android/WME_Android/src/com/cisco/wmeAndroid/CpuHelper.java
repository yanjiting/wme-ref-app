package com.cisco.wmeAndroid;

import java.io.BufferedReader;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.RandomAccessFile;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.Display;
import android.view.WindowManager;

import com.cisco.webex.wme.WmeClient;
import com.webex.wseclient.WseEngine;




public class CpuHelper {
	
	private MessageHandleThread m_MH_thread = null;
    private Handler m_MessageHandler = null;
    private int cpu_snapshot_interval = 0;
    private String[] cpu_snapshot = null;
    public long usage = -1;
    
    public void EnableUsage(int interval) {
    	if (m_MH_thread != null)
    	{
    		return;
    	}
    	cpu_snapshot_interval = interval;
    	m_MH_thread = new MessageHandleThread();
    	m_MH_thread.start();
    	int wait_time = 0;
    	while(m_MessageHandler == null)
    	{
	    	try {
				Thread.sleep(50);
				wait_time += 50;
				if (wait_time > 2000)
					break;
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
    	}

        m_MessageHandler.sendEmptyMessageDelayed(Constants.EVENT_CPU_USAGE_SNAPSHOT, 0);
    }
	
	public void DisableUsage() {
		if (m_MessageHandler != null)
    	{
    		m_MessageHandler.removeMessages(Constants.EVENT_CPU_USAGE_SNAPSHOT);
    		m_MessageHandler.getLooper().quit();
    	}
		
		m_MH_thread = null;
		usage = -1;
	}
	
	private long calcUsage(String[] cpuInfos)
	{
		long delta_user = Long.parseLong(cpuInfos[2]) + Long.parseLong(cpuInfos[3]) - (Long.parseLong(cpu_snapshot[2]) + Long.parseLong(cpu_snapshot[3]));
		long delta_sys = Long.parseLong(cpuInfos[4]) - Long.parseLong(cpu_snapshot[4]);
		long delta_idle = Long.parseLong(cpuInfos[5]) - Long.parseLong(cpu_snapshot[5]);
		long delta_io = Long.parseLong(cpuInfos[6]) + Long.parseLong(cpuInfos[7]) + Long.parseLong(cpuInfos[8])
				- (Long.parseLong(cpu_snapshot[6]) + Long.parseLong(cpu_snapshot[7]) + Long.parseLong(cpu_snapshot[8]));
		
		if (delta_user < 0	||
			delta_sys < 0	||
			delta_idle < 0	||
			delta_io < 0)
		{
			Log.e("cpu", "CpuHelper, calc WRONG cpu load: delta_user="+delta_user+",delta_sys="+delta_sys+",delta_idle="+delta_idle+",delta_io="+delta_io);
			return -1;
		}
		
		
		float v = (float)(delta_user + delta_sys + delta_io) / (delta_user + delta_sys + delta_io + delta_idle);
		
		//Log.i("cpu","calcUsage, v="+v+",delta_user="+delta_user+",delta_sys="+delta_sys+",delta_idle="+delta_idle);
		//Log.i("cpu","calcUsage, idle="+cpuInfos[5]+", idle="+cpu_snapshot[5]);
		return (long) (v * 100);
	}
	
	
	
//	public static CPUInfo getCPUInfo() {
//        String strInfo = null;
//        try
//        {
//                byte[] bs = new byte[1024];
//                RandomAccessFile reader = new RandomAccessFile("/proc/cpuinfo", "r");
//                reader.read(bs);
//                String ret = new String(bs);
//                int index = ret.indexOf(0);
//                if(index != -1) {
//                        strInfo = ret.substring(0, index);
//                } else {
//                        strInfo = ret;
//                }
//        }
//        catch (IOException ex)
//        {
//                strInfo = "";
//                ex.printStackTrace();
//        }
//        
//        CPUInfo info = parseCPUInfo(strInfo);
//        //info.mCPUMaxFreq = getMaxCpuFreq();
//                        
//        return info;
//	}
//	
//	private static CPUInfo parseCPUInfo(String cpuInfo) {
//        if (cpuInfo == null || "".equals(cpuInfo)) {
//                return null;
//        }
//
//        CPUInfo ci = new CPUInfo();
//        ci.mCPUType = "Unknown";
//        ci.mCPUFeature = "";
//        ci.mCPUCount = 1;
//
//        if (cpuInfo.contains("ARMv5")) {
//                ci.mCPUType = "armv5te";
//        } else if (cpuInfo.contains("ARMv6")) {
//                ci.mCPUType = "armv6";
//        } else if (cpuInfo.contains("ARMv7")) {
//                ci.mCPUType = "armv7";
//        }
//
//        if (cpuInfo.contains("neon")) {
//                ci.mCPUFeature += "neon";
//        }
//        if (cpuInfo.contains("vfp")) {
//                ci.mCPUFeature += "_vfp";
//        }
//
//        String[] items = cpuInfo.split("\n");
//
//        for (String item : items) {
//                if (item.contains("CPU variant")) {
//                        int index = item.indexOf(": ");
//                        if (index >= 0) {
//                                String value = item.substring(index + 2);
//                                try {
//                                        ci.mCPUCount = Integer.decode(value);
//                                        ci.mCPUCount = ci.mCPUCount == 0 ? 1 : ci.mCPUCount;
//                                } catch (NumberFormatException e) {
//                                        ci.mCPUCount = 1;
//                                }
//                        }
//                } 
//        }
//        
//        return ci;
//	}
//	
//	public static class CPUInfo{
//        public CPUInfo(){
//                
//        }
//        
//        public String mCPUType;
//        public int mCPUCount;
//        public String mCPUFeature;                
//        //public long mCPUMaxFreq;
//	}
	
	
	private class MessageHandleThread extends Thread{				
		@Override
		public void run(){
			Looper.prepare();
			m_MessageHandler = new Handler(){
				@Override
				public void handleMessage(Message msg){					
					//Log.i("wme_android","CpuHelper, handleMessage, what= "+msg.what);
					switch (msg.what) {
						case Constants.EVENT_CPU_USAGE_SNAPSHOT:
						{
							String[] cpuInfos = null;
						    try {
						        BufferedReader reader = new BufferedReader(new InputStreamReader(
						                new FileInputStream("/proc/stat")), 1000);
						        String load = reader.readLine();
						        reader.close();
						        cpuInfos = load.split(" ");
						    } catch (IOException ex) {
						        ex.printStackTrace();
						    }
							
						    if (cpu_snapshot != null)
						    {
						    	long cpuload = calcUsage(cpuInfos);
						    	if (cpuload <= 100 && cpuload >= 0)
						    	{
						    		usage = cpuload;
						    	}
						    	else
						    	{
						    		Log.e("cpu", "CpuHelper, calc WRONG cpu load: "+cpuload);
						    	}
						    }
						    cpu_snapshot = cpuInfos;
							
							
							m_MessageHandler.sendEmptyMessageDelayed(Constants.EVENT_CPU_USAGE_SNAPSHOT, cpu_snapshot_interval);
						}
						break;

						default:
						break;
					}					
				}
			};
			Looper.loop();
		}		
	}
}