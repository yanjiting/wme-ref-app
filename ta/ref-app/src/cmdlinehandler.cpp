#include "ClickCall.h"
#include "Loopback.h"
#include "PeerCall.h"
#include "BackdoorAgent.h"
#include "atdef.h"

char* getCmdOption(char ** begin, char ** end, const std::string & option) {
	char ** itr = std::find(begin, end, option);
	if (itr != end && ++itr != end) {
		return *itr;
	}
	return NULL;
}

bool cmdOptionExists(char** begin, char** end, const std::string& option) {
	return std::find(begin, end, option) != end;
}

extern "C" const char* getLogName() {
    if(TestConfig::i().m_sLogFileName.empty())
        return NULL;
    return TestConfig::i().m_sLogFileName.c_str();
}

void setVideoSize()
{
	Json::Value decode(Json::arrayValue);
	if (TestConfig::i().m_nVideoSize != 2)
	{
		std::string sProfileID = "420028";
		if (TestConfig::i().m_nVideoSize == 3)
			sProfileID = "420028";
		else if (TestConfig::i().m_nVideoSize == 1)
			sProfileID = "42e014";
		else if (TestConfig::i().m_nVideoSize == 0)
			sProfileID = "42e00a";

		Json::Value svc(Json::objectValue);
		svc["codec"] = "H264-SVC";
		svc["uProfileLevelID"] = sProfileID;
		svc["max-fps"] = TestConfig::i().m_nMaxFps;

		Json::Value avc(Json::objectValue);
		avc["codec"] = "H264";
		avc["uProfileLevelID"] = sProfileID;
		avc["max-fps"] = TestConfig::i().m_nMaxFps;
		decode[0] = svc;
		decode[1] = avc;
	}
	else{
		if (TestConfig::i().m_nMaxFps < 3000){
		}
	}
	TestConfig::i().m_videoParam["decodeCodec"] = decode;
	if (TestConfig::i().m_nMaxFps < 3000)
		TestConfig::i().m_videoParam["encodeCodec"] = decode;
}

void InitCmdArguments2(int argc, char **argv){

	char* szTAPort = getCmdOption(argv, argv + argc, "--ta");
	if (szTAPort)
		TestConfig::Instance().m_nTAPort = atoi(szTAPort);

#ifndef WP8
	TestConfig::Instance().m_bTraceToWbxTraceFile = cmdOptionExists(argv, argv + argc, "--trace-2-wbxtracefile") || getenv("WME_TEST_TRACE_2_WBXTRACEFILE");
#endif
	TestConfig::Instance().m_bSharer = cmdOptionExists(argv, argv + argc, "--sharer");
    TestConfig::i().m_bAppshare = cmdOptionExists(argv, argv + argc, "--appshare");
    TestConfig::i().m_bUsingVPIO = !cmdOptionExists(argv, argv + argc, "--remoteIO");
    TestConfig::i().m_bUsingTCAEC = cmdOptionExists(argv, argv + argc, "--tc-aec");
    TestConfig::Instance().m_bTrace2File = !cmdOptionExists(argv, argv + argc, "--no-trace-2-file");
    TestConfig::i().m_bVideoHW = cmdOptionExists(argv, argv + argc, "--video-hardware");
    TestConfig::i().m_bQoSEnable = !cmdOptionExists(argv, argv + argc, "--disable-qos");
    TestConfig::i().m_bDPC = !cmdOptionExists(argv, argv + argc, "--disable-dpc");
	TestConfig::i().m_bASPreview = cmdOptionExists(argv, argv + argc, "--appshare-preview");
    TestConfig::i().m_bAutoRequestFloor = !cmdOptionExists(argv, argv + argc, "--appshare-no-requestfloor");
	TestConfig::i().m_bAutoStart = cmdOptionExists(argv, argv + argc, "--auto-start");
	//	TestConfig::i().m_bFakeVideoByShare = cmdOptionExists(argv, argv + argc, "--appshare-main-line");
	//TestConfig::i().m_bHost = cmdOptionExists(argv, argv + argc, "--host");
	TestConfig::i().m_bCalliope = cmdOptionExists(argv, argv + argc, "--calliope");
	TestConfig::i().m_bLoopback = cmdOptionExists(argv, argv + argc, "--loopback");
	TestConfig::i().m_bHasVideo = !cmdOptionExists(argv, argv + argc, "--no-video");
    TestConfig::i().m_bHasSVS   = cmdOptionExists(argv, argv + argc, "--svs");
	TestConfig::i().m_bHasAudio = !cmdOptionExists(argv, argv + argc, "--no-audio");
    if (cmdOptionExists(argv, argv + argc, "--alpha-aec") ) {
         TestConfig::i().m_aectype = "WmeAecTypeAlpha";
    }
    else
    {
        TestConfig::i().m_aectype = "WmeAecTypeWmeDefault";
        
    }
   
    TestConfig::i().m_bMultiStreamEnable = !cmdOptionExists(argv, argv + argc, "--disable-multistream");
	if (cmdOptionExists(argv, argv + argc, "--no-av")){
		TestConfig::i().m_bHasVideo = false;
		TestConfig::i().m_bHasAudio = false;
	}
	TestConfig::i().m_bMuteVideo = cmdOptionExists(argv, argv + argc, "--mute-video");
	TestConfig::i().m_bMuteAudio = cmdOptionExists(argv, argv + argc, "--mute-audio");
	if (cmdOptionExists(argv, argv + argc, "--mute-av")){
		TestConfig::i().m_bHasVideo = true;
		TestConfig::i().m_bHasAudio = true;
	}
    
    if(cmdOptionExists(argv, argv + argc, "--svc")) {
        TestConfig::i().m_bForceSVC = true;
    }

	TestConfig::i().m_audioDebugOption["enableICE"] = !cmdOptionExists(argv, argv + argc, "--no-ice");
	TestConfig::i().m_videoDebugOption["enableICE"] = !cmdOptionExists(argv, argv + argc, "--no-ice");
	TestConfig::i().m_shareDebugOption["enableICE"] = !cmdOptionExists(argv, argv + argc, "--no-ice-appshare");

	const char *option_argument = getCmdOption(argv, argv + argc, "--audio-port");
	if (option_argument){
		TestConfig::i().m_nAudioPort = atoi(option_argument);
		TestConfig::i().m_audioDebugOption["localPortBegin"] = TestConfig::i().m_nAudioPort;
		TestConfig::i().m_audioDebugOption["localPortEnd"] = TestConfig::i().m_nAudioPort + 4;
	}
    
    option_argument = getCmdOption(argv, argv + argc, "--local-ip");
    if (option_argument){
        TestConfig::i().m_videoDebugOption["localIP"] = std::string("0.0.0.0");
        TestConfig::i().m_audioDebugOption["localIP"] = std::string("0.0.0.0");
        TestConfig::i().m_shareDebugOption["localIP"] = std::string("0.0.0.0");
        TestConfig::i().m_videoDebugOption["localVIP"] = std::string(option_argument);
        TestConfig::i().m_audioDebugOption["localVIP"] = std::string(option_argument);
        TestConfig::i().m_shareDebugOption["localVIP"] = std::string(option_argument);
    }

    //WSE_DATA_DUMP_FLAG_OPTION
    option_argument = getCmdOption(argv, argv + argc, "--dumpvideo-flag");
    if(option_argument){
        TestConfig::i().m_videoParam["uVideoDataDumpFlag"] = atoi(option_argument);
    }
    
	option_argument = getCmdOption(argv, argv + argc, "--video-port");
	if (option_argument){
		TestConfig::i().m_nVideoPort = atoi(option_argument);
		TestConfig::i().m_videoDebugOption["localPortBegin"] = TestConfig::i().m_nVideoPort;
		TestConfig::i().m_videoDebugOption["localPortEnd"] = TestConfig::i().m_nVideoPort + 4;
	}
	option_argument = getCmdOption(argv, argv + argc, "--ice-policy");
	if (option_argument){
		TestConfig::i().m_audioDebugOption["icePolicy"] = atoi(option_argument);
		TestConfig::i().m_videoDebugOption["icePolicy"] = atoi(option_argument);
        TestConfig::i().m_shareDebugOption["icePolicy"] = atoi(option_argument);
	}
	option_argument = getCmdOption(argv, argv + argc, "--ice-policy-appshare");
	if (option_argument){
		TestConfig::i().m_shareDebugOption["icePolicy"] = atoi(option_argument);
	}

	//	if (cmdOptionExists(argv, argv + argc, "--enable-multistream")){
	//		TestConfig::i().m_audioParam["supportCmulti"] = true;
	//        TestConfig::i().m_videoParam["supportCmulti"] = true;
	//        TestConfig::i().m_shareParam["supportCmulti"] = true;
	//	}

	option_argument = getCmdOption(argv, argv + argc, "--appshare-max-capture-fps");
	if (option_argument){
		TestConfig::i().m_nMaxScreenCaptureFps = atoi(option_argument);
	}
    
    option_argument = getCmdOption(argv, argv + argc, "--appshare-max-bandwidth");
    if (option_argument){
        TestConfig::i().m_nAppshareInitBw = atoi(option_argument);
    }
    
    option_argument = getCmdOption(argv, argv + argc, "--appshare-max-capture-downsample-min-height");
    if (option_argument){
        TestConfig::i().m_nSceenCaptureDownSampleMin = atoi(option_argument);
    }
    
	option_argument = getCmdOption(argv, argv + argc, "--wme-trace-level");
	if (option_argument){
		TestConfig::i().m_eTraceLevel = (WmeTraceLevel)atoi(option_argument);
	}
	option_argument = getCmdOption(argv, argv + argc, "--video-size");
	if (option_argument){
		int nVideoSize = atoi(option_argument);
		if (nVideoSize != 2){
			TestConfig::i().m_nVideoSize = nVideoSize;
		}
		setVideoSize();
	}

	option_argument = getCmdOption(argv, argv + argc, "--fps");
	if (option_argument){
		TestConfig::i().m_nMaxFps = atoi(option_argument);
	}
    
    option_argument = getCmdOption(argv, argv + argc, "--run-time");
    if (option_argument){
        TestConfig::i().m_tRunSeconds = atoi(option_argument);
        TestConfig::i().m_tStartSeconds=time(NULL);
    }
    
	//--screenshare-qrcode-scan-config "{'view_x': 0,'view_y' : 0,'view_w' : 500,'view_h' : 400,'qrcode_grid_row' : 2,'qrcode_grid_col' : 2}"
	option_argument = getCmdOption(argv, argv + argc, "--screenshare-qrcode-scan-config");
	if (option_argument){
		TestConfig::i().m_bScreenTrackViewAnalyzer = true;
		TestConfig::i().m_bScreenReplayTAQRcodeFiles = cmdOptionExists(argv, argv + argc, "--screenshare-qrcode-ta-replay");
		TestConfig::i().m_strScreenScanConfigJson = option_argument;
		std::replace(TestConfig::i().m_strScreenScanConfigJson.begin(),
			TestConfig::i().m_strScreenScanConfigJson.end(), '\'', '\"');
	}
	TestConfig::i().m_bDumpReceivedScreenSharingToImage = cmdOptionExists(argv, argv + argc, "--appshare-dump2file");
	//--appshare-source-name monitor or --appshare-source-name app-friend-name
	option_argument = getCmdOption(argv, argv + argc, "--appshare-source-name");
	if (option_argument){
		TestConfig::i().m_strScreenSharingAutoLaunchSourceName = option_argument;
	}

    option_argument = getCmdOption(argv, argv + argc, "--appshare-file-capture-filepath");
    if (option_argument){
        TestConfig::i().m_strScreenSharingCaptureFile = option_argument;
    }
    
    option_argument = getCmdOption(argv, argv + argc, "--appshare-file-capture-skip-frames");
    if (option_argument){
        TestConfig::i().m_nScreenSharingFileCaptureSkip = atoi(option_argument);
    }
    
    option_argument = getCmdOption(argv, argv + argc, "--auth");
    if (option_argument){
        TestConfig::i().m_sAuthHeader = option_argument;
    }
    
    option_argument = getCmdOption(argv, argv + argc, "--venue");
    if (option_argument){
        TestConfig::i().m_sVenuUrl = option_argument;
    }
    
    option_argument = getCmdOption(argv, argv + argc, "--orpheus");
    if (option_argument){
        TestConfig::i().m_sOrpheusUrl = option_argument;
    }
    
    option_argument = getCmdOption(argv, argv + argc, "--logname");
    if (option_argument){
        TestConfig::i().m_sLogFileName = option_argument;
    }
    
    option_argument = getCmdOption(argv, argv + argc, "--uid");
    if (option_argument){
        TestConfig::i().m_sOAuthUID = option_argument;
    }
    option_argument = getCmdOption(argv, argv + argc, "--passwd");
    if (option_argument){
        TestConfig::i().m_sOAuthPasswd = option_argument;
    }
    
	if (cmdOptionExists(argv, argv + argc, "--no-signal")){
		TestConfig::i().m_bNoSignal = true;
	}

	//"10.224.166.31:8082";
	const char *server = getCmdOption(argv, argv + argc, "--server");
	if (!server)
		server = "10.224.166.110:8080/echo?r=123";
	TestConfig::i().m_sWSUrl = server;

	const char *linus = getCmdOption(argv, argv + argc, "--linus");
	if (!linus)
		linus = "http://10.224.166.110:5000/";
    if (linus){
		TestConfig::i().m_sLinusUrl = linus;
        //fix osx instructments tool bug , dont support --linus ""
        if(TestConfig::i().m_sLinusUrl=="null") {
            TestConfig::i().m_sLinusUrl = "";
            TestConfig::i().m_bLocalLinus = false;
        }
    }

	if (cmdOptionExists(argv, argv + argc, "--appshare-render-original")){
		TestConfig::i().m_eRenderModeScreenSharing = WmeRenderModeOriginal;
	}
	else if (cmdOptionExists(argv, argv + argc, "--appshare-render-letterbox")){
		TestConfig::i().m_eRenderModeScreenSharing = WmeRenderModeLetterBox;
	}
	else if (cmdOptionExists(argv, argv + argc, "--appshare-render-fill")){
		TestConfig::i().m_eRenderModeScreenSharing = WmeRenderModeFill;
	}
	else if (cmdOptionExists(argv, argv + argc, "--appshare-render-cropfill")){
		TestConfig::i().m_eRenderModeScreenSharing = WmeRenderModeCropFill;
	}
    
    if(cmdOptionExists(argv, argv + argc, "--duplication")) {
        TestConfig::i().m_bDuplication = true;
    }
    if (cmdOptionExists(argv, argv + argc, "--no-dup")) {
        TestConfig::i().m_bDuplication = false;
    }

    if(cmdOptionExists(argv, argv + argc, "--server-mix")) {
        TestConfig::i().m_uMaxAudioStreams = 1;
    }
    
    option_argument = getCmdOption(argv, argv + argc, "--video-stream");
    if(option_argument)
        TestConfig::i().m_uMaxVideoStreams = atoi(option_argument);
    
    if(cmdOptionExists(argv, argv + argc, "--groupcall")) {
#ifdef AT_WIN
        TestConfig::i().m_uMaxVideoStreams = 4;
#else
        TestConfig::i().m_uMaxVideoStreams = 3;
#endif
    }
    
    option_argument = getCmdOption(argv, argv + argc, "--consume-memory");
    if(option_argument){
        TestConfig::i().m_nProfileConsumerMemorySize = atoi(option_argument) * 1024 * 1024;
    }
    
    if(cmdOptionExists(argv, argv + argc, "--no-simul")) {
        TestConfig::i().m_bEnableAVCSimulcast = false;
    }
    if(cmdOptionExists(argv, argv + argc, "--no-srtp")) {
        TestConfig::i().m_audioDebugOption["enableSRTP"] = false;
        TestConfig::i().m_videoDebugOption["enableSRTP"] = false;
    }
    if(cmdOptionExists(argv, argv + argc, "--enable-dtls-srtp")) {
        TestConfig::i().m_audioDebugOption["enableDtlsSRTP"] = true;
        TestConfig::i().m_videoDebugOption["enableDtlsSRTP"] = true;
        TestConfig::i().m_bEnableDtlsSrtp = true;
    }

    if (cmdOptionExists(argv, argv + argc, "--test-netwarnning")){
        TestConfig::i().m_bTestNetWarnning = true;
    }
    
    if(cmdOptionExists(argv, argv + argc, "--disable-fec")) {
        TestConfig::i().m_videoParam["fecParams"]["bEnableFec"] = false;
        TestConfig::i().m_audioParam["fecParams"]["bEnableFec"] = false;
    }

    option_argument = getCmdOption(argv, argv + argc, "--proxyuser");
    if (option_argument){
        TestConfig::i().m_sProxyUser = option_argument;
    }
    option_argument = getCmdOption(argv, argv + argc, "--proxypasswd");
    if (option_argument){
        TestConfig::i().m_sProxyPasswd = option_argument;
    }
    TestConfig::i().m_bDumpRTP = cmdOptionExists(argv, argv + argc, "--dump-rtp");
    TestConfig::i().m_bDataTerminatedAtRTP = cmdOptionExists(argv, argv + argc, "--data-terminate-rtp");
    option_argument = getCmdOption(argv, argv + argc, "--trace-type");
    if (option_argument){
        cm_set_trace_option(atoi(option_argument));
        TestConfig::i().m_bTraceToWbxTraceFile = true;
    }
    

    if(cmdOptionExists(argv, argv + argc, "--replay")) {
        TestConfig::i().m_bTrain = 1;
        TestConfig::i().m_bPlayback = 1;
        TestConfig::i().m_audioDebugOption["enableICE"] = false;
        TestConfig::i().m_videoDebugOption["enableICE"] = false;
        //TestConfig::i().m_shareDebugOption["enableICE"] = false;

        TestConfig::i().m_audioDebugOption["enableRTCPMux"] = true;
        TestConfig::i().m_videoDebugOption["enableRTCPMux"] = true;
        //TestConfig::i().m_shareDebugOption["enableRTCPMux"] = true;

        TestConfig::i().m_nAudioPort = 20170;
        option_argument = getCmdOption(argv, argv + argc, "--audio-port");
        if (option_argument){
            TestConfig::i().m_nAudioPort = atoi(option_argument);
        }

        TestConfig::i().m_nVideoPort = 20172;
        option_argument = getCmdOption(argv, argv + argc, "--video-port");
        if (option_argument){
            TestConfig::i().m_nVideoPort = atoi(option_argument);
        }
        
        //share

        option_argument = getCmdOption(argv, argv + argc, "--audio-srtp-key");
        std::string audioSrtpKey = "";
        if (option_argument) {
            audioSrtpKey = option_argument;
        }

        if (audioSrtpKey.empty()) {
            TestConfig::i().m_audioDebugOption["enableSRTP"] = false;
        } else {
            TestConfig::i().m_audioDebugOption["enableSRTP"] = true;
            TestConfig::i().m_audioParam["srtpKey"] = audioSrtpKey;
        }

        option_argument = getCmdOption(argv, argv + argc, "--video-srtp-key");
        std::string videoSrtpKey = "";
        if (option_argument) {
            videoSrtpKey = option_argument;
        }

        if (videoSrtpKey.empty()) {
            TestConfig::i().m_videoDebugOption["enableSRTP"] = false;
        } else {
            TestConfig::i().m_videoDebugOption["enableSRTP"] = true;
            TestConfig::i().m_videoParam["srtpKey"] = videoSrtpKey;
        }

        TestConfig::i().m_audioParam["supportCmulti"] = true;
        TestConfig::i().m_videoParam["supportCmulti"] = true;

        TestConfig::i().m_bMuteAudio = true;
        TestConfig::i().m_bMuteVideo = true;

        std::string sdp = "\
v=0\n\
o=wme-mac-4.12.0 0 1 IN IP4 127.0.0.1\n\
s=-\n\
c=IN IP4 127.0.0.1\n\
b=TIAS:4064000\n\
t=0 0\n\
a=cisco-mari:v1\n\
a=cisco-mari-rate\n";

        std::string audioSdp = "\
m=audio 20336 RTP/SAVPF 101 0 8 9 102 112\n\
c=IN IP4 192.168.1.1\n\
b=TIAS:64000\n\
a=content:main\n\
a=sendonly\n\
a=rtpmap:101 opus/48000/2\n\
a=fmtp:101 maxplaybackrate=48000;maxaveragebitrate=64000;stereo=1\n\
a=rtpmap:0 PCMU/8000\n\
a=rtpmap:8 PCMA/8000\n\
a=rtpmap:9 G722/8000\n\
a=rtpmap:102 iLBC/8000\n\
a=rtpmap:112 x-ulpfecuc/8000\n\
a=fmtp:112 max_esel=1400;max_n=255;m=8;multi_ssrc=1;FEC_ORDER=FEC_SRTP;non_seq=1\n\
a=extmap:1/sendrecv http://protocols.cisco.com/virtualid\n\
a=extmap:2/sendrecv urn:ietf:params:rtp-hdrext:ssrc-audio-level\n\
a=extmap:3/sendrecv urn:ietf:params:rtp-hdrext:toffset\n\
a=extmap:4/sendrecv http://protocols.cisco.com/timestamp#100us\n\
a=rtcp-mux\n\
a=sprop-source:0 csi=1978662656;count=20\n\
a=sprop-simul:0 100 *\n\
a=rtcp-fb:* ccm cisco-scr\n";

        if ( !audioSrtpKey.empty() ) {
            // we just need a key to be able to negotiate successfully
            audioSdp += "a=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:" + audioSrtpKey + "\n";
        }

        std::string videoSdp = "\
m=video 20026 RTP/SAVPF 117 97 111\n\
c=IN IP4 192.168.1.1\n\
b=TIAS:4000000\n\
a=content:main\n\
a=sendonly\n\
a=rtpmap:117 H264/90000\n\
a=fmtp:117 profile-level-id=420016;packetization-mode=1;max-mbps=115200;max-fs=3840;max-fps=3000;max-br=2000;max-dpb=11520;level-asymmetry-allowed=1\n\
a=rtpmap:97 H264/90000\n\
a=fmtp:97 profile-level-id=420016;packetization-mode=0;max-mbps=115200;max-fs=3840;max-fps=3000;max-br=2000;max-dpb=11520;level-asymmetry-allowed=1\n\
a=rtpmap:111 x-ulpfecuc/8000\n\
a=fmtp:111 max_esel=1400;max_n=255;m=8;multi_ssrc=1;FEC_ORDER=FEC_SRTP;non_seq=1\n\
a=rtcp-fb:* nack pli\n\
a=rtcp-fb:* ccm tmmbr\n\
a=extmap:1/sendrecv http://protocols.cisco.com/virtualid\n\
a=extmap:2/sendrecv http://protocols.cisco.com/framemarking\n\
a=extmap:3/sendrecv urn:ietf:params:rtp-hdrext:toffset\n\
a=extmap:4/sendrecv http://protocols.cisco.com/timestamp#100us\n\
a=extmap:5/recvonly urn:3gpp:video-orientation\n\
a=rtcp-mux\n\
a=sprop-source:0 csi=1978662657;count=20;simul=100,101,102,103|104,105,106,107;lrotation=1\n\
a=sprop-simul:0 100 117 profile-level-id=42e00b;max-mbps=3000;max-fs=396;max-fps=3000;\n\
a=sprop-simul:0 101 117 profile-level-id=42e00c;max-mbps=7200;max-fs=396;max-fps=3000;\n\
a=sprop-simul:0 102 117 profile-level-id=42e014;max-mbps=27600;max-fs=920;max-fps=3000;\n\
a=sprop-simul:0 103 117 profile-level-id=42e015;max-mbps=108000;max-fs=3600;max-fps=3000;\n\
a=sprop-simul:0 104 97 profile-level-id=42e00b;max-mbps=3000;max-fs=396;max-fps=3000;\n\
a=sprop-simul:0 105 97 profile-level-id=42e00c;max-mbps=7200;max-fs=396;max-fps=3000;\n\
a=sprop-simul:0 106 97 profile-level-id=42e014;max-mbps=27600;max-fs=920;max-fps=3000;\n\
a=sprop-simul:0 107 97 profile-level-id=42e015;max-mbps=108000;max-fs=3600;max-fps=3000;\n\
a=rtcp-fb:* ccm cisco-scr\n";

        if ( !videoSrtpKey.empty() ) {
            // we just need a key to be able to negotiate successfully
            videoSdp += "a=crypto:1 AES_CM_128_HMAC_SHA1_80 inline:" + videoSrtpKey + "\n";
        }

        TestConfig::Instance().m_sSdp = sdp + audioSdp + videoSdp;

        TestConfig::i().m_videoDebugOption["enableDataDump"] = WME_DATA_DUMP_VIDEO_NAL_TO_DECODER;

        TestConfig::i().m_audioParam["enableAllDumpFile"] = true;
    }
}

void InitCmdArguments(int argc,char**argv)
{
    bool useSpefiedCmd=false;//please set to false when commit code
    if(!useSpefiedCmd)
        InitCmdArguments2(argc, argv);
    else
    {
        /*
         WME_PERFORMANCE_TEST_VIDEO_SIZE=
         WME_PERFORMANCE_TEST_ios_clock_no_prompt=
         WME_PERFORMANCE_TEST_Capture_Limit_Fps=20
         WME_PERFORMANCE_TEST_WseVideoReceivingBuffer_AddPacket_i=
         */
		//pass env by command line , need not set it by code exept for android. and wp8 don't support environment variables 
        //setenv("WME_PERFORMANCE_TEST_VIDEO_SIZE","360",true);
        //setenv("WME_PERFORMANCE_TEST_ios_clock_no_prompt","1",true);
        
        /*
         --disable-qos
         --no-dumpvideo
         --no-audio
         --no-srtp
         --linus http://10.224.166.110:5000/
         --linus ""
         --server 10.224.166.110:8080/echo?r=11123
         --calliope
         --video-size 3
         --auto-start
         --no-trace-2-file
         --video-hardware
         --loopback
         */
        char* newArgv[]={
            (char*)"--disable-qos",
            (char*)"--no-dumpvideo",
            (char*)"--linus", (char*)"http://10.224.166.110:5000/",
            (char*)"--server", (char*)"10.224.166.110:8080/echo?r=11123",
            (char*)"--calliope",
            (char*)"--auto-start",
            //(char*)"--video-hardware",
            
            NULL
        };
        int newArgc=sizeof(newArgv)/sizeof(newArgv[0])-1;
        InitCmdArguments2(newArgc,newArgv);
    }
}
void doStart(int argc, char **argv, void *winRemote, void *winLocal, void *winScreenViewer)
{
	CM_INFO_TRACE("doStart");
	InitApp(argc, argv);
	doStartCall(winRemote, winLocal, winScreenViewer);
}

void InitApp(int argc, char **argv)
{
	InitMediaEngine(true);
    InitCmdArguments(argc, argv);
}

void doStartCall(void *winRemote, void *winLocal, void *winScreenViewer, IAppSink *pApp)
{
    TestConfig::i().m_pAppSink = pApp;
	if (TestConfig::i().m_bLoopback) {
		LoopbackCall::Instance()->startLoopback(winRemote, winLocal, winScreenViewer);
	}
	else {
        PeerCall::Instance()->pushRemoteWindow(winRemote);
        if (pApp) {
            pApp->onVenueUrl(TestConfig::i().m_sVenuUrl);
            pApp->onCheckMultiVideo();
        }
        PeerCall::Instance()->startPeer(winLocal, winScreenViewer, pApp);
		PeerCall::Instance()->connect(TestConfig::i().m_sWSUrl.c_str());
	}
}
void doStop() {
	if (PeerCall::Instance()->isActive()) {
		PeerCall::Instance()->stopPeer();
	}
	else {
		LoopbackCall::Instance()->stopLoopback();
	}
	//UninitMediaEngine();
}
