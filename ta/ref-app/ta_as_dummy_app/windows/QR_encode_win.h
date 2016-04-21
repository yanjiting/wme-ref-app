#if !defined _QR_ENCODE_WIN_H_
#define _QR_ENCODE_WIN_H_

#include "QR_Encode.h"

class CQR_Encode_win : public CQR_Encode
{
public:
	CQR_Encode_win();
	virtual ~CQR_Encode_win();

public:
	virtual void ShowImage(void* wnd);
    virtual void MakeImage();

	CDC*      m_pSymbleDC;
	CBitmap*  m_pSymbleBitmap;
	CBitmap   m_OldBitmap;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined _QR_ENCODE_WIN_H_
