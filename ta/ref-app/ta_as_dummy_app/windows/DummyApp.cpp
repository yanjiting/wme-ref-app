// DummyApp.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "DummyApp.h"
#include "DummyBackdoorAgent.h"
#include "QRview.h"
#include "DummyDbg.h"
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
extern CDummyBackdoorAgent *g_DummyBackdoorAgent;

//===============================================================================================
//
typedef
enum {
	WINUNKNOW = 0, /*!< don't know OS */
	WIN31,		/*!< don't WINNT 3.1 OS */
	WINNT,		/*!< don't WINNT	OS */
	WIN95,		/*!< don't Window 95 OS */
	WIN98,		/*!< don't Window 98 OS */
	WINME,		/*!< don't Window ME 3.1 OS */
	WIN2000,	/*!< don't Window 2000 OS */
	WINXP,		/*!< don't Window XP OS */
	WIN2003,		/*!< don't Window 2003 OS */
	WINVista,	/*!< don't Window Vista OS */  //fixed bug #344127,#344793,#345305
	WIN7,
	WIN8,

	WINNew = 20		/* */
}WIN_OS;
WIN_OS WbxGetOSVersion()
{
	static WIN_OS sWinOS = WINUNKNOW;

	if (sWinOS != WINUNKNOW) return sWinOS;

	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;

	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi)))
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO *)&osvi))
			return WINUNKNOW;
	}

	switch (osvi.dwPlatformId)
	{
		// Tests for Windows NT product family.
	case VER_PLATFORM_WIN32_NT:

		// Test for the product.
		if (osvi.dwMajorVersion <= 4)
		{
			sWinOS = WINNT;
			return WINNT;
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
		{
			sWinOS = WIN2000;
			return WIN2000;
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
		{
			sWinOS = WINXP;
			return WINXP;
		}

		if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
		{
			sWinOS = WIN2003;
			return WIN2003;
		}

		if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
		{
			sWinOS = WINVista;
			return WINVista;
		}

		//fixed bug #344127,#344793,#345305
		if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
		{
			sWinOS = WIN7;
			return WIN7;
		}

		if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
		{
			sWinOS = WIN8;
			return WIN8;
		}

		if (osvi.dwMajorVersion >= 6)
		{
			sWinOS = WINNew;
			return WINNew;  //20090217
		}

		break;

		// Test for the Windows 95 product family.
	case VER_PLATFORM_WIN32_WINDOWS:

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 0)
		{
			sWinOS = WIN95;
			return WIN95;
		}

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 10)
		{
			sWinOS = WIN98;
			return WIN98;
		}

		if (osvi.dwMajorVersion == 4 && osvi.dwMinorVersion == 90)
		{
			sWinOS = WINME;
			return WINME;
		}
		break;
	}
	return WINUNKNOW;

}

//===============================================================================================
//Win 8.1//In Shcore.dll
//#include <ShellScalingAPI.h>
typedef enum PROCESS_DPI_AWARENESS {
	PROCESS_DPI_UNAWARE = 0,
	PROCESS_SYSTEM_DPI_AWARE = 1,
	PROCESS_PER_MONITOR_DPI_AWARE = 2
} PROCESS_DPI_AWARENESS;
typedef HRESULT(WINAPI *FunSetProcessDpiAwareness)(PROCESS_DPI_AWARENESS value);
typedef HRESULT(WINAPI *FunGetProcessDpiAwareness)(HANDLE hprocess, PROCESS_DPI_AWARENESS *value);
//Win8.0//in User32.dll
typedef BOOL(WINAPI *FunSetProcessDPIAware0)();
typedef BOOL(WINAPI *FunIsProcessDPIAware)();

void DisalbeAppDPIAware()
{
	if (WbxGetOSVersion() >= WIN8){

		FunSetProcessDpiAwareness pSetProcessDpiAwareness(NULL);

		HMODULE hShcore = LoadLibrary(_T("Shcore.DLL"));
		if (hShcore)
		{//Win8.1
			FunSetProcessDpiAwareness pSetProcessDpiAwareness = (FunSetProcessDpiAwareness)GetProcAddress(hShcore,
				"SetProcessDpiAwareness");
			FunGetProcessDpiAwareness pGetProcessDpiAwareness = (FunGetProcessDpiAwareness)GetProcAddress(hShcore,
				"GetProcessDpiAwareness");
			if (pGetProcessDpiAwareness && pSetProcessDpiAwareness)
			{
				PROCESS_DPI_AWARENESS ePROCESS_DPI_AWARENESS;
				if (pGetProcessDpiAwareness(::GetCurrentProcess(), &ePROCESS_DPI_AWARENESS) == S_OK){
					if (!(ePROCESS_DPI_AWARENESS == PROCESS_SYSTEM_DPI_AWARE))
						pSetProcessDpiAwareness(PROCESS_SYSTEM_DPI_AWARE);
				}
			}
			FreeLibrary(hShcore);
		}
		else { //WIN8
			HMODULE hUser32 = LoadLibrary(_T("User32.DLL"));
			if (hUser32)
			{//Win8.1
				FunSetProcessDPIAware0 pSetProcessDpiAwareness = (FunSetProcessDPIAware0)GetProcAddress(hUser32,
					"SetProcessDpiAwareness");
				FunIsProcessDPIAware pIsProcessDPIAware = (FunIsProcessDPIAware)GetProcAddress(hUser32,
					"IsProcessDPIAware");

				if (pSetProcessDpiAwareness && pIsProcessDPIAware){
					if (!pIsProcessDPIAware())
						pSetProcessDpiAwareness();
				}
				FreeLibrary(hUser32);
			}
		}
	}
}
// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

CQRView g_CQRView;

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	InitAgentByInputParams(__argc, (const char **)__argv);
	DisalbeAppDPIAware();
	
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_DUMMYAPP, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DUMMYAPP));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	CleanupAgent();
	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DUMMYAPP));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DUMMYAPP);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP,
      0, 0, 1, 1, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		if (g_DummyBackdoorAgent &&
			DummyConfig::Instance().m_strShowQRCode.size() > 0)
			g_DummyBackdoorAgent->backdoorShowQRCodeView(DummyConfig::Instance().m_strShowQRCode);
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		HideQRCodeView();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}


void ShowQRCodeView(int x, int y, int w, int h, int row, int column, std::vector<std::string>& v_qr_contents)
{
	if (w <= 0)
		w = GetSystemMetrics(SM_CXSCREEN);
	if (h <= 0)
		h = GetSystemMetrics(SM_CYSCREEN);

	g_CQRView.Initialize();
	g_CQRView.MoveWindow(x, y, w, h);
	g_CQRView.SetQRGrid(row, column);

	int nRow = 0;
	int nCol = 0;
	std::vector<std::string>::iterator it;
	for (it = v_qr_contents.begin(); it != v_qr_contents.end(); it++){
		g_CQRView.SetQRCode(nRow, nCol, it->c_str());

		nCol++;
		if (!(nCol < column)){
			nCol = 0;
			nRow++;
		}
		if (nRow >= row)
			break;
	}

}
void HideQRCodeView()
{
	if (g_CQRView.IsWindow())
		g_CQRView.DestroyWindow();
}