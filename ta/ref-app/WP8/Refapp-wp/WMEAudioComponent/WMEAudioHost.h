#pragma once
#include "IBackgroundEventListener.h"
#include "BackdoorAgent.h"
#include "wputil.h"
#include "StatsInfo.h"

class CBackdoorAgent;
class TAServer;

//#define TEST_RINGTONE
class RTPFrame;

using namespace Windows::System::Threading;

namespace WMEAudioComponent
{
    ref class CallController;
    class CAppSink : public IAppSink, public StatsInfoSink
    {
    public:
        virtual void onCheckMultiVideo() {}
        virtual void onSdpReady(std::string from)
        {
            if (m_listener && from == "onSdpReady")
            {
                m_listener->OnBackgroundDataAvailable("onSdpReady", "");
            }
        }
        virtual void onCallback(std::string key, std::string value)
        {
            if (m_listener  && key == "OnAudioTimeStampChanged")
            {
                m_listener->OnBackgroundDataAvailable(stdStrToPlatStr(key), stdStrToPlatStr(value));
            }
        }

        virtual void OnStatsInfo(const char *strInfo, const char *audioStats, const char *videoStats, const char *screenStats)
        {
            if (m_listener)
            {
                m_listener->OnBackgroundDataAvailable("AudioStatisticsInfo", stdStrToPlatStr(std::string(audioStats)));
            }
        }

        IBackgroundEventListener^  m_listener;
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

    public ref class WMEAudioHost sealed
    {
    public:
        WMEAudioHost();
        virtual ~WMEAudioHost();
        void Start(bool isTa, unsigned int csi);
        void Stop();
        void Mute(bool isMute);
        void SetStatisticStatus(bool open);
        void StartCall(Platform::String^ server, Platform::String^ linus, SWITCHINFO switchinfo);
        void _startCall();
        void StopCall();
        void CreateHeartbeatsThread(bool isTa);
        Platform::String^ GetLocalSdp();
        void SetRemoteSdp(Platform::String^ sdp);
        void SetBackgroundEvent(IBackgroundEventListener^ bgEventListener);

		void InitTAContext();

    private:
#ifdef TEST_RINGTONE
        RTPFrame* BuildRTP(int frameSize, void* framedata,
            unsigned char playloadtype, WORD sequence, DWORD timestamp, DWORD SSID);
        int PlayFunc();
		void PlayRingtone();
        void OnRingtoneTimer();
#endif

    private:
        ThreadPoolTimer^ m_periodicTimer;
        WMETraceSinkEnd* m_pTracer;
        bool m_bStopHeartBeatThreadFlag;
        bool m_bStopCall;
        bool m_bStartedCall;
        //switch info
        SWITCHINFO m_bSwitchInfo;
        Platform::String^ m_server;
        Platform::String^ m_linus;
        IAsyncAction^  m_heartbeatThread;

		CBackdoorAgent   *m_pBackdoorAgent;
		TAServer		 *m_pTAServer;
        CAppSink m_sink;
        StatsInfo m_statsInfo;
        CallController^ m_call;
    };
}
