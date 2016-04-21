package com.cisco.wmeAndroid;


import java.util.regex.PatternSyntaxException;

import com.cisco.webex.wme.WmeClient;
import com.cisco.webex.wme.WmeNative;
import com.cisco.webex.wme.WmeParameters;
import com.webex.wseclient.WseEngine;
import com.webex.wseclient.WseGLView;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.content.pm.ActivityInfo;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.Log;
import android.view.Display;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.TextView;



public class VideoSettingFragment extends Fragment implements 
	ActionBar.TabListener, 
	OnItemSelectedListener, 
	View.OnClickListener,
	CompoundButton.OnCheckedChangeListener,
	com.webex.wseclient.WseGLView.RendererCallback{

	private CheckBox m_checkEnableVideo;
	boolean m_bUIEnableVideo;
	
	private Spinner mVideoCodecParamSpinner;
	private Spinner mCaptureParamSpinner;
	private Spinner mCameraSpinner;
	private Spinner mVQualitySpinner;
	private Spinner mRenderSpinner;
	private int mCamIndex;
	private boolean mPreviewIsOn;
	//private SlidingDrawer mPreviewDrawer;
	private com.webex.wseclient.WseGLView mGLView;
	private boolean mForceLandscapeRender;
	private CheckBox mCBForceLandscapeRender;
	private Handler m_MessageHandler;
	
	private int m_selectedCodecParamIndex;
	private int m_selectedCaptureParamIndex;
	private int mVQualityIndex;
	private int mRenderModesIndex;
		
	// for data dump
	private int m_iDumpFlags;
	private CheckBox m_checkDumpRawCapture;
	private CheckBox m_checkDumpEncodeRtp;
	private CheckBox m_checkDumpRecvNal;
	private CheckBox m_checkDumpDecodeNal;
	private CheckBox m_checkDumpRenderData;
	
	// for external track	
	private CheckBox m_checkEnableExternalTrack;
	boolean m_bUIEnableExternalTrack;
	private Button m_btnSelectFile;
	private TextView m_txtSelectedView;
	private EditText m_editWidth;
	private EditText m_editHeight;
	private EditText m_editFPS;
	private Spinner m_ColorSpaceSpinner;
	private Button m_btnApplyExternal;
	private String m_rootPath = "/sdcard";	
	private TextView m_tvPreviewInfo;
	
	private TextView m_tvOutputPath;
	private CheckBox m_cbApplyOutput;
	private String mOutputPath;
	private boolean m_bApplyOutput;
	
	private int m_viFPS;
	private int m_viHeight;
	private int m_viWidth;
	private String mInputPath;
	
	public VideoSettingFragment() {
		m_bUIEnableVideo = true;
		mCamIndex = 0;
		mVQualityIndex = 2;	//means SD as default
		mRenderModesIndex = 1;
		mPreviewIsOn = false;
		m_selectedCodecParamIndex = 0;
		m_selectedCaptureParamIndex = 0;
		mForceLandscapeRender = false;
		m_MessageHandler = null;
		m_iDumpFlags = 0;
		m_bUIEnableExternalTrack = false;
		m_bApplyOutput = false;
		m_viFPS = 0;
		m_viHeight = 0;
		m_viWidth = 0;
		mInputPath = "";
		Log.i("wme_android", "VideoSettingFragment constuctor");
	}
	
	public int getCodecParamIndex()
	{
		return m_selectedCodecParamIndex;
	}
	
	public int getVideoQualityIndex()
	{
		return mVQualityIndex;
	}
	
	public int getCameraIndex()
	{
		return mCamIndex;
	}
	
	public int getCameraParamIndex()
	{
		return m_selectedCaptureParamIndex;
	}
	
	public boolean getEnableFileRender()
	{
		return m_bApplyOutput;
	}
	
	private void setExternalTrackUI(boolean enabled, boolean ischecked) 
	{		
		//enabled = false; // now only for testing
		if (m_checkEnableExternalTrack == null)
			return;
		
		if(!enabled) {
			ischecked = false;			
			m_checkEnableExternalTrack.setEnabled(false);
		}else {
			m_checkEnableExternalTrack.setEnabled(true);
		}
		
		m_checkEnableExternalTrack.setChecked(ischecked);
		if (!ischecked) {
			enabled = false;			
		}
		WmeClient.instance().EnableExternalVideoInput(enabled);

		if (m_btnSelectFile != null)
			m_btnSelectFile.setEnabled(enabled);	
		if (m_txtSelectedView != null)
			m_txtSelectedView.setEnabled(enabled);
		if (m_editWidth != null)
			m_editWidth.setEnabled(enabled);
		if (m_editHeight != null)
			m_editHeight.setEnabled(enabled);
		if (m_editFPS != null)
			m_editFPS.setEnabled(enabled);
		if (m_ColorSpaceSpinner != null)
			m_ColorSpaceSpinner.setEnabled(enabled);
		if (m_btnApplyExternal != null)
			m_btnApplyExternal.setEnabled(enabled);
	}
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
		Log.i("wme_android", "VideoSettingFragment::onCreate ++");
        super.onCreate(savedInstanceState);
        
        mOutputPath = "/sdcard/video_out.yuv";
        
        //TODO: add widgets
        getActivity().setContentView(R.layout.videosetting);
        
        m_checkEnableVideo = (CheckBox)getActivity().findViewById(R.id.checkBoxEnableVideo);
        if (m_checkEnableVideo != null) {
        	m_checkEnableVideo.setChecked(m_bUIEnableVideo);
        	m_checkEnableVideo.setOnCheckedChangeListener(this);
        }
        
        // for external track
        m_checkEnableExternalTrack = (CheckBox)getActivity().findViewById(R.id.checkBoxEnableExternalTrack);
        if (m_checkEnableExternalTrack != null) {        	
        	m_checkEnableExternalTrack.setOnCheckedChangeListener(this);
        }
        m_btnSelectFile = (Button)getActivity().findViewById(R.id.btnSelectFile);
        if (m_btnSelectFile != null) {        	
        	m_btnSelectFile.setOnClickListener(this);        	
        } 
        m_txtSelectedView = (TextView)getActivity().findViewById(R.id.textViewSelectedFile);   
        if (m_txtSelectedView != null) {
			m_txtSelectedView.setText(mInputPath);
		}	
        
        m_editWidth = (EditText)getActivity().findViewById(R.id.editTextWidth);
        m_editWidth.setText(Integer.toString(m_viWidth));
        m_editHeight = (EditText)getActivity().findViewById(R.id.editTextHeight);
        m_editHeight.setText(Integer.toString(m_viHeight));
        m_editFPS = (EditText)getActivity().findViewById(R.id.editTextFPS);
        m_editFPS.setText(Integer.toString(m_viFPS));
        
        m_ColorSpaceSpinner = (Spinner)getActivity().findViewById(R.id.rawVideoColorspace);
        if (m_ColorSpaceSpinner != null) {
        	String[] cs_items = getActivity().getResources().getStringArray(R.array.video_colorspaces);;
        	ArrayAdapter<String> cs_adapter = new ArrayAdapter<String>(getActivity().getBaseContext(), android.R.layout.simple_spinner_item, cs_items);
        	cs_adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        	m_ColorSpaceSpinner.setAdapter(cs_adapter);
        	m_ColorSpaceSpinner.setSelection(1);	//default
        	m_ColorSpaceSpinner.setOnItemSelectedListener(this);
        }
        m_btnApplyExternal = (Button)getActivity().findViewById(R.id.btnApplyExternal);
        if (m_btnApplyExternal != null) {
        	m_btnApplyExternal.setOnClickListener(this); 
        }
        setExternalTrackUI(m_bUIEnableVideo, m_bUIEnableExternalTrack);
        
        //for data dump
    	m_checkDumpRawCapture = (CheckBox)getActivity().findViewById(R.id.checkBoxDumpRawCapture);
    	if (m_checkDumpRawCapture != null) {
    		boolean isChecked = (m_iDumpFlags & WmeParameters.WME_DATA_DUMP_VIDEO_RAW_CAPTURE) == WmeParameters.WME_DATA_DUMP_VIDEO_RAW_CAPTURE;
    		m_checkDumpRawCapture.setChecked(isChecked);
    		m_checkDumpRawCapture.setOnCheckedChangeListener(this);
    	}
    	
    	m_checkDumpEncodeRtp = (CheckBox)getActivity().findViewById(R.id.checkBoxDumpEncodeRtp);
    	if (m_checkDumpEncodeRtp != null) {
    		boolean isChecked = (m_iDumpFlags & WmeParameters.WME_DATA_DUMP_VIDEO_ENCODE_RTP_LAYER) == WmeParameters.WME_DATA_DUMP_VIDEO_ENCODE_RTP_LAYER;
    		m_checkDumpEncodeRtp.setChecked(isChecked);
    		m_checkDumpEncodeRtp.setOnCheckedChangeListener(this);
    	}
    	
    	m_checkDumpRecvNal = (CheckBox)getActivity().findViewById(R.id.checkBoxDumpRecvNal);
    	if (m_checkDumpRecvNal != null) {
    		boolean isChecked = (m_iDumpFlags & WmeParameters.WME_DATA_DUMP_VIDEO_NAL_TO_LISTEN_CHANNEL) == WmeParameters.WME_DATA_DUMP_VIDEO_NAL_TO_LISTEN_CHANNEL;
    		m_checkDumpRecvNal.setChecked(isChecked);
    		m_checkDumpRecvNal.setOnCheckedChangeListener(this);
    	}
    	
    	m_checkDumpDecodeNal = (CheckBox)getActivity().findViewById(R.id.checkBoxDumpDecodeNal);
    	if (m_checkDumpDecodeNal != null) {
    		boolean isChecked = (m_iDumpFlags & WmeParameters.WME_DATA_DUMP_VIDEO_NAL_TO_DECODER) == WmeParameters.WME_DATA_DUMP_VIDEO_NAL_TO_DECODER;
    		m_checkDumpDecodeNal.setChecked(isChecked);
    		m_checkDumpDecodeNal.setOnCheckedChangeListener(this);
    	}
    	
    	m_checkDumpRenderData = (CheckBox)getActivity().findViewById(R.id.checkBoxDumpRenderData);
    	if (m_checkDumpRenderData != null) {
    		boolean isChecked = (m_iDumpFlags & WmeParameters.WME_DATA_DUMP_VIDEO_RAW_AFTER_DECODE_TO_RENDER) == WmeParameters.WME_DATA_DUMP_VIDEO_RAW_AFTER_DECODE_TO_RENDER;
    		m_checkDumpRenderData.setChecked(isChecked);
    		m_checkDumpRenderData.setOnCheckedChangeListener(this);
    	}
        
        mVideoCodecParamSpinner = (Spinner)getActivity().findViewById(R.id.videoCodecParamSpinner);
        mCaptureParamSpinner = (Spinner)getActivity().findViewById(R.id.CaptureParamSpinner);
        mCameraSpinner = (Spinner)getActivity().findViewById(R.id.cameraSpinner);
        mVQualitySpinner = (Spinner)getActivity().findViewById(R.id.VQualitySpinner);
        mRenderSpinner = (Spinner)getActivity().findViewById(R.id.RenderSpinner);
        
        String[] codec_items = WmeClient.instance().GetVideoCapabilityList();	//for real
    	//String[] codec_items = getActivity().getResources().getStringArray(R.array.video_codec_params);
    	ArrayAdapter<String> codec_adapter = new ArrayAdapter<String>(getActivity().getBaseContext(), android.R.layout.simple_spinner_item, codec_items);
    	codec_adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
    	mVideoCodecParamSpinner.setAdapter(codec_adapter);
        mVideoCodecParamSpinner.setSelection(m_selectedCodecParamIndex);
        mVideoCodecParamSpinner.setOnItemSelectedListener(this);
        
        String[] camera_items = WmeClient.instance().GetVideoDeviceList();
        for (int i=0;i<camera_items.length;i++)
        {
        	if (camera_items[i].equals("0"))
        	{
        		camera_items[i] += "(back)";
        	}
        	else if (camera_items[i].equals("1"))
        	{
        		camera_items[i] += "(front)";
        	}
        }
        //String[] camera_items = getActivity().getResources().getStringArray(R.array.camera_facing);
        ArrayAdapter<String> camera_adapter = new ArrayAdapter<String>(getActivity().getBaseContext(), android.R.layout.simple_spinner_item, camera_items);
        camera_adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mCameraSpinner.setAdapter(camera_adapter);
        mCameraSpinner.setOnItemSelectedListener(this);
        mCameraSpinner.setSelection(mCamIndex);
        
        String[] capture_items = WmeClient.instance().GetVideoCaptureParamList(mCamIndex);	//for real
    	ArrayAdapter<String> capture_adapter = new ArrayAdapter<String>(getActivity().getBaseContext(), android.R.layout.simple_spinner_item, capture_items);
    	capture_adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
    	mCaptureParamSpinner.setAdapter(capture_adapter);
    	mCaptureParamSpinner.setSelection(m_selectedCaptureParamIndex);
    	mCaptureParamSpinner.setOnItemSelectedListener(this);
        
        String[] video_qualitys = getActivity().getResources().getStringArray(R.array.video_quality);
        ArrayAdapter<String> vquality_adapter = new ArrayAdapter<String>(getActivity().getBaseContext(), android.R.layout.simple_spinner_item, video_qualitys);
        vquality_adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mVQualitySpinner.setAdapter(vquality_adapter);
        mVQualitySpinner.setOnItemSelectedListener(this);
        mVQualitySpinner.setSelection(mVQualityIndex);
        
        String[] render_modes = getActivity().getResources().getStringArray(R.array.render_mode);
        ArrayAdapter<String> rmodes_adapter = new ArrayAdapter<String>(getActivity().getBaseContext(), android.R.layout.simple_spinner_item, render_modes);
        vquality_adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mRenderSpinner.setAdapter(rmodes_adapter);
        mRenderSpinner.setOnItemSelectedListener(this);
        mRenderSpinner.setSelection(mRenderModesIndex);
           
        mCBForceLandscapeRender = (CheckBox)getActivity().findViewById(R.id.checkBoxForceLandscapeRender);
        mCBForceLandscapeRender.setChecked(mForceLandscapeRender);
        mCBForceLandscapeRender.setOnCheckedChangeListener(this);
        mCBForceLandscapeRender.setEnabled(false);	//disable the feature, no use
        
        mGLView = (com.webex.wseclient.WseGLView)getActivity().findViewById(R.id.PreviewVideoGLView); 
        mGLView.setOnClickListener(this);
        mGLView.setRendererCallback(this);
        
        
        m_tvOutputPath = (TextView)getActivity().findViewById(R.id.textview_video_output_path);
        m_tvOutputPath.setText(mOutputPath);
        
        m_cbApplyOutput = (CheckBox)getActivity().findViewById(R.id.checkBoxApplyVideoOutputFile);
        m_cbApplyOutput.setChecked(m_bApplyOutput);
        m_cbApplyOutput.setOnCheckedChangeListener(this);
        
        
        //preview statistic
        m_tvPreviewInfo = (TextView)getActivity().findViewById(R.id.PreviewStatistic);
        
//        mPreviewDrawer = (SlidingDrawer)getActivity().findViewById(R.id.slidingdrawer);
//        mPreviewDrawer.setOnDrawerOpenListener(new SlidingDrawer.OnDrawerOpenListener()
//        {
//        	@Override
//        	public void onDrawerOpened() {
//        		Log.i("wme_android", "onDrawerOpened, so start preview");  
//        		WmeClient.instance().SetPreviewWindow(mGLView);
//        		WmeClient.instance().StartPreviewTrack();
//	            mPreviewIsOn = true;
//        	}
//        	   
//        });
//        
//        mPreviewDrawer.setOnDrawerCloseListener(new SlidingDrawer.OnDrawerCloseListener()
//        {
//        	@Override
//        	public void onDrawerClosed() {
//        		Log.i("wme_android", "onDrawerClosed, so stop preview");
//        		WmeClient.instance().StopPreviewTrack();
//        		WmeClient.instance().SetPreviewWindow(null);
//        		mPreviewIsOn = false;
//        	}
//        	   
//       });
        
        m_MessageHandler = new Handler(){
			@Override
			public void handleMessage(Message msg){					
				
				switch (msg.what) {
					case Constants.EVENT_ADJUST_PREVIEW_ORIENTATION:
					{
						adjustPreviewOrientation(getResources().getConfiguration().orientation);
						
						removeMessages(msg.what);
					}
					break;
					case Constants.EVENT_STATISTIC_REFRESH:
					{
						if (m_tvPreviewInfo != null)
						{
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
							
							m_tvPreviewInfo.setText(content_capture);
							sendEmptyMessageDelayed(Constants.EVENT_STATISTIC_REFRESH, Constants.STATISTIC_INTERVAL);
						}
					}
					
					default:
					break;
				}
			}
        };
        
        Log.i("wme_android", "VideoSettingFragment::onCreate --");
	}
	
	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		Log.i("wme_android", "VideoSettingFragment::onConfigurationChanged, orientation="+newConfig.orientation);
		
		 adjustPreviewOrientation(newConfig.orientation);
		
		 WindowManager windowManager = getActivity().getWindowManager();  
         Display display = windowManager.getDefaultDisplay();  
         int screenRotation = display.getRotation(); 
         WseEngine.setDisplayRotation(screenRotation);
         Log.i("wme_android","VideoSettingFragment, screenRotation = "+screenRotation);
         
         
         super.onConfigurationChanged(newConfig);
	}
	
	private void adjustPreviewOrientation(int current_orientation)
	{
		if(current_orientation == ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE ||
				current_orientation == ActivityInfo.SCREEN_ORIENTATION_USER)	//FIXME: why SCREEN_ORIENTATION_USER on Galaxy S4 ?
		{
			int w = mGLView.getWidth();
			int h = mGLView.getHeight();
			Log.i("wme_android", "VideoSettingFragment::adjustPreviewOrientation, landscape, w="+w+",h="+h);
			if (w < h)
			{
				ViewGroup.LayoutParams param = mGLView.getLayoutParams();
				param.height = w;
	            param.width = h;
				mGLView.setLayoutParams(param);
			}
		}
		else if(current_orientation == ActivityInfo.SCREEN_ORIENTATION_PORTRAIT)
		{		
			int w = mGLView.getWidth();
			int h = mGLView.getHeight();
			Log.i("wme_android", "VideoSettingFragment::adjustPreviewOrientation, portrait, w="+w+",h="+h+",mForceLandscapeRender="+mForceLandscapeRender);
			if ((w > h && mForceLandscapeRender == false)
				|| (w < h && mForceLandscapeRender == true))
			{
				ViewGroup.LayoutParams param = mGLView.getLayoutParams();
				param.height = w;
	            param.width = h;
				mGLView.setLayoutParams(param);
				
				WmeClient.instance().SetRenderAspectRatioSameWithSource(WmeParameters.WME_PREVIEW_TRACK, !mForceLandscapeRender);
			}
		}
	}
	
//	@Override
//    public void onDestroy()
//    {
//		Log.i("wme_android", "VideoSettingFragment::onDestroy");
//    	super.onDestroy();
//    }
	
	@Override
	 public void onPause()
	 {
		Log.i("wme_android", "VideoSettingFragment::onPause");
		if (mGLView != null)
		{
			mGLView.onPause();
		}
		
		if (mPreviewIsOn == true)
		{
			Log.i("wme_android", "VideoSettingFragment::onPause, stop preview");
			if (m_tvPreviewInfo != null && m_MessageHandler != null)
	        {
	        	m_MessageHandler.removeMessages(Constants.EVENT_STATISTIC_REFRESH);
	        }
    		WmeClient.instance().StopPreviewTrack();
    		WmeClient.instance().SetPreviewWindow(null);
    		mPreviewIsOn = false;
    		
    		WmeClient.instance().DeletePreviewClient();	//release resources ahead
		}
		
		
		
		super.onPause();
	 }
	
	@Override
	public void onResume()
	{
		Log.i("wme_android", "VideoSettingFragment::onResume");
		
		if (mGLView != null)
		{
			mGLView.onResume();
		}
		
//		WmeClient.instance().CreatePreviewClient();
//        WmeClient.instance().SetRenderAspectRatioSameWithSource(WmeParameters.WME_PREVIEW_TRACK, !mForceLandscapeRender);
		super.onResume();
	}
	
	public boolean isPreviewOn()
	{
		return mPreviewIsOn;
	}
	
	@Override
	public void onTabReselected(Tab arg0, FragmentTransaction arg1) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onTabSelected(Tab tab, FragmentTransaction ft) {
		// TODO Auto-generated method stub
		Log.i("wme_android","VideoSettingFragment::onTabSelected");
		ft.add(android.R.id.content, this);
	}

	@Override
	public void onTabUnselected(Tab tab, FragmentTransaction ft) {
		// TODO Auto-generated method stub
		Log.i("wme_android","VideoSettingFragment::onTabUnselected");
		ft.remove(this);
		if (mPreviewIsOn == true)
		{
			if (m_tvPreviewInfo != null && m_MessageHandler != null)
	        {
	        	m_MessageHandler.removeMessages(Constants.EVENT_STATISTIC_REFRESH);
	        }
			WmeClient.instance().StopPreviewTrack();
			WmeClient.instance().SetPreviewWindow(null);
    		mPreviewIsOn = false;
    		
    		WmeClient.instance().DeletePreviewClient();
		}
		
	}
	
//	@Override
//    public void onTabSelected(Tab tab, FragmentTransaction ft) {
//      FragmentManager fm = getFragmentManager();
//      if(fm.findFragmentByTag(tab.getTag().toString()) == null){
//           ft = fm.beginTransaction();
//           //VideoSettingFragment contentfrag = new VideoSettingFragment();
//           ft.add(android.R.id.content, this, tab.getTag().toString());
//           ft.addToBackStack("BackStack" + tab.getTag().toString());
//       }
//       else{
//           Fragment frag = fm.findFragmentByTag(tab.getTag().toString());
//           ft.show(frag);
//      }
//	}
//
//    @Override
//    public void onTabUnselected(Tab tab, FragmentTransaction ft) {
//        Fragment frag = this.getFragmentManager().findFragmentByTag(tab.getTag().toString());
//        ft.hide(frag);
//
//    }

	@Override
	public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2,
			long arg3) {
		// TODO Auto-generated method stub
		switch(arg0.getId())
		{
		case R.id.rawVideoColorspace:
			WmeClient.instance().SetVideoColor(arg2);
			break;
		case R.id.videoCodecParamSpinner:
			m_selectedCodecParamIndex = arg2;	//锟斤拷锟街�
			break;
		case R.id.cameraSpinner:
			if (mCamIndex != arg2)
			{
				String[] capture_items = WmeClient.instance().GetVideoCaptureParamList(arg2);	//for real
		    	ArrayAdapter<String> capture_adapter = new ArrayAdapter<String>(getActivity().getBaseContext(), android.R.layout.simple_spinner_item, capture_items);
		    	capture_adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
		    	mCaptureParamSpinner.setAdapter(capture_adapter);
		    	mCaptureParamSpinner.setSelection(0);
		    	mCaptureParamSpinner.invalidate();
		        Log.i("wme_android", "update camera param list when switch camera, len="+capture_items.length);  
			}
			mCamIndex = arg2;		//锟斤拷锟街�
			WmeClient.instance().SwitchCamera(WmeParameters.WME_PREVIEW_TRACK, mCamIndex);
			break;
		case R.id.VQualitySpinner:
			mVQualityIndex  = arg2;
			break;
		case R.id.RenderSpinner:
			mRenderModesIndex = arg2;
			WmeClient.instance().SetRenderMode(WmeParameters.WME_PREVIEW_TRACK, mRenderModesIndex);
			break;
		case R.id.CaptureParamSpinner:
			m_selectedCaptureParamIndex = arg2;
			if (mPreviewIsOn == true)
			{
				if (m_tvPreviewInfo != null && m_MessageHandler != null)
		        {
		        	m_MessageHandler.removeMessages(Constants.EVENT_STATISTIC_REFRESH);
		        }
				WmeClient.instance().StopPreviewTrack();
				WmeClient.instance().SetPreviewWindow(null);
				
				WmeClient.instance().SetVideoCaptureParam(WmeParameters.WME_PREVIEW_TRACK, mCamIndex, m_selectedCaptureParamIndex);
				
				WmeClient.instance().SetPreviewWindow(mGLView);
				WmeClient.instance().StartPreviewTrack();
				if (m_tvPreviewInfo != null && m_MessageHandler != null)
		        {
		        	m_MessageHandler.sendEmptyMessageDelayed(Constants.EVENT_STATISTIC_REFRESH, Constants.STATISTIC_INTERVAL);
		        }
			}
			else {
				WmeClient.instance().SetVideoCaptureParam(WmeParameters.WME_PREVIEW_TRACK, mCamIndex, m_selectedCaptureParamIndex);
			}
			
			break;
		}
	}

	@Override
	public void onNothingSelected(AdapterView<?> arg0) {
		// TODO Auto-generated method stub
		
	}


	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch(v.getId())
		{
		case R.id.PreviewVideoGLView:
			if (mPreviewIsOn == false)
			{
				Log.i("wme_android", "VideoSettingFragment::onClick, so start preview");  
								
				WmeClient.instance().CreatePreviewClient();
		        WmeClient.instance().SetRenderAspectRatioSameWithSource(WmeParameters.WME_PREVIEW_TRACK, !mForceLandscapeRender);
				
        		WmeClient.instance().SetPreviewWindow(mGLView);
        		WmeClient.instance().StartPreviewTrack();
        		
        		if (m_tvPreviewInfo != null && m_MessageHandler != null)
		        {
		        	m_MessageHandler.sendEmptyMessageDelayed(Constants.EVENT_STATISTIC_REFRESH, Constants.STATISTIC_INTERVAL);
		        }
	            mPreviewIsOn = true;
			}
			else
			{
				Log.i("wme_android", "VideoSettingFragment::onClick, so stop preview");
				if (m_tvPreviewInfo != null && m_MessageHandler != null)
		        {
		        	m_MessageHandler.removeMessages(Constants.EVENT_STATISTIC_REFRESH);
		        }
        		WmeClient.instance().StopPreviewTrack();
        		WmeClient.instance().SetPreviewWindow(null);
        		
        		WmeClient.instance().DeletePreviewClient();
        		mPreviewIsOn = false;
			}
			break;
		case R.id.btnSelectFile:
			browseFiles(m_rootPath);
			break;
//		case R.id.editTextWidth:
//			break;
//		case R.id.editTextHeight:
//			break;
		case R.id.btnApplyExternal:
			int height = 0;
			int width = 0;
			int fps = 0;
			try {
				height = Integer.parseInt(m_editHeight.getText().toString());
				width = Integer.parseInt(m_editWidth.getText().toString()); 
				fps = Integer.parseInt(m_editFPS.getText().toString()); 
				m_viFPS = fps;
				m_viHeight = height;
				m_viWidth = width;
			}catch(Exception e) {}
			WmeClient.instance().SetVideoWidth(width);
			WmeClient.instance().SetVideoHeight(height);
			WmeClient.instance().SetFPS(fps);
			//WmeClient.instance().ApplyExternalTrack();
			break;
		default:
			break;
		}
	}

	@Override
	public void onCheckedChanged(CompoundButton arg0, boolean isChecked) {
		// TODO Auto-generated method stub
		boolean isDataDump = false;
		int iDumpFlag = 0;
		
		switch(arg0.getId())
		{
		case R.id.checkBoxForceLandscapeRender:
			mForceLandscapeRender = isChecked;
			
			boolean bNeedRestartPreview = false;
			if (mPreviewIsOn == true)
			{
				if (m_tvPreviewInfo != null && m_MessageHandler != null)
		        {
		        	m_MessageHandler.removeMessages(Constants.EVENT_STATISTIC_REFRESH);
		        }
				
				WmeClient.instance().StopPreviewTrack();
				WmeClient.instance().SetPreviewWindow(null);
				mPreviewIsOn = false;
				bNeedRestartPreview = true;
			}
			
			adjustPreviewOrientation(getResources().getConfiguration().orientation);
			
			if (bNeedRestartPreview == true)
			{
				WmeClient.instance().SetPreviewWindow(mGLView);
				WmeClient.instance().StartPreviewTrack();
				
				if (m_tvPreviewInfo != null && m_MessageHandler != null)
		        {
		        	m_MessageHandler.sendEmptyMessageDelayed(Constants.EVENT_STATISTIC_REFRESH, Constants.STATISTIC_INTERVAL);
		        }
				mPreviewIsOn = true;
			}
			break;
		case R.id.checkBoxEnableVideo:
			Log.i("wme_android", "VideoSettingFragment::onCheckedChanged, for checkBoxEnableVideo");
			m_bUIEnableVideo = isChecked;
			mVideoCodecParamSpinner.setEnabled(isChecked);
			mCaptureParamSpinner.setEnabled(isChecked);
			mCameraSpinner.setEnabled(isChecked);
			mVQualitySpinner.setEnabled(isChecked);
			mRenderSpinner.setEnabled(isChecked);
			mCBForceLandscapeRender.setEnabled(isChecked);
			mGLView.setEnabled(isChecked);
			
			setExternalTrackUI(m_bUIEnableVideo, m_bUIEnableExternalTrack);
			
			// for data dump
			m_checkDumpRawCapture.setEnabled(isChecked);
			m_checkDumpEncodeRtp.setEnabled(isChecked);
			m_checkDumpRecvNal.setEnabled(isChecked);
			m_checkDumpDecodeNal.setEnabled(isChecked);
			m_checkDumpRenderData.setEnabled(isChecked);
			
			if (!isChecked) {						
				WmeClient.instance().EnableVideo(false);
				WmeClient.instance().DeleteMediaClient(WmeParameters.WME_MEDIA_VIDEO);
			}else {				
				WmeClient.instance().CreateMediaClient(WmeParameters.WME_MEDIA_VIDEO);
				WmeClient.instance().EnableVideo(true);
			}
			break;	
		case R.id.checkBoxEnableExternalTrack:
			setExternalTrackUI(m_bUIEnableVideo, isChecked);	
			m_bUIEnableExternalTrack = isChecked;
			break;
		case R.id.checkBoxDumpRawCapture:
			isDataDump = true;
			iDumpFlag = WmeParameters.WME_DATA_DUMP_VIDEO_RAW_CAPTURE;
			break;
		case R.id.checkBoxDumpEncodeRtp:
			isDataDump = true;
			iDumpFlag = WmeParameters.WME_DATA_DUMP_VIDEO_ENCODE_RTP_LAYER;
			break;
		case R.id.checkBoxDumpRecvNal:
			isDataDump = true;
			iDumpFlag = WmeParameters.WME_DATA_DUMP_VIDEO_NAL_TO_LISTEN_CHANNEL;
			break;
		case R.id.checkBoxDumpDecodeNal:
			isDataDump = true;
			iDumpFlag = WmeParameters.WME_DATA_DUMP_VIDEO_NAL_TO_DECODER;
			break;
		case R.id.checkBoxDumpRenderData:
			isDataDump = true;
			iDumpFlag = WmeParameters.WME_DATA_DUMP_VIDEO_RAW_AFTER_DECODE_TO_RENDER;
			break;		
			
		case R.id.checkBoxApplyVideoOutputFile:
			if (m_bApplyOutput == isChecked)
				break;
			m_bApplyOutput = isChecked;
			WmeClient.instance().EnableVideoOutputFile(m_bApplyOutput);
			if (m_bApplyOutput == true)
			{
				WmeClient.instance().SetVideoOutputFile(mOutputPath);
			}
			break;
		}
		
		if (isDataDump) {
			if (isChecked){
				m_iDumpFlags = m_iDumpFlags ^ iDumpFlag;
			}else {
				m_iDumpFlags = m_iDumpFlags & (~iDumpFlag);
			}
			Log.i("wme_android","VideoSettingFragment::onCheckedChanged, flag=" + iDumpFlag + ", set flags=" + m_iDumpFlags);
			WmeNative.SetDumpEnabled(m_iDumpFlags);
		}
	}
	
	public void browseFiles(String rootPath) {
		Intent intent = new Intent();
		intent.setClassName("com.cisco.wmeAndroid", "com.cisco.wmeAndroid.FileDialog");
        intent.putExtra(FileDialog.ROOT_PATH, rootPath);       
        intent.putExtra(FileDialog.SUPPORT_DIR, false);
        //intent.putExtra(FileDialog.FILE_SUFFIX, new String[] { "yuv" });
        startActivityForResult(intent, 0);
	}
	
	public void bdEnableExternalVideoTrack(boolean isChecked) {
		setExternalTrackUI(m_bUIEnableVideo, isChecked);	
		m_bUIEnableExternalTrack = isChecked;
	}
	
	public void bdApplyExternalVideoTrack() {
		int height = 0;
		int width = 0;
		int fps = 0;
		try {
			height = Integer.parseInt(m_editHeight.getText().toString());
			width = Integer.parseInt(m_editWidth.getText().toString()); 
			fps = Integer.parseInt(m_editFPS.getText().toString()); 
			m_viFPS = fps;
			m_viHeight = height;
			m_viWidth = width;
			Log.i("VideoSetting", "bdApplyExternalTrack:");
		}catch(Exception e) {}
		WmeClient.instance().SetVideoWidth(width);
		WmeClient.instance().SetVideoHeight(height);
		WmeClient.instance().SetFPS(fps);
	}

	public void bdSelectRawVideoFile() {
		mInputPath = "/sdcard/test_580x580_10fps_test.yuv";
		Log.i("wme_android", "bdSelectRawFile video file selected = " + mInputPath);
		if (m_txtSelectedView != null) {
			m_txtSelectedView.setText(mInputPath);
		}			
		WmeClient.instance().SetVideoInputFile(mInputPath);
		Log.i("wme_android", "bdSelectRawFile SetVideoInputFile = " + mInputPath);
		int []whfps = getW_H_FPS(mInputPath);
		
		if (whfps != null)
		{
			m_viFPS = whfps[2];
			m_viHeight = whfps[1];
			m_viWidth = whfps[0];
			
			m_editWidth.setText(Integer.toString(m_viWidth));
			m_editHeight.setText(Integer.toString(m_viHeight));
			m_editFPS.setText(Integer.toString(m_viFPS));
		}
	}

	public void bdApplyVideoOutputFile(boolean isChecked) {
		if (m_bApplyOutput == isChecked)
			return;
		m_bApplyOutput = isChecked;
		Log.i("bdApplyOutputFile", "bapplyout is:"+m_bApplyOutput);
		WmeClient.instance().EnableVideoOutputFile(m_bApplyOutput);
		if (m_bApplyOutput == true)
		{
			mOutputPath = "/sdcard/video.yuv";
			Log.i("bdApplyOutputFile","mOutputPath is:"+mOutputPath);
			WmeClient.instance().SetVideoOutputFile(mOutputPath);
		}
	}
	    
	@Override
	public synchronized void onActivityResult(final int requestCode,
			int resultCode, final Intent data) {
		if (resultCode == Activity.RESULT_OK) {
			mInputPath = data.getStringExtra(FileDialog.CHOSEN_PATH);
			Log.i("wme_android", "video file selected = " + mInputPath);
			if (m_txtSelectedView != null) {
				m_txtSelectedView.setText(mInputPath);
			}			
			WmeClient.instance().SetVideoInputFile(mInputPath);
			int []whfps = getW_H_FPS(mInputPath);
			
			if (whfps != null)
			{
				m_viFPS = whfps[2];
				m_viHeight = whfps[1];
				m_viWidth = whfps[0];
				
				m_editWidth.setText(Integer.toString(m_viWidth));
				m_editHeight.setText(Integer.toString(m_viHeight));
				m_editFPS.setText(Integer.toString(m_viFPS));
			}
			
		} else if (resultCode == Activity.RESULT_CANCELED) {
			Log.i("wme_android", "file not selected");
		}
    }
	
	private int[] getW_H_FPS(String path_re)
	{
		//example: /sdcard/encTest/abc_640x320_25fps_900.yuv
		//example: /sdcard/encTest/640x320_25fps_test.yuv
		//example: /sdcard/encTest/640x320_25fps.yuv
		try {
			if (!path_re.matches(".*\\w*_\\d{1,4}x\\d{1,4}_\\d{1,3}fps\\w*\\.\\w*"))
			{
				return null;
			}
		}catch(PatternSyntaxException ex)
		{
			ex.printStackTrace();
		}
		
		int[] whfps = new int[3];
		int idx = 0;
		
		String[] splits = path_re.split("_");
		for (int i=0; i<splits.length; i++)
		{
			if (splits[i].matches("\\d{1,4}x\\d{1,4}"))
			{
				String[] wh = splits[i].split("x");
				for (int j=0; j<wh.length; j++)
				{
					whfps[idx] = Integer.valueOf(wh[j]);
					idx++;
				}
			}
			
			if (splits[i].matches("\\d{1,2}fps"))
			{
				String[] fps = splits[i].split("fps");
				whfps[idx] = Integer.valueOf(fps[0]);
				break;
			}
		}
		
		
		return whfps;
		
	}
	
	
	@Override
	public void onDrawFrame() {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onSurfaceChanged(int arg0, int arg1) {
		// TODO Auto-generated method stub
		
		
	}

	@Override
	public void onSurfaceCreated() {
		// TODO Auto-generated method stub
		if (m_MessageHandler != null)
		{
			m_MessageHandler.sendEmptyMessage(Constants.EVENT_ADJUST_PREVIEW_ORIENTATION);
		}
	}
	
	public boolean isVideoOn() {
		return m_bUIEnableVideo;
	}

	public void bdSelectResolution(int nIdx){
                m_selectedCodecParamIndex = nIdx;
        Log.i("WME_Android", "bdSelectResolution_VideoSettingFragment" + m_selectedCodecParamIndex);
                mVideoCodecParamSpinner.setSelection(m_selectedCodecParamIndex);
        }

	@Override
	public void OnRequestRender(WseGLView arg0, int arg1, int arg2) {
		// TODO Auto-generated method stub
		
	}

}
