package com.wx2.clickcall;

import java.io.File;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import com.cisco.wme.appshare.ScreenShareContext;
import com.webex.wme.MediaConfig;
import com.webex.wme.NativeMediaSession;
import com.webex.wme.NativeMediaSession.TraceLevelMask;
import com.wx2.control.EditControl;
import com.wx2.control.ToggleControl;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.media.projection.MediaProjectionManager;
import android.os.Bundle;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.*;
import android.util.Log;
import android.os.Handler;

public class MainActivity extends Activity {
    protected MyApplication mMyApp;
    private boolean doubleBackToExitPressedOnce;
    private boolean mbCallerFirst;
    private boolean mbShown=false;
    private EditControl mLinusRow, mWSRow;
    //private boolean bEnableAS;
    private ToggleControl m_btnLoopback, m_btnCalliope, m_btnEnableAS, m_btnEnableScreenHW, m_btnIsSharer, m_btnHWCodec, m_btnActiveVideo,
    	IceEnabled, srtpEnable, dtlsSrtpEnable, multiEnable, qosEnable, m_btnEnableAudio, m_btnEnableVideo, m_btnEnableAaec;
    private ToggleControl cvoEnable;
    
    // Instance States
    private final String STATE_LINUS_EDIT = "STATE_LINUS_EDIT";
    private final String STATE_WS_EDIT = "STATE_WS_EDIT";
    private final String STATE_CALLIOPE_TOGGLE = "STATE_CALLIOPE_TOGGLE";
    private final String STATE_LOOP_TOGGLE = "STATE_LOOP_TOGGLE";
    private final String STATE_AS_TOGGLE = "STATE_AS_TOGGLE";
    private final String STATE_AS_SHARE_HW_TOGGLE = "STATE_AS_SHARE_HW_TOGGLE";
    private final String STATE_HAS_AUDIO_TOGGLE = "STATE_HAS_AUDIO_TOGGLE";
    private final String STATE_HAS_VIDEO_TOGGLE = "STATE_HAS_VIDEO_TOGGLE";
    private final String STATE_AS_SHARE_TOGGLE = "STATE_SHARER_TOGGLE";
    private final String STATE_SRTP_TOGGLE = "STATE_SRTP_TOGGLE";
    private final String STATE_DTLS_SRTP_TOGGLE = "STATE_DTLS_SRTP_TOGGLE";
    private final String STATE_MULTISTREAM_TOGGLE = "STATE_MULTISTREAM_TOGGLE";
    private final String STATE_ICE_TOGGLE = "STATE_ICE_TOGGLE";
    private final String STATE_HWCODEC_TOGGLE = "STATE_HWCODEC_TOGGLE";
    private final String STATE_4VIDEO_TOGGLE = "STATE_4VIDEO_TOGGLE";
    private final String STATE_QOS_TOGGLE = "STATE_QOS_TOGGLE";
    private final String STATE_CVO_TOGGLE = "STATE_CVO_TOGGLE";
    private final String STATE_HAS_AAEC_TOGGLE = "STATE_HAS_AAEC_TOGGLE";

    private final int SCREEN_CAPTURER_PERMISSION_CODE=1;
    private MediaProjectionManager mProjectionManager;
    private int mMediaProjectionResultCode;
    private Intent mMediaProjectionData;
    private ScreenShareContext.OnShareStoppedListener mListener;

    /*
    Fake thread for CPU frequence testing.
     */
    class FakeTask implements Runnable {

        private int nTest = 0;
        @Override
        public void run() {
            while (true) {
                nTest++;
                if(nTest>1000000) {
                    nTest = 1;
                    try {
                        Thread.sleep(10);
                    }catch(Exception e) {
                        Log.e("onClick", " fake thread sleep fail");
                    }
                }
            }
        }

    };
    Vector vecFakeThreads = new Vector();
    private int nFakeThreadCount = 0;

    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mbCallerFirst = false;
        NativeClickCall.doGCov(0);
        //example>> adb shell am start -n com.wx2.clickcall/com.wx2.clickcall.MainActivity -e WME_PERFORMANCE_TEST_720p 1
        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_VIDEO_SIZE"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_VIDEO_SIZE",getIntent().getStringExtra("WME_PERFORMANCE_TEST_VIDEO_SIZE"));

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_Skip_CWseVideoListenChannel_Render"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_Skip_CWseVideoListenChannel_Render","1");

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_Skip_SendRTPPacket"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_Skip_SendRTPPacket","1");

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_ReceiveRTP_Skip"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_ReceiveRTP_Skip",getIntent().getStringExtra("WME_PERFORMANCE_TEST_ReceiveRTP_Skip"));

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_skip_CWbxAeRecordChannel::SendPacket"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_skip_CWbxAeRecordChannel::SendPacket","1");

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_skip_CWbxAeEncoderThread::PushPacket"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_skip_CWbxAeEncoderThread::PushPacket","1");

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_skip_CWbxAudioEngineImpl::ReceivedRTPPacket"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_skip_CWbxAudioEngineImpl::ReceivedRTPPacket","1");

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_Skip_SmoothSendBuffer_InputData"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_Skip_SmoothSendBuffer_InputData","1");

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_Capture_Limit_Fps"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_Capture_Limit_Fps",getIntent().getStringExtra("WME_PERFORMANCE_TEST_Capture_Limit_Fps"));

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_Skip_CWseVideoEncoder_OnOutputData"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_Skip_CWseVideoEncoder_OnOutputData","1");

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_source_channel_disable_AV_Sync"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_source_channel_disable_AV_Sync","1");

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_Skip_CWseVideoListenChannel_DecodeNoRender"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_Skip_CWseVideoListenChannel_DecodeNoRender","1");

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_Skip_CWseVideoListenChannel_Render"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_Skip_CWseVideoListenChannel_Render","1");

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_source_channel_skip_OnRecvMediaData"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_source_channel_skip_OnRecvMediaData","1");

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_source_channel_skip_DeliverImage"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_source_channel_skip_DeliverImage","1");

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_source_channel_skip_DeliverImage"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_source_channel_skip_DeliverImage","1");
        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_Skip_CWseRtpPacker_OnOutputData"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_Skip_CWseRtpPacker_OnOutputData","1");


        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_Encode_Thread_Number"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_Encode_Thread_Number",getIntent().getStringExtra("WME_PERFORMANCE_TEST_Encode_Thread_Number"));
        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_source_channel_capture_thread_low"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_source_channel_capture_thread_low","1");

        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_Encode_Slice_Number"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_Encode_Slice_Number",getIntent().getStringExtra("WME_PERFORMANCE_TEST_Encode_Slice_Number"));
        if (getIntent().hasExtra("WME_PERFORMANCE_TEST_source_channel_decode_thread_high"))
            NativeClickCall.SetEnv("WME_PERFORMANCE_TEST_source_channel_decode_thread_high","1");

        long nVideoDumpFlag = 0;
        if(getIntent().hasExtra("video-dump-file-flag"))
            nVideoDumpFlag = Integer.parseInt(getIntent().getStringExtra("video-dump-file-flag"));


        if(getIntent().hasExtra("fake-thread-num"))
            nFakeThreadCount = Integer.parseInt(getIntent().getStringExtra("fake-thread-num"));


        boolean bAutoStart = getIntent().hasExtra("auto-start");
        boolean bLoopBack = getIntent().hasExtra("loopback");
        mbCallerFirst = getIntent().hasExtra("callerfirst");
        boolean bCalliope = getIntent().hasExtra("calliope");
        boolean bAppshare = getIntent().hasExtra("appshare");
        boolean bSharer = getIntent().hasExtra("sharer");
        boolean bHardWare = getIntent().hasExtra("video-hardware");
        boolean bScreenHardWare = getIntent().hasExtra("screen-hardware");
        boolean bDisableQos = getIntent().hasExtra("disable-qos");
        boolean bDisableAudio = getIntent().hasExtra("no-audio");
        boolean bDisableVideo = getIntent().hasExtra("no-video");
        boolean bDisableICE = getIntent().hasExtra("no-ice");
        boolean bEnableCVO = getIntent().hasExtra("enable-cvo");
        boolean bEnableAAEC = getIntent().hasExtra("enable-aaec");
        boolean bEnableMultiStream = true;
        if(getIntent().hasExtra("enable-multi-stream"))
            bEnableMultiStream = (Integer.parseInt(getIntent().getStringExtra("enable-multi-stream"))==1);
        boolean bEnableSimulcast = true;
        if(getIntent().hasExtra("enable-simulcast"))
            bEnableSimulcast = (Integer.parseInt(getIntent().getStringExtra("enable-simulcast"))==1);

        if(getIntent().hasExtra("ice-policy")) {
            int nIcePolicy = 3;
            nIcePolicy = Integer.parseInt(getIntent().getStringExtra("ice-policy"));
            Log.i("ClickCall", "icepolicy=" + nIcePolicy);
            MyApplication.m_nIcePolicy = MediaConfig.WmeConnectionPolicy.of(nIcePolicy);
        }

        String sRoom = "1217";
        if (getIntent().hasExtra("room"))
            sRoom = getIntent().getStringExtra("room");

        String sLinus = "10.224.166.110:5000";
        if (getIntent().hasExtra("Linus")) {
            sLinus = getIntent().getStringExtra("Linus");
            if(sLinus.equals("null"))
                sLinus = "";
        }

        if(getIntent().hasExtra("uid"))
            MyApplication.USER_ID = getIntent().getStringExtra("uid");
        if(getIntent().hasExtra("passwd"))
            MyApplication.USER_PASSWORD = getIntent().getStringExtra("passwd");

        if(getIntent().hasExtra("wme-trace-level"))
            NativeMediaSession.setTraceMask(TraceLevelMask.values()[Integer.parseInt(getIntent().getStringExtra("wme-trace-level"))]);
        else
            NativeMediaSession.setTraceMask(TraceLevelMask.TRACE_LEVEL_MASK_INFO);

        setContentView(R.layout.main);
        mMyApp = (MyApplication)this.getApplicationContext();
        mMyApp.setCurrentActivity(this);
        mMyApp.m_nVideoDumpFlag = nVideoDumpFlag;
        mMyApp.m_bEnableSimulcast = bEnableSimulcast;
        mMyApp.m_bEnableCVO = bEnableCVO;
        doubleBackToExitPressedOnce = false;
        mMyApp.m_bHasAudio = !bDisableAudio;
        mMyApp.m_bHasVideo = !bDisableVideo;
        mMyApp.m_bEnableAaec = bEnableAAEC;
        mMyApp.m_bScreenHWCodec = bScreenHardWare;
        NativeMediaSession.SetContext(this);

        m_btnEnableScreenHW = (ToggleControl)findViewById(R.id.enableHWScreenVideo);
        m_btnEnableScreenHW.setChecked(bScreenHardWare);
        m_btnEnableAudio = (ToggleControl)findViewById(R.id.enableAudio);
        m_btnEnableAudio.setChecked(!bDisableAudio);

        m_btnEnableAaec = (ToggleControl)findViewById(R.id.enableAaec);
        m_btnEnableAaec.setChecked(bEnableAAEC);

        m_btnEnableVideo = (ToggleControl)findViewById(R.id.enableVideo);
        m_btnEnableVideo.setChecked(!bDisableVideo);
        IceEnabled = (ToggleControl)findViewById(R.id.enableICE);
        IceEnabled.setChecked(!bDisableICE);
        srtpEnable = (ToggleControl)findViewById(R.id.enableSRTP);
        dtlsSrtpEnable = (ToggleControl)findViewById(R.id.enableDtlsSRTP);
        multiEnable = (ToggleControl)findViewById(R.id.enableMulti);
        multiEnable.setChecked(bEnableMultiStream);
        qosEnable = (ToggleControl)findViewById(R.id.enableQoS);
        qosEnable.setChecked(!bDisableQos);
        cvoEnable = (ToggleControl)findViewById(R.id.enableCVO);
        cvoEnable.setChecked(bEnableCVO);
        m_btnLoopback = (ToggleControl)findViewById(R.id.isLoopback);
        m_btnLoopback.setChecked(bLoopBack);
        m_btnCalliope = (ToggleControl)findViewById(R.id.isCalliope);
        m_btnCalliope.setChecked(bCalliope);
        m_btnEnableAS = (ToggleControl)findViewById(R.id.enableAS);
        m_btnEnableAS.setChecked(bAppshare);
        m_btnIsSharer = (ToggleControl)findViewById(R.id.enableASSharer);
        m_btnIsSharer.setChecked(bSharer);
        m_btnIsSharer.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                if(b && mbShown){
                    initSharingContext();
                }
            }
            });
        m_btnHWCodec = (ToggleControl)findViewById(R.id.enableHWVideo);
        m_btnHWCodec.setChecked(bHardWare);
        m_btnActiveVideo = (ToggleControl)findViewById(R.id.enableFilmStrip);
        mLinusRow = (EditControl)findViewById(R.id.linus_row);
        mWSRow = (EditControl)findViewById(R.id.ws_row);
        mLinusRow.setText(sLinus);
        mWSRow.setText(sRoom);
        mLinusRow.setVisibility(bCalliope ? View.VISIBLE : View.INVISIBLE);
        mWSRow.setVisibility(!bLoopBack ? View.VISIBLE : View.INVISIBLE);

        Button btnStartJava = (Button) findViewById(R.id.StartButtonJava);
        btnStartJava.setOnClickListener(new OnClickListener() {
            public void onClick(View v) {
            	String sVersion = NativeMediaSession.getVersion();
            	Log.v("mediasession", "onCreate, version=" + sVersion);

            	File theFile = new File(getFilesDir().getPath());
            	String path = theFile.getParent();
            	path += File.separator + "lib";
            	//path = "/data/data/com.wx2.clickcall/lib";
                try {
                	NativeMediaSession.class.getMethod("setLibPath",String.class).invoke(null, path);                	                 
                } catch(Exception e) {
                	Log.e("onClick", " can't find method setLibPath in object NativeMediaSession");
                }
                
                PeerCall.instance().enableIce(IceEnabled.isChecked());
                Loopback.instance().enableIce(IceEnabled.isChecked());
                Loopback.instance().mbCallerFirst = mbCallerFirst;
                PeerCall.instance().enableSrtp(srtpEnable.isChecked());
                PeerCall.instance().enableDtlsSrtp(dtlsSrtpEnable.isChecked());
                
                mMyApp.mEnableCMulti = multiEnable.isChecked();
                PeerCall.instance().enableMulti(multiEnable.isChecked());
                Loopback.instance().enableMulti(multiEnable.isChecked());

                PeerCall.instance().enableQos(qosEnable.isChecked());
                Loopback.instance().enableQos(qosEnable.isChecked());
                PeerCall.instance().setWmeDataDump(8);
                Loopback.instance().setWmeDataDump(8);

                MainActivity.this.startJava();
            }
        });

        m_btnLoopback.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                mWSRow.setVisibility(b ? View.INVISIBLE : View.VISIBLE);
            }
        });
        m_btnCalliope.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            public void onCheckedChanged(CompoundButton compoundButton, boolean b) {
                mLinusRow.setVisibility(b ? View.VISIBLE : View.INVISIBLE);
            }
        });
        findViewById(R.id.StartButtonJava).requestFocus();
        
     // Handle MyApplication, switchToMainActivity()
        if (savedInstanceState == null && mMyApp.mainActivityExtras != null) {
        	mLinusRow.setText(mMyApp.mainActivityExtras.getString(STATE_LINUS_EDIT));
        	mWSRow.setText(mMyApp.mainActivityExtras.getString(STATE_WS_EDIT));
        	m_btnCalliope.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_CALLIOPE_TOGGLE));
        	m_btnLoopback.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_LOOP_TOGGLE));
        	m_btnEnableAS.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_AS_TOGGLE));
            m_btnIsSharer.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_AS_SHARE_TOGGLE));
        	m_btnHWCodec.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_HWCODEC_TOGGLE));
        	m_btnActiveVideo.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_4VIDEO_TOGGLE));
        	srtpEnable.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_SRTP_TOGGLE));
            dtlsSrtpEnable.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_DTLS_SRTP_TOGGLE));
        	multiEnable.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_MULTISTREAM_TOGGLE));
        	IceEnabled.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_ICE_TOGGLE));
        	qosEnable.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_QOS_TOGGLE));
        	cvoEnable.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_CVO_TOGGLE));
            m_btnEnableScreenHW.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_AS_SHARE_HW_TOGGLE));
            m_btnEnableAudio.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_HAS_AUDIO_TOGGLE));
            m_btnEnableVideo.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_HAS_VIDEO_TOGGLE));
            m_btnEnableAaec.setChecked(mMyApp.mainActivityExtras.getBoolean(STATE_HAS_AAEC_TOGGLE));
        }

        if(bAutoStart){
            messageHandler.sendEmptyMessageDelayed(1, 1000);
        }
    }

    @Override
    protected void onResume () {
        super.onResume();
        if(m_btnIsSharer.isChecked()) {
            initSharingContext();
        }
        mbShown = true;
    }

    protected void initSharingContext () {
        if(mProjectionManager==null){
            mProjectionManager =
                    (MediaProjectionManager) getSystemService(Context.MEDIA_PROJECTION_SERVICE);
        }
        if(!mMyApp.mInitSharingContext) {
            startActivityForResult(mProjectionManager.createScreenCaptureIntent(), SCREEN_CAPTURER_PERMISSION_CODE);
        }
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        if (requestCode != SCREEN_CAPTURER_PERMISSION_CODE) {
            Log.e("ClickCall", "onActivityResult Unknown request code: " + requestCode);
            return;
        }

        if (resultCode != RESULT_OK) {
            Log.w("ClickCall", "User denied screen sharing permission" + requestCode);
            m_btnIsSharer.setChecked(false);
            return;
        }
        mMediaProjectionResultCode = resultCode;
        mMediaProjectionData = data;

        initScreenShareContext();
    }
    private void initScreenShareContext(){
        if(ScreenShareContext.getInstance().init(this, mMediaProjectionResultCode, mMediaProjectionData)){
            if(mListener==null) {
                mListener = new ScreenShareContext.OnShareStoppedListener() {
                    @Override
                    public void onShareStopped() {
                        mMyApp.mInitSharingContext = false;
                    }
                };
            }
            ScreenShareContext.getInstance().registerCallback(mListener);
            mMyApp.mInitSharingContext = true;
        }
        else {
            mMyApp.mInitSharingContext = false;
        }
    }
    private Handler messageHandler = new Handler() {
        public void handleMessage(Message msg) {
            for(int i=0;i<nFakeThreadCount;i++){
                Thread threadFake = new Thread(new FakeTask());
                threadFake.setName("FakeThread_"+i);
                threadFake.start();
                vecFakeThreads.add(threadFake);
            }
            Button btnStartJava = (Button) findViewById(R.id.StartButtonJava);
            btnStartJava.callOnClick();
        }
    };
    public void startJava(){
    	IPAddressValidator addrValidator = new IPAddressValidator();
    	String wsAddr = mWSRow.getText();
    	if(addrValidator.validate(wsAddr)){
    		Log.i("MediaSession", "startJava: " + wsAddr);
    	}else{
    		Log.i("MediaSession", "startJava, not ip address included: " + wsAddr);
    		wsAddr = "10.224.166.110:8080/?r=" + wsAddr;
    	}
    	mMyApp.m_bHWCodec = m_btnHWCodec.isChecked();
        mMyApp.m_bEnableCVO = cvoEnable.isChecked();
    	mMyApp.m_uMaxVideoStreams = m_btnActiveVideo.isChecked() ? 4 : 1;
    	mMyApp.m_uMaxAudioStreams = mMyApp.m_uMaxVideoStreams > 1 ? 3 : 1;
        mMyApp.m_bHasShare = m_btnEnableAS.isChecked();
        mMyApp.mIsViewer = !m_btnIsSharer.isChecked();
        mMyApp.m_bScreenHWCodec = m_btnEnableScreenHW.isChecked();
        mMyApp.m_bHasVideo = m_btnEnableVideo.isChecked();
        mMyApp.m_bHasAudio = m_btnEnableAudio.isChecked();
        mMyApp.m_bEnableAaec = m_btnEnableAaec.isChecked();

        if(m_btnIsSharer.isChecked() && !mMyApp.mInitSharingContext) {
            initScreenShareContext();
        }
        mMyApp.switchToCallActivity(m_btnLoopback.isChecked(), mLinusRow.getText(),
        		m_btnCalliope.isChecked(), wsAddr, false, m_btnEnableAS.isChecked());
    }

    public void onBackPressed() {
        for(int i=0;i<vecFakeThreads.size();i++){
            Thread threadFake = (Thread)vecFakeThreads.get(i);
            threadFake.stop();
        }
        vecFakeThreads.clear();

        if (doubleBackToExitPressedOnce) {
            Log.i("MainActivity", "should exit the process");
            Intent homeIntent = new Intent(Intent.ACTION_MAIN);
            homeIntent.addCategory(Intent.CATEGORY_HOME);
            homeIntent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP);
            startActivity(homeIntent);
            //Need not to unit here, otherwise, it will not recover because dll will not be reloaded.
            //NativeMediaSession.unInitWME();

            return;
        }

        this.doubleBackToExitPressedOnce = true;
        Toast.makeText(this, "Please click BACK again to exit", Toast.LENGTH_SHORT).show();

        new Handler().postDelayed(new Runnable() {
            public void run() {
                doubleBackToExitPressedOnce=false;
            }
        }, 2000);
    }

    @Override
    protected void onSaveInstanceState(Bundle savedState) {
        savedState.putString(STATE_LINUS_EDIT, mLinusRow.getText());
        savedState.putString(STATE_WS_EDIT, mWSRow.getText());
        savedState.putBoolean(STATE_CALLIOPE_TOGGLE, m_btnCalliope.isChecked());
    	savedState.putBoolean(STATE_LOOP_TOGGLE,  m_btnLoopback.isChecked());   	
    	savedState.putBoolean(STATE_AS_TOGGLE,  m_btnEnableAS.isChecked());
        savedState.putBoolean(STATE_AS_SHARE_TOGGLE, m_btnIsSharer.isChecked());
    	savedState.putBoolean(STATE_HWCODEC_TOGGLE,  m_btnHWCodec.isChecked());
    	savedState.putBoolean(STATE_4VIDEO_TOGGLE,  m_btnActiveVideo.isChecked()); 	
    	savedState.putBoolean(STATE_SRTP_TOGGLE,  srtpEnable.isChecked());
        savedState.putBoolean(STATE_DTLS_SRTP_TOGGLE,  dtlsSrtpEnable.isChecked());
    	savedState.putBoolean(STATE_MULTISTREAM_TOGGLE,  multiEnable.isChecked());
    	savedState.putBoolean(STATE_ICE_TOGGLE,  IceEnabled.isChecked());
    	savedState.putBoolean(STATE_QOS_TOGGLE,  qosEnable.isChecked());
    	savedState.putBoolean(STATE_CVO_TOGGLE,  cvoEnable.isChecked());
        savedState.putBoolean(STATE_AS_SHARE_HW_TOGGLE, m_btnEnableScreenHW.isChecked());
        savedState.putBoolean(STATE_HAS_AUDIO_TOGGLE,  m_btnEnableAudio.isChecked());
        savedState.putBoolean(STATE_HAS_VIDEO_TOGGLE,  m_btnEnableVideo.isChecked());
        savedState.putBoolean(STATE_HAS_AAEC_TOGGLE,  m_btnEnableAaec.isChecked());

    	mMyApp.mainActivityExtras = savedState;
    	super.onSaveInstanceState(savedState);
    }
    
    @Override
    protected void onRestoreInstanceState(Bundle savedState) {
    	super.onRestoreInstanceState(savedState);
    	mLinusRow.setText(savedState.getString(STATE_LINUS_EDIT));
    	mWSRow.setText(savedState.getString(STATE_WS_EDIT));
    	m_btnCalliope.setChecked(savedState.getBoolean(STATE_CALLIOPE_TOGGLE));
    	m_btnLoopback.setChecked(savedState.getBoolean(STATE_LOOP_TOGGLE));
    	m_btnEnableAS.setChecked(savedState.getBoolean(STATE_AS_TOGGLE));
        m_btnIsSharer.setChecked(savedState.getBoolean(STATE_AS_SHARE_TOGGLE));
    	m_btnHWCodec.setChecked(savedState.getBoolean(STATE_HWCODEC_TOGGLE));
    	m_btnActiveVideo.setChecked(savedState.getBoolean(STATE_4VIDEO_TOGGLE));
    	srtpEnable.setChecked(savedState.getBoolean(STATE_SRTP_TOGGLE));
        dtlsSrtpEnable.setChecked(savedState.getBoolean(STATE_DTLS_SRTP_TOGGLE));
    	multiEnable.setChecked(savedState.getBoolean(STATE_MULTISTREAM_TOGGLE));
    	IceEnabled.setChecked(savedState.getBoolean(STATE_ICE_TOGGLE));
    	qosEnable.setChecked(savedState.getBoolean(STATE_QOS_TOGGLE));
    	cvoEnable.setChecked(savedState.getBoolean(STATE_CVO_TOGGLE));
        m_btnEnableScreenHW.setChecked(savedState.getBoolean(STATE_AS_SHARE_HW_TOGGLE));
        m_btnEnableAudio.setChecked(savedState.getBoolean(STATE_HAS_AUDIO_TOGGLE));
        m_btnEnableVideo.setChecked(savedState.getBoolean(STATE_HAS_VIDEO_TOGGLE));
    }

	public class IPAddressValidator{
		 
	    private Pattern pattern;
	    private Matcher matcher;
	 
	    private static final String IPADDRESS_PATTERN = 
			"^([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\." +
			"([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\." +
			"([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\." +
			"([01]?\\d\\d?|2[0-4]\\d|25[0-5])$";
	 
	    public IPAddressValidator(){
		  pattern = Pattern.compile(IPADDRESS_PATTERN);
	    }
	 
	   /**
	    * Validate ip address with regular expression
	    * @param ip ip address for validation
	    * @return true valid ip address, false invalid ip address
	    */
	    public boolean validate(final String ip){		  
		  matcher = pattern.matcher(ip);
		  return matcher.matches();	    	    
	    }

	}
}

