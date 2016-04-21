#if !defined(CLICKCALL_WME_REF_APP_CONFIG_DIALOG__INCLUDED_)
#define CLICKCALL_WME_REF_APP_CONFIG_DIALOG__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CConfigDialog  
{
	public:
		CConfigDialog(int nResId, HWND hParent = NULL);
		virtual ~CConfigDialog();

		int DoModal(void);
		
        static INT_PTR CALLBACK DialogProcStatic(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
		BOOL CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

		void OnOK(void);
		void OnCancel(void);
	protected:
		void InitScreenSource();
	protected:
		int m_nResId;
		HWND m_hParent;

		static HWND m_hWindow;
		static void* m_lSaveThis;

		static void ScreenSourceChanged();
};


#endif // !defined(CLICKCALL_WME_REF_APP_CONFIG_DIALOG__INCLUDED_)
