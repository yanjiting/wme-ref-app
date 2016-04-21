package com.cisco.wme.unittest;

import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.view.View;
import android.widget.TextView;
import android.util.Log;
import android.os.Process;

public class MainActivity extends Activity {
	final boolean mGdbDebug = false;
	private TextView mStatusView;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		mStatusView = (TextView)findViewById(R.id.status_view);
		CharSequence text = "Go to run ....";
		mStatusView.setText(text);
		
		if (mGdbDebug) {
			UnitTestNative.preload_symbols(); 
			mStatusView.setOnClickListener(new View.OnClickListener() {
				public void onClick(View v) {
					String module = "shark";
					String path = "/sdcard/wme-shark.xml";
					UnitTestNative.GotoUnittest(module, path);
				}
			});
		}else {
			runUnitTest();
		}
	}

	@Override
	public void onDestroy() {
		Process.killProcess(Process.myPid());
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

    // am start -es string(key:value) -ei int(key:value)
    // am start -es module "wme" -es path "/sdcard"
	public void runUnitTest() {
		Thread thread = new Thread() {
			@Override
			public void run() {
				Log.i("wme_unittest", "WME unittest begin");
                String module = getIntent().getStringExtra("module");
                if (module.length() <= 0) {
                    module = "all";
                }
                String path = getIntent().getStringExtra("path");
                if (path.length() <= 0) {
                    path = "/sdcard/wme-all-gtests.xml";
                }

                CharSequence text = "Run [" + module + "] unittesting..., xml=" + path;
                mStatusView.setText(text);

				Log.i("wme_unittest", "WME unittest running, module=" + module + ", path=" + path);
				UnitTestNative.GotoUnittest(module, path);
				Log.i("wme_unittest", "WME unittest end, pid=" + Process.myPid());
				Process.killProcess(Process.myPid());
				finish();
			}
		};
		
		thread.start();
	}
}

