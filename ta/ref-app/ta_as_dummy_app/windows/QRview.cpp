#include "stdafx.h"
#include "qrcodectrl.h"
#include "QRview.h"



CQRView::CQRView()
{
	m_nRowCount = 4;
	m_nColCount = 8;
}

CQRView::~CQRView()
{
	Cleanup();
}

void CQRView::Cleanup()
{
	for (UINT i = 0; i < m_vctQRCtrls.size(); i++)
	{
		if (m_vctQRCtrls[i]->IsWindow())
			m_vctQRCtrls[i]->DestroyWindow();
		SAFE_DELETE(m_vctQRCtrls[i]);
	}

	m_vctQRCtrls.clear();
}


LRESULT CQRView::OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;


	InitQRCtrls();

	return 0;
}

LRESULT CQRView::OnLButtonDoubleClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL &bHandled)
{
	bHandled = FALSE;
	MoveWindow(0, 0, 800, 600);
	//DestroyWindow();
	//Cleanup();

	return 0;
}

CQRCodeCtrl *CQRView::GetQRCtrl(UINT nRow, UINT nCol)
{
	CQRCodeCtrl *p = NULL;
	if (nRow * m_nColCount + nCol >= m_vctQRCtrls.size())
	{
		p = new CQRCodeCtrl;
		m_vctQRCtrls.push_back(p);
	}
	else
	{
		p = m_vctQRCtrls[nRow * m_nColCount + nCol];
	}

	return p;
}

void CQRView::InitOneQRCtrl(UINT nRow, UINT nCol)
{
	CQRCodeCtrl *p = GetQRCtrl(nRow, nCol);

	if (!p->IsWindow())
	{
		CRect rcClient;
		GetClientRect(rcClient);
		int nWidth = rcClient.Width() / m_nColCount;
		int nHeight = rcClient.Height() / m_nRowCount;

		RECT rcQR = { nCol*nWidth, nRow * nHeight, nCol*nWidth + nWidth, nRow * nHeight + nHeight };
		HWND hWnd = p->Create(m_hWnd, rcQR, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
	}

}


void CQRView::InitQRCtrls()
{
	for (int row = 0; row < m_nRowCount; row++)
	{
		for (int col = 0; col < m_nColCount; col++)
		{
			InitOneQRCtrl(row, col);
		}
	}
}

void CQRView::Initialize()
{
	if (m_hWnd)
		return;

	Create(GetDesktopWindow(), rcDefault, NULL, WS_POPUP | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS , WS_EX_TOPMOST ); 
	

	if (!m_hWnd)
	{
		return;
	}

}

void CQRView::SetQRGrid(UINT nRowCount, UINT nColCount) 
{ 
	Cleanup();

	m_nRowCount = nRowCount; 
	m_nColCount = nColCount;

	InitQRCtrls();
}


void CQRView::SetQRCode(UINT nRow, UINT nCol, LPCSTR lpszQRCode)
{
	CQRCodeCtrl *p = GetQRCtrl(nRow, nCol);

	p->SetCode(lpszQRCode);
}

