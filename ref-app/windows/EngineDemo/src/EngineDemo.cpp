// EngineDemo.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "EngineDemo.h"
#include "EngineDemoDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#include <windows.h>
#include <Dbghelp.h>
using namespace std;

#pragma comment( lib, "DbgHelp" )

LONG WINAPI MyUnhandledExceptionFilter(struct _EXCEPTION_POINTERS* ExceptionInfo    )
{
    HANDLE lhDumpFile = CreateFile(_T("EngineDemo.dmp"), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL ,NULL);
 
    MINIDUMP_EXCEPTION_INFORMATION loExceptionInfo;
    loExceptionInfo.ExceptionPointers = ExceptionInfo;
    loExceptionInfo.ThreadId = GetCurrentThreadId();
    loExceptionInfo.ClientPointers = TRUE;
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),lhDumpFile, MiniDumpNormal, &loExceptionInfo, NULL, NULL);
 
    CloseHandle(lhDumpFile);
 
    return EXCEPTION_EXECUTE_HANDLER;
}

// CEngineDemoApp

BEGIN_MESSAGE_MAP(CEngineDemoApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CEngineDemoApp construction

CEngineDemoApp::CEngineDemoApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CEngineDemoApp object

CEngineDemoApp theApp;


// CEngineDemoApp initialization

BOOL CEngineDemoApp::InitInstance()
{
	SetUnhandledExceptionFilter(MyUnhandledExceptionFilter);
//TODO: call AfxInitRichEdit2() to initialize richedit2 library.
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

#ifndef ENABLE_COMMAND_LINE
	CEngineDemoDlg dlg;
#else
	//Command-line inputs added by jonsnyde@cisco.com 11/2013
	
	CCustomCommandLineInfo oInfo;
	ParseCommandLine(oInfo);
	CEngineDemoDlg dlg(NULL , &oInfo);
#endif

	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
