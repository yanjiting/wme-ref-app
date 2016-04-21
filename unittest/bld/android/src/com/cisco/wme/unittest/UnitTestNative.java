package com.cisco.wme.unittest;

import android.util.Log;


/**
 * class WmeNative for internal using with JNI lib
 * The value of parameters can be referred in WmeParameters.java
 *  @mtype:  Wme Media Type
 *  @ttype:  Wme Track Type
 *  @dtype:  Wme Device Type
 *  @ctype:  Wme Codec Type
 *  @atype:  Wme Audio Out Type
 */
public class UnitTestNative {
	//load shared library
    static {
    	try {
    		System.loadLibrary("c++_shared");
    		
    		//load common library
    		System.loadLibrary("util");
    		System.loadLibrary("wqos");
    		System.loadLibrary("srtp");
    		System.loadLibrary("wrtp");
    		System.loadLibrary("wtp");
    		
    		//load codec library
//    		System.loadLibrary("welsvp");
//    		System.loadLibrary("welsenc");
//    		System.loadLibrary("welsdec");
    		
    		//load wme library
            System.loadLibrary("wmeutil");
    		System.loadLibrary("wsertp");
    		System.loadLibrary("wseclient");
            System.loadLibrary("appshare");
    		System.loadLibrary("audioengine");
    		System.loadLibrary("wmeclient");
    		
    		//load unittest library
    		System.loadLibrary("wtp-unittest");
    		System.loadLibrary("util-unittest");
    		System.loadLibrary("wqos-unittest");
    		System.loadLibrary("wrtp-unittest");
    		System.loadLibrary("dolphin-unittest");
    		System.loadLibrary("shark-unittest");
    		System.loadLibrary("wme-unittest");
    		System.loadLibrary("appshare-unittest");
    		System.loadLibrary("all-unittest");
    	} catch (Exception e){
    		Log.v("wme_android","Load library failed");
    	}
    }

    public static void preload_symbols(){  
        Log.i("native_hack", "to preload library symbols before debugging");  
    }  
    
    public static native int GotoUnittest(String module, String path);
}

