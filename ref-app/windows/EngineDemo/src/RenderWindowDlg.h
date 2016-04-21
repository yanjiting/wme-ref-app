#pragma once


// CRenderWindowDlg dialog

class CRenderWindowDlg : public CDialog
{
	DECLARE_DYNAMIC(CRenderWindowDlg)

public:
	CRenderWindowDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRenderWindowDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_RENDER_WINDOW };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
