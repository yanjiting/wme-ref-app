package com.wx2.control;

import com.wx2.clickcall.R;

import android.content.Context;
import android.content.res.TypedArray;
import android.os.Parcelable;
import android.util.AttributeSet;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.AutoCompleteTextView;
import android.widget.Button;
import android.widget.TableRow;
import android.widget.TextView;

public class EditControl extends TableRow{
	private String mDefaultText;
	private AutoCompleteTextView mTextView;
	public EditControl(Context context, AttributeSet attrs) {
		super(context, attrs);
		setOrientation(HORIZONTAL);
		setGravity(Gravity.CENTER);
		setWeightSum(1.0f);
		
		LayoutInflater.from(context).inflate(R.layout.edit_control, this, true);
		TypedArray array = context.obtainStyledAttributes(attrs, R.styleable.EditControl, 0, 0);
		
		String text = array.getString(R.styleable.EditControl_label);
		if (text == null)
			text = "Label:";
		((TextView)findViewById(R.id.description)).setText(text);
		
		mDefaultText = array.getString(R.styleable.EditControl_text);
		if (mDefaultText == null) 
			mDefaultText = "";
		mTextView = (AutoCompleteTextView)findViewById(R.id.edit_view);
		mTextView.setText(mDefaultText);
		mTextView.setId(NO_ID);
		
		text = array.getString(R.styleable.EditControl_values);
		String[] linusServers = {};
		if(text != null)
			linusServers = text.split("\\s*;\\s*");
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this.getContext(), 
        		android.R.layout.simple_dropdown_item_1line, linusServers);
        mTextView.setThreshold(1);
        mTextView.setAdapter(adapter);
        
        Button btnClear = (Button)findViewById(R.id.clear_button);
        btnClear.setOnClickListener(new OnClickListener(){
            @Override
            public void onClick(View view) {
            	mTextView.setText("");
            }
        });
		array.recycle();
	}
	
	public String getText(){
		return mTextView.getText().toString();
	}
	
	public void setText(String txt) {
		mTextView.setText(txt);
	}
	
	public void restore(){
		setText(mDefaultText);
	}
	
	@Override
	public Parcelable onSaveInstanceState() {
		mDefaultText = getText();
		return super.onSaveInstanceState();
	}
	@Override
	public void onRestoreInstanceState(Parcelable state) {
		super.onRestoreInstanceState(state);
		restore();
	}
}
