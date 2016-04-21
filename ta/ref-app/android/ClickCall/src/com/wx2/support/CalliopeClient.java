package com.wx2.support;

import android.util.Log;

import com.wx2.clickcall.MyApplication;

import java.util.UUID;

/**
 * Created by wilsonc on 9/2/15.
 */
public abstract class CalliopeClient {
    public abstract void onVenue(String url);
    public abstract void onConfluence(String sdp, String url);

    private String mLinusUrl;
    private String mTrackingID;
    private long mNativeRef;
    private boolean mConfluenceDeleted = false;
    private boolean mConnectLocalLinus;
    public CalliopeClient() {
        mTrackingID = "WMETEST_" + UUID.randomUUID() + "_345";
        mNativeRef = create(mTrackingID, this);
    }

    public void setLinusUrl(String linusUrl, boolean bLocalLinus) {
        mLinusUrl = linusUrl;
        mConnectLocalLinus = bLocalLinus;
        setLinus(mLinusUrl, mConnectLocalLinus);
        setUserPassword(MyApplication.USER_ID, MyApplication.USER_PASSWORD);
    }

    public void setLinusUrl(String linusUrl) {
        mConnectLocalLinus = (linusUrl != null && !linusUrl.isEmpty());
        if(mConnectLocalLinus){
            if(!linusUrl.startsWith("http://"))
                mLinusUrl = "http://" + linusUrl;
            else
                mLinusUrl = linusUrl;
            if(!mLinusUrl.endsWith("/")){
                mLinusUrl += "/";
            }
        }
        setLinus(mLinusUrl, mConnectLocalLinus);
        setUserPassword(MyApplication.USER_ID, MyApplication.USER_PASSWORD);
    }

    public void setUserPassword(String uid, String pwd) {
        if(uid != null && pwd != null) {
            setUserPasswd(uid, pwd);
        }
    }

    public void createVenue() {
        if(mConnectLocalLinus) {
            Log.i("ClickCall", "calliopeclient::createVenue, local");
            String venueURL = "" + UUID.randomUUID();
            onVenue(venueURL);
        } else {
            createVenue(mNativeRef);
        }
    }

    public void createConfluence(String venueUrl, String sdp) {
        createConfluence(mNativeRef, venueUrl, sdp, UUID.randomUUID().toString());
    }

    public void requestFloor() {
        requestFloor(mNativeRef);
    }

    public void releaseFloor() {
        releaseFloor(mNativeRef);
    }

    public void deleteVenue(String venueUrl) {
        if(!mConfluenceDeleted) {
            doDelete(mNativeRef, venueUrl, 0);
        }
    }

    public void deleteConfluence(String confluenceUrl) {
        doDelete(mNativeRef, confluenceUrl, 1);
        mConfluenceDeleted = true;
    }

    public void destroy() {
        destroy(mNativeRef);
        mNativeRef = 0;
    }

    private static native long create(String trackingID, CalliopeClient callback);
    private static native void setLinus(String linusUrl, boolean bLocalLinus);
    private static native void setUserPasswd(String uid, String pwd);
    private static native int requestFloor(long ref);
    private static native int releaseFloor(long ref);
    private static native int createVenue(long ref);
    private static native int createConfluence(long ref, String venueUrl, String sdp, String uuid);
    private static native int doDelete(long ref, String url, int type);
    private static native int destroy(long ref);
}
