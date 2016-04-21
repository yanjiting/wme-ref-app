#pragma once
class CProxyAuthenication
{
public:
    CProxyAuthenication(HWND hParent, int resId = IDD_PROXY_AUTHENTICATION);
    virtual ~CProxyAuthenication();

public:
    void SetDescription(const char* szDesc);
    int DoModal(void);

protected:
    static INT_PTR CALLBACK DialogProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    BOOL CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
    void OnOKCancel(UINT id);

private:
    CCmString m_sProxyInfo;
    int m_nResId;
    HWND m_hParent;

    HWND m_hWnd;
    static CProxyAuthenication *m_pSaveThis;
};
