// EngineDemoDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <vector>
#include "afxcmn.h"
#include <list>
#include <map>

#include "DemoParameters.h"
#include "DemoClient.h"
#include "StatisticsDlg.h"
#ifdef TA_ENABLE
#include "TAManager.h"
#endif

#ifdef ENABLE_COMMAND_LINE
#include "CustomCommandLineInfo.h"
#include "Syslog.h"
#endif


typedef enum
{
	ORIGINAL_AS_NONE		= 0,
	ORIGINAL_AS_HOST		= 1,
	ORIGINAL_AS_RECEIVER	= 2,
}ORIGINAL_NETWORK_USER;

typedef enum
{
	CONNECTION_STATUS_DISABLED = 0,
	CONNECTION_STATUS_CONNECTING = 1,
	CONNECTION_STATUS_CONNECTED = 2,
	CONNECTION_STATUS_WAITING = 3,	
}CONNECTION_STATUS;

#define MAX_CODEC_NUM 10
#define MAX_NAME_LEN 16

// CEngineDemoDlg dialog
class CEngineDemoDlg : public CDialog
					  ,public backUISink

{
// Construction
public:
#ifndef ENABLE_COMMAND_LINE
	CEngineDemoDlg(CWnd* pParent = NULL);	// standard constructor
#else
	CEngineDemoDlg(CWnd* pParent = NULL , CCustomCommandLineInfo *clInitInfo = NULL);
#endif
	~CEngineDemoDlg();

// Dialog Data
	enum { IDD = IDD_ENGINEDEMO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
	//afx_msg virtual void OnCancel();
#ifdef ENABLE_COMMAND_LINE
#ifdef STUNPOC
	void InitSocketForStun()
	SOCKET m_ServerSocket;
#endif
#endif

public:

	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedProperty();
	afx_msg void OnBnClickedAsHost();
	afx_msg void OnBnClickedAsReceiver();
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedDisConnect();
	afx_msg void OnCbnSelchangeComboCameraList();
	afx_msg void OnBnClickedCheckSendVideo();
	afx_msg void OnBnClickedCheckSendAudio();
	afx_msg void OnCbnSelchangeComboLayer();
	afx_msg void OnCbnSelchangeComboMicList();
	afx_msg void OnCbnSelchangeComboAudioParam();
	afx_msg void OnCbnSelchangeComboSpeakerList2();
	afx_msg void OnBnClickedCheckNatTrans();
	afx_msg void OnEnChangeEditJingleServer3();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedButtonPlayVolumeMute();
	afx_msg void OnBnClickedButtonCaptureVolumeMute();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonPreview();
	afx_msg void OnBnClickedCheckViewStatistics();

	//process custom message
	afx_msg LRESULT OnRenderWindowPositionChanged(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnDiaplayResolutionChanged(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnRenderWindowClosed(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnStatisticsWindowClosed(WPARAM wParam,LPARAM lParam);

#ifdef ENABLE_COMMAND_LINE
	//LRESULT CEngineDemoDlg::OnAsyncEvent(WPARAM wParam, LPARAM lParam);
#endif

public:
	CString m_strMyName;
	CString m_strHostName;
	CSliderCtrl m_SlicerCtrlCaptureVolume;
	CSliderCtrl m_SlicerCtrlPlayVolume;
	int m_nJinglePort;
	int m_nStunPort;
	CString m_strJingleIP;
	CString m_strStunIP;
	CComboBox m_ComboBoxResolution;
	UINT m_uFrameRate;

	CComboBox m_ComboBoxMicList;
	CComboBox m_ComboBoxAudioParam;
	CComboBox m_ComboBoxSpeakerList;
	CComboBox m_ComboBoxCameraList;

#ifdef ENABLE_COMMAND_LINE
	Syslog *logger;
	CCustomCommandLineInfo* clInfo;
#endif

public:

	virtual void CreateRenderWindow(DEMO_WINDOW_TYPE type);
	virtual void CloseRenderWindow(HWND pWnd);
    virtual void networkDisconnectFromClient(DEMO_MEDIA_TYPE eType);
    virtual void connectRemoteFailureFromClient(DEMO_MEDIA_TYPE eType);
    virtual void connectRemoteOKFromClient(DEMO_MEDIA_TYPE mtype);
    virtual void OnDeviceChanged(DeviceProperty *pDP, WmeEventDataDeviceChanged &changeEvent);
    virtual void OnVolumeChange(WmeEventDataVolumeChanged &changeEvent);
	virtual void OnDecodeSizeChanged(uint32_t uLabel, uint32_t uWidth, uint32_t uHeight);

	DemoClient *GetDemoClient(){return m_pDemoClient;}
private:
	void UpdateAudioVolume();
	void LoadDefaultValues();
	void SaveCurrentValues();
	void EnableControls();
	BOOL CheckValues();
	void RemoveDevice(DeviceProperty *pDP);
	void AddDevice(DeviceProperty *pDP);
	void UpdateDefaultDevice(DeviceProperty *pDP);
	void UpdateUsingDevice(DeviceProperty *pDP);
	CComboBox *GetComboBoxByDevice(DeviceProperty *pDP);
	long UpdateCameraList();
	long UpdateScreenSourceList();
	long UpdateMicList();
	long UpdateSpeakerList();
	long UpdateVideoCapabilitiesList();
	long UpdateAudioCapabilitiesList();
	long UpdateVideoFormatList();
	void CleanUp();
	HWND createWindow(char *pName, DEMO_WINDOW_TYPE eWindowType);
	long SetRenderMode();
	long UpdateCameraCapabilityList();
	long InitVideoClient();
	long InitAudioClient();
	long ConnectMediaDirectly(DEMO_MEDIA_TYPE mType, bool bHost,char *pServerIP);
	long ConnectMediaThroughICE(DEMO_MEDIA_TYPE mType, bool bHost, const char* pMyName, const char *pHostName, char *pJingleIP, int nJinglePort, char *pStunIP, int nStunPort);

	long UninitVideoClient();
	long UninitAudioClient();
	long DisconnectMedia(DEMO_MEDIA_TYPE mType);

	long InitScreenClient();
	long UninitScreenClient();
	bool CanShareScreen();
	void CheckStartJoiningShareScreen();

	HWND m_receiverScreenWindowHandle;
private:
	DemoClient *m_pDemoClient;
	ORIGINAL_NETWORK_USER m_cFlagHostOrReceiverAtFirst;
	HWND					m_senderWindowHandle;
	HWND					m_receiverWindowHandle;
	HWND					m_previewerWindowHandle;
	long DeleteWindowHandle(HWND windowHandle);
	bool m_bUseWindowLessRender;
	WmeTraceLevel m_eMaxTraceLevel;
	bool m_bVideoInPreview;

	UINT_PTR m_nTimerID;
	int m_nTimerInterval;

	CStatisticsDlg *m_pStatisticsDlg;
	std::map<int, CString> m_mapVideoCodecName;
	std::map<int, CString> m_mapAudioCodecName;
	WmeTrackRenderScalingModeType m_eRenderMode;
	WmeTrackRenderScalingModeType m_eRenderModeScreenSharing;

	CONNECTION_STATUS m_eVideoConnectionStatus;
	CONNECTION_STATUS m_eAudioConnectionStatus;

	unsigned int m_uDumpFlag;
	

#ifdef TA_ENABLE
	TAManager *m_pTAManager;
#endif

public:
	afx_msg void OnBnClickedRadioFill();
	afx_msg void OnBnClickedRadioLetterbox();
	afx_msg void OnBnClickedRadioCropfill();
	afx_msg void OnBnClickedCheckEnableEc();
	afx_msg void OnBnClickedCheckEnableVad();
	afx_msg void OnBnClickedCheckEnableNs();
	afx_msg void OnBnClickedCheckEnableAgc();
	afx_msg void OnBnClickedCheckEnableAutodropdata();
	afx_msg void OnCbnSelchangeComboCameraCapability();
	CComboBox m_ComboBoxCameraCapability;
	afx_msg void OnBnClickedCheckDumpData();
	afx_msg void OnBnClickedCheckEnableVideo();
	afx_msg void OnBnClickedCheckEnableAudio();
	afx_msg void OnBnClickedRadioUseVideoFile();
	afx_msg void OnBnClickedRadioUseCamera();
	afx_msg void OnBnClickedButtonSelectVideoFile();
	UINT m_nVideoWidth;
	UINT m_nVideoHeight;
	UINT m_nVideoFPS;
	CComboBox m_ComboBoxVideoFormat;
	CString m_strVideoFileName;
	afx_msg void OnBnClickedCheckEnableScreen();
	afx_msg void OnCbnSelchangeComboScreenSourceList();
	CComboBox m_ComboBoxScreenSourceList;
	afx_msg void OnBnClickedRadioOriginal();
	afx_msg void OnBnClickedRadioAsFill();
	afx_msg void OnBnClickedRadioAsLetterBox();
	afx_msg void OnBnClickedRadioAsCrop();
};

