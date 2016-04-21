#pragma once

HWND CreateAWindow(int x, int y, int cx, int cy, HWND hParent, WORD menuId,
    int bkColor = GRAY_BRUSH);

class CClickCallApp : public IAppSink, public IProxyCredentialSink
{
public:
    CClickCallApp() {
        g_winMain = NULL;
        g_winActive = NULL;
        g_winLocal = NULL;
        g_winScreenViewer = NULL;
		g_wndASPreview = NULL;
        g_wndCameraPreview = NULL;
        memset(m_filmStrips, 0, sizeof(m_filmStrips));
        memset(g_wndMuteView, 0, sizeof(g_wndMuteView));
    }

    void InitGUI();
    void LoadShareSource();
    void LoadCameras();
    void OnShareChanged(BOOL bShareDesktop);
    void ShareDesktop(int nMenuItem);
    void ShareOneApp(int nMenuItem);
    void UnshareOneApp(int nMenuItem);
    void ClickOneApp(int nMenuItem);
    void DoAppShareCommand(int nCmdID);
    void DoVideoDeviceCommand(int nID);

    void OnPaint(HWND hWnd);
    void OnDbClick(HWND hWnd);
    void Resize();
    void DoExit(HWND hWnd);
	void ShowScreenSharePreview();

    ///IAppSink
    virtual void onCheckMultiVideo();
    virtual void onVenueUrl(std::string sVenueUrl);
    virtual void ShowWindow(void *pOverlapWindow, bool bShow);
    virtual void ShowMuteWindow(void *pOverlapWindow, bool bShow);

    //Get the proxy credential for particular proxy server:port
    virtual void OnProxyCredentialRequired(const char* szProxy, uint16_t port, const char* szRealm);

public:
    HWND g_winMain;
    HWND g_winActive;
    HWND g_winLocal;
	HWND g_winScreenViewer; 
	HWND g_wndASPreview;
    HWND g_wndCameraPreview;
    std::string m_sCurrentCamera;
    std::map <int, std::string>   g_mapAppMenuItem;
    std::map <int, std::string>   g_mapDesktopMenuItem;
    std::map <std::string, IWmeMediaDevice *> g_cameras;
    Endpoint *g_presenter = NULL;
    BOOL g_bShareDesktop = TRUE;
    BOOL g_bShareStarted = FALSE;
    HWND g_wndMuteView[6];
    HWND m_filmStrips[5];
    std::string m_sVenueUrl;
    CCameraPreview m_cameraPreview;

private:
    const int nChildWinHeight = 90;
    const int nChildWinWidth = 160;
};
