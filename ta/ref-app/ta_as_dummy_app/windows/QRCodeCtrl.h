// QRCodeCtrl.h: interface for the CQRCodeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_QRCODECTRL_H__66CAE52F_F222_4793_8A92_B915C53B057C__INCLUDED_)
#define AFX_QRCODECTRL_H__66CAE52F_F222_4793_8A92_B915C53B057C__INCLUDED_

#include "qr_encode_win.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CQRCodeCtrl  : public CWindowImpl<CQRCodeCtrl, CWindow>
{
public:

	CQRCodeCtrl();
	virtual ~CQRCodeCtrl();
	
	BEGIN_MSG_MAP_EX(CQRCodeCtrl) 
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDoubleClick)
		REFLECT_NOTIFICATIONS()
	END_MSG_MAP()

	void SetCode(LPCSTR lpszCode);
	
protected:
	LRESULT OnLButtonDoubleClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	void Draw();
	LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	LRESULT OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled);
	
private:
	CQR_Encode_win      m_QREncoder;
	std::string  	m_strCode;
	
};

#endif // !defined(AFX_QRCODECTRL_H__66CAE52F_F222_4793_8A92_B915C53B057C__INCLUDED_)
