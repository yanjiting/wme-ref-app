using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
//using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using wme4net;
using System.Runtime.InteropServices;
using System.Timers;
using System.ComponentModel;

namespace wme4netTest
{
    public class ScreenSourceCallback : IScreenSourceDelegate
    {
        private List<ScreenSource> mSources;
        public void listScreen(){
            mSources = ScreenSourceManager.Instance.sources();
            ScreenSourceManager.Instance.AddObserver(this);
            var a = ScreenSourceManager.Instance.FindSourceByFriendlyName("MainWindow");
            var b = ScreenSourceManager.Instance.FindSourceByUniqueName("7860");
            var c = ScreenSourceManager.Instance.defaultSource();
        }
        public void OnSourceAdded(ScreenSource source)
        {
            Console.WriteLine("OnSourceAdded");
        }

        public void OnSourceRemoved(ScreenSource source)
        {
            Console.WriteLine("OnSourceRemoved");
        }

        public void OnSourceUpdated(ScreenSource source)
        {
            Console.WriteLine("OnSourceUpdated");
        }
    }
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window, IMediaConnectionSink, ITraceSink
    {
        private MediaConnection mConnLocal, mConnRemote;
        private MediaTrack mLocalAudio, mRemoteAudio, mLocalShare;
        private MediaTrack mLocalVideo, mRemoteVideo, mRemoteShare;
        private Timer mTimer;

        public MainWindow()
        {
            WmeStatic.initMediaEngine();
            WmeStatic.setTraceLevel(WmeTraceLevel.WME_TRACE_LEVEL_ALL);
            //WmeStatic.setTraceSink(this);
            InitializeComponent();
            Closing += OnWindowClosing;
        }

        private void OnWindowClosing(object sender, CancelEventArgs e)
        {
            if (mConnLocal != null)
                mConnLocal.Stop();
            if (mConnRemote != null)
                mConnRemote.Stop();
            WmeStatic.setTraceSink(null);
        }

        public void OnDecodeSizeChanged(uint mid, uint csi, uint uWidth, uint uHeight)
        {
            if (mid == 2000) {
                //ShareViewImage.UpdateSource(uWidth, uHeight);
            }
        }

        public void OnRenderSizeChanged(uint mid, uint vid, WmeDirection direction, WmeVideoSizeInfo sizeInfo)
        {
        }

        public void OnEncodeSizeChanged(uint mid, uint uWidth, uint uHeight)
        {
        }

        public void OnCSIsChanged(uint mid, uint vid, uint[] oldCSIArray, uint[] newCSIArray)
        {

        }

        public void OnAvailableMediaChanged(uint mid, int count)
        {

        }

        public void OnMediaStatus(uint mid, uint vid, WmeMediaStatus status, bool hasCSI, uint csi)
        {

        }

        public void OnMediaError(uint mid, uint vid, int errorCode)
        {

        }

        public int OnTrace(uint level, uint threadid, string msg)
        {
            //This is too slow.
            //Console.WriteLine(msg);
            return 0;
        }

        public void OnError(int errorCode)
        {

        }
        public void OnMediaBlocked(uint mid, uint csi, bool blocked)
        {
        }

        public void OnMediaReady(uint mid, WmeDirection direction, WmeSessionType mediaType, MediaTrack pTrack)
        {
            if (direction == WmeDirection.WmeDirection_Send)
            {
                if (mid > 10)
                    return;
                if (mediaType == WmeSessionType.WmeSessionType_Audio)
                {
                    mLocalAudio = pTrack;
                }
                else if (mediaType == WmeSessionType.WmeSessionType_Video)
                {
                    mLocalVideo = pTrack;
                    //pTrack.AddRenderWindow(mVideoForm.GetSelfView());
                    //pTrack.AddRenderWindow(SelfViewPictureBox.Handle);
                    //pTrack.AddImageRender(LocalVideoImage);

                }else if(mediaType == WmeSessionType.WmeSessionType_ScreenShare)
                {
                    mLocalShare = pTrack;
                    if (mid == 2)
                        mLocalShare.AddScreenSource(ScreenSourceManager.Instance.defaultSource());
                }
                pTrack.Start(false);
            }
            else
            {
                if (mid < 10)
                    return;
                if (mediaType == WmeSessionType.WmeSessionType_Audio)
                {
                    mRemoteAudio = pTrack;
                }
                else if (mediaType == WmeSessionType.WmeSessionType_Video)
                {
                    mRemoteVideo = pTrack;                
                    pTrack.AddImageRender(RemoteVideoImage1);
                    //pTrack.AddImageRender(RemoteVideoImage2);
                    //pTrack.AddImageRender(RemoteVideoImage3);
                    //pTrack.AddImageRender(RemoteVideoImage4);
                }
                else if (mediaType == WmeSessionType.WmeSessionType_ScreenShare)
                {
                    mRemoteShare = pTrack;
                    if (mid == 2000)
                    {
                        //pTrack.AddRenderWindow(ShareViewPictureBox.Handle);
                        pTrack.AddImageRender(ShareViewImage);
                    }
                }
                pTrack.Start(false);
            }
        }

        public void OnSDPReady(WmeSdpType sdpType, string sdp)
        {
            if (sdpType == WmeSdpType.WmeSdp_Offer)
            {
                WmeSdpParsedInfo[] medias = mConnRemote.SetReceivedSDP(sdpType, sdp);
                foreach(WmeSdpParsedInfo mline in medias){
                    Console.WriteLine(mline.mid);
                }
                mConnRemote.CreateAnswer();
            }
            else
            {
                WmeSdpParsedInfo[] medias = mConnLocal.SetReceivedSDP(sdpType, sdp);
            }
        }

        public void OnSessionStatus(uint mid, WmeSessionType mediaType, WmeSessionStatus status)
        {

        }

        public void OnNetworkStatus(WmeNetworkStatus status, WmeNetworkDirection direc)
        {

        }

        private void Stats_Timer(object sender, ElapsedEventArgs e)
        {
            WmeVideoConnectionStatistics stats = mConnLocal.GetVideoStatistics(0);
            string localIp = stats.connStat.localIp.ToString();
            string remoteIp = stats.connStat.remoteIp.ToString();
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            mTimer = new Timer(1000);
            mTimer.Elapsed += new ElapsedEventHandler(Stats_Timer);
            mTimer.Enabled = true; // Enable it

            mConnLocal = new MediaConnection();
            mConnRemote = new MediaConnection();

            mConnLocal.StartMediaLogging(1000);
            mConnLocal.SetSink(this);
            mConnRemote.SetSink(this);
            var rv = mConnLocal.AddMedia(WmeSessionType.WmeSessionType_Video,
                WmeDirection.WmeDirection_SendRecv, 0, "");
            rv = mConnRemote.AddMedia(WmeSessionType.WmeSessionType_Video,
                WmeDirection.WmeDirection_SendRecv, 1000, "");
            rv = mConnLocal.AddMedia(WmeSessionType.WmeSessionType_ScreenShare,
                WmeDirection.WmeDirection_SendRecv, 2, "");
            rv = mConnRemote.AddMedia(WmeSessionType.WmeSessionType_ScreenShare,
                WmeDirection.WmeDirection_SendRecv, 2000, "");

            //mConnLocal.GetAudioConfig(1).EnableEC(true);
            //mConnLocal.GetVideoConfig(0).SetSelectedCodec(WmeCodecType.WmeCodecType_AVC);
            mConnLocal.GetVideoConfig(0).SetEncodeParams(WmeCodecType.WmeCodecType_AVC, 0x42000c,
                27600, 920, 200, 4000);
            mConnLocal.GetVideoConfig(0).SetEncodeParams(WmeCodecType.WmeCodecType_SVC, 0x42000c,
                27600, 920, 200, 4000);
            //mConnLocal.GetGlobalConfig().EnableMultiStream(true);
            //mConnRemote.GetGlobalConfig().EnableMultiStream(true);
            mConnLocal.CreateOffer();
        }

        private ScreenSourceCallback mScreenEnum;
        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            mScreenEnum = new ScreenSourceCallback();
            mScreenEnum.listScreen();
        }

        private void Button_Click_2(object sender, RoutedEventArgs e)
        {
            var mics = DeviceManager.Instance.listDevices(WmeSessionType.WmeSessionType_Audio, FlowType.Capture);
            var spks = DeviceManager.Instance.listDevices(WmeSessionType.WmeSessionType_Audio, FlowType.Render);
            var cams = DeviceManager.Instance.listDevices(WmeSessionType.WmeSessionType_Video, FlowType.Capture);
            if (mRemoteVideo != null)
            {
                uint[] csis = mRemoteVideo.GetCSI();
                uint vid = mRemoteVideo.GetVID();
                int nLen = csis.Length;
            }
            if (mLocalVideo != null)
                mLocalVideo.GetCSI();
        }

        private void Window_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            if (mConnLocal != null)
            {
                WmeCpuUsage cpuUsage = mConnLocal.GetCpuUsage();
            }
            //if (mLocalVideo != null)
            //    mLocalVideo.UpdateRenderWindow(SelfViewPictureBox.Handle);
            //if (mRemoteVideo != null)
            //    mRemoteVideo.UpdateRenderWindow(RemoteViewPictureBox.Handle);
            
            
            //if (mRemoteShare != null)
            //    mRemoteShare.UpdateRenderWindow(ShareViewPictureBox.Handle);
        }
    }
}
