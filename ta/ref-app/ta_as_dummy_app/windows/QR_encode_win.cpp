#include "stdafx.h"
#include "QR_encode_win.h"


CQR_Encode_win::CQR_Encode_win()
{
	m_pSymbleBitmap = NULL;
	m_pSymbleDC = NULL;

}

CQR_Encode_win::~CQR_Encode_win()
{
	if (m_pSymbleDC != NULL)
	{
		m_pSymbleDC->SelectBitmap(m_OldBitmap);
		
		m_pSymbleDC->DeleteDC();
		SAFE_DELETE( m_pSymbleBitmap);
		SAFE_DELETE(m_pSymbleDC);
	}
}

void CQR_Encode_win::MakeImage()
{
	if (m_pSymbleDC != NULL)
	{
		m_pSymbleDC->SelectBitmap(m_OldBitmap);
		
		SAFE_DELETE( m_pSymbleBitmap);
		SAFE_DELETE( m_pSymbleDC);
	}
	
	int nSymbleSize = m_nSymbleSize + (QR_MARGIN * 2);
	
	m_pSymbleBitmap = new CBitmap;
	m_pSymbleBitmap->CreateBitmap(nSymbleSize, nSymbleSize, 1, 1, NULL);
	
	m_pSymbleDC = new CDC;
	m_pSymbleDC->CreateCompatibleDC(NULL);
	m_OldBitmap.m_hBitmap = (HBITMAP)m_pSymbleDC->SelectBitmap(*m_pSymbleBitmap);
	m_pSymbleDC->PatBlt(0, 0, nSymbleSize, nSymbleSize, WHITENESS);
	
	int i, j;
	for (i = 0; i < nSymbleSize; ++i)
	{
		for (j = 0; j < nSymbleSize; ++j)
		{
			if (m_byModuleData[i][j])
			{
				m_pSymbleDC->SetPixel(i + QR_MARGIN, j + QR_MARGIN, RGB(0, 0, 0));
			}
		}
	}
}

void CQR_Encode_win::ShowImage(void* wnd)
{
	CRect rcClient;
	::GetClientRect((HWND)wnd, &rcClient);

	CPaintDC dc((HWND)wnd);
    dc.FillSolidRect(&rcClient, RGB(255, 255, 255));

	if (m_pSymbleDC != NULL)
	{
		int nSrcSize = m_nSymbleSize + (QR_MARGIN * 2);;
		int nDstSize = min(rcClient.Width(), rcClient.Height());

		CDC dcWorkDC;
		dcWorkDC.CreateCompatibleDC(NULL);

		CBitmap bmpWorkBitmap;
		bmpWorkBitmap.CreateBitmap(nDstSize, nDstSize, 1, 1, NULL);

		HBITMAP hOldBitmap = dcWorkDC.SelectBitmap(bmpWorkBitmap.m_hBitmap);
		dcWorkDC.StretchBlt(0, 0, nDstSize, nDstSize, *m_pSymbleDC, 0, 0, nSrcSize, nSrcSize, SRCCOPY);
		dc.BitBlt(0, 0, nDstSize, nDstSize, dcWorkDC, 0, 0, SRCCOPY);
		dcWorkDC.SelectBitmap(hOldBitmap);
		dcWorkDC.DeleteDC();
	}
}


