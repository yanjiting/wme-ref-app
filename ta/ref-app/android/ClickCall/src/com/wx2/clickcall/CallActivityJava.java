package com.wx2.clickcall;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.graphics.PointF;
import android.graphics.Rect;
import android.media.AudioManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.support.v4.view.MotionEventCompat;
import android.util.Log;
import android.view.*;
import android.widget.*;

import com.webex.wme.DeviceManager.AudioOutType;
import com.webex.wme.DeviceManager.CameraType;
import com.webex.wme.MediaStatistics.AudioStatistics;
import com.webex.wme.MediaStatistics.CpuUsage;
import com.webex.wme.MediaStatistics.MemoryUsage;
import com.webex.wme.MediaStatistics.VideoStatistics;
import com.webex.wme.MediaStatistics.SharingStatistics;
import com.webex.wme.MediaConnection.MediaDirection;
import com.webex.wme.MediaConnection.MediaType;
import com.webex.wme.StunTrace;
import com.webex.wme.TraceServer;
import com.webex.wme.WmeProxyCredentialSink;
import com.webex.wme.WmeProxyManager;
import com.webex.wseclient.WseEngine;
import com.webex.wseclient.WseSurfaceView;

import java.util.Date;
import org.json.JSONObject;

public class CallActivityJava extends Activity implements WmeProxyCredentialSink {
    private MyApplication mMyApp;
    private long m_dateTime;
    private Handler m_handler = new Handler();
    private Runnable m_runnable = new Runnable() {
        public void run() {
            onTimer();
        }
    };
    private boolean m_timerStop, m_bLoopback;
    private boolean m_showStats;

    public static final String FRONT_CAMERA = "1";
    public static final String BACK_CAMERA = "0";
    private ToggleButton bttnMute;
    private CameraType mCameraType = CameraType.Front;

    private OrientationEventListener mOrientationEventListener;

    // Test Interface
    private AudioOutType currAudioType = AudioOutType.Speaker;

    private float mTouchBeginX, mTouchBeginY;
    private int mLeft, mTop;
    private int mActivePointerId = MotionEvent.INVALID_POINTER_ID;

    private Rect rcScaleBegin = new Rect();
    private ScaleGestureDetector mScaleGestureDetector;

    private WseSurfaceView local;
    private WseSurfaceView remote;
    private WseSurfaceView as, viewToBeScaled = null;

    private View viewToBeMoved = null;
    private int mASWidth = 1024, mASHeight = 768;
    private final int remoteViewIDs[] = {R.id.RemoteView1, R.id.RemoteView2, R.id.RemoteView3};

    private final ScaleGestureDetector.OnScaleGestureListener mScaleGestureListener
            = new ScaleGestureDetector.SimpleOnScaleGestureListener() {
        /**
         * This is the active focal point in terms of the viewport. Could be a local
         * variable but kept here to minimize per-frame allocations.
         */
        private PointF viewportFocus = new PointF();
        private float beginSpanX;
        private float beginSpanY;

        @Override
        public boolean onScaleBegin(ScaleGestureDetector scaleGestureDetector) {
            beginSpanX = scaleGestureDetector.getCurrentSpanX();
            beginSpanY = scaleGestureDetector.getCurrentSpanY();
            viewportFocus.x = scaleGestureDetector.getFocusX();
            viewportFocus.y = scaleGestureDetector.getFocusY();
            viewToBeScaled.getHitRect(rcScaleBegin);
            viewToBeMoved = null;
            return true;
        }

        @Override
        public boolean onScale(ScaleGestureDetector scaleGestureDetector) {
            float spanX = scaleGestureDetector.getCurrentSpanX();
            float spanY = scaleGestureDetector.getCurrentSpanY();
            float focusX = scaleGestureDetector.getFocusX() - viewportFocus.x;
            float focusY = scaleGestureDetector.getFocusY() - viewportFocus.y;
            float scale = (float) Math.sqrt((spanX * spanX + spanY * spanY) / (beginSpanX * beginSpanX + beginSpanY * beginSpanY)) - 1.f;
            int left = rcScaleBegin.left + (int)((rcScaleBegin.left - viewportFocus.x) * scale) + (int)focusX;
            int top = rcScaleBegin.top + (int)((rcScaleBegin.top - viewportFocus.y) * scale) + (int)focusY;
            int right = rcScaleBegin.right + (int)((rcScaleBegin.right - viewportFocus.x) * scale) + (int)focusX;
            int bottom = rcScaleBegin.bottom + (int)((rcScaleBegin.bottom - viewportFocus.y) * scale) + (int)focusY;

            ViewGroup.LayoutParams params = viewToBeScaled.getLayoutParams();
            if( params instanceof FrameLayout.LayoutParams )
            {
                FrameLayout.LayoutParams flp = new FrameLayout.LayoutParams(right-left, bottom-top);
                flp.leftMargin = left;
                flp.topMargin = top;
                viewToBeScaled.setLayoutParams(flp);
            }
            return true;
        }
        public void onScaleEnd(ScaleGestureDetector scaleGestureDetector)
        {
        }
    };

    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.requestWindowFeature(Window.FEATURE_NO_TITLE);
        setContentView(R.layout.call_activity);
        mMyApp = (MyApplication)this.getApplicationContext();
        mMyApp.setCurrentActivity(this);

        Button btnStop = (Button) findViewById(R.id.StopButton);
        btnStop.getBackground().setAlpha(60);

        btnStop.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                CallActivityJava.this.stop();
            }
        });

        // BUTTON: Switch camera start
        Button bttnCameraSwitch;

        bttnCameraSwitch = (Button) findViewById (R.id.CameraSwitch);
        bttnCameraSwitch.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				Log.i("CallActivityJava", "Java Camera Switch start");
				if(mCameraType == CameraType.Front)
					mCameraType = CameraType.Back;
				else
					mCameraType = CameraType.Front;

		        if(m_bLoopback){
		        	Loopback.instance().switchCamera(mCameraType);
		        }else{
		        	PeerCall.instance().switchCamera(mCameraType);
		        }

				Log.i("CallActivityJava", "Java Camera Switch end");
			}
		});
        // BUTTON: Switch camera end

        // BUTTON: Mute/Unmute
        bttnMute = (ToggleButton) findViewById (R.id.toggle);
        bttnMute.setOnClickListener(new View.OnClickListener() {
			public void onClick(View v) {
				if (bttnMute.isChecked()) {
					if (m_bLoopback) {
						Loopback.instance().muteTrackLocal(MediaType.Audio);
					} else {
						PeerCall.instance().muteTrackLocal(MediaType.Audio);
					}
				} else {
					if (m_bLoopback) {
						Loopback.instance().unmuteTrackLocal(MediaType.Audio);
					} else {
						PeerCall.instance().unmuteTrackLocal(MediaType.Audio);
					}
				}
			}
		});
         // BUTTON: Mute/Unmute end

        // BUTTON: Test features
        Button testButton;
        testButton = (Button) findViewById (R.id.testButton);
        testButton.setOnClickListener(new View.OnClickListener() {
            public void onClick(View v) {
                if (!m_bLoopback) {
                    if (currAudioType == AudioOutType.Speaker) {
                        currAudioType = AudioOutType.Earpiece;
                        PeerCall.instance().setAudioOutType(currAudioType);
                    }
                    else {
                        currAudioType = AudioOutType.Speaker;
                        PeerCall.instance().setAudioOutType(currAudioType);
                    }
                    PeerCall.instance().startStopTrack(MediaType.Audio, MediaDirection.RecvOnly, false);
                    PeerCall.instance().startStopTrack(MediaType.Audio, MediaDirection.RecvOnly, true);
                } else {
                    if (currAudioType == AudioOutType.Speaker) {
                        currAudioType = AudioOutType.Earpiece;
                        Loopback.instance().setAudioOutType(currAudioType);
                    }
                    else {
                        currAudioType = AudioOutType.Speaker;
                        Loopback.instance().setAudioOutType(currAudioType);
                    }
                    Loopback.instance().startStopTrack(MediaType.Audio, MediaDirection.RecvOnly, false);
                    Loopback.instance().startStopTrack(MediaType.Audio, MediaDirection.RecvOnly, true);
                }
            }
        });
        // BUTTON: Test end

        //Add statistics handler
        m_showStats = false;
        Button btnShowStats = (Button)findViewById(R.id.ShowStatistics);
        btnShowStats.setOnClickListener(new View.OnClickListener(){
            public void onClick(View v) {
                if (m_bLoopback) {
                    Loopback.instance().mCaller.testCSIClash();
                } else {
                    PeerCall.instance().mCall.testCSIClashes();
                }
            	CallActivityJava.this.m_showStats = !CallActivityJava.this.m_showStats;
            	CallActivityJava.this.showStatsText(m_showStats);
            }
        });
        showStatsText(m_showStats);
        TextView txtStats = (TextView)findViewById(R.id.ForStatistics);
        txtStats.getBackground().setAlpha(40);

        m_timerStop = false;
        m_dateTime = new Date().getTime();
        m_runnable.run();

        Intent intent = getIntent();
        boolean bCalliope, bBackdoor, bEnableAS;
        m_bLoopback = Boolean.parseBoolean(intent.getStringExtra(MyApplication.LOOPBACK));
        bCalliope = Boolean.parseBoolean(intent.getStringExtra(MyApplication.CALLIOPE));
        bBackdoor = Boolean.parseBoolean(intent.getStringExtra(MyApplication.BACKDOOR));
        bEnableAS = Boolean.parseBoolean(intent.getStringExtra(MyApplication.HASSHARING));

        String m_linusAddr, m_wsAddr;

        m_linusAddr = intent.getStringExtra(MyApplication.LINUS_ADDRESS);
        m_wsAddr = intent.getStringExtra(MyApplication.WS_ADDRESS);

        remote = (WseSurfaceView)findViewById(R.id.RemoteVideoGLView);
        local = (WseSurfaceView)findViewById(R.id.LocalVideoGLView);
        as = (WseSurfaceView)findViewById(R.id.ASGLView);

        //local.setRendererCallback(this);

//        remote.bringToFront();
//        local.bringToFront();
//        ViewGroup vg = (ViewGroup)as.getParent();
//        if( vg instanceof  ViewGroup ) {
//            vg.requestLayout();
//            vg.invalidate();
//        }
//        as.bringToFront();
        //as.setRendererCallback(this);

        remote.setZOrderMediaOverlay(true);
        local.setZOrderMediaOverlay(true);
        local.setZOrderOnTop(true);
        /*
        remote.setZOrderOnTop(false);
        remote.setZOrderMediaOverlay(true);
        local.setZOrderOnTop(true);
        local.setZOrderMediaOverlay(true);
         */
        
    	remote.setVisibility(bEnableAS ? View.GONE : View.VISIBLE);
    	int i = 0;
        for(int viewId : remoteViewIDs) {
    		WseSurfaceView viewRemote = (WseSurfaceView)findViewById(viewId);
    		viewRemote.setZOrderMediaOverlay(true);
    		if(bEnableAS && i == 0)
    			remote = viewRemote;
    		else
    			viewRemote.setVisibility(mMyApp.m_uMaxVideoStreams < 2 ? View.GONE : View.VISIBLE);
    		i++;
        }

        if( !bEnableAS )
            as = null;
        else {
        	WseEngine.setDisplayRotation(Surface.ROTATION_90);
        	setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
        }

        Log.i("MediaSession", "Command line is: loopback=" + m_bLoopback + ", calliope=" + bCalliope +
        		", linus=" + m_linusAddr + ", ws=" + m_wsAddr + ", backdoor=" + bBackdoor + ", enableAS=" + bEnableAS);
        if(bBackdoor){
        	if(mMyApp.getSdpOffer() == null){
        		PeerCall.instance().preview(local, remote, as);
        		setupStartupParams(true);
        		PeerCall.instance().startCall();
        	}
        	else{
        		PeerCall.instance().preview(local, remote, as);
        		setupStartupParams(true);
        		PeerCall.instance().acceptCall(mMyApp.getSdpOffer(), false);
        	}
        }
        else if(m_bLoopback){
        	Loopback.instance().preview(local, remote, as);
        	setupStartupParams(false);
        	if(!bCalliope)
        		Loopback.instance().startCall();
        	else
        		Loopback.instance().startCall(m_linusAddr);
        }else if(m_wsAddr !=null && !m_wsAddr.isEmpty()){
    		PeerCall.instance().preview(local, remote, as);
    		setupStartupParams(true);
        	if(!bCalliope)
        		PeerCall.instance().startCall(m_wsAddr);
        	else
        		PeerCall.instance().startCall(m_linusAddr, m_wsAddr);
        }

        AudioManager audioManager;
        audioManager = (AudioManager) getSystemService(AUDIO_SERVICE);
        VolumeSettingChangedListener volumeChaneListener;
        volumeChaneListener = new VolumeSettingChangedListener(new Handler(), audioManager, m_bLoopback);
        getContentResolver().registerContentObserver(android.provider.Settings.System.CONTENT_URI,
                true, volumeChaneListener);
        
        mScaleGestureDetector = new ScaleGestureDetector(this, mScaleGestureListener);

        StunTraceSinkTA mStunTraceSink = new StunTraceSinkTA();
        ((MyApplication)this.getApplicationContext()).mStunTraceSink = mStunTraceSink;
        StunTrace.INSTANCE.setStunTraceSink(mStunTraceSink);
        Log.v("CallActivity", "Created StunTraceSink");

        TraceServerSinkTA mTraceServerSink = new TraceServerSinkTA();
        ((MyApplication)this.getApplicationContext()).mTraceServerSink = mTraceServerSink;
        TraceServer.INSTANCE.setTraceServerSink(mTraceServerSink);

        String traceNodeList = "192.168.31.1:10000\n192.168.31.1:10001\n192.168.31.1:10002\n192.168.31.1:10003";

        TraceServer.INSTANCE.startTraceServer(traceNodeList, traceNodeList.length());

        Log.v("CallActivity", "Created TraceServerSink");

        WmeProxyManager.INSTANCE.setProxyCredentialSink(this);

        if(mOrientationEventListener==null){
            mOrientationEventListener = new OrientationEventListener(this){
                @Override
                public void onOrientationChanged(int orientation){
                    Log.i("ClickCall", "onOrientationChanged, orientation="+orientation);
                    updateDisplayRotation();
                }
            };
            mOrientationEventListener.enable();
        }
    }

    private AlertDialog mDlg;
    public void onProxyCredentialRequired(String proxyAddr, int port, String realm) {
        Log.i("ClickCall", "onProxyCredentialRequired, proxy=" + proxyAddr + ", port=" + port + ", realm=" + realm);
        if(MyApplication.mProxyUserName != null && MyApplication.mProxyPasswd != null) {
            WmeProxyManager.INSTANCE.setProxyUsernamePassword(MyApplication.mProxyUserName, MyApplication.mProxyPasswd);
            Log.i("ClickCall", "onProxyCredentialRequired, preset username and password.");
            return;
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        LayoutInflater inflater = getLayoutInflater();

        final View authView = inflater.inflate(R.layout.proxy_authentication, null);
        TextView viewProxyInfo = (TextView) authView.findViewById(R.id.textView);
        viewProxyInfo.setText(proxyAddr + ":" + port + " said:" + realm);

        builder.setView(authView)
                .setPositiveButton(R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int id) {
                        EditText userName = (EditText) authView.findViewById(R.id.username);
                        String sUserName = userName.getText().toString();
                        EditText passWord = (EditText) authView.findViewById(R.id.password);
                        String sPassword = passWord.getText().toString();
                        WmeProxyManager.INSTANCE.setProxyUsernamePassword(sUserName, sPassword);
                        mDlg.cancel();
                    }
                })
                .setNegativeButton(R.string.cancel, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        WmeProxyManager.INSTANCE.setProxyUsernamePassword(null, null);
                        mDlg.cancel();
                    }
                });
        mDlg = builder.create();
        mDlg.show();
    }
    
    private void setupStartupParams(boolean bPeer){
		JSONObject params = mMyApp.getStartupParams();
		if(bPeer){
			PeerCall.instance().setParam(MediaType.Audio, params);
			PeerCall.instance().setParam(MediaType.Video, params);
			PeerCall.instance().setParam(MediaType.Sharing, params);
			int i = 1;
	        for(int viewId : remoteViewIDs) {
	    		WseSurfaceView viewRemote = (WseSurfaceView)findViewById(viewId);
				PeerCall.instance().pushRemoteView(i, viewRemote);
				i++;
	        }
		}
		else{
			Loopback.instance().setParam(MediaType.Audio, params);
			Loopback.instance().setParam(MediaType.Video, params);
			Loopback.instance().setParam(MediaType.Sharing, params);
		}
    }

//    public void UpdateContentLayout()
//    {
//        FrameLayout fl = (FrameLayout)as.getParent();
//        if( fl instanceof  FrameLayout )
//        {
//            int width = fl.getWidth();
//            int height = fl.getHeight();
//        }
//    }

    protected void updateDisplayRotation(){
        Display display = ((WindowManager) getSystemService(WINDOW_SERVICE)).getDefaultDisplay();
        int rotation = display.getRotation();
        if (as != null ) {
            View parentView = (View)as.getParent();
            if( parentView != null) {
                new Handler(Looper.getMainLooper()).postDelayed(new UpdateViewRunnable(), 500);
            }
        }
        WseEngine.setDisplayRotation(rotation);
        Log.i("ClickCall", "updateDisplayRotation,rotation=" + rotation);
    }
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.i("ClickCall", "onConfigurationChanged,orientation="+((newConfig.orientation==Configuration.ORIENTATION_LANDSCAPE)?"ORIENTATION_LANDSCAPE":"ORIENTATION_PORTRAIT"));
        updateDisplayRotation();
    }

    public boolean onTouchEvent(MotionEvent event)
    {
        Log.i("onTouchEvent", "fired");
        final int action = MotionEventCompat.getActionMasked(event);

        if( viewToBeScaled != null ) {
            mScaleGestureDetector.onTouchEvent(event);
        }
        switch (action) {
            case MotionEvent.ACTION_DOWN: {
                final int pointerIndex = MotionEventCompat.getActionIndex(event);
                final float x = MotionEventCompat.getX(event, pointerIndex);
                final float y = MotionEventCompat.getY(event, pointerIndex);

                Rect rc = new Rect();
                local.getHitRect(rc);
                if( rc.contains((int)x, (int)y) )
                {
                    viewToBeMoved = local;
                }
                else {
                    remote.getHitRect(rc);
                    if (rc.contains((int) x, (int) y)) {
                        viewToBeMoved = remote;
                    }
                    else if(as != null){
                        as.getHitRect(rc);
                        if( rc.contains((int) x, (int) y))
                        {
                            viewToBeMoved = as;
                            viewToBeScaled = as;
                        }
                    }
                }
                if( viewToBeMoved != null )
                {
                    mTouchBeginX = x;
                    mTouchBeginY = y;
                    mActivePointerId = MotionEventCompat.getPointerId(event, 0);
                    ViewGroup.LayoutParams layoutParams = viewToBeMoved.getLayoutParams();
                    if( layoutParams instanceof FrameLayout.LayoutParams ) {
                        mLeft = viewToBeMoved.getLeft();
                        mTop = viewToBeMoved.getTop();
                        FrameLayout.LayoutParams params;
                        params = new FrameLayout.LayoutParams(viewToBeMoved.getWidth(), viewToBeMoved.getHeight());
                        params.leftMargin = mLeft;
                        params.topMargin = mTop;
                        viewToBeMoved.setLayoutParams(params);
                    }
                    if( null == viewToBeScaled )
                        return true;
                }
                if( viewToBeScaled != null ) {
                    mScaleGestureDetector.onTouchEvent(event);
                    return true;
                }

                break;
            }

            case MotionEvent.ACTION_MOVE: {
                // Find the index of the active pointer and fetch its position
                if( viewToBeMoved != null ) {
                    final int pointerIndex =
                            MotionEventCompat.findPointerIndex(event, mActivePointerId);

                    final float x = MotionEventCompat.getX(event, pointerIndex);
                    final float y = MotionEventCompat.getY(event, pointerIndex);

                    // Calculate the distance moved
                    final int dx = (int)(x - mTouchBeginX);
                    final int dy = (int)(y - mTouchBeginY);

                    int newLeft = mLeft + dx;
                    int newTop = mTop + dy;
                    ViewGroup.LayoutParams layoutParams = viewToBeMoved.getLayoutParams();
                    if( layoutParams instanceof FrameLayout.LayoutParams ) {
                        FrameLayout.LayoutParams params;
                        params = new FrameLayout.LayoutParams(viewToBeMoved.getWidth(), viewToBeMoved.getHeight());
                        params.leftMargin = newLeft;
                        params.topMargin = newTop;
                        viewToBeMoved.setLayoutParams(params);
                    }
                    return true;
                }
                break;
            }

            case MotionEvent.ACTION_UP: {
                mActivePointerId = MotionEvent.INVALID_POINTER_ID;

                viewToBeMoved = null;
                viewToBeScaled = null;
                break;
            }

            case MotionEvent.ACTION_CANCEL: {
                mActivePointerId = MotionEvent.INVALID_POINTER_ID;

                viewToBeMoved = null;
                viewToBeScaled = null;
                break;
            }

            case MotionEvent.ACTION_POINTER_UP: {

                if( viewToBeMoved != null ) {
                    final int pointerIndex = MotionEventCompat.getActionIndex(event);
                    final int pointerId = MotionEventCompat.getPointerId(event, pointerIndex);

                    if (pointerId == mActivePointerId) {
                        // This was our active pointer going up. Choose a new
                        // active pointer and adjust accordingly.
                        final int newPointerIndex = pointerIndex == 0 ? 1 : 0;
                        mTouchBeginX = MotionEventCompat.getX(event, newPointerIndex);
                        mTouchBeginY = MotionEventCompat.getY(event, newPointerIndex);
                        Rect rc = new Rect();
                        viewToBeMoved.getHitRect(rc);
                        if( rc.contains((int)mTouchBeginX, (int)mTouchBeginY) ) {
                            mLeft = viewToBeMoved.getLeft();
                            mTop = viewToBeMoved.getTop();
                            mActivePointerId = MotionEventCompat.getPointerId(event, newPointerIndex);
//                            local.bringToFront();
//                            remote.bringToFront();

                            return true;
                        }
                        else {
                            mActivePointerId = MotionEvent.INVALID_POINTER_ID;
                            viewToBeMoved = null;
                        }
                    }
                }
                break;
            }
        }
        return super.onTouchEvent(event);
    }

    public void showStatsText(boolean bShow){
        TextView txtStats = (TextView)findViewById(R.id.ForStatistics);
        if(bShow)
            txtStats.setVisibility(View.VISIBLE);
        else
            txtStats.setVisibility(View.INVISIBLE);
    }

    public void onTimer()
    {
        if(m_timerStop) {
            Log.i("CallActivity", "timer has been cancelled.");
            return;
        }
        long dateTime = new Date().getTime();
        if(dateTime > m_dateTime + 1000){
            m_dateTime = dateTime;
        	AudioStatistics audio = null;
        	VideoStatistics video = null;
        	SharingStatistics screen = null;
        	CpuUsage cpu = null;
        	MemoryUsage mem = null;

            if(m_showStats || mMyApp.bPrintStatisticsLog()){
            	if(m_bLoopback){
            		audio = Loopback.instance().getAudioStats();
            		video = Loopback.instance().getVideoStats();
            		screen = Loopback.instance().getSharingStats();
            		cpu = Loopback.instance().getCpuUsage();
            		mem = Loopback.instance().getMemoryUsage();
            	}
            	else{
            		audio = PeerCall.instance().getAudioStats();
            		video = PeerCall.instance().getVideoStats();
            		screen = PeerCall.instance().getSharingStats();
            		cpu = PeerCall.instance().getCpuUsage();
            		mem = PeerCall.instance().getMemoryUsage();
            	}
            	String audioStr = Endpoint.formatStatisticsAudio(audio);
            	String videoStr = Endpoint.formatStatisticsVideo(video);
            	String screenStr = Endpoint.formatStatisticsScreen(screen);
            	Log.i("CallActivity", audioStr);
            	Log.i("CallActivity", videoStr);
            	Log.i("CallActivity", screenStr);
            }
            if(m_showStats){    
                String str = Endpoint.formatStatistics(audio, video,screen, cpu, mem);
                TextView txtStats = (TextView)findViewById(R.id.ForStatistics);
                txtStats.setText(str);
            }
        }

        m_handler.postDelayed(m_runnable, 500);
    }

    protected void stop(){
        m_timerStop = true;
        if(m_bLoopback){
        	Loopback.instance().stopCall();
        }else{
        	PeerCall.instance().stopCall();
        }
        mMyApp.switchToMainActivity();
        mMyApp.resetParam();
    }

    public void onBackPressed(){
        if(m_showStats){
            m_showStats = false;
            showStatsText(false);
        }else{
            stop();
        }
    }
//
//    @Override
//    public void onSurfaceCreated() {
//
//    }
//
//    @Override
//    public void onSurfaceChanged(int width, int height) {
//
//    }
//
//    @Override
//    public void onDrawFrame() {
//
//    }
//
//    @Override
//    public void OnRequestRender(WseSurfaceView wseView, int width, int height) {
//        if( wseView == as )
//        {
//            if( width > 0 && height > 0 )
//            {
//                if( as.getVisibility() != View.VISIBLE )
//                    as.setVisibility(View.VISIBLE);
//                if( width != mASWidth || height != mASHeight )
//                {
//                    mASWidth = width;
//                    mASHeight = height;
//                    new Handler(Looper.getMainLooper()).post(new UpdateViewRunnable());
//                }
//
//            }
//        }
//    }

    private class UpdateViewRunnable implements Runnable
    {
        private int oldWidth = 0;
        private int oldHeight = 0;

        @Override
        public void run() {

            View parentView = (View)as.getParent();
            if( parentView != null )
            {
                int pw = parentView.getWidth();
                int ph = parentView.getHeight();
                if( (pw != oldWidth || ph != oldHeight) && pw > 0 && ph > 0 )
                {
                    oldWidth = pw;
                    oldHeight = ph;
                    if( (float)mASWidth / mASHeight > (float)pw / ph )
                    {
                        int height = (int)(mASHeight * (float)pw/mASWidth);
                        ViewGroup.LayoutParams params = as.getLayoutParams();
                        if( params instanceof FrameLayout.LayoutParams )
                        {
                            FrameLayout.LayoutParams flp = new FrameLayout.LayoutParams(pw,  height);
                            flp.leftMargin = 0;
                            flp.topMargin = (ph - height)/2;
                            as.setLayoutParams(flp);
                        }
                    }
                    else {
                        int width = (int)(mASWidth * (float)ph / mASHeight);
                        ViewGroup.LayoutParams params = as.getLayoutParams();
                        if( params instanceof FrameLayout.LayoutParams )
                        {
                            FrameLayout.LayoutParams flp = new FrameLayout.LayoutParams(width,  ph);
                            flp.leftMargin = (pw - width) / 2;
                            flp.topMargin = 0;
                            as.setLayoutParams(flp);
                        }
                    }
                }
            }
        }
    }
}
