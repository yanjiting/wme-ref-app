#pragma once


// CDumppingDataFlagDlg dialog

class CDumppingDataFlagDlg : public CDialog
{
	DECLARE_DYNAMIC(CDumppingDataFlagDlg)

public:
	CDumppingDataFlagDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDumppingDataFlagDlg();
	unsigned int GetDumpFlag();
	void SetDumpFlag(unsigned int uDumpFlag);
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_DIALOG_DUMP_DATA_OPTIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	unsigned int m_uDumpFlag;
public:
	afx_msg void OnBnClickedOk();
};
