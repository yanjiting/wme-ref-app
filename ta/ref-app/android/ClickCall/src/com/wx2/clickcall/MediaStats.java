package com.wx2.clickcall;

import org.json.JSONException;
import org.json.JSONObject;

public class MediaStats {
    String localIP;
    int localAudioPort;
    int localVideoPort;

    String remoteIP;
    int remoteAudioPort;
    int remoteVideoPort;

    int audioRoundTripTime;
    int audioPacketsSent;
    float audioLossRatioSent;
    int audioJitterSent;
    int audioBytesSent;
    int audioBytesReceived;
    int audioPacketsReceived;
    float audioLossRatioReceived;
    int audioJitterReceived;

    int audioBitrateSent;
    int audioBitrateReceived;

    int videoRoundTripTime;
    int videoPacketsSent;
    int videoBytesSent;
    float videoLossRatioSent;
    int videoJitterSent;
    int videoBytesReceived;
    float videoLossRatioReceived;
    int videoJitterReceived;
    int videoPacketsReceived;

    int videoResolutionWidthSent;
    int videoResolutionHeightSent;
    int videoFramesPerSecSent;
    int videoBitrateSent;

    int videoResolutionWidthReceived;
    int videoResolutionHeightReceived;
    int videoFramesPerSecReceived;
    int videoBitrateReceived;
    int videoFramesRenderCount;

    public MediaStats() {
    }

    public String toString(){
        String retStr = "";

        retStr += "LocalIP:" + localIP + "\r\n";
        retStr += "RemoteIP:" + remoteIP + "\r\n";
        retStr += "Audio:\r\n";
        retStr += "  RTT:" + audioRoundTripTime + "\r\n";
        retStr += "  Sent(" + localAudioPort + "):\r\n";
        retStr += String.format("    Bytes(Packets):%d(%d)\r\n", audioBytesSent, audioPacketsSent);
        retStr += String.format("    Loss(Jitter):%s(%d)\r\n", audioLossRatioSent, audioJitterSent);
        retStr += "    Bitrate:" + audioBitrateSent * 8 + "\r\n";
        retStr += "  Received(" + remoteAudioPort + "):\r\n";
        retStr += String.format("    Bytes(Packets):%d(%d)\r\n", audioBytesReceived, audioPacketsReceived);
        retStr += String.format("    Loss(Jitter):%s(%d)\r\n", audioLossRatioReceived, audioJitterReceived);
        retStr += "    Bitrate:" + audioBitrateReceived * 8 + "\r\n";
        retStr += "Video:\r\n";
        retStr += "  RTT:" + videoRoundTripTime + "\r\n";
        retStr += "  Sent(" + localVideoPort + "):\r\n";
        retStr += String.format("    Bytes(Packets):%d(%d)\r\n", videoBytesSent, videoPacketsSent);
        retStr += String.format("    Loss(Jitter):%s(%d)\r\n", videoLossRatioSent, videoJitterSent);
        retStr += String.format("    Size:%dx%d (%d)\r\n", videoResolutionWidthSent, videoResolutionHeightSent, videoFramesPerSecSent);
        retStr += "    Bitrate:" + videoBitrateSent + "\r\n";
        retStr += "  Received(" + remoteVideoPort + "):\r\n";
        retStr += String.format("    Bytes(Packets):%d(%d)\r\n", videoBytesReceived, videoPacketsReceived);
        retStr += String.format("    Loss(Jitter):%s(%d)\r\n", videoLossRatioReceived, videoJitterReceived);
        retStr += String.format("    Size:%dx%d (%d)\r\n", videoResolutionWidthReceived, videoResolutionHeightReceived, videoFramesPerSecReceived);
        retStr += "    Bitrate:" + videoBitrateReceived + "\r\n";
        retStr += "    Frames:" + videoFramesRenderCount + "\r\n";

        return retStr;
    }

    public JSONObject toJSON() {
        JSONObject root = new JSONObject();
        try {
            JSONObject audioConn = new JSONObject();
            audioConn.put("localIp", localIP);
            audioConn.put("remoteIp", remoteIP);
            audioConn.put("uLocalPort", localAudioPort);
            audioConn.put("uRemotePort", remoteAudioPort);
            root.put("audioConn", audioConn);

            JSONObject videoConn = new JSONObject();
            videoConn.put("localIp", localIP);
            videoConn.put("remoteIp", remoteIP);
            videoConn.put("uLocalPort", localVideoPort);
            videoConn.put("uRemotePort", remoteVideoPort);
            root.put("videoConn", videoConn);

            JSONObject videoOutNet = new JSONObject();
            videoOutNet.put("uPackets", videoPacketsSent);
            videoOutNet.put("fLossRatio", videoLossRatioSent);
            videoOutNet.put("uJitter", videoJitterSent);
            videoOutNet.put("uRoundTripTime", audioRoundTripTime);
            videoOutNet.put("uBytes", videoBytesSent);
            root.put("videoOutNet", videoOutNet);

            JSONObject videoInNet = new JSONObject();
            videoInNet.put("uPackets", videoPacketsReceived);
            videoInNet.put("fLossRatio", videoLossRatioReceived);
            videoInNet.put("uJitter", videoJitterReceived);
            videoInNet.put("uRoundTripTime", videoRoundTripTime);
            videoInNet.put("uBytes", videoBytesReceived);
            root.put("videoInNet", videoInNet);

            JSONObject audioInNet = new JSONObject();
            audioInNet.put("uPackets", audioPacketsReceived);
            audioInNet.put("fLossRatio", audioLossRatioReceived);
            audioInNet.put("uJitter", audioJitterReceived);
            audioInNet.put("uRoundTripTime", audioRoundTripTime);
            audioInNet.put("uBytes", audioBytesReceived);
            root.put("audioInNet", audioInNet);

            JSONObject audioOutNet = new JSONObject();
            audioOutNet.put("uPackets", audioPacketsSent);
            audioOutNet.put("fLossRatio", audioLossRatioSent);
            audioOutNet.put("uJitter", audioJitterSent);
            audioOutNet.put("uRoundTripTime", audioRoundTripTime);
            audioOutNet.put("uBytes", audioBytesSent);
            root.put("audioOutNet", audioOutNet);

            JSONObject audioRemote = new JSONObject();
            audioRemote.put("uBitRate", audioBitrateReceived);
            root.put("audioRemote", audioRemote);

            JSONObject audioLocal = new JSONObject();
            audioLocal.put("uBitRate", audioBitrateSent);
            root.put("audioLocal", audioLocal);

            JSONObject videoLocal = new JSONObject();
            videoLocal.put("uWidth", videoResolutionWidthSent);
            videoLocal.put("uHeight", videoResolutionHeightSent);
            videoLocal.put("fFrameRate", videoFramesPerSecSent);
            videoLocal.put("fBitRate", videoBitrateSent);
            root.put("videoLocal", videoLocal);

            JSONObject videoRemote = new JSONObject();
            videoRemote.put("uWidth", videoResolutionWidthReceived);
            videoRemote.put("uHeight", videoResolutionHeightReceived);
            videoRemote.put("fFrameRate", videoFramesPerSecReceived);
            videoRemote.put("fBitRate", videoBitrateReceived);
            videoRemote.put("uRenderFrameCount", videoFramesRenderCount);
            root.put("videoRemote", videoRemote);

        } catch (JSONException e) {
            e.printStackTrace();
        }

        return root;
    }

    public int getRemoteVideoPort() {
        return remoteVideoPort;
    }

    public void setRemoteVideoPort(int remoteVideoPort) {
        this.remoteVideoPort = remoteVideoPort;
    }

    public int getRemoteAudioPort() {
        return remoteAudioPort;
    }

    public void setRemoteAudioPort(int remoteAudioPort) {
        this.remoteAudioPort = remoteAudioPort;
    }

    public int getLocalVideoPort() {
        return localVideoPort;
    }

    public void setLocalVideoPort(int localVideoPort) {
        this.localVideoPort = localVideoPort;
    }

    public int getLocalAudioPort() {
        return localAudioPort;
    }

    public void setLocalAudioPort(int localAudioPort) {
        this.localAudioPort = localAudioPort;
    }

    public String getRemoteIP() {
        return remoteIP;
    }

    public void setRemoteIP(String remoteIP) {
        this.remoteIP = remoteIP;
    }

    public String getLocalIP() {
        return localIP;
    }

    public void setLocalIP(String localIP) {
        this.localIP = localIP;
    }

    public int getAudioRoundTripTime() {
        return audioRoundTripTime;
    }

    public void setAudioRoundTripTime(int audioRoundTripTime) {
        this.audioRoundTripTime = audioRoundTripTime;
    }

    public int getVideoRoundTripTime() {
        return videoRoundTripTime;
    }

    public void setVideoRoundTripTime(int videoRoundTripTime) {
        this.videoRoundTripTime = videoRoundTripTime;
    }

    public int getAudioPacketsSent() {
        return audioPacketsSent;
    }

    public void setAudioPacketsSent(int audioPacketsSent) {
        this.audioPacketsSent = audioPacketsSent;
    }

    public int getAudioPacketsReceived() {
        return audioPacketsReceived;
    }

    public void setAudioPacketsReceived(int audioPacketsReceived) {
        this.audioPacketsReceived = audioPacketsReceived;
    }


    public int getAudioBitrateSent() {
        return audioBitrateSent;
    }

    public void setAudioBitrateSent(int audioBitrateSent) {
        this.audioBitrateSent = audioBitrateSent;
    }

    public int getAudioBitrateReceived() {
        return audioBitrateReceived;
    }

    public void setAudioBitrateReceived(int audioBitrateReceived) {
        this.audioBitrateReceived = audioBitrateReceived;
    }

    public int getVideoPacketsSent() {
        return videoPacketsSent;
    }

    public void setVideoPacketsSent(int videoPacketsSent) {
        this.videoPacketsSent = videoPacketsSent;
    }

    public int getVideoPacketsReceived() {
        return videoPacketsReceived;
    }

    public void setVideoPacketsReceived(int videoPacketsReceived) {
        this.videoPacketsReceived = videoPacketsReceived;
    }

    public int getVideoResolutionWidthSent() {
        return videoResolutionWidthSent;
    }

    public void setVideoResolutionWidthSent(int videoResolutionWidthSent) {
        this.videoResolutionWidthSent = videoResolutionWidthSent;
    }

    public int getVideoResolutionHeightSent() {
        return videoResolutionHeightSent;
    }

    public void setVideoResolutionHeightSent(int videoResolutionHeightSent) {
        this.videoResolutionHeightSent = videoResolutionHeightSent;
    }

    public int getVideoFramesPerSecSent() {
        return videoFramesPerSecSent;
    }

    public void setVideoFramesPerSecSent(int videoFramesPerSecSent) {
        this.videoFramesPerSecSent = videoFramesPerSecSent;
    }
    
    public int getVideoFramesRenderCount() {
        return videoFramesRenderCount;
    }
    
    public void setVideoFramesRenderCount(int videoFramesRenderCount) {
        this.videoFramesRenderCount = videoFramesRenderCount;
    }

    public int getVideoBitrateSent() {
        return videoBitrateSent;
    }

    public void setVideoBitrateSent(int videoBitrateSent) {
        this.videoBitrateSent = videoBitrateSent;
    }

    public int getVideoResolutionWidthReceived() {
        return videoResolutionWidthReceived;
    }

    public void setVideoResolutionWidthReceived(int videoResolutionWidthReceived) {
        this.videoResolutionWidthReceived = videoResolutionWidthReceived;
    }

    public int getVideoResolutionHeightReceived() {
        return videoResolutionHeightReceived;
    }

    public void setVideoResolutionHeightReceived(int videoResolutionHeightReceived) {
        this.videoResolutionHeightReceived = videoResolutionHeightReceived;
    }

    public int getVideoFramesPerSecReceived() {
        return videoFramesPerSecReceived;
    }

    public void setVideoFramesPerSecReceived(int videoFramesPerSecReceived) {
        this.videoFramesPerSecReceived = videoFramesPerSecReceived;
    }

    public int getVideoBitrateReceived() {
        return videoBitrateReceived;
    }

    public void setVideoBitrateReceived(int videoBitrateReceived) {
        this.videoBitrateReceived = videoBitrateReceived;
    }

    public int getAudioJitterReceived() {
        return audioJitterReceived;
    }

    public void setAudioJitterReceived(int audioJitterReceived) {
        this.audioJitterReceived = audioJitterReceived;
    }

    public float getAudioLossRatioReceived() {

        return audioLossRatioReceived;
    }

    public void setAudioLossRatioReceived(float audioLossRatioReceived) {
        this.audioLossRatioReceived = audioLossRatioReceived;
    }

    public int getAudioJitterSent() {

        return audioJitterSent;
    }

    public void setAudioJitterSent(int audioJitterSent) {
        this.audioJitterSent = audioJitterSent;
    }

    public float getAudioLossRatioSent() {

        return audioLossRatioSent;
    }

    public void setAudioLossRatioSent(float audioLossRatioSent) {
        this.audioLossRatioSent = audioLossRatioSent;
    }

    public int getVideoBytesSent() {
        return videoBytesSent;
    }

    public void setVideoBytesSent(int videoBytesSent) {
        this.videoBytesSent = videoBytesSent;
    }

    public float getVideoLossRatioSent() {
        return videoLossRatioSent;
    }

    public void setVideoLossRatioSent(float videoLossRatioSent) {
        this.videoLossRatioSent = videoLossRatioSent;
    }

    public int getVideoJitterSent() {
        return videoJitterSent;
    }

    public void setVideoJitterSent(int videoJitterSent) {
        this.videoJitterSent = videoJitterSent;
    }

    public int getVideoBytesReceived() {
        return videoBytesReceived;
    }

    public void setVideoBytesReceived(int videoBytesReceived) {
        this.videoBytesReceived = videoBytesReceived;
    }

    public float getVideoLossRatioReceived() {
        return videoLossRatioReceived;
    }

    public void setVideoLossRatioReceived(float videoLossRatioReceived) {
        this.videoLossRatioReceived = videoLossRatioReceived;
    }

    public int getVideoJitterReceived() {
        return videoJitterReceived;
    }

    public void setVideoJitterReceived(int videoJitterReceived) {
        this.videoJitterReceived = videoJitterReceived;
    }

    public int getAudioBytesReceived() {

        return audioBytesReceived;
    }

    public void setAudioBytesReceived(int audioBytesReceived) {
        this.audioBytesReceived = audioBytesReceived;
    }

    public int getAudioBytesSent() {

        return audioBytesSent;
    }

    public void setAudioBytesSent(int audioBytesSent) {
        this.audioBytesSent = audioBytesSent;
    }
}
