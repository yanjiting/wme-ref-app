package com.cisco.wmeAndroid;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.BatteryManager;


public class BatteryHelper extends BroadcastReceiver {
	public int battery_percent = 0;
	public boolean charging = false;

	@Override
	public void onReceive(Context context, Intent intent) {
		if (Intent.ACTION_BATTERY_CHANGED.equals(intent.getAction())) {
            int level = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0);
            int scale = intent.getIntExtra(BatteryManager.EXTRA_SCALE, 0);
            battery_percent = level * 100 / scale;
            
            int status = intent.getIntExtra("status", 0);
            if (status == BatteryManager.BATTERY_STATUS_CHARGING || status == BatteryManager.BATTERY_STATUS_FULL)
            {
            	charging  = true;
            }
        }
	}
	
}