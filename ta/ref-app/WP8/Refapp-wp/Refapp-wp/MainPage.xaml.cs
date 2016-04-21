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

using System.Windows.Threading;
using System.Runtime.InteropServices;
using Microsoft.Phone.Networking.Voip;
using System.Diagnostics;
using System.Threading.Tasks;

namespace Refapp_wp
{
    public static class Extensions
    {
        private static object Data;
        private static String string1;
        private static String string2;
        private static int pid;
        private static bool TA;

        /// <summary>
        /// Navigates to the content specified by uniform resource identifier (URI).
        /// </summary>
        /// <param name="navigationService">The navigation service.</param>
        /// <param name="source">The URI of the content to navigate to.</param>
        /// <param name="data">The data that you need to pass to the other page 
        /// specified in URI.</param>
        public static void Navigate(this NavigationService navigationService, Uri source, object data, String str1, String str2, int id, bool bTA)
        {
            Data = data;
            string1 = str1;
            string2 = str2;
            pid = id;
            TA = bTA;
            navigationService.Navigate(source);
        }

        /// <summary>
        /// Gets the navigation data passed from the previous page.
        /// </summary>
        /// <param name="service">The service.</param>
        /// <returns>System.Object.</returns>
        public static object GetNavigationData(this NavigationService service)
        {
            return Data;
        }

        public static String GetNavigationString1(this NavigationService service)
        {
            return string1;
        }

        public static String GetNavigationString2(this NavigationService service)
        {
            return string2;
        }

        public static int GetNavigationInt(this NavigationService service)
        {
            return pid;
        }

        public static bool GetNavigationTA(this NavigationService service)
        {
            return TA;
        }
    }

    public partial class MainPage : PhoneApplicationPage
    {
        public SWITCHINFO m_SwitchInfo;
        public String m_server = "";
        public String m_linus = "";

        // Constructor
        public MainPage()
        {
            InitializeComponent();
            var stopwatch = new Stopwatch();
            stopwatch.Start();
            try
            {
                VoipBackgroundProcess.Launch();
            }
            catch (InvalidCastException err)
            {
                Debug.WriteLine("[App] Error launching VoIP background process. UI may no longer be in the foreground. Exception: " + err.Message);
                throw;
            }
            long elapsed_time = stopwatch.ElapsedMilliseconds;
            Debug.WriteLine(elapsed_time);
            //Task.Run(() => { VoipBackgroundProcess.Launch(); }
            //    );
#if TA
            Loaded += MainPage_Loaded;
#endif
        }


        void MainPage_Loaded(object sender, RoutedEventArgs e)
        {
            NavigationService.Navigate(new Uri("/VideoPage.xaml", UriKind.Relative), m_SwitchInfo, m_server, m_linus, 0, true);
        }


        //Initial the default call status
        private void InitDefaultSwithStatus()
        {
            m_SwitchInfo.calliope = (calliope_cb.IsChecked == true);
            m_SwitchInfo.loop = (loop_cb.IsChecked == true);
            m_SwitchInfo.enable_audio = (enableaudio_cb.IsChecked == true);
            m_SwitchInfo.srtp = (srtp_cb.IsChecked == true);
            m_SwitchInfo.multi_stream = (multistream_cb.IsChecked == true);

            m_SwitchInfo.fec = (fec_cb.IsChecked == true);
            m_SwitchInfo.video_hw = (videohw_cb.IsChecked == true);
            m_SwitchInfo.app_share = (appshare_cb.IsChecked == true);
            m_SwitchInfo.dump_video = (video_dump_cb.IsChecked == true);
            m_SwitchInfo.tc_aec = (tcaec_cb.IsChecked == true);
            m_SwitchInfo.enable_video = (enablevideo_cb.IsChecked == true);
            m_SwitchInfo.dump_audio = (audio_dump_cb.IsChecked == true);
            m_server = host_tb.Text;
            m_linus = linus_tb.Text;
        }

        private void StartCallBtn_Click(object sender, RoutedEventArgs e)
        {
            InitDefaultSwithStatus();
            NavigationService.Navigate(new Uri("/VideoPage.xaml", UriKind.Relative), m_SwitchInfo, m_server, m_linus, 0, false);
        }

        private void Button_Click_AddOne(object sender, RoutedEventArgs e)
        {
            String ws_server = host_tb.Text;
            String[] strArray = ws_server.Split('=');
            int new_id = Convert.ToInt32(strArray[1]) + 1;
            host_tb.Text = strArray[0] + '=' + Convert.ToString(new_id);
        }
    }
}