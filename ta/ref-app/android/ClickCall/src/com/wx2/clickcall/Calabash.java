package com.wx2.clickcall;

import com.webex.wme.MediaConnection;
import com.webex.wme.MediaTrack;

import java.util.ArrayList;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

/**
 * Created by dan on 8/14/14.
 */
public class Calabash {
    private ArrayList<OnMediaReadyArgs> onMediaReadyInfoList = new ArrayList<OnMediaReadyArgs>();
    private JSONObject mCSIChangeHistory = new JSONObject();

    public static class OnMediaReadyArgs {
        int mid;
        MediaConnection.MediaDirection dir;
        MediaConnection.MediaType type;
        MediaTrack track;
    }
    
    public String GetCSIChangeHistory() {
    	return mCSIChangeHistory.toString();
    }

    /**
     * onMediaReady: Store and verify callback variables
     * @param mid
     * @param dir
     * @param type
     * @param track
     * @return
     */
    public void onMediaReadyStore (int mid, MediaConnection.MediaDirection dir, MediaConnection.MediaType type, MediaTrack track) {
        OnMediaReadyArgs myArgs = new OnMediaReadyArgs();
        myArgs.mid = mid;
        myArgs.dir = dir;
        myArgs.type = type;
        myArgs.track = track;

        onMediaReadyInfoList.add(myArgs);
    }

    public void onCSIChanged(String mediaType, long vid, long[] newCSIArray) {
		try {
	    	if(!mCSIChangeHistory.has(mediaType))
					mCSIChangeHistory.put(mediaType, new JSONObject());
	    	JSONObject csiMedia = mCSIChangeHistory.optJSONObject(mediaType);
	    	String sVid = String.valueOf(vid);
	    	if(!csiMedia.has(sVid))
	    		csiMedia.put(sVid, new JSONArray());
	    	JSONArray csiArray = csiMedia.optJSONArray(sVid);
	    	JSONArray newCSIs = new JSONArray();
	    	for(long csi : newCSIArray) {
	    		newCSIs.put(csi);
	    	}
	    	csiArray.put(newCSIs);
		} catch (JSONException e) {
			e.printStackTrace();
		}
    }

    public ArrayList<OnMediaReadyArgs> getOnMediaReadyInfoList() {
        return onMediaReadyInfoList;
    }
}
