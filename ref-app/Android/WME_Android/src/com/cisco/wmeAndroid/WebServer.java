package com.cisco.wmeAndroid;

import java.io.IOException;
import java.net.SocketTimeoutException;
import java.util.List;

import org.apache.http.HttpResponse;
import org.apache.http.HttpStatus;
import org.apache.http.NameValuePair;
import org.apache.http.client.ClientProtocolException;
import org.apache.http.client.HttpClient;
import org.apache.http.client.entity.UrlEncodedFormEntity;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.client.methods.HttpPost;
import org.apache.http.client.methods.HttpRequestBase;
import org.apache.http.entity.StringEntity;
import org.apache.http.impl.client.DefaultHttpClient;
import org.apache.http.params.BasicHttpParams;
import org.apache.http.params.HttpConnectionParams;
import org.apache.http.protocol.HTTP;
import org.apache.http.util.EntityUtils;

import android.content.Context;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;


public class WebServer {	
	public static final int MSG_HTTP_GET = 0;
	public static final int MSG_HTTP_POST = 1;
	

	// private final Activity mActivity = null;
	private Context mContext = null;
	private HttpClient mHttpClient = null;
	private OnHttpServerListener mHttpServerListener = null;
	private ServerThread mServerThread = null; 
	private Handler mThreadHandle = null;
	private boolean mServerThreadAvalible = true;
	private HttpRequestBase mHttpRequest = null;	
	
	private static final int REQUEST_TIMEOUT = 60 * 1000;// 设置请求超时60秒钟
	private static final int SO_TIMEOUT = 60 * 1000; // 设置等待数据超时时间60秒钟
	
	
	private final static Object mObjServerSync = new Object();
	public static Object getServerSyncObj(){
		return mObjServerSync;
	}
	
	public WebServer(Context context) {
		mContext = context;
	}
	
	private static WebServer mWebServer = null;
	public static WebServer getInstance(Context context){
		if(mWebServer == null){
			mWebServer = new WebServer(context);
			mWebServer.init();
		}
		return mWebServer;
	}
	
	
	public void init(){
		BasicHttpParams httpParams = new BasicHttpParams();
		HttpConnectionParams.setConnectionTimeout(httpParams, REQUEST_TIMEOUT); 
		HttpConnectionParams.setSoTimeout(httpParams, SO_TIMEOUT); 
		mHttpClient = new DefaultHttpClient(httpParams);
		mServerThread = new ServerThread();
		if(null != mServerThread)
			mServerThread.start();
	}
	
	public void destroy(){
		Log.i("wme_android", "webserver destroy");
		if (mThreadHandle != null)
		{
			mThreadHandle.removeMessages(MSG_HTTP_POST);
			mThreadHandle.removeMessages(MSG_HTTP_GET);
			mThreadHandle.getLooper().quit();
		}
		Log.i("wme_android", "webserver destroy, clean msg");
		
		if(null != mHttpClient){
			mHttpClient.getConnectionManager().shutdown();
			if (mHttpRequest != null)
				mHttpRequest.abort();
			synchronized (getServerSyncObj()){
				mServerThreadAvalible = false;
			}		
		}
		Log.i("wme_android", "webserver destroy, shut down connection");
		if(mWebServer != null){
			mWebServer = null;
		}	
	}
	
	public boolean isRunning() {
		boolean run = false;
		if (mThreadHandle != null)
			run = true;
		return run;
	}
	 
	public void cancelRequest(){
		if(null != mHttpRequest)
			mHttpRequest.abort();
	}
	
	public void setHttpServerListener(OnHttpServerListener listner){
		mHttpServerListener = listner;
	}
	
	public interface OnHttpServerListener{
		void onHttpResult(int msg, int result, String strData);
	}
	
	public void postHttp(int msg, String strUrl, List<NameValuePair> postParams){
		Log.i("wme_android", "postHttp, msg="+msg+", url:"+strUrl);
		if(null == mThreadHandle){
			if(null != mHttpServerListener)
				mHttpServerListener.onHttpResult(msg, Constants.MSG_ERR_FAIL, "mThreadHandle is null");
			return;
		}
		
		if(!isConnect()){
			if(null != mHttpServerListener)
				mHttpServerListener.onHttpResult(msg, Constants.MSG_ERR_NO_NET_CONNECTION, "not connected");
			return;
		}
		HttpParamStruct paramStruct = new HttpParamStruct(strUrl, postParams);
		Message message = new Message();
		message.what = MSG_HTTP_POST;
		message.arg1 = msg;
		message.obj = paramStruct;
		mThreadHandle.sendMessage(message);
	}
	
	public void postHttp(int msg, String strUrl, StringEntity strEntity) {
		if (null == mThreadHandle) {
			if (null != mHttpServerListener)
				mHttpServerListener.onHttpResult(msg, Constants.MSG_ERR_FAIL,
						null);
			return;
		}

		if (!isConnect()) {
			if (null != mHttpServerListener)
				mHttpServerListener.onHttpResult(msg,
						Constants.MSG_ERR_NO_NET_CONNECTION, null);
			return;
		}
		HttpParamStruct paramStruct = new HttpParamStruct(strUrl, strEntity);
		Message message = new Message();
		message.what = MSG_HTTP_POST;
		message.arg1 = msg;
		message.obj = paramStruct;
		mThreadHandle.sendMessage(message);
	}

	public void getHttp(int msg, String strUrl){
		Log.d("WebServer", "getHttp(): strUrl="+strUrl);
		Log.d("WebServer", "getHttp(): mHttpServerListener="+mHttpServerListener);
		if(null == mThreadHandle){
			Log.d("WebServer", "null == mThreadHandle");
			if(null != mHttpServerListener)
				mHttpServerListener.onHttpResult(msg, Constants.MSG_ERR_FAIL, null);
			return;
		}
		
		if(!isConnect()){
			Log.d("WebServer", "getHttp(): !isConnect()");
			if(null != mHttpServerListener)
				mHttpServerListener.onHttpResult(msg, Constants.MSG_ERR_NO_NET_CONNECTION, null);
			return;
		}
		
		HttpParamStruct paramStruct = new HttpParamStruct(strUrl);
		Message message = new Message();
		message.what = MSG_HTTP_GET;
		message.arg1 = msg;
		message.obj = paramStruct;
		mThreadHandle.sendMessage(message);
	}
	
	
	private class ServerThread extends Thread{				
		@Override
		public void run(){
			Looper.prepare();
			mThreadHandle = new Handler(){
				@Override
				public void handleMessage(Message msg){					
					synchronized (getServerSyncObj()){
						Log.i("wme_android", "handle Http, what="+msg.what+", msg:"+msg.arg1);
						if(!mServerThreadAvalible || null == mHttpClient || null == mHttpServerListener){	
							Log.e("WebServer", "!mServerThreadAvalible || null == mHttpClient || null == mHttpServerListener");
							return;
						}
						switch (msg.what) {
						case MSG_HTTP_GET:{
							try {								
								mHttpRequest = new HttpGet(((HttpParamStruct)msg.obj).mUrl);
								HttpResponse httpResponse = mHttpClient.execute(mHttpRequest);
								mHttpRequest = null;
								if (httpResponse.getStatusLine().getStatusCode() == HttpStatus.SC_OK) {
									String strResult = EntityUtils.toString(httpResponse.getEntity());
									mHttpServerListener.onHttpResult(msg.arg1, Constants.MSG_ERR_NONE, strResult);
								}else{
									mHttpServerListener.onHttpResult(msg.arg1, Constants.MSG_ERR_FAIL, null);
								}
							} catch (ClientProtocolException e) {
								e.printStackTrace();								
								mHttpServerListener.onHttpResult(msg.arg1, Constants.MSG_ERR_FAIL, null);
							} catch (IOException e) {
								if(e instanceof SocketTimeoutException)
									mHttpServerListener.onHttpResult(msg.arg1, Constants.MSG_ERR_TIMEOUT, null);
								else if(e.getMessage().equals("Request already aborted"))
									mHttpServerListener.onHttpResult(msg.arg1, Constants.MSG_ERR_CANCEL, null);
								else if(e.getMessage().equals("Connection already shutdown"))
									mHttpServerListener.onHttpResult(msg.arg1, Constants.MSG_ERR_NO_NET_CONNECTION, null);
								else
									mHttpServerListener.onHttpResult(msg.arg1, Constants.MSG_ERR_FAIL, null);
								e.printStackTrace();
//								if(e.getMessage())
								
							}
							
						}							
							break;
							
						case MSG_HTTP_POST:{
							try {
								HttpParamStruct httpParamStruct = (HttpParamStruct) msg.obj;
								mHttpRequest = new HttpPost(httpParamStruct.mUrl);
								if (null != httpParamStruct.mValuePairs) {
									UrlEncodedFormEntity formEntity = new UrlEncodedFormEntity(
											httpParamStruct.mValuePairs, HTTP.UTF_8);
									((HttpPost) mHttpRequest)
											.setEntity(formEntity);
								} else if (null != httpParamStruct.mStrEntity) {
									((HttpPost) mHttpRequest)
											.setEntity(httpParamStruct.mStrEntity);
								}
								
								HttpResponse httpResponse = mHttpClient.execute(mHttpRequest);
								int httpstatus = httpResponse.getStatusLine().getStatusCode();
								if (httpstatus == HttpStatus.SC_OK) {
									String strResult = EntityUtils.toString(httpResponse.getEntity());
									mHttpServerListener.onHttpResult(msg.arg1, Constants.MSG_ERR_NONE, strResult);
								}else{
									mHttpServerListener.onHttpResult(msg.arg1, Constants.MSG_ERR_FAIL, Integer.toString(httpstatus));
								}
								mHttpRequest = null;
							} catch (ClientProtocolException e) {
								e.printStackTrace();
								mHttpServerListener.onHttpResult(msg.arg1, Constants.MSG_ERR_FAIL, e.toString());
							} catch (IOException e) {
								e.printStackTrace();
								mHttpServerListener.onHttpResult(msg.arg1, Constants.MSG_ERR_FAIL, e.toString());
							}
							
						}							
							break;

						default:
							break;
						}
						
					}					
				}
			};
			Looper.loop();
		}		
	}
	
	public class HttpParamStruct{
		public String mUrl = null;
		public List<NameValuePair> mValuePairs = null;
		public StringEntity mStrEntity = null;

		public HttpParamStruct(String url) {
			mUrl = url;
		}

		public HttpParamStruct(String url, List<NameValuePair> valuePairs) {
			mUrl = url;
			mValuePairs = valuePairs;
		}
		
		public HttpParamStruct(String url, StringEntity strEntity) {
			mUrl = url;
			mStrEntity = strEntity;
		}
	}
	
	private boolean isConnect(){
		if (null == mContext)
			return false;
		ConnectivityManager manager = (ConnectivityManager) mContext
				.getSystemService(Context.CONNECTIVITY_SERVICE);
		if(manager!=null){
		     NetworkInfo[] infos = manager.getAllNetworkInfo();
		     if(infos[0].isConnected()||infos[1].isConnected())	
		     {	
		    	 return true;
		     }			

			return false;
		}
		return false;
	}
}
