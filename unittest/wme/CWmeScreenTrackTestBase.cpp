#include "CWmeScreenTrackTestBase.h"
#if defined (WIN32)
#include <Windows.h>	
#include <tchar.h>
#endif

//==================================================================================
//UT Util Intefaces


#if defined(WIN32)
void* CreateWindowX()
{
	HINSTANCE hInstance;
	hInstance = GetModuleHandle(NULL);
	WNDCLASS Render_WND;	
	Render_WND.cbClsExtra = 0;
	Render_WND.cbWndExtra = 0;
	Render_WND.hCursor = LoadCursor(hInstance, IDC_ARROW);		
	Render_WND.hIcon = LoadIcon(hInstance, IDI_APPLICATION);	
	Render_WND.lpszMenuName = NULL;								
	Render_WND.style = CS_HREDRAW | CS_VREDRAW;					
	Render_WND.hbrBackground = (HBRUSH)COLOR_WINDOW;			
	Render_WND.lpfnWndProc = DefWindowProc;				
	Render_WND.lpszClassName = _T("RenderWindow");				
	Render_WND.hInstance = hInstance;
	RegisterClass(&Render_WND);
	return (void*)CreateWindow(_T("RenderWindow"),_T("CreateMockWindow"),WS_OVERLAPPEDWINDOW,50,50,640,480,NULL,NULL,hInstance,NULL);
    return NULL;
}

void DestroyWindowX(void* window)
{
	if(window==NULL) return ;
	
    HWND hWnd = (HWND)window;
	DestroyWindow(hWnd);
}


void ShowWindowX(void* window)
{
	if(window==NULL) return ;
	
    HWND hWnd = (HWND)window;
	ShowWindow(hWnd, SW_SHOW);
	UpdateWindow(hWnd);
}

void LaunchOneApp() {

}

#elif defined(MACOS)
//ObjectC->CWmeScreenTrackTestBaseMac.mm
#else
void* CreateWindowX()
{
    return NULL;
}

void DestroyWindowX(void* window)
{
}


void ShowWindowX(void* window)
{
}

void LaunchOneApp() {

}

#endif



////////
//==========================================================================
//

#include "WseTaskBase.h"
#include "WseTaskThread.h"

class CThreadX : public WSEUTIL::ComTaskThread
	, public WSEUTIL::ITimerHandler
{
public:
	CThreadX(){
		m_fnThreadRunFunction = WBXNull;
		m_pThreadFunctionObj = WBXNull;
	}
	virtual ~CThreadX(){

	}

	//
	WBXResult CreateCaptureThread(ThreadRunFunction fnThreadRunFunction,WBXLpvoid pThreadFunctionObj){
		m_fnThreadRunFunction = fnThreadRunFunction;
		m_pThreadFunctionObj = pThreadFunctionObj;
		WBXResult ret =Create("scn-cap", 1, TF_JOINABLE);
		if(ret!=WBX_SUCCESS)
			DestroyCaptureThread();
		return ret;
	}
	WBXResult DestroyCaptureThread(){
		Stop();
		Join();
		Destory(0);
		return WBX_SUCCESS;
	}

	virtual void OnThreadInit(){
		WSEUTIL::ComTaskThread::OnThreadInit();

		WSEUTIL::ITimerQueue* pTimerQueue = GetTimerQueue();
		if(pTimerQueue!=WBXNull){
			CCmTimeValue interval(0,THREAD_HEART_BEAT_TIME*1000);//50 ms,
			pTimerQueue->ScheduleTimer(this, NULL, interval, 0);
		}
	}

	//WSEUTIL::ITimerHandler
	virtual void OnTimeout(const CCmTimeValue &aCurTime, LPVOID aArg){
		if(m_fnThreadRunFunction!=WBXNull)
			m_fnThreadRunFunction(m_pThreadFunctionObj);
		return ;
	}

protected:
	ThreadRunFunction m_fnThreadRunFunction;
	WBXLpvoid m_pThreadFunctionObj;
};



WBXLpvoid CreateThreadX(ThreadRunFunction funCallback, WBXLpvoid pObj)
{
	CThreadX * pCThreadX = new CThreadX;
	if(pCThreadX==NULL)
		return NULL;
	if(pCThreadX->CreateCaptureThread(funCallback,pObj)!=WBX_SUCCESS){
		pCThreadX->DestroyCaptureThread();
		return NULL;
	}
	return pCThreadX;

}
void DestroyThreadX(WBXLpvoid pThread)
{
	CThreadX * pCThreadX = (CThreadX *) pThread;
	if(pCThreadX)
		pCThreadX->DestroyCaptureThread();
}

#ifdef _WIN32
#include <windows.h>
#else 
#include <unistd.h>
#endif
void SleepX(WBXInt32 nMs)
{
#ifdef _WIN32
	::Sleep(nMs);
#else 
	usleep(nMs*1000);
#endif
}