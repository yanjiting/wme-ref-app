#include "QR_encode_mac.h"


CQR_Encode_mac::CQR_Encode_mac()
{
    img = NULL;
}

CQR_Encode_mac::~CQR_Encode_mac()
{
    if( NULL != img )
        CGImageRelease(img);
}

void CQR_Encode_mac::MakeImage()
{
    CGColorSpaceRef deviceGray = CGColorSpaceCreateDeviceGray();
    int bytesPerLine = ((8*m_nSymbleSize+3)/4)*4;
    CGContextRef bmpContext = CGBitmapContextCreate(NULL, m_nSymbleSize, m_nSymbleSize, 8, bytesPerLine, deviceGray, kCGImageAlphaNone);
    unsigned char *pImgData = (unsigned char *)CGBitmapContextGetData(bmpContext);
    for( int i = 0; i<m_nSymbleSize; i++ )
    {
        unsigned char *pLine = pImgData + (m_nSymbleSize - i - 1)*bytesPerLine;
        for (int j=0; j<m_nSymbleSize; j++) {
            pLine[j] = m_byModuleData[i][j]?0x0:0xff;
        }
    }
    if( NULL != img )
        CGImageRelease(img);
    img = CGBitmapContextCreateImage(bmpContext);
    CGContextRelease(bmpContext);
    CGColorSpaceRelease(deviceGray);
}

void CQR_Encode_mac::ShowImage(void* wnd)
{

}


