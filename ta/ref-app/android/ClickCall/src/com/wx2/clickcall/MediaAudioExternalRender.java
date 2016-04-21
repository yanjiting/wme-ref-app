package com.wx2.clickcall;

import android.util.Log;
import com.webex.wme.*;
import com.webex.wme.MediaTrack.ExternalAudioRenderType;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Date;
/**
 * Created by alfang on 2/04/15. based on boris
 */
public class MediaAudioExternalRender implements IWmeExternalRender {
    private String mDebugDumpFilename = "";
    private long mCount = 0;
    public MediaAudioExternalRender(ExternalAudioRenderType nType)
    {
    	switch(nType)
    	{
			case CaptureFromHardware:
			    mDebugDumpFilename = "/sdcard/CaptureRaw_"+new Date().getTime()+".pcm";
			    break;
			case CaptureBeforeEncode:
			    mDebugDumpFilename = "/sdcard/CaptureBeforeEncode_"+new Date().getTime()+".pcm";
			    break;
			case PlaybackToHardware:
			    mDebugDumpFilename = "/sdcard/Playback_"+new Date().getTime()+".pcm";
			    break;
			default:
			    break;
    	}
    	Log.i("MediaAudioExternalRender","File name:"+mDebugDumpFilename);
    }
    @Override
    public void RenderMediaData(int nTimestamp,
                                int nMediaType, // 0: AudioRaw   2: VideoRaw
                                Object objFormat, //Depends on the nMediaType. nMediaType: 0 (WmeAudioRawFormat) nMediaType: 1 (WmeVideoRawFormat)
                                byte[] RawData,
                                int nLength)
    {
    	//Log.i("MediaAudioExternalRender","RenderMediaData,filename:"+mDebugDumpFilename+",nMediaType:"+nMediaType);
        if (nMediaType == 0) {
            WmeAudioRawFormat wmeFormat = (WmeAudioRawFormat) objFormat;
            int nRawType = wmeFormat.GetRawType();
            int nChannel = wmeFormat.GetChannels();
            int nSampleRate = wmeFormat.GetSampleRate();
            int nBitPerSample = wmeFormat.GetBitsPerSample();
            try {
                dumpData(RawData);
                mCount += nLength;
            } catch (IOException ie) {
                ie.printStackTrace();
            }
        }
    }
    @Override
    public long RegisterRequestAnswerer(IWmeExternalRenderAnswerer answerer){
        return -1;
    }

    @Override
    public long UnregisterRequestAnswerer(){
        return -1;
    }

    private void dumpData(byte[] rawData)throws IOException{//Only for test
        try{
            boolean bAppend = true;
            if (mCount >= (1024 *1024) || mCount == 0){
                mCount = 0;
                bAppend = false;
            }
            //Log.i("MediaAudioExternalRender","dumpData,filename:"+mDebugDumpFilename);
            FileOutputStream fout = new FileOutputStream(mDebugDumpFilename,bAppend);
            fout.write(rawData);
            fout.close();
        }
        catch(Exception e){
            e.printStackTrace();
        }
    }
}
