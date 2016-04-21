package com.cisco.wmeAndroid;

import android.app.ActionBar;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.os.Bundle;
import android.util.Log;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.text.TextWatcher;
import android.text.Editable;
import android.view.View;

import com.cisco.webex.wme.WmeClient;

/**
 * Created by C3-Team on 5/14/14.
 */
public class QoSSettingFragment extends Fragment implements ActionBar.TabListener, View.OnClickListener, CompoundButton.OnCheckedChangeListener {
    CheckBox m_checkEnableQoS;
    boolean m_qosEnabled = true;
    float m_maxLossRatio = 0.0f;
    int m_minBandwidth = 0;
    int m_initBandwidth = 0;
    boolean m_maxLossRatioChanged = false;
    boolean m_minBandwidthChanged = false;
    boolean m_initBandwidthChanged = false;

    public QoSSettingFragment() {
        Log.i("wme_android", "QoSSettingFragment constructor");
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //TODO: add widgets
        getActivity().setContentView(R.layout.qossetting);

        m_checkEnableQoS = (CheckBox)getActivity().findViewById(R.id.checkBoxEnableQoS);
        if (m_checkEnableQoS != null) {
            m_checkEnableQoS.setChecked(m_qosEnabled);
            m_checkEnableQoS.setOnCheckedChangeListener(this);
        }
        
        EditText editMaxLossRatio = (EditText)getActivity().findViewById(R.id.editTextQoSMaxLossRatio);
        if (editMaxLossRatio != null) {
            editMaxLossRatio.setText(Float.toString(m_maxLossRatio));
        	editMaxLossRatio.setOnClickListener(this);
            editMaxLossRatio.addTextChangedListener(new TextWatcher(){
                public void afterTextChanged(Editable s) {
                    String str = s.toString();
                    if(0 == str.length()){
                        return;
                    }
                    m_maxLossRatio = Float.parseFloat(str);
                    WmeClient.instance().SetQoSMaxLossRatio(m_maxLossRatio);
                }
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
                public void onTextChanged(CharSequence s, int start, int before, int count) { m_maxLossRatioChanged = true; }
            });
        }
        
        EditText editMinBandwidth = (EditText)getActivity().findViewById(R.id.editTextQoSMinBandwidth);
        if (editMinBandwidth != null) {
            editMinBandwidth.setText(Integer.toString(m_minBandwidth));
        	editMinBandwidth.setOnClickListener(this);
            editMinBandwidth.addTextChangedListener(new TextWatcher(){
                public void afterTextChanged(Editable s) {
                    String str = s.toString();
                    if(0 == str.length()){
                        return;
                    }
                    m_minBandwidth = Integer.parseInt(str);
                    WmeClient.instance().SetQoSMinBandwidth(m_minBandwidth);
                }
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
                public void onTextChanged(CharSequence s, int start, int before, int count) { m_minBandwidthChanged = true; }
            });
        }
        
        EditText editInitBandwidth = (EditText)getActivity().findViewById(R.id.editTextQoSInitBandwidth);
        if (editInitBandwidth != null) {
            editInitBandwidth.setText(Integer.toString(m_initBandwidth));
        	editInitBandwidth.setOnClickListener(this);
            editInitBandwidth.addTextChangedListener(new TextWatcher(){
                public void afterTextChanged(Editable s) {
                    String str = s.toString();
                    if(0 == str.length()){
                        return;
                    }
                    m_initBandwidth = Integer.parseInt(str);
                    WmeClient.instance().SetInitialBandwidth(m_initBandwidth);
                }
                public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
                public void onTextChanged(CharSequence s, int start, int before, int count) { m_initBandwidthChanged = true; }
            });
        }

        Log.i("wme_android", "QoSSettingFragment::onCreate");
    }

    @Override
    public void onTabReselected(ActionBar.Tab arg0, FragmentTransaction arg1) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onTabSelected(ActionBar.Tab tab, FragmentTransaction ft) {
        // TODO Auto-generated method stub

        Log.i("wme_android","QoSSettingFragment::onTabSelected");

        ft.add(android.R.id.content, this);
        //ft.attach(this);
    }

    @Override
    public void onTabUnselected(ActionBar.Tab tab, FragmentTransaction ft) {
        // TODO Auto-generated method stub
        Log.i("wme_android","QoSSettingFragment::onTabUnselected");
        ft.remove(this);
    }
    
    @Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch(v.getId())
		{
            case R.id.editTextQoSMaxLossRatio:
                break;
            case R.id.editTextQoSMinBandwidth:
                break;
            case R.id.editTextQoSInitBandwidth:
                break;
            default:
                break;
		}
	}

    @Override
    public void onCheckedChanged(CompoundButton arg0, boolean isChecked) {
        // TODO Auto-generated method stub
        switch(arg0.getId())
        {
            case R.id.checkBoxEnableQoS:
                m_qosEnabled = isChecked;
                WmeClient.instance().EnableQoS(isChecked);
                break;
        }
    }
}
