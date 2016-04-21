#include "ClickCall.h"
#include "resource.h"
#include "BackdoorAgent.h"
#include "CameraPreview.h"
#include "ClickCall_win.h"
#include "loopback.h"
#include <tchar.h>
#include "PeerCall.h"
#include "asenum.h"
#include "TAHTTPServer.h"
#include "Shlwapi.h"
#include "ConfigDialog.h"
#include "ProxyAuthenication.h"

const TCHAR* ENDPOINT_WIN_CLASS = _T("CLICK_TO_CALL_WINDOW_CLASS");
const TCHAR* ENDPOINT_ASVIEW_CLASS = _T("CLICK_TO_CALL_ASVIEW_CLASS");

const UINT g_nProxyMessageID = WM_USER + 1030;

CClickCallApp theApp;
HINSTANCE g_hInstance = NULL;

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED           0x00080000
#define LWA_COLORKEY            0x00000001
#define LWA_ALPHA               0x00000002
#define ULW_ALPHA               0x00000002
#define ULW_COLORKEY			0x00000001
#endif // ndef WS_EX_LAYERED
BOOL WINAPI WbxUI_IsMagicWindow(HWND hWnd)
{
	if (!hWnd || !::IsWindow(hWnd))
		return FALSE;

	BOOL bRet = FALSE;
	DWORD_PTR dwStyle = ::GetWindowLongPtr(hWnd, GWL_EXSTYLE);
	if (dwStyle & WS_EX_LAYERED)
		bRet = TRUE;
	return bRet;
}

typedef BOOL(WINAPI *lpfnSetLayeredWindowAttributes)(HWND hWnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);
BOOL  WINAPI WbxUI_SetMagicWindow(HWND hWnd, int nPercent=100)
{
	OSVERSIONINFO versionInfo;
	versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&versionInfo);
	//if not Win2K and WinXP
	if (!(versionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&
		versionInfo.dwMajorVersion >= 5))
		return FALSE;

	if (!hWnd || !::IsWindow(hWnd))
		return FALSE;

	TCHAR strClassName[10] = { 0 };
	::GetClassName(hWnd, strClassName, sizeof(strClassName) / sizeof(TCHAR));

	BOOL bPopupMenu = (lstrcmp(strClassName, _T("#32768")) == 0); //elliot>bug #399332

	if (WbxUI_IsMagicWindow(hWnd) && !bPopupMenu)
		return TRUE;

	BOOL bRet = FALSE;


	// Remove for Popup menu issue
	/*
	DWORD_PTR dwStyle = ::GetWindowLongPtr(hWnd,GWL_EXSTYLE) ;
	if (dwStyle & WS_EX_LAYERED == WS_EX_LAYERED)
	return TRUE ;   */


	// have the layered property 
	lpfnSetLayeredWindowAttributes pSetLayeredWindowAttributes(NULL);

	HMODULE hUser32 = GetModuleHandle(_T("USER32.DLL"));
	if (hUser32)
	{
		pSetLayeredWindowAttributes = (lpfnSetLayeredWindowAttributes)GetProcAddress(hUser32,
			"SetLayeredWindowAttributes");
		if (pSetLayeredWindowAttributes)
		{
			int nRet1 = 0;
			BOOL bRet2 = FALSE;
			nRet1 = ::SetWindowLongPtr(hWnd, GWL_EXSTYLE, ::GetWindowLongPtr(hWnd, GWL_EXSTYLE)
				| WS_EX_LAYERED);

			bRet2 = pSetLayeredWindowAttributes(hWnd, 0, (255 * nPercent) / 100, LWA_ALPHA);
			bRet = (nRet1 && bRet2);
		}
	}
	else
		bRet = FALSE;
	return bRet;

}

void WINAPI WbxUI_ClearMagicWindow(HWND hWindow)
{
	if (!hWindow)
		return;
	DWORD_PTR dwStyle = ::GetWindowLongPtr(hWindow, GWL_EXSTYLE);
	if ((dwStyle & WS_EX_LAYERED) == WS_EX_LAYERED)
	{
		dwStyle &= (~WS_EX_LAYERED);
		::SetWindowLongPtr(hWindow, GWL_EXSTYLE, dwStyle);
	}
}

class CStatsWnd : public StatsInfoSink{
public:
	CStatsWnd() :m_hWnd(NULL) {}
	~CStatsWnd() {
		Stop();
	}
	HWND GetHWND() { return m_hWnd; }
	void Stop(){
		m_StatsInfo.SetSink(NULL);
		m_StatsInfo.Stop();
		if (m_hWnd && IsWindow(m_hWnd))
			DestroyWindow(m_hWnd);
		m_hWnd = NULL;
	}

	void Start(){
		m_hWnd = CreateAWindow(0, 0, 360, 560, NULL, 0, WHITE_BRUSH);
		m_StatsInfo.SetSink(this);
		m_StatsInfo.Start();
	}

	LRESULT OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam){
		if (uMsg == WM_CLOSE){
			Stop();
			return 0;
		}
			
		return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
	}
protected:
	virtual void OnStatsInfo(const char *strInfo, const char *audioStats, const char *videoStats, const char *screenStats)
	{
		if (!m_hWnd || !IsWindow(m_hWnd) || strInfo==NULL)
			return;

		HDC dc = GetDC(m_hWnd);
		RECT rc;
		GetClientRect(m_hWnd, &rc);
		HBRUSH brush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
		SelectObject(dc, brush);
		Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);
		SetBkMode(dc, TRANSPARENT);
		rc.left += 10;
		rc.top += 10;
		
		::DrawText(dc, strInfo, lstrlen(strInfo), &rc, DT_LEFT | DT_EXTERNALLEADING | DT_WORDBREAK);
		::ReleaseDC(m_hWnd, dc);
	}

private:
	HWND m_hWnd;
	CCmTimerWrapperID m_Timer;
	StatsInfo m_StatsInfo;
};

CStatsWnd g_statsWnd;

void CClickCallApp::DoVideoDeviceCommand(int nID){
    if (nID == IDM_NEXT_DEVICE) {
        if (g_cameras.size() < 2)
            return;
        auto it = g_cameras.find(m_sCurrentCamera);
        it++;
        if (it == g_cameras.end())
            it = g_cameras.begin();
        m_sCurrentCamera = it->first;
        m_cameraPreview.ChangeDevice(it->second);
    }
    else if (nID == IDM_APPLY_DEVICE) {
        auto it = g_cameras.find(m_sCurrentCamera);
        if (PeerCall::Instance()->isActive()) {
            PeerCall::Instance()->m_endCaller->setCamera(it->second);
        }
        else {
            LoopbackCall::Instance()->m_endCaller.setCamera(it->second);
        }
    }
    if (nID > IDM_VIDEO_DEVICES && nID < IDM_VIDEO_DEVICES_MAX)
    {
        HMENU hMenu = GetMenu(g_winMain);
        HMENU hVideoMenu = GetSubMenu(hMenu, 3);
        HMENU hDeviceMenu = GetSubMenu(hVideoMenu, 4);

        WCHAR szMenuItem[256] = { 0 };
        char szMenuItemUtf8[1024] = { 0 };
        GetMenuStringW(hDeviceMenu, nID, szMenuItem, 256, MF_BYCOMMAND);
        WideCharToMultiByte(CP_UTF8, 0, szMenuItem, wcslen(szMenuItem), szMenuItemUtf8, 1024, NULL, NULL);
        IWmeMediaDevice *pDevice = g_cameras[szMenuItemUtf8];
        m_sCurrentCamera = szMenuItemUtf8;
        CM_ASSERTE(pDevice != NULL);
        g_wndCameraPreview = CreateAWindow(0, 0, 640, 360, NULL, IDC_VIDEODEVICE_MENU);
        m_cameraPreview.Preview(pDevice, g_wndCameraPreview);
    }
}

void CClickCallApp::LoadCameras()
{
    g_cameras = Endpoint::getCameraList();
    HMENU hMenu = GetMenu(g_winMain);
    HMENU hVideoMenu = GetSubMenu(hMenu, 3);
    HMENU hDeviceMenu = GetSubMenu(hVideoMenu, 4);

    int i = 0;
    for (auto it : g_cameras) {
        WCHAR szMenuItem[256] = { 0 };
        MultiByteToWideChar(CP_UTF8, 0, it.first.c_str(), it.first.length(), szMenuItem, 256);
        i++;
        AppendMenuW(hDeviceMenu, MF_STRING, IDM_VIDEO_DEVICES + i, szMenuItem);
    }
    m_cameraPreview.Init();
}

void CClickCallApp::LoadShareSource()
{
	std::map <std::string, IWmeScreenSource *> mapScreenSource = CASEnum::Instance()->getScreenSourceList();

    HMENU hMenu = GetMenu(g_winMain);
	HMENU hShareMenu = GetSubMenu(hMenu, 1);
//	AppendMenu(hShareMenu, MF_SEPARATOR, 0, _T(""));

	g_mapAppMenuItem.clear();
	g_mapDesktopMenuItem.clear();

	int nCountApp = 0;
	int nCountMonitor = 0;

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

		if (stricmp(szName, "") != 0)
		{
			WCHAR szMenuItem[256] = { 0 };
			MultiByteToWideChar(CP_UTF8, 0, szName, strlen(szName), szMenuItem, 256);

			char szID[256] = { 0 };
			pIWmeScreenSource->GetUniqueName(szID, len);

			if (type == WmeScreenSourceTypeApplication){
				nCountApp++;
				AppendMenuW(hShareMenu, MF_STRING, IDM_SHARE_APP + nCountApp, szMenuItem);
				g_mapAppMenuItem.insert(std::make_pair(IDM_SHARE_APP + nCountApp, szID));
			}
			else if (type == WmeScreenSourceTypeDesktop){
				nCountMonitor++;
				InsertMenuW(hShareMenu, 0, MF_STRING, IDM_SHARE_DESKTOP + nCountMonitor, szMenuItem);
				g_mapDesktopMenuItem.insert(std::make_pair(IDM_SHARE_DESKTOP + nCountMonitor, szID));
			}
		}
	}

    DrawMenuBar(g_winMain);
}

void CClickCallApp::OnShareChanged(BOOL bShareDesktop)
{
    HMENU hMenu = GetMenu(g_winMain);
	HMENU hShareMenu = GetSubMenu(hMenu, 1);

	MENUITEMINFO info;
	info.cbSize = sizeof (MENUITEMINFO);
	info.fMask = MIIM_STATE | MIIM_ID;
	GetMenuItemInfo(hShareMenu, 0, TRUE, &info);


	if (bShareDesktop)
	{
		CheckMenuItem(hShareMenu, info.wID, MF_BYCOMMAND | MF_CHECKED);
	}
	else
	{
		CheckMenuItem(hShareMenu, info.wID, MF_BYCOMMAND | MF_UNCHECKED);
	}

	if (bShareDesktop)
	{
		int nCount = GetMenuItemCount(hShareMenu);

		for (int i = 2; i < nCount; i++)
		{
			GetMenuItemInfo(hShareMenu, i, TRUE, &info);
			CheckMenuItem(hShareMenu, info.wID, MF_BYCOMMAND | MF_UNCHECKED);
		}
	}
}

void CClickCallApp::ShareDesktop(int nMenuItem)
{
	//IWmeScreenSource *pSrc = CASEnum::Instance()->GetOneDefaultScreenSource();
	IWmeScreenSource *pSrc = CASEnum::Instance()->FindScreenSource(g_mapDesktopMenuItem[nMenuItem].c_str());
	if (!pSrc)
		return;


	if (g_bShareStarted && g_bShareDesktop)
		return;
	else
		g_bShareDesktop = TRUE;

	g_bShareStarted = TRUE;

	if (!g_presenter)
		return;

	if (!TestConfig::i().m_bSharer && !IsWindowVisible(g_winScreenViewer))
	{
		ShowWindow(g_winScreenViewer, SW_SHOW);
	}

	doStop();

	g_presenter->addScreenSouce(pSrc);

//	doStart(__argc, __argv, g_winLocal, g_winLocal, g_winScreenViewer);

	doStartCall(g_winActive, g_winLocal, g_winScreenViewer);

	ShowScreenSharePreview();


	OnShareChanged(TRUE);
}

void CClickCallApp::ShareOneApp(int nMenuItem)
{
	if (!g_presenter)
		return;

	IWmeScreenSource *pSrc = CASEnum::Instance()->FindScreenSource(g_mapAppMenuItem[nMenuItem].c_str());

	if (!pSrc)
		return;

	g_bShareStarted = TRUE;

	if (!TestConfig::i().m_bSharer && !IsWindowVisible(g_winScreenViewer))
	{
		ShowWindow(g_winScreenViewer, SW_SHOW);
	}

	if (g_bShareDesktop)
	{
		g_bShareDesktop = FALSE;

		doStop();

		g_presenter->addScreenSouce(pSrc);

//		doStart(__argc, __argv, g_winLocal, g_winLocal, g_winScreenViewer);
		doStartCall(g_winActive, g_winLocal, g_winScreenViewer);

		ShowScreenSharePreview();

	}
	else
	{
		g_presenter->addScreenSouce(pSrc);
	}

	OnShareChanged(FALSE);
}

void CClickCallApp::UnshareOneApp(int nMenuItem)
{
	if (g_presenter)
	{
		IWmeScreenSource *pSrc = CASEnum::Instance()->FindScreenSource(g_mapAppMenuItem[nMenuItem].c_str());

		if (!pSrc)
			return;

		g_presenter->removeScreenSouce(pSrc);
	}
}

void CClickCallApp::ClickOneApp(int nMenuItem)
{
	if (g_mapAppMenuItem.find(nMenuItem) != g_mapAppMenuItem.end())
	{
		ShareOneApp(nMenuItem);
	}
}

void MuteDevice(bool bMute, bool bSpeaker)
{
	if (PeerCall::Instance()->isActive()) {
		PeerCall::Instance()->mute(WmeSessionType_Audio, bMute, !bSpeaker);
	}
	else {
		LoopbackCall::Instance()->mute(WmeSessionType_Audio, bMute, !bSpeaker);
	}
}

void MuteVideo(bool bMute, bool bRemote)
{
    if (PeerCall::Instance()->isActive()) {
        PeerCall::Instance()->mute(WmeSessionType_Video, bMute, !bRemote);
    }
    else {
        LoopbackCall::Instance()->mute(WmeSessionType_Video, bMute, !bRemote);
    }
}

void MuteShare()
{
	if (PeerCall::Instance()->isActive()) {
		PeerCall::Instance()->mute(WmeSessionType_ScreenShare, true, true);
	}
	else {
		LoopbackCall::Instance()->mute(WmeSessionType_ScreenShare, true, true);
	}
}

void UnmuteShare()
{
	if (PeerCall::Instance()->isActive()) {
		PeerCall::Instance()->mute(WmeSessionType_ScreenShare, false, true);
	}
	else {
		LoopbackCall::Instance()->mute(WmeSessionType_ScreenShare, false, true);
	}
}
void AddAutoLaunchShareSource()
{
	if (TestConfig::Instance().m_bSharer
		&& TestConfig::Instance().m_bAutoStart
		&& TestConfig::Instance().m_strScreenSharingAutoLaunchSourceName.size() > 0){
		IWmeScreenSource *pShareSource = NULL;
		pShareSource = CASEnum::Instance()->FindScreenSourceByFriendlyName(
			TestConfig::Instance().m_strScreenSharingAutoLaunchSourceName.c_str(),
			false);
		if (pShareSource){
			if (TestConfig::Instance().m_bLoopback){
				LoopbackCall::Instance()->m_endCaller.addScreenSouce(pShareSource);
			}
			else{
				if (PeerCall::Instance()->m_endCaller)
					PeerCall::Instance()->m_endCaller->addScreenSouce(pShareSource);
			}
		}
	}

}

void CClickCallApp::DoExit(HWND hWnd)
{
    if (g_wndCameraPreview == hWnd) {
        m_cameraPreview.StopPreview(g_wndCameraPreview);
        DestroyWindow(g_wndCameraPreview);
        g_wndCameraPreview = NULL;
        return;
    }

    m_cameraPreview.UnInit();
	doStop();
	::ShowWindow(g_winMain, SW_HIDE);
	::ShowWindow(g_winScreenViewer, SW_HIDE);

	if (g_wndASPreview && IsWindow(g_wndASPreview))
	{
		DestroyWindow(g_wndASPreview);
		g_wndASPreview = NULL;
	}

	CConfigDialog configDlg(IDD_DIALOG_CONFIG, NULL);
	if (IDOK == configDlg.DoModal()){

		AddAutoLaunchShareSource();

        m_cameraPreview.Init();
		doStartCall(g_winActive, g_winLocal, g_winScreenViewer, &theApp);
        ::ShowWindow(g_winMain, SW_SHOW);

		ShowScreenSharePreview();

	}
	else{
        UninitMediaEngine();
		PostQuitMessage(0);
	}
}

void CClickCallApp::DoAppShareCommand(int nID)
{
    if (nID > IDM_SHARE_APP && nID < IDM_SHARE_APP_MAX)
    {
        HMENU hMenu = GetMenu(g_winMain);
        HMENU hShareMenu = GetSubMenu(hMenu, 1);

        MENUITEMINFO info;
        info.cbSize = sizeof(MENUITEMINFO);
        info.fMask = MIIM_STATE;
        GetMenuItemInfo(hShareMenu, nID, FALSE, &info);

        if (g_mapAppMenuItem.find(nID) != g_mapAppMenuItem.end())
        {

            if (info.fState & MF_CHECKED)
            {
                CheckMenuItem(hShareMenu, nID, MF_BYCOMMAND | MF_UNCHECKED);
                UnshareOneApp(nID);
            }
            else
            {
                CheckMenuItem(hShareMenu, nID, MF_BYCOMMAND | MF_CHECKED);
                ShareOneApp(nID);
            }
        }
    }
    else if (nID > IDM_SHARE_DESKTOP && nID < IDM_SHARE_DESKTOP_MAX){
        ShareDesktop(nID);
    }
}

static LRESULT CALLBACK EndpointWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (hwnd == g_statsWnd.GetHWND())
		return g_statsWnd.OnMessage(uMsg, wParam, lParam);

	if (uMsg == WM_CLOSE)
	{
        theApp.DoExit(hwnd);
		return 0;
	}
	else if (uMsg == WM_SIZE){
        theApp.Resize();

		if (PeerCall::Instance()->isActive()
			&& PeerCall::Instance()->m_endCaller){
			PeerCall::Instance()->m_endCaller->resizeRender();
		}
		else{
			LoopbackCall::Instance()->m_endCaller.resizeRender();
			if (LoopbackCall::Instance()->m_endCallee)
				LoopbackCall::Instance()->m_endCallee->resizeRender();
		}
	}
	else if (uMsg == WM_COMMAND)
	{
		int nID = LOWORD(wParam);

		switch (nID)
		{
			case IDM_EXIT:
				doStop();
				//PostQuitMessage(0);
				break;
			case IDM_AUDIO_UNMUTE:
				MuteDevice(false, false);
				break;
			case IDM_AUDIO_MUTE:
				MuteDevice(true, false);
				break;
			case IDM_AUDIO_UNMUTE_SPK:
				MuteDevice(false, true);
				break;
			case IDM_AUDIO_MUTE_SPK:
				MuteDevice(true, true);
				break;
            case IDM_VIDEO_UNMUTE:
                MuteVideo(false, false);
                break;
            case IDM_VIDEO_MUTE:
                MuteVideo(true, false);
                break;
            case IDM_VIDEO_UNMUTE_REMOTE:
                MuteVideo(false, true);
                break;
            case IDM_VIDEO_MUTE_REMOTE:
                MuteVideo(true, true);
                break;
            case IDM_SHOW_STATISTICS:
				g_statsWnd.Start();
				break;
			case IDM_SHARE_MUTE:
				MuteShare();
				break;
			case IDM_SHARE_UNMUTE:
				UnmuteShare();
				break;
			default:
			{
                theApp.DoVideoDeviceCommand(nID);
                theApp.DoAppShareCommand(nID);
				break;
			}
		}
	}
    else if (WM_PAINT == uMsg) {
        theApp.OnPaint(hwnd);
    }
    else if (WM_LBUTTONDBLCLK == uMsg) {
        theApp.OnDbClick(hwnd);
    }
    else if (g_nProxyMessageID == uMsg) {
        CProxyAuthenication dlg(hwnd);
        std::string *pszDescription = (std::string*)wParam;
        dlg.SetDescription(pszDescription->c_str());
        delete pszDescription;
        dlg.DoModal();
    }

	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

HWND CreateAWindow(int x, int y, int cx, int cy, HWND hParent, WORD menuId, int bkColor)
{
	WNDCLASS wc = { 0 };

	HINSTANCE hInstance = GetModuleHandle(NULL);
    wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = (WNDPROC)EndpointWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(void*);
	wc.hInstance = hInstance;
	wc.hIcon = 0;
	wc.hCursor = 0;
	wc.hbrBackground = (HBRUSH)GetStockObject(bkColor);
    wc.lpszMenuName = (menuId != 0) ? MAKEINTRESOURCE(menuId) : NULL;
	
    std::string sClassString(ENDPOINT_WIN_CLASS);
    if (menuId != 0) {
        sClassString += "_";
        sClassString += std::to_string(menuId);
    }
    sClassString += "_";
    sClassString += std::to_string(bkColor);

    wc.lpszClassName = sClassString.c_str();

	RegisterClass(&wc);

    DWORD dwBasic = WS_CLIPCHILDREN;
    DWORD dwStyle = hParent ? WS_CHILD | dwBasic : WS_OVERLAPPEDWINDOW | dwBasic;
    HWND hWnd = CreateWindow(wc.lpszClassName, NULL, dwStyle, x,
		y, cx, cy, hParent, NULL, 0, 0);

    WbxUI_SetMagicWindow(hWnd);
	ShowWindow(hWnd, SW_SHOW);
	return hWnd;
}

void CClickCallApp::ShowMuteWindow(void *pOverlapWindow, bool bShow)
{
    int i = 0;
    if (pOverlapWindow == g_winActive) {
        i = 5;
    }
    else{
        for (i = 0; i < 5; i++) {
            if (pOverlapWindow == m_filmStrips[i]) {
                break;
            }
        }
    }

    if (i > 5)
        return;

    if (IsWindow(g_wndMuteView[i])) {
        //MoveWindow(g_wndMuteView[i], 0, 0, 120, 30, TRUE);
    }
    else{
        g_wndMuteView[i] = CreateAWindow(0, 0, 120, 30, (HWND)pOverlapWindow, 100, WHITE_BRUSH);
    }
    ShowWindow(g_wndMuteView[i], bShow ? SW_SHOW : SW_HIDE);
}

void CClickCallApp::InitGUI()
{
    int nWindowHeight = 700;
    int nWindowWidth = 400;

    g_winMain = CreateAWindow(0, 0, nWindowHeight, nWindowWidth, NULL, IDC_CLICKCALL);
	WbxUI_ClearMagicWindow(g_winMain);
    ::SetWindowText(g_winMain, _T("wme-ref-app"));
	TestConfig::Instance().m_handleSharedWindow = g_winMain;
    RECT rcClient;
    GetClientRect(g_winMain, &rcClient);
    g_winActive = CreateAWindow(0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, g_winMain, TRUE);
    g_winLocal = CreateAWindow(rcClient.right - 160, rcClient.bottom - 90, 160, 90, g_winActive, TRUE);

    g_winScreenViewer = CreateAWindow(0, 0, 1024, 768, NULL, 0);
    if (g_winScreenViewer){
        ::SetWindowText(g_winScreenViewer, _T("Viewing screen sharing"));
        ::ShowWindow(g_winScreenViewer, SW_HIDE);
    }

    int i = 0;
    for (i = 1; i < 5; i++) {
        m_filmStrips[i] = CreateAWindow(0, 0, 0, 0, g_winMain, 0, BLACK_BRUSH);
    }
}

void CClickCallApp::Resize() 
{
    const unsigned int &videoStreams = TestConfig::i().m_uMaxVideoStreams;
    RECT rcClient;
    GetClientRect(g_winMain, &rcClient);
    if (videoStreams > 1)
        rcClient.bottom -= nChildWinHeight;

    MoveWindow(g_winActive, 0, 0, rcClient.right - rcClient.left, rcClient.bottom - rcClient.top, FALSE);
    MoveWindow(g_winLocal, rcClient.right - 160, rcClient.bottom - 90, 160, 90, FALSE);

    CM_ASSERTE_RETURN_VOID(videoStreams <= 4);
    int i = 0;
    rcClient.bottom += nChildWinHeight;
    rcClient.top = rcClient.bottom - nChildWinHeight;
    for (i = 1; i < videoStreams; i++) {
        rcClient.left = (i - 1) * nChildWinWidth;
        rcClient.right = i * nChildWinWidth;
        HWND hChild = m_filmStrips[i];
        MoveWindow(hChild, rcClient.left, rcClient.top, nChildWinWidth, nChildWinHeight, TRUE);
    }
}

void CClickCallApp::OnDbClick(HWND hWnd) 
{
    if (hWnd == g_winActive) {
        HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, m_sVenueUrl.size());
        memcpy(GlobalLock(hMem), m_sVenueUrl.c_str(), m_sVenueUrl.size());
        GlobalUnlock(hMem);
        OpenClipboard(0);
        EmptyClipboard();
        SetClipboardData(CF_TEXT, hMem);
        CloseClipboard();
    }
}

void CClickCallApp::ShowWindow(void *pOverlapWindow, bool bShow) 
{
    if(IsWindow((HWND)pOverlapWindow))
        ::ShowWindow((HWND)pOverlapWindow, bShow ? SW_SHOW : SW_HIDE);
}

void CClickCallApp::OnPaint(HWND hWnd)
{
    if (hWnd == g_winActive && m_sVenueUrl.length() > 0) {

        HDC dc = GetDC(g_winActive);
        RECT rc;
        GetClientRect(g_winActive, &rc);
        HBRUSH brush = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
        SelectObject(dc, brush);
        Rectangle(dc, rc.left, rc.top, rc.right, rc.bottom);
        SetBkMode(dc, TRANSPARENT);
        rc.left += 10;
        rc.top += 10;

        ::DrawTextA(dc, m_sVenueUrl.c_str(), m_sVenueUrl.length(), &rc, DT_LEFT | DT_EXTERNALLEADING | DT_WORDBREAK);
    }
    else {
        for (int i = 0; i < sizeof(g_wndMuteView)/sizeof(HWND); i++) {
            if (g_wndMuteView[i] == hWnd) {
                HDC dc = GetDC(g_wndMuteView[i]);
                std::string szUnAvailableTxt = "Unavailable - " + std::to_string(i % 5);
                RECT rc = { 0, 0, 120, 30 };
                DrawTextA(dc, szUnAvailableTxt.c_str(), szUnAvailableTxt.length(), &rc, 0);
                break;
            }
        }
    }
}

void CClickCallApp::onVenueUrl(std::string sVenueUrl)
{
    m_sVenueUrl = sVenueUrl;
    OnPaint(g_winActive);
}

void CClickCallApp::onCheckMultiVideo() 
{
    CM_ASSERTE_RETURN_VOID(g_winMain != NULL);
    const unsigned int &videoStreams = TestConfig::i().m_uMaxVideoStreams;
    if (videoStreams <= 1)
        return;

    RECT rcClient, rcWin;
    GetClientRect(g_winMain, &rcClient);
    GetWindowRect(g_winMain, &rcWin);
    rcWin.bottom += nChildWinHeight;
    MoveWindow(g_winMain, rcWin.left, rcWin.top, rcWin.right - rcWin.left,
        rcWin.bottom - rcWin.top, TRUE);

    int i = 1;
    rcClient.top = rcClient.bottom;
    CM_ASSERTE_RETURN_VOID(videoStreams <= 4);
    for (i = 1; i < videoStreams + 1; i++) {
        rcClient.left = (i - 1) * nChildWinWidth;
        rcClient.right = i * nChildWinWidth;
        HWND hChild = m_filmStrips[i];
        MoveWindow(hChild, rcClient.left, rcClient.top, nChildWinWidth, nChildWinHeight, TRUE);
        PeerCall::Instance()->pushRemoteWindow(hChild);
    }
}

void CClickCallApp::ShowScreenSharePreview()
{
	if (TestConfig::i().m_bAppshare &&  TestConfig::i().m_bSharer && TestConfig::i().m_bASPreview
		&& !g_wndASPreview && g_presenter)
	{
		g_wndASPreview = CreateAWindow(0, 0, 700, 400, NULL, 0);
		if (g_wndASPreview)
		{
			::SetWindowText(g_wndASPreview, _T("AS Preview"));
			ShowWindow(g_wndASPreview, SW_SHOW);
			g_presenter->ShowScreenSharePreview(g_wndASPreview);
		}
	}
}

void CClickCallApp::OnProxyCredentialRequired(const char* szProxy, uint16_t port, const char* szRealm)
{
    if (!TestConfig::i().m_sProxyUser.empty() && !TestConfig::i().m_sProxyPasswd.empty()) {
        wme::SetProxyUsernamePassword(TestConfig::i().m_sProxyUser.c_str(), TestConfig::i().m_sProxyPasswd.c_str());
        return;
    }
    std::string *pszDescription = new std::string();
    if (szProxy) {
        *pszDescription += szProxy;
        *pszDescription += ":";
        *pszDescription += std::to_string(port);
    }
    if (szRealm) {
        *pszDescription += ", Server says:";
        *pszDescription += szRealm;
    }

    PostMessage(g_winMain, g_nProxyMessageID, (WPARAM)pszDescription, 0);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	g_hInstance = hInstance;
    TestConfig::i().m_pAppSink = &theApp;

	OleInitialize(NULL);
	InitMediaEngine(false);


    std::string sAppName = "ClickCall_" + newUUID();
    InitMinidump(sAppName.c_str());
	WmeSetTraceMaxLevel(WME_TRACE_LEVEL_INFO);
	CCmThreadManager *pThreadManager = CCmThreadManager::Instance();
	ACmThread *pMainBase = pThreadManager->GetThread(TT_MAIN);
    SetProxyCredentialSink(&theApp);
    
    TAServer *g_taServer = NULL;
	CBackdoorAgent *g_backdoorAgent = NULL;
	const char* szTAPort = getCmdOption(__argv, __argv + __argc, "--ta");
	if (!szTAPort)
	{
		InitApp(__argc, __argv);
		
		if (!TestConfig::i().m_bAutoStart){
            //TestConfig::i().m_bLoopback = true; //comment: don't hard code ,please use command param --loopback
            //TestConfig::i().m_bCalliope = true; //comment: don't hard code ,please use command param --calliope
			CConfigDialog config(IDD_DIALOG_CONFIG, NULL);
			if (IDOK != config.DoModal())
				return 0;
		}

		theApp.InitGUI();
        theApp.LoadShareSource();
        theApp.LoadCameras();

		AddAutoLaunchShareSource();
		
		SetStunTraceSink(CTaIWmeStunTraceSink::GetInstance());

		SetTraceServerSink(CTaIWmeTraceServerSink::GetInstance());

        doStartCall(theApp.g_winActive, theApp.g_winLocal, theApp.g_winScreenViewer, &theApp);
		
		if (TestConfig::i().m_bLoopback)
		{
            theApp.g_presenter = &LoopbackCall::Instance()->m_endCaller;
		}
		else
		{
            theApp.g_presenter = PeerCall::Instance()->m_endCaller;
		}

		theApp.ShowScreenSharePreview();


	}
	else{
        theApp.InitGUI();
		TCHAR szFileName[1024] = { 0 };
		GetModuleFileName(g_hInstance, szFileName, sizeof(szFileName));
		PathRemoveFileSpec(szFileName);
		PathAppend(szFileName, "../../../../ta/ref-app/");

        g_backdoorAgent = new CBackdoorAgent(szFileName, theApp.g_winActive, theApp.g_winLocal,
            theApp.g_winScreenViewer, &theApp);
		unsigned short nTAPort = atoi(szTAPort);

		SetStunTraceSink(CTaIWmeStunTraceSink::GetInstance());

		SetTraceServerSink(CTaIWmeTraceServerSink::GetInstance());

		g_taServer = new TAServer(g_backdoorAgent);
		CCmInetAddr addr("127.0.0.1", nTAPort);
		g_taServer->Init(addr);
	}
	TestConfig::i().Dump();
	pMainBase->OnThreadRun();

	if (!szTAPort)
		doStop();
	else{
		delete g_taServer;
		delete g_backdoorAgent;
	}

    g_statsWnd.Stop();
    PeerCall::Instance()->m_linusCall.Close();
	LoopbackCall::Instance()->m_linusCallee.Close();
	LoopbackCall::Instance()->m_linusCaller.Close();

	UninitMediaEngine();
	OleUninitialize();

	return 0;
}
