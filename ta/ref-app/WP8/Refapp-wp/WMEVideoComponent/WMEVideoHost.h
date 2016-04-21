#pragma once
#include "ExternalRenderContentProvider.h"
#include "BackdoorAgent.h"
#include "wputil.h"
#include "StatsInfo.h"
#include "testconfig.h"

class CBackdoorAgent;
class TAServer;


namespace WMEVideoComponent
{
    public delegate void DataChannelDelegate(Platform::String^ key, Platform::String^ value);
    class CAppSink : public IAppSink, public StatsInfoSink
    {
    public:
        virtual void onCheckMultiVideo() {}
        virtual void onSdpReady(std::string from)
        {
            if (from == "OnSDPReceived") {
                std::string sdp = TestConfig::i().m_bgSdpAnwser;
                Platform::String^ audiosdp = stdStrToPlatStr(sdp);
                m_delegate("audioSdp", audiosdp);
            }            
        }

        virtual void OnStatsInfo(const char *strInfo, const char *audioStats, const char *videoStats, const char *screenStats)
        {
            m_delegate("VideoStatisticsInfo", stdStrToPlatStr(std::string(videoStats)));
        }

		virtual void onQuitWP8()
		{
			m_delegate("QuitWP8", "");
		}



        DataChannelDelegate^ m_delegate;
    };

    public value struct SWITCHINFO
    {
        bool calliope;
        bool loop;
        bool app_share;
        bool srtp;
        bool multi_stream;
        bool fec;
        bool video_hw;
        bool vpio;
        bool tc_aec;
        bool enable_audio;
        bool enable_video;
        bool dump_audio;
        bool dump_video;
    };

    public enum class RenderType
    {
        LocalRender = 0,
        RemoteRender,
        ASRender,
    };


    public ref class WMEVideoHost sealed
    {
    public:
        WMEVideoHost();
        virtual ~WMEVideoHost();
        void Start(unsigned int csi);
        void Stop();
        void Mute(bool isMute);
        void SetStatisticStatus(bool open);
        void StartCall(Platform::String^ server, Platform::String^ linus, SWITCHINFO switchinfo);
        void StopCall();
        void SetAudioLocalSDP(Platform::String^ sdp);
        void OnTimer();
        RenderDemoWindowMode^ GetRender(RenderType kType);
        void SetParam(Platform::String^ key, Platform::String^ value);

        void InitTAContext();

        void ChangeOrientation(int eFlag);
        void StartStopTrack(bool bStart, bool bRemote);
        void EnableVideoDump(bool enable);

    public:
        void SetDataChannelDelegate(DataChannelDelegate^ aDelegate);

    private:
        WMETraceSinkEnd* m_pTracer;
        Windows::System::Threading::ThreadPoolTimer ^  m_HeartBeatTimer;
        bool m_bStopHeartBeatThreadFlag;
        RenderDemoWindowMode^ m_pWp8LocalRender;
        RenderDemoWindowMode^ m_pWp8RemoteRender;
        RenderDemoWindowMode^ m_pWp8ASRender;
        //switch info
        SWITCHINFO m_bSwitchInfo;
        Platform::String^ m_server;
        Platform::String^ m_linus;
        
        CBackdoorAgent   *m_pBackdoorAgent;
	TAServer		 *m_pTAServer;

        CAppSink m_sink;
        StatsInfo m_statsInfo;
    };
}
