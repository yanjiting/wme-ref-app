#pragma once

#include <vector>

class CQRCodeCtrl;

class CQRView : public CWindowImpl<CQRView, CWindow>
{
public:
	CQRView();
	virtual ~CQRView();

	BEGIN_MSG_MAP_EX(CQRView)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDoubleClick)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

protected:
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnLButtonDoubleClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
public:
	void Initialize();
	void SetQRGrid(UINT nRowCount, UINT nColCount);
	void SetQRCode(UINT nRow, UINT nCol, LPCSTR lpszQRCode);
	UINT GetRowCount() { return m_nRowCount;  }
	UINT GetColCount() { return m_nColCount;  }
private:
	CQRCodeCtrl *GetQRCtrl(UINT nRow, UINT nCol);
	void InitOneQRCtrl(UINT nRow, UINT nCol);
	void Cleanup();
	void InitQRCtrls();
	ATOM RegisterClass(HINSTANCE hInstance);
private:
	std::vector<CQRCodeCtrl*> m_vctQRCtrls;
	UINT   m_nRowCount;
	UINT   m_nColCount;
};