package com.wx2.support;

/**
 * Created by wilsonc on 9/1/15.
 */
public abstract  class WSClient {
    public abstract void onMessage(String type, String message);
    public abstract void onOpen();
    public abstract void onStartCall(int count);

    protected String mURI;
    protected long mNativeRef;

    public WSClient(String sURI) {
        mURI = sURI;
    }

    public int send(String type, String msg) {
        return send(mNativeRef, type, msg);
    }

    public void connect() {
        mNativeRef = connect(mURI, this);
    }

    public void close() {
        close(mNativeRef);
        mNativeRef = 0;
        mURI = null;
    }

    private static native long connect(String sURI, WSClient sink);
    private static native int send(long mNativeRef, String type, String msg);
    private static native int close(long mNativeRef);
}
