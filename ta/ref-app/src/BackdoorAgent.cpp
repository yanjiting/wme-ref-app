#include "BackdoorAgent.h"
#include "json/json.h"
#include "Loopback.h"
#include "PeerCall.h"
#include "ClickCall.h"
#include "asenum.h"
#include "ScreenTrackViewAnalyzer.h"
#include "atdef.h"
#include "ClickCall.h"

extern "C" void __gcov_flush();

using namespace wme;

#define REGISTER_BACKDOOR(x) m_mapBackdoors.insert(std::make_pair(#x, &CBackdoorAgent::##x)); 

CBackdoorAgent::CBackdoorAgent(const char*szRefPath, void *winRemote, void *winLocal, void *winScreenViewer, IAppSink* sink)
{
	m_sRefPath = szRefPath;

	m_winRemote = winRemote;
	m_winLocal = winLocal;
	m_winScreenViewer = winScreenViewer;
    m_sink = sink;

    //XCode raise fail for ::##
    /*
	REGISTER_BACKDOOR(backdoorStartCall);
	REGISTER_BACKDOOR(backdoorIsFileCaptureEnded);

	REGISTER_BACKDOOR(backdoorGetLocalSdp);
	REGISTER_BACKDOOR(backdoorSetRemoteSdp);
	REGISTER_BACKDOOR(backdoorGetStatistics);
	REGISTER_BACKDOOR(backdoorStopCall);
	REGISTER_BACKDOOR(backdoorMuteUnMute);
	REGISTER_BACKDOOR(backdoorSetParam);
	REGISTER_BACKDOOR(backdoorGetParam);

	//Screen Sharing
	REGISTER_BACKDOOR(backdoorStartSharing); 
	REGISTER_BACKDOOR(backdoorStopSharing);
	REGISTER_BACKDOOR(backdoorGetSharingStatus);
	REGISTER_BACKDOOR(backdoorGetSharingResult);
    */
    m_mapBackdoors.insert(std::make_pair("backdoorStartCall",&CBackdoorAgent::backdoorStartCall));
    m_mapBackdoors.insert(std::make_pair("backdoorIsFileCaptureEnded",&CBackdoorAgent::backdoorIsFileCaptureEnded));
    
    m_mapBackdoors.insert(std::make_pair("backdoorGetLocalSdp",&CBackdoorAgent::backdoorGetLocalSdp));
    m_mapBackdoors.insert(std::make_pair("backdoorSetRemoteSdp",&CBackdoorAgent::backdoorSetRemoteSdp));
    m_mapBackdoors.insert(std::make_pair("backdoorGetStatistics",&CBackdoorAgent::backdoorGetStatistics));
    m_mapBackdoors.insert(std::make_pair("backdoorGetFeatureToggles", &CBackdoorAgent::backdoorGetFeatureToggles));
    m_mapBackdoors.insert(std::make_pair("backdoorGetNetworkMetrics",&CBackdoorAgent::backdoorGetNetworkMetrics));
    m_mapBackdoors.insert(std::make_pair("backdoorStopCall",&CBackdoorAgent::backdoorStopCall));
    m_mapBackdoors.insert(std::make_pair("backdoorMuteUnMute",&CBackdoorAgent::backdoorMuteUnMute));
	m_mapBackdoors.insert(std::make_pair("backdoorStartStopTrack", &CBackdoorAgent::backdoorStartStopTrack));
	m_mapBackdoors.insert(std::make_pair("backdoorUpdateSdp", &CBackdoorAgent::backdoorUpdateSdp));
    m_mapBackdoors.insert(std::make_pair("backdoorSetRemoteOffer",&CBackdoorAgent::backdoorSetRemoteOffer));
    m_mapBackdoors.insert(std::make_pair("backdoorSetParam",&CBackdoorAgent::backdoorSetParam));
    m_mapBackdoors.insert(std::make_pair("backdoorGetParam",&CBackdoorAgent::backdoorGetParam));
    m_mapBackdoors.insert(std::make_pair("backdoorGetVideoParameters", &CBackdoorAgent::backdoorGetVideoParameters));
    m_mapBackdoors.insert(std::make_pair("backdoorGetAudioParameters", &CBackdoorAgent::backdoorGetAudioParameters));
    m_mapBackdoors.insert(std::make_pair("backdoorSetAudioParam", &CBackdoorAgent::
        backdoorSetAudioParam));
    m_mapBackdoors.insert(std::make_pair("backdoorSetVideoParam", &CBackdoorAgent::
        backdoorSetVideoParam));
    m_mapBackdoors.insert(std::make_pair("backdoorSetShareParam", &CBackdoorAgent::
                                         backdoorSetShareParam));
    m_mapBackdoors.insert(std::make_pair("backdoorGetCSIChangeHistory", &CBackdoorAgent::
        backdoorGetCSIChangeHistory));
    m_mapBackdoors.insert(std::make_pair("backdoorGetCSICount", &CBackdoorAgent::
                                         backdoorGetCSICount));
    m_mapBackdoors.insert(std::make_pair("backdoorGetMediaStatus", &CBackdoorAgent::
                                         backdoorGetMediaStatus));
    m_mapBackdoors.insert(std::make_pair("backdoorGetVideoCSI", &CBackdoorAgent::
                                         backdoorGetVideoCSI));
    
    m_mapBackdoors.insert(std::make_pair("backdoorCheckHWSupport", &CBackdoorAgent::backdoorCheckHWSupport));
    m_mapBackdoors.insert(std::make_pair("backdoorSetManualBandwidth", &CBackdoorAgent::backdoorSetManualBandwidth));
    m_mapBackdoors.insert(std::make_pair("backdoorGetMemory", &CBackdoorAgent::backdoorGetMemory));
    //Screen Sharing
//	m_mapBackdoors.insert(std::make_pair("backdoorSetSharingContext", &CBackdoorAgent::backdoorSetSharingContext));
//	m_mapBackdoors.insert(std::make_pair("backdoorAddOneResource", &CBackdoorAgent::backdoorAddOneResource));
//    m_mapBackdoors.insert(std::make_pair("backdoorStartSharing",&CBackdoorAgent::backdoorStartSharing));
    m_mapBackdoors.insert(std::make_pair("backdoorStopSharing",&CBackdoorAgent::backdoorStopSharing));
    m_mapBackdoors.insert(std::make_pair("backdoorGetSharingStatus",&CBackdoorAgent::backdoorGetSharingStatus));
	m_mapBackdoors.insert(std::make_pair("backdoorGetSharingResult", &CBackdoorAgent::backdoorGetSharingResult));
	m_mapBackdoors.insert(std::make_pair("backdoorSetQRCodeContext", &CBackdoorAgent::backdoorSetQRCodeContext));
	m_mapBackdoors.insert(std::make_pair("backdoorCollectReceivedQRCodeContents", &CBackdoorAgent::backdoorCollectReceivedQRCodeContents));
    m_mapBackdoors.insert(std::make_pair("backdoorSubscribe", &CBackdoorAgent::backdoorSubscribe));
    m_mapBackdoors.insert(std::make_pair("backdoorRequestVideo", &CBackdoorAgent::backdoorRequestVideo));
    m_mapBackdoors.insert(std::make_pair("backdoorRequestFloor", &CBackdoorAgent::backdoorRequestFloor));
    m_mapBackdoors.insert(std::make_pair("backdoorOutputTrace", &CBackdoorAgent::backdoorOutputTrace));
    m_mapBackdoors.insert(std::make_pair("backdoorScreenChangeCaptureFps", &CBackdoorAgent::backdoorScreenChangeCaptureFps));
    m_mapBackdoors.insert(std::make_pair("backdoorSetScreenFileCaptureFilePath", &CBackdoorAgent::backdoorSetScreenFileCaptureFilePath));
    m_mapBackdoors.insert(std::make_pair("backdoorGetVoiceLevel", &CBackdoorAgent::backdoorGetVoiceLevel));
    m_mapBackdoors.insert(std::make_pair("backdoorOverridePerfJson", &CBackdoorAgent::backdoorOverridePerfJson));
    
    //lip sync
    m_mapBackdoors.insert(std::make_pair("backdoorCheckSyncStatus",&CBackdoorAgent::backdoorCheckSyncStatus));
    
    //mari fec
    m_mapBackdoors.insert(std::make_pair("backdoorSetFecParam", &CBackdoorAgent::backdoorSetFecParam));
    
    m_mapBackdoors.insert(std::make_pair("backdoorSetFeatureToggles", &CBackdoorAgent::backdoorSetFeatureToggles));
    
    m_mapBackdoors.insert(std::make_pair("backdoorGetCPUCores", &CBackdoorAgent::backdoorGetCPUCores));
	m_mapBackdoors.insert(std::make_pair("backdoorQuitWP8", &CBackdoorAgent::backdoorQuitWP8));
    
    //stun trace
    m_mapBackdoors.insert(std::make_pair("backdoorGetStunTraceResult", &CBackdoorAgent::backdoorGetStunTraceResult));
    
    //trace server
    m_mapBackdoors.insert(std::make_pair("backdoorGetTraceServerResult", &CBackdoorAgent::backdoorGetTraceServerResult));
    
    //misc
    m_mapBackdoors.insert(std::make_pair("backdoorConfig", &CBackdoorAgent::backdoorConfig));
    
    m_mapBackdoors.insert(std::make_pair("backdoorGetAssertionsCount", &CBackdoorAgent::backdoorGetAssertionsCount));
    //svs, this case is for pc only right now
    m_mapBackdoors.insert(std::make_pair("backdoorChangeLocalTrack", &CBackdoorAgent::backdoorChangeLocalTrack));
}

CBackdoorAgent::~CBackdoorAgent()
{
}

std::string CBackdoorAgent::ExpandPath(std::string relPath)
{
	return m_sRefPath + "/" + relPath;
}

bool CBackdoorAgent::OnRequest(std::string selector, std::string arg, std::string &result)
{
	CM_INFO_TRACE("CBackdoorAgent::OnRequest, selector=" << selector << ", arg=" << arg);
	auto method = m_mapBackdoors.find(selector);
	if (method == m_mapBackdoors.end()){
		CM_INFO_TRACE("CBackdoorAgent::OnRequest, method not found.");
		result += "Backdoor: ";
		result += selector + "not found.";
		return false; 
	}
	result = (this->*(method->second))(arg);
	if (result.compare("Failed") == 0)
		return false;
	return true; 
}

void CBackdoorAgent::UpdateFileCaptureRender(Json::Value &root)
{
    std::string linusAddr = root["linus"].asString();
    bool bFileMode = root["filemode"].asBool();
    bool bTimestampMode = root["timestamp"].asBool();
    std::string defaultAudioRenderPath = ExpandPath("Temp/audio_1_8000_16.pcm");
    if (bFileMode) {
        TestConfig::i().setFilePath(ExpandPath("sample/audio_1_8000_16.pcm").c_str(), defaultAudioRenderPath.c_str(), WmeSessionType_Audio);

        int resolution = 2;
        if (root.isMember("resolution")) {
            resolution = root["resolution"].asInt();
        }
        if (0 == resolution) {
            TestConfig::i().setFilePath(ExpandPath("sample/video_160_90_6_i420.yuv").c_str(), ExpandPath("Temp/video_160_90_6_i420.yuv").c_str(), WmeSessionType_Video);
        }
        else if (1 == resolution) {
            TestConfig::i().setFilePath(ExpandPath("sample/video_320_180_12_i420.yuv").c_str(), ExpandPath("Temp/video_320_180_12_i420.yuv").c_str(), WmeSessionType_Video);
        }
        else if (2 == resolution){
            TestConfig::i().setFilePath(ExpandPath("sample/video_640_360_24_i420.yuv").c_str(), ExpandPath("Temp/video_640_360_24_i420.yuv").c_str(), WmeSessionType_Video);
        } else {
            TestConfig::i().setFilePath(ExpandPath("sample/video_1280_720_30_i420.yuv").c_str(), ExpandPath("Temp/video_1280_720_30_i420.yuv").c_str(), WmeSessionType_Video);
        }

        if (root.isMember("audiosouce_file") && root["audiosouce_file"].asString().size()>0){
            TestConfig::i().setFilePath(ExpandPath("sample/" + root["audiosouce_file"].asString()).c_str(), "", WmeSessionType_Audio);
        }
        if (root.isMember("videosouce_file") && root["videosouce_file"].asString().size()>0){
            TestConfig::i().setFilePath(ExpandPath("sample/" + root["videosouce_file"].asString()).c_str(), "", WmeSessionType_Video);
        }

        if (root.isMember("isLoopFile")) {
            Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
            capture["isLoop"] = root["isLoopFile"].asBool();
        }

        if (bTimestampMode) {
            Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
            capture["timestamp"] = Json::Value(bTimestampMode ? true : false);
        }
    }
    if (root.isMember("files")) {
        Json::Value &files = root["files"];
        if (files.isMember("audio") && files["audio"].isMember("source")) {
            std::string audio_src = files["audio"]["source"].asString();
            if(files["audio"].isMember("render"))
                defaultAudioRenderPath = ExpandPath("Temp/" + files["audio"]["render"].asString());
            if (!audio_src.empty()) {
                TestConfig::i().setFilePath(ExpandPath("sample/" + audio_src).c_str(), defaultAudioRenderPath.c_str(), WmeSessionType_Audio);
                bFileMode = true;
            }
            Json::Value &capture = TestConfig::i().m_audioDebugOption["fileCapture"];
            if (files["audio"].isMember("loop"))
                capture["isLoop"] = files["audio"]["loop"].asBool();
            if (files["audio"].isMember("timestamp"))
                capture["timestamp"] = files["audio"]["timestamp"].asBool();
            Json::Value &filesAudio = files["audio"];
            if( filesAudio.isMember("feed_mixed") )
            {
                
            }
            if( filesAudio.isMember("feed_mic") )
            {
                
            }
        }
        if (files.isMember("video") && files["video"].isMember("source")) {
            std::string video_src = files["video"]["source"].asString();
            if (!video_src.empty()) {
                TestConfig::i().setFilePath(ExpandPath("sample/" + video_src).c_str(), ExpandPath("Temp/" + video_src).c_str(), WmeSessionType_Video);
                bFileMode = true;
            }
            Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
            if (files["video"].isMember("loop"))
                capture["isLoop"] = files["video"]["loop"].asBool();
            if (files["video"].isMember("timestamp"))
                capture["timestamp"] = files["video"]["timestamp"].asBool();
        }
    }
    if (!linusAddr.empty()) {
        TestConfig::i().m_sLinusUrl = linusAddr;
        TestConfig::i().m_bCalliope = true;
    }

    if (bFileMode) {
        TestConfig::i().m_sLinusUrl = linusAddr;
        TestConfig::i().m_bCalliope = true;
    }
}

std::string CBackdoorAgent::backdoorStartCall(std::string arg)
{
	CM_INFO_TRACE("CBackdoorAgent::backdoorStartCall start arg=" << arg.c_str());
    puts("backdoorStartCall");
    puts(arg.c_str());
	Json::Reader r;
	Json::Value root;
	r.parse(arg, root, true);

	bool bLoopback = root["loopback"].asBool();
	TestConfig::i().m_bLoopback = bLoopback;
	std::string sdp = root["sdp"].asString();
	if (root.isMember("overrideip")){
		std::string sOverrideIp = root["overrideip"].asString();
		if (!sOverrideIp.empty()){
			TestConfig::i().m_audioDebugOption["overrideMediaIPAddress"] = sOverrideIp;
			TestConfig::i().m_videoDebugOption["overrideMediaIPAddress"] = sOverrideIp;
			TestConfig::i().m_shareDebugOption["overrideMediaIPAddress"] = sOverrideIp;
		}
	}
	if (root.isMember("overrideports")){
		Json::Value ports = root["overrideports"];
		if (ports.isMember("audio")){
			TestConfig::i().m_audioDebugOption["overrideMediaPort"] = ports["audio"].asInt();
			TestConfig::i().m_videoDebugOption["overrideMediaPort"] = ports["video"].asInt();
			TestConfig::i().m_shareDebugOption["overrideMediaPort"] = ports["sharing"].asInt();
		}
	}
    if (root.isMember("bgra")){
        TestConfig::i().m_videoDebugOption["enableBGRA"] = root["bgra"].asBool();
    }
    if (root.isMember("ice")){
        TestConfig::i().m_audioDebugOption["enableICE"] = root["ice"].asBool();
        TestConfig::i().m_videoDebugOption["enableICE"] = root["ice"].asBool();
    }
    if (root.isMember("mux")){
        TestConfig::i().m_audioDebugOption["enableRTCPMux"] = root["mux"].asBool();
        TestConfig::i().m_videoDebugOption["enableRTCPMux"] = root["mux"].asBool();
    }
    if (root.isMember("srtp")){
        TestConfig::i().m_audioDebugOption["enableSRTP"] = root["srtp"].asBool();
        TestConfig::i().m_videoDebugOption["enableSRTP"] = root["srtp"].asBool();
    }
    if (root.isMember("dtls_srtp")){
        TestConfig::i().m_audioDebugOption["enableDtlsSRTP"] = root["dtls_srtp"].asBool();
        TestConfig::i().m_videoDebugOption["enableDtlsSRTP"] = root["dtls_srtp"].asBool();
    }
    if (root.isMember("is_dtls_server")){
        TestConfig::i().m_audioDebugOption["isDtlsServer"] = root["is_dtls_server"].asBool();
        TestConfig::i().m_videoDebugOption["isDtlsServer"] = root["is_dtls_server"].asBool();
    }
    if (root.isMember("mute")){
        TestConfig::i().m_bMuteAudio = root["mute"].asBool();
        TestConfig::i().m_bMuteVideo = root["mute"].asBool();
    }
    if (root.isMember("svs")) {
        TestConfig::i().m_bHasSVS = root["svs"].asBool();
    }
    if (root.isMember("resolution")){
		TestConfig::i().m_nVideoSize = root["resolution"].asInt();
    }
    if (root.isMember("limitNalSize")){
        TestConfig::i().m_videoParam["bLimitNalSize"] = root["limitNalSize"].asBool();
    }
    if (root.isMember("videoStreams")){
        TestConfig::i().m_uMaxVideoStreams = root["videoStreams"].asInt();
    }
    if (root.isMember("audiocount")){
        TestConfig::i().m_uMaxAudioStreams = root["audiocount"].asInt();
    }
    if (root.isMember("train")){
        TestConfig::i().m_bTrain = root["train"].asBool();
    }
    
    if( root.isMember("audioDumpPath") )
        TestConfig::i().m_audioParam["enableKeyDumpFilesPath"] = root["audioDumpPath"].asString();
    
    if (root.isMember("enableQos")) {
        TestConfig::i().m_bQoSEnable = root["enableQos"].asBool();
    } else {
        TestConfig::i().m_bQoSEnable = true;
    }
    
    if (root.isMember("enablePerformanceStatDump")) {
        TestConfig::i().m_globalParam["enablePerformanceStatDump"] = root["enablePerformanceStatDump"].asString();
    }
    
    if (root.isMember("params")) {
        Json::Value params = root["params"];
        Json::Value src;
        if (params.isMember("deviceCapability")) {
            TestConfig::i().m_globalParam["deviceCapability"] = params["deviceCapability"];
        }
        if (params.isMember("video")) {
            Json::Value vParam = params["video"];
            if (vParam.isMember("bHWAcceleration")) {
                TestConfig::i().m_bVideoHW = vParam["bHWAcceleration"].asBool();
                TestConfig::i().m_videoParam["bHWAcceleration"] = vParam["bHWAcceleration"];
            }
            if (vParam.isMember("uVideoDataDumpFlag")) {
                TestConfig::i().m_videoParam["uVideoDataDumpFlag"] = vParam["uVideoDataDumpFlag"].asInt();
            }
        }
        
    }

	if (root.isMember("params")){
        Json::Value params = root["params"];
        Json::Value src;
        if(params.isMember("audio"))
        {
            src = params["audio"];
            for (Json::ValueIterator it = src.begin(); it != src.end(); it++) {
                TestConfig::i().m_audioParam[it.memberName()] = *it;
            }
        }
        
        if(params.isMember("video"))
        {
            src = params["video"];
            for (Json::ValueIterator it = src.begin(); it != src.end(); it++) {
                TestConfig::i().m_videoParam[it.memberName()] = *it;
            }
        }
        
        if (params.isMember("global_param")) {
            src = params["global_param"];
            for (Json::ValueIterator it = src.begin(); it != src.end(); ++it) {
                TestConfig::i().m_globalParam[it.memberName()] = *it;
            }
        }
        
        if (params.isMember("share"))
        {
            src = params["share"];
            for (Json::ValueIterator it = src.begin(); it != src.end(); it++) {
                TestConfig::i().m_shareParam[it.memberName()] = *it;
            }
        }
	}

#ifndef OS_MAC_IOS // the path is set at WMEAppDelegate+Calabash.mm for ios
    UpdateFileCaptureRender(root);
#endif
    
    TestConfig::i().m_audioParam["supportCmulti"] = true; //root["multilayer"].asBool();
    TestConfig::i().m_videoParam["supportCmulti"] = true; //root["multilayer"].asBool();
    TestConfig::i().m_shareParam["supportCmulti"] = true; //root["multilayer"].asBool();

    if( root.isMember("option") )
    {
		Json::Value options = root["option"];

		if (options.isMember("audio"))
			TestConfig::i().m_bHasAudio = options["audio"].asBool();
		if (options.isMember("video"))
			TestConfig::i().m_bHasVideo = options["video"].asBool();
		if (options.isMember("share")){
			std::string share_role = options["share"].asString();
			TestConfig::i().m_bAppshare = !share_role.empty();
            TestConfig::i().m_bSharer = share_role.compare("sharer") == 0;
//            if (TestConfig::i().m_bAppshare
//                && options.isMember("share_mline_fake"))
//                TestConfig::i().m_bFakeVideoByShare = options["share_mline_fake"].asBool();
		}

        if( TestConfig::i().m_bAppshare ) {
            //TestConfig::i().m_bCalliope = false;
            if( TestConfig::i().m_bSharer )
            {
                if( root.isMember("sharesource") )
                    TestConfig::i().m_strScreenSharingAutoLaunchSourceName = root["sharesource"].asCString();
                LoadScreenSource();
                if( root.isMember("sharesource_file") && root["sharesource_file"].asString().size()>0 )
                    TestConfig::i().m_strScreenSharingCaptureFile = ExpandPath("sample/"+ root["sharesource_file"].asString());
            }
        }
        if(options.isMember("activerequestsd")) {
            TestConfig::i().m_bTestRequestVideoOverrideDefault = options["activerequestsd"].asBool();
        }
        if (options.isMember("calliope")){
            TestConfig::i().m_bCalliope = options["calliope"].asBool();
        }
    }

    puts(TestConfig::i().ToString().c_str());
	if (!bLoopback){
		TestConfig::i().m_bLoopback = false;
#ifndef OS_MAC_IOS
        PeerCall::Instance()->pushRemoteWindow(m_winRemote);
        if(m_sink)
        {
            m_sink->onCheckMultiVideo();
        }
		PeerCall::Instance()->startPeer(m_winLocal, m_winScreenViewer);

		if (sdp.empty())
			PeerCall::Instance()->m_endCaller->startCall();
		else
			PeerCall::Instance()->m_endCaller->acceptCall(sdp.c_str());
#endif
        
	}
    else {
#ifndef OS_MAC_IOS
        LoopbackCall::Instance()->startLoopback(m_winRemote, m_winLocal, m_winScreenViewer);
#endif
	}
    
    TestConfig::i().Dump();
	return "";
}

std::string CBackdoorAgent::backdoorTestEcho(std::string arg)
{
    CM_INFO_TRACE("CBackdoorAgent::backdoorTestEcho start arg=" << arg.c_str());
    
    Json::Reader r;
    Json::Value root;
    r.parse(arg, root, true);
    
    bool bLoopback = root["loopback"].asBool();
    TestConfig::i().m_bLoopback = bLoopback;
    std::string linusAddr = root["linus"].asString();
    bool bFileMode = root["filemode"].asBool();
    bool bTimestampMode = root["timestamp"].asBool();
    std::string sdp = root["sdp"].asString();
    if (root.isMember("overrideip")){
        std::string sOverrideIp = root["overrideip"].asString();
        if (!sOverrideIp.empty()){
            TestConfig::i().m_audioDebugOption["overrideMediaIPAddress"] = sOverrideIp;
            TestConfig::i().m_videoDebugOption["overrideMediaIPAddress"] = sOverrideIp;
            TestConfig::i().m_shareDebugOption["overrideMediaIPAddress"] = sOverrideIp;
        }
    }
    if (root.isMember("overrideports")){
        Json::Value ports = root["overrideports"];
        if (ports.isMember("audio")){
            TestConfig::i().m_audioDebugOption["overrideMediaPort"] = ports["audio"].asInt();
            TestConfig::i().m_videoDebugOption["overrideMediaPort"] = ports["video"].asInt();
            TestConfig::i().m_shareDebugOption["overrideMediaPort"] = ports["sharing"].asInt();
        }
    }
    if (root.isMember("bgra")){
        TestConfig::i().m_videoDebugOption["enableBGRA"] = root["bgra"].asBool();
    }
    if (root.isMember("ice")){
        TestConfig::i().m_audioDebugOption["enableICE"] = root["ice"].asBool();
        TestConfig::i().m_videoDebugOption["enableICE"] = root["ice"].asBool();
    }
    if (root.isMember("mux")){
        TestConfig::i().m_audioDebugOption["enableRTCPMux"] = root["mux"].asBool();
        TestConfig::i().m_videoDebugOption["enableRTCPMux"] = root["mux"].asBool();
    }
    if (root.isMember("srtp")){
        TestConfig::i().m_audioDebugOption["enableSRTP"] = root["srtp"].asBool();
        TestConfig::i().m_videoDebugOption["enableSRTP"] = root["srtp"].asBool();
    }
    if (root.isMember("mute")){
        TestConfig::i().m_bMuteAudio = root["mute"].asBool();
        TestConfig::i().m_bMuteVideo = root["mute"].asBool();
    }
    if (root.isMember("resolution")){
        TestConfig::i().m_nVideoSize = root["resolution"].asInt();
    }
    if (root.isMember("limitNalSize")){
        TestConfig::i().m_videoParam["bLimitNalSize"] = root["limitNalSize"].asBool();
    }
    if (root.isMember("videoStreams")){
        TestConfig::i().m_uMaxVideoStreams = root["videoStreams"].asInt();
    }
    if (root.isMember("audiocount")){
        TestConfig::i().m_uMaxAudioStreams = root["audiocount"].asInt();
    }
    
    if (root.isMember("params")) {
        Json::Value params = root["params"];
        if (params.isMember("deviceCapability")) {
            TestConfig::i().m_globalParam["deviceCapability"] = params["deviceCapability"];
        }
        if (params.isMember("video")) {
            Json::Value vParam = params["video"];
            if (vParam.isMember("bHWAcceleration")) {
                TestConfig::i().m_bVideoHW = vParam["bHWAcceleration"].isBool();
            }
        }
    }
    
    std::string sParams;
    if (root.isMember("params")){
        Json::Value params = root["params"];
        Json::FastWriter w;
        sParams = w.write(params);
    }
    //    if (root.isMember("share_mline_fake")){
    //        TestConfig::i().m_bFakeVideoByShare = root["share_mline_fake"].asBool();
    //    }
#ifndef OS_MAC_IOS // the path is set at WMEAppDelegate+Calabash.mm for ios
   
    if (bFileMode) {
        TestConfig::i().setFilePath(ExpandPath("sample/audio_1_8000_16.pcm").c_str(), ExpandPath("Temp/audio_1_8000_16.pcm").c_str(), WmeSessionType_Audio);
        
        int resolution = 2;
        if (root.isMember("resolution")) {
            resolution = root["resolution"].asInt();
        }
        if (0 == resolution) {
            TestConfig::i().setFilePath(ExpandPath("sample/video_160_90_6_i420.yuv").c_str(), ExpandPath("Temp/video_160_90_6_i420.yuv").c_str(), WmeSessionType_Video);
        } else if (1 == resolution) {
            TestConfig::i().setFilePath(ExpandPath("sample/video_320_180_12_i420.yuv").c_str(), ExpandPath("Temp/video_320_180_12_i420.yuv").c_str(), WmeSessionType_Video);
        } else if (2 == resolution){
            TestConfig::i().setFilePath(ExpandPath("sample/video_640_360_24_i420.yuv").c_str(), ExpandPath("Temp/video_640_360_24_i420.yuv").c_str(), WmeSessionType_Video);
        } else {
            TestConfig::i().setFilePath(ExpandPath("sample/video_1280_720_30_i420.yuv").c_str(), ExpandPath("Temp/video_1280_720_30_i420.yuv").c_str(), WmeSessionType_Video);
        }

        if (root.isMember("audiosouce_file") && root["audiosouce_file"].asString().size()>0){
            TestConfig::i().setFilePath(ExpandPath("sample/" + root["audiosouce_file"].asString()).c_str(), "", WmeSessionType_Audio);
        }
        if (root.isMember("videosouce_file") && root["videosouce_file"].asString().size()>0){
            TestConfig::i().setFilePath(ExpandPath("sample/" + root["videosouce_file"].asString()).c_str(), "", WmeSessionType_Video);
        }

        if (root.isMember("isLoopFile")) {
            Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
            capture["isLoop"] = root["isLoopFile"].asBool();
        }
        
        if (bTimestampMode) {
            Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
            capture["timestamp"] = Json::Value(bTimestampMode ? true : false);
        }
        
        TestConfig::i().m_sLinusUrl = linusAddr;
        TestConfig::i().m_bCalliope = true;
    }
#endif
    
    TestConfig::i().m_audioParam["supportCmulti"] = true; //root["multilayer"].asBool();
    TestConfig::i().m_videoParam["supportCmulti"] = true; //root["multilayer"].asBool();
    TestConfig::i().m_shareParam["supportCmulti"] = true; //root["multilayer"].asBool();
    
    if( root.isMember("option") )
    {
        Json::Value options = root["option"];
        
        if (options.isMember("audio"))
            TestConfig::i().m_bHasAudio = options["audio"].asBool();
            if (options.isMember("video"))
                TestConfig::i().m_bHasVideo = options["video"].asBool();
                if (options.isMember("share")){
                    std::string share_role = options["share"].asString();
                    TestConfig::i().m_bAppshare = !share_role.empty();
                    TestConfig::i().m_bSharer = share_role.compare("sharer") == 0;
                    //            if (TestConfig::i().m_bAppshare
                    //                && options.isMember("share_mline_fake"))
                    //                TestConfig::i().m_bFakeVideoByShare = options["share_mline_fake"].asBool();
                }
        
        if( TestConfig::i().m_bAppshare ) {
            //TestConfig::i().m_bCalliope = false;
            if( TestConfig::i().m_bSharer )
            {
                if( root.isMember("sharesource") )
                    TestConfig::i().m_strScreenSharingAutoLaunchSourceName = root["sharesource"].asCString();
                    LoadScreenSource();
                    if( root.isMember("sharesource_file") && root["sharesource_file"].asString().size()>0 )
                        TestConfig::i().m_strScreenSharingCaptureFile = ExpandPath("sample/"+ root["sharesource_file"].asString());
                        }
        }
    }
    
    puts(TestConfig::i().ToString().c_str());
    if (!bLoopback){
        TestConfig::i().m_bLoopback = false;
#ifndef OS_MAC_IOS
        PeerCall::Instance()->pushRemoteWindow(m_winRemote);
        if(m_sink)
        {
            m_sink->onCheckMultiVideo();
        }
        PeerCall::Instance()->startPeer(m_winLocal, m_winScreenViewer);
#endif
        //This code should be removed, that has been duplicated called in startCall
        if (!sParams.empty()){
            PeerCall::Instance()->m_endCaller->setParam(PARAM_AUDIO, sParams.c_str());
            PeerCall::Instance()->m_endCaller->setParam(PARAM_VIDEO, sParams.c_str());
            PeerCall::Instance()->m_endCaller->setParam(PARAM_SCREEN, sParams.c_str());
        }
#ifndef OS_MAC_IOS
        if (sdp.empty())
            PeerCall::Instance()->m_endCaller->startCall();
            else
                PeerCall::Instance()->m_endCaller->acceptCall(sdp.c_str());
#endif
                
                }
    else {
#ifndef OS_MAC_IOS
        LoopbackCall::Instance()->startLoopback(m_winRemote, m_winLocal, m_winScreenViewer);
#endif
        //This code should be removed, that has been duplicated called in startCall
        if (!sParams.empty()){
            PeerCall::Instance()->m_endCaller->setParam(PARAM_AUDIO, sParams.c_str());
            PeerCall::Instance()->m_endCaller->setParam(PARAM_VIDEO, sParams.c_str());
            PeerCall::Instance()->m_endCaller->setParam(PARAM_SCREEN, sParams.c_str());
        }
    }
    
    TestConfig::i().Dump();
    return "";
}

std::string CBackdoorAgent::backdoorCalMos(std::string arg)
{
    CM_INFO_TRACE("CBackdoorAgent::backdoorCalMos start arg=" << arg.c_str());
    
    Json::Reader r;
    Json::Value root;
    r.parse(arg, root, true);
    
    bool bLoopback = root["loopback"].asBool();
    TestConfig::i().m_bLoopback = bLoopback;
    std::string linusAddr = root["linus"].asString();
    bool bFileMode = root["filemode"].asBool();
    bool bTimestampMode = root["timestamp"].asBool();
    std::string sdp = root["sdp"].asString();
    if (root.isMember("overrideip")){
        std::string sOverrideIp = root["overrideip"].asString();
        if (!sOverrideIp.empty()){
            TestConfig::i().m_audioDebugOption["overrideMediaIPAddress"] = sOverrideIp;
            TestConfig::i().m_videoDebugOption["overrideMediaIPAddress"] = sOverrideIp;
            TestConfig::i().m_shareDebugOption["overrideMediaIPAddress"] = sOverrideIp;
        }
    }
    if (root.isMember("overrideports")){
        Json::Value ports = root["overrideports"];
        if (ports.isMember("audio")){
            TestConfig::i().m_audioDebugOption["overrideMediaPort"] = ports["audio"].asInt();
            TestConfig::i().m_videoDebugOption["overrideMediaPort"] = ports["video"].asInt();
            TestConfig::i().m_shareDebugOption["overrideMediaPort"] = ports["sharing"].asInt();
        }
    }
    if (root.isMember("bgra")){
        TestConfig::i().m_videoDebugOption["enableBGRA"] = root["bgra"].asBool();
    }
    if (root.isMember("ice")){
        TestConfig::i().m_audioDebugOption["enableICE"] = root["ice"].asBool();
        TestConfig::i().m_videoDebugOption["enableICE"] = root["ice"].asBool();
    }
    if (root.isMember("mux")){
        TestConfig::i().m_audioDebugOption["enableRTCPMux"] = root["mux"].asBool();
        TestConfig::i().m_videoDebugOption["enableRTCPMux"] = root["mux"].asBool();
    }
    if (root.isMember("srtp")){
        TestConfig::i().m_audioDebugOption["enableSRTP"] = root["srtp"].asBool();
        TestConfig::i().m_videoDebugOption["enableSRTP"] = root["srtp"].asBool();
    }
    if (root.isMember("mute")){
        TestConfig::i().m_bMuteAudio = root["mute"].asBool();
        TestConfig::i().m_bMuteVideo = root["mute"].asBool();
    }
    if (root.isMember("resolution")){
        TestConfig::i().m_nVideoSize = root["resolution"].asInt();
    }
    if (root.isMember("limitNalSize")){
        TestConfig::i().m_videoParam["bLimitNalSize"] = root["limitNalSize"].asBool();
    }
    if (root.isMember("videoStreams")){
        TestConfig::i().m_uMaxVideoStreams = root["videoStreams"].asInt();
    }
    if (root.isMember("audiocount")){
        TestConfig::i().m_uMaxAudioStreams = root["audiocount"].asInt();
    }
    
    if (root.isMember("params")) {
        Json::Value params = root["params"];
        if (params.isMember("deviceCapability")) {
            TestConfig::i().m_globalParam["deviceCapability"] = params["deviceCapability"];
        }
        if (params.isMember("video")) {
            Json::Value vParam = params["video"];
            if (vParam.isMember("bHWAcceleration")) {
                TestConfig::i().m_bVideoHW = vParam["bHWAcceleration"].isBool();
            }
        }
    }
    
    std::string sParams;
    if (root.isMember("params")){
        Json::Value params = root["params"];
        Json::FastWriter w;
        sParams = w.write(params);
    }
    //    if (root.isMember("share_mline_fake")){
    //        TestConfig::i().m_bFakeVideoByShare = root["share_mline_fake"].asBool();
    //    }
#ifndef OS_MAC_IOS // the path is set at WMEAppDelegate+Calabash.mm for ios
    if( root.isMember("audiosouce_file") && root["audiosouce_file"].asString().size()>0 ){
        TestConfig::i().setFilePath( ExpandPath("sample/"+ root["audiosouce_file"].asString()).c_str(), "", WmeSessionType_Audio);
        Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
        capture["isLoop"] = true;
        capture["timestamp"] = true;
        
    }
    if( root.isMember("videosouce_file") && root["videosouce_file"].asString().size()>0 ){
        TestConfig::i().setFilePath( ExpandPath("sample/"+ root["videosouce_file"].asString()).c_str(), "", WmeSessionType_Video);
        
        Json::Value &capture = TestConfig::i().m_audioDebugOption["fileCapture"];
        capture["isLoop"] = true;
        capture["timestamp"] = true;
    }
    
    if (bFileMode) {
        TestConfig::i().setFilePath(ExpandPath("sample/audio_1_8000_16.pcm").c_str(), ExpandPath("Temp/audio_1_8000_16.pcm").c_str(), WmeSessionType_Audio);
        
        int resolution = 2;
        if (root.isMember("resolution")) {
            resolution = root["resolution"].asInt();
        }
        if (0 == resolution) {
            TestConfig::i().setFilePath(ExpandPath("sample/video_160_90_6_i420.yuv").c_str(), ExpandPath("Temp/video_160_90_6_i420.yuv").c_str(), WmeSessionType_Video);
        } else if (1 == resolution) {
            TestConfig::i().setFilePath(ExpandPath("sample/video_320_180_12_i420.yuv").c_str(), ExpandPath("Temp/video_320_180_12_i420.yuv").c_str(), WmeSessionType_Video);
        } else {
            TestConfig::i().setFilePath(ExpandPath("sample/video_640_360_24_i420.yuv").c_str(), ExpandPath("Temp/video_640_360_24_i420.yuv").c_str(), WmeSessionType_Video);
        }
        
        if (root.isMember("isLoopFile")) {
            Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
            capture["isLoop"] = root["isLoopFile"].asBool();
        }
        
        if (bTimestampMode) {
            Json::Value &capture = TestConfig::i().m_videoDebugOption["fileCapture"];
            capture["timestamp"] = Json::Value(bTimestampMode ? true : false);
        }
        
        TestConfig::i().m_sLinusUrl = linusAddr;
        TestConfig::i().m_bCalliope = true;
    }
#endif
    
    TestConfig::i().m_audioParam["supportCmulti"] = true; //root["multilayer"].asBool();
    TestConfig::i().m_videoParam["supportCmulti"] = true; //root["multilayer"].asBool();
    TestConfig::i().m_shareParam["supportCmulti"] = true; //root["multilayer"].asBool();
    
    if( root.isMember("option") )
    {
        Json::Value options = root["option"];
        
        if (options.isMember("audio"))
            TestConfig::i().m_bHasAudio = options["audio"].asBool();
            if (options.isMember("video"))
                TestConfig::i().m_bHasVideo = options["video"].asBool();
                if (options.isMember("share")){
                    std::string share_role = options["share"].asString();
                    TestConfig::i().m_bAppshare = !share_role.empty();
                    TestConfig::i().m_bSharer = share_role.compare("sharer") == 0;
                    //            if (TestConfig::i().m_bAppshare
                    //                && options.isMember("share_mline_fake"))
                    //                TestConfig::i().m_bFakeVideoByShare = options["share_mline_fake"].asBool();
                }
        
        if( TestConfig::i().m_bAppshare ) {
            //TestConfig::i().m_bCalliope = false;
            if( TestConfig::i().m_bSharer )
            {
                if( root.isMember("sharesource") )
                    TestConfig::i().m_strScreenSharingAutoLaunchSourceName = root["sharesource"].asCString();
                    LoadScreenSource();
                    if( root.isMember("sharesource_file") && root["sharesource_file"].asString().size()>0 )
                        TestConfig::i().m_strScreenSharingCaptureFile = ExpandPath("sample/"+ root["sharesource_file"].asString());
                        }
        }
    }
    
    puts(TestConfig::i().ToString().c_str());
    if (!bLoopback){
        TestConfig::i().m_bLoopback = false;
#ifndef OS_MAC_IOS
        PeerCall::Instance()->pushRemoteWindow(m_winRemote);
        if(m_sink)
        {
            m_sink->onCheckMultiVideo();
        }
        PeerCall::Instance()->startPeer(m_winLocal, m_winScreenViewer);
#endif
        //This code should be removed, that has been duplicated called in startCall
        if (!sParams.empty()){
            PeerCall::Instance()->m_endCaller->setParam(PARAM_AUDIO, sParams.c_str());
            PeerCall::Instance()->m_endCaller->setParam(PARAM_VIDEO, sParams.c_str());
            PeerCall::Instance()->m_endCaller->setParam(PARAM_SCREEN, sParams.c_str());
        }
#ifndef OS_MAC_IOS
        if (sdp.empty())
            PeerCall::Instance()->m_endCaller->startCall();
            else
                PeerCall::Instance()->m_endCaller->acceptCall(sdp.c_str());
#endif
                
                }
    else {
#ifndef OS_MAC_IOS
        LoopbackCall::Instance()->startLoopback(m_winRemote, m_winLocal, m_winScreenViewer);
#endif
        //This code should be removed, that has been duplicated called in startCall
        if (!sParams.empty()){
            PeerCall::Instance()->m_endCaller->setParam(PARAM_AUDIO, sParams.c_str());
            PeerCall::Instance()->m_endCaller->setParam(PARAM_VIDEO, sParams.c_str());
            PeerCall::Instance()->m_endCaller->setParam(PARAM_SCREEN, sParams.c_str());
        }
    }
    
    TestConfig::i().Dump();
    return "";
}

std::string CBackdoorAgent::backdoorCheckHWSupport(std::string arg)
{
    std::string ret = "false";
    if (true == m_sink->onCheckHWEnable()) {
        ret = "true";
    }
    return ret;
}

void CBackdoorAgent::LoadScreenSource(){
    IWmeScreenSource* pIWmeScreenSource = NULL;
    if( TestConfig::i().m_strScreenSharingAutoLaunchSourceName == "screen:")
    {
        pIWmeScreenSource = CASEnum::Instance()->GetOneDefaultScreenSource(WmeScreenSourceTypeDesktop);
    }
    else {
        pIWmeScreenSource = CASEnum::Instance()->FindScreenSourceByFriendlyName(TestConfig::i().m_strScreenSharingAutoLaunchSourceName.c_str());
        if( NULL == pIWmeScreenSource ) {
            CM_ERROR_TRACE("CBackdoorAgent::LoadScreenSource fail, not soucename=" << TestConfig::i().m_strScreenSharingAutoLaunchSourceName.c_str());
            pIWmeScreenSource = CASEnum::Instance()->GetOneDefaultScreenSource(WmeScreenSourceTypeApplication);
        }
    }
    
    if (pIWmeScreenSource){
        if (TestConfig::i().m_bLoopback){
            LoopbackCall::Instance()->m_endCaller.addScreenSouce(pIWmeScreenSource);
        }
        else{
            if (PeerCall::Instance()->m_endCaller)
                PeerCall::Instance()->m_endCaller->addScreenSouce(pIWmeScreenSource);
        }
    }
}

std::string CBackdoorAgent::backdoorIsFileCaptureEnded(std::string arg)
{
	bool ret = false;
    if (arg.empty() || arg == "all") {
        if (!PeerCall::Instance()->isActive()) {
            ret = LoopbackCall::Instance()->m_endCaller.m_audioFileEnded
                && LoopbackCall::Instance()->m_endCaller.m_videoFileEnded;
        }
        else {
            ret = PeerCall::Instance()->m_endCaller->m_audioFileEnded
                && PeerCall::Instance()->m_endCaller->m_videoFileEnded;
        }
    }
    else if (arg == "audio") {
        if (!PeerCall::Instance()->isActive()) {
            ret = LoopbackCall::Instance()->m_endCaller.m_audioFileEnded;
        }
        else {
            ret = PeerCall::Instance()->m_endCaller->m_audioFileEnded;
        }
    }
    else if (arg == "video") {
        if (!PeerCall::Instance()->isActive()) {
            ret = LoopbackCall::Instance()->m_endCaller.m_videoFileEnded;
        }
        else {
            ret = PeerCall::Instance()->m_endCaller->m_videoFileEnded;
        }
    }
	return ret ? "ended" : "playing";
}

std::string CBackdoorAgent::backdoorGetLocalSdp(std::string arg)
{
	if (PeerCall::Instance()->isActive() && PeerCall::Instance()->m_endCaller)
		return PeerCall::Instance()->m_endCaller->m_localSdp;
    else
        return LoopbackCall::Instance()->m_endCaller.m_localSdp;
	return "Failed";
}

std::string CBackdoorAgent::backdoorSetRemoteSdp(std::string arg)
{
	if (PeerCall::Instance()->m_endCaller){
		PeerCall::Instance()->m_endCaller->answerReceived(arg.c_str());
		return "";
	}

	return "Failed";
}

Json::Value CBackdoorAgent::GetStatsNetworkInfo(WmeNetworkStatistics &netStats)
{
	Json::Value root(Json::objectValue);
	root["fLossRatio"] = Json::Value(netStats.fLossRatio);
	root["uJitter"] = Json::Value(netStats.uJitter);
	root["uRoundTripTime"] = Json::Value(netStats.uRoundTripTime);
	root["uBytes"] = Json::Value(netStats.uBytes);
	root["uPackets"] = Json::Value(netStats.uPackets);
	root["uFailedIOPackets"] = Json::Value(netStats.uFailedIOPackets);
	root["uProcessedPackets"] = Json::Value(netStats.uProcessedPackets);
	root["uBitRate"] = Json::Value(netStats.uBitRate);
	root["uOOOPackets"] = Json::Value(netStats.uOOOPackets);
	root["uDupPackets"] = Json::Value(netStats.uDupPackets);
	root["uLostPackets"] = Json::Value(netStats.uLostPackets);
	root["uRRtimeWindowMs"] = Json::Value(netStats.uRRtimeWindowMs);
    root["bFecEnabled"] = Json::Value(netStats.bFecEnabled);
	root["uFECLevel"] = Json::Value(netStats.uFECLevel);
	root["uFECRecoveredPackets"] = Json::Value(netStats.uFECRecoveredPackets);
	root["uFECBitRate"] = Json::Value(netStats.uFECBitRate);
    root["uFECResidualLoss"] = Json::Value(netStats.fFECResidualLoss);
	root["fRTPDropRatio"] = Json::Value(netStats.fRTPDropRatio);
	root["uEvaluatedBW"] = Json::Value(netStats.uEvaluatedBW);
	root["uFECBW"] = Json::Value(netStats.uFECBW);
    root["uInputBitRate"] = Json::Value(netStats.uInputBitRate);
    root["uAdaptedFrames"] = Json::Value(netStats.uAdaptedFrames);
    root["uDroppedFrames"] = Json::Value(netStats.uDroppedFrames);
    root["uMaxBufferedMs"] = Json::Value(netStats.uMaxBufferedMs);
	return root;
}

Json::Value CBackdoorAgent::GetStatsConnInfo(WmeConnectionStatistics& connStats)
{
	Json::Value root(Json::objectValue);
	root["uRTPReceived"] = Json::Value(connStats.uRTPReceived);
	root["uRTPSent"] = Json::Value(connStats.uRTPSent);
	root["uRTCPReceived"] = Json::Value(connStats.uRTCPReceived);
	root["uRTCPSent"] = Json::Value(connStats.uRTCPSent);
	root["uRemotePort"] = Json::Value(connStats.uRemotePort);
	root["uLocalPort"] = Json::Value(connStats.uLocalPort);
	root["remoteIp"] = Json::Value(connStats.remoteIp);
	root["localIp"] = Json::Value(connStats.localIp);
	return root;
}

Json::Value CBackdoorAgent::GetStatsAudioInfo(WmeAudioStatistics &audioStats)
{
	Json::Value root(Json::objectValue);
	root["bEnableCodecFEC"] = Json::Value(audioStats.bEnableCodecFEC);
	root["uCodecFECRecoveredPackets"] = Json::Value(audioStats.uCodecFECRecoveredPackets);
	root["uJitterDropMs"] = Json::Value(audioStats.uJitterDropMs);
	root["uBitRate"] = Json::Value(audioStats.uBitRate);
    root["codecType"] = Json::Value(audioStats.codecType);

	return root;
}

Json::Value CBackdoorAgent::GetStatsVideoInfo(WmeVideoStatistics &videoStats)
{
	Json::Value root(Json::objectValue);
	root["uWidth"] = Json::Value(videoStats.uWidth);
	root["uHeight"] = Json::Value(videoStats.uHeight);
    root["uStreamNum"] = Json::Value(videoStats.uStreamNum);
	root["fFrameRate"] = Json::Value(videoStats.fFrameRate);
	root["fBitRate"] = Json::Value(videoStats.fBitRate);
	root["fAvgBitRate"] = Json::Value(videoStats.fAvgBitRate);
	root["uIDRReqNum"] = Json::Value(videoStats.uIDRReqNum);
	root["uIDRSentNum"] = Json::Value(videoStats.uIDRSentNum);
	root["fDecoderFrameDropRatio"] = Json::Value(videoStats.fDecoderFrameDropRatio);
    root["uRenderFrameCount"] = Json::Value(videoStats.uRenderFrameCount);
    root["uEcNumCount"] = Json::Value(videoStats.uEcNumCount);
    root["bHWEnable"] = Json::Value(videoStats.bHWEnable);
    root["uEncodeFrameCount"] = Json::Value(videoStats.uEncodeFrameCount);
    root["uExceedMaxNalSizeCount"] = Json::Value(videoStats.uExceedMaxNalSizeCount);
    
	return root;
}

Json::Value CBackdoorAgent::GetStatsShareInfo(WmeScreenShareStatistics &shareStats)
{
    Json::Value root(Json::objectValue);
	root["uWidth"] = Json::Value(shareStats.stVideoStat.uWidth);
	root["uHeight"] = Json::Value(shareStats.stVideoStat.uHeight);
	root["fFrameRate"] = Json::Value(shareStats.stVideoStat.fFrameRate);
	root["fBitRate"] = Json::Value(shareStats.stVideoStat.fBitRate);
	root["fAvgBitRate"] = Json::Value(shareStats.stVideoStat.fAvgBitRate);
	root["uIDRReqNum"] = Json::Value(shareStats.stVideoStat.uIDRReqNum);
	root["uIDRSentNum"] = Json::Value(shareStats.stVideoStat.uIDRSentNum);
	root["fDecoderFrameDropRatio"] = Json::Value(shareStats.stVideoStat.fDecoderFrameDropRatio);
    root["uRenderFrameCount"] = Json::Value(shareStats.stVideoStat.uRenderFrameCount);
    root["uEcNumCount"] = Json::Value(shareStats.stVideoStat.uEcNumCount);
    
    root["uTotalCapturedFrames"] = Json::Value(shareStats.stScreenShareCaptureStatistics.m_nTotalCapturedFrames);
    root["uMsAverageCaptureTime"] = Json::Value(shareStats.stScreenShareCaptureStatistics.m_nMsAverageCaptureTime);
    

	return root;
}

std::string CBackdoorAgent::backdoorGetStatistics(std::string arg)
{
	WmeAudioConnectionStatistics audioStats = { 0 };
	WmeVideoConnectionStatistics videoStats = { 0 };
    WmeScreenConnectionStatistics shareStats = { 0 };
    
	if (PeerCall::Instance()->isActive()) {
		PeerCall::Instance()->m_endCaller->getStatistics(audioStats, videoStats);
        PeerCall::Instance()->m_endCaller->getScreenStatistics(shareStats);
	}
	else{
		LoopbackCall::Instance()->getStatistics(audioStats, videoStats);
        LoopbackCall::Instance()->getScreenStatistics(shareStats);
	}

	Json::Value root(Json::objectValue);
	root["audioConn"] = GetStatsConnInfo(audioStats.connStat);
	root["videoConn"] = GetStatsConnInfo(videoStats.connStat);
  	root["shareConn"] = GetStatsConnInfo(shareStats.connStat);
    
	root["audioInNet"] = GetStatsNetworkInfo(audioStats.sessStat.stInNetworkStat);
	root["audioOutNet"] = GetStatsNetworkInfo(audioStats.sessStat.stOutNetworkStat);
	root["videoInNet"] = GetStatsNetworkInfo(videoStats.sessStat.stInNetworkStat);
	root["videoOutNet"] = GetStatsNetworkInfo(videoStats.sessStat.stOutNetworkStat);
 	root["shareInNet"] = GetStatsNetworkInfo(shareStats.sessStat.stInNetworkStat);
	root["shareOutNet"] = GetStatsNetworkInfo(shareStats.sessStat.stOutNetworkStat);
    
	root["audioLocal"] = GetStatsAudioInfo(audioStats.localAudioStat);
	root["audioRemote"] = GetStatsAudioInfo(audioStats.remoteAudioStat);
	root["videoLocal"] = GetStatsVideoInfo(videoStats.localVideoStat);
	root["videoRemote"] = GetStatsVideoInfo(videoStats.remoteVideoStat);
    root["shareLocal"] = GetStatsShareInfo(shareStats.localScreenStat);
	root["shareRemote"] = GetStatsShareInfo(shareStats.remoteScreenStat);
    
    if (PeerCall::Instance()->isActive())
    {
        Json::Value tracks(Json::arrayValue);
        unsigned index = 0;
        while (true) {
            WmeVideoStatistics video = {0};
            if(PeerCall::Instance()->m_endCaller->getTrackStatistics(index, video)) {
                tracks[index] = GetStatsVideoInfo(video);
                index++;
            }
            else {
                break;
            }
        }
        root["videoTracks"] = tracks;
    } else {
        Json::Value tracks(Json::arrayValue);
        unsigned index = 0;
        while (true) {
            WmeVideoStatistics video = {0};
            if(LoopbackCall::Instance()->m_endCallee->getTrackStatistics(index, video)) {
                tracks[index] = GetStatsVideoInfo(video);
                index++;
            }
            else {
                break;
            }
        }
        root["videoTracks"] = tracks;
    }

	Json::FastWriter w;
	std::string sRet = w.write(root);
	CM_INFO_TRACE("backdoorGetStatistics, info=" << sRet);
	return sRet;
}

std::string CBackdoorAgent::backdoorGetFeatureToggles(std::string arg)
{
    string policy;
    
    if (PeerCall::Instance()->isActive()) {
        PeerCall::Instance()->m_endCaller->getFeatureToggleStatistics(policy);
    }
    else{
        LoopbackCall::Instance()->getFeatureToggleStatistics(policy);
    }

    CM_INFO_TRACE("backdoorGetFeatureToggles, info=" << policy);
    return policy;
}

Json::Value CBackdoorAgent::GetRunningStats(wme::WmeNetworkMetricStats& runningStats)
{
    Json::Value result(Json::objectValue);
    Json::Value histogram(Json::arrayValue);
    for (int i = 0; i < 21; i++) {
        histogram[i] = runningStats.vHistogram[i];
    }

    result["histo"] = histogram;
    result["stddev"] = Json::Value(runningStats.dStdDev);
    result["cov"] = Json::Value(runningStats.dCov);
    result["mean"] = Json::Value(runningStats.dMean);
    
    return result;
}

Json::Value CBackdoorAgent::GetAggressiveness(wme::WmeAggregateNetworkMetricStats& networkMetrics)
{
    Json::Value aggressiveness(Json::arrayValue);
    int index = 0;
    for (int i =0; i < 10; i++) {
        if (networkMetrics.stAggressiveSmoothness[i].eTrend == wme::BANDWIDTH_UPGRADING) {
            Json::Value aggressivenessItem(Json::objectValue);
            aggressivenessItem["timecost"] = networkMetrics.stAggressiveSmoothness[i].uTimeCost;
            aggressivenessItem["start_bw"] = networkMetrics.stAggressiveSmoothness[i].dStartBandwidth;
            aggressivenessItem["end_bw"] = networkMetrics.stAggressiveSmoothness[i].dEndBandwidth;
            aggressivenessItem["aggressiveness"] = networkMetrics.stAggressiveSmoothness[i].dAggressiveSmoothness;
            aggressiveness[index] = aggressivenessItem;
            index++;
        }
    }
    
    return aggressiveness;
}

Json::Value CBackdoorAgent::GetSmoothness(wme::WmeAggregateNetworkMetricStats& networkMetrics)
{
    Json::Value smoothness(Json::arrayValue);
    int index = 0;
    for (int i =0; i < 10; i++) {
        if (networkMetrics.stAggressiveSmoothness[i].eTrend == wme::BANDWIDTH_DOWNGRADING) {
            Json::Value smoothnessItem(Json::objectValue);
            smoothnessItem["timecost"] = networkMetrics.stAggressiveSmoothness[i].uTimeCost;
            smoothnessItem["start_bw"] = networkMetrics.stAggressiveSmoothness[i].dStartBandwidth;
            smoothnessItem["end_bw"] = networkMetrics.stAggressiveSmoothness[i].dEndBandwidth;
            smoothnessItem["smoothness"] = networkMetrics.stAggressiveSmoothness[i].dAggressiveSmoothness;
            smoothness[index] = smoothnessItem;
            index++;
        }
    }
    
    return smoothness;
}

Json::Value CBackdoorAgent::GetOscillation(wme::WmeAggregateNetworkMetricStats& networkMetrics)
{
    Json::Value oscillation(Json::arrayValue);
    int index = 0;
    for (int i =0; i < 10; i++) {
        if (networkMetrics.stAggressiveSmoothness[i].eTrend == wme::BANDWIDTH_OSCILLATION) {
            Json::Value oscillationItem(Json::objectValue);
            oscillationItem["timecost"] = networkMetrics.stAggressiveSmoothness[i].uTimeCost;
            oscillationItem["start_bw"] = networkMetrics.stAggressiveSmoothness[i].dStartBandwidth;
            oscillationItem["end_bw"] = networkMetrics.stAggressiveSmoothness[i].dEndBandwidth;
            oscillationItem["oscillation"] = networkMetrics.stAggressiveSmoothness[i].dAggressiveSmoothness;
            oscillation[index] = oscillationItem;
            index++;
        }
    }
    
    return oscillation;
}

std::string CBackdoorAgent::backdoorGetNetworkMetrics(std::string arg)
{
    wme::WmeAggregateNetworkMetricStats networkMetrics;
    
    if (PeerCall::Instance()->isActive()) {
        PeerCall::Instance()->m_endCaller->GetNetworkMetrics(networkMetrics);
    }
    else{
        LoopbackCall::Instance()->GetNetworkMetrics(networkMetrics);
    }
    
    Json::Value qos(Json::objectValue);
    qos["estibw"] = GetRunningStats(networkMetrics.stBandwidthStats);
    qos["recvrate"] = GetRunningStats(networkMetrics.stRecvRateStats);
    qos["sendrate"] = GetRunningStats(networkMetrics.stSendRateStats);
    qos["rtt"] = GetRunningStats(networkMetrics.stRttStats);
    qos["qdelay"] = GetRunningStats(networkMetrics.stQdelayStats);
    qos["lossrate"] = GetRunningStats(networkMetrics.stLossStats);
    
    qos["stability"] = networkMetrics.fStabilityRatio;
    qos["bandwidthusage"] = networkMetrics.fBandwidthUsage;

    qos["upgrading"] = GetAggressiveness(networkMetrics);
    qos["downgrading"] = GetSmoothness(networkMetrics);
    qos["oscillation"] = GetOscillation(networkMetrics);
    
    Json::Value root(Json::objectValue);
    root["qos"] = qos;
    
    Json::FastWriter w;
    std::string sRet = w.write(root);
    CM_INFO_TRACE("backdoorGetNetworkMetrics, info=" << sRet);
    return sRet;
}

std::string CBackdoorAgent::backdoorStopCall(std::string arg)
{
	std::string ret;
	if (PeerCall::Instance()->isActive())
		ret = PeerCall::Instance()->stopPeer();
	else
		ret = LoopbackCall::Instance()->stopLoopback();
    
#ifdef ENABLED_GCOV_FLAG
    __gcov_flush();
#endif
	return ret;
}

WmeSessionType CBackdoorAgent::ofSessionType(std::string sType)
{
	WmeSessionType sessType = WmeSessionType_Audio;
	if (sType.compare("audio") == 0)
		sessType = WmeSessionType_Audio;
	else if (sType.compare("video") == 0)
		sessType = WmeSessionType_Video;
	else if (sType.compare("sharing") == 0)
		sessType = WmeSessionType_ScreenShare;
	return sessType;
}

wme::WmeDirection CBackdoorAgent::ofDirection(std::string sDirection)
{
    WmeDirection direction = WmeDirection_Inactive;
    if (sDirection.compare("send") == 0)
        direction = WmeDirection_Send;
    else if (sDirection.compare("recv") == 0)
        direction = WmeDirection_Recv;
    else if (sDirection.compare("sendrecv") == 0)
        direction = WmeDirection_SendRecv;
    return direction;
}

std::string CBackdoorAgent::backdoorStartStopTrack(std::string arg)
{
	Json::Reader r;
	Json::Value root;
	r.parse(arg, root, true);

	WmeSessionType sessType = ofSessionType(root["type"].asString());
	bool bStart= root["start"].asBool();
	bool bRemote = root.isMember("remote") && root["remote"].asBool();
	BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
	long ret = -77;
    
    if(sessType==WmeSessionType_ScreenShare
       && bStart){
        LoadScreenSource();
        UpdateScreenCaptureFile();
    }

	if (bLoopbackCall){
		ret = LoopbackCall::Instance()->startStopTrack(sessType, bRemote, bStart);
	}
	else{
		ret = PeerCall::Instance()->startStopTrack(sessType, bRemote, bStart);
	}
	ostringstream os;
	os << ret;
	return os.str();
}

std::string CBackdoorAgent::backdoorMuteUnMute(std::string arg)
{
	Json::Reader r;
	Json::Value root;
	r.parse(arg, root, true);

	WmeSessionType sessType = ofSessionType(root["type"].asString());

	bool bMute = root["mute"].asBool();
	bool bSpeaker = root.isMember("speaker") && root["speaker"].asBool();
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    CM_INFO_TRACE("CBackdoorAgent::backdoorMuteUnMute, loopback call="<<bLoopbackCall<<", sessType=" << sessType << ", bMute="<<bMute<<", bSpeaker"<<bSpeaker);
	if (bLoopbackCall){
		LoopbackCall::Instance()->mute(sessType, bMute, bSpeaker);
    }
	else{
		PeerCall::Instance()->mute(sessType, bMute, bSpeaker);
    }

	return "";
}

std::string CBackdoorAgent::backdoorSetManualBandwidth(std::string arg)
{
    uint32_t uBandwidth = atoi(arg.c_str()) * 1000;

    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    if (bLoopbackCall){
        LoopbackCall::Instance()->m_endCaller.setBandwidthMaually(uBandwidth);
    }
    else{
        PeerCall::Instance()->m_endCaller->setBandwidthMaually(uBandwidth);
    }
    return "";
}

std::string CBackdoorAgent::backdoorSetParam(std::string arg)
{
    std::string s_type = arg.substr(0,2);
    int paraType = atoi(s_type.c_str());
    std::string sParam = arg.substr(2);

    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
	if (bLoopbackCall){
		LoopbackCall::Instance()->m_endCaller.setParam((PARAM_TYPE_NUM)paraType, sParam.c_str());
    }
	else{
		PeerCall::Instance()->m_endCaller->setParam((PARAM_TYPE_NUM)paraType, sParam.c_str());
    }
	return "";
}

std::string CBackdoorAgent::backdoorSetAudioParam(std::string arg)
{
    Json::Reader r;
	Json::Value root;
	r.parse(arg, root, true);
	BOOL isCaller = root["caller"].asBool();
    
    Json::Value obj = root["param"];
    Json::FastWriter w;
	std::string sParam = w.write(obj);
    
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    if (bLoopbackCall){
        if(isCaller)
            LoopbackCall::Instance()->m_endCaller.setAudioParam(sParam.c_str());
            else
                LoopbackCall::Instance()->m_endCallee->setAudioParam(sParam.c_str());
                }
    else{
        PeerCall::Instance()->m_endCaller->setAudioParam(sParam.c_str());
    }
    
    return "";
}

std::string CBackdoorAgent::backdoorSetVideoParam(std::string arg)
{
    Json::Reader r;
    Json::Value root;
    r.parse(arg, root, true);
    BOOL isCaller = root["caller"].asBool();
    
    Json::Value obj = root["param"];
    Json::FastWriter w;
    std::string sParam = w.write(obj);
    
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    if (bLoopbackCall) {
        if (isCaller)
            LoopbackCall::Instance()->m_endCaller.setVideoParam(sParam.c_str());
        else
            LoopbackCall::Instance()->m_endCallee->setVideoParam(sParam.c_str());
    }
    else {
        PeerCall::Instance()->m_endCaller->setVideoParam(sParam.c_str());
    }
    
    return "";
}

std::string CBackdoorAgent::backdoorSetShareParam(std::string arg)
{
    Json::Reader r;
    Json::Value root;
    r.parse(arg, root, true);
    BOOL isCaller = root["caller"].asBool();
    
    Json::Value obj = root["param"];
    Json::FastWriter w;
    std::string sParam = w.write(obj);
    
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    if (bLoopbackCall) {
        if (isCaller)
            LoopbackCall::Instance()->m_endCaller.setScreenParam(sParam.c_str());
        else
            LoopbackCall::Instance()->m_endCallee->setScreenParam(sParam.c_str());
    }
    else {
        PeerCall::Instance()->m_endCaller->setScreenParam(sParam.c_str());
    }
    
    return "";
}


std::string CBackdoorAgent::backdoorGetParam(std::string arg)
{
	return "Failed";
}

std::string CBackdoorAgent::backdoorSetFecParam(std::string arg) {
    //if need to enable fec, call this backdoor before startcall
    Json::Reader r;
    Json::Value root;
    r.parse(arg, root, true);
    CM_INFO_TRACE("backdoorSetFecParam with param:" << arg);
    if (root.isMember("rsfec")) {
        int sessionType = root["rsfec"]["uSessionType"].asInt();
        Json::Value fecParams;
        if (WmeSessionType_Audio == sessionType && TestConfig::i().m_audioParam.isMember("fecParams")) {
            fecParams = TestConfig::i().m_audioParam["fecParams"];
            CM_INFO_TRACE("backdoorSetFecParam Audio fecParams exists, merge them");
        } else if (WmeSessionType_Video == sessionType && TestConfig::i().m_videoParam.isMember("fecParams")) {
            fecParams = TestConfig::i().m_videoParam["fecParams"];
            CM_INFO_TRACE("backdoorSetFecParam Video fecParams exists, merge them");
        } else if (WmeSessionType_ScreenShare == sessionType && TestConfig::i().m_shareParam.isMember("fecParams")) {
            fecParams = TestConfig::i().m_shareParam["fecParams"];
            CM_INFO_TRACE("backdoorSetFecParam Share fecParams exists, merge them");
        }
        if (root["rsfec"].isMember("uPayloadType")) {
            fecParams["uPayloadType"] = root["rsfec"]["uPayloadType"].asInt();
            CM_INFO_TRACE("backdoorSetFecParam SetFecParam uPayloadType");
        }
        if (root["rsfec"].isMember("uClockRate")) {
            fecParams["uClockRate"] = root["rsfec"]["uClockRate"].asInt();
            CM_INFO_TRACE("backdoorSetFecParam SetFecParam uClockRate");
        }
        if (root["rsfec"].isMember("dynamicFecScheme")) {
            fecParams["dynamicFecScheme"] = root["rsfec"]["dynamicFecScheme"].asString();
            CM_INFO_TRACE("backdoorSetFecParam SetFecParam dynamicFecScheme");
        }
        if (root["rsfec"].isMember("bFecProbing")) {
            fecParams["bFecProbing"] = root["rsfec"]["bFecProbing"].asBool();
            CM_INFO_TRACE("backdoorSetFecParam SetFecParam bFecProbing");
        }
        if (root["rsfec"].isMember("uMaxFecOverhead")) {
            fecParams["uMaxFecOverhead"] = root["rsfec"]["uMaxFecOverhead"].asInt();
            CM_INFO_TRACE("backdoorSetFecParam SetFecParam uMaxFecOverhead");
        }
        if (root["rsfec"].isMember("bEnableFec")) {
            fecParams["bEnableFec"] = root["rsfec"]["bEnableFec"].asBool();
            CM_INFO_TRACE("backdoorSetFecParam SetFecParam bEnableFec");
        }
        if (root["rsfec"].isMember("fecFmtInfo") && root["rsfec"]["fecFmtInfo"].isMember("srtpFecOrder")) {
            Json::Value fmtInfo;
            fmtInfo["srtpFecOrder"] = root["rsfec"]["fecFmtInfo"]["srtpFecOrder"].asCString();
            fecParams["fecFmtInfo"] = fmtInfo;
            CM_INFO_TRACE("backdoorSetFecParam SetFecParam srtpFecOrder");
        }
        if (WmeSessionType_Audio == sessionType) {
            TestConfig::i().m_audioParam["fecParams"] = fecParams;
        } else if (WmeSessionType_Video == sessionType) {
            TestConfig::i().m_videoParam["fecParams"] = fecParams;
        } else if (WmeSessionType_ScreenShare == sessionType) {
            TestConfig::i().m_shareParam["fecParams"] = fecParams;
        }
    }
    
    return "";
}

std::string CBackdoorAgent::backdoorSetFeatureToggles(std::string arg)
{
    CM_INFO_TRACE("backdoorSetFeatureToggles with param:" << arg);
    TestConfig::i().m_bSetFeatureToggles = true;
    TestConfig::i().m_strFeatureToggles = arg;
    return "";
}

std::string CBackdoorAgent::backdoorGetCSIChangeHistory(std::string arg)
{
    std::string sRet;
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    if (bLoopbackCall){
        sRet = LoopbackCall::Instance()->m_endCaller.getCalabash()->GetCSIChangeHistory();
    }
    else{
        sRet = PeerCall::Instance()->m_endCaller->getCalabash()->GetCSIChangeHistory();
    }
    return sRet;
}

std::string CBackdoorAgent::backdoorGetCSICount(std::string arg)
{
    int count = 0;
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    if (bLoopbackCall){
        count = LoopbackCall::Instance()->m_endCaller.getCSICount();
    }
    else{
        count = PeerCall::Instance()->m_endCaller->getCSICount();
    }
    char strCount[16] = {0};
    snprintf(strCount, sizeof(strCount), "%d", count);
    return strCount;
}

std::string CBackdoorAgent::backdoorGetMediaStatus(std::string arg)
{
    WmeSessionType mediaType = ofSessionType(arg);
    WmeMediaStatus status = WmeMedia_Available;
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    if (bLoopbackCall){
        status = LoopbackCall::Instance()->m_endCaller.GetMediaStatus(mediaType);
    }
    else{
        status = PeerCall::Instance()->m_endCaller->GetMediaStatus(mediaType);
    }
    std::string strRet = (status == WmeMedia_ERR_TEMP_UNAVAIL_NO_MEDIA)? "unavailable" : "available";
    return strRet;
}

std::string CBackdoorAgent::backdoorGetVideoCSI(std::string arg)
{
    CM_INFO_TRACE("CBackdoorAgent::backdoorGetVideoCSI, arg=" << arg.c_str());
    WmeDirection direction = ofDirection(arg);
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    unsigned int csi = 0;
    if (bLoopbackCall){
        csi = LoopbackCall::Instance()->m_endCaller.getVideoCSI(direction);
    }
    else{
        csi = PeerCall::Instance()->m_endCaller->getVideoCSI(direction);
    }
    char strCsi[255] = {0};
    snprintf(strCsi, sizeof(strCsi), "%u", csi);
    return strCsi;
}

std::string CBackdoorAgent::backdoorGetMemory(std::string arg)
{
    WmeMemoryUsage memory = {0};
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    if (bLoopbackCall){
        LoopbackCall::Instance()->m_endCaller.GetMemoryUsage(memory);
    }
    else{
        PeerCall::Instance()->m_endCaller->GetMemoryUsage(memory);
    }
    
    Json::Value obj(Json::objectValue);
    obj["fMemroyUsage"] = memory.fMemroyUsage;
    obj["uMemoryUsed"] = memory.uMemoryUsed;
    obj["uMemoryTotal"] = memory.uMemoryTotal;
    obj["uProcessMemroyUsed"] = memory.uProcessMemroyUsed;
    
    Json::FastWriter w;
    string sResult = w.write(obj);
    return sResult;
}
std::string CBackdoorAgent::backdoorGetVideoParameters(std::string arg)
{
    std::string result;
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
	if (bLoopbackCall){
		result = LoopbackCall::Instance()->m_endCaller.getParameters(PARAM_VIDEO);
    }
	else{
		result = PeerCall::Instance()->m_endCaller->getParameters(PARAM_VIDEO);
    }
    Json::Reader r;
    Json::Value root;
    r.parse(result.c_str(), root, true);
    Json::Value obj = root["video"];
    std::string ret = obj[arg.c_str()].asString();
    return ret;
}

std::string CBackdoorAgent::backdoorGetAudioParameters(std::string arg)
{
    std::string result;
    BOOL bLoopbackCall = !PeerCall::Instance()->isActive();
    if (bLoopbackCall) {
        result = LoopbackCall::Instance()->m_endCaller.getParameters(PARAM_AUDIO);
    }
    else {
        result = PeerCall::Instance()->m_endCaller->getParameters(PARAM_AUDIO);
    }
    Json::Reader r;
    Json::Value  root;
    r.parse(result.c_str(), root, true);
    Json::Value obj = root["audio"];
    std::string ret = obj[arg.c_str()].asString();
    return ret;
}
//=========================================================================
//Screen sharing
/*
std::string  CBackdoorAgent::backdoorSetSharingContext(std::string arg){
	CM_INFO_TRACE("[ScreenShare]CBackdoorAgent::backdoorSetSharingContext" << arg);
	Json::Reader r;
	Json::Value root;
	r.parse(arg, root, true);

	if (root.isMember("video")) TestConfig::i().m_bHasAudio = root["video"].asBool();
    if (root.isMember("audio")) TestConfig::i().m_bHasVideo = root["audio"].asBool();
    if (root.isMember("loopback")) TestConfig::i().m_bLoopback = root["loopback"].asBool();
    if (root.isMember("calliope")) TestConfig::i().m_bCalliope = root["calliope"].asBool();
//    if (root.isMember("share_fake_mline"))  TestConfig::i().m_bFakeVideoByShare = root["share_fake_mline"].asBool();
    
    if (root.isMember("linus"))  TestConfig::i().m_sLinusUrl = root["linus"].asString();
    if (root.isMember("share")) {
        std::string share_role = root["share_role"].asString();
        TestConfig::i().m_bAppshare = !share_role.empty();
        TestConfig::i().m_bSharer = share_role.compare("sharer") == 0;
    }
    
    TestConfig::i().m_audioParam["supportCmulti"] = root["multilayer"].asBool();
    TestConfig::i().m_videoParam["supportCmulti"] = root["multilayer"].asBool();
    TestConfig::i().m_shareParam["supportCmulti"] = root["multilayer"].asBool();

	return "{}";
}

std::string CBackdoorAgent::backdoorStartSharing(std::string arg)
{
	CM_INFO_TRACE("[ScreenShare]CBackdoorAgent::backdoorStartSharing" << arg);
	Json::Reader r;
	Json::Value root;
	r.parse(arg, root, true);

	TestConfig::i().m_bSharer = root["sharer"].asBool();
	TestConfig::i().m_bAppshare = true;
	
	TestConfig::i().Dump();
	if (TestConfig::i().m_bLoopback){
		LoopbackCall::Instance()->startLoopback(m_winRemote, m_winLocal, m_winScreenViewer);
	}
	else{
		PeerCall::Instance()->startPeer(m_winRemote, m_winLocal, m_winScreenViewer);
		PeerCall::Instance()->connect(TestConfig::i().m_sWSUrl.c_str());
	}

	return "{}";
}*/

std::string  CBackdoorAgent::backdoorStopSharing(std::string arg)
{
	CM_INFO_TRACE("[ScreenShare]CBackdoorAgent::backdoorStopSharing" << arg);
	Json::Reader r;
	Json::Value root;
	r.parse(arg, root, true);

	bool bLoopback = root["loopback"].asBool();
	if (bLoopback){
		LoopbackCall::Instance()->stopLoopback();
	}
	else
		PeerCall::Instance()->stopPeer();
	return "{}";
}
std::string CBackdoorAgent::backdoorGetSharingStatus(std::string arg)
{
	CM_INFO_TRACE("[ScreenShare]CBackdoorAgent::backdoorGetSharingStatus"<<arg);
	Json::Reader r;
	Json::Value root;
	r.parse(arg, root, true);

	std::string strAction = root["action"].asString();
	if (strAction == "ShareSharingRuningStatus"){
		return _ScreenSharingRuningStatus();
	}
	return "{}";
}

std::string CBackdoorAgent::backdoorGetSharingResult(std::string arg)
{
	CM_INFO_TRACE("[ScreenShare]CBackdoorAgent::backdoorGetSharingResult"<<arg);

	WmeScreenConnectionStatistics screenStats = { 0 };
	if (PeerCall::Instance()->isActive()) {
		PeerCall::Instance()->m_endCaller->getScreenStatistics(screenStats);
	}
	else{
		LoopbackCall::Instance()->getScreenStatistics(screenStats);
	}


	Json::Reader r;
	Json::Value root;
	r.parse(arg, root, true);

	Json::Value ret_root(Json::objectValue);

	std::string strAction = root["action"].asString();
	std::string strRoleEnd = root["what_role"].asString();
	if (strAction == "ShareScreenLastResult"){
		//Common
		Json::Value screenCommon(Json::objectValue);
		screenCommon["screenConn"] = GetStatsConnInfo(screenStats.connStat);
		screenCommon["screenInNet"] = GetStatsNetworkInfo(screenStats.sessStat.stInNetworkStat);
		screenCommon["screenOutNet"] = GetStatsNetworkInfo(screenStats.sessStat.stOutNetworkStat);
		ret_root["common"] = screenCommon;

		//collect result statistic for sharer end 
		if (strRoleEnd == "both" || strRoleEnd == "sharer"){
			Json::Value screencapturer(Json::objectValue);
			screencapturer["average_captured_time"] = screenStats.localScreenStat.stScreenShareCaptureStatistics.m_nMsAverageCaptureTime;
			screencapturer["total_captured_frames"] = screenStats.localScreenStat.stScreenShareCaptureStatistics.m_nTotalCapturedFrames;

			Json::Value sharer(Json::objectValue);
			sharer["video"] = GetStatsVideoInfo(screenStats.localScreenStat.stVideoStat);
			sharer["capturer"] = screencapturer;
			ret_root["sharer"] = sharer;
		}

		//collect result statistic for viewer end 
		if (strRoleEnd == "both" || strRoleEnd == "viewer"){
			
			Json::Value viewer(Json::objectValue);
			viewer["video"] = GetStatsVideoInfo(screenStats.remoteScreenStat.stVideoStat);
			ret_root["viewer"] = viewer;
		}
	}

	Json::FastWriter w;
	string sResult = w.write(ret_root);
	return sResult;
}

/*
std::string CBackdoorAgent::backdoorAddOneResource(std::string arg){
	CM_INFO_TRACE("[ScreenShare]CBackdoorAgent::backdoorAddOneResource" << arg);
	Json::Reader r;
	Json::Value root;
	r.parse(arg, root, true);
	TestConfig::i().m_bScreenTrackViewAnalyzer = true;
	//add one default resource
	IWmeScreenSource* pIWmeScreenSource = NULL;
	if (root["source_type"].asString() == "app"){
		if (root["source_name"].asString() != ""){
			//enhence it in future, how identify differnt source ?
		}
		else{
			pIWmeScreenSource = CASEnum::Instance()->GetOneDefaultScreenSource(WmeScreenSourceTypeApplication);
		}
	}
	else
		pIWmeScreenSource = CASEnum::Instance()->GetOneDefaultScreenSource(WmeScreenSourceTypeDesktop);

	if (pIWmeScreenSource){
		if (TestConfig::i().m_bLoopback){
			LoopbackCall::Instance()->m_endCaller.addScreenSouce(pIWmeScreenSource);
		}
		else{
			if (PeerCall::Instance()->m_endCaller)
				PeerCall::Instance()->m_endCaller->addScreenSouce(pIWmeScreenSource);
		}
	}

	return"{}";
}*/
/*
	{
		"view_x": 0,
		"view_y" : 0,
		"view_w" : 500,
		"view_h" : 400,
		"qrcode_grid_row" : 2,
		"qrcode_grid_col" : 2,
	}
*/
std::string CBackdoorAgent::backdoorSetQRCodeContext(std::string arg)
{	
	//Specific Loigc
	CScreenTrackViewAnalyzer * pCScreenTrackViewAnalyzer = NULL; 
	if (TestConfig::i().m_bLoopback){
		if (LoopbackCall::Instance()->m_endCallee)
			pCScreenTrackViewAnalyzer = LoopbackCall::Instance()->m_endCallee->getCScreenTrackViewAnalyzer();
	}
	else {
		if(PeerCall::Instance()->m_endCaller)
			pCScreenTrackViewAnalyzer = PeerCall::Instance()->m_endCaller->getCScreenTrackViewAnalyzer();
	}

	if (pCScreenTrackViewAnalyzer != NULL){
		pCScreenTrackViewAnalyzer->SetQRCodeContextWithJson(arg);
	}
	else
		CM_ERROR_TRACE("backdoorShowQRCodeView, pCScreenTrackViewAnalyzer=NULL");

	//generate json result
	Json::Value ret(Json::objectValue);
	ret["ret"] = "true";

	Json::FastWriter w;
	std::string sRet = w.write(ret);
	CM_INFO_TRACE("backdoorShowQRCodeView, result=" << sRet);
	return sRet;
}
std::string CBackdoorAgent::backdoorCollectReceivedQRCodeContents(std::string arg)
{
	//pass input arg
	Json::Reader r;
	Json::Value input;
	r.parse(arg, input, true);

	/*
	std::string strQRCodeContentList = "{\"qrcode_contents\":\
		[\"1.1\",\"1.2\",\"1.3\",\"1.4\",\"1.5\",\"1.6\",\"1.7\",\"1.8\" \
		,\"2.1\",\"2.2\",\"2.3\",\"2.4\",\"2.5\",\"2.6\",\"2.7\",\"2.8\" \
		,\"3.1\",\"3.2\",\"3.3\",\"3.4\",\"3.5\",\"3.6\",\"3.7\",\"3.8\" \
		,\"4.1\",\"4.2\",\"4.3\",\"4.4\",\"4.5\",\"4.6\",\"4.7\",\"4.8\" \
						]}";	
	*/

	string strQRCodeContentList = "{ \"qrcode_contents\":[] }";
	CScreenTrackViewAnalyzer * pCScreenTrackViewAnalyzer = NULL;
	if (TestConfig::i().m_bLoopback){
		if (LoopbackCall::Instance()->m_endCallee)
			pCScreenTrackViewAnalyzer = LoopbackCall::Instance()->m_endCallee->getCScreenTrackViewAnalyzer();
	}
	else {
		if (PeerCall::Instance()->m_endCaller)
			pCScreenTrackViewAnalyzer = PeerCall::Instance()->m_endCaller->getCScreenTrackViewAnalyzer();
	}

	if (pCScreenTrackViewAnalyzer != NULL){
		//collect QRCode contents list.
		std::vector<std::string> vec_qr_codes = pCScreenTrackViewAnalyzer->GetQRList();
		std::vector<std::string>::iterator it;
		strQRCodeContentList = "{\"qrcode_contents\":[";
		for (it = vec_qr_codes.begin(); it != vec_qr_codes.end(); it++){
			std::string strOneQRCode = *it;
			CM_INFO_TRACE("backdoorCollectReceivedQRCodeContents, oneQRCode=" << strOneQRCode);
			if (it != vec_qr_codes.begin())
				strQRCodeContentList += ",";
			strQRCodeContentList += "\"";
			strQRCodeContentList += *it; 
			strQRCodeContentList += "\"";
		}
		strQRCodeContentList += "]}";
		//cleanup QRCode contents list.
		pCScreenTrackViewAnalyzer->CleanQRList();
	}
	else
		CM_ERROR_TRACE("backdoorCollectReceivedQRCodeContents, pCScreenTrackViewAnalyzer=NULL");
	CM_INFO_TRACE("backdoorCollectReceivedQRCodeContents, All QRCodes =" << strQRCodeContentList);
	//generate json result
	Json::Value ret(Json::objectValue);
	ret["ret"] = "true";
	Json::Value qrcode_contents;
	{
		Json::Reader r;
		r.parse(strQRCodeContentList, qrcode_contents, true);
		qrcode_contents["qrcode_contents"].isArray();
	}
	//qrcode_contents.append("");
	ret["qrcode_contents"] = qrcode_contents["qrcode_contents"];

	Json::FastWriter w;
	std::string sRet = w.write(ret);
	CM_INFO_TRACE("backdoorCollectReceivedQRCodeContents, result=" << sRet);
	return sRet;
}

std::string CBackdoorAgent::_ScreenSharingRuningStatus()
{
	Json::Value ret_root;

	//check current sharing status , it is "start" or "end"
	//need to refactoring EndPoint to support specific 
	ret_root["ShareSharingRuningStatus"] = Json::Value("end");

	Json::FastWriter w;
	string sResult = w.write(ret_root);
	return sResult;
}

std::string CBackdoorAgent::backdoorCheckSyncStatus(std::string arg)
{
    CM_INFO_TRACE("CBackdoorAgent::backdoorCheckSyncStatus, arg:" << arg);
    bool ret = false;
    
    Json::Reader r;
	Json::Value root;
	r.parse(arg, root, true);
    
    std::string result = root["result"].asString();
    int rate = root["rate"].asInt();
    
    if (TestConfig::i().m_bLoopback) {
        if(LoopbackCall::Instance()->m_endCallee) {
            ret = LoopbackCall::Instance()->m_endCallee->checkSyncStatus(result.c_str(), rate);
        }
    }
    else {
        if (PeerCall::Instance()->m_endCaller){
            ret = PeerCall::Instance()->m_endCaller->checkSyncStatus(result.c_str(), rate);
        }
    }
    
	return ret?"pass":"false";
}

std::string CBackdoorAgent::backdoorUpdateSdp(std::string arg)
{
    CM_INFO_TRACE("backdoorUpdateSdp with param:" << arg);
    Json::Reader r;
	Json::Value root;
	r.parse(arg, root, true);
    std::string op = root["op"].asString();
    if(op == "hold" || op == "unhold")
    {
        bool hold = (op == "hold");
        if (TestConfig::i().m_bLoopback){
            LoopbackCall::Instance()->m_endCaller.holdCall(hold);
        }
        else if (PeerCall::Instance()->m_endCaller){
            PeerCall::Instance()->m_endCaller->holdCall(hold);
        }
    }
    
	return "";
}

std::string CBackdoorAgent::backdoorSetRemoteOffer(std::string arg)
{
    if (PeerCall::Instance()->m_endCaller){
		PeerCall::Instance()->m_endCaller->acceptCall(arg.c_str(), true);
		return "";
	}
    
	return "Failed";
}


std::string CBackdoorAgent::backdoorSetScreenFileCaptureFilePath(std::string arg){
    CM_INFO_TRACE("backdoorSetScreenFileCaptureFilePath with param:" << arg);
    Json::Reader r;
    Json::Value root;
    r.parse(arg, root, true);
    if(root["sharesource_file"].asString().size()>0)
        TestConfig::i().m_strScreenSharingCaptureFile = ExpandPath("sample/"+ root["sharesource_file"].asString());
    UpdateScreenCaptureFile();
    
    return "";

}

void CBackdoorAgent::UpdateScreenCaptureFile(){
    if (TestConfig::i().m_bLoopback){
        LoopbackCall::Instance()->m_endCaller.updateScreenFileCaptureFileName();
    }
    else{
        if (PeerCall::Instance()->m_endCaller)
            PeerCall::Instance()->m_endCaller->updateScreenFileCaptureFileName();
    }
}
std::string CBackdoorAgent::backdoorScreenChangeCaptureFps(std::string arg){
    CM_INFO_TRACE("backdoorScreenChangeCaptureFps with param:" << arg);
    Json::Reader r;
    Json::Value root;
    r.parse(arg, root, true);
    int  nScreenCaptureFps = root["screenCaptureFps"].asInt();
    TestConfig::i().m_nMaxScreenCaptureFps = nScreenCaptureFps;
    if (TestConfig::i().m_bLoopback){
        LoopbackCall::Instance()->m_endCaller.updateScreenCaptureMacFps();
    }
    else{
        if (PeerCall::Instance()->m_endCaller)
            PeerCall::Instance()->m_endCaller->updateScreenCaptureMacFps();
    }
    
    return "";
}

std::string CBackdoorAgent::backdoorOutputTrace(std::string arg){
    CM_INFO_TRACE("backdoorOutputTrace with param:" << arg);
    Json::Reader r;
    Json::Value root;
    r.parse(arg, root, true);
    std::string strTraceInfoLog = root["trace_info"].asString();
    CM_INFO_TRACE("TA case Trace:" << strTraceInfoLog
                  << ",DeviceModelName=" << TestConfig::i().m_strDeviceModelName
                    << ",SystemOSVersion=" << TestConfig::i().m_strSystemOSVersion);
    return "";
}
std::string CBackdoorAgent::backdoorRequestFloor(std::string arg)
{
    CM_INFO_TRACE("backdoorRequestFloor with param:" << arg);
    Json::Reader r;
    Json::Value root;
    r.parse(arg, root, true);
    bool bRequestFloor = root["bRequestFloor"].asBool();
    
    if (TestConfig::Instance().m_bLoopback)
    {
        if(bRequestFloor)
            LoopbackCall::Instance()->requestFloor();
        else
            LoopbackCall::Instance()->releaseFloor();
    }
    else
    {
        if(bRequestFloor)
            PeerCall::Instance()->requestFloor();
        else
            PeerCall::Instance()->releaseFloor();
    }
    
    return "";
}
std::string CBackdoorAgent::backdoorSubscribe(std::string arg)
{
    CM_INFO_TRACE("backdoorSubscribe with param:" << arg);
    Json::Reader r;
    Json::Value root;
    r.parse(arg, root, true);
    int resolution = root["resolution"].asInt();
    bool newTrack = root["newTrack"].asBool();
    bool unsubscribe = root["unsubscribe"].asBool();
    if(newTrack && m_sink)
    {
        TestConfig::Instance().m_uMaxVideoStreams++;
        m_sink->onCheckMultiVideo();
    }
    if (TestConfig::i().m_bLoopback){
        if(LoopbackCall::Instance()->m_endCallee){
            if(unsubscribe)
                LoopbackCall::Instance()->m_endCallee->unsubscribeVideo(1);
            else
                LoopbackCall::Instance()->m_endCallee->subscribeVideo((WmeVideoQualityType)resolution, newTrack, WmePolicy_ActiveSpeaker, 0);
        }
    }
    else if (PeerCall::Instance()->m_endCaller){
        if(unsubscribe)
            PeerCall::Instance()->m_endCaller->unsubscribeVideo(1);
        else
            PeerCall::Instance()->m_endCaller->subscribeVideo((WmeVideoQualityType)resolution, newTrack, WmePolicy_ActiveSpeaker, 0);
    }
    
    return "";
}
std::string CBackdoorAgent::backdoorRequestVideo(std::string arg)
{
    CM_INFO_TRACE("backdoorRequestVideo with param:" << arg);
    Json::Reader r;
    Json::Value obj;
    r.parse(arg, obj, true);
    
    if (obj.isMember("scr"))//only for TA
    {
        Json::Value scrs = obj["scr"];
        int vid = 0; // the first remotetrack
        uint32_t uBitrate = 15000 * 1000;
        uint8_t priority = 255;
        uint8_t grouping = 0;
        bool duplicate = true;
        
        if (TestConfig::i().m_bLoopback) {
            for (int scrIdx=0; scrIdx<LoopbackCall::Instance()->m_endCallee->m_pVideoTrackCallee.size(); scrIdx++) {
                if (scrIdx<scrs.size()) {
                    
                    Json::Value scr = scrs[scrIdx];
                    if (scr.isMember("br"))
                        uBitrate = scr["br"].asInt();
                    if (scr.isMember("priority"))
                        priority = scr["priority"].asInt();
                    if (scr.isMember("grouping"))
                        grouping = scr["grouping"].asInt();
                    if (scr.isMember("duplicate"))
                        duplicate = scr["duplicate"].asBool();
                    
                    priority = 255-scrIdx;
                    grouping = scrIdx;
                    
                    if (scr.isMember("mbps") && scr.isMember("fs") && scr.isMember("fps") && scr.isMember("dpb")){
                        if(LoopbackCall::Instance()->m_endCallee && LoopbackCall::Instance()->m_endCallee->m_pVideoTrackCallee.size()>scrIdx){
                            LoopbackCall::Instance()->m_endCallee->m_pVideoTrackCallee[scrIdx]->RequestVideo(scr["fs"].asInt(), scr["fps"].asInt(), uBitrate, scr["dpb"].asInt(), scr["mbps"].asInt(), priority, grouping, duplicate);
                        }
                    }
                }
                else{
                    if(LoopbackCall::Instance()->m_endCallee){
                        LoopbackCall::Instance()->m_endCallee->m_pVideoTrackCallee[scrIdx]->Mute();
                    }
                }
            }
        } else {
            for (int scrIdx=0; scrIdx<scrs.size(); scrIdx++) {
                
                Json::Value scr = scrs[scrIdx];
                if (scr.isMember("br"))
                    uBitrate = scr["br"].asInt();
                if (scr.isMember("priority"))
                    priority = scr["priority"].asInt();
                if (scr.isMember("grouping"))
                    grouping = scr["grouping"].asInt();
                if (scr.isMember("duplicate"))
                    duplicate = scr["duplicate"].asBool();
                
                priority = 255-scrIdx;
                grouping = scrIdx;
                
                if (scr.isMember("mbps") && scr.isMember("fs") && scr.isMember("fps") && scr.isMember("dpb")){
                    if (PeerCall::Instance()->m_endCaller && PeerCall::Instance()->m_endCaller->m_pVideoTrackCallee.size()>scrIdx){
                        PeerCall::Instance()->m_endCaller->m_pVideoTrackCallee[scrIdx]->RequestVideo(scr["fs"].asInt(), scr["fps"].asInt(), uBitrate, scr["dpb"].asInt(), scr["mbps"].asInt(), priority, grouping, duplicate);
                    }
                }
                
            }
        }
        
    }

    return "";

}
std::string CBackdoorAgent::backdoorOverridePerfJson(std::string arg)
{
    TestConfig::i().m_sOverrideDeviceSetting = arg;
    
    return "";
}


std::string CBackdoorAgent::backdoorGetVoiceLevel(std::string arg)
{
    bool bSpk = false;
    if (arg == "remote" || arg == "speaker")
        bSpk = true;
    if (TestConfig::i().m_bLoopback){
        return std::to_string(LoopbackCall::Instance()->getVoiceLevel(bSpk));
    }
    return std::to_string(PeerCall::Instance()->getVoiceLevel(bSpk));
}

std::string CBackdoorAgent::backdoorGetCPUCores(std::string arg) {
    WmeCpuUsage cpuUsage = { 0 };
    PeerCall::Instance()->m_endCaller->GetCpuUsage(cpuUsage);
    int nCores = cpuUsage.nCores;
    
    return std::to_string(nCores);
}

//stuntrace
std::string CBackdoorAgent::backdoorGetStunTraceResult(std::string arg)
{
	std::string ret;

#if defined(AT_MAC) || defined (AT_IOS) || defined (AT_ANDROID) || defined(_WIN32)
    CTaIWmeStunTraceSink *pSink = CTaIWmeStunTraceSink::GetInstance();
    if (pSink->getStunTraceRet() == -1) {
        ret = "failed";
    }
    else
    {
        ret = pSink->getJsonRet();
    }    
#endif

	return ret;
}

//stuntrace
std::string CBackdoorAgent::backdoorGetTraceServerResult(std::string arg)
{
    std::string ret;
    
#if !defined(WP8)
    CTaIWmeTraceServerSink *pSink = CTaIWmeTraceServerSink::GetInstance();
    if (pSink->getTraceServerRet() == -1) {
        ret = "failed";
    }
    else
    {
        ret = pSink->getJsonRet();
    }
#endif
    
    return ret;
}

std::string CBackdoorAgent::backdoorChangeLocalTrack(std::string arg) {
    if(arg == "camera") {
        if (TestConfig::Instance().m_bLoopback)
        {
            LoopbackCall::Instance()->setCamera(NULL);
        }
        else
        {
            PeerCall::Instance()->setCamera(NULL);
        }
    } else if (arg == "svs"){
        if (TestConfig::Instance().m_bLoopback)
        {
            LoopbackCall::Instance()->startSVS();
        }
        else
        {
            PeerCall::Instance()->startSVS();
        }
    }

    return "";
}

std::string CBackdoorAgent::backdoorGetAssertionsCount(std::string arg) {
    return std::to_string(cm_get_assertions_count());
}
    
std::string CBackdoorAgent::backdoorQuitWP8(std::string arg) {
	if (m_sink)
	{
		m_sink->onQuitWP8();
	}


    return "";
}

#if defined (AT_IOS)
extern int GetiOSDocumentPath(char *pPath, int &uPathLen);
void UpdateSourcePath(std::string& strConfigPath)
{
    char szPath[256];
    int nLen = 256;
    GetiOSDocumentPath(szPath,nLen);
    std::string strPath = szPath;
    std::string strSearch = "/Documents";
    std::string::size_type i = strPath.find(strSearch);
    if (i != std::string::npos)
        strPath.erase(i, strSearch.length());
    strConfigPath= strPath + strConfigPath;
}
#endif

#if defined(_WIN32)
    #ifndef WP8 
        #define setenv(name,value,boverwrite) _putenv_s(name,value)
    #else
        #define setenv(name,value,boverwrite) 
    #endif
#endif
std::string CBackdoorAgent::backdoorConfig(std::string arg){
    CM_INFO_TRACE("backdoorConfig with param:" << arg);
    Json::Reader r;
    Json::Value root;
    r.parse(arg, root, true);
    if (root.isMember("video_config")) {
        Json::Value video_config = root["video_config"];
        if(video_config.isMember("video_capture_feed_file")){
            std::string  video_capture_feed_file = video_config["video_capture_feed_file"].asString();
            if(video_capture_feed_file.size()>0){
#if defined (AT_IOS)
                UpdateSourcePath(video_capture_feed_file);

#endif
                setenv("WME_PERFORMANCE_TEST_FeedVideoCaptureData_file",video_capture_feed_file.c_str(),1);
            }
        }
        
        if(video_config.isMember("video_size")){
            std::string  video_size = video_config["video_size"].asString();
            if(video_size.size()>0){
                setenv("WME_PERFORMANCE_TEST_VIDEO_SIZE",video_size.c_str(),1);
            }
        }
    }
    
    if (root.isMember("audio_config")) {
        Json::Value audio_config = root["audio_config"];
        if(audio_config.isMember("audio_capture_feed_file")){
            std::string  audio_capture_feed_file = audio_config["audio_capture_feed_file"].asString();
            if(audio_capture_feed_file.size()>0){
#if defined (AT_IOS)
                UpdateSourcePath(audio_capture_feed_file);
#endif
                setenv("WME_PERFORMANCE_TEST_FeedSource_Enable","1",1);
                setenv("WME_PERFORMANCE_TEST_FeedSource_Format","0",1);
                setenv("WME_PERFORMANCE_TEST_CaptureReplaceFileName",audio_capture_feed_file.c_str(),1);
                setenv("WME_PERFORMANCE_TEST_CaptureReplaceCircleEnable","1",1);
            }
        }
    }
    return "";
}
