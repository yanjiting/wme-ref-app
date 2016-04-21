package com.wx2.clickcall;

import android.util.Log;
import com.webex.wme.*;

import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;


/**
 * Created by boris on 1/28/15.
 */
public class MediaAudioPairingExternalRender implements IWmeExternalRender {
    private String mDebugDumpFilename = "/sdcard/audiopairingdumpfile.pcm";
    private long mCount = 0;
    @Override
    public void RenderMediaData(int nTimestamp,
                                int nMediaType, // 0: AudioRaw   2: VideoRaw
                                Object objFormat, //Depends on the nMediaType. nMediaType: 0 (WmeAudioRawFormat) nMediaType: 1 (WmeVideoRawFormat)
                                byte[] RawData,
                                int nLength)
    {
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
            FileOutputStream fout = new FileOutputStream(mDebugDumpFilename,bAppend);
            fout.write(rawData);
            fout.close();
        }
        catch(Exception e){
            e.printStackTrace();
        }
    }
}
