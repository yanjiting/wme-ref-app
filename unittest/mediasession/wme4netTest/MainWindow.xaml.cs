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
using System.Collections.ObjectModel;
using System.Globalization;
namespace wme4netTest
{
    //Add by Boris
    //For UI
    public sealed class WbxTraceColorConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter,
            CultureInfo culture)
        {
            ListBoxItem item = (ListBoxItem)value;
            if (item == null || item.DataContext == null)
                return value;
            LogItemData logitem = item.DataContext as LogItemData;
            if (logitem == null)
                return value;

            if (logitem.wbxLevel == WbxTraceLevel.WBX_TRACE_ERROR)
                return Brushes.Red;
            else if (logitem.wbxLevel == WbxTraceLevel.WBX_TRACE_WARNING)
                return Brushes.Orange;
            else
                return Brushes.Blue;
        }
        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            return value;
        }
    }
    public sealed class DeviceListItemData
    {
        public MediaDevice mediaDevice { get; set; }
        public String displayName { get; set; }
    }

    public enum WbxTraceLevel
    {
        WBX_TRACE_INTERNAL = -1,
        WBX_TRACE_ERROR = 0,
        WBX_TRACE_WARNING,
        WBX_TRACE_INFO,
        WBX_TRACE_DEBUG,
        WBX_TRACE_DETAIL
    }
    public sealed class LogItemData
    {
        public LogItemData()
        {
            DateTime cur = DateTime.Now;
            _timeStamp = cur.Date.ToShortDateString() + " " +cur.ToShortTimeString();
        }
        public UInt64 seqNum { get; set; }
        private string _timeStamp="";
        public string timeStamp
        {
            get
            {
                return _timeStamp;
            }
        }
        private string _levelDisplay;
        public string levelDispaly
        {
            get
            {
                return _levelDisplay;
            }
        }
        private WbxTraceLevel _wbxLevel;
        public WbxTraceLevel wbxLevel { 
            get
            {
                return _wbxLevel;
            }
            set
            {
                _wbxLevel = value;
                _levelDisplay = value.ToString();
            }
        }
        public String wbxLogInfo { get; set; }
    }
    public class Wme4NetViewModal :INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        public ObservableCollection<DeviceListItemData> audioCaptureDeviceListView { get; set; }
        public ObservableCollection<DeviceListItemData> audioPlaybackDeviceListView { get; set; }
        public ObservableCollection<DeviceListItemData> videoDeviceListView { get; set; }
        public ObservableCollection<LogItemData> audioTraceView { get; set; }
        public ObservableCollection<String> aecTypeView { get; set; }

        private uint _audioVolume = 0;
        public uint audioVolume
        {
            get
            {
                return this._audioVolume;
            }

            set
            {
                if (this._audioVolume != value)
                {
                    this._audioVolume = (value <=65535) ? value : 65535;
                    this.OnPropertyChanged("audioVolume");
                }
            }

        }
        private uint _maxLines;
        public uint maxLines
        {
            get
            {
                return this._maxLines;
            }

            set
            {
                if (this._maxLines != value)
                {
                    this._maxLines = value;
                    this.OnPropertyChanged("maxLines");
                }
            }

        }

        private string _keyword;
        public string keyWord
        {
            get
            {
                return this._keyword;
            }

            set
            {
                if (this._keyword != value)
                {
                    this._keyword = value;
                    this.OnPropertyChanged("keyword");
                }
            }

        }

        private bool _MultiStreamEnabled;
        public bool multistreamEnabled
        {
            get
            {
                return this._MultiStreamEnabled;
            }

            set
            {
                if (this._MultiStreamEnabled != value)
                {
                    this._MultiStreamEnabled = value;
                    this.OnPropertyChanged("multistreamEnabled");
                }
            }

        }

        private string _soundEffectStateMic;
        public string soundEffectStateMic
        {
            get
            {
                return this._soundEffectStateMic;
            }

            set
            {
                if (this._soundEffectStateMic != value)
                {
                    this._soundEffectStateMic = value;
                    this.OnPropertyChanged("soundEffectStateMic");
                }
            }

        }

        private string _soundEffectStateSpk;
        public string soundEffectStateSpk
        {
            get
            {
                return this._soundEffectStateSpk;
            }

            set
            {
                if (this._soundEffectStateSpk != value)
                {
                    this._soundEffectStateSpk = value;
                    this.OnPropertyChanged("soundEffectStateSpk");
                }
            }

        }

        private bool _audioEngineBackgroudEnabled;
        public bool audioEngineBackgroudEnabled
        {
            get
            {
                return this._audioEngineBackgroudEnabled;
            }

            set
            {
                if (this._audioEngineBackgroudEnabled != value)
                {
                    this._audioEngineBackgroudEnabled = value;
                    this.OnPropertyChanged("audioEngineBackgroudEnabled");
                }
            }

        }
        public Wme4NetViewModal()
        {
            audioCaptureDeviceListView = new ObservableCollection<DeviceListItemData>();
            audioPlaybackDeviceListView = new ObservableCollection<DeviceListItemData>();
            videoDeviceListView = new ObservableCollection<DeviceListItemData>();
            audioTraceView = new ObservableCollection<LogItemData>();
            aecTypeView = new ObservableCollection<String>();
            aecTypeView.Add("WmeAecTypeNone");
            aecTypeView.Add("WmeAecTypeBuildin");
            aecTypeView.Add("WmeAecTypeWmeDefault");
            aecTypeView.Add("WmeAecTypeTc");
            aecTypeView.Add("WmeAecTypeAlpha");
            audioVolume = 0;
            maxLines = 20000;
            _keyword = "";
            multistreamEnabled = true;
            _audioEngineBackgroudEnabled = false;
        }

        protected void OnPropertyChanged(string propertyName)
        {
            if (PropertyChanged != null)
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
        }
    }

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

    public delegate void AudioVolumeChangedEventHandler(WmeAudioVolumeChangedParam Param);
    public class AudioVolumeChangedDelegate : IAudioVolumeChangedDelegate
    {
        public event AudioVolumeChangedEventHandler OnVolumeChanged;    
        public void OnAudioVolumeChanged(WmeAudioVolumeChangedParam audioVolumeParam)
        {
            if (audioVolumeParam == null)
                return;
            string szMsg = "AudioVolumeChangedDelegate::OnAudioVolumeChanged() iChangeType = ";
            szMsg += audioVolumeParam.iChangedType.ToString();
            szMsg += " iVolumeCrtlType = ";
            szMsg += audioVolumeParam.iVolumeCrtlType.ToString();
            szMsg += " iDeviceType = ";
            szMsg += audioVolumeParam.iDeviceType.ToString();
            szMsg += " bMute = ";
            szMsg += audioVolumeParam.bMute.ToString();
            szMsg += " nVolume = ";
            szMsg += audioVolumeParam.nVolume.ToString();
            szMsg += "\n";
            Console.WriteLine(szMsg);
            if (OnVolumeChanged != null)
                OnVolumeChanged(audioVolumeParam);
        }
    }

    public delegate void MediaDeviceChangedEventHandler(WmeDeviceChangedParam e);
    public class MediaDeviceChangedDelegate : IMediaDeviceChangedDelegate
    {
        public event MediaDeviceChangedEventHandler OnMediaDeviceChanged;
        public void OnDeviceChanged(WmeDeviceChangedParam deviceChangedParam)
        {
            if (deviceChangedParam == null)
                return;
            string szMsg = "MediaDeviceChangedDelegate::OnDeviceChanged()  name = ";
            szMsg += deviceChangedParam.uniqueName;
            szMsg += " ChangedType = ";
            szMsg += deviceChangedParam.iType.ToString();
            szMsg += " SessionType = ";
            szMsg += deviceChangedParam.sessionType.ToString();
            szMsg += " FlowType = ";
            szMsg += deviceChangedParam.flowType.ToString();
            szMsg += "\n";
            Console.WriteLine(szMsg);

            if (OnMediaDeviceChanged != null)
                OnMediaDeviceChanged(deviceChangedParam);           
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
        private Timer mTimer = null;
        private Timer mStressTimer = null;
        private AudioVolumeChangedDelegate mAudioVolumeChangedCallback = null;
        private MediaDeviceChangedDelegate mAudioDeviceChangedDelegate = null;
        private MediaDeviceChangedDelegate mVideoDeviceChangedDelegate = null;
        private bool bLoopbackStarted;
        private Wme4NetViewModal mWme4NetViewModal = null;
        private UInt64 mLogSeq;
        private MediaDevice mCurMic = null;
        private MediaDevice mCurSpk = null;
        private MediaDevice mCurCam = null;

        private MediaConnection mConnLocalBackground = null, mConnRemoteBackground = null;
        private MediaTrack mLocalAudioBackground = null, mRemoteAudioBackground = null;
        private int nBackgroundAudioEngineTestIdx = 0;

        public MainWindow()
        {
            mWme4NetViewModal = new Wme4NetViewModal();
            mWme4NetViewModal.PropertyChanged += mWme4NetViewModal_PropertyChanged;
            this.DataContext = mWme4NetViewModal;
            WmeStatic.initMediaEngine();
            WmeStatic.setTraceLevel(WmeTraceLevel.WME_TRACE_LEVEL_ALL);
            //WmeStatic.setTraceSink(this);
            InitializeComponent();
            WmeAecType.SelectedValue = "WmeAecTypeWmeDefault";
            Closing += OnWindowClosing;
            bLoopbackStarted = false;
            mAudioVolumeChangedCallback = new AudioVolumeChangedDelegate();
            if (mAudioVolumeChangedCallback != null)
            {
                mAudioVolumeChangedCallback.OnVolumeChanged += mAudioVolumeChangedCallback_OnVolumeChanged;
                DeviceManager.Instance.AddAudioVolumeChangedDelegate(FlowType.Render, mAudioVolumeChangedCallback);
            }

            mAudioDeviceChangedDelegate = new MediaDeviceChangedDelegate();
            if (mAudioDeviceChangedDelegate != null)
            {
                mAudioDeviceChangedDelegate.OnMediaDeviceChanged += mAudioDeviceChangedDelegate_OnMediaDeviceChanged;
                DeviceManager.Instance.AddMediaDeviceChangedDelegate(WmeSessionType.WmeSessionType_Audio, FlowType.Capture, mAudioDeviceChangedDelegate);
                DeviceManager.Instance.AddMediaDeviceChangedDelegate(WmeSessionType.WmeSessionType_Audio, FlowType.Render, mAudioDeviceChangedDelegate);
            }
            mVideoDeviceChangedDelegate = new MediaDeviceChangedDelegate();//0:Audio 1: Video
            if (mVideoDeviceChangedDelegate != null)
            {
                mVideoDeviceChangedDelegate.OnMediaDeviceChanged += mAudioDeviceChangedDelegate_OnMediaDeviceChanged;
                DeviceManager.Instance.AddMediaDeviceChangedDelegate(WmeSessionType.WmeSessionType_Video, FlowType.Capture, mVideoDeviceChangedDelegate);
            }

            mWme4NetViewModal.soundEffectStateMic = "Not start";
            mWme4NetViewModal.soundEffectStateSpk = "Not start";
            mLogSeq = 0;
        }

        void mSoundEffectNotificationSink_OnSoundEffectNotification(string szCoreID, FlowType eType, bool bChecked, long lReserver)
        {
            string szMsg = "The option ";
            if (bChecked)
                szMsg += "has been checked!";
            else
                szMsg += "hasn't been checked!";
            if (eType == FlowType.Capture)
                mWme4NetViewModal.soundEffectStateMic = szMsg;
            else
                mWme4NetViewModal.soundEffectStateSpk = szMsg;
            Console.WriteLine("[Callback]Sound Effect State. UniqueName =" 
                + szCoreID 
                + "Flow = " 
                + (eType == FlowType.Capture ? "Capture" : "Playback")
                + "IsChecked = "
                + (bChecked == true ? "Checked" : "UnChecked"));
        }

   

        private void OnWindowClosing(object sender, CancelEventArgs e)
        {
            WmeStatic.setTraceSink(null); 
            if (mStressTimer != null)
                mStressTimer.Close();
            mStressTimer = null;
            StopLoopback();
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

        public void OnMediaStatus(uint mid, uint vid, WmeMediaStatus status, bool hasCSI, uint csi)
        {
            Console.WriteLine("OnMediaStatus mid" + mid.ToString() + " status = " + status.ToString());
        }

        public void OnMediaError(uint mid, uint vid, int errorCode)
        {

        }

        public int OnTrace(uint level, uint threadId, string msg)
        {
            //This is too slow.
            //Console.WriteLine(msg);
            if (level<=2)
                ShowLog(msg, (WbxTraceLevel)level);
            return 0;
        }

        public void OnError(int errorCode)
        {

        }
        public void OnMediaBlocked(uint mid, uint csi, bool blocked)
        {
        }

        public void OnAvailableMediaChanged(uint mid, int count)
        {

        }

        public void OnMediaReady(uint mid, WmeDirection direction, WmeSessionType mediaType, MediaTrack pTrack)
        {
            if (direction == WmeDirection.WmeDirection_Send)
            {
                if (mid > 10)
                    return;
                if (mWme4NetViewModal.audioEngineBackgroudEnabled == true
                    && mid == 9)
                {
                    if (mediaType == WmeSessionType.WmeSessionType_Audio)
                    {
                        mLocalAudioBackground = pTrack;
                        if (mLocalAudioBackground != null)
                        {
                            mLocalAudioBackground.SetCaptureDevice(DeviceManager.Instance.GetSystemDefaultDevice(mediaType,FlowType.Capture));
                        }
                    }
                }
                else
                {
                    if (mediaType == WmeSessionType.WmeSessionType_Audio)
                    {
                        mLocalAudio = pTrack;
                        if (mLocalAudio != null && mCurMic != null)
                            mLocalAudio.SetCaptureDevice(mCurMic);
                    }
                    else if (mediaType == WmeSessionType.WmeSessionType_Video)
                    {
                        mLocalVideo = pTrack;
                        if (mLocalVideo != null && mCurCam != null)
                            mLocalVideo.SetCaptureDevice(mCurCam);
                        //pTrack.AddRenderWindow(mVideoForm.GetSelfView());
                        //pTrack.AddRenderWindow(SelfViewPictureBox.Handle);
                        pTrack.AddImageRender(LocalVideoImage);
                    }
                    else if (mediaType == WmeSessionType.WmeSessionType_ScreenShare)
                    {
                        mLocalShare = pTrack;
                        if (mid == 2)
                            mLocalShare.AddScreenSource(ScreenSourceManager.Instance.defaultSource());
                    }
                }
                pTrack.Start(false);
            }
            else
            {
                if (mid < 10)
                    return;
                if (mWme4NetViewModal.audioEngineBackgroudEnabled == true
                    && mid == 3009)
                {
                    if (mediaType == WmeSessionType.WmeSessionType_Audio)
                    {
                        mRemoteAudioBackground = pTrack;
                        if (mRemoteAudioBackground != null)
                        {
                              mRemoteAudioBackground.SetPlayoutDevice(DeviceManager.Instance.GetSystemDefaultDevice(mediaType, FlowType.Render));
                        }
                    }
                }
                else
                {
                    if (mediaType == WmeSessionType.WmeSessionType_Audio)
                    {
                        mRemoteAudio = pTrack;
                        if (mWme4NetViewModal != null)
                        {
                            mWme4NetViewModal.audioVolume = mRemoteAudio.GetVolume();
                        }
                        if (mRemoteAudio != null && mCurSpk != null)
                            mRemoteAudio.SetPlayoutDevice(mCurSpk);
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
                }
                pTrack.Start(false);
            }
        }

        public void OnSDPReady(WmeSdpType sdpType, string sdp)
        {
            if (sdpType == WmeSdpType.WmeSdp_Offer)
            {
                if (mConnRemote != null)
                {
                    WmeSdpParsedInfo[] medias = mConnRemote.SetReceivedSDP(sdpType, sdp);
                    foreach (WmeSdpParsedInfo mline in medias)
                    {
                        Console.WriteLine(mline.mid);
                    }
                    mConnRemote.CreateAnswer();
                }
                if (mWme4NetViewModal.audioEngineBackgroudEnabled == true)
                {
                    if (mConnRemoteBackground != null)
                    {
                        WmeSdpParsedInfo[] medias1 = mConnRemoteBackground.SetReceivedSDP(sdpType, sdp);
                        foreach (WmeSdpParsedInfo mline in medias1)
                        {
                            Console.WriteLine(mline.mid);
                            if (mline.mid == 3009)
                                mConnRemoteBackground.CreateAnswer();
                        }
                    }
                }
            }
            else
            {
                if (mConnLocal != null)
                {
                    WmeSdpParsedInfo[] medias = mConnLocal.SetReceivedSDP(sdpType, sdp);
                }
                if (mWme4NetViewModal.audioEngineBackgroudEnabled == true)
                {
                    if (mConnLocalBackground != null)
                    {
                        WmeSdpParsedInfo[] medias1 = mConnLocalBackground.SetReceivedSDP(sdpType, sdp);
                    }
                }
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
            if (mConnLocal != null)
            {
                try
                {
                    WmeVideoConnectionStatistics stats = mConnLocal.GetVideoStatistics(0);
                    string localIp = stats.connStat.localIp.ToString();
                    string remoteIp = stats.connStat.remoteIp.ToString();
                }
                catch(Exception err)
                {
                    Console.WriteLine(err.Message);
                }
            }
        }

        private void Button_Click_StartLoopBack(object sender, RoutedEventArgs e)
        {
            StartLoopBack(false);
        }
        private void StartLoopBack(bool appShareEnable = true, bool videoEnable = true, bool audioEnable = true)
        {
           if (MediaOptionPanel != null)
               MediaOptionPanel.IsEnabled = false;
           if (bLoopbackStarted == true)
                return;
            bLoopbackStarted = true;

            mConnLocal = new MediaConnection();
            mConnRemote = new MediaConnection();

            mConnLocal.StartMediaLogging(1000);
            mConnLocal.SetSink(this);
            mConnRemote.SetSink(this);
            int nRet;
            mConnLocal.GetGlobalConfig().EnableMultiStream(mWme4NetViewModal.multistreamEnabled);
            mConnRemote.GetGlobalConfig().EnableMultiStream(mWme4NetViewModal.multistreamEnabled);

            if (videoOption != null && videoOption.IsChecked == true)
            {
                if (mTimer == null)
                    mTimer = new Timer(1000);
                mTimer.Elapsed += new ElapsedEventHandler(Stats_Timer);
                mTimer.Enabled = true; // Enable it
                nRet = mConnLocal.AddMedia(WmeSessionType.WmeSessionType_Video,
                WmeDirection.WmeDirection_SendRecv, 0, "");
                nRet = mConnRemote.AddMedia(WmeSessionType.WmeSessionType_Video,
                WmeDirection.WmeDirection_SendRecv, 1000, "");
                //mConnLocal.GetVideoConfig(0).SetSelectedCodec(WmeCodecType.WmeCodecType_AVC);
                mConnLocal.GetVideoConfig(0).SetEncodeParams(WmeCodecType.WmeCodecType_AVC, 0x42000c,
                    27600, 920, 200, 4000);
                mConnLocal.GetVideoConfig(0).SetEncodeParams(WmeCodecType.WmeCodecType_SVC, 0x42000c,
                    27600, 920, 200, 4000);
            }
            if (screenSharingOption != null && screenSharingOption.IsChecked == true)
            {
                nRet = mConnLocal.AddMedia(WmeSessionType.WmeSessionType_ScreenShare,
                    WmeDirection.WmeDirection_SendRecv, 2, "");
                nRet = mConnRemote.AddMedia(WmeSessionType.WmeSessionType_ScreenShare,
                    WmeDirection.WmeDirection_SendRecv, 2000, "");
            }

            if (audioOption != null && audioOption.IsChecked == true)
            {
                nRet = mConnLocal.AddMedia(WmeSessionType.WmeSessionType_Audio,
                WmeDirection.WmeDirection_SendRecv, 3, "");
                nRet = mConnRemote.AddMedia(WmeSessionType.WmeSessionType_Audio,
                WmeDirection.WmeDirection_SendRecv, 3000, "");
                mConnLocal.GetAudioConfig(3).SetAutoSwitchDefaultMicrophoneDeviceFlag((AutoSwitchDefaultMic.IsChecked == true) ? true : false);
                mConnRemote.GetAudioConfig(3000).SetAutoSwitchDefaultSpeakerDeviceFlag((AutoSwitchDefaultSpeaker.IsChecked == true) ? true : false);
            }
            int aectype = 2;
            switch (WmeAecType.SelectedIndex)
            {
                case 0:
                    aectype = 0;
                    break;
                case 1:
                    aectype = 1;
                    break;
                case 2:
                    aectype = 2;
                    break;
                case 3:
                    aectype = 3;
                    break;
                case 4:
                    aectype = 4;
                    break;
            }

            mConnLocal.GetAudioConfig(3).SetECType(aectype);

            mConnLocal.CreateOffer();   

            if (mWme4NetViewModal.audioEngineBackgroudEnabled == true)
            {
                mConnLocalBackground = new MediaConnection();
                mConnRemoteBackground = new MediaConnection();
                mConnLocalBackground.StartMediaLogging(1000);
                mConnLocalBackground.SetSink(this);
                mConnRemoteBackground.SetSink(this);
                int nRet1 = mConnLocalBackground.AddMedia(WmeSessionType.WmeSessionType_Audio,
                WmeDirection.WmeDirection_SendRecv, 9, "");
                nRet1 = mConnRemoteBackground.AddMedia(WmeSessionType.WmeSessionType_Audio,
                WmeDirection.WmeDirection_SendRecv, 3009, "");
                mConnLocalBackground.GetAudioConfig(9).SetAutoSwitchDefaultMicrophoneDeviceFlag((AutoSwitchDefaultMic.IsChecked == true) ? true : false);
                mConnRemoteBackground.GetAudioConfig(3009).SetAutoSwitchDefaultSpeakerDeviceFlag((AutoSwitchDefaultSpeaker.IsChecked == true) ? true : false);

                mConnLocalBackground.CreateOffer();
            }
        }

        private void StopLoopback()
        {
            if (MediaOptionPanel != null)
                MediaOptionPanel.IsEnabled = true;
            if (bLoopbackStarted == false)
                return;
            bLoopbackStarted = false;
            if (mTimer != null)
                mTimer.Close();
            mTimer = null;
            if (mLocalAudio != null)
                mLocalAudio.Stop();
            mLocalAudio = null;
            if (mRemoteAudio != null)
                mRemoteAudio.Stop();
            mRemoteAudio = null;

            if (mLocalVideo != null)
                mLocalVideo.Stop();
            mLocalVideo = null;
            if (mRemoteVideo != null)
                mRemoteVideo.Stop();
            mRemoteVideo = null;

            if (mLocalShare != null)
                mLocalShare.Stop();
            mLocalShare = null;
            if (mRemoteShare != null)
                mRemoteShare.Stop();
            mRemoteShare = null;

            if (mConnLocal != null)
                mConnLocal.Stop();
            mConnLocal = null;
            if (mConnRemote != null)
                mConnRemote.Stop();
            mConnRemote = null;

            if (mWme4NetViewModal.audioEngineBackgroudEnabled == true)
            {
                if (mConnLocalBackground != null)
                    mConnLocalBackground.Stop();
                mConnLocalBackground = null;
                if (mConnRemoteBackground != null)
                    mConnRemoteBackground.Stop();
                mConnRemoteBackground = null;
            }
        }
        private ScreenSourceCallback mScreenEnum;
        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            mScreenEnum = new ScreenSourceCallback();
            mScreenEnum.listScreen();
        }

        private void Button_Click_LoadDevice(object sender, RoutedEventArgs e)
        {
            LoadAudioDevcie();
            LoadVideoDevcie();
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

        private void LoadAudioDevcie()
        {
            Dispatcher.BeginInvoke((Action)(() =>
            {
            if (mWme4NetViewModal == null)
                return;
            mWme4NetViewModal.audioCaptureDeviceListView.Clear();
            mWme4NetViewModal.audioPlaybackDeviceListView.Clear();

            var mics = DeviceManager.Instance.listDevices(WmeSessionType.WmeSessionType_Audio, FlowType.Capture);
            var spks = DeviceManager.Instance.listDevices(WmeSessionType.WmeSessionType_Audio, FlowType.Render);

            var defaultMic = DeviceManager.Instance.GetSystemDefaultDevice(WmeSessionType.WmeSessionType_Audio, FlowType.Capture);
            var defaultSpeaker = DeviceManager.Instance.GetSystemDefaultDevice(WmeSessionType.WmeSessionType_Audio, FlowType.Render);
            foreach (MediaDevice device in mics)
            {
                DeviceListItemData data = new DeviceListItemData();
                data.mediaDevice = device;
                if (defaultMic.uniqueName == device.uniqueName)
                {
                    data.displayName = device.friendName + "--Default";
                    mWme4NetViewModal.audioCaptureDeviceListView.Add(data);
                    AudioCaptureDevice.SelectedItem = data;
                }
                else
                {
                    data.displayName = device.friendName;
                    mWme4NetViewModal.audioCaptureDeviceListView.Add(data);
                }
            }

            foreach (MediaDevice device in spks)
            {
                DeviceListItemData data = new DeviceListItemData();
                data.mediaDevice = device;
                if (defaultSpeaker.uniqueName == device.uniqueName)
                {
                    data.displayName = device.friendName + "--Default";
                    mWme4NetViewModal.audioPlaybackDeviceListView.Add(data);
                    AudioPlaybackDevice.SelectedItem = data;
                }
                else
                {
                    data.displayName = device.friendName;
                    mWme4NetViewModal.audioPlaybackDeviceListView.Add(data);
                }
            }
            }));
        }
        private void SetAudioVolumeD(uint dwVolume)
        {
            if (DigtialVolumeForSpeaker.IsChecked == true)
            {
                if (mRemoteAudio != null)
                {
                    if (mRemoteAudio.GetVolume(WmeAudioVolumeCtrlType.WME_VOL_DIGITAL) != dwVolume)
                        mRemoteAudio.SetVolume(dwVolume, WmeAudioVolumeCtrlType.WME_VOL_DIGITAL);
                }          
            }

            if (DigtialVolumeForMic.IsChecked == true)
            {
                if (mLocalAudio != null)
                {
                    if (mLocalAudio.GetVolume(WmeAudioVolumeCtrlType.WME_VOL_DIGITAL) != dwVolume)
                        mLocalAudio.SetVolume(dwVolume, WmeAudioVolumeCtrlType.WME_VOL_DIGITAL);
                }
            }
        }
        private void SetAudioVolume(uint dwVolume)
        {
            if (DigtialVolumeForSpeaker.IsChecked == true || DigtialVolumeForMic.IsChecked == true)
            {
                SetAudioVolumeD(dwVolume);
                return;
            }

            if (mRemoteAudio != null)
            {
                if (mRemoteAudio.GetVolume() != dwVolume)
                    mRemoteAudio.SetVolume(dwVolume);
            }
           
        }

        public void OnMediaDeviceChanged(WmeDeviceChangedParam param)
        {
            Dispatcher.BeginInvoke((Action)(() =>
            {
            if (param == null)
                return;

            if (mWme4NetViewModal == null)
                return;
            if (param.sessionType == WmeSessionType.WmeSessionType_Audio)
            {
                if (param.iType == WmeDeviceChangedType.WmeDeviceAdded)
                {
                    if (param.flowType == FlowType.Capture)
                    {
                        var mics = DeviceManager.Instance.listDevices(WmeSessionType.WmeSessionType_Audio, FlowType.Capture);
                        foreach (MediaDevice device in mics)
                        {
                            if (device.uniqueName.CompareTo(param.uniqueName) == 0)
                            {
                                DeviceListItemData data = new DeviceListItemData();
                                data.mediaDevice = device;
                                data.displayName = device.friendName;
                                mWme4NetViewModal.audioCaptureDeviceListView.Add(data);
                                break;
                            }
                        }
                    }
                    else if (param.flowType == FlowType.Render)
                    {
                        var spks = DeviceManager.Instance.listDevices(WmeSessionType.WmeSessionType_Audio, FlowType.Render);
                        foreach (MediaDevice device in spks)
                        {
                            if (device.uniqueName.CompareTo(param.uniqueName) == 0)
                            {
                                DeviceListItemData data = new DeviceListItemData();
                                data.mediaDevice = device;
                                data.displayName = device.friendName;
                                mWme4NetViewModal.audioPlaybackDeviceListView.Add(data);
                                break;
                            }
                        }
                    }
                }
                else if (param.iType == WmeDeviceChangedType.WmeDeviceRemoved)
                {
                    if (param.flowType == FlowType.Capture)
                    {
                        foreach (DeviceListItemData device in mWme4NetViewModal.audioCaptureDeviceListView)
                        {
                            if (device.mediaDevice.uniqueName.CompareTo(param.uniqueName) == 0)
                            {
                                mWme4NetViewModal.audioCaptureDeviceListView.Remove(device);
                                break;
                            }
                        }
                    }
                    else if (param.flowType == FlowType.Render)
                    {
                        foreach (DeviceListItemData device in mWme4NetViewModal.audioPlaybackDeviceListView)
                        {
                            if (device.mediaDevice.uniqueName.CompareTo(param.uniqueName) == 0)
                            {
                                mWme4NetViewModal.audioPlaybackDeviceListView.Remove(device);
                                break;
                            }
                        }
                    }
                }
                else if (param.iType == WmeDeviceChangedType.WmeDefaultDeviceChanged)
                {
                    if (param.flowType == FlowType.Capture)
                    {
                        DeviceListItemData selItem = AudioCaptureDevice.SelectedItem as DeviceListItemData;
                        foreach (DeviceListItemData item in mWme4NetViewModal.audioCaptureDeviceListView)
                        {
                            if (item.mediaDevice.uniqueName.CompareTo(param.uniqueName) == 0)
                            {
                                item.displayName = item.mediaDevice.friendName + "--Default";
                                if (AutoSwitchDefaultMic.IsChecked == true || bLoopbackStarted == false)
                                    selItem = item;
                            }
                            else
                                item.displayName = item.mediaDevice.friendName;
                        }
                        //ComBoBox issue. Have to do below to refresh the data source.
                        AudioCaptureDevice.ItemsSource = null;
                        AudioCaptureDevice.ItemsSource = mWme4NetViewModal.audioCaptureDeviceListView;
                        AudioCaptureDevice.SelectedItem = selItem;
                    }
                    else if (param.flowType == FlowType.Render)
                    {
                        DeviceListItemData selItem = AudioPlaybackDevice.SelectedItem as DeviceListItemData;
                        foreach (DeviceListItemData item in mWme4NetViewModal.audioPlaybackDeviceListView)
                        {
                            if (item.mediaDevice.uniqueName.CompareTo(param.uniqueName) == 0)
                            {
                                item.displayName = item.mediaDevice.friendName + "--Default";
                                if (AutoSwitchDefaultSpeaker.IsChecked == true || bLoopbackStarted == false)
                                    selItem = item;
                            }
                            else
                                item.displayName = item.mediaDevice.friendName;
                        }
                        AudioPlaybackDevice.ItemsSource = null;
                        AudioPlaybackDevice.ItemsSource = mWme4NetViewModal.audioPlaybackDeviceListView;
                        AudioPlaybackDevice.SelectedItem = selItem;
                    }
                }
            }
            else if (param.sessionType == WmeSessionType.WmeSessionType_Video)
            {
                LoadVideoDevcie();
            }
            }));    
        }

        private void LoadVideoDevcie()
        {
            Dispatcher.BeginInvoke((Action)(() =>
            {
            if (mWme4NetViewModal == null)
                return;
            mWme4NetViewModal.videoDeviceListView.Clear();
            var cams = DeviceManager.Instance.listDevices(WmeSessionType.WmeSessionType_Video, FlowType.Capture);
            foreach (MediaDevice device in cams)
            {
                DeviceListItemData data = new DeviceListItemData();
                data.mediaDevice = device;
                data.displayName = device.friendName;
                mWme4NetViewModal.videoDeviceListView.Add(data);
            }
            if (mWme4NetViewModal.videoDeviceListView.Count() > 0)
                VideoCapDevice.SelectedIndex = 0;
            }));
        }

        private void OnSelectedAudioCapDeviceChanged(object sender, SelectionChangedEventArgs e)
        {
            DeviceListItemData item = (AudioCaptureDevice.SelectedItem) as DeviceListItemData;
            if (item == null)
                return;

            if (mCurMic == null)
            {
                if (mLocalAudio != null)
                    mLocalAudio.SetCaptureDevice(item.mediaDevice);
            }
            else
            {
                if (mLocalAudio != null)
                {
                    if (item.mediaDevice == null)
                        mLocalAudio.SetCaptureDevice(item.mediaDevice);
                    else if ((item.mediaDevice == mCurMic) == false)
                        mLocalAudio.SetCaptureDevice(item.mediaDevice);
                }
            }
            mCurMic = item.mediaDevice;
            if (mCurMic != null)
            {
                int nRet = DeviceManager.Instance.GetSoundEffectState(mCurMic, 0);
                if (nRet == 0)
                {
                    mWme4NetViewModal.soundEffectStateMic = "'Disable all enhancements' is unchecked.";
                }
                else if (nRet == 1)
                {
                    mWme4NetViewModal.soundEffectStateMic = "'Disable all enhancements' is checked.";
                }
                else
                {
                    mWme4NetViewModal.soundEffectStateMic = "Not detected";
                }
            }
        }

        private void OnSelectedAudioPlyDeviceChanged(object sender, SelectionChangedEventArgs e)
        {
            DeviceListItemData item = (AudioPlaybackDevice.SelectedItem) as DeviceListItemData;
            if (item == null)
                return;

            if (mCurSpk == null)
            {
                if (mRemoteAudio != null)
                    mRemoteAudio.SetPlayoutDevice(item.mediaDevice);
            }
            else
            {
                if (mRemoteAudio != null)
                {
                    if (item.mediaDevice == null)
                        mRemoteAudio.SetPlayoutDevice(item.mediaDevice);
                    else if ((item.mediaDevice == mCurSpk) == false)
                        mRemoteAudio.SetPlayoutDevice(item.mediaDevice);
                }
            }
            mCurSpk = item.mediaDevice;
            int nRet = DeviceManager.Instance.GetSoundEffectState(mCurSpk, 0);
            if (nRet == 0)
            {
                mWme4NetViewModal.soundEffectStateSpk = "'Disable all enhancements' is unchecked.";
            }
            else if (nRet == 1)
            {
                mWme4NetViewModal.soundEffectStateSpk = "'Disable all enhancements' is checked.";
            }
            else
            {
                mWme4NetViewModal.soundEffectStateSpk = "Not detected";
            }
        }

        private void OnSelectedVideoDeviceChanged(object sender, SelectionChangedEventArgs e)
        {
            DeviceListItemData item = (VideoCapDevice.SelectedItem) as DeviceListItemData;
            if (item == null)
                return;

            if (mCurCam == null)
            {
                if (mLocalVideo != null)
                    mLocalVideo.SetCaptureDevice(item.mediaDevice);
            }
            else
            {
                if (mLocalVideo != null)
                {
                    if (item.mediaDevice == null)
                        mLocalVideo.SetCaptureDevice(item.mediaDevice);
                    else if ((item.mediaDevice == mCurCam) == false)
                        mLocalVideo.SetCaptureDevice(item.mediaDevice);
                }
            }
            mCurCam = item.mediaDevice;
        }
        private void Window_Initialized(object sender, EventArgs e)
        {
            LoadAudioDevcie();
            LoadVideoDevcie();
        }

        void mAudioDeviceChangedDelegate_OnMediaDeviceChanged(WmeDeviceChangedParam deviceChangedParam)
        {
            if (deviceChangedParam == null || mWme4NetViewModal == null)
                return;

            OnMediaDeviceChanged(deviceChangedParam);
 
            
            StringBuilder szMsg =new StringBuilder("[OnMediaDeviceChanged][");
            if (deviceChangedParam.sessionType == WmeSessionType.WmeSessionType_Audio)
                szMsg.Append("Audio");
            else if (deviceChangedParam.sessionType == WmeSessionType.WmeSessionType_Video)
                szMsg.Append("Video");

            szMsg.Append("][");
            szMsg.Append(deviceChangedParam.uniqueName);
            szMsg.Append("][");
            szMsg.Append(deviceChangedParam.iType.ToString());
            szMsg.Append("][");
            szMsg.Append(deviceChangedParam.sessionType.ToString());
            szMsg.Append("][");
            szMsg.Append(deviceChangedParam.flowType.ToString());
            szMsg.Append("]");
            ShowLog(szMsg.ToString(), WbxTraceLevel.WBX_TRACE_INTERNAL);          
       }

        private void mWme4NetViewModal_PropertyChanged(object sender, PropertyChangedEventArgs e)
        {
            if (sender == null || e == null)
                return;
            switch (e.PropertyName)
            {
                case "audioVolume":
                    {
                        Wme4NetViewModal modal = sender as Wme4NetViewModal;
                        if (modal != null)
                            SetAudioVolume(modal.audioVolume);
                    }
                    break;
            }
        }
        private void mAudioVolumeChangedCallback_OnVolumeChanged(WmeAudioVolumeChangedParam Param)
        {
            if (Param == null)
                return;
            if (mWme4NetViewModal == null)
                return;
      
            StringBuilder szMsg = new StringBuilder ("[OnVolumeChanged][");
            szMsg.Append(Param.iChangedType.ToString());
            szMsg.Append("][");
            szMsg.Append(Param.iVolumeCrtlType.ToString());
            szMsg.Append("][");
            szMsg.Append(Param.iDeviceType.ToString());
            szMsg.Append("][");
            szMsg.Append(Param.bMute.ToString());
            szMsg.Append("][");
            szMsg.Append(Param.nVolume.ToString());
            szMsg.Append("]");
            ShowLog(szMsg.ToString(), WbxTraceLevel.WBX_TRACE_INTERNAL);
        }

        private void ShowLog(string szMsg, WbxTraceLevel level = WbxTraceLevel.WBX_TRACE_INFO)
        {
            if (szMsg == null)
                return;
            Dispatcher.BeginInvoke((Action)(() =>
            {
                if (mWme4NetViewModal.audioTraceView.Count >= mWme4NetViewModal.maxLines)
                {
                    for (int i = 0; i <= (mWme4NetViewModal.maxLines / 10); i++)
                        mWme4NetViewModal.audioTraceView.RemoveAt(0);
                }
                if (ShowAllTrace.IsChecked == false)
                {
                    if (level >= WbxTraceLevel.WBX_TRACE_INFO && level != WbxTraceLevel.WBX_TRACE_INTERNAL)
                        return;
                }
                if (mWme4NetViewModal.keyWord != null && mWme4NetViewModal.keyWord.Length > 0 )
                {
                    string szBackup = szMsg.ToLower();
                    if (szBackup.IndexOf(mWme4NetViewModal.keyWord.ToLower(), 0) == -1)
                        return;
                }
                LogItemData data = new LogItemData { wbxLevel = level, wbxLogInfo = szMsg, seqNum = mLogSeq};
                mWme4NetViewModal.audioTraceView.Add(data);
                mLogSeq++;
                if (LogOutput != null)
                {
                    int nIndex = mWme4NetViewModal.audioTraceView.Count() - 1;
                    LogOutput.ScrollIntoView(LogOutput.Items[nIndex > 0 ? nIndex : 0]);
                }
            }));
        }

        private void ClearLog_Click(object sender, RoutedEventArgs e)
        {
            if (mWme4NetViewModal == null)
                return;
            mWme4NetViewModal.audioTraceView.Clear();
        }

        private void MuteAudio_Click(object sender, RoutedEventArgs e)
        {
            if (mLocalAudio != null)
                mLocalAudio.Mute();
        }

        private void UnmuteAudio_Click(object sender, RoutedEventArgs e)
        {
            if (mLocalAudio != null)
                mLocalAudio.Unmute();
        }

        private void MuteSpeaker_Click(object sender, RoutedEventArgs e)
        {
            if (mRemoteAudio != null)
                mRemoteAudio.Mute();
        }

        private void UnmuteSperker_Click(object sender, RoutedEventArgs e)
        {
            if (mRemoteAudio != null)
                mRemoteAudio.Unmute();
        }

        private void Button_Click_Stop(object sender, RoutedEventArgs e)
        {
            StopLoopback();
        }

        private void Button_Click_StressTest(object sender, RoutedEventArgs e)
        {
            if (sender == null)
                return;

  
            System.Windows.Controls.Primitives.ToggleButton stressbtn = sender as System.Windows.Controls.Primitives.ToggleButton;
            if (stressbtn == null)
                return;
            if (stressbtn.IsChecked == true)
            {
                AutoSwitchDefaultMic.IsEnabled = false;
                AutoSwitchDefaultSpeaker.IsEnabled = false;

                if (MediaOptionPanel != null)
                    MediaOptionPanel.IsEnabled = false;

                if (mStressTimer == null)
                {
                    if (mWme4NetViewModal.audioEngineBackgroudEnabled == true)
                    {
                        StartLoopBack();
                        mStressTimer = new Timer(4000);
                    }
                    else
                        mStressTimer = new Timer(15000);
                }
                mStressTimer.Elapsed += new ElapsedEventHandler(Stress_Timer);
                mStressTimer.Enabled = true; // Enable it
            }
            else
            {
                if (MediaOptionPanel != null)
                    MediaOptionPanel.IsEnabled = true;
                if (mStressTimer != null)
                    mStressTimer.Close();
                mStressTimer = null;
                StopLoopback();
                AutoSwitchDefaultMic.IsEnabled = true;
                AutoSwitchDefaultSpeaker.IsEnabled = true;

            }
        }

        private void Stress_Timer(object sender, ElapsedEventArgs e)
        {
            if (mWme4NetViewModal != null && mWme4NetViewModal.audioEngineBackgroudEnabled == true)
            {
                if (bLoopbackStarted == true)
                {
                    Dispatcher.BeginInvoke((Action)(() =>
                    {
                        if (mWme4NetViewModal == null)
                            return;

                        int nCapCount = mWme4NetViewModal.audioCaptureDeviceListView.Count;
                        if (nBackgroundAudioEngineTestIdx < nCapCount)
                        {
                            AudioCaptureDevice.SelectedIndex = nBackgroundAudioEngineTestIdx;
                            ShowLog("[Test for train. Switch device with background audio engine]---Switch microphone...", WbxTraceLevel.WBX_TRACE_INTERNAL);
                        }
                        else
                        {
                            int nPlyCount = mWme4NetViewModal.audioPlaybackDeviceListView.Count;
                            if (nBackgroundAudioEngineTestIdx - nCapCount < nPlyCount)
                            {
                                AudioPlaybackDevice.SelectedIndex = (nBackgroundAudioEngineTestIdx - nCapCount);
                                ShowLog("[Test for train. Switch device with background audio engine]---Switch speaker...", WbxTraceLevel.WBX_TRACE_INTERNAL);
                            }
                            else
                            {
                                nBackgroundAudioEngineTestIdx = 0;
                                ShowLog("[Test for train. Switch device with background audio engine]---Test all device. Reset device index...", WbxTraceLevel.WBX_TRACE_INTERNAL);
                                return;
                            }
                        }
                        nBackgroundAudioEngineTestIdx++;
                    }));
                }
                else
                {
                    Dispatcher.BeginInvoke((Action)(() =>
                    {
                       StartLoopBack();
                    }));
                    ShowLog("[Test for train. Switch device with background audio engine]---Start...", WbxTraceLevel.WBX_TRACE_INTERNAL);
                }
            }
            else
            {
                if (bLoopbackStarted == true)
                {
                    Dispatcher.BeginInvoke((Action)(() =>
                    {
                        StopLoopback();
                    }));
                    ShowLog("[Stress test]---Stop...", WbxTraceLevel.WBX_TRACE_INTERNAL);
                }
                else
                {
                    Dispatcher.BeginInvoke((Action)(() =>
                    {
                        StartLoopBack((screenSharingOption.IsChecked == true), (videoOption.IsChecked == true), (audioOption.IsChecked == true));
                    }));

                    ShowLog("[Stress test]---Start...", WbxTraceLevel.WBX_TRACE_INTERNAL);
                }
            }
        }

        private void AttachWmeTrace_Click(object sender, RoutedEventArgs e)
        {
            if (AttachWmeTrace == null)
                return;
            if (AttachWmeTrace.IsChecked == true)
            {
                ShowAllTrace.IsEnabled = true;
                WmeStatic.setTraceSink(this);
            }
            else
            {
                ShowAllTrace.IsEnabled = false;
                WmeStatic.setTraceSink(null);
            }
        }

        private void CopyToClipBoard_Click(object sender, RoutedEventArgs e)
        {
            if (sender == null)
                return;

            if (mWme4NetViewModal == null)
                return;

            if (mWme4NetViewModal.audioTraceView == null)
                return;

            if (mWme4NetViewModal.audioTraceView.Count <= 0)
                return;

            StringBuilder infobuilder = new StringBuilder();
            foreach (LogItemData data in mWme4NetViewModal.audioTraceView)
            {
                if (data!= null)
                {
                    infobuilder.Append(data.seqNum.ToString());
                    infobuilder.Append("\t");
                    infobuilder.Append(data.timeStamp);
                    infobuilder.Append("\t");
                    infobuilder.Append(data.wbxLogInfo);
                    infobuilder.Append("\t");
                    infobuilder.Append(data.wbxLevel.ToString());
                    infobuilder.Append("\r\n");
                }
            }
            if (infobuilder.Length>0)
                Clipboard.SetDataObject(infobuilder.ToString(),true);
        }

        private void LogOutput_MouseDoubleClick(object sender, MouseButtonEventArgs e)
        {
            if (sender == null)
                return;
            ListViewItem item = sender as ListViewItem;
            if (item == null)
                return;

            LogItemData data = item.DataContext as LogItemData;
            if (data == null)
                return;
            StringBuilder infobuilder = new StringBuilder();
            infobuilder.Append(data.seqNum.ToString());
            infobuilder.Append("\t");
            infobuilder.Append(data.timeStamp);
            infobuilder.Append("\t");
            infobuilder.Append(data.wbxLogInfo);
            infobuilder.Append("\t");
            infobuilder.Append(data.wbxLevel.ToString());
            infobuilder.Append("\r\n");
            if (infobuilder.Length > 0)
                Clipboard.SetDataObject(infobuilder.ToString(), true);
        }

        private void OnAutoSwitchDefaultMic(object sender, RoutedEventArgs e)
        {
            if (sender == null)
                return;
            if (mConnLocal != null)
            {
                if (AutoSwitchDefaultMic.IsChecked == true
                    && AutoSwitchDefaultSpeaker.IsChecked == true)
                    mConnLocal.GetAudioConfig(3).SetAutoSwitchDefaultDeviceFlagEx(true);
                mConnLocal.GetAudioConfig(3).SetAutoSwitchDefaultMicrophoneDeviceFlag((AutoSwitchDefaultMic.IsChecked == true) ? true : false);
            }
        }

        private void OnAutoSwitchDefaultSpeaker(object sender, RoutedEventArgs e)
        {
            if (sender == null)
                return;
            if (mConnRemote != null)
            {
                if (AutoSwitchDefaultMic.IsChecked == true
                    && AutoSwitchDefaultSpeaker.IsChecked == true)
                    mConnRemote.GetAudioConfig(3000).SetAutoSwitchDefaultDeviceFlagEx(true);
                mConnRemote.GetAudioConfig(3000).SetAutoSwitchDefaultSpeakerDeviceFlag((AutoSwitchDefaultSpeaker.IsChecked == true) ? true : false);
            }
        }

        private void OnSoundProperty(object sender, RoutedEventArgs e)
        {
            System.Diagnostics.Process.Start("rundll32.exe", "shell32.dll, Control_RunDLL mmsys.cpl, , 0");
       }
    }
}
