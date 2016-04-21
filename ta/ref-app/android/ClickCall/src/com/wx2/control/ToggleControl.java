package com.wx2.control;

import com.wx2.clickcall.R;

import android.content.Context;
import android.content.res.TypedArray;
import android.os.Parcelable;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.ToggleButton;

public class ToggleControl extends TableRow {
	private boolean mIsCheckedDefault = true;
	private ToggleButton mToggleButton;
	
	public ToggleControl(Context context, AttributeSet attrs) {
		super(context, attrs);
		setOrientation(HORIZONTAL);
		setGravity(Gravity.CENTER);
		setWeightSum(1.0f);
		
		LayoutInflater.from(context).inflate(R.layout.toggle_control, this, true);
		TypedArray array = context.obtainStyledAttributes(attrs, R.styleable.EditControl, 0, 0);
		
		String text = array.getString(R.styleable.EditControl_label);
		if (text == null)
			text = "Label:";
		((TextView)findViewById(R.id.toggle_label)).setText(text);
		
		mIsCheckedDefault = array.getBoolean(R.styleable.EditControl_checked, true);
		mToggleButton = (ToggleButton)findViewById(R.id.toggle_value);
		mToggleButton.setId( NO_ID );
		mToggleButton.setChecked(mIsCheckedDefault);
        
        array.recycle();
	}
	
	public boolean isChecked(){
		return mToggleButton.isChecked();
	}

	public void setOnCheckedChangeListener(OnCheckedChangeListener onCheckedChangeListener) {
		mToggleButton.setOnCheckedChangeListener(onCheckedChangeListener);
	}
	
	public void setChecked(boolean isChecked){
		mToggleButton.setChecked(isChecked);
	}
	
	public void restore(){
		mToggleButton.setChecked(mIsCheckedDefault);
	}
	@Override
	public void onRestoreInstanceState(Parcelable state) {
		super.onRestoreInstanceState(state);
		restore();
	}
	@Override
	public Parcelable onSaveInstanceState() {
		mIsCheckedDefault = isChecked();
		return super.onSaveInstanceState();
	}
}
