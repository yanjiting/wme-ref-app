package com.cisco.wmeAndroid;

import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;
import android.opengl.GLSurfaceView;
import android.util.Log;

public class MyGLRenderer implements GLSurfaceView.Renderer {
	FPSCounter fps_counter = null;
	
    public void onSurfaceCreated(GL10 gl, EGLConfig config) {
        // Do nothing special.
    	fps_counter = new FPSCounter();
    }

    public void onSurfaceChanged(GL10 gl, int w, int h) {
        gl.glViewport(0, 0, w, h);
    }

    public void onDrawFrame(GL10 gl) {
        gl.glClearColor(mRed, mGreen, mBlue, 1.0f);
        gl.glClear(GL10.GL_COLOR_BUFFER_BIT | GL10.GL_DEPTH_BUFFER_BIT);
        
        if (fps_counter != null)
        	fps_counter.logFrame();
    }

    public void setColor(float r, float g, float b) {
        mRed = r;
        mGreen = g;
        mBlue = b;
    }
    
    
    private class FPSCounter {  
        long startTime = System.nanoTime();  
        int frames = 0;  
          
       /** 
        * 计算每秒执行了多少次 
        */  
        public void logFrame() {  
            frames++;  
            if(System.nanoTime() - startTime >= 1000000000) {  
                Log.d("FPSCounter", "fps: " + frames);  
                frames = 0;  
                startTime = System.nanoTime();  
            }  
        }  
    }  

    private float mRed;
    private float mGreen;
    private float mBlue;
}

