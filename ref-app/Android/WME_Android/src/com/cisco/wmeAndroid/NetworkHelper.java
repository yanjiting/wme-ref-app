package com.cisco.wmeAndroid;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;

import org.apache.http.conn.util.InetAddressUtils;

import android.util.Log;



public class NetworkHelper {
	
	public static String getLocalIpAddressV4()
    {
        String ip ="";
        try
        {
            for (Enumeration<NetworkInterface> en = NetworkInterface.getNetworkInterfaces(); en.hasMoreElements();)
            {
                NetworkInterface intf = en.nextElement();
                for (Enumeration<InetAddress> enumIpAddr = intf.getInetAddresses(); enumIpAddr.hasMoreElements();)
                {
                    InetAddress inetAddress = enumIpAddr.nextElement();
                    if (!inetAddress.isLoopbackAddress() && InetAddressUtils.isIPv4Address(inetAddress.getHostAddress()))  //这里做了一步IPv4的判定
                    {
                        ip = inetAddress.getHostAddress().toString();
                        return ip;
                    }
                }
            }
        } catch (SocketException e)
        {
            Log.i("SocketException--->", ""+e.getLocalizedMessage());
            return "ip is error";
        }
        return ip;
    }
	
	public static boolean isIPaddressValid(String ipAddress)
	{
		boolean isValidIP = true;
		//check IP address
		if (!ipAddress.matches ("^\\d{1,3}(\\.(\\d{1,3}(\\.(\\d{1,3}(\\.(\\d{1,3})?)?)?)?)?)?")) 
		{ 
			isValidIP = false;
		} else 
		{
	        String[] splits = ipAddress.split("\\.");
	        if (splits.length != 4)
	        {
	        	isValidIP = false;
	        }
	        else {
	        	for (int i=0; i<splits.length; i++) {
		            if (Integer.valueOf(splits[i]) > 255) {
		            	isValidIP = false;
		            	break;
		            }
		        }
	        }
		}
		return isValidIP;
	}
	
	public static boolean isPortValid(String port)
	{
		boolean isValidPort = true;
		if (!port.matches("\\d{1,5}"))
		{
			isValidPort = false;
		}
		else
		{
			if (Integer.valueOf(port) > 65535)
			{
				isValidPort = false;
			}
		}
		
		
		return isValidPort;
	}
}