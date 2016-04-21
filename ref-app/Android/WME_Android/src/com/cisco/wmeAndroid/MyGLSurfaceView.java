package com.cisco.wmeAndroid;

import android.content.Context;
import android.opengl.GLSurfaceView;
import android.util.AttributeSet;
import android.view.MotionEvent;

public class MyGLSurfaceView extends GLSurfaceView {
	MyGLRenderer mRenderer;
	public MyGLSurfaceView(Context context, AttributeSet attrs) {
		super(context, attrs);
		mRenderer = new MyGLRenderer();
		setRenderer(mRenderer);
	}

	public boolean onTouchEvent(final MotionEvent event) {
		queueEvent(new Runnable(){
			public void run() {
				float x = event.getX();
				float y = event.getY();
				int w = getWidth();
				int h = getHeight();
				float red = x > w ? (w/x) : (x/w);
				float green = y > h ? (h/y) : (y/h);
				mRenderer.setColor(red,
						green, 1.0f);
			}});
		return true;
	}


}

