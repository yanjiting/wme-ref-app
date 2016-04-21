#ifndef MediaSessionTest_TEST_CONFIG_h
#define MediaSessionTest_TEST_CONFIG_h

#pragma once

class IAppSink;
class TestConfig{
public:
	static TestConfig& Instance() { return testConfig; }
	static TestConfig& i() { return testConfig; }

	bool m_bScreenTrackViewAnalyzer;
	bool m_bScreenReplayTAQRcodeFiles;
	bool m_bDumpReceivedScreenSharingToImage;
	std::string m_strScreenScanConfigJson;
	unsigned int m_nMaxScreenCaptureFps;
    unsigned int m_nSceenCaptureDownSampleMin;
    unsigned int m_uMaxVideoStreams;
    unsigned int m_uMaxAudioStreams;
    bool m_bEnableAVCSimulcast;
    bool m_bShareFilterSelf;
    bool m_bEnableDtlsSrtp;

	int m_nTAPort;
	bool m_bTraceToWbxTraceFile;
    std::string m_sLogFileName; //Mac only
	wme::WmeTraceLevel m_eTraceLevel;
	bool m_bASPreview;
	bool m_bAppshare;
	void * m_handleSharedWindow;
    bool m_bTrace2File;
    uint32_t m_nAppshareInitBw;
    bool m_bAutoRequestFloor;
	bool m_bAutoStart;
	bool m_bSharer;
	//bool m_bHost;
	bool m_bCalliope;
	bool m_bHasVideo;
    bool m_bHasSVS;
    bool m_bForceSVC;
	bool m_bHasAudio;
	bool m_bMuteVideo;
	bool m_bMuteAudio;
	bool m_bLoopback;
    bool m_bVideoHW;
    bool m_bVideoScreenHW;
    bool m_bUsingVPIO;
    bool m_bUsingTCAEC;
	bool m_bQoSEnable;
    bool m_bSetFeatureToggles;
    bool m_bDPC;
	bool m_bMultiStreamEnable;
    bool m_bDuplication;
    bool m_bTestNetWarnning;
    bool m_bTestRequestVideoOverrideDefault;
    bool m_bAutoSwitchAudioMicDevice;
    bool m_bAutoSwitchAudioSpeakerDevice;
    unsigned int  m_mauallyAdaptationBps;
    bool m_bEnableCVO;
    bool m_bDumpRTP = false;
    bool m_bDataTerminatedAtRTP = false;
    IAppSink *m_pAppSink;

	int  m_nVideoSize;
	unsigned short m_nAudioPort;
	unsigned short m_nVideoPort;
	unsigned short m_nMaxFps;
    time_t      m_tRunSeconds;
    time_t      m_tStartSeconds;
	std::string m_sLinusUrl;
    bool m_bLocalLinus;
	std::string m_sWSUrl;
    std::string m_strFeatureToggles;
	std::string m_strScreenSharingAutoLaunchSourceName;
    std::string m_strScreenSharingCaptureFile;
    int m_nScreenSharingFileCaptureSkip;
    std::string m_strDeviceModelName;
    std::string m_strSystemOSVersion;

	Json::Value m_audioParam;
	Json::Value m_videoParam;
	Json::Value m_shareParam;
    Json::Value m_globalParam;

	wme::WmeTrackRenderScalingModeType m_eRenderModeScreenSharing;

	Json::Value m_audioDebugOption;
	Json::Value m_videoDebugOption;
	Json::Value m_shareDebugOption;

    bool m_bNoSignal;
    std::string m_sVenuUrl;
    std::string m_sAuthHeader;
    std::string m_sOrpheusUrl;
    std::string m_sOverrideDeviceSetting;

    bool m_bTrain;
    bool m_bPlayback;
    std::string m_sSdp;
    bool m_bFakeSVCByAVC; // for demo
    bool m_bAudioProcess; // for wp8
    std::string m_bgSdpOffer; //for wp8
    std::string m_bgSdpAnwser; //for wp8
    unsigned int m_csi; // for wp8
    
    std::string m_sOAuthUID;
    std::string m_sOAuthPasswd;
    std::string m_sProxyUser;
    std::string m_sProxyPasswd;
    
    std::string m_aectype;
    
    
    int m_nProfileConsumerMemorySize;
    
    bool m_bDisable90PVideo;

	void setFilePath(const char *srcPath, const char *dstPath, WmeSessionType mediaType)
	{
		Json::Value* root = &m_audioDebugOption;
		if (mediaType == WmeSessionType_Audio)
			root = &m_audioDebugOption;
		else if (mediaType == WmeSessionType_Video)
			root = &m_videoDebugOption;
		else{
			CM_ASSERTE(FALSE);
			return;
		}

		if (srcPath && *srcPath != 0)
		{
			Json::Value &capture = (*root)["fileCapture"] = Json::Value(Json::objectValue);
			capture["path"] = Json::Value(srcPath);
			capture["isLoop"] = Json::Value(false);
		}
		if (dstPath && *dstPath != 0)
		{
			Json::Value &render = (*root)["fileRender"] = Json::Value(Json::objectValue);
			render["path"] = Json::Value(dstPath);
		}

	}

	std::string ToString(){
		std::ostringstream ret;
		ret << " bAppShare = " << m_bAppshare <<
            ", bDPC = " << m_bDPC <<
			", bCalliope=" << m_bCalliope <<
			", hasVideo=" << m_bHasVideo <<
			", hasAudio=" << m_bHasAudio <<
			", qosEnable=" << m_bQoSEnable <<
			", muteVideo=" << m_bMuteVideo <<
			", muteAudio=" << m_bMuteAudio <<
			", bLoopback=" << m_bLoopback <<
			", linus=" << m_sLinusUrl <<
			", WS=" << m_sWSUrl <<
            ", isFeatureTogglesSet=" << m_bSetFeatureToggles <<
            ", media-feature-toggles=" << m_strFeatureToggles <<
			", m_strScreenSharingAutoLaunchSourceName=" << m_strScreenSharingAutoLaunchSourceName <<
			", bSharer=" << m_bSharer <<
			", m_nVideoSize=" << m_nVideoSize <<
			//			", bFakeVideoByShare=" << m_bFakeVideoByShare <<
			", m_nAudioPortBegin=" << m_nAudioPort <<
			", m_nVideoPortBegin=" << m_nVideoPort <<
			", max-fps=" << m_nMaxFps <<
			", bASPreview" << m_bASPreview <<
            ", bAutoSwitchAudioMicDevice" << m_bAutoSwitchAudioMicDevice <<
            ", bAutoSwitchAudioSpeakerDevice" << m_bAutoSwitchAudioSpeakerDevice <<
            ", bEnableCVO = " << m_bEnableCVO <<
            ", bVideoHW = " << m_bVideoHW;

		return ret.str();
	}

	void Dump(){
		CM_INFO_TRACE("[ScreenShare][Audio][Video] ClickCall::TestConfig," << ToString());
	}

protected:
	TestConfig()
		: m_audioParam(Json::objectValue)
		, m_videoParam(Json::objectValue)
		, m_shareParam(Json::objectValue)
        , m_globalParam(Json::objectValue)
		, m_audioDebugOption(Json::objectValue)
		, m_videoDebugOption(Json::objectValue)
		, m_shareDebugOption(Json::objectValue)
        , m_uMaxVideoStreams(1)
        , m_uMaxAudioStreams(3)
        , m_csi(0)
	{
		m_bAppshare = false;
		m_handleSharedWindow = NULL;
        m_nAppshareInitBw = 0;
		m_bASPreview = false;
        m_bAutoRequestFloor = true;
		m_bSharer = false;
        m_bDPC = true;
		//m_bHost = false;
		m_bCalliope = false;
		m_bHasVideo = true;
		m_bHasAudio = true;
		m_bQoSEnable = true;
        m_bSetFeatureToggles = false;
		m_bMuteVideo = false;
		m_bMuteAudio = false;
		m_bLoopback = true;
        m_bVideoHW  = false;
        m_bVideoScreenHW = false;
        m_bUsingVPIO = true;
        m_bUsingTCAEC = false;
		m_bMultiStreamEnable = true;
		m_nVideoSize = 2;
        m_bAutoSwitchAudioMicDevice = false;
        m_bAutoSwitchAudioSpeakerDevice = false;
        m_bEnableCVO = false;
        m_aectype = "WmeAecTypeWmeDefault";
        
		m_eRenderModeScreenSharing = WmeRenderModeLetterBox;

        m_mauallyAdaptationBps = 0;
        
        m_bEnableAVCSimulcast = true;
        m_bDuplication = false;
        m_bTestNetWarnning = false;
        m_bTestRequestVideoOverrideDefault = false;
        m_pAppSink = NULL;
        m_bLocalLinus = true;
        m_bForceSVC = false;
        
        m_bDisable90PVideo = true;
        
		m_audioDebugOption["enableICE"] = true;
		m_audioDebugOption["enableSRTP"] = true;
        m_audioDebugOption["enableDtlsSRTP"] = false;
		m_audioDebugOption["enableRTCPMux"] = true;
		m_videoDebugOption["enableICE"] = true;
		m_videoDebugOption["enableSRTP"] = true;
        m_videoDebugOption["enableDtlsSRTP"] = false;
		m_videoDebugOption["enableRTCPMux"] = true;
		m_videoDebugOption["enableBGRA"] = false;
        //m_videoDebugOption["enableDataDump"] = WME_DATA_DUMP_VIDEO_NAL_TO_DECODER;

        TestConfig::i().m_videoParam["fecParams"]["uClockRate"] = 8000;
        TestConfig::i().m_videoParam["fecParams"]["uPayloadType"] = 111;
        TestConfig::i().m_videoParam["fecParams"]["bEnableFec"] = true;
        
        TestConfig::i().m_audioParam["fecParams"]["uClockRate"] = 8000;
        TestConfig::i().m_audioParam["fecParams"]["uPayloadType"] = 112;
        TestConfig::i().m_audioParam["fecParams"]["bEnableFec"] = true;

		m_shareDebugOption["enableICE"] = true;
		m_eTraceLevel = WME_TRACE_LEVEL_INFO;
		/*
		typedef enum{
		UDPOnly = 1,
		TCPOnly = 2,
		WSOnly = 4,
		UDPFallToTCP = UDPOnly | TCPOnly,	//fallback to
		UDPFallToWS = UDPOnly | WSOnly,
		TCPFallToWS = TCPOnly | WSOnly
		}ConnectionPolicy;
		*/
		m_shareDebugOption["icePolicy"] = 2;//CIceConnector::TCPOnly;

		m_bTraceToWbxTraceFile = false;
		m_nAudioPort = 0;
		m_nVideoPort = 0;
		m_nTAPort = 0;
		m_bScreenTrackViewAnalyzer = false;
		m_bDumpReceivedScreenSharingToImage = false;
		m_strScreenScanConfigJson = "";
		m_strScreenSharingAutoLaunchSourceName = "";
		m_bAutoStart = false;
		m_bScreenReplayTAQRcodeFiles = false;

        m_bShareFilterSelf = true;
        m_bEnableDtlsSrtp = false;
		// Use AVC in sdp first as client do.
		Json::Value selected(Json::arrayValue);
		selected[0] = "H264";
		m_videoParam["selectedCodec"] = selected;
        
        m_bFakeSVCByAVC = false;
        if(m_bFakeSVCByAVC)
        {
            Json::Value selected(Json::arrayValue);
            selected[0] = "H264-SVC";
            m_videoParam["selectedCodec"] = selected;
        }
        
		m_nMaxScreenCaptureFps = 0;
        m_nSceenCaptureDownSampleMin = 0;
        m_nScreenSharingFileCaptureSkip = 0;

		m_nMaxFps = 3000;
        m_tRunSeconds=0;
        m_tStartSeconds=0;
        m_bNoSignal = false;

		/*
		Json::Value avc(Json::objectValue);
		avc["codec"] = "H264";
		avc["uProfileLevelID"] = "42000C";
		avc["max-mbps"] = 7200;
		avc["max-fs"] = 396;
		avc["max-fps"] = 2400;
		Json::Value encode(Json::arrayValue);
		encode[0] = avc;
		m_videoParam["encodeCodec"] = encode;
		*/
        
        m_bTrain = false;
        m_bPlayback = false;
        m_bTrace2File = true;
        m_bAudioProcess = false;
        
        m_nProfileConsumerMemorySize = 0;
	}
	~TestConfig(){};
	static TestConfig testConfig;
};


#endif //!define MediaSessionTest_TEST_CONFIG_h

