#include "StatsInfo.h"
#include "ClickCall.h"
#include "PeerCall.h"
#include "loopback.h"
#include "math.h"

StatsInfo::StatsInfo()
{
    CM_INFO_TRACE_THIS("StatsInfo::StatsInfo");
}

StatsInfo::~StatsInfo()
{
    CM_INFO_TRACE_THIS("StatsInfo::~StatsInfo");
}

void StatsInfo::SetSink(StatsInfoSink *sink)
{
    m_sink = sink;
}

void StatsInfo::Start()
{
    m_Timer.ScheduleInThread(TT_MAIN, this, CCmTimeValue(1, 0));
}

void StatsInfo::Stop()
{
    m_Timer.Cancel();
}

std::string StatsInfo::GetConnectionType(wme::WmeTransportType type)
{
    switch (type) {
        case WmeTransportType_TCP:
            return "TCP";
        case WmeTransportType_UDP:
            return "UDP";
        default:
            return "NONE";
    }
}
string StatsInfo::formatStatisticsAudio(WmeAudioConnectionStatistics stStat)
{
    std::ostringstream  retStr;
    retStr.setf(ios::fixed);
    streamsize precisionBackup = retStr.precision(2);
    
    retStr << "AudioStatistics("<<GetConnectionType(stStat.connStat.connectionType)<<"): ";
    
    retStr << "Sent(" << stStat.connStat.uLocalPort << "): ";
    retStr << "RTT=" << stStat.sessStat.stOutNetworkStat.uRoundTripTime << ", ";
    retStr << "Bytes=" << stStat.sessStat.stOutNetworkStat.uBytes << ", Packets=" << stStat.sessStat.stOutNetworkStat.uPackets << ", ";
    retStr << "LossRatio=" << stStat.sessStat.stOutNetworkStat.fLossRatio << ", LossPackets=" << stStat.sessStat.stOutNetworkStat.uLostPackets << ", ";
    retStr << "Jitter=" << stStat.sessStat.stOutNetworkStat.uJitter << ", ";
    retStr << "Bitrate=" << stStat.localAudioStat.uBitRate * 8 << "; ";
    retStr << "Received(" << stStat.connStat.uRemotePort << "): ";
    retStr << "Bytes=" << stStat.sessStat.stInNetworkStat.uBytes << ", Packets=" << stStat.sessStat.stInNetworkStat.uPackets << ", ";
    retStr << "LossRatio=" << stStat.sessStat.stInNetworkStat.fLossRatio << ", LossPackets=" << stStat.sessStat.stInNetworkStat.uLostPackets << ", ";
    retStr << "Jitter=" << stStat.sessStat.stInNetworkStat.uJitter << ", ";
    retStr << "Bitrate=" << stStat.remoteAudioStat.uBitRate * 8;
    
    retStr.precision(precisionBackup);
    retStr.unsetf(ios::floatfield);
    
    return retStr.str();
}

string StatsInfo::formatStatisticsVideo(WmeVideoConnectionStatistics stStat)
{
    std::ostringstream  retStr;
    retStr.setf(ios::fixed);
    streamsize precisionBackup = retStr.precision(2);
    
    retStr << "VideoStatistics("<<GetConnectionType(stStat.connStat.connectionType)<<"): ";
    
    retStr << "Sent(" << stStat.connStat.uLocalPort << "): ";
    retStr << "RTT=" << stStat.sessStat.stOutNetworkStat.uRoundTripTime << ", ";
    retStr << "Bytes=" << stStat.sessStat.stOutNetworkStat.uBytes << ", Packets=" << stStat.sessStat.stOutNetworkStat.uPackets << ", ";
    retStr << "LossRatio=" << stStat.sessStat.stOutNetworkStat.fLossRatio << ", LossPackets=" << stStat.sessStat.stOutNetworkStat.uLostPackets << ", ";
    retStr << "Jitter=" << stStat.sessStat.stOutNetworkStat.uJitter << ", ";
    retStr << "Size:" << stStat.localVideoStat.uWidth << "x" << stStat.localVideoStat.uHeight << ", ";
    retStr << "Fps=" << (double)round(stStat.localVideoStat.fFrameRate*100)/100 << ", ";
    retStr << "Bitrate=" << static_cast<unsigned int>(stStat.localVideoStat.fBitRate) << "; ";
    retStr << "Received(" << stStat.connStat.uRemotePort << "): ";
    retStr << "Bytes=" << stStat.sessStat.stInNetworkStat.uBytes << ", Packets=" << stStat.sessStat.stInNetworkStat.uPackets << ", ";
    retStr << "LossRatio=" << stStat.sessStat.stInNetworkStat.fLossRatio << ", LossPackets=" << stStat.sessStat.stInNetworkStat.uLostPackets << ", ";
    retStr << "Jitter=" << stStat.sessStat.stInNetworkStat.uJitter << ", ";
    retStr << "Size:" << stStat.remoteVideoStat.uWidth << "x" << stStat.remoteVideoStat.uHeight << ", ";
    retStr << "Fps=" << (double)round(stStat.remoteVideoStat.fFrameRate*100)/100 << ", ";
    retStr << "Bitrate=" << static_cast<unsigned int>(stStat.remoteVideoStat.fBitRate) << ", ";
    retStr << "RenderedFrame=" << stStat.remoteVideoStat.uRenderFrameCount;
    
    retStr.precision(precisionBackup);
    retStr.unsetf(ios::floatfield);
    
    return retStr.str();
}

string StatsInfo::formatStatisticsScreen(WmeScreenConnectionStatistics stStat)
{
    std::ostringstream  retStr;
    retStr.setf(ios::fixed);
    streamsize precisionBackup = retStr.precision(2);
    
    retStr << "ScreenStatistics("<<GetConnectionType(stStat.connStat.connectionType)<<"): ";
    
    retStr << "Sent(" << stStat.connStat.uLocalPort << "): ";
    retStr << "RTT=" << stStat.sessStat.stOutNetworkStat.uRoundTripTime << ", ";
    retStr << "Bytes=" << stStat.sessStat.stOutNetworkStat.uBytes << ", Packets=" << stStat.sessStat.stOutNetworkStat.uPackets << ", FailedIOPackets=" << stStat.sessStat.stOutNetworkStat.uFailedIOPackets << ", ";
    retStr << "LossRatio=" << stStat.sessStat.stOutNetworkStat.fLossRatio << ", LossPackets=" << stStat.sessStat.stOutNetworkStat.uLostPackets << ", ";
    retStr << "Jitter=" << stStat.sessStat.stOutNetworkStat.uJitter << ", ";
    retStr << "Bitrate=" << stStat.sessStat.stOutNetworkStat.uBitRate << ", ";
    retStr << "Size:" << stStat.localScreenStat.stVideoStat.uWidth << "x" << stStat.localScreenStat.stVideoStat.uHeight << ", ";
    retStr << "Fps=" << (double)round(stStat.localScreenStat.stVideoStat.fFrameRate*100)/100 << ", ";
    retStr << "IDRRecvNum=" << stStat.localScreenStat.stVideoStat.uIDRRecvNum <<  ", ";
    retStr << "IDRReqNum=" << stStat.localScreenStat.stVideoStat.uIDRReqNum << ", ";
    retStr << "TotalCapturedFrames=" << stStat.localScreenStat.stScreenShareCaptureStatistics.m_nTotalCapturedFrames << ", ";
    retStr << "AverageCaptureTime=" << stStat.localScreenStat.stScreenShareCaptureStatistics.m_nMsAverageCaptureTime << "; ";
    
    retStr << "Received(" << stStat.connStat.uRemotePort << "): ";
    retStr << "Bytes=" << stStat.sessStat.stInNetworkStat.uBytes << ", Packets=" << stStat.sessStat.stInNetworkStat.uPackets << ", FailedIOPackets=" << stStat.sessStat.stInNetworkStat.uFailedIOPackets << ", ";
    retStr << "LossRatio=" << stStat.sessStat.stInNetworkStat.fLossRatio << ", LossPackets=" << stStat.sessStat.stInNetworkStat.uLostPackets << ", ";
    retStr << "Jitter=" << stStat.sessStat.stInNetworkStat.uJitter << ", ";
    retStr << "Bitrate=" << stStat.sessStat.stInNetworkStat.uBitRate << ", ";
    retStr << "Size:" << stStat.remoteScreenStat.stVideoStat.uWidth << "x" << stStat.remoteScreenStat.stVideoStat.uHeight << ", ";
    retStr << "Fps=" << (double)round(stStat.remoteScreenStat.stVideoStat.fFrameRate*100)/100 << ", ";
    retStr << "RenderedFrame=" << stStat.remoteScreenStat.stVideoStat.uRenderFrameCount << ", ";
    retStr << "IDRRecvNum=" << stStat.remoteScreenStat.stVideoStat.uIDRRecvNum <<  ", ";
    retStr << "IDRReqNum=" << stStat.remoteScreenStat.stVideoStat.uIDRReqNum << ", ";
    retStr << "TotalCapturedFrames=" << stStat.remoteScreenStat.stScreenShareCaptureStatistics.m_nTotalCapturedFrames << ", ";
    retStr << "AverageCaptureTime=" << stStat.remoteScreenStat.stScreenShareCaptureStatistics.m_nMsAverageCaptureTime;
    
    retStr.precision(precisionBackup);
    retStr.unsetf(ios::floatfield);
    
    return retStr.str();
}

void StatsInfo::OnTimer(CCmTimerWrapperID* aId)
{
    WmeCpuDescpription cpuDesc;
    GetCpuDescription(cpuDesc);
    WmeAudioConnectionStatistics audioStats ={0};
    WmeVideoConnectionStatistics videoStats ={0};
    WmeScreenConnectionStatistics screenStats ={0};
	WmeCpuUsage cpuUsage = { 0 };
    WmeMemoryUsage memoryUsage = {0};

    WmeNetworkIndex idxUplink = NETWORK_EXCELLENT;
    WmeNetworkIndex idxDownlink = NETWORK_EXCELLENT;
    WmeNetworkIndex idxBothlink = NETWORK_EXCELLENT;
    WmeNetworkStatus uplinkStatus;
    if (PeerCall::Instance()->isActive()) {
        PeerCall::Instance()->m_endCaller->getStatistics(audioStats, videoStats);
        PeerCall::Instance()->m_endCaller->getScreenStatistics(screenStats);
        PeerCall::Instance()->m_endCaller->getNetworkIndex(DIRECTION_UPLINK, idxUplink);
        PeerCall::Instance()->m_endCaller->getNetworkIndex(DIRECTION_DOWNLINK, idxDownlink);
        PeerCall::Instance()->m_endCaller->getNetworkIndex(DIRECTION_BOTHLINK, idxBothlink);
        PeerCall::Instance()->m_endCaller->getUplinkNetworkStatus(uplinkStatus);
		PeerCall::Instance()->m_endCaller->GetCpuUsage(cpuUsage);
        PeerCall::Instance()->m_endCaller->GetMemoryUsage(memoryUsage);
    }
    else{
        LoopbackCall::Instance()->getStatistics(audioStats, videoStats);
        LoopbackCall::Instance()->getScreenStatistics(screenStats);
        LoopbackCall::Instance()->getNetworkIndex(DIRECTION_UPLINK, idxUplink);
        LoopbackCall::Instance()->getNetworkIndex(DIRECTION_DOWNLINK, idxDownlink);
        LoopbackCall::Instance()->getNetworkIndex(DIRECTION_BOTHLINK, idxBothlink);
        LoopbackCall::Instance()->getUplinkNetworkStatus(uplinkStatus);
		if (LoopbackCall::Instance()->m_endCallee)
        {
			LoopbackCall::Instance()->m_endCallee->GetCpuUsage(cpuUsage);
            LoopbackCall::Instance()->m_endCallee->GetMemoryUsage(memoryUsage);
        }
	}
    string strAudio = formatStatisticsAudio(audioStats);
    string strVideo = formatStatisticsVideo(videoStats);
    string strScreen = formatStatisticsScreen(screenStats);
    
    std::ostringstream ostr;
	if (cpuUsage.nCores > 0){
		ostr << "proc=" << (int)cpuUsage.fProcessUsage << " total=" << (int)cpuUsage.fTotalUsage;
		for (int i = 0; i < cpuUsage.nCores; i++)
			ostr << " core" << i << "=" << (int)cpuUsage.pfCores[i];
		ostr << "\r\n";
	}
    ostr << "Process Memory:" << memoryUsage.uProcessMemroyUsed << "  Memory Used:" << memoryUsage.uMemoryUsed << "  Memory Total:" << memoryUsage.uMemoryTotal << "  Memory Usage:" << memoryUsage.fMemroyUsage << std::endl << std::endl;
    
    ostr << "Local IP:" << audioStats.connStat.localIp << "  RemoteIP:" << audioStats.connStat.remoteIp << std::endl;
   
    ostr << "Audio:" << GetConnectionType(audioStats.connStat.connectionType)<< "\r\n";
    ostr << "  local Port:" << audioStats.connStat.uLocalPort
         << "  Remote Port:" << audioStats.connStat.uRemotePort << "\r\n";
    ostr << "  RTT:" << audioStats.sessStat.stOutNetworkStat.uRoundTripTime << "\r\n";
    
    ostr << "  Sent:        ";
    ostr << "      Bytes(Packets):" << audioStats.sessStat.stOutNetworkStat.uBytes
    << "(" << audioStats.sessStat.stOutNetworkStat.uPackets  << ")";
    ostr << "  Loss:" << audioStats.sessStat.stOutNetworkStat.fLossRatio <<
    ", Jitter:" << audioStats.sessStat.stOutNetworkStat.uJitter;
    ostr << "  Bitrate:" << audioStats.localAudioStat.uBitRate << "\r\n";
    
    ostr << "  Received: ";
    ostr << "      Bytes(Packets):" << audioStats.sessStat.stInNetworkStat.uBytes <<
    "(" << audioStats.sessStat.stInNetworkStat.uPackets << ")";
    ostr << "  Loss(Count):" << audioStats.sessStat.stInNetworkStat.fLossRatio <<
    "(" << audioStats.sessStat.stInNetworkStat.uLostPackets <<
    ")  Jitter:" << audioStats.sessStat.stInNetworkStat.uJitter;
    ostr << "  Bitrate:" << audioStats.remoteAudioStat.uBitRate << "\r\n\r\n";
    
    ostr << "Video:" << GetConnectionType(videoStats.connStat.connectionType)<< "\r\n";
    ostr << "  local Port:" << videoStats.connStat.uLocalPort
         << "  Remote Port:" << videoStats.connStat.uRemotePort << "\r\n";
    ostr << "  RTT:" << videoStats.sessStat.stOutNetworkStat.uRoundTripTime << "\r\n";

    ostr << "  Sent:        ";
    ostr << "      Bytes(Packets):" << videoStats.sessStat.stOutNetworkStat.uBytes <<
    "(" << videoStats.sessStat.stOutNetworkStat.uPackets << ")";
    ostr << "  Loss:" << videoStats.sessStat.stOutNetworkStat.fLossRatio <<
    ", Jitter:" << videoStats.sessStat.stOutNetworkStat.uJitter;
    ostr << "  Size:" << videoStats.localVideoStat.uWidth << "x" <<
    videoStats.localVideoStat.uHeight << " (" << videoStats.localVideoStat.fFrameRate << ")";
    ostr <<  "  Bitrate:" << videoStats.localVideoStat.fBitRate << "\r\n";
    
    ostr << "  Received: ";
    ostr << "      Bytes(Packets):" << videoStats.sessStat.stInNetworkStat.uBytes <<
    "(" << videoStats.sessStat.stInNetworkStat.uPackets << ")";
    ostr << "  Loss(Count):" << videoStats.sessStat.stInNetworkStat.fLossRatio <<
    "(" << videoStats.sessStat.stInNetworkStat.uLostPackets <<
    ")  Jitter:" << videoStats.sessStat.stInNetworkStat.uJitter << "\r\n";
    
    ostr << "  Size:" << videoStats.remoteVideoStat.uWidth << "x" <<
    videoStats.remoteVideoStat.uHeight << " (" << videoStats.remoteVideoStat.fFrameRate <<")";
    ostr <<  "  Bitrate:" << videoStats.remoteVideoStat.fBitRate <<  "  RenderFrameCount:" << videoStats.remoteVideoStat.uRenderFrameCount << "\r\n";
    if (PeerCall::Instance()->isActive())
    {
        unsigned index = 1;
        while (true) {
            WmeVideoStatistics video = {0};
            if(PeerCall::Instance()->m_endCaller->getTrackStatistics(index, video)) {
                ostr << "  Size:" << video.uWidth << "x" <<
                video.uHeight << " (" << video.fFrameRate <<")";
                ostr <<  "  Bitrate:" << video.fBitRate <<  "  RenderFrameCount:" << video.uRenderFrameCount << "\r\n";
                index++;
            }
            else {
                break;
            }
        }
    }
    ostr << "\r\n";
    
    ostr << "Screen:" << GetConnectionType(screenStats.connStat.connectionType)<< "\r\n";
    ostr << "  local Port:" << screenStats.connStat.uLocalPort
         << "  Remote Port:" << screenStats.connStat.uRemotePort << "\r\n";
    ostr << "  RTT:" << screenStats.sessStat.stOutNetworkStat.uRoundTripTime << "\r\n";
    ostr << "  Sent:        ";
    ostr << "      Bytes(Packets):" << screenStats.sessStat.stOutNetworkStat.uBytes <<
    "(" << screenStats.sessStat.stOutNetworkStat.uPackets <<")";
    ostr << "      uFailedIOPackets:" << screenStats.sessStat.stOutNetworkStat.uFailedIOPackets;
    ostr << "  Loss:" << screenStats.sessStat.stOutNetworkStat.fLossRatio <<
    ", Jitter:" << screenStats.sessStat.stOutNetworkStat.uJitter;
    ostr << "  Bitrate:" << screenStats.sessStat.stOutNetworkStat.uBitRate ;
    ostr << "  Size:" << screenStats.localScreenStat.stVideoStat.uWidth << "x" <<
    screenStats.localScreenStat.stVideoStat.uHeight << " (" << screenStats.localScreenStat.stVideoStat.fFrameRate <<
    ")";
    ostr << "  uIDRRecvNum:" << screenStats.localScreenStat.stVideoStat.uIDRRecvNum <<
    "  uIDRReqNum:" << screenStats.localScreenStat.stVideoStat.uIDRReqNum ;
    ostr << "  CaptureFrameTotalCount:" << screenStats.localScreenStat.stScreenShareCaptureStatistics.m_nTotalCapturedFrames <<
    "  CaptureAverageTime(ms):" << screenStats.localScreenStat.stScreenShareCaptureStatistics.m_nMsAverageCaptureTime << "\r\n";
    
    ostr << "  Received: ";
    ostr << "      Bytes(Packets):" << screenStats.sessStat.stInNetworkStat.uBytes <<
    "(" << screenStats.sessStat.stInNetworkStat.uPackets << ")";
    ostr << "  Loss(Count):" << screenStats.sessStat.stInNetworkStat.fLossRatio <<
    "(" << screenStats.sessStat.stInNetworkStat.uLostPackets <<
    ")  Jitter:" << screenStats.sessStat.stInNetworkStat.uJitter;
    ostr << "  Bitrate:" << screenStats.sessStat.stInNetworkStat.uBitRate;
    ostr << "  Size:" << screenStats.remoteScreenStat.stVideoStat.uWidth << "x"
         <<screenStats.remoteScreenStat.stVideoStat.uHeight << " (" << screenStats.remoteScreenStat.stVideoStat.fFrameRate <<
    ")";
    ostr << "  uIDRSentNum:" << screenStats.remoteScreenStat.stVideoStat.uIDRSentNum <<
            "  uIDRReqNum:" << screenStats.remoteScreenStat.stVideoStat.uIDRReqNum;
    ostr << "  FrameCount:" << screenStats.remoteScreenStat.stVideoStat.uRenderFrameCount
         << "\r\n\r\n";
    
    ostr << "NetworkIndex:\r\n";
    ostr << "  uplink: " << idxUplink <<", downlink: " << idxDownlink << ", bothlink: " << idxBothlink << std::endl;
    switch (uplinkStatus) {
        case WmeNetwork_recovered:
            ostr << "  Video ON";
            break;
        case WmeNetwork_bad:
            ostr << "  Bad Network";
            break;
        case WmeNetwork_video_off:
            ostr << "  Video OFF";
            break;
        default:
            break;
    }
    ostr << "\r\n";

	m_strStatistic = ostr.str();
    if (m_sink) {
		m_sink->OnStatsInfo(m_strStatistic.c_str(), strAudio.c_str(), strVideo.c_str(), strScreen.c_str());
    }
}
