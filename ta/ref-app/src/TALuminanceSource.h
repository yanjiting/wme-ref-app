//
//  TALuminanceSource.h
//  MediaSessionTest

#ifndef __MediaSessionTest__TALuminanceSource__
#define __MediaSessionTest__TALuminanceSource__
#include <zxing/LuminanceSource.h>

namespace zxing {
class TALuminanceSource : public LuminanceSource
{
public:
    TALuminanceSource(void *pRawData, int w, int h, int dataType, int rows, int cols, int bytesPerPix);
    ~TALuminanceSource();
public:
    ArrayRef<char> getRow(int y, ArrayRef<char> row) const;
    ArrayRef<char> getMatrix() const;
    
    bool isCropSupported() const { return true; };
    Ref<LuminanceSource> crop(int left, int top, int width, int height) const;
    
//    bool isRotateSupported() const;
//    
//    Ref<LuminanceSource> invert() const;
//    
//    Ref<LuminanceSource> rotateCounterClockwise() const;
    protected:
    char *m_pRawData;
    int m_nDataType;
    int m_nRows;
    int m_nCols;
    int m_nBytesPerPix;
};
}

int DecodeQRCodeInFrame(std::vector<std::string> &result, void *pRawData, int w, int h, int dataType, int rows, int cols, int bytesPerPix);
extern char GrayScalePixel(const char *pixData, int type);

#endif /* defined(__MediaSessionTest__TALuminanceSource__) */
