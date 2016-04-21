package com.cisco.wmeAndroid;

import com.cisco.webex.wme.WmeClient;
import com.cisco.webex.wme.WmeNative;
import com.cisco.webex.wme.WmeParameters;
import com.cisco.wmeAndroid.R;
import com.webex.wseclient.WseEngine;

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Display;
import android.view.Window;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.LinearLayout;
import android.widget.LinearLayout.LayoutParams;
import android.widget.Spinner;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.TextView;
import android.widget.Toast;

public class PlayPanelActivity extends Activity implements CompoundButton.OnCheckedChangeListener {

	private TextView m_tvStat;
	private CheckBox m_checkAudio;
	private CheckBox m_checkVideo;
    private CheckBox m_checkShare;
	private CheckBox m_checkMute;
	private CheckBox m_checkSpeaker;
	private CheckBox m_checkSwitch;
	private com.webex.wseclient.WseGLView mLocalGLView;
	private com.webex.wseclient.WseGLView mRemoteGLView;
    private com.webex.wseclient.WseGLView mRemoteShareGLView;
	//private Button m_switchCamera;
	private boolean m_bUISpeaker;
	private boolean m_bUIMute;
	private boolean m_bUIAudio;
	private boolean m_bUIVideo;
    private boolean m_bUIShare;
	private boolean m_bUIBackCam;
	private int m_iCamIdx;
	private int m_iCodecParamIndex;
	private int m_iVideoQualityIndex;
	private int m_iCameraParamIdx;
	private boolean m_bVideoFileRender;
	private boolean m_bAudioFileRender;
	private Handler m_MainThreadHandler = null;
	private TextView m_tvRenderInfo;
	
	private class MySaveObject {
		public boolean _bUISpeaker;
		public boolean _bUIMute;
		public boolean _bUIAudio;
		public boolean _bUIVideo;
        public boolean _bUIShare;
		public boolean _bUIBackCam;
		public int _iCodecParamIndex;
		public int _iVideoQualityIndex;
		public boolean _bVideoFileRender;
		public boolean _bAudioFileRender;
	}
	
	public PlayPanelActivity()
	{
		Log.i("wme_android", "PlayPanelActivity constructor");
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);

		//���ر�����
		this.requestWindowFeature(Window.FEATURE_NO_TITLE);
		//����״̬��
		this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		//for orientation change(restore UI here)
		final MySaveObject data = (MySaveObject) getLastNonConfigurationInstance();
	    if (data != null) {
	        loadData(data);
	    }
	    else{
	    	//init values
			m_bUIAudio = true;
			m_bUIVideo = true;
            m_bUIShare = true;
			m_bUISpeaker = true;
			m_bUIMute = false;
			m_iCamIdx = getIntent().getIntExtra(Constants.CAMERA_INDEX, 0);
			m_bUIBackCam = (m_iCamIdx == 0) ? true : false;
	    }

		Boolean pip = getIntent().getBooleanExtra(Constants.PICTURE_IN_PICTURE, false);
		if (pip == true)
		{
			setContentView(R.layout.meeting_pip);
		}
		else
		{
			setContentView(R.layout.meeting);
		}
		
		m_iCodecParamIndex = getIntent().getIntExtra(Constants.CODEC_PARAM_INDEX, 0);
		m_iVideoQualityIndex = getIntent().getIntExtra(Constants.VIDEO_QUALITY_INDEX, 0);
		m_iCameraParamIdx = getIntent().getIntExtra(Constants.CAMERA_PARAM_INDEX, 0);
		m_bVideoFileRender = getIntent().getBooleanExtra(Constants.ENABLE_VIDEO_FILE_RENDER, false);
		m_bAudioFileRender = getIntent().getBooleanExtra(Constants.ENABLE_AUDIO_FILE_RENDER, false);

		//m_switchCamera = (Button)findViewById(R.id.btSwitchCamera);
		//m_switchCamera.setEnabled(false);	//disable first
		m_checkMute = (CheckBox)findViewById(R.id.checkBoxMute);
		m_checkAudio = (CheckBox)findViewById(R.id.checkBoxAudio);
		m_checkVideo = (CheckBox)findViewById(R.id.checkBoxVideo);
        m_checkShare = (CheckBox)findViewById(R.id.checkBoxShare);
		m_checkSpeaker = (CheckBox)findViewById(R.id.checkBoxSpeaker);
		m_checkSwitch = (CheckBox)findViewById(R.id.checkBoxSwitch);
		m_tvStat = (TextView)findViewById(R.id.Statistic);
		m_tvRenderInfo = (TextView)findViewById(R.id.RenderInfo);
			
	    m_checkSpeaker.setChecked(m_bUISpeaker);
	    m_checkMute.setChecked(m_bUIMute);
	    m_checkAudio.setChecked(m_bUIAudio);
	    m_checkVideo.setChecked(m_bUIVideo);
        m_checkShare.setChecked(m_bUIShare);
	    m_checkSwitch.setChecked(m_bUIBackCam);
	    
		mLocalGLView = (com.webex.wseclient.WseGLView)findViewById(R.id.LocalVideoGLView);
		mRemoteGLView = (com.webex.wseclient.WseGLView)findViewById(R.id.RemoteVideoGLView);
		mRemoteShareGLView = (com.webex.wseclient.WseGLView)findViewById(R.id.RemoteShareGLView);
		WindowManager windowManager = getWindowManager();  
        Display display = windowManager.getDefaultDisplay();  
        int screenRotation = display.getRotation(); 
        WseEngine.setDisplayRotation(screenRotation);
        Log.i("wme_android","PlayPanelActivity::onCreate, screenRotation = "+screenRotation);
		
        m_checkSpeaker.setOnCheckedChangeListener(this);
		m_checkVideo.setOnCheckedChangeListener(this);
        m_checkShare.setOnCheckedChangeListener(this);
		m_checkAudio.setOnCheckedChangeListener(this);
		m_checkMute.setOnCheckedChangeListener(this);
		m_checkSwitch.setOnCheckedChangeListener(this);		
		
		boolean audioenable = WmeClient.instance().IsEnableAudio();
		m_checkSpeaker.setEnabled(audioenable);
		m_checkMute.setEnabled(audioenable);
		m_checkAudio.setEnabled(audioenable);
		
		boolean videoenable = WmeClient.instance().IsEnableVideo();
		m_checkVideo.setEnabled(videoenable);
		m_checkSwitch.setEnabled(videoenable);
        mLocalGLView.setEnabled(videoenable);
        mRemoteGLView.setEnabled(videoenable);
        if( videoenable ) {
            mLocalGLView.setVisibility(View.VISIBLE);
            mRemoteGLView.setVisibility(View.VISIBLE);
        }
        else {
            mLocalGLView.setVisibility(View.INVISIBLE);
            mRemoteGLView.setVisibility(View.INVISIBLE);
        }

        mRemoteShareGLView.setEnabled(true);

		if (m_tvRenderInfo != null)
		{
			String txt = "";
			if(m_bVideoFileRender == true)
			{
				txt += "video file render \n";
			}
			if(m_bAudioFileRender == true)
			{
				txt += "audio file render \n";
			}
			m_tvRenderInfo.setText(txt);
		}
		
		m_MainThreadHandler = new Handler(){
			@Override
			public void handleMessage(Message msg){					
				
				switch (msg.what) {
					case Constants.EVENT_STATISTIC_REFRESH:
					{
						if (m_tvStat != null)
						{
						    WmeClient.instance().GetSessionStatistic();
							WmeClient.instance().GetVideoStatistic();
							
							String content_capture = "video capture: \n";
							content_capture += " width:";
							content_capture += Integer.toString(StatisticData.videoCaptureWidth);
							content_capture += "\n";
							content_capture += " height:";
							content_capture += Integer.toString(StatisticData.videoCaptureHeight);
							content_capture += "\n";
							content_capture += " fps:";
							content_capture += Integer.toString(StatisticData.videoCaptureFPS);
							content_capture += "\n";
							
							String content_encode = "video encode: \n";
							content_encode += " width:";
							content_encode += Integer.toString(StatisticData.videoEncodeMaxWidth);
							content_encode += "\n";
							content_encode += " height:";
							content_encode += Integer.toString(StatisticData.videoEncodeMaxHeight);
							content_encode += "\n";
							content_encode += " fps:";
							content_encode += Integer.toString(StatisticData.videoEncodeMaxFPS);
							content_encode += "\n";
							
							String content_decode = "video decode: \n";
							content_decode += " width:";
							content_decode += Integer.toString(StatisticData.videoDecodeWidth);
							content_decode += "\n";
							content_decode += " height:";
							content_decode += Integer.toString(StatisticData.videoDecodeHeight);
							content_decode += "\n";
							content_decode += " fps:";
							content_decode += Integer.toString(StatisticData.videoDecodeFPS);
							content_decode += "\n";
							
							m_tvStat.setText(content_capture+content_encode+content_decode);
							sendEmptyMessageDelayed(Constants.EVENT_STATISTIC_REFRESH, Constants.STATISTIC_INTERVAL);
						}
						
					}
					break;
					
					default:
					break;
				}
			}
        };
		
		Log.i("wme_android","PlayPanelActivity::onCreate ");
	}
	
	//for orientation change(remember UI here)
	@Override
	public Object onRetainNonConfigurationInstance() {
	    final MySaveObject data = collectMyLoadedData();
	    return data;
	}
	
	private MySaveObject collectMyLoadedData()
	{
		MySaveObject data = new MySaveObject();
		data._bUISpeaker = m_bUISpeaker;
		data._bUIMute = m_bUIMute;
		data._bUIVideo = m_bUIVideo;
        data._bUIShare = m_bUIShare;
		data._bUIAudio = m_bUIAudio;
		data._bUIBackCam = m_bUIBackCam;
		data._iCodecParamIndex = m_iCodecParamIndex;
		data._iVideoQualityIndex = m_iVideoQualityIndex;
		data._bVideoFileRender = m_bVideoFileRender;
		data._bAudioFileRender = m_bAudioFileRender;
		return data;
	}
	
	private void loadData(MySaveObject data)
	{
		m_bUISpeaker = data._bUISpeaker;
		m_bUIMute = data._bUIMute;
		m_bUIAudio = data._bUIAudio;
		m_bUIVideo = data._bUIVideo;
        m_bUIShare = data._bUIVideo;
		m_bUIBackCam = data._bUIBackCam;
		m_iCodecParamIndex = data._iCodecParamIndex;
		m_iVideoQualityIndex = data._iVideoQualityIndex;
		m_bVideoFileRender = data._bVideoFileRender;
		m_bAudioFileRender = data._bAudioFileRender;
	}
	
	private void stopAV()
	{
		stopVideo();
		stopAudio();
        stopShareReceiver();
	}
	
	private void stopVideo()
	{
		WmeClient.instance().StopVideoTrack();
		WmeClient.instance().StopVideoSending();
		WmeClient.instance().SetLocalWindow(null);
		WmeClient.instance().SetRemoteWindow(null);
		Log.i("wme_android","PlayPanelActivity::stopAV, StopVideoTrack");
	}
	
	private void stopAudio()
	{
		WmeClient.instance().StopAudioTrack();
		WmeClient.instance().StopAudioSending();
		Log.i("wme_android","PlayPanelActivity::stopAV, StopAudioTrack");
	}

    private void  stopShareReceiver()
    {
        WmeClient.instance().SetRemoteShareWindow(null);
        WmeClient.instance().StopRemoteShareTrack();
        Log.i("wme_android","PlayPanelActivity::stopAV, stopShareReceiver");
    }
	
	private void startAV()
	{
		startAudio();
		startVideo();
        startShareReceiver();
	}
	
	private void startAudio()
	{
		if (m_bUIAudio == true)
		{
			WmeClient.instance().StartAudioTrack();
			WmeClient.instance().StartAudioSending();
		}
	}
		
	private void startVideo()
	{
		if (m_bUIVideo == true)
		{
			WmeClient.instance().SetLocalWindow(mLocalGLView);
			if (m_bVideoFileRender == false) {
                WmeClient.instance().SetRemoteWindow(null);
                WmeClient.instance().SetRemoteWindow(m_bUIShare?mRemoteGLView:mRemoteShareGLView);
                mRemoteGLView.setZOrderOnTop(true);
            }
			else
				WmeClient.instance().SetRemoteWindow(null);
			WmeClient.instance().StartVideoTrack();
			WmeClient.instance().StartVideoSending();
		}
	}

    private void startShareReceiver()
    {
        if( m_bUIShare ) {
            WmeClient wmeClient = WmeClient.instance();
            if( m_bUIVideo && m_bVideoFileRender == false ) {
                WmeClient.instance().SetRemoteWindow(null);
                WmeClient.instance().SetRemoteWindow(m_bUIShare ? mRemoteGLView : mRemoteShareGLView);
            }
            wmeClient.SetRemoteShareWindow(mRemoteShareGLView);
            wmeClient.StartRemoteShareTrack();
        }
    }
	@Override
	public void onPause()
	{
		Log.i("wme_android","PlayPanelActivity::onPause ");
		
		if (m_tvStat != null && m_MainThreadHandler != null)
		{
			m_MainThreadHandler.removeMessages(Constants.EVENT_STATISTIC_REFRESH);
		}
		
		stopAV();
		
		Message msg = new Message();
		msg.what = Constants.EVENT_STOP_CALL;
		SettingActivity.m_MessageHandler.sendMessage(msg);
		
		if (mLocalGLView != null)
		{
			mLocalGLView.onPause();
		}
		if (mRemoteGLView != null)
		{
			mRemoteGLView.onPause();
		}
		if( mRemoteShareGLView != null )
            mRemoteShareGLView.onPause();

		super.onPause();
	}

	@Override
	public void onResume() {
		Log.i("wme_android","PlayPanelActivity::onResume ");
	
		if (mLocalGLView != null)
		{
			mLocalGLView.onResume();
			mLocalGLView.setZOrderOnTop(true);
		}
		if (mRemoteGLView != null)
		{
			mRemoteGLView.onResume();
            mRemoteGLView.setZOrderOnTop(true);
		}
		if( mRemoteShareGLView != null )
            mRemoteShareGLView.onResume();

		WmeClient.instance().SetLocalVideoCapability(m_iCodecParamIndex);
		WmeClient.instance().SetVideoQuality(m_iVideoQualityIndex);
		
		m_iCamIdx = (m_bUIBackCam == true ? 0 : 1);
		WmeClient.instance().SwitchCamera(WmeParameters.WME_LOCAL_TRACK, m_iCamIdx);
		
		WmeClient.instance().SetVideoCaptureParam(WmeParameters.WME_LOCAL_TRACK, m_iCamIdx, m_iCameraParamIdx);
			
		if(m_bUISpeaker == true){ 
			WmeClient.instance().SetAudioOutType(WmeParameters.WME_AUDIO_OUT_SPEAKER);
		}else{ 
			WmeClient.instance().SetAudioOutType(WmeParameters.WME_AUDIO_OUT_VOICE);
		} 
		WmeClient.instance().SetSpeakerVolume(Constants.DEFAULT_SPEAKER_VOLUME);
		
		WmeClient.instance().SetMicState(m_bUIMute);
		WmeClient.instance().SetMicVolume(Constants.DEFAULT_MIC_VOLUME);
		
		startAV();
		
		if (m_tvStat != null && m_MainThreadHandler != null)
		{
			m_MainThreadHandler.sendEmptyMessageDelayed(Constants.EVENT_STATISTIC_REFRESH, Constants.STATISTIC_INTERVAL);
		}
		
		CallStartParam param = new CallStartParam();
		if (m_iCodecParamIndex == 1)
		{
			param.res_width = 160;
			param.res_height = 90;
			param.fps = 8;
		}
		else if (m_iCodecParamIndex == 2)
		{
			param.res_width = 320;
			param.res_height = 180;
			param.fps = 15;
		}
		else if (m_iCodecParamIndex == 3)
		{
			param.res_width = 640;
			param.res_height = 360;
			param.fps = 30;
		}
		else {
			param.res_width = 1;
			param.res_height = 1;
			param.fps = 1;
		}
		
		Message msg = new Message();
		msg.what = Constants.EVENT_START_CALL;
		msg.obj = param;
		SettingActivity.m_MessageHandler.sendMessage(msg);
		
		super.onResume();
	}

	@Override
	protected void onNewIntent (Intent i){
	  if( i.getBooleanExtra("finish",false) == true ){
		 Log.i("wme_android", "PlayPanelActivity::onNewIntent, finish by force");
	     finish();
	  }
	}

//	@Override
//	public void onBackPressed() {
//		Log.i("wme_android","PlayPanelActivity::onBackPressed ");
//		super.onBackPressed();
//		Log.i("wme_android","PlayPanelActivity::onBackPressed, over");
//	}

	@Override
	public void onDestroy()
	{
		Log.i("wme_android","PlayPanelActivity::onDestroy ");
		
		StatisticData.resetAll();
		
		//WmeClient.instance().Disconnect();
		//Log.i("wme_android","PlayPanelActivity::onDestroy, Disconnect");
		super.onDestroy();
	}

	
	@Override
	public void onCheckedChanged(CompoundButton arg0, boolean isChecked) {
		// TODO Auto-generated method stub
		switch(arg0.getId())
		{
		case R.id.checkBoxSpeaker:
			m_bUISpeaker = isChecked;
			if(isChecked){ 
				WmeClient.instance().SetAudioOutType(WmeParameters.WME_AUDIO_OUT_SPEAKER);
			}else{ 
				WmeClient.instance().SetAudioOutType(WmeParameters.WME_AUDIO_OUT_VOICE);
			} 
			break;
		case R.id.checkBoxMute:
			m_bUIMute = isChecked;
			WmeClient.instance().SetMicState(m_bUIMute);
			break;
		case R.id.checkBoxAudio:
			m_bUIAudio = isChecked;
			if(isChecked){ 
				WmeClient.instance().StartAudioTrack();
				WmeClient.instance().StartAudioSending();
			}else{ 
				WmeClient.instance().StopAudioSending();
				WmeClient.instance().StopAudioTrack();
			} 
			break;
		case R.id.checkBoxVideo:
			m_bUIVideo = isChecked;
			if(isChecked){
                mLocalGLView.setVisibility(View.VISIBLE);
                mRemoteGLView.setVisibility(View.VISIBLE);
				WmeClient.instance().SetLocalWindow(mLocalGLView);
				if (m_bVideoFileRender == false) {
                    WmeClient.instance().SetRemoteWindow(null);
                    WmeClient.instance().SetRemoteWindow(m_bUIShare?mRemoteGLView:mRemoteShareGLView);
                }
				else
					WmeClient.instance().SetRemoteWindow(null);
				WmeClient.instance().StartVideoSending();
				WmeClient.instance().StartVideoTrack();
			}else{
                mLocalGLView.setVisibility(View.INVISIBLE);
                mRemoteGLView.setVisibility(View.INVISIBLE);
				WmeClient.instance().StopVideoSending();
				WmeClient.instance().StopVideoTrack();
				WmeClient.instance().SetLocalWindow(null);
				WmeClient.instance().SetRemoteWindow(null);
			} 
			break;

            case R.id.checkBoxShare:
                m_bUIShare = isChecked;
                WmeClient.instance().SetRemoteShareWindow(null);
                if( m_bUIVideo  && m_bVideoFileRender == false ) {
                    WmeClient.instance().SetRemoteWindow(null);
                    WmeClient.instance().SetRemoteWindow(m_bUIShare ? mRemoteGLView : mRemoteShareGLView);
                }
                if( isChecked )
                {
                    WmeClient.instance().SetRemoteShareWindow(mRemoteShareGLView);
                    WmeClient.instance().StartRemoteShareTrack();
                }
                else {
                    mRemoteGLView.setZOrderOnTop(false);
                    WmeClient.instance().StopRemoteShareTrack();
                }
                break;
		case R.id.checkBoxSwitch:
			m_bUIBackCam = isChecked;
			m_iCamIdx = (m_bUIBackCam == true ? 0 : 1);
			WmeClient.instance().SwitchCamera(WmeParameters.WME_LOCAL_TRACK, m_iCamIdx);
			break;		
		}
	}
	
	public class CallStartParam {
		public int res_width;
		public int res_height;
		public int fps;
	}
	
}
