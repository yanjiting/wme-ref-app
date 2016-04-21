#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <stdlib.h>     //for using the function sleep
#include "./windows/share_source_util_win.h"
#include <atlbase.h>

using namespace testing;

LPCWSTR ModuleName = TEXT("notepad.exe");
LPCWSTR ApplicationName = TEXT("Untitled - Notepad");
LPCWSTR ApplicationName_CN = TEXT("无标题 - 记事本");

class CShareSourceUtilWinTest : public Test
{
public:
    CShareSourceUtilWinTest::CShareSourceUtilWinTest()
    {
        m_processID = 0;
    }

    virtual void SetUp()
    {
        USES_CONVERSION;

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_SHOWMINNOACTIVE;
        ZeroMemory(&pi, sizeof(pi));


        WCHAR szSystemRoot[MAX_PATH] = {0};
        if (!::GetSystemDirectory(szSystemRoot, sizeof(szSystemRoot))) {
            _tcsncpy(szSystemRoot, _T("c:\\Windows\\System32"), sizeof(szSystemRoot));
        }

        if (szSystemRoot[_tcslen(szSystemRoot) - 1] != _T('\\')) {
            _tcsncat(szSystemRoot, _T("\\"), sizeof(szSystemRoot));
        }

        _tcsncat(szSystemRoot, ModuleName, sizeof(szSystemRoot));

        if (CreateProcess(szSystemRoot,   //module name
                          NULL,           // Command line
                          NULL,           // Process handle not inheritable
                          NULL,           // Thread handle not inheritable
                          FALSE,          // Set handle inheritance to FALSE
                          0,              // No creation flags
                          NULL,           // Use parent's environment block
                          NULL,           // Use parent's starting directory
                          &si,            // Pointer to STARTUPINFO structure
                          &pi)            // Pointer to PROCESS_INFORMATION structure
           ) {
            m_processID = pi.dwProcessId;
            DWORD dwSessionID = -1;
            ProcessIdToSessionId(m_processID, &dwSessionID);
            HWINSTA hWorkstation = GetProcessWindowStation();
            TCHAR szWorkStationName[256] = { 0 };
            DWORD nLen = 256;
            GetUserObjectInformation(hWorkstation, UOI_NAME, szWorkStationName, 256, &nLen);
            WBXAS_INFO_TRACE("create szSystemRoot=" << W2A(szSystemRoot)
                             << ",dwSessionID = " << dwSessionID
                             << ",currentWorkStation = " << W2A(szWorkStationName));
        } else {
            WBXAS_ERROR_TRACE("fail create process szSystemRoot=" << szSystemRoot);
        }
    }
    virtual void TearDown()
    {
        if (m_processID) {
            DWORD dwDesiredAccess = PROCESS_TERMINATE;
            BOOL  bInheritHandle  = FALSE;
            HANDLE hProcess = OpenProcess(dwDesiredAccess, bInheritHandle, m_processID);
            if (hProcess == NULL) {
                return;
            }
            BOOL result = TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
        }
    }

protected:
    TCHAR m_cBuff[MAX_PATH];
    WBXInt32 m_processID;
};


TEST_F(CShareSourceUtilWinTest, GetExePath)
{
    Sleep(200);
    ASSERT_TRUE(CShareSourceUtilWin::GetExePath(m_processID, m_cBuff, MAX_PATH) == TRUE);
    // TODO: test m_cBuff
    ASSERT_TRUE(CShareSourceUtilWin::GetExePath(m_processID, m_cBuff, 1) == FALSE);
    ASSERT_TRUE(CShareSourceUtilWin::GetExePath(m_processID, NULL, MAX_PATH) == FALSE);
}

#ifdef _DEBUG
TEST_F(CShareSourceUtilWinTest, GetApplicationName)
#else
TEST_F(CShareSourceUtilWinTest, DISABLED_GetApplicationName)
#endif
{
    USES_CONVERSION;
    Sleep(200);
    ASSERT_TRUE(CShareSourceUtilWin::GetApplicationName(m_processID, m_cBuff, MAX_PATH) == TRUE);
    WBXAS_INFO_TRACE("CShareSourceUtilWinTest GetApplicationName ApplicationName=" << W2A(ApplicationName));
    WBXAS_INFO_TRACE("CShareSourceUtilWinTest GetApplicationName m_cBuff=" << W2A(m_cBuff));
    ASSERT_TRUE(0 == wcscmp(ApplicationName, m_cBuff) || 0 == wcscmp(ApplicationName_CN, m_cBuff));
    ASSERT_TRUE(CShareSourceUtilWin::GetApplicationName(m_processID, NULL, MAX_PATH) == FALSE);
    ASSERT_TRUE(CShareSourceUtilWin::GetApplicationName(m_processID, m_cBuff, 1) == TRUE);
    ASSERT_TRUE(0 == wcscmp(_T(""), m_cBuff));
    ASSERT_TRUE(CShareSourceUtilWin::GetApplicationName(m_processID, m_cBuff, 0) == FALSE);
}

#ifdef _DEBUG
TEST_F(CShareSourceUtilWinTest, GetIcon)
#else
TEST_F(CShareSourceUtilWinTest, DISABLED_GetIcon)
#endif
{
    Sleep(200);
    HICON hIcon = NULL;
    ASSERT_TRUE(CShareSourceUtilWin::GetIcon(m_processID, hIcon) == TRUE);
    //TODO: test hIcon
}