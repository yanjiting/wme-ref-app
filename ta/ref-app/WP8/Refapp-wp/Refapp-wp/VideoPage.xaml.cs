using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Navigation;
using Microsoft.Phone.Controls;
using Microsoft.Phone.Shell;
using Refapp_wp.Resources;
using WMEAudioComponent;
using WMEVideoComponent;
using System.ComponentModel;

using System.Windows.Threading;
using System.Runtime.InteropServices;
using Microsoft.Phone.Networking.Voip;
using System.Runtime.InteropServices.WindowsRuntime;
using System.Diagnostics;
using Microsoft.Phone.Info;
using System.Threading;
using Windows.UI.Popups;
using Windows.Foundation;
using Windows.Devices.Sensors;
using Windows.UI.Core;

namespace Refapp_wp
{

    public partial class VideoPage : PhoneApplicationPage, IBackgroundEventListener
    {
        static private WMEAudioHost m_AudioHost = null;
        private WMEVideoHost m_Videohost = null;
        private WMEAudioComponent.SWITCHINFO m_audioSwitchInfo;
        private WMEVideoComponent.SWITCHINFO m_videoSwitchInfo;
        public String m_server = "";
        public String m_linus = "";
        public int m_bgProcessid = 0;
        public uint m_csi = 0;
        private bool m_StatisticEnable = false;
        private bool m_AudioMuted = false;
        private bool m_VideoMuted = false;
        private bool m_VideoDumpEnabled = false;
        private string m_audioStatisticString = "";
        private string m_videoStatisticString = "";
        private bool m_bTA = false;
        private bool m_bQuit = false;
        private Windows.Devices.Sensors.Accelerometer accelerometer;
        private PageOrientation orientation;
       

      

        private DispatcherTimer m_HeartBeatTimer;
        private DispatcherTimer m_StatisticTimer;
        delegate void StartAudioCallDelegate(VideoPage host);
        delegate void StartVideoCallDelegate(VideoPage host);
        delegate void AccelerometerDelegate(VideoPage host, AccelerometerReadingChangedEventArgs e);
        public System.Windows.Threading.Dispatcher m_dispatcher;
        //private Stopwatch m_stopwatch;


        public VideoPage()
        {
            InitializeComponent();
            PhoneApplicationService.Current.Activated += App_Activated;
            PhoneApplicationService.Current.Deactivated += App_Deactivated;
            this.Loaded += PageLoaded;

            orientation = PageOrientation.PortraitUp;
            m_Videohost = new WMEVideoHost();
            //m_dispatcher = Deployment.Current.Dispatcher;
            //m_stopwatch = new Stopwatch();
            //m_stopwatch.Start();
        }

        private void App_Activated(Object sender, ActivatedEventArgs e)
        {
            // TODO: Refresh network data
            m_Videohost.StartStopTrack(true, false);
            m_Videohost.StartStopTrack(true, true);
            m_AudioHost.SetBackgroundEvent(this);
        }

        private void App_Deactivated(Object sender, DeactivatedEventArgs e)
        {
            // TODO: This is the time to save app data in case the process is terminated
            m_Videohost.StartStopTrack(false, false);
            m_Videohost.StartStopTrack(false, true);
            m_AudioHost.SetBackgroundEvent(null);
        }

        private void PhoneApplicationPage_OrientationChanged(object sender, OrientationChangedEventArgs e)
        {
            /*if (e.Orientation == PageOrientation.PortraitUp)
            {
                m_Videohost.ChangeOrientation(0);
                VideoPreviewSurface.Margin = new Thickness(98, 257, 0, 0);
            }
            else if (e.Orientation == PageOrientation.PortraitDown)
            {
                m_Videohost.ChangeOrientation(1);
                VideoPreviewSurface.Margin = new Thickness(98, 257, 0, 0);
            }
            else if (e.Orientation == PageOrientation.LandscapeLeft)
            {
                m_Videohost.ChangeOrientation(2);
                VideoPreviewSurface.Margin = new Thickness(-450, 200, 0, 0);
            }
            else if (e.Orientation == PageOrientation.LandscapeRight)
            {
                m_Videohost.ChangeOrientation(3);
                VideoPreviewSurface.Margin = new Thickness(-450, 200, 0, 0);
            }*/
        }

        void ChangeOrientation(PageOrientation e)
        {
            orientation = e;
            if (e == PageOrientation.PortraitUp)
            {
                m_Videohost.ChangeOrientation(0);
                VideoPreviewSurface.Margin = new Thickness(98, 257, 0, 0);
            }
            else if (e == PageOrientation.PortraitDown)
            {
                m_Videohost.ChangeOrientation(1);
                VideoPreviewSurface.Margin = new Thickness(98, 257, 0, 0);
            }
            else if (e == PageOrientation.LandscapeLeft)
            {
                m_Videohost.ChangeOrientation(2);
                //VideoPreviewSurface.Margin = new Thickness(-450, 200, 0, 0);
                VideoPreviewSurface.Margin = new Thickness(0, 257, 98, 0);
            }
            else if (e == PageOrientation.LandscapeRight)
            {
                m_Videohost.ChangeOrientation(3);
                //VideoPreviewSurface.Margin = new Thickness(-450, 200, 0, 0);
                VideoPreviewSurface.Margin = new Thickness(98, 257, 0, 0);
            }
        }

        void PageLoaded(object sender, RoutedEventArgs e)
        {
            accelerometer = Windows.Devices.Sensors.Accelerometer.GetDefault();

            accelerometer.ReadingChanged += new Windows.Foundation.TypedEventHandler<Accelerometer, AccelerometerReadingChangedEventArgs>(ReadingChanged);
            Random rnd = new Random();
            m_csi = ((uint)rnd.Next()) & 0xFFFFFF00;

            m_audioSwitchInfo = (WMEAudioComponent.SWITCHINFO)NavigationService.GetNavigationData();
            m_videoSwitchInfo.app_share = m_audioSwitchInfo.app_share;
            m_videoSwitchInfo.enable_audio = m_audioSwitchInfo.enable_audio;
            m_videoSwitchInfo.enable_video = m_audioSwitchInfo.enable_video;
            m_videoSwitchInfo.calliope = m_audioSwitchInfo.calliope;
            m_videoSwitchInfo.fec = m_audioSwitchInfo.fec;
            m_videoSwitchInfo.loop = m_audioSwitchInfo.loop;
            m_videoSwitchInfo.multi_stream = m_audioSwitchInfo.multi_stream;
            m_videoSwitchInfo.srtp = m_audioSwitchInfo.srtp;
            m_videoSwitchInfo.tc_aec = m_audioSwitchInfo.tc_aec;
            m_videoSwitchInfo.video_hw = m_audioSwitchInfo.video_hw;
            m_videoSwitchInfo.vpio = m_audioSwitchInfo.vpio;
            m_videoSwitchInfo.dump_audio = m_audioSwitchInfo.dump_audio;
            m_videoSwitchInfo.dump_video = m_audioSwitchInfo.dump_video;
        
            m_server = NavigationService.GetNavigationString1();
            m_linus = NavigationService.GetNavigationString2();
            m_bgProcessid = NavigationService.GetNavigationInt();

            m_bTA = NavigationService.GetNavigationTA();

            
            m_Videohost.SetDataChannelDelegate(DataChannelDataAvailable);
            m_Videohost.Start(m_csi);
            StartHeartBeatTimer();
            StartStatisticTimer();

            //var stopwatch = new Stopwatch();
            //stopwatch.Start();
            try
            {
                if (m_AudioHost == null)
                    m_AudioHost = (WMEAudioHost)WindowsRuntimeMarshal.GetActivationFactory(typeof(WMEAudioHost)).ActivateInstance();
            }
            catch (InvalidCastException err)
            {
                Debug.WriteLine("[App] Error launching VoIP background process. UI may no longer be in the foreground. Exception: " + err.Message);
                throw;
            }
            //long elapsed_time = stopwatch.ElapsedMilliseconds;
            //Debug.WriteLine(elapsed_time);

            m_AudioHost.Start(m_bTA, m_csi);
            Start_Call();
            //this.Dispatcher.BeginInvoke(new StartAudioCallDelegate(Start_Call), this);

            if (m_bTA)
            {
                m_Videohost.InitTAContext();
            }

        }

        private void ReadingChanged(object sender, AccelerometerReadingChangedEventArgs e)
        {
            this.Dispatcher.BeginInvoke(new AccelerometerDelegate(AccelerometerReadChanged), this, e);
        }

        protected override void OnBackKeyPress(CancelEventArgs e)
        {
            base.OnBackKeyPress(e);

            m_AudioHost.StopCall();
            m_Videohost.StopCall();
        }

        private void HeartBeatTimer_Tick(object sender, EventArgs e)
        {
            //DispatcherTimer in main thread
            if (m_bQuit)
            {
                Thread.Sleep(1000);
                Application.Current.Terminate();
                return;
            }

            m_Videohost.OnTimer();
            //long elapsed_time = m_stopwatch.ElapsedMilliseconds;
            //m_stopwatch = new Stopwatch();
           // m_stopwatch.Start();
            
        }

        private void StartHeartBeatTimer()
        {
            m_HeartBeatTimer = new System.Windows.Threading.DispatcherTimer();
            m_HeartBeatTimer.Tick += new EventHandler(HeartBeatTimer_Tick);
            m_HeartBeatTimer.Interval = new TimeSpan(0, 0, 0, 0, 100);
            m_HeartBeatTimer.Start();
        }

        private void ShowMemoryUsage()
        {
            long memoryUsage = Microsoft.Phone.Info.DeviceStatus.ApplicationCurrentMemoryUsage;
            UInt32 uMemoryMB = (UInt32)((float)memoryUsage / (1024 * 1024));
            memorySize_tb.Text = "MU:" + uMemoryMB.ToString() + "MB";
            UpdateStatisticsInfo();
        }

        private void StatisticTimer_Tick(object sender, EventArgs e)
        {
            // m_WMEAudioHost.GetStatistics();
            ShowMemoryUsage();
        }

        private void StartStatisticTimer()
        {
            m_StatisticTimer = new System.Windows.Threading.DispatcherTimer();
            m_StatisticTimer.Tick += new EventHandler(StatisticTimer_Tick);
            m_StatisticTimer.Interval = new TimeSpan(0, 0, 1);
            m_StatisticTimer.Start();
        }

		private void VideoPreviewSurface_Loaded(object sender, RoutedEventArgs e)
        {
            WMEVideoComponent.RenderDemoWindowMode wp8Render = m_Videohost.GetRender(RenderType.LocalRender);
            if (wp8Render == null)
            {
                return;
            }

            // Set window bounds in dips
            wp8Render.WindowBounds = new Windows.Foundation.Size(
                (float)VideoPreviewSurface.ActualWidth,
                (float)VideoPreviewSurface.ActualHeight
                );

            // Set native resolution in pixels
            wp8Render.NativeResolution = new Windows.Foundation.Size(
                (float)Math.Floor(VideoPreviewSurface.ActualWidth * Application.Current.Host.Content.ScaleFactor / 100.0f + 0.5f),
                (float)Math.Floor(VideoPreviewSurface.ActualHeight * Application.Current.Host.Content.ScaleFactor / 100.0f + 0.5f)
                );

            // Set render resolution to the full native resolution
            wp8Render.RenderResolution = wp8Render.NativeResolution;

            // Hook-up native component to DrawingSurface
            VideoPreviewSurface.SetContentProvider(wp8Render.CreateContentProvider());
            VideoPreviewSurface.SetManipulationHandler(wp8Render);

        }

        private void VideoRemoteSurface_Loaded(object sender, RoutedEventArgs e)
        {
            RenderDemoWindowMode wp8Render = m_Videohost.GetRender(RenderType.RemoteRender);
            if (wp8Render == null)
            {
                return;
            }

            // Set window bounds in dips
            wp8Render.WindowBounds = new Windows.Foundation.Size(
                (float)VideoRemoteSurface.ActualWidth,
                (float)VideoRemoteSurface.ActualHeight
                );

            // Set native resolution in pixels
            wp8Render.NativeResolution = new Windows.Foundation.Size(
                (float)Math.Floor(VideoRemoteSurface.ActualWidth * Application.Current.Host.Content.ScaleFactor / 100.0f + 0.5f),
                (float)Math.Floor(VideoRemoteSurface.ActualHeight * Application.Current.Host.Content.ScaleFactor / 100.0f + 0.5f)
                );

            // Set render resolution to the full native resolution
            wp8Render.RenderResolution = wp8Render.NativeResolution;

            // Hook-up native component to DrawingSurface
            // TODO: Uncomment it when Video and Audio is splitted
            VideoRemoteSurface.SetContentProvider(wp8Render.CreateContentProvider());
            VideoRemoteSurface.SetManipulationHandler(wp8Render);
        }

        void Start_Call()
        {
            if (m_audioSwitchInfo.enable_audio)
            {
                m_AudioHost.StartCall(m_server, m_linus, m_audioSwitchInfo);
                m_AudioHost.SetBackgroundEvent(this);
            }
            else
            {
                Start_VideoCall(this);
            }
        }

        static void AccelerometerReadChanged(VideoPage host, AccelerometerReadingChangedEventArgs e)
        {
            AccelerometerReading reading = e.Reading;
            PageOrientation ori = host.orientation;
            if( Math.Abs(Math.Abs(reading.AccelerationX) - Math.Abs(reading.AccelerationY)) > 0.1)
            {
                if(Math.Abs(reading.AccelerationX) > Math.Abs(reading.AccelerationY))
                {
                    if (reading.AccelerationX > 0)
                        ori = PageOrientation.LandscapeRight;
                    else
                        ori = PageOrientation.LandscapeLeft;
                }
                else 
                {
                    if (reading.AccelerationY > 0)
                        ori = PageOrientation.PortraitDown;
                    else
                        ori = PageOrientation.PortraitUp;
                }
            }
            else
            {
                return;
            }

            if (ori != host.orientation)
                host.ChangeOrientation(ori);
            
        }

        private void ScreenShareSurface_Loaded(object sender, RoutedEventArgs e)
        {
            //does not need AS right now
            /*RenderDemoWindowMode wp8Render = m_Videohost.GetRender(RenderType.ASRender);
            if (wp8Render == null)
            {
                return;
            }

            // Set window bounds in dips
            wp8Render.WindowBounds = new Windows.Foundation.Size(
                (float)ScreenShareSurface.ActualWidth,
                (float)ScreenShareSurface.ActualHeight
                );

            // Set native resolution in pixels
            wp8Render.NativeResolution = new Windows.Foundation.Size(
                (float)Math.Floor(ScreenShareSurface.ActualWidth * Application.Current.Host.Content.ScaleFactor / 100.0f + 0.5f),
                (float)Math.Floor(ScreenShareSurface.ActualHeight * Application.Current.Host.Content.ScaleFactor / 100.0f + 0.5f)
                );

            // Set render resolution to the full native resolution
            wp8Render.RenderResolution = wp8Render.NativeResolution;

            // Hook-up native component to DrawingSurface
            // TODO:
            ScreenShareSurface.SetContentProvider(wp8Render.CreateContentProvider());
            ScreenShareSurface.SetManipulationHandler(wp8Render);*/
        }

        static void Start_VideoCall(VideoPage host)
        {
            if (host.m_audioSwitchInfo.enable_video)
            {
                host.m_Videohost.StartCall(host.m_server, host.m_linus, host.m_videoSwitchInfo);
            }
        }

        public void DataChannelDataAvailable(string key, string value)
        {
            if (key == "VideoStatisticsInfo" && m_audioSwitchInfo.enable_audio)
            {
                m_videoStatisticString = value;
            } else if(key == "audioSdp" && m_audioSwitchInfo.enable_audio)
            {
                m_AudioHost.SetRemoteSdp(value);
            }
            else if (m_bTA && key == "QuitWP8")
            {
                m_bQuit = true;
            }
        }

        public void OnBackgroundDataAvailable(string key, string value)
        {
            if (key == "OnAudioTimeStampChanged")
            {
                m_Videohost.SetParam(key, value);
            } 
            else if (key == "AudioStatisticsInfo")
            {
                m_audioStatisticString = value;
            }
            else if (key == "onSdpReady")
            {
                // for audio only case, we should handle in clickcall
                String localSdp = m_AudioHost.GetLocalSdp();
                m_Videohost.SetAudioLocalSDP(localSdp);

                this.Dispatcher.BeginInvoke(new StartVideoCallDelegate(Start_VideoCall), this);
            }
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            m_HeartBeatTimer.Stop();
            //m_StatisticTimer.Stop();

            if (null != m_AudioHost)
            {
                m_AudioHost.StopCall();
            }
            if (null != m_Videohost)
            {
                m_Videohost.StopCall();
            }
            Application.Current.Terminate();
        }

        private void UpdateStatisticsInfo()
        {
            if (true == m_StatisticEnable)
            {
                string info = m_audioStatisticString + "\n\n" + m_videoStatisticString;
                statistics_tb.Text = info;
            }
        }

        private void Button_Statistics_Click(object sender, RoutedEventArgs e)
        {
            statistics_tb.Text = "";
            Button btn = sender as Button;
            if (false == m_StatisticEnable)
            {
                statistics_tb.Visibility = Visibility.Visible;
                m_StatisticEnable = true;
                m_AudioHost.SetStatisticStatus(true);
                m_Videohost.SetStatisticStatus(true);
                btn.Content = "Statis-Off";
            } 
            else
            {
                m_StatisticEnable = false;
                m_AudioHost.SetStatisticStatus(false);
                m_Videohost.SetStatisticStatus(false);
                statistics_tb.Visibility = Visibility.Collapsed;
                btn.Content = "Statis-On";
            }
        }

        private void Button_Click_MuteAudio(object sender, RoutedEventArgs e)
        {
            Button btn = sender as Button;
            if (false == m_AudioMuted)
            {
                m_AudioMuted = true;
                m_AudioHost.Mute(true);
                btn.Content = "UnMuteAudio";
            } 
            else
            {
                m_AudioMuted = false;
                m_AudioHost.Mute(false);
                btn.Content = "MuteAudio";
            }

        }

        private void Button_Click_MuteVideo(object sender, RoutedEventArgs e)
        {
            Button btn = sender as Button;
            if (false == m_VideoMuted)
            {
              m_VideoMuted = true;
              m_Videohost.Mute(true);              
              btn.Content = "UnMuteVideo";
            }
            else
            {
                m_VideoMuted = false;
                m_Videohost.Mute(false);
                btn.Content = "MuteVideo";
            }
        }

        private void Button_DumpVideo_Click(object sender, RoutedEventArgs e)
        {
            Button btn = sender as Button;
            if (false == m_VideoDumpEnabled)
            {
                m_VideoDumpEnabled = true;
                m_Videohost.EnableVideoDump(true);
                btn.Content = "VDump-Off";
            }
            else
            {
                m_VideoDumpEnabled = false;
                m_Videohost.EnableVideoDump(false);
                btn.Content = "VDump-On";
            }
        }
    }
}