// QRCodeCtrl.cpp: implementation of the CQRCodeCtrl class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "QRCodeCtrl.h"

#define QRCODE_DEFAULT_LEVEL			1
#define QRCODE_DEFAULT_VERSION			0
#define QRCODE_DEFAULT_MASKINGNO		-1



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQRCodeCtrl::CQRCodeCtrl()
{

}

CQRCodeCtrl::~CQRCodeCtrl()
{

}


LRESULT CQRCodeCtrl::OnEraseBkgnd(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = TRUE;
	return 1;
}

LRESULT CQRCodeCtrl::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;


	return 0;
}

LRESULT CQRCodeCtrl::OnLButtonDoubleClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;

	HWND hwndParent = GetParent();

	if (hwndParent)
		::PostMessage(hwndParent, uMsg, wParam, lParam);

	return 0;
}

LRESULT CQRCodeCtrl::OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;
	
	Draw();
	
	return 0;
}

void CQRCodeCtrl::SetCode(LPCSTR lpszCode)
{
	m_strCode = lpszCode;

	if (IsWindow())
	{
		InvalidateRect(NULL);

		m_QREncoder.EncodeData(QRCODE_DEFAULT_LEVEL, QRCODE_DEFAULT_VERSION,
			1, QRCODE_DEFAULT_MASKINGNO, lpszCode);
	}
}


void CQRCodeCtrl::Draw()
{
	std::string strCode;
	if (m_strCode.empty())
	{
		strCode = "000000";
	}
	else
	{
		strCode = m_strCode;
	}

	bool bAutoExtent = 1;
	
	
	if (m_QREncoder.EncodeData(QRCODE_DEFAULT_LEVEL, QRCODE_DEFAULT_VERSION,
		bAutoExtent, QRCODE_DEFAULT_MASKINGNO, strCode.c_str()))
	{

		m_QREncoder.ShowImage(m_hWnd);
	}
}
