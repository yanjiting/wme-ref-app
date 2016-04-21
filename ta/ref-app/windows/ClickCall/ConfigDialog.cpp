#define WINVER 0x0500
#include "ClickCall.h"
#include "Windows.h"
#include "Windowsx.h"
#include "Resource.h"
#include "ConfigDialog.h"
#include "asenum.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
HWND CConfigDialog::m_hWindow = NULL;
void* CConfigDialog::m_lSaveThis = NULL;

void CConfigDialog::ScreenSourceChanged()
{
	CConfigDialog *pThis = (CConfigDialog*)m_lSaveThis; /// typecast stored this-pointer to CBaseDialog pointer
	if (pThis)
		pThis->InitScreenSource();
}

CConfigDialog::CConfigDialog(int nResId, HWND hParent)
{
	m_nResId = nResId;
	m_hParent = hParent;
	m_lSaveThis = this;
}

CConfigDialog::~CConfigDialog()
{
	m_hWindow = NULL;
	m_lSaveThis = NULL;
}

int CConfigDialog::DoModal(void)
{
	return (DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(m_nResId), m_hParent, DialogProcStatic));
}

INT_PTR CALLBACK CConfigDialog::DialogProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	if(m_hWindow == NULL)
	{
		m_hWindow = hDlg;
	}

	CConfigDialog *pThis = (CConfigDialog*)m_lSaveThis; /// typecast stored this-pointer to CBaseDialog pointer

	return(pThis->DialogProc(hDlg, message, wParam, lParam));
}

extern BOOL  WINAPI WbxUI_SetMagicWindow(HWND hWnd, int nPercent = 100);
BOOL CALLBACK CConfigDialog::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL bRet = FALSE;
	if (message == WM_INITDIALOG){
		Edit_SetText(GetDlgItem(hDlg, ID_EDIT_LINUS), TestConfig::Instance().m_sLinusUrl.c_str());
		Edit_SetText(GetDlgItem(hDlg, ID_EDIT_WSADDR), TestConfig::Instance().m_sWSUrl.c_str());
		Button_SetCheck(GetDlgItem(m_hWindow, ID_CHECK_CALLIOPE), TestConfig::Instance().m_bCalliope);
		Button_SetCheck(GetDlgItem(m_hWindow, ID_CHECK_LOOPBACK), TestConfig::Instance().m_bLoopback);
		Button_SetCheck(GetDlgItem(m_hWindow, ID_CHECK_CVO), TestConfig::Instance().m_bEnableCVO);
		Button_SetCheck(GetDlgItem(m_hWindow, ID_CHECK_AS), TestConfig::Instance().m_bAppshare);
		Button_SetCheck(GetDlgItem(m_hWindow, ID_CHECK_ASPREVIEW), TestConfig::Instance().m_bASPreview);
		Button_SetCheck(GetDlgItem(m_hWindow, ID_CHECK_ISSHARER), TestConfig::Instance().m_bSharer);
        Button_SetCheck(GetDlgItem(m_hWindow, ID_CHECK_QOS), TestConfig::Instance().m_bQoSEnable);
        bool bFec = true;
        if (TestConfig::i().m_videoParam.isMember("fecParams") && TestConfig::i().m_videoParam["fecParams"].isMember("bEnableFec")) {
            bFec = TestConfig::i().m_videoParam["fecParams"]["bEnableFec"].asBool();
        }
        Button_SetCheck(GetDlgItem(m_hWindow, ID_CHECK_FEC), bFec);
        Button_SetCheck(GetDlgItem(m_hWindow, ID_CHECK_FILTER_SELF), TestConfig::Instance().m_bShareFilterSelf);

        bool bIce = true;
        if (TestConfig::i().m_audioDebugOption.isMember("enableICE"))
            bIce = TestConfig::i().m_audioDebugOption["enableICE"].asBool();
        Button_SetCheck(GetDlgItem(m_hWindow, ID_CHECK_ICE), bIce);
        Button_SetCheck(GetDlgItem(m_hWindow, ID_CHECK_MULTI), TestConfig::i().m_bMultiStreamEnable);

		HWND hVideoSize = GetDlgItem(m_hWindow, ID_COMBO_VIDEOSIZE);
		ComboBox_AddString(hVideoSize, "180p");
		ComboBox_SetItemData(hVideoSize, 0, 1);
		ComboBox_AddString(hVideoSize, "360p");
		ComboBox_SetItemData(hVideoSize, 1, 2);
		ComboBox_AddString(hVideoSize, "720p");
		ComboBox_SetItemData(hVideoSize, 2, 3);
		ComboBox_SetCurSel(hVideoSize, 1);

        HWND hActiveVideo = GetDlgItem(m_hWindow, ID_COMBO_ACTIVEVIDEO_COUNT);
        int i = 0;
        for (i = 1; i < 5; i++) {
            ComboBox_AddString(hActiveVideo, std::to_string(i).c_str());
            ComboBox_SetItemData(hActiveVideo, i - 1, i);
        }
        ComboBox_SetCurSel(hActiveVideo, TestConfig::i().m_uMaxVideoStreams - 1);

		InitScreenSource();
		CASEnum::Instance()->SetNotifyFunction(ScreenSourceChanged);
		WbxUI_SetMagicWindow(hDlg);
	}
	else if (message == WM_COMMAND){
		UINT id = LOWORD(wParam);
		switch (id){
		case IDOK:
			bRet = TRUE;
			OnOK();
			break;
		case IDCANCEL:
			bRet = TRUE;
			OnCancel();
			break;
        case ID_CHECK_NOSIGNAL: {
            BOOL bCheck = Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_NOSIGNAL));
            Button_SetCheck(GetDlgItem(m_hWindow, ID_CHECK_LOOPBACK), !bCheck);
            Button_Enable(GetDlgItem(m_hWindow, ID_CHECK_LOOPBACK), !bCheck);
            Button_Enable(GetDlgItem(m_hWindow, ID_EDIT_WSADDR), !bCheck);
            Button_Enable(GetDlgItem(m_hWindow, ID_EDIT_VENUEURL), bCheck);
            bRet = TRUE;
            break;
        }
		default:
			break;
		}
	}
	return bRet;
}

void CConfigDialog::InitScreenSource(){
	HWND hScreenSource = GetDlgItem(m_hWindow, IDC_COMBO_AS_SOURCE);
	std::map <std::string, IWmeScreenSource *> mapScreenSource = CASEnum::Instance()->getScreenSourceList();
	std::string strPreferSourceName = TestConfig::Instance().m_strScreenSharingAutoLaunchSourceName;
	bool bAddPreferScreenShource = strPreferSourceName.size()>0;
	
	SendMessageW(hScreenSource, CB_RESETCONTENT, 0L, 0L);
	std::string strSelectSourceName;
	for (std::map <std::string, IWmeScreenSource *>::iterator iter = mapScreenSource.begin();
		iter != mapScreenSource.end(); ++iter)
	{
		IWmeScreenSource *pIWmeScreenSource = iter->second;
		if (pIWmeScreenSource == NULL) continue;
		WmeScreenSourceType type = WmeScreenSourceTypeUnknow;
		pIWmeScreenSource->GetSourceType(type);
		char szName[256] = { 0 };
		int len = 256;
		pIWmeScreenSource->GetFriendlyName(szName, len);
		if (type == WmeScreenSourceTypeDesktop){
			HMONITOR hMonitor = (HMONITOR)pIWmeScreenSource->GetSourceHandle();
			MONITORINFOEX monitorInfoEx;
			memset(&monitorInfoEx, 0, sizeof(MONITORINFOEX));
			monitorInfoEx.cbSize = sizeof(MONITORINFOEX);
			GetMonitorInfo(hMonitor, &monitorInfoEx);
			std::string strOldName = szName;
			sprintf_s(szName, 256, "%s:[%dx%d]", strOldName.c_str(),
				monitorInfoEx.rcMonitor.right - monitorInfoEx.rcMonitor.left,
				monitorInfoEx.rcMonitor.bottom - monitorInfoEx.rcMonitor.top);
		}

		if (stricmp(szName, "") != 0)
		{
			int nIndex = 0;

			WCHAR szUnicodeName[256] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, szName, strlen(szName), szUnicodeName, 256);


			if (type == WmeScreenSourceTypeApplication){
				nIndex = SendMessageW(hScreenSource, CB_ADDSTRING, 0L, (LPARAM)szUnicodeName);
			}
			else{
				nIndex = SendMessageW(hScreenSource, CB_INSERTSTRING, 0, (LPARAM)szUnicodeName);
			}
			if (bAddPreferScreenShource){
				std::string strFriendeName = szName;
				if (strFriendeName.find(strPreferSourceName) != std::string::npos)
					strSelectSourceName = strFriendeName;
			}
		}
	}
	int nSelectIndex = 0;
	if (strSelectSourceName.length() > 0){

		WCHAR strUnicodeSelectSourceName[256] = { 0 };
		MultiByteToWideChar(CP_UTF8, 0, strSelectSourceName.c_str(), strSelectSourceName.length(), strUnicodeSelectSourceName, 256);

		//nSelectIndex = ComboBox_FindString(hScreenSource, 0, strUnicodeSelectSourceName);
		

		nSelectIndex = SendMessageW(hScreenSource, CB_FINDSTRING, 0, (LPARAM)strUnicodeSelectSourceName);

	}
	ComboBox_SetCurSel(hScreenSource, nSelectIndex);
}

void CConfigDialog::OnOK(void)
{
	char szTemp[2048];
	int nMaxChar = 2048;
	Edit_GetText(GetDlgItem(m_hWindow, ID_EDIT_LINUS), szTemp, nMaxChar);
	TestConfig::i().m_sLinusUrl = szTemp;
	Edit_GetText(GetDlgItem(m_hWindow, ID_EDIT_WSADDR), szTemp, nMaxChar);
	TestConfig::i().m_sWSUrl = szTemp;
	TestConfig::i().m_bCalliope = Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_CALLIOPE));
	TestConfig::i().m_bLoopback = Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_LOOPBACK));
	TestConfig::i().m_bEnableCVO = Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_CVO));
	TestConfig::i().m_bAppshare = Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_AS));
	TestConfig::i().m_bSharer = Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_ISSHARER));
    TestConfig::i().m_bMultiStreamEnable = Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_MULTI));
    TestConfig::i().m_bQoSEnable = Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_QOS));
    TestConfig::i().m_audioParam["enableQos"] = TestConfig::i().m_bQoSEnable;
	TestConfig::i().m_videoParam["enableQos"] = TestConfig::i().m_bQoSEnable;
    bool enableFec = Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_FEC));
    if (enableFec)
    {
        TestConfig::i().m_videoParam["fecParams"]["bEnableFec"] = true;
        TestConfig::i().m_videoParam["fecParams"]["uClockRate"] = 8000;
        TestConfig::i().m_videoParam["fecParams"]["uPayloadType"] = 111;
    }
    bool enableDtlsSrtp = Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_DTLS_SRTP));
    if (enableDtlsSrtp) {
        TestConfig::i().m_bEnableDtlsSrtp = true;
    } else {
        TestConfig::i().m_bEnableDtlsSrtp = false;
    }
	TestConfig::i().m_bASPreview = Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_ASPREVIEW));
	HWND hCheckFilterSelf = GetDlgItem(m_hWindow, ID_CHECK_FILTER_SELF);
	if ( hCheckFilterSelf )
		TestConfig::i().m_bShareFilterSelf = Button_GetCheck(hCheckFilterSelf);
	if (TestConfig::i().m_bAppshare){
		HWND hScreenSource = GetDlgItem(m_hWindow, IDC_COMBO_AS_SOURCE);
		int nIndex = ComboBox_GetCurSel(hScreenSource);
//		char szSelectSource[MAX_PATH] = { 0 };
		WCHAR szSelectSource[MAX_PATH] = { 0 };

		
//		ComboBox_GetLBText(hScreenSource, nIndex, szSelectSource);
		//TestConfig::i().m_strScreenSharingAutoLaunchSourceName = "DISPLAY";

		SendMessageW(hScreenSource, CB_GETLBTEXT, nIndex, (LPARAM)szSelectSource);

		char szUtf8SelectSource[MAX_PATH] = { 0 };
		WideCharToMultiByte(CP_UTF8, 0, szSelectSource, wcslen(szSelectSource), szUtf8SelectSource, MAX_PATH, NULL, NULL);
		std::string strSourceName = szUtf8SelectSource;
		strSourceName = strSourceName.substr(0, strSourceName.find(":"));
		TestConfig::i().m_strScreenSharingAutoLaunchSourceName = strSourceName.c_str();
		if (TestConfig::i().m_bCalliope){
			//			TestConfig::i().m_bFakeVideoByShare = false;
			TestConfig::i().m_bHasVideo = true;
		}
		TestConfig::i().m_bAutoStart = true;
	}
    if (!TestConfig::i().m_bLoopback)
	{
		TestConfig::i().m_bSharer = Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_ISSHARER));
    }

	TestConfig::i().m_audioDebugOption["enableICE"] = !!Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_ICE));
	TestConfig::i().m_videoDebugOption["enableICE"] = !!Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_ICE));
	TestConfig::i().m_shareDebugOption["enableICE"] = !!Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_ICE));

	HWND hVideoSize = GetDlgItem(m_hWindow, ID_COMBO_VIDEOSIZE);
    HWND hActiveVideo = GetDlgItem(m_hWindow, ID_COMBO_ACTIVEVIDEO_COUNT);
    TestConfig::i().m_nVideoSize = ComboBox_GetItemData(hVideoSize, ComboBox_GetCurSel(hVideoSize));
    TestConfig::i().m_uMaxVideoStreams = ComboBox_GetItemData(hActiveVideo, ComboBox_GetCurSel(hActiveVideo));
    TestConfig::i().m_bNoSignal = Button_GetCheck(GetDlgItem(m_hWindow, ID_CHECK_NOSIGNAL));;
    Edit_GetText(GetDlgItem(m_hWindow, ID_EDIT_VENUEURL), szTemp, nMaxChar);
    TestConfig::i().m_sVenuUrl = szTemp;

	EndDialog(m_hWindow, IDOK);
}

void CConfigDialog::OnCancel(void)
{
	EndDialog(m_hWindow, IDCANCEL);
}
