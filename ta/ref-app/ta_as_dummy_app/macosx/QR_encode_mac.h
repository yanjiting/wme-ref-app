#if !defined _QR_ENCODE_MAC_H_
#define _QR_ENCODE_MAC_H_

#include "QR_Encode.h"

class CQR_Encode_mac : public CQR_Encode
{
public:
	CQR_Encode_mac();
	virtual~CQR_Encode_mac();

public:
	virtual void ShowImage(void* wnd);
    virtual void MakeImage();

    CGImageRef img;
};

/////////////////////////////////////////////////////////////////////////////

#endif // !defined _QR_ENCODE_MAC_H_
