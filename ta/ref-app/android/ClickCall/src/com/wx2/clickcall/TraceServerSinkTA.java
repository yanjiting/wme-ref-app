package com.wx2.clickcall;

import com.webex.wme.*;
import android.util.Log;

public class TraceServerSinkTA implements TraceServerSink {

    public String json;

    public void OnTraceServerResult(WmeStunTraceResult reason, String jsonRet) {
        Log.v("ClickCall", "TraceServerSinkTA - onResult is called : " + jsonRet);
        json = jsonRet;
    }
}
