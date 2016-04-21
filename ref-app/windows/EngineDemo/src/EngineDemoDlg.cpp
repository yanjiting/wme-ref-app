// EngineDemoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EngineDemo.h"
#include "EngineDemoDlg.h"
#include <AtlBase.h>

#ifdef ENABLE_COMMAND_LINE
#include <winsock.h>
#endif

#include "DemoParameters.h"
#include "string"
#include "resource.h"
#include "WmeDataDump.h"
#include "DumppingDataFlagDlg.h"

#ifdef ENABLE_COMMAND_LINE
#include "stunlib.h"
#define WM_ASYNC_EVT           (WM_USER + 3)
#endif


//extern DemoClient * retGlobalClietDemo();
//extern void setGlobalClietDemo(DemoClient * pDemoClient);

#define STATISTICS_REFRESH_INTERVAL 500

#define ProcessRetAndReturnNULL(ret, pErrStr) \
	if(ret != WME_S_OK) {\
	MessageBox(pErrStr);\
	return;}

#define ProcessRet(ret, pErrStr) \
	if(ret != WME_S_OK) {\
	MessageBox(pErrStr);}

#define ProcessRetAndReturnValue(ret1, pErrStr, ret2) \
	if(ret1 != WME_S_OK) {\
	MessageBox(pErrStr);\
	return ret2;}

TCHAR gClassNameInterWindow[] = _T("INTERWINDOW");

typedef struct
{
	WmeVideoRawType type;
	const TCHAR *string;
}FORMAT_MAP_INFO;

const static FORMAT_MAP_INFO kFormatMapInfo[] = {
	{WmeVideoUnknown,    _T("Unknown")},
	{WmeI420,            _T("I420")   },
	{WmeYV12,            _T("YV12")   },
	{WmeYUY2,            _T("YUY2")   },
	{WmeRGB24,           _T("RGB24")   },
	{WmeBGR24,           _T("BGR24" )  },
	{WmeRGB24Flip,       _T("RGB24Flip")},
	{WmeBGR24Flip,       _T("BGR24Flip")},
	{WmeRGBA32,          _T("RGBA32")   },
	{WmeBGRA32,          _T("BGRA32")   },
	{WmeARGB32,          _T("ARGB32")   },
	{WmeABGR32,          _T("ABGR32")   },
	{WmeRGBA32Flip,      _T("RGBA32Flip")},
	{WmeBGRA32Flip,      _T("BGRA32Flip")},
	{WmeARGB32Flip,      _T("ARGB32Flip")},
	{WmeABGR32Flip,      _T("ABGR32Flip")},
};


LRESULT CALLBACK backInterWindow(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

//	DemoClient * pDemoClient = NULL;
//	HWND tmpWnd = hWnd;
	HWND *pWnd = new HWND;
	*pWnd = hWnd;

	switch (message) 
	{
	case WM_CLOSE:
//		pDemoClient = retGlobalClietDemo();
//		if (pDemoClient)
//			pDemoClient->DeleteWindowHandle(hWnd);

		theApp.m_pMainWnd->SendMessage(WM_MESSAGE_CLOSE_VIDEO_WINDOW, (UINT_PTR)(pWnd), 0);
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_MOVE:
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_SIZE:
		theApp.m_pMainWnd->SendMessage(WM_MESSAGE_VIDEO_WINDOW_POSITION_CHANGE, (UINT_PTR)(pWnd), 0);
		return DefWindowProc(hWnd, message, wParam, lParam);

	case WM_DISPLAYCHANGE:
		theApp.m_pMainWnd->SendMessage(WM_MESSAGE_VIDEO_WINDOW_DISPLAY_CHANGE, 0, 0);
		return DefWindowProc(hWnd, message, wParam, lParam);
		
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}



static std::string GetCurrentLibPath()
{
	std::string tempPath;
	tempPath.resize(MAX_PATH);
	GetModuleFileNameA(NULL, (char*)tempPath.c_str(), tempPath.size());
	size_t found = tempPath.rfind('\\');
	if (found != std::string::npos) {
		tempPath.replace(found, std::string::npos, "\\");
	}
	return tempPath;
}

#ifndef ENABLE_COMMAND_LINE
CEngineDemoDlg::CEngineDemoDlg(CWnd* pParent /*=NULL*/)
#else
CEngineDemoDlg::CEngineDemoDlg(CWnd* pParent /*=NULL*/, CCustomCommandLineInfo* clInitInfo)
#endif
: CDialog(CEngineDemoDlg::IDD, pParent)
	, m_nVideoWidth(640)
	, m_nVideoHeight(360)
	, m_nVideoFPS(15)
	, m_strVideoFileName(_T(""))
	, m_uFrameRate(0)
	, m_nJinglePort(0)
	, m_nStunPort(0)
	, m_strJingleIP(_T(""))
	, m_strStunIP(_T(""))
	, m_strMyName(_T(""))
	, m_strHostName(_T(""))
	, m_pStatisticsDlg(NULL)
	, m_nTimerInterval(STATISTICS_REFRESH_INTERVAL)
	, m_senderWindowHandle(0)
	, m_receiverWindowHandle(0)
	, m_receiverScreenWindowHandle(0)
	, m_previewerWindowHandle(0)
	, m_bUseWindowLessRender(false)
	, m_eMaxTraceLevel(WME_TRACE_LEVEL_ALL)
	, m_eVideoConnectionStatus(CONNECTION_STATUS_DISABLED)
	, m_eAudioConnectionStatus(CONNECTION_STATUS_DISABLED)
#ifdef ENABLE_COMMAND_LINE
	, logger(NULL)
	, clInfo(NULL)
#endif
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_mapVideoCodecName[WmeCodecType_AVC] = _T("AVC");
	m_mapVideoCodecName[WmeCodecType_SVC] = _T("SVC");
	m_mapVideoCodecName[WmeCodecType_HEVC] = _T("HEVC");
	m_mapVideoCodecName[WmeCodecType_VP8] = _T("VP8");
	m_mapVideoCodecName[WmeCodecType_Unknown] = _T("Unknown");

	m_mapAudioCodecName[WmeCodecType_Unknown] = _T("Unknown");
	m_mapAudioCodecName[WmeCodecType_G711_ULAW] = _T("G711_ULAW");
	m_mapAudioCodecName[WmeCodecType_G711_ALAW] = _T("G711_ALAW");
	m_mapAudioCodecName[WmeCodecType_ILBC] = _T("ILBC");
	m_mapAudioCodecName[WmeCodecType_OPUS] = _T("OPUS");
	m_mapAudioCodecName[WmeCodecType_G722] = _T("G722");
	m_mapAudioCodecName[WmeCodecType_CNG] = _T("CNG");
#ifdef TA_ENABLE
	m_pTAManager = NULL;
#endif

#ifdef ENABLE_COMMAND_LINE
	clInfo = clInitInfo;
#ifdef STUNPOC
	m_ServerSocket = INVALID_SOCKET;
#endif
#endif

	m_uDumpFlag = ((((unsigned int)0x1)<<WME_DATA_DUMP_VIDEO_ENCODE_RTP_LAYER) | 
					(((unsigned int)0x1)<<WME_DATA_DUMP_VIDEO_NAL_TO_LISTEN_CHANNEL)|
					(((unsigned int)0x1)<<WME_DATA_DUMP_VIDEO_RAW_CAPTURE)|
					(((unsigned int)0x1)<<WME_DATA_DUMP_VIDEO_NAL_TO_DECODER)|
					(((unsigned int)0x1)<<WME_DATA_DUMP_VIDEO_RAW_AFTER_DECODE_TO_RENDER)					
					);

}

CEngineDemoDlg::~CEngineDemoDlg()
{

}

void CEngineDemoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_COMBO_CAMERA_LIST, m_ComboBoxCameraList);
	DDX_Control(pDX, IDC_COMBO_LAYER, m_ComboBoxResolution);

	DDX_Control(pDX, IDC_COMBO_MIC_LIST, m_ComboBoxMicList);
	DDX_Control(pDX, IDC_COMBO_AUDIO_PARAM, m_ComboBoxAudioParam);
	DDX_Control(pDX, IDC_COMBO_SPEAKER_LIST2, m_ComboBoxSpeakerList);
	DDX_Text(pDX, IDC_EDIT_JINGLE_PORT, m_nJinglePort);
	DDV_MinMaxInt(pDX, m_nJinglePort, 1, 65535);
	DDX_Text(pDX, IDC_EDIT_STUN_PORT, m_nStunPort);
	DDV_MinMaxInt(pDX, m_nStunPort, 1, 65535);
	DDX_Text(pDX, IDC_EDIT_JINGLE_SERVER, m_strJingleIP);
	DDV_MaxChars(pDX, m_strJingleIP, 16);
	DDX_Text(pDX, IDC_EDIT_STUN_SERVER, m_strStunIP);
	DDV_MaxChars(pDX, m_strStunIP, 16);
	DDX_Text(pDX, IDC_EDIT_MY_NAME, m_strMyName);
	DDX_Text(pDX, IDC_EDIT_HOST_NAME, m_strHostName);
	DDX_Control(pDX, IDC_SLIDER_CAPTURE_VOLUME, m_SlicerCtrlCaptureVolume);
	DDX_Control(pDX, IDC_SLIDER_PLAY_VOLUME, m_SlicerCtrlPlayVolume);
	DDX_Control(pDX, IDC_COMBO_CAMERA_CAPABILITY, m_ComboBoxCameraCapability);
	DDX_Text(pDX, IDC_EDIT_VIDEO_WIDTH, m_nVideoWidth);
	DDV_MinMaxUInt(pDX, m_nVideoWidth, 1, 3840);
	DDX_Text(pDX, IDC_EDIT_VIDEO_HEIGHT, m_nVideoHeight);
	DDV_MinMaxUInt(pDX, m_nVideoHeight, 1, 2160);
	DDX_Text(pDX, IDC_EDIT_VIDEO_FPS, m_nVideoFPS);
	DDV_MinMaxFloat(pDX, m_nVideoFPS, 1, 50);
	DDX_Control(pDX, IDC_COMBO_VIDEO_FORMAT, m_ComboBoxVideoFormat);
	DDX_Text(pDX, IDC_EDIT_VIDEO_FILE, m_strVideoFileName);
	DDX_Control(pDX, IDC_COMBO_SCREEN_SOURCE_LIST, m_ComboBoxScreenSourceList);
}

BEGIN_MESSAGE_MAP(CEngineDemoDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_WM_DESTROY()


	ON_BN_CLICKED(IDC_AS_HOST, &CEngineDemoDlg::OnBnClickedAsHost)
	ON_BN_CLICKED(IDC_AS_RECEIVER, &CEngineDemoDlg::OnBnClickedAsReceiver)

	ON_BN_CLICKED(IDC_CONNECT, &CEngineDemoDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_DISCONNECT, &CEngineDemoDlg::OnBnClickedDisConnect)


//	ON_BN_CLICKED(IDC_VIDEO_STOP, &CEngineDemoDlg::OnBnClickedVideoStop)

//	ON_BN_CLICKED(IDC_AUDIO_STOP, &CEngineDemoDlg::OnBnClickedAudioStop)

	ON_CBN_SELCHANGE(IDC_COMBO_CAMERA_LIST, &CEngineDemoDlg::OnCbnSelchangeComboCameraList)
	ON_BN_CLICKED(IDC_CHECK_SEND_VIDEO, &CEngineDemoDlg::OnBnClickedCheckSendVideo)
	ON_BN_CLICKED(IDC_CHECK_SEND_AUDIO, &CEngineDemoDlg::OnBnClickedCheckSendAudio)
	ON_CBN_SELCHANGE(IDC_COMBO_LAYER, &CEngineDemoDlg::OnCbnSelchangeComboLayer)
	ON_CBN_SELCHANGE(IDC_COMBO_MIC_LIST, &CEngineDemoDlg::OnCbnSelchangeComboMicList)
	ON_CBN_SELCHANGE(IDC_COMBO_AUDIO_PARAM, &CEngineDemoDlg::OnCbnSelchangeComboAudioParam)
	ON_CBN_SELCHANGE(IDC_COMBO_SPEAKER_LIST2, &CEngineDemoDlg::OnCbnSelchangeComboSpeakerList2)
	ON_BN_CLICKED(IDC_CHECK_NAT_TRANS, &CEngineDemoDlg::OnBnClickedCheckNatTrans)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_BUTTON_PLAY_VOLUME_MUTE, &CEngineDemoDlg::OnBnClickedButtonPlayVolumeMute)
	ON_BN_CLICKED(IDC_BUTTON_CAPTURE_VOLUME_MUTE, &CEngineDemoDlg::OnBnClickedButtonCaptureVolumeMute)
	ON_BN_CLICKED(IDC_BUTTON_PREVIEW, &CEngineDemoDlg::OnBnClickedButtonPreview)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_CHECK_VIEW_STATISTICS, &CEngineDemoDlg::OnBnClickedCheckViewStatistics)
	ON_MESSAGE(WM_MESSAGE_STATISTICS_WINDOW_CLOSE,&CEngineDemoDlg::OnStatisticsWindowClosed)
	ON_MESSAGE(WM_MESSAGE_CLOSE_VIDEO_WINDOW,&CEngineDemoDlg::OnRenderWindowClosed)
	ON_MESSAGE(WM_MESSAGE_VIDEO_WINDOW_POSITION_CHANGE,&CEngineDemoDlg::OnRenderWindowPositionChanged)
	ON_MESSAGE(WM_MESSAGE_VIDEO_WINDOW_DISPLAY_CHANGE,&CEngineDemoDlg::OnDiaplayResolutionChanged)

	ON_BN_CLICKED(IDC_RADIO_FILL, &CEngineDemoDlg::OnBnClickedRadioFill)
	ON_BN_CLICKED(IDC_RADIO_LETTERBOX, &CEngineDemoDlg::OnBnClickedRadioLetterbox)
	ON_BN_CLICKED(IDC_RADIO_CROPFILL, &CEngineDemoDlg::OnBnClickedRadioCropfill)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_EC, &CEngineDemoDlg::OnBnClickedCheckEnableEc)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_VAD, &CEngineDemoDlg::OnBnClickedCheckEnableVad)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_NS, &CEngineDemoDlg::OnBnClickedCheckEnableNs)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_AGC, &CEngineDemoDlg::OnBnClickedCheckEnableAgc)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_AUTODROPDATA, &CEngineDemoDlg::OnBnClickedCheckEnableAutodropdata)
	ON_CBN_SELCHANGE(IDC_COMBO_CAMERA_CAPABILITY, &CEngineDemoDlg::OnCbnSelchangeComboCameraCapability)
#ifdef ENABLE_COMMAND_LINE
	//ON_MESSAGE(WM_ASYNC_EVT, &CEngineDemoDlg::OnAsyncEvent)
#endif

	ON_BN_CLICKED(IDC_CHECK_DUMP_DATA, &CEngineDemoDlg::OnBnClickedCheckDumpData)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_VIDEO, &CEngineDemoDlg::OnBnClickedCheckEnableVideo)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_AUDIO, &CEngineDemoDlg::OnBnClickedCheckEnableAudio)
	ON_BN_CLICKED(IDC_RADIO_USE_VIDEO_FILE, &CEngineDemoDlg::OnBnClickedRadioUseVideoFile)
	ON_BN_CLICKED(IDC_RADIO_USE_CAMERA, &CEngineDemoDlg::OnBnClickedRadioUseCamera)
	ON_BN_CLICKED(IDC_BUTTON_SELECT_VIDEO_FILE, &CEngineDemoDlg::OnBnClickedButtonSelectVideoFile)
	ON_BN_CLICKED(IDC_CHECK_ENABLE_SCREEN, &CEngineDemoDlg::OnBnClickedCheckEnableScreen)
	ON_CBN_SELCHANGE(IDC_COMBO_SCREEN_SOURCE_LIST, &CEngineDemoDlg::OnCbnSelchangeComboScreenSourceList)
	ON_BN_CLICKED(IDC_RADIO_ORIGINAL, &CEngineDemoDlg::OnBnClickedRadioOriginal)
	ON_BN_CLICKED(IDC_RADIO_AS_FILL, &CEngineDemoDlg::OnBnClickedRadioAsFill)
	ON_BN_CLICKED(IDC_RADIO_AS_LETTER_BOX, &CEngineDemoDlg::OnBnClickedRadioAsLetterBox)
	ON_BN_CLICKED(IDC_RADIO_AS_CROP, &CEngineDemoDlg::OnBnClickedRadioAsCrop)
END_MESSAGE_MAP()


BOOL CEngineDemoDlg::OnInitDialog()
{

	CDialog::OnInitDialog();
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);		// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	WNDCLASS wcInter;
	wcInter.style         = 0;
	wcInter.lpfnWndProc   = &backInterWindow;//&backInterWindow;
	wcInter.cbClsExtra    = 0;
	wcInter.cbWndExtra    = 0;
	wcInter.hInstance     = AfxGetInstanceHandle();
	wcInter.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wcInter.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wcInter.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcInter.lpszMenuName  = NULL;
	wcInter.lpszClassName = gClassNameInterWindow;
	//wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);
	if (!RegisterClass(&wcInter))
	{
		return FALSE;
	}
#ifdef TA_ENABLE
	//init TA
	m_pTAManager = new TAManager();
	m_pDemoClient = new DemoClient(m_pTAManager);
	if (NULL == m_pDemoClient)
	{
		return FALSE;
	}
	else
	{
		m_pDemoClient->Init(m_eMaxTraceLevel);
	}

	m_pTAManager->SetDemoClient(m_pDemoClient);

	m_pTAManager->CreateTANetwork();


#else

	UpdateVideoFormatList();
	LoadDefaultValues();	

	((CButton *)GetDlgItem(IDC_CHECK_ENABLE_AUDIO))->SetCheck(1);
	((CButton *)GetDlgItem(IDC_CHECK_ENABLE_VIDEO))->SetCheck(1);
	((CButton *)GetDlgItem(IDC_CHECK_ENABLE_SCREEN))->SetCheck(1);

	OnBnClickedCheckEnableAudio();
	OnBnClickedCheckEnableVideo();
	OnBnClickedCheckEnableScreen();

	m_SlicerCtrlCaptureVolume.SetRange(0, 65535);
	m_SlicerCtrlPlayVolume.SetRange(0, 65535);

	m_pDemoClient = new DemoClient(this);
	if (NULL == m_pDemoClient)
	{
		return FALSE;
	}
	else
	{
		m_pDemoClient->Init(m_eMaxTraceLevel);
		UpdateCameraList();
		UpdateCameraCapabilityList();
		UpdateVideoCapabilitiesList();
		UpdateMicList();
		UpdateSpeakerList();
		UpdateAudioCapabilitiesList();
		UpdateAudioVolume();
		UpdateScreenSourceList();
	}

//	setGlobalClietDemo(m_pDemoClient);


	SetWindowPos(NULL, 900, 500, 0, 0, SWP_NOSIZE);

	((CButton*)GetDlgItem(IDC_AS_HOST))->SetCheck(TRUE);
	((CButton*)GetDlgItem(IDC_AS_RECEIVER))->SetCheck(FALSE);
	OnBnClickedAsHost();
//	m_cFlagHostOrReceiverAtFirst = ORIGINAL_AS_NONE;

	((CButton *)GetDlgItem(IDC_CHECK_NAT_TRANS))->EnableWindow(TRUE);
	((CButton *)GetDlgItem(IDC_CHECK_NAT_TRANS))->SetCheck(0);
	OnBnClickedCheckNatTrans();

	GetDlgItem(IDC_DISCONNECT)->EnableWindow(FALSE);

	((CButton *)GetDlgItem(IDC_CHECK_SEND_VIDEO))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_CHECK_SEND_AUDIO))->SetCheck(TRUE);

	OnBnClickedRadioFill();
	OnBnClickedRadioOriginal();

	((CButton*)(GetDlgItem(IDC_RADIO_USE_CAMERA)))->SetCheck(TRUE);
	OnBnClickedRadioUseCamera();

	UpdateData(FALSE);
	m_bVideoInPreview = false;


	m_nTimerID = SetTimer(10010, m_nTimerInterval, 0);
#endif

#ifdef ENABLE_COMMAND_LINE
	if (clInfo->IsHost()) 
	{
		OnBnClickedAsHost();
		OnBnClickedConnect();
	}
	if (clInfo->IsClient()) {
		//populate the "Host IP Address" with what was passed in on the CLI
		OnBnClickedAsReceiver();
		((CIPAddressCtrl *)GetDlgItem(IDC_REMOTE_RECEIVER_IPADDRESS))->SetWindowText(clInfo->ClientIP());
		OnBnClickedConnect();
	}

	if (clInfo->IsSyslogEnabled()) {
		CString syslogHost = clInfo->SyslogIP();
		const size_t nsw = (syslogHost.GetLength() + 1) * 2;
		char *syslogIP = new char[nsw];
		size_t convertedCharsw = 0;
		wcstombs_s(&convertedCharsw, syslogIP, nsw, syslogHost, _TRUNCATE);
		//MessageBox(CString(syslogIP));
		logger = new Syslog(syslogIP);
		logger->log("New Client Instantiated");
	}
#endif
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEngineDemoDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEngineDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CEngineDemoDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default
	CleanUp();
	CDialog::OnClose();
}

void CEngineDemoDlg::OnDestroy()
{
//	CleanUp();

#ifdef ENABLE_COMMAND_LINE
#ifdef STUNPOC
	//jon
	
	if (m_ServerSocket != INVALID_SOCKET) {
		closesocket(m_ServerSocket);
	}
	WSACleanup();
#endif	
#endif

	CDialog::OnDestroy();
}

void CEngineDemoDlg::CleanUp()
{
#ifdef TA_ENABLE
	if(m_pTAManager)
	{
		delete m_pTAManager;
		m_pTAManager = NULL;
	}
#endif

	if(m_pStatisticsDlg)
	{
		delete m_pStatisticsDlg;
		m_pStatisticsDlg = NULL;
	}

	if(m_nTimerID != 0)
	{
		KillTimer(m_nTimerID);
	}

	int n = m_ComboBoxCameraList.GetCount();

	for(int i=0; i<n; i++)
	{
		DeviceProperty *pDP = (DeviceProperty*)(m_ComboBoxCameraList.GetItemData(i));
		if(pDP)
		{
			if(pDP->dev)
			{
				pDP->dev->Release();
				pDP->dev = NULL;
			}
			delete pDP;
		}
	}
	m_ComboBoxCameraList.Clear();

	n = m_ComboBoxMicList.GetCount();

	for(int i=0; i<n; i++)
	{
		DeviceProperty *pDP = (DeviceProperty*)(m_ComboBoxMicList.GetItemData(i));
		if(pDP)
		{
			if(pDP->dev)
			{
				pDP->dev->Release();
				pDP->dev = NULL;
			}
			delete pDP;
		}
	}
	m_ComboBoxMicList.Clear();

	n = m_ComboBoxSpeakerList.GetCount();
	for(int i=0; i<n; i++)
	{
		DeviceProperty *pDP = (DeviceProperty*)(m_ComboBoxSpeakerList.GetItemData(i));
		if(pDP)
		{
			if(pDP->dev)
			{
				pDP->dev->Release();
				pDP->dev = NULL;
			}
			delete pDP;
		}
	}
	m_ComboBoxSpeakerList.Clear();

	n = m_ComboBoxResolution.GetCount();
	for(int i=0; i<n; i++)
	{
		WmeVideoMediaCapability *pMC = (WmeVideoMediaCapability*)(m_ComboBoxResolution.GetItemData(i));
		if(pMC)
		{
			delete pMC;
		}
	}
	m_ComboBoxResolution.Clear();


	n = m_ComboBoxAudioParam.GetCount();
	for(int i=0; i<n; i++)
	{
		WmeAudioMediaCapability *pMC = (WmeAudioMediaCapability*)(m_ComboBoxAudioParam.GetItemData(i));
		if(pMC)
		{
			delete pMC;
		}
	}
	m_ComboBoxAudioParam.Clear();

	n = m_ComboBoxCameraCapability.GetCount();
	for(int i=0; i<n; i++)
	{
		WmeDeviceCapability *pCC = (WmeDeviceCapability*)(m_ComboBoxCameraCapability.GetItemData(i));
		if(pCC)
		{
			delete pCC;
		}
	}
	m_ComboBoxCameraCapability.Clear();

	if (m_pDemoClient)
	{
		m_pDemoClient->UnInit();
		SAFE_DELETE(m_pDemoClient);
	}

//	setGlobalClietDemo(NULL);
#if 0
	if(m_nTimerID != 0)
	{
		KillTimer(m_nTimerID);
		m_nTimerID = 0;
	}
#endif
}


void CEngineDemoDlg::OnBnClickedProperty()
{
	// TODO: Add your control notification handler code here
	char szTitle[] = "hello";
}

void CEngineDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialog::OnSysCommand(nID, lParam);

}

LRESULT CEngineDemoDlg::OnRenderWindowPositionChanged(WPARAM wParam,LPARAM lParam)
{
	HWND *pWnd = (HWND *)wParam;

	DEMO_TRACK_TYPE ttype = DEMO_LOCAL_TRACK;

	if (m_senderWindowHandle == *pWnd)
	{
		ttype = DEMO_LOCAL_TRACK;
	}
	else if (m_receiverWindowHandle == *pWnd)
	{
		ttype = DEMO_REMOTE_TRACK;
	}
	else if (m_receiverScreenWindowHandle == *pWnd)
	{
		ttype = DEMO_REMOTE_DESKTOP_SHARE_TRACK;
	}
	else if(m_previewerWindowHandle == *pWnd)
	{
		ttype = DEMO_PREVIEW_TRACK;
	}

	if(m_pDemoClient)
	{
		m_pDemoClient->OnRenderWindowPositionChanged(ttype, *pWnd);
	}
	delete pWnd;
	return S_OK;
}

LRESULT CEngineDemoDlg::OnDiaplayResolutionChanged(WPARAM wParam,LPARAM lParam)
{
	if(m_pDemoClient)
	{
		m_pDemoClient->OnRenderingDisplayChanged(DEMO_LOCAL_TRACK);
		m_pDemoClient->OnRenderingDisplayChanged(DEMO_REMOTE_TRACK);
		m_pDemoClient->OnRenderingDisplayChanged(DEMO_PREVIEW_TRACK);
	}
	return S_OK;
}

LRESULT CEngineDemoDlg::OnRenderWindowClosed(WPARAM wParam,LPARAM lParam)
{

	HWND *pWnd = (HWND *)wParam;

	if(!(*pWnd))
		return S_OK;

	if (m_senderWindowHandle == *pWnd)
	{
		m_senderWindowHandle = NULL;
		if(m_pDemoClient)
		{
			m_pDemoClient->OnRenderWindowClosed(DEMO_WINDOW_SENDER);
		}
	}
	else if (m_receiverWindowHandle == *pWnd )
	{
		m_receiverWindowHandle = NULL;
		if(m_pDemoClient)
		{
			m_pDemoClient->OnRenderWindowClosed(DEMO_WINDOW_RECEIVER);
		}
	}
	else if (m_receiverScreenWindowHandle == *pWnd )
	{
		m_receiverScreenWindowHandle = NULL;
		if(m_pDemoClient)
		{
			m_pDemoClient->OnRenderWindowClosed(DEMO_WINDOW_DESKSHARING);
		}
	}
	else if(m_previewerWindowHandle == *pWnd)
	{
		m_previewerWindowHandle = NULL;	
		if(m_pDemoClient)
		{
			m_pDemoClient->OnRenderWindowClosed(DEMO_WINDOW_PREVIEW);
		}
		OnBnClickedButtonPreview();
	}

	delete pWnd;
	return S_OK;
}

LRESULT CEngineDemoDlg::OnStatisticsWindowClosed(WPARAM wParam,LPARAM lParam)
{
	((CButton *)GetDlgItem(IDC_CHECK_VIEW_STATISTICS))->SetCheck(FALSE);
	return S_OK;
}

void CEngineDemoDlg::OnBnClickedAsHost()
{
	m_cFlagHostOrReceiverAtFirst = ORIGINAL_AS_HOST;

	//GetDlgItem(IDC_AS_RECEIVER)->EnableWindow(FALSE);
	((CEdit*)GetDlgItem(IDC_EDIT_HOST_NAME))->EnableWindow(FALSE);
	GetDlgItem(IDC_REMOTE_RECEIVER_IPADDRESS)->EnableWindow(FALSE);

	if(1 == ((CButton*)GetDlgItem(IDC_CHECK_NAT_TRANS))->GetCheck())
	{
		((CEdit*)GetDlgItem(IDC_EDIT_MY_NAME))->EnableWindow(TRUE);	
	}
	else
	{
		((CEdit*)GetDlgItem(IDC_EDIT_MY_NAME))->EnableWindow(FALSE);	
	}

}

void CEngineDemoDlg::OnBnClickedAsReceiver()
{
	m_cFlagHostOrReceiverAtFirst = ORIGINAL_AS_RECEIVER;

	//GetDlgItem(IDC_AS_HOST)->EnableWindow(FALSE);
	if(0 == ((CButton*)GetDlgItem(IDC_CHECK_NAT_TRANS))->GetCheck())
	{
		GetDlgItem(IDC_REMOTE_RECEIVER_IPADDRESS)->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_MY_NAME))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_HOST_NAME))->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_REMOTE_RECEIVER_IPADDRESS)->EnableWindow(FALSE);	
		((CEdit*)GetDlgItem(IDC_EDIT_MY_NAME))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_HOST_NAME))->EnableWindow(TRUE);
	}


}

#ifdef ENABLE_COMMAND_LINE
#ifdef STUNPOC
LRESULT CEngineDemoDlg::OnAsyncEvent(WPARAM wParam, LPARAM lParam) {
	int nSelectError = (int)WSAGETSELECTERROR(lParam),
		nSelectEvent = (int)WSAGETSELECTEVENT(lParam),
		nLastError, rc, alen = sizeof (SOCKADDR_IN);

	IN_ADDR aa;
	SOCKADDR_IN ra;
	BOOL bFreeSession = FALSE;

	u_short rhwnd;
#define MSGLEN 1500
	char rbuffer[MSGLEN];
	memset(rbuffer, 0, sizeof(rbuffer));

	if (!nSelectError) {
		switch (nSelectEvent) {
		case FD_READ:
			memset(&ra, 0, alen);

			rc = recvfrom(m_ServerSocket, rbuffer, MSGLEN, 0, (LPSOCKADDR)& ra, &alen);

			//MessageBox(_T("Received something on my socket"));
			//POCTODO decode STUN
			IStunMessage recvStun;
			recvStun.Clear();
			if (recvStun.Decode(rbuffer, rc)) {
				//looks like a valid STUN packet
				
				E_StunMessageMethod stunMethod = e_StunMsgMethodCiscoMedia;
				if (STUN_GET_METHOD(recvStun.GetMethod()) == stunMethod) {
					//keep looking
					

					E_StunMessageAttributeID attrID = e_StunMsgAttrData; // e_StunMsgAttrCiscoMediaFlowActions;
					//const VEC_ATTRTLVS *bla = recvStun.GetAttrList();

					/*
					for (CONST_ITER_ATTRTLVS i = bla->begin(); i != bla->end(); ++i) {
						logger->log("veciter type:%d", i->m_eaType);
					}*/

					StunAttrBinary *binAttr = (StunAttrBinary *)recvStun.GetAttribute(attrID);

					if (binAttr != NULL) {
						//we got something we're interested in
						//now let's pull the string out
						CiscoMediaFlowInfo *cmfInfo = (CiscoMediaFlowInfo *)binAttr->data;
						char *cmfActions = cmfInfo->data;
						std::string actions(cmfActions);
						if (actions.compare(0, 4, "ACRP") == 0) {
							//MessageBox(_T("Got an admission reply from STUN"));
							if (actions.compare(6, 2, "OK") == 0) {
								if (actions.compare(8, 3, "mbr") == 0) {
									//we got a max bit rate constraint. for now we abort the media stream on this
									logger->log("Received a CiscoMediaFlow STUN admission reply with a bitrate constraint. Terminating.");
									OnBnClickedDisConnect();
								}
								else {
									//everything's cool
									logger->log("Received a CiscoMediaFlowSTUN admission reply of OK. Continuing.");
								}
							}
							else {
								//there was an admission reply but it wasn't OK
								logger->log("Received a CiscoMediaFlow STUN admission reply but the value is not OK. Terminating.");
								OnBnClickedDisConnect();
							}
						}
						else {
							logger->log("Received a unknown CiscoMediaFlowAction STUN message:%s mbr=%lu", cmfActions, cmfInfo->maxbitrate);
						}
					}
					
					else {
						//MessageBox(_T("Got a  STUN packet with CiscoMedia but no MediaFlowActions Attribute"));
						logger->log("STUN received without MediaFlowActions attribute. Looking for attrid:%d rc:%d attrs:%d", attrID, rc, recvStun.GetAttrsLength());
						
							//logger->log("attr ID of 0th attr:%d", bla->at(0).m_eaID);
						
					}
					
				}
				
				else {
					//MessageBox(_T("Received a STUN packet, but it's not CiscoMedia"));
					logger->log("STUN received of method:%d type:%d compmethod:%d", recvStun.GetMethod(), recvStun.GetType(), stunMethod );
				}
				
			}
			else {
				//garbage
				logger->log("Received a packet but it does not seem to be a STUN packet.");
			}
			/*
			if (rc == MSGLEN) {

				rbuffer[MSGLEN] = 0x00;
				memcpy(&aa, &ra.sin_addr.s_addr, 4);
				
				TRACE("Addr %d.%d.%d.%d port %d\n",
					aa.S_un.S_un_b.s_b1, aa.S_un.S_un_b.s_b2,
					aa.S_un.S_un_b.s_b3, aa.S_un.S_un_b.s_b4, ntohs(ra.sin_port));
					
			}
			*/
			
			break;
			/*
		case FD_WRITE:
		case FD_CLOSE:
			break;
			*/
		}
	}
	else {
		nLastError = WSAGetLastError();
	}
	return 0;
}
#endif
#endif
long CEngineDemoDlg::InitVideoClient()
{
	if(!m_pDemoClient)
	{
		return WME_E_FAIL;
	}

	if(1 == ((CButton*)GetDlgItem(IDC_RADIO_USE_VIDEO_FILE))->GetCheck())
	{
		int nVideoFormatIndex = m_ComboBoxVideoFormat.GetCurSel();

		WmeVideoRawFormat rawFormat;
		rawFormat.eRawType = (wme::WmeVideoRawType)(nVideoFormatIndex + 1);
		rawFormat.fFrameRate = m_nVideoFPS;
		rawFormat.iWidth = m_nVideoWidth;
		rawFormat.iHeight = m_nVideoHeight;
		rawFormat.uTimestamp = 0;

		char szFileName[256]; 
		memset(szFileName, 0, sizeof(szFileName));
		int nLength = WideCharToMultiByte(CP_ACP, 0, m_strVideoFileName.GetBuffer(), -1, NULL, 0, NULL, NULL);  
		WideCharToMultiByte(CP_ACP, 0, m_strVideoFileName.GetBuffer(), -1,szFileName, nLength, NULL, NULL);   


		m_pDemoClient->SetVideoInputFile(szFileName ,&rawFormat);
		m_pDemoClient->SetVideoSource(1);
	}
	else
	{
		m_pDemoClient->SetVideoSource(0);
	}

	//Create video client
	long ret = m_pDemoClient->CreateMediaClient(DEMO_MEDIA_VIDEO);

	if(WME_S_OK != ret)
	{
		MessageBox(_T("Failed to create video client!"));
		return WME_E_FAIL;
	}


	if(1 == ((CButton*)GetDlgItem(IDC_RADIO_USE_CAMERA))->GetCheck())
	{
		//set video parameters
		OnCbnSelchangeComboCameraList();
		OnCbnSelchangeComboCameraCapability();
	}

	OnCbnSelchangeComboLayer();

	//Win PC default video 720p, and high performance
	m_pDemoClient->SetVideoQuality(DEMO_LOCAL_TRACK, WmeVideoQuality_HD_720P);
	m_pDemoClient->SetStaticPerformance(WmePerformanceProfileHigh);


	//create and set video render window
	CreateRenderWindow(DEMO_WINDOW_SENDER);
	CreateRenderWindow(DEMO_WINDOW_RECEIVER);

	ret = m_pDemoClient->SetRenderView(DEMO_LOCAL_TRACK, m_senderWindowHandle, m_bUseWindowLessRender);
	ProcessRet(ret, _T("Failed to call m_pDemoClient->SetRenderView(DEMO_LOCAL_TRACK,...)"));

	ret = m_pDemoClient->SetRenderView(DEMO_REMOTE_TRACK, m_receiverWindowHandle, m_bUseWindowLessRender);
	ProcessRet(ret, _T("Failed to call m_pDemoClient->SetRenderView(DEMO_REMOTE_TRACK,...)"));


	ret = SetRenderMode();
	//start video track
	ret = m_pDemoClient->StartMediaTrack(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK);
	ProcessRet(ret, _T("Failed to call m_pDemoClient->StartMediaTrack(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK)"));


	ret = m_pDemoClient->StartMediaTrack(DEMO_MEDIA_VIDEO, DEMO_REMOTE_TRACK);
	ProcessRet(ret, _T("Failed to call m_pDemoClient->StartMediaTrack(DEMO_MEDIA_VIDEO, DEMO_REMOTE_TRACK)"));


	//check whether need to send video
	bool bStartVideo = true;
	if (0 == ((CButton*)GetDlgItem(IDC_CHECK_SEND_VIDEO))->GetCheck())
		bStartVideo = false;

	m_pDemoClient->EnableMyMedia(DEMO_MEDIA_VIDEO, bStartVideo);
	if(!bStartVideo)
	{
		m_pDemoClient->StopMediaSending(DEMO_MEDIA_VIDEO);
	}

	return ret;
}

long CEngineDemoDlg::InitAudioClient()
{
	if(!m_pDemoClient)
	{
		return WME_E_FAIL;
	}

	//create audio client
	long ret = m_pDemoClient->CreateMediaClient(DEMO_MEDIA_AUDIO);

	if(WME_S_OK != ret)
	{
		MessageBox(_T("Failed to create audio client!"));
		return ret;
	}

	//set audio paramters
	OnCbnSelchangeComboMicList();
	OnCbnSelchangeComboAudioParam();
	OnCbnSelchangeComboSpeakerList2();

	OnBnClickedCheckEnableEc();
	OnBnClickedCheckEnableVad();
	OnBnClickedCheckEnableNs();
	OnBnClickedCheckEnableAgc();
	OnBnClickedCheckEnableAutodropdata();
	//start audio track
	ret = m_pDemoClient->StartMediaTrack(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK);
	ProcessRet(ret, _T("Failed to call m_pDemoClient->StartMediaTrack(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK)"));

	ret = m_pDemoClient->StartMediaTrack(DEMO_MEDIA_AUDIO, DEMO_REMOTE_TRACK);
	ProcessRet(ret, _T("Failed to call m_pDemoClient->StartMediaTrack(DEMO_MEDIA_AUDIO, DEMO_REMOTE_TRACK)"));

	//check whether need to send audio data
	bool bStartAudio = true;
	if (0 == ((CButton*)GetDlgItem(IDC_CHECK_SEND_AUDIO))->GetCheck())
		bStartAudio = false;

	m_pDemoClient->EnableMyMedia(DEMO_MEDIA_AUDIO, bStartAudio);
	if(!bStartAudio)
	{
		m_pDemoClient->StopMediaSending(DEMO_MEDIA_AUDIO);
	}

	return ret;
}



long CEngineDemoDlg::ConnectMediaDirectly(DEMO_MEDIA_TYPE mType, bool bHost,char *pServerIP)
{
	if(!m_pDemoClient)
	{
		return WME_E_FAIL;
	}

	if(!bHost && !pServerIP)
	{
		return WME_E_INVALIDARG;
	}

	long ret = WME_S_OK;

	if(bHost)
	{
		ret = m_pDemoClient->InitHost(mType);
		ProcessRetAndReturnValue(ret, _T("Failed to call m_pDemoClient->InitHost(DEMO_MEDIA_VIDEO)"), ret);

		if(mType == DEMO_MEDIA_AUDIO)
		{
			m_eAudioConnectionStatus = CONNECTION_STATUS_WAITING;
		}
		if(mType == DEMO_MEDIA_VIDEO)
		{
			m_eVideoConnectionStatus = CONNECTION_STATUS_WAITING;
		}
	}
	else
	{
		ret = m_pDemoClient->ConnectRemote(mType, pServerIP);
		ProcessRetAndReturnValue(ret, _T("Failed to call m_pDemoClient->ConnectRemote(DEMO_MEDIA_VIDEO,...)"), ret);
	
		if(mType == DEMO_MEDIA_AUDIO)
		{
			m_eAudioConnectionStatus = CONNECTION_STATUS_CONNECTING;
		}
		if(mType == DEMO_MEDIA_VIDEO)
		{
			m_eVideoConnectionStatus = CONNECTION_STATUS_CONNECTING;
		}

	}

	return ret;
}

long CEngineDemoDlg::ConnectMediaThroughICE(DEMO_MEDIA_TYPE mType, bool bHost, const char* pMyName, const char *pHostName, char *pJingleIP, int nJinglePort, char *pStunIP, int nStunPort)
{
	if(!m_pDemoClient || !pMyName || !pJingleIP || !pStunIP)
	{
		return WME_E_INVALIDARG;
	}

	if(nStunPort <= 0 || nStunPort > 65535 || nJinglePort <= 0 || nJinglePort > 65535)
	{
		return WME_E_INVALIDARG;
	}

	if(!bHost && !pHostName)
	{
		return WME_E_INVALIDARG;
	}

	long ret = WME_S_OK;

	//init network connection
	if (bHost)
	{
		ret = m_pDemoClient->InitHost(mType, pMyName, pJingleIP, nJinglePort,
			pStunIP, nStunPort);		
		ProcessRetAndReturnValue(ret, _T("Failed to call m_pDemoClient->InitHost(DEMO_MEDIA_VIDEO,...)"), ret);

		if(mType == DEMO_MEDIA_AUDIO)
		{
			m_eAudioConnectionStatus = CONNECTION_STATUS_WAITING;
		}
		if(mType == DEMO_MEDIA_VIDEO)
		{
			m_eVideoConnectionStatus = CONNECTION_STATUS_WAITING;
		}
	}
	else 
	{

		ret = m_pDemoClient->ConnectRemote(mType, pMyName, pHostName, pJingleIP, nJinglePort,
			pStunIP, nStunPort);
		ProcessRetAndReturnValue(ret, _T("Failed to call m_pDemoClient->ConnectRemote(DEMO_MEDIA_VIDEO,...)"), ret);

		if(mType == DEMO_MEDIA_AUDIO)
		{
			m_eAudioConnectionStatus = CONNECTION_STATUS_CONNECTING;
		}
		if(mType == DEMO_MEDIA_VIDEO)
		{
			m_eVideoConnectionStatus = CONNECTION_STATUS_CONNECTING;
		}
	}

	return ret;
}

long CEngineDemoDlg::UninitVideoClient()
{
	long ret = WME_S_OK;
	if (m_pDemoClient)
	{
		ret = m_pDemoClient->StopMediaTrack(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK);
		ProcessRetAndReturnValue(ret, _T("Failed to call m_pDemoClient->StopMediaTrack(...)"), ret);

		ret = m_pDemoClient->StopMediaTrack(DEMO_MEDIA_VIDEO, DEMO_REMOTE_TRACK);
		ProcessRetAndReturnValue(ret, _T("Failed to call m_pDemoClient->StopMediaTrack(...)"), ret);

		CloseRenderWindow(m_senderWindowHandle);
		CloseRenderWindow(m_receiverWindowHandle);
		m_pDemoClient->DeleteMediaClient(DEMO_MEDIA_VIDEO);
	}

	return ret;
}

long CEngineDemoDlg::UninitAudioClient()
{
	long ret = WME_S_OK;

	if (m_pDemoClient)
	{
		ret = m_pDemoClient->StopMediaTrack(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK);
		ProcessRetAndReturnValue(ret, _T("Failed to call m_pDemoClient->StopMediaTrack(...)"), ret);

		ret = m_pDemoClient->StopMediaTrack(DEMO_MEDIA_AUDIO, DEMO_REMOTE_TRACK);
		ProcessRetAndReturnValue(ret, _T("Failed to call m_pDemoClient->StopMediaTrack(...)"), ret);


		m_pDemoClient->DeleteMediaClient(DEMO_MEDIA_AUDIO);
	}
	return ret;
}

long CEngineDemoDlg::DisconnectMedia(DEMO_MEDIA_TYPE mType)
{
	if(!m_pDemoClient)
	{
		return WME_E_FAIL;
	}

	long ret = m_pDemoClient->DisConnect(mType);

	if(ret == WME_S_OK)
	{
		if(mType == DEMO_MEDIA_AUDIO)
		{
			m_eAudioConnectionStatus = CONNECTION_STATUS_DISABLED;
		}
		if(mType == DEMO_MEDIA_VIDEO)
		{
			m_eVideoConnectionStatus = CONNECTION_STATUS_DISABLED;
		}
	}

	return ret;
}

void CEngineDemoDlg::OnBnClickedConnect()
{
	// TODO: Add your control notification handler code here

	UpdateData(TRUE);

	if(!CheckValues())
	{
		return;
	}

	SaveCurrentValues();

	if(!m_pDemoClient)
	{
		return;
	}

	if(m_bVideoInPreview)
	{
		OnBnClickedButtonPreview();
	}

	long ret = WME_S_OK;
	bool bHost = true;

	if (ORIGINAL_AS_RECEIVER == m_cFlagHostOrReceiverAtFirst)
	{
		bHost = false;
	}

	bool bUseICE = false;
	if (1 == ((CButton*)GetDlgItem(IDC_CHECK_NAT_TRANS))->GetCheck())
	{
		bUseICE = true;
	}

	char szHostIP[128], szJingleIP[64], szStunIP[64], szMyName[64], szHostName[64];

	memset(szHostIP, 0, sizeof(szHostIP));
	memset(szJingleIP, 0, sizeof(szJingleIP));
	memset(szStunIP, 0, sizeof(szStunIP));
	memset(szMyName, 0, sizeof(szMyName));
	memset(szHostName, 0, sizeof(szHostName));

	int nLength = 0;


	if(!bUseICE)
	{
		if(!bHost)
		{
			CIPAddressCtrl *pIPAddrCtrl = (CIPAddressCtrl *)GetDlgItem(IDC_REMOTE_RECEIVER_IPADDRESS); 

			if (pIPAddrCtrl->IsBlank())
			{
				MessageBox(_T("Please enter into remote host IP address"));
				return;
			}

			BYTE IP0=0,IP1=0,IP2=0,IP3=0;
			pIPAddrCtrl->GetAddress(IP0, IP1, IP2, IP3);
			sprintf_s(szHostIP, sizeof(szHostIP), "%d.%d.%d.%d", IP0, IP1, IP2, IP3);
		}
	}
	else
	{
		nLength = WideCharToMultiByte(CP_ACP, 0, m_strJingleIP.GetBuffer(), -1, NULL, 0, NULL, NULL);  
		WideCharToMultiByte(CP_ACP, 0, m_strJingleIP.GetBuffer(), -1,szJingleIP, nLength, NULL, NULL);   

		nLength = WideCharToMultiByte(CP_ACP, 0, m_strStunIP.GetBuffer(), -1, NULL, 0, NULL, NULL);  
		WideCharToMultiByte(CP_ACP, 0, m_strStunIP.GetBuffer(), -1,szStunIP, nLength, NULL, NULL);   

		nLength = WideCharToMultiByte(CP_ACP, 0, m_strMyName.GetBuffer(), -1, NULL, 0, NULL, NULL);  
		WideCharToMultiByte(CP_ACP, 0, m_strMyName.GetBuffer(), -1,szMyName, nLength, NULL, NULL); 

		if(!bHost)
		{
			nLength = WideCharToMultiByte(CP_ACP, 0, m_strHostName.GetBuffer(), -1, NULL, 0, NULL, NULL);  
			WideCharToMultiByte(CP_ACP, 0, m_strHostName.GetBuffer(), -1,szHostName, nLength, NULL, NULL);   
		}
	}

	//Create video client
	if(1 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_VIDEO))->GetCheck())
	{
		

		ret = InitVideoClient();

		if(!bUseICE)
		{
			ret = ConnectMediaDirectly(DEMO_MEDIA_VIDEO, bHost, szHostIP);
		}
		else
		{
			ret = ConnectMediaThroughICE(DEMO_MEDIA_VIDEO, bHost, szMyName, szHostName, szJingleIP, m_nJinglePort
				, szStunIP, m_nStunPort);
		}

		//check whether need to send video
		bool bStartVideo = true;
		if (0 == ((CButton*)GetDlgItem(IDC_CHECK_SEND_VIDEO))->GetCheck())
			bStartVideo = false;

		m_pDemoClient->EnableMyMedia(DEMO_MEDIA_VIDEO, bStartVideo);
		if(!bStartVideo)
		{
			m_pDemoClient->StopMediaSending(DEMO_MEDIA_VIDEO);
		}
	}

	//Create Screen client
	if(1 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_SCREEN))->GetCheck())
	{
		ret = InitScreenClient();

		if(!bUseICE)
		{
			ret = ConnectMediaDirectly(DEMO_MEDIA_DESKSHARE, bHost, szHostIP);
		}
		else
		{
			ret = ConnectMediaThroughICE(DEMO_MEDIA_DESKSHARE, bHost, szMyName, szHostName, szJingleIP, m_nJinglePort
				, szStunIP, m_nStunPort);
		}

		//check whether need to share screen
		CheckStartJoiningShareScreen();
	}


	//Create audio client
	if(1 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_AUDIO))->GetCheck())
	{
		ret = InitAudioClient();

		if(!bUseICE)
		{
			ret = ConnectMediaDirectly(DEMO_MEDIA_AUDIO, bHost, szHostIP);
		}
		else
		{
			ret = ConnectMediaThroughICE(DEMO_MEDIA_AUDIO, bHost, szMyName, szHostName, szJingleIP, m_nJinglePort
				, szStunIP, m_nStunPort);
		}

		//check whether need to send audio data
		bool bStartAudio = true;
		if (0 == ((CButton*)GetDlgItem(IDC_CHECK_SEND_AUDIO))->GetCheck())
			bStartAudio = false;

		m_pDemoClient->EnableMyMedia(DEMO_MEDIA_AUDIO, bStartAudio);
		if(!bStartAudio)
		{
			m_pDemoClient->StopMediaSending(DEMO_MEDIA_AUDIO);
		}

	}

	//Enable or disable QoS
	bool bEnableQoS = false;
	if (1 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_QOS))->GetCheck())
	{
		bEnableQoS = true;
	}
	m_pDemoClient->EnableQoS(bEnableQoS);

	bool bDumpData = false;
	if(1 == ((CButton*)GetDlgItem(IDC_CHECK_DUMP_DATA))->GetCheck())
	{
		bDumpData = true;
	}
	m_pDemoClient->SetDumpDataEnabled(bDumpData);


	GetDlgItem(IDC_CONNECT)->EnableWindow(FALSE);
	GetDlgItem(IDC_DISCONNECT)->EnableWindow(TRUE);
	((CButton*)GetDlgItem(IDC_CHECK_NAT_TRANS))->EnableWindow(FALSE);

	EnableControls();

#ifdef ENABLE_COMMAND_LINE
#ifdef STUNPOC
	InitSocketForStun();
#endif
#endif
}


void CEngineDemoDlg::OnBnClickedDisConnect()
{
	CM_INFO_TRACE_THIS("CEngineDemoDlg::OnBnClickedDisConnect");

	if(!m_pDemoClient)
	{
		return;
	}

	long ret = WME_S_OK;

	if(1 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_VIDEO))->GetCheck())
	{
		ret = UninitVideoClient();
		ret = DisconnectMedia(DEMO_MEDIA_VIDEO);
	}

	if(1 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_AUDIO))->GetCheck())
	{
		ret = UninitAudioClient();
		ret = DisconnectMedia(DEMO_MEDIA_AUDIO);
	}

	if(1 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_SCREEN))->GetCheck())
	{
		ret = UninitScreenClient();
		ret = DisconnectMedia(DEMO_MEDIA_DESKSHARE);
	}

	GetDlgItem(IDC_CONNECT)->EnableWindow(TRUE);
	GetDlgItem(IDC_DISCONNECT)->EnableWindow(FALSE);
	((CButton*)GetDlgItem(IDC_CHECK_NAT_TRANS))->EnableWindow(TRUE);

	EnableControls();

#ifdef ENABLE_COMMAND_LINE
#ifdef STUNPOC
		if (m_ServerSocket != INVALID_SOCKET) 
		{
			closesocket(m_ServerSocket);
		}
#endif
#endif
	
}

#ifdef ENABLE_COMMAND_LINE
#ifdef STUNPOC
void CEngineDemoDlg::InitSocketForStun()
{
	//wm_ send STUN admission req, wait for response
	if (clInfo->IsStunPoc() && (m_cFlagHostOrReceiverAtFirst == ORIGINAL_AS_RECEIVER)) {
		MessageBox(_T("STUN POC!"));
		//do our STUN magic
		/*send packets toward destination IP on STUN UDP port 3478
		include our custom messages and attributes
		listen asynchronously for incoming STUN packets*/
		//MessageBox(_T("about to instantiate syslog"));

		//MessageBox(_T("done with syslog"));

		CIPAddressCtrl *pIPAddrCtrl = (CIPAddressCtrl *)GetDlgItem(IDC_REMOTE_RECEIVER_IPADDRESS);
		
		if (pIPAddrCtrl->IsBlank())
		{
			MessageBox(_T("Please enter the remote host IP address"));
			return;
		}

		WORD wVersionRequested = MAKEWORD(2, 2);
		WSADATA wsaData;
		int err;

		err = WSAStartup(wVersionRequested, &wsaData);
		if (err != 0)
		{
			MessageBox(_T("You asked for the WiFi PoC but there was a failure starting winsock. ab2"));
			return;
		}


		m_ServerSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (m_ServerSocket == INVALID_SOCKET) {
			//unable to listen for STUN
			//since the poc flag was set, and this won't work, bail.
			MessageBox(_T("You asked for the WiFi PoC but there was a failure creating a socket. ab3"));
			return;
		}

		struct sockaddr_in addr;
		memset((char*)&(addr), 0, sizeof((addr)));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(3478);

		u_long nonBlocking = TRUE;
		int r = ioctlsocket(m_ServerSocket, FIONBIO, (u_long FAR *) &nonBlocking);

		if (bind(m_ServerSocket, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR)
		{
			int e = errno;
			MessageBox(_T("Error binding server socket"));

			switch (e)
			{
			case EADDRINUSE:
			{
				MessageBox(_T("You asked for the WiFi PoC but there was a failure creating a socket. ab41"));
				//return;
			}
				break;
			case EADDRNOTAVAIL:
			{
				MessageBox(_T("You asked for the WiFi PoC but there was a failure creating a socket. ab42"));
				//return;
			}
				break;
			default:
			{
				MessageBox(_T("You asked for the WiFi PoC but there was a failure creating a socket. ab43"));
				//return;
			}
				break;
			}
		}
		else {
			//MessageBox(_T("WSAAsyncSelect about to run"));
			WSAAsyncSelect(m_ServerSocket, m_hWnd, WM_ASYNC_EVT, FD_READ | FD_WRITE);
			//MessageBox(_T("WSAAsyncSelect completed"));
		}
		if (m_ServerSocket == INVALID_SOCKET) {
			MessageBox(_T("You asked for the WiFi PoC but there was a failure creating a socket. ab5"));
			//return;
		}


		//MessageBox(_T("About to build STUN msg"));

		DWORD destaddr;
		pIPAddrCtrl->GetAddress(destaddr);
		DWORD destaddrnb = htonl(destaddr);

		IStunMessage stunMsg;
		
		E_StunMessageClass stunMC = e_StunMsgClassRequest;
		stunMsg.SetClass(stunMC);
		//MessageBox(_T("2"));
		E_StunMessageMethod stunMT = e_StunMsgMethodCiscoMedia;
		//E_StunMessageMethod stunMT = e_StunMsgMethodBinding;
		stunMsg.SetMethod(stunMT);

		//std::basic_string origString("Test String");
		/*
		std::wstring wstr(L"Test String");
		int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
		std::string msgString(size_needed, 0);
		WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &msgString[0], size_needed, NULL, NULL);
		*/
		E_StunMessageAttributeID attrID = e_StunMsgAttrData; // e_StunMsgAttrCiscoMediaFlowInfo;
		/*
		typedef struct CiscoMediaFlowInfo {
			uint32_t maxbitrate;
			uint32_t avgbitrate;
			uint32_t srcip;
			uint32_t dstip;
			uint16_t srcport;
			uint16_t dstport;
			uint8_t  protocol;
			uint8_t  reserved[3];
			char     data[32];
		} CiscoMediaFlowInfoAttr;
		*/
		CiscoMediaFlowInfoAttr thisflow;
		thisflow.maxbitrate = 2621440;
		thisflow.avgbitrate = 1310720;
		//TODO change to my IP
		thisflow.srcip = INADDR_ANY;
		thisflow.dstip = destaddrnb;
		//TODO can we make this accurate?
		thisflow.srcport = thisflow.dstport = 0;
		thisflow.protocol = IPPROTO_UDP;
		thisflow.reserved[0] = 0;
		thisflow.reserved[1] = 0;
		thisflow.reserved[2] = 0;
		memset(thisflow.data, 0, sizeof(thisflow.data));
		strcpy(thisflow.data, "ACRQ");


		//E_StunMessageAttributeID attrID = e_StunMsgAttrUserName;
		//MessageBox(_T("3"));
		//uint8_t data[] = "testaroux";
		stunMsg.SetAttrBinary(attrID, sizeof(thisflow), (uint8_t *)&thisflow);
		//stunMsg.SetAttrString(e_StunMsgAttrUserName, msgString);
		//MessageBox(_T("4"));
		/*
		uint8_t flowaction[] = "ACRQ";
		attrID = e_StunMsgAttrCiscoMediaFlowActions;
		stunMsg.SetAttrBinary(attrID, sizeof(flowaction), flowaction);
		*/
		stunMsg.SetTransactionID(stunMsg.GenerateTransactionID());

		uint32_t stunMessageLen;
		const char *stunMessage = stunMsg.Encode(stunMessageLen);
		

		struct sockaddr_in to;
		int toLen = sizeof(to);
		memset(&to, 0, toLen);
		to.sin_family = AF_INET;
		to.sin_port = htons(3478);
		to.sin_addr.s_addr = destaddrnb;

		/*
		CString msg;
		msg.Format(_T("About to send STUNmsg with stunMessageLen=%d"), stunMessageLen);
		MessageBox(msg);
		*/

		int s = sendto(m_ServerSocket, stunMessage, stunMessageLen, 0, (sockaddr *)&to, toLen);

		if (s == SOCKET_ERROR)
		{
			int e = errno;
			switch (e)
			{
				case ECONNREFUSED:
				case EHOSTDOWN:
				case EHOSTUNREACH:
				{
								 // quietly ignore this 
				}
					break;
				case EAFNOSUPPORT:
				{
					MessageBox(_T("You asked for the WiFi PoC but there was a failure sending on a socket. ab52"));
				}
					break;
				default:
				{
					MessageBox(_T("You asked for the WiFi PoC but there was a failure creating a socket. ab53"));
					//strerror(e)
				}
			}
			return;
		}
		if (s == 0) {
			MessageBox(_T("You asked for the WiFi PoC but no data was sent when transmitting STUN. ab54"));
			return;
		}
		if (s != stunMessageLen) {
			MessageBox(_T("You asked for the WiFi PoC but the amount of data sent doesn't match the stunMessageLen. ab55"));
			return;
		}

		//MessageBox(_T("OK, STUN PoC mode is pretty much done, cleaning up."));
		//WSACleanup();
	}
}
#endif
#endif

long CEngineDemoDlg::UpdateCameraList()
{
	if(!m_pDemoClient)
	{
		return WME_E_FAIL;
	}


	DemoClient::DevicePropertyList dpList;
	if(m_pDemoClient->GetDeviceList(DEMO_MEDIA_VIDEO, DEV_TYPE_CAMERA, dpList) != WME_S_OK)
	{
		return WME_E_FAIL;
	}

	int nDefaultIndex = 0;
	const int nMaxNameLen = 512;
	WCHAR wcString[nMaxNameLen];
	for(int i=0; i< dpList.size(); i++)
	{
		memset(wcString, 0, sizeof(wcString));
		DeviceProperty *pDP = new DeviceProperty;
		memcpy_s(pDP, sizeof(DeviceProperty), &(dpList.at(i)), sizeof(DeviceProperty));

		if(pDP->is_default_dev == 1)
		{
			nDefaultIndex = i;
		}

		char *p = pDP->dev_name;
		MultiByteToWideChar(CP_UTF8, 0, p, -1, wcString, nMaxNameLen);
		int index = m_ComboBoxCameraList.AddString(wcString);
		m_ComboBoxCameraList.SetItemData(index, (DWORD)pDP);
	}

	m_ComboBoxCameraList.SetCurSel(nDefaultIndex);

	return WME_S_OK;
}

void CEngineDemoDlg::OnCbnSelchangeComboCameraList()
{
	// TODO: Add your control notification handler code here
	if(m_pDemoClient)
	{
		int index = m_ComboBoxCameraList.GetCurSel();
		if(index < 0)
		{
			return;
		}

		DeviceProperty *pDP = (DeviceProperty*)(m_ComboBoxCameraList.GetItemData(index));

#if 0
		DeviceProperty myDP;

		WMERESULT ret = m_pDemoClient->GetDeviceByDevIndex(WmeMediaTypeVideo,WmeDeviceIn,pDP->dev_index, myDP);

		if(ret != WME_S_OK)
		{
			MessageBox(_T("Can't find the device!"));
			delete pDP;
			m_ComboBoxCameraList.DeleteString(index);
			return;
		}
		IWmeMediaDevice* pCamera = myDP.dev;
#endif
		IWmeMediaDevice* pCamera = pDP->dev;
		m_pDemoClient->SetCamera(DEMO_LOCAL_TRACK, pCamera);
	}
}


long CEngineDemoDlg::UpdateVideoCapabilitiesList()
{
	if(!m_pDemoClient)
	{
		return WME_E_FAIL;
	}

	DemoClient::VideoMediaCapabilityList mcList;
	if(m_pDemoClient->GetMediaCapabilities(DEMO_MEDIA_VIDEO, &mcList) != WME_S_OK)
	{
		return WME_E_FAIL;
	}

	int nDefaultIndex = 0, nMaxWidth = 0;

	for(int i=0; i< mcList.size(); i++)
	{

//		memset(wstr, 0, sizeof(wstr));
		WmeVideoMediaCapability *pMC = new WmeVideoMediaCapability;
		memcpy_s(pMC, sizeof(WmeVideoMediaCapability), &(mcList.at(i)), sizeof(WmeVideoMediaCapability));

		float fps = pMC->frame_layer[pMC->frame_layer_number-1]*1.0/100;

		CString str;

//		str.Format(_T("SVC, %dx%d@%.1ffps"), pMC->width, pMC->height, fps);
		str.Format(_T("%s, %dx%d"), m_mapVideoCodecName[pMC->eCodecType].GetString(), pMC->width, pMC->height);

#if 0
		if(pMC->frame_layer_number == 1)
		{
			str.Format(_T("SVC, %dx%d@%.1ffps with %d spacial layer"), pMC->width, pMC->height, fps, pMC->frame_layer_number );
		}
		else
		{
			str.Format(_T("SVC, %dx%d@%.1ffps with %d spacial layers"), pMC->width, pMC->height, fps, pMC->frame_layer_number );		
		}
#endif
		int index = m_ComboBoxResolution.AddString(str);
		m_ComboBoxResolution.SetItemData(index, (DWORD)pMC);

		if(mcList.at(i).eCodecType == WmeCodecType_SVC && pMC->width > nMaxWidth)
		{
			nDefaultIndex = index;
			nMaxWidth = pMC->width;
		}
	}

	m_ComboBoxResolution.SetCurSel(nDefaultIndex);

	return WME_S_OK;

}

long CEngineDemoDlg::UpdateMicList()
{
	if(!m_pDemoClient)
	{
		return WME_E_FAIL;
	}

	DemoClient::DevicePropertyList dpList;
	if(m_pDemoClient->GetDeviceList(DEMO_MEDIA_AUDIO, DEV_TYPE_MIC, dpList) != WME_S_OK)
	{
		return WME_E_FAIL;
	}

	int nDefaultIndex = 0;
	const int nMaxNameLen = 512;
	WCHAR wcString[nMaxNameLen];
	for(int i=0; i< dpList.size(); i++)
	{
		memset(wcString, 0, sizeof(wcString));
		DeviceProperty *pDP = new DeviceProperty;
		memcpy_s(pDP, sizeof(DeviceProperty), &(dpList.at(i)), sizeof(DeviceProperty));

		if(pDP->is_default_dev == 1)
		{
			nDefaultIndex = i;
		}

		char *p = pDP->dev_name;
		MultiByteToWideChar(CP_UTF8, 0, p, -1, wcString, nMaxNameLen);
		int index = m_ComboBoxMicList.AddString(wcString);
		m_ComboBoxMicList.SetItemData(index, (DWORD)pDP);
	}

	m_ComboBoxMicList.SetCurSel(nDefaultIndex);

	return WME_S_OK;
}

long CEngineDemoDlg::UpdateSpeakerList()
{
	if(!m_pDemoClient)
	{
		return WME_E_FAIL;
	}

	DemoClient::DevicePropertyList dpList;
	if(m_pDemoClient->GetDeviceList(DEMO_MEDIA_AUDIO, DEV_TYPE_SPEAKER, dpList) != WME_S_OK)
	{
		return WME_E_FAIL;
	}

	const int nMaxNameLen = 512;
	int nDefaultIndex = 0;
	WCHAR wcString[nMaxNameLen];
	for(int i=0; i< dpList.size(); i++)
	{
		memset(wcString, 0, sizeof(wcString));
		DeviceProperty *pDP = new DeviceProperty;
		memcpy_s(pDP, sizeof(DeviceProperty), &(dpList.at(i)), sizeof(DeviceProperty));
		char *p = pDP->dev_name;
		MultiByteToWideChar(CP_UTF8, 0, p, -1, wcString, nMaxNameLen);

		if(pDP->is_default_dev == 1)
		{
			nDefaultIndex = i;
		}

		int index = m_ComboBoxSpeakerList.AddString(wcString);
		m_ComboBoxSpeakerList.SetItemData(index, (DWORD)pDP);
	}

	m_ComboBoxSpeakerList.SetCurSel(nDefaultIndex);

	return WME_S_OK;
}

long CEngineDemoDlg::UpdateAudioCapabilitiesList()
{
	if(!m_pDemoClient)
	{
		return WME_E_FAIL;
	}
	DemoClient::AudioMediaCapabilityList mcList;
#if 1
	if(m_pDemoClient->GetMediaCapabilities( DEMO_MEDIA_AUDIO, &mcList) != WME_S_OK)
	{
		return WME_E_FAIL;
	}
#else
	if(m_pDemoClient->GetAudioCodecList( mcList) != WME_S_OK)
	{
		return WME_E_FAIL;
	}
#endif

	int nDefaultIndex = 0;
	const int nMaxNameLen = 512;
	WCHAR wcString[nMaxNameLen];

	for(int i=0; i< mcList.size(); i++)
	{
		CString str;
//		memset(wstr, 0, sizeof(wstr));
		WmeAudioMediaCapability *pMC = new WmeAudioMediaCapability;
		memcpy_s(pMC, sizeof(WmeAudioMediaCapability), &(mcList.at(i)), sizeof(WmeAudioMediaCapability));

		memset(wcString, 0, sizeof(wcString));
		MultiByteToWideChar(CP_UTF8, 0, pMC->stdname, -1, wcString, nMaxNameLen);

#if 0
		str.Format(_T("%s, sample freq: %dKHz, bitrate: %dKbps"), wcString, pMC->clockrate/1000, pMC->rate/1000);
#else
		str.Format(_T("%s"), wcString);
#endif
		int index = m_ComboBoxAudioParam.AddString(str);
		m_ComboBoxAudioParam.SetItemData(index, (DWORD)pMC);

		if(pMC->eCodecType == WmeCodecType_OPUS)
		{
			nDefaultIndex = index;	
		}
	}


	m_ComboBoxAudioParam.SetCurSel(nDefaultIndex);
	return WME_S_OK;
}

void CEngineDemoDlg::OnBnClickedCheckSendVideo()
{
	// TODO: Add your control notification handler code here
	if(((CButton*)GetDlgItem(IDC_CHECK_SEND_VIDEO))->GetCheck() == 1)
	{
		if (m_pDemoClient)
		{
			m_pDemoClient->EnableMyMedia(DEMO_MEDIA_VIDEO, true);
			m_pDemoClient->StartMediaSending(DEMO_MEDIA_VIDEO);
		}	
	}
	else
	{
		if (m_pDemoClient)
		{
			m_pDemoClient->EnableMyMedia(DEMO_MEDIA_VIDEO, false);
			m_pDemoClient->StopMediaSending(DEMO_MEDIA_VIDEO);
		}	
	}
}

void CEngineDemoDlg::OnBnClickedCheckSendAudio()
{
	// TODO: Add your control notification handler code here
	if (0 == ((CButton*)GetDlgItem(IDC_CHECK_SEND_AUDIO))->GetCheck())
	{
		if (m_pDemoClient)
		{
			m_pDemoClient->EnableMyMedia(DEMO_MEDIA_AUDIO, false);
			m_pDemoClient->StopMediaSending(DEMO_MEDIA_AUDIO);
		}
	}
	else
	{
		if (m_pDemoClient)
		{
			m_pDemoClient->EnableMyMedia(DEMO_MEDIA_AUDIO, true);
			m_pDemoClient->StartMediaSending(DEMO_MEDIA_AUDIO);
		}
	}
}

void CEngineDemoDlg::OnCbnSelchangeComboLayer()
{
	// TODO: Add your control notification handler code here
	if(!m_pDemoClient)
	{
		return;
	}

	int index = m_ComboBoxResolution.GetCurSel();

	if(index < 0)
	{
		return;
	}

	WmeVideoMediaCapability *pMC = (WmeVideoMediaCapability*)(m_ComboBoxResolution.GetItemData(index));
	
	if(!pMC)
	{
		return;
	}

	if(m_pDemoClient)
	{
		
		m_pDemoClient->SetMediaCodec(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK, pMC->eCodecType);
		m_pDemoClient->SetMediaCapability(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK, pMC);
		m_pDemoClient->SetMediaSession(DEMO_MEDIA_VIDEO, DEMO_LOCAL_TRACK, pMC->eCodecType);

		m_pDemoClient->SetMediaCodec(DEMO_MEDIA_VIDEO, DEMO_REMOTE_TRACK, pMC->eCodecType);
//		m_pDemoClient->SetMediaCapability(DEMO_MEDIA_VIDEO, DEMO_REMOTE_TRACK, pMC);
		m_pDemoClient->SetMediaSession(DEMO_MEDIA_VIDEO, DEMO_REMOTE_TRACK, pMC->eCodecType);
	}
}

void CEngineDemoDlg::OnCbnSelchangeComboMicList()
{
	// TODO: Add your control notification handler code here
	if(!m_pDemoClient)
	{
		return;
	}

	int index = m_ComboBoxMicList.GetCurSel();
	if(index < 0)
	{
		return;
	}
	DeviceProperty *pDP = (DeviceProperty*)(m_ComboBoxMicList.GetItemData(index));

#if 0
	DeviceProperty myDP;

	WMERESULT ret = m_pDemoClient->GetDeviceByDevIndex(WmeMediaTypeAudio,WmeDeviceIn,pDP->dev_index, myDP);

	if(ret != WME_S_OK)
	{
		MessageBox(_T("Can't find the device!"));
		delete pDP;
		m_ComboBoxMicList.DeleteString(index);
		return;
	}

	IWmeMediaDevice* pMic = myDP.dev;
#endif

	IWmeMediaDevice* pMic = pDP->dev;
	m_pDemoClient->SetMic(pMic);

}

void CEngineDemoDlg::OnCbnSelchangeComboAudioParam()
{
	// TODO: Add your control notification handler code here
	if(!m_pDemoClient)
	{
		return;
	}

	int index = m_ComboBoxAudioParam.GetCurSel();
	if(index < 0)
	{
		return;
	}

	WmeAudioMediaCapability *pMC = (WmeAudioMediaCapability*)(m_ComboBoxAudioParam.GetItemData(index));
	
	if(!pMC)
	{
		return;
	}

	m_pDemoClient->SetMediaCodec(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK, pMC->eCodecType);
	m_pDemoClient->SetMediaCapability(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK, pMC);
	m_pDemoClient->SetMediaSession(DEMO_MEDIA_AUDIO, DEMO_LOCAL_TRACK, pMC->eCodecType);

	m_pDemoClient->SetMediaCodec(DEMO_MEDIA_AUDIO, DEMO_REMOTE_TRACK, pMC->eCodecType);
	//m_pDemoClient->SetMediaCapability(DEMO_MEDIA_AUDIO, DEMO_REMOTE_TRACK, pMC);
	m_pDemoClient->SetMediaSession(DEMO_MEDIA_AUDIO, DEMO_REMOTE_TRACK, pMC->eCodecType);
}

void CEngineDemoDlg::OnCbnSelchangeComboSpeakerList2()
{
	// TODO: Add your control notification handler code here
	if(!m_pDemoClient)
	{
		return;
	}

	int index = m_ComboBoxSpeakerList.GetCurSel();
	if(index < 0)
	{
		return;
	}

	DeviceProperty *pDP = (DeviceProperty*)(m_ComboBoxSpeakerList.GetItemData(index));
#if 0
	DeviceProperty myDP;

	WMERESULT ret = m_pDemoClient->GetDeviceByDevIndex(WmeMediaTypeAudio,WmeDeviceOut,pDP->dev_index, myDP);

	if(ret != WME_S_OK)
	{
		MessageBox(_T("Can't find the device!"));
		delete pDP;
		m_ComboBoxSpeakerList.DeleteString(index);
		return;
	}
	IWmeMediaDevice* pSpeaker = myDP.dev;
#endif
	IWmeMediaDevice* pSpeaker = pDP->dev;

	m_pDemoClient->SetSpeaker(pSpeaker);
}


void CEngineDemoDlg::UpdateUsingDevice(DeviceProperty *pDP)
{

	UpdateData(FALSE);

	if(!pDP)
	{
		return;
	}
	


	switch (pDP->dev_type)
	{
	case DEV_TYPE_CAMERA:
		OnCbnSelchangeComboCameraList();
		break;

	case DEV_TYPE_MIC:
		OnCbnSelchangeComboMicList();
		break;

	case DEV_TYPE_SPEAKER:
		OnCbnSelchangeComboSpeakerList2();
		break;

	default:
		return;


	}


	return;
}

CComboBox* CEngineDemoDlg::GetComboBoxByDevice(DeviceProperty *pDP)
{
	if(!pDP)
	{
		return NULL;
	}
	

	CComboBox *pComboBox = NULL;

	switch (pDP->dev_type)
	{
	case DEV_TYPE_CAMERA:
		pComboBox = &m_ComboBoxCameraList;
		break;

	case DEV_TYPE_MIC:
		pComboBox = &m_ComboBoxMicList;
		break;

	case DEV_TYPE_SPEAKER:
		pComboBox = &m_ComboBoxSpeakerList;
		break;

	default:
		pComboBox = NULL;


	}


	return pComboBox;
}

void CEngineDemoDlg::RemoveDevice(DeviceProperty *pDP)
{
	//check which device changed

	CComboBox *pComboBox = GetComboBoxByDevice(pDP);
	
	if(!pComboBox)
	{
		return ;
	}

	int nCount = pComboBox->GetCount();

	int nCurrentSel = pComboBox->GetCurSel();

	bool bNoDevice = false;
	bool bChanged =false;

	for(int j=0; j < nCount; j++)
	{
		DeviceProperty *pDP1 = (DeviceProperty*)(pComboBox->GetItemData(j));
		if(pDP1 && pDP1->dev)
		{
			if(pDP1->dev->IsSameDevice(pDP->unique_name, pDP->unique_name_len) == WME_S_OK)
			{
				//check if this device is in use
				
				if(nCurrentSel == j)
				{
					if(j < nCount-1)
					{
						pComboBox->SetCurSel(j+1);
					}
					else if(j > 0)
					{
						pComboBox->SetCurSel(j-1);
					}
					else
					{
						bNoDevice =  true;
					}
					bChanged = true;
				}
				pDP1->dev->Release();
				delete pDP1;
				pComboBox->DeleteString(j);
				break;
			}
		}
	}

	if(bNoDevice)
	{
		pComboBox->SetCurSel(0);
	}

	if(bChanged)
	{
		UpdateUsingDevice(pDP);
	}

	return;
}
void CEngineDemoDlg::AddDevice(DeviceProperty *pDP)
{
	CComboBox *pComboBox = GetComboBoxByDevice(pDP);
	
	if(!pComboBox)
	{
		return ;
	}

	const int nMaxNameLen = 512;
	WCHAR wcString[nMaxNameLen];

	memset(wcString, 0, sizeof(wcString));
	DeviceProperty *pDP1 = new DeviceProperty;
	memcpy_s(pDP1, sizeof(DeviceProperty), pDP, sizeof(DeviceProperty));

	char *p = pDP1->dev_name;
	MultiByteToWideChar(CP_UTF8, 0, p, -1, wcString, nMaxNameLen);
	int index = pComboBox->AddString(wcString);
	pComboBox->SetItemData(index, (DWORD)pDP1);

	bool bChanged = false;

	if(pComboBox->GetCount() == 1)
	{
		pComboBox->SetCurSel(index);
		bChanged = true;
	}
	else if(pDP1->is_default_dev == 1 && GetDlgItem(IDC_CONNECT)->IsWindowEnabled())
	{
		pComboBox->SetCurSel(index);
		bChanged = true;
	}

	if(	bChanged)
	{
		UpdateUsingDevice(pDP);	
	}

	return;
}

void CEngineDemoDlg::UpdateDefaultDevice(DeviceProperty *pDP)
{
	//check which device changed

	CComboBox *pComboBox = GetComboBoxByDevice(pDP);
	
	if(!pComboBox)
	{
		return ;
	}

	int nCount = pComboBox->GetCount();

	int nCurrentSel = pComboBox->GetCurSel();

	bool bNoDevice = false;
	bool bChanged =false;

	for(int j=0; j < nCount; j++)
	{
		DeviceProperty *pDP1 = (DeviceProperty*)(pComboBox->GetItemData(j));
		if(pDP1 && pDP1->dev)
		{
			if(pDP1->dev->IsSameDevice(pDP->unique_name, pDP->unique_name_len) == WME_S_OK)
			{
				
				pDP1->is_default_dev = 1;

				//check if this device is in use
				if(nCurrentSel != j && GetDlgItem(IDC_CONNECT)->IsWindowEnabled())
				{
					pComboBox->SetCurSel(j);
				}
			}
		}
	}

	return;
}

void CEngineDemoDlg::OnBnClickedCheckNatTrans()
{
	// TODO: Add your control notification handler code here

	if (0 == ((CButton*)GetDlgItem(IDC_CHECK_NAT_TRANS))->GetCheck())
	{
		((CEdit*)GetDlgItem(IDC_EDIT_JINGLE_SERVER))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_STUN_SERVER))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_JINGLE_PORT))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_STUN_PORT))->EnableWindow(FALSE);

	}
	else
	{
		((CEdit*)GetDlgItem(IDC_EDIT_JINGLE_SERVER))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_STUN_SERVER))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_JINGLE_PORT))->EnableWindow(TRUE);
		((CEdit*)GetDlgItem(IDC_EDIT_STUN_PORT))->EnableWindow(TRUE);
	}

	if(1 == ((CButton*)GetDlgItem(IDC_AS_RECEIVER))->GetCheck())
	{
		OnBnClickedAsReceiver();
	}
	else
	{
		OnBnClickedAsHost();		
	}

}

void CEngineDemoDlg::LoadDefaultValues()
{
	std::string strCurrentPath = GetCurrentLibPath();
	strCurrentPath += "Config.ini";

	const int nMaxNameLen = 512;
	WCHAR wcConfig[nMaxNameLen];

	MultiByteToWideChar(CP_UTF8, 0, strCurrentPath.c_str(), -1, wcConfig, nMaxNameLen);

	WCHAR wcString[nMaxNameLen];
	memset(wcString, 0, sizeof(WCHAR)*nMaxNameLen);
	if(GetPrivateProfileString(_T("Host Info"), _T("IP"), NULL, wcString, 64, wcConfig))
	{
		((CIPAddressCtrl *)GetDlgItem(IDC_REMOTE_RECEIVER_IPADDRESS))->SetWindowText(wcString);
	}

	memset(wcString, 0, sizeof(WCHAR)*nMaxNameLen);
	if(GetPrivateProfileString(_T("Network Info"), _T("Jingle IP"), NULL, wcString, 64, wcConfig))
	{
		m_strJingleIP = wcString;

//		((CIPAddressCtrl *)GetDlgItem(IDC_REMOTE_RECEIVER_IPADDRESS))->SetWindowText(wcString);
	}

	memset(wcString, 0, sizeof(WCHAR)*nMaxNameLen);
	if(GetPrivateProfileString(_T("Network Info"), _T("Stun IP"), NULL, wcString, 64, wcConfig))
	{
		m_strStunIP = wcString;
//		((CIPAddressCtrl *)GetDlgItem(IDC_REMOTE_RECEIVER_IPADDRESS))->SetWindowText(wcString);
	}

	m_nJinglePort = GetPrivateProfileInt(_T("Network Info"), _T("Jingle Port"), 5050, wcConfig);
	m_nStunPort = GetPrivateProfileInt(_T("Network Info"), _T("Stun Port"), 3478, wcConfig);

	memset(wcString, 0, sizeof(WCHAR)*nMaxNameLen);
	if(GetPrivateProfileString(_T("Network Info"), _T("My Name"), NULL, wcString, 64, wcConfig))
	{
		m_strMyName = wcString;
	}

	memset(wcString, 0, sizeof(WCHAR)*nMaxNameLen);
	if(GetPrivateProfileString(_T("Network Info"), _T("Host Name"), NULL, wcString, 64, wcConfig))
	{
		m_strHostName = wcString;
	}

	int nUseWindowLessRender = GetPrivateProfileInt(_T("Render Info"), _T("UseWindowlessRender"), 0, wcConfig);
	if(nUseWindowLessRender == 0)
	{
		m_bUseWindowLessRender = false;
	}
	else
	{
		m_bUseWindowLessRender = true;	
	}

	int nMaxTraceLevel = GetPrivateProfileInt(_T("Trace Level"), _T("TraceLevel"), WME_TRACE_LEVEL_ALL, wcConfig);

	if(nMaxTraceLevel>= (int)WME_TRACE_LEVEL_NOTRACE && nMaxTraceLevel <= (int)WME_TRACE_LEVEL_ALL)
	{
		m_eMaxTraceLevel = (WmeTraceLevel)nMaxTraceLevel;
	}

	memset(wcString, 0, sizeof(WCHAR)*nMaxNameLen);
	if(GetPrivateProfileString(_T("Video File"), _T("File Name"), NULL, wcString, nMaxNameLen, wcConfig))
	{
		m_strVideoFileName = wcString;
	}

	m_nVideoWidth = GetPrivateProfileInt(_T("Video File"), _T("Video Width"), 640, wcConfig);
	m_nVideoHeight = GetPrivateProfileInt(_T("Video File"), _T("Video Height"), 360, wcConfig);
	m_nVideoFPS = GetPrivateProfileInt(_T("Video File"), _T("Video FPS"), 15, wcConfig);


	int nIndex = GetPrivateProfileInt(_T("Video File"), _T("Video Format"), 0, wcConfig);

	m_ComboBoxVideoFormat.SetCurSel(nIndex);

}

void CEngineDemoDlg::SaveCurrentValues()
{
	std::string strCurrentPath = GetCurrentLibPath();
	strCurrentPath += "Config.ini";

	const int nMaxNameLen = 512;
	WCHAR wcConfig[nMaxNameLen];

	MultiByteToWideChar(CP_UTF8, 0, strCurrentPath.c_str(), -1, wcConfig, nMaxNameLen);


	if (0 == ((CButton*)GetDlgItem(IDC_CHECK_NAT_TRANS))->GetCheck())
	{
		if(1 == ((CButton*)GetDlgItem(IDC_AS_RECEIVER))->GetCheck())
		{
	//		WCHAR wcString[64];
			CString strIP;
			((CIPAddressCtrl *)GetDlgItem(IDC_REMOTE_RECEIVER_IPADDRESS))->GetWindowText(strIP);	

	//		::SendMessage(g_hEdit_OutputFile_Path, WM_GETTEXT, LEN_OF_BUFFER, (LPARAM)wcString);
			WritePrivateProfileString(_T("Host Info"), _T("IP"), (LPCTSTR)strIP, wcConfig);
		}

	}
	else
	{
		WritePrivateProfileString(_T("Network Info"), _T("Jingle IP"), (LPCTSTR)m_strJingleIP,wcConfig);

		CString strTmp;
		strTmp.Format(_T("%d"), m_nJinglePort);
		WritePrivateProfileString(_T("Network Info"), _T("Jingle Port"), (LPCTSTR)strTmp, wcConfig);

		WritePrivateProfileString(_T("Network Info"), _T("Stun IP"), (LPCTSTR)m_strStunIP, wcConfig);

		strTmp.Format(_T("%d"), m_nStunPort);
		WritePrivateProfileString(_T("Network Info"), _T("Stun Port"), (LPCTSTR)strTmp, wcConfig);

		WritePrivateProfileString(_T("Network Info"), _T("My Name"), (LPCTSTR)m_strMyName, wcConfig);
		WritePrivateProfileString(_T("Network Info"), _T("Host Name"), (LPCTSTR)m_strHostName, wcConfig);

	}

	if(1 == ((CButton*)GetDlgItem(IDC_RADIO_USE_VIDEO_FILE))->GetCheck())
	{
		WritePrivateProfileString(_T("Video File"), _T("File Name"), (LPCTSTR)m_strVideoFileName,wcConfig);

		CString strTmp;
		strTmp.Format(_T("%d"), m_nVideoWidth);
		WritePrivateProfileString(_T("Video File"), _T("Video Width"), (LPCTSTR)strTmp,wcConfig);

		strTmp.Format(_T("%d"), m_nVideoHeight);
		WritePrivateProfileString(_T("Video File"), _T("Video Height"), (LPCTSTR)strTmp,wcConfig);

		strTmp.Format(_T("%d"), m_nVideoFPS);
		WritePrivateProfileString(_T("Video File"), _T("Video FPS"), (LPCTSTR)strTmp,wcConfig);

		int nIndex = m_ComboBoxVideoFormat.GetCurSel();
		strTmp.Format(_T("%d"), nIndex);
		WritePrivateProfileString(_T("Video File"), _T("Video Format"), (LPCTSTR)strTmp,wcConfig);
	}
}

static bool CheckIP(CString strIP)
{
	if(strIP.IsEmpty())
	{
		return false;
	}

	if(strIP.GetLength() < 7 || strIP.GetLength() > 15)
	{
		return false;
	}

	return true;
}

static bool CheckPort(int nPort)
{
	if(nPort <= 0 || nPort > 65535)
		return false;
	return true;
}

static bool CheckName(CString strName)
{
	if(strName.IsEmpty())
	{
		return false;
	}

	return true;
}

BOOL CEngineDemoDlg::CheckValues()
{
	if (0 == ((CButton*)GetDlgItem(IDC_CHECK_NAT_TRANS))->GetCheck())
	{
		return TRUE;
	}

	if(!CheckIP(m_strJingleIP))
	{
		AfxMessageBox(L"Jingle Server IP Address Error!");
		GetDlgItem(IDC_EDIT_JINGLE_SERVER)->SetFocus();
		return FALSE;
	}

	if(!CheckIP(m_strStunIP))
	{
		AfxMessageBox(L"Stun Server IP Address Error!");
		GetDlgItem(IDC_EDIT_STUN_SERVER)->SetFocus();
		return FALSE;
	}

	if(!CheckPort(m_nJinglePort))
	{
		AfxMessageBox(L"Jingle Server Port Error!");
		GetDlgItem(IDC_EDIT_JINGLE_PORT)->SetFocus();
		return FALSE;
	}

	if(!CheckPort(m_nStunPort))
	{
		AfxMessageBox(L"Stun Server Port Error!");
		GetDlgItem(IDC_EDIT_STUN_PORT)->SetFocus();
		return FALSE;
	}

	if(!CheckName(m_strMyName))
	{
		AfxMessageBox(L"My Name Error!");
		GetDlgItem(IDC_EDIT_MY_NAME)->SetFocus();
		return FALSE;	
	}


	if(ORIGINAL_AS_RECEIVER == m_cFlagHostOrReceiverAtFirst)
	{
		if(!CheckName(m_strHostName))
		{
			AfxMessageBox(L"Host Name Error!");
			GetDlgItem(IDC_EDIT_HOST_NAME)->SetFocus();
			return FALSE;	
		}		
	}

	return TRUE;
}


void CEngineDemoDlg::EnableControls()
{

	 if(GetDlgItem(IDC_CONNECT)->IsWindowEnabled())
	 {
		GetDlgItem(IDC_CHECK_ENABLE_VIDEO)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_ENABLE_AUDIO)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_ENABLE_SCREEN)->EnableWindow(TRUE);
		GetDlgItem(IDC_DISCONNECT)->EnableWindow(FALSE);
		GetDlgItem(IDC_AS_HOST)->EnableWindow(TRUE);
		GetDlgItem(IDC_AS_RECEIVER)->EnableWindow(TRUE);
		((CButton*)GetDlgItem(IDC_CHECK_NAT_TRANS))->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_PREVIEW)->EnableWindow(TRUE);
		GetDlgItem(IDC_CHECK_ENABLE_QOS)->EnableWindow(TRUE);

		 OnBnClickedCheckNatTrans();
		 OnBnClickedCheckEnableVideo();
		 OnBnClickedCheckEnableAudio();
		 OnBnClickedCheckEnableScreen();

		 if(1 == ((CButton*)(GetDlgItem(IDC_RADIO_USE_VIDEO_FILE)))->GetCheck())
		 {
			 OnBnClickedRadioUseCamera();
		 }
		 else
		 {
			 OnBnClickedRadioUseVideoFile();
		 }
	 }
	 else
	 {
		GetDlgItem(IDC_DISCONNECT)->EnableWindow(TRUE);	 
		((CButton*)GetDlgItem(IDC_CHECK_NAT_TRANS))->EnableWindow(FALSE);	
		((CEdit*)GetDlgItem(IDC_EDIT_JINGLE_SERVER))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_STUN_SERVER))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_JINGLE_PORT))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_STUN_PORT))->EnableWindow(FALSE);	
		((CEdit*)GetDlgItem(IDC_EDIT_MY_NAME))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_EDIT_HOST_NAME))->EnableWindow(FALSE);	
		GetDlgItem(IDC_AS_HOST)->EnableWindow(FALSE);
		GetDlgItem(IDC_AS_RECEIVER)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_PREVIEW)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_ENABLE_QOS)->EnableWindow(FALSE);

		GetDlgItem(IDC_CHECK_ENABLE_VIDEO)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_ENABLE_AUDIO)->EnableWindow(FALSE);
		GetDlgItem(IDC_CHECK_ENABLE_SCREEN)->EnableWindow(FALSE);


		BOOL bEnable = FALSE;
		(GetDlgItem(IDC_RADIO_USE_VIDEO_FILE))->EnableWindow(bEnable);
		(GetDlgItem(IDC_EDIT_VIDEO_WIDTH))->EnableWindow(bEnable);
		(GetDlgItem(IDC_EDIT_VIDEO_HEIGHT))->EnableWindow(bEnable);	
		(GetDlgItem(IDC_EDIT_VIDEO_FPS))->EnableWindow(bEnable);
		(GetDlgItem(IDC_EDIT_VIDEO_FILE))->EnableWindow(bEnable);	
		(GetDlgItem(IDC_COMBO_VIDEO_FORMAT))->EnableWindow(bEnable);	
	 }
}

void CEngineDemoDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: Add your message handler code here and/or call default

	CSliderCtrl   *pSlidCtrlCapture=(CSliderCtrl*)GetDlgItem(IDC_SLIDER_CAPTURE_VOLUME);
	CSliderCtrl   *pSlidCtrlPlay=(CSliderCtrl*)GetDlgItem(IDC_SLIDER_PLAY_VOLUME);

	if(pScrollBar->GetSafeHwnd() == pSlidCtrlCapture->GetSafeHwnd())
	{
		if(m_pDemoClient)
		{
			int nVol = pSlidCtrlCapture->GetPos();
			m_pDemoClient->SetAudioVolume(DEV_TYPE_MIC, nVol);
		}
	}
	else if(pScrollBar->GetSafeHwnd()== pSlidCtrlPlay->GetSafeHwnd())
	{
		if(m_pDemoClient)
		{
			int nVol = pSlidCtrlPlay->GetPos();
			m_pDemoClient->SetAudioVolume(DEV_TYPE_SPEAKER, nVol);
		}
	}
		

	__super::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CEngineDemoDlg::OnBnClickedButtonPlayVolumeMute()
{
	// TODO: Add your control notification handler code here
	CButton *pButton = ((CButton *)(GetDlgItem(IDC_BUTTON_PLAY_VOLUME_MUTE)));
	CString strTitle;
	pButton->GetWindowText(strTitle);

	if(strTitle.Compare(_T("X"))  == 0)
	{
		if(m_pDemoClient)
		{
			m_pDemoClient->MuteAudio(DEV_TYPE_SPEAKER, true);
		}
		pButton->SetWindowText(_T("O"));
		GetDlgItem(IDC_SLIDER_PLAY_VOLUME)->EnableWindow(FALSE);

	}
	else
	{
		if(m_pDemoClient)
		{
			m_pDemoClient->MuteAudio(DEV_TYPE_SPEAKER, false);
		}
		pButton->SetWindowText(_T("X"));
		GetDlgItem(IDC_SLIDER_PLAY_VOLUME)->EnableWindow(TRUE);
	}
}

void CEngineDemoDlg::OnBnClickedButtonCaptureVolumeMute()
{
	// TODO: Add your control notification handler code here
	CButton *pButton = ((CButton *)(GetDlgItem(IDC_BUTTON_CAPTURE_VOLUME_MUTE)));
	CString strTitle;
	pButton->GetWindowText(strTitle);

	if(strTitle.Compare(_T("X"))  == 0)
	{
		if(m_pDemoClient)
		{
			m_pDemoClient->MuteAudio(DEV_TYPE_MIC, true);
		}
		pButton->SetWindowText(_T("O"));
		GetDlgItem(IDC_SLIDER_CAPTURE_VOLUME)->EnableWindow(FALSE);

	}
	else
	{
		if(m_pDemoClient)
		{
			m_pDemoClient->MuteAudio(DEV_TYPE_MIC, false);
		}
		pButton->SetWindowText(_T("X"));
		GetDlgItem(IDC_SLIDER_CAPTURE_VOLUME)->EnableWindow(TRUE);

	}
}

void CEngineDemoDlg::UpdateAudioVolume()
{
	if(!m_pDemoClient)
	{
		return;
	}

	bool bMicMute = m_pDemoClient->IsAudioMute(DEV_TYPE_MIC);
	int nMicVol = 0;
	m_pDemoClient->GetAudioVolume(DEV_TYPE_MIC, nMicVol);

	bool bSpeakerMute = m_pDemoClient->IsAudioMute(DEV_TYPE_SPEAKER);
	int nSpeakerVol = 0;
	m_pDemoClient->GetAudioVolume(DEV_TYPE_SPEAKER, nSpeakerVol);

	m_SlicerCtrlCaptureVolume.SetPos(nMicVol);
	m_SlicerCtrlPlayVolume.SetPos(nSpeakerVol);

	if(bMicMute)
	{
		OnBnClickedButtonCaptureVolumeMute();		
	}

	if(bSpeakerMute)
	{
		OnBnClickedButtonPlayVolumeMute();	
	}

}

void CEngineDemoDlg::OnBnClickedButtonPreview()
{
	// TODO: Add your control notification handler code here
	long ret = WME_E_FAIL;
	if(m_pDemoClient && !m_bVideoInPreview)
	{
		CreateRenderWindow(DEMO_WINDOW_PREVIEW);

		ret = m_pDemoClient->CreateMediaClient(DEMO_MEDIA_VIDEO);
		ProcessRetAndReturnNULL(ret, _T("Failed to call m_pDemoClient->CreateMediaClient(DEMO_MEDIA_VIDEO)"));

		ret = m_pDemoClient->SetRenderView(DEMO_PREVIEW_TRACK, m_previewerWindowHandle, m_bUseWindowLessRender);
		ProcessRetAndReturnNULL(ret, _T("Failed to call m_pDemoClient->SetRenderView(DEMO_PREVIEW_TRACK,...)"));

		OnCbnSelchangeComboCameraList();
		OnCbnSelchangeComboCameraCapability();
		OnCbnSelchangeComboLayer();
		ret = SetRenderMode();

		ret = m_pDemoClient->StartMediaTrack(DEMO_MEDIA_VIDEO, DEMO_PREVIEW_TRACK);
		ProcessRetAndReturnNULL(ret, _T("Failed to call m_pDemoClient->StartMediaTrack(DEMO_MEDIA_VIDEO,...)"));

		m_bVideoInPreview = true;
	}
	else if(m_pDemoClient && m_bVideoInPreview)
	{
		ret = m_pDemoClient->StopMediaTrack(DEMO_MEDIA_VIDEO, DEMO_PREVIEW_TRACK);
		ProcessRetAndReturnNULL(ret, _T("Failed to call m_pDemoClient->StopMediaTrack(DEMO_MEDIA_VIDEO,...)"));

		m_pDemoClient->DeleteMediaClient(DEMO_MEDIA_VIDEO);
		CloseRenderWindow(m_previewerWindowHandle);
		m_bVideoInPreview = false;
	}
}

void CEngineDemoDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: Add your message handler code here and/or call default
	if(m_nTimerID == nIDEvent)
	{
		 if(!m_pDemoClient)
			 return;

		if(!m_pStatisticsDlg)		
			return;

		WmeSessionStatistics sSessionStatistics;

		//Show video statistics
		memset(&sSessionStatistics, 0, sizeof(sSessionStatistics));
		long ret = WME_E_FAIL;
		
		ret = m_pDemoClient->GetVideoStatistics(sSessionStatistics);

		WmeVideoStatistics sVideoStatistics;

		memset(&sVideoStatistics, 0, sizeof(sVideoStatistics));
		if(m_pDemoClient->GetVideoStatistics(DEMO_LOCAL_TRACK, sVideoStatistics) != WME_S_OK && ret != WME_S_OK)
		{
			m_pStatisticsDlg->UpdateLocalVideoStatistics(NULL, NULL);
		}
		else
		{
#ifdef ENABLE_COMMAND_LINE
			if (!clInfo->IsSyslogEnabled()) 
			{
				if (sSessionStatistics.stOutNetworkStat.uPackets > 0 && logger)
					logger->log("video-out:%u,%u,%u,%u,%u,%u,%u,%.02f",
					static_cast<unsigned int>(sSessionStatistics.stOutNetworkStat.fLossRatio * 100),
					sSessionStatistics.stOutNetworkStat.uRoundTripTime,
					sSessionStatistics.stOutNetworkStat.uJitter,
					sSessionStatistics.stOutNetworkStat.uBytes,
					sSessionStatistics.stOutNetworkStat.uPackets,
					sVideoStatistics.uWidth,
					sVideoStatistics.uHeight,
					sVideoStatistics.fFrameRate
					);
			}
#endif
			m_pStatisticsDlg->UpdateLocalVideoStatistics(&sVideoStatistics, &(sSessionStatistics.stOutNetworkStat));
		}

		memset(&sVideoStatistics, 0, sizeof(sVideoStatistics));
		if(m_pDemoClient->GetVideoStatistics(DEMO_REMOTE_TRACK, sVideoStatistics) != WME_S_OK && ret != WME_S_OK)
		{
			m_pStatisticsDlg->UpdateRemoteVideoStatistics(NULL, NULL);
		}
		else
		{
#ifdef ENABLE_COMMAND_LINE
			if (!clInfo->IsSyslogEnabled()) 
			{
				if (sSessionStatistics.stInNetworkStat.uPackets > 0  && logger)
					logger->log("video-in:%u,%u,%u,%u,%u,%u,%u,%.02f",
					static_cast<unsigned int>(sSessionStatistics.stInNetworkStat.fLossRatio * 100),
					sSessionStatistics.stInNetworkStat.uRoundTripTime,
					sSessionStatistics.stInNetworkStat.uJitter,
					sSessionStatistics.stInNetworkStat.uBytes,
					sSessionStatistics.stInNetworkStat.uPackets,
					sVideoStatistics.uWidth,
					sVideoStatistics.uHeight,
					sVideoStatistics.fFrameRate
					);
			}
#endif
			m_pStatisticsDlg->UpdateRemoteVideoStatistics(&sVideoStatistics, &(sSessionStatistics.stInNetworkStat));
		}


		//Show audio statistics
		memset(&sSessionStatistics, 0, sizeof(sSessionStatistics));
		ret = m_pDemoClient->GetAudioStatistics(sSessionStatistics);

		WmeAudioStatistics sAudioStatistics;

		memset(&sAudioStatistics, 0, sizeof(sAudioStatistics));
		if(m_pDemoClient->GetAudioStatistics(DEMO_LOCAL_TRACK, sAudioStatistics) != WME_S_OK && ret != WME_S_OK)
		{
			m_pStatisticsDlg->UpdateLocalAudioStatistics(NULL, NULL);
		}
		else
		{
#ifdef ENABLE_COMMAND_LINE
			if (!clInfo->IsSyslogEnabled()) 
			{
				if (sSessionStatistics.stOutNetworkStat.uPackets > 0  && logger)
					logger->log("audio-out:%u,%u,%u,%u,%u",
					static_cast<unsigned int>(sSessionStatistics.stOutNetworkStat.fLossRatio * 100),
					sSessionStatistics.stOutNetworkStat.uRoundTripTime,
					sSessionStatistics.stOutNetworkStat.uJitter,
					sSessionStatistics.stOutNetworkStat.uBytes,
					sSessionStatistics.stOutNetworkStat.uPackets
					);
			}
#endif

			m_pStatisticsDlg->UpdateLocalAudioStatistics(&sAudioStatistics, &(sSessionStatistics.stOutNetworkStat));
		}

		memset(&sAudioStatistics, 0, sizeof(sAudioStatistics));
		if(m_pDemoClient->GetAudioStatistics(DEMO_REMOTE_TRACK, sAudioStatistics) != WME_S_OK && ret != WME_S_OK)
		{
			m_pStatisticsDlg->UpdateRemoteAudioStatistics(NULL, NULL);
		}
		else
		{
#ifdef ENABLE_COMMAND_LINE
			if (!clInfo->IsSyslogEnabled()) 
			{
				if (sSessionStatistics.stInNetworkStat.uPackets > 0  && logger)
					logger->log("audio-in:%u,%u,%u,%u,%u",
					static_cast<unsigned int>(sSessionStatistics.stInNetworkStat.fLossRatio * 100),
					sSessionStatistics.stInNetworkStat.uRoundTripTime,
					sSessionStatistics.stInNetworkStat.uJitter,
					sSessionStatistics.stInNetworkStat.uBytes,
					sSessionStatistics.stInNetworkStat.uPackets
					);
			}
#endif

			m_pStatisticsDlg->UpdateRemoteAudioStatistics(&sAudioStatistics, &(sSessionStatistics.stInNetworkStat));
		}
	}
	__super::OnTimer(nIDEvent);
}

void CEngineDemoDlg::OnBnClickedCheckViewStatistics()
{
	// TODO: Add your control notification handler code here
	if (0 == ((CButton*)GetDlgItem(IDC_CHECK_VIEW_STATISTICS))->GetCheck())
	{
		if(m_pStatisticsDlg)
		{
			delete m_pStatisticsDlg;
			m_pStatisticsDlg = NULL;
		}
		return;
	}

	else
	{
		if(m_pStatisticsDlg)
		{
			m_pStatisticsDlg->ShowWindow(SW_SHOW);
			return;
		}

		m_pStatisticsDlg = new CStatisticsDlg(this);
		m_pStatisticsDlg->Create(CStatisticsDlg::IDD);
		m_pStatisticsDlg->ShowWindow(SW_SHOW);
	}

	return;

}

HWND CEngineDemoDlg::createWindow(char *pName, DEMO_WINDOW_TYPE eWindowType)
{
	HWND windowHandle = NULL;

	CString sName(pName);  
	LPCTSTR lpszName = sName; 
	int nScreenX = GetSystemMetrics(SM_CXSCREEN);
	int nScreenY = GetSystemMetrics(SM_CYSCREEN);
	int nStartY = 0;
	unsigned int x = 0;
	unsigned int y = 0;

	int nWidth = 640, nHeight = 360, nGap = 0;


	if(nScreenY > (nHeight + 20)*2)
	{
		nGap = (nScreenY - (nHeight + 20)*2)/3;
	}

	nStartY = nGap;


	if (DEMO_WINDOW_SENDER == eWindowType)
	{
		x = 200;
		y = nStartY;
	}
	else if (DEMO_WINDOW_RECEIVER == eWindowType)
	{
		x = 200;
		y = nStartY + nHeight + 20 + nGap ;
	}
	else if(DEMO_WINDOW_PREVIEW == eWindowType)
	{
		x = 600;
		y = nStartY;	
	}

	// TODO: Add your control notification handler code here
	windowHandle = ::CreateWindow(gClassNameInterWindow,
		lpszName,
		WS_VISIBLE|WS_SYSMENU|WS_THICKFRAME ,
		x,
		y,
		nWidth,
		nHeight+20,
		m_hWnd,
		NULL,
		AfxGetInstanceHandle(),
		0);

	return windowHandle;
}



long CEngineDemoDlg::DeleteWindowHandle(HWND windowHandle)
{
	if (m_senderWindowHandle == windowHandle)
	{
		m_senderWindowHandle = NULL;
	}
	else if (m_receiverWindowHandle == windowHandle)
	{
		m_receiverWindowHandle = NULL;
	}
	else if (m_receiverScreenWindowHandle == windowHandle)
	{
		m_receiverScreenWindowHandle = NULL;
	}
	else if(m_previewerWindowHandle == windowHandle)
	{
		m_previewerWindowHandle = NULL;	
	}

	return WME_S_OK;
}

void CEngineDemoDlg::CreateRenderWindow(DEMO_WINDOW_TYPE type)
{
	if(type == DEMO_WINDOW_SENDER)
	{
		if(!m_senderWindowHandle)
		{
			m_senderWindowHandle =  createWindow("video preview", type);
		}

		return;
	}

	if(type == DEMO_WINDOW_RECEIVER)
	{
		if(!m_receiverWindowHandle)
		{
			m_receiverWindowHandle =  createWindow("remote preview", type);
		}

		return;
	}

	if(type == DEMO_WINDOW_DESKSHARING)
	{
		if(!m_receiverScreenWindowHandle)
		{
			m_receiverScreenWindowHandle =  createWindow("view host's screen sharing", type);
			::ShowWindow(m_receiverScreenWindowHandle,SW_SHOW);
		}

		return;
	}

	if(type == DEMO_WINDOW_PREVIEW)
	{
		if(!m_previewerWindowHandle)
		{
			m_previewerWindowHandle =  createWindow("video preview", type);
		}

		return;
	
	}

	return;
}

void CEngineDemoDlg::CloseRenderWindow(HWND pWnd)
{
	

	if (m_senderWindowHandle == pWnd)
	{
		::DestroyWindow(m_senderWindowHandle);
		m_senderWindowHandle = NULL;
	}
	else if (m_receiverWindowHandle == pWnd)
	{
		::DestroyWindow(m_receiverWindowHandle);
		m_receiverWindowHandle = NULL;
	}
	else if (m_receiverScreenWindowHandle == pWnd)
	{
		::DestroyWindow(m_receiverScreenWindowHandle);
		m_receiverScreenWindowHandle = NULL;
	}
	else if(m_previewerWindowHandle == pWnd)
	{
		::DestroyWindow(m_previewerWindowHandle);
		m_previewerWindowHandle = NULL;
	}

	return;

}

void CEngineDemoDlg::networkDisconnectFromClient(DEMO_MEDIA_TYPE eType)
{

	long ret = WME_S_OK;

	if (DEMO_MEDIA_AUDIO == eType)
	{
		ret = UninitAudioClient();

		if(m_eAudioConnectionStatus != CONNECTION_STATUS_DISABLED)
		{
			ret = DisconnectMedia(DEMO_MEDIA_AUDIO);
		}

		MessageBox(_T("lost remote connect for audio"));
	}
	else if (DEMO_MEDIA_VIDEO == eType)
	{
		ret = UninitVideoClient();

		if(m_eVideoConnectionStatus != CONNECTION_STATUS_DISABLED)
		{
			ret = DisconnectMedia(DEMO_MEDIA_VIDEO);
		}
		MessageBox(_T("lost remote connect for video"));
	}
	else if (DEMO_MEDIA_DESKSHARE == eType)
	{
		ret = UninitScreenClient();

		if(m_eVideoConnectionStatus != CONNECTION_STATUS_DISABLED)
		{
			ret = DisconnectMedia(DEMO_MEDIA_DESKSHARE);
		}
		MessageBox(_T("lost remote connect for screen sharing"));
	}

	if(m_eAudioConnectionStatus == CONNECTION_STATUS_DISABLED && m_eVideoConnectionStatus == CONNECTION_STATUS_DISABLED)
	{
		GetDlgItem(IDC_CONNECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_DISCONNECT)->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_CHECK_NAT_TRANS))->EnableWindow(TRUE);

		EnableControls();
	}
}


void CEngineDemoDlg::connectRemoteFailureFromClient(DEMO_MEDIA_TYPE eType)
{
	long ret = WME_S_OK;

	if (DEMO_MEDIA_AUDIO == eType)
	{
		ret = UninitAudioClient();

		if(m_eAudioConnectionStatus != CONNECTION_STATUS_DISABLED)
		{
			ret = DisconnectMedia(DEMO_MEDIA_AUDIO);
		}

		MessageBox(_T("wrong remote Host address or remote Host not reply for audio"));
	}
	else if (DEMO_MEDIA_VIDEO == eType)
	{
		ret = UninitVideoClient();

		if(m_eVideoConnectionStatus != CONNECTION_STATUS_DISABLED)
		{
			ret = DisconnectMedia(DEMO_MEDIA_VIDEO);
		}
		MessageBox(_T("wrong remote Host address or remote Host not reply for video"));
	}
	else if (DEMO_MEDIA_DESKSHARE == eType)
	{
		ret = UninitScreenClient();

		if(m_eVideoConnectionStatus != CONNECTION_STATUS_DISABLED)
		{
			ret = DisconnectMedia(DEMO_MEDIA_DESKSHARE);
		}
		MessageBox(_T("wrong remote Host address or remote Host not reply for screen sharing"));
	}

	if(m_eAudioConnectionStatus == CONNECTION_STATUS_DISABLED && m_eVideoConnectionStatus == CONNECTION_STATUS_DISABLED)
	{
		GetDlgItem(IDC_CONNECT)->EnableWindow(TRUE);
		GetDlgItem(IDC_DISCONNECT)->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDC_CHECK_NAT_TRANS))->EnableWindow(TRUE);

		EnableControls();
	}
}

void CEngineDemoDlg::connectRemoteOKFromClient(DEMO_MEDIA_TYPE mtype)
{
	if(mtype == DEMO_MEDIA_AUDIO)
	{
		m_eAudioConnectionStatus = CONNECTION_STATUS_CONNECTED;
		OnBnClickedCheckSendAudio();
	}
	else
	{
		m_eVideoConnectionStatus = CONNECTION_STATUS_CONNECTED;
		OnBnClickedCheckSendVideo();	
	}

}

void CEngineDemoDlg::OnDeviceChanged(DeviceProperty *pDP, WmeEventDataDeviceChanged &changeEvent)
{
	if(changeEvent.iType == WmeDeviceAdded)
	{
		AddDevice(pDP);
	}
	else if(changeEvent.iType == WmeDeviceRemoved)
	{
		RemoveDevice(pDP);
	}
	else if(changeEvent.iType == WmeDefaultDeviceChanged)
	{
		UpdateDefaultDevice(pDP);
	}

	UpdateData(TRUE);

	if(pDP->dev_type == DEV_TYPE_CAMERA)
	{
		UpdateCameraCapabilityList();
	}
	return;
}

void CEngineDemoDlg::OnVolumeChange(WmeEventDataVolumeChanged &changeEvent)
{
	return;
}

void CEngineDemoDlg::OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight)
{
	if(m_receiverScreenWindowHandle && WmeRenderModeOriginal == m_eRenderModeScreenSharing)
	{
		::MoveWindow(m_receiverScreenWindowHandle, 0, 0, uWidth, uHeight, TRUE);
	}
}

void CEngineDemoDlg::OnBnClickedRadioFill()
{
	// TODO: Add your control notification handler code here
	m_eRenderMode = WmeRenderModeFill;
	((CButton *)GetDlgItem(IDC_RADIO_FILL))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_RADIO_LETTERBOX))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_CROPFILL))->SetCheck(FALSE);
	SetRenderMode();
}

void CEngineDemoDlg::OnBnClickedRadioLetterbox()
{
	// TODO: Add your control notification handler code here
	m_eRenderMode = WmeRenderModeLetterBox;
	((CButton *)GetDlgItem(IDC_RADIO_FILL))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_LETTERBOX))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_RADIO_CROPFILL))->SetCheck(FALSE);
	SetRenderMode();
}

void CEngineDemoDlg::OnBnClickedRadioCropfill()
{
	// TODO: Add your control notification handler code here
	m_eRenderMode = WmeRenderModeCropFill;
	((CButton *)GetDlgItem(IDC_RADIO_FILL))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_LETTERBOX))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_CROPFILL))->SetCheck(TRUE);
	SetRenderMode();
}

long CEngineDemoDlg::SetRenderMode()
{
	if(!m_pDemoClient)
	{
		MessageBox(_T("m_pDemoClient = NULL !!!!")); 
		return WME_E_FAIL;
	}

	long ret = WME_E_FAIL;
	ret = m_pDemoClient->SetRenderMode(DEMO_LOCAL_TRACK, m_eRenderMode);
	ret = m_pDemoClient->SetRenderMode(DEMO_PREVIEW_TRACK, m_eRenderMode);
	ret = m_pDemoClient->SetRenderMode(DEMO_REMOTE_TRACK, m_eRenderMode);
	ret = m_pDemoClient->SetRenderMode(DEMO_REMOTE_DESKTOP_SHARE_TRACK, m_eRenderModeScreenSharing);
	return ret;
}

void CEngineDemoDlg::OnBnClickedCheckEnableEc()
{
	// TODO: Add your control notification handler code here
	if(!m_pDemoClient)
	{
		return;
	}

	long ret = WME_E_FAIL;

	if(1 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_EC))->GetCheck())
	{
		ret = m_pDemoClient->EnableEC(true);	
	}
	else
	{
		ret = m_pDemoClient->EnableEC(false);	
	}

	return;
}

void CEngineDemoDlg::OnBnClickedCheckEnableVad()
{
	// TODO: Add your control notification handler code here
	if(!m_pDemoClient)
	{
		return;
	}

	long ret = WME_E_FAIL;

	if(1 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_VAD))->GetCheck())
	{
		ret = m_pDemoClient->EnableVAD(true);	
	}
	else
	{
		ret = m_pDemoClient->EnableVAD(false);	
	}

	return;
}

void CEngineDemoDlg::OnBnClickedCheckEnableNs()
{
	// TODO: Add your control notification handler code here
	if(!m_pDemoClient)
	{
		return;
	}

	long ret = WME_E_FAIL;

	if(1 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_NS))->GetCheck())
	{
		ret = m_pDemoClient->EnableNS(true);	
	}
	else
	{
		ret = m_pDemoClient->EnableNS(false);	
	}

	return;
}

void CEngineDemoDlg::OnBnClickedCheckEnableAgc()
{
	// TODO: Add your control notification handler code here
	if(!m_pDemoClient)
	{
		return;
	}

	long ret = WME_E_FAIL;

	if(1 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_AGC))->GetCheck())
	{
		ret = m_pDemoClient->EnableAGC(true);	
	}
	else
	{
		ret = m_pDemoClient->EnableAGC(false);	
	}

	return;
}

void CEngineDemoDlg::OnBnClickedCheckEnableAutodropdata()
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
	if(!m_pDemoClient)
	{
		return;
	}

	long ret = WME_E_FAIL;

	if(1 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_AUTODROPDATA))->GetCheck())
	{
		ret = m_pDemoClient->EnableAutoDropData(true);	
	}
	else
	{
		ret = m_pDemoClient->EnableAutoDropData(false);	
	}

	return;
}

void CEngineDemoDlg::OnCbnSelchangeComboCameraCapability()
{
	// TODO: Add your control notification handler code here
	if(!m_pDemoClient)
	{
		return;
	}

	int index = m_ComboBoxCameraCapability.GetCurSel();

	if(index < 0)
	{
		return;
	}

	WmeDeviceCapability *pCC = (WmeDeviceCapability*)(m_ComboBoxCameraCapability.GetItemData(index));

	if(!pCC)
	{
		return;
	}

	m_pDemoClient->SetCameraCapability(DEMO_LOCAL_TRACK, pCC);
	m_pDemoClient->SetCameraCapability(DEMO_PREVIEW_TRACK, pCC);
}

long CEngineDemoDlg::UpdateCameraCapabilityList()
{
	if(!m_pDemoClient)
	{
		return WME_E_FAIL;
	}

	int n = m_ComboBoxCameraCapability.GetCount();
	for(int i=0; i<n; i++)
	{
		WmeDeviceCapability *pCC = (WmeDeviceCapability*)(m_ComboBoxCameraCapability.GetItemData(i));
		if(pCC)
		{
			delete pCC;
		}
	}
	m_ComboBoxCameraCapability.Clear();

	int index = m_ComboBoxCameraList.GetCurSel();
	if(index < 0)
	{
		return WME_E_FAIL;
	}

	DeviceProperty *pDP = (DeviceProperty*)(m_ComboBoxCameraList.GetItemData(index));

	IWmeMediaDevice* pCamera = pDP->dev;

	if(!pCamera)
	{
		return WME_E_FAIL;
	}

	DemoClient::DeviceCapabilityList dcList;
	if(m_pDemoClient->GetDeviceCapabilities(DEV_TYPE_CAMERA, pCamera, dcList) != WME_S_OK)
	{
		return WME_E_FAIL;
	}

	int nDefaultIndex = 0;
	bool bFind = false;
	for(int i=0; i< dcList.size(); i++)
	{
		WmeDeviceCapability *pDC = new WmeDeviceCapability;
		memcpy_s(pDC, sizeof(WmeDeviceCapability), &(dcList.at(i)), sizeof(WmeDeviceCapability));

		WmeCameraCapability *pCC = (WmeCameraCapability *)(pDC->pCapalibity);

		if(!pCC)
		{
			continue;
		}

		if(pCC->type ==  WmeVideoUnknown)
		{
//			continue;
		}

		const TCHAR *string = NULL;
		for (int j=0; j<sizeof(kFormatMapInfo)/sizeof(FORMAT_MAP_INFO); j++) {
			if (kFormatMapInfo[j].type == pCC->type) {
				string = kFormatMapInfo[j].string;
				break;
			}
		}
		if (string == NULL) {
			continue;
		}

		if (pCC->width >= 1280 && pCC->height >= 720 && !bFind) {
			nDefaultIndex = i;
			bFind = true;
		}


		CString str;


		str.Format(_T("%s, %dx%d@%.01ffps"), string, pCC->width, pCC->height, pCC->MaxFPS);

		
		int index = m_ComboBoxCameraCapability.AddString(str);
		m_ComboBoxCameraCapability.SetItemData(index, (DWORD)pDC);
	}

	m_ComboBoxCameraCapability.SetCurSel(nDefaultIndex);

	return WME_S_OK;
}
void CEngineDemoDlg::OnBnClickedCheckDumpData()
{
	// TODO: Add your control notification handler code here
	if(!m_pDemoClient)
	{
		return;
	}

	if(1 == ((CButton*)GetDlgItem(IDC_CHECK_DUMP_DATA))->GetCheck())
	{
//		m_pDemoClient->SetDumpDataEnabled(true);
		CDumppingDataFlagDlg tmpDlg;
		tmpDlg.SetDumpFlag(m_uDumpFlag);
		if(IDOK == tmpDlg.DoModal())
		{
			m_uDumpFlag = tmpDlg.GetDumpFlag();
			m_pDemoClient->SetDumpDataEnabled(m_uDumpFlag);
		}
		else
		{
			((CButton*)GetDlgItem(IDC_CHECK_DUMP_DATA))->SetCheck(0);
		}
	}
	else
	{
		m_pDemoClient->SetDumpDataEnabled(false);
	}
}

void CEngineDemoDlg::OnBnClickedCheckEnableVideo()
{
	// TODO: Add your control notification handler code here
	BOOL bEnable = TRUE;
	if(0 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_VIDEO))->GetCheck())
	{
		bEnable = FALSE;
		if(0 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_AUDIO))->GetCheck()
			&& 0 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_SCREEN))->GetCheck())
		{
			MessageBox(_T("You should enble at least one media!"));
			((CButton*)GetDlgItem(IDC_CHECK_ENABLE_VIDEO))->SetCheck(1);
			return;
		}
	}

	(GetDlgItem(IDC_COMBO_CAMERA_LIST))->EnableWindow(bEnable);	
	(GetDlgItem(IDC_BUTTON_PREVIEW))->EnableWindow(bEnable);
	(GetDlgItem(IDC_COMBO_CAMERA_CAPABILITY))->EnableWindow(bEnable);
	(GetDlgItem(IDC_COMBO_LAYER))->EnableWindow(bEnable);
	(GetDlgItem(IDC_RADIO_FILL))->EnableWindow(bEnable);	
	(GetDlgItem(IDC_RADIO_LETTERBOX))->EnableWindow(bEnable);
	(GetDlgItem(IDC_RADIO_CROPFILL))->EnableWindow(bEnable);
}


void CEngineDemoDlg::OnBnClickedCheckEnableAudio()
{
	// TODO: Add your control notification handler code here
	BOOL bEnable = TRUE;
	if(0 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_AUDIO))->GetCheck())
	{
		bEnable = FALSE;

		if(0 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_VIDEO))->GetCheck()
			&& 0 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_SCREEN))->GetCheck())
		{
			MessageBox(_T("You should enble at least one media!"));
			((CButton*)GetDlgItem(IDC_CHECK_ENABLE_AUDIO))->SetCheck(1);
			return;
		}
	}

	(GetDlgItem(IDC_COMBO_MIC_LIST))->EnableWindow(bEnable);	
	(GetDlgItem(IDC_BUTTON_CAPTURE_VOLUME_MUTE))->EnableWindow(bEnable);
	(GetDlgItem(IDC_SLIDER_CAPTURE_VOLUME))->EnableWindow(bEnable);
	(GetDlgItem(IDC_COMBO_AUDIO_PARAM))->EnableWindow(bEnable);
	(GetDlgItem(IDC_COMBO_SPEAKER_LIST2))->EnableWindow(bEnable);	
	(GetDlgItem(IDC_BUTTON_PLAY_VOLUME_MUTE))->EnableWindow(bEnable);
	(GetDlgItem(IDC_SLIDER_PLAY_VOLUME))->EnableWindow(bEnable);


	(GetDlgItem(IDC_CHECK_ENABLE_EC))->EnableWindow(bEnable);
	(GetDlgItem(IDC_CHECK_ENABLE_VAD))->EnableWindow(bEnable);
	(GetDlgItem(IDC_CHECK_ENABLE_NS))->EnableWindow(bEnable);	
	(GetDlgItem(IDC_CHECK_ENABLE_AGC))->EnableWindow(bEnable);
	(GetDlgItem(IDC_CHECK_ENABLE_AUTODROPDATA))->EnableWindow(bEnable);

}

void CEngineDemoDlg::OnBnClickedRadioUseVideoFile()
{
	// TODO: Add your control notification handler code here	
	
	BOOL bEnable = TRUE;
	(GetDlgItem(IDC_EDIT_VIDEO_WIDTH))->EnableWindow(bEnable);
	(GetDlgItem(IDC_EDIT_VIDEO_HEIGHT))->EnableWindow(bEnable);	
	(GetDlgItem(IDC_EDIT_VIDEO_FPS))->EnableWindow(bEnable);
	(GetDlgItem(IDC_EDIT_VIDEO_FILE))->EnableWindow(bEnable);	
	(GetDlgItem(IDC_COMBO_VIDEO_FORMAT))->EnableWindow(bEnable);	

	bEnable = FALSE;
	((CButton*)(GetDlgItem(IDC_RADIO_USE_CAMERA)))->SetCheck(0);
	(GetDlgItem(IDC_COMBO_CAMERA_LIST))->EnableWindow(bEnable);
	(GetDlgItem(IDC_BUTTON_PREVIEW))->EnableWindow(bEnable);	
	(GetDlgItem(IDC_COMBO_CAMERA_CAPABILITY))->EnableWindow(bEnable);
}

void CEngineDemoDlg::OnBnClickedRadioUseCamera()
{
	// TODO: Add your control notification handler code here
	BOOL bEnable = FALSE;
	((CButton*)(GetDlgItem(IDC_RADIO_USE_VIDEO_FILE)))->SetCheck(0);
	(GetDlgItem(IDC_EDIT_VIDEO_WIDTH))->EnableWindow(bEnable);
	(GetDlgItem(IDC_EDIT_VIDEO_HEIGHT))->EnableWindow(bEnable);	
	(GetDlgItem(IDC_EDIT_VIDEO_FPS))->EnableWindow(bEnable);
	(GetDlgItem(IDC_EDIT_VIDEO_FILE))->EnableWindow(bEnable);	
	(GetDlgItem(IDC_COMBO_VIDEO_FORMAT))->EnableWindow(bEnable);	

	bEnable = TRUE;
	(GetDlgItem(IDC_COMBO_CAMERA_LIST))->EnableWindow(bEnable);
	(GetDlgItem(IDC_BUTTON_PREVIEW))->EnableWindow(bEnable);	
	(GetDlgItem(IDC_COMBO_CAMERA_CAPABILITY))->EnableWindow(bEnable);
}

void CEngineDemoDlg::OnBnClickedButtonSelectVideoFile()
{
	// TODO: Add your control notification handler code here

	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, this);

	TCHAR szCurDir[MAX_PATH];
	memset(szCurDir, 0, sizeof(szCurDir));
	GetModuleFileName(NULL, szCurDir, MAX_PATH);

	for(int i= MAX_PATH-1; i>=0; i--)
	{
		if(szCurDir[i] == '\\')
		{
			szCurDir[i] = '\0';
			break;
		}
	}

	dlg.m_ofn.lpstrInitialDir = szCurDir;
	CString strFilePath;
	if(dlg.DoModal() == IDOK)
	{
//		CArray<CString, CString> aryFilename;
//		POSITION posFile=dlg.GetStartPosition();
		m_strVideoFileName = dlg.GetPathName();
		/*
		while(posFile!=NULL)
		{
			aryFilename.Add(dlg.GetNextPathName(posFile));
		}

		for(int i=0;i<aryFilename.GetSize();i++)
		{
			if(m_strVideoFileName.GetLength()>0)
			{
				.AppendChar(';');
			}
			m_strVideoFileName+= aryFilename.GetAt(i);
		}
		*/
	}

	/*
	BROWSEINFO bi;	
	ZeroMemory(&bi,sizeof(BROWSEINFO));    
	bi.ulFlags=BIF_BROWSEINCLUDEFILES;        
	//bi.ulFlags=BIF_NEWDIALOGSTYLE;            
	bi.lpszTitle=L"Please select a video file:";       
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);   //Displays a dialog box enabling the user to select a Shell folder.
	TCHAR path[MAX_PATH];
	if(pidl != NULL)
	{
		SHGetPathFromIDList(pidl,path);      //Converts an item identifier list to a file system path
		m_strVideoFileName = path;
	}
	*/

	UpdateData(FALSE);
}

long CEngineDemoDlg::UpdateVideoFormatList()
{
//	const TCHAR *string = NULL;
	for (int j=1; j<sizeof(kFormatMapInfo)/sizeof(FORMAT_MAP_INFO); j++) 
	{
		m_ComboBoxVideoFormat.AddString(kFormatMapInfo[j].string);
	}
	
	m_ComboBoxVideoFormat.SetCurSel(0);
	return WME_S_OK;
}


/*
WmeVideoRawFormat rawFormat;
rawFormat.eRawType = WmeI420;
rawFormat.fFrameRate = 15;
rawFormat.iWidth = 640;
rawFormat.iHeight = 360;
rawFormat.uTimestamp = 0;

m_pFileCapEngine->SetVideoFile("xuemei_640x360.yuv", &rawFormat);
m_pFileCapEngine->Start();

*/

void CEngineDemoDlg::OnBnClickedCheckEnableScreen()
{
	// TODO: Add your control notification handler code here
	BOOL bEnable = TRUE;
	if(0 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_SCREEN))->GetCheck())
	{
		bEnable = FALSE;
		if(0 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_AUDIO))->GetCheck()
			&& 0 == ((CButton*)GetDlgItem(IDC_CHECK_ENABLE_VIDEO))->GetCheck())
		{
			MessageBox(_T("You should enble at least one media!"));
			((CButton*)GetDlgItem(IDC_CHECK_ENABLE_SCREEN))->SetCheck(1);
			return;
		}
	}

	(GetDlgItem(IDC_COMBO_SCREEN_SOURCE_LIST))->EnableWindow(bEnable);
	(GetDlgItem(IDC_RADIO_ORIGINAL))->EnableWindow(bEnable);

}


long  CEngineDemoDlg::InitScreenClient()
{
	if(!m_pDemoClient)
	{
		return WME_E_FAIL;
	}

	//Create screen client
	long ret = m_pDemoClient->CreateMediaClient(DEMO_MEDIA_DESKSHARE);

	if(WME_S_OK != ret)
	{
		MessageBox(_T("Failed to create video client!"));
		return WME_E_FAIL;
	}

	//Win PC default video 720p, and high performance
	//m_pDemoClient->SetVideoQuality(DEMO_LOCAL_TRACK, WmeVideoQuality_HD_720P);
	//m_pDemoClient->SetStaticPerformance(WmePerformanceProfileHigh);

	if(ORIGINAL_AS_RECEIVER == m_cFlagHostOrReceiverAtFirst)
	{
		//create and set video render window
		CreateRenderWindow(DEMO_WINDOW_DESKSHARING);
		ret = m_pDemoClient->SetRenderView(DEMO_REMOTE_DESKTOP_SHARE_TRACK, m_receiverScreenWindowHandle, m_bUseWindowLessRender);
		ProcessRet(ret, _T("InitScreenClient Failed to call m_pDemoClient->SetRenderView(DEMO_REMOTE_DESKTOP_SHARE_TRACK,...)"));
		ret = SetRenderMode();

		//start screen track both remote
		ret = m_pDemoClient->StartMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_REMOTE_DESKTOP_SHARE_TRACK);
		ProcessRet(ret, _T("InitScreenClient Failed to call  m_pDemoClient->StartMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_REMOTE_DESKTOP_SHARE_TRACK)"));
	}

	//Set Screen Source
	//m_pDemoClient->SetScreenSource(m_pDemoClient->GetOneScreenSource());

	//check whether need to share screen 
	//CheckStartJoiningShareScreen();

	return ret;
}

long CEngineDemoDlg::UninitScreenClient()
{
	long ret = WME_S_OK;
	if (m_pDemoClient)
	{
		ret = m_pDemoClient->StopMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_LOCAL_DESKTOP_SHARE_TRACK);
		ProcessRetAndReturnValue(ret, _T("UninitScreenClient Failed to call m_pDemoClient->StopMediaTrack(...)"), ret);

		ret = m_pDemoClient->StopMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_REMOTE_DESKTOP_SHARE_TRACK);
		ProcessRetAndReturnValue(ret, _T("UninitScreenClient Failed to call m_pDemoClient->StopMediaTrack(...)"), ret);

		CloseRenderWindow(m_receiverScreenWindowHandle);
		m_pDemoClient->DeleteMediaClient(DEMO_MEDIA_DESKSHARE);
	}

	return ret;
}

bool CEngineDemoDlg::CanShareScreen()
{
	return (m_cFlagHostOrReceiverAtFirst==ORIGINAL_AS_HOST);
}

void CEngineDemoDlg::CheckStartJoiningShareScreen()
{
	bool bShareScreen = CanShareScreen();

	if(m_receiverScreenWindowHandle)
		::ShowWindow(m_receiverScreenWindowHandle,bShareScreen?SW_HIDE:SW_SHOW);

	m_pDemoClient->EnableMyMedia(DEMO_MEDIA_DESKSHARE, bShareScreen);
	CM_INFO_TRACE_THIS("CEngineDemoDlg::CheckStartJoiningShareScreen bShareScreen="<<bShareScreen);
	if(bShareScreen)
	{
		//Set Screen Source
		int index = m_ComboBoxScreenSourceList.GetCurSel();
		if(index <= 0)
		{
			m_pDemoClient->SetScreenSource(m_pDemoClient->GetOneScreenSource());
		}
		else
		{
			IWmeScreenSource *source = m_pDemoClient->GetShareSourceManager()->GetAppSourceSourceByIndex(index-1);
			if(source)
				m_pDemoClient->SetScreenSource(source);
		}

		//start media sending
		m_pDemoClient->StartMediaSending(DEMO_MEDIA_DESKSHARE);


		//Start Local 
		WMERESULT ret = m_pDemoClient->StartMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_LOCAL_DESKTOP_SHARE_TRACK);
		ProcessRet(ret, _T("InitScreenClient Failed to call  m_pDemoClient->StartMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_LOCAL_DESKTOP_SHARE_TRACK)"));
	}
	else{

		//stop media sending
		m_pDemoClient->StopMediaSending(DEMO_MEDIA_DESKSHARE);

		//Start Local 
		WMERESULT ret = m_pDemoClient->StopMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_LOCAL_DESKTOP_SHARE_TRACK);
		ProcessRet(ret, _T("InitScreenClient Failed to call  m_pDemoClient->StopMediaTrack(DEMO_MEDIA_DESKSHARE, DEMO_LOCAL_DESKTOP_SHARE_TRACK"));
	}
}
void CEngineDemoDlg::OnCbnSelchangeComboScreenSourceList()
{
	// TODO: Add your control notification handler code here
}

long CEngineDemoDlg::UpdateScreenSourceList()
{
	if(!m_pDemoClient)
	{
		return WME_E_FAIL;
	}

	std::vector<std::string> vctSourceUniqueName;
	//m_pDemoClient->GetShareSourceManager()->GetScreenSourceNames(vctSourceUniqueName);
	m_pDemoClient->GetShareSourceManager()->GetAppSourceSourceNames(vctSourceUniqueName);

	int nDefaultIndex = 0;
	m_ComboBoxScreenSourceList.AddString(TEXT("Main Screen"));
	for(int i=0; i< vctSourceUniqueName.size(); i++)
	{
		wstring ws;
		ws.assign(vctSourceUniqueName[i].begin(),vctSourceUniqueName[i].end());
		int index = m_ComboBoxScreenSourceList.AddString(ws.c_str());
		//m_ComboBoxScreenSourceList.SetItemData(index, (DWORD)vctIWmeScreenSource[i]);
	}
	m_ComboBoxScreenSourceList.SetCurSel(nDefaultIndex);

	return WME_S_OK;
}
void CEngineDemoDlg::OnBnClickedRadioOriginal()
{
	// TODO: Add your control notification handler code here
	m_eRenderModeScreenSharing = WmeRenderModeOriginal;
	((CButton *)GetDlgItem(IDC_RADIO_ORIGINAL))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_RADIO_AS_FILL))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_AS_LETTER_BOX))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_AS_CROP))->SetCheck(FALSE);
	SetRenderMode();
}

void CEngineDemoDlg::OnBnClickedRadioAsFill()
{
	// TODO: Add your control notification handler code here
	m_eRenderModeScreenSharing = WmeRenderModeFill;
	((CButton *)GetDlgItem(IDC_RADIO_ORIGINAL))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_AS_FILL))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_RADIO_AS_LETTER_BOX))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_AS_CROP))->SetCheck(FALSE);
	SetRenderMode();
}

void CEngineDemoDlg::OnBnClickedRadioAsLetterBox()
{
	// TODO: Add your control notification handler code here
	m_eRenderModeScreenSharing = WmeRenderModeLetterBox;
	((CButton *)GetDlgItem(IDC_RADIO_ORIGINAL))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_AS_FILL))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_AS_LETTER_BOX))->SetCheck(TRUE);
	((CButton *)GetDlgItem(IDC_RADIO_AS_CROP))->SetCheck(FALSE);
	SetRenderMode();
}

void CEngineDemoDlg::OnBnClickedRadioAsCrop()
{
	// TODO: Add your control notification handler code here
	m_eRenderModeScreenSharing = WmeRenderModeCropFill;
	((CButton *)GetDlgItem(IDC_RADIO_ORIGINAL))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_AS_FILL))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_AS_LETTER_BOX))->SetCheck(FALSE);
	((CButton *)GetDlgItem(IDC_RADIO_AS_CROP))->SetCheck(TRUE);
	SetRenderMode();
}
