package com.wx2.clickcall;

import com.webex.wme.*;
import android.util.Log;

public class StunTraceSinkTA implements StunTraceSink {

    public String json;

    public void OnResult(WmeStunTraceResult reason, String jsonRet) {
        Log.v("ClickCall", "StunTraceSinkTA - onResult is called : " + jsonRet);
        json = jsonRet;
    }
}