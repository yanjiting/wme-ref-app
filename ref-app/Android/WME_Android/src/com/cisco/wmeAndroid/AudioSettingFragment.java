package com.cisco.wmeAndroid;

import java.util.regex.PatternSyntaxException;

import com.cisco.webex.wme.WmeClient;
import com.cisco.webex.wme.WmeParameters;

import android.app.ActionBar;
import android.app.ActionBar.Tab;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AdapterView.OnItemSelectedListener;



public class AudioSettingFragment extends Fragment implements ActionBar.TabListener, OnItemSelectedListener, CompoundButton.OnCheckedChangeListener, OnClickListener {

	private CheckBox m_checkEnableAudio;
	private boolean m_bUIEnableAudio;
	
	private Spinner mAudioCodecTypeSpinner;
	private int m_selectedCodecIndex;
	private Button m_btnSelectFile;
	private String m_rootPath = "/sdcard";
	private EditText m_etChannels;
	private EditText m_etSampleRate;
	private EditText m_etBitsPerSample;
	private CheckBox m_cbApplyRaw;
	private boolean m_bApplyRaw;
	private boolean m_bApplyOutput;
	private String m_rawPath = "";
	private TextView m_tvRawPath;
	private TextView m_tvOutputPath;
	private CheckBox m_cbApplyOutput;
	
	private String mChannels;
	private String mSamplerate;
	private String mBitsPerSample;
	
	private String mOutputPath;
	
	public AudioSettingFragment() {
		Log.i("wme_android", "AudioSettingFragment constuctor");
		m_bUIEnableAudio = true;
		m_bApplyRaw = false;
		mChannels = "";
		mSamplerate = "";
		mBitsPerSample = "";
		m_bApplyOutput = false;
	}
	
	public boolean getEnableFileRender()
	{
		return m_bApplyOutput;
	}
	
	@Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        
        mOutputPath = "/sdcard/audio_output.pcm";
        
        //TODO: add widgets
        getActivity().setContentView(R.layout.audiosetting);
        
        m_checkEnableAudio = (CheckBox)getActivity().findViewById(R.id.checkBoxEnableAudio);
        if (m_checkEnableAudio != null) {
        	m_checkEnableAudio.setChecked(m_bUIEnableAudio);
        	m_checkEnableAudio.setOnCheckedChangeListener(this);
        }
        
        m_cbApplyRaw = (CheckBox)getActivity().findViewById(R.id.checkBoxApplyAudioRawFile);
        m_cbApplyRaw.setChecked(m_bApplyRaw);
        m_cbApplyRaw.setOnCheckedChangeListener(this);
        
        mAudioCodecTypeSpinner = (Spinner)getActivity().findViewById(R.id.audioCodecTypeSpinner);
        
        
        String[] codec_items = WmeClient.instance().GetAudioCapabilityList();	//for real
        //String[] codec_items = getActivity().getResources().getStringArray(R.array.audio_codec_params);
        ArrayAdapter<String> codec_adapter = new ArrayAdapter<String>(getActivity().getBaseContext(), android.R.layout.simple_spinner_item, codec_items);
        codec_adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mAudioCodecTypeSpinner.setAdapter(codec_adapter);
        mAudioCodecTypeSpinner.setOnItemSelectedListener(this);
        mAudioCodecTypeSpinner.setSelection(m_selectedCodecIndex);
        
        m_btnSelectFile = (Button)getActivity().findViewById(R.id.btnSelectAudioFile);
        m_btnSelectFile.setOnClickListener(this);
        
        m_etChannels = (EditText)getActivity().findViewById(R.id.editChannels);
        m_etChannels.setText(mChannels);
        m_etSampleRate = (EditText)getActivity().findViewById(R.id.editSampleRate);
        m_etSampleRate.setText(mSamplerate);
        m_etBitsPerSample = (EditText)getActivity().findViewById(R.id.editBitsPerSample);
        m_etBitsPerSample.setText(mBitsPerSample);
        
        m_tvRawPath = (TextView)getActivity().findViewById(R.id.tviewAudioRawPath);
        m_tvRawPath.setText(m_rawPath);
        
        m_tvOutputPath = (TextView)getActivity().findViewById(R.id.textview_audio_output_path);
        m_tvOutputPath.setText(mOutputPath);
        
        m_cbApplyOutput = (CheckBox)getActivity().findViewById(R.id.checkBoxApplyAudioOutputFile);
        m_cbApplyOutput.setChecked(m_bApplyOutput);
        m_cbApplyOutput.setOnCheckedChangeListener(this);
        
        Log.i("wme_android", "AudioSettingFragment::onCreate");
	}
	

	@Override
	public void onTabReselected(Tab arg0, FragmentTransaction arg1) {
		// TODO Auto-generated method stub
		
	}

	@Override
	public void onTabSelected(Tab tab, FragmentTransaction ft) {
		// TODO Auto-generated method stub

		Log.i("wme_android","AudioSettingFragment::onTabSelected");

		ft.add(android.R.id.content, this);
        //ft.attach(this);
	}

	@Override
	public void onTabUnselected(Tab tab, FragmentTransaction ft) {
		// TODO Auto-generated method stub
		Log.i("wme_android","AudioSettingFragment::onTabUnselected");
		ft.remove(this);
	}
	

	@Override
	public void onItemSelected(AdapterView<?> arg0, View arg1, int arg2,
			long arg3) {
		// TODO Auto-generated method stub
		Log.i("wme_android","AudioSettingFragment::onItemSelected");
		switch(arg0.getId())
		{
		case R.id.audioCodecTypeSpinner:
			m_selectedCodecIndex = arg2;
			WmeClient.instance().SetLocalAudioCapability(m_selectedCodecIndex);
			break;
		}
	}

	@Override
	public void onNothingSelected(AdapterView<?> arg0) {
		// TODO Auto-generated method stub
		Log.i("wme_android","AudioSettingFragment::onNothingSelected");
	}

	@Override
	public void onCheckedChanged(CompoundButton arg0, boolean isChecked) {
		// TODO Auto-generated method stub
		switch(arg0.getId())
		{
		case R.id.checkBoxEnableAudio:
			m_bUIEnableAudio = isChecked;
			mAudioCodecTypeSpinner.setEnabled(isChecked);
			if (!isChecked) {
				WmeClient.instance().EnableAudio(false);
				WmeClient.instance().DeleteMediaClient(WmeParameters.WME_MEDIA_AUDIO);				
			}else {
				WmeClient.instance().DeleteMediaClient(WmeParameters.WME_MEDIA_AUDIO);
				WmeClient.instance().EnableAudio(true);
			}
			break;
		case R.id.checkBoxApplyAudioRawFile:
			if (m_bApplyRaw == isChecked)
				break;
			
			if (isChecked == true)
			{
				String strChannles = m_etChannels.getText().toString();
				String strSampleRate = m_etSampleRate.getText().toString();
				String strBitsPerSample = m_etBitsPerSample.getText().toString();
				if (isNumberValid(strChannles) &&
						isNumberValid(strSampleRate) &&
						isNumberValid(strBitsPerSample))
				{
					m_bApplyRaw = true;
					WmeClient.instance().SetChannels(Integer.parseInt(strChannles));
					WmeClient.instance().SetSampleRate(Integer.parseInt(strSampleRate));
					WmeClient.instance().SetBitsPerSample(Integer.parseInt(strBitsPerSample));
					mChannels = strChannles;
					mSamplerate = strSampleRate;
					mBitsPerSample = strBitsPerSample;
				}
				else
				{
					m_bApplyRaw = false;
					m_cbApplyRaw.setChecked(false);
					Toast.makeText(getActivity(), "Invalid audio param", Toast.LENGTH_SHORT).show();
				}
			}
			else
			{
				m_bApplyRaw = false;
			}
			WmeClient.instance().EnableExternalAudioInput(m_bApplyRaw);
			break;
			
		case R.id.checkBoxApplyAudioOutputFile:
			if (m_bApplyOutput == isChecked)
				break;
			
			m_bApplyOutput = isChecked;
			WmeClient.instance().EnableAudioOutputFile(m_bApplyOutput);
			if (m_bApplyOutput == true)
			{
				WmeClient.instance().SetAudioOutputFile(mOutputPath);
			}
			
			break;
		}
	}
	
	private boolean isNumberValid(String param)
	{
		boolean isValid = false;
		if (param.matches ("^\\d{1,6}")) 
		{ 
			isValid = true;
		}
		return isValid;
	}
	
	public boolean isAudioOn() {
		return m_bUIEnableAudio;
	}
	
	public void browseFiles(String rootPath) {
		Intent intent = new Intent();
		intent.setClassName("com.cisco.wmeAndroid", "com.cisco.wmeAndroid.FileDialog");
        intent.putExtra(FileDialog.ROOT_PATH, rootPath);       
        intent.putExtra(FileDialog.SUPPORT_DIR, false);
        //intent.putExtra(FileDialog.FILE_SUFFIX, new String[] { "pcm" });
        startActivityForResult(intent, 0);
	}
	
	private int[] getRawInfo(String path)
	{
		//example path: /sdcard/audio/abc_2_48000_16_xyz.pcm
		try {
			if (!path.matches(".*\\w*_\\d{1,2}_\\d{1,6}_\\d{1,3}\\w*\\.\\w*"))
			{
				return null;
			}
		}catch(PatternSyntaxException ex)
		{
			ex.printStackTrace();
		}
		
		int[] ch_sr_bps = new int[3];
		int idx = 0;
		
		String[] splits = path.split("_");
		for (int i=0; i<splits.length; i++)
		{
			if (splits[i].matches("\\d+"))
			{
				ch_sr_bps[idx] = Integer.valueOf(splits[i]);
				idx++;
				if (idx == ch_sr_bps.length)
					break;
			}
		}
		
		
		return ch_sr_bps;
	}
	
		public void bdApplyAudioRawFile() {
		
		m_bApplyRaw = true;

        Log.i("wme_android", "bdApplyAudioRawFile");
		String strChannles = m_etChannels.getText().toString();
		String strSampleRate = m_etSampleRate.getText().toString();
		String strBitsPerSample = m_etBitsPerSample.getText().toString();
		if (isNumberValid(strChannles) &&
				isNumberValid(strSampleRate) &&
				isNumberValid(strBitsPerSample))
		{
			m_bApplyRaw = true;
			WmeClient.instance().SetChannels(Integer.parseInt(strChannles));
			WmeClient.instance().SetSampleRate(Integer.parseInt(strSampleRate));
			WmeClient.instance().SetBitsPerSample(Integer.parseInt(strBitsPerSample));
			mChannels = strChannles;
			mSamplerate = strSampleRate;
			mBitsPerSample = strBitsPerSample;
		}
		else
		{
            Log.i("wme_android", "bdApplyAudioRawFile false");
			m_bApplyRaw = false;
			m_cbApplyRaw.setChecked(false);
			Toast.makeText(getActivity(), "Invalid audio param", Toast.LENGTH_SHORT).show();
		}

		WmeClient.instance().EnableExternalAudioInput(m_bApplyRaw);
		
	}

	public void bdSelectRawAudioFile(String str) {
		
		String filePath = "/sdcard/" + str;
		Log.i("wme_android", "bdSelectRawAudioFile file = " + filePath);
		
		
		int[] info = getRawInfo(filePath);
		if (info != null && info.length == 3)
		{
			m_etChannels.setText(Integer.toString(info[0]));
			m_etSampleRate.setText(Integer.toString(info[1]));
			m_etBitsPerSample.setText(Integer.toString(info[2]));
			m_rawPath = filePath;
			m_tvRawPath.setText(m_rawPath);
		}
		
		WmeClient.instance().SetAudioInputFile(filePath);
	}

	public void bdApplyAudioOutputFile(boolean isChecked) {
		if (m_bApplyOutput == isChecked)
			return;
		Log.i("wme_android", "bdApplyAudioOutputFile");
		mOutputPath = "/sdcard/audio_output.pcm";
		m_bApplyOutput = isChecked;
		WmeClient.instance().EnableAudioOutputFile(m_bApplyOutput);
		
		WmeClient.instance().SetAudioOutputFile(mOutputPath);
	
	}
	
	@Override
	public synchronized void onActivityResult(final int requestCode,
			int resultCode, final Intent data) {
		if (resultCode == Activity.RESULT_OK) {
			String filePath = data.getStringExtra(FileDialog.CHOSEN_PATH);
			Log.i("wme_android", "audio file selected = " + filePath);
			
			
			//test only
			//filePath = "/sdcard/audio/abc_2_48000_16_xyz.pcm";
			int[] info = getRawInfo(filePath);
			if (info != null && info.length == 3)
			{
				m_etChannels.setText(Integer.toString(info[0]));
				m_etSampleRate.setText(Integer.toString(info[1]));
				m_etBitsPerSample.setText(Integer.toString(info[2]));
				m_rawPath = filePath;
				m_tvRawPath.setText(m_rawPath);
			}
			
			WmeClient.instance().SetAudioInputFile(filePath);
			
		} else if (resultCode == Activity.RESULT_CANCELED) {
			Log.i("wme_android", "file not selected");
		}
    }

	@Override
	public void onClick(View arg0) {
		// TODO Auto-generated method stub
		switch(arg0.getId())
		{
		case R.id.btnSelectAudioFile:
			browseFiles(m_rootPath);
			break;
		}
	}
	
	
}