package com.cisco.wmeAndroid;
 
import com.cisco.wmeAndroid.R;

import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;
import android.app.ActionBar.Tab;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.app.ActionBar;
import android.content.Intent;
 
public class IceFragment extends Fragment implements ActionBar.TabListener {
 
    private Fragment mFragment;
    private Switch mUserSwitch;
    private EditText mToEdit;
    private TextView mToText;
    private Button mConnectBttn;
 
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Get the view from fragment1.xml
        getActivity().setContentView(R.layout.icestun);
        
        // Sync XML views with Java
        mUserSwitch = (Switch) getActivity().findViewById(R.id.iceSwitch);
        mToEdit = (EditText) getActivity().findViewById(R.id.iceToEdit);
        mToText = (TextView) getActivity().findViewById(R.id.iceToText);
        mConnectBttn = (Button) getActivity().findViewById(R.id.iceConnectBttn);
        
        // Setup Listeners
        mUserSwitch.setOnCheckedChangeListener(mUserListen);
        mConnectBttn.setOnClickListener(mConnectListen);
        
        // Hide Host IP entries for Host, as application launches
        mToEdit.setVisibility(View.INVISIBLE);
		mToText.setVisibility(View.INVISIBLE);
		
		
    }
    
    private OnClickListener mConnectListen = new OnClickListener() {
    	public void onClick (View v) {
    		// TODO ADD IF CONDITIONS WHEN ENTRIES ARE EMPTY

    		Toast.makeText(getActivity().getApplicationContext(), "ICE Connection NOT available",Toast.LENGTH_LONG).show();
    		
    		/*
    		Intent i = new Intent(getActivity(), Meeting.class);     
    	    startActivity(i);
    	    getActivity().finish();
    	    */
    	    
    	}
    };
    
    private OnCheckedChangeListener mUserListen = new OnCheckedChangeListener() {
    	public void onCheckedChanged (CompoundButton buttonView, boolean isChecked) {
    		if (isChecked) {
    			mToEdit.setVisibility(View.VISIBLE);
    			mToText.setVisibility(View.VISIBLE);
    		}
    		else {
    			mToEdit.setVisibility(View.INVISIBLE);
    			mToText.setVisibility(View.INVISIBLE);
    		}
    		
    	}
    };
 
    public void onTabSelected(Tab tab, FragmentTransaction ft) {
        // TODO Auto-generated method stub
        mFragment = new IceFragment();
        // Attach fragment1.xml layout
        ft.add(android.R.id.content, mFragment);
        ft.attach(mFragment);
    }
 
    public void onTabUnselected(Tab tab, FragmentTransaction ft) {
        // TODO Auto-generated method stub
        // Remove fragment1.xml layout
        ft.remove(mFragment);
    }
 
    public void onTabReselected(Tab tab, FragmentTransaction ft) {
        // TODO Auto-generated method stub
 
    }
 
}