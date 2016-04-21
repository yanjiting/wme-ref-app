package com.wx2.clickcall;

import android.util.Log;

public class NativeClickCall {
    static {
        try {
            System.loadLibrary("c++_shared");
            System.loadLibrary("clickcall");
        }catch (Exception e){
            Log.w("ClickCall", "Failed to load native library: " + e.toString());
        }
    }

    public static native void doGCov(int action);
    public static native void SetEnv(String name,String value);
}
