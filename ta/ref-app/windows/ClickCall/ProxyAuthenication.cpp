#include "ClickCall.h"
#include "Windowsx.h"
#include "Resource.h"
#include "ProxyAuthenication.h"

CProxyAuthenication *CProxyAuthenication::m_pSaveThis = NULL;

CProxyAuthenication::CProxyAuthenication(HWND hParent, int resId)
    :m_hParent(hParent), m_nResId(resId)
{
    m_pSaveThis = this;
    m_hWnd = NULL;
}

CProxyAuthenication::~CProxyAuthenication()
{
}

int CProxyAuthenication::DoModal(void)
{
    return (DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(m_nResId), m_hParent, DialogProcStatic));
}

void CProxyAuthenication::SetDescription(const char* szDesc)
{
    if (szDesc != NULL) {
        m_sProxyInfo = szDesc;
    }
}

INT_PTR CALLBACK CProxyAuthenication::DialogProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    CM_ASSERTE(m_pSaveThis != NULL);

    if (m_pSaveThis->m_hWnd == NULL)
    {
        m_pSaveThis->m_hWnd = hDlg;
    }

    return(m_pSaveThis->DialogProc(hDlg, message, wParam, lParam));
}

BOOL CALLBACK CProxyAuthenication::DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    BOOL bRet = FALSE;
    if (message == WM_INITDIALOG) {
        Edit_SetText(GetDlgItem(hDlg, ID_PROXY_DESCRIPTION), m_sProxyInfo.c_str());
    }
    else if (message == WM_COMMAND){
        UINT id = LOWORD(wParam);
        switch (id){
        case IDOK:
        case IDCANCEL:
            bRet = TRUE;
            OnOKCancel(id);
            break;
        default:
            break;
        }
    }
    return bRet;
}

void CProxyAuthenication::OnOKCancel(UINT id)
{
    if (id == IDOK) {
        char szUserName[2048] = { 0 };
        int nMaxChar = 2048;
        Edit_GetText(GetDlgItem(m_hWnd, ID_PROXY_USERNAME), szUserName, nMaxChar);

        char szPassword[2048] = { 0 };
        nMaxChar = 2048;
        Edit_GetText(GetDlgItem(m_hWnd, ID_PROXY_PASSWORD), szPassword, nMaxChar);

        if (szUserName[0] != 0 || szPassword[0] != 0) {
            wme::SetProxyUsernamePassword(szUserName, szPassword);
            EndDialog(m_hWnd, id);
            return;
        }
    }

    wme::SetProxyUsernamePassword(NULL, NULL);
    EndDialog(m_hWnd, id);
}
